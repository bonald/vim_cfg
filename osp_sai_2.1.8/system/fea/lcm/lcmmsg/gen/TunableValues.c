/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "TunableValues.h"

static asn_TYPE_member_t asn_MBR_TunableValues_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, tunablesupport),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunablesupport"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, tunemode),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunemode"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, firstfreq),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"firstfreq"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, lastfreq),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lastfreq"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, stepfreq),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"stepfreq"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, tunablewavelength),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"tunablewavelength"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, dithersupport),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dithersupport"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, wavechannels),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wavechannels"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct TunableValues, wavechannelidx),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wavechannelidx"
		},
};
static ber_tlv_tag_t asn_DEF_TunableValues_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_TunableValues_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* tunablesupport at 507 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* tunemode at 508 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* firstfreq at 509 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* lastfreq at 510 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* stepfreq at 511 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* tunablewavelength at 512 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* dithersupport at 513 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* wavechannels at 514 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 } /* wavechannelidx at 516 */
};
static asn_SEQUENCE_specifics_t asn_SPC_TunableValues_specs_1 = {
	sizeof(struct TunableValues),
	offsetof(struct TunableValues, _asn_ctx),
	asn_MAP_TunableValues_tag2el_1,
	9,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_TunableValues = {
	"TunableValues",
	"TunableValues",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_TunableValues_tags_1,
	sizeof(asn_DEF_TunableValues_tags_1)
		/sizeof(asn_DEF_TunableValues_tags_1[0]), /* 1 */
	asn_DEF_TunableValues_tags_1,	/* Same as above */
	sizeof(asn_DEF_TunableValues_tags_1)
		/sizeof(asn_DEF_TunableValues_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_TunableValues_1,
	9,	/* Elements count */
	&asn_SPC_TunableValues_specs_1	/* Additional specs */
};

