/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#ifndef	_HalMsgQosInvalidFlowEntryReq_H_
#define	_HalMsgQosInvalidFlowEntryReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgQosFlowEntryId.h>
#include <HalMsgFlowRule.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgQosInvalidFlowEntryReq */
typedef struct HalMsgQosInvalidFlowEntryReq {
	long	 label;
	HalMsgQosFlowEntryId_t	 entryId;
	long	 invalid;
	HalMsgFlowRule_t	 rule;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgQosInvalidFlowEntryReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgQosInvalidFlowEntryReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgQosInvalidFlowEntryReq_H_ */
