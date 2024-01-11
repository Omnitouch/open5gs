/*
 * Copyright (C) 2023 by Ryan Dimsey <ryan@omnitouch.com.au>
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

#ifndef USAGE_LOGGER_CONTEXT_H
#define USAGE_LOGGER_CONTEXT_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

enum
{
    FILENAME_MAX_LEN = 128,
    SGW_NAME_STR_MAX_LEN = 32,
    IPV4_STR_MAX_LEN = 16,
    IMSI_STR_MAX_LEN = 16,
    APN_STR_MAX_LEN = 64,
    MSISDN_BCD_STR_MAX_LEN = 16,
    IMEISV_BCD_STR_MAX_LEN = 32,
    TIMEZONE_RAW_STR_MAX_LEN = 16,
    EVENT_STR_MAX_LEN = 32,
    IP_STR_MAX_LEN = 64,
    LOG_DIR_STR_MAX_LEN = 64
};

typedef struct
{
    char imsi[IMSI_STR_MAX_LEN];
    char apn[APN_STR_MAX_LEN];
    uint8_t qci;
    uint64_t octets_in;
    uint64_t octets_out;

    char event[EVENT_STR_MAX_LEN];
    uint32_t charging_id;
    char msisdn_bcd[MSISDN_BCD_STR_MAX_LEN];
    char imeisv_bcd[IMEISV_BCD_STR_MAX_LEN];
    char timezone_raw[TIMEZONE_RAW_STR_MAX_LEN];
    uint32_t plmn;
    uint16_t tac;
    uint32_t eci;
    char sgw_ip[IP_STR_MAX_LEN];
    char ue_ip[IP_STR_MAX_LEN];
    char pgw_ip[IP_STR_MAX_LEN];
} UsageLoggerData;

typedef struct
{
    /* Developer should set these fields (E.g. via config) */
    bool enabled;
    uint64_t file_capture_period_sec;
    uint64_t reporting_period_sec;
    char sgw_name[SGW_NAME_STR_MAX_LEN];
    char log_dir[LOG_DIR_STR_MAX_LEN];

    /* The following are to be used 
     * internally by the module and
     * shouldn't be directly written
     * to */
    char filename[FILENAME_MAX_LEN];
    time_t _file_start_time;
    time_t _file_end_time;
} UsageLoggerState;

bool log_usage_data(UsageLoggerState *state, time_t current_epoch_sec, UsageLoggerData data);

#endif /* USAGE_LOGGER_CONTEXT_H */
