/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-LACP"
 * 	found in "../l2/hal_msg_agg.asn1"
 */

#ifndef	_HalMsgMemportPropSyncReq_H_
#define	_HalMsgMemportPropSyncReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMemportPropSyncReq */
typedef struct HalMsgMemportPropSyncReq {
	long	 portid;
	long	 routeEn;
	long	 defEntrySync;
	long	 secbindSync;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMemportPropSyncReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMemportPropSyncReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMemportPropSyncReq_H_ */