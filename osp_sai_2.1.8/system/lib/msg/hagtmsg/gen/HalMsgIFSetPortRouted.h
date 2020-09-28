/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#ifndef	_HalMsgIFSetPortRouted_H_
#define	_HalMsgIFSetPortRouted_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgIntList.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgIFSetPortRouted */
typedef struct HalMsgIFSetPortRouted {
	long	 routeEn;
	long	 unTagDefVid;
	long	 l3ifid;
	long	 gportid;
	HalMsgIntList_t	 memportid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgIFSetPortRouted_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgIFSetPortRouted;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgIFSetPortRouted_H_ */