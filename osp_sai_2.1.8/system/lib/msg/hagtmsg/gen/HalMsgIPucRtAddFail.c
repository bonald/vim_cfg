/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#include <asn_internal.h>

#include "HalMsgIPucRtAddFail.h"

static asn_TYPE_member_t asn_MBR_HalMsgIPucRtAddFail_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, vrfid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vrfid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, masklen),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"masklen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, ipVer),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipVer"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, ipAddr),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, nhid),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIPucRtAddFail, isEcmp),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isEcmp"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgIPucRtAddFail_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgIPucRtAddFail_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* vrfid at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* masklen at 88 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ipVer at 89 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ipAddr at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* nhid at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* isEcmp at 93 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgIPucRtAddFail_specs_1 = {
	sizeof(struct HalMsgIPucRtAddFail),
	offsetof(struct HalMsgIPucRtAddFail, _asn_ctx),
	asn_MAP_HalMsgIPucRtAddFail_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgIPucRtAddFail = {
	"HalMsgIPucRtAddFail",
	"HalMsgIPucRtAddFail",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgIPucRtAddFail_tags_1,
	sizeof(asn_DEF_HalMsgIPucRtAddFail_tags_1)
		/sizeof(asn_DEF_HalMsgIPucRtAddFail_tags_1[0]), /* 1 */
	asn_DEF_HalMsgIPucRtAddFail_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgIPucRtAddFail_tags_1)
		/sizeof(asn_DEF_HalMsgIPucRtAddFail_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgIPucRtAddFail_1,
	6,	/* Elements count */
	&asn_SPC_HalMsgIPucRtAddFail_specs_1	/* Additional specs */
};

