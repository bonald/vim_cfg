
#ifndef __CONFIG_FILE_PARSER_H__
#define __CONFIG_FILE_PARSER_H__

enum{
	CONFIG_FEATURE_NULL = 0,
    CONFIG_FEATURE_TAP_TIMESTAMP,
    CONFIG_FEATURE_TAP_TRUNCTION,
    CONFIG_FEATURE_TAP_TRUNCTION_LENGTH_MAX,
    CONFIG_FEATURE_TAP_UNTAG,
    CONFIG_FEATURE_HYBRID_FORCE_BACK,
    CONFIG_FEATURE_HYBRID_MPLS,
    CONFIG_FEATURE_HYBRID_TUNNEL,
    CONFIG_FEATURE_HYBRID_ARP_EDIT,
    CONFIG_FEATURE_HYBRID_SET_QINQ,
    CONFIG_FEATURE_HYBRID_NEED_MPLS_PARSER_CLI,
    CONFIG_FEATURE_HYBRID_SOME_FIELD,
    CONFIG_FEATURE_ACL_VXLAN,
    CONFIG_FEATURE_ACL_EDIT_SRC_IP,
    CONFIG_FEATURE_ACL_STRIP_HEADER,
    CONFIG_FEATURE_PARSER_LINKAGG_INNER_HASH,
    CONFIG_FEATURE_SELECT_HASH_KEY_NOT_ONLY_IP,
    CONFIG_FEATURE_ACL_GRE,
    CONFIG_FEATURE_ACL_NVGRE,
    CONFIG_FEATURE_ACL_FLOW_MATCH,
    CONFIG_FEATURE_ACL_UDF,
    CONFIG_FEATURE_ACL_TCAM_RES_SHARE,
    CONFIG_FEATURE_HYBRID_MPLS_TP_OAM,
    CONFIG_FEATURE_HYBRID_VLAN_EDIT_BEFORE_GROUP,
    CONFIG_FEATURE_HYBRID_V4_V6_EXCHANGE,
    CONFIG_FEATURE_HYBRID_IPV6_EDIT,
    CONFIG_FEATURE_HYBRID_POP_ALL_MPLS,
    CONFIG_FEATURE_MAX
};

uint32
check_feature_support(uint32 feature_id);
int32	
config_get_int32(uint32 feature_id, int32 num);
#endif /* !__CONFIG_FILE_PARSER_H__ */