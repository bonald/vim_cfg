/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM"
 * 	found in "../lcm.asn1"
 */

#include <asn_internal.h>

#include "LcChsmPortGetStatsAck.h"

static asn_TYPE_member_t asn_MBR_LcChsmPortGetStatsAck_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, slot),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SlotNo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"slot"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, port),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PortNo,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"port"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxPackets),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxPackets"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, txPackets),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"txPackets"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxBytes),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxBytes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, txBytes),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"txBytes"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxJabber),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxJabber"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxFragments),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxFragments"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxFrame),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxFrame"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, rxCrc),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"rxCrc"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, txCollisions),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"txCollisions"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, txCarrier),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"txCarrier"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct LcChsmPortGetStatsAck, txCrc),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"txCrc"
		},
};
static ber_tlv_tag_t asn_DEF_LcChsmPortGetStatsAck_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_LcChsmPortGetStatsAck_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* slot at 798 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* port at 799 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* rxPackets at 800 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* txPackets at 801 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* rxBytes at 802 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* txBytes at 803 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* rxJabber at 804 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* rxFragments at 805 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* rxFrame at 806 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* rxCrc at 807 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* txCollisions at 808 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* txCarrier at 809 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 } /* txCrc at 811 */
};
static asn_SEQUENCE_specifics_t asn_SPC_LcChsmPortGetStatsAck_specs_1 = {
	sizeof(struct LcChsmPortGetStatsAck),
	offsetof(struct LcChsmPortGetStatsAck, _asn_ctx),
	asn_MAP_LcChsmPortGetStatsAck_tag2el_1,
	13,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_LcChsmPortGetStatsAck = {
	"LcChsmPortGetStatsAck",
	"LcChsmPortGetStatsAck",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_LcChsmPortGetStatsAck_tags_1,
	sizeof(asn_DEF_LcChsmPortGetStatsAck_tags_1)
		/sizeof(asn_DEF_LcChsmPortGetStatsAck_tags_1[0]), /* 1 */
	asn_DEF_LcChsmPortGetStatsAck_tags_1,	/* Same as above */
	sizeof(asn_DEF_LcChsmPortGetStatsAck_tags_1)
		/sizeof(asn_DEF_LcChsmPortGetStatsAck_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_LcChsmPortGetStatsAck_1,
	13,	/* Elements count */
	&asn_SPC_LcChsmPortGetStatsAck_specs_1	/* Additional specs */
};

