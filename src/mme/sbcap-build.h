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

#ifndef SBCAP_BUILD_H
#define SBCAP_BUILD_H

#include "mme-context.h"

#ifdef __cplusplus
extern "C" {
#endif

ogs_pkbuf_t *sbcap_build_write_replace_warning_response(ogs_sbcap_message_t *request);
ogs_pkbuf_t *sbcap_build_stop_warning_response(ogs_sbcap_message_t *request);

#ifdef __cplusplus
}
#endif

#endif /* SBCAP_BUILD_H */
