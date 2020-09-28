/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#ifndef	_HalMsgOpenFlowAction_H_
#define	_HalMsgOpenFlowAction_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowAction */
typedef struct HalMsgOpenFlowAction {
	long	 policerId;
	long	 statsEn;
	long	 statsId;
	long	 priorityValid;
	long	 priority;
	long	 color;
	long	 nexthopId;
    long	 nexthopOffset;
	long	 denyRouting;
	long	 denyBridging;
	long	 denyLearning;
	long	 popAllMpls;
	long	 isDsMplsKey;
	long	 mplsActionType;
	long	 resetPayloadOffset;
	long	 oldLogSessionId;
	long	 logSessionId;
	long	 logDestPort;
	long	 truncateLen;
	long	 groupVlanId;
	long	 groupVlanCos;
	long	 groupVlanType;
	long	 stripPacket;
    long	 stripLen;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowAction_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowAction;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowAction_H_ */
