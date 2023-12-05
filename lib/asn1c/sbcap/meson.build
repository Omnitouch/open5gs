# Copyright (C) 2023 by Ryan Dimsey <ryan@omnitouch.com.au>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

libasn1c_sbcap_sources = files('''
    SBcAP_Broadcast-Cancelled-Area-List-5GS.c
    SBcAP_EmergencyAreaID-Cancelled-List.c
    SBcAP_PLMNidentity.c
    SBcAP_SuccessfulOutcome.c
    SBcAP_Broadcast-Cancelled-Area-List.c
    SBcAP_Emergency-Area-ID-List.c
    SBcAP_Presence.c
    SBcAP_TAC-5GS.c
    SBcAP_Broadcast-Empty-Area-List-5GS.c
    SBcAP_ENB-ID.c
    SBcAP_ProcedureCode.c
    SBcAP_TAC.c
    SBcAP_Broadcast-Empty-Area-List.c
    SBcAP_Error-Indication.c
    SBcAP_ProtocolExtensionContainer.c
    SBcAP_TAI-5GS.c
    SBcAP_Broadcast-Scheduled-Area-List-5GS.c
    SBcAP_EUTRAN-CGI.c
    SBcAP_ProtocolExtensionField.c
    SBcAP_TAI-Broadcast-List-5GS.c
    SBcAP_Broadcast-Scheduled-Area-List.c
    SBcAP_Extended-Repetition-Period.c
    SBcAP_ProtocolExtensionID.c
    SBcAP_TAI-Broadcast-List.c
    SBcAP_CancelledCellinEAI.c
    SBcAP_EXTERNAL.c
    SBcAP_ProtocolIE-Container.c
    SBcAP_TAI-Broadcast-List-Item.c
    SBcAP_CancelledCellinEAI-Item.c
    SBcAP_Failed-Cell-List.c
    SBcAP_ProtocolIE-ContainerList.c
    SBcAP_TAI.c
    SBcAP_CancelledCellinTAI-5GS.c
    SBcAP_Failed-Cell-List-NR.c
    SBcAP_ProtocolIE-Field.c
    SBcAP_TAI-Cancelled-List-5GS.c
    SBcAP_CancelledCellinTAI.c
    SBcAP_Global-ENB-ID.c
    SBcAP_ProtocolIE-ID.c
    SBcAP_TAI-Cancelled-List.c
    SBcAP_CancelledCellinTAI-Item.c
    SBcAP_Global-GNB-ID.c
    SBcAP_PWS-Failure-Indication.c
    SBcAP_TAI-Cancelled-List-Item.c
    SBcAP_Cause.c
    SBcAP_Global-NgENB-ID.c
    SBcAP_PWS-Restart-Indication.c
    SBcAP_TAI-List-for-Warning.c
    SBcAP_CellId-Broadcast-List-5GS.c
    SBcAP_Global-RAN-Node-ID.c
    SBcAP_RAT-Selector-5GS.c
    SBcAP_TBCD-STRING.c
    SBcAP_CellId-Broadcast-List.c
    SBcAP_GNB-ID.c
    SBcAP_Repetition-Period.c
    SBcAP_TriggeringMessage.c
    SBcAP_CellId-Broadcast-List-Item.c
    SBcAP_InitiatingMessage.c
    SBcAP_Restarted-Cell-List.c
    SBcAP_TypeOfError.c
    SBcAP_CellID-Cancelled-Item.c
    SBcAP_List-of-5GS-Cells-for-Failure.c
    SBcAP_Restarted-Cell-List-NR.c
    SBcAP_Unknown-5GS-Tracking-Area-List.c
    SBcAP_CellID-Cancelled-List-5GS.c
    SBcAP_List-of-5GS-TAI-for-Restart.c
    SBcAP_SBC-AP-PDU.c
    SBcAP_Unknown-Tracking-Area-List.c
    SBcAP_CellID-Cancelled-List.c
    SBcAP_List-of-5GS-TAIs.c
    SBcAP_ScheduledCellinEAI.c
    SBcAP_UnsuccessfulOutcome.c
    SBcAP_CellIdentity.c
    SBcAP_List-of-EAIs-Restart.c
    SBcAP_ScheduledCellinEAI-Item.c
    SBcAP_Warning-Area-Coordinates.c
    SBcAP_Concurrent-Warning-Message-Indicator.c
    SBcAP_List-of-TAIs.c
    SBcAP_ScheduledCellinTAI-5GS.c
    SBcAP_Warning-Area-List-5GS.c
    SBcAP_Criticality.c
    SBcAP_List-of-TAIs-Restart.c
    SBcAP_ScheduledCellinTAI.c
    SBcAP_Warning-Area-List.c
    SBcAP_Criticality-Diagnostics.c
    SBcAP_Message-Identifier.c
    SBcAP_ScheduledCellinTAI-Item.c
    SBcAP_Warning-Message-Content.c
    SBcAP_CriticalityDiagnostics-IE-List.c
    SBcAP_NgENB-ID.c
    SBcAP_Send-Stop-Warning-Indication.c
    SBcAP_Warning-Security-Information.c
    SBcAP_Data-Coding-Scheme.c
    SBcAP_NRCellIdentity.c
    SBcAP_Send-Write-Replace-Warning-Indication.c
    SBcAP_Warning-Type.c
    SBcAP_ECGIList.c
    SBcAP_NR-CGI.c
    SBcAP_Serial-Number.c
    SBcAP_Write-Replace-Warning-Indication.c
    SBcAP_EmergencyAreaID-Broadcast-List.c
    SBcAP_NR-CGIList.c
    SBcAP_Stop-All-Indicator.c
    SBcAP_Write-Replace-Warning-Request.c
    SBcAP_EmergencyAreaID-Broadcast-List-Item.c
    SBcAP_NumberOfBroadcasts.c
    SBcAP_Stop-Warning-Indication.c
    SBcAP_Write-Replace-Warning-Response.c
    SBcAP_Emergency-Area-ID.c
    SBcAP_Number-of-Broadcasts-Requested.c
    SBcAP_Stop-Warning-Request.c
    SBcAP_EmergencyAreaID-Cancelled-Item.c
    SBcAP_Omc-Id.c
    SBcAP_Stop-Warning-Response.c
'''.split())

libasn1c_sbcap_inc = include_directories('.')

libasn1c_sbcap = library('ogsasn1c-sbcap',
    sources : libasn1c_sbcap_sources,
    version : libogslib_version,
    c_args : libasn1c_common_cc_flags,
    include_directories : libasn1c_sbcap_inc,
    dependencies : [libasn1c_common_dep, libasn1c_util_dep],
    install : true)

libasn1c_sbcap_dep = declare_dependency(
    link_with : libasn1c_sbcap,
    include_directories : libasn1c_sbcap_inc,
    dependencies : [libasn1c_common_dep, libasn1c_util_dep])
