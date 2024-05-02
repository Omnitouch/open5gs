/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ogs-gtp.h"

#include "mme-event.h"
#include "mme-gtp-path.h"
#include "mme-path.h"
#include "s1ap-path.h"
#include "mme-s11-build.h"
#include "mme-sm.h"
#include "dns_resolvers.h"

static void _gtpv2_c_recv_cb(short when, ogs_socket_t fd, void *data)
{
    int rv;
    char buf[OGS_ADDRSTRLEN];

    ssize_t size;
    mme_event_t *e = NULL;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_sockaddr_t from;
    mme_sgw_t *sgw = NULL;

    ogs_assert(fd != INVALID_SOCKET);

    pkbuf = ogs_pkbuf_alloc(NULL, OGS_MAX_SDU_LEN);
    ogs_assert(pkbuf);
    ogs_pkbuf_put(pkbuf, OGS_MAX_SDU_LEN);

    size = ogs_recvfrom(fd, pkbuf->data, pkbuf->len, 0, &from);
    if (size <= 0) {
        ogs_log_message(OGS_LOG_ERROR, ogs_socket_errno,
                "ogs_recvfrom() failed");
        ogs_pkbuf_free(pkbuf);
        return;
    }

    ogs_pkbuf_trim(pkbuf, size);

    sgw = mme_sgw_find_by_addr(&from);
    if (!sgw) {
        sgw = mme_sgw_roaming_find_by_addr(&from);
    }

    if (!sgw) {
        ogs_error("Unknown SGW : %s", OGS_ADDR(&from, buf));
        ogs_pkbuf_free(pkbuf);
        return;
    }
    ogs_assert(sgw);

    e = mme_event_new(MME_EVENT_S11_MESSAGE);
    ogs_assert(e);
    e->gnode = (ogs_gtp_node_t *)sgw;
    e->pkbuf = pkbuf;

    rv = ogs_queue_push(ogs_app()->queue, e);
    if (rv != OGS_OK) {
        ogs_error("ogs_queue_push() failed:%d", (int)rv);
        ogs_pkbuf_free(e->pkbuf);
        mme_event_free(e);
    }
}

static void timeout(ogs_gtp_xact_t *xact, void *data)
{
    int r;
    mme_ue_t *mme_ue = NULL;
    enb_ue_t *enb_ue = NULL;
    mme_sess_t *sess = NULL;
    mme_bearer_t *bearer = NULL;
    uint8_t type = 0;

    xact = ogs_gtp_xact_cycle(xact);
    if (NULL == xact) {
        ogs_error("xact no longer valid");
        return;
    }
    type = xact->seq[0].type;

    switch (type) {
    case OGS_GTP2_MODIFY_BEARER_REQUEST_TYPE:
    case OGS_GTP2_RELEASE_ACCESS_BEARERS_REQUEST_TYPE:
    case OGS_GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
    case OGS_GTP2_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
        mme_ue = mme_ue_cycle(data);
        break;
    case OGS_GTP2_CREATE_SESSION_REQUEST_TYPE:
    case OGS_GTP2_DELETE_SESSION_REQUEST_TYPE:
        sess = mme_sess_cycle(data);
        mme_ue = sess ? mme_ue_cycle(sess->mme_ue) : NULL;
        break;
    case OGS_GTP2_BEARER_RESOURCE_COMMAND_TYPE:
        bearer = mme_bearer_cycle(data);
        sess = bearer ? mme_sess_cycle(bearer->sess) : NULL;
        mme_ue = sess ? mme_ue_cycle(sess->mme_ue) : NULL;
        break;
    default:
        ogs_fatal("Invalid type [%d]", type);
        ogs_assert_if_reached();
        break;
    }

    if (NULL == mme_ue) {
        ogs_error("Invalid context");
        return;
    }

    switch (type) {
    case OGS_GTP2_DELETE_SESSION_REQUEST_TYPE:
        /*
         * If SESSION_CONTEXT_WILL_DELETED(MME_UE) is not cleared,
         * The MME cannot send Delete-Session-Request to the SGW-C.
         * As such, it could be the infinite loop occurred in EMM state machine.
         *
         * To prevent this situation,
         * force clearing SESSION_CONTEXT_WILL_DELETED variable
         * when MME does not receive Delete-Session-Response message from SGW-C.
         */
        CLEAR_SESSION_CONTEXT(mme_ue);

        enb_ue = enb_ue_cycle(mme_ue->enb_ue);
        if (enb_ue) {
            r = s1ap_send_ue_context_release_command(enb_ue,
                    S1AP_Cause_PR_nas, S1AP_CauseNas_normal_release,
                    S1AP_UE_CTX_REL_UE_CONTEXT_REMOVE, 0);
            ogs_expect(r == OGS_OK);
            ogs_assert(r != OGS_ERROR);
        } else {
            ogs_warn("No S1 Context");
        }
        break;
    case OGS_GTP2_BEARER_RESOURCE_COMMAND_TYPE:
        /* Nothing to do */
        break;
    default:
        mme_send_delete_session_or_mme_ue_context_release(mme_ue);
        break;
    }

    ogs_error("GTP Timeout : IMSI[%s] Message-Type[%d]",
            mme_ue->imsi_bcd, type);
}

