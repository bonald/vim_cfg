/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmStackingTxTrunkMsg_H_
#define	_ChsmStackingTxTrunkMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmStackingTxTrunkMsg */
typedef struct ChsmStackingTxTrunkMsg {
	long	 member;
	long	 trunk;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmStackingTxTrunkMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmStackingTxTrunkMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmStackingTxTrunkMsg_H_ */