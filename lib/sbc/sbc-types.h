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

#if !defined(OGS_SBC_INSIDE) && !defined(OGS_SBC_TYPES_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_SBC_TYPES_MESSAGE_H
#define OGS_SBC_TYPES_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

enum { MAX_WARNING_CONTENT_MESSAGE_SIZE = 255 };

typedef struct sbc_write_replace_warning_request_s {
    uint8_t presence;
    uint16_t message_identifier;
    uint16_t serial_number;
    uint16_t repetition_period;
    uint16_t number_of_broadcasts_requested;
    uint8_t data_coding_scheme;
    uint8_t warning_message_content[MAX_WARNING_CONTENT_MESSAGE_SIZE];
    size_t warning_message_content_size;
} sbc_write_replace_warning_request_t;

typedef struct ogs_sbc_message_s {
    union {
        sbc_write_replace_warning_request_t write_replace_warning_request;
    } choice;
} ogs_sbc_message_t;

#ifdef __cplusplus
}
#endif

#endif
