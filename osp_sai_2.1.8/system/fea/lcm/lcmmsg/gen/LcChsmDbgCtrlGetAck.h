/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_LcChsmDbgCtrlGetAck_H_
#define	_LcChsmDbgCtrlGetAck_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SlotNo.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LcChsmDbgCtrlGetAck */
typedef struct LcChsmDbgCtrlGetAck {
	SlotNo_t	 slot;
	long	 moduleID;
	long	 enableBMP;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LcChsmDbgCtrlGetAck_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LcChsmDbgCtrlGetAck;

#ifdef __cplusplus
}
#endif

#endif	/* _LcChsmDbgCtrlGetAck_H_ */