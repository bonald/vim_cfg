/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-EFD"
 * 	found in "../misc/hal_msg_efd.asn1"
 */

#ifndef	_HalMsgFlowColorSetReq_H_
#define	_HalMsgFlowColorSetReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgFlowColorSetReq */
typedef struct HalMsgFlowColorSetReq {
	long	 color;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgFlowColorSetReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgFlowColorSetReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgFlowColorSetReq_H_ */
