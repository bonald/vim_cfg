/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "LCM-DEBUG"
 * 	found in "../lcm_debug.asn1"
 */

#include <asn_internal.h>

#include "XGmacMdioCmd.h"

static asn_TYPE_member_t asn_MBR_XGmacMdioCmd_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct XGmacMdioCmd, xgmacMdiocmdSt),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"xgmacMdiocmdSt"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct XGmacMdioCmd, xgmacMdiocmdOp),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"xgmacMdiocmdOp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct XGmacMdioCmd, xgmacMdiocmdPrtad),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"xgmacMdiocmdPrtad"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct XGmacMdioCmd, xgmacMdiocmdDevad),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"xgmacMdiocmdDevad"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct XGmacMdioCmd, xgmacMdiocmdData),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"xgmacMdiocmdData"
		},
};
static ber_tlv_tag_t asn_DEF_XGmacMdioCmd_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_XGmacMdioCmd_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* xgmacMdiocmdSt at 861 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* xgmacMdiocmdOp at 862 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* xgmacMdiocmdPrtad at 863 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* xgmacMdiocmdDevad at 864 */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 } /* xgmacMdiocmdData at 866 */
};
static asn_SEQUENCE_specifics_t asn_SPC_XGmacMdioCmd_specs_1 = {
	sizeof(struct XGmacMdioCmd),
	offsetof(struct XGmacMdioCmd, _asn_ctx),
	asn_MAP_XGmacMdioCmd_tag2el_1,
	5,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_XGmacMdioCmd = {
	"XGmacMdioCmd",
	"XGmacMdioCmd",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_XGmacMdioCmd_tags_1,
	sizeof(asn_DEF_XGmacMdioCmd_tags_1)
		/sizeof(asn_DEF_XGmacMdioCmd_tags_1[0]), /* 1 */
	asn_DEF_XGmacMdioCmd_tags_1,	/* Same as above */
	sizeof(asn_DEF_XGmacMdioCmd_tags_1)
		/sizeof(asn_DEF_XGmacMdioCmd_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_XGmacMdioCmd_1,
	5,	/* Elements count */
	&asn_SPC_XGmacMdioCmd_specs_1	/* Additional specs */
};

