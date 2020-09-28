/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN-MAPPING"
 * 	found in "../l2/hal_msg_vlan_mapping.asn1"
 */

#ifndef	_HalMsgL2VlanMappingDefVidListReq_H_
#define	_HalMsgL2VlanMappingDefVidListReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct HalMsgL2VlanMappingDefVidReq;

/* HalMsgL2VlanMappingDefVidListReq */
typedef struct HalMsgL2VlanMappingDefVidListReq {
	struct vlanMappingDefVidlistEntry {
		A_SEQUENCE_OF(struct HalMsgL2VlanMappingDefVidReq) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} vlanMappingDefVidlistEntry;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanMappingDefVidListReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanMappingDefVidListReq;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <HalMsgL2VlanMappingDefVidReq.h>

#endif	/* _HalMsgL2VlanMappingDefVidListReq_H_ */
