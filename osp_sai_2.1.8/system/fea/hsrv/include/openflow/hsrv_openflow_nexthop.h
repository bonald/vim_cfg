#ifndef __HSRV_OPENFLOW_NEXTHOP_H__
#define __HSRV_OPENFLOW_NEXTHOP_H__

#define HSRV_OF_NH_RESV_NHID_FOR_DROP  1 /* HSRV_NH_RESOLVED_NHID_FOR_DROP */
#define HSRV_OF_NH_INVAID_INDEX        -1

enum hsrv_of_nh_type_e
{
    HSRV_OF_NH_TYPE_NH_NONE,
    HSRV_OF_NH_TYPE_NH_MCAST,
    HSRV_OF_NH_TYPE_NH_ECMP,
    HSRV_OF_NH_TYPE_NH_FLEX,
    HSRV_OF_NH_TYPE_NH_FLEX_CPU,
    HSRV_OF_NH_TYPE_NH_IPUC,
    HSRV_OF_NH_TYPE_NH_IP_TUNNEL,
    HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID,
    HSRV_OF_NH_TYPE_NH_MPLS,
    HSRV_OF_NH_TYPE_NH_XLATE,
    HSRV_OF_NH_TYPE_NH_ILOOP,
    HSRV_OF_NH_TYPE_NH_DROP,
    HSRV_OF_NH_TYPE_MAX /**< Type Max */
};
typedef enum hsrv_of_nh_type_e hsrv_of_nh_type_t;

#define HSRV_OF_NH_TYPE_STR(type)                      \
    (HSRV_OF_NH_TYPE_NH_NONE == (type)      ? "NONE" : \
     HSRV_OF_NH_TYPE_NH_MCAST == (type)      ? "MCAST" : \
     HSRV_OF_NH_TYPE_NH_ECMP == (type)      ? "ECMP" : \
     HSRV_OF_NH_TYPE_NH_IPUC == (type)      ? "IPUC" : \
     HSRV_OF_NH_TYPE_NH_IP_TUNNEL == (type)      ? "IP_TUNNEL" : \
     HSRV_OF_NH_TYPE_NH_IP_TUNNEL_WITH_TUN_ID == (type)      ? "IP_TUNNEL_WITH_TUN_ID" : \
     HSRV_OF_NH_TYPE_NH_MPLS == (type)      ? "MPLS" : \
     HSRV_OF_NH_TYPE_NH_FLEX == (type)      ? "FLEX" : \
     HSRV_OF_NH_TYPE_NH_FLEX_CPU == (type) ? "FLEX_CPU" : \
     HSRV_OF_NH_TYPE_NH_DROP == (type) ? "RESV_DROP" : \
     "UNKNOWN")

/* hsrv openflow memory nexthop related macros */
#define HSRV_OPENFLOW_NH_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_NH, size)
#define HSRV_OPENFLOW_NH_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_NH, size)
#define HSRV_OPENFLOW_NH_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_NH, ptr);  \
    ptr = NULL; \
}while(0)

struct hsrv_of_nh_offset_s
{
    uint32 nhid;
    /*for reserved nh id, offset and nh_type will not be used*/
    uint32 offset;
    hsrv_of_nh_type_t nh_type;
        
    bool port_check_discard; /* If set 1, discard the packet if it's destination port is same with ingress port */
    bool redirect_to_ingress_port; /* If set 1, redirect the packet to the ingress port
                                      and the port_check_discard flag is nonsens in this case. */
    bool stats_en;
    bool is_resvd;
    uint32 stats_id;
};
typedef struct hsrv_of_nh_offset_s hsrv_of_nh_offset_t;

struct hsrv_of_nh_info_s
{
    hsrv_of_nh_offset_t nh_offset;
    uint16              gport;
    uint16              rsv;
    uint32              ifindex;
    uint32              tid;
    bool                to_all;          /**< member port of 'action=ALL' */
    void*               share_info;      /**< hsrv_ofact_flex_nh_info_t */
                                         /**< hsrv_ofact_mpls_nh_info_t */
                                         /**< hsrv_ofact_ipuc_nh_info_t */
                                         /**< hsrv_ofact_ip_tunnel_nh_info_t */
                                         /**< hsrv_ofact_xlate_nh_info_t */
};
typedef struct hsrv_of_nh_info_s hsrv_of_nh_info_t;

struct hsrv_openflow_nexthop_info_s
{
    bool use_mcast;
    bool use_ecmp;
    bool use_dynamic_tunnel;
    
