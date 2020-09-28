/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "VlanClassvlanEntry.h"

static asn_TYPE_member_t asn_MBR_VlanClassvlanEntry_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct VlanClassvlanEntry, key),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UsrIDVlanKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"key"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VlanClassvlanEntry, mask),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_UsrIDVlanKey,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"mask"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct VlanClassvlanEntry, ds),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DSUsrID,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"ds"
		},
};
static ber_tlv_tag_t asn_DEF_VlanClassvlanEntry_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_VlanClassvlanEntry_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* key at 1101 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* mask at 1102 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* ds at 1104 */
};
static asn_SEQUENCE_specifics_t asn_SPC_VlanClassvlanEntry_specs_1 = {
	sizeof(struct VlanClassvlanEntry),
	offsetof(struct VlanClassvlanEntry, _asn_ctx),
	asn_MAP_VlanClassvlanEntry_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_VlanClassvlanEntry = {
	"VlanClassvlanEntry",
	"VlanClassvlanEntry",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_VlanClassvlanEntry_tags_1,
	sizeof(asn_DEF_VlanClassvlanEntry_tags_1)
		/sizeof(asn_DEF_VlanClassvlanEntry_tags_1[0]), /* 1 */
	asn_DEF_VlanClassvlanEntry_tags_1,	/* Same as above */
	sizeof(asn_DEF_VlanClassvlanEntry_tags_1)
		/sizeof(asn_DEF_VlanClassvlanEntry_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_VlanClassvlanEntry_1,
	3,	/* Elements count */
	&asn_SPC_VlanClassvlanEntry_specs_1	/* Additional specs */
};

