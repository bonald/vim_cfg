/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HAL-MSG-MPLS"
 * 	found in "../mpls/hal_msg_mpls.asn1"
 */

#include <asn_internal.h>

#include "HalMsgMplsApsNhAddReq.h"

static asn_TYPE_member_t asn_MBR_HalMsgMplsApsNhAddReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, firstcreate),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"firstcreate"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, working),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"working"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, apsgroupid),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apsgroupid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, apsnhid),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apsnhid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, nhtype),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhtype"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, apsoffset),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"apsoffset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, spec),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"spec"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, nhExt),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"nhExt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, workingvalid),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"workingvalid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wnexthopid),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wnexthopid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, woffset),
		(ASN_TAG_CLASS_CONTEXT | (10 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"woffset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wnhType),
		(ASN_TAG_CLASS_CONTEXT | (11 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wnhType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wspec),
		(ASN_TAG_CLASS_CONTEXT | (12 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wspec"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wnhExt),
		(ASN_TAG_CLASS_CONTEXT | (13 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wnhExt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wgportid),
		(ASN_TAG_CLASS_CONTEXT | (14 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wgportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wvid),
		(ASN_TAG_CLASS_CONTEXT | (15 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wvid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, woifType),
		(ASN_TAG_CLASS_CONTEXT | (16 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"woifType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, woutVlanIsSvlan),
		(ASN_TAG_CLASS_CONTEXT | (17 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"woutVlanIsSvlan"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wmacAddr),
		(ASN_TAG_CLASS_CONTEXT | (18 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wmacAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wstatsptr),
		(ASN_TAG_CLASS_CONTEXT | (19 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wstatsptr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wuseTtlFromPacket),
		(ASN_TAG_CLASS_CONTEXT | (20 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wuseTtlFromPacket"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wfid),
		(ASN_TAG_CLASS_CONTEXT | (21 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wfid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wvplsPortType),
		(ASN_TAG_CLASS_CONTEXT | (22 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wvplsPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wvplsPort),
		(ASN_TAG_CLASS_CONTEXT | (23 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wvplsPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wmaxChipNum),
		(ASN_TAG_CLASS_CONTEXT | (24 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wmaxChipNum"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, walreadyChipBMP),
		(ASN_TAG_CLASS_CONTEXT | (25 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"walreadyChipBMP"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wlabelinfo),
		(ASN_TAG_CLASS_CONTEXT | (26 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wlabelinfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wl2vpnTagop),
		(ASN_TAG_CLASS_CONTEXT | (27 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wl2vpnTagop"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, wcwen),
		(ASN_TAG_CLASS_CONTEXT | (28 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"wcwen"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, protectingvalid),
		(ASN_TAG_CLASS_CONTEXT | (29 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"protectingvalid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pnexthopid),
		(ASN_TAG_CLASS_CONTEXT | (30 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pnexthopid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, poffset),
		(ASN_TAG_CLASS_CONTEXT | (31 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"poffset"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pnhType),
		(ASN_TAG_CLASS_CONTEXT | (32 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pnhType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pspec),
		(ASN_TAG_CLASS_CONTEXT | (33 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pspec"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pnhExt),
		(ASN_TAG_CLASS_CONTEXT | (34 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pnhExt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pgportid),
		(ASN_TAG_CLASS_CONTEXT | (35 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pgportid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pvid),
		(ASN_TAG_CLASS_CONTEXT | (36 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pvid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, poifType),
		(ASN_TAG_CLASS_CONTEXT | (37 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"poifType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, poutVlanIsSvlan),
		(ASN_TAG_CLASS_CONTEXT | (38 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"poutVlanIsSvlan"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pmacAddr),
		(ASN_TAG_CLASS_CONTEXT | (39 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pmacAddr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pstatsptr),
		(ASN_TAG_CLASS_CONTEXT | (40 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pstatsptr"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, puseTtlFromPacket),
		(ASN_TAG_CLASS_CONTEXT | (41 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"puseTtlFromPacket"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pfid),
		(ASN_TAG_CLASS_CONTEXT | (42 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pfid"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pvplsPortType),
		(ASN_TAG_CLASS_CONTEXT | (43 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pvplsPortType"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pvplsPort),
		(ASN_TAG_CLASS_CONTEXT | (44 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pvplsPort"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pmaxChipNum),
		(ASN_TAG_CLASS_CONTEXT | (45 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pmaxChipNum"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, palreadyChipBMP),
		(ASN_TAG_CLASS_CONTEXT | (46 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"palreadyChipBMP"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, plabelinfo),
		(ASN_TAG_CLASS_CONTEXT | (47 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PrintableString,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"plabelinfo"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pl2vpnTagop),
		(ASN_TAG_CLASS_CONTEXT | (48 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pl2vpnTagop"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct HalMsgMplsApsNhAddReq, pcwen),
		(ASN_TAG_CLASS_CONTEXT | (49 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"pcwen"
		},
};
static ber_tlv_tag_t asn_DEF_HalMsgMplsApsNhAddReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_HalMsgMplsApsNhAddReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* firstcreate at 42 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* working at 43 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* apsgroupid at 44 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* apsnhid at 45 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* nhtype at 46 */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* apsoffset at 47 */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* spec at 48 */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* nhExt at 49 */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* workingvalid at 50 */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 }, /* wnexthopid at 51 */
    { (ASN_TAG_CLASS_CONTEXT | (10 << 2)), 10, 0, 0 }, /* woffset at 52 */
    { (ASN_TAG_CLASS_CONTEXT | (11 << 2)), 11, 0, 0 }, /* wnhType at 53 */
    { (ASN_TAG_CLASS_CONTEXT | (12 << 2)), 12, 0, 0 }, /* wspec at 54 */
    { (ASN_TAG_CLASS_CONTEXT | (13 << 2)), 13, 0, 0 }, /* wnhExt at 55 */
    { (ASN_TAG_CLASS_CONTEXT | (14 << 2)), 14, 0, 0 }, /* wgportid at 57 */
    { (ASN_TAG_CLASS_CONTEXT | (15 << 2)), 15, 0, 0 }, /* wvid at 58 */
    { (ASN_TAG_CLASS_CONTEXT | (16 << 2)), 16, 0, 0 }, /* woifType at 59 */
    { (ASN_TAG_CLASS_CONTEXT | (17 << 2)), 17, 0, 0 }, /* woutVlanIsSvlan at 60 */
    { (ASN_TAG_CLASS_CONTEXT | (18 << 2)), 18, 0, 0 }, /* wmacAddr at 61 */
    { (ASN_TAG_CLASS_CONTEXT | (19 << 2)), 19, 0, 0 }, /* wstatsptr at 63 */
    { (ASN_TAG_CLASS_CONTEXT | (20 << 2)), 20, 0, 0 }, /* wuseTtlFromPacket at 64 */
    { (ASN_TAG_CLASS_CONTEXT | (21 << 2)), 21, 0, 0 }, /* wfid at 65 */
    { (ASN_TAG_CLASS_CONTEXT | (22 << 2)), 22, 0, 0 }, /* wvplsPortType at 66 */
    { (ASN_TAG_CLASS_CONTEXT | (23 << 2)), 23, 0, 0 }, /* wvplsPort at 67 */
    { (ASN_TAG_CLASS_CONTEXT | (24 << 2)), 24, 0, 0 }, /* wmaxChipNum at 68 */
    { (ASN_TAG_CLASS_CONTEXT | (25 << 2)), 25, 0, 0 }, /* walreadyChipBMP at 69 */
    { (ASN_TAG_CLASS_CONTEXT | (26 << 2)), 26, 0, 0 }, /* wlabelinfo at 70 */
    { (ASN_TAG_CLASS_CONTEXT | (27 << 2)), 27, 0, 0 }, /* wl2vpnTagop at 71 */
    { (ASN_TAG_CLASS_CONTEXT | (28 << 2)), 28, 0, 0 }, /* wcwen at 72 */
    { (ASN_TAG_CLASS_CONTEXT | (29 << 2)), 29, 0, 0 }, /* protectingvalid at 74 */
    { (ASN_TAG_CLASS_CONTEXT | (30 << 2)), 30, 0, 0 }, /* pnexthopid at 75 */
    { (ASN_TAG_CLASS_CONTEXT | (31 << 2)), 31, 0, 0 }, /* poffset at 76 */
    { (ASN_TAG_CLASS_CONTEXT | (32 << 2)), 32, 0, 0 }, /* pnhType at 77 */
    { (ASN_TAG_CLASS_CONTEXT | (33 << 2)), 33, 0, 0 }, /* pspec at 78 */
    { (ASN_TAG_CLASS_CONTEXT | (34 << 2)), 34, 0, 0 }, /* pnhExt at 79 */
    { (ASN_TAG_CLASS_CONTEXT | (35 << 2)), 35, 0, 0 }, /* pgportid at 81 */
    { (ASN_TAG_CLASS_CONTEXT | (36 << 2)), 36, 0, 0 }, /* pvid at 82 */
    { (ASN_TAG_CLASS_CONTEXT | (37 << 2)), 37, 0, 0 }, /* poifType at 83 */
    { (ASN_TAG_CLASS_CONTEXT | (38 << 2)), 38, 0, 0 }, /* poutVlanIsSvlan at 84 */
    { (ASN_TAG_CLASS_CONTEXT | (39 << 2)), 39, 0, 0 }, /* pmacAddr at 85 */
    { (ASN_TAG_CLASS_CONTEXT | (40 << 2)), 40, 0, 0 }, /* pstatsptr at 87 */
    { (ASN_TAG_CLASS_CONTEXT | (41 << 2)), 41, 0, 0 }, /* puseTtlFromPacket at 88 */
    { (ASN_TAG_CLASS_CONTEXT | (42 << 2)), 42, 0, 0 }, /* pfid at 89 */
    { (ASN_TAG_CLASS_CONTEXT | (43 << 2)), 43, 0, 0 }, /* pvplsPortType at 90 */
    { (ASN_TAG_CLASS_CONTEXT | (44 << 2)), 44, 0, 0 }, /* pvplsPort at 91 */
    { (ASN_TAG_CLASS_CONTEXT | (45 << 2)), 45, 0, 0 }, /* pmaxChipNum at 92 */
    { (ASN_TAG_CLASS_CONTEXT | (46 << 2)), 46, 0, 0 }, /* palreadyChipBMP at 93 */
    { (ASN_TAG_CLASS_CONTEXT | (47 << 2)), 47, 0, 0 }, /* plabelinfo at 94 */
    { (ASN_TAG_CLASS_CONTEXT | (48 << 2)), 48, 0, 0 }, /* pl2vpnTagop at 95 */
    { (ASN_TAG_CLASS_CONTEXT | (49 << 2)), 49, 0, 0 } /* pcwen at 97 */
};
static asn_SEQUENCE_specifics_t asn_SPC_HalMsgMplsApsNhAddReq_specs_1 = {
	sizeof(struct HalMsgMplsApsNhAddReq),
	offsetof(struct HalMsgMplsApsNhAddReq, _asn_ctx),
	asn_MAP_HalMsgMplsApsNhAddReq_tag2el_1,
	50,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_HalMsgMplsApsNhAddReq = {
	"HalMsgMplsApsNhAddReq",
	"HalMsgMplsApsNhAddReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_HalMsgMplsApsNhAddReq_tags_1,
	sizeof(asn_DEF_HalMsgMplsApsNhAddReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsApsNhAddReq_tags_1[0]), /* 1 */
	asn_DEF_HalMsgMplsApsNhAddReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_HalMsgMplsApsNhAddReq_tags_1)
		/sizeof(asn_DEF_HalMsgMplsApsNhAddReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_HalMsgMplsApsNhAddReq_1,
	50,	/* Elements count */
	&asn_SPC_HalMsgMplsApsNhAddReq_specs_1	/* Additional specs */
};
