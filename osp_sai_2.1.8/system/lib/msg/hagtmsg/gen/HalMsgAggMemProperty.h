/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-LACP"
 * 	found in "../l2/hal_msg_agg.asn1"
 */

#ifndef	_HalMsgAggMemProperty_H_
#define	_HalMsgAggMemProperty_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgAggMemProperty */
typedef struct HalMsgAggMemProperty {
	long	 portId;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgAggMemProperty_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgAggMemProperty;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgAggMemProperty_H_ */