/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-STATS"
 * 	found in "../common/hal_msg_stats.asn1"
 */

#ifndef	_HalMsgPortQueueKey_H_
#define	_HalMsgPortQueueKey_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPortQueueKey */
typedef struct HalMsgPortQueueKey {
	long	 port;
	long	 qid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPortQueueKey_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPortQueueKey;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPortQueueKey_H_ */