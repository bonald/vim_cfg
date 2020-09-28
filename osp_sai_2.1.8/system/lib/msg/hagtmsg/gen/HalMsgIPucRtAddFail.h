/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#ifndef	_HalMsgIPucRtAddFail_H_
#define	_HalMsgIPucRtAddFail_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIPucRtAddFail */
typedef struct HalMsgIPucRtAddFail {
	long	 vrfid;
	long	 masklen;
	long	 ipVer;
	PrintableString_t	 ipAddr;
	long	 nhid;
	long	 isEcmp;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIPucRtAddFail_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIPucRtAddFail;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIPucRtAddFail_H_ */
