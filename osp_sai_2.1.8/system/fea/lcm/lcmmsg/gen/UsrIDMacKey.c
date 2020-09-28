/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "UsrIDMacKey.h"

static asn_TYPE_member_t asn_MBR_UsrIDMacKey_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, tblid0),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tblid0"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, useridLabel),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"useridLabel"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, cos),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, cfi),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cfi"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, l2type),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, macdaUpper),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macdaUpper"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, macdaLower),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macdaLower"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, l3type),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l3type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, vlanid),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, macsaUpper),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macsaUpper"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, macsaLower),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macsaLower"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, tableid1),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tableid1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UsrIDMacKey, delbit),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"delbit"
		},
};
static ber_tlv_tag_t asn_DEF_UsrIDMacKey_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_UsrIDMacKey_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* tblid0 at 1008 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* useridLabel at 1009 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* cos at 1010 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* cfi at 1011 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* l2type at 1012 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* macdaUpper at 1013 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* macdaLower at 1014 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* l3type at 1015 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* vlanid at 1016 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* macsaUpper at 1017 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* macsaLower at 1018 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* tableid1 at 1019 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 } /* delbit at 1021 */
};
static asn_SEQUENCE_specifics_t asn_SPC_UsrIDMacKey_specs_1 = {
	sizeof(struct UsrIDMacKey),
	offsetof(struct UsrIDMacKey, _asn_ctx),
	asn_MAP_UsrIDMacKey_tag2el_1,
	13,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_UsrIDMacKey = {
	"UsrIDMacKey",
	"UsrIDMacKey",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_UsrIDMacKey_tags_1,
	sizeof(asn_DEF_UsrIDMacKey_tags_1)
		/sizeof(asn_DEF_UsrIDMacKey_tags_1[0]), /* 1 */
	asn_DEF_UsrIDMacKey_tags_1,	/* Same as above */
	sizeof(asn_DEF_UsrIDMacKey_tags_1)
		/sizeof(asn_DEF_UsrIDMacKey_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_UsrIDMacKey_1,
	13,	/* Elements count */
	&asn_SPC_UsrIDMacKey_specs_1	/* Additional specs */
};

