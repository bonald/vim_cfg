/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_L3EditMpls4w_H_
#define	_L3EditMpls4w_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* L3EditMpls4w */
typedef struct L3EditMpls4w {
	long	 mapExp0;
	long	 exp0;
	long	 ttl0;
	long	 label0;
	long	 martiniEncapValid;
	long	 mapTtl0;
	long	 labelValid;
	long	 mapTtl1;
	long	 mcastLabel0;
	long	 mcastLabel1;
	long	 mapExp1;
	long	 exp1;
	long	 ttl1;
	long	 label1;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} L3EditMpls4w_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_L3EditMpls4w;

#ifdef __cplusplus
}
#endif

#endif	/* _L3EditMpls4w_H_ */