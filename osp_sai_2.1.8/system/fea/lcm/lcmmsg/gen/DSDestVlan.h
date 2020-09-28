/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_DSDestVlan_H_
#define	_DSDestVlan_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DSDestVlan */
typedef struct DSDestVlan {
	long	 mtuExceptionen;
	long	 transmitEn;
	long	 v4UcastEn;
	long	 v4McastEn;
	long	 v6UcastEn;
	long	 v6McastEn;
	long	 mtuCheckEn;
	long	 replaceDscp;
	long	 mtuSize;
	long	 mcastTtlThreshold;
	long	 macsaType;
	long	 stpid;
	long	 l3AclRoutedOnly;
	long	 l3AclEn;
	long	 l3QosLookupEn;
	long	 l3SpanEn;
	long	 l3SpanId;
	long	 fwdStatBucket;
	long	 macsa;
	long	 l3AclLabel;
	long	 l3QosLabel;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} DSDestVlan_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_DSDestVlan;

#ifdef __cplusplus
}
#endif

#endif	/* _DSDestVlan_H_ */
