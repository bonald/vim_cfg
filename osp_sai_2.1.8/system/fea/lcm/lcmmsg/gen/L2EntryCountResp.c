/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "L2EntryCountResp.h"

static asn_TYPE_member_t asn_MBR_L2EntryCountResp_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, filterCnt),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"filterCnt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, withVIDCnt),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"withVIDCnt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, withoutVIDCnt),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"withoutVIDCnt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, snoopingCnt),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"snoopingCnt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, defaultCnt),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"defaultCnt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct L2EntryCountResp, mcastCnt),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mcastCnt"
		},
};
static ber_tlv_tag_t asn_DEF_L2EntryCountResp_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_L2EntryCountResp_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* filterCnt at 298 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* withVIDCnt at 299 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* withoutVIDCnt at 300 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* snoopingCnt at 301 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* defaultCnt at 302 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* mcastCnt at 304 */
};
static asn_SEQUENCE_specifics_t asn_SPC_L2EntryCountResp_specs_1 = {
	sizeof(struct L2EntryCountResp),
	offsetof(struct L2EntryCountResp, _asn_ctx),
	asn_MAP_L2EntryCountResp_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_L2EntryCountResp = {
	"L2EntryCountResp",
	"L2EntryCountResp",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_L2EntryCountResp_tags_1,
	sizeof(asn_DEF_L2EntryCountResp_tags_1)
		/sizeof(asn_DEF_L2EntryCountResp_tags_1[0]), /* 1 */
	asn_DEF_L2EntryCountResp_tags_1,	/* Same as above */
	sizeof(asn_DEF_L2EntryCountResp_tags_1)
		/sizeof(asn_DEF_L2EntryCountResp_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_L2EntryCountResp_1,
	6,	/* Elements count */
	&asn_SPC_L2EntryCountResp_specs_1	/* Additional specs */
};
