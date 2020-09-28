/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#ifndef	_HalMsgQosPendFlowEntry_H_
#define	_HalMsgQosPendFlowEntry_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgQosPendFlowEntry */
typedef struct HalMsgQosPendFlowEntry {
	long	 label;
	long	 ruleId;
	long	 pendType;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgQosPendFlowEntry_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgQosPendFlowEntry;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgQosPendFlowEntry_H_ */
