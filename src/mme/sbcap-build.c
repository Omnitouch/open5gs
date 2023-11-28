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
#include "sbcap-build.h"

ogs_pkbuf_t *sbcap_build_write_replace_warning_response(ogs_sbcap_message_t *request)
{
    ogs_sbcap_message_t response = {};
    SBcAP_SuccessfulOutcome_t *successfulOutcome = NULL;
    SBcAP_Write_Replace_Warning_Response_t *writeReplaceWarningResponse = NULL;
    SBcAP_Write_Replace_Warning_Response_IEs_t *response_ie = NULL;
    SBcAP_Message_Identifier_t *response_MessageIdentifier = NULL;
    SBcAP_Serial_Number_t *response_SerialNumber = NULL;
    SBcAP_Cause_t *response_Cause = NULL;

    SBcAP_Write_Replace_Warning_Request_IEs_t *request_ie = NULL;
    SBcAP_Write_Replace_Warning_Request_t *writeReplaceWarningRequest =
        &request->choice.initiatingMessage.value.choice.Write_Replace_Warning_Request;

    ogs_debug("Write-Replace-Warning-Response");

    memset(&response, 0, sizeof(response));
    response.present = SBcAP_SBC_AP_PDU_PR_successfulOutcome;
    successfulOutcome = &response.choice.successfulOutcome;

    successfulOutcome->procedureCode = SBcAP_ProcedureCode_id_Write_Replace_Warning;
    successfulOutcome->criticality = SBcAP_Criticality_reject;
    successfulOutcome->value.present =
        SBcAP_SuccessfulOutcome__value_PR_Write_Replace_Warning_Response;

    writeReplaceWarningResponse = &successfulOutcome->value.choice.Write_Replace_Warning_Response;

    /* Copy the "MessageIdentifier" and "SerialNumber" IEs from the request */
    for (int i = 0; i < writeReplaceWarningRequest->protocolIEs.list.count; i++) {
        request_ie = writeReplaceWarningRequest->protocolIEs.list.array[i];

        switch (request_ie->id) {
            case SBcAP_ProtocolIE_ID_Message_Identifier:
            {
                SBcAP_Message_Identifier_t *request_MessageIdentifier =
                    &request_ie->value.choice.Message_Identifier;

                response_ie = CALLOC(1, sizeof(SBcAP_Write_Replace_Warning_Response_IEs_t));
                ASN_SEQUENCE_ADD(&writeReplaceWarningResponse->protocolIEs, response_ie);

                response_ie->id = SBcAP_ProtocolIE_ID_Message_Identifier;
                response_ie->criticality = request_ie->criticality;
                response_ie->value.present =
                    SBcAP_Write_Replace_Warning_Response_IEs__value_PR_Message_Identifier;

                response_MessageIdentifier = &response_ie->value.choice.Message_Identifier;
                
                response_MessageIdentifier->size = request_MessageIdentifier->size;
                response_MessageIdentifier->buf =
                    CALLOC(response_MessageIdentifier->size, sizeof(uint8_t));
                response_MessageIdentifier->bits_unused = request_MessageIdentifier->bits_unused;
                memcpy(response_MessageIdentifier->buf, request_MessageIdentifier->buf, response_MessageIdentifier->size);
                
                break;
            }
            case SBcAP_ProtocolIE_ID_Serial_Number:
            {
                SBcAP_Serial_Number_t *request_SerialNumber =
                    &request_ie->value.choice.Serial_Number;

                response_ie = CALLOC(1, sizeof(SBcAP_Write_Replace_Warning_Response_IEs_t));
                ASN_SEQUENCE_ADD(&writeReplaceWarningResponse->protocolIEs, response_ie);

                response_ie->id = SBcAP_ProtocolIE_ID_Serial_Number;
                response_ie->criticality = response_ie->criticality;
                response_ie->value.present =
                    SBcAP_Write_Replace_Warning_Response_IEs__value_PR_Serial_Number;

                response_SerialNumber = &response_ie->value.choice.Serial_Number;

                response_SerialNumber->size = request_SerialNumber->size;
                response_SerialNumber->buf =
                    CALLOC(response_SerialNumber->size, sizeof(uint8_t));
                response_SerialNumber->bits_unused = request_SerialNumber->bits_unused;
                memcpy(response_SerialNumber->buf, request_SerialNumber->buf, response_SerialNumber->size);

                break;
            }
            default:
                break;
        }
    }

    /* Make the "Cause" IE */
    {
        response_ie = CALLOC(1, sizeof(SBcAP_Write_Replace_Warning_Response_IEs_t));
        ASN_SEQUENCE_ADD(&writeReplaceWarningResponse->protocolIEs, response_ie);

        response_ie->id = SBcAP_ProtocolIE_ID_Cause;
        response_ie->criticality = SBcAP_Criticality_ignore;
        response_ie->value.present =
            SBcAP_Write_Replace_Warning_Response_IEs__value_PR_Cause;

        response_Cause = &response_ie->value.choice.Cause;

        /* TODO: Cause can be more than just "message accepted" */
        *response_Cause = SBcAP_Cause_message_accepted;
    }

    return ogs_sbcap_encode(&response);
}

ogs_pkbuf_t *sbcap_build_stop_warning_response(ogs_sbcap_message_t *request)
{
    ogs_sbcap_message_t response = {};
    SBcAP_SuccessfulOutcome_t *successfulOutcome = NULL;
    SBcAP_Stop_Warning_Response_t *stopWarningResponse = NULL;
    SBcAP_Stop_Warning_Response_IEs_t *response_ie = NULL;
    SBcAP_Message_Identifier_t *response_MessageIdentifier = NULL;
    SBcAP_Serial_Number_t *response_SerialNumber = NULL;
    SBcAP_Cause_t *response_Cause = NULL;

    SBcAP_Stop_Warning_Request_IEs_t *request_ie = NULL;
    SBcAP_Stop_Warning_Request_t *stopWarningRequest =
        &request->choice.initiatingMessage.value.choice.Stop_Warning_Request;

    ogs_debug("Stop-Warning-Response");

    memset(&response, 0, sizeof(response));
    response.present = SBcAP_SBC_AP_PDU_PR_successfulOutcome;
    successfulOutcome = &response.choice.successfulOutcome;

    successfulOutcome->procedureCode = SBcAP_ProcedureCode_id_Stop_Warning;
    successfulOutcome->criticality = SBcAP_Criticality_reject;
    successfulOutcome->value.present =
        SBcAP_SuccessfulOutcome__value_PR_Stop_Warning_Response;

    stopWarningResponse = &successfulOutcome->value.choice.Stop_Warning_Response;

    /* Copy the "MessageIdentifier" and "SerialNumber" IEs from the request */
    for (int i = 0; i < stopWarningRequest->protocolIEs.list.count; i++) {
        request_ie = stopWarningRequest->protocolIEs.list.array[i];

        switch (request_ie->id) {
            case SBcAP_ProtocolIE_ID_Message_Identifier:
            {
                SBcAP_Message_Identifier_t *request_MessageIdentifier =
                    &request_ie->value.choice.Message_Identifier;

                response_ie = CALLOC(1, sizeof(SBcAP_Stop_Warning_Response_IEs_t));
                ASN_SEQUENCE_ADD(&stopWarningResponse->protocolIEs, response_ie);

                response_ie->id = SBcAP_ProtocolIE_ID_Message_Identifier;
                response_ie->criticality = request_ie->criticality;
                response_ie->value.present =
                    SBcAP_Stop_Warning_Response_IEs__value_PR_Message_Identifier;

                response_MessageIdentifier = &response_ie->value.choice.Message_Identifier;
                
                response_MessageIdentifier->size = request_MessageIdentifier->size;
                response_MessageIdentifier->buf =
                    CALLOC(response_MessageIdentifier->size, sizeof(uint8_t));
                response_MessageIdentifier->bits_unused = request_MessageIdentifier->bits_unused;
                memcpy(response_MessageIdentifier->buf, request_MessageIdentifier->buf, response_MessageIdentifier->size);
                
                break;
            }
            case SBcAP_ProtocolIE_ID_Serial_Number:
            {
                SBcAP_Serial_Number_t *request_SerialNumber =
                    &request_ie->value.choice.Serial_Number;

                response_ie = CALLOC(1, sizeof(SBcAP_Stop_Warning_Response_IEs_t));
                ASN_SEQUENCE_ADD(&stopWarningResponse->protocolIEs, response_ie);

                response_ie->id = SBcAP_ProtocolIE_ID_Serial_Number;
                response_ie->criticality = response_ie->criticality;
                response_ie->value.present =
                    SBcAP_Stop_Warning_Response_IEs__value_PR_Serial_Number;

                response_SerialNumber = &response_ie->value.choice.Serial_Number;

                response_SerialNumber->size = request_SerialNumber->size;
                response_SerialNumber->buf =
                    CALLOC(response_SerialNumber->size, sizeof(uint8_t));
                response_SerialNumber->bits_unused = request_SerialNumber->bits_unused;
                memcpy(response_SerialNumber->buf, request_SerialNumber->buf, response_SerialNumber->size);

                break;
            }
            default:
                break;
        }
    }

    /* Make the "Cause" IE */
    {
        response_ie = CALLOC(1, sizeof(SBcAP_Stop_Warning_Response_IEs_t));
        ASN_SEQUENCE_ADD(&stopWarningResponse->protocolIEs, response_ie);

        response_ie->id = SBcAP_ProtocolIE_ID_Cause;
        response_ie->criticality = SBcAP_Criticality_ignore;
        response_ie->value.present =
            SBcAP_Stop_Warning_Response_IEs__value_PR_Cause;

        response_Cause = &response_ie->value.choice.Cause;

        /* TODO: Cause can be more than just "message accepted" */
        *response_Cause = SBcAP_Cause_message_accepted;
    }

    return ogs_sbcap_encode(&response);
}
