/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-COMM-IF"
 * 	found in "../intf/hal_msg_if.asn1"
 */

#include <asn_internal.h>

#include "HalMsgCOMMIfSetPortIsolateGroupReq.h"

static asn_TYPE_member_t asn_MBR_isolatePortList_2[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		""
		},
};
static ber_tlv_tag_t asn_DEF_isolatePortList_tags_2[] = {
	(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_isolatePortList_specs_2 = {
	sizeof(struct isolatePortList),
	offsetof(struct isolatePortList, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_isolatePortList_2 = {
	"isolatePortList",
	"isolatePortList",
	SEQUENCE_OF_free,
	SEQUENCE_OF_print,
	SEQUENCE_OF_constraint,
	SEQUENCE_OF_decode_ber,
	SEQUENCE_OF_encode_der,
	SEQUENCE_OF_decode_xer,
	SEQUENCE_OF_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_isolatePortList_tags_2,
	sizeof(asn_DEF_isolatePortList_tags_2)
		/sizeof(asn_DEF_isolatePortList_tags_2[0]) - 1, /* 1 */
	asn_DEF_isolatePortList_tags_2,	/* Same as above */
	sizeof(asn_DEF_isolatePortList_tags_2)
		/sizeof(asn_DEF_isolatePortList_tags_2[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_isolatePortList_2,
	1,	/* Single element */
	&asn_SPC_isolatePortList_specs_2	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_HalMsgCOMMIfSetPortIsolateGroupReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgCOMMIfSetPortIsolateGroupReq, isolatePortList),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_isolatePortList_2,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"isolatePortList"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgCOMMIfSetPortIsolateGroupReq, groupID),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"groupID"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgCOMMIfSetPortIsolateGroupReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* isolatePortList at 329 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* groupID at 331 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgCOMMIfSetPortIsolateGroupReq_specs_1 = {
	sizeof(struct HalMsgCOMMIfSetPortIsolateGroupReq),
	offsetof(struct HalMsgCOMMIfSetPortIsolateGroupReq, _asn_ctx),
	asn_MAP_HalMsgCOMMIfSetPortIsolateGroupReq_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq = {
	"HalMsgCOMMIfSetPortIsolateGroupReq",
	"HalMsgCOMMIfSetPortIsolateGroupReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1,
	sizeof(asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1)
		/sizeof(asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1)
		/sizeof(asn_DEF_HalMsgCOMMIfSetPortIsolateGroupReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgCOMMIfSetPortIsolateGroupReq_1,
	2,	/* Elements count */
	&asn_SPC_HalMsgCOMMIfSetPortIsolateGroupReq_specs_1	/* Additional specs */
};
