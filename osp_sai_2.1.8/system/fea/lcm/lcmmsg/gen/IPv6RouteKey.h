/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_IPv6RouteKey_H_
#define	_IPv6RouteKey_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IPv6RouteKey */
typedef struct IPv6RouteKey {
	long	 cos;
	long	 cfi;
	long	 delbit;
	long	 tblid0;
	long	 routedPacket;
	long	 istcp;
	long	 isudp;
	long	 isapp;
	long	 dscp;
	long	 ipv6ExtHeaders;
	long	 ipv6FlowLabel;
	long	 vid;
	long	 macdah;
	long	 macsah;
	long	 macdal;
	long	 macsal;
	long	 vrfid;
	long	 l3type;
	long	 l4InfoMapped;
	long	 l4DstPort;
	long	 l4SrcPort;
	long	 etherType;
	long	 tblid1;
	long	 tblid2;
	long	 iphdrErr;
	long	 ipOptions;
	long	 fragInfo;
	long	 layer4Type;
	long	 l2Type;
	long	 ipsa4;
	long	 ipsa2;
	long	 ipsa3;
	long	 ipsa1;
	long	 ipsa0;
	long	 ipda4;
	long	 ipda2;
	long	 ipda3;
	long	 ipda1;
	long	 ipda0;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} IPv6RouteKey_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_IPv6RouteKey;

#ifdef __cplusplus
}
#endif

#endif	/* _IPv6RouteKey_H_ */