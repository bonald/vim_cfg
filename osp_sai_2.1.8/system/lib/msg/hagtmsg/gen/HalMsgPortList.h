/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-TYPES"
 * 	found in "../common/hal_msg_types.asn1"
 */

#ifndef	_HalMsgPortList_H_
#define	_HalMsgPortList_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgPortList */
typedef struct HalMsgPortList {
	long	 portNum;
	struct portId {
		A_SEQUENCE_OF(long) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} portId;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgPortList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgPortList;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgPortList_H_ */