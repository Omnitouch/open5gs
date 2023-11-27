/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../../src/sbcap/asn1/SBC_AP_IEs.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_Broadcast_Scheduled_Area_List_5GS_H_
#define	_SBcAP_Broadcast_Scheduled_Area_List_5GS_H_


#include <asn_application.h>

/* Including external dependencies */
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBcAP_CellId_Broadcast_List_5GS;
struct SBcAP_TAI_Broadcast_List_5GS;
struct SBcAP_EmergencyAreaID_Broadcast_List;
struct SBcAP_ProtocolExtensionContainer;

/* SBcAP_Broadcast-Scheduled-Area-List-5GS */
typedef struct SBcAP_Broadcast_Scheduled_Area_List_5GS {
	struct SBcAP_CellId_Broadcast_List_5GS	*cellId_Broadcast_List_5GS;	/* OPTIONAL */
	struct SBcAP_TAI_Broadcast_List_5GS	*tAI_Broadcast_List_5GS;	/* OPTIONAL */
	struct SBcAP_EmergencyAreaID_Broadcast_List	*emergencyAreaID_Broadcast_List;	/* OPTIONAL */
	struct SBcAP_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_Broadcast_Scheduled_Area_List_5GS_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_Broadcast_Scheduled_Area_List_5GS;
extern asn_SEQUENCE_specifics_t asn_SPC_SBcAP_Broadcast_Scheduled_Area_List_5GS_specs_1;
extern asn_TYPE_member_t asn_MBR_SBcAP_Broadcast_Scheduled_Area_List_5GS_1[4];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SBcAP_CellId-Broadcast-List-5GS.h>
#include <SBcAP_TAI-Broadcast-List-5GS.h>
#include <SBcAP_EmergencyAreaID-Broadcast-List.h>

#endif	/* _SBcAP_Broadcast_Scheduled_Area_List_5GS_H_ */
#include <asn_internal.h>
