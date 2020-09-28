/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-DOT1X"
 * 	found in "../l2/hal_msg_dot1x.asn1"
 */

#ifndef	_HalMsgDot1xAuth_H_
#define	_HalMsgDot1xAuth_H_


#include <asn_application.h>

/* Including external dependencies */
#include <Dot1xAuthType.h>
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgDot1xAuth */
typedef struct HalMsgDot1xAuth {
	Dot1xAuthType_t	 type;
	long	 portid;
	HalMsgDefL2MacAddr_t	 mac;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgDot1xAuth_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgDot1xAuth;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgDot1xAuth_H_ */