/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIFSetLinkStatus_H_
#define	_HalMsgIFSetLinkStatus_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgFlowCtrl.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIFSetLinkStatus */
typedef struct HalMsgIFSetLinkStatus {
	long	 slotno;
	long	 portno;
	long	 subportno;
	long	 duplex;
	long	 speed;
	long	 linkStatus;
	long	 media;
	HalMsgFlowCtrl_t	 flowCtrl;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIFSetLinkStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIFSetLinkStatus;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIFSetLinkStatus_H_ */
