/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SBC-AP-Containers"
 * 	found in "../../src/sbcap/asn1/SBC_AP_Containers.asn"
 * 	`asn1c -S /home/pespin/dev/sysmocom/build/new/out/share/asn1c -fcompound-names -gen-APER -no-gen-BER -no-gen-XER -no-gen-JER -no-gen-OER -no-gen-UPER -no-gen-example`
 */

#ifndef	_SBcAP_ProtocolIE_Container_H_
#define	_SBcAP_ProtocolIE_Container_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SBcAP_Write_Replace_Warning_Request_IEs;
struct SBcAP_Write_Replace_Warning_Response_IEs;
struct SBcAP_Stop_Warning_Request_IEs;
struct SBcAP_Stop_Warning_Response_IEs;
struct SBcAP_Write_Replace_Warning_Indication_IEs;
struct SBcAP_Stop_Warning_Indication_IEs;
struct SBcAP_PWS_Restart_Indication_IEs;
struct SBcAP_PWS_Failure_Indication_IEs;
struct SBcAP_ErrorIndicationIEs;

/* SBcAP_ProtocolIE-Container */
typedef struct SBcAP_ProtocolIE_Container_86P0 {
	A_SEQUENCE_OF(struct SBcAP_Write_Replace_Warning_Request_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P0_t;
typedef struct SBcAP_ProtocolIE_Container_86P1 {
	A_SEQUENCE_OF(struct SBcAP_Write_Replace_Warning_Response_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P1_t;
typedef struct SBcAP_ProtocolIE_Container_86P2 {
	A_SEQUENCE_OF(struct SBcAP_Stop_Warning_Request_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P2_t;
typedef struct SBcAP_ProtocolIE_Container_86P3 {
	A_SEQUENCE_OF(struct SBcAP_Stop_Warning_Response_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P3_t;
typedef struct SBcAP_ProtocolIE_Container_86P4 {
	A_SEQUENCE_OF(struct SBcAP_Write_Replace_Warning_Indication_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P4_t;
typedef struct SBcAP_ProtocolIE_Container_86P5 {
	A_SEQUENCE_OF(struct SBcAP_Stop_Warning_Indication_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P5_t;
typedef struct SBcAP_ProtocolIE_Container_86P6 {
	A_SEQUENCE_OF(struct SBcAP_PWS_Restart_Indication_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P6_t;
typedef struct SBcAP_ProtocolIE_Container_86P7 {
	A_SEQUENCE_OF(struct SBcAP_PWS_Failure_Indication_IEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P7_t;
typedef struct SBcAP_ProtocolIE_Container_86P8 {
	A_SEQUENCE_OF(struct SBcAP_ErrorIndicationIEs) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SBcAP_ProtocolIE_Container_86P8_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P0;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P0_specs_1;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P0_1[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P0_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P1;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P1_specs_3;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P1_3[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P1_constr_3;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P2;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P2_specs_5;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P2_5[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P2_constr_5;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P3;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P3_specs_7;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P3_7[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P3_constr_7;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P4;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P4_specs_9;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P4_9[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P4_constr_9;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P5;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P5_specs_11;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P5_11[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P5_constr_11;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P6;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P6_specs_13;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P6_13[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P6_constr_13;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P7;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P7_specs_15;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P7_15[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P7_constr_15;
extern asn_TYPE_descriptor_t asn_DEF_SBcAP_ProtocolIE_Container_86P8;
extern asn_SET_OF_specifics_t asn_SPC_SBcAP_ProtocolIE_Container_86P8_specs_17;
extern asn_TYPE_member_t asn_MBR_SBcAP_ProtocolIE_Container_86P8_17[1];
extern asn_per_constraints_t asn_PER_type_SBcAP_ProtocolIE_Container_86P8_constr_17;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SBcAP_ProtocolIE-Field.h>

#endif	/* _SBcAP_ProtocolIE_Container_H_ */
#include <asn_internal.h>
