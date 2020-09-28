/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_LcChsmPortGetInfoAck_H_
#define	_LcChsmPortGetInfoAck_H_


#include <asn_application.h>

/* Including external dependencies */
#include <SlotNo.h>
#include <PortNo.h>
#include <BOOLEAN.h>
#include <DuplexMode.h>
#include <SpeedMode.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LcChsmPortGetInfoAck */
typedef struct LcChsmPortGetInfoAck {
	SlotNo_t	 slot;
	PortNo_t	 port;
	BOOLEAN_t	 linkup;
	DuplexMode_t	 duplex;
	SpeedMode_t	 speed;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LcChsmPortGetInfoAck_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LcChsmPortGetInfoAck;

#ifdef __cplusplus
}
#endif

#endif	/* _LcChsmPortGetInfoAck_H_ */