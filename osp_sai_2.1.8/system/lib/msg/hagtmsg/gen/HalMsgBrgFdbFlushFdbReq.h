/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#ifndef	_HalMsgBrgFdbFlushFdbReq_H_
#define	_HalMsgBrgFdbFlushFdbReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgBrgFdbFlushFdbReq */
typedef struct HalMsgBrgFdbFlushFdbReq {
	long	 type;
	long	 fid;
	long	 glPort;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBrgFdbFlushFdbReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBrgFdbFlushFdbReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgBrgFdbFlushFdbReq_H_ */
