/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ETH-OAM"
 * 	found in "../l2/hal_msg_eth_oam.asn1"
 */

#ifndef	_HalMsgEthOamCfmPortEnableReq_H_
#define	_HalMsgEthOamCfmPortEnableReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgIntList.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgEthOamCfmPortEnableReq */
typedef struct HalMsgEthOamCfmPortEnableReq {
	HalMsgIntList_t	 glPort;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgEthOamCfmPortEnableReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgEthOamCfmPortEnableReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgEthOamCfmPortEnableReq_H_ */
