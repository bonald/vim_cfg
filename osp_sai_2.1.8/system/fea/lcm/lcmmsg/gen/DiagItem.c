/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "DiagItem.h"

static asn_TYPE_member_t asn_MBR_DiagItem_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, itemId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"itemId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, level),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"level"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, attr),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"attr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, rslt),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rslt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, runTime),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"runTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DiagItem, name),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"name"
		},
};
static ber_tlv_tag_t asn_DEF_DiagItem_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DiagItem_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* itemId at 473 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* level at 474 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* attr at 475 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* rslt at 476 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* runTime at 477 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* name at 478 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DiagItem_specs_1 = {
	sizeof(struct DiagItem),
	offsetof(struct DiagItem, _asn_ctx),
	asn_MAP_DiagItem_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DiagItem = {
	"DiagItem",
	"DiagItem",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DiagItem_tags_1,
	sizeof(asn_DEF_DiagItem_tags_1)
		/sizeof(asn_DEF_DiagItem_tags_1[0]), /* 1 */
	asn_DEF_DiagItem_tags_1,	/* Same as above */
	sizeof(asn_DEF_DiagItem_tags_1)
		/sizeof(asn_DEF_DiagItem_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DiagItem_1,
	6,	/* Elements count */
	&asn_SPC_DiagItem_specs_1	/* Additional specs */
};

