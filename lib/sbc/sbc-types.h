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

/* ******** */
/* Messages */
/* ******** */

enum { MAX_WARNING_CONTENT_MESSAGE_SIZE = 255 };

enum { CAUSE_MESSAGE_ACCEPTED = 0 };

/* TODO: Improve the sub-structures to include the presence flag internally */
typedef struct {
    uint8_t presence;

    uint8_t message_identifier_presence;
	uint16_t message_identifier;

    uint8_t serial_number_presence;
	uint16_t serial_number;

    uint8_t repetition_period_presence;
	uint16_t repetition_period;

    uint8_t number_of_broadcasts_requested_presence;
	uint16_t number_of_broadcasts_requested;

    uint8_t data_coding_scheme_presence;
	uint8_t data_coding_scheme;

    uint8_t warning_message_content_presence;
	uint8_t warning_message_content[MAX_WARNING_CONTENT_MESSAGE_SIZE];
	size_t warning_message_content_size;

    uint8_t cause_presence;
	uint8_t cause;
} sbc_write_replace_warning_t;


typedef enum {
	SBC_MESSAGE_PR_NOTHING,	/* No components present */
	SBC_MESSAGE_PR_WRITE_REPLACE_WARNING_REQUEST,
} SBC_Message_PR;

/* ***************** */
/* InitiatingMessage */
/* ***************** */

typedef struct {
	uint8_t procedureCode;
	uint8_t criticality;
	SBC_Message_PR present;

	union {
		sbc_write_replace_warning_t write_replace_warning;
	} choice;
} SBC_InitiatingMessage;


/* ***************** */
/* SuccessfulOutcome */
/* ***************** */

typedef struct {
	uint8_t procedureCode;
	uint8_t criticality;
	SBC_Message_PR present;
	
	union {
		sbc_write_replace_warning_t write_replace_warning;
	} choice;
} SBC_SuccessfulOutcome;


/* ******************* */
/* UnsuccessfulOutcome */
/* ******************* */

typedef struct {
	uint8_t procedureCode;
	uint8_t criticality;
	SBC_Message_PR present;
	
	union {
		sbc_write_replace_warning_t write_replace_warning;
	} choice;
} SBC_UnsuccessfulOutcome;


/* ********* */
/* Top Level */
/* ********* */

typedef enum SBC_PDU_PR {
	SBC_PDU_PR_NOTHING,	/* No components present */
	SBC_PDU_PR_INITIATINGMESSAGE,
	SBC_PDU_PR_SUCCESSFULOUTCOME,
	SBC_PDU_PR_UNSUCCESSFULOUTCOME
	/* Extensions may appear below */
} SBC_PDU_PR;

typedef enum SBC_PDU_VAL {
	SBC_PDU_VAL_INITIATINGMESSAGE = 0x00,
	SBC_PDU_VAL_SUCCESSFULOUTCOME = 0x20,
	// SBC_PDU_VAL_UNSUCCESSFULOUTCOME // TODO
	/* Extensions may appear below */
} SBC_PDU_VAL;

typedef struct {
	SBC_PDU_PR present;
	union {
		SBC_InitiatingMessage initiatingMessage;
		SBC_SuccessfulOutcome successfulOutcome;
		SBC_UnsuccessfulOutcome unsuccessfulOutcome;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
} ogs_sbc_message_t;

#ifdef __cplusplus
}
#endif

#endif