    hsrv_of_nh_info_t main_nh;

    uint32 member_count;
    hsrv_of_nh_info_t member_nh[GLB_OPENFLOW_MAX_OUTPUT_PER_FLOW];
    
};
typedef struct hsrv_openflow_nexthop_info_s hsrv_openflow_nexthop_info_t;

struct hsrv_openflow_nexthop_mcast_param_s
{
    bool stats_en;
    bool no_fwd;
    bool is_resvd;
};
typedef struct hsrv_openflow_nexthop_mcast_param_s hsrv_ofnh_mcast_param_t;

struct hsrv_openflow_action_combo_s
{
    /* flags */
    uint32 flag[GLB_OFPACT_FLD_FLAG_NUM]; /**< bitmap of glb_openflow_action_field_t */

    /* output */
    uint16 output_gport;
    uint16 output_linkagg_gport;

    /*added by hansf for v4/v6 transform*/
    uint8   flow_tunnel_type;   /* glb_openflow_flow_tnl_type_e */
    uint8   payload_packet_type; /* glb_openflow_flow_payload_packet_type_e */
    
    /* others */
    uint16 is_resvd_nh; /**< Whether this nexthop is created during system startup */
    uint16 chip_flags;

    uint16 queue_id;
    uint32 statsid;
    uint32 tunnel_id;
    uint32 meter_id;

    /* mac fields */
    mac_addr_t  mac_da;
    mac_addr_t  mac_sa;
    uint16 vlan_id;
    uint16 cvlan_id;
    uint8  vlan_pcp;
    uint8  cvlan_pcp;
    uint16 stag_tpid;
    uint16 ctag_tpid;
    uint16 eth_type;

    /* layer2 fields */
    union
    {
        struct 
        {
            /* same with ipv4 & ipv6*/
            uint8  ecn;
            uint8  dscp;
            uint8  ttl;
            uint8  proto;
        }ip;
        
        struct 
        {
            uint8  ecn;
            uint8  dscp;
            uint8  ttl;
            uint8  proto;

            ip_addr_t ipsa;
            ip_addr_t ipda;
            
        }ipv4;

        struct 
        {
            uint8  ecn;
            uint8  dscp;
            uint8  ttl;
            uint8  proto;

            ipv6_addr_t ipsa;
            ipv6_addr_t ipda;

            uint32 label;
        }ipv6;
        
        struct 
        {
            uint16 arp_op;
            uint16 resv;
            uint32 arp_spa;
            uint32 arp_tpa;

            mac_addr_t  arp_sha;
            mac_addr_t  arp_tha;
        }arp;
    }l3;
    
    /* layer4 fields */
    union 
    {
        struct
        {
            uint16 src_port;
            uint16 dst_port;
        }l4port;    /* ipv4/ipv6, tcp/udp/sctp */

        struct
        {
            uint8 type;
            uint8 code;
        }icmp;      /* ipv4/ipv6 */
    }l4;
};
typedef struct hsrv_openflow_action_combo_s hsrv_openflow_action_combo_t;

struct hsrv_openflow_action_flex_nh_info_s
{
    /* param */
    hsrv_openflow_action_combo_t combo;
    
    /* Private data */
    hsrv_of_nh_offset_t nh_offset;      /**< Nexthop offset */
    
    uint32 ref_cnt;             /**< Reference count */
};
typedef struct hsrv_openflow_action_flex_nh_info_s hsrv_ofact_flex_nh_info_t;

/* Added by weizj for tunnel */
struct hsrv_openflow_ip_nh_tunnel_info_s
{
    uint16 gport;           /**< Output gport */
    uint16 vlan_id;         /**< Vid */
    uint8  tunnel_type;     /**< Ip tunnel type (glb_openflow_tunnel_port_type_t) */
    uint8  ttl;             /**< TTL value */
    uint16 fid;             /**< Fid */

    uint8  dscp_select;     /**< Dscp select mode */
    uint8  dscp_or_tos;     /**< Dscp value for Ip version 4 , tos for Ip version 6 */
    uint16 mtu_size;        /**< MTU size */

    uint32 flow_label;      /**< Ip version 6 flow label */
    uint8  flow_label_mode; /**< Ip version 6 flow label mode,refer to ctc_nh_flow_label_mode_t */
    uint16 l4_dst_port;     /**< Layer4 destination port used by NAPT */

    uint8 strip_svlan;      /**< Strip svlan tag before encapsulation */
    uint8 strip_cvlan;      /**< Strip cvlan tag before encapsulation */

