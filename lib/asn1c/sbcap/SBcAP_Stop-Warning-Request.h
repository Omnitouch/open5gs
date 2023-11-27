/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-PDU-Contents"
 * 	found in "../../src/sbcap/asn1/SBC_AP_PDU_Contents.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_Stop_Warning_Request_H_
#define	_SBcAP_Stop_Warning_Request_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SBcAP_ProtocolIE-Container.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBcAP_ProtocolExtensionContainer;

/* SBcAP_Stop-Warning-Request */
typedef struct SBcAP_Stop_Warning_Request {
	SBcAP_ProtocolIE_Container_86P2_t	 protocolIEs;
	struct SBcAP_ProtocolExtensionContainer	*protocolExtensions;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_Stop_Warning_Request_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_Stop_Warning_Request;
extern asn_SEQUENCE_specifics_t asn_SPC_SBcAP_Stop_Warning_Request_specs_1;
extern asn_TYPE_member_t asn_MBR_SBcAP_Stop_Warning_Request_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SBcAP_ProtocolExtensionContainer.h>

#endif	/* _SBcAP_Stop_Warning_Request_H_ */
#include <asn_internal.h>
