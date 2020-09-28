/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN-MAPPING"
 * 	found in "../l2/hal_msg_vlan_mapping.asn1"
 */

#ifndef	_HalMsgL2VlanMappingEntryDefaultVidType_H_
#define	_HalMsgL2VlanMappingEntryDefaultVidType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum HalMsgL2VlanMappingEntryDefaultVidType {
	HalMsgL2VlanMappingEntryDefaultVidType_noAddOrDelete	= 0,
	HalMsgL2VlanMappingEntryDefaultVidType_onlyAddDefaultVidEntry	= 1,
	HalMsgL2VlanMappingEntryDefaultVidType_addDefaultVidEntry	= 2,
	HalMsgL2VlanMappingEntryDefaultVidType_onlyDeleteDefaultVidEntry	= 3,
	HalMsgL2VlanMappingEntryDefaultVidType_deleteDefaultVidEntry	= 4
} HalMsgL2VlanMappingEntryDefaultVidType_e;

/* HalMsgL2VlanMappingEntryDefaultVidType */
typedef long	 HalMsgL2VlanMappingEntryDefaultVidType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanMappingEntryDefaultVidType;
asn_struct_free_f HalMsgL2VlanMappingEntryDefaultVidType_free;
asn_struct_print_f HalMsgL2VlanMappingEntryDefaultVidType_print;
asn_constr_check_f HalMsgL2VlanMappingEntryDefaultVidType_constraint;
ber_type_decoder_f HalMsgL2VlanMappingEntryDefaultVidType_decode_ber;
der_type_encoder_f HalMsgL2VlanMappingEntryDefaultVidType_encode_der;
xer_type_decoder_f HalMsgL2VlanMappingEntryDefaultVidType_decode_xer;
xer_type_encoder_f HalMsgL2VlanMappingEntryDefaultVidType_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VlanMappingEntryDefaultVidType_H_ */
