/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#ifndef	_HalIPucNhidInfo_H_
#define	_HalIPucNhidInfo_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalIPucNhidInfo */
typedef struct HalIPucNhidInfo {
	long	 nhid;
	long	 l3ifid;
	long	 rtFlag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalIPucNhidInfo_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalIPucNhidInfo;

#ifdef __cplusplus
}
#endif

#endif	/* _HalIPucNhidInfo_H_ */
