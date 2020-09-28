/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ACL"
 * 	found in "../aclqos/hal_msg_acl.asn1"
 */

#ifndef	_HalMsgAddAccessGroupReq_H_
#define	_HalMsgAddAccessGroupReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct HalMsgAce;

/* HalMsgAddAccessGroupReq */
typedef struct HalMsgAddAccessGroupReq {
	long	 label;
	struct aceList {
		A_SEQUENCE_OF(struct HalMsgAce) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} aceList;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgAddAccessGroupReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgAddAccessGroupReq;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <HalMsgAce.h>

#endif	/* _HalMsgAddAccessGroupReq_H_ */