/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-FLOW"
 * 	found in "../openflow/hal_msg_openflow_flow.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowAction.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowAction_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, policerId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"policerId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, statsEn),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, statsId),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, priorityValid),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"priorityValid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, priority),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"priority"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, color),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"color"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, nexthopId),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nexthopId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, denyRouting),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"denyRouting"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, denyBridging),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"denyBridging"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, denyLearning),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"denyLearning"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, popAllMpls),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"popAllMpls"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, isDsMplsKey),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isDsMplsKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, mplsActionType),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mplsActionType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, resetPayloadOffset),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"resetPayloadOffset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, oldLogSessionId),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"oldLogSessionId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, logSessionId),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logSessionId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, logDestPort),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"logDestPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, truncateLen),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"truncateLen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, groupVlanId),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupVlanId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, groupVlanCos),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupVlanCos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, groupVlanType),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupVlanType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, stripPacket),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"stripPacket"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowAction, stripLen),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"stripLen"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowAction_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowAction_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* policerId at 176 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* statsEn at 177 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* statsId at 178 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* priorityValid at 180 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* priority at 181 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* color at 182 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* nexthopId at 184 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* denyRouting at 185 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* denyBridging at 186 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* denyLearning at 187 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* popAllMpls at 188 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* isDsMplsKey at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* mplsActionType at 190 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* resetPayloadOffset at 191 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* oldLogSessionId at 192 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* logSessionId at 193 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* logDestPort at 194 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* truncateLen at 195 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* groupVlanId at 196 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* groupVlanCos at 197 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* groupVlanType at 198 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* stripPacket at 200 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }  /* stripLen at 201 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowAction_specs_1 = {
	sizeof(struct HalMsgOpenFlowAction),
	offsetof(struct HalMsgOpenFlowAction, _asn_ctx),
	asn_MAP_HalMsgOpenFlowAction_tag2el_1,
	22,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowAction = {
	"HalMsgOpenFlowAction",
	"HalMsgOpenFlowAction",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowAction_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowAction_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowAction_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowAction_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowAction_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowAction_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowAction_1,
	22,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowAction_specs_1	/* Additional specs */
};
