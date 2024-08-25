/*
 * Copyright (C) 2024 by Ryan Dimsey <ryan@omnitouch.com.au>
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

#include "sgwc-usage-logger.h"

void log_deletion_usage_reports_session_deletion_response(sgwc_sess_t *sess, ogs_pfcp_session_deletion_response_t *pfcp_rsp) {
    int i = 0;
    sgwc_ue_t *sgwc_ue = NULL;
    sgwc_bearer_t *bearer = NULL;
    
    sess = sgwc_sess_cycle(sess);
    ogs_assert(sess);
    sgwc_ue = sgwc_ue_cycle(sess->sgwc_ue);
    if (NULL == sgwc_ue) {
        ogs_error("Failed to log_deletion_usage_reports_session_deletion_response as sgwc_ue does not exist!");
        return;
    }

    for (i = 0; i < OGS_ARRAY_SIZE(pfcp_rsp->usage_report); i++) {
        ogs_pfcp_tlv_usage_report_session_deletion_response_t *usage_report =
            &pfcp_rsp->usage_report[i];

        ogs_pfcp_volume_measurement_t volume;
        UsageLoggerData usageLoggerData = {0};

        if (0 == usage_report->presence) {
            /* We have reached the end of the usage_report list */
            break;
        }

        if (0 == usage_report->urr_id.presence) {
            ogs_error("Usage report URR has no ID field!");
            continue;
        }

        bearer = sgwc_bearer_cycle(
            sgwc_bearer_find_by_sess_urr_id(sess, usage_report->urr_id.u32));
        
        if (NULL == bearer) {
            ogs_error("Could not find bearer for usage report with id %d!", usage_report->urr_id.u32);
            continue;
        }

        if (0 == usage_report->volume_measurement.presence) {
            ogs_error("No volume measurements in usage report!");
            continue;
        }

        ogs_pfcp_parse_volume_measurement(&volume, &usage_report->volume_measurement);
        if (0 == volume.ulvol) {
            ogs_error("URR did not contain uplink volume measurement!");
            continue;
        } 
        if (0 == volume.dlvol) {
            ogs_error("URR did not contain downlink volume measurement!");
            continue;
        }

        usageLoggerData = build_usage_logger_data(bearer, "end", volume.uplink_volume, volume.downlink_volume);
        log_usage_logger_data(usageLoggerData);
    }
}

void log_deletion_usage_reports_session_modification_response(sgwc_sess_t *sess, ogs_pfcp_session_modification_response_t *pfcp_rsp) {
    int i = 0;
    sgwc_ue_t *sgwc_ue = NULL;
    sgwc_bearer_t *bearer = NULL;
    
    sess = sgwc_sess_cycle(sess);
    ogs_assert(sess);
    sgwc_ue = sgwc_ue_cycle(sess->sgwc_ue);
    if (NULL == sgwc_ue) {
        ogs_error("Failed to log_deletion_usage_reports_session_modification_response as sgwc_ue does not exist!");
        return;
    }

    for (i = 0; i < OGS_ARRAY_SIZE(pfcp_rsp->usage_report); i++) {
        ogs_pfcp_tlv_usage_report_session_modification_response_t *usage_report =
            &pfcp_rsp->usage_report[i];

        ogs_pfcp_volume_measurement_t volume;
        UsageLoggerData usageLoggerData = {0};

        if (0 == usage_report->presence) {
            /* We have reached the end of the usage_report list */
            break;
        }

        if (0 == usage_report->urr_id.presence) {
            ogs_error("Usage report URR has no ID field!");
            continue;
        }

        bearer = sgwc_bearer_cycle(
            sgwc_bearer_find_by_sess_urr_id(sess, usage_report->urr_id.u32));
        
        if (NULL == bearer) {
            ogs_error("Could not find bearer for usage report with id %d!", usage_report->urr_id.u32);
            continue;
        }

        if (0 == usage_report->volume_measurement.presence) {
            ogs_error("No volume measurements in usage report!");
            continue;
        }

        ogs_pfcp_parse_volume_measurement(&volume, &usage_report->volume_measurement);
        if (0 == volume.ulvol) {
            ogs_error("URR did not contain uplink volume measurement!");
            continue;
        } 
        if (0 == volume.dlvol) {
            ogs_error("URR did not contain downlink volume measurement!");
            continue;
        }

        usageLoggerData = build_usage_logger_data(bearer, "end", volume.uplink_volume, volume.downlink_volume);
        log_usage_logger_data(usageLoggerData);
    }
}

