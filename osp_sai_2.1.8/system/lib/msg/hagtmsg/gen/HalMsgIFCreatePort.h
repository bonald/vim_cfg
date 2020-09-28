/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIFCreatePort_H_
#define	_HalMsgIFCreatePort_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIFCreatePort */
typedef struct HalMsgIFCreatePort {
	long	 gportid;
	long	 framesize;
	long	 slotno;
	long	 portno;
	long	 subportno;
	long	 enable;
	HalMsgDefL2MacAddr_t	 portmac;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIFCreatePort_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIFCreatePort;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIFCreatePort_H_ */