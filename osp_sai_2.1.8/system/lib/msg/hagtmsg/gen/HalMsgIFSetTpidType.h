/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIFSetTpidType_H_
#define	_HalMsgIFSetTpidType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgIntList.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIFSetTpidType */
typedef struct HalMsgIFSetTpidType {
	HalMsgIntList_t	 port;
	long	 type;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIFSetTpidType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIFSetTpidType;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIFSetTpidType_H_ */
