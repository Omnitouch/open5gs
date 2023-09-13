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
#include "mme-gn-build.h"
#include "mme-gtp-path.h"
#include "mme-path.h"
#include "s1ap-path.h"
#include "mme-s11-build.h"
#include "mme-sm.h"
#include "dns_resolvers.h"

static void _gtpv1v2_c_recv_cb(short when, ogs_socket_t fd, void *data)
{
    int rv;
    char buf[OGS_ADDRSTRLEN];

    ssize_t size;
    mme_event_t *e = NULL;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_sockaddr_t from;
    mme_sgw_t *sgw = NULL;
    mme_sgsn_t *sgsn = NULL;
    uint8_t gtp_ver;

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

    gtp_ver = ((ogs_gtp2_header_t *)pkbuf->data)->version;
    switch (gtp_ver) {
    case 1:
        sgsn = mme_sgsn_find_by_addr(&from);
        if (!sgsn) {
            ogs_error("Unknown SGSN : %s", OGS_ADDR(&from, buf));
            ogs_pkbuf_free(pkbuf);
            return;
        }
        ogs_assert(sgsn);
        e = mme_event_new(MME_EVENT_GN_MESSAGE);
        ogs_assert(e);
        e->gnode = &sgsn->gnode;
        break;
    case 2:
        sgw = mme_sgw_find_by_addr(&from);
        if (!sgw) {
            ogs_error("Unknown SGW : %s", OGS_ADDR(&from, buf));
            ogs_pkbuf_free(pkbuf);
            return;
        }
        ogs_assert(sgw);
        e = mme_event_new(MME_EVENT_S11_MESSAGE);
        ogs_assert(e);
        e->gnode = &sgw->gnode;
        break;
    default:
        ogs_warn("Rx unexpected GTP version %u", gtp_ver);
        ogs_pkbuf_free(pkbuf);
        return;
    }

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

    ogs_assert(xact);
    type = xact->seq[0].type;

    switch (type) {
    case OGS_GTP2_MODIFY_BEARER_REQUEST_TYPE:
    case OGS_GTP2_RELEASE_ACCESS_BEARERS_REQUEST_TYPE:
    case OGS_GTP2_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
    case OGS_GTP2_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
        mme_ue = data;
        ogs_assert(mme_ue);
        break;
    case OGS_GTP2_CREATE_SESSION_REQUEST_TYPE:
    case OGS_GTP2_DELETE_SESSION_REQUEST_TYPE:
        sess = data;
        ogs_assert(sess);
        mme_ue = sess->mme_ue;
        ogs_assert(mme_ue);
        break;
    case OGS_GTP2_BEARER_RESOURCE_COMMAND_TYPE:
        bearer = data;
        ogs_assert(bearer);
        sess = bearer->sess;
        ogs_assert(sess);
        mme_ue = sess->mme_ue;
        ogs_assert(mme_ue);
        break;
    default:
        ogs_fatal("Invalid type [%d]", type);
        ogs_assert_if_reached();
        break;
    }

    ogs_assert(mme_ue);

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
    mme_sgsn_t *sgsn = NULL;

    ogs_list_for_each(&ogs_gtp_self()->gtpc_list, node) {
        sock = ogs_gtp_server(node);
        if (!sock) return OGS_ERROR;

        node->poll = ogs_pollset_add(ogs_app()->pollset,
                OGS_POLLIN, sock->fd, _gtpv1v2_c_recv_cb, sock);
        ogs_assert(node->poll);
    }
    ogs_list_for_each(&ogs_gtp_self()->gtpc_list6, node) {
        sock = ogs_gtp_server(node);
        if (!sock) return OGS_ERROR;

        node->poll = ogs_pollset_add(ogs_app()->pollset,
                OGS_POLLIN, sock->fd, _gtpv1v2_c_recv_cb, sock);
        ogs_assert(node->poll);
    }

    OGS_SETUP_GTPC_SERVER;

    ogs_list_for_each(&mme_self()->sgw_list, sgw) {
        rv = ogs_gtp_connect(
                ogs_gtp_self()->gtpc_sock, ogs_gtp_self()->gtpc_sock6,
                &sgw->gnode);
        ogs_assert(rv == OGS_OK);
    }

    ogs_list_for_each(&mme_self()->sgsn_list, sgsn) {
        rv = ogs_gtp_connect(
                ogs_gtp_self()->gtpc_sock, ogs_gtp_self()->gtpc_sock6,
                &sgsn->gnode);
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

    mme_ue = sess->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = sgw_ue_cycle(mme_ue->sgw_ue);
    ogs_assert(sgw_ue);

    if (create_action == OGS_GTP_CREATE_IN_PATH_SWITCH_REQUEST) {
        sgw_ue = sgw_ue_cycle(sgw_ue->target_ue);
        ogs_assert(sgw_ue);
    }

    memset(&h, 0, sizeof(ogs_gtp2_header_t));
    h.type = OGS_GTP2_CREATE_SESSION_REQUEST_TYPE;
    h.teid = sgw_ue->sgw_s11_teid;

    pkbuf = mme_s11_build_create_session_request(h.type, sess, create_action);
    if (!pkbuf) {
        ogs_error("mme_s11_build_create_session_request() failed");
        return OGS_ERROR;
    }

    if (mme_self()->dns_target_sgw) {
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
            /* Clobber the SGW addr if we resolved */
            ogs_info("Successfully clobbered the SGW IP in CSR with '%s'", ipv4);
            ogs_ipv4_from_string((uint32_t*)&sgw_ue->gnode->addr.sa.sa_data[2], ipv4);
        }
        else {
            ogs_warn("Failed to resolve dns and update SGW IP in CSR");
        }
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
    ogs_assert(sgw_ue);

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
    ogs_assert(sess);
    mme_ue = sess->mme_ue;
    ogs_assert(mme_ue);
    ogs_assert(sgw_ue);

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

    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);
    ogs_assert(action);

    ogs_list_for_each_safe(&mme_ue->sess_list, next_sess, sess) {
        if (MME_HAVE_SGW_S1U_PATH(sess)) {
            mme_gtp_send_delete_session_request(sgw_ue, sess, action);
        } else {
            MME_SESS_CLEAR(sess);
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

    ogs_assert(bearer);
    mme_ue = bearer->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);
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

    ogs_assert(bearer);
    mme_ue = bearer->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);
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

    ogs_assert(bearer);
    mme_ue = bearer->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);
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
    ogs_assert(sgw_ue);

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
        mme_ue = enb_ue->mme_ue;

        if (mme_ue) {
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

    ogs_assert(bearer);
    xact = ogs_gtp_xact_cycle(bearer->notify.xact);
    if (!xact) {
        ogs_warn("GTP transaction(NOTIFY) has already been removed");
        return OGS_OK;
    }
    mme_ue = bearer->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);

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
    ogs_assert(sgw_ue);

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
    ogs_assert(sgw_ue);

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

    ogs_assert(bearer);
    mme_ue = bearer->mme_ue;
    ogs_assert(mme_ue);
    sgw_ue = mme_ue->sgw_ue;
    ogs_assert(sgw_ue);

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


int mme_gtp1_send_ran_information_relay(
        mme_sgsn_t *sgsn, const uint8_t *buf, size_t len,
        const ogs_nas_rai_t *rai, uint16_t cell_id)
{
    int rv;
    ogs_gtp1_header_t h;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_gtp_xact_t *xact = NULL;

    ogs_assert(sgsn);
    ogs_assert(buf);

    memset(&h, 0, sizeof(ogs_gtp1_header_t));
    h.type = OGS_GTP1_RAN_INFORMATION_RELAY_TYPE;
    h.teid = 0;

    pkbuf = mme_gn_build_ran_information_relay(h.type, buf, len, rai, cell_id);
    if (!pkbuf) {
        ogs_error("mme_gn_build_ran_information_relay() failed");
        return OGS_ERROR;
    }

    xact = ogs_gtp1_xact_local_create(&sgsn->gnode, &h, pkbuf, NULL, NULL);
    if (!xact) {
        ogs_error("ogs_gtp1_xact_local_create() failed");
        return OGS_ERROR;
    }
    /* TS 29.060 8.2: "The RAN Information Relay message, where the Tunnel
     * Endpoint Identifier shall be set to all zeroes." */
    xact->local_teid = 0;

    rv = ogs_gtp_xact_commit(xact);
    ogs_expect(rv == OGS_OK);

    return rv;
}
