/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-RATELIMIT"
 * 	found in "../l2/hal_msg_ratelimit.asn1"
 */

#ifndef	_HalMsgL2RatelimitSetIpgEnReq_H_
#define	_HalMsgL2RatelimitSetIpgEnReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2RatelimitSetIpgEnReq */
typedef struct HalMsgL2RatelimitSetIpgEnReq {
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2RatelimitSetIpgEnReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2RatelimitSetIpgEnReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2RatelimitSetIpgEnReq_H_ */
