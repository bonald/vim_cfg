/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#ifndef	_HalMsgBrgSetDiscardReq_H_
#define	_HalMsgBrgSetDiscardReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgBrgSetDiscardReq */
typedef struct HalMsgBrgSetDiscardReq {
	long	 fid;
	long	 type;
	long	 discard;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBrgSetDiscardReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBrgSetDiscardReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgBrgSetDiscardReq_H_ */
