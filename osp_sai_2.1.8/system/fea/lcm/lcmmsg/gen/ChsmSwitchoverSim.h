/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmSwitchoverSim_H_
#define	_ChsmSwitchoverSim_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmSwitchoverSim */
typedef struct ChsmSwitchoverSim {
	long	 slot;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmSwitchoverSim_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmSwitchoverSim;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmSwitchoverSim_H_ */
