/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-FLOW"
 * 	found in "../aclqos/hal_msg_flow.asn1"
 */

#ifndef	_HalMsgFlowMacRule_H_
#define	_HalMsgFlowMacRule_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgIpv4addr.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgFlowMacRule */
typedef struct HalMsgFlowMacRule {
	long	 flag;
	HalMsgIpv4addr_t	 senderIp;
	HalMsgIpv4addr_t	 senderIpMask;
	HalMsgIpv4addr_t	 targetIp;
	HalMsgIpv4addr_t	 targetIpMask;
	HalMsgDefL2MacAddr_t	 macSa;
	HalMsgDefL2MacAddr_t	 macSaMask;
	HalMsgDefL2MacAddr_t	 macDa;
	HalMsgDefL2MacAddr_t	 macDaMask;
	long	 cVlanId;
	long	 sVlanId;
	long	 cTagCos;
	long	 sTagCos;
	long	 l3Type;
	long	 l2Type;
	long	 ethType;
	long	 ethTypeMask;
	long	 arpOpCode;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgFlowMacRule_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgFlowMacRule;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgFlowMacRule_H_ */
