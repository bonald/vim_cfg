/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#ifndef	_HalMsgOpenFlowIpv4Rule_H_
#define	_HalMsgOpenFlowIpv4Rule_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgIpv4addr.h>
#include <HalMsgOpenFlowL4PortType.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowIpv4Rule */
typedef struct HalMsgOpenFlowIpv4Rule {
	long	 flag;
	HalMsgIpv4addr_t	 ipSa;
	HalMsgIpv4addr_t	 ipSaMask;
	HalMsgIpv4addr_t	 ipDa;
	HalMsgIpv4addr_t	 ipDaMask;
	HalMsgOpenFlowL4PortType_t	 l4SrcPort;
	HalMsgOpenFlowL4PortType_t	 l4DestPort;
	long	 dscp;
	long	 dscpMask;
	long	 l4Protocol;
	long	 icmpType;
	long	 icmpCode;
	long	 igmpType;
	long	 tcpFlags;
	long	 ipFrag;
	long	 ipFragMask;
	long	 ecn;
	long	 ecnMask;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowIpv4Rule_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowIpv4Rule;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowIpv4Rule_H_ */
