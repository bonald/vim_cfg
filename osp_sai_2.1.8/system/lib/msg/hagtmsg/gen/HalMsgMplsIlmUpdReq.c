/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#include <asn_internal.h>

#include "HalMsgMplsIlmUpdReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgMplsIlmUpdReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, label),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"label"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, nexthopid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nexthopid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, vrf),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vrf"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, type),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, spaceid),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"spaceid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, model),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"model"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, cwen),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cwen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, pop),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pop"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, fid),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"fid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, vplsPort),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vplsPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, vplsPortType),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vplsPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsIlmUpdReq, trustExp),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"trustExp"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgMplsIlmUpdReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgMplsIlmUpdReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* label at 231 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* nexthopid at 232 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* vrf at 233 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* type at 234 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* spaceid at 235 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* model at 236 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* cwen at 237 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* pop at 238 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* fid at 239 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* vplsPort at 240 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* vplsPortType at 241 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 } /* trustExp at 243 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgMplsIlmUpdReq_specs_1 = {
	sizeof(struct HalMsgMplsIlmUpdReq),
	offsetof(struct HalMsgMplsIlmUpdReq, _asn_ctx),
	asn_MAP_HalMsgMplsIlmUpdReq_tag2el_1,
	12,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgMplsIlmUpdReq = {
	"HalMsgMplsIlmUpdReq",
	"HalMsgMplsIlmUpdReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgMplsIlmUpdReq_tags_1,
	sizeof(asn_DEF_HalMsgMplsIlmUpdReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsIlmUpdReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgMplsIlmUpdReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgMplsIlmUpdReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsIlmUpdReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgMplsIlmUpdReq_1,
	12,	/* Elements count */
	&asn_SPC_HalMsgMplsIlmUpdReq_specs_1	/* Additional specs */
};
