/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_LcAttachMsg_H_
#define	_LcAttachMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SlotNo.h>
#include <BoardType.h>
#include <NativeInteger.h>
#include <PrintableString.h>
#include <TmprLimit.h>
#include <SDKGlbInfo.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct SDKTableType;

/* LcAttachMsg */
typedef struct LcAttachMsg {
	SlotNo_t	 logicSlot;
	SlotNo_t	 phySlot;
	BoardType_t	 lcType;
	long	 stmMode;
	long	 stmExtFlag;
	long	 portNum;
	long	 poePortNum;
	long	 tmprNum;
	PrintableString_t	 serialNo;
	PrintableString_t	 bootromVer;
	long	 hwVer;
	long	 epldVer;
	long	 epldDate;
	long	 epldTime;
	long	 fpgaVer;
	PrintableString_t	 swVer;
	long	 tcamType;
	long	 sramType;
	long	 flashSize;
	long	 dramSize;
	long	 extCardType;
	long	 extCardVer;
	long	 datapathMode;
	TmprLimit_t	 tmprLimit;
	long	 cpuType;
	long	 platformType;
	struct sdkTable {
		A_SEQUENCE_OF(struct SDKTableType) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} sdkTable;
	SDKGlbInfo_t	 sdkGlbeInfo;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LcAttachMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LcAttachMsg;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <SDKTableType.h>

#endif	/* _LcAttachMsg_H_ */