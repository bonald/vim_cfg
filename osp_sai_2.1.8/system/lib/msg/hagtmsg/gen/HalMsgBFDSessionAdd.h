/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-BFD"
 * 	found in "../common/hal_msg_bfd.asn1"
 */

#ifndef	_HalMsgBFDSessionAdd_H_
#define	_HalMsgBFDSessionAdd_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgBFDSessionAdd */
typedef struct HalMsgBFDSessionAdd {
	long	 gport;
	long	 srcudpport;
	long	 dstudpport;
	long	 localdisc;
	long	 remotedisc;
	long	 remotediscold;
	long	 state;
	long	 diag;
	long	 txinterval;
	long	 rxinterval;
	long	 realtx;
	long	 realrx;
	long	 txmult;
	long	 rxmult;
	long	 nhid;
	long	 nhoffset;
	HalMsgDefL2MacAddr_t	 mac;
	long	 vid;
	long	 ipver;
	PrintableString_t	 src;
	PrintableString_t	 dst;
	long	 ttl;
	long	 dscp;
	long	 updateflag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBFDSessionAdd_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBFDSessionAdd;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgBFDSessionAdd_H_ */