/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ETH-OAM"
 * 	found in "../l2/hal_msg_eth_oam.asn1"
 */

#include <asn_internal.h>

#include "HalMsgEthOamCfmSetLmEnableReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgEthOamCfmSetLmEnableReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, glPort),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"glPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, vlanid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, direction),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"direction"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, level),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"level"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, mepid),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mepid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, enable),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"enable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, lmType),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, cosType),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cosType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, cosValue),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cosValue"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, statsInterval),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsInterval"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, portLMenable),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"portLMenable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgEthOamCfmSetLmEnableReq, lmMembPort),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgIntList,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lmMembPort"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgEthOamCfmSetLmEnableReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* glPort at 191 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* vlanid at 192 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* direction at 193 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* level at 194 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* mepid at 195 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* enable at 196 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* lmType at 197 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* cosType at 198 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* cosValue at 199 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* statsInterval at 200 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* portLMenable at 201 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 } /* lmMembPort at 203 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgEthOamCfmSetLmEnableReq_specs_1 = {
	sizeof(struct HalMsgEthOamCfmSetLmEnableReq),
	offsetof(struct HalMsgEthOamCfmSetLmEnableReq, _asn_ctx),
	asn_MAP_HalMsgEthOamCfmSetLmEnableReq_tag2el_1,
	12,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgEthOamCfmSetLmEnableReq = {
	"HalMsgEthOamCfmSetLmEnableReq",
	"HalMsgEthOamCfmSetLmEnableReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1,
	sizeof(asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1)
		/sizeof(asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1)
		/sizeof(asn_DEF_HalMsgEthOamCfmSetLmEnableReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgEthOamCfmSetLmEnableReq_1,
	12,	/* Elements count */
	&asn_SPC_HalMsgEthOamCfmSetLmEnableReq_specs_1	/* Additional specs */
};

