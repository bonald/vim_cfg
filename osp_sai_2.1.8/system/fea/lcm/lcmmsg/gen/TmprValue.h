/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_TmprValue_H_
#define	_TmprValue_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TmprValue */
typedef struct TmprValue {
	long	 index;
	long	 value;
	long	 pos;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TmprValue_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TmprValue;

#ifdef __cplusplus
}
#endif

#endif	/* _TmprValue_H_ */