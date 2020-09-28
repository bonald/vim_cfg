/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_UsrIDVlanKey_H_
#define	_UsrIDVlanKey_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UsrIDVlanKey */
typedef struct UsrIDVlanKey {
	long	 usridLable;
	long	 delbit;
	long	 tblid;
	long	 vlanid;
	long	 gsrcPort;
	long	 cos;
	long	 customerid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UsrIDVlanKey_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UsrIDVlanKey;

#ifdef __cplusplus
}
#endif

#endif	/* _UsrIDVlanKey_H_ */