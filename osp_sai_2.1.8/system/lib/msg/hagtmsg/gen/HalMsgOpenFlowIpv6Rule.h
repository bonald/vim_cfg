/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#ifndef	_HalMsgOpenFlowIpv6Rule_H_
#define	_HalMsgOpenFlowIpv6Rule_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgIpv6addr.h>
#include <HalMsgOpenFlowL4PortType.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowIpv6Rule */
typedef struct HalMsgOpenFlowIpv6Rule {
	long	 flag;
	HalMsgIpv6addr_t	 sprefix;
	HalMsgIpv6addr_t	 sprefixMask;
	HalMsgIpv6addr_t	 dprefix;
	HalMsgIpv6addr_t	 dprefixMask;
	HalMsgOpenFlowL4PortType_t	 l4SrcPort;
	HalMsgOpenFlowL4PortType_t	 l4DestPort;
	long	 ipOptions;
	long	 routedPacket;
	long	 l4Protocol;
	long	 icmpType;
	long	 icmpCode;
	long	 dscp;
	long	 dscpMask;
	long	 ipFrag;
	long	 ipFragMask;
	long	 ecn;
	long	 ecnMask;
	long	 flowLabel;
	long	 flowLabelMask;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowIpv6Rule_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowIpv6Rule;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowIpv6Rule_H_ */
