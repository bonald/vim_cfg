/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ETH-OAM"
 * 	found in "../l2/hal_msg_eth_oam.asn1"
 */

#ifndef	_HalMsgEthOamCfmPort_H_
#define	_HalMsgEthOamCfmPort_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgEthOamCfmPort */
typedef struct HalMsgEthOamCfmPort {
	long	 portId;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgEthOamCfmPort_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgEthOamCfmPort;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgEthOamCfmPort_H_ */
