/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmInitReadyMsg_H_
#define	_ChsmInitReadyMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SlotNo.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmInitReadyMsg */
typedef struct ChsmInitReadyMsg {
	SlotNo_t	 logicSlot;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmInitReadyMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmInitReadyMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmInitReadyMsg_H_ */