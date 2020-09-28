/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#ifndef	_HalMsgQosSetPortQueueDropReq_H_
#define	_HalMsgQosSetPortQueueDropReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgQosQueueDrop.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgQosSetPortQueueDropReq */
typedef struct HalMsgQosSetPortQueueDropReq {
	long	 port;
	long	 qid;
	HalMsgQosQueueDrop_t	 drop;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgQosSetPortQueueDropReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgQosSetPortQueueDropReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgQosSetPortQueueDropReq_H_ */