/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "DSDestPort.h"

static asn_TYPE_member_t asn_MBR_DSDestPort_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, dot1qEtherTypeEn),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"dot1qEtherTypeEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, untagDefautVid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"untagDefautVid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, portPolicerValid),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"portPolicerValid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, transmitEn),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"transmitEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2AclEn),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2AclEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2QosLookupEn),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2QosLookupEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, bridgeEn),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"bridgeEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2SpanEn),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2SpanEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2SpanId),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2SpanId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, forceMackey),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"forceMackey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2QosHighPrio),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2QosHighPrio"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, egressFilterEn),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"egressFilterEn"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, muxPortType),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"muxPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, routedPort),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"routedPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, globalDestPort),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"globalDestPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, vlanFlowPolicerValid),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanFlowPolicerValid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2AclHighPrio),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2AclHighPrio"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, stpCheckDisable),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"stpCheckDisable"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, replaceCos),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"replaceCos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, vplsPortType),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vplsPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, replaceDscp),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"replaceDscp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, destPortBundleId),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destPortBundleId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, defaultVid),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"defaultVid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2AclLabel),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2AclLabel"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSDestPort, l2QosLabel),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"l2QosLabel"
		},
};
static ber_tlv_tag_t asn_DEF_DSDestPort_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DSDestPort_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* dot1qEtherTypeEn at 432 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* untagDefautVid at 433 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* portPolicerValid at 434 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* transmitEn at 435 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* l2AclEn at 436 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* l2QosLookupEn at 437 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* bridgeEn at 438 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* l2SpanEn at 439 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* l2SpanId at 440 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* forceMackey at 441 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* l2QosHighPrio at 442 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* egressFilterEn at 443 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* muxPortType at 444 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* routedPort at 445 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* globalDestPort at 446 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* vlanFlowPolicerValid at 447 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* l2AclHighPrio at 448 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* stpCheckDisable at 449 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* replaceCos at 450 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* vplsPortType at 451 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* replaceDscp at 452 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* destPortBundleId at 453 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* defaultVid at 454 */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 }, /* l2AclLabel at 455 */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 24, 0, 0 } /* l2QosLabel at 457 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DSDestPort_specs_1 = {
	sizeof(struct DSDestPort),
	offsetof(struct DSDestPort, _asn_ctx),
	asn_MAP_DSDestPort_tag2el_1,
	25,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DSDestPort = {
	"DSDestPort",
	"DSDestPort",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DSDestPort_tags_1,
	sizeof(asn_DEF_DSDestPort_tags_1)
		/sizeof(asn_DEF_DSDestPort_tags_1[0]), /* 1 */
	asn_DEF_DSDestPort_tags_1,	/* Same as above */
	sizeof(asn_DEF_DSDestPort_tags_1)
		/sizeof(asn_DEF_DSDestPort_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DSDestPort_1,
	25,	/* Elements count */
	&asn_SPC_DSDestPort_specs_1	/* Additional specs */
};

