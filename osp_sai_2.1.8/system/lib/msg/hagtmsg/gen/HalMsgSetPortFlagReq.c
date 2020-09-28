/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-LACP"
 * 	found in "../l2/hal_msg_agg.asn1"
 */

#include <asn_internal.h>

#include "HalMsgSetPortFlagReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgSetPortFlagReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgSetPortFlagReq, enable),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"enable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgSetPortFlagReq, flag),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgSetPortFlagReq, portid),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"portid"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgSetPortFlagReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgSetPortFlagReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* enable at 41 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* flag at 42 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* portid at 44 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgSetPortFlagReq_specs_1 = {
	sizeof(struct HalMsgSetPortFlagReq),
	offsetof(struct HalMsgSetPortFlagReq, _asn_ctx),
	asn_MAP_HalMsgSetPortFlagReq_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgSetPortFlagReq = {
	"HalMsgSetPortFlagReq",
	"HalMsgSetPortFlagReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgSetPortFlagReq_tags_1,
	sizeof(asn_DEF_HalMsgSetPortFlagReq_tags_1)
		/sizeof(asn_DEF_HalMsgSetPortFlagReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgSetPortFlagReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgSetPortFlagReq_tags_1)
		/sizeof(asn_DEF_HalMsgSetPortFlagReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgSetPortFlagReq_1,
	3,	/* Elements count */
	&asn_SPC_HalMsgSetPortFlagReq_specs_1	/* Additional specs */
};

