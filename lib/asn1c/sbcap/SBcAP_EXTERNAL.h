/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ASN1C-UsefulInformationObjectClasses"
 * 	found in "/home/pespin/dev/sysmocom/build/new/out/share/asn1c/standard-modules/ASN1C-UsefulInformationObjectClasses.asn1"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_EXTERNAL_H_
#define	_SBcAP_EXTERNAL_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OBJECT_IDENTIFIER.h>
#include <NativeInteger.h>
#include <ObjectDescriptor.h>
#include <ANY.h>
#include <OCTET_STRING.h>
#include <BIT_STRING.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SBcAP_EXTERNAL__encoding_PR {
	SBcAP_EXTERNAL__encoding_PR_NOTHING,	/* No components present */
	SBcAP_EXTERNAL__encoding_PR_single_ASN1_type,
	SBcAP_EXTERNAL__encoding_PR_octet_aligned,
	SBcAP_EXTERNAL__encoding_PR_arbitrary
} SBcAP_EXTERNAL__encoding_PR;

/* SBcAP_EXTERNAL */
typedef struct SBcAP_EXTERNAL {
	OBJECT_IDENTIFIER_t	*direct_reference;	/* OPTIONAL */
	long	*indirect_reference;	/* OPTIONAL */
	ObjectDescriptor_t	*data_value_descriptor;	/* OPTIONAL */
	struct SBcAP_EXTERNAL__encoding {
		SBcAP_EXTERNAL__encoding_PR present;
		union SBcAP_EXTERNAL__SBcAP_encoding_u {
			ANY_t	 single_ASN1_type;
			OCTET_STRING_t	 octet_aligned;
			BIT_STRING_t	 arbitrary;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} encoding;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_EXTERNAL_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_EXTERNAL;

#ifdef __cplusplus
}
#endif

#endif	/* _SBcAP_EXTERNAL_H_ */
#include <asn_internal.h>
