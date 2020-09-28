/*
 * Generated by asn1c-0.9.20 (http://lionet.info/asn1c)
 * From ASN.1 module "HSRV2AGT-MSG"
 * 	found in "../hsrv2AgtMsg.asn1"
 */

#include <asn_internal.h>

#include "Hsrv2AgtMsgInitSeqReq.h"

static int
memb_initseq_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_consume_bytes_f *app_errlog, void *app_key) {
	long value;
	
	if(!sptr) {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 2147483647)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_ERRLOG(app_errlog, app_key,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_TYPE_member_t asn_MBR_Hsrv2AgtMsgInitSeqReq_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Hsrv2AgtMsgInitSeqReq, initseq),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		memb_initseq_constraint_1,
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"initseq"
		},
};
static ber_tlv_tag_t asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_Hsrv2AgtMsgInitSeqReq_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* initseq at 1291 */
};
static asn_SEQUENCE_specifics_t asn_SPC_Hsrv2AgtMsgInitSeqReq_specs_1 = {
	sizeof(struct Hsrv2AgtMsgInitSeqReq),
	offsetof(struct Hsrv2AgtMsgInitSeqReq, _asn_ctx),
	asn_MAP_Hsrv2AgtMsgInitSeqReq_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_Hsrv2AgtMsgInitSeqReq = {
	"Hsrv2AgtMsgInitSeqReq",
	"Hsrv2AgtMsgInitSeqReq",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0,	/* No PER decoder, -gen-PER to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1,
	sizeof(asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1)
		/sizeof(asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1[0]), /* 1 */
	asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1,	/* Same as above */
	sizeof(asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1)
		/sizeof(asn_DEF_Hsrv2AgtMsgInitSeqReq_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_Hsrv2AgtMsgInitSeqReq_1,
	1,	/* Elements count */
	&asn_SPC_Hsrv2AgtMsgInitSeqReq_specs_1	/* Additional specs */
};
