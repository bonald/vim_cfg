/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-QOS"
 * 	found in "../aclqos/hal_msg_qos.asn1"
 */

#include <asn_internal.h>

#include "HalMsgQosDeinitPortCfgReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgQosDeinitPortCfgReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, port),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"port"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, flag),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"flag"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, domain),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"domain"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, trust),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"trust"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, cos),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"cos"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, wdrrWeight),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wdrrWeight"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, wdrrWeightBmp),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wdrrWeightBmp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, queueClassBmp),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"queueClassBmp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, queueClassId),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"queueClassId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, portShape),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgQosPortShape,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"portShape"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, queueDrop),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HalMsgQosQueueDrop,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"queueDrop"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, queueDropBmp),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"queueDropBmp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgQosDeinitPortCfgReq, queueShapeBmp),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"queueShapeBmp"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgQosDeinitPortCfgReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* port at 369 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* flag at 370 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* domain at 371 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* trust at 372 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* cos at 373 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* wdrrWeight at 374 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* wdrrWeightBmp at 375 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* queueClassBmp at 376 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* queueClassId at 377 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* portShape at 378 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* queueDrop at 379 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* queueDropBmp at 380 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 } /* queueShapeBmp at 382 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgQosDeinitPortCfgReq_specs_1 = {
	sizeof(struct HalMsgQosDeinitPortCfgReq),
	offsetof(struct HalMsgQosDeinitPortCfgReq, _asn_ctx),
	asn_MAP_HalMsgQosDeinitPortCfgReq_tag2el_1,
	13,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgQosDeinitPortCfgReq = {
	"HalMsgQosDeinitPortCfgReq",
	"HalMsgQosDeinitPortCfgReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1,
	sizeof(asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1)
		/sizeof(asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1)
		/sizeof(asn_DEF_HalMsgQosDeinitPortCfgReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgQosDeinitPortCfgReq_1,
	13,	/* Elements count */
	&asn_SPC_HalMsgQosDeinitPortCfgReq_specs_1	/* Additional specs */
};

