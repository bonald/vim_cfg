/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#ifndef	_HalMsgMplsNhToFwdReq_H_
#define	_HalMsgMplsNhToFwdReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMplsNhToFwdReq */
typedef struct HalMsgMplsNhToFwdReq {
	long	 nexthopid;
	long	 offset;
	long	 nhType;
	long	 spec;
	long	 nhExt;
	long	 gportid;
	long	 vid;
	long	 oifType;
	long	 outVlanIsSvlan;
	PrintableString_t	 macAddr;
	long	 statsptr;
	long	 useTtlFromPacket;
	long	 fid;
	long	 vplsPortType;
	long	 vplsPort;
	long	 maxChipNum;
	long	 alreadyChipBMP;
	PrintableString_t	 labelinfo;
	long	 l2vpnTagop;
	long	 cwen;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMplsNhToFwdReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMplsNhToFwdReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMplsNhToFwdReq_H_ */