/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#ifndef	_HalMsgQosFlowPolicerOpReq_H_
#define	_HalMsgQosFlowPolicerOpReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgQosFlowPolicerOp.h>
#include <NativeInteger.h>
#include <HalMsgQosPolicer.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgQosFlowPolicerOpReq */
typedef struct HalMsgQosFlowPolicerOpReq {
	HalMsgQosFlowPolicerOp_t	 opCode;
	long	 policerId;
	HalMsgQosPolicer_t	 policer;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgQosFlowPolicerOpReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgQosFlowPolicerOpReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgQosFlowPolicerOpReq_H_ */