/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "DSFwd.h"

static asn_TYPE_member_t asn_MBR_DSFwd_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, lengthAdjType),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"lengthAdjType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, critiPkt),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"critiPkt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, destMap),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destMap"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, destIdDicard),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"destIdDicard"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, statsPtrValid),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsPtrValid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, sendLocalPhyPort),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sendLocalPhyPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, nexthopPtr),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nexthopPtr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DSFwd, statsPtr),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsPtr"
		},
};
static ber_tlv_tag_t asn_DEF_DSFwd_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_DSFwd_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* lengthAdjType at 406 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* critiPkt at 407 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* destMap at 408 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* destIdDicard at 409 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* statsPtrValid at 410 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* sendLocalPhyPort at 411 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* nexthopPtr at 412 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 } /* statsPtr at 414 */
};
static asn_SEQUENCE_specifics_t asn_SPC_DSFwd_specs_1 = {
	sizeof(struct DSFwd),
	offsetof(struct DSFwd, _asn_ctx),
	asn_MAP_DSFwd_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_DSFwd = {
	"DSFwd",
	"DSFwd",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_DSFwd_tags_1,
	sizeof(asn_DEF_DSFwd_tags_1)
		/sizeof(asn_DEF_DSFwd_tags_1[0]), /* 1 */
	asn_DEF_DSFwd_tags_1,	/* Same as above */
	sizeof(asn_DEF_DSFwd_tags_1)
		/sizeof(asn_DEF_DSFwd_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_DSFwd_1,
	8,	/* Elements count */
	&asn_SPC_DSFwd_specs_1	/* Additional specs */
};

