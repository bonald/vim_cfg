/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-PTP"
 * 	found in "../misc/hal_msg_ptp.asn1"
 */

#ifndef	_HalMsgPtpSetGlobalEnableReq_H_
#define	_HalMsgPtpSetGlobalEnableReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPtpSetGlobalEnableReq */
typedef struct HalMsgPtpSetGlobalEnableReq {
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPtpSetGlobalEnableReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPtpSetGlobalEnableReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPtpSetGlobalEnableReq_H_ */
