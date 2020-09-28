/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-G8031"
 * 	found in "../l2/hal_msg_g8031.asn1"
 */

#ifndef	_HalMsgG8031GroupAddVlanReq_H_
#define	_HalMsgG8031GroupAddVlanReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgG8031GroupAddVlanReq */
typedef struct HalMsgG8031GroupAddVlanReq {
	long	 groupid;
	long	 workingport;
	long	 protectingport;
	long	 nhid;
	long	 fid;
	long	 chipnum;
	long	 chipbmp;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgG8031GroupAddVlanReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgG8031GroupAddVlanReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgG8031GroupAddVlanReq_H_ */