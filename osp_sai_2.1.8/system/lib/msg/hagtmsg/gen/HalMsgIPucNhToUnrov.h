/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#ifndef	_HalMsgIPucNhToUnrov_H_
#define	_HalMsgIPucNhToUnrov_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIPucNhToUnrov */
typedef struct HalMsgIPucNhToUnrov {
	long	 nhid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIPucNhToUnrov_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIPucNhToUnrov;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIPucNhToUnrov_H_ */