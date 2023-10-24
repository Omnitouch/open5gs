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

#ifndef OGS_SBC_H
#define OGS_SBC_H

#include "core/ogs-core.h"

#define OGS_SBC_INSIDE

#include "sbc/sbc-types.h"
#include "sbc/message.h"

#undef OGS_SBC_INSIDE

#ifdef __cplusplus
extern "C" {
#endif

extern int __ogs_sbc_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __ogs_sbc_domain

#ifdef __cplusplus
}
#endif

#endif
