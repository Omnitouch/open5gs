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

#include "pfcp-path.h"
#include "gtp-path.h"
#include "usage_logger.h"

void log_deletion_usage_reports_session_deletion_response(sgwc_sess_t *sess, ogs_pfcp_session_deletion_response_t *pfcp_rsp);
void log_deletion_usage_reports_session_modification_response(sgwc_sess_t *sess, ogs_pfcp_session_modification_response_t *pfcp_rsp);
UsageLoggerData build_usage_logger_data(sgwc_bearer_t *bearer, char const* event, uint64_t octets_in, uint64_t octets_out);
void log_usage_logger_data(UsageLoggerData usageLoggerData);
bool hex_array_to_string(uint8_t* hex_array, size_t hex_array_len, char* hex_string, size_t hex_string_len);
void log_start_usage_reports(sgwc_bearer_t *bearer);
