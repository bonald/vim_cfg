/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#ifndef	_HalMsgBrgFdbEntryListRawReq_H_
#define	_HalMsgBrgFdbEntryListRawReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgBrgFdbEntryListRawReq */
typedef struct HalMsgBrgFdbEntryListRawReq {
	long	 count;
	PrintableString_t	 rawData;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBrgFdbEntryListRawReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBrgFdbEntryListRawReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgBrgFdbEntryListRawReq_H_ */
