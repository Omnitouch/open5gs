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

enum { MESSAGE_IDENTIFIER = 5,
       SERIAL_NUMBER = 11,
       REPETITION_PERIOD = 10,
       NUMBER_OF_BROADCASTS_REQUESTED = 7,
       DATA_CODING_SCHEME = 3,
       WARNING_MESSAGE_CONTENT = 16,
       CAUSE = 1,
       WARNING_TYPE = 18,
       WARNING_SECURITY_INFORMATION = 17 };

/* The decode functions are assuming a big endian
 * byte order and that the caller is sure the 
 * buffer size is fine*/
static uint16_t decode_uint16(uint8_t *buf);
static uint32_t decode_uint24(uint8_t *buf);
static void encode_uint16(uint16_t val, uint8_t *buf);
static void encode_uint24(uint32_t val, uint8_t *buf);
static size_t decode_sbc_payload(
    sbc_payload_t *payload,
    uint8_t *buf,
    size_t buf_sz);
static size_t encode_sbc_payload(sbc_payload_t *message, uint8_t *buf);
static ogs_pkbuf_t *encode_successful_outcome(SBC_SuccessfulOutcome *message);


ogs_pkbuf_t *ogs_sbc_encode(ogs_sbc_message_t *message)
{
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(message);

    switch(message->present) {
        case SBC_PDU_PR_SUCCESSFULOUTCOME:
            pkbuf = encode_successful_outcome(&message->choice.successfulOutcome);
        break;

        default:
            ogs_error("Unsupported PDU type");
        break;
    }

    return pkbuf;
}

/* See TS 129.168 for more info on SBC-AP */
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

    memset(message, 0, sizeof(ogs_sbc_message_t));

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

    /* This is a hack but it works for now... for some reason there is
     * sometimes an extra byte before the length byte (typically 0x80) maybe
     * to indicate that the value is greater than 128 bytes? In any case, we
     * expect that the specified length and bytes decoded before it will add
     * up to the number of bytes in the packet */
    if (value_length + bytes_decoded != pkbuf->len) {
        /* Maybe this next byte is the length? */
        value_length = buf[bytes_decoded];
        bytes_decoded += 1;
    }

    if (value_length + bytes_decoded != pkbuf->len) {
        ogs_error("Packet size (%u) and calculated SBC message size (%lu) are not the same!", pkbuf->len, value_length + bytes_decoded);
        ogs_error("Value length: %u, Bytes decoded: %lu, Packet size: %u", value_length, bytes_decoded, pkbuf->len);
        return OGS_ERROR;
    }

    /* FIXME: We're assuming the the message being decoded is always an initiatingMessage */
    switch (procedure_code) {
        case PROCEDURE_CODE_WRITE_REPLACE_WARNING:
            message->present = SBC_PDU_PR_INITIATINGMESSAGE;
            message->choice.initiatingMessage.present = SBC_MESSAGE_PR_WRITE_REPLACE_WARNING_REQUEST;
        break;

        case PROCEDURE_CODE_STOP_WARNING:
            message->present = SBC_PDU_PR_INITIATINGMESSAGE;
            message->choice.initiatingMessage.present = SBC_MESSAGE_PR_STOP_WARNING_REQUEST;
        break;

        default:
            ogs_error("Unsupported SBC procedure code of %u", procedure_code);
            return OGS_ERROR;
        break;
    }

    bytes_decoded += decode_sbc_payload(
                &message->choice.initiatingMessage.payload,
                &buf[bytes_decoded],
                value_length
    );

    if (bytes_decoded != pkbuf->len) {
        ogs_error("Failed to decode the full length of the packet, only %lu/%u bytes decoded", bytes_decoded, pkbuf->len);
        return OGS_ERROR;
    }

    return OGS_OK;
}

static size_t decode_sbc_payload(
    sbc_payload_t *payload,
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
        uint8_t criticality;
        uint8_t ie_value_length;

        id = decode_uint16(&buf[bytes_decoded]);
        bytes_decoded += 2;

        criticality = buf[bytes_decoded] >> 6;
        bytes_decoded += 1;

        ie_value_length = buf[bytes_decoded];
        bytes_decoded += 1;

        if (buf_sz < bytes_decoded + ie_value_length) {
            ogs_error("Malformed SBC IE data detected for IE with ID of %u", id);
            return 0;
        }

        switch (id) {
            case MESSAGE_IDENTIFIER:
                payload->message_identifier_criticality = criticality;
                ogs_expect(2 == ie_value_length);
                payload->message_identifier_presence = 1;
                payload->message_identifier = decode_uint16(&buf[bytes_decoded]);
            break;

            case SERIAL_NUMBER:
                payload->serial_number_criticality = criticality;
                ogs_expect(2 == ie_value_length);
                payload->serial_number_presence = 1;
                payload->serial_number = decode_uint16(&buf[bytes_decoded]);
            break;

            case REPETITION_PERIOD:
                payload->repetition_period_criticality = criticality;
                ogs_expect(2 == ie_value_length);
                payload->repetition_period_presence = 1;
                payload->repetition_period = decode_uint16(&buf[bytes_decoded]);
            break;

            case NUMBER_OF_BROADCASTS_REQUESTED:
                payload->number_of_broadcasts_requested_criticality = criticality;
                ogs_expect(2 == ie_value_length);
                payload->number_of_broadcasts_requested_presence = 1;
                payload->number_of_broadcasts_requested = decode_uint16(&buf[bytes_decoded]);
            break;

            case DATA_CODING_SCHEME:
                payload->data_coding_scheme_criticality = criticality;
                ogs_expect(1 == ie_value_length);
                payload->data_coding_scheme_presence = 1;
                payload->data_coding_scheme = buf[bytes_decoded];
            break;

            case WARNING_TYPE:
                payload->warning_type_criticality = criticality;
                ogs_expect(2 == ie_value_length);
                payload->warning_type_presence = 1;
                payload->warning_type = decode_uint16(&buf[bytes_decoded]);
            break;

            case WARNING_SECURITY_INFORMATION:
                payload->warning_security_information_criticality = criticality;
                /* Since ie_value_length is 8-bit we can guarantee that
                 * it will fit in the warning_message_content buf */
                memcpy(
                    payload->warning_security_information,
                    &buf[bytes_decoded],
                    ie_value_length
                );
                payload->warning_security_information_presence = 1;
                payload->warning_security_information_size = ie_value_length;
            break;

            case WARNING_MESSAGE_CONTENT:
                payload->warning_message_content_criticality = criticality;
                /* Since ie_value_length is 8-bit we can guarantee that
                 * it will fit in the warning_message_content buf */
                memcpy(
                    payload->warning_message_content,
                    &buf[bytes_decoded],
                    ie_value_length
                );
                payload->warning_message_content_presence = 1;
                payload->warning_message_content_size = ie_value_length;
            break;

            default:
                ogs_error("Unsupported IE with ID of %u, skipping decode of this IE", id);
            break;
        }

        bytes_decoded += ie_value_length;
    }

    return bytes_decoded;
}

