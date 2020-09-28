/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-TUNNEL"
 * 	found in "../openflow/hal_msg_openflow_tunnel.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowLspApsTunnelReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowLspApsTunnelReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, update),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"update"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, tunnelId),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunnelId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, apsGroupId),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apsGroupId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, labelNumber),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"labelNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, tunnelLabel1),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunnelLabel1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, mapTTL1),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mapTTL1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, tunnelLabel2),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunnelLabel2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, mapTTL2),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mapTTL2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, apsType),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apsType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, gportid),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, macAddr),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, statsen),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, statsid),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, unrov),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"unrov"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, vid),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, iftype),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"iftype"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, ptunnelId),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ptunnelId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, papsGroupId),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"papsGroupId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, plabelNumber),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"plabelNumber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, ptunnelLabel1),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ptunnelLabel1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pmapTTL1),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pmapTTL1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, ptunnelLabel2),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ptunnelLabel2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pmapTTL2),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pmapTTL2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, papsType),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"papsType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pgportid),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pgportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pmacAddr),
		(ASN_TAG_CLASS_CONTEXT | (25 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pmacAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pstatsen),
		(ASN_TAG_CLASS_CONTEXT | (26 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pstatsen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pstatsid),
		(ASN_TAG_CLASS_CONTEXT | (27 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pstatsid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, punrov),
		(ASN_TAG_CLASS_CONTEXT | (28 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"punrov"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, pvid),
		(ASN_TAG_CLASS_CONTEXT | (29 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pvid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, piftype),
		(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"piftype"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowLspApsTunnelReq, isSpme),
		(ASN_TAG_CLASS_CONTEXT | (31 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isSpme"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowLspApsTunnelReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* update at 80 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* tunnelId at 81 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* apsGroupId at 82 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* labelNumber at 83 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* tunnelLabel1 at 84 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* mapTTL1 at 85 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* tunnelLabel2 at 86 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* mapTTL2 at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* apsType at 88 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* gportid at 89 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* macAddr at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* statsen at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* statsid at 92 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* unrov at 93 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* vid at 94 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* iftype at 95 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* ptunnelId at 97 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* papsGroupId at 98 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* plabelNumber at 99 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* ptunnelLabel1 at 100 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* pmapTTL1 at 101 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* ptunnelLabel2 at 102 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* pmapTTL2 at 103 */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 }, /* papsType at 104 */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 24, 0, 0 }, /* pgportid at 105 */
    { (ASN_TAG_CLASS_CONTEXT | (25 << 2)), 25, 0, 0 }, /* pmacAddr at 106 */
    { (ASN_TAG_CLASS_CONTEXT | (26 << 2)), 26, 0, 0 }, /* pstatsen at 107 */
    { (ASN_TAG_CLASS_CONTEXT | (27 << 2)), 27, 0, 0 }, /* pstatsid at 108 */
    { (ASN_TAG_CLASS_CONTEXT | (28 << 2)), 28, 0, 0 }, /* punrov at 109 */
    { (ASN_TAG_CLASS_CONTEXT | (29 << 2)), 29, 0, 0 }, /* pvid at 110 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 30, 0, 0 }, /* piftype at 111 */
    { (ASN_TAG_CLASS_CONTEXT | (31 << 2)), 31, 0, 0 } /* isSpme at 113 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowLspApsTunnelReq_specs_1 = {
	sizeof(struct HalMsgOpenFlowLspApsTunnelReq),
	offsetof(struct HalMsgOpenFlowLspApsTunnelReq, _asn_ctx),
	asn_MAP_HalMsgOpenFlowLspApsTunnelReq_tag2el_1,
	32,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowLspApsTunnelReq = {
	"HalMsgOpenFlowLspApsTunnelReq",
	"HalMsgOpenFlowLspApsTunnelReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowLspApsTunnelReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowLspApsTunnelReq_1,
	32,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowLspApsTunnelReq_specs_1	/* Additional specs */
};

