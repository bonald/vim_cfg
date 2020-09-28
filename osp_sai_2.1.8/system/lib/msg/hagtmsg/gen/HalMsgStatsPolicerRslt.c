/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-STATS"
 * 	found in "../common/hal_msg_stats.asn1"
 */

#include <asn_internal.h>

#include "HalMsgStatsPolicerRslt.h"

static asn_TYPE_member_t asn_MBR_HalMsgStatsPolicerRslt_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, confirmPackets),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"confirmPackets"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, confirmBytes),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"confirmBytes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, exceedPackets),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"exceedPackets"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, exceedBytes),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"exceedBytes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, violatePackets),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"violatePackets"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, violateBytes),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"violateBytes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, resourceId),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"resourceId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgStatsPolicerRslt, supportFlag),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"supportFlag"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgStatsPolicerRslt_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgStatsPolicerRslt_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* confirmPackets at 181 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* confirmBytes at 182 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* exceedPackets at 183 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* exceedBytes at 184 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* violatePackets at 185 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* violateBytes at 186 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* resourceId at 187 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 } /* supportFlag at 189 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgStatsPolicerRslt_specs_1 = {
	sizeof(struct HalMsgStatsPolicerRslt),
	offsetof(struct HalMsgStatsPolicerRslt, _asn_ctx),
	asn_MAP_HalMsgStatsPolicerRslt_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgStatsPolicerRslt = {
	"HalMsgStatsPolicerRslt",
	"HalMsgStatsPolicerRslt",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgStatsPolicerRslt_tags_1,
	sizeof(asn_DEF_HalMsgStatsPolicerRslt_tags_1)
		/sizeof(asn_DEF_HalMsgStatsPolicerRslt_tags_1[0]), /* 1 */
	asn_DEF_HalMsgStatsPolicerRslt_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgStatsPolicerRslt_tags_1)
		/sizeof(asn_DEF_HalMsgStatsPolicerRslt_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgStatsPolicerRslt_1,
	8,	/* Elements count */
	&asn_SPC_HalMsgStatsPolicerRslt_specs_1	/* Additional specs */
};

