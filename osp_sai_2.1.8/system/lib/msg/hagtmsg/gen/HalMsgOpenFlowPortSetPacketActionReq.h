/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-PORT"
 * 	found in "../openflow/hal_msg_openflow_port.asn1"
 */

#ifndef	_HalMsgOpenFlowPortSetPacketActionReq_H_
#define	_HalMsgOpenFlowPortSetPacketActionReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgOpenFlowPortSetPacketActionReq */
typedef struct HalMsgOpenFlowPortSetPacketActionReq {
	long	 gport;
	long	 stpAction;
	long	 lacpAction;
	long	 lldpAction;
	long	 igmpAction;
	long	 erpsAction;
	long	 dot1xAction;
	long	 arpAction;
	long	 dhcpAction;
	long	 ospfAction;
	long	 pimAction;
	long	 vrrpAction;
	long	 bgpAction;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgOpenFlowPortSetPacketActionReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowPortSetPacketActionReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgOpenFlowPortSetPacketActionReq_H_ */