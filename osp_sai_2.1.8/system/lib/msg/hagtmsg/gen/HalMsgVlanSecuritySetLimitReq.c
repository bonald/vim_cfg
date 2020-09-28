/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-VLAN-SECURITY"
 * 	found in "../l2/hal_msg_vlan_security.asn1"
 */

#include <asn_internal.h>

#include "HalMsgVlanSecuritySetLimitReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgVlanSecuritySetLimitReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgVlanSecuritySetLimitReq, vid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgVlanSecuritySetLimitReq, maxMacNum),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"maxMacNum"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgVlanSecuritySetLimitReq, action),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"action"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgVlanSecuritySetLimitReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* vid at 6 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* maxMacNum at 7 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* action at 9 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgVlanSecuritySetLimitReq_specs_1 = {
	sizeof(struct HalMsgVlanSecuritySetLimitReq),
	offsetof(struct HalMsgVlanSecuritySetLimitReq, _asn_ctx),
	asn_MAP_HalMsgVlanSecuritySetLimitReq_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgVlanSecuritySetLimitReq = {
	"HalMsgVlanSecuritySetLimitReq",
	"HalMsgVlanSecuritySetLimitReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1,
	sizeof(asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1)
		/sizeof(asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1)
		/sizeof(asn_DEF_HalMsgVlanSecuritySetLimitReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgVlanSecuritySetLimitReq_1,
	3,	/* Elements count */
	&asn_SPC_HalMsgVlanSecuritySetLimitReq_specs_1	/* Additional specs */
};

