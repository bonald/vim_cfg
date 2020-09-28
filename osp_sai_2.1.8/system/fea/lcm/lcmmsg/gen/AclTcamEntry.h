/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_AclTcamEntry_H_
#define	_AclTcamEntry_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <DSAclQosMacKey.h>
#include <DSAclQosIpv4Key.h>
#include <DSAclQosIpv6Key.h>
#include <constr_CHOICE.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum key_PR {
	key_PR_NOTHING,	/* No components present */
	key_PR_macKey,
	key_PR_ipv4Key,
	key_PR_ipv6Key,
} key_PR;
typedef enum mask_PR {
	mask_PR_NOTHING,	/* No components present */
	mask_PR_macMask,
	mask_PR_ipv4Mask,
	mask_PR_ipv6Mask,
} mask_PR;

/* AclTcamEntry */
typedef struct AclTcamEntry {
	long	 index;
	long	 chipid;
	long	 ifType;
	long	 type;
	struct key {
		key_PR present;
		union {
			DSAclQosMacKey_t	 macKey;
			DSAclQosIpv4Key_t	 ipv4Key;
			DSAclQosIpv6Key_t	 ipv6Key;
		};
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} key;
	struct mask {
		mask_PR present;
		union {
			DSAclQosMacKey_t	 macMask;
			DSAclQosIpv4Key_t	 ipv4Mask;
			DSAclQosIpv6Key_t	 ipv6Mask;
		};
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} mask;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AclTcamEntry_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AclTcamEntry;

#ifdef __cplusplus
}
#endif

#endif	/* _AclTcamEntry_H_ */
