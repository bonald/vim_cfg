/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-OPENFLOW-METER"
 * 	found in "../openflow/hal_msg_openflow_meter.asn1"
 */

#include <asn_internal.h>

#include "HalMsgOpenFlowMeterBandKey.h"

static asn_TYPE_member_t asn_MBR_HalMsgOpenFlowMeterBandKey_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowMeterBandKey, type),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"type"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowMeterBandKey, rate),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rate"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgOpenFlowMeterBandKey, burst),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"burst"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgOpenFlowMeterBandKey_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* type at 10 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* rate at 11 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* burst at 13 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgOpenFlowMeterBandKey_specs_1 = {
	sizeof(struct HalMsgOpenFlowMeterBandKey),
	offsetof(struct HalMsgOpenFlowMeterBandKey, _asn_ctx),
	asn_MAP_HalMsgOpenFlowMeterBandKey_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgOpenFlowMeterBandKey = {
	"HalMsgOpenFlowMeterBandKey",
	"HalMsgOpenFlowMeterBandKey",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1,
	sizeof(asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1[0]), /* 1 */
	asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1)
		/sizeof(asn_DEF_HalMsgOpenFlowMeterBandKey_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgOpenFlowMeterBandKey_1,
	3,	/* Elements count */
	&asn_SPC_HalMsgOpenFlowMeterBandKey_specs_1	/* Additional specs */
};
