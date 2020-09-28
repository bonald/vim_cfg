#ifndef __HSRV_OPENFLOW_GROUP_H__
#define __HSRV_OPENFLOW_GROUP_H__


#define HSRV_COMMON_GROUP_WATCH_IGNORE    0
#define HSRV_OFPP_NONE                    0xffffffff
#define HSRV_COMMON_GROUP_WATCH_PORT_IGNORE(watch_port)     \
    ((watch_port) == HSRV_OFPP_NONE ||                      \
     (watch_port) == HSRV_COMMON_GROUP_WATCH_IGNORE)

/* hsrv openflow memory group related macros */
#define HSRV_OPENFLOW_GROUP_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_GROUP, size)
#define HSRV_OPENFLOW_GROUP_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_GROUP, size)
#define HSRV_OPENFLOW_GROUP_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_GROUP, ptr);  \
    ptr = NULL; \
}while(0)

#define HSRV_OF_GROUP_TYPE_STR(type)                \
    (GLB_OF_GROUP_TYPE_ALL == (type)      ? "all" : \
     GLB_OF_GROUP_TYPE_SELECT == (type)   ? "select" : \
     GLB_OF_GROUP_TYPE_INDIRECT == (type) ? "indirect" : \
     GLB_OF_GROUP_TYPE_FF == (type)       ? "ff" : \
     GLB_OF_GROUP_TYPE_LSP_APS == (type)       ? "lsp_aps" : \
    GLB_OF_GROUP_TYPE_PW_APS == (type)       ? "pw_aps" : \
     "UNKNOWN")


/* Group numbering. Groups can use any number up to OFPG_MAX. */
enum hsrv_ofp_group {
    /* Last usable group number. */
    HSRV_OPENFLOW_GROUP_MAX        = 0xffffff00,

    /* Fake groups. */
    HSRV_OPENFLOW_GROUP_ALL        = 0xfffffffc,  /* All groups, for group delete commands. */
    HSRV_OPENFLOW_GROUP_ANY        = 0xffffffff   /* Wildcard, for flow stats requests. */
};

#define HSRV_OPENFLOW_GROUP_ID_INVALID            (HSRV_OPENFLOW_GROUP_MAX + 1)
#define HSRV_OPENFLOW_GROUP_BUCKET_ACTION_MAX     13

/**
 * @brief Hsrv layer group bucket nexthop info
 */
struct hsrv_of_bucket_info_s
{
    uint32 member_count;
    hsrv_of_nh_info_t member_nh[HSRV_OPENFLOW_GROUP_BUCKET_ACTION_MAX];
};
typedef struct hsrv_of_bucket_info_s hsrv_of_bucket_info_t;

/**
 * @brief Hsrv layer group nexthop info
 */
struct hsrv_openflow_group_nexthop_info_s
{
    bool use_mcast;
    bool use_ecmp;
    bool use_linkagg;

    uint32 bucket_count;

    hsrv_of_nh_info_t      main_nh;
    hsrv_of_nh_info_t      linkagg_nh;
    hsrv_of_bucket_info_t* buckets;
 };
typedef struct hsrv_openflow_group_nexthop_info_s hsrv_openflow_group_nexthop_info_t;

/**
 * @brief Hsrv layer group default info
 */
struct hsrv_openflow_group_dft_info_s
{
    bool stats_en;

    uint32 stats_id;
    hsrv_openflow_group_nexthop_info_t nh_info; 
};
typedef struct hsrv_openflow_group_dft_info_s hsrv_openflow_group_dft_info_t;

/**
 * @brief Hsrv layer group common info
 */
union hsrv_openflow_group_info_s
{
    hsrv_openflow_group_dft_info_t dft;
    /* more */
};
typedef union hsrv_openflow_group_info_s hsrv_openflow_group_info_t;


/* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-10 */
struct hsrv_openflow_aps_group_s
{
    uint32                               aps_group_offset;       /* aps group id for chip */
    uint32                               fast_aps_en;
    uint32                               tunnel_id;
    uint32                               ptunnel_id;
    uint32                               pw_label;
    uint32                               ppw_label;
    uint32                               gport;
    uint32                               pgport;
    uint32                               aps_nhid;
    uint32                               aps_offset;
    uint32                               aps_nhid_count;
    uint32                               mcast_nhid;
    uint32                               mcast_offset;
    uint32                               bucket_lsp_aps_group_id;/*modified by yejl for bug 47825, 2019-06-25*/
    uint32                               bucket_plsp_aps_group_id;/*modified by yejl for bug 47825, 2019-06-25*/
    uint32                               lsp_aps_group_id;
    uint32                               plsp_aps_group_id;
    uint32                               working_mepIndex;
    uint32                               protection_mepIndex;
    ctclib_hash_t*                    lsp_aps_nh_hash;       /**< hsrv_ofact_mpls_nh_info_t */
};
typedef struct hsrv_openflow_aps_group_s hsrv_openflow_aps_group_t;
/* liwh end */

/**
 * @brief Hsrv layer group info
 */
struct hsrv_openflow_group_s
{
    glb_openflow_group_t   group;
    hsrv_openflow_group_info_t u;

    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-04-10 */
    hsrv_openflow_aps_group_t      aps_group;
    /* liwh end */
    
    glb_openflow_tpoam_bucket_t* bucket_list;   /**< glb_openflow_bucket_t */
};
typedef struct hsrv_openflow_group_s hsrv_openflow_group_t;

struct hsrv_openflow_bound_group_s
{
    uint32 group_id;
    uint32 group_nhid;
    bool   is_bound;
};
typedef struct hsrv_openflow_bound_group_s hsrv_openflow_bound_group_t;

/**
 * @brief Hsrv layer group debug data
 */
struct hsrv_openflow_group_debug_data_s
{
    FILE* fp;
};
typedef struct hsrv_openflow_group_debug_data_s hsrv_openflow_group_debug_data_t;

struct hsrv_group_watch_port_info_s
{
    ctclib_list_node_t node;
    
    uint32_ofp group_id;
    uint32_ofp bucket_idx;
};
typedef struct hsrv_group_watch_port_info_s hsrv_group_watch_port_info_t;


/**
 * @brief Hsrv layer group mod group
 */
int32 
fei_api_openflow_mod_group(hsrv_openflow_group_t* p_old_group, glb_openflow_group_t *p_fei_group);

/**
 * @brief Hsrv layer group lookup groupdb
 */
int32
hsrv_openflow_group_lookup_group(uint32 group_id, hsrv_openflow_group_t** pp_group);

int32
_hsrv_openflow_group_update_ff_group_liveness(hsrv_openflow_group_t* p_group);

/**
 * @brief Hsrv layer group watch port link status
 */
int32
hsrv_openflow_group_update_group_liveness(uint32 ifindex, bool link_enable);

/**
 * @brief Hsrv layer group get drop ingress, Added by weizj for bug 35135
 */
bool
hsrv_openflow_group_get_drop_ingress(void);

/**
 * @brief Hsrv layer group show group brief
 */
int32
hsrv_openflow_show_group_brief(FILE *fp);

/**
 * @brief Hsrv layer group show group info
 */
int32
hsrv_openflow_show_group(FILE *fp, uint32 group_id);

/**
 * @brief Hsrv layer group start
 */
int32
hsrv_openflow_group_start(void);

/**
 * @brief Fea layer groupdb lookup group
 */
int32
hsrv_openflow_groupdb_lookup_group(uint32 group_id, hsrv_openflow_group_t** pp_group);


#endif /* !__HSRV_OPENFLOW_GROUP_H__ */
