/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "MPLSEntriesMsg.h"

static asn_TYPE_member_t asn_MBR_mplsEntry_4[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_MPLSEntry,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_mplsEntry_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_mplsEntry_specs_4 = {
	sizeof(struct mplsEntry),
	offsetof(struct mplsEntry, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_mplsEntry_4 = {
	"mplsEntry",
	"mplsEntry",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_mplsEntry_tags_4,
	sizeof(asn_DEF_mplsEntry_tags_4)
		/sizeof(asn_DEF_mplsEntry_tags_4[0]) - 1, /* 1 */
	asn_DEF_mplsEntry_tags_4,	/* Same as above */
	sizeof(asn_DEF_mplsEntry_tags_4)
		/sizeof(asn_DEF_mplsEntry_tags_4[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_mplsEntry_4,
	1,	/* Single element */
	&asn_SPC_mplsEntry_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_MPLSEntriesMsg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MPLSEntriesMsg, entryNumThisTime),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"entryNumThisTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MPLSEntriesMsg, bitmapUpto),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"bitmapUpto"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MPLSEntriesMsg, mplsEntry),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_mplsEntry_4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mplsEntry"
		},
};
static ber_tlv_tag_t asn_DEF_MPLSEntriesMsg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_MPLSEntriesMsg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* entryNumThisTime at 1197 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* bitmapUpto at 1198 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* mplsEntry at 1200 */
};
static asn_SEQUENCE_specifics_t asn_SPC_MPLSEntriesMsg_specs_1 = {
	sizeof(struct MPLSEntriesMsg),
	offsetof(struct MPLSEntriesMsg, _asn_ctx),
	asn_MAP_MPLSEntriesMsg_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_MPLSEntriesMsg = {
	"MPLSEntriesMsg",
	"MPLSEntriesMsg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_MPLSEntriesMsg_tags_1,
	sizeof(asn_DEF_MPLSEntriesMsg_tags_1)
		/sizeof(asn_DEF_MPLSEntriesMsg_tags_1[0]), /* 1 */
	asn_DEF_MPLSEntriesMsg_tags_1,	/* Same as above */
	sizeof(asn_DEF_MPLSEntriesMsg_tags_1)
		/sizeof(asn_DEF_MPLSEntriesMsg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_MPLSEntriesMsg_1,
	3,	/* Elements count */
	&asn_SPC_MPLSEntriesMsg_specs_1	/* Additional specs */
};

