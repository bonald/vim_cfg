/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-FLOW"
 * 	found in "../aclqos/hal_msg_flow.asn1"
 */

#include <asn_internal.h>

#include "HalMsgFlowMacRule.h"

static asn_TYPE_member_t asn_MBR_HalMsgFlowMacRule_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, flag),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, senderIp),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"senderIp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, senderIpMask),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"senderIpMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, targetIp),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"targetIp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, targetIpMask),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"targetIpMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, macSa),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macSa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, macSaMask),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macSaMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, macDa),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macDa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, macDaMask),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macDaMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, cVlanId),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cVlanId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, sVlanId),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sVlanId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, cTagCos),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cTagCos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, sTagCos),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sTagCos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, l3Type),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l3Type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, l2Type),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2Type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, ethType),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ethType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, ethTypeMask),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ethTypeMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowMacRule, arpOpCode),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"arpOpCode"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgFlowMacRule_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgFlowMacRule_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flag at 29 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* senderIp at 30 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* senderIpMask at 31 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* targetIp at 32 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* targetIpMask at 33 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* macSa at 34 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* macSaMask at 35 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* macDa at 36 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* macDaMask at 37 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* cVlanId at 38 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* sVlanId at 39 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* cTagCos at 40 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* sTagCos at 41 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* l3Type at 42 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* l2Type at 43 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* ethType at 44 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* ethTypeMask at 45 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 } /* arpOpCode at 47 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgFlowMacRule_specs_1 = {
	sizeof(struct HalMsgFlowMacRule),
	offsetof(struct HalMsgFlowMacRule, _asn_ctx),
	asn_MAP_HalMsgFlowMacRule_tag2el_1,
	18,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgFlowMacRule = {
	"HalMsgFlowMacRule",
	"HalMsgFlowMacRule",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgFlowMacRule_tags_1,
	sizeof(asn_DEF_HalMsgFlowMacRule_tags_1)
		/sizeof(asn_DEF_HalMsgFlowMacRule_tags_1[0]), /* 1 */
	asn_DEF_HalMsgFlowMacRule_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgFlowMacRule_tags_1)
		/sizeof(asn_DEF_HalMsgFlowMacRule_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgFlowMacRule_1,
	18,	/* Elements count */
	&asn_SPC_HalMsgFlowMacRule_specs_1	/* Additional specs */
};

