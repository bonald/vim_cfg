/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-TUNNEL"
 * 	found in "../openflow/hal_msg_openflow_tunnel.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenDecapFlowRule.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenDecapFlowRule_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenDecapFlowRule, flags),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flags"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenDecapFlowRule, vni),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vni"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenDecapFlowRule, metadata),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"metadata"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenDecapFlowRule, ipSa),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipSa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenDecapFlowRule, ipDa),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipDa"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenDecapFlowRule_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenDecapFlowRule_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flags at 15 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* vni at 16 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* metadata at 17 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ipSa at 18 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* ipDa at 20 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenDecapFlowRule_specs_1 = {
	sizeof(struct HalMsgOpenDecapFlowRule),
	offsetof(struct HalMsgOpenDecapFlowRule, _asn_ctx),
	asn_MAP_HalMsgOpenDecapFlowRule_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenDecapFlowRule = {
	"HalMsgOpenDecapFlowRule",
	"HalMsgOpenDecapFlowRule",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenDecapFlowRule_tags_1,
	sizeof(asn_DEF_HalMsgOpenDecapFlowRule_tags_1)
		/sizeof(asn_DEF_HalMsgOpenDecapFlowRule_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenDecapFlowRule_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenDecapFlowRule_tags_1)
		/sizeof(asn_DEF_HalMsgOpenDecapFlowRule_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenDecapFlowRule_1,
	5,	/* Elements count */
	&asn_SPC_HalMsgOpenDecapFlowRule_specs_1	/* Additional specs */
};

