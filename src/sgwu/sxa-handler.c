/*
 * Copyright (C) 2019-2023 by Sukchan Lee <acetcom@gmail.com>
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

#include "context.h"
#include "pfcp-path.h"
#include "gtp-path.h"
#include "sxa-handler.h"

static void sgwu_sxa_handle_create_urr(sgwu_sess_t *sess, ogs_pfcp_tlv_create_urr_t *create_urr_arr,
                              uint8_t *cause_value, uint8_t *offending_ie_value)
{
    int i;
    ogs_pfcp_urr_t *urr;

    *cause_value = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    for (i = 0; i < OGS_MAX_NUM_OF_URR; i++) {
        urr = ogs_pfcp_handle_create_urr(&sess->pfcp, &create_urr_arr[i],
                    cause_value, offending_ie_value);
        if (!urr)
            return;
        
        /* TODO: enable counters somewhere else if ISTM not set, upon first pkt received */
        if (urr->meas_info.istm) {
            sgwu_sess_urr_acc_timers_setup(sess, urr);
        }
    }
}

void sgwu_sxa_handle_session_establishment_request(
        sgwu_sess_t *sess, ogs_pfcp_xact_t *xact, 
        ogs_pfcp_session_establishment_request_t *req)
{
    ogs_pfcp_pdr_t *pdr = NULL;
    ogs_pfcp_far_t *far = NULL;
    ogs_pfcp_pdr_t *created_pdr[OGS_MAX_NUM_OF_PDR];
    int num_of_created_pdr = 0;
    uint8_t cause_value = 0;
    uint8_t offending_ie_value = 0;
    int i;

    ogs_pfcp_sereq_flags_t sereq_flags;
    bool restoration_indication = false;

    ogs_assert(xact);
    ogs_assert(req);

    ogs_debug("Session Establishment Request");

    cause_value = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    sess = sgwu_sess_cycle(sess);
    if (!sess) {
        ogs_error("No Context");
        ogs_pfcp_send_error_message(xact, 0,
                OGS_PFCP_SESSION_ESTABLISHMENT_RESPONSE_TYPE,
                OGS_PFCP_CAUSE_MANDATORY_IE_MISSING, 0);
        return;
    }

    /* PFCPSEReq-Flags */
    memset(&sereq_flags, 0, sizeof(sereq_flags));
    if (req->pfcpsereq_flags.presence == 1) {
        sereq_flags.value = req->pfcpsereq_flags.u8;
    }

    for (i = 0; i < OGS_MAX_NUM_OF_PDR; i++) {
        created_pdr[i] = ogs_pfcp_handle_create_pdr(&sess->pfcp,
                &req->create_pdr[i], &sereq_flags,
                &cause_value, &offending_ie_value);
        if (created_pdr[i] == NULL)
            break;
    }
    num_of_created_pdr = i;
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_FAR; i++) {
        if (ogs_pfcp_handle_create_far(&sess->pfcp, &req->create_far[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    sgwu_sxa_handle_create_urr(sess, &req->create_urr[0], &cause_value, &offending_ie_value);
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_QER; i++) {
        if (ogs_pfcp_handle_create_qer(&sess->pfcp, &req->create_qer[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    ogs_pfcp_handle_create_bar(&sess->pfcp, &req->create_bar,
                &cause_value, &offending_ie_value);
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    /* PFCPSEReq-Flags */
    if (sereq_flags.restoration_indication == 1) {
        for (i = 0; i < num_of_created_pdr; i++) {
            pdr = created_pdr[i];
            ogs_assert(pdr);

            if (pdr->f_teid_len)
                ogs_pfcp_pdr_swap_teid(pdr);
        }
        restoration_indication = true;
    }

    /* Setup GTP Node */
    ogs_list_for_each(&sess->pfcp.far_list, far) {
        ogs_assert(OGS_ERROR != ogs_pfcp_setup_far_gtpu_node(far));
        if (far->gnode)
            ogs_pfcp_far_f_teid_hash_set(far);
    }

    for (i = 0; i < num_of_created_pdr; i++) {
        pdr = created_pdr[i];
        ogs_assert(pdr);

        /* Setup TEID Hash */
        if (pdr->f_teid_len)
            ogs_pfcp_object_teid_hash_set(
                    OGS_PFCP_OBJ_PDR_TYPE, pdr, restoration_indication);
    }

    /* Send Buffered Packet to gNB */
    ogs_list_for_each(&sess->pfcp.pdr_list, pdr) {
        if (pdr->src_if == OGS_PFCP_INTERFACE_CORE) { /* Downlink */
            ogs_pfcp_send_buffered_packet(pdr);
        }
    }

    if (restoration_indication == true ||
        ogs_pfcp_self()->up_function_features.ftup == 0)
        ogs_assert(OGS_OK ==
            sgwu_pfcp_send_session_establishment_response(
                xact, sess, NULL, 0));
    else
        ogs_assert(OGS_OK ==
            sgwu_pfcp_send_session_establishment_response(
                xact, sess, created_pdr, num_of_created_pdr));
    return;

cleanup:
    ogs_pfcp_sess_clear(&sess->pfcp);
    ogs_pfcp_send_error_message(xact, sess ? sess->sgwu_sxa_seid : 0,
            OGS_PFCP_SESSION_ESTABLISHMENT_RESPONSE_TYPE,
            cause_value, offending_ie_value);
}

void sgwu_sxa_handle_session_modification_request(
        sgwu_sess_t *sess, ogs_pfcp_xact_t *xact, 
        ogs_pfcp_session_modification_request_t *req)
{
    ogs_pfcp_pdr_t *pdr = NULL;
    ogs_pfcp_far_t *far = NULL;
    ogs_pfcp_pdr_t *created_pdr[OGS_MAX_NUM_OF_PDR];
    int num_of_created_pdr = 0;
    ogs_pfcp_user_plane_report_t report = {};
    uint8_t cause_value = 0;
    uint8_t offending_ie_value = 0;
    int i;

    ogs_assert(xact);
    ogs_assert(req);

    ogs_debug("Session Modification Request");

    cause_value = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    sess = sgwu_sess_cycle(sess);
    if (!sess) {
        ogs_error("No Context");
        ogs_pfcp_send_error_message(xact, 0,
                OGS_PFCP_SESSION_MODIFICATION_RESPONSE_TYPE,
                OGS_PFCP_CAUSE_SESSION_CONTEXT_NOT_FOUND, 0);
        return;
    }

    /* Copy all the latest info on the usage of the URRs before removing them */
    report.type.usage_report = 1;
    for (i = 0; i < OGS_MAX_NUM_OF_URR; i++) {
        if (req->remove_urr[i].presence) {
            if (req->remove_urr[i].urr_id.presence) {
                ogs_pfcp_urr_t *urr = ogs_pfcp_urr_find(&sess->pfcp, req->remove_urr->urr_id.u32);

                if (NULL != urr) {
                    sgwu_sess_urr_acc_fill_usage_report(sess, urr, &report, report.num_of_usage_report);
                    report.usage_report[report.num_of_usage_report].rep_trigger.termination_report = 1;
                    report.num_of_usage_report++;
                    sgwu_sess_urr_acc_snapshot(sess, urr);
                } else {
                    ogs_error("Could not remove URR with id %d as it could not be found", req->remove_urr->urr_id.u32);
                }

            } else {
                ogs_error("Cannot remove a URR if no id was provided");
            }
        }

        if (report.num_of_usage_report == 8) {
            ogs_error("Cant make enough final usage reports for all these URRs");
            break;
        }
    }

    for (i = 0; i < OGS_MAX_NUM_OF_PDR; i++) {
        created_pdr[i] = ogs_pfcp_handle_create_pdr(&sess->pfcp,
                &req->create_pdr[i], NULL, &cause_value, &offending_ie_value);
        if (created_pdr[i] == NULL)
            break;
    }
    num_of_created_pdr = i;
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_PDR; i++) {
        if (ogs_pfcp_handle_remove_pdr(&sess->pfcp, &req->remove_pdr[i],
                &cause_value, &offending_ie_value) == false)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_FAR; i++) {
        if (ogs_pfcp_handle_create_far(&sess->pfcp, &req->create_far[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_FAR; i++) {
        if (ogs_pfcp_handle_update_far_flags(&sess->pfcp, &req->update_far[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    sgwu_sxa_handle_create_urr(sess, &req->create_urr[0], &cause_value, &offending_ie_value);
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED) {
        ogs_error("Failed to handle create URR!");
        goto cleanup;
    }

    /* Send End Marker to gNB */
    ogs_list_for_each(&sess->pfcp.pdr_list, pdr) {
        if (pdr->src_if == OGS_PFCP_INTERFACE_CORE) { /* Downlink */
            far = pdr->far;
            if (far && far->smreq_flags.send_end_marker_packets)
                ogs_assert(OGS_ERROR != ogs_pfcp_send_end_marker(pdr));
        }
    }
    /* Clear PFCPSMReq-Flags */
    ogs_list_for_each(&sess->pfcp.far_list, far)
        far->smreq_flags.value = 0;

    for (i = 0; i < OGS_MAX_NUM_OF_FAR; i++) {
        if (ogs_pfcp_handle_update_far(&sess->pfcp, &req->update_far[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_FAR; i++) {
        if (ogs_pfcp_handle_remove_far(&sess->pfcp, &req->remove_far[i],
                &cause_value, &offending_ie_value) == false)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_QER; i++) {
        if (ogs_pfcp_handle_create_qer(&sess->pfcp, &req->create_qer[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_QER; i++) {
        if (ogs_pfcp_handle_update_qer(&sess->pfcp, &req->update_qer[i],
                    &cause_value, &offending_ie_value) == NULL)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    for (i = 0; i < OGS_MAX_NUM_OF_QER; i++) {
        if (ogs_pfcp_handle_remove_qer(&sess->pfcp, &req->remove_qer[i],
                &cause_value, &offending_ie_value) == false)
            break;
    }
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    ogs_pfcp_handle_create_bar(&sess->pfcp, &req->create_bar,
                &cause_value, &offending_ie_value);
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    ogs_pfcp_handle_remove_bar(&sess->pfcp, &req->remove_bar,
            &cause_value, &offending_ie_value);
    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED)
        goto cleanup;

    /* Setup GTP Node */
    ogs_list_for_each(&sess->pfcp.far_list, far) {
        ogs_assert(OGS_ERROR != ogs_pfcp_setup_far_gtpu_node(far));
        if (far->gnode)
            ogs_pfcp_far_f_teid_hash_set(far);
    }

    for (i = 0; i < num_of_created_pdr; i++) {
        pdr = created_pdr[i];
        ogs_assert(pdr);

        /* Setup TEID Hash */
        if (pdr->f_teid_len)
            ogs_pfcp_object_teid_hash_set(OGS_PFCP_OBJ_PDR_TYPE, pdr, false);
    }

    /* Send Buffered Packet to gNB */
    ogs_list_for_each(&sess->pfcp.pdr_list, pdr) {
        if (pdr->src_if == OGS_PFCP_INTERFACE_CORE) { /* Downlink */
            ogs_pfcp_send_buffered_packet(pdr);
        }
    }

    if (ogs_pfcp_self()->up_function_features.ftup == 0)
        ogs_assert(OGS_OK ==
            sgwu_pfcp_send_session_modification_response(
                xact, sess, NULL, 0, NULL));
    else
        ogs_assert(OGS_OK ==
            sgwu_pfcp_send_session_modification_response(
                xact, sess, created_pdr, num_of_created_pdr, &report));
    return;

cleanup:
    ogs_pfcp_sess_clear(&sess->pfcp);
    ogs_pfcp_send_error_message(xact, sess ? sess->sgwu_sxa_seid : 0,
            OGS_PFCP_SESSION_MODIFICATION_RESPONSE_TYPE,
            cause_value, offending_ie_value);
}

void sgwu_sxa_handle_session_deletion_request(
        sgwu_sess_t *sess, ogs_pfcp_xact_t *xact,
        ogs_pfcp_session_deletion_request_t *req)
{
    ogs_assert(xact);
    ogs_assert(req);

    ogs_debug("Session Deletion Request");

    if (!sess) {
        ogs_error("No Context");
        ogs_pfcp_send_error_message(xact, 0,
                OGS_PFCP_SESSION_DELETION_RESPONSE_TYPE,
                OGS_PFCP_CAUSE_SESSION_CONTEXT_NOT_FOUND, 0);
        return;
    }

    ogs_assert(sess);

    sgwu_pfcp_send_session_deletion_response(xact, sess);

    sgwu_sess_remove(sess);
}

void sgwu_sxa_handle_session_report_response(
        sgwu_sess_t *sess, ogs_pfcp_xact_t *xact,
        ogs_pfcp_session_report_response_t *rsp)
{
    uint8_t cause_value = 0;

    ogs_assert(xact);
    ogs_assert(rsp);

    ogs_pfcp_xact_commit(xact);

    ogs_debug("Session report resopnse");

    cause_value = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    if (!sess) {
        ogs_warn("No Context");
        cause_value = OGS_PFCP_CAUSE_SESSION_CONTEXT_NOT_FOUND;
    }

    if (rsp->cause.presence) {
        if (rsp->cause.u8 != OGS_PFCP_CAUSE_REQUEST_ACCEPTED) {
            ogs_error("PFCP Cause[%d] : Not Accepted", rsp->cause.u8);
            cause_value = rsp->cause.u8;
        }
    } else {
        ogs_error("No Cause");
        cause_value = OGS_PFCP_CAUSE_MANDATORY_IE_MISSING;
    }

    if (cause_value != OGS_PFCP_CAUSE_REQUEST_ACCEPTED) {
        ogs_error("Cause request not accepted[%d]", cause_value);
        sgwu_metrics_inst_global_inc(SGWU_METR_GLOB_CTR_SM_REPORTPFCPSESSIONFAIL);
        return;
    } else {
        sgwu_metrics_inst_global_inc(SGWU_METR_GLOB_CTR_SM_REPORTPFCPSESSIONSUCC);
    }
}
