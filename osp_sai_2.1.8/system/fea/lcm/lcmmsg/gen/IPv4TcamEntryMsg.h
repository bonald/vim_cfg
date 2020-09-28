/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_IPv4TcamEntryMsg_H_
#define	_IPv4TcamEntryMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <IPv4RouteKey.h>
#include <DSIpDA.h>
#include <DSIpSA.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IPv4TcamEntryMsg */
typedef struct IPv4TcamEntryMsg {
	long	 index;
	long	 chipid;
	IPv4RouteKey_t	 keydata;
	IPv4RouteKey_t	 keymask;
	DSIpDA_t	 dsipda;
	DSIpSA_t	 dsipsa;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IPv4TcamEntryMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IPv4TcamEntryMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _IPv4TcamEntryMsg_H_ */
