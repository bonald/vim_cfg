/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "LcDiagResultMsg.h"

static asn_TYPE_member_t asn_MBR_items_4[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_DiagItem,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_items_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_items_specs_4 = {
	sizeof(struct items),
	offsetof(struct items, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_items_4 = {
	"items",
	"items",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_items_tags_4,
	sizeof(asn_DEF_items_tags_4)
		/sizeof(asn_DEF_items_tags_4[0]) - 1, /* 1 */
	asn_DEF_items_tags_4,	/* Same as above */
	sizeof(asn_DEF_items_tags_4)
		/sizeof(asn_DEF_items_tags_4[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_items_4,
	1,	/* Single element */
	&asn_SPC_items_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_LcDiagResultMsg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct LcDiagResultMsg, logicSlotNo),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logicSlotNo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcDiagResultMsg, itemNum),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"itemNum"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcDiagResultMsg, items),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_items_4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"items"
		},
};
static ber_tlv_tag_t asn_DEF_LcDiagResultMsg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LcDiagResultMsg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* logicSlotNo at 483 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* itemNum at 484 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* items at 486 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LcDiagResultMsg_specs_1 = {
	sizeof(struct LcDiagResultMsg),
	offsetof(struct LcDiagResultMsg, _asn_ctx),
	asn_MAP_LcDiagResultMsg_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LcDiagResultMsg = {
	"LcDiagResultMsg",
	"LcDiagResultMsg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_LcDiagResultMsg_tags_1,
	sizeof(asn_DEF_LcDiagResultMsg_tags_1)
		/sizeof(asn_DEF_LcDiagResultMsg_tags_1[0]), /* 1 */
	asn_DEF_LcDiagResultMsg_tags_1,	/* Same as above */
	sizeof(asn_DEF_LcDiagResultMsg_tags_1)
		/sizeof(asn_DEF_LcDiagResultMsg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_LcDiagResultMsg_1,
	3,	/* Elements count */
	&asn_SPC_LcDiagResultMsg_specs_1	/* Additional specs */
};

