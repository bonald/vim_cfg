/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-G8031"
 * 	found in "../l2/hal_msg_g8031.asn1"
 */

#ifndef	_HalMsgG8031GroupDelReq_H_
#define	_HalMsgG8031GroupDelReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgG8031GroupDelReq */
typedef struct HalMsgG8031GroupDelReq {
	long	 groupid;
	long	 nhid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgG8031GroupDelReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgG8031GroupDelReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgG8031GroupDelReq_H_ */