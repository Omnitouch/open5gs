/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../../src/sbcap/asn1/SBC_AP_IEs.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_CellId_Broadcast_List_Item_H_
#define	_SBcAP_CellId_Broadcast_List_Item_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SBcAP_EUTRAN-CGI.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBcAP_ProtocolExtensionContainer;

/* SBcAP_CellId-Broadcast-List-Item */
typedef struct SBcAP_CellId_Broadcast_List_Item {
	SBcAP_EUTRAN_CGI_t	 eCGI;
	struct SBcAP_ProtocolExtensionContainer	*iE_Extensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_CellId_Broadcast_List_Item_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_CellId_Broadcast_List_Item;
extern asn_SEQUENCE_specifics_t asn_SPC_SBcAP_CellId_Broadcast_List_Item_specs_1;
extern asn_TYPE_member_t asn_MBR_SBcAP_CellId_Broadcast_List_Item_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SBcAP_ProtocolExtensionContainer.h>

#endif	/* _SBcAP_CellId_Broadcast_List_Item_H_ */
#include <asn_internal.h>
