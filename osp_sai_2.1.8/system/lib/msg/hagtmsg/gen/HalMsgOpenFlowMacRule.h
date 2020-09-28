/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#ifndef	_HalMsgOpenFlowMacRule_H_
#define	_HalMsgOpenFlowMacRule_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowMacRule */
typedef struct HalMsgOpenFlowMacRule {
	long	 flag;
	HalMsgDefL2MacAddr_t	 macSa;
	HalMsgDefL2MacAddr_t	 macSaMask;
	HalMsgDefL2MacAddr_t	 macDa;
	HalMsgDefL2MacAddr_t	 macDaMask;
	long	 sTagValid;
	long	 sVlanId;
	long	 sVlanIdMask;
	long	 sTagCos;
	long	 sTagCosMask;
	long	 ipv4Pkt;
	long	 arpPkt;
	long	 mplsPkt;
	long	 ethType;
	long	 ethTypeMask;
	long	 cTagValid;
	long	 cVlanId;
	long	 cVlanIdMask;
	long	 cTagCos;
	long	 cTagCosMask;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowMacRule_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowMacRule;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowMacRule_H_ */
