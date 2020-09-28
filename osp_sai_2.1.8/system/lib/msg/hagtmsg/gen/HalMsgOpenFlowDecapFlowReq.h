/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-TUNNEL"
 * 	found in "../openflow/hal_msg_openflow_tunnel.asn1"
 */

#ifndef	_HalMsgOpenFlowDecapFlowReq_H_
#define	_HalMsgOpenFlowDecapFlowReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgOpenDecapFlowRule.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowDecapFlowReq */
typedef struct HalMsgOpenFlowDecapFlowReq {
	long	 decapMode;
	long	 type;
	long	 fid;
	HalMsgOpenDecapFlowRule_t	 tnl;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowDecapFlowReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowDecapFlowReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowDecapFlowReq_H_ */