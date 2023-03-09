/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "../support/s1ap-r17.3.0/36413-h30.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps -no-gen-BER -no-gen-XER -no-gen-OER -no-gen-UPER`
 */

#ifndef	_S1AP_GWContextReleaseIndication_H_
#define	_S1AP_GWContextReleaseIndication_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum S1AP_GWContextReleaseIndication {
	S1AP_GWContextReleaseIndication_true	= 0
	/*
	 * Enumeration is extensible
	 */
} e_S1AP_GWContextReleaseIndication;

/* S1AP_GWContextReleaseIndication */
typedef long	 S1AP_GWContextReleaseIndication_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_S1AP_GWContextReleaseIndication_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_GWContextReleaseIndication;
extern const asn_INTEGER_specifics_t asn_SPC_S1AP_GWContextReleaseIndication_specs_1;
asn_struct_free_f S1AP_GWContextReleaseIndication_free;
asn_struct_print_f S1AP_GWContextReleaseIndication_print;
asn_constr_check_f S1AP_GWContextReleaseIndication_constraint;
jer_type_encoder_f S1AP_GWContextReleaseIndication_encode_jer;
per_type_decoder_f S1AP_GWContextReleaseIndication_decode_aper;
per_type_encoder_f S1AP_GWContextReleaseIndication_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1AP_GWContextReleaseIndication_H_ */
#include <asn_internal.h>
