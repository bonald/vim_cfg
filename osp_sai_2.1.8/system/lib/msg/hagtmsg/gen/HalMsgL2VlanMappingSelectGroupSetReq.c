/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L2-VLAN-MAPPING"
 * 	found in "../l2/hal_msg_vlan_mapping.asn1"
 */

#include <asn_internal.h>

#include "HalMsgL2VlanMappingSelectGroupSetReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgL2VlanMappingSelectGroupSetReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, groupId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, add),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"add"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin0),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin0"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax0),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax0"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin1),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax1),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin2),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax2),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin3),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin3"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax3),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax3"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin4),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin4"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax4),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax4"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin5),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin5"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax5),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax5"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin6),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin6"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax6),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax6"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMin7),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMin7"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, vlanMax7),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"vlanMax7"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgL2VlanMappingSelectGroupSetReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* groupId at 105 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* add at 106 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* vlanMin0 at 107 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* vlanMax0 at 108 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* vlanMin1 at 109 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* vlanMax1 at 110 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* vlanMin2 at 111 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* vlanMax2 at 112 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* vlanMin3 at 113 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* vlanMax3 at 114 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* vlanMin4 at 115 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* vlanMax4 at 116 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* vlanMin5 at 117 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* vlanMax5 at 118 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* vlanMin6 at 119 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* vlanMax6 at 120 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* vlanMin7 at 121 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 } /* vlanMax7 at 123 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgL2VlanMappingSelectGroupSetReq_specs_1 = {
	sizeof(struct HalMsgL2VlanMappingSelectGroupSetReq),
	offsetof(struct HalMsgL2VlanMappingSelectGroupSetReq, _asn_ctx),
	asn_MAP_HalMsgL2VlanMappingSelectGroupSetReq_tag2el_1,
	18,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq = {
	"HalMsgL2VlanMappingSelectGroupSetReq",
	"HalMsgL2VlanMappingSelectGroupSetReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1,
	sizeof(asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1)
		/sizeof(asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1)
		/sizeof(asn_DEF_HalMsgL2VlanMappingSelectGroupSetReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgL2VlanMappingSelectGroupSetReq_1,
	18,	/* Elements count */
	&asn_SPC_HalMsgL2VlanMappingSelectGroupSetReq_specs_1	/* Additional specs */
};

