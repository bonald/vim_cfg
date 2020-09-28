/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-STATS"
 * 	found in "../common/hal_msg_stats.asn1"
 */

#ifndef	_HalMsgStatsResponseReq_H_
#define	_HalMsgStatsResponseReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgStatsResult.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgStatsResponseReq */
typedef struct HalMsgStatsResponseReq {
	long	 type;
	long	 notify;
	HalMsgStatsResult_t	 rslt;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgStatsResponseReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgStatsResponseReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgStatsResponseReq_H_ */