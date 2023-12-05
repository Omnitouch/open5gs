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

#ifndef OGS_SBCAP_H
#define OGS_SBCAP_H

#include "core/ogs-core.h"

#include "SBcAP_Broadcast-Cancelled-Area-List-5GS.h"
#include "SBcAP_EmergencyAreaID-Cancelled-List.h"
#include "SBcAP_PLMNidentity.h"
#include "SBcAP_SuccessfulOutcome.h"
#include "SBcAP_Broadcast-Cancelled-Area-List.h"
#include "SBcAP_Emergency-Area-ID-List.h"
#include "SBcAP_Presence.h"
#include "SBcAP_TAC-5GS.h"
#include "SBcAP_Broadcast-Empty-Area-List-5GS.h"
#include "SBcAP_ENB-ID.h"
#include "SBcAP_ProcedureCode.h"
#include "SBcAP_TAC.h"
#include "SBcAP_Broadcast-Empty-Area-List.h"
#include "SBcAP_Error-Indication.h"
#include "SBcAP_ProtocolExtensionContainer.h"
#include "SBcAP_TAI-5GS.h"
#include "SBcAP_Broadcast-Scheduled-Area-List-5GS.h"
#include "SBcAP_EUTRAN-CGI.h"
#include "SBcAP_ProtocolExtensionField.h"
#include "SBcAP_TAI-Broadcast-List-5GS.h"
#include "SBcAP_Broadcast-Scheduled-Area-List.h"
#include "SBcAP_Extended-Repetition-Period.h"
#include "SBcAP_ProtocolExtensionID.h"
#include "SBcAP_TAI-Broadcast-List.h"
#include "SBcAP_CancelledCellinEAI.h"
#include "SBcAP_EXTERNAL.h"
#include "SBcAP_ProtocolIE-Container.h"
#include "SBcAP_TAI-Broadcast-List-Item.h"
#include "SBcAP_CancelledCellinEAI-Item.h"
#include "SBcAP_Failed-Cell-List.h"
#include "SBcAP_ProtocolIE-ContainerList.h"
#include "SBcAP_TAI.h"
#include "SBcAP_CancelledCellinTAI-5GS.h"
#include "SBcAP_Failed-Cell-List-NR.h"
#include "SBcAP_ProtocolIE-Field.h"
#include "SBcAP_TAI-Cancelled-List-5GS.h"
#include "SBcAP_CancelledCellinTAI.h"
#include "SBcAP_Global-ENB-ID.h"
#include "SBcAP_ProtocolIE-ID.h"
#include "SBcAP_TAI-Cancelled-List.h"
#include "SBcAP_CancelledCellinTAI-Item.h"
#include "SBcAP_Global-GNB-ID.h"
#include "SBcAP_PWS-Failure-Indication.h"
#include "SBcAP_TAI-Cancelled-List-Item.h"
#include "SBcAP_Cause.h"
#include "SBcAP_Global-NgENB-ID.h"
#include "SBcAP_PWS-Restart-Indication.h"
#include "SBcAP_TAI-List-for-Warning.h"
#include "SBcAP_CellId-Broadcast-List-5GS.h"
#include "SBcAP_Global-RAN-Node-ID.h"
#include "SBcAP_RAT-Selector-5GS.h"
#include "SBcAP_TBCD-STRING.h"
#include "SBcAP_CellId-Broadcast-List.h"
#include "SBcAP_GNB-ID.h"
#include "SBcAP_Repetition-Period.h"
#include "SBcAP_TriggeringMessage.h"
#include "SBcAP_CellId-Broadcast-List-Item.h"
#include "SBcAP_InitiatingMessage.h"
#include "SBcAP_Restarted-Cell-List.h"
#include "SBcAP_TypeOfError.h"
#include "SBcAP_CellID-Cancelled-Item.h"
#include "SBcAP_List-of-5GS-Cells-for-Failure.h"
#include "SBcAP_Restarted-Cell-List-NR.h"
#include "SBcAP_Unknown-5GS-Tracking-Area-List.h"
#include "SBcAP_CellID-Cancelled-List-5GS.h"
#include "SBcAP_List-of-5GS-TAI-for-Restart.h"
#include "SBcAP_SBC-AP-PDU.h"
#include "SBcAP_Unknown-Tracking-Area-List.h"
#include "SBcAP_CellID-Cancelled-List.h"
#include "SBcAP_List-of-5GS-TAIs.h"
#include "SBcAP_ScheduledCellinEAI.h"
#include "SBcAP_UnsuccessfulOutcome.h"
#include "SBcAP_CellIdentity.h"
#include "SBcAP_List-of-EAIs-Restart.h"
#include "SBcAP_ScheduledCellinEAI-Item.h"
#include "SBcAP_Warning-Area-Coordinates.h"
#include "SBcAP_Concurrent-Warning-Message-Indicator.h"
#include "SBcAP_List-of-TAIs.h"
#include "SBcAP_ScheduledCellinTAI-5GS.h"
#include "SBcAP_Warning-Area-List-5GS.h"
#include "SBcAP_Criticality.h"
#include "SBcAP_List-of-TAIs-Restart.h"
#include "SBcAP_ScheduledCellinTAI.h"
#include "SBcAP_Warning-Area-List.h"
#include "SBcAP_Criticality-Diagnostics.h"
#include "SBcAP_Message-Identifier.h"
#include "SBcAP_ScheduledCellinTAI-Item.h"
#include "SBcAP_Warning-Message-Content.h"
#include "SBcAP_CriticalityDiagnostics-IE-List.h"
#include "SBcAP_NgENB-ID.h"
#include "SBcAP_Send-Stop-Warning-Indication.h"
#include "SBcAP_Warning-Security-Information.h"
#include "SBcAP_Data-Coding-Scheme.h"
#include "SBcAP_NRCellIdentity.h"
#include "SBcAP_Send-Write-Replace-Warning-Indication.h"
#include "SBcAP_Warning-Type.h"
#include "SBcAP_ECGIList.h"
#include "SBcAP_NR-CGI.h"
#include "SBcAP_Serial-Number.h"
#include "SBcAP_Write-Replace-Warning-Indication.h"
#include "SBcAP_EmergencyAreaID-Broadcast-List.h"
#include "SBcAP_NR-CGIList.h"
#include "SBcAP_Stop-All-Indicator.h"
#include "SBcAP_Write-Replace-Warning-Request.h"
#include "SBcAP_EmergencyAreaID-Broadcast-List-Item.h"
#include "SBcAP_NumberOfBroadcasts.h"
#include "SBcAP_Stop-Warning-Indication.h"
#include "SBcAP_Write-Replace-Warning-Response.h"
#include "SBcAP_Emergency-Area-ID.h"
#include "SBcAP_Number-of-Broadcasts-Requested.h"
#include "SBcAP_Stop-Warning-Request.h"
#include "SBcAP_EmergencyAreaID-Cancelled-Item.h"
#include "SBcAP_Omc-Id.h"
#include "SBcAP_Stop-Warning-Response.h"

#include "asn1c/util/conv.h"
#include "asn1c/util/message.h"

#define OGS_SBCAP_INSIDE

#include "sbcap/conv.h"
#include "sbcap/message.h"
#include "sbcap/build.h"

#undef OGS_SBCAP_INSIDE

#ifdef __cplusplus
extern "C" {
#endif

extern int __ogs_sbcap_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __ogs_sbcap_domain

#ifdef __cplusplus
}
#endif

#endif
