/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "VcmChannelCfg.h"

static asn_TYPE_member_t asn_MBR_VcmChannelCfg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, flag),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, currDependPin),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"currDependPin"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, voltSplitFactor),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"voltSplitFactor"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, low),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"low"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, high),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeReal,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"high"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VcmChannelCfg, name),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"name"
		},
};
static ber_tlv_tag_t asn_DEF_VcmChannelCfg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_VcmChannelCfg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flag at 734 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* currDependPin at 735 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* voltSplitFactor at 736 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* low at 737 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* high at 738 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* name at 739 */
};
static asn_SEQUENCE_specifics_t asn_SPC_VcmChannelCfg_specs_1 = {
	sizeof(struct VcmChannelCfg),
	offsetof(struct VcmChannelCfg, _asn_ctx),
	asn_MAP_VcmChannelCfg_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_VcmChannelCfg = {
	"VcmChannelCfg",
	"VcmChannelCfg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_VcmChannelCfg_tags_1,
	sizeof(asn_DEF_VcmChannelCfg_tags_1)
		/sizeof(asn_DEF_VcmChannelCfg_tags_1[0]), /* 1 */
	asn_DEF_VcmChannelCfg_tags_1,	/* Same as above */
	sizeof(asn_DEF_VcmChannelCfg_tags_1)
		/sizeof(asn_DEF_VcmChannelCfg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_VcmChannelCfg_1,
	6,	/* Elements count */
	&asn_SPC_VcmChannelCfg_specs_1	/* Additional specs */
};

