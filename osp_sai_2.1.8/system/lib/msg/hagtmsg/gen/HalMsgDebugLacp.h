/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-LACP"
 * 	found in "../l2/hal_msg_agg.asn1"
 */

#ifndef	_HalMsgDebugLacp_H_
#define	_HalMsgDebugLacp_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgDebugLacp */
typedef struct HalMsgDebugLacp {
	long	 tidorPortorVid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgDebugLacp_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgDebugLacp;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgDebugLacp_H_ */