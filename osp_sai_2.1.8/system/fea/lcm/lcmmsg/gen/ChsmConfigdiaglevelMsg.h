/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmConfigdiaglevelMsg_H_
#define	_ChsmConfigdiaglevelMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmConfigdiaglevelMsg */
typedef struct ChsmConfigdiaglevelMsg {
	long	 level;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmConfigdiaglevelMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmConfigdiaglevelMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmConfigdiaglevelMsg_H_ */
