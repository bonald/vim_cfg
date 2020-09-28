/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN"
 * 	found in "../l2/hal_msg_vlan.asn1"
 */

#ifndef	_HalMsgL2VoiceVlanEntrySetReq_H_
#define	_HalMsgL2VoiceVlanEntrySetReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2VoiceVlanEntrySetReq */
typedef struct HalMsgL2VoiceVlanEntrySetReq {
	long	 entryId;
	long	 vid;
	HalMsgDefL2MacAddr_t	 mac;
	HalMsgDefL2MacAddr_t	 mask;
	long	 cos;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VoiceVlanEntrySetReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VoiceVlanEntrySetReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VoiceVlanEntrySetReq_H_ */