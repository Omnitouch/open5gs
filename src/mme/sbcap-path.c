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

#include "sbcap-build.h"
#include "sbcap-path.h"

int sbcap_open(void)
{
    ogs_socknode_t *node = NULL;

    ogs_list_for_each(&mme_self()->sbcap_list, node)
        if (sbcap_server(node) == NULL) return OGS_ERROR;

    ogs_list_for_each(&mme_self()->sbcap_list6, node)
        if (sbcap_server(node) == NULL) return OGS_ERROR;

    return OGS_OK;
}

void sbcap_close(void)
{
    ogs_socknode_remove_all(&mme_self()->sbcap_list);
    ogs_socknode_remove_all(&mme_self()->sbcap_list6);
}

int sbcap_send_to_cbc(mme_cbc_t *cbc, ogs_pkbuf_t *pkbuf)
{
    char buf[OGS_ADDRSTRLEN];

    ogs_assert(pkbuf);

    if (!mme_cbc_initialised()) {
        ogs_error("Can't send data to a SBCAP that is not initialised!");
        ogs_pkbuf_free(pkbuf);
        return OGS_ERROR;
    }

    ogs_assert(cbc->sctp.sock);
    if (cbc->sctp.sock->fd == INVALID_SOCKET) {
        ogs_fatal("SBCAP SCTP socket has already been destroyed");
        ogs_log_hexdump(OGS_LOG_FATAL, pkbuf->data, pkbuf->len);
        ogs_pkbuf_free(pkbuf);
        return OGS_ERROR;
    }

    ogs_debug("Sending data to cbc on '%s'",
            OGS_ADDR(cbc->sctp.addr, buf));

    ogs_sctp_ppid_in_pkbuf(pkbuf) = OGS_SCTP_SBCAP_PPID;
    // ogs_sctp_stream_no_in_pkbuf(pkbuf) = 0; // S1AP_NON_UE_SIGNALLING

    if (cbc->sctp.type == SOCK_STREAM) {
        ogs_sctp_write_to_buffer(&cbc->sctp, pkbuf);
        return OGS_OK;
    } else {
        return ogs_sctp_senddata(cbc->sctp.sock, pkbuf, cbc->sctp.addr);
    }
}

int sbcap_send_write_replace_warning_response(mme_cbc_t *cbc, ogs_sbcap_message_t *request)
{
    int rv;
    ogs_pkbuf_t *sbcap_buffer;
    
    ogs_debug("SBCAP Write Replace Warning Response");

    if (!cbc->state.initialised) {
        ogs_error("cbc is not initialised!");
        return OGS_ERROR;
    }

    sbcap_buffer = sbcap_build_write_replace_warning_response(request);
    if (!sbcap_buffer) {
        ogs_error("sbcap_build_write_replace_warning_response() failed");
        return OGS_ERROR;
    }

    rv = sbcap_send_to_cbc(cbc, sbcap_buffer);
    ogs_expect(rv == OGS_OK);

    return rv;
}

int sbcap_send_stop_warning_response(mme_cbc_t *cbc, ogs_sbcap_message_t *request)
{
    int rv;
    ogs_pkbuf_t *sbcap_buffer;
    
    ogs_debug("SBCAP Write Replace Warning Response");

    if (!cbc->state.initialised) {
        ogs_error("cbc is not initialised!");
        return OGS_ERROR;
    }

    sbcap_buffer = sbcap_build_stop_warning_response(request);
    if (!sbcap_buffer) {
        ogs_error("sbcap_build_write_replace_warning_response() failed");
        return OGS_ERROR;
    }

    rv = sbcap_send_to_cbc(cbc, sbcap_buffer);
    ogs_expect(rv == OGS_OK);

    return rv;
}
