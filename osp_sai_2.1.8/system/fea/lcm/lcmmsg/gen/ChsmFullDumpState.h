/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmFullDumpState_H_
#define	_ChsmFullDumpState_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ChsmFullDumpState {
	ChsmFullDumpState_fullDumpInit	= 0,
	ChsmFullDumpState_fullDumpRequest	= 1,
	ChsmFullDumpState_fullDumpStart	= 2,
	ChsmFullDumpState_fullDumpEnd	= 3
} ChsmFullDumpState_e;

/* ChsmFullDumpState */
typedef long	 ChsmFullDumpState_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmFullDumpState;
asn_struct_free_f ChsmFullDumpState_free;
asn_struct_print_f ChsmFullDumpState_print;
asn_constr_check_f ChsmFullDumpState_constraint;
ber_type_decoder_f ChsmFullDumpState_decode_ber;
der_type_encoder_f ChsmFullDumpState_encode_der;
xer_type_decoder_f ChsmFullDumpState_decode_xer;
xer_type_encoder_f ChsmFullDumpState_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmFullDumpState_H_ */