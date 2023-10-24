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

#ifndef SBC_PATH_H
#define SBC_PATH_H

#include "mme-context.h"
#include "mme-event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sbc_event_push  mme_sctp_event_push

int sbc_open(void);
void sbc_close(void);

ogs_sock_t *sbc_server(ogs_socknode_t *node);
void sbc_recv_upcall(short when, ogs_socket_t fd, void *data);

#ifdef __cplusplus
}
#endif

#endif /* SBC_PATH_H */
