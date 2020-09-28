/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowIpv4Rule.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowIpv4Rule_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, flag),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipSa),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipSa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipSaMask),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipSaMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipDa),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipDa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipDaMask),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipDaMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, l4SrcPort),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgOpenFlowL4PortType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l4SrcPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, l4DestPort),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgOpenFlowL4PortType,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l4DestPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, dscp),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dscp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, dscpMask),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dscpMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, l4Protocol),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l4Protocol"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, icmpType),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"icmpType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, icmpCode),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"icmpCode"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, igmpType),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"igmpType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, tcpFlags),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tcpFlags"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipFrag),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipFrag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ipFragMask),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipFragMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ecn),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ecn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Rule, ecnMask),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ecnMask"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowIpv4Rule_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flag at 70 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ipSa at 71 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ipSaMask at 72 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ipDa at 73 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ipDaMask at 74 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* l4SrcPort at 75 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* l4DestPort at 76 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* dscp at 77 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* dscpMask at 78 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* l4Protocol at 79 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* icmpType at 80 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* icmpCode at 81 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* igmpType at 82 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* tcpFlags at 83 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* ipFrag at 84 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* ipFragMask at 85 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* ecn at 86 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 } /* ecnMask at 88 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowIpv4Rule_specs_1 = {
	sizeof(struct HalMsgOpenFlowIpv4Rule),
	offsetof(struct HalMsgOpenFlowIpv4Rule, _asn_ctx),
	asn_MAP_HalMsgOpenFlowIpv4Rule_tag2el_1,
	18,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowIpv4Rule = {
	"HalMsgOpenFlowIpv4Rule",
	"HalMsgOpenFlowIpv4Rule",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowIpv4Rule_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowIpv4Rule_1,
	18,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowIpv4Rule_specs_1	/* Additional specs */
};

