/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-ACL"
 * 	found in "../aclqos/hal_msg_acl.asn1"
 */

#include <asn_internal.h>

#include "HalMsgAclAction.h"

static asn_TYPE_member_t asn_MBR_HalMsgAclAction_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, flag),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, redirect),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"redirect"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, logId),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, logWeight),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logWeight"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, priority),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"priority"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, color),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"color"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, trust),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"trust"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, macroPolicerId),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macroPolicerId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, microPolicerId),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"microPolicerId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, dscp),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dscp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, qosDomain),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"qosDomain"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgAclAction, cos),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cos"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgAclAction_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgAclAction_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flag at 14 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* redirect at 15 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* logId at 16 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* logWeight at 17 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* priority at 18 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* color at 19 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* trust at 20 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* macroPolicerId at 21 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* microPolicerId at 22 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* dscp at 23 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* qosDomain at 24 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 } /* cos at 27 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgAclAction_specs_1 = {
	sizeof(struct HalMsgAclAction),
	offsetof(struct HalMsgAclAction, _asn_ctx),
	asn_MAP_HalMsgAclAction_tag2el_1,
	12,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgAclAction = {
	"HalMsgAclAction",
	"HalMsgAclAction",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgAclAction_tags_1,
	sizeof(asn_DEF_HalMsgAclAction_tags_1)
		/sizeof(asn_DEF_HalMsgAclAction_tags_1[0]), /* 1 */
	asn_DEF_HalMsgAclAction_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgAclAction_tags_1)
		/sizeof(asn_DEF_HalMsgAclAction_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgAclAction_1,
	12,	/* Elements count */
	&asn_SPC_HalMsgAclAction_specs_1	/* Additional specs */
};

