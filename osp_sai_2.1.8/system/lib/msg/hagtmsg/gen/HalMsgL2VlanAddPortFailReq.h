/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN"
 * 	found in "../l2/hal_msg_vlan.asn1"
 */

#ifndef	_HalMsgL2VlanAddPortFailReq_H_
#define	_HalMsgL2VlanAddPortFailReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2VlanAddPortFailReq */
typedef struct HalMsgL2VlanAddPortFailReq {
	long	 port;
	long	 fid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanAddPortFailReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanAddPortFailReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VlanAddPortFailReq_H_ */