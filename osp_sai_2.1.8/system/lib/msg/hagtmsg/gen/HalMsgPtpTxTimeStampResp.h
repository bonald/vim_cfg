/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-PTP"
 * 	found in "../misc/hal_msg_ptp.asn1"
 */

#ifndef	_HalMsgPtpTxTimeStampResp_H_
#define	_HalMsgPtpTxTimeStampResp_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgPtpTimeInternal.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPtpTxTimeStampResp */
typedef struct HalMsgPtpTxTimeStampResp {
	long	 gport;
	HalMsgPtpTimeInternal_t	 ts;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPtpTxTimeStampResp_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPtpTxTimeStampResp;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPtpTxTimeStampResp_H_ */
