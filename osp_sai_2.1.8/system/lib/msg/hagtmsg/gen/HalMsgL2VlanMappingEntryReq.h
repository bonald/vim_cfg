/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN-MAPPING"
 * 	found in "../l2/hal_msg_vlan_mapping.asn1"
 */

#ifndef	_HalMsgL2VlanMappingEntryReq_H_
#define	_HalMsgL2VlanMappingEntryReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* HalMsgL2VlanMappingEntryReq */
typedef struct HalMsgL2VlanMappingEntryReq {
	long	 add;
	long	 offset;
	long	 replace;
	long	 glPort;
	long	 rawVid;
	long	 rawVidMax;
	long	 groupId;
	long	 egress;
	long	 rawSvid;
	long	 rawCvid;
	long	 mapSvid;
	long	 mapCvid;
	long	 fid;
	long	 serviceId;
	long	 defVid;
	long	 changeDefVid;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanMappingEntryReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanMappingEntryReq;

#ifdef __cplusplus
}
#endif

#endif	/* _HalMsgL2VlanMappingEntryReq_H_ */