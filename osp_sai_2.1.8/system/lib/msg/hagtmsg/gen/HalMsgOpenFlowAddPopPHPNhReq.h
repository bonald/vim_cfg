/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-NEXTHOP"
 * 	found in "../openflow/hal_msg_openflow_nexthop.asn1"
 */

#ifndef	_HalMsgOpenFlowAddPopPHPNhReq_H_
#define	_HalMsgOpenFlowAddPopPHPNhReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowAddPopPHPNhReq */
typedef struct HalMsgOpenFlowAddPopPHPNhReq {
	long	 nhid;
	long	 offset;
	long	 gport;
	HalMsgDefL2MacAddr_t	 nhMac;
	long	 outerVlanVid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowAddPopPHPNhReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowAddPopPHPNhReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowAddPopPHPNhReq_H_ */