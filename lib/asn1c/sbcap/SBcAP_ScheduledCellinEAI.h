/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../../src/sbcap/asn1/SBC_AP_IEs.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_ScheduledCellinEAI_H_
#define	_SBcAP_ScheduledCellinEAI_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBcAP_ScheduledCellinEAI_Item;

/* SBcAP_ScheduledCellinEAI */
typedef struct SBcAP_ScheduledCellinEAI {
	A_SEQUENCE_OF(struct SBcAP_ScheduledCellinEAI_Item) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ScheduledCellinEAI_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ScheduledCellinEAI;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ScheduledCellinEAI_specs_1;
extern asn_TYPE_member_t asn_MBR_SBcAP_ScheduledCellinEAI_1[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ScheduledCellinEAI_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SBcAP_ScheduledCellinEAI-Item.h>

#endif	/* _SBcAP_ScheduledCellinEAI_H_ */
#include <asn_internal.h>
