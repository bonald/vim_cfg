/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#include <asn_internal.h>

#include "HalMsgBrgFilterToCpuListReq.h"

static asn_TYPE_member_t asn_MBR_filterToCpuArray_3[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_HalMsgBrgFilterToCpuReq,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_filterToCpuArray_tags_3[] = {
	(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_filterToCpuArray_specs_3 = {
	sizeof(struct filterToCpuArray),
	offsetof(struct filterToCpuArray, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_filterToCpuArray_3 = {
	"filterToCpuArray",
	"filterToCpuArray",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_filterToCpuArray_tags_3,
	sizeof(asn_DEF_filterToCpuArray_tags_3)
		/sizeof(asn_DEF_filterToCpuArray_tags_3[0]) - 1, /* 1 */
	asn_DEF_filterToCpuArray_tags_3,	/* Same as above */
	sizeof(asn_DEF_filterToCpuArray_tags_3)
		/sizeof(asn_DEF_filterToCpuArray_tags_3[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_filterToCpuArray_3,
	1,	/* Single element */
	&asn_SPC_filterToCpuArray_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_HalMsgBrgFilterToCpuListReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFilterToCpuListReq, err),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefErrorCode,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"err"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFilterToCpuListReq, filterToCpuArray),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_filterToCpuArray_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"filterToCpuArray"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgBrgFilterToCpuListReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* err at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* filterToCpuArray at 89 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgBrgFilterToCpuListReq_specs_1 = {
	sizeof(struct HalMsgBrgFilterToCpuListReq),
	offsetof(struct HalMsgBrgFilterToCpuListReq, _asn_ctx),
	asn_MAP_HalMsgBrgFilterToCpuListReq_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgBrgFilterToCpuListReq = {
	"HalMsgBrgFilterToCpuListReq",
	"HalMsgBrgFilterToCpuListReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1,
	sizeof(asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgFilterToCpuListReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgBrgFilterToCpuListReq_1,
	2,	/* Elements count */
	&asn_SPC_HalMsgBrgFilterToCpuListReq_specs_1	/* Additional specs */
};

