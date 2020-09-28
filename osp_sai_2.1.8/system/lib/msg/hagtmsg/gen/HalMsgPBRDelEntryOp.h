/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-PBR"
 * 	found in "../l3/hal_msg_pbr.asn1"
 */

#ifndef	_HalMsgPBRDelEntryOp_H_
#define	_HalMsgPBRDelEntryOp_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum HalMsgPBRDelEntryOp {
	HalMsgPBRDelEntryOp_removefromlabel	= 0,
	HalMsgPBRDelEntryOp_removefromentryid	= 1
} HalMsgPBRDelEntryOp_e;

/* HalMsgPBRDelEntryOp */
typedef long	 HalMsgPBRDelEntryOp_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPBRDelEntryOp;
asn_struct_free_f HalMsgPBRDelEntryOp_free;
asn_struct_print_f HalMsgPBRDelEntryOp_print;
asn_constr_check_f HalMsgPBRDelEntryOp_constraint;
ber_type_decoder_f HalMsgPBRDelEntryOp_decode_ber;
der_type_encoder_f HalMsgPBRDelEntryOp_encode_der;
xer_type_decoder_f HalMsgPBRDelEntryOp_decode_xer;
xer_type_encoder_f HalMsgPBRDelEntryOp_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPBRDelEntryOp_H_ */
