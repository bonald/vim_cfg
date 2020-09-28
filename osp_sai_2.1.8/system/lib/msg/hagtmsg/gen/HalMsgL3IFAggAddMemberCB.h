/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L3IF"
 * 	found in "../intf/hal_msg_l3if.asn1"
 */

#ifndef	_HalMsgL3IFAggAddMemberCB_H_
#define	_HalMsgL3IFAggAddMemberCB_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL3IFAggAddMemberCB */
typedef struct HalMsgL3IFAggAddMemberCB {
	long	 portid;
	long	 l3ifid;
	long	 flagAgg;
	long	 mtu;
	long	 vrfid;
	long	 txEn;
	long	 rxEn;
	long	 flagBmp;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL3IFAggAddMemberCB_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL3IFAggAddMemberCB;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL3IFAggAddMemberCB_H_ */