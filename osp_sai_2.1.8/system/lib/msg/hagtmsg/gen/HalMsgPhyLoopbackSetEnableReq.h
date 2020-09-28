/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-PHY-LPBK"
 * 	found in "../l2/hal_msg_phy_loopback.asn1"
 */

#ifndef	_HalMsgPhyLoopbackSetEnableReq_H_
#define	_HalMsgPhyLoopbackSetEnableReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPhyLoopbackSetEnableReq */
typedef struct HalMsgPhyLoopbackSetEnableReq {
	long	 srcGport;
	long	 srcSlotno;
	long	 srcPortno;
	long	 type;
	long	 interPort;
	long	 swapMacEn;
	long	 dstGport;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPhyLoopbackSetEnableReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPhyLoopbackSetEnableReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPhyLoopbackSetEnableReq_H_ */
