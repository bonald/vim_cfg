/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN-MAPPING"
 * 	found in "../l2/hal_msg_vlan_mapping.asn1"
 */

#ifndef	_HalMsgL2VlanMappingL2DefEntryListReq_H_
#define	_HalMsgL2VlanMappingL2DefEntryListReq_H_


#include <asn_application.h>

/* Including external dependencies */
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct HalMsgL2VlanMappingL2DefEntryReq;

/* HalMsgL2VlanMappingL2DefEntryListReq */
typedef struct HalMsgL2VlanMappingL2DefEntryListReq {
	struct vlanMappingL2DefEntrylistEntry {
		A_SEQUENCE_OF(struct HalMsgL2VlanMappingL2DefEntryReq) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} vlanMappingL2DefEntrylistEntry;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} HalMsgL2VlanMappingL2DefEntryListReq_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanMappingL2DefEntryListReq;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include <HalMsgL2VlanMappingL2DefEntryReq.h>

#endif	/* _HalMsgL2VlanMappingL2DefEntryListReq_H_ */
