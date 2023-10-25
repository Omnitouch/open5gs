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

#if !defined(OGS_SBC_INSIDE) && !defined(OGS_SBC_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_SBC_MESSAGE_H
#define OGS_SBC_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

int ogs_sbc_decode(ogs_sbc_message_t *message, ogs_pkbuf_t *pkbuf);
ogs_pkbuf_t *ogs_sbc_encode(ogs_sbc_message_t *message);

#ifdef __cplusplus
}
#endif

#endif



