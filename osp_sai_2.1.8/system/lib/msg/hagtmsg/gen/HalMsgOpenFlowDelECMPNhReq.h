/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-NEXTHOP"
 * 	found in "../openflow/hal_msg_openflow_nexthop.asn1"
 */

#ifndef	_HalMsgOpenFlowDelECMPNhReq_H_
#define	_HalMsgOpenFlowDelECMPNhReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowDelECMPNhReq */
typedef struct HalMsgOpenFlowDelECMPNhReq {
	long	 nhid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowDelECMPNhReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowDelECMPNhReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowDelECMPNhReq_H_ */