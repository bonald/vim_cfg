/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#ifndef	_HalMsgMplsNhToSpecReq_H_
#define	_HalMsgMplsNhToSpecReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMplsNhToSpecReq */
typedef struct HalMsgMplsNhToSpecReq {
	long	 nexthopid;
	long	 maxChipNum;
	long	 fid;
	long	 port;
	long	 alreadyChipBMP;
	long	 vplsPort;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMplsNhToSpecReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMplsNhToSpecReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMplsNhToSpecReq_H_ */