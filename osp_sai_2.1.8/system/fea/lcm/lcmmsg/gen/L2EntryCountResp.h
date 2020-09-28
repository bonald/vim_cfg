/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_L2EntryCountResp_H_
#define	_L2EntryCountResp_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* L2EntryCountResp */
typedef struct L2EntryCountResp {
	long	 filterCnt;
	long	 withVIDCnt;
	long	 withoutVIDCnt;
	long	 snoopingCnt;
	long	 defaultCnt;
	long	 mcastCnt;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} L2EntryCountResp_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_L2EntryCountResp;

#ifdef __cplusplus
}
#endif

#endif	/* _L2EntryCountResp_H_ */