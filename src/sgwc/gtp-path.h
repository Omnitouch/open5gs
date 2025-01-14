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

#ifndef SGWC_GTP_PATH_H
#define SGWC_GTP_PATH_H

#include "s11-build.h"

#ifdef __cplusplus
extern "C" {
#endif

int sgwc_gtp_open(void);
void sgwc_gtp_close(void);

int sgwc_gtp_send_create_session_response(
    sgwc_sess_t *sess, ogs_gtp_xact_t *xact);

int sgwc_gtp_send_downlink_data_notification(
    uint8_t cause_value, sgwc_bearer_t *bearer);

int sgwc_gtp2_send_delete_bearer_request(
    sgwc_bearer_t *bearer, uint8_t pti, uint8_t cause_value);

#ifdef __cplusplus
}
#endif

#endif /* SGWC_GTP_PATH_H */
