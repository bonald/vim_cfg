/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2MC"
 * 	found in "../l2/hal_msg_l2mc.asn1"
 */

#include <asn_internal.h>

#include "HalMsgL2mcPortReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgL2mcPortReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2mcPortReq, fid),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"fid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2mcPortReq, mac),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgDefL2MacAddr,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mac"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2mcPortReq, gport),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"gport"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2mcPortReq, localMemberBmp),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"localMemberBmp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2mcPortReq, maxGchipNum),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"maxGchipNum"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgL2mcPortReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgL2mcPortReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* fid at 28 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* mac at 29 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* gport at 30 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* localMemberBmp at 31 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* maxGchipNum at 33 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgL2mcPortReq_specs_1 = {
	sizeof(struct HalMsgL2mcPortReq),
	offsetof(struct HalMsgL2mcPortReq, _asn_ctx),
	asn_MAP_HalMsgL2mcPortReq_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgL2mcPortReq = {
	"HalMsgL2mcPortReq",
	"HalMsgL2mcPortReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgL2mcPortReq_tags_1,
	sizeof(asn_DEF_HalMsgL2mcPortReq_tags_1)
		/sizeof(asn_DEF_HalMsgL2mcPortReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgL2mcPortReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgL2mcPortReq_tags_1)
		/sizeof(asn_DEF_HalMsgL2mcPortReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgL2mcPortReq_1,
	5,	/* Elements count */
	&asn_SPC_HalMsgL2mcPortReq_specs_1	/* Additional specs */
};

