/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#ifndef	_HalMsgMplsVSIDelReq_H_
#define	_HalMsgMplsVSIDelReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMplsVSIDelReq */
typedef struct HalMsgMplsVSIDelReq {
	long	 vplsid;
	long	 fid;
	long	 groupid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMplsVSIDelReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMplsVSIDelReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMplsVSIDelReq_H_ */