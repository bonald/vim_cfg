/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#ifndef	_HalMsgBrgQueryHwDB_H_
#define	_HalMsgBrgQueryHwDB_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <HalMsgDefL2MacAddr.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgBrgQueryHwDB */
typedef struct HalMsgBrgQueryHwDB {
	long	 fid;
	HalMsgDefL2MacAddr_t	 mac;
	long	 glPort;
	long	 queryType;
	long	 queryFlag;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgBrgQueryHwDB_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgBrgQueryHwDB;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgBrgQueryHwDB_H_ */