/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_TmprLimit_H_
#define	_TmprLimit_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TmprLimit */
typedef struct TmprLimit {
	long	 boardLow;
	long	 boardHigh;
	long	 boardCrit;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TmprLimit_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TmprLimit;

#ifdef __cplusplus
}
#endif

#endif	/* _TmprLimit_H_ */
