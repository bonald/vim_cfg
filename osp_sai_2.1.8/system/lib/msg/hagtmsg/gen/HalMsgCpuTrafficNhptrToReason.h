/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-CPU-TRAFFIC"
 * 	found in "../common/hal_msg_cpu_traffic.asn1"
 */

#ifndef	_HalMsgCpuTrafficNhptrToReason_H_
#define	_HalMsgCpuTrafficNhptrToReason_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgCpuTrafficNhptrToReason */
typedef struct HalMsgCpuTrafficNhptrToReason {
	long	 nhPtr;
	long	 reason;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgCpuTrafficNhptrToReason_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgCpuTrafficNhptrToReason;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgCpuTrafficNhptrToReason_H_ */