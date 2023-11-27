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

#if !defined(OGS_SBCAP_INSIDE) && !defined(OGS_SBCAP_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_SBCAP_BUILD_H
#define OGS_SBCAP_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif

// ogs_pkbuf_t *ogs_s1ap_build_error_indication(
//     SBCAP_MME_UE_SBCAP_ID_t *mme_ue_s1ap_id,
//     SBCAP_ENB_UE_SBCAP_ID_t *enb_ue_s1ap_id,
//     SBCAP_Cause_PR group, long cause);

// ogs_pkbuf_t *ogs_s1ap_build_s1_reset(
//     SBCAP_Cause_PR group, long cause,
//     SBCAP_UE_associatedLogicalS1_ConnectionListRes_t *partOfS1_Interface);

// void ogs_s1ap_build_part_of_s1_interface(
//     SBCAP_UE_associatedLogicalS1_ConnectionListRes_t **list,
//     uint32_t *mme_ue_s1ap_id,
//     uint32_t *enb_ue_s1ap_id);

// ogs_pkbuf_t *ogs_s1ap_build_s1_reset_ack(
//     SBCAP_UE_associatedLogicalS1_ConnectionListRes_t *partOfS1_Interface);

#ifdef __cplusplus
}
#endif

#endif /* OGS_SBCAP_BUILD_H */
