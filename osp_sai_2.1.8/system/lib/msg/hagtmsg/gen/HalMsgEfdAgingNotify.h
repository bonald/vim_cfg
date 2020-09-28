/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-EFD"
 * 	found in "../misc/hal_msg_efd.asn1"
 */

#ifndef	_HalMsgEfdAgingNotify_H_
#define	_HalMsgEfdAgingNotify_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgEfdAgingNotify */
typedef struct HalMsgEfdAgingNotify {
	struct flowIdList {
		A_SEQUENCE_OF(long) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} flowIdList;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgEfdAgingNotify_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgEfdAgingNotify;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgEfdAgingNotify_H_ */
