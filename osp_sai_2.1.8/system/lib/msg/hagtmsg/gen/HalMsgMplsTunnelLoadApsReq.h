/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#ifndef	_HalMsgMplsTunnelLoadApsReq_H_
#define	_HalMsgMplsTunnelLoadApsReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMplsTunnelLoadApsReq */
typedef struct HalMsgMplsTunnelLoadApsReq {
	long	 apsgroupid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMplsTunnelLoadApsReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMplsTunnelLoadApsReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMplsTunnelLoadApsReq_H_ */
