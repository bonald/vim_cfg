/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "FanValue.h"

static asn_TYPE_member_t asn_MBR_FanValue_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, index),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"index"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, num),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"num"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, speedAdjust),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"speedAdjust"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, present),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"present"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, status),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"status"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, speedLevel),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"speedLevel"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FanValue, speedRate),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"speedRate"
		},
};
static ber_tlv_tag_t asn_DEF_FanValue_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_FanValue_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* index at 781 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* num at 782 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* speedAdjust at 783 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* present at 784 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* status at 785 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* speedLevel at 786 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 } /* speedRate at 788 */
};
static asn_SEQUENCE_specifics_t asn_SPC_FanValue_specs_1 = {
	sizeof(struct FanValue),
	offsetof(struct FanValue, _asn_ctx),
	asn_MAP_FanValue_tag2el_1,
	7,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_FanValue = {
	"FanValue",
	"FanValue",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_FanValue_tags_1,
	sizeof(asn_DEF_FanValue_tags_1)
		/sizeof(asn_DEF_FanValue_tags_1[0]), /* 1 */
	asn_DEF_FanValue_tags_1,	/* Same as above */
	sizeof(asn_DEF_FanValue_tags_1)
		/sizeof(asn_DEF_FanValue_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_FanValue_1,
	7,	/* Elements count */
	&asn_SPC_FanValue_specs_1	/* Additional specs */
};
