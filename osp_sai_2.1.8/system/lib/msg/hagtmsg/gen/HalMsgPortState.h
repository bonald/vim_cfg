/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN"
 * 	found in "../l2/hal_msg_vlan.asn1"
 */

#ifndef	_HalMsgPortState_H_
#define	_HalMsgPortState_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPortState */
typedef struct HalMsgPortState {
	long	 port;
	long	 setDefEntry;
	long	 maxGchipNum;
	long	 setVlanStatus;
	long	 localMemberBmp;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPortState_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPortState;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPortState_H_ */