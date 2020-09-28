
#ifndef __STBL_SAI_GLB_DEFINE_H__
#define __STBL_SAI_GLB_DEFINE_H__

/* STBL_SAI_GLB field defines */
typedef enum
{
    STBL_SAI_GLB_FLD_NH_MAX_COUNT         = 0 ,  /* READ */
    STBL_SAI_GLB_FLD_NH_GROUP_MAX_COUNT   = 1 ,  /* READ */
    STBL_SAI_GLB_FLD_VRF_MAX_COUNT        = 2 ,  /* READ */
    STBL_SAI_GLB_FLD_RIF_MAX_COUNT        = 3 ,  /* READ */
    STBL_SAI_GLB_FLD_ROUTE_MAX_COUNT      = 4 ,  /* READ */
    STBL_SAI_GLB_FLD_STP_ID_MAX_COUNT     = 5 ,  /* READ */
    STBL_SAI_GLB_FLD_NH_GROUP_CURR_COUNT  = 6 ,  /* READ */
    STBL_SAI_GLB_FLD_HOSTIF_MAX_COUNT     = 7 ,  /* READ */
    STBL_SAI_GLB_FLD_HOSTIF_GROUP_MAX_COUNT = 8 ,  /* READ */
    STBL_SAI_GLB_FLD_VRRP_MAX_COUNT       = 9 ,  /* READ */
    STBL_SAI_GLB_FLD_VLAN_CLASS_MAX_COUNT = 10,  /* READ */
    STBL_SAI_GLB_FLD_HASH_MAX_COUNT       = 11,  /* READ */
    STBL_SAI_GLB_FLD_LAG_MAX_COUNT        = 12,  /* READ */
    STBL_SAI_GLB_FLD_QOS_WRED_MAX_COUNT   = 13,  /* READ */
    STBL_SAI_GLB_FLD_QOS_MAP_MAX_COUNT    = 14,  /* READ */
    STBL_SAI_GLB_FLD_QOS_SCHEDULER_MAX_COUNT = 15,  /* READ */
    STBL_SAI_GLB_FLD_QOS_POLICER_MAX_COUNT = 16,  /* READ */
    STBL_SAI_GLB_FLD_DEFAULT_STP_ID       = 17,  /* READ */
    STBL_SAI_GLB_FLD_HOSTIF_FD_HASH       = 18,  /* READ */
    STBL_SAI_GLB_FLD_HOSTIF_PORTIF_HASH   = 19,  /* READ */
    STBL_SAI_GLB_FLD_HOSTIF_VLANIF_HASH   = 20,  /* READ */
    STBL_SAI_GLB_FLD_L2MC_GROUP_OPB       = 21,  /* READ */
    STBL_SAI_GLB_FLD_ISOLATION_ID_OPB     = 22,  /* READ */
    STBL_SAI_GLB_FLD_MIRROR_ID_OPB        = 23,  /* READ */
    STBL_SAI_GLB_FLD_NEXTHOP_ID_OPB       = 24,  /* READ */
    STBL_SAI_GLB_FLD_NEXTHOP_GROUP_ID_OPB = 25,  /* READ */
    STBL_SAI_GLB_FLD_STPID_OPB            = 26,  /* READ */
    STBL_SAI_GLB_FLD_TAP_GROUP_OPB        = 27,  /* READ */
    STBL_SAI_GLB_FLD_VLAN_CLASS_OPB       = 28,  /* READ */
    STBL_SAI_GLB_FLD_VRF_ID_OPB           = 29,  /* READ */
    STBL_SAI_GLB_FLD_HASH_OPB             = 30,  /* READ */
    STBL_SAI_GLB_FLD_LAG_OPB              = 31,  /* READ */
    STBL_SAI_GLB_FLD_QOS_WRED_OPB         = 32,  /* READ */
    STBL_SAI_GLB_FLD_QOS_MAP_OPB          = 33,  /* READ */
    STBL_SAI_GLB_FLD_QOS_SCHEDULER_OPB    = 34,  /* READ */
    STBL_SAI_GLB_FLD_QOS_POLICER_OPB      = 35,  /* READ */
    STBL_SAI_GLB_FLD_UDF_OPB              = 36,  /* READ */
    STBL_SAI_GLB_FLD_COPP_ENTRY_OPB       = 37,  /* READ */
    STBL_SAI_GLB_FLD_MAX                  = 38
} stbl_sai_glb_field_id_t;

/* STBL_SAI_GLB defines */
typedef struct
{
    uint32               nh_max_count;
    uint32               nh_group_max_count;
    uint32               vrf_max_count;
    uint32               rif_max_count;
    uint32               route_max_count;
    uint32               stp_id_max_count;
    uint32               nh_group_curr_count;
    uint32               hostif_max_count;
    uint32               hostif_group_max_count;
    uint32               vrrp_max_count;
    uint32               vlan_class_max_count;
    uint32               hash_max_count;
    uint32               lag_max_count;
    uint32               qos_wred_max_count;
    uint32               qos_map_max_count;
    uint32               qos_scheduler_max_count;
    uint32               qos_policer_max_count;
    sai_object_id_t      default_stp_id;
    ctclib_hash_t        *hostif_fd_hash;
    ctclib_hash_t        *hostif_portif_hash;
    ctclib_hash_t        *hostif_vlanif_hash;
    ctclib_opb_t         l2mc_group_opb;
    ctclib_opb_t         isolation_id_opb;
    ctclib_opb_t         mirror_id_opb;
    ctclib_opb_t         nexthop_id_opb;
    ctclib_opb_t         nexthop_group_id_opb;
    ctclib_opb_t         stpid_opb;
    ctclib_opb_t         tap_group_opb;
    ctclib_opb_t         vlan_class_opb;
    ctclib_opb_t         vrf_id_opb;
    ctclib_opb_t         hash_opb;
    ctclib_opb_t         lag_opb;
    ctclib_opb_t         qos_wred_opb;
    ctclib_opb_t         qos_map_opb;
    ctclib_opb_t         qos_scheduler_opb;
    ctclib_opb_t         qos_policer_opb;
    ctclib_opb_t         udf_opb;
    ctclib_opb_t         copp_entry_opb;
} stbl_sai_glb_t;

#endif /* !__STBL_SAI_GLB_DEFINE_H__ */

