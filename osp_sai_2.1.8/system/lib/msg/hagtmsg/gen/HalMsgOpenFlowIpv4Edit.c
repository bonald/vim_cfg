/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-NEXTHOP"
 * 	found in "../openflow/hal_msg_openflow_nexthop.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowIpv4Edit.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowIpv4Edit_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, ipSa),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipSa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, ipDa),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIpv4addr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipDa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, l4SrcPort),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l4SrcPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, l4DestPort),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l4DestPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, dscp),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dscp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, ecn),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ecn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, ttl),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ttl"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, protocol),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"protocol"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, icmpType),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"icmpType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowIpv4Edit, icmpCode),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"icmpCode"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowIpv4Edit_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* ipSa at 43 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ipDa at 44 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* l4SrcPort at 45 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* l4DestPort at 46 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* dscp at 47 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* ecn at 48 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* ttl at 49 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* protocol at 50 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* icmpType at 51 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* icmpCode at 53 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowIpv4Edit_specs_1 = {
	sizeof(struct HalMsgOpenFlowIpv4Edit),
	offsetof(struct HalMsgOpenFlowIpv4Edit, _asn_ctx),
	asn_MAP_HalMsgOpenFlowIpv4Edit_tag2el_1,
	10,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowIpv4Edit = {
	"HalMsgOpenFlowIpv4Edit",
	"HalMsgOpenFlowIpv4Edit",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowIpv4Edit_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowIpv4Edit_1,
	10,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowIpv4Edit_specs_1	/* Additional specs */
};

