/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#ifndef	_VlanClassTcamEntryMsg_H_
#define	_VlanClassTcamEntryMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VlanClassTcamEntryMsg */
typedef struct VlanClassTcamEntryMsg {
	long	 chipid;
	long	 keytype;
	long	 index;
	long	 bitmapUpto;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VlanClassTcamEntryMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VlanClassTcamEntryMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _VlanClassTcamEntryMsg_H_ */
