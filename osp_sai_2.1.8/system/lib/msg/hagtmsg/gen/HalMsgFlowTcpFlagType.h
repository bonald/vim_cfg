/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-FLOW"
 * 	found in "../aclqos/hal_msg_flow.asn1"
 */

#ifndef	_HalMsgFlowTcpFlagType_H_
#define	_HalMsgFlowTcpFlagType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgFlowTcpFlagType */
typedef struct HalMsgFlowTcpFlagType {
	long	 operator;
	long	 flag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgFlowTcpFlagType_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgFlowTcpFlagType;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgFlowTcpFlagType_H_ */