int mme_gtp_open(void)
{
    int rv;
    ogs_socknode_t *node = NULL;
    ogs_sock_t *sock = NULL;
    mme_sgw_t *sgw = NULL;

    ogs_list_for_each(&ogs_gtp_self()->gtpc_list, node) {
        sock = ogs_gtp_server(node);
        if (!sock) return OGS_ERROR;

        node->poll = ogs_pollset_add(ogs_app()->pollset,
                OGS_POLLIN, sock->fd, _gtpv2_c_recv_cb, sock);
        ogs_assert(node->poll);
    }
    ogs_list_for_each(&ogs_gtp_self()->gtpc_list6, node) {
        sock = ogs_gtp_server(node);
        if (!sock) return OGS_ERROR;

        node->poll = ogs_pollset_add(ogs_app()->pollset,
                OGS_POLLIN, sock->fd, _gtpv2_c_recv_cb, sock);
        ogs_assert(node->poll);
    }

    OGS_SETUP_GTPC_SERVER;

    ogs_list_for_each(&mme_self()->sgw_list, sgw) {
        rv = ogs_gtp_connect(
                ogs_gtp_self()->gtpc_sock, ogs_gtp_self()->gtpc_sock6,
                (ogs_gtp_node_t *)sgw);
        ogs_assert(rv == OGS_OK);
    }

    ogs_list_for_each(&mme_self()->sgw_roaming_list, sgw) {
        rv = ogs_gtp_connect(
                ogs_gtp_self()->gtpc_sock, ogs_gtp_self()->gtpc_sock6,
                (ogs_gtp_node_t *)sgw);
        ogs_assert(rv == OGS_OK);
    }

    return OGS_OK;
}

void mme_gtp_close(void)
{
    ogs_socknode_remove_all(&ogs_gtp_self()->gtpc_list);
    ogs_socknode_remove_all(&ogs_gtp_self()->gtpc_list6);
}

int mme_gtp_send_create_session_request(mme_sess_t *sess, int create_action)
{
    int rv;
    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;
    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;
    ogs_session_t *session = NULL;

    sess = mme_sess_cycle(sess);
    mme_ue = sess ? mme_ue_cycle(sess->mme_ue) : NULL;
    session = sess ? sess->session : NULL;
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;

    if ((NULL == sess)    ||
        (NULL == mme_ue)  ||
        (NULL == session))
    {
        ogs_error("Invalid context");
        return OGS_ERROR;
    }

    /* Pick SGW if one has not been chosen */
    if (NULL == sgw_ue) {
        mme_sgw_t *sgw = NULL;

        if (0 == strcmp(session->name, "sos")) {
            /* If APN is SOS then skip DNS lookup and assign SGW/PGW from local config */
            sgw = select_random_sgw();
        } else if (imsi_is_roaming(&mme_ue->nas_mobile_identity_imsi)) {
            sgw = select_random_sgw_roaming();
        } else if (mme_self()->dns_target_sgw) {
            char ipv4[INET_ADDRSTRLEN] = "";
            ResolverContext context = {};
            /* We do not include the APN for SGW lookups */
            snprintf(context.mnc, DNS_RESOLVERS_MAX_MNC_STR, "%03u", ogs_plmn_id_mnc(&mme_ue->tai.plmn_id));
            snprintf(context.mcc, DNS_RESOLVERS_MAX_MCC_STR, "%03u", ogs_plmn_id_mcc(&mme_ue->tai.plmn_id));
            strncpy(context.target, "sgw", DNS_RESOLVERS_MAX_TARGET_STR);
            strncpy(context.interface, "s11", DNS_RESOLVERS_MAX_INTERFACE_STR);
            strncpy(context.protocol, "gtp", DNS_RESOLVERS_MAX_PROTOCOL_STR);
            strncpy(context.domain_suffix, mme_self()->dns_base_domain, DNS_RESOLVERS_MAX_DOMAIN_SUFFIX_STR);

            if (true == resolve_naptr(&context, ipv4, INET_ADDRSTRLEN)) {
                ogs_sockaddr_t *sgw_addr = NULL;

                ogs_info("NAPTR resolve success, SGW address is '%s'", ipv4);

                ogs_addaddrinfo(
                    &sgw_addr,
                    AF_INET,
                    ipv4,
                    ogs_gtp_self()->gtpc_port,
                    0
                );

                if (NULL != sgw_addr) {
                    sgw = mme_sgw_find_by_addr(sgw_addr);

                    if (NULL == sgw) {
                        ogs_debug("Looks like we haven't used this SGW (%s) yet, lets add it and connect to it", ipv4);
                        sgw = mme_sgw_add(sgw_addr);
                        
                        rv = ogs_gtp_connect(
                            ogs_gtp_self()->gtpc_sock,
                            ogs_gtp_self()->gtpc_sock6,
                            (ogs_gtp_node_t *)sgw
                        );

                        if (OGS_OK != rv) {
                            ogs_error("Failed to connect to new SGW with address '%s'", ipv4);
                            mme_sgw_remove(sgw);
                            return OGS_ERROR;
                        }
                    }
                } else {
                    ogs_error("Failed to set SGW address to '%s', falling back to default selection method", ipv4);
                }
            }
            else {
                ogs_error("Failed to resolve dns and update SGW IP in CSR, falling back to default selection method");
            }
        }

        if (NULL == sgw) {
            sgw = select_random_sgw();
        }

        ogs_assert(sgw);
        sgw_ue = sgw_ue_add(sgw);
        ogs_assert(sgw_ue);
        ogs_assert(sgw_ue->gnode); /* sgw_ue->gnode is a union with the sgw_ue->sgw */
        sgw_ue_associate_mme_ue(sgw_ue, mme_ue);
    }
    ogs_assert(sgw_ue);

    /* If this is a SOS APN the we want to set the address in the session to a local PGW */
    if (0 == strcmp(session->name, "sos")) {
        /* The sessions PGW is of higher priority it will be the one chosen in mme_s11_build_create_session_request */
        if ((NULL == session->pgw_addr) && (NULL == session->pgw_addr6)) {
            session->pgw_addr = mme_pgw_addr_select_random(
                &mme_self()->pgw_list, AF_INET);
            // Currently only support ipv4 on pgw connection
            // session->pgw_addr6 = mme_pgw_addr_select_random(
            //     &mme_self()->pgw_list, AF_INET6);            
        }
    } else if ((NULL == session->pgw_addr) && (NULL == session->pgw_addr6)) {
        /* Pick PGW if one has not been chosen */

        if (mme_self()->dns_target_pgw) {
            bool resolved_dns = false;
            enum { MAX_MCC_MNC_STR = 4 };
            char ipv4[INET_ADDRSTRLEN] = "";
            ResolverContext context = {};
            char mme_mcc[MAX_MCC_MNC_STR] = "";
            char mme_mnc[MAX_MCC_MNC_STR] = "";
            char imsi_mcc[MAX_MCC_MNC_STR] = "000";
            char imsi_mnc_2[MAX_MCC_MNC_STR] = "000";
            char imsi_mnc_3[MAX_MCC_MNC_STR] = "000";
            
            /* Load MCC and MNC from config and format them */
            snprintf(mme_mcc, MAX_MCC_MNC_STR, "%03u", ogs_plmn_id_mcc(&mme_ue->tai.plmn_id));
            snprintf(mme_mnc, MAX_MCC_MNC_STR, "%03u", ogs_plmn_id_mnc(&mme_ue->tai.plmn_id));
            strncpy(context.apn, sess->session->name, DNS_RESOLVERS_MAX_APN_STR);
            strncpy(context.target, "pgw", DNS_RESOLVERS_MAX_TARGET_STR);
            strncpy(context.protocol, "gtp", DNS_RESOLVERS_MAX_PROTOCOL_STR);
            /* Load our domain suffix from the config */
            strncpy(context.domain_suffix, mme_self()->dns_base_domain, DNS_RESOLVERS_MAX_DOMAIN_SUFFIX_STR);

            memcpy(imsi_mcc, &mme_ue->imsi_bcd[0], 3);
            memcpy(&imsi_mnc_2[1], &mme_ue->imsi_bcd[3], 2);
            memcpy(imsi_mnc_3, &mme_ue->imsi_bcd[3], 3);

            strncpy(context.mcc, imsi_mcc, DNS_RESOLVERS_MAX_MCC_STR);

            if (imsi_is_roaming(&mme_ue->nas_mobile_identity_imsi)) {
                /* This is roaming, check roaming with a 3 digit MNC */
                strncpy(context.interface, "s8", DNS_RESOLVERS_MAX_INTERFACE_STR);
                strcpy(context.mnc, imsi_mnc_3);

                ogs_debug("Attempting NAPTR resolv for roming [MCC:%s] [MNC:%s]\n", context.mcc, context.mnc);
                if (true == resolve_naptr(&context, ipv4, INET_ADDRSTRLEN)) {
                    /* We resolved for roming */
                    resolved_dns = true;
                } else {
                    /* We failed to resolve with assumption of a 3 digit MNC,
                    * try the 2 digit MNC */
                    strcpy(context.mnc, imsi_mnc_2);

                    ogs_debug("Attempting NAPTR resolv for roming [MCC:%s] [MNC:%s]\n", context.mcc, context.mnc);
                    resolved_dns = resolve_naptr(&context, ipv4, INET_ADDRSTRLEN);
                }
            } else {
                /* Might be home, check home */
                strncpy(context.interface, "s5", DNS_RESOLVERS_MAX_INTERFACE_STR);
                strncpy(context.mnc, mme_mnc, DNS_RESOLVERS_MAX_MNC_STR);
                
                ogs_debug("Attempting NAPTR resolv for home [MCC:%s] [MNC:%s]\n", context.mcc, context.mnc);
                resolved_dns = resolve_naptr(&context, ipv4, INET_ADDRSTRLEN);
            }

            if (resolved_dns) {
                ogs_info("NAPTR resolve success, PGW address is '%s'", ipv4);
                ogs_addaddrinfo(
                    &session->pgw_addr,
                    AF_INET,
                    ipv4,
                    ogs_gtp_self()->gtpc_port,
                    0
                );
            } else {
                ogs_info("Failed to resolve dns and update PGW IP in CSR, cannot send Create Session Request");
            }
        } else {
            session->pgw_addr = mme_pgw_addr_select_random(
                &mme_self()->pgw_list, AF_INET);
            // Currently only support ipv4 on pgw connection
            // session->pgw_addr6 = mme_pgw_addr_select_random(
            //     &mme_self()->pgw_list, AF_INET6);
        }
    }

    if ((NULL == session->pgw_addr) && (NULL == session->pgw_addr6)) {
        /* If we failed to assign an address return error */
        ogs_error("Failed to assign a PGW address");
        return OGS_ERROR;
    }

    if (create_action == OGS_GTP_CREATE_IN_PATH_SWITCH_REQUEST) {
        sgw_ue = sgw_ue_cycle(sgw_ue->target_ue);
        ogs_assert(sgw_ue);
    }

    if ((true == imsi_is_roaming(&mme_ue->nas_mobile_identity_imsi)) && 
        (false == session->served_party_ip_address))
    {
        /* We don't want to use the same UE IP if we're roaming and IP isn't static */
        memset(&session->paa, 0, sizeof(session->paa));
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_CREATE_SESSION_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_create_session_request(h.type, sess, create_action);
    if (!pkbuf) {
        ogs_error("mme_s11_build_create_session_request() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, sess);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->create_action = create_action;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_modify_bearer_request(
        mme_ue_t *mme_ue, int uli_presence, int modify_action)
{
    int rv;

    ogs_gtp_xact_t *xact = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Trying to send a modify bearer request before create session request has been sent");
        ogs_error("\tuli_presence: %i, modify_action: %i", uli_presence, modify_action);
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_MODIFY_BEARER_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_modify_bearer_request(h.type, mme_ue, uli_presence);
    if (!pkbuf) {
        ogs_error("mme_s11_build_modify_bearer_request() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, mme_ue);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->modify_action = modify_action;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_delete_session_request(
        sgw_ue_t *sgw_ue, mme_sess_t *sess, int action)
{
    int rv;
    ogs_pkbuf_t *s11buf = NULL;
    ogs_gtp2_header_t h;
    ogs_gtp_xact_t *xact = NULL;
    mme_ue_t *mme_ue = NULL;

    ogs_assert(action);
    sess = mme_sess_cycle(sess);
    mme_ue = sess ? mme_ue_cycle(sess->mme_ue) : NULL;
    sgw_ue = sgw_ue_cycle(sgw_ue);

    if ((NULL == mme_ue) || (NULL == sgw_ue)) {
        /* If the sgw_ue was never set we don't need to do anything */
        ogs_warn("Invalid context");
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_DELETE_SESSION_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    s11buf = mme_s11_build_delete_session_request(h.type, sess, action);
    if (!s11buf) {
        ogs_error("mme_s11_build_delete_session_request() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, s11buf, timeout, sess);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->delete_action = action;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    /* If we make it here than that means we have successfully sent the delete session */
    mme_metrics_ue_session_clear(mme_ue->imsi_bcd, sess->session->name);

    return rv;
}

void mme_gtp_send_delete_all_sessions(mme_ue_t *mme_ue, int action)
{
    mme_sess_t *sess = NULL, *next_sess = NULL;
    sgw_ue_t *sgw_ue = NULL;

    mme_ue = mme_ue_cycle(mme_ue);
    if (NULL == mme_ue) {
        ogs_error("Trying to delete all sessions from mme_ue that doesn't exist!");
        return;
    }

    ogs_assert(action);
    
    sgw_ue = sgw_ue_cycle(mme_ue->sgw_ue);

    ogs_list_for_each_safe(&mme_ue->sess_list, next_sess, sess) {
        if (NULL == mme_sess_cycle(sess)) {
            ogs_error("Found a invalid sess in mme_ue->sess_list");
            break;
        }
        
        if (sgw_ue && MME_HAVE_SGW_S1U_PATH(sess)) {
            mme_gtp_send_delete_session_request(sgw_ue, sess, action);
        } else {
            mme_sess_remove(sess);
        }
    }
}

int mme_gtp_send_create_bearer_response(
        mme_bearer_t *bearer, uint8_t cause_value)
{
    int rv;

    ogs_gtp_xact_t *xact = NULL;
    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;

    bearer = mme_bearer_cycle(bearer);
    mme_ue = bearer ? mme_ue_cycle(bearer->mme_ue) : NULL;
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Invalid context");
        ogs_error("\tcause_value: %i", cause_value);
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_cycle(bearer->create.xact);
    if (!xact) {
        ogs_warn("GTP transaction(CREATE) has already been removed");
        return OGS_OK;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_CREATE_BEARER_RESPONSE_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_create_bearer_response(h.type, bearer, cause_value);
    if (!pkbuf) {
        ogs_error("mme_s11_build_create_bearer_response() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_update_tx(xact, &h, pkbuf);
    if (rv != OGS_OK) {
        ogs_error("ogs_gtp_xact_update_tx() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_update_bearer_response(
        mme_bearer_t *bearer, uint8_t cause_value)
{
    int rv;

    ogs_gtp_xact_t *xact = NULL;
    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;

    bearer = mme_bearer_cycle(bearer);
    mme_ue = bearer ? mme_ue_cycle(bearer->mme_ue) : NULL;
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Invalid context");
        ogs_error("\tcause_value: %i", cause_value);
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_cycle(bearer->update.xact);
    if (!xact) {
        ogs_warn("GTP transaction(UPDATE) has already been removed");
        return OGS_OK;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_UPDATE_BEARER_RESPONSE_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_update_bearer_response(h.type, bearer, cause_value);
    if (!pkbuf) {
        ogs_error("mme_s11_build_update_bearer_response() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_update_tx(xact, &h, pkbuf);
    if (rv != OGS_OK) {
        ogs_error("ogs_gtp_xact_update_tx() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_delete_bearer_response(
        mme_bearer_t *bearer, uint8_t cause_value)
{
    int rv;

    ogs_gtp_xact_t *xact = NULL;
    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;

    bearer = mme_bearer_cycle(bearer);
    mme_ue = bearer ? mme_ue_cycle(bearer->mme_ue) : NULL;
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Invalid context");
        ogs_error("\tcause_value: %i", cause_value);
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_cycle(bearer->delete.xact);
    if (!xact) {
        ogs_warn("GTP transaction(DELETE) has already been removed");
        return OGS_OK;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_DELETE_BEARER_RESPONSE_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_delete_bearer_response(h.type, bearer, cause_value);
    if (!pkbuf) {
        ogs_error("mme_s11_build_delete_bearer_response() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_update_tx(xact, &h, pkbuf);
    if (rv != OGS_OK) {
        ogs_error("ogs_gtp_xact_update_tx() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_release_access_bearers_request(mme_ue_t *mme_ue, int action)
{
    int rv;
    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_assert(action);
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Trying to send a release access bearers request before create session request has been sent");
        ogs_error("\taction: %i", action);
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_RELEASE_ACCESS_BEARERS_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_release_access_bearers_request(h.type);
    if (!pkbuf) {
        ogs_error("mme_s11_build_release_access_bearers_request() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, mme_ue);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->release_action = action;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

void mme_gtp_send_release_all_ue_in_enb(mme_enb_t *enb, int action)
{
    mme_ue_t *mme_ue = NULL;
    enb_ue_t *enb_ue = NULL, *next = NULL;

    ogs_list_for_each_safe(&enb->enb_ue_list, next, enb_ue) {
        enb_ue = enb_ue_cycle(enb_ue);
        mme_ue = enb_ue ? mme_ue_cycle(enb_ue->mme_ue) : NULL;

        if (mme_ue && sgw_ue_cycle(mme_ue->sgw_ue)) {
            if (action == OGS_GTP_RELEASE_S1_CONTEXT_REMOVE_BY_LO_CONNREFUSED) {
                /*
                 * https://github.com/open5gs/open5gs/pull/1497
                 *
                 * 1. eNB, SGW-U and UPF go offline at the same time.
                 * 2. MME sends Release Access Bearer Request to SGW-C
                 * 3. SGW-C/SMF sends PFCP modification,
                 *    but SGW-U/UPF does not respond.
                 * 4. MME does not receive Release Access Bearer Response.
                 * 5. timeout()
                 * 6. MME sends Delete Session Request to the SGW-C/SMF
                 * 7. No SGW-U/UPF, so timeout()
                 * 8. MME sends UEContextReleaseRequest enb_ue.
                 * 9. But there is no enb_ue, so MME crashed.
                 *
                 * To solve this situation,
                 * Execute enb_ue_unlink(mme_ue) and enb_ue_remove(enb_ue)
                 * before mme_gtp_send_release_access_bearers_request()
                 */
                enb_ue_unlink(mme_ue);
                enb_ue_remove(enb_ue);
            }

            ogs_assert(OGS_OK ==
                mme_gtp_send_release_access_bearers_request(mme_ue, action));
        } else {
            ogs_warn("mme_gtp_send_release_all_ue_in_enb()");
            ogs_warn("    ENB_UE_S1AP_ID[%d] MME_UE_S1AP_ID[%d] Action[%d]",
                enb_ue->enb_ue_s1ap_id, enb_ue->mme_ue_s1ap_id, action);

            if (action == OGS_GTP_RELEASE_S1_CONTEXT_REMOVE_BY_LO_CONNREFUSED ||
                action == OGS_GTP_RELEASE_S1_CONTEXT_REMOVE_BY_RESET_ALL) {
                enb_ue_remove(enb_ue);
            } else {
                /* At this point, it does not support other action */
                ogs_assert_if_reached();
            }
        }
    }
}

int mme_gtp_send_downlink_data_notification_ack(
        mme_bearer_t *bearer, uint8_t cause_value)
{
    int rv;
    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;
    ogs_gtp_xact_t *xact = NULL;

    ogs_gtp2_header_t h;
    ogs_pkbuf_t *s11buf = NULL;

    bearer = mme_bearer_cycle(bearer);
    if (NULL == bearer) {
        ogs_error("Invalid context");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_cycle(bearer->notify.xact);
    if (!xact) {
        ogs_warn("GTP transaction(NOTIFY) has already been removed");
        return OGS_OK;
    }
    
    mme_ue = mme_ue_cycle(bearer->mme_ue);
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;
    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Invalid context");
        ogs_error("\tcause_value: %i", cause_value);
        return OGS_ERROR;
    }

    /* Build Downlink data notification ack */
    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_DOWNLINK_DATA_NOTIFICATION_ACKNOWLEDGE_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    s11buf = mme_s11_build_downlink_data_notification_ack(h.type, cause_value);
    if (!s11buf) {
        ogs_error("mme_s11_build_downlink_data_notification_ack() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_update_tx(xact, &h, s11buf);
    if (rv != OGS_OK) {
        ogs_error("ogs_gtp_xact_update_tx() failed");
        return OGS_ERROR;
    }

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_create_indirect_data_forwarding_tunnel_request(
        mme_ue_t *mme_ue)
{
    int rv;
    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Trying to send a create indirect data forwarding tunnel request before create session request has been sent");
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_create_indirect_data_forwarding_tunnel_request(
            h.type, mme_ue);
    if (!pkbuf) {
        ogs_error("mme_s11_build_create_indirect_data_forwarding_"
                "tunnel_request() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, mme_ue);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_delete_indirect_data_forwarding_tunnel_request(
        mme_ue_t *mme_ue, int action)
{
    int rv;
    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;
    sgw_ue_t *sgw_ue = NULL;

    ogs_assert(action);
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;

    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Trying to send a delete indirect data forwarding tunnel request before create session request has been sent");
        ogs_error("\taction: %i", action);
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = ogs_pkbuf_alloc(NULL, OGS_TLV_MAX_HEADROOM);
    if (!pkbuf) {
        ogs_error("ogs_pkbuf_alloc() failed");
        return OGS_ERROR;
    }
    ogs_pkbuf_reserve(pkbuf, OGS_TLV_MAX_HEADROOM);

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, mme_ue);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->delete_indirect_action = action;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int mme_gtp_send_bearer_resource_command(
        mme_bearer_t *bearer, ogs_nas_eps_message_t *nas_message)
{
    int rv;
    ogs_gtp2_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;

    mme_ue_t *mme_ue = NULL;
    sgw_ue_t *sgw_ue = NULL;

    bearer = mme_bearer_cycle(bearer);
    mme_ue = bearer ? mme_ue_cycle(bearer->mme_ue) : NULL;
    sgw_ue = mme_ue ? sgw_ue_cycle(mme_ue->sgw_ue) : NULL;
    
    if (NULL == sgw_ue) {
        /* sgw_ue is set in mme_gtp_send_create_session_request */
        ogs_error("Invalid context");
        return OGS_ERROR;
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_BEARER_RESOURCE_COMMAND_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_bearer_resource_command(h.type, bearer, nas_message);
    if (!pkbuf) {
        ogs_error("mme_s11_build_bearer_resource_command() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp_xact_local_create(sgw_ue->gnode, &h, pkbuf, timeout, bearer);
    if (!xact) {
        ogs_error("ogs_gtp_xact_local_create() failed");
        return OGS_ERROR;
    }
    xact->xid |= OGS_GTP_CMD_XACT_ID;
    xact->local_teid = mme_ue->mme_s11_teid;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}
