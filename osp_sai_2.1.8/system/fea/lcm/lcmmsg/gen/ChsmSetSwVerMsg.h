/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#ifndef	_ChsmSetSwVerMsg_H_
#define	_ChsmSetSwVerMsg_H_


#include <asn_application.h>

/* Including external dependencies */
#include <PrintableString.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ChsmSetSwVerMsg */
typedef struct ChsmSetSwVerMsg {
	PrintableString_t	 swVer;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ChsmSetSwVerMsg_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ChsmSetSwVerMsg;

#ifdef __cplusplus
}
#endif

#endif	/* _ChsmSetSwVerMsg_H_ */