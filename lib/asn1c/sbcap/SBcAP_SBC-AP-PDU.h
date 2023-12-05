/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-PDU-Descriptions"
 * 	found in "../../src/sbcap/asn1/SBC_AP_PDU_Descriptions.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_SBC_AP_PDU_H_
#define	_SBcAP_SBC_AP_PDU_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SBcAP_InitiatingMessage.h>
#include <SBcAP_SuccessfulOutcome.h>
#include <SBcAP_UnsuccessfulOutcome.h>
#include <constr_CHOICE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SBcAP_SBC_AP_PDU_PR {
	SBcAP_SBC_AP_PDU_PR_NOTHING,	/* No components present */
	SBcAP_SBC_AP_PDU_PR_initiatingMessage,
	SBcAP_SBC_AP_PDU_PR_successfulOutcome,
	SBcAP_SBC_AP_PDU_PR_unsuccessfulOutcome
	/* Extensions may appear below */
	
} SBcAP_SBC_AP_PDU_PR;

/* SBcAP_SBC-AP-PDU */
typedef struct SBcAP_SBC_AP_PDU {
	SBcAP_SBC_AP_PDU_PR present;
	union SBcAP_SBC_AP_PDU_u {
		SBcAP_InitiatingMessage_t	 initiatingMessage;
		SBcAP_SuccessfulOutcome_t	 successfulOutcome;
		SBcAP_UnsuccessfulOutcome_t	 unsuccessfulOutcome;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_SBC_AP_PDU_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_SBC_AP_PDU;

#ifdef __cplusplus
}
#endif

#endif	/* _SBcAP_SBC_AP_PDU_H_ */
#include <asn_internal.h>