/* TODO: Refector and do this proper one day */
/* We're assuming the the caller knows that the data we will encode will fit... */
static size_t encode_sbc_payload(sbc_payload_t *payload, uint8_t *buf)
{
    size_t bytes_encoded = 0;

    uint32_t num_ies = payload->message_identifier_presence
                     + payload->serial_number_presence
                     + payload->repetition_period_presence
                     + payload->number_of_broadcasts_requested_presence
                     + payload->data_coding_scheme_presence
                     + payload->warning_message_content_presence
                     + payload->cause_presence;
    encode_uint24(num_ies, &buf[bytes_encoded]);
    bytes_encoded += 3;

    if (1 == payload->message_identifier_presence) {
        encode_uint16(MESSAGE_IDENTIFIER, &buf[bytes_encoded]);
        bytes_encoded += 2;

        /* TODO: IE Criticality */
        buf[bytes_encoded] = 0x00;
        bytes_encoded += 1;

        /* TODO: Calculate IE value length */
        buf[bytes_encoded] = 0x02;
        bytes_encoded += 1;

        encode_uint16(payload->message_identifier, &buf[bytes_encoded]);
        bytes_encoded += 2;
    }

    if (1 == payload->serial_number_presence) {
        encode_uint16(SERIAL_NUMBER, &buf[bytes_encoded]);
        bytes_encoded += 2;

        /* TODO: IE Criticality */
        buf[bytes_encoded] = 0x00;
        bytes_encoded += 1;

        /* TODO: Calculate IE value length */
        buf[bytes_encoded] = 0x02;
        bytes_encoded += 1;

        encode_uint16(payload->serial_number, &buf[bytes_encoded]);
        bytes_encoded += 2;
    }

    if (1 == payload->cause_presence) {
        encode_uint16(CAUSE, &buf[bytes_encoded]);
        bytes_encoded += 2;

        /* TODO: IE Criticality */
        buf[bytes_encoded] = 0x00;
        bytes_encoded += 1;

        /* TODO: Calculate IE value length */
        buf[bytes_encoded] = 0x01;
        bytes_encoded += 1;

        encode_uint16(payload->cause, &buf[bytes_encoded]);
        bytes_encoded += 1;
    }

    return bytes_encoded;
}

static ogs_pkbuf_t *encode_successful_outcome(SBC_SuccessfulOutcome *message)
{
    ogs_pkbuf_t *pkbuf = ogs_pkbuf_alloc(NULL, OGS_MAX_SDU_LEN);
    if (!pkbuf) {
        ogs_error("ogs_pkbuf_alloc() failed");
        return NULL;
    }
    ogs_pkbuf_put(pkbuf, OGS_MAX_SDU_LEN);

    uint8_t *buf = pkbuf->data;
    size_t bytes_encoded = 0;
    size_t payload_sz = 0;

    /* Specify that this is a SuccessfulOutcome */
    buf[bytes_encoded] = SBC_PDU_VAL_SUCCESSFULOUTCOME;
    bytes_encoded += 1;

    buf[bytes_encoded] = message->procedureCode;
    bytes_encoded += 1;

    buf[bytes_encoded] = message->criticality;
    bytes_encoded += 1;

    /* Encode the payload THEN go back and encode the size (note the "+ 1") */
    payload_sz += encode_sbc_payload(&message->payload, &buf[bytes_encoded + 1]);

    buf[bytes_encoded] = payload_sz;
    bytes_encoded += 1 + payload_sz;

    ogs_pkbuf_trim(pkbuf, bytes_encoded);

    return pkbuf;
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

static void encode_uint16(uint16_t val, uint8_t *buf)
{
    /* High byte */
    buf[0] = (uint8_t)(val >> 8);
    
    /* Low byte */
    buf[1] = (uint8_t)(val);
}

static void encode_uint24(uint32_t val, uint8_t *buf)
{
    /* High byte */
    buf[0] = (uint8_t)(val >> 16);
    
    /* Mid byte */
    buf[1] = (uint8_t)(val >> 8);
    
    /* Low byte */
    buf[2] = (uint8_t)(val);
}