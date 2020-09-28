/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-STATS"
 * 	found in "../common/hal_msg_stats.asn1"
 */

#ifndef	_HalMsgStatsPolicerRslt_H_
#define	_HalMsgStatsPolicerRslt_H_


#include <asn_application.h>

/* Including external dependencies */
#include <OCTET_STRING.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgStatsPolicerRslt */
typedef struct HalMsgStatsPolicerRslt {
	OCTET_STRING_t	 confirmPackets;
	OCTET_STRING_t	 confirmBytes;
	OCTET_STRING_t	 exceedPackets;
	OCTET_STRING_t	 exceedBytes;
	OCTET_STRING_t	 violatePackets;
	OCTET_STRING_t	 violateBytes;
	long	 resourceId;
	long	 supportFlag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgStatsPolicerRslt_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgStatsPolicerRslt;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgStatsPolicerRslt_H_ */
