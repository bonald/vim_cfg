/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-SFLOW"
 * 	found in "../misc/hal_msg_sflow.asn1"
 */

#ifndef	_HalMsgSflowSetSampleReq_H_
#define	_HalMsgSflowSetSampleReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgSflowSetSampleReq */
typedef struct HalMsgSflowSetSampleReq {
	struct gportList {
		A_SEQUENCE_OF(long) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} gportList;
	long	 dir;
	long	 enable;
	long	 weight;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgSflowSetSampleReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgSflowSetSampleReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgSflowSetSampleReq_H_ */
