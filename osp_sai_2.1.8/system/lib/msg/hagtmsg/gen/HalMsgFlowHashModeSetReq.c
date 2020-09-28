/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-EFD"
 * 	found in "../misc/hal_msg_efd.asn1"
 */

#include <asn_internal.h>

#include "HalMsgFlowHashModeSetReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgFlowHashModeSetReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowHashModeSetReq, flowhashoutertype),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flowhashoutertype"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgFlowHashModeSetReq, flowhashinnertype),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flowhashinnertype"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgFlowHashModeSetReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgFlowHashModeSetReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* flowhashoutertype at 47 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* flowhashinnertype at 49 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgFlowHashModeSetReq_specs_1 = {
	sizeof(struct HalMsgFlowHashModeSetReq),
	offsetof(struct HalMsgFlowHashModeSetReq, _asn_ctx),
	asn_MAP_HalMsgFlowHashModeSetReq_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgFlowHashModeSetReq = {
	"HalMsgFlowHashModeSetReq",
	"HalMsgFlowHashModeSetReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgFlowHashModeSetReq_tags_1,
	sizeof(asn_DEF_HalMsgFlowHashModeSetReq_tags_1)
		/sizeof(asn_DEF_HalMsgFlowHashModeSetReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgFlowHashModeSetReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgFlowHashModeSetReq_tags_1)
		/sizeof(asn_DEF_HalMsgFlowHashModeSetReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgFlowHashModeSetReq_1,
	2,	/* Elements count */
	&asn_SPC_HalMsgFlowHashModeSetReq_specs_1	/* Additional specs */
};

