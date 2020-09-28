/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-DOT1X"
 * 	found in "../l2/hal_msg_dot1x.asn1"
 */

#ifndef	_Dot1xAuthType_H_
#define	_Dot1xAuthType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum Dot1xAuthType {
	Dot1xAuthType_portEn	= 0,
	Dot1xAuthType_portDis	= 1,
	Dot1xAuthType_macEn	= 2,
	Dot1xAuthType_macDis	= 3,
	Dot1xAuthType_portAuth	= 4,
	Dot1xAuthType_portUnauth	= 5,
	Dot1xAuthType_macAuth	= 6,
	Dot1xAuthType_macUnauth	= 7
} Dot1xAuthType_e;

/* Dot1xAuthType */
typedef long	 Dot1xAuthType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Dot1xAuthType;
asn_struct_free_f Dot1xAuthType_free;
asn_struct_print_f Dot1xAuthType_print;
asn_constr_check_f Dot1xAuthType_constraint;
ber_type_decoder_f Dot1xAuthType_decode_ber;
der_type_encoder_f Dot1xAuthType_encode_der;
xer_type_decoder_f Dot1xAuthType_decode_xer;
xer_type_encoder_f Dot1xAuthType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _Dot1xAuthType_H_ */
