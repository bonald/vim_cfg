/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-TUNNEL"
 * 	found in "../openflow/hal_msg_openflow_tunnel.asn1"
 */

#ifndef	_HalMsgOpenFlowTnlDecapModeReq_H_
#define	_HalMsgOpenFlowTnlDecapModeReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowTnlDecapModeReq */
typedef struct HalMsgOpenFlowTnlDecapModeReq {
	long	 decapMode;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowTnlDecapModeReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowTnlDecapModeReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowTnlDecapModeReq_H_ */