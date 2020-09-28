/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-TPOAM"
 * 	found in "../mpls/hal_msg_tpoam.asn1"
 */

#ifndef	_HalMsgTpoamLMepKey_H_
#define	_HalMsgTpoamLMepKey_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgTpoamLMepKey */
typedef struct HalMsgTpoamLMepKey {
	long	 oamType;
	long	 id;
	OCTET_STRING_t	 megid;
	long	 mepid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgTpoamLMepKey_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgTpoamLMepKey;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgTpoamLMepKey_H_ */