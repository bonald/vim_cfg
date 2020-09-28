/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#include <asn_internal.h>

#include "HalMsgMplsApsNhDelReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgMplsApsNhDelReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, nexthopid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nexthopid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, offset),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"offset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, nhType),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, spec),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"spec"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, nhExt),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhExt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, gportid),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, vid),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, oifType),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"oifType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, outVlanIsSvlan),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"outVlanIsSvlan"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, macAddr),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, statsptr),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsptr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, useTtlFromPacket),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"useTtlFromPacket"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, fid),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"fid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, vplsPortType),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vplsPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, vplsPort),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vplsPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, maxChipNum),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"maxChipNum"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, alreadyChipBMP),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"alreadyChipBMP"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, labelinfo),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"labelinfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, l2vpnTagop),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2vpnTagop"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, cwen),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cwen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhDelReq, working),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"working"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgMplsApsNhDelReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgMplsApsNhDelReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* nexthopid at 101 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* offset at 102 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* nhType at 103 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* spec at 104 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* nhExt at 105 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* gportid at 107 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* vid at 108 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* oifType at 109 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* outVlanIsSvlan at 110 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* macAddr at 111 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* statsptr at 113 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* useTtlFromPacket at 114 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* fid at 115 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* vplsPortType at 116 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* vplsPort at 117 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* maxChipNum at 118 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* alreadyChipBMP at 119 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* labelinfo at 120 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* l2vpnTagop at 121 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* cwen at 122 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 } /* working at 124 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgMplsApsNhDelReq_specs_1 = {
	sizeof(struct HalMsgMplsApsNhDelReq),
	offsetof(struct HalMsgMplsApsNhDelReq, _asn_ctx),
	asn_MAP_HalMsgMplsApsNhDelReq_tag2el_1,
	21,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgMplsApsNhDelReq = {
	"HalMsgMplsApsNhDelReq",
	"HalMsgMplsApsNhDelReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgMplsApsNhDelReq_tags_1,
	sizeof(asn_DEF_HalMsgMplsApsNhDelReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsApsNhDelReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgMplsApsNhDelReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgMplsApsNhDelReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsApsNhDelReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgMplsApsNhDelReq_1,
	21,	/* Elements count */
	&asn_SPC_HalMsgMplsApsNhDelReq_specs_1	/* Additional specs */
};
