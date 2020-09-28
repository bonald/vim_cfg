/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L3MC"
 * 	found in "../l3/hal_msg_l3mc.asn1"
 */

#ifndef	_HalMsgL3mcStatResponse_H_
#define	_HalMsgL3mcStatResponse_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL3mcStatResponse */
typedef struct HalMsgL3mcStatResponse {
	long	 group;
	long	 source;
	OCTET_STRING_t	 byteCnt;
	OCTET_STRING_t	 pktCnt;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL3mcStatResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL3mcStatResponse;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL3mcStatResponse_H_ */
