/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#ifndef	_HalMsgBrgSystemEntryListReq_H_
#define	_HalMsgBrgSystemEntryListReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgDefErrorCode.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct HalMsgBrgSystemEntryReq;

/* HalMsgBrgSystemEntryListReq */
typedef struct HalMsgBrgSystemEntryListReq {
	HalMsgDefErrorCode_t	 err;
	struct entryArray {
		A_SEQUENCE_OF(struct HalMsgBrgSystemEntryReq) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} entryArray;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBrgSystemEntryListReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBrgSystemEntryListReq;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <HalMsgBrgSystemEntryReq.h>

#endif	/* _HalMsgBrgSystemEntryListReq_H_ */
