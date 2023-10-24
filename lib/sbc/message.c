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

#include "ogs-sbc.h"

int __ogs_sbc_domain;

enum { WRITE_REPLACE_WARNING = 0 };

enum { MESSAGE_IDENTIFIER = 5,
       SERIAL_NUMBER = 11,
       REPETITION_PERIOD = 10,
       NUMBER_OF_BROADCASTS_REQUESTED = 7,
       DATA_CODING_SCHEME = 3,
       WARNING_MESSAGE_CONTENT = 16 };

/* The decode functions are assuming a big endian byte order */
static uint16_t decode_uint16(uint8_t *buf);
static uint32_t decode_uint24(uint8_t *buf);
static size_t decode_write_replace_warning_request(
    sbc_write_replace_warning_request_t *write_replace_warning_request,
    uint8_t *buf,
    size_t buf_sz);

int ogs_sbc_decode(ogs_sbc_message_t *message, ogs_pkbuf_t *pkbuf)
{
    uint8_t message_type;
    uint8_t procedure_code;
    uint8_t criticality;
    uint8_t value_length;
    ogs_assert(message);
    ogs_assert(pkbuf);
    ogs_assert(pkbuf->data);
    ogs_assert(pkbuf->len);

    size_t bytes_decoded = 0;
    uint8_t *buf = pkbuf->data;

    message_type = buf[bytes_decoded];
    (void)message_type; /* TODO: Save these values in struct? */
    bytes_decoded += 1;

    procedure_code = buf[bytes_decoded];
    bytes_decoded += 1;

    criticality = buf[bytes_decoded];
    (void)criticality; /* TODO: Save these values in struct? */
    bytes_decoded += 1;

    value_length = buf[bytes_decoded];
    bytes_decoded += 1;

    if (value_length + bytes_decoded != pkbuf->len) {
        ogs_error("Packet size and SBC message size are not the same!");
        return OGS_ERROR;
    }

    switch (procedure_code) {
        case WRITE_REPLACE_WARNING:
            bytes_decoded += decode_write_replace_warning_request(
                &message->choice.write_replace_warning_request,
                &buf[bytes_decoded],
                value_length
            );
        break;

        default:
            ogs_error("Unsupported SBC procedure code of %u", procedure_code);
            return OGS_ERROR;
        break;
    }

    if (bytes_decoded != pkbuf->len) {
        return OGS_ERROR;
    }

    return OGS_OK;
}

static size_t decode_write_replace_warning_request(
    sbc_write_replace_warning_request_t *write_replace_warning_request,
    uint8_t *buf,
    size_t buf_sz)
{
    uint32_t number_of_ies;
    int i = 0;
    size_t bytes_decoded = 0;

    /* There are 7 bytes until we get the next size info */
    if (buf_sz < (3 + 2 + 1 + 1)) {
        ogs_error("Not enough bytes to decode IEs");
        return 0;
    }

    /* 3 bytes */
    number_of_ies = decode_uint24(&buf[bytes_decoded]);
    bytes_decoded += 3;

    for (i = 0; i < number_of_ies; ++i) {
        uint16_t id;
        uint8_t criticality_ie;
        uint8_t ie_value_length;

        id = decode_uint16(&buf[bytes_decoded]);
        bytes_decoded += 2;

        criticality_ie = buf[bytes_decoded];
        (void)criticality_ie; /* TODO: Save these values in struct? */
        bytes_decoded += 1;

        ie_value_length = buf[bytes_decoded];
        bytes_decoded += 1;

        if (buf_sz < bytes_decoded + ie_value_length) {
            ogs_error("Malformed SBC IE data detected for IE with ID of %u", id);
            return 0;
        }

        switch (id) {
            case MESSAGE_IDENTIFIER:
                ogs_expect(2 == ie_value_length);
                write_replace_warning_request->message_identifier = decode_uint16(&buf[bytes_decoded]);
            break;

            case SERIAL_NUMBER:
                ogs_expect(2 == ie_value_length);
                write_replace_warning_request->serial_number = decode_uint16(&buf[bytes_decoded]);
            break;

            case REPETITION_PERIOD:
                ogs_expect(2 == ie_value_length);
                write_replace_warning_request->repetition_period = decode_uint16(&buf[bytes_decoded]);
            break;

            case NUMBER_OF_BROADCASTS_REQUESTED:
                ogs_expect(2 == ie_value_length);
                write_replace_warning_request->number_of_broadcasts_requested = decode_uint16(&buf[bytes_decoded]);
            break;

            case DATA_CODING_SCHEME:
                ogs_expect(1 == ie_value_length);
                write_replace_warning_request->data_coding_scheme = buf[bytes_decoded];
            break;

            case WARNING_MESSAGE_CONTENT:
                /* Since ie_value_length is 8-bit we can guarantee that
                 * it will fit in the warning_message_content buf */
                memcpy(
                    write_replace_warning_request->warning_message_content,
                    &buf[bytes_decoded],
                    ie_value_length
                );
                write_replace_warning_request->warning_message_content_size = ie_value_length;
            break;

            default:
                ogs_error("Unsupported IE with ID of %u", id);
            break;
        }

        bytes_decoded += ie_value_length;
    }

    return bytes_decoded;
}

static uint16_t decode_uint16(uint8_t *buf)
{
    /* High byte */
    uint16_t result = buf[0];
    result <<= 8;

    /* Low byte */
    result |= buf[1];

    return result;
}

static uint32_t decode_uint24(uint8_t *buf)
{
    /* High bytes */
    uint32_t result = decode_uint16(buf);
    result <<= 8;

    /* Low byte */
    result |= buf[2];

    return result;
}
