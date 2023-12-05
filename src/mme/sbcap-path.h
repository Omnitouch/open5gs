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

#ifndef SBCAP_PATH_H
#define SBCAP_PATH_H

#include "mme-context.h"
#include "mme-event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sbcap_event_push  mme_sctp_event_push

int sbcap_open(void);
void sbcap_close(void);

ogs_sock_t *sbcap_server(ogs_socknode_t *node);
void sbcap_recv_upcall(short when, ogs_socket_t fd, void *data);
int sbcap_send_to_cbc(mme_cbc_t *cbc, ogs_pkbuf_t *pkbuf);
int sbcap_send_write_replace_warning_response(mme_cbc_t *cbc, ogs_sbcap_message_t *request);
int sbcap_send_stop_warning_response(mme_cbc_t *cbc, ogs_sbcap_message_t *request);

#ifdef __cplusplus
}
#endif

#endif /* SBCAP_PATH_H */
