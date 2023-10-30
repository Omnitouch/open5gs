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

#include "mme-context.h"

#include "mme-sm.h"
#include "sbc-build.h"

ogs_pkbuf_t *sbc_build_write_replace_warning_response(ogs_sbc_message_t *request)
{
    ogs_sbc_message_t response = {};
    sbc_payload_t *write_replace_warning_response =
        &response.choice.successfulOutcome.payload;
    
    sbc_payload_t *write_replace_warning_request =
        &request->choice.initiatingMessage.payload;

    /* Specify what kind of response this is */
    response.present = SBC_PDU_PR_SUCCESSFULOUTCOME;
    response.choice.successfulOutcome.present = 
        SBC_MESSAGE_PR_WRITE_REPLACE_WARNING_REQUEST;
    
    response.choice.successfulOutcome.procedureCode =
        PROCEDURE_CODE_WRITE_REPLACE_WARNING;

    write_replace_warning_response->message_identifier_presence = 
        write_replace_warning_request->message_identifier_presence;
    write_replace_warning_response->message_identifier =
        write_replace_warning_request->message_identifier;

    write_replace_warning_response->serial_number_presence =
        write_replace_warning_request->serial_number_presence;
    write_replace_warning_response->serial_number =
        write_replace_warning_request->serial_number;

    write_replace_warning_response->cause_presence = 1;
    write_replace_warning_response->cause = CAUSE_MESSAGE_ACCEPTED;

    return ogs_sbc_encode(&response);
}

ogs_pkbuf_t *sbc_build_stop_warning_response(ogs_sbc_message_t *request)
{
    ogs_sbc_message_t response = {};
    sbc_payload_t *stop_warning_response =
        &response.choice.successfulOutcome.payload;
    
    sbc_payload_t *stop_warning_request =
        &request->choice.initiatingMessage.payload;

    /* Specify what kind of response this is */
    response.present = SBC_PDU_PR_SUCCESSFULOUTCOME;
    response.choice.successfulOutcome.present = 
        SBC_MESSAGE_PR_STOP_WARNING_REQUEST;

    response.choice.successfulOutcome.procedureCode =
        PROCEDURE_CODE_STOP_WARNING;

    stop_warning_response->message_identifier_presence = 
        stop_warning_request->message_identifier_presence;
    stop_warning_response->message_identifier =
        stop_warning_request->message_identifier;

    stop_warning_response->serial_number_presence =
        stop_warning_request->serial_number_presence;
    stop_warning_response->serial_number =
        stop_warning_request->serial_number;

    stop_warning_response->cause_presence = 1;
    stop_warning_response->cause = CAUSE_MESSAGE_ACCEPTED;

    return ogs_sbc_encode(&response);
}