    uint8 mac_da[MAC_ADDR_LEN];     /**< Mac da */
    uint8 mac_sa[MAC_ADDR_LEN];     /**< Mac sa */
    uint32 tunnel_ifindex;          /**< Tunnel port ifindex */

    uint8 inner_mac_da[MAC_ADDR_LEN];     /**< Mac da */
    uint8 inner_mac_sa[MAC_ADDR_LEN];     /**< Mac sa */

    union
    {
        ip_addr_t   ipv4;
        ipv6_addr_t ipv6;
    } ip_sa;                    /**< New ip sa address in ip header*/

    union
    {
        ip_addr_t   ipv4;
        ipv6_addr_t ipv6;
    } ip_da;                    /**< New ip da address in ip header*/

    struct
    {
        uint32 tunnel_key;      /**< Tunnel Key */ 
        uint16 protocol_type;   /**< Tunnel header's Protocol Type field */
        uint8  rsv[2];
    } inner;
};
typedef struct hsrv_openflow_ip_nh_tunnel_info_s hsrv_openflow_ip_nh_tunnel_info_t;
/* End by weizj for tunnel */

struct hsrv_openflow_nexthop_flex_param_s
{
    hsrv_openflow_action_combo_t* p_action_combo;
};
typedef struct hsrv_openflow_nexthop_flex_param_s hsrv_ofnh_flex_param_t;

/* Added by weizj for tunnel */
struct hsrv_openflow_nexthop_tunnel_param_s
{
    hsrv_openflow_action_combo_t* p_action_combo;
    hsrv_openflow_ip_nh_tunnel_info_t tnl;  
};
typedef struct hsrv_openflow_nexthop_tunnel_param_s hsrv_ofnh_tunnel_param_t;
/* End by weizj for tunnel */

struct hsrv_openflow_nexthop_cpu_flex_param_s
{
    uint32 flow_id;
    bool   need_dsfwd;
    uint8  table_id;
    bool   is_local; /* Added by weizj for inband */
};
typedef struct hsrv_openflow_nexthop_cpu_flex_param_s hsrv_ofnh_cpu_flex_param_t;

/* for cubro mpls. */
struct hsrv_openflow_nexthop_mpls_param_s
{
    glb_openflow_mpls_nexthop_type_t type;
    uint16 gport;  /* gport is shared attribute. */
    union 
    {
        struct 
        {
            uint8  nexthop_mac[MAC_ADDR_LEN];  
            uint16  outer_vlan_vid;
            glb_openflow_mpls_nh_label_param_t tunnel_label;
            glb_openflow_mpls_nh_label_param_t pw_label;
            glb_openflow_mpls_nexthop_tag_op_t inner_svlan_tag_op;
            uint16 inner_svlan_id;
        } push_l2vpn;
        /* modified by yaom for openflow MPLS L2VPN 20160214 */
        struct 
        {
            uint8  nexthop_mac[MAC_ADDR_LEN];  
            uint16  outer_vlan_vid;
            glb_openflow_mpls_nh_label_param_t tunnel_label;
        } push_l3vpn;
        struct 
        {
            uint8  nexthop_mac[MAC_ADDR_LEN];  
            uint16  outer_vlan_vid;
            uint8 pw_label_valid;
            glb_openflow_mpls_nh_label_param_t pw_label;
            glb_openflow_mpls_nh_label_param_t tunnel_label;
        } swap_mpls;       
        struct 
        {
            uint8  nexthop_mac[MAC_ADDR_LEN];  
            uint16  outer_vlan_vid;
        } pop_php;       
    } u;
};
typedef struct hsrv_openflow_nexthop_mpls_param_s hsrv_ofnh_mpls_param_t;

/* modified by yaom for openflow MPLS L2VPN 20160214 */
struct hsrv_openflow_nexthop_ipuc_param_s
{
    uint16 gport;
    uint8  nexthop_mac[MAC_ADDR_LEN]; 
    uint16 vlan_vid;    
    uint8 rewrite_ether_type;
};
typedef struct hsrv_openflow_nexthop_ipuc_param_s hsrv_ofnh_ipuc_param_t;

struct hsrv_openflow_nexthop_xlate_param_s
{
    uint16 gport;
    glb_openflow_mpls_nexthop_tag_op_t inner_svlan_tag_op;
    uint16 inner_svlan_id;
};
typedef struct hsrv_openflow_nexthop_xlate_param_s hsrv_ofnh_xlate_param_t;

enum hsrv_openflow_nexthop_mcast_member_op_type_e
{
    HSRV_OFNH_MCAST_OP_ADD_MEMBER,
    HSRV_OFNH_MCAST_OP_DEL_MEMBER,
};
typedef enum hsrv_openflow_nexthop_mcast_member_op_type_e hsrv_ofnh_mcast_member_op_type_t;

struct hsrv_openflow_nexthop_mcast_member_param_s
{
    uint32 group_nhid;
    uint8  op_type;         /* hsrv_ofnh_mcast_member_op_type_t */
    
    uint16 mem_gport;
    uint32 mem_nhid;
    bool   port_check_discard;
    bool   redirect_to_ingress_port; /* When this flag is set, port_check_discard is nonsense. */
};
typedef struct hsrv_openflow_nexthop_mcast_member_param_s hsrv_ofnh_mcast_member_param_t;

struct hsrv_openflow_action_ip_tunnel_nh_info_s
{
    /* param */
    /* Added by weizj for tunnel */
    hsrv_openflow_ip_nh_tunnel_info_t param;

    /* Private data */
    hsrv_of_nh_offset_t nh_offset;      /**< Nexthop offset */

    uint32 ref_cnt;             /**< Reference count */
};
typedef struct hsrv_openflow_action_ip_tunnel_nh_info_s hsrv_ofact_ip_tunnel_nh_info_t;

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-04 */
struct hsrv_openflow_pw_aps_nh_info_s
{
    uint32      aps_id;
    uint32      tunnel_id;
    uint32      ptunnel_id;
    uint32      pw_label;
    uint32      ppw_label;
    uint32      gport;
    uint32      pgport;
    uint32      lsp_aps_group_id;
    uint32      plsp_aps_group_id;
    /* modified by yangl for bug 53718 to support mpls tpoam, 2019-10-24 */
    uint16      mpls_ttl0;
    uint16      mpls_tc0;
    uint16      mpls_tc0_is_set;
    uint16      mpls_ttl1;
    uint16      mpls_tc1;
    uint16      mpls_tc1_is_set;
    /*yangl end*/
    
};
typedef struct hsrv_openflow_pw_aps_nh_info_s hsrv_openflow_pw_aps_nh_info_t;

struct hsrv_openflow_lsp_aps_nh_s
{
    uint32                               pw_label;
    uint32                               nhid;
    uint32                               offset;  
    uint32                               mcastnhid;
    uint32                               mcastoffset;
    uint32                               count;
};
typedef struct hsrv_openflow_lsp_aps_nh_s hsrv_openflow_lsp_aps_nh_t;
/* liwh end */

int32
hsrv_openflow_nexthop_alloc_mcast_group(hsrv_ofnh_mcast_param_t* p_param, hsrv_of_nh_offset_t* p_group_nh);

int32
hsrv_openflow_nexthop_alloc_ecmp_group(hsrv_ofnh_mcast_param_t* p_param, hsrv_of_nh_offset_t* p_group_nh);

int32
hsrv_openflow_nexthop_alloc_flex_nh(hsrv_ofnh_flex_param_t* p_param, hsrv_of_nh_info_t* p_mem_info);

int32
hsrv_openflow_nexthop_alloc_cpu_flex_nh(hsrv_ofnh_cpu_flex_param_t* p_param, hsrv_of_nh_info_t* p_info);

/* Added by weizj for tunnel */
int32
hsrv_openflow_nexthop_alloc_ip_tunnel_nh(hsrv_openflow_ip_nh_tunnel_info_t* p_nh_param, hsrv_of_nh_info_t* p_mem_info);

int32
hsrv_openflow_nexthop_release_ip_tunnel_nh(hsrv_of_nh_info_t* p_nh_info);
/* End by weizj for tunnel */

int32
hsrv_openflow_nexthop_alloc_port_flex_nh(uint16 gport, hsrv_of_nh_info_t* p_info);

/* for cubro mpls. */
int32
hsrv_openflow_nexthop_alloc_mpls_nh(hsrv_ofnh_mpls_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info);

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-03 */
int32
hsrv_openflow_nexthop_get_tunnelid(hsrv_of_nh_info_t* p_mem_info, uint32* p_tunnel_id);
/* liwh end */

int32
hsrv_openflow_nexthop_release_mpls_nh(hsrv_of_nh_info_t* p_nh_info);
int32
hsrv_openflow_nexthop_alloc_ipuc_nh(hsrv_ofnh_ipuc_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info);

