/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#ifndef	_HalMsgQosUnsetPortPolicerReq_H_
#define	_HalMsgQosUnsetPortPolicerReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgQosUnsetPortPolicerReq */
typedef struct HalMsgQosUnsetPortPolicerReq {
	long	 port;
	long	 dir;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgQosUnsetPortPolicerReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgQosUnsetPortPolicerReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgQosUnsetPortPolicerReq_H_ */