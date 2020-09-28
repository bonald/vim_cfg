/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IPUC"
 * 	found in "../l3/hal_msg_ipuc.asn1"
 */

#include <asn_internal.h>

#include "HalIPucNexthop.h"

static asn_TYPE_member_t asn_MBR_HalIPucNexthop_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalIPucNexthop, gportid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalIPucNexthop, vid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalIPucNexthop, oifType),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"oifType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalIPucNexthop, outVlanIsSvlan),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"outVlanIsSvlan"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalIPucNexthop, macAddr),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macAddr"
		},
};
static ber_tlv_tag_t asn_DEF_HalIPucNexthop_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalIPucNexthop_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* gportid at 29 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* vid at 30 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* oifType at 31 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* outVlanIsSvlan at 32 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* macAddr at 33 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalIPucNexthop_specs_1 = {
	sizeof(struct HalIPucNexthop),
	offsetof(struct HalIPucNexthop, _asn_ctx),
	asn_MAP_HalIPucNexthop_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalIPucNexthop = {
	"HalIPucNexthop",
	"HalIPucNexthop",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalIPucNexthop_tags_1,
	sizeof(asn_DEF_HalIPucNexthop_tags_1)
		/sizeof(asn_DEF_HalIPucNexthop_tags_1[0]), /* 1 */
	asn_DEF_HalIPucNexthop_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalIPucNexthop_tags_1)
		/sizeof(asn_DEF_HalIPucNexthop_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalIPucNexthop_1,
	5,	/* Elements count */
	&asn_SPC_HalIPucNexthop_specs_1	/* Additional specs */
};

