/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmOemInfoGetMsg_H_
#define	_ChsmOemInfoGetMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmOemInfoGetMsg */
typedef struct ChsmOemInfoGetMsg {
	PrintableString_t	 oemInfo;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmOemInfoGetMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmOemInfoGetMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmOemInfoGetMsg_H_ */
