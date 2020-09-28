/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_UsrIDIPv4Key_H_
#define	_UsrIDIPv4Key_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UsrIDIPv4Key */
typedef struct UsrIDIPv4Key {
	long	 etherTypeUpper;
	long	 ethertypeLower;
	long	 macdaUpper;
	long	 macdaLower;
	long	 istcp;
	long	 isudp;
	long	 isapp;
	long	 ipop;
	long	 vlanid;
	long	 macsaUpper;
	long	 macsaLower;
	long	 tblid288;
	long	 tblid144;
	long	 fraginf;
	long	 dscp;
	long	 l3type;
	long	 l4infMap;
	long	 l4dstPort;
	long	 l4srcPort;
	long	 ipda;
	long	 ipsa;
	long	 tblidPub;
	long	 cos;
	long	 cfi;
	long	 l2type;
	long	 useridLabel;
	long	 delbit;
	long	 routedpkt;
	long	 iphdrErr;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} UsrIDIPv4Key_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_UsrIDIPv4Key;

#ifdef __cplusplus
}
#endif

#endif	/* _UsrIDIPv4Key_H_ */
