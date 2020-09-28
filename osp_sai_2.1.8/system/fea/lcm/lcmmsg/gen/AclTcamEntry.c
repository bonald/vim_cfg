/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "AclTcamEntry.h"

static asn_TYPE_member_t asn_MBR_key_6[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct key, macKey),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosMacKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macKey"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct key, ipv4Key),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosIpv4Key,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv4Key"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct key, ipv6Key),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosIpv6Key,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv6Key"
		},
};
static asn_TYPE_tag2member_t asn_MAP_key_tag2el_6[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* macKey at 195 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ipv4Key at 196 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ipv6Key at 198 */
};
static asn_CHOICE_specifics_t asn_SPC_key_specs_6 = {
	sizeof(struct key),
	offsetof(struct key, _asn_ctx),
	offsetof(struct key, present),
	sizeof(((struct key *)0)->present),
	asn_MAP_key_tag2el_6,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_key_6 = {
	"key",
	"key",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_key_6,
	3,	/* Elements count */
	&asn_SPC_key_specs_6	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_mask_10[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct mask, macMask),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosMacKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"macMask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct mask, ipv4Mask),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosIpv4Key,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv4Mask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct mask, ipv6Mask),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSAclQosIpv6Key,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ipv6Mask"
		},
};
static asn_TYPE_tag2member_t asn_MAP_mask_tag2el_10[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* macMask at 202 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* ipv4Mask at 203 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ipv6Mask at 205 */
};
static asn_CHOICE_specifics_t asn_SPC_mask_specs_10 = {
	sizeof(struct mask),
	offsetof(struct mask, _asn_ctx),
	offsetof(struct mask, present),
	sizeof(((struct mask *)0)->present),
	asn_MAP_mask_tag2el_10,
	3,	/* Count of tags in the map */
	0,
	-1	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_mask_10 = {
	"mask",
	"mask",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	0,	/* No PER visible constraints */
	asn_MBR_mask_10,
	3,	/* Elements count */
	&asn_SPC_mask_specs_10	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_AclTcamEntry_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, index),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"index"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, chipid),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"chipid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, ifType),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ifType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, type),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, key),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_key_6,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"key"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct AclTcamEntry, mask),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_mask_10,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mask"
		},
};
static ber_tlv_tag_t asn_DEF_AclTcamEntry_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_AclTcamEntry_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* index at 189 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* chipid at 190 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* ifType at 191 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* type at 192 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* key at 195 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 } /* mask at 202 */
};
static asn_SEQUENCE_specifics_t asn_SPC_AclTcamEntry_specs_1 = {
	sizeof(struct AclTcamEntry),
	offsetof(struct AclTcamEntry, _asn_ctx),
	asn_MAP_AclTcamEntry_tag2el_1,
	6,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_AclTcamEntry = {
	"AclTcamEntry",
	"AclTcamEntry",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_AclTcamEntry_tags_1,
	sizeof(asn_DEF_AclTcamEntry_tags_1)
		/sizeof(asn_DEF_AclTcamEntry_tags_1[0]), /* 1 */
	asn_DEF_AclTcamEntry_tags_1,	/* Same as above */
	sizeof(asn_DEF_AclTcamEntry_tags_1)
		/sizeof(asn_DEF_AclTcamEntry_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_AclTcamEntry_1,
	6,	/* Elements count */
	&asn_SPC_AclTcamEntry_specs_1	/* Additional specs */
};

