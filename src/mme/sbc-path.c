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

