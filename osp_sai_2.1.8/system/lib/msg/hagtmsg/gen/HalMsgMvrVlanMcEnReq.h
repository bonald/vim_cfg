/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L3MC"
 * 	found in "../l3/hal_msg_l3mc.asn1"
 */

#ifndef	_HalMsgMvrVlanMcEnReq_H_
#define	_HalMsgMvrVlanMcEnReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMvrVlanMcEnReq */
typedef struct HalMsgMvrVlanMcEnReq {
	long	 l3ifId;
	long	 isEn;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMvrVlanMcEnReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMvrVlanMcEnReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMvrVlanMcEnReq_H_ */
