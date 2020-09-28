#ifndef _CTC_GOLDENGATE_DKIT_TBL_H_
#define _CTC_GOLDENGATE_DKIT_TBL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "sal.h"

struct ctc_dkits_tcam_key_inf_s
{
    tbls_id_t tblid;
    uint8     key_type;
};
typedef struct ctc_dkits_tcam_key_inf_s ctc_dkits_tcam_key_inf_t;

struct ctc_dkits_hash_tbl_info_s
{
    uint32       valid;
    uint32       hash_type;
    tbls_id_t    ad_tblid;
    fld_id_t     adidx_fldid;
};
typedef struct ctc_dkits_hash_tbl_info_s ctc_dkits_hash_tbl_info_t;

extern tbls_id_t usw_fib_hash_bridge_key_tbl[];
extern tbls_id_t usw_fib_hash_ipuc_ipv4_key_tbl[];
extern tbls_id_t usw_fib_hash_ipuc_ipv6_key_tbl[];
extern tbls_id_t usw_fib_hash_ipmc_ipv4_key_tbl[];
extern tbls_id_t usw_fib_hash_ipmc_ipv6_key_tbl[];
extern tbls_id_t usw_fib_hash_fcoe_key_tbl[];
extern tbls_id_t usw_fib_hash_trill_key_tbl[];
extern tbls_id_t usw_scl_hash_bridge_key_tbl[];
extern tbls_id_t usw_scl_hash_ipv4_key_tbl[];
extern tbls_id_t usw_scl_hash_ipv6_key_tbl[];
extern tbls_id_t usw_scl_hash_iptunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_nvgre_tunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_vxlan_tunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_mpls_tunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_pbb_tunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_trill_tunnel_key_tbl[];
extern tbls_id_t usw_scl_hash_wlan_key_tbl[];
extern tbls_id_t usw_ipfix_hash_bridge_key_tbl[];
extern tbls_id_t usw_ipfix_hash_l2l3_key_tbl[];
extern tbls_id_t usw_ipfix_hash_ipv4_key_tbl[];
extern tbls_id_t usw_ipfix_hash_ipv6_key_tbl[];
extern tbls_id_t usw_ipfix_hash_mpls_key_tbl[];
extern tbls_id_t usw_flow_hash_bridge_key_tbl[];
extern tbls_id_t usw_flow_hash_l2l3_key_tbl[];
extern tbls_id_t usw_flow_hash_ipv4_key_tbl[];
extern tbls_id_t usw_flow_hash_ipv6_key_tbl[];
extern tbls_id_t usw_flow_hash_mpls_key_tbl[];
extern tbls_id_t usw_xcoam_hash_scl_tbl[];
extern tbls_id_t usw_xcoam_hash_tunnel_tbl[];
extern tbls_id_t usw_xcoam_hash_oam_tbl[];
extern tbls_id_t usw_oam_tbl[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_l2l3_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_ipv4_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_ipv6_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_bridge_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_interface_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_fwd_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_acl_tcam_category_key_inf[];

extern tbls_id_t usw_acl_tcam_l2l3_ad_tbl[];
extern tbls_id_t usw_acl_tcam_ipv4_ad_tbl[];
extern tbls_id_t usw_acl_tcam_ipv6_ad_tbl[];
extern tbls_id_t usw_acl_tcam_bridge_ad_tbl[];
extern ctc_dkits_tcam_key_inf_t usw_scl_tcam_l2l3_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_scl_tcam_ipv4_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_scl_tcam_ipv6_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_scl_tcam_bridge_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_scl_tcam_userid_key_inf[];
extern tbls_id_t usw_scl_tcam_l2l3_ad_tbl[];
extern tbls_id_t usw_scl_tcam_ipv4_ad_tbl[];
extern tbls_id_t usw_scl_tcam_ipv6_ad_tbl[];
extern tbls_id_t usw_scl_tcam_bridge_ad_tbl[];
extern ctc_dkits_tcam_key_inf_t usw_ip_tcam_prefix_key_inf[];
extern tbls_id_t usw_ip_tcam_prefix_ad_tbl[];
extern ctc_dkits_tcam_key_inf_t usw_ip_tcam_nat_ipv4_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_ip_tcam_nat_ipv6_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_ip_tcam_pbr_ipv4_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_ip_tcam_pbr_ipv6_key_inf[];
extern tbls_id_t usw_ip_tcam_nat_ipv4_ad_tbl[];
extern tbls_id_t usw_ip_tcam_nat_ipv6_ad_tbl[];
extern tbls_id_t usw_ip_tcam_pbr_ipv4_ad_tbl[];
extern tbls_id_t usw_ip_tcam_pbr_ipv6_ad_tbl[];
extern ctc_dkits_tcam_key_inf_t usw_static_cid_tcam_key_inf[];
extern ctc_dkits_tcam_key_inf_t usw_static_qmgr_tcam_key_inf[];
extern tbls_id_t usw_nh_fwd_tbl[];
extern tbls_id_t usw_nh_met_tbl[];
extern tbls_id_t usw_nh_nexthop_tbl[];
extern tbls_id_t usw_nh_edit_tbl[];
extern tbls_id_t usw_auto_genernate_tbl[];
extern tbls_id_t usw_exclude_tbl[];
extern tbls_id_t dt2_serdes_tbl[];
extern tbls_id_t tm_serdes_tbl[];

#ifdef __cplusplus
}
#endif

#endif

