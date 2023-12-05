/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-IEs"
 * 	found in "../../src/sbcap/asn1/SBC_AP_IEs.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_Stop_All_Indicator_H_
#define	_SBcAP_Stop_All_Indicator_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SBcAP_Stop_All_Indicator {
	SBcAP_Stop_All_Indicator_true	= 0
} e_SBcAP_Stop_All_Indicator;

/* SBcAP_Stop-All-Indicator */
typedef long	 SBcAP_Stop_All_Indicator_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_SBcAP_Stop_All_Indicator_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_Stop_All_Indicator;
extern const asn_INTEGER_specifics_t asn_SPC_SBcAP_Stop_All_Indicator_specs_1;
asn_struct_free_f SBcAP_Stop_All_Indicator_free;
asn_struct_print_f SBcAP_Stop_All_Indicator_print;
asn_constr_check_f SBcAP_Stop_All_Indicator_constraint;
per_type_decoder_f SBcAP_Stop_All_Indicator_decode_aper;
per_type_encoder_f SBcAP_Stop_All_Indicator_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _SBcAP_Stop_All_Indicator_H_ */
#include <asn_internal.h>
