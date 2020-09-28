/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-RATELIMIT"
 * 	found in "../l2/hal_msg_ratelimit.asn1"
 */

#ifndef	_HalMsgSyncRatelimitItem_H_
#define	_HalMsgSyncRatelimitItem_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgSyncRatelimitItem */
typedef struct HalMsgSyncRatelimitItem {
	long	 port;
	long	 enable;
	long	 unit;
	long	 thresh;
	long	 op;
	long	 vlanId;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgSyncRatelimitItem_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgSyncRatelimitItem;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgSyncRatelimitItem_H_ */