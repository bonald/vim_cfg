/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-STATS"
 * 	found in "../openflow/hal_msg_openflow_stats.asn1"
 */

#ifndef	_HalMsgOpenFlowGetStatsReq_H_
#define	_HalMsgOpenFlowGetStatsReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgOpenFlowStatsResourceType.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowGetStatsReq */
typedef struct HalMsgOpenFlowGetStatsReq {
	long	 statsId;
	HalMsgOpenFlowStatsResourceType_t	 resourceType;
	long	 resourceId;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowGetStatsReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowGetStatsReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowGetStatsReq_H_ */