UsageLoggerData build_usage_logger_data(sgwc_bearer_t *bearer, char const* event, uint64_t octets_in, uint64_t octets_out) {
    sgwc_ue_t *sgwc_ue = NULL;
    UsageLoggerData usageLoggerData = {0};
    sgwc_sess_t *sess = NULL;
    
    bearer = sgwc_bearer_cycle(bearer);
    sess = bearer ? sgwc_sess_cycle(bearer->sess) : NULL;
    if (NULL == sess) {
        ogs_error("Failed to build_usage_logger_data as sess does not exist");
        return usageLoggerData;
    }
    
    sgwc_ue = sgwc_ue_cycle(sess->sgwc_ue);
    if (NULL == sgwc_ue) {
        ogs_error("Failed to build_usage_logger_data as sgwc_ue does not exist");
        return usageLoggerData;
    }

    usageLoggerData.charging_id = bearer->charging_id;
    strncpy(usageLoggerData.event, event, EVENT_STR_MAX_LEN);
    strncpy(usageLoggerData.imsi, sgwc_ue->imsi_bcd, IMSI_STR_MAX_LEN);
    strncpy(usageLoggerData.apn, sess->session.name, APN_STR_MAX_LEN);
    usageLoggerData.qci = bearer->qci;
    usageLoggerData.octets_in = octets_in;
    usageLoggerData.octets_out = octets_out;
    usageLoggerData.dedicated_bearer = bearer->dedicated;

    strncpy(usageLoggerData.msisdn_bcd, sgwc_ue->msisdn_bcd, MSISDN_BCD_STR_MAX_LEN);
    strncpy(usageLoggerData.imeisv_bcd, sgwc_ue->imeisv_bcd, IMEISV_BCD_STR_MAX_LEN);
    if (!hex_array_to_string(sess->timezone_raw, sess->timezone_raw_len, usageLoggerData.timezone_raw, TIMEZONE_RAW_STR_MAX_LEN)) {
        ogs_error("Failed to convert raw timezone bytes to timezone hex string!");
    }
    usageLoggerData.plmn = ogs_plmn_id_hexdump(&sgwc_ue->e_tai.plmn_id);
    usageLoggerData.tac = sgwc_ue->e_tai.tac;
    usageLoggerData.eci = sgwc_ue->e_cgi.cell_id;
    memcpy(usageLoggerData.ue_ipv4, sess->ue_ipv4, OGS_ADDRSTRLEN);
    memcpy(usageLoggerData.ue_ipv6, sess->ue_ipv6, OGS_ADDRSTRLEN);
    if (!hex_array_to_string(sess->pgw_ip_raw, sess->pgw_ip_raw_len, usageLoggerData.pgw_ip, IP_STR_MAX_LEN)) {
        ogs_error("Failed to convert raw IP bytes to IP hex string!");
    }
    ogs_assert(OGS_ADDRSTRLEN < IP_STR_MAX_LEN);
    OGS_ADDR(ogs_gtp_self()->gtpc_addr, usageLoggerData.sgw_ip);

    return usageLoggerData;
}

void log_usage_logger_data(UsageLoggerData usageLoggerData) {
    time_t current_epoch_sec = time(NULL);
    bool log_res = log_usage_data(&ogs_pfcp_self()->usageLoggerState, current_epoch_sec, usageLoggerData);

    if (!log_res) {
        ogs_info("Failed to log usage data to file %s", ogs_pfcp_self()->usageLoggerState.filename);
    }
}

bool hex_array_to_string(uint8_t* hex_array, size_t hex_array_len, char* hex_string, size_t hex_string_len) {
    int i;
    for (i = 0; i < hex_array_len; i++) {
        if (hex_string_len < i) {
            return false;
        }
        
        sprintf(hex_string + (i * 2), "%02X", hex_array[i]);
    }

    return true;
}

void log_start_usage_reports(sgwc_bearer_t *bearer) {
    UsageLoggerData usageLoggerData = build_usage_logger_data(bearer, "start", 0, 0);
    log_usage_logger_data(usageLoggerData);
}

void log_uli_update(sgwc_bearer_t *bearer) {
    UsageLoggerData usageLoggerData = build_usage_logger_data(bearer, "uli_update", 0, 0);
    log_usage_logger_data(usageLoggerData);
}
