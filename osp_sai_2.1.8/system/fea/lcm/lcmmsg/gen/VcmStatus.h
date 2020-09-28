/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_VcmStatus_H_
#define	_VcmStatus_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <NativeReal.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct VcmMonitor;

/* VcmStatus */
typedef struct VcmStatus {
	long	 channelNum;
	double	 inVolt;
	struct vcmMonitor {
		A_SEQUENCE_OF(struct VcmMonitor) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} vcmMonitor;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VcmStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VcmStatus;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <VcmMonitor.h>

#endif	/* _VcmStatus_H_ */