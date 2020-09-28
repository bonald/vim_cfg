/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#ifndef	_HalMsgMplsAddTunnelLabelReq_H_
#define	_HalMsgMplsAddTunnelLabelReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgMplsAddTunnelLabelReq */
typedef struct HalMsgMplsAddTunnelLabelReq {
	long	 update;
	long	 tunnelId;
	long	 apsGroupId;
	long	 labelNumber;
	long	 tunnelLabel1;
	long	 mapTTL1;
	long	 tunnelLabel2;
	long	 mapTTL2;
	long	 apsType;
	long	 gportid;
	PrintableString_t	 macAddr;
	long	 statsen;
	long	 statsid;
	long	 unrov;
	long	 vid;
	long	 iftype;
	long	 ptunnelId;
	long	 papsGroupId;
	long	 plabelNumber;
	long	 ptunnelLabel1;
	long	 pmapTTL1;
	long	 ptunnelLabel2;
	long	 pmapTTL2;
	long	 papsType;
	long	 pgportid;
	PrintableString_t	 pmacAddr;
	long	 pstatsen;
	long	 pstatsid;
	long	 punrov;
	long	 pvid;
	long	 piftype;
	long	 isSpme;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgMplsAddTunnelLabelReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgMplsAddTunnelLabelReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgMplsAddTunnelLabelReq_H_ */
