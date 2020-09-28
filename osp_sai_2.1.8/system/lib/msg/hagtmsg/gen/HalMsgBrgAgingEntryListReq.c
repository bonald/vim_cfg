/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#include <asn_internal.h>

#include "HalMsgBrgAgingEntryListReq.h"

static asn_TYPE_member_t asn_MBR_agingList_4[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_HalMsgBrgAgingEntryReq,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_agingList_tags_4[] = {
	(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_agingList_specs_4 = {
	sizeof(struct agingList),
	offsetof(struct agingList, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_agingList_4 = {
	"agingList",
	"agingList",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_agingList_tags_4,
	sizeof(asn_DEF_agingList_tags_4)
		/sizeof(asn_DEF_agingList_tags_4[0]) - 1, /* 1 */
	asn_DEF_agingList_tags_4,	/* Same as above */
	sizeof(asn_DEF_agingList_tags_4)
		/sizeof(asn_DEF_agingList_tags_4[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_agingList_4,
	1,	/* Single element */
	&asn_SPC_agingList_specs_4	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_HalMsgBrgAgingEntryListReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgAgingEntryListReq, err),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefErrorCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"err"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgAgingEntryListReq, gchip),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gchip"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgAgingEntryListReq, agingList),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_agingList_4,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"agingList"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgBrgAgingEntryListReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgBrgAgingEntryListReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* err at 155 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* gchip at 156 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* agingList at 158 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgBrgAgingEntryListReq_specs_1 = {
	sizeof(struct HalMsgBrgAgingEntryListReq),
	offsetof(struct HalMsgBrgAgingEntryListReq, _asn_ctx),
	asn_MAP_HalMsgBrgAgingEntryListReq_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgBrgAgingEntryListReq = {
	"HalMsgBrgAgingEntryListReq",
	"HalMsgBrgAgingEntryListReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgBrgAgingEntryListReq_tags_1,
	sizeof(asn_DEF_HalMsgBrgAgingEntryListReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgAgingEntryListReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgBrgAgingEntryListReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgBrgAgingEntryListReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgAgingEntryListReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgBrgAgingEntryListReq_1,
	3,	/* Elements count */
	&asn_SPC_HalMsgBrgAgingEntryListReq_specs_1	/* Additional specs */
};

