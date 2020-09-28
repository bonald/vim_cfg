/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_DSSrcVlan_H_
#define	_DSSrcVlan_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DSSrcVlan */
typedef struct DSSrcVlan {
	long	 v6UcastSaType;
	long	 l3SpanId;
	long	 l3AclRoutedOnly;
	long	 l3AclEn;
	long	 l3QosLookupEn;
	long	 l3IfType;
	long	 v4UcastEn;
	long	 v4McastEn;
	long	 v6UcastEn;
	long	 v6McastEn;
	long	 v4UcastSaType;
	long	 v4McastRpfEn;
	long	 v6McastRpfEn;
	long	 mplsEn;
	long	 igmpSnoopEn;
	long	 routeAllPackets;
	long	 routerMacType;
	long	 stpId;
	long	 routerMacLabel;
	long	 l3SpanEn;
	long	 receieveEn;
	long	 fwdStatsBucket;
	long	 vrfidORfid;
	long	 l3AclLabel;
	long	 l3QosLabel;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DSSrcVlan_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DSSrcVlan;

#ifdef __cplusplus
}
#endif

#endif	/* _DSSrcVlan_H_ */