/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-PORT"
 * 	found in "../openflow/hal_msg_openflow_port.asn1"
 */

#ifndef	_HalMsgOpenFlowPortSetConfigReq_H_
#define	_HalMsgOpenFlowPortSetConfigReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowPortSetConfigReq */
typedef struct HalMsgOpenFlowPortSetConfigReq {
	long	 gport;
	long	 portConfig;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowPortSetConfigReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowPortSetConfigReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowPortSetConfigReq_H_ */