/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#include <asn_internal.h>

#include "HalMsgQosInitPortCfgReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgQosInitPortCfgReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, port),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"port"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, domain),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"domain"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, trust),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"trust"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, cos),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, replaceDscp),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"replaceDscp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, replaceCos),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"replaceCos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, wdrrWeight),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wdrrWeight"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosInitPortCfgReq, drop),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgQosQueueDrop,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"drop"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgQosInitPortCfgReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgQosInitPortCfgReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* port at 340 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* domain at 341 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* trust at 342 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* cos at 343 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* replaceDscp at 344 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* replaceCos at 345 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* wdrrWeight at 346 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 } /* drop at 348 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgQosInitPortCfgReq_specs_1 = {
	sizeof(struct HalMsgQosInitPortCfgReq),
	offsetof(struct HalMsgQosInitPortCfgReq, _asn_ctx),
	asn_MAP_HalMsgQosInitPortCfgReq_tag2el_1,
	8,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgQosInitPortCfgReq = {
	"HalMsgQosInitPortCfgReq",
	"HalMsgQosInitPortCfgReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgQosInitPortCfgReq_tags_1,
	sizeof(asn_DEF_HalMsgQosInitPortCfgReq_tags_1)
		/sizeof(asn_DEF_HalMsgQosInitPortCfgReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgQosInitPortCfgReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgQosInitPortCfgReq_tags_1)
		/sizeof(asn_DEF_HalMsgQosInitPortCfgReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgQosInitPortCfgReq_1,
	8,	/* Elements count */
	&asn_SPC_HalMsgQosInitPortCfgReq_specs_1	/* Additional specs */
};
