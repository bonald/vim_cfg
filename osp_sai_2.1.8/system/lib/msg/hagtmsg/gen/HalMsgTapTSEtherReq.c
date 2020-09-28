/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-TAP"
 * 	found in "../tap/hal_msg_tap.asn1"
 */

#include <asn_internal.h>

#include "HalMsgTapTSEtherReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgTapTSEtherReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgTapTSEtherReq, macda),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macda"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgTapTSEtherReq, macsa),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macsa"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgTapTSEtherReq, etherType),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"etherType"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgTapTSEtherReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgTapTSEtherReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* macda at 43 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* macsa at 44 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* etherType at 46 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgTapTSEtherReq_specs_1 = {
	sizeof(struct HalMsgTapTSEtherReq),
	offsetof(struct HalMsgTapTSEtherReq, _asn_ctx),
	asn_MAP_HalMsgTapTSEtherReq_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgTapTSEtherReq = {
	"HalMsgTapTSEtherReq",
	"HalMsgTapTSEtherReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgTapTSEtherReq_tags_1,
	sizeof(asn_DEF_HalMsgTapTSEtherReq_tags_1)
		/sizeof(asn_DEF_HalMsgTapTSEtherReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgTapTSEtherReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgTapTSEtherReq_tags_1)
		/sizeof(asn_DEF_HalMsgTapTSEtherReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgTapTSEtherReq_1,
	3,	/* Elements count */
	&asn_SPC_HalMsgTapTSEtherReq_specs_1	/* Additional specs */
};
