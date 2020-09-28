/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_VcmChannelCfg_H_
#define	_VcmChannelCfg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NativeReal.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VcmChannelCfg */
typedef struct VcmChannelCfg {
	long	 flag;
	double	 currDependPin;
	double	 voltSplitFactor;
	double	 low;
	double	 high;
	PrintableString_t	 name;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VcmChannelCfg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VcmChannelCfg;

#ifdef __cplusplus
}
#endif

#endif	/* _VcmChannelCfg_H_ */
