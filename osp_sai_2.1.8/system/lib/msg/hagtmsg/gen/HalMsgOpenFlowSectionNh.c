/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-NEXTHOP"
 * 	found in "../openflow/hal_msg_openflow_nexthop.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowSectionNh.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowSectionNh_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowSectionNh, nhid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowSectionNh, offset),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowSectionNh, tunnelId),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunnelId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowSectionNh, nhInfo),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgOpenFlowIpucNh,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhInfo"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowSectionNh_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowSectionNh_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nhid at 346 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* offset at 347 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* tunnelId at 348 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* nhInfo at 350 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowSectionNh_specs_1 = {
	sizeof(struct HalMsgOpenFlowSectionNh),
	offsetof(struct HalMsgOpenFlowSectionNh, _asn_ctx),
	asn_MAP_HalMsgOpenFlowSectionNh_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowSectionNh = {
	"HalMsgOpenFlowSectionNh",
	"HalMsgOpenFlowSectionNh",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowSectionNh_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowSectionNh_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowSectionNh_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowSectionNh_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowSectionNh_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowSectionNh_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowSectionNh_1,
	4,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowSectionNh_specs_1	/* Additional specs */
};

