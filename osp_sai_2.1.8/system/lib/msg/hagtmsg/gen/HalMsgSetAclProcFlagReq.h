/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ACL"
 * 	found in "../aclqos/hal_msg_acl.asn1"
 */

#ifndef	_HalMsgSetAclProcFlagReq_H_
#define	_HalMsgSetAclProcFlagReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgSetAclProcFlagReq */
typedef struct HalMsgSetAclProcFlagReq {
	long	 flag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgSetAclProcFlagReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgSetAclProcFlagReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgSetAclProcFlagReq_H_ */
