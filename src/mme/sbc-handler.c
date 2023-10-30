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

#include "mme-context.h"
#include "s1ap-path.h"
#include "s1ap-build.h"
#include "sbc-handler.h"

/* Builds and sends a Write-Replace-Warning-Request to all eNBs over S1AP */
void sbc_handle_write_replace_warning_request(sbc_payload_t *request)
{
    mme_enb_t *enb = NULL;

    ogs_list_for_each(&mme_self()->enb_list, enb) {
        int rv;
        ogs_pkbuf_t *s1apbuf = NULL;

        s1apbuf = s1ap_build_write_replace_warning_request(request);
        if (!s1apbuf) {
            ogs_error("s1ap_build_write_replace_warning_request() failed");
            return;
        }

        /* Send to enb */
        rv = s1ap_send_to_enb(enb, s1apbuf, S1AP_NON_UE_SIGNALLING);
        ogs_expect(rv == OGS_OK);
    }
}

void sbc_handle_stop_warning_request(sbc_payload_t *request)
{
    mme_enb_t *enb = NULL;

    ogs_list_for_each(&mme_self()->enb_list, enb) {
        int rv;
        ogs_pkbuf_t *s1apbuf = NULL;

        s1apbuf = s1ap_build_kill_request(request);
        if (!s1apbuf) {
            ogs_error("s1ap_build_kill_request() failed");
            return;
        }

        /* Send to enb */
        rv = s1ap_send_to_enb(enb, s1apbuf, S1AP_NON_UE_SIGNALLING);
        ogs_expect(rv == OGS_OK);
    }
}
