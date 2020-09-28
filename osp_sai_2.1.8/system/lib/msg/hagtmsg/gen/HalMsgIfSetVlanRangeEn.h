/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIfSetVlanRangeEn_H_
#define	_HalMsgIfSetVlanRangeEn_H_


#include <asn_application.h>

/* Including external dependencies */
#include <HalMsgIntList.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIfSetVlanRangeEn */
typedef struct HalMsgIfSetVlanRangeEn {
	HalMsgIntList_t	 gportid;
	long	 group;
	long	 enable;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIfSetVlanRangeEn_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIfSetVlanRangeEn;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIfSetVlanRangeEn_H_ */
