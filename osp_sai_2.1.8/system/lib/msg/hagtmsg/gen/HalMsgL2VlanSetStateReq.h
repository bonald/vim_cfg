/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN"
 * 	found in "../l2/hal_msg_vlan.asn1"
 */

#ifndef	_HalMsgL2VlanSetStateReq_H_
#define	_HalMsgL2VlanSetStateReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2VlanSetStateReq */
typedef struct HalMsgL2VlanSetStateReq {
	long	 vid;
	long	 state;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanSetStateReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanSetStateReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VlanSetStateReq_H_ */
