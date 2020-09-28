/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-LACP"
 * 	found in "../l2/hal_msg_agg.asn1"
 */

#include <asn_internal.h>

#include "HalMsgAggPropSyncReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgAggPropSyncReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAggPropSyncReq, memPortid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"memPortid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAggPropSyncReq, aggPortid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"aggPortid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAggPropSyncReq, routeEn),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routeEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAggPropSyncReq, l3ifid),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l3ifid"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgAggPropSyncReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgAggPropSyncReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* memPortid at 24 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* aggPortid at 25 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* routeEn at 26 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* l3ifid at 29 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgAggPropSyncReq_specs_1 = {
	sizeof(struct HalMsgAggPropSyncReq),
	offsetof(struct HalMsgAggPropSyncReq, _asn_ctx),
	asn_MAP_HalMsgAggPropSyncReq_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgAggPropSyncReq = {
	"HalMsgAggPropSyncReq",
	"HalMsgAggPropSyncReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgAggPropSyncReq_tags_1,
	sizeof(asn_DEF_HalMsgAggPropSyncReq_tags_1)
		/sizeof(asn_DEF_HalMsgAggPropSyncReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgAggPropSyncReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgAggPropSyncReq_tags_1)
		/sizeof(asn_DEF_HalMsgAggPropSyncReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgAggPropSyncReq_1,
	4,	/* Elements count */
	&asn_SPC_HalMsgAggPropSyncReq_specs_1	/* Additional specs */
};

