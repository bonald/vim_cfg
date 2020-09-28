/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-BRG"
 * 	found in "../l2/hal_msg_bridge.asn1"
 */

#include <asn_internal.h>

#include "HalMsgBrgFdbOpReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgBrgFdbOpReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, fid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"fid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, mac),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mac"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, glPort),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"glPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, label),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"label"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, gsrcportType),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gsrcportType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, flag),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, nhid),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgBrgFdbOpReq, virtualPort),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"virtualPort"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgBrgFdbOpReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgBrgFdbOpReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* fid at 13 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* mac at 14 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* glPort at 15 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* label at 16 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* gsrcportType at 17 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* flag at 18 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* nhid at 19 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 } /* virtualPort at 21 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgBrgFdbOpReq_specs_1 = {
	sizeof(struct HalMsgBrgFdbOpReq),
	offsetof(struct HalMsgBrgFdbOpReq, _asn_ctx),
	asn_MAP_HalMsgBrgFdbOpReq_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgBrgFdbOpReq = {
	"HalMsgBrgFdbOpReq",
	"HalMsgBrgFdbOpReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgBrgFdbOpReq_tags_1,
	sizeof(asn_DEF_HalMsgBrgFdbOpReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgFdbOpReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgBrgFdbOpReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgBrgFdbOpReq_tags_1)
		/sizeof(asn_DEF_HalMsgBrgFdbOpReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgBrgFdbOpReq_1,
	8,	/* Elements count */
	&asn_SPC_HalMsgBrgFdbOpReq_specs_1	/* Additional specs */
};
