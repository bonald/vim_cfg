/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#ifndef	_HalMsgOpenFlowSetCtpidReq_H_
#define	_HalMsgOpenFlowSetCtpidReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowSetCtpidReq */
typedef struct HalMsgOpenFlowSetCtpidReq {
	long	 ctpid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowSetCtpidReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowSetCtpidReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowSetCtpidReq_H_ */