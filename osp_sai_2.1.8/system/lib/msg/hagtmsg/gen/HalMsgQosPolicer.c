/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#include <asn_internal.h>

#include "HalMsgQosPolicer.h"

static asn_TYPE_member_t asn_MBR_HalMsgQosPolicer_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, cir),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cir"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, cbs),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cbs"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, pir),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pir"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, pbs),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pbs"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, useL3Length),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"useL3Length"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, isSrTcm),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isSrTcm"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, isColorBlind),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isColorBlind"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, dropColor),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dropColor"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, mode),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mode"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosPolicer, isStatsEn),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isStatsEn"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgQosPolicer_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgQosPolicer_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* cir at 36 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* cbs at 37 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* pir at 38 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* pbs at 39 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* useL3Length at 40 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* isSrTcm at 41 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* isColorBlind at 42 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* dropColor at 43 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* mode at 44 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* isStatsEn at 46 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgQosPolicer_specs_1 = {
	sizeof(struct HalMsgQosPolicer),
	offsetof(struct HalMsgQosPolicer, _asn_ctx),
	asn_MAP_HalMsgQosPolicer_tag2el_1,
	10,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgQosPolicer = {
	"HalMsgQosPolicer",
	"HalMsgQosPolicer",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgQosPolicer_tags_1,
	sizeof(asn_DEF_HalMsgQosPolicer_tags_1)
		/sizeof(asn_DEF_HalMsgQosPolicer_tags_1[0]), /* 1 */
	asn_DEF_HalMsgQosPolicer_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgQosPolicer_tags_1)
		/sizeof(asn_DEF_HalMsgQosPolicer_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgQosPolicer_1,
	10,	/* Elements count */
	&asn_SPC_HalMsgQosPolicer_specs_1	/* Additional specs */
};

