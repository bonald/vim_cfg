/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-TYPES"
 * 	found in "../common/hal_msg_types.asn1"
 */

#ifndef	_HalMsgSyncPortPhyItem_H_
#define	_HalMsgSyncPortPhyItem_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgSyncPortPhyItem */
typedef struct HalMsgSyncPortPhyItem {
	long	 portId;
	long	 portNo;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgSyncPortPhyItem_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgSyncPortPhyItem;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgSyncPortPhyItem_H_ */