/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HSRV2AGT-MSG"
 * 	found in "../hsrv2AgtMsg.asn1"
 */

#ifndef	_Hsrv2AgtMsgSynDoneReq_H_
#define	_Hsrv2AgtMsgSynDoneReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Hsrv2AgtMsgSynDoneReq */
typedef struct Hsrv2AgtMsgSynDoneReq {
	long	 done;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Hsrv2AgtMsgSynDoneReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Hsrv2AgtMsgSynDoneReq;

#ifdef __cplusplus
}
#endif

#endif	/* _Hsrv2AgtMsgSynDoneReq_H_ */