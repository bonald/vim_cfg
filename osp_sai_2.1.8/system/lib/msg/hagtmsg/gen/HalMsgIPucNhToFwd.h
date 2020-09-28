/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#ifndef	_HalMsgIPucNhToFwd_H_
#define	_HalMsgIPucNhToFwd_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalIPucNexthop.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIPucNhToFwd */
typedef struct HalMsgIPucNhToFwd {
	long	 nhid;
	HalIPucNexthop_t	 nhInfo;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIPucNhToFwd_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIPucNhToFwd;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIPucNhToFwd_H_ */
