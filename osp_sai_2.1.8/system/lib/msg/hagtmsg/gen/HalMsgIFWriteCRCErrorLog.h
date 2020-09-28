/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIFWriteCRCErrorLog_H_
#define	_HalMsgIFWriteCRCErrorLog_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIFWriteCRCErrorLog */
typedef struct HalMsgIFWriteCRCErrorLog {
	long	 gportid;
	OCTET_STRING_t	 newCRCno;
	OCTET_STRING_t	 totalCRCno;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIFWriteCRCErrorLog_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIFWriteCRCErrorLog;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIFWriteCRCErrorLog_H_ */
