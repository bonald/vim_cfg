/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_LcPortInfoMsg_H_
#define	_LcPortInfoMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LcPortInfoMsg */
typedef struct LcPortInfoMsg {
	long	 portIdx;
	long	 portLogicIdx;
	long	 portGchipIdx;
	long	 portMedia;
	long	 portPhyType;
	long	 portPanelSlot;
	long	 portPanelPort;
	long	 subportPanelPort;
	long	 poesupport;
	long	 portspeedability;
	long	 portspeedreal;
	long	 lpsupport;
	long	 iscombo;
	long	 eeesupport;
	long	 unidirsupport;
	long	 mediaSwitchSupport;
	long	 speedmodecfgen;
	long	 speedmode;
	PrintableString_t	 mac;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} LcPortInfoMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_LcPortInfoMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _LcPortInfoMsg_H_ */