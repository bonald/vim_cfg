/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#include <asn_internal.h>

#include "HalMsgQosSetPortQueueClassResp.h"

static asn_TYPE_member_t asn_MBR_HalMsgQosSetPortQueueClassResp_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosSetPortQueueClassResp, port),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"port"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosSetPortQueueClassResp, qid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"qid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosSetPortQueueClassResp, Class),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"class"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosSetPortQueueClassResp, success),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"success"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgQosSetPortQueueClassResp_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* port at 351 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* qid at 352 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* class at 353 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* success at 355 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgQosSetPortQueueClassResp_specs_1 = {
	sizeof(struct HalMsgQosSetPortQueueClassResp),
	offsetof(struct HalMsgQosSetPortQueueClassResp, _asn_ctx),
	asn_MAP_HalMsgQosSetPortQueueClassResp_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgQosSetPortQueueClassResp = {
	"HalMsgQosSetPortQueueClassResp",
	"HalMsgQosSetPortQueueClassResp",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1,
	sizeof(asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1)
		/sizeof(asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1[0]), /* 1 */
	asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1)
		/sizeof(asn_DEF_HalMsgQosSetPortQueueClassResp_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgQosSetPortQueueClassResp_1,
	4,	/* Elements count */
	&asn_SPC_HalMsgQosSetPortQueueClassResp_specs_1	/* Additional specs */
};
