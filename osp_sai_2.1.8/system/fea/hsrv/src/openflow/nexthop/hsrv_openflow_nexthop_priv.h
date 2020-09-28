#ifndef __HSRV_OPENFLOW_NEXTHOP_PRIV_H__
#define __HSRV_OPENFLOW_NEXTHOP_PRIV_H__

struct hsrv_openflow_action_ipuc_nh_info_s
{
    /* param */
    hsrv_ofnh_ipuc_param_t param; /* modified by yaom for openflow MPLS L2VPN 20160214 */

    /* Private data */
    hsrv_of_nh_offset_t nh_offset;      /**< Nexthop offset */
    
    uint32 ref_cnt;             /**< Reference count */
};
typedef struct hsrv_openflow_action_ipuc_nh_info_s hsrv_ofact_ipuc_nh_info_t;

struct hsrv_openflow_action_xlate_nh_info_s
{
    /* param */
    hsrv_ofnh_xlate_param_t param; /* modified by yaom for openflow MPLS L2VPN 20160214 */

    /* Private data */
    hsrv_of_nh_offset_t nh_offset;      /**< Nexthop offset */
    
    uint32 ref_cnt;             /**< Reference count */
};
typedef struct hsrv_openflow_action_xlate_nh_info_s hsrv_ofact_xlate_nh_info_t;

/* for cubro mpls. */
struct hsrv_openflow_mpls_tunnel_s
{
    /* param */
    glb_openflow_mpls_tunnel_param_t tunnel_param;
    
    /* Private data */
    uint32 tunnel_id;                /**< tunnelid */
    uint32 ref_cnt;                 /**< Reference count */
    uint32 nh_id;                     /* no label will be added for this nexthop id */
    uint32 offset;
    void* p_mpls_nh_info;
};
typedef struct hsrv_openflow_mpls_tunnel_s hsrv_openflow_mpls_tunnel_t;

struct hsrv_openflow_action_mpls_nh_info_s
{
    /* param */
    hsrv_ofnh_mpls_param_t param;
    
    /* Private data */
    hsrv_of_nh_offset_t nh_offset;      /**< Nexthop offset */
    hsrv_openflow_mpls_tunnel_t *p_mpls_tunnel;

    uint32 ref_cnt;             /**< Reference count */
};
typedef struct hsrv_openflow_action_mpls_nh_info_s hsrv_ofact_mpls_nh_info_t;

enum hsrv_openflow_nexthop_opf_type_e
{
    HSRV_OPENFLOW_NEXTHOP_OPF_NONE,
    HSRV_OPENFLOW_NEXTHOP_OPF_NHID,
    HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET,
    HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET,
    HSRV_OPENFLOW_NEXTHOP_OPF_MPLS_TUNNELID,
    HSRV_OPENFLOW_NEXTHOP_OPF_MAX,
};
typedef enum hsrv_openflow_nexthop_opf_type_e hsrv_openflow_nexthop_opf_type_t;
#define HSRV_OPENFLOW_NEXTHOP_OPF_TYPE_STR(type)    \
    ((HSRV_OPENFLOW_NEXTHOP_OPF_NONE == type) ? "none" : \
     (HSRV_OPENFLOW_NEXTHOP_OPF_NHID == type) ? "nhid" : \
     (HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET == type) ? "nh_offset" : \
     (HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET == type) ? "mc_offset" : \
     (HSRV_OPENFLOW_NEXTHOP_OPF_MPLS_TUNNELID == type) ? "mc_offset" : \
     "unknown")

struct hsrv_openflow_nexthop_master_s
{
    hsrv_of_nh_offset_t mcast_all_nh;

    ctclib_hash_t* flex_nh_hash;       /**< hsrv_ofact_flex_nh_info_t */
    ctclib_hash_t* mpls_nh_hash;       /**< hsrv_ofact_mpls_nh_info_t */
    ctclib_hash_t* ipuc_nh_hash;       /**< hsrv_ofact_ipuc_nh_info_t */
    ctclib_hash_t* ip_tunnel_nh_hash;  /**< hsrv_ofact_ip_tunnel_nh_info_t */
    ctclib_hash_t* xlate_nh_hash;      /**< hsrv_ofact_xlate_nh_info_t */
    ctclib_hash_t* pst_mpls_tunnel_hash;    /* store tunnel resource. */
    ctclib_hash_t* pst_mpls_nh2tunnel_hash; /* store nh id 2 tunnel id mapping. */
    uint32  reserved_nh_count;
    uint32  nexthop_max_num;         /**< max nexthop number */

    ctclib_opf_t* pst_opf[HSRV_OPENFLOW_NEXTHOP_OPF_MAX];
    uint32  resvd_opf[HSRV_OPENFLOW_NEXTHOP_OPF_MAX];
    uint32  nh_type_count[HSRV_OF_NH_TYPE_MAX];
};
typedef struct hsrv_openflow_nexthop_master_s hsrv_openflow_nexthop_master_t;


#define HSRV_OFNH_FLEX_NH_HASH        g_pst_openflow_nexthop_master->flex_nh_hash
#define HSRV_OFNH_MPLS_NH_HASH        g_pst_openflow_nexthop_master->mpls_nh_hash
#define HSRV_OFNH_IPUC_NH_HASH        g_pst_openflow_nexthop_master->ipuc_nh_hash
#define HSRV_OFNH_IP_TUNNEL_NH_HASH   g_pst_openflow_nexthop_master->ip_tunnel_nh_hash
#define HSRV_OFNH_XLATE_NH_HASH       g_pst_openflow_nexthop_master->xlate_nh_hash
#define HSRV_OFNH_MPLS_TUNNEL_HASH    g_pst_openflow_nexthop_master->pst_mpls_tunnel_hash


#define HSRV_OFNH_OPF                 g_pst_openflow_nexthop_master->pst_opf
#define HSRV_OFNH_NH_ID_OPF           g_pst_openflow_nexthop_master->pst_opf[HSRV_OPENFLOW_NEXTHOP_OPF_NHID]
#define HSRV_OFNH_NH_OFFSET_OPF       g_pst_openflow_nexthop_master->pst_opf[HSRV_OPENFLOW_NEXTHOP_OPF_NH_OFFSET]
#define HSRV_OFNH_MC_OFFSET_OPF       g_pst_openflow_nexthop_master->pst_opf[HSRV_OPENFLOW_NEXTHOP_OPF_MC_OFFSET]
#define HSRV_OFNH_MPLS_TUNNEL_ID_OPF  g_pst_openflow_nexthop_master->pst_opf[HSRV_OPENFLOW_NEXTHOP_OPF_MPLS_TUNNELID]

/* Added by weizj for tunnel */
#define ADPT_NEXTHOP_IP_TUNNEL_NH_BLOCK_SIZE    1024

/* curbro mpls. */
#define ADPT_NEXTHOP_MPLS_NH_BLOCK_SIZE    1024

/* modified by yaom for openflow MPLS L2VPN 20160214 */
#define ADPT_NEXTHOP_IPUC_NH_BLOCK_SIZE    1024
#define ADPT_NEXTHOP_XLATE_NH_BLOCK_SIZE   1024

#endif /* !__HSRV_OPENFLOW_NEXTHOP_PRIV_H__ */
