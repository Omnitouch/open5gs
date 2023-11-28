/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-CommonDataTypes"
 * 	found in "../../src/sbcap/asn1/SBC_AP_CommonDataTypes.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_ProcedureCode_H_
#define	_SBcAP_ProcedureCode_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SBcAP_ProcedureCode */
typedef long	 SBcAP_ProcedureCode_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_SBcAP_ProcedureCode_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProcedureCode;
asn_struct_free_f SBcAP_ProcedureCode_free;
asn_struct_print_f SBcAP_ProcedureCode_print;
asn_constr_check_f SBcAP_ProcedureCode_constraint;
per_type_decoder_f SBcAP_ProcedureCode_decode_aper;
per_type_encoder_f SBcAP_ProcedureCode_encode_aper;
#define SBcAP_ProcedureCode_id_Write_Replace_Warning ((SBcAP_ProcedureCode_t)0)
#define SBcAP_ProcedureCode_id_Stop_Warning ((SBcAP_ProcedureCode_t)1)
#define SBcAP_ProcedureCode_id_Error_Indication ((SBcAP_ProcedureCode_t)2)
#define SBcAP_ProcedureCode_id_Write_Replace_Warning_Indication ((SBcAP_ProcedureCode_t)3)
#define SBcAP_ProcedureCode_id_Stop_Warning_Indication ((SBcAP_ProcedureCode_t)4)
#define SBcAP_ProcedureCode_id_PWS_Restart_Indication ((SBcAP_ProcedureCode_t)5)
#define SBcAP_ProcedureCode_id_PWS_Failure_Indication ((SBcAP_ProcedureCode_t)6)

#ifdef __cplusplus
}
#endif

#endif	/* _SBcAP_ProcedureCode_H_ */
#include <asn_internal.h>
