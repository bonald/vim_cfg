/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MIRROR"
 * 	found in "../common/hal_msg_mirror.asn1"
 */

#ifndef	_HalMsgMirrorCfgDiscardEnReq_H_
#define	_HalMsgMirrorCfgDiscardEnReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMirrorCfgDiscardEnReq */
typedef struct HalMsgMirrorCfgDiscardEnReq {
	long	 dir;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMirrorCfgDiscardEnReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMirrorCfgDiscardEnReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMirrorCfgDiscardEnReq_H_ */
