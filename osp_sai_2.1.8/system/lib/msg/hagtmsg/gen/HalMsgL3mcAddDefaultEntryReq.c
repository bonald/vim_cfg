/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-L3MC"
 * 	found in "../l3/hal_msg_l3mc.asn1"
 */

#include <asn_internal.h>

#include "HalMsgL3mcAddDefaultEntryReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgL3mcAddDefaultEntryReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcAddDefaultEntryReq, isIpv4En),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isIpv4En"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgL3mcAddDefaultEntryReq, isIpv6En),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isIpv6En"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgL3mcAddDefaultEntryReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* isIpv4En at 64 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* isIpv6En at 66 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgL3mcAddDefaultEntryReq_specs_1 = {
	sizeof(struct HalMsgL3mcAddDefaultEntryReq),
	offsetof(struct HalMsgL3mcAddDefaultEntryReq, _asn_ctx),
	asn_MAP_HalMsgL3mcAddDefaultEntryReq_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgL3mcAddDefaultEntryReq = {
	"HalMsgL3mcAddDefaultEntryReq",
	"HalMsgL3mcAddDefaultEntryReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1,
	sizeof(asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1)
		/sizeof(asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1)
		/sizeof(asn_DEF_HalMsgL3mcAddDefaultEntryReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgL3mcAddDefaultEntryReq_1,
	2,	/* Elements count */
	&asn_SPC_HalMsgL3mcAddDefaultEntryReq_specs_1	/* Additional specs */
};