int32
hsrv_openflow_nexthop_alloc_xlate_nh(hsrv_ofnh_xlate_param_t *p_nh_param, hsrv_of_nh_info_t* p_mem_info);

/* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-19 */
int32 
hsrv_openflow_nexthop_alloc_tpoam_nh(uint16 out_gport, glb_openflow_tpoam_t* flow, uint32* nhid, uint32* offset);

int32 
hsrv_openflow_nexthop_release_tpoam(uint32     oam_type, uint32 nhid, uint32 offset);

int32
hsrv_openflow_alloc_mpls_tunnelid(uint32* p_tunnelId);

int32
hsrv_openflow_add_lsp_aps_tunnel(glb_openflow_tpoam_bucket_t*   p_tpoam_bucket_w, glb_openflow_tpoam_bucket_t*   p_tpoam_bucket_p,
    uint32* p_aps_group_offset, uint32* p_tunnel_id);
int32
hsrv_openflow_delete_lsp_aps_tunnel(uint32 aps_group_offset, uint32 tunnel_id);

int32
hsrv_openflow_add_pw_tunnel(glb_openflow_tpoam_bucket_t*   p_tpoam_bucket, uint32* p_tunnel_id);

int32 
hsrv_openflow_nexthop_alloc_lsp_aps_nh(uint32 tunnel_id, uint32 outlabel_pw, uint32* nhid, uint32* offset, uint32* mcastnhid, uint32* mcastoffset, ctclib_hash_t*   lsp_aps_nh_hash);

int32 
hsrv_openflow_nexthop_alloc_pw_aps_nh(hsrv_openflow_pw_aps_nh_info_t* p_nh_info, uint32* nhid, uint32* offset, uint32* mcastnhid, uint32* mcastoffset);

int32 
hsrv_openflow_nexthop_release_pw_aps_nh(uint32  aps_group_id, uint32 nhid, uint32 offset, uint32 mcastnhid, uint32 mcastoffset);

int32 
hsrv_openflow_nexthop_release_aps(uint32 nhid, uint32 mcastnhid);

int32
hsrv_openflow_nexthop_release_lsp_aps_nh(ctclib_hash_t*  lsp_aps_nh_hash, uint32  pw_label);

int32
hsrv_openflow_nexthop_remove_mpls_tunnel(uint32 tunnel_id);

int32
hsrv_openflow_nexthop_create_lsp_aps_tbl(ctclib_hash_t**   lsp_aps_nh_hash);
/* liwh end */

int32
hsrv_openflow_nexthop_add_mcast_member(hsrv_ofnh_mcast_member_param_t* p_param);

int32
hsrv_openflow_nexthop_add_ecmp_member(uint32 group_nhid, uint32 stats_id, uint32 member_count, uint32* nhid_array, bool first);

int32
hsrv_openflow_nexthop_remove_mcast_member(hsrv_ofnh_mcast_member_param_t* p_param);

int32
hsrv_openflow_nexthop_release_nh_info(hsrv_of_nh_info_t *p_nh_info);

int32
hsrv_openflow_nexthop_release_nexthop_info(hsrv_openflow_nexthop_info_t* p_nh_info);

int32
hsrv_openflow_nexthop_release_mcast_group(hsrv_of_nh_offset_t* p_group_nh);

int32
hsrv_openflow_nexthop_release_ecmp_group(hsrv_of_nh_offset_t* p_group_nh);

int32
hsrv_openflow_nexthop_remove_ecmp_member(uint32 group_nhid, uint32 member_count, uint32* nhid_array);

#if 0 /* Deleted by weizj for tunnel bug 33724 */
int32
hsrv_openflow_nexthop_get_output_all_nhid(hsrv_of_nh_offset_t* p_nh_offset);
#endif

int32
hsrv_openflow_nexthop_offset_alloc(hsrv_of_nh_offset_t* p_nh_offset);

int32
hsrv_openflow_nexthop_offset_release(hsrv_of_nh_offset_t* p_nh_offset);

int32
hsrv_openflow_show_nexthop_brief(FILE *fp);

int32
hsrv_openflow_show_nexthop_opf(FILE *fp, char* type);

int32
hsrv_openflow_show_nexthop_hash_type(FILE *fp, char* type);

int32
hsrv_openflow_show_nexthop_hash_ptr(FILE *fp, uint32 hash_ptr);

int32
hsrv_openflow_nexthop_start(void);

int32
hsrv_openflow_nexthop_init(hsrv_system_spec_info_t* p_system_info);

#endif /* !__HSRV_OPENFLOW_NEXTHOP_H__ */
