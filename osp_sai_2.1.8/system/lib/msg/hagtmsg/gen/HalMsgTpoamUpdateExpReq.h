/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-TPOAM"
 * 	found in "../mpls/hal_msg_tpoam.asn1"
 */

#ifndef	_HalMsgTpoamUpdateExpReq_H_
#define	_HalMsgTpoamUpdateExpReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgTpoamLMepKey.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgTpoamUpdateExpReq */
typedef struct HalMsgTpoamUpdateExpReq {
	HalMsgTpoamLMepKey_t	 key;
	long	 exp;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgTpoamUpdateExpReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgTpoamUpdateExpReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgTpoamUpdateExpReq_H_ */
