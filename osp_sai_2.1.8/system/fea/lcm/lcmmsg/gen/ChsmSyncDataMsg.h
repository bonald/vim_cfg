/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmSyncDataMsg_H_
#define	_ChsmSyncDataMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <ChsmSyncDataType.h>
#include <ChsmSyncDataState.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmSyncDataMsg */
typedef struct ChsmSyncDataMsg {
	ChsmSyncDataType_t	 syncType;
	ChsmSyncDataState_t	 syncState;
	OCTET_STRING_t	 data;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmSyncDataMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmSyncDataMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmSyncDataMsg_H_ */
