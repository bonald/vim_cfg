/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-PBR"
 * 	found in "../l3/hal_msg_pbr.asn1"
 */

#ifndef	_HalMsgPBRAction_H_
#define	_HalMsgPBRAction_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPBRAction */
typedef struct HalMsgPBRAction {
	long	 flag;
	long	 nhid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPBRAction_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPBRAction;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPBRAction_H_ */
