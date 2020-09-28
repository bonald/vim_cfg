/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAGT2SRV-MSG"
 * 	found in "../hagt2SrvMsg.asn1"
 */

#include <asn_internal.h>

#include "Hagt2SrvMsgAgentDbSyncReq.h"

static asn_TYPE_member_t asn_MBR_Hagt2SrvMsgAgentDbSyncReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Hagt2SrvMsgAgentDbSyncReq, physlot),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"physlot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Hagt2SrvMsgAgentDbSyncReq, logicslot),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logicslot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Hagt2SrvMsgAgentDbSyncReq, type),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"type"
		},
};
static ber_tlv_tag_t asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Hagt2SrvMsgAgentDbSyncReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* physlot at 558 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* logicslot at 559 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* type at 561 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Hagt2SrvMsgAgentDbSyncReq_specs_1 = {
	sizeof(struct Hagt2SrvMsgAgentDbSyncReq),
	offsetof(struct Hagt2SrvMsgAgentDbSyncReq, _asn_ctx),
	asn_MAP_Hagt2SrvMsgAgentDbSyncReq_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Hagt2SrvMsgAgentDbSyncReq = {
	"Hagt2SrvMsgAgentDbSyncReq",
	"Hagt2SrvMsgAgentDbSyncReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1,
	sizeof(asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1)
		/sizeof(asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1[0]), /* 1 */
	asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1)
		/sizeof(asn_DEF_Hagt2SrvMsgAgentDbSyncReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Hagt2SrvMsgAgentDbSyncReq_1,
	3,	/* Elements count */
	&asn_SPC_Hagt2SrvMsgAgentDbSyncReq_specs_1	/* Additional specs */
};
