/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN"
 * 	found in "../l2/hal_msg_vlan.asn1"
 */

#ifndef	_HalMsgL2VlanSetDhcpExcpTypeReq_H_
#define	_HalMsgL2VlanSetDhcpExcpTypeReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2VlanSetDhcpExcpTypeReq */
typedef struct HalMsgL2VlanSetDhcpExcpTypeReq {
	long	 vid;
	long	 type;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanSetDhcpExcpTypeReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanSetDhcpExcpTypeReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VlanSetDhcpExcpTypeReq_H_ */
