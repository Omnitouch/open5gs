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

#include "ogs-sctp.h"

#include "mme-event.h"
#include "mme-timer.h"

#include "nas-security.h"
#include "nas-path.h"

#include "sbc-build.h"
#include "sbc-path.h"

int sbc_open(void)
{
    ogs_socknode_t *node = NULL;

    ogs_list_for_each(&mme_self()->sbc_list, node)
        if (sbc_server(node) == NULL) return OGS_ERROR;

    ogs_list_for_each(&mme_self()->sbc_list6, node)
        if (sbc_server(node) == NULL) return OGS_ERROR;

    return OGS_OK;
}

void sbc_close(void)
{
    ogs_socknode_remove_all(&mme_self()->sbc_list);
    ogs_socknode_remove_all(&mme_self()->sbc_list6);
}

int sbc_send_to_cbs(mme_cbs_t *cbs, ogs_pkbuf_t *pkbuf)
{
    char buf[OGS_ADDRSTRLEN];

    ogs_assert(pkbuf);

    if (!mme_cbs_initialised()) {
        ogs_error("Can't send data to a CBS that is not initialised!");
        ogs_pkbuf_free(pkbuf);
        return OGS_ERROR;
    }

    ogs_assert(cbs->sctp.sock);
    if (cbs->sctp.sock->fd == INVALID_SOCKET) {
        ogs_fatal("CBS SCTP socket has already been destroyed");
        ogs_log_hexdump(OGS_LOG_FATAL, pkbuf->data, pkbuf->len);
        ogs_pkbuf_free(pkbuf);
        return OGS_ERROR;
    }

    ogs_debug("Sending data to CBS on '%s'",
            OGS_ADDR(cbs->sctp.addr, buf));

    ogs_sctp_ppid_in_pkbuf(pkbuf) = OGS_SCTP_SBC_PPID;
    // ogs_sctp_stream_no_in_pkbuf(pkbuf) = 0; // S1AP_NON_UE_SIGNALLING

    if (cbs->sctp.type == SOCK_STREAM) {
        ogs_sctp_write_to_buffer(&cbs->sctp, pkbuf);
        return OGS_OK;
    } else {
        return ogs_sctp_senddata(cbs->sctp.sock, pkbuf, cbs->sctp.addr);
    }
}

int sbc_send_write_replace_warning_response(mme_cbs_t *cbs, ogs_sbc_message_t *request)
{
    int rv;
    ogs_pkbuf_t *sbc_buffer;
    
    ogs_debug("SBC-CBS Write Replace Warning Response");

    if (!cbs->state.initialised) {
        ogs_error("CBS is not initialised!");
        return OGS_ERROR;
    }

    sbc_buffer = sbc_build_write_replace_warning_response(request);
    if (!sbc_buffer) {
        ogs_error("sbc_build_write_replace_warning_response() failed");
        return OGS_ERROR;
    }

    rv = sbc_send_to_cbs(cbs, sbc_buffer);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int sbc_send_stop_warning_response(mme_cbs_t *cbs, ogs_sbc_message_t *request)
{
    int rv;
    ogs_pkbuf_t *sbc_buffer;
    
    ogs_debug("SBC-CBS Write Replace Warning Response");

    if (!cbs->state.initialised) {
        ogs_error("CBS is not initialised!");
        return OGS_ERROR;
    }

    sbc_buffer = sbc_build_stop_warning_response(request);
    if (!sbc_buffer) {
        ogs_error("sbc_build_write_replace_warning_response() failed");
        return OGS_ERROR;
    }

    rv = sbc_send_to_cbs(cbs, sbc_buffer);
    ogs_expect(rv == OGS_OK);

    return rv;
}
