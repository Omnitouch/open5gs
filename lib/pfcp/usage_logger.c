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

#include "usage_logger.h"
#include <string.h>

static bool file_elapsed(UsageLoggerState const *state, time_t current_epoch_sec);
static void refresh_state(UsageLoggerState *state, time_t current_epoch_sec);
static void get_time_string(time_t time, char *buf, size_t buf_sz);
static bool create_new_file(UsageLoggerState const *state);
static bool step(UsageLoggerState *state, time_t current_epoch_sec);

bool log_usage_data(UsageLoggerState *state, time_t current_epoch_sec, UsageLoggerData data)
{
    bool result = false;
    bool stepped = step(state, current_epoch_sec);

    if (stepped) {
        FILE *fptr = fopen(state->filename, "a");

        if (NULL != fptr)
        {
            int fprint_result = fprintf(
                fptr,
                "%li,"   /* epoch */
                "%s,"    /* imsi */
                "%s,"    /* event */
                "%u,"    /* charging_id */
                "%s,"    /* msisdn */
                "%s,"    /* ue_imei */
                "%s,"    /* timezone_raw */
                "%u,"    /* plmn */
                "%u,"    /* tac */
                "%u,"    /* eci */
                "%s,"    /* sgw_ip */
                "%s,"    /* ue_ip */
                "%s,"    /* pgw_ip */
                "%s,"    /* apn */
                "%u,"    /* qci */
                "%lu,"   /* octets_in */
                "%lu\n", /* octets_out */
                current_epoch_sec,
                data.imsi,
                data.event,
                data.charging_id,
                data.msisdn_bcd,
                data.imeisv_bcd,
                data.timezone_raw,
                data.plmn,
                data.tac,
                data.eci,
                data.sgw_ip,
                data.ue_ip,
                data.pgw_ip,
                data.apn,
                data.qci,
                data.octets_in,
                data.octets_out);

            int fclose_result = fclose(fptr);

            if ((0 < fprint_result) &&
                (0 == fclose_result))
            {
                result = true;
            }
        }
    }

    return result;
}

static bool step(UsageLoggerState *state, time_t current_epoch_sec)
{
    if (NULL != state)
    {
        if (file_elapsed(state, current_epoch_sec))
        {
            refresh_state(state, current_epoch_sec);
            create_new_file(state);
        }
        return true;
    }
    return false;
}

static bool file_elapsed(UsageLoggerState const *state, time_t current_epoch_sec)
{
    return state->_file_end_time <= current_epoch_sec;
}

static void refresh_state(UsageLoggerState *state, time_t current_epoch_sec)
{
    /* Generate and set filename */
    snprintf(state->filename, FILENAME_MAX_LEN, "%s/%lu", state->log_dir, current_epoch_sec);

    /* Set the file capture window */
    state->_file_start_time = current_epoch_sec;
    state->_file_end_time = state->_file_start_time + state->file_capture_period_sec;
}

static bool create_new_file(UsageLoggerState const *state)
{
    enum
    {
        CAPTURE_TIME_MAX_SZ = 16
    };

    bool result = false;
    char file_capture_time_start[CAPTURE_TIME_MAX_SZ] = "";
    char file_capture_time_end[CAPTURE_TIME_MAX_SZ] = "";
    FILE *fptr = fopen(state->filename, "wb");

    get_time_string(state->_file_end_time, file_capture_time_end, CAPTURE_TIME_MAX_SZ);
    get_time_string(state->_file_start_time, file_capture_time_start, CAPTURE_TIME_MAX_SZ);

    if (NULL != fptr)
    {
        int fprint_result = fprintf(
            fptr,
            "# SWG CDR File:\n"
            "# File Start Time: %s (%li)\n"
            "# File End Time: %s (%li)\n"
            "# SGW Name: %s\n"
            "# epoch,imsi,event,charging_id,msisdn,ue_imei,timezone_raw,plmn,tac,eci,sgw_ip,ue_ip,pgw_ip,apn,qci,octets_in,octets_out\n",
            file_capture_time_start,
            state->_file_end_time,
            file_capture_time_end,
            state->_file_start_time,
            state->sgw_name);

        int fclose_result = fclose(fptr);

        if ((0 < fprint_result) &&
            (0 == fclose_result))
        {
            result = true;
        }
    }

    return result;
}

static void get_time_string(time_t time, char *buf, size_t buf_sz)
{
    struct tm *tmp = localtime(&time);
    snprintf(buf, buf_sz, "%02d:%02d:%02d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}
