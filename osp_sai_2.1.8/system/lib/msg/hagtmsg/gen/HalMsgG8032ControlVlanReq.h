/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-G8032"
 * 	found in "../l2/hal_msg_g8032.asn1"
 */

#ifndef	_HalMsgG8032ControlVlanReq_H_
#define	_HalMsgG8032ControlVlanReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgG8032ControlVlanReq */
typedef struct HalMsgG8032ControlVlanReq {
	long	 vid;
	long	 fid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgG8032ControlVlanReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgG8032ControlVlanReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgG8032ControlVlanReq_H_ */