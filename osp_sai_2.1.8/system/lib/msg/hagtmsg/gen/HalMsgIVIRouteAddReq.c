/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-IVI"
 * 	found in "../l3/hal_msg_ivi.asn1"
 */

#include <asn_internal.h>

#include "HalMsgIVIRouteAddReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgIVIRouteAddReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, vrfid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vrfid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv4Addr),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv4Addr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv4masklen),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv4masklen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv4nhid),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv4nhid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv6Addr),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv6Addr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv6masklen),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv6masklen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgIVIRouteAddReq, ipv6nhid),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv6nhid"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgIVIRouteAddReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgIVIRouteAddReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* vrfid at 17 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ipv4Addr at 18 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ipv4masklen at 19 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* ipv4nhid at 20 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* ipv6Addr at 21 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* ipv6masklen at 22 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 } /* ipv6nhid at 24 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgIVIRouteAddReq_specs_1 = {
	sizeof(struct HalMsgIVIRouteAddReq),
	offsetof(struct HalMsgIVIRouteAddReq, _asn_ctx),
	asn_MAP_HalMsgIVIRouteAddReq_tag2el_1,
	7,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgIVIRouteAddReq = {
	"HalMsgIVIRouteAddReq",
	"HalMsgIVIRouteAddReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgIVIRouteAddReq_tags_1,
	sizeof(asn_DEF_HalMsgIVIRouteAddReq_tags_1)
		/sizeof(asn_DEF_HalMsgIVIRouteAddReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgIVIRouteAddReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgIVIRouteAddReq_tags_1)
		/sizeof(asn_DEF_HalMsgIVIRouteAddReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgIVIRouteAddReq_1,
	7,	/* Elements count */
	&asn_SPC_HalMsgIVIRouteAddReq_specs_1	/* Additional specs */
};

