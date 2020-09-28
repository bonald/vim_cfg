/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L3MC"
 * 	found in "../l3/hal_msg_l3mc.asn1"
 */

#include <asn_internal.h>

#include "HalMsgL3mcIPv6EntryStatPollReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgL3mcIPv6EntryStatPollReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, group),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"group"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, source),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"source"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, groupMaskLen),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupMaskLen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, sourceMaskLen),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"sourceMaskLen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, statsid),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"statsid"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgL3mcIPv6EntryStatPollReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* group at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* source at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* groupMaskLen at 92 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* sourceMaskLen at 93 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* statsid at 95 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgL3mcIPv6EntryStatPollReq_specs_1 = {
	sizeof(struct HalMsgL3mcIPv6EntryStatPollReq),
	offsetof(struct HalMsgL3mcIPv6EntryStatPollReq, _asn_ctx),
	asn_MAP_HalMsgL3mcIPv6EntryStatPollReq_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgL3mcIPv6EntryStatPollReq = {
	"HalMsgL3mcIPv6EntryStatPollReq",
	"HalMsgL3mcIPv6EntryStatPollReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1,
	sizeof(asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1)
		/sizeof(asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1)
		/sizeof(asn_DEF_HalMsgL3mcIPv6EntryStatPollReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgL3mcIPv6EntryStatPollReq_1,
	5,	/* Elements count */
	&asn_SPC_HalMsgL3mcIPv6EntryStatPollReq_specs_1	/* Additional specs */
};

