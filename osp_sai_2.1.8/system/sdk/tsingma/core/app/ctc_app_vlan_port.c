#include "ctc_app.h"
#include "ctc_app_index.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_opf.h"
#include "ctc_spool.h"
#include "ctc_warmboot.h"
#include "ctc_app_vlan_port_api.h"
#include "ctc_app_vlan_port.h"

#if defined(GREATBELT) || defined(DUET2)
ctc_app_vlan_port_api_t ctc_gb_dt2_app_vlan_port_api =
{
    ctc_gbx_app_vlan_port_init,
    ctc_gbx_app_vlan_port_get_nni,
    ctc_gbx_app_vlan_port_destory_nni,
    ctc_gbx_app_vlan_port_create_nni,
    ctc_gbx_app_vlan_port_update_nni,
    ctc_gbx_app_vlan_port_update,
    ctc_gbx_app_vlan_port_get,
    ctc_gbx_app_vlan_port_destory,
    ctc_gbx_app_vlan_port_create,
    ctc_gbx_app_vlan_port_update_gem_port,
    ctc_gbx_app_vlan_port_get_gem_port,
    ctc_gbx_app_vlan_port_destory_gem_port,
    ctc_gbx_app_vlan_port_create_gem_port,
    ctc_gbx_app_vlan_port_get_uni,
    ctc_gbx_app_vlan_port_destory_uni,
    ctc_gbx_app_vlan_port_create_uni,
    ctc_gbx_app_vlan_port_get_by_logic_port,
    ctc_gbx_app_vlan_port_get_global_cfg,
    ctc_gbx_app_vlan_port_set_global_cfg,
    ctc_gbx_app_vlan_port_get_fid_mapping_info,
    ctc_gbx_app_vlan_port_alloc_fid,
    ctc_gbx_app_vlan_port_free_fid,
};

#define CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE 0xFFFFFFFF

#define CTC_APP_VLAN_PORT_CREAT_LOCK(lchip)                 \
    do                                                      \
    {                                                       \
        sal_mutex_create(&p_g_app_vlan_port_master->mutex); \
        if (NULL == p_g_app_vlan_port_master->mutex)        \
        {                                                   \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY);              \
        }                                                   \
    } while (0)

#define CTC_APP_VLAN_PORT_LOCK(lchip) \
    if(p_g_app_vlan_port_master->mutex) sal_mutex_lock(p_g_app_vlan_port_master->mutex)

#define CTC_APP_VLAN_PORT_UNLOCK(lchip) \
    if(p_g_app_vlan_port_master->mutex) sal_mutex_unlock(p_g_app_vlan_port_master->mutex)

#define CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(op)               \
    do                                                          \
    {                                                           \
        int32 rv = (op);                                        \
        if (rv < 0)                                             \
        {                                                       \
            sal_mutex_unlock(p_g_app_vlan_port_master->mutex);  \
            return rv;                                          \
        }                                                       \
    } while (0)

enum ctc_app_vlan_port_nhid_e
{
    CTC_APP_VLAN_PORT_NHID_RSV_ILOOP = CTC_NH_RESERVED_NHID_MAX, /*3*/
    CTC_APP_VLAN_PORT_NHID_RSV_E2ILOOP, /*4*/
    CTC_APP_VLAN_PORT_NHID_ALLOC,       /*5*/
    CTC_APP_VLAN_PORT_NHID_MAX
};
typedef enum ctc_app_vlan_port_nhid_e ctc_app_vlan_port_nhid_t;

enum ctc_app_vlan_port_opf_e
{
    CTC_APP_VLAN_PORT_OPF_LOGIC_PORT,  /*logic port*/
    CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID,
    CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN,
    CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP,
    CTC_APP_VLAN_PORT_OPF_FID,
    CTC_APP_VLAN_PORT_OPF_GEM_PORT,
    CTC_APP_VLAN_PORT_OPF_MAX
};
typedef enum ctc_app_vlan_port_opf_e ctc_app_vlan_port_opf_t;

struct ctc_app_vlan_port_uni_db_s
{
    uint16 ref_cnt;

    uint32 iloop_port;        /*[UC]*/
    uint32 iloop_nhid;        /*[UC]*/
    uint32 e2iloop_nhid;      /*[UC]*/

    uint32 onu_e2eloop_port;      /*[UC] for onu service*/
    uint32 onu_e2eloop_sec_port;  /*[UC] for onu service, for del cvlan*/
    uint32 e2eloop_port;          /*[UC] for pon service*/

    uint32 bc_eloop_port;
    uint32 bc_xlate_nhid;
    uint32 bc_e2iloop_nhid;      /*[BC]*/

    uint32 mc_eloop_port;
    uint32 mc_xlate_nhid;
    uint32 mc_e2iloop_nhid;      /*[MC]*/

    uint8 bc_mc_installed;
    uint8 vlan_range_grp;
    uint16 vdev_id;

    uint32 iloop_sec_port;
    uint32 eloop_sec_port;
    uint32 iloop_sec_nhid;
    uint32 e2iloop_sec_nhid;
};
typedef struct ctc_app_vlan_port_uni_db_s ctc_app_vlan_port_uni_db_t;

struct ctc_app_vlan_port_nni_port_db_s
{
    uint16 vdev_id;
    uint8 rx_en;
    uint8 rsv;
    uint32 port;
    uint32 nni_ad_index;
    uint32 nni_nh_id;
    uint32 nni_logic_port;
};
typedef struct ctc_app_vlan_port_nni_port_db_s ctc_app_vlan_port_nni_port_db_t;

struct ctc_app_vlan_port_master_s
{

    uint32 downlink_eloop_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];  /*from onu to uplink*/

    uint32 flex_1st_iloop_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];  /*flexible QinQ*/
    uint32 flex_1st_iloop_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];  /*flexible QinQ*/
    uint32 flex_2nd_iloop_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];  /*flexible QinQ*/
    uint32 flex_2nd_iloop_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];  /*flexible QinQ*/
    uint32 flex_2nd_e2iloop_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];

    uint32 downlink_iloop_default_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 downlink_iloop_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];

    ctc_hash_t* fdb_entry_hash;
    ctc_hash_t* nni_port_hash;
    ctc_hash_t* gem_port_hash;
    ctc_hash_t* vlan_port_hash;
    ctc_hash_t* vlan_port_key_hash;
    ctc_hash_t* vlan_port_logic_vlan_hash;
    ctc_spool_t* fid_spool;

    uint16 gem_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 vlan_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 nni_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];

    uint16 nni_logic_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 nni_mcast_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 default_logic_dest_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 uni_outer_isolate_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 uni_inner_isolate_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 unknown_mcast_drop_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 default_bcast_fid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 default_deny_learning_fid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 vdev_num;
    uint16 mcast_max_group_num;
    uint16 max_fid_num;
    uint32 vlan_isolate_bmp[CTC_MAX_VLAN_ID/CTC_UINT32_BITS+1];
    uint16 mcast_tunnel_vlan;
    uint16 bcast_tunnel_vlan;
    uint32 flag;
    ctc_app_vlan_port_uni_db_t* p_port_pon;
    sal_mutex_t* mutex;

    int32 (*ctc_global_set_xgpon_en)(uint8 lchip, uint8 enable);
    int32 (*ctc_port_set_service_policer_en)(uint8 lchip, uint32 gport, bool enable);
    int32 (*ctc_nh_set_xgpon_en)(uint8 lchip, uint8 enable, uint32 flag);
    int32 (*ctc_port_set_global_port)(uint8 lchip, uint8 lport, uint16 gport);
    int32 (*ctc_nh_update_logic_port)(uint8 lchip, uint32 nhid, uint32 logic_port);
    int32 (*ctc_qos_policer_index_get)(uint8 lchip, ctc_qos_policer_type_t type,  uint16 plc_id, uint16* p_index);
    int32 (*ctc_qos_policer_reserv_service_hbwp)(uint8 lchip);
    int32 (*ctc_nh_set_mcast_bitmap_ptr)(uint8 lchip, uint32 offset);
    int32 (*ctc_l2_hw_sync_add_db)(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
    int32 (*ctc_l2_hw_sync_remove_db)(uint8 lchip, ctc_l2_addr_t* l2_addr);
    int32 (*ctc_l2_hw_sync_alloc_ad_idx)(uint8 lchip, uint32* ad_index);
    int32 (*ctc_l2_hw_sync_free_ad_idx)(uint8 lchip, uint32 ad_index);
    int32 (*ctc_ftm_alloc_table_offset_from_position)(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);
    int32 (*ctc_port_set_mac_rx_en)(uint8 lchip, uint32 port, uint32 value);
    int32 (*ctc_port_set_logic_port_en)(uint8 lchip, uint32 gport, uint32 value);
    int32 (*ctc_mac_security_set_fid_learn_limit_action)(uint8 lchip, uint16 fid, ctc_maclimit_action_t action);
    int32 (*ctc_port_set_ingress_property)(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);
    int32 (*ctc_nh_update_xlate)(uint8 lchip, uint32 nhid, ctc_vlan_egress_edit_info_t* p_vlan_info, ctc_vlan_edit_nh_param_t* p_nh_param);
    int32 (*ctc_l2_fdb_set_station_move_action)(uint8 lchip, uint32 ad_index, uint32 value);
};
typedef struct ctc_app_vlan_port_master_s ctc_app_vlan_port_master_t;

struct ctc_app_vlan_port_gem_port_db_s
{
    uint32 port;
    uint32 tunnel_value;

    uint32 logic_port;
    uint32 ref_cnt;

    uint8  flag;
    uint8  igs_vlan_maping_use_flex;

    uint32 e2iloop_nhid;      /*[D2] e2iloop to downlink policer*/
    uint32 xlate_nhid;        /*[UC.MC]*/
    uint32 e2eloop_nhid;      /*PON servie nexthop*/
    uint32 flex_e2eloop_nhid; /*PON flex servie nexthop*/

    uint32 onu_eloop_nhid;      /*ONU servie nexthop*/

    uint32 ad_index;


    uint16 logic_dest_port;
    uint16 ingress_policer_id;
    uint16 egress_policer_id;
    uint8 egress_cos_idx;
    uint16 vdev_id;
    uint32 limit_num;
    uint32 limit_count;
    uint16 limit_action;
    uint8  mac_security;
    uint8  rsv;

    uint32 ingress_stats_id;
    uint32 egress_stats_id;
    uint32 custom_id;
    uint8 station_move_action;
};
typedef struct ctc_app_vlan_port_gem_port_db_s ctc_app_vlan_port_gem_port_db_t;

struct ctc_app_vlan_port_db_s
{
    uint32 criteria;
    uint32 port;
    uint32 tunnel_value;
    uint16 match_svlan;
    uint16 match_cvlan;
    uint16 match_svlan_end;
    uint16 match_cvlan_end;
    ctc_acl_key_t flex_key;
    ctc_l2_mcast_addr_t l2mc_addr;

    uint32 calc_key_len[0];

    uint32 vlan_port_id;


    ctc_app_vlan_action_set_t ingress_vlan_action_set;  /* Ingress vlan action */
    ctc_app_vlan_action_set_t egress_vlan_action_set;   /* Egress vlan action */

    uint32 ad_index;


    uint32 logic_port;
    uint32 xlate_nhid;
    uint32 xlate_nhid_onu;  /*for del cvlan*/
    uint16 pkt_svlan;
    uint16 pkt_cvlan;
    uint16 fid;
    uint16 fid_mapping;
    uint8 egs_vlan_mapping_en;
    uint8 logic_port_b_en;
    uint8 igs_vlan_mapping_use_flex;

    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db;

    uint32 flex_xlate_nhid;
    uint32 flex_add_vlan_xlate_nhid;
    uint16 flex_logic_port_vlan;

    uint16 logic_dest_port;
    uint16 ingress_policer_id;
    uint16 egress_policer_id;
    uint16 vdev_id;

    uint32 pon_uplink_iloop_port;
    uint32 pon_uplink_iloop_nhid;
    uint32 pon_uplink_e2iloop_nhid;
    uint32 pon_downlink_iloop_port;
    uint32 pon_downlink_iloop_nhid;
    uint32 pon_downlink_e2iloop_nhid;

    uint32 ingress_stats_id;
    uint32 egress_stats_id;
    uint32 count;
};
typedef struct ctc_app_vlan_port_db_s ctc_app_vlan_port_db_t;

struct ctc_app_vlan_port_find_s
{
    uint32 logic_port;
    uint16 match_svlan;
    uint16 match_cvlan;

    ctc_app_vlan_port_db_t* p_vlan_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db;
};
typedef struct ctc_app_vlan_port_find_s ctc_app_vlan_port_find_t;

struct ctc_app_vlan_port_sync_db_s
{
    uint32 logic_port;
    uint16 match_svlan;
    uint16 match_cvlan;

    uint32 oun_nhid;
    uint32 pon_nhid;

    uint32 ad_index;
    uint16 vdev_id;
};
typedef struct ctc_app_vlan_port_sync_db_s ctc_app_vlan_port_sync_db_t;

struct ctc_app_vlan_port_fid_db_s
{
    uint16 vdev_id;              /**< [GB.D2] [in] virtual device Identification */
    uint16 pkt_svlan;            /**< [GB.D2] [in] svlan id in packet */
    uint16 pkt_cvlan;            /**< [GB.D2] [in] cvlan id in packet */
    uint16 is_flex;              /**< [GB.D2] [in] flex qinq */
    uint16 fid;                  /**< [GB.D2] [out] forwarding id */
    uint16 rsv;
    uint32 limit_count;          /**< [D2] [out] limit count */
    uint32 limit_num;            /**< [D2] [out] limit number */
    uint16 limit_action;         /**< [D2] [out] limit action */
    uint16 rsv1;
};
typedef struct ctc_app_vlan_port_fid_db_s ctc_app_vlan_port_fid_db_t;

struct ctc_app_vlan_port_fdb_node_s
{
    mac_addr_t mac;
    uint16     fid;

    uint16 logic_port;
    uint32 ad_index;

};
typedef struct ctc_app_vlan_port_fdb_node_s   ctc_app_vlan_port_fdb_node_t;

enum ctc_app_vlan_port_fdb_flush_type_e
{
    CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT = 0,
    CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT_FID,
    CTC_APP_VLAN_PORT_FDB_FLUSH_MAX
};
typedef enum ctc_app_vlan_port_fdb_flush_type_e ctc_app_vlan_port_fdb_flush_type_t;

struct ctc_app_vlan_port_fdb_flush_s
{
    uint16     fid;
    uint16 logic_port;
    uint8 type;   /*refer to ctc_app_vlan_port_fdb_flush_type_t*/
    uint8 lchip;
};
typedef struct ctc_app_vlan_port_fdb_flush_s   ctc_app_vlan_port_fdb_flush_t;


#define CTC_APP_VLAN_PORT_GEM_LMT_ACL_GRP CTC_ACL_GROUP_ID_NORMAL
#define CTC_APP_VLAN_PORT_GEM_LMT_ACL_PRI 3
#define CTC_APP_VLAN_PORT_LOGIC_PORT_NUM  16*1024

#define CTC_APP_MAP_VLAN_PORT_NHID(port) (port+CTC_APP_VLAN_PORT_NHID_ALLOC)
#define CTC_APP_VLAN_PORT_DEFULAT_FID 1
#define CTC_APP_VLAN_PORT_NNI_ISOLATION_ID 0xF
#define CTC_APP_VLAN_PORT_MAX_FID 12*1024

#define CTC_APP_VLAN_PORT_VLAN_ID_CHECK(val) \
do { \
    if ((val) > CTC_MAX_VLAN_ID)\
    {\
        return CTC_E_BADID;\
    }\
}while(0)

#define CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, GPORT) \
do { \
    uint8 gchip_id = 0; \
    CTC_GLOBAL_PORT_CHECK(GPORT); \
    ctc_get_gchip_id((lchip), &gchip_id); \
    if ((CTC_MAP_GPORT_TO_GCHIP(GPORT) != 0x1f) && (CTC_MAP_GPORT_TO_GCHIP(GPORT) != gchip_id)) \
    { \
        return CTC_E_BADID; \
    } \
}while(0)

#define CTC_APP_VLAN_PORT_VDEV_ID_CHECK(ID) \
do {\
    if ((ID) >= p_g_app_vlan_port_master->vdev_num)\
        return CTC_E_INVALID_PARAM; \
}while(0)
#define CTC_APP_VLAN_PORT_INIT_CHECK() \
do {\
    if(NULL == p_g_app_vlan_port_master) return CTC_E_NOT_INIT;\
}while(0);


#define CTC_APP_VLAN_PORT_API_CALL(api, lchip, ...) \
    api? api(lchip, ##__VA_ARGS__) : CTC_E_NONE

#define CTC_APP_VLAN_PORT_API_ERROR_RETURN(api, lchip, ...) \
{ \
    int32 ret = CTC_E_NOT_SUPPORT; \
    ret = api? api(lchip, ##__VA_ARGS__) : CTC_E_NONE; \
    ret = ( ret < 0 )? ctc_error_code_mapping(ret) : ret; \
    return ret;\
}

#ifdef DUET2
    #define CTC_APP_VLAN_PORT_NNI_CLASS_ID  0xFF
#else
    #define CTC_APP_VLAN_PORT_NNI_CLASS_ID  61
#endif 

#define CTC_APP_VLAN_PORT_RSV_MCAST_GROUP_ID(id) (id+1)
#define ENCODE_SCL_XC_ENTRY_ID(entry_id) (1 << 24 | (entry_id))
#define ENCODE_SCL_DICARD_ENTRY_ID(entry_id) (2 << 24 | (entry_id))
#define ENCODE_SCL_VLAN_ENTRY_ID(entry_id, offset) (3 << 24 | ((2*entry_id)+ offset))
#define ENCODE_SCL_DOWNLINK_ENTRY_ID(entry_id, id) (4<<24 | (entry_id*2) | id)
#define ENCODE_SCL_UPLINK_ENTRY_ID(fid, port) (5<<24 | (port)<<13 |(fid))
#define ENCODE_SCL_DOWNLINK_ILOOP_ENTRY_ID(port) (6<<24 | port)

#define CTC_APP_VLAN_PORT_ACL_GROUP_ID 1
#define ENCODE_ACL_HASH_ENTRY_ID(port, entry_id) (1<<24 | (port)<<13 |(entry_id))
#define ENCODE_ACL_DOWNLINK_ENTRY_ID(entry_id) (4<<24 | (entry_id))
#define ENCODE_ACL_UPLINK_ENTRY_ID(entry_id) (5<<24 | (entry_id))
#define ENCODE_MCAST_GROUP_ID(GROUP_ID) (p_g_app_vlan_port_master->mcast_max_group_num-1-GROUP_ID)
#define ENCODE_ACL_METADATA(id) (0x3FFC | (id))

#define CTC_APP_DBG_FUNC()          CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__)
#define CTC_APP_DBG_INFO(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_ERROR(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_PARAM(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_DUMP(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__)

#define CTC_APP_DBG_PARAM_ST(param) CTC_APP_DBG_PARAM("%-40s :%10d\n", #param, p_vlan_port->param)
#define CTC_APP_VLAN_PORT_FID_COPY(p_dst_db, p_src_db) \
    do {\
        (p_dst_db)->vdev_id = (p_src_db)->vdev_id;\
        (p_dst_db)->pkt_svlan = (p_src_db)->pkt_svlan;\
        (p_dst_db)->pkt_cvlan = (p_src_db)->pkt_cvlan;\
        (p_dst_db)->is_flex = (p_src_db)->is_flex;\
        (p_dst_db)->fid = (p_src_db)->fid;\
        (p_dst_db)->rsv = (p_src_db)->rsv;\
    }while(0)

ctc_app_vlan_port_master_t *p_g_app_vlan_port_master = NULL;

#if defined (GREATBELT)
extern int32 sys_greatbelt_global_set_xgpon_en(uint8 lchip, uint8 enable);
extern int32 sys_greatbelt_port_set_service_policer_en(uint8 lchip, uint32 gport, bool enable);
extern int32 sys_greatbelt_nh_set_xgpon_en(uint8 lchip, uint8 enable, uint32 flag);
extern int32 sys_greatbelt_port_set_global_port(uint8 lchip, uint8 lport, uint16 gport);
extern int32 sys_greatbelt_nh_update_logic_port(uint8 lchip, uint32 nhid, uint32 logic_port);
extern int32 sys_greatbelt_qos_policer_index_get(uint8 lchip, ctc_qos_policer_type_t type, uint16 plc_id, uint16* p_index);
extern int32 sys_greatbelt_qos_policer_reserv_service_hbwp(uint8 lchip);
extern int32 sys_greatbelt_l2_hw_sync_add_db(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
extern int32 sys_greatbelt_l2_hw_sync_remove_db(uint8 lchip, ctc_l2_addr_t* l2_addr);
extern int32  sys_greatbelt_l2_hw_sync_alloc_ad_idx(uint8 lchip, uint32* ad_index);
extern int32  sys_greatbelt_l2_hw_sync_free_ad_idx(uint8 lchip, uint32 ad_index);
extern int32 sys_greatbelt_nh_set_mcast_bitmap_ptr(uint8 lchip, uint32 offset);
extern int32 sys_greatbelt_port_set_mac_rx_en(uint8 lchip, uint32 gport, uint32 value);
extern int32 sys_greatbelt_port_set_logic_port_check_en(uint8 lchip, uint32 gport, uint32 value);
#endif

#if defined (DUET2)
extern int32 sys_usw_global_set_xgpon_en(uint8 lchip, uint8 enable);
extern int32 sys_usw_nh_set_xgpon_en(uint8 lchip, uint8 enable, uint32 flag);
extern int32 sys_usw_nh_update_logic_port(uint8 lchip, uint32 nhid, uint32 logic_port);
extern int32 sys_usw_qos_policer_index_get(uint8 lchip, ctc_qos_policer_type_t type, uint16 plc_id, uint16* p_index);
extern int32 sys_usw_nh_set_mcast_bitmap_ptr(uint8 lchip, uint32 offset);
extern int32 sys_usw_l2_hw_sync_add_db(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
extern int32 sys_usw_l2_hw_sync_remove_db(uint8 lchip, ctc_l2_addr_t* l2_addr);
extern int32  sys_usw_l2_hw_sync_alloc_ad_idx(uint8 lchip, uint32* ad_index);
extern int32  sys_usw_l2_hw_sync_free_ad_idx(uint8 lchip, uint32 ad_index);
extern int32 sys_usw_ftm_alloc_table_offset_from_position(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);
extern int32 sys_usw_mac_set_mac_rx_en(uint8 lchip, uint32 gport, uint32 value);
extern int32 sys_usw_mac_security_set_fid_learn_limit_action(uint8 lchip, uint16 fid, ctc_maclimit_action_t action);
extern int32 sys_usw_port_set_ingress_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value);
extern int32 sys_usw_egress_vlan_edit_nh_update(uint8 lchip, uint32 nhid, ctc_vlan_egress_edit_info_t* p_vlan_info, ctc_vlan_edit_nh_param_t* p_nh_param);
extern int32 sys_usw_l2_update_station_move_action(uint8 lchip, uint32 ad_index, uint32 type);
#endif

STATIC int32 _ctc_gbx_app_vlan_port_set_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);
STATIC int32 _ctc_gbx_app_vlan_port_get_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg);
STATIC int32 _ctc_gbx_app_vlan_port_update_mac_limit_action(uint8 lchip, uint32 mac_limit_action, ctc_app_vlan_port_gem_port_db_t* p_gem_port_db, uint8 is_add);
extern int32 _ctc_gbx_app_vlan_port_get_fid_by_fid(ctc_app_vlan_port_fid_db_t* p_port_fid);
STATIC int32 _ctc_gbx_app_vlan_port_sync_fid_mac_limit(uint8 lchip, uint16 fid, uint8 enable, uint8* need_add);

uint32
_ctc_gbx_app_vlan_port_fdb_flush_func(ctc_app_vlan_port_fdb_node_t *p_fdb_node, void *user_data)
{
    ctc_app_vlan_port_fdb_flush_t* p_fdb_flush = (ctc_app_vlan_port_fdb_flush_t*)user_data;

    if (((CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT == p_fdb_flush->type)&& (p_fdb_flush->logic_port == p_fdb_node->logic_port))
        || ((CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT_FID == p_fdb_flush->type) && (p_fdb_flush->logic_port == p_fdb_node->logic_port)
        && (p_fdb_flush->fid == p_fdb_node->fid)))
    {
        ctc_app_vlan_port_fid_db_t port_fid_db;
        ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;
        uint8 need_fid_add = 0;
        sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));

        port_fid_db.fid = p_fdb_node->fid;
        _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
        p_port_fid_db = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
        if (p_port_fid_db && p_port_fid_db->limit_count)
        {
            _ctc_gbx_app_vlan_port_sync_fid_mac_limit(0, port_fid_db.fid, FALSE, &need_fid_add);
        }

        mem_free(p_fdb_node);
        return 1;
    }

    return CTC_E_NONE;
}

uint32
_ctc_gbx_app_vlan_port_fdb_flush(uint8 lchip, uint16 fid, uint16 logic_port, uint32 type)
{
    ctc_app_vlan_port_fdb_flush_t fdb_flush;

    sal_memset(&fdb_flush, 0, sizeof(ctc_app_vlan_port_fdb_flush_t));
    fdb_flush.fid = fid;
    fdb_flush.logic_port = logic_port;
    fdb_flush.type = type;
    fdb_flush.lchip = lchip;
    ctc_hash_traverse_remove(p_g_app_vlan_port_master->fdb_entry_hash,
                        (hash_traversal_fn) _ctc_gbx_app_vlan_port_fdb_flush_func, (void *)&fdb_flush);

    return CTC_E_NONE;
}

uint32
_ctc_gbx_app_vlan_port_fdb_hash_make(ctc_app_vlan_port_fdb_node_t* node)
{
    return ctc_hash_caculate(sizeof(mac_addr_t)+sizeof(uint16), &(node->mac));
}

bool
_ctc_gbx_app_vlan_port_fdb_hash_compare(ctc_app_vlan_port_fdb_node_t* stored_node, ctc_app_vlan_port_fdb_node_t* lkup_node)
{
    if (!stored_node || !lkup_node)
    {
        return TRUE;
    }

    if (!sal_memcmp(stored_node->mac, lkup_node->mac, CTC_ETH_ADDR_LEN)
        && (stored_node->fid == lkup_node->fid))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_ctc_gbx_app_vlan_port_fdb_hash_add(uint8 lchip, ctc_l2_addr_t* p_l2_addr, uint32 ad_index)
{
    ctc_app_vlan_port_fdb_node_t* p_fdb_node = NULL;

    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_fdb_node = (ctc_app_vlan_port_fdb_node_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_app_vlan_port_fdb_node_t));
    if (NULL == p_fdb_node)
    {
        return CTC_E_NO_MEMORY;
    }

    sal_memcpy(p_fdb_node->mac, p_l2_addr->mac, sizeof(mac_addr_t));
    p_fdb_node->fid = p_l2_addr->fid;
    p_fdb_node->logic_port = p_l2_addr->gport;
    p_fdb_node->ad_index = ad_index;
    if (NULL == ctc_hash_insert(p_g_app_vlan_port_master->fdb_entry_hash, p_fdb_node))
    {
        mem_free(p_fdb_node);
        return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_fdb_hash_remove(uint8 lchip, ctc_l2_addr_t* p_l2_addr)
{
    ctc_app_vlan_port_fdb_node_t* p_fdb_node = NULL;
    ctc_app_vlan_port_fdb_node_t fdb_node;

    sal_memset(&fdb_node, 0, sizeof(ctc_app_vlan_port_fdb_node_t));
    sal_memcpy(&fdb_node.mac, p_l2_addr->mac, sizeof(mac_addr_t));
    fdb_node.fid = p_l2_addr->fid;
    p_fdb_node = ctc_hash_lookup(p_g_app_vlan_port_master->fdb_entry_hash, &fdb_node);
    if (NULL == p_fdb_node)
    {
        return CTC_E_NOT_EXIST;
    }

    if (!ctc_hash_remove(p_g_app_vlan_port_master->fdb_entry_hash, p_fdb_node))
    {
        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [FDB] Hash remove failed \n");
        return CTC_E_NOT_EXIST;
    }
    mem_free(p_fdb_node);

    return CTC_E_NONE;
}

STATIC ctc_app_vlan_port_fdb_node_t*
_ctc_gbx_app_vlan_port_fdb_hash_lkup(uint8 lchip, ctc_l2_addr_t* p_l2_addr)
{
    ctc_app_vlan_port_fdb_node_t node;

    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&node, 0, sizeof(ctc_app_vlan_port_fdb_node_t));

    node.fid = p_l2_addr->fid;
    sal_memcpy(&node.mac, p_l2_addr->mac, sizeof(mac_addr_t));

    return ctc_hash_lookup(p_g_app_vlan_port_master->fdb_entry_hash, &node);
}

int32
_ctc_gbx_app_vlan_port_alloc_offset(uint8 lchip, uint32* p_offset)
{
    int32 ret = 0;

    ret = CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_alloc_ad_idx, lchip, p_offset);
    CTC_APP_DBG_INFO("Alloc ad index :%d\n", *p_offset);

    return  ret;
}

int32
_ctc_gbx_app_vlan_port_free_offset(uint8 lchip, uint32 offset)
{
    int32 ret = 0;

    ret = CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_free_ad_idx, lchip, offset);
    CTC_APP_DBG_INFO("Free ad index :%d\n", offset);

    return  ret;
}

int32
_ctc_gbx_app_vlan_port_alloc_nhid(uint8 lchip, uint32* p_nhid)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    ctc_get_gchip_id(lchip, &gchip_id);
    sal_memset(&app_index, 0, sizeof(app_index));
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    app_index.entry_num = 1;


    ctc_app_index_alloc(&app_index);

    *p_nhid = app_index.index;

    return 0;
}

int32
_ctc_gbx_app_vlan_port_free_nhid(uint8 lchip, uint32 nhid)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    sal_memset(&app_index, 0, sizeof(app_index));

    ctc_get_gchip_id(lchip, &gchip_id);
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    app_index.entry_num = 1;
    app_index.index = nhid;

    ctc_app_index_free(&app_index);

    return 0;
}

int32
_ctc_gbx_app_vlan_port_match_gem_port(ctc_app_vlan_port_gem_port_db_t *p_gem_port_db,
                                   ctc_app_vlan_port_find_t *p_vlan_port_match)
{
    CTC_PTR_VALID_CHECK(p_vlan_port_match);
    CTC_PTR_VALID_CHECK(p_gem_port_db);

    if (p_gem_port_db->logic_port == p_vlan_port_match->logic_port)
    {
        p_vlan_port_match->p_gem_port_db = p_gem_port_db;
        return -1;
    }

    return 0;
}

ctc_app_vlan_port_gem_port_db_t*
_ctc_gbx_app_vlan_port_find_gem_port_db(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_g_app_vlan_port_master->gem_port_hash,
                      (hash_traversal_fn)_ctc_gbx_app_vlan_port_match_gem_port, &vlan_port_find);

    return vlan_port_find.p_gem_port_db;
}

int32
_ctc_gbx_app_vlan_port_match_nni_port(ctc_app_vlan_port_nni_port_db_t *p_nni_port_db,
                                   ctc_app_vlan_port_find_t *p_vlan_port_match)
{
    CTC_PTR_VALID_CHECK(p_vlan_port_match);
    CTC_PTR_VALID_CHECK(p_nni_port_db);

    if (p_nni_port_db->nni_logic_port == p_vlan_port_match->logic_port)
    {
        p_vlan_port_match->p_nni_port_db = p_nni_port_db;
        return -1;
    }

    return 0;
}

ctc_app_vlan_port_nni_port_db_t*
_ctc_gbx_app_vlan_port_find_nni_port(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_g_app_vlan_port_master->nni_port_hash,
                      (hash_traversal_fn)_ctc_gbx_app_vlan_port_match_nni_port, &vlan_port_find);

    return vlan_port_find.p_nni_port_db;
}

int32
_ctc_gbx_app_vlan_port_match_vlan_port(ctc_app_vlan_port_db_t *p_vlan_port_db,
                                   ctc_app_vlan_port_find_t *p_vlan_port_match)
{
    CTC_PTR_VALID_CHECK(p_vlan_port_match);
    CTC_PTR_VALID_CHECK(p_vlan_port_db);

    if (p_vlan_port_db->logic_port == p_vlan_port_match->logic_port)
    {
        p_vlan_port_match->p_vlan_port_db = p_vlan_port_db;
        return -1;
    }

    return 0;
}

ctc_app_vlan_port_db_t*
_ctc_gbx_app_vlan_port_find_vlan_port_db(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_hash,
                      (hash_traversal_fn)_ctc_gbx_app_vlan_port_match_vlan_port, &vlan_port_find);

    return vlan_port_find.p_vlan_port_db;
}

STATIC uint32
_ctc_gbx_app_vlan_port_hash_nni_port_make(ctc_app_vlan_port_nni_port_db_t *p_nni_port_db)
{
    uint32 data;
    uint32 length = 0;

    data = p_nni_port_db->port;
    length = sizeof(uint32);

    return ctc_hash_caculate(length, (uint8*)&data);
}

STATIC bool
_ctc_gbx_app_vlan_port_hash_nni_port_cmp(ctc_app_vlan_port_nni_port_db_t *p_data0,
                                     ctc_app_vlan_port_nni_port_db_t *p_data1)
{
    if (p_data0->port == p_data1->port)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_gbx_app_vlan_port_hash_gem_port_make(ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
    uint32 data[2] = {0};
    uint32 length = 0;

    data[0] = p_gem_port_db->port;
    data[1] = p_gem_port_db->tunnel_value;
    length = sizeof(uint32)*2;

    return ctc_hash_caculate(length, (uint8*)data);
}

STATIC bool
_ctc_gbx_app_vlan_port_hash_gem_port_cmp(ctc_app_vlan_port_gem_port_db_t *p_data0,
                                     ctc_app_vlan_port_gem_port_db_t *p_data1)
{
    if (p_data0->port == p_data1->port &&
        p_data0->tunnel_value == p_data1->tunnel_value)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_gbx_app_vlan_port_hash_make(ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    uint32 data;
    uint32 length = 0;

    data = p_vlan_port_db->vlan_port_id;

    length = sizeof(uint32);

    return ctc_hash_caculate(length, (uint8*)&data);
}

STATIC bool
_ctc_gbx_app_vlan_port_hash_cmp(ctc_app_vlan_port_db_t *p_data0,
                            ctc_app_vlan_port_db_t *p_data1)
{
    if (p_data0->vlan_port_id == p_data1->vlan_port_id)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_gbx_app_vlan_port_key_hash_make(ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    uint32 length = 0;

    length = CTC_OFFSET_OF(ctc_app_vlan_port_db_t, calc_key_len);

    return ctc_hash_caculate(length, (uint8*)p_vlan_port_db);
}

STATIC bool
_ctc_gbx_app_vlan_port_key_hash_cmp(ctc_app_vlan_port_db_t *p_data0,
                            ctc_app_vlan_port_db_t *p_data1)
{
    uint32 length = 0;

    length = CTC_OFFSET_OF(ctc_app_vlan_port_db_t, calc_key_len);

    if (0 == sal_memcmp(p_data0, p_data1, length))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_gbx_app_vlan_port_logic_vlan_hash_make(ctc_app_vlan_port_sync_db_t *p_sync_db)
{
    uint32 length = 0;
    uint32 data[2] = {0};

    data[0] = p_sync_db->logic_port;
    data[1] = (( p_sync_db->match_svlan << 16) | p_sync_db->match_cvlan);

    length = sizeof(data);

    return ctc_hash_caculate(length, (uint8*)data);
}

STATIC bool
_ctc_gbx_app_vlan_port_logic_vlan_hash_cmp(ctc_app_vlan_port_sync_db_t *p_data0,
                                       ctc_app_vlan_port_sync_db_t *p_data1)
{
    if (p_data0->logic_port == p_data1->logic_port &&
        p_data0->match_svlan == p_data1->match_svlan &&
    p_data0->match_cvlan == p_data1->match_cvlan)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_gbx_app_vlan_port_fid_hash_make(ctc_app_vlan_port_fid_db_t* p_port_fid)
{
    uint32 length = 0;
    uint16 data[4] = {0};

    data[0] = p_port_fid->pkt_svlan;
    data[1] = p_port_fid->pkt_cvlan;
    data[2] = p_port_fid->vdev_id;
    data[3] = p_port_fid->is_flex;

    length = sizeof(data);

    return ctc_hash_caculate(length, (uint8*)data);
}

STATIC bool
_ctc_gbx_app_vlan_port_fid_hash_cmp(ctc_app_vlan_port_fid_db_t* p_data0,
                                       ctc_app_vlan_port_fid_db_t* p_data1)
{
    if ((p_data0->pkt_svlan == p_data1->pkt_svlan)
        && (p_data0->pkt_cvlan == p_data1->pkt_cvlan)
        && (p_data0->vdev_id == p_data1->vdev_id)
        && (p_data0->is_flex == p_data1->is_flex))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC bool
_ctc_gbx_app_vlan_port_fid_build_index(ctc_app_vlan_port_fid_db_t* p_port_fid, uint8* p_lchip)
{
    uint32 fid = 0;
    ctc_opf_t opf;
    ctc_scl_entry_t scl_entry;
    ctc_l2dflt_addr_t l2dflt;
    ctc_vlan_uservlan_t user_vlan;

    CTC_PTR_VALID_CHECK(p_port_fid);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;

    CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, &fid));
    p_port_fid->fid = fid;
    p_port_fid->limit_num = CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE;

    sal_memset(&user_vlan, 0, sizeof(user_vlan));
    user_vlan.vlan_id = fid;
    user_vlan.user_vlanptr = fid;
    user_vlan.fid = fid;
    CTC_SET_FLAG(user_vlan.flag, CTC_MAC_LEARN_USE_LOGIC_PORT);
    CTC_ERROR_RETURN(ctc_vlan_create_uservlan(&user_vlan));
    CTC_ERROR_RETURN(ctc_vlan_set_property(fid, CTC_VLAN_PROP_FID, fid));
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = fid;
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    CTC_ERROR_RETURN(ctc_l2_add_default_entry(&l2dflt));

    /*Add BC member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = fid;
    l2dflt.remote_chip = 1;
    l2dflt.member.mem_port = CTC_APP_VLAN_PORT_RSV_MCAST_GROUP_ID(p_port_fid->vdev_id);
    CTC_ERROR_RETURN(ctc_l2_add_port_to_default_entry(&l2dflt));

    if (p_port_fid->is_flex)
    {
        return CTC_E_NONE;
    }

    if ((p_port_fid->pkt_svlan != 0 && p_port_fid->pkt_cvlan != 0)
        || (p_port_fid->pkt_svlan == 0 && p_port_fid->pkt_cvlan == 0))
    {
        sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
        scl_entry.entry_id = ENCODE_SCL_DOWNLINK_ENTRY_ID(fid, 0);
        scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
        scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
        scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT_CLASS;
        scl_entry.key.u.hash_port_2vlan_key.gport = CTC_APP_VLAN_PORT_NNI_CLASS_ID;
        if ((0 == p_port_fid->pkt_svlan) && (0 != p_port_fid->pkt_cvlan))
        {
            scl_entry.key.u.hash_port_2vlan_key.svlan = p_port_fid->pkt_cvlan;
            scl_entry.key.u.hash_port_2vlan_key.cvlan = p_port_fid->pkt_svlan;
        }
        else
        {
            scl_entry.key.u.hash_port_2vlan_key.svlan = p_port_fid->pkt_svlan;
            scl_entry.key.u.hash_port_2vlan_key.cvlan = p_port_fid->pkt_cvlan;
        }
        scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
        CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
        CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
        scl_entry.action.u.igs_action.fid = fid;
        scl_entry.action.u.igs_action.user_vlanptr = fid;
        if(CTC_E_HASH_CONFLICT == ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry))
        {
#ifdef DUET2
            CTC_APP_DBG_INFO("Hash Confict!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            scl_entry.resolve_conflict = 1;
            CTC_ERROR_RETURN(ctc_scl_add_entry(CTC_SCL_GROUP_ID_TCAM0, &scl_entry));
#else
            return CTC_E_HASH_CONFLICT;
#endif            
        }
        CTC_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id));
        CTC_APP_DBG_INFO("downlink double vlan pkt_svlan %d pkt_cvlan %d entry_id %d\n", p_port_fid->pkt_svlan, p_port_fid->pkt_cvlan, scl_entry.entry_id);
    }
    else
    {
        sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
        scl_entry.entry_id = ENCODE_SCL_DOWNLINK_ENTRY_ID(fid, 0);
        scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        scl_entry.key.u.hash_port_svlan_key.dir = CTC_INGRESS;
        scl_entry.key.u.hash_port_svlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT_CLASS;
        scl_entry.key.u.hash_port_svlan_key.gport = CTC_APP_VLAN_PORT_NNI_CLASS_ID;
        if ((0 == p_port_fid->pkt_svlan) && (0 != p_port_fid->pkt_cvlan))
        {
            scl_entry.key.u.hash_port_2vlan_key.svlan = p_port_fid->pkt_cvlan;
        }
        else
        {
            scl_entry.key.u.hash_port_2vlan_key.svlan = p_port_fid->pkt_svlan;
        }
        scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
        CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
        CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
        scl_entry.action.u.igs_action.fid = fid;
        scl_entry.action.u.igs_action.user_vlanptr = fid;
        if(CTC_E_HASH_CONFLICT == ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_SVLAN, &scl_entry))
        {
#ifdef DUET2
            CTC_APP_DBG_INFO("Hash Confict!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            scl_entry.resolve_conflict = 1;
            CTC_ERROR_RETURN(ctc_scl_add_entry(CTC_SCL_GROUP_ID_TCAM1, &scl_entry));
#else
            return CTC_E_HASH_CONFLICT;
#endif            
        }
        CTC_ERROR_RETURN(ctc_scl_install_entry(scl_entry.entry_id));
        CTC_APP_DBG_INFO("downlink svlan pkt_svlan %d entry_id %d\n", p_port_fid->pkt_svlan, scl_entry.entry_id);
    }
    return CTC_E_NONE;
}

/* free index of HASH ad */
STATIC bool
_ctc_gbx_app_vlan_port_fid_free_index(ctc_app_vlan_port_fid_db_t* p_port_fid, uint8* p_lchip)
{
    ctc_opf_t opf;
    uint32 entry_id = 0;
    ctc_l2dflt_addr_t l2dflt;

    CTC_PTR_VALID_CHECK(p_port_fid);

    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = p_port_fid->fid;
    l2dflt.l2mc_grp_id = p_port_fid->fid;
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    ctc_l2_remove_default_entry(&l2dflt);
    ctc_vlan_destroy_vlan(p_port_fid->fid);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;
    CTC_ERROR_RETURN(ctc_opf_free_offset(&opf, 1, p_port_fid->fid));

    if (p_port_fid->is_flex)
    {
        return CTC_E_NONE;
    }

    entry_id = ENCODE_SCL_DOWNLINK_ENTRY_ID(p_port_fid->fid, 0);
    CTC_ERROR_RETURN(ctc_scl_uninstall_entry(entry_id));
    CTC_ERROR_RETURN(ctc_scl_remove_entry(entry_id));

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_set_xc_port_discard(uint8 lchip, uint32 src_gport, uint32 dst_gport, uint8 discard)
{
    uint8 chip_type = 0;
    int32 ret = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_scl_entry_t  scl_entry;

    if (discard)
    {
        sal_memset(&scl_entry, 0, sizeof(scl_entry));
        chip_type = ctc_get_chip_type();
        switch(chip_type)
        {
#ifdef GREATBELT
            case CTC_CHIP_GREATBELT:
                scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_IPV4;
                scl_entry.key.u.hash_port_ipv4_key.gport_type = CTC_SCL_GPROT_TYPE_PORT;
                scl_entry.key.u.hash_port_ipv4_key.gport = src_gport;
                scl_entry.key.u.hash_port_ipv4_key.ip_sa = dst_gport;
                scl_entry.key.u.hash_port_ipv4_key.rsv0 = 1;

                scl_entry.action.type =  CTC_SCL_ACTION_EGRESS;

                scl_entry.entry_id = ENCODE_SCL_XC_ENTRY_ID(src_gport*100 + dst_gport);
                CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_KNOW_UCAST);
                CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_KNOW_MCAST);
                CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_UCAST);
                CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_MCAST);
                CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_BCAST);

                CTC_APP_DBG_INFO("EntryId :%d\n", scl_entry.entry_id );
                CTC_ERROR_GOTO(ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_IPV4, &scl_entry), ret, roll_back_0);
                CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_1);
                break;
#endif
#ifdef DUET2
            case CTC_CHIP_DUET2:
                {
                    ctc_field_key_t field_key;
                    ctc_field_port_t field_port;
                    ctc_scl_field_action_t field_action;
                    scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_CROSS;
                    scl_entry.mode = 1;
                    scl_entry.action_type =  CTC_SCL_ACTION_EGRESS;
                    scl_entry.entry_id = ENCODE_SCL_XC_ENTRY_ID(src_gport*100 + dst_gport);
                    CTC_ERROR_GOTO(ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_IPV4, &scl_entry), ret, roll_back_0);
                    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
                    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
                    field_key.type = CTC_FIELD_KEY_PORT;
                    field_port.gport = src_gport;
                    field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                    field_key.ext_data = &field_port;
                    CTC_ERROR_GOTO(ctc_scl_add_key_field(scl_entry.entry_id, &field_key), ret, roll_back_1);
                    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
                    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
                    field_key.type = CTC_FIELD_KEY_DST_GPORT;
                    field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                    field_port.gport = dst_gport;
                    field_key.ext_data = &field_port;
                    CTC_ERROR_GOTO(ctc_scl_add_key_field(scl_entry.entry_id, &field_key), ret, roll_back_1);
                    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
                    field_key.type = CTC_FIELD_KEY_HASH_VALID;
                    CTC_ERROR_GOTO(ctc_scl_add_key_field(scl_entry.entry_id, &field_key), ret, roll_back_1);
                    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
                    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST;
                    CTC_ERROR_GOTO(ctc_scl_add_action_field(scl_entry.entry_id, &field_action), ret, roll_back_1);
                    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST;
                    CTC_ERROR_GOTO(ctc_scl_add_action_field(scl_entry.entry_id, &field_action), ret, roll_back_1);
                    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST;
                    CTC_ERROR_GOTO(ctc_scl_add_action_field(scl_entry.entry_id, &field_action), ret, roll_back_1);
                    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST;
                    CTC_ERROR_GOTO(ctc_scl_add_action_field(scl_entry.entry_id, &field_action), ret, roll_back_1);
                    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST;
                    CTC_ERROR_GOTO(ctc_scl_add_action_field(scl_entry.entry_id, &field_action), ret, roll_back_1);
                    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_1);
                }
                break;
#endif
            default:
                return CTC_E_INVALID_PARAM;
        }
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_EGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_XC;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(dst_gport, &scl_prop), ret, roll_back_2);
    }
    else
    {
        sal_memset(&scl_entry, 0, sizeof(scl_entry));
        scl_entry.entry_id = ENCODE_SCL_XC_ENTRY_ID(src_gport*100 + dst_gport);
        CTC_APP_DBG_INFO("EntryId :%d\n", scl_entry.entry_id );
        CTC_ERROR_GOTO(ctc_scl_uninstall_entry(scl_entry.entry_id), ret, roll_back_0);
        CTC_ERROR_GOTO(ctc_scl_remove_entry(scl_entry.entry_id), ret, roll_back_0);
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_EGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(dst_gport, &scl_prop), ret, roll_back_0);
    }

    return CTC_E_NONE;

roll_back_2:
    ctc_scl_uninstall_entry(scl_entry.entry_id);

roll_back_1:
    ctc_scl_remove_entry(scl_entry.entry_id);
roll_back_0:

    return ret;
}

int32
ctc_gbx_app_vlan_port_set_uni_inner_isolate(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg)
{
    uint8 gchip = 0;
    uint32 lport = 0;
    ctc_port_restriction_t isolation;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint8 discard = 0;

    ctc_get_gchip_id(lchip, &gchip);

    discard = p_glb_cfg->value?1:0;

    if (p_g_app_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id] == discard)
    {
        return CTC_E_NONE;
    }

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

        if ((0 == p_uni_port->iloop_port) || (p_uni_port->vdev_id != p_glb_cfg->vdev_id))
        {
            continue;
        }

        if (p_glb_cfg->value)
        {
            /***********************************************/
            /** PORT ISOLATION*/
            sal_memset(&isolation, 0, sizeof(isolation));
            isolation.dir = CTC_EGRESS;
            isolation.isolated_id  = p_uni_port->vlan_range_grp + 1;
            isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
            isolation.type = CTC_PORT_ISOLATION_ALL;
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->e2eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->onu_e2eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->mc_eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->bc_eloop_port, &isolation));
            isolation.dir = CTC_INGRESS;
            CTC_ERROR_RETURN(ctc_port_set_restriction(CTC_MAP_LPORT_TO_GPORT(gchip, lport), &isolation));
        }
        else
        {
            /***********************************************/
            /** PORT ISOLATION*/
            sal_memset(&isolation, 0, sizeof(isolation));
            isolation.dir = CTC_EGRESS;
            isolation.isolated_id  = 0;
            isolation.mode = CTC_PORT_RESTRICTION_DISABLE;
            isolation.type = CTC_PORT_ISOLATION_ALL;
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->e2eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->onu_e2eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->mc_eloop_port, &isolation));
            CTC_ERROR_RETURN(ctc_port_set_restriction(p_uni_port->bc_eloop_port, &isolation));

            isolation.dir = CTC_INGRESS;
            CTC_ERROR_RETURN(ctc_port_set_restriction(CTC_MAP_LPORT_TO_GPORT(gchip, lport), &isolation));
        }
        /*PON and passthrough inner isolate*/
        CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->iloop_port, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, p_glb_cfg->value? 0: 1));
        CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN, p_glb_cfg->value? 0: 1));
    }

    p_g_app_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id] = p_glb_cfg->value?1:0;

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_set_uni_outer_isolate(uint8 lchip,
                                        ctc_app_global_cfg_t *p_glb_cfg)
{
    uint8 gchip = 0;
    uint8 discard = 0;
    uint32 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port_src = NULL;
    uint16 uni_port[64] = {0};
    uint8 idx = 0;
    uint8 idx2 = 0;
    uint8 idx3 = 0;

    ctc_get_gchip_id(lchip, &gchip);

    discard = p_glb_cfg->value?1:0;

    if (p_g_app_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id] == discard)
    {
        return CTC_E_NONE;
    }

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

        if ((0 == p_uni_port->iloop_port) || (p_uni_port->vdev_id != p_glb_cfg->vdev_id))
        {
            continue;
        }

        uni_port[idx++] = lport;
    }

    /*destination*/
    for (idx2 = 0; idx2 < idx; idx2++)
    {
        p_uni_port_src = &p_g_app_vlan_port_master->p_port_pon[uni_port[idx2]];

        for (idx3 = 0; idx3 < idx; idx3++)
        {
            if ((idx2 == idx3) || (p_uni_port_src->vdev_id != p_glb_cfg->vdev_id))
            {
                continue;
            }

#if 0
            CTC_ERROR_RETURN(ctc_app_vlan_port_set_xc_port_discard(lchip,
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_src->iloop_port[CTC_APP_VLAN_PORT_MAC_LMT_NONE]),
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_dst->e2eloop_port),
                                                                   discard));
            CTC_ERROR_RETURN(ctc_app_vlan_port_set_xc_port_discard(lchip,
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_src->iloop_port[CTC_APP_VLAN_PORT_MAC_LMT_NONE]),
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_dst->onu_e2eloop_port),
                                                                   discard));
            CTC_ERROR_RETURN(ctc_app_vlan_port_set_xc_port_discard(lchip,
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_src->iloop_port[CTC_APP_VLAN_PORT_MAC_LMT_NONE]),
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_dst->bc_eloop_port),
                                                                   discard));
            CTC_ERROR_RETURN(ctc_app_vlan_port_set_xc_port_discard(lchip,
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_src->iloop_port[CTC_APP_VLAN_PORT_MAC_LMT_NONE]),
                                                                   CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port_dst->mc_eloop_port),
                                                                   discard));
#else
            CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_xc_port_discard(lchip,
                                                                    p_uni_port_src->iloop_port,
                                                                    CTC_MAP_LPORT_TO_GPORT(gchip, uni_port[idx3]),
                                                                    discard));
            CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_xc_port_discard(lchip,
                                                                    p_uni_port_src->iloop_sec_port,
                                                                    CTC_MAP_LPORT_TO_GPORT(gchip, uni_port[idx3]),
                                                                    discard));
#endif
        }

    }

    p_g_app_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id] = p_glb_cfg->value?1:0;

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_set_unknown_mcast_drop(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg)
{
    uint32 lport = 0;
    uint16 vlan_id = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint8 drop_en  = 0;

    drop_en = p_glb_cfg->value?1:0;

    if (p_g_app_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id] == drop_en)
    {
        return CTC_E_NONE;
    }

    vlan_id = p_g_app_vlan_port_master->default_bcast_fid[p_glb_cfg->vdev_id];

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

        if ((0 == p_uni_port->iloop_port) || (p_uni_port->vdev_id != p_glb_cfg->vdev_id))
        {
            continue;
        }

        if (drop_en)
        {
            /*Remove MC member*/
            sal_memset(&l2dflt, 0, sizeof(l2dflt));
            l2dflt.fid = vlan_id;
            l2dflt.l2mc_grp_id = p_glb_cfg->vdev_id;
            l2dflt.with_nh = 1;
            l2dflt.member.nh_id = p_uni_port->mc_e2iloop_nhid;
            CTC_ERROR_RETURN(ctc_l2_remove_port_from_default_entry(&l2dflt));

        }
        else
        {
            /*Add MC member*/
            sal_memset(&l2dflt, 0, sizeof(l2dflt));
            l2dflt.fid = vlan_id;
            l2dflt.l2mc_grp_id = p_glb_cfg->vdev_id;
            l2dflt.with_nh = 1;
            l2dflt.member.nh_id = p_uni_port->mc_e2iloop_nhid;
            CTC_ERROR_RETURN(ctc_l2_add_port_to_default_entry(&l2dflt));
        }
    }

    p_g_app_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id] = p_glb_cfg->value?1:0;

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_set_leaf_by_fid(void* node, void* user_data)
{
    uint32 entry_id = 0;
    ctc_scl_field_action_t field_action;
    ctc_app_vlan_port_fid_db_t* p_port_fid = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = (ctc_app_vlan_port_fid_db_t*)user_data;

    if (p_port_fid->pkt_svlan == p_port_fid_temp->pkt_svlan)
    {
        entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(p_port_fid->fid, 0);
        sal_memset(&field_action, 0, sizeof(field_action));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;
        p_port_fid_temp->rsv1? ctc_scl_add_action_field(entry_id, &field_action): ctc_scl_remove_action_field(entry_id, &field_action);
        CTC_APP_DBG_INFO("vlan isolate :%s\n", p_port_fid_temp->rsv1? "add": "remove");
    }
    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_set_leaf_by_svlan(void* bucket_data, void* user_data)
{
    uint32 vlan_mapping_port = 0;
    uint32 lport = 0;
    uint32* p_user_data = (uint32*)user_data;
    uint16 pkt_svlan = p_user_data[0];
    uint16 is_add = p_user_data[1];
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_app_vlan_port_db_t* p_vlan_port_db = (ctc_app_vlan_port_db_t*)bucket_data;
    ctc_vlan_mapping_t vlan_mapping;

    if ((p_vlan_port_db->match_svlan != pkt_svlan) || (p_vlan_port_db->match_svlan_end))
    {
        return CTC_E_NONE;
    }
    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];
    sal_memset(&vlan_mapping, 0 , sizeof(vlan_mapping));
    if (p_vlan_port_db->tunnel_value)
    {
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        vlan_mapping_port = p_vlan_port_db->p_gem_port_db->logic_port;
    }
    else
    {
        vlan_mapping_port = p_uni_port->iloop_port;
    }
    if (p_vlan_port_db->igs_vlan_mapping_use_flex)
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    }
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.old_svid = p_vlan_port_db->match_svlan;
    vlan_mapping.old_cvid = p_vlan_port_db->match_cvlan;
/* vlan range not support yet
    if (p_vlan_port_db->match_svlan_end)
    {
        vlan_mapping.svlan_end = p_vlan_port_db->match_svlan_end;
        vlan_mapping.vrange_grpid = p_uni_port->vlan_range_grp;
    }
*/
    CTC_ERROR_RETURN(ctc_vlan_get_vlan_mapping(vlan_mapping_port, &vlan_mapping));
    if (is_add)
    {
        CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF);
    }
    else
    {
        CTC_UNSET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF);
    }
    CTC_ERROR_RETURN(ctc_vlan_update_vlan_mapping(vlan_mapping_port, &vlan_mapping));
    return CTC_E_NONE;
}


STATIC int32
_ctc_gbx_app_vlan_port_set_vlan_isolate(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg)
{
    uint8 enable = p_glb_cfg->value? TRUE: FALSE;
    uint8 vlan_is_set = CTC_BMP_ISSET(p_g_app_vlan_port_master->vlan_isolate_bmp, p_glb_cfg->vlan_id)? 1: 0;
    uint32 user_data[2] = {0};
    ctc_app_vlan_port_fid_db_t port_fid_db;
    CTC_VLAN_RANGE_CHECK(p_glb_cfg->vlan_id);

    if (enable == vlan_is_set)
    {
        return CTC_E_NONE;
    }
    /*Passthrough update*/
    sal_memset(&port_fid_db, 0, sizeof(port_fid_db));
    port_fid_db.pkt_svlan = p_glb_cfg->vlan_id;
    port_fid_db.rsv1 = enable;  /*here rsv1 used as enable*/
    ctc_spool_traverse(p_g_app_vlan_port_master->fid_spool, _ctc_gbx_app_vlan_port_set_leaf_by_fid, &port_fid_db);
    user_data[0] = p_glb_cfg->vlan_id;
    user_data[1] = enable;
    ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_hash, _ctc_gbx_app_vlan_port_set_leaf_by_svlan, user_data);
    if (enable)
    {
        CTC_BMP_SET(p_g_app_vlan_port_master->vlan_isolate_bmp, p_glb_cfg->vlan_id);
    }
    else
    {
        CTC_BMP_UNSET(p_g_app_vlan_port_master->vlan_isolate_bmp, p_glb_cfg->vlan_id);
    }
    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg)
{
    CTC_PTR_VALID_CHECK(p_glb_cfg);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_glb_cfg->vdev_id);

    switch(p_glb_cfg->cfg_type)
    {
    case CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE:
        CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_uni_outer_isolate(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE:
        CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_uni_inner_isolate(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP:
        CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_unknown_mcast_drop(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT:
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_set_mac_learn_limit(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_VLAN_ISOLATE:
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_set_vlan_isolate(lchip, p_glb_cfg));
        break;

    default:
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t *p_glb_cfg)
{
    CTC_PTR_VALID_CHECK(p_glb_cfg);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_glb_cfg->vdev_id);

    switch(p_glb_cfg->cfg_type)
    {
        case CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE:
            p_glb_cfg->value = p_g_app_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id];
            break;

        case CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE:
            p_glb_cfg->value = p_g_app_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id];
            break;

        case CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP:
            p_glb_cfg->value = p_g_app_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id];
            break;

        case CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT:
            CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_get_mac_learn_limit(lchip, p_glb_cfg));
            break;

        case CTC_APP_VLAN_CFG_VLAN_ISOLATE:
            CTC_VLAN_RANGE_CHECK(p_glb_cfg->vlan_id);
            p_glb_cfg->value = CTC_BMP_ISSET(p_g_app_vlan_port_master->vlan_isolate_bmp, p_glb_cfg->vlan_id)? 1: 0;
            break;

        default:
            return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t *p_vlan_port)
{
    ctc_app_vlan_port_db_t *p_vlan_port_find = NULL;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_find = NULL;
    ctc_app_vlan_port_nni_port_db_t *p_nni_port_find = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();

    p_vlan_port_find = _ctc_gbx_app_vlan_port_find_vlan_port_db(lchip, logic_port);

    if (p_vlan_port_find)
    {
        p_vlan_port->vlan_port_id = p_vlan_port_find->vlan_port_id;
        p_vlan_port->criteria = p_vlan_port_find->criteria;
        p_vlan_port->port = p_vlan_port_find->port;
        p_vlan_port->match_tunnel_value = p_vlan_port_find->tunnel_value;
        p_vlan_port->match_svlan = p_vlan_port_find->match_svlan;
        p_vlan_port->match_cvlan = p_vlan_port_find->match_cvlan;
        p_vlan_port->match_svlan_end = p_vlan_port_find->match_svlan_end;
        p_vlan_port->match_cvlan_end = p_vlan_port_find->match_cvlan_end;
        sal_memcpy(&p_vlan_port->flex_key, &p_vlan_port_find->flex_key, sizeof(ctc_acl_key_t));
        sal_memcpy(&p_vlan_port->ingress_vlan_action_set, &p_vlan_port_find->ingress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));
        sal_memcpy(&p_vlan_port->egress_vlan_action_set, &p_vlan_port_find->egress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));
        p_vlan_port->logic_port = p_vlan_port_find->logic_port;
        p_vlan_port->flex_nhid = p_vlan_port_find->flex_add_vlan_xlate_nhid;

        return CTC_E_NONE;
    }

    p_gem_port_find = _ctc_gbx_app_vlan_port_find_gem_port_db(lchip, logic_port);
    if (p_gem_port_find)
    {
        p_vlan_port->port = p_gem_port_find->port;
        p_vlan_port->match_tunnel_value = p_gem_port_find->tunnel_value;
        p_vlan_port->logic_port = p_gem_port_find->logic_port;
        return CTC_E_NONE;
    }

    p_nni_port_find = _ctc_gbx_app_vlan_port_find_nni_port(lchip, logic_port);
    if (p_nni_port_find)
    {
        p_vlan_port->port = p_nni_port_find->port;
        return CTC_E_NONE;
    }

    return CTC_E_NOT_EXIST;
}

int32
ctc_gbx_app_vlan_port_add_sync_db(uint8 lchip, ctc_app_vlan_port_db_t *p_vlan_port_db, ctc_app_vlan_port_match_t criteria)
{
    ctc_app_vlan_port_sync_db_t sync_db;
    ctc_app_vlan_port_sync_db_t *p_sync_db = NULL;
    sal_memset(&sync_db, 0, sizeof(sync_db));

    sync_db.vdev_id = p_vlan_port_db->vdev_id;
    sync_db.logic_port = p_vlan_port_db->logic_port;
    if (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == criteria)
    {
        sync_db.match_svlan = p_vlan_port_db->fid;
        sync_db.match_cvlan = p_vlan_port_db->pkt_cvlan;
    }
    else
    {
        sync_db.match_svlan = p_vlan_port_db->match_svlan;
        sync_db.match_cvlan = p_vlan_port_db->match_cvlan;
    }

    p_sync_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash, &sync_db);
    if (NULL != p_sync_db)
    {
        return CTC_E_NONE;
    }

    /* Build node */
    MALLOC_POINTER(ctc_app_vlan_port_sync_db_t, p_sync_db);
    if (NULL == p_sync_db)
    {
        return CTC_E_NO_MEMORY;
    }

    p_sync_db->vdev_id = p_vlan_port_db->vdev_id;
    p_sync_db->logic_port = p_vlan_port_db->logic_port;
    if (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == criteria)
    {
        p_sync_db->match_svlan = p_vlan_port_db->fid;
        p_sync_db->match_cvlan = p_vlan_port_db->pkt_cvlan;
    }
    else
    {
        p_sync_db->match_svlan = p_vlan_port_db->match_svlan;
        p_sync_db->match_cvlan = p_vlan_port_db->match_cvlan;
    }

    p_sync_db->oun_nhid = p_vlan_port_db->xlate_nhid;
    p_sync_db->ad_index = p_vlan_port_db->ad_index;

    ctc_hash_insert(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash, p_sync_db);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_remove_sync_db(uint8 lchip, ctc_app_vlan_port_db_t *p_vlan_port_db, ctc_app_vlan_port_match_t criteria)
{
    ctc_app_vlan_port_sync_db_t sync_db;
    ctc_app_vlan_port_sync_db_t *p_sync_db = NULL;
    sal_memset(&sync_db, 0, sizeof(sync_db));

    sync_db.vdev_id = p_vlan_port_db->vdev_id;
    sync_db.logic_port = p_vlan_port_db->logic_port;
    if (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == criteria)
    {
        sync_db.match_svlan = p_vlan_port_db->fid;
        sync_db.match_cvlan = p_vlan_port_db->pkt_cvlan;
    }
    else
    {
        sync_db.match_svlan = p_vlan_port_db->match_svlan;
        sync_db.match_cvlan = p_vlan_port_db->match_cvlan;
    }
    p_sync_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash, &sync_db);
    if (NULL == p_sync_db)
    {
        return CTC_E_NONE;
    }

    ctc_hash_remove(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash, p_sync_db);
    mem_free(p_sync_db);

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_compare_fid(void* node, void* user_data)
{
    ctc_app_vlan_port_fid_db_t* p_port_fid = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    ctc_app_vlan_port_fid_db_t* p_port_fid_data = (ctc_app_vlan_port_fid_db_t*)user_data;

    if (p_port_fid->fid == p_port_fid_data->fid)
    {
        p_port_fid_data->vdev_id = p_port_fid->vdev_id;
        p_port_fid_data->pkt_svlan = p_port_fid->pkt_svlan;
        p_port_fid_data->pkt_cvlan = p_port_fid->pkt_cvlan;
        p_port_fid_data->is_flex = p_port_fid->is_flex;
        p_port_fid_data->rsv = p_port_fid->rsv;
        p_port_fid_data->fid = p_port_fid->fid;
        p_port_fid_data->limit_action = p_port_fid->limit_action;
        p_port_fid_data->limit_count = p_port_fid->limit_count;
        p_port_fid_data->limit_num = p_port_fid->limit_num;
        return -1;
    }
    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_get_fid_by_fid(ctc_app_vlan_port_fid_db_t* p_port_fid)
{
    CTC_ERROR_RETURN(ctc_spool_traverse(p_g_app_vlan_port_master->fid_spool,
                          (spool_traversal_fn)_ctc_gbx_app_vlan_port_compare_fid, p_port_fid));

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    int ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_port_fid);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_port_fid->vdev_id);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id",      p_port_fid->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "packet svlan",         p_port_fid->pkt_svlan);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "packet cvlan",         p_port_fid->pkt_cvlan);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    if (p_port_fid->fid)
    {
        port_fid_db.fid = p_port_fid->fid;
        ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
        if (!ret)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }
        CTC_APP_VLAN_PORT_FID_COPY(p_port_fid, &port_fid_db);
    }
    else
    {
        port_fid_db.fid = p_port_fid->fid;
        port_fid_db.vdev_id = p_port_fid->vdev_id;
        port_fid_db.pkt_cvlan = p_port_fid->pkt_cvlan;
        port_fid_db.pkt_svlan = p_port_fid->pkt_svlan;
        port_fid_db.is_flex = p_port_fid->is_flex;
        port_fid_db.rsv = p_port_fid->rsv;
        p_port_fid_temp = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);

        if (NULL == p_port_fid_temp)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }
        p_port_fid->fid = p_port_fid_temp->fid;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_INFO("Fid: %d!\n", p_port_fid->fid);

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_add_uplink_svlan_entry(uint8 lchip, uint16 port, uint16 pkt_svlan, uint16 pkt_cvlan, uint16 fid)
{
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(fid, 0);
    if (0 == pkt_cvlan)
    {
        scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        scl_entry.key.u.hash_port_svlan_key.dir = CTC_INGRESS;
        scl_entry.key.u.hash_port_svlan_key.gport_type = CTC_SCL_GPROT_TYPE_LOGIC_PORT;
        scl_entry.key.u.hash_port_svlan_key.gport = port;
        scl_entry.key.u.hash_port_svlan_key.svlan = pkt_svlan;
    }
    else
    {
        scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
        scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
        scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_LOGIC_PORT;
        scl_entry.key.u.hash_port_2vlan_key.gport = port;
        scl_entry.key.u.hash_port_2vlan_key.svlan = pkt_svlan;
        scl_entry.key.u.hash_port_2vlan_key.cvlan = pkt_cvlan;
    }

    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
    scl_entry.action.u.igs_action.fid = fid;
    scl_entry.action.u.igs_action.user_vlanptr = fid;
    if (CTC_BMP_ISSET(p_g_app_vlan_port_master->vlan_isolate_bmp, pkt_svlan))
    {
        CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_ETREE_LEAF);
    }
    if (0 == pkt_cvlan)
    {
        ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_SVLAN, &scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry);
    }
    if (ret)
    {
        if (ret == CTC_E_EXIST)
        {
            ret = CTC_E_NONE;
        }
        goto roll_back_0;
    }
    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_0);
    CTC_APP_DBG_INFO("add uplink svlan port %d pkt_svlan %d pkt_cvlan %d fid %d entry_id %d\n", port, pkt_svlan, pkt_cvlan, fid, scl_entry.entry_id);

    return CTC_E_NONE;
roll_back_0:
    ctc_scl_remove_entry(scl_entry.entry_id);

    return ret;
}

int32
_ctc_gbx_app_vlan_port_remove_uplink_svlan_entry(uint8 lchip, uint16 port, uint16 pkt_svlan, uint16 fid)
{
    uint32 entry_id = 0;

    entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(fid, 0);
    ctc_scl_uninstall_entry(entry_id);
    ctc_scl_remove_entry(entry_id);
    CTC_APP_DBG_INFO("remove uplink port %d pkt_svlan %d fid %d entry_id %d\n", port, pkt_svlan, fid, entry_id);

    return CTC_E_NONE;
}

#define _____ALLOC_FID____ ""
int32
ctc_gbx_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    int ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_port_fid);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_port_fid->pkt_svlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_port_fid->pkt_cvlan);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_port_fid->vdev_id);
    if (0 == p_g_app_vlan_port_master->nni_port_cnt[p_port_fid->vdev_id])
    {
        return CTC_E_NOT_EXIST;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id",              p_port_fid->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "packet svlan",         p_port_fid->pkt_svlan);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "packet cvlan",         p_port_fid->pkt_cvlan);

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    CTC_APP_VLAN_PORT_LOCK(lchip);

    port_fid_db.pkt_svlan = p_port_fid->pkt_svlan;
    port_fid_db.pkt_cvlan = p_port_fid->pkt_cvlan;
    port_fid_db.vdev_id = p_port_fid->vdev_id;

    p_port_fid_temp = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
    if (p_port_fid_temp)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }

    ret = ctc_spool_add(p_g_app_vlan_port_master->fid_spool, &port_fid_db, NULL, &p_port_fid_temp);
    if (ret)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return ret;
    }
    p_port_fid->fid = p_port_fid_temp->fid;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_INFO("Fid: %u!\n", p_port_fid_temp->fid);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    int ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_port_fid);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_port_fid->pkt_svlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_port_fid->pkt_cvlan);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_port_fid->vdev_id);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id",      p_port_fid->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "packet svlan",         p_port_fid->pkt_svlan);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "packet cvlan",         p_port_fid->pkt_cvlan);

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    CTC_APP_VLAN_PORT_LOCK(lchip);

    port_fid_db.fid = p_port_fid->fid;
    port_fid_db.pkt_svlan = p_port_fid->pkt_svlan;
    port_fid_db.pkt_cvlan = p_port_fid->pkt_cvlan;
    port_fid_db.vdev_id = p_port_fid->vdev_id;

    p_port_fid_temp = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
    if (NULL == p_port_fid_temp)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    ret = ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid_db, NULL);
    if (ret)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return ret;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_INFO("Fid: %u!\n", p_port_fid->fid);

    return CTC_E_NONE;
}

#define _____UNI_PORT_____ ""

int32
ctc_gbx_app_vlan_port_set_xlate_port(uint8 lchip, uint32 gport, uint8 drop_mc)
{
    int32 ret = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_scl_default_action_t def_action;

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = gport;
    def_action.action.type =  CTC_SCL_ACTION_EGRESS;
    if (1 == drop_mc)
    {
        CTC_SET_FLAG(def_action.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_MCAST);
    }
    else
    {
        CTC_SET_FLAG(def_action.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_UCAST);
        CTC_SET_FLAG(def_action.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_BCAST);
    }
    CTC_ERROR_GOTO(ctc_scl_set_default_action(&def_action), ret, roll_back_0);

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_GOTO(ctc_port_set_scl_property(gport, &scl_prop), ret, roll_back_0);

    return CTC_E_NONE;

roll_back_0:
    return ret;
}

int32
ctc_gbx_app_vlan_port_unset_xlate_port(uint8 lchip, uint32 gport, uint8 drop_mc)
{
    ctc_port_scl_property_t scl_prop;
    ctc_scl_default_action_t def_action;

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = gport;
    def_action.action.type =  CTC_SCL_ACTION_EGRESS;
    CTC_ERROR_RETURN(ctc_scl_set_default_action(&def_action));

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(gport, &scl_prop));

    return CTC_E_NONE;
}

/*
Each pon port: e2loop nhid + xlate nhid
*/
int32
ctc_gbx_app_vlan_port_create_mc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_vlan_edit_nh_param_t gem_xlate_nh;

    /* Save MC e2iloop nhid to port DB*/
    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];


    _ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->mc_xlate_nhid);
    _ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->mc_e2iloop_nhid);

    CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->mc_eloop_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->mc_eloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_xlate_port(lchip, p_uni_port->mc_eloop_port, 0));
    /* Create xlate nh */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    gem_xlate_nh.vlan_edit_info.output_svid = p_g_app_vlan_port_master->mcast_tunnel_vlan;
    gem_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
    {
        CTC_SET_FLAG(gem_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_LENGTH_ADJUST_EN); /*Use Resever offset*/
    }

    CTC_APP_DBG_INFO("nh xlate nhid: %d\n", p_uni_port->mc_xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_uni_port->mc_xlate_nhid, &gem_xlate_nh), ret, roll_back_1);

    /* Create gem port xlate nh */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = p_uni_port->mc_eloop_port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.loop_nhid = p_uni_port->mc_xlate_nhid;
    CTC_SET_FLAG(gem_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_uni_port->mc_e2iloop_nhid, &gem_xlate_nh), ret, roll_back_1);

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_1:

    return ret;
}

/*
Each pon port: e2loop nhid + xlate nhid
*/
int32
ctc_gbx_app_vlan_port_destroy_mc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    /* Remove  gem port xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_uni_port->mc_e2iloop_nhid));

    /* Remove  xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_uni_port->mc_xlate_nhid));

    CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_unset_xlate_port(lchip, p_uni_port->mc_eloop_port, 0));

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->mc_xlate_nhid));
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->mc_e2iloop_nhid));

    p_uni_port->mc_xlate_nhid = 0;
    p_uni_port->mc_e2iloop_nhid = 0;

    return ret;
}

int32
ctc_gbx_app_vlan_port_create_bc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_vlan_edit_nh_param_t gem_xlate_nh;

    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->bc_xlate_nhid), ret, roll_back_1);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->bc_e2iloop_nhid), ret, roll_back_1);

    CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->bc_eloop_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN(ctc_port_set_property(p_uni_port->bc_eloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_set_xlate_port(lchip, p_uni_port->bc_eloop_port, 1));

    /* Create xlate nh */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    gem_xlate_nh.vlan_edit_info.output_svid = p_g_app_vlan_port_master->bcast_tunnel_vlan;
    gem_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
    {
        CTC_SET_FLAG(gem_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_LENGTH_ADJUST_EN); /*Use Resever offset*/
    }
    CTC_APP_DBG_INFO("nh xlate nhid: %d\n", p_uni_port->bc_xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_uni_port->bc_xlate_nhid, &gem_xlate_nh), ret, roll_back_1);

    /* Create gem port xlate nh */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = p_uni_port->bc_eloop_port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.loop_nhid = p_uni_port->bc_xlate_nhid;
    CTC_SET_FLAG(gem_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_uni_port->bc_e2iloop_nhid, &gem_xlate_nh), ret, roll_back_1);

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_1:

    return ret;
}

int32
ctc_gbx_app_vlan_port_destroy_bc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    /* Remove gem port xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_uni_port->bc_e2iloop_nhid));

    /* Remove gem port xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_uni_port->bc_xlate_nhid));

    CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_unset_xlate_port(lchip, p_uni_port->bc_eloop_port, 1));

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->bc_xlate_nhid));
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->bc_e2iloop_nhid));

    p_uni_port->bc_xlate_nhid = 0;
    p_uni_port->bc_e2iloop_nhid = 0;

    return ret;
}

int32
ctc_gbx_app_vlan_port_get_uni_port(uint8 lchip, uint32 port, ctc_app_vlan_port_uni_db_t **pp_uni_port)
{
    uint16 lport = 0;

    CTC_ERROR_RETURN(ctc_port_set_property(port, CTC_PORT_PROP_PORT_EN, 1));
    lport = CTC_MAP_GPORT_TO_LPORT(port);

    *pp_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    int32 ret = 0;
    uint8 gchip  = 0;
    uint8 chip_type = 0;
    uint32 vlan_range_grp = 0;
    ctc_internal_port_assign_para_t alloc_port;
    ctc_loopback_nexthop_param_t gem_port_iloop_nh;
    ctc_vlan_edit_nh_param_t gem_xlate_nh;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_port_scl_property_t scl_prop;
    uint32 gem_iloop_gport = 0;
    ctc_vlan_range_info_t vlan_range;
    ctc_vlan_miss_t vlan_mappng_miss;
    ctc_opf_t opf;
    uint16 vlan_id = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_port_restriction_t isolation;
#ifdef DUET2
    ctc_port_isolation_t port_isolation;
#endif
    ctc_scl_default_action_t def_action;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_uni->port);
    if (CTC_IS_LINKAGG_PORT(p_uni->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));
    if (0 != p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }

    /***********************************************/
    /* untag svlan disable*/
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    /***********************************************/
    /* Vlan range enalbe*/
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_opf_alloc_offset(&opf, 1, &vlan_range_grp));
    CTC_APP_DBG_INFO("OPF vlan_range_grp: %d\n", vlan_range_grp);

    ctc_get_gchip_id(lchip, &gchip);
    p_uni_port->vdev_id = p_uni->vdev_id;

    /***********************************************/
    /* Alloc port eloop port for PON and iloop port  */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->e2eloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->e2eloop_port);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_set_internal_port(&alloc_port));
    p_uni_port->iloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
    gem_iloop_gport = p_uni_port->iloop_port;
    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_ingress_property, lchip, p_uni->port, CTC_PORT_PROP_BRIDGE_EN, FALSE);

    /* Alloc gem port eloop port for ONU  */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->onu_e2eloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->onu_e2eloop_sec_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    /* Alloc gem port eloop port for uplink to onu for mcast*/
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->mc_eloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    /* Alloc gem port eloop port for uplink to onu for bcast*/
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->bc_eloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    /***********************************************/
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->iloop_nhid));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->e2iloop_nhid));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->iloop_sec_nhid));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_uni_port->e2iloop_sec_nhid));

    /***********************************************/
    /* Enable scl lookup for GEM port */
    chip_type = ctc_get_chip_type();
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(p_uni->port, &scl_prop));
        break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
        {
            ctc_acl_property_t acl_prop;
            sal_memset(&acl_prop, 0, sizeof(acl_prop));
            acl_prop.acl_en = 1;
            acl_prop.direction = CTC_INGRESS;
            acl_prop.acl_priority = 0;
            acl_prop.hash_field_sel_id = 0;
            acl_prop.hash_lkup_type = CTC_ACL_HASH_LKUP_TYPE_L2;
            CTC_SET_FLAG(acl_prop.flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_acl_property(p_uni->port, &acl_prop));
        }
        break;
#endif
        default:
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }
    sal_memset(&vlan_mappng_miss, 0, sizeof(vlan_mappng_miss));
    vlan_mappng_miss.flag = CTC_VLAN_MISS_ACTION_DISCARD;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_add_default_vlan_mapping(p_uni->port, &vlan_mappng_miss));

    /***********************************************/
    /**ILOOP POTY property*/
    /* Create gem port iloop nh */
    sal_memset(&gem_port_iloop_nh, 0, sizeof(gem_port_iloop_nh));
    gem_port_iloop_nh.lpbk_lport = p_uni_port->iloop_port;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_nh_add_iloop(p_uni_port->iloop_nhid, &gem_port_iloop_nh));

    /* Create gem port xlate nh, downlink to uplink */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = p_g_app_vlan_port_master->downlink_eloop_port[p_uni->vdev_id];
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
            gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;
#endif
        default:
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.loop_nhid = p_uni_port->iloop_nhid;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_nh_add_xlate(p_uni_port->e2iloop_nhid, &gem_xlate_nh));
    /* Enable scl lookup for double */
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(gem_iloop_gport, &scl_prop));

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(gem_iloop_gport, &scl_prop));

    /* Enable port en */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(gem_iloop_gport, CTC_PORT_PROP_LEARNING_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(gem_iloop_gport, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(gem_iloop_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(gem_iloop_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(gem_iloop_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    /* Alloc uplink iloop second port for through */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->iloop_sec_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
    p_uni_port->eloop_sec_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_LEARNING_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->iloop_sec_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    /* Create uplink port iloop nh */
    sal_memset(&gem_port_iloop_nh, 0, sizeof(gem_port_iloop_nh));
    gem_port_iloop_nh.lpbk_lport = p_uni_port->iloop_sec_port;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_nh_add_iloop(p_uni_port->iloop_sec_nhid, &gem_port_iloop_nh));

    /* Create uplink to with svlan forwarding */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = p_uni_port->eloop_sec_port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    gem_xlate_nh.vlan_edit_info.loop_nhid = p_uni_port->iloop_sec_nhid;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_nh_add_xlate(p_uni_port->e2iloop_sec_nhid, &gem_xlate_nh));

    /* Enable scl lookup for uplink iloop port */
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(p_uni_port->iloop_sec_port, &scl_prop));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_default_vlan(p_uni_port->iloop_sec_port, p_g_app_vlan_port_master->default_bcast_fid[p_uni_port->vdev_id]));

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = p_uni_port->iloop_sec_port;
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    def_action.action.u.igs_action.fid = p_g_app_vlan_port_master->default_bcast_fid[p_uni_port->vdev_id];
    CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
    def_action.action.u.igs_action.user_vlanptr = p_g_app_vlan_port_master->default_bcast_fid[p_uni_port->vdev_id];
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_set_default_action(&def_action));

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = gem_iloop_gport;
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_REDIRECT);
    def_action.action.u.igs_action.nh_id = p_uni_port->e2iloop_sec_nhid;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_set_default_action(&def_action));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_default_vlan(gem_iloop_gport, p_g_app_vlan_port_master->default_deny_learning_fid[p_uni_port->vdev_id]));

    /* Vlan range enalbe*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_INGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_create_vlan_range_group(&vlan_range, TRUE));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_vlan_range(gem_iloop_gport, &vlan_range, TRUE));
    p_uni_port->vlan_range_grp = vlan_range_grp;

    /***********************************************/
    /**PON ELOOP PORT property*/
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->e2eloop_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->e2eloop_port, CTC_PORT_PROP_IS_LEAF, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->e2eloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->eloop_sec_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->eloop_sec_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    /* Vlan range enalbe*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_EGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_create_vlan_range_group(&vlan_range, FALSE));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_vlan_range(p_uni_port->e2eloop_port, &vlan_range, TRUE));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_vlan_range(p_uni_port->bc_eloop_port, &vlan_range, TRUE));

#if 0
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_service_policer_en,
                    lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->e2eloop_port), 1));
#endif

    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_set_xlate_port(lchip, p_uni_port->e2eloop_port, 1));

    /* For pon service do vlan xlate*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(p_uni_port->e2eloop_port, &scl_prop));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_scl_property(p_uni_port->bc_eloop_port, &scl_prop));

    /***********************************************/
    /**ONU ELOOP PORT property*/
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->onu_e2eloop_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->onu_e2eloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->onu_e2eloop_sec_port, CTC_PORT_PROP_PORT_EN, 1));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_property(p_uni_port->onu_e2eloop_sec_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_set_xlate_port(lchip, p_uni_port->onu_e2eloop_port, 1));

    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_set_xlate_port(lchip, p_uni_port->onu_e2eloop_sec_port, 1));
#if 1
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_service_policer_en,
                    lchip, p_uni_port->onu_e2eloop_port, 1));
#else

    /* For onu service do vlan xlate*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.use_logic_port_en  = 1;
    CTC_ERROR_RETURN(ctc_port_set_scl_property(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->onu_e2eloop_port), &scl_prop));
#endif

    /***********************************************/
    /** BC/MC PORT property*/
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_create_mc_gem_port(lchip, p_uni->port));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_create_bc_gem_port(lchip, p_uni->port));

    vlan_id = p_g_app_vlan_port_master->default_bcast_fid[p_uni->vdev_id];
    /*Add BC member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = vlan_id;
    l2dflt.l2mc_grp_id = p_uni->vdev_id;
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = p_uni_port->bc_e2iloop_nhid;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_add_port_to_default_entry(&l2dflt));

    /*Add MC member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = vlan_id;
    l2dflt.l2mc_grp_id = p_uni->vdev_id;
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = p_uni_port->mc_e2iloop_nhid;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_add_port_to_default_entry(&l2dflt));
    /***********************************************/
    /** PORT ISOLATION*/
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_EGRESS;
    isolation.isolated_id  = vlan_range_grp + 1;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_restriction(p_uni_port->e2eloop_port, &isolation));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_restriction(p_uni_port->onu_e2eloop_port, &isolation));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_restriction(p_uni_port->mc_eloop_port, &isolation));
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_restriction(p_uni_port->bc_eloop_port, &isolation));
    isolation.dir = CTC_INGRESS;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_restriction(p_uni->port, &isolation));
#ifdef DUET2
    sal_memset(&port_isolation, 0, sizeof(ctc_port_isolation_t));
    port_isolation.gport = vlan_range_grp+1;
    port_isolation.use_isolation_id = 1;
    port_isolation.pbm[0] = 1<<(vlan_range_grp+1);
    port_isolation.isolation_pkt_type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_port_set_isolation(lchip, &port_isolation));
#endif

#if 0
    /***********************************************/
    /** PORT ISOLATION*/
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_BOTH_DIRECTION;
    isolation.isolated_id  = 1;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->onu_e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->mc_eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->bc_eloop_port), &isolation));


    /***********************************************/
    /** PORT ISOLATION*/
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_BOTH_DIRECTION;
    isolation.isolated_id  =  p_g_app_vlan_port_master->uni_outer_isolate_en?1:0;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->onu_e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->mc_eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->bc_eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->iloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, p_uni->port, &isolation));


    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->e2eloop_port, gem_iloop_gport));
    CTC_ERROR_RETRUN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->onu_e2eloop_port, gem_iloop_gport));
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->bc_eloop_port, gem_iloop_gport));
#endif

    /***********************************************/
    /** Return param*/
    p_uni->mc_nhid = p_uni_port->mc_xlate_nhid;
    p_uni->associate_port = p_uni_port->iloop_port;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t *p_uni)
{
    int32 ret = 0;
    uint8 gchip  = 0;
    uint8 chip_type = 0;
    ctc_internal_port_assign_para_t alloc_port;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_port_scl_property_t scl_prop;
    ctc_vlan_range_info_t vlan_range;
    ctc_opf_t opf;
    uint16 vlan_id = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_port_restriction_t isolation;
    ctc_vlan_miss_t vlan_mappng_miss;
    ctc_scl_default_action_t def_action;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_uni->port);
    if (CTC_IS_LINKAGG_PORT(p_uni->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    ctc_get_gchip_id(lchip, &gchip);

    /***********************************************/
    /** PORT ISOLATION*/
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_EGRESS;
    isolation.isolated_id  = 0;
    isolation.mode = CTC_PORT_RESTRICTION_DISABLE;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    ctc_port_set_restriction(p_uni_port->e2eloop_port, &isolation);
    ctc_port_set_restriction(p_uni_port->onu_e2eloop_port, &isolation);
    ctc_port_set_restriction(p_uni_port->mc_eloop_port, &isolation);
    ctc_port_set_restriction(p_uni_port->bc_eloop_port, &isolation);

    isolation.dir = CTC_INGRESS;
    ctc_port_set_restriction( p_uni->port, &isolation);

#if 0
    /***********************************************/
    /** PORT ISOLATION*/
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_BOTH_DIRECTION;
    isolation.isolated_id  =  0;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->onu_e2eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->mc_eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->bc_eloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->iloop_port), &isolation));
    CTC_ERROR_RETURN(ctc_port_set_restriction(lchip, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni->port), &isolation));


    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->e2eloop_port, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->e2eloop_port)));
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->onu_e2eloop_port, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->onu_e2eloop_port)));
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_global_port, lchip, p_uni_port->bc_eloop_port, CTC_MAP_LPORT_TO_GPORT(gchip, p_uni_port->bc_eloop_port)));
#endif

    /***********************************************/
    /* Remove bc/mc nhid from vlan */
    vlan_id = p_g_app_vlan_port_master->default_bcast_fid[p_uni->vdev_id];
    /*Add BC member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = vlan_id;
    l2dflt.l2mc_grp_id = p_uni->vdev_id;
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = p_uni_port->bc_e2iloop_nhid;
    ctc_l2_remove_port_from_default_entry(&l2dflt);

    /*Add MC member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = vlan_id;
    l2dflt.l2mc_grp_id = p_uni->vdev_id;
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = p_uni_port->mc_e2iloop_nhid;
    ctc_l2_remove_port_from_default_entry(&l2dflt);

    ctc_gbx_app_vlan_port_destroy_bc_gem_port(lchip, p_uni->port);
    ctc_gbx_app_vlan_port_destroy_mc_gem_port(lchip, p_uni->port);

    /***********************************************/
    /* Free gem port eloop port */
    ctc_gbx_app_vlan_port_unset_xlate_port(lchip, p_uni_port->onu_e2eloop_port, 1);
    ctc_gbx_app_vlan_port_unset_xlate_port(lchip, p_uni_port->onu_e2eloop_sec_port, 1);
    /***********************************************/
    /* Disable egress scl pon service do vlan xlate*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni_port->e2eloop_port, &scl_prop);

    ctc_gbx_app_vlan_port_unset_xlate_port(lchip, p_uni_port->e2eloop_port, 1);

    /***********************************************/
    /* Vlan range disable*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_EGRESS;
    vlan_range.vrange_grpid = p_uni_port->vlan_range_grp;
    ctc_port_set_vlan_range(p_uni_port->e2eloop_port, &vlan_range, FALSE);
    ctc_vlan_destroy_vlan_range_group(&vlan_range);

    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_INGRESS;
    vlan_range.vrange_grpid = p_uni_port->vlan_range_grp;
    ctc_port_set_vlan_range(p_uni_port->iloop_port, &vlan_range, FALSE);

    ctc_vlan_destroy_vlan_range_group(&vlan_range);
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    ctc_opf_free_offset(&opf, 1, p_uni_port->vlan_range_grp);

    /***********************************************/
    /* Disable ingress scl*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni_port->iloop_port, &scl_prop);

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni_port->iloop_port, &scl_prop);

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni_port->iloop_sec_port, &scl_prop);

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    def_action.gport = p_uni_port->iloop_port;
    ctc_scl_set_default_action(&def_action);

    sal_memset(&vlan_mappng_miss, 0, sizeof(vlan_mappng_miss));
    vlan_mappng_miss.flag = CTC_VLAN_MISS_ACTION_DO_NOTHING;
    ctc_vlan_add_default_vlan_mapping(p_uni->port, &vlan_mappng_miss);

    /***********************************************/
    /* Remove nhid*/
    ctc_nh_remove_xlate(p_uni_port->e2iloop_nhid);
    ctc_nh_remove_iloop(p_uni_port->iloop_nhid);
    ctc_nh_remove_xlate(p_uni_port->e2iloop_sec_nhid);
    ctc_nh_remove_iloop(p_uni_port->iloop_sec_nhid);

    /***********************************************/
    /* Disable scl lookup for GEM port */
    chip_type = ctc_get_chip_type();
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            sal_memset(&scl_prop, 0, sizeof(scl_prop));
            scl_prop.scl_id = 0;
            scl_prop.direction = CTC_INGRESS;
            scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
            scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
            ctc_port_set_scl_property(p_uni->port, &scl_prop);
        break;
 #endif
 #ifdef DUET2
        case CTC_CHIP_DUET2:
        {
            ctc_acl_property_t acl_prop;
            sal_memset(&acl_prop, 0, sizeof(acl_prop));
            acl_prop.acl_en = 0;
            acl_prop.direction = CTC_INGRESS;
            acl_prop.acl_priority = 0;
            acl_prop.hash_lkup_type = CTC_ACL_HASH_LKUP_TYPE_DISABLE;
            CTC_SET_FLAG(acl_prop.flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);
            ctc_port_set_acl_property(p_uni->port, &acl_prop);
        }
        break;
#endif
        default:
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }

    ctc_vlan_remove_default_vlan_mapping(p_uni->port);

    /***********************************************/
    /* Free nhid resource */
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->iloop_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->e2iloop_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->iloop_sec_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_uni_port->e2iloop_sec_nhid);

    /***********************************************/
    /* Free internal port port */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->bc_eloop_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->mc_eloop_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->onu_e2eloop_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->onu_e2eloop_sec_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->iloop_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->iloop_sec_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->eloop_sec_port);
    ctc_free_internal_port(&alloc_port);

    /***********************************************/
    /* Free gem port eloop port */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_uni_port->e2eloop_port);
    ctc_free_internal_port(&alloc_port);

    /***********************************************/
    /* untag svlan enable*/
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
    sal_memset(p_uni_port, 0, sizeof(ctc_app_vlan_port_uni_db_t));
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t *p_uni)
{
    int32 ret = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_uni->port);
    if (CTC_IS_LINKAGG_PORT(p_uni->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    p_uni->vdev_id = p_uni_port->vdev_id;
    p_uni->mc_nhid = p_uni_port->mc_xlate_nhid;
    p_uni->bc_nhid = p_uni_port->bc_xlate_nhid;
    p_uni->associate_port = p_uni_port->iloop_port;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

#define _____GEM_PORT_____ ""

int32
_ctc_gbx_app_vlan_port_alloc_custom_id(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                                                          ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
#ifdef DUET2
    ctc_opf_t opf;
    uint32 custom_id = 0;

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_GEM_PORT;
    CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, &custom_id));
    p_gem_port_db->custom_id = custom_id;
#endif

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_free_custom_id(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                                                          ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
#ifdef DUET2
    ctc_opf_t opf;

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_GEM_PORT;
    ctc_opf_free_offset(&opf, 1, p_gem_port_db->custom_id);
    p_gem_port_db->custom_id = 0;
#endif

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_create_gem_port_downlink_scl_entry(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                                                          ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
    int32 ret = CTC_E_NONE;
#ifdef DUET2
    uint32 entry_id = 0;
    uint32 group_id = 0;
    ctc_scl_entry_t scl_entry;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_scl_field_action_t field_action;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    entry_id = ENCODE_SCL_DOWNLINK_ILOOP_ENTRY_ID(p_gem_port_db->custom_id);
    scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_MAC;
    scl_entry.mode = 1;
    scl_entry.action_type =  CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = entry_id;
    group_id = CTC_SCL_GROUP_ID_HASH_PORT_MAC;
    
resolve_conflict:
    
    CTC_ERROR_RETURN(ctc_scl_add_entry(group_id, &scl_entry));

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
    field_key.type = CTC_FIELD_KEY_PORT;
    field_port.gport = p_g_app_vlan_port_master->downlink_iloop_default_port[p_gem_port_db->vdev_id];
    field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
    field_key.ext_data = &field_port;
    CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &field_key), ret, roll_back_0);
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_CUSTOMER_ID;
    field_key.data = p_gem_port_db->custom_id;
    CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &field_key), ret, roll_back_0);
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    field_key.type = CTC_FIELD_KEY_HASH_VALID;
    ret =  ctc_scl_add_key_field(entry_id, &field_key);
    if(CTC_E_HASH_CONFLICT == ret)
    {
        ctc_scl_remove_entry(entry_id);
        scl_entry.resolve_conflict = 1;
        group_id = CTC_SCL_GROUP_ID_TCAM0;
        goto resolve_conflict;
    }
    else if(0 != ret)
    {
        goto roll_back_0;
    }
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    field_action.data0 = p_gem_port_db->xlate_nhid;
    CTC_ERROR_GOTO(ctc_scl_add_action_field(entry_id, &field_action), ret, roll_back_0);
    CTC_ERROR_GOTO(ctc_scl_install_entry(entry_id), ret, roll_back_0);

    return CTC_E_NONE;

roll_back_0:
    ctc_scl_remove_entry(entry_id);

#endif
    return ret;
}

int32
_ctc_gbx_app_vlan_port_destroy_gem_port_downlink_scl_entry(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                                                          ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
#ifdef DUET2
    uint32 entry_id = 0;

    entry_id = ENCODE_SCL_DOWNLINK_ILOOP_ENTRY_ID(p_gem_port_db->custom_id);
    ctc_scl_uninstall_entry(entry_id);
    ctc_scl_remove_entry(entry_id);
#endif

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port,
                                     ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
    int32 ret = 0;
    ctc_vlan_edit_nh_param_t vlan_xlate_nh;
#ifdef GREATBELT
    ctc_vlan_mapping_t vlan_mapping;
#endif
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint8 chip_type = 0;

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_offset(lchip, &p_gem_port_db->ad_index), ret, roll_back_0);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_gem_port_db->xlate_nhid), ret, roll_back_1);

    lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    /***********************************************
    **  from ONU to uplink
    ***********************************************/
    /* Add vlan mapping to e2iloop nexthop */
    chip_type = ctc_get_chip_type();
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
            CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
            vlan_mapping.old_svid = p_gem_port->tunnel_value;
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_NHID);
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SVID);
            vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
            vlan_mapping.logic_src_port = p_gem_port_db->logic_port;
            vlan_mapping.u3.nh_id = p_uni_port->e2iloop_nhid;
            vlan_mapping.logic_port_type = 0;

            ret = ctc_vlan_add_vlan_mapping(p_gem_port->port, &vlan_mapping);
            if (ret == CTC_E_HASH_CONFLICT)
            {
                CTC_APP_DBG_INFO("Hash Confict!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
                CTC_ERROR_GOTO(ctc_vlan_add_vlan_mapping(p_gem_port->port, &vlan_mapping), ret, roll_back_2);
                p_gem_port_db->igs_vlan_maping_use_flex = 1;
            }
            else if(ret != CTC_E_NONE)
            {
                goto roll_back_2;
            }
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
        {
            ctc_acl_entry_t acl_entry;
            ctc_field_key_t key_field;
            ctc_acl_field_action_t action_field;
            ctc_field_port_t field_port;
            uint32 entry_id = 0;

            entry_id = ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value);
            sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
            acl_entry.key_type = CTC_ACL_KEY_HASH_MAC;
            acl_entry.entry_id = entry_id;
            acl_entry.mode = 1;
            CTC_ERROR_GOTO(ctc_acl_add_entry(CTC_ACL_GROUP_ID_HASH_MAC, &acl_entry), ret, roll_back_2);

            sal_memset(&key_field, 0, sizeof(key_field));
            key_field.type = CTC_FIELD_KEY_SVLAN_ID;
            key_field.data = p_gem_port->tunnel_value;
            CTC_ERROR_GOTO(ctc_acl_add_key_field(entry_id, &key_field), ret, roll_back_3);
            sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
            sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
            key_field.type = CTC_FIELD_KEY_PORT;
            field_port.gport = p_gem_port->port;
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            key_field.ext_data = &field_port;
            CTC_ERROR_GOTO(ctc_acl_add_key_field(entry_id, &key_field), ret, roll_back_3);

            sal_memset(&key_field, 0, sizeof(key_field));
            key_field.type = CTC_FIELD_KEY_STAG_VALID;
            key_field.data = 1;
            CTC_ERROR_GOTO(ctc_acl_add_key_field(entry_id, &key_field), ret, roll_back_3);

            sal_memset(&key_field, 0, sizeof(key_field));
            key_field.type = CTC_FIELD_KEY_HASH_VALID;
            CTC_ERROR_GOTO(ctc_acl_add_key_field(entry_id, &key_field), ret, roll_back_3);

            sal_memset(&action_field, 0, sizeof(action_field));
            action_field.type = CTC_ACL_FIELD_ACTION_REDIRECT;
            action_field.data0 = p_uni_port->e2iloop_nhid;
            CTC_ERROR_GOTO(ctc_acl_add_action_field(entry_id, &action_field), ret, roll_back_3);

            sal_memset(&action_field, 0, sizeof(action_field));
            action_field.type = CTC_ACL_FIELD_ACTION_LOGIC_PORT;
            action_field.data0 = p_gem_port_db->logic_port;
            CTC_ERROR_GOTO(ctc_acl_add_action_field(entry_id, &action_field), ret, roll_back_3);

            if (p_gem_port->ingress_stats_id && CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
            {
                sal_memset(&action_field, 0, sizeof(action_field));
                action_field.type = CTC_ACL_FIELD_ACTION_STATS;
                action_field.data0 = p_gem_port->ingress_stats_id;
                CTC_ERROR_GOTO(ctc_acl_add_action_field(entry_id, &action_field), ret, roll_back_3);
                p_gem_port_db->ingress_stats_id = p_gem_port->ingress_stats_id;
            }

            CTC_ERROR_GOTO(ctc_acl_install_entry(entry_id), ret, roll_back_3);
        }
            break;
#endif
        default:
            return CTC_E_INVALID_PARAM;
    }

    /***********************************************
    **  from uplink to ONU
    ***********************************************/
    /* Create xlate nh for leaning */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_gem_port->port;
    vlan_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    vlan_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    vlan_xlate_nh.vlan_edit_info.output_svid = p_gem_port->tunnel_value;
    vlan_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
    vlan_xlate_nh.vlan_edit_info.stag_cos = 0;
    if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
    {
        CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_LENGTH_ADJUST_EN); /*Use Resever offset*/
    }
    else
    {
        if (p_gem_port->egress_stats_id)
        {
            vlan_xlate_nh.vlan_edit_info.stats_id = p_gem_port->egress_stats_id;
            vlan_xlate_nh.vlan_edit_info.flag |= CTC_VLAN_NH_STATS_EN;
            p_gem_port_db->egress_stats_id = p_gem_port->egress_stats_id;
        }
    }
    CTC_APP_DBG_INFO("add gem port xlate nhid: %d\n", p_gem_port_db->xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_gem_port_db->xlate_nhid, &vlan_xlate_nh), ret, roll_back_4);
    p_gem_port_db->logic_dest_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];

#ifdef GREATBELT
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_gem_port_db->e2eloop_nhid), ret, roll_back_5);
#else
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_gem_port_db->e2iloop_nhid), ret, roll_back_5);
#endif

#ifdef APP_VLAN_FLEX_QINQ
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_gem_port_db->flex_e2eloop_nhid), ret, roll_back_6);
#endif
    /* Create xlate nh */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_uni_port->e2eloop_port;
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG); /*LogicReplicate*/
#ifdef GREATBELT
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_gem_port_db->xlate_nhid;
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_gem_port_db->e2eloop_nhid, &vlan_xlate_nh), ret, roll_back_7);
    CTC_APP_DBG_INFO("add gem port e2eloop nhid: %d\n", p_gem_port_db->e2eloop_nhid);
#else
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_g_app_vlan_port_master->downlink_iloop_nhid[p_uni_port->vdev_id];
    vlan_xlate_nh.logic_port = p_gem_port_db->custom_id;
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_gem_port_db->e2iloop_nhid, &vlan_xlate_nh), ret, roll_back_7);
    CTC_APP_DBG_INFO("add gem port e2iloop nhid: %d\n", p_gem_port_db->e2iloop_nhid);
#endif

#ifdef APP_VLAN_FLEX_QINQ
    /* Create xlate nh */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_g_app_vlan_port_master->downlink_eloop_port[p_uni_port->vdev_id];
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_gem_port_db->xlate_nhid;
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG); /*LogicReplicate*/
#ifdef DUET2
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);
#endif
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_gem_port_db->flex_e2eloop_nhid, &vlan_xlate_nh), ret, roll_back_8);
#endif
    CTC_APP_DBG_INFO("add gem port e2eloop nhid: %d\n", p_gem_port_db->e2eloop_nhid);

#ifdef DUET2
    /* Bind logic port and nhid which for hw learning */
    /*CTC_ERROR_GOTO(ctc_l2_set_nhid_by_logic_port(p_gem_port_db->logic_port, p_gem_port_db->e2iloop_nhid), ret, roll_back_8);*/
#endif

    /*PON port refcnt ++*/
    p_uni_port->ref_cnt++;

    p_gem_port->ga_gport =  p_uni_port->iloop_port;

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
#ifdef APP_VLAN_FLEX_QINQ
roll_back_8:
#ifdef GREATBELT
   ctc_nh_remove_xlate(p_gem_port_db->e2eloop_nhid);
#else
   ctc_nh_remove_xlate(p_gem_port_db->e2iloop_nhid);
#endif
#endif
roll_back_7:
#ifdef APP_VLAN_FLEX_QINQ
   _ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->flex_e2eloop_nhid);
roll_back_6:
#endif
#ifdef GREATBELT
   _ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->e2eloop_nhid);
#else
  _ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->e2iloop_nhid);
#endif

roll_back_5:
    ctc_nh_remove_xlate(p_gem_port_db->xlate_nhid);

roll_back_4:
#ifdef GREATBELT
    ctc_vlan_remove_vlan_mapping(p_gem_port->port, &vlan_mapping);
#endif
#ifdef DUET2
    ctc_acl_uninstall_entry(ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value));
roll_back_3:
    ctc_acl_remove_entry(ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value));
#endif
roll_back_2:
   _ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->xlate_nhid);

roll_back_1:
   _ctc_gbx_app_vlan_port_free_offset(lchip, p_gem_port_db->ad_index);

roll_back_0:
    return ret;
}

int32
_ctc_gbx_app_vlan_port_destroy_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port,
                                     ctc_app_vlan_port_gem_port_db_t *p_gem_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint8 chip_type = 0;

    lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

     /***********************************************
     **  from uplink to ONU
     ***********************************************/
#if 0
    /* Create xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_gem_port_db->onu_eloop_nhid));
    CTC_ERROR_RETURN(_ctc_app_vlan_port_free_nhid(p_gem_port_db->onu_eloop_nhid));
#endif

#ifdef GREATBELT
    if (p_gem_port_db->e2eloop_nhid)
    {
        /* Create xlate nh */
        CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_gem_port_db->e2eloop_nhid));
    }
#else
    if (p_gem_port_db->e2iloop_nhid)
    {
        /* Create xlate nh */
        CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_gem_port_db->e2iloop_nhid));
    }
#endif
#ifdef APP_VLAN_FLEX_QINQ
    if (p_gem_port_db->flex_e2eloop_nhid)
    {
        /* Create xlate nh */
        CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_gem_port_db->flex_e2eloop_nhid));
    }
#endif
    /* Remove xlate nh for leaning */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_gem_port_db->xlate_nhid));

    CTC_APP_DBG_INFO("Remove gem port e2eloop nhid: %d\n", p_gem_port_db->e2eloop_nhid);

    /***********************************************
    **  from ONU to uplink
    ***********************************************/
    chip_type = ctc_get_chip_type();
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            {
                /* Remove vlan mapping to e2iloop nexthop */
                ctc_vlan_mapping_t vlan_mapping;

                sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
                CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
                vlan_mapping.old_svid = p_gem_port->tunnel_value;
                if (p_gem_port_db->igs_vlan_maping_use_flex)
                {
                    CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
                }
                CTC_ERROR_RETURN(ctc_vlan_remove_vlan_mapping(p_gem_port->port, &vlan_mapping));
            }
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
            {
            /* Remove hash acl redirect to e2iloop nexthop */
            uint32 entry_id = 0;

            entry_id = ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value);

            ctc_acl_uninstall_entry(entry_id);
            ctc_acl_remove_entry(entry_id);
            }
            break;
#endif
        default:
            return CTC_E_INVALID_PARAM;
    }

#ifdef GREATBELT
    if (p_gem_port_db->e2eloop_nhid)
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->e2eloop_nhid));
    }
#else
    if (p_gem_port_db->e2iloop_nhid)
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->e2iloop_nhid));
    }
#endif

#ifdef APP_VLAN_FLEX_QINQ
    if (p_gem_port_db->flex_e2eloop_nhid)
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->flex_e2eloop_nhid));
    }
#endif
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_gem_port_db->xlate_nhid));
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_offset(lchip, p_gem_port_db->ad_index));

    /*PON port refcnt ++*/
    p_uni_port->ref_cnt--;

    return ret;
}

int32
ctc_gbx_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port)
{
    int32 ret = 0;
    uint32 logic_port = 0;
    ctc_opf_t opf;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_gem_port->port);
    if (CTC_IS_LINKAGG_PORT(p_gem_port->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port",         p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "tunnel_value", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL != p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }

    MALLOC_POINTER(ctc_app_vlan_port_gem_port_db_t, p_gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_gem_port_db, 0, sizeof(ctc_app_vlan_port_gem_port_db_t));
    p_gem_port_db->vdev_id = p_uni_port->vdev_id;
    p_gem_port_db->port = p_gem_port->port;
    p_gem_port_db->tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db->limit_num = CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE;

    /* OPF --> logicPort A */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, free_meomory);
    p_gem_port_db->logic_port = logic_port;
    p_gem_port->logic_port = logic_port;
    CTC_APP_DBG_INFO("OPF gem logic port: %d\n", logic_port);

    /*Create gem port sevice*/
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_custom_id(lchip, p_gem_port, p_gem_port_db), ret, roll_back_0);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_create_gem_port(lchip, p_gem_port, p_gem_port_db), ret, roll_back_1);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_create_gem_port_downlink_scl_entry(lchip, p_gem_port, p_gem_port_db), ret, roll_back_2);

    /* DB & STATS */
    ctc_hash_insert(p_g_app_vlan_port_master->gem_port_hash, p_gem_port_db);
    p_g_app_vlan_port_master->gem_port_cnt[p_gem_port_db->vdev_id]++;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
roll_back_2:
    _ctc_gbx_app_vlan_port_destroy_gem_port(lchip, p_gem_port, p_gem_port_db);
roll_back_1:
    _ctc_gbx_app_vlan_port_free_custom_id(lchip, p_gem_port, p_gem_port_db);
roll_back_0:
    ctc_opf_free_offset(&opf, 1, logic_port);

free_meomory:
    mem_free(p_gem_port_db);
    p_gem_port_db = NULL;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port)
{
    uint32 logic_port = 0;
    ctc_opf_t opf;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_l2_flush_fdb_t flush;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_gem_port->port);
    if (CTC_IS_LINKAGG_PORT(p_gem_port->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "tunnel_value", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    ctc_gbx_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port);

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (p_gem_port_db->ref_cnt)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return  CTC_E_IN_USE;
    }

    logic_port = p_gem_port_db->logic_port;
    CTC_APP_DBG_INFO("Gem logic port: %d\n", logic_port);

    /*destroy gem port downlink scl entry*/
    _ctc_gbx_app_vlan_port_destroy_gem_port_downlink_scl_entry(lchip, p_gem_port, p_gem_port_db);
    /*Create gem port sevice*/
    _ctc_gbx_app_vlan_port_destroy_gem_port(lchip, p_gem_port, p_gem_port_db);
    /*free custom id*/
    _ctc_gbx_app_vlan_port_free_custom_id(lchip, p_gem_port, p_gem_port_db);

    /* flush logic port fdb count*/
    if (p_gem_port_db->limit_count)
    {
        _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_db->limit_action, p_gem_port_db, FALSE);
        p_gem_port_db->limit_count = 0;
    }
    sal_memset(&flush, 0, sizeof(flush));
    flush.gport = logic_port;
    flush.use_logic_port = 1;
    flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
    flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    ctc_l2_flush_fdb(&flush);
    _ctc_gbx_app_vlan_port_fdb_flush(lchip, 0, logic_port, CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT);

    /* Free OPF */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    ctc_opf_free_offset(&opf, 1, logic_port);

    ctc_hash_remove(p_g_app_vlan_port_master->gem_port_hash, p_gem_port_db);
    p_g_app_vlan_port_master->gem_port_cnt[p_uni_port->vdev_id]--;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    mem_free(p_gem_port_db);
    p_gem_port_db = NULL;

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_update_station_move_action(ctc_app_vlan_port_db_t *p_vlan_port_db,
                                  void* p_gem_port_temp)
{
    ctc_app_gem_port_t* p_gem_port = NULL;

    p_gem_port = (ctc_app_gem_port_t*)p_gem_port_temp;
    if (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port_db->criteria)
    {
        if ((p_gem_port->port == p_vlan_port_db->port)
            && (p_gem_port->tunnel_value == p_vlan_port_db->tunnel_value))
        {
            CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_fdb_set_station_move_action,
                        0, p_vlan_port_db->ad_index, p_gem_port->station_move_action);
        }
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port)
{
    int32 ret = 0;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_gem_port->port);
    if (CTC_IS_LINKAGG_PORT(p_gem_port->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (CTC_APP_GEM_PORT_UPDATE_ISOLATE == p_gem_port->update_type)
    {
    uint8 chip_type = 0;
    chip_type = ctc_get_chip_type();
    switch(chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            {
                ctc_vlan_mapping_t vlan_mapping;

                sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
                CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
                vlan_mapping.old_svid = p_gem_port->tunnel_value;
                CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
                CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_NHID);
                CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
                vlan_mapping.logic_src_port = p_gem_port_db->logic_port;
                vlan_mapping.u3.nh_id = p_uni_port->e2iloop_nhid;
                vlan_mapping.logic_port_type = p_gem_port->isolation_en?1:0;
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_update_vlan_mapping(p_gem_port->port, &vlan_mapping));
            }
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
        {
            ctc_acl_field_action_t action_field;
            uint32 entry_id = 0;

            entry_id = ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value);

            sal_memset(&action_field, 0, sizeof(action_field));
            action_field.type = CTC_ACL_FIELD_ACTION_LOGIC_PORT;
            action_field.data0 = p_gem_port_db->logic_port;
            ctc_acl_add_action_field(entry_id, &action_field);
        }
            break;
#endif
        default:
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
    }


    }
    else if(CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT == p_gem_port->update_type ||
        CTC_APP_GEM_PORT_UPDATE_UNBIND_VLAN_PORT == p_gem_port->update_type)
    {
        ctc_app_vlan_port_db_t* p_vlan_port_db_tmp = NULL;
        ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
        ctc_l2dflt_addr_t l2dflt;

        /* DB */
        p_vlan_port_db_tmp = (ctc_app_vlan_port_db_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_app_vlan_port_db_t));
        if (NULL == p_vlan_port_db_tmp)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_vlan_port_db_tmp, 0, sizeof(ctc_app_vlan_port_db_t));
        p_vlan_port_db_tmp->vlan_port_id = p_gem_port->vlan_port_id;

        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_hash, p_vlan_port_db_tmp);
        mem_free(p_vlan_port_db_tmp);
        if (NULL == p_vlan_port_db)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }

        if ((p_vlan_port_db->port != p_gem_port->port)
            || ((CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN != p_vlan_port_db->criteria)
            && (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX != p_vlan_port_db->criteria)))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_CONFIG;
        }

        /*Add/Remove vlan member*/
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = p_vlan_port_db->fid_mapping;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid_mapping;
        l2dflt.with_nh = 1;
#ifdef GREATBELT
        l2dflt.member.nh_id = p_gem_port_db->e2eloop_nhid;
#else
        l2dflt.member.nh_id = p_gem_port_db->e2iloop_nhid;
#endif
        CTC_APP_DBG_INFO("p_vlan_port_db->logic_port:%d\n", p_vlan_port_db->logic_port);

        if (CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT == p_gem_port->update_type )
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_add_port_to_default_entry(&l2dflt));
            CTC_APP_DBG_INFO("Group(%d) add ONU nhid: %d\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
        }
        else
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_remove_port_from_default_entry(&l2dflt));
            CTC_APP_DBG_INFO("Group(%d) remove ONU nhid: %d\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
        }
        CTC_APP_VLAN_PORT_UNLOCK(lchip);

        return ret;
    }
    else if(CTC_APP_GEM_PORT_UPDATE_IGS_POLICER == p_gem_port->update_type)
    {
        ctc_qos_policer_t policer;
        ctc_vlan_mapping_t vlan_mapping;

        if (p_gem_port->ingress_policer_id == p_gem_port_db->ingress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        /*-----------------------------------------*/
        /*Ingress policetr*/
        sal_memset(&policer, 0, sizeof(policer));
        if (p_gem_port->ingress_policer_id)
        {
            policer.type = CTC_QOS_POLICER_TYPE_FLOW;
            policer.id.policer_id = p_gem_port->ingress_policer_id;
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_qos_get_policer(&policer));
        }

        /*Update ingress*/
        sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
        vlan_mapping.old_svid = p_gem_port->tunnel_value;

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_get_vlan_mapping(p_gem_port->port, &vlan_mapping));
        vlan_mapping.policer_id = p_gem_port->ingress_policer_id;

        if (1 == policer.hbwp_en)
        {
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN);
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_update_vlan_mapping(p_gem_port->port, &vlan_mapping));

        p_gem_port_db->ingress_policer_id = p_gem_port->ingress_policer_id;
    }
    else if(CTC_APP_GEM_PORT_UPDATE_EGS_POLICER == p_gem_port->update_type)
    {
        ctc_qos_policer_t policer;
        uint16 logic_dest_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];
#ifdef DUET2
        uint32 entry_id = 0;
        ctc_scl_field_action_t field_action;
#endif
#ifdef GREATBELT
        if (p_gem_port->egress_policer_id == p_gem_port_db->egress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        /*-----------------------------------------*/
        /*Egress policetr*/
        if (p_gem_port->egress_policer_id)
        {
            uint16 policer_ptr = 0;

            sal_memset(&policer, 0, sizeof(policer));
            policer.type = CTC_QOS_POLICER_TYPE_FLOW;
            policer.id.policer_id = p_gem_port->egress_policer_id;
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_qos_get_policer(&policer));

            if (0 == policer.hbwp_en)
            {
                CTC_APP_VLAN_PORT_UNLOCK(lchip);
                return CTC_E_INVALID_CONFIG;
            }

            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_qos_policer_index_get, lchip, CTC_QOS_POLICER_TYPE_FLOW, p_gem_port->egress_policer_id, &policer_ptr));

            logic_dest_port = policer_ptr/4;
        }

        /*Update egress*/
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_update_logic_port, lchip, p_gem_port_db->xlate_nhid, logic_dest_port));
#endif
#ifdef DUET2
        if ((p_gem_port->egress_policer_id == p_gem_port_db->egress_policer_id)
            && (p_gem_port->egress_cos_idx == p_gem_port_db->egress_cos_idx))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        entry_id = ENCODE_SCL_DOWNLINK_ILOOP_ENTRY_ID(p_gem_port_db->custom_id);
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_COS_HBWP_POLICER;
        field_action.data0 = p_gem_port->egress_policer_id;
        field_action.data1 = p_gem_port->egress_cos_idx;
        if (p_gem_port->egress_policer_id)
        {
            sal_memset(&policer, 0, sizeof(policer));
            policer.type = CTC_QOS_POLICER_TYPE_FLOW;
            policer.id.policer_id = p_gem_port->egress_policer_id;
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_qos_get_policer(&policer));
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_add_action_field(entry_id, &field_action));
        }
        else
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_remove_action_field(entry_id, &field_action));
        }
        p_gem_port_db->egress_cos_idx = p_gem_port->egress_cos_idx;
#endif
        p_gem_port_db->logic_dest_port = logic_dest_port;
        p_gem_port_db->egress_policer_id = p_gem_port->egress_policer_id;
    }
    else if (CTC_APP_GEM_PORT_UPDATE_IGS_STATS == p_gem_port->update_type)
    {
        uint32 entry_id = 0;
        ctc_acl_field_action_t action_field;

        if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;
        }

        entry_id = ENCODE_ACL_HASH_ENTRY_ID(p_gem_port->port, p_gem_port->tunnel_value);
        if (p_gem_port->ingress_stats_id == p_gem_port_db->ingress_stats_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        /*-----------------------------------------*/
        /*Ingress stats*/
        sal_memset(&action_field, 0, sizeof(action_field));
        action_field.type = CTC_ACL_FIELD_ACTION_STATS;
        action_field.data0 = p_gem_port->ingress_stats_id;
        if (p_gem_port->ingress_stats_id)
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_acl_add_action_field(entry_id, &action_field));
        }
        else
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_acl_remove_action_field(entry_id, &action_field));
        }
        p_gem_port_db->ingress_stats_id = p_gem_port->ingress_stats_id;
    }
    else if (CTC_APP_GEM_PORT_UPDATE_EGS_STATS == p_gem_port->update_type)
    {
        ctc_vlan_edit_nh_param_t vlan_xlate_nh;

        if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;
        }

        sal_memset(&vlan_xlate_nh, 0, sizeof(ctc_vlan_edit_nh_param_t));
        vlan_xlate_nh.gport_or_aps_bridge_id = p_gem_port->port;
        vlan_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
        vlan_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
        vlan_xlate_nh.vlan_edit_info.output_svid = p_gem_port->tunnel_value;
        vlan_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
        CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
        vlan_xlate_nh.vlan_edit_info.stag_cos = 0;
        if (p_gem_port->egress_stats_id)
        {
            vlan_xlate_nh.vlan_edit_info.stats_id = p_gem_port->egress_stats_id;
            vlan_xlate_nh.vlan_edit_info.flag |= CTC_VLAN_NH_STATS_EN;
        }
        CTC_APP_DBG_INFO("update gem port xlate nhid: %d, stats_id %d\n", p_gem_port_db->xlate_nhid, p_gem_port->egress_stats_id);
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_update_xlate,
                    lchip, p_gem_port_db->xlate_nhid, &vlan_xlate_nh.vlan_edit_info, &vlan_xlate_nh));
        p_gem_port_db->egress_stats_id = p_gem_port->egress_stats_id;
    }
    else if (CTC_APP_GEM_PORT_UPDATE_STATION_MOVE_ACTION == p_gem_port->update_type)
    {
        uint32 station_move_action = 0;
        if (p_gem_port->station_move_action >= CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD_TOCPU)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        station_move_action = p_gem_port->station_move_action;
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_fdb_set_station_move_action,
                    lchip, p_gem_port_db->ad_index, station_move_action));

        ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_hash,
                          (hash_traversal_fn)_ctc_gbx_app_vlan_port_update_station_move_action, (void*)p_gem_port);
        p_gem_port_db->station_move_action = p_gem_port->station_move_action;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t *p_gem_port)
{
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_gem_port->port);
    if (CTC_IS_LINKAGG_PORT(p_gem_port->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    p_gem_port->ingress_policer_id = p_gem_port_db->ingress_policer_id;
    p_gem_port->egress_policer_id = p_gem_port_db->egress_policer_id;
    p_gem_port->ingress_stats_id = p_gem_port_db->ingress_stats_id;
    p_gem_port->egress_stats_id = p_gem_port_db->egress_stats_id;
    p_gem_port->logic_port = p_gem_port_db->logic_port;
    p_gem_port->ga_gport =  p_uni_port->iloop_port;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _____GLOBAL_PROPERTY_____ ""

struct ctc_app_vlan_port_vlan_mapping_data_s
{
    uint32  port;
    uint32  tunnel_value;
    uint8   lchip;
    uint8   is_add;
    uint32  vlan_range_grp;
};
typedef struct ctc_app_vlan_port_vlan_mapping_data_s ctc_app_vlan_port_vlan_mapping_data_t;

STATIC int32
_ctc_gbx_app_vlan_port_update_vlan_mapping(void* bucket_data, void* user_data)
{
    ctc_app_vlan_port_db_t* p_vlan_port_db = (ctc_app_vlan_port_db_t*)bucket_data;
    ctc_app_vlan_port_vlan_mapping_data_t* p_user_data = (ctc_app_vlan_port_vlan_mapping_data_t*)user_data;
    ctc_vlan_mapping_t vlan_mapping;

    if ((p_vlan_port_db->port != p_user_data->port) || (p_vlan_port_db->tunnel_value != p_user_data->tunnel_value))
    {
        return CTC_E_NONE;
    }
    sal_memset(&vlan_mapping, 0 , sizeof(vlan_mapping));
    vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.old_svid = p_vlan_port_db->match_svlan;
    vlan_mapping.old_cvid = p_vlan_port_db->match_cvlan;
    if (p_vlan_port_db->match_svlan_end)
    {
        vlan_mapping.svlan_end = p_vlan_port_db->match_svlan_end;
        vlan_mapping.vrange_grpid = p_user_data->vlan_range_grp;
    }
    CTC_ERROR_RETURN(ctc_vlan_get_vlan_mapping(p_vlan_port_db->p_gem_port_db->logic_port, &vlan_mapping));
    if (p_user_data->is_add)
    {
        CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN);
        vlan_mapping.l2vpn_oam_id = 3;
        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan mapping fwd add\n");
    }
    else
    {
        CTC_UNSET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN);
        vlan_mapping.l2vpn_oam_id = 0;
        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan mapping fwd remove\n");
    }
    CTC_ERROR_RETURN(ctc_vlan_update_vlan_mapping(p_vlan_port_db->p_gem_port_db->logic_port, &vlan_mapping));
    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_update_mac_limit_action(uint8 lchip, uint32 mac_limit_action, ctc_app_vlan_port_gem_port_db_t* p_gem_port_db, uint8 is_add)
{
    uint8 update_acl_flow = FALSE;
    uint16 lport = 0;
    uint32 entry_id = 0;
    ctc_field_key_t field_key;
    ctc_acl_field_action_t acl_field_action;
    ctc_acl_to_cpu_t acl_cp_to_cpu;
    ctc_app_vlan_port_vlan_mapping_data_t user_data;
    ctc_app_vlan_port_db_t vlan_port_db_key;
    ctc_app_vlan_port_uni_db_t* p_vlan_port_uni_db = NULL;
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&acl_field_action, 0, sizeof(ctc_acl_field_action_t));
    sal_memset(&acl_cp_to_cpu, 0, sizeof(ctc_acl_to_cpu_t));
    sal_memset(&user_data, 0, sizeof(ctc_app_vlan_port_vlan_mapping_data_t));
    sal_memset(&vlan_port_db_key, 0, sizeof(ctc_app_vlan_port_db_t));
    entry_id = ENCODE_ACL_HASH_ENTRY_ID(p_gem_port_db->port, p_gem_port_db->tunnel_value);
    switch(mac_limit_action)
    {
        case CTC_MACLIMIT_ACTION_DISCARD:
            acl_field_action.type = CTC_ACL_FIELD_ACTION_METADATA;
            acl_field_action.data0 = ENCODE_ACL_METADATA(1);
            update_acl_flow = TRUE;
            break;
        case CTC_MACLIMIT_ACTION_FWD:
            break;
        case CTC_MACLIMIT_ACTION_TOCPU:
            acl_field_action.type = CTC_ACL_FIELD_ACTION_METADATA;
            acl_field_action.data0 = ENCODE_ACL_METADATA(3);
            update_acl_flow = TRUE;
            break;
        default:
            return CTC_E_NONE;
    }

    if (update_acl_flow)
    {
        is_add? ctc_acl_add_action_field(entry_id, &acl_field_action): ctc_acl_remove_action_field(entry_id, &acl_field_action);
    }
    /*update ONU service*/
    user_data.lchip = lchip;
    user_data.is_add = is_add;
    user_data.port = p_gem_port_db->port;
    user_data.tunnel_value = p_gem_port_db->tunnel_value;
    lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_db->port);
    p_vlan_port_uni_db= &p_g_app_vlan_port_master->p_port_pon[lport];
    user_data.vlan_range_grp = p_vlan_port_uni_db->vlan_range_grp;
    ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_key_hash, _ctc_gbx_app_vlan_port_update_vlan_mapping, &user_data);
    /*update PON service*/
    acl_field_action.type = CTC_ACL_FIELD_ACTION_REDIRECT;
    acl_field_action.data0 = p_vlan_port_uni_db->e2iloop_nhid;
    ctc_acl_add_action_field(entry_id, &acl_field_action);
    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_set_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    int32 ret = CTC_E_NONE;
    ctc_app_vlan_port_mac_learn_limit_t* p_mac_learn = NULL;
    ctc_security_learn_limit_t learn_limit;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    sal_memset(&learn_limit, 0, sizeof(ctc_security_learn_limit_t));
    p_mac_learn = &p_glb_cfg->mac_learn;
    switch(p_mac_learn->limit_type)
    {
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID:
            sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
            port_fid_db.fid = p_mac_learn->fid;
            ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
            if (!ret)
            {
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }

            p_port_fid_db = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
            if (NULL == p_port_fid_db)
            {
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }

            if ((p_mac_learn->limit_num == CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE))
            {
                CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, CTC_MACLIMIT_ACTION_NONE);
            }
            else if ((p_mac_learn->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_action != p_mac_learn->limit_action))
            {
                CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
            }

            if (0 == p_mac_learn->limit_num)
            {
                CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, p_mac_learn->limit_action);
            }
            p_port_fid_db->limit_action = p_mac_learn->limit_action;
            p_port_fid_db->limit_num = p_mac_learn->limit_num;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT:
            sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
            gem_port_db.port = p_mac_learn->gport;
            gem_port_db.tunnel_value = p_mac_learn->tunnel_value;
            p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
            if (NULL == p_gem_port_db)
            {
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }

            if ((p_mac_learn->limit_num == CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE))
            {
                _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_db->limit_action, p_gem_port_db, FALSE);
                p_gem_port_db->mac_security = CTC_MACLIMIT_ACTION_NONE;
            }
            else if ((p_mac_learn->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_action != p_mac_learn->limit_action))
            {
                CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
            }

            if ((0 == p_mac_learn->limit_num) && (p_mac_learn->limit_action != CTC_MACLIMIT_ACTION_NONE))
            {
                _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_mac_learn->limit_action, p_gem_port_db, TRUE);
                p_gem_port_db->mac_security = p_mac_learn->limit_action;
            }
            if (p_mac_learn->limit_num > p_gem_port_db->limit_num)  /*support update limit num*/
            {
                _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_mac_learn->limit_action, p_gem_port_db, FALSE);
            }
            p_gem_port_db->limit_num = p_mac_learn->limit_num;
            p_gem_port_db->limit_action = p_mac_learn->limit_action;
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_get_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    int32 ret = CTC_E_NONE;
    ctc_app_vlan_port_mac_learn_limit_t* p_mac_learn = NULL;
    ctc_security_learn_limit_t learn_limit;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    sal_memset(&learn_limit, 0, sizeof(ctc_security_learn_limit_t));
    p_mac_learn = &p_glb_cfg->mac_learn;
    switch(p_mac_learn->limit_type)
    {
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID:
            sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
            port_fid_db.fid = p_mac_learn->fid;
            ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
            if (!ret)
            {
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }

            p_port_fid_db = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
            if (NULL == p_port_fid_db)
            {
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }
            p_mac_learn->limit_count = p_port_fid_db->limit_count;
            p_mac_learn->limit_num = p_port_fid_db->limit_num;
            p_mac_learn->limit_action = p_port_fid_db->limit_action;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT:
            sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
            gem_port_db.port = p_mac_learn->gport;
            gem_port_db.tunnel_value = p_mac_learn->tunnel_value;
            p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
            if (NULL == p_gem_port_db)
            {
                CTC_APP_VLAN_PORT_UNLOCK(lchip);
                CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
            }

            p_mac_learn->limit_count = p_gem_port_db->limit_count;
            p_mac_learn->limit_num = p_gem_port_db->limit_num;
            p_mac_learn->limit_action = p_gem_port_db->limit_action;
            break;
        default:
            break;
    }
    return CTC_E_NONE;
}

#define _____VLAN_PORT_____ ""

STATIC int32
_ctc_gbx_app_vlan_port_vlan_edit(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port,
                                uint16* pkt_svid,
                                uint16* pkt_cvid)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    if (CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.svid)
    {
        if(CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.cvid)
        {
            *pkt_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            *pkt_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
        }
        else
        {
            *pkt_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            if (p_vlan_port->match_svlan_end)
            {
                *pkt_cvid = p_vlan_port->match_cvlan;
            }
            else
            {
                *pkt_cvid = p_vlan_port->match_svlan;
            }
        }
    }
    else if(CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.svid)
    {
        if (CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.cvid)
        {
            *pkt_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            *pkt_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
        }
        else if (CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.cvid)
        {
            *pkt_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            *pkt_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
        }
        else
        {
            *pkt_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            *pkt_cvid = p_vlan_port->match_cvlan;
        }
    }
    else if(CTC_APP_VLAN_ACTION_NONE == p_vlan_port->ingress_vlan_action_set.svid)
    {
        if (CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.cvid)
        {
            *pkt_svid = p_vlan_port->match_svlan;
            *pkt_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
        }
        else if (CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.cvid)
        {
            *pkt_svid = p_vlan_port->match_svlan;
            *pkt_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
        }
        else
        {
            *pkt_svid = p_vlan_port->match_svlan;
            *pkt_cvid = p_vlan_port->match_cvlan;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_vlan_mapping(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port,
                               ctc_vlan_mapping_t *p_vlan_mapping)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    /*SVLAN op*/
    switch(p_vlan_port->ingress_vlan_action_set.svid)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            if (0 == p_vlan_port->ingress_vlan_action_set.new_svid)
            {
                return CTC_E_INVALID_PARAM;
            }
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
            p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_ADD;
            p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            if (0 == p_vlan_port->ingress_vlan_action_set.new_svid)
            {
                return CTC_E_INVALID_PARAM;
            }
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
            p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
            p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            break;

        case CTC_APP_VLAN_ACTION_DEL:
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
            p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_DEL;
            return CTC_E_INVALID_PARAM;
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
            p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_VALID;
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.cvid)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            if (0 == p_vlan_port->ingress_vlan_action_set.new_cvid)
            {
                return CTC_E_INVALID_PARAM;
            }
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
            p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_ADD;
            p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            if (0 == p_vlan_port->ingress_vlan_action_set.new_cvid)
            {
                return CTC_E_INVALID_PARAM;
            }
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
            p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
            p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            break;

        case CTC_APP_VLAN_ACTION_DEL:
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
            p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_DEL;
            return CTC_E_INVALID_PARAM;
            break;


         case CTC_APP_VLAN_ACTION_NONE:
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.scos)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_scos = p_vlan_port->ingress_vlan_action_set.new_scos;
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_scos = p_vlan_port->ingress_vlan_action_set.new_scos;
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
            if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->stag_op)
            {
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
            }
            break;

        case  CTC_APP_VLAN_ACTION_NONE:
            p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.ccos)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_ccos = p_vlan_port->ingress_vlan_action_set.new_ccos;
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
          break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
            p_vlan_mapping->new_ccos = p_vlan_port->ingress_vlan_action_set.new_ccos;
            CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
            if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->ctag_op)
            {
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
            }
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_nh_xlate_mapping(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port,
                               ctc_vlan_egress_edit_info_t *p_xlate)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    /* SVLAN op */
    switch(p_vlan_port->egress_vlan_action_set.svid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_svid = p_vlan_port->egress_vlan_action_set.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_APP_VLAN_ACTION_ADD:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_svid = p_vlan_port->egress_vlan_action_set.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CVLAN op */
    switch(p_vlan_port->egress_vlan_action_set.cvid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
#ifdef DUET2
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
#endif
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_cvid = p_vlan_port->egress_vlan_action_set.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;

        case CTC_APP_VLAN_ACTION_NONE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_APP_VLAN_ACTION_ADD:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_cvid = p_vlan_port->egress_vlan_action_set.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;


        default:
            return CTC_E_INVALID_PARAM;
    }

    /* SCOS op */
    switch(p_vlan_port->egress_vlan_action_set.scos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
            p_xlate->stag_cos = p_vlan_port->egress_vlan_action_set.new_scos;

         break;
        case CTC_APP_VLAN_ACTION_DEL:
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CCOS op */
    switch(p_vlan_port->egress_vlan_action_set.ccos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            return CTC_E_INVALID_PARAM;

        case CTC_APP_VLAN_ACTION_DEL:
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_igs_nh_xlate_mapping(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port,
                               ctc_vlan_egress_edit_info_t *p_xlate)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    /* SVLAN op */
    switch(p_vlan_port->ingress_vlan_action_set.svid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_APP_VLAN_ACTION_ADD:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CVLAN op */
    switch(p_vlan_port->ingress_vlan_action_set.cvid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;

        case CTC_APP_VLAN_ACTION_NONE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_APP_VLAN_ACTION_ADD:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;


        default:
            return CTC_E_INVALID_PARAM;
    }

    /* SCOS op */
    switch(p_vlan_port->ingress_vlan_action_set.scos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
            p_xlate->stag_cos = p_vlan_port->ingress_vlan_action_set.new_scos;

         break;
        case CTC_APP_VLAN_ACTION_DEL:
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CCOS op */
    switch(p_vlan_port->ingress_vlan_action_set.ccos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            return CTC_E_INVALID_PARAM;

        case CTC_APP_VLAN_ACTION_DEL:
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_scl_xlate_mapping(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port,
                               ctc_scl_vlan_edit_t *p_xlate)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    /* SVLAN op */
    switch(p_vlan_port->egress_vlan_action_set.svid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
            p_xlate->stag_op = CTC_VLAN_TAG_OP_DEL;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->stag_op = CTC_VLAN_TAG_OP_REP;
            p_xlate->svid_sl = CTC_VLAN_TAG_SL_NEW;
            p_xlate->svid_new = p_vlan_port->egress_vlan_action_set.new_svid;
            break;

        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CVLAN op */
    switch(p_vlan_port->egress_vlan_action_set.cvid)
    {
        case CTC_APP_VLAN_ACTION_DEL:
            p_xlate->ctag_op = CTC_VLAN_TAG_OP_DEL;
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->ctag_op = CTC_VLAN_TAG_OP_REP;
            p_xlate->cvid_sl = CTC_VLAN_TAG_SL_NEW;
            p_xlate->cvid_new = p_vlan_port->egress_vlan_action_set.new_cvid;
            break;

        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        default:
            return CTC_E_INVALID_PARAM;
    }


    /* SCOS op */
    switch(p_vlan_port->egress_vlan_action_set.scos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            p_xlate->stag_op = CTC_VLAN_TAG_OP_REP;
            p_xlate->scos_sl = CTC_VLAN_TAG_SL_NEW;
            p_xlate->scos_new = p_vlan_port->egress_vlan_action_set.new_scos;

         break;
        case CTC_APP_VLAN_ACTION_DEL:
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CCOS op */
    switch(p_vlan_port->egress_vlan_action_set.ccos)
    {
        case CTC_APP_VLAN_ACTION_NONE:
            break;

        case CTC_APP_VLAN_ACTION_ADD:
        case CTC_APP_VLAN_ACTION_REPLACE:
            return CTC_E_INVALID_PARAM;

        case CTC_APP_VLAN_ACTION_DEL:
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_add_vlan(uint8 lchip,
                           ctc_app_vlan_port_t *p_vlan_port,
                           ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    int32 ret = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /*Add vlan member*/
    if (NULL != p_gem_port_db)
    {
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = p_vlan_port_db->fid_mapping;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid_mapping;
        l2dflt.with_nh = 1;


        l2dflt.member.nh_id = p_vlan_port_db->xlate_nhid;
        CTC_ERROR_RETURN(ctc_l2_add_port_to_default_entry(&l2dflt));


        CTC_APP_DBG_INFO("Group(%d) add ONU nhid: %d\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
    }

    return ret;
}

int32
ctc_gbx_app_vlan_port_remove_vlan(uint8 lchip,
                           ctc_app_vlan_port_t *p_vlan_port,
                           ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    int32 ret = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /*Add vlan member*/
    if (NULL != p_gem_port_db)
    {
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = p_vlan_port_db->fid_mapping;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid_mapping;
        l2dflt.with_nh = 1;


        l2dflt.member.nh_id = p_vlan_port_db->xlate_nhid;
        ctc_l2_remove_port_from_default_entry(&l2dflt);


        CTC_APP_DBG_INFO("Group(%d) add ONU nhid: %d\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
    }

    return ret;
}

int32
ctc_gbx_app_vlan_port_downlink_scl_add(uint8 lchip,
                                    ctc_app_vlan_port_t *p_vlan_port,
                                    ctc_app_vlan_port_db_t *p_vlan_port_db,
                                    ctc_app_vlan_port_uni_db_t *p_uni_port)
{
    int32 ret = 0;
    ctc_vlan_range_info_t range_info;
    ctc_vlan_range_t vlan_range;
    ctc_scl_entry_t  scl_entry;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    sal_memset(&range_info, 0, sizeof(range_info));
    sal_memset(&vlan_range, 0, sizeof(vlan_range));

    if (p_vlan_port->match_svlan_end)
    {
        range_info.direction = CTC_EGRESS;
        range_info.vrange_grpid = p_uni_port->vlan_range_grp;
        vlan_range.vlan_start = p_vlan_port->match_svlan;
        vlan_range.vlan_end   = p_vlan_port->match_svlan_end;

        CTC_ERROR_GOTO(ctc_vlan_add_vlan_range(&range_info, &vlan_range), ret, roll_back_0);
    }

    sal_memset(&scl_entry, 0, sizeof(scl_entry));

    scl_entry.entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);

    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
    {
        scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_LOGIC_PORT;
        scl_entry.key.u.hash_port_key.gport = p_gem_port_db->logic_port;
    }
    else
    {
        scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT;
        scl_entry.key.u.hash_port_key.gport = p_uni_port->e2eloop_port;
    }

    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    scl_entry.key.u.hash_port_2vlan_key.dir = CTC_EGRESS;
    scl_entry.key.u.hash_port_2vlan_key.svlan = p_vlan_port_db->pkt_svlan;

    if (p_vlan_port->match_svlan_end)
    {
        scl_entry.key.u.hash_port_2vlan_key.cvlan = p_vlan_port->match_svlan_end;
    }
    else
    {
        scl_entry.key.u.hash_port_2vlan_key.cvlan = p_vlan_port_db->pkt_cvlan;
    }

    scl_entry.action.type =  CTC_SCL_ACTION_EGRESS;
    CTC_SET_FLAG(scl_entry.action.u.egs_action.block_pkt_type, CTC_SCL_BLOCKING_UNKNOW_MCAST);
    CTC_SET_FLAG(scl_entry.action.u.egs_action.flag, CTC_SCL_EGS_ACTION_FLAG_VLAN_EDIT);

    if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN) && (0 != p_vlan_port->egress_stats_id))
    {
        scl_entry.action.u.egs_action.stats_id = p_vlan_port->egress_stats_id;
        CTC_SET_FLAG(scl_entry.action.u.egs_action.flag, CTC_SCL_EGS_ACTION_FLAG_STATS);
    }

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_scl_xlate_mapping(lchip, p_vlan_port, &scl_entry.action.u.egs_action.vlan_edit), ret, roll_back_1);

    ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry);

    if (ret != CTC_E_NONE)
    {
        /*HASH conflict*/
        if (ret == CTC_E_HASH_CONFLICT &&
            (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ))
        {
            p_vlan_port_db->egs_vlan_mapping_en = 0;
            ret = CTC_E_NONE;
            goto roll_back_1;
        }

        /*Error return*/
        goto roll_back_1;
    }

    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_2);
    CTC_APP_DBG_INFO("entry_id %d\n", scl_entry.entry_id);

    /* for pon bc qinq*/
    if (p_vlan_port_db->criteria != CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN)
    {
        scl_entry.entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 1);
        scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT;
        scl_entry.key.u.hash_port_key.gport = p_uni_port->bc_eloop_port;
        ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry);

        if (ret != CTC_E_NONE)
        {
            /*HASH conflict*/
            if (ret == CTC_E_HASH_CONFLICT &&
                (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ))
            {
                p_vlan_port_db->egs_vlan_mapping_en = 0;
                ret = CTC_E_NONE;
                goto roll_back_3;
            }

            /*Error return*/
            goto roll_back_3;
        }
        CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_4);
        CTC_APP_DBG_INFO("bc entry_id %d\n", scl_entry.entry_id);

    }
    p_vlan_port_db->egress_stats_id = p_vlan_port->egress_stats_id;

    return CTC_E_NONE;

roll_back_4:
    ctc_scl_remove_entry(scl_entry.entry_id);

roll_back_3:
    scl_entry.entry_id -= 1;
    ctc_scl_uninstall_entry(scl_entry.entry_id);
roll_back_2:
    ctc_scl_remove_entry(scl_entry.entry_id);

roll_back_1:
    if (p_vlan_port->match_svlan_end)
    {
        ctc_vlan_remove_vlan_range(&range_info, &vlan_range);
    }

roll_back_0:

    return ret;
}

int32
ctc_gbx_app_vlan_port_downlink_scl_remove(uint8 lchip,
                                      ctc_app_vlan_port_db_t *p_vlan_port_db,
                                      ctc_app_vlan_port_uni_db_t *p_uni_port)
{
    ctc_vlan_range_info_t range_info;
    ctc_vlan_range_t vlan_range;
    uint32 entry_id = 0;

    sal_memset(&range_info, 0, sizeof(range_info));
    sal_memset(&vlan_range, 0, sizeof(vlan_range));

    entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);
    CTC_ERROR_RETURN(ctc_scl_uninstall_entry(entry_id));
    CTC_ERROR_RETURN(ctc_scl_remove_entry(entry_id));

    if (p_vlan_port_db->criteria != CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN)
    {
        entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 1);
        CTC_ERROR_RETURN(ctc_scl_uninstall_entry(entry_id));
        CTC_ERROR_RETURN(ctc_scl_remove_entry(entry_id));
    }

    if (p_vlan_port_db->match_svlan_end)
    {
        range_info.direction = CTC_EGRESS;
        range_info.vrange_grpid = p_uni_port->vlan_range_grp;
        vlan_range.vlan_start = p_vlan_port_db->match_svlan;
        vlan_range.vlan_end   = p_vlan_port_db->match_svlan_end;
        CTC_ERROR_RETURN(ctc_vlan_remove_vlan_range(&range_info, &vlan_range));
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_downlink_nexthop_add(uint8 lchip,
                                        ctc_app_vlan_port_t *p_vlan_port,
                                        ctc_app_vlan_port_db_t *p_vlan_port_db,
                                        ctc_app_vlan_port_uni_db_t *p_uni_port)
{
    int32 ret = 0;
#ifdef GREATBELT
    uint32 loop_nhid = 0;
#endif
    ctc_vlan_edit_nh_param_t vlan_xlate_nh;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

#ifdef GREATBELT
    loop_nhid = p_gem_port_db->xlate_nhid;
    if (CTC_APP_VLAN_ACTION_DEL == p_vlan_port->egress_vlan_action_set.cvid)
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->xlate_nhid_onu));

        sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
        vlan_xlate_nh.dsnh_offset = p_vlan_port_db->fid;
        vlan_xlate_nh.gport_or_aps_bridge_id = p_uni_port->onu_e2eloop_sec_port;
        vlan_xlate_nh.vlan_edit_info.loop_nhid = loop_nhid;
        CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG); /*logicRep*/
        vlan_xlate_nh.logic_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];
        vlan_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
        CTC_APP_DBG_INFO("nh xlate onu nhid: %d\n", p_vlan_port_db->xlate_nhid_onu);
        CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->xlate_nhid_onu, &vlan_xlate_nh), ret, roll_back_0);
        loop_nhid = p_vlan_port_db->xlate_nhid_onu;
    }
#endif
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_offset(lchip, &p_vlan_port_db->ad_index), ret, roll_back_1);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->xlate_nhid), ret, roll_back_2);

    /* Create xlate nh */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.dsnh_offset = p_vlan_port_db->fid;
    vlan_xlate_nh.gport_or_aps_bridge_id = p_uni_port->onu_e2eloop_port;
#ifdef GREATBELT
    vlan_xlate_nh.vlan_edit_info.loop_nhid = loop_nhid;
    vlan_xlate_nh.logic_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];
#else
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_g_app_vlan_port_master->downlink_iloop_nhid[p_uni_port->vdev_id];
    vlan_xlate_nh.logic_port = p_gem_port_db->custom_id;
#endif
    /*CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG);*/ /*logicRep*/
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_ETREE_LEAF);

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_nh_xlate_mapping(lchip, p_vlan_port, &vlan_xlate_nh.vlan_edit_info), ret, roll_back_3);
    if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN) && (0 != p_vlan_port->egress_stats_id))
    {
        vlan_xlate_nh.vlan_edit_info.stats_id = p_vlan_port->egress_stats_id;
        vlan_xlate_nh.vlan_edit_info.flag |= CTC_VLAN_NH_STATS_EN;
    }
    CTC_APP_DBG_INFO("nh xlate nhid: %d\n", p_vlan_port_db->xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->xlate_nhid, &vlan_xlate_nh), ret, roll_back_3);
    p_vlan_port_db->logic_dest_port = p_vlan_port_db->logic_port;

    if (p_vlan_port_db->logic_port_b_en)
    {
        CTC_ERROR_GOTO(ctc_l2_set_nhid_by_logic_port(p_vlan_port_db->logic_port, p_vlan_port_db->xlate_nhid), ret, roll_back_4);
    }
    p_vlan_port_db->egs_vlan_mapping_en = 0;
    p_vlan_port_db->egress_stats_id = p_vlan_port->egress_stats_id;

    return CTC_E_NONE;

roll_back_4:
    ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid);

roll_back_3:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid);

roll_back_2:
    _ctc_gbx_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index);

roll_back_1:
#ifdef GREATBELT
    if ((CTC_APP_VLAN_ACTION_DEL == p_vlan_port->egress_vlan_action_set.cvid))
    {
        ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid_onu);
    }

roll_back_0:
    if ((CTC_APP_VLAN_ACTION_DEL == p_vlan_port->egress_vlan_action_set.cvid))
    {
        _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid_onu);
    }
#endif

    return ret;
}

int32
ctc_gbx_app_vlan_port_downlink_nexthop_remove(uint8 lchip,
                                      ctc_app_vlan_port_db_t *p_vlan_port_db,
                                      ctc_app_vlan_port_uni_db_t *p_uni_port)
{
    ctc_opf_t opf;

    sal_memset(&opf, 0, sizeof(opf));
#ifdef GREATBELT
    if ((CTC_APP_VLAN_ACTION_DEL == p_vlan_port_db->egress_vlan_action_set.cvid))
    {
        CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid_onu));
        _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid_onu);
    }
#endif

    if (p_vlan_port_db->logic_port_b_en)
    {
        CTC_ERROR_RETURN(ctc_l2_set_nhid_by_logic_port(p_vlan_port_db->logic_port, 0));
    }

    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid));

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid));
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index));

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_basic_create(uint8 lchip,
                             ctc_app_vlan_port_t *p_vlan_port,
                             ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    uint32 vlan_mapping_port = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fid_db_t* p_port_fid = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;


    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_vlan_edit(lchip, p_vlan_port, &pkt_svlan, &pkt_cvlan), ret, roll_back_0);
    p_vlan_port_db->pkt_svlan = pkt_svlan;
    p_vlan_port_db->pkt_cvlan = pkt_cvlan;

    if (0 == p_vlan_port_db->pkt_svlan)
    {
        ctc_port_get_default_vlan(p_vlan_port_db->port, &p_vlan_port_db->fid);
    }
    else
    {
        p_vlan_port_db->fid = p_vlan_port_db->pkt_svlan;
    }

    if ( p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN )
    {
        p_vlan_port_db->egs_vlan_mapping_en = 1;
    }

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_downlink_scl_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }

    if (0 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_downlink_nexthop_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }

    /* Add vlan mapping */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
    {
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        vlan_mapping_port =  p_gem_port_db->logic_port;
    }
    else
    {
        vlan_mapping_port = p_uni_port->iloop_port;
    }

    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.old_svid = p_vlan_port->match_svlan;
    vlan_mapping.old_cvid = p_vlan_port->match_cvlan;

    /* Add vlan range */
    if (p_vlan_port->match_svlan_end)
    {
        ctc_vlan_range_info_t range_info;
        ctc_vlan_range_t vlan_range;

        sal_memset(&range_info, 0, sizeof(range_info));
        sal_memset(&vlan_range, 0, sizeof(vlan_range));

        range_info.direction = CTC_INGRESS;
        range_info.vrange_grpid = p_uni_port->vlan_range_grp;
        vlan_range.vlan_start = p_vlan_port->match_svlan;
        vlan_range.vlan_end   = p_vlan_port->match_svlan_end;

        CTC_ERROR_GOTO(ctc_vlan_add_vlan_range(&range_info, &vlan_range), ret, roll_back_1);

        vlan_mapping.svlan_end = p_vlan_port->match_svlan_end;
        vlan_mapping.vrange_grpid = p_uni_port->vlan_range_grp;
    }

    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
    {
        /*known onu service*/
        CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
        vlan_mapping.logic_src_port = p_vlan_port_db->logic_port;
        if ((p_gem_port_db->limit_num == p_gem_port_db->limit_count) && (p_gem_port_db->limit_action != CTC_MACLIMIT_ACTION_NONE))
        {
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_MAC_LIMIT_DISCARD_EN);
            vlan_mapping.l2vpn_oam_id = 3;
        }
    }

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_vlan_mapping(lchip, p_vlan_port, &vlan_mapping), ret, roll_back_2);
    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = pkt_svlan;
    if (0 == p_vlan_port->match_svlan_end)
    {
        port_fid.pkt_cvlan = pkt_cvlan;
    }
    CTC_ERROR_GOTO(ctc_spool_add(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL, &p_port_fid), ret, roll_back_2);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_FID);
    vlan_mapping.u3.fid = p_port_fid->fid;
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_VLANPTR);
    vlan_mapping.user_vlanptr = p_port_fid->fid;

    p_vlan_port_db->fid_mapping = p_port_fid->fid;
    if (CTC_BMP_ISSET(p_g_app_vlan_port_master->vlan_isolate_bmp, p_vlan_port->match_svlan))
    {
        CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_ETREE_LEAF);
    }
    if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN) && (0 != p_vlan_port->ingress_stats_id))
    {
        vlan_mapping.stats_id = p_vlan_port->ingress_stats_id;
        vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_STATS_EN;
    }

    ret = ctc_vlan_add_vlan_mapping(vlan_mapping_port , &vlan_mapping);
    if (ret ==  CTC_E_HASH_CONFLICT)
    {
        CTC_APP_DBG_INFO("Hash Confict!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
        CTC_ERROR_GOTO(ctc_vlan_add_vlan_mapping(vlan_mapping_port , &vlan_mapping), ret, roll_back_3);
        p_vlan_port_db->igs_vlan_mapping_use_flex = 1;
    }
    else if(ret != CTC_E_NONE)
    {
        goto roll_back_3;
    }
    p_vlan_port_db->fid = p_vlan_port_db->fid_mapping;
    p_vlan_port_db->ingress_stats_id = p_vlan_port->ingress_stats_id;

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_3:
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);

roll_back_2:
    if (p_vlan_port->match_svlan_end)
    {
        ctc_vlan_range_info_t range_info;
        ctc_vlan_range_t vlan_range;

        sal_memset(&range_info, 0, sizeof(range_info));
        sal_memset(&vlan_range, 0, sizeof(vlan_range));

        range_info.direction = CTC_INGRESS;
        range_info.vrange_grpid = p_uni_port->vlan_range_grp;
        vlan_range.vlan_start = p_vlan_port->match_svlan;
        vlan_range.vlan_end   = p_vlan_port->match_svlan_end;
        ctc_vlan_remove_vlan_range(&range_info, &vlan_range);
    }

roll_back_1:
    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        ctc_gbx_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db, p_uni_port);
    }
    if (0 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        ctc_gbx_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

roll_back_0:

    return ret;
}

int32
ctc_gbx_app_vlan_port_basic_destroy(uint8 lchip,
                             ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    uint32 vlan_mapping_port = 0;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_app_vlan_port_fid_db_t port_fid;

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /* Remove vlan mapping */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
    {
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        vlan_mapping_port =  p_gem_port_db->logic_port;
    }
    else
    {
        vlan_mapping_port = p_uni_port->iloop_port;
    }

    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.old_svid = p_vlan_port_db->match_svlan;
    vlan_mapping.old_cvid = p_vlan_port_db->match_cvlan;
    vlan_mapping.svlan_end = p_vlan_port_db->match_svlan_end;
    vlan_mapping.vrange_grpid = p_uni_port->vlan_range_grp;

    if (p_vlan_port_db->igs_vlan_mapping_use_flex)
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    }

    CTC_ERROR_RETURN(ctc_vlan_remove_vlan_mapping(vlan_mapping_port , &vlan_mapping));

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    if (0 == p_vlan_port_db->match_svlan_end)
    {
        port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    }
    CTC_ERROR_RETURN(ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL));

    /* Remove vlan range */
    if (p_vlan_port_db->match_svlan_end)
    {
        ctc_vlan_range_info_t range_info;
        ctc_vlan_range_t vlan_range;

        sal_memset(&range_info, 0, sizeof(range_info));
        sal_memset(&vlan_range, 0, sizeof(vlan_range));

        range_info.direction = CTC_INGRESS;
        range_info.vrange_grpid = p_uni_port->vlan_range_grp;
        vlan_range.vlan_start = p_vlan_port_db->match_svlan;
        vlan_range.vlan_end   = p_vlan_port_db->match_svlan_end;

        CTC_ERROR_RETURN(ctc_vlan_remove_vlan_range(&range_info, &vlan_range));
    }

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db,  p_uni_port));
    }

    if (0 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        CTC_ERROR_RETURN(ctc_gbx_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db,  p_uni_port));
    }

    return ret;
}

int32
_ctc_gbx_app_vlan_port_flex_create_pon(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port, ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    uint8 gchip = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint16 lport = 0;
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;
    ctc_port_scl_property_t scl_prop;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fid_db_t* p_port_fid = NULL;
    ctc_internal_port_assign_para_t alloc_port;
    ctc_vlan_edit_nh_param_t vlan_xlate_nh;
    ctc_loopback_nexthop_param_t flex_port_iloop_nh;

    ctc_get_gchip_id(lchip, &gchip);

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    port_fid.is_flex = 1;

    CTC_ERROR_RETURN(ctc_spool_add(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL, &p_port_fid));
    p_vlan_port_db->fid_mapping = p_port_fid->fid;

    /* Alloc port iloop port for uplink to pon flex qinq */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_GOTO(ctc_alloc_internal_port(&alloc_port), ret, roll_back_0);
    p_vlan_port_db->pon_uplink_iloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    CTC_ERROR_GOTO(ctc_alloc_internal_port(&alloc_port), ret, roll_back_1);
    p_vlan_port_db->pon_downlink_iloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->pon_downlink_iloop_nhid), ret, roll_back_2);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->pon_downlink_e2iloop_nhid), ret, roll_back_3);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->pon_uplink_iloop_nhid), ret, roll_back_4);
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->pon_uplink_e2iloop_nhid), ret, roll_back_5);

    sal_memset(&flex_port_iloop_nh, 0, sizeof(flex_port_iloop_nh));
    flex_port_iloop_nh.lpbk_lport = p_vlan_port_db->pon_uplink_iloop_port;
    CTC_ERROR_GOTO(ctc_nh_add_iloop(p_vlan_port_db->pon_uplink_iloop_nhid, &flex_port_iloop_nh), ret, roll_back_6);

    /* Create flex port xlate nh, downlink to uplink */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_g_app_vlan_port_master->downlink_eloop_port[p_uni_port->vdev_id];
    vlan_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    vlan_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_vlan_port_db->pon_uplink_iloop_nhid;
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->pon_uplink_e2iloop_nhid, &vlan_xlate_nh), ret, roll_back_7);

    sal_memset(&flex_port_iloop_nh, 0, sizeof(flex_port_iloop_nh));
    flex_port_iloop_nh.lpbk_lport = p_vlan_port_db->pon_downlink_iloop_port;
    CTC_ERROR_GOTO(ctc_nh_add_iloop(p_vlan_port_db->pon_downlink_iloop_nhid, &flex_port_iloop_nh), ret, roll_back_8);

    /* Create flex port xlate nh, downlink to uplink */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_g_app_vlan_port_master->downlink_eloop_port[p_uni_port->vdev_id];
    vlan_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    vlan_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_vlan_port_db->pon_downlink_iloop_nhid;
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->pon_downlink_e2iloop_nhid, &vlan_xlate_nh), ret, roll_back_9);

    ctc_port_set_property(p_vlan_port_db->pon_uplink_iloop_port, CTC_PORT_PROP_LEARNING_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_uplink_iloop_port, CTC_PORT_PROP_PORT_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_uplink_iloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1);
    ctc_port_set_property(p_vlan_port_db->pon_uplink_iloop_port, CTC_PORT_PROP_PORT_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_uplink_iloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE);

    ctc_port_set_property(p_vlan_port_db->pon_downlink_iloop_port, CTC_PORT_PROP_LEARNING_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_downlink_iloop_port, CTC_PORT_PROP_PORT_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_downlink_iloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1);
    ctc_port_set_property(p_vlan_port_db->pon_downlink_iloop_port, CTC_PORT_PROP_PORT_EN, 1);
    ctc_port_set_property(p_vlan_port_db->pon_downlink_iloop_port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE);
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_vlan_port_db->pon_uplink_iloop_port, &scl_prop);
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_vlan_port_db->pon_downlink_iloop_port, &scl_prop);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(p_port_fid->fid, 1);
    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
    scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT;
    scl_entry.key.u.hash_port_2vlan_key.gport = p_vlan_port_db->pon_uplink_iloop_port;
    scl_entry.key.u.hash_port_2vlan_key.svlan = p_vlan_port_db->pkt_svlan;
    scl_entry.key.u.hash_port_2vlan_key.cvlan = p_vlan_port_db->pkt_cvlan;
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    scl_entry.action.u.igs_action.fid = p_port_fid->fid;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
    scl_entry.action.u.igs_action.user_vlanptr = p_port_fid->fid;
    scl_entry.action.u.igs_action.logic_port.logic_port_type = 0;
    CTC_ERROR_GOTO(ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry), ret, roll_back_10);
    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_11);
    CTC_APP_DBG_INFO("uplink pkt_svlan %d pkt_cvlan %d entry_id %d\n", p_vlan_port_db->pkt_svlan, p_vlan_port_db->pkt_cvlan, scl_entry.entry_id);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = ENCODE_SCL_DOWNLINK_ENTRY_ID(p_port_fid->fid, 1);
    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
    scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_PORT;
    scl_entry.key.u.hash_port_2vlan_key.gport = p_vlan_port_db->pon_downlink_iloop_port;
    scl_entry.key.u.hash_port_2vlan_key.svlan = p_vlan_port->egress_vlan_action_set.new_svid;
    scl_entry.key.u.hash_port_2vlan_key.cvlan = p_vlan_port->egress_vlan_action_set.new_cvid;
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    scl_entry.action.u.igs_action.fid = p_port_fid->fid;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
    scl_entry.action.u.igs_action.user_vlanptr = p_port_fid->fid;
    CTC_ERROR_GOTO(ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry), ret, roll_back_12);
    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_13);
    CTC_APP_DBG_INFO("uplink pkt_svlan %d pkt_cvlan %d entry_id %d\n", p_vlan_port_db->pkt_svlan, p_vlan_port_db->pkt_cvlan, scl_entry.entry_id);

    p_vlan_port->flex_nhid = p_vlan_port_db->pon_uplink_e2iloop_nhid;
    p_vlan_port->pon_flex_nhid = p_vlan_port_db->pon_downlink_e2iloop_nhid;

    return CTC_E_NONE;


roll_back_13:
    ctc_scl_remove_entry(ENCODE_SCL_DOWNLINK_ENTRY_ID(p_port_fid->fid, 1));

roll_back_12:
    ctc_scl_uninstall_entry(ENCODE_SCL_UPLINK_ENTRY_ID(p_port_fid->fid, 1));

roll_back_11:
    ctc_scl_remove_entry(ENCODE_SCL_UPLINK_ENTRY_ID(p_port_fid->fid, 1));

roll_back_10:
    ctc_nh_remove_xlate(p_vlan_port_db->pon_downlink_e2iloop_nhid);

roll_back_9:
    ctc_nh_remove_iloop(p_vlan_port_db->pon_downlink_iloop_nhid);

roll_back_8:
    ctc_nh_remove_xlate(p_vlan_port_db->pon_uplink_e2iloop_nhid);

roll_back_7:
    ctc_nh_remove_iloop(p_vlan_port_db->pon_uplink_iloop_nhid);

roll_back_6:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_uplink_e2iloop_nhid);

roll_back_5:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_uplink_iloop_nhid);

roll_back_4:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_downlink_e2iloop_nhid);

roll_back_3:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_downlink_iloop_nhid);

roll_back_2:
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->pon_downlink_iloop_port);
    ctc_free_internal_port(&alloc_port);

roll_back_1:
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->pon_uplink_iloop_port);
    ctc_free_internal_port(&alloc_port);

roll_back_0:
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);
    return ret;
}

int32
_ctc_gbx_app_vlan_port_flex_create_onu(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port, ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    int32 ret = 0;
    uint32 logic_vlan = 0;
    uint32 flex_1st_iloop_gport = 0;
    uint32 flex_e2iloop_gport = 0;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_vlan_edit_nh_param_t vlan_add_xlate_nh;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_vlan_edit_nh_param_t vlan_xlate_nh;
    ctc_opf_t opf;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint16 lport = 0;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fid_db_t* p_port_fid=NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    flex_1st_iloop_gport = p_g_app_vlan_port_master->flex_1st_iloop_port[p_uni_port->vdev_id];
    flex_e2iloop_gport   = p_g_app_vlan_port_master->downlink_eloop_port[p_uni_port->vdev_id];

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_offset(lchip, &p_vlan_port_db->ad_index));
    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    CTC_ERROR_GOTO(ctc_spool_add(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL, &p_port_fid), ret, roll_back_0);
    p_vlan_port_db->fid_mapping = p_port_fid->fid;
    /*1*/
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->flex_add_vlan_xlate_nhid), ret, roll_back_1);
    /*2*/
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->xlate_nhid), ret, roll_back_2);
    /*3*/
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->flex_xlate_nhid), ret, roll_back_3);

    /***********************************************/
    /* 4. Alloc logic port vlan */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_vlan), ret, roll_back_4);
    CTC_APP_DBG_INFO("OPF logic vlan: %d\n", logic_vlan);
    p_vlan_port_db->flex_logic_port_vlan = logic_vlan;

    /***********************************************/
    /* 5. Create xlate nh */
    sal_memset(&vlan_add_xlate_nh, 0, sizeof(vlan_add_xlate_nh));
    vlan_add_xlate_nh.gport_or_aps_bridge_id = flex_e2iloop_gport;
    vlan_add_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    vlan_add_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    vlan_add_xlate_nh.vlan_edit_info.output_svid = p_vlan_port_db->flex_logic_port_vlan;
    vlan_add_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    vlan_add_xlate_nh.vlan_edit_info.loop_nhid = p_g_app_vlan_port_master->flex_1st_iloop_nhid[p_uni_port->vdev_id];
    CTC_APP_DBG_INFO("nh xlate nhid: %d, add logic port vlan:%d\n", p_vlan_port_db->flex_add_vlan_xlate_nhid, p_vlan_port_db->flex_logic_port_vlan);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->flex_add_vlan_xlate_nhid, &vlan_add_xlate_nh), ret, roll_back_5);

    /***********************************************/
    /* 6. Add vlan mapping to e2iloop nexthop */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    vlan_mapping.old_svid = p_vlan_port_db->flex_logic_port_vlan;
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_NHID);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SVID);
    vlan_mapping.stag_op = CTC_VLAN_TAG_OP_DEL;
    vlan_mapping.logic_src_port = p_vlan_port_db->logic_port;
    vlan_mapping.u3.nh_id = p_g_app_vlan_port_master->flex_2nd_e2iloop_nhid[p_uni_port->vdev_id];
    ret = ctc_vlan_add_vlan_mapping(flex_1st_iloop_gport, &vlan_mapping);
    if (ret ==  CTC_E_HASH_CONFLICT)
    {
        CTC_APP_DBG_INFO("Hash Confict!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
        CTC_ERROR_GOTO(ctc_vlan_add_vlan_mapping(flex_1st_iloop_gport, &vlan_mapping), ret, roll_back_6);
        p_vlan_port_db->igs_vlan_mapping_use_flex = 1;
    }
    else if(ret != CTC_E_NONE)
    {
        goto roll_back_5;
    }

    /***********************************************/
    /* 7. Create e2e nexthop xlate nh */

    sal_memset(&vlan_add_xlate_nh, 0, sizeof(vlan_add_xlate_nh));
    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_igs_nh_xlate_mapping(lchip, p_vlan_port, &vlan_add_xlate_nh.vlan_edit_info), ret, roll_back_7);
    vlan_add_xlate_nh.gport_or_aps_bridge_id = flex_1st_iloop_gport;
    vlan_add_xlate_nh.vlan_edit_info.loop_nhid = p_vlan_port_db->flex_add_vlan_xlate_nhid;

    CTC_APP_DBG_INFO("nh xlate nhid: %d, add e2eloop  flex_add_vlan_xlate_nhid:%d\n",
        p_vlan_port_db->flex_xlate_nhid, p_vlan_port_db->flex_add_vlan_xlate_nhid);

    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->flex_xlate_nhid, &vlan_add_xlate_nh), ret, roll_back_7);

    /***********************************************/

    /***********************************************/
    /* 8. Create xlate nh */

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.gport_or_aps_bridge_id = p_uni_port->onu_e2eloop_port;
    vlan_xlate_nh.vlan_edit_info.loop_nhid = p_gem_port_db->xlate_nhid;
    vlan_xlate_nh.dsnh_offset = p_vlan_port_db->fid_mapping;
    vlan_xlate_nh.logic_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];
    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG);/*LogicReplicate*/
#if 0
    if (p_vlan_port->egress_vlan_action_set.svid == CTC_APP_VLAN_ACTION_DEL &&
        p_vlan_port->egress_vlan_action_set.cvid == CTC_APP_VLAN_ACTION_DEL)
    {   /*use egress scl do del*/
        p_vlan_port_db->egs_vlan_mapping_en = 1;
    }
    else
    {
        CTC_ERROR_GOTO(_ctc_app_vlan_port_nh_xlate_mapping(lchip, p_vlan_port, &vlan_xlate_nh.vlan_edit_info), ret, roll_back_5);
    }
#endif

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_nh_xlate_mapping(lchip, p_vlan_port, &vlan_xlate_nh.vlan_edit_info), ret, roll_back_8);

    CTC_APP_DBG_INFO("nh xlate nhid: %d\n", p_vlan_port_db->xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->xlate_nhid, &vlan_xlate_nh), ret, roll_back_9);
    p_vlan_port_db->logic_dest_port = p_vlan_port_db->logic_port;

    /* Bind logic port and nhid which for hw learning */
    CTC_ERROR_GOTO(ctc_l2_set_nhid_by_logic_port(p_vlan_port_db->logic_port, p_vlan_port_db->xlate_nhid), ret, roll_back_9);
    /* Return user xlate nh  for acl*/
    p_vlan_port->flex_nhid = p_vlan_port_db->flex_xlate_nhid;

    /***********************************************/

    return CTC_E_NONE;

roll_back_9:
    ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid);

roll_back_8:
    ctc_nh_remove_xlate(p_vlan_port_db->flex_xlate_nhid);

roll_back_7:
    ctc_vlan_remove_vlan_mapping(flex_1st_iloop_gport, &vlan_mapping);

roll_back_6:
    ctc_nh_remove_xlate(p_vlan_port_db->flex_add_vlan_xlate_nhid);

roll_back_5:
    ctc_opf_free_offset(&opf, 1, logic_vlan);

roll_back_4:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->flex_xlate_nhid);

roll_back_3:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid);

roll_back_2:
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->flex_add_vlan_xlate_nhid);

roll_back_1:
    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);
roll_back_0:
    _ctc_gbx_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index);
    return ret;
}

int32
ctc_gbx_app_vlan_port_flex_create(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port, ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;

    /* Get fid */
    if (0 == p_vlan_port_db->pkt_svlan)
    {
        ctc_port_get_default_vlan(p_vlan_port_db->port, &p_vlan_port_db->fid);
    }
    else
    {
        p_vlan_port_db->fid = p_vlan_port_db->pkt_svlan;
    }

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_vlan_edit(lchip, p_vlan_port, &pkt_svlan, &pkt_cvlan));

    p_vlan_port_db->pkt_svlan = pkt_svlan;
    p_vlan_port_db->pkt_cvlan = pkt_cvlan;

    if (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port->criteria)
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_flex_create_pon(lchip, p_vlan_port, p_vlan_port_db));
    }
    else
    {
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_flex_create_onu(lchip, p_vlan_port, p_vlan_port_db));

    }

    /***********************************************/

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_flex_destory_pon(uint8 lchip, ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    uint8 gchip = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    uint16 lport = 0;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_internal_port_assign_para_t alloc_port;
    ctc_port_scl_property_t scl_prop;

    ctc_get_gchip_id(lchip, &gchip);

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->port);
    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    /***********************************************/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_vlan_port_db->pon_uplink_iloop_port, &scl_prop);
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_vlan_port_db->pon_downlink_iloop_port, &scl_prop);

    /* Remove scl entry mapping fid */
    ctc_scl_uninstall_entry(ENCODE_SCL_DOWNLINK_ENTRY_ID(p_vlan_port_db->fid_mapping, 1));
    ctc_scl_remove_entry(ENCODE_SCL_DOWNLINK_ENTRY_ID(p_vlan_port_db->fid_mapping, 1));
    ctc_scl_uninstall_entry(ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->fid_mapping, 1));
    ctc_scl_remove_entry(ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->fid_mapping, 1));

    /***********************************************/
    /* Remove e2iloop nexthop */
    ctc_nh_remove_xlate(p_vlan_port_db->pon_downlink_e2iloop_nhid);
    ctc_nh_remove_iloop(p_vlan_port_db->pon_downlink_iloop_nhid);
    ctc_nh_remove_xlate(p_vlan_port_db->pon_uplink_e2iloop_nhid);
    ctc_nh_remove_iloop(p_vlan_port_db->pon_uplink_iloop_nhid);

    /***********************************************/
    /* Free nhid */
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_uplink_e2iloop_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_uplink_iloop_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_downlink_e2iloop_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->pon_downlink_iloop_nhid);

    /***********************************************/
    /* Free inter port */
    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->pon_downlink_iloop_port);
    ctc_free_internal_port(&alloc_port);

    sal_memset(&alloc_port, 0, sizeof(alloc_port));
    alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    alloc_port.gchip = gchip;
    alloc_port.inter_port = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->pon_uplink_iloop_port);
    ctc_free_internal_port(&alloc_port);

    /***********************************************/
    /* Remove fid mapping */
    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_uni_port->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    port_fid.is_flex = 1;
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);

    return CTC_E_NONE;

}

int32
_ctc_gbx_app_vlan_port_flex_destory_onu(uint8 lchip, ctc_app_vlan_port_db_t *p_vlan_port_db)
{
    ctc_opf_t opf;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_fid_db_t port_fid;

    ctc_l2_set_nhid_by_logic_port(p_vlan_port_db->logic_port, 0);

     /***********************************************/
    /* Remove xlate nh */
    ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid);
    ctc_nh_remove_xlate(p_vlan_port_db->flex_xlate_nhid);

    /***********************************************/
    /* Remove vlan mapping to e2iloop nexthop */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    vlan_mapping.old_svid = p_vlan_port_db->flex_logic_port_vlan;
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_NHID);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_SVID);

    if (p_vlan_port_db->igs_vlan_mapping_use_flex)
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    }
    ctc_vlan_remove_vlan_mapping(p_g_app_vlan_port_master->flex_1st_iloop_port[p_vlan_port_db->vdev_id], &vlan_mapping);

    /***********************************************/
    /* Remove xlate nh */
    ctc_nh_remove_xlate(p_vlan_port_db->flex_add_vlan_xlate_nhid);

    /***********************************************/
    /* Free logic port vlan */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN;
    ctc_opf_free_offset(&opf, 1, p_vlan_port_db->flex_logic_port_vlan);

    /***********************************************/
    /* Free nhid */
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->flex_xlate_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid);
    _ctc_gbx_app_vlan_port_free_nhid(lchip, p_vlan_port_db->flex_add_vlan_xlate_nhid);

    /* Remove fid mapping */
    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_vlan_port_db->vdev_id;
    port_fid.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);

    /***********************************************/
    /* Free ad index */
    _ctc_gbx_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_flex_destroy(uint8 lchip, ctc_app_vlan_port_db_t *p_vlan_port_db)
{

    if (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port_db->criteria)
    {
        _ctc_gbx_app_vlan_port_flex_destory_pon(lchip, p_vlan_port_db);
    }
    else
    {
        _ctc_gbx_app_vlan_port_flex_destory_onu(lchip, p_vlan_port_db);
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port)
{
    int32 ret = 0;
    uint8 onu_sevice_en = 0;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_opf_t opf;
    uint32 logic_port = 0;
    uint32 vlan_port_id = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_l2_flush_fdb_t flush;
    uint8 chip_type = 0;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_svlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_cvlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_svlan_end);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_cvlan_end);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_tunnel_value);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_vlan_port->port);
    if (CTC_IS_LINKAGG_PORT(p_vlan_port->port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM_ST(criteria);
    CTC_APP_DBG_PARAM_ST(port);
    CTC_APP_DBG_PARAM_ST(match_tunnel_value);
    CTC_APP_DBG_PARAM_ST(match_svlan);
    CTC_APP_DBG_PARAM_ST(match_cvlan);
    CTC_APP_DBG_PARAM_ST(match_svlan_end);
    CTC_APP_DBG_PARAM_ST(match_cvlan_end);

    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.svid);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.new_svid);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.cvid);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.new_cvid);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.scos);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.new_scos);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.ccos);
    CTC_APP_DBG_PARAM_ST(ingress_vlan_action_set.new_ccos);

    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.svid);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.new_svid);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.cvid);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.new_cvid);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.scos);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.new_scos);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.ccos);
    CTC_APP_DBG_PARAM_ST(egress_vlan_action_set.new_ccos);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_vlan_port->port, &p_uni_port));

    if (0 == p_uni_port->iloop_port)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (0 == p_g_app_vlan_port_master->nni_port_cnt[p_uni_port->vdev_id])
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port->criteria)
        || (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX))
    {
        onu_sevice_en = 1; /*gem port onu base vlan edit*/
    }
    else if((CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN == p_vlan_port->criteria)
        || (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port->criteria))
    {
         onu_sevice_en = 0; /*port base vlan edit*/
    }
    else if (CTC_APP_VLAN_PORT_MATCH_MCAST == p_vlan_port->criteria)
    {
        onu_sevice_en = 1; /*known mcast gem port base vlan edit*/
        if ((p_vlan_port->l2mc_addr.fid >= p_g_app_vlan_port_master->max_fid_num) || (p_vlan_port->l2mc_addr.fid < p_g_app_vlan_port_master->vdev_num))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        if ((p_vlan_port->l2mc_addr.l2mc_grp_id < p_g_app_vlan_port_master->max_fid_num)||
            (p_vlan_port->l2mc_addr.l2mc_grp_id > (p_g_app_vlan_port_master->mcast_max_group_num -1 -p_g_app_vlan_port_master->vdev_num)))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;
    }

    if (p_vlan_port->match_cvlan_end)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;
    }

    if (p_vlan_port->match_svlan_end &&
        p_vlan_port->ingress_vlan_action_set.svid != CTC_APP_VLAN_ACTION_ADD)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;
    }

    if (onu_sevice_en)
    {
        sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
        gem_port_db.port = p_vlan_port->port;
        gem_port_db.tunnel_value = p_vlan_port->match_tunnel_value;
        p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);

        if (NULL == p_gem_port_db)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }
    }

    sal_memset(&vlan_port_db_tmp, 0, sizeof(ctc_app_vlan_port_db_t));
    vlan_port_db_tmp.criteria = p_vlan_port->criteria;
    vlan_port_db_tmp.port = p_vlan_port->port;
    vlan_port_db_tmp.tunnel_value = p_vlan_port->match_tunnel_value;
    vlan_port_db_tmp.match_svlan = p_vlan_port->match_svlan;
    vlan_port_db_tmp.match_svlan_end = p_vlan_port->match_svlan_end;
    vlan_port_db_tmp.match_cvlan = p_vlan_port->match_cvlan;
    vlan_port_db_tmp.match_cvlan_end = p_vlan_port->match_cvlan_end;
    sal_memcpy(&vlan_port_db_tmp.flex_key, &p_vlan_port->flex_key, sizeof(vlan_port_db_tmp.flex_key));
    sal_memcpy(&vlan_port_db_tmp.l2mc_addr, &p_vlan_port->l2mc_addr, sizeof(vlan_port_db_tmp.l2mc_addr));

    p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);

    if (NULL != p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_ENTRY_EXIST;
    }

    /* OPF --> vlan port id*/
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &vlan_port_id), ret, roll_back_0);
    CTC_APP_DBG_INFO("OPF vlan port id: %d\n", vlan_port_id);

    /* Build node */
    MALLOC_POINTER(ctc_app_vlan_port_db_t, p_vlan_port_db);
    if (NULL == p_vlan_port_db)
    {
        goto roll_back_0;
    }

    sal_memset(p_vlan_port_db, 0, sizeof(ctc_app_vlan_port_db_t));
    p_vlan_port_db->vdev_id = p_uni_port->vdev_id;
    p_vlan_port_db->vlan_port_id = vlan_port_id;
    p_vlan_port_db->criteria = p_vlan_port->criteria;
    p_vlan_port_db->port = p_vlan_port->port;
    p_vlan_port_db->tunnel_value = p_vlan_port->match_tunnel_value;
    p_vlan_port_db->match_svlan = p_vlan_port->match_svlan;
    p_vlan_port_db->match_svlan_end = p_vlan_port->match_svlan_end;
    p_vlan_port_db->match_cvlan = p_vlan_port->match_cvlan;
    p_vlan_port_db->match_cvlan_end = p_vlan_port->match_cvlan_end;
    sal_memcpy(&p_vlan_port_db->flex_key, &p_vlan_port->flex_key, sizeof(ctc_acl_key_t));
    sal_memcpy(&p_vlan_port_db->l2mc_addr, &p_vlan_port->l2mc_addr, sizeof(ctc_l2_mcast_addr_t));
    sal_memcpy(&p_vlan_port_db->ingress_vlan_action_set, &p_vlan_port->ingress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));
    sal_memcpy(&p_vlan_port_db->egress_vlan_action_set, &p_vlan_port->egress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));

    p_vlan_port_db->p_gem_port_db = p_gem_port_db;
    chip_type = ctc_get_chip_type(lchip);
    switch (chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria)
                || ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port->criteria) && p_vlan_port->match_svlan_end))
            {
                /* OPF --> vlan port loigcPort B*/
                sal_memset(&opf, 0, sizeof(opf));
                opf.pool_type = CTC_OPF_VLAN_PORT;
                opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
                CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_1);
                p_vlan_port_db->logic_port = logic_port;
                p_vlan_port_db->logic_port_b_en = 1;
                CTC_APP_DBG_INFO("OPF vlan logic port: %d\n", logic_port);
            }
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
            if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria)
                || (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port->criteria))
            {
                /* OPF --> vlan port loigcPort B*/
                sal_memset(&opf, 0, sizeof(opf));
                opf.pool_type = CTC_OPF_VLAN_PORT;
                opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
                CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_1);
                p_vlan_port_db->logic_port = logic_port;
                p_vlan_port_db->logic_port_b_en = 1;
                CTC_APP_DBG_INFO("OPF vlan logic port: %d\n", logic_port);
            }
            break;
#endif
        default:
            return CTC_E_INVALID_PARAM;
    }
    /* VLAN edit */
    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_basic_create(lchip, p_vlan_port, p_vlan_port_db),ret, roll_back_2);
    }
    else if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria) ||
        (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port->criteria))
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_flex_create(lchip, p_vlan_port, p_vlan_port_db),ret, roll_back_2);
    }
    else if (CTC_APP_VLAN_PORT_MATCH_MCAST == p_vlan_port->criteria)
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_downlink_nexthop_add(lchip, p_vlan_port, p_vlan_port_db, p_uni_port), ret, roll_back_2);
        p_vlan_port->flex_nhid = p_vlan_port_db->xlate_nhid;
    }

    /*Add vlan*/
    if (CTC_APP_VLAN_PORT_MATCH_MCAST != p_vlan_port->criteria)
    {
        CTC_ERROR_GOTO(ctc_gbx_app_vlan_port_add_vlan(lchip, p_vlan_port, p_vlan_port_db), ret, roll_back_3);
    }

    ctc_hash_insert(p_g_app_vlan_port_master->vlan_port_hash, p_vlan_port_db);
    ctc_hash_insert(p_g_app_vlan_port_master->vlan_port_key_hash, p_vlan_port_db);

    if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria)
        || (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port->criteria))
    {
        if (!p_vlan_port_db->logic_port_b_en)
        {
            p_vlan_port_db->logic_port = p_gem_port_db->logic_port;
        }

        ctc_gbx_app_vlan_port_add_sync_db(lchip, p_vlan_port_db, p_vlan_port->criteria);
    }

    p_g_app_vlan_port_master->vlan_port_cnt[p_uni_port->vdev_id]++;


    if(onu_sevice_en)
    {
        p_gem_port_db->ref_cnt++;
    }

    p_vlan_port->vlan_port_id = vlan_port_id;
    p_vlan_port->logic_port = logic_port;

    if (p_vlan_port_db->fid_mapping)
    {
        ctc_app_vlan_port_fid_db_t port_fid_db;
        ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;
        sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));

        port_fid_db.fid = p_vlan_port_db->fid_mapping;
        _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
        p_port_fid_db = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
        if (p_port_fid_db && p_port_fid_db->limit_count)
        {
            ctc_l2_fdb_query_t fdb_query;
            uint32 count = 0;

            sal_memset(&fdb_query, 0, sizeof(ctc_l2_fdb_query_t));
            fdb_query.fid = p_vlan_port_db->fid_mapping;
            fdb_query.gport = p_vlan_port_db->logic_port;
            fdb_query.use_logic_port = 1;
            fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
            fdb_query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
            ctc_l2_get_fdb_count(&fdb_query);
            count = fdb_query.count;
            if (p_gem_port_db && count)
            {
                _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_db->limit_action, p_gem_port_db, FALSE);
                p_gem_port_db->limit_count = ((p_gem_port_db->limit_count > count)?(p_gem_port_db->limit_count-count):0);
            }
        }
        sal_memset(&flush, 0, sizeof(flush));
        flush.gport = p_vlan_port_db->logic_port;
        flush.fid = p_vlan_port_db->fid_mapping;
        flush.use_logic_port = 1;
        flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
        ctc_l2_flush_fdb(&flush);
        _ctc_gbx_app_vlan_port_fdb_flush(lchip, p_vlan_port_db->fid_mapping, p_vlan_port_db->logic_port, CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT_FID);
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
roll_back_3:
    /* VLAN edit */
    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        ctc_gbx_app_vlan_port_basic_destroy(lchip, p_vlan_port_db);
    }
    else  if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria) ||
        (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port->criteria))
    {
        ctc_gbx_app_vlan_port_flex_destroy(lchip, p_vlan_port_db);
    }
    else if (CTC_APP_VLAN_PORT_MATCH_MCAST == p_vlan_port->criteria)
    {
        ctc_gbx_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

roll_back_2:
    if (p_vlan_port_db->logic_port_b_en)
    {
        opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        ctc_opf_free_offset(&opf, 1, logic_port);
    }

roll_back_1:
    mem_free(p_vlan_port_db);
    p_vlan_port_db = NULL;


roll_back_0:
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    ctc_opf_free_offset(&opf, 1, vlan_port_id);
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port)
{
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;
    ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
    ctc_opf_t opf;
    uint32 logic_port = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_l2_flush_fdb_t flush;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    if (0 == p_vlan_port->vlan_port_id)
    {
        CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_vlan_port->port);
        if (CTC_IS_LINKAGG_PORT(p_vlan_port->port))
        {
            return CTC_E_INVALID_GLOBAL_PORT;
        }
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM_ST(vlan_port_id);
    CTC_APP_DBG_PARAM_ST(criteria);
    CTC_APP_DBG_PARAM_ST(port);
    CTC_APP_DBG_PARAM_ST(match_tunnel_value);
    CTC_APP_DBG_PARAM_ST(match_svlan);
    CTC_APP_DBG_PARAM_ST(match_cvlan);
    CTC_APP_DBG_PARAM_ST(match_svlan_end);
    CTC_APP_DBG_PARAM_ST(match_cvlan_end);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* DB */
    sal_memset(&vlan_port_db_tmp, 0, sizeof(vlan_port_db_tmp));

    if ( p_vlan_port->vlan_port_id)
    {
        vlan_port_db_tmp.vlan_port_id = p_vlan_port->vlan_port_id;
        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
    }
    else
    {
        vlan_port_db_tmp.criteria = p_vlan_port->criteria;
        vlan_port_db_tmp.port = p_vlan_port->port;
        vlan_port_db_tmp.tunnel_value = p_vlan_port->match_tunnel_value;
        vlan_port_db_tmp.match_svlan = p_vlan_port->match_svlan;
        vlan_port_db_tmp.match_svlan_end = p_vlan_port->match_svlan_end;
        vlan_port_db_tmp.match_cvlan = p_vlan_port->match_cvlan;
        vlan_port_db_tmp.match_cvlan_end = p_vlan_port->match_cvlan_end;
        sal_memcpy(&vlan_port_db_tmp.flex_key, &p_vlan_port->flex_key, sizeof(vlan_port_db_tmp.flex_key));
        sal_memcpy(&vlan_port_db_tmp.l2mc_addr, &p_vlan_port->l2mc_addr, sizeof(vlan_port_db_tmp.l2mc_addr));

        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    logic_port = p_vlan_port_db->logic_port;

    /*Add vlan*/
    ctc_gbx_app_vlan_port_remove_vlan(lchip, p_vlan_port, p_vlan_port_db);

    /* flush fdb */
    if (p_vlan_port_db->fid_mapping)
    {
        uint32 count = 0;

        count = p_vlan_port_db->count;
        if (p_gem_port_db && count)
        {
            _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_db->limit_action, p_gem_port_db, FALSE);
            p_gem_port_db->limit_count = ((p_gem_port_db->limit_count > count)?(p_gem_port_db->limit_count-count):0);
        }

        sal_memset(&flush, 0, sizeof(flush));
        flush.gport = p_vlan_port_db->logic_port;
        flush.fid = p_vlan_port_db->fid_mapping;
        flush.use_logic_port = 1;
        flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
        ctc_l2_flush_fdb(&flush);
        _ctc_gbx_app_vlan_port_fdb_flush(lchip, p_vlan_port_db->fid_mapping, p_vlan_port_db->logic_port, CTC_APP_VLAN_PORT_FDB_FLUSH_BY_LPORT_FID);
    }

    /* VLAN edit */
    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        ctc_gbx_app_vlan_port_basic_destroy(lchip, p_vlan_port_db);
    }
    else  if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port_db->criteria) ||
        (CTC_APP_VLAN_PORT_MATCH_PORT_FLEX == p_vlan_port_db->criteria))
    {
        ctc_gbx_app_vlan_port_flex_destroy(lchip, p_vlan_port_db);
    }
    else if (CTC_APP_VLAN_PORT_MATCH_MCAST == p_vlan_port_db->criteria)
    {
        ctc_gbx_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

    /* Free opf */
    sal_memset(&opf, 0, sizeof(opf));

    if (p_vlan_port_db->logic_port_b_en)
    {
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        ctc_opf_free_offset(&opf, 1, logic_port);
    }

    /* Free vlan port id*/
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    ctc_opf_free_offset(&opf, 1, p_vlan_port_db->vlan_port_id);

    ctc_hash_remove(p_g_app_vlan_port_master->vlan_port_hash, p_vlan_port_db);
    ctc_hash_remove(p_g_app_vlan_port_master->vlan_port_key_hash, p_vlan_port_db);

    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX ||
        p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN)
    {
        ctc_gbx_app_vlan_port_remove_sync_db(lchip, p_vlan_port_db, p_vlan_port_db->criteria);
    }

    p_g_app_vlan_port_master->vlan_port_cnt[p_uni_port->vdev_id]--;

    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX
        || (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_MCAST))
    {
        p_gem_port_db->ref_cnt--;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    /* free memory */
    mem_free(p_vlan_port_db);
    p_vlan_port_db = NULL;

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port)
{
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    if (0 == p_vlan_port->vlan_port_id)
    {
        CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_vlan_port->port);
        if (CTC_IS_LINKAGG_PORT(p_vlan_port->port))
        {
            return CTC_E_INVALID_GLOBAL_PORT;
        }
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM_ST(vlan_port_id);
    CTC_APP_DBG_PARAM_ST(criteria);
    CTC_APP_DBG_PARAM_ST(port);
    CTC_APP_DBG_PARAM_ST(match_tunnel_value);
    CTC_APP_DBG_PARAM_ST(match_svlan);
    CTC_APP_DBG_PARAM_ST(match_cvlan);
    CTC_APP_DBG_PARAM_ST(match_svlan_end);
    CTC_APP_DBG_PARAM_ST(match_cvlan_end);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* DB */
    sal_memset(&vlan_port_db_tmp, 0, sizeof(vlan_port_db_tmp));

    if (p_vlan_port->vlan_port_id)
    {
        vlan_port_db_tmp.vlan_port_id = p_vlan_port->vlan_port_id;
        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
    }
    else
    {
        vlan_port_db_tmp.criteria = p_vlan_port->criteria;
        vlan_port_db_tmp.port = p_vlan_port->port;
        vlan_port_db_tmp.tunnel_value = p_vlan_port->match_tunnel_value;
        vlan_port_db_tmp.match_svlan = p_vlan_port->match_svlan;
        vlan_port_db_tmp.match_svlan_end = p_vlan_port->match_svlan_end;
        vlan_port_db_tmp.match_cvlan = p_vlan_port->match_cvlan;
        vlan_port_db_tmp.match_cvlan_end = p_vlan_port->match_cvlan_end;
        sal_memcpy(&vlan_port_db_tmp.flex_key, &p_vlan_port->flex_key, sizeof(vlan_port_db_tmp.flex_key));
        sal_memcpy(&vlan_port_db_tmp.l2mc_addr, &p_vlan_port->l2mc_addr, sizeof(vlan_port_db_tmp.l2mc_addr));

        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    p_vlan_port->vlan_port_id = p_vlan_port_db->vlan_port_id;
    p_vlan_port->logic_port = p_vlan_port_db->logic_port;
    p_vlan_port->flex_nhid = p_vlan_port_db->flex_xlate_nhid;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t *p_vlan_port)
{
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_qos_policer_t policer;
    ctc_vlan_mapping_t vlan_mapping;
    uint32 vlan_mapping_port = 0;
    ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    if (0 == p_vlan_port->vlan_port_id)
    {
        CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_vlan_port->port);
        if (CTC_IS_LINKAGG_PORT(p_vlan_port->port))
        {
            return CTC_E_INVALID_GLOBAL_PORT;
        }
    }

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM_ST(vlan_port_id);
    CTC_APP_DBG_PARAM_ST(criteria);
    CTC_APP_DBG_PARAM_ST(port);
    CTC_APP_DBG_PARAM_ST(match_tunnel_value);
    CTC_APP_DBG_PARAM_ST(match_svlan);
    CTC_APP_DBG_PARAM_ST(match_cvlan);
    CTC_APP_DBG_PARAM_ST(match_svlan_end);
    CTC_APP_DBG_PARAM_ST(match_cvlan_end);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* DB */
    sal_memset(&vlan_port_db_tmp, 0, sizeof(vlan_port_db_tmp));
    sal_memset(&policer, 0, sizeof(policer));

    if (p_vlan_port->vlan_port_id)
    {
        vlan_port_db_tmp.vlan_port_id = p_vlan_port->vlan_port_id;
        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
    }
    else
    {
        vlan_port_db_tmp.criteria = p_vlan_port->criteria;
        vlan_port_db_tmp.port = p_vlan_port->port;
        vlan_port_db_tmp.tunnel_value = p_vlan_port->match_tunnel_value;
        vlan_port_db_tmp.match_svlan = p_vlan_port->match_svlan;
        vlan_port_db_tmp.match_svlan_end = p_vlan_port->match_svlan_end;
        vlan_port_db_tmp.match_cvlan = p_vlan_port->match_cvlan;
        vlan_port_db_tmp.match_cvlan_end = p_vlan_port->match_cvlan_end;
        sal_memcpy(&vlan_port_db_tmp.flex_key, &p_vlan_port->flex_key, sizeof(vlan_port_db_tmp.flex_key));
        sal_memcpy(&vlan_port_db_tmp.l2mc_addr, &p_vlan_port->l2mc_addr, sizeof(vlan_port_db_tmp.l2mc_addr));

        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    /*-----------------------------------------*/
    /*Ingress policetr*/
    if (p_vlan_port->update_type == CTC_APP_VLAN_PORT_UPDATE_IGS_POLICER)
    {
        if (p_vlan_port->ingress_policer_id == p_vlan_port_db->ingress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        if (p_vlan_port->ingress_policer_id)
        {
            policer.type = CTC_QOS_POLICER_TYPE_FLOW;
            policer.id.policer_id = p_vlan_port->ingress_policer_id;
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_qos_get_policer(&policer));
        }

        /*-----------------------------------------*/
        /*Update ingress policetr*/

        p_gem_port_db = p_vlan_port_db->p_gem_port_db;

        /* Add vlan mapping */
        sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

        if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
        {
            vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
            vlan_mapping_port =  p_gem_port_db->logic_port;
        }
        else
        {
            vlan_mapping_port = p_uni_port->iloop_port;
        }

        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
        vlan_mapping.old_svid = p_vlan_port_db->match_svlan;
        vlan_mapping.old_cvid = p_vlan_port_db->match_cvlan;


        /* Add vlan range */
        if (p_vlan_port_db->match_svlan_end)
        {
            vlan_mapping.svlan_end = p_vlan_port_db->match_svlan_end;
            vlan_mapping.vrange_grpid = p_uni_port->vlan_range_grp;
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_get_vlan_mapping(vlan_mapping_port , &vlan_mapping));

        vlan_mapping.policer_id = p_vlan_port->ingress_policer_id;

        if (1 == policer.hbwp_en)
        {
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN);
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_update_vlan_mapping(vlan_mapping_port , &vlan_mapping));

        p_vlan_port_db->ingress_policer_id  = p_vlan_port->ingress_policer_id;
    }

    /*-----------------------------------------*/
    /*Egress policetr*/
    if (p_vlan_port->update_type == CTC_APP_VLAN_PORT_UPDATE_EGS_POLICER)
    {
        uint16 logic_dest_port = p_g_app_vlan_port_master->default_logic_dest_port[p_vlan_port_db->vdev_id];


        if (p_vlan_port->egress_policer_id == p_vlan_port_db->egress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        if (p_vlan_port->egress_policer_id)
        {
            uint16 policer_ptr = 0;
            policer.type = CTC_QOS_POLICER_TYPE_FLOW;
            policer.id.policer_id = p_vlan_port->egress_policer_id;
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_qos_get_policer(&policer));

            if (0 == policer.hbwp_en)
            {
                CTC_APP_VLAN_PORT_UNLOCK(lchip);
                return CTC_E_INVALID_CONFIG;
            }
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_qos_policer_index_get, lchip, CTC_QOS_POLICER_TYPE_FLOW, p_vlan_port->egress_policer_id, &policer_ptr));

            logic_dest_port = policer_ptr/4;
        }

        /*-----------------------------------------*/
        /*Update Egress policetr*/
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_update_logic_port, lchip, p_vlan_port_db->xlate_nhid, logic_dest_port));

        p_vlan_port_db->logic_dest_port = logic_dest_port;
        p_vlan_port_db->egress_policer_id  = p_vlan_port->egress_policer_id;

    }

    if (CTC_APP_VLAN_PORT_UPDATE_IGS_STATS == p_vlan_port->update_type)
    {
        if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;
        }

        if ((CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN != p_vlan_port_db->criteria)
            && (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN != p_vlan_port_db->criteria))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;
        }

        if (p_vlan_port->ingress_stats_id == p_vlan_port_db->ingress_stats_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        /*-----------------------------------------*/
        /*Update ingress stats*/

        p_gem_port_db = p_vlan_port_db->p_gem_port_db;

        /* Add vlan mapping */
        sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

        if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
        {
            vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
            vlan_mapping_port =  p_gem_port_db->logic_port;
        }
        else
        {
            vlan_mapping_port = p_uni_port->iloop_port;
        }

        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
        vlan_mapping.old_svid = p_vlan_port_db->match_svlan;
        vlan_mapping.old_cvid = p_vlan_port_db->match_cvlan;


        /* Add vlan range */
        if (p_vlan_port_db->match_svlan_end)
        {
            vlan_mapping.svlan_end = p_vlan_port_db->match_svlan_end;
            vlan_mapping.vrange_grpid = p_uni_port->vlan_range_grp;
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_get_vlan_mapping(vlan_mapping_port , &vlan_mapping));

        if (p_vlan_port->ingress_stats_id)
        {
            vlan_mapping.stats_id = p_vlan_port->ingress_stats_id;
            vlan_mapping.action |= CTC_VLAN_MAPPING_FLAG_STATS_EN;
        }
        else
        {
            vlan_mapping.stats_id = 0;
            CTC_UNSET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_STATS_EN);
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_update_vlan_mapping(vlan_mapping_port , &vlan_mapping));

        p_vlan_port_db->ingress_stats_id  = p_vlan_port->ingress_stats_id;
    }
    else if (CTC_APP_VLAN_PORT_UPDATE_EGS_STATS == p_vlan_port->update_type)
    {
        if (!CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN))
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;
        }

        if (CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN == p_vlan_port_db->criteria)
        {
            uint32 entry_id = 0;
            ctc_scl_field_action_t action_field;

            entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);
            sal_memset(&action_field, 0, sizeof(ctc_scl_field_action_t));
            action_field.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
            action_field.data0 = p_vlan_port->egress_stats_id;
            if (p_vlan_port->egress_stats_id)
            {
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_add_action_field(entry_id, &action_field));
                entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 1);
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_add_action_field(entry_id, &action_field));
            }
            else
            {
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_remove_action_field(entry_id, &action_field));
                entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 1);
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_remove_action_field(entry_id, &action_field));
            }
        }
        else if (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port_db->criteria
            || (CTC_APP_VLAN_PORT_MATCH_MCAST == p_vlan_port_db->criteria))
        {
            ctc_vlan_edit_nh_param_t vlan_xlate_nh;

            p_gem_port_db = p_vlan_port_db->p_gem_port_db;
            sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
            vlan_xlate_nh.dsnh_offset = p_vlan_port_db->fid;
            vlan_xlate_nh.gport_or_aps_bridge_id = p_uni_port->onu_e2eloop_port;
#ifdef GREATBELT
            vlan_xlate_nh.vlan_edit_info.loop_nhid = p_gem_port_db->xlate_nhid;
            if (CTC_APP_VLAN_ACTION_DEL == p_vlan_port_db->egress_vlan_action_set.cvid)
            {
                vlan_xlate_nh.vlan_edit_info.loop_nhid = p_vlan_port_db->xlate_nhid_onu;
            }
            vlan_xlate_nh.logic_port = p_g_app_vlan_port_master->default_logic_dest_port[p_uni_port->vdev_id];
#else
            vlan_xlate_nh.vlan_edit_info.loop_nhid = p_g_app_vlan_port_master->downlink_iloop_nhid[p_uni_port->vdev_id];
            vlan_xlate_nh.logic_port = p_gem_port_db->custom_id;
#endif
            sal_memcpy(&p_vlan_port->egress_vlan_action_set, &p_vlan_port_db->egress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_gbx_app_vlan_port_nh_xlate_mapping(lchip, p_vlan_port, &vlan_xlate_nh.vlan_edit_info));
            if (0 != p_vlan_port->egress_stats_id)
            {
                vlan_xlate_nh.vlan_edit_info.stats_id = p_vlan_port->egress_stats_id;
                vlan_xlate_nh.vlan_edit_info.flag |= CTC_VLAN_NH_STATS_EN;
            }

            CTC_APP_DBG_INFO("update nh xlate nhid: %d\n", p_vlan_port_db->xlate_nhid);
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_update_xlate,
                    lchip, p_vlan_port_db->xlate_nhid, &vlan_xlate_nh.vlan_edit_info, &vlan_xlate_nh));
        }
        p_vlan_port_db->egress_stats_id  = p_vlan_port->egress_stats_id;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _____NNI_PORT_____ ""

int32
ctc_gbx_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t *p_nni)
{
    int32 ret = 0;
    uint32 logic_port = 0;
    ctc_opf_t opf = {0};
    uint32 nh_id = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_port_scl_property_t scl_prop;
    uint8 chip_type = 0;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;
    uint32* p_gports = NULL;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint16 mem_count = 0;
    uint16 mem_index = 0;
    uint32 temp_gport = 0;
    uint32 mcast_nh_id = 0;
    ctc_mcast_nh_param_group_t mcast_group;
    ctc_scl_default_action_t def_action;
    ctc_port_restriction_t isolation;
#ifdef DUET2
    ctc_port_isolation_t port_isolation;
#endif

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_nni->port);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check the nni exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL != p_nni_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }

    MALLOC_POINTER(ctc_app_vlan_port_nni_port_db_t, p_nni_port_db);
    if (NULL == p_nni_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NO_MEMORY;
    }

    ctc_linkagg_get_max_mem_num(&max_num);
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32) * max_num);
    if (NULL == p_gports)
    {
        ret = CTC_E_NO_MEMORY;
        goto roll_back_0;
    }

    sal_memset(p_gports, 0, sizeof(uint32)*max_num);
    if (CTC_IS_LINKAGG_PORT(p_nni->port))
    {
        linkagg_id = CTC_GPORT_LINKAGG_ID(p_nni->port);
        ctc_linkagg_get_member_ports(linkagg_id, p_gports, &mem_count);
    }
    else
    {
        mem_count = 1;
        *p_gports = p_nni->port;
    }

    if ((0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni->vdev_id]) || (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN)))
    {
        /* OPF */
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index= CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_0);
        CTC_APP_DBG_INFO("OPF nni logic port: %d\n", logic_port);

        if (0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni->vdev_id])
        {
            _ctc_gbx_app_vlan_port_alloc_nhid(lchip, &mcast_nh_id);
            sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
            mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_nni->vdev_id);
            CTC_ERROR_GOTO(ctc_nh_add_mcast(mcast_nh_id, &mcast_group), ret, roll_back_1);
        }
        else
        {
            mcast_nh_id = p_g_app_vlan_port_master->nni_mcast_nhid[p_nni->vdev_id];
        }
    }
    else
    {
        logic_port = p_g_app_vlan_port_master->nni_logic_port[p_nni->vdev_id];
        mcast_nh_id = p_g_app_vlan_port_master->nni_mcast_nhid[p_nni->vdev_id];
    }

    chip_type = ctc_get_chip_type();
    /* linkagg nni port property should set by user */
    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];

        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE), ret, roll_back_2);
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1), ret, roll_back_2);
        switch(chip_type)
        {
#ifdef GREATBELT
            case CTC_CHIP_GREATBELT:
                CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_logic_port_en, lchip, temp_gport, 1), ret, roll_back_2);
                break;
#endif
#ifdef DUET2
            case CTC_CHIP_DUET2:
                CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 1), ret, roll_back_2);
                break;
#endif
            default:
                goto roll_back_2;
        }


        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        scl_prop.class_id_en = 1;
        scl_prop.class_id = CTC_APP_VLAN_PORT_NNI_CLASS_ID;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(temp_gport, &scl_prop), ret, roll_back_2);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        scl_prop.class_id_en = 1;
        scl_prop.class_id = CTC_APP_VLAN_PORT_NNI_CLASS_ID;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(temp_gport, &scl_prop), ret, roll_back_2);

        /* Set port logic_port */
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, logic_port), ret, roll_back_2);

        /* Set nni port scl default action*/
        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
        def_action.action.u.igs_action.fid = p_g_app_vlan_port_master->default_bcast_fid[p_nni->vdev_id];
        CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_USER_VLANPTR);
        def_action.action.u.igs_action.user_vlanptr = p_g_app_vlan_port_master->default_bcast_fid[p_nni->vdev_id];
        CTC_ERROR_GOTO(ctc_scl_set_default_action(&def_action), ret, roll_back_2);

        CTC_ERROR_GOTO(ctc_port_set_default_vlan(temp_gport, p_g_app_vlan_port_master->default_deny_learning_fid[p_nni->vdev_id]), ret, roll_back_2);
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0), ret, roll_back_2);

        if (!p_nni->rx_en)
        {
            CTC_ERROR_GOTO(ctc_port_set_receive_en(temp_gport, FALSE), ret, roll_back_2);
        }

        /** PORT ISOLATION*/
        if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN))
        {
            sal_memset(&isolation, 0, sizeof(isolation));
            isolation.dir = CTC_EGRESS;
            isolation.isolated_id  = CTC_APP_VLAN_PORT_NNI_ISOLATION_ID;
            isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
            isolation.type = CTC_PORT_ISOLATION_ALL;
            CTC_ERROR_GOTO(ctc_port_set_restriction(temp_gport, &isolation), ret, roll_back_2);
            isolation.dir = CTC_INGRESS;
            CTC_ERROR_GOTO(ctc_port_set_restriction(temp_gport, &isolation), ret, roll_back_2);
#ifdef DUET2
            sal_memset(&port_isolation, 0, sizeof(ctc_port_isolation_t));
            port_isolation.gport = CTC_APP_VLAN_PORT_NNI_ISOLATION_ID;
            port_isolation.use_isolation_id = 1;
            port_isolation.pbm[0] = 1<<(CTC_APP_VLAN_PORT_NNI_ISOLATION_ID);
            port_isolation.isolation_pkt_type = CTC_PORT_ISOLATION_ALL;
            CTC_ERROR_GOTO(ctc_port_set_isolation(lchip, &port_isolation), ret, roll_back_2);
#endif
        }
    }

    p_nni->logic_port = logic_port;

    /* Bind logic port and nhid which for hw learning */
    CTC_ERROR_GOTO(ctc_nh_get_l2uc(p_nni->port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id), ret, roll_back_2);
    /*CTC_ERROR_GOTO(ctc_l2_set_nhid_by_logic_port(logic_port, nh_id), ret, roll_back_2);*/
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_nni->vdev_id);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    CTC_ERROR_GOTO(ctc_nh_update_mcast(mcast_nh_id, &mcast_group), ret, roll_back_3);

    /*Add nni member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = p_g_app_vlan_port_master->default_bcast_fid[p_nni->vdev_id];
    l2dflt.l2mc_grp_id = p_nni->vdev_id;
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = nh_id;
    CTC_ERROR_GOTO(ctc_l2_add_port_to_default_entry(&l2dflt), ret, roll_back_4);


    /***********************************************/

    p_g_app_vlan_port_master->nni_logic_port[p_nni->vdev_id] = logic_port;
    p_g_app_vlan_port_master->nni_mcast_nhid[p_nni->vdev_id] = mcast_nh_id;

    CTC_ERROR_GOTO(_ctc_gbx_app_vlan_port_alloc_offset(lchip, &p_nni_port_db->nni_ad_index), ret, roll_back_5);

    p_nni_port_db->port = p_nni->port;
    p_nni_port_db->vdev_id = p_nni->vdev_id;
    p_nni_port_db->nni_logic_port = logic_port;
    p_nni_port_db->nni_nh_id = nh_id;
    p_nni_port_db->rx_en = p_nni->rx_en;
    ctc_hash_insert(p_g_app_vlan_port_master->nni_port_hash, (void*)p_nni_port_db);

    p_g_app_vlan_port_master->nni_port_cnt[p_nni->vdev_id]++;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    mem_free(p_gports);

    return CTC_E_NONE;

roll_back_5:
    l2dflt.fid = p_g_app_vlan_port_master->default_bcast_fid[p_nni->vdev_id];
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = nh_id;
    ctc_l2_remove_port_from_default_entry(&l2dflt);

roll_back_4:
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_nni->vdev_id);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_update_mcast(mcast_nh_id, &mcast_group);

roll_back_3:
    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];

        ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);
        switch(chip_type)
        {
#ifdef GREATBELT
            case CTC_CHIP_GREATBELT:
                CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_logic_port_en, lchip, temp_gport, 0);
                break;
#endif
#ifdef DUET2
            case CTC_CHIP_DUET2:
                ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 0);
                break;
#endif
            default:
                break;
        }

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.class_id_en = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.class_id_en = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        /* Set port logic_port */
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, 0);

        /* Set nni port scl default action*/
        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        ctc_scl_set_default_action(&def_action);
        ctc_port_set_default_vlan(temp_gport, 1);

        if (!p_nni->rx_en)
        {
            ctc_port_set_receive_en(temp_gport, TRUE);
        }

        /** PORT ISOLATION*/
        if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN))
        {
            sal_memset(&isolation, 0, sizeof(isolation));
            isolation.dir = CTC_EGRESS;
            isolation.isolated_id  = 0;
            isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
            isolation.type = CTC_PORT_ISOLATION_ALL;
            ctc_port_set_restriction(temp_gport, &isolation);
            isolation.dir = CTC_INGRESS;
            ctc_port_set_restriction(temp_gport, &isolation);
        }
    }

roll_back_2:
    if (0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni->vdev_id])
    {
        ctc_nh_remove_mcast(mcast_nh_id);
        _ctc_gbx_app_vlan_port_free_nhid(lchip, mcast_nh_id);
    }
roll_back_1:
    if ((0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni->vdev_id]) || (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN)))
    {
        ctc_opf_free_offset(&opf, 1, logic_port);
    }
roll_back_0:
    if (p_gports)
    {
        mem_free(p_gports);
    }
    if (p_nni_port_db)
    {
        mem_free(p_nni_port_db);
    }

    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t *p_nni)
{
    ctc_opf_t opf;
    ctc_l2dflt_addr_t l2dflt;
    uint32 nh_id = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;
    uint32* p_gports = NULL;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint8 chip_type = 0;
    uint16 mem_count = 0;
    uint16 mem_index = 0;
    uint32 temp_gport = 0;
    int32 ret = CTC_E_NONE;
    ctc_scl_default_action_t def_action;
    ctc_mcast_nh_param_group_t mcast_group;
    ctc_port_restriction_t isolation;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    _ctc_gbx_app_vlan_port_free_offset(lchip, p_nni_port_db->nni_ad_index);

    CTC_APP_DBG_INFO("NNI logic port: %d\n", p_nni_port_db->nni_logic_port);

    ctc_nh_get_l2uc(p_nni->port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id);

    /*Add nni member*/
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = p_g_app_vlan_port_master->default_bcast_fid[p_nni_port_db->vdev_id];
    l2dflt.with_nh = 1;
    l2dflt.member.nh_id = nh_id;
    ctc_l2_remove_port_from_default_entry(&l2dflt);

    chip_type = ctc_get_chip_type();
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_nni->vdev_id);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_update_mcast(p_g_app_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id], &mcast_group);

    /* Unbind logic port and nhid */
    /*CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_set_nhid_by_logic_port(p_nni_port_db->nni_logic_port, 0));*/

    ctc_linkagg_get_max_mem_num(&max_num);
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32) * max_num);
    if (NULL == p_gports)
    {
        ret = CTC_E_NO_MEMORY;
        goto roll_back_0;
    }

    sal_memset(p_gports, 0, sizeof(uint32) * max_num);
    if (CTC_IS_LINKAGG_PORT(p_nni->port))
    {
        linkagg_id = CTC_GPORT_LINKAGG_ID(p_nni->port);
        ctc_linkagg_get_member_ports(linkagg_id, p_gports, &mem_count);
    }
    else
    {
        mem_count = 1;
        *p_gports = p_nni->port;
    }

    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];
        /* Clear port logic_port*/
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, 0);
        switch(chip_type)
        {
#ifdef GREATBELT
            case CTC_CHIP_GREATBELT:
                CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_port_set_logic_port_en, lchip, temp_gport, 0);
                break;
#endif
#ifdef DUET2
            case CTC_CHIP_DUET2:
                ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 0);
                break;
#endif
            default:
                break;
        }

        /* Clear port cfg*/
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.class_id_en = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.class_id_en = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        ctc_scl_set_default_action(&def_action);
        ctc_port_set_default_vlan(temp_gport, 1);

        if (!p_nni->rx_en)
        {
            ctc_port_set_receive_en(temp_gport, TRUE);
        }

        /** PORT ISOLATION*/
        if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN))
        {
            sal_memset(&isolation, 0, sizeof(isolation));
            isolation.dir = CTC_EGRESS;
            isolation.isolated_id  = 0;
            isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
            isolation.type = CTC_PORT_ISOLATION_ALL;
            ctc_port_set_restriction(temp_gport, &isolation);
            isolation.dir = CTC_INGRESS;
            ctc_port_set_restriction(temp_gport, &isolation);
        }
    }

    p_g_app_vlan_port_master->nni_port_cnt[p_nni_port_db->vdev_id]--;
    if ((0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni_port_db->vdev_id]) || (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN)))
    {
        /* Free opf offset */
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index= CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        ctc_opf_free_offset(&opf, 1, p_nni_port_db->nni_logic_port);
        p_g_app_vlan_port_master->nni_logic_port[p_nni_port_db->vdev_id] = 0;

        if (0 == p_g_app_vlan_port_master->nni_port_cnt[p_nni_port_db->vdev_id])
        {
            ctc_nh_remove_mcast(p_g_app_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id]);
            _ctc_gbx_app_vlan_port_free_nhid(lchip, p_g_app_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id]);
            p_g_app_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id] = 0;
        }
    }

    /*remove nni port db from hash*/
    ctc_hash_remove(p_g_app_vlan_port_master->nni_port_hash, (void*)p_nni_port_db);
    mem_free(p_nni_port_db);
roll_back_0:
    mem_free(p_gports);
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;
    uint32 value = 0;
    uint32* p_gports = NULL;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint16 mem_count = 0;
    uint16 mem_index = 0;
    uint32 temp_gport = 0;
    int32 ret = CTC_E_NONE;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_nni->port);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        ret =  CTC_E_NOT_EXIST;
        goto roll_back_0;
    }

    ctc_linkagg_get_max_mem_num(&max_num);
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32) * max_num);
    if (NULL == p_gports)
    {
        ret = CTC_E_NO_MEMORY;
        goto roll_back_0;
    }

    sal_memset(p_gports, 0, sizeof(uint32)*max_num);
    if (CTC_IS_LINKAGG_PORT(p_nni->port))
    {
        linkagg_id = CTC_GPORT_LINKAGG_ID(p_nni->port);
        ctc_linkagg_get_member_ports(linkagg_id, p_gports, &mem_count);
    }
    else
    {
        mem_count = 1;
        *p_gports = p_nni->port;
    }

    if(p_nni->rx_en != p_nni_port_db->rx_en)
    {
        value = p_nni->rx_en?1:0;
        for (mem_index=0; mem_index<mem_count; mem_index++)
        {
            temp_gport = p_gports[mem_index];
            ctc_port_set_receive_en(temp_gport, value);
        }
        p_nni_port_db->rx_en = p_nni->rx_en;
    }

    mem_free(p_gports);

roll_back_0:
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    uint32 logic_port = 0;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, p_nni->port);

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10d\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    logic_port = p_nni_port_db->nni_logic_port;

    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    if (0 == logic_port)
    {
        return CTC_E_NOT_EXIST;
    }

    p_nni->logic_port = logic_port;

    return CTC_E_NONE;
}

#define _______LEARN_AGING________

STATIC int32
_ctc_gbx_app_vlan_port_sync_fid_mac_limit(uint8 lchip, uint16 fid, uint8 enable, uint8* need_add)
{
    int32 ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));

    *need_add = TRUE;
    port_fid_db.fid = fid;
    ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid_db);
    if (!ret)
    {
        return CTC_E_NOT_EXIST;
    }

    p_port_fid_db = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid_db);
    if (NULL == p_port_fid_db)
    {
        return CTC_E_NOT_EXIST;
    }

    if (enable && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_port_fid_db->limit_count == p_port_fid_db->limit_num))
    {
        *need_add = FALSE;
    }
    else if ((!enable) && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_port_fid_db->limit_count == (p_port_fid_db->limit_num)))
    {
        CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, CTC_MACLIMIT_ACTION_NONE);
    }

    if (*need_add)
    {
        if (enable)
        {
            p_port_fid_db->limit_count++;
        }
        else
        {
            if (p_port_fid_db->limit_count)
            {
                p_port_fid_db->limit_count--;
            }
        }

        if (enable && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
            && (p_port_fid_db->limit_count == p_port_fid_db->limit_num))
        {
            CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, p_port_fid_db->limit_action);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(uint8 lchip, ctc_app_vlan_port_gem_port_db_t* p_gem_port_find, uint8 enable, uint8* need_add)
{
    if (p_gem_port_find)
    {
        *need_add = TRUE;
        if (enable && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num))
        {
            *need_add = FALSE;
        }
        else if ((!enable) && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num))
        {
            _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_find->limit_action, p_gem_port_find, FALSE);
            p_gem_port_find->mac_security = CTC_MACLIMIT_ACTION_NONE;
        }

        if (*need_add)
        {
            if (enable)
            {
                p_gem_port_find->limit_count++;
            }
            else
            {
                if (p_gem_port_find->limit_count)
                {
                    p_gem_port_find->limit_count--;
                }
            }

            if (enable && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num)
                && (p_gem_port_find->limit_action != CTC_MACLIMIT_ACTION_NONE))
            {
                _ctc_gbx_app_vlan_port_update_mac_limit_action(lchip, p_gem_port_find->limit_action, p_gem_port_find, TRUE);
                p_gem_port_find->mac_security = p_gem_port_find->limit_action;
            }
        }
    }

    return CTC_E_NONE;
}

#define _______INIT________

STATIC int32
_ctc_gbx_app_vlan_port_init_db(uint8 lchip)
{
    ctc_spool_t spool;
    CTC_PTR_VALID_CHECK(p_g_app_vlan_port_master);

    p_g_app_vlan_port_master->nni_port_hash = ctc_hash_create(1,
                                                               20,
                                                               (hash_key_fn) _ctc_gbx_app_vlan_port_hash_nni_port_make,
                                                               (hash_cmp_fn) _ctc_gbx_app_vlan_port_hash_nni_port_cmp);

    p_g_app_vlan_port_master->gem_port_hash = ctc_hash_create(10,
                                                              100,
                                                              (hash_key_fn) _ctc_gbx_app_vlan_port_hash_gem_port_make,
                                                              (hash_cmp_fn) _ctc_gbx_app_vlan_port_hash_gem_port_cmp);

    if (NULL == p_g_app_vlan_port_master->gem_port_hash)
    {
        return CTC_E_NO_MEMORY;
    }

    p_g_app_vlan_port_master->vlan_port_hash = ctc_hash_create(10,
                                                               100,
                                                               (hash_key_fn) _ctc_gbx_app_vlan_port_hash_make,
                                                               (hash_cmp_fn) _ctc_gbx_app_vlan_port_hash_cmp);

    p_g_app_vlan_port_master->vlan_port_key_hash = ctc_hash_create(10,
                                                               100,
                                                               (hash_key_fn) _ctc_gbx_app_vlan_port_key_hash_make,
                                                               (hash_cmp_fn) _ctc_gbx_app_vlan_port_key_hash_cmp);

    p_g_app_vlan_port_master->vlan_port_logic_vlan_hash = ctc_hash_create(100,
                                                               200,
                                                               (hash_key_fn) _ctc_gbx_app_vlan_port_logic_vlan_hash_make,
                                                               (hash_cmp_fn) _ctc_gbx_app_vlan_port_logic_vlan_hash_cmp);

    p_g_app_vlan_port_master->fdb_entry_hash = ctc_hash_create(CTC_VEC_BLOCK_NUM(8192, 64),
                                           64,
                                           (hash_key_fn) _ctc_gbx_app_vlan_port_fdb_hash_make,
                                           (hash_cmp_fn) _ctc_gbx_app_vlan_port_fdb_hash_compare);
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 4*1024;
    spool.max_count = 4*1024;
    spool.user_data_size = sizeof(ctc_app_vlan_port_fid_db_t);
    spool.spool_key = (hash_key_fn)_ctc_gbx_app_vlan_port_fid_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_ctc_gbx_app_vlan_port_fid_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_ctc_gbx_app_vlan_port_fid_build_index;
    spool.spool_free = (spool_free_fn)_ctc_gbx_app_vlan_port_fid_free_index;
    p_g_app_vlan_port_master->fid_spool = ctc_spool_create(&spool);

    if (NULL == p_g_app_vlan_port_master->vlan_port_hash)
    {
        return CTC_E_NO_MEMORY;
    }

    p_g_app_vlan_port_master->p_port_pon = (ctc_app_vlan_port_uni_db_t*)mem_malloc(MEM_SYSTEM_MODULE, 512*sizeof(ctc_app_vlan_port_uni_db_t));
    if (NULL == p_g_app_vlan_port_master->p_port_pon)
    {
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_g_app_vlan_port_master->p_port_pon, 0, 512*sizeof(ctc_app_vlan_port_uni_db_t));

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_init_opf(uint8 lchip, uint16 vdev_num)
{
    ctc_opf_t opf;

    ctc_opf_init(CTC_OPF_VLAN_PORT, CTC_APP_VLAN_PORT_OPF_MAX);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    ctc_opf_init_offset(&opf, 1, CTC_APP_VLAN_PORT_LOGIC_PORT_NUM-1);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    ctc_opf_init_offset(&opf, 1, 32*1024);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN;
    ctc_opf_init_offset(&opf, 1, 4095);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    ctc_opf_init_offset(&opf, 0, 8);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;
    ctc_opf_init_offset(&opf, vdev_num+1, p_g_app_vlan_port_master->max_fid_num);

#ifdef DUET2
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_GEM_PORT;
    ctc_opf_init_offset(&opf, 1, 8192+1);
#endif

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_init_port(uint8 lchip, uint8 vdev_num)
{
    uint8 gchip = 0;
    ctc_internal_port_assign_para_t alloc_port;
    ctc_loopback_nexthop_param_t flex_iloop_nh;
    ctc_port_scl_property_t scl_prop;
    uint32 downlink_eloop_gport = 0;
    uint32 flex_1st_iloop_gport = 0;
    uint32 flex_2nd_iloop_gport = 0;
    ctc_vlan_edit_nh_param_t flex_e2iloop_xlate_nh;
    ctc_l2dflt_addr_t l2dflt;
    uint16 vdev_index = 0;
    uint32 fid = 0;
    ctc_opf_t opf;
    ctc_acl_group_info_t group_info;
    ctc_vlan_uservlan_t user_vlan;

    ctc_get_gchip_id(lchip, &gchip);
    for (vdev_index=0; vdev_index<vdev_num; vdev_index++)
    {
        /* Alloc gem port eloop port for uplink to onu*/
        sal_memset(&alloc_port, 0, sizeof(alloc_port));
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));
        p_g_app_vlan_port_master->downlink_eloop_port[vdev_index] = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));

        downlink_eloop_gport = p_g_app_vlan_port_master->downlink_eloop_port[vdev_index];
        CTC_ERROR_RETURN(ctc_port_set_property(downlink_eloop_gport, CTC_PORT_PROP_PORT_EN, 1));
        CTC_ERROR_RETURN(ctc_port_set_property(downlink_eloop_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE));

        /* Alloc flexiable QinQ 1st iloop port */
        sal_memset(&alloc_port, 0, sizeof(alloc_port));
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));
        p_g_app_vlan_port_master->flex_1st_iloop_port[vdev_index] = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));

        flex_1st_iloop_gport = p_g_app_vlan_port_master->flex_1st_iloop_port[vdev_index];

        CTC_ERROR_RETURN(ctc_port_set_property(flex_1st_iloop_gport, CTC_PORT_PROP_PORT_EN, 1));
        CTC_ERROR_RETURN(ctc_port_set_property(flex_1st_iloop_gport, CTC_PORT_PROP_LEARNING_EN, 0));
        CTC_ERROR_RETURN(ctc_port_set_property(flex_1st_iloop_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0));
        CTC_ERROR_RETURN(ctc_port_set_property(flex_1st_iloop_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

        /* Enable scl lookup for GEM port */
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_RETURN(ctc_port_set_scl_property(p_g_app_vlan_port_master->flex_1st_iloop_port[vdev_index], &scl_prop));

        /* Create flexiable QinQ 1st iloop nh */
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_g_app_vlan_port_master->flex_1st_iloop_nhid[vdev_index]));
        sal_memset(&flex_iloop_nh, 0, sizeof(flex_iloop_nh));
        flex_iloop_nh.lpbk_lport = p_g_app_vlan_port_master->flex_1st_iloop_port[vdev_index];
        CTC_ERROR_RETURN(ctc_nh_add_iloop(p_g_app_vlan_port_master->flex_1st_iloop_nhid[vdev_index], &flex_iloop_nh));

        /* Alloc flexiable QinQ 2nd iloop port */
        sal_memset(&alloc_port, 0, sizeof(alloc_port));
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));
        p_g_app_vlan_port_master->flex_2nd_iloop_port[vdev_index] = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));

        flex_2nd_iloop_gport = p_g_app_vlan_port_master->flex_2nd_iloop_port[vdev_index];

        CTC_ERROR_RETURN(ctc_port_set_property(flex_2nd_iloop_gport, CTC_PORT_PROP_LEARNING_EN, 1));
        CTC_ERROR_RETURN(ctc_port_set_property(flex_2nd_iloop_gport, CTC_PORT_PROP_PORT_EN, 1));
        CTC_ERROR_RETURN(ctc_port_set_property(flex_2nd_iloop_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

        /* Create flexiable QinQ 2nd iloop nh */
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_g_app_vlan_port_master->flex_2nd_iloop_nhid[vdev_index]));
        sal_memset(&flex_iloop_nh, 0, sizeof(flex_iloop_nh));
        flex_iloop_nh.lpbk_lport = p_g_app_vlan_port_master->flex_2nd_iloop_port[vdev_index];
        CTC_ERROR_RETURN(ctc_nh_add_iloop(p_g_app_vlan_port_master->flex_2nd_iloop_nhid[vdev_index], &flex_iloop_nh));

        /* Create gem port xlate nh, downlink to uplink */
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_g_app_vlan_port_master->flex_2nd_e2iloop_nhid[vdev_index]));
        sal_memset(&flex_e2iloop_xlate_nh, 0, sizeof(flex_e2iloop_xlate_nh));
        flex_e2iloop_xlate_nh.gport_or_aps_bridge_id = p_g_app_vlan_port_master->downlink_eloop_port[vdev_index];
        flex_e2iloop_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
        flex_e2iloop_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
        flex_e2iloop_xlate_nh.vlan_edit_info.loop_nhid = p_g_app_vlan_port_master->flex_2nd_iloop_nhid[vdev_index];
        CTC_ERROR_RETURN(ctc_nh_add_xlate(p_g_app_vlan_port_master->flex_2nd_e2iloop_nhid[vdev_index], &flex_e2iloop_xlate_nh));
#ifdef DUET2
        CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_alloc_nhid(lchip, &p_g_app_vlan_port_master->downlink_iloop_nhid[vdev_index]));
        /* Alloc downlink iloop port to with svlan forwarding */
        sal_memset(&alloc_port, 0, sizeof(alloc_port));
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ELOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_alloc_internal_port(&alloc_port));
        sal_memset(&alloc_port, 0, sizeof(alloc_port));
        alloc_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
        alloc_port.gchip = gchip;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_port));
        p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index] = CTC_MAP_LPORT_TO_GPORT(gchip, alloc_port.inter_port);
        CTC_ERROR_RETURN(ctc_port_set_property(p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index], CTC_PORT_PROP_LEARNING_EN, 0));
        CTC_ERROR_RETURN(ctc_port_set_property(p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index], CTC_PORT_PROP_PORT_EN, 1));
        CTC_ERROR_RETURN(ctc_port_set_property(p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index], CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

        /* Create downlink port iloop nh */
        sal_memset(&flex_iloop_nh, 0, sizeof(flex_iloop_nh));
        flex_iloop_nh.lpbk_lport = p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index];
        flex_iloop_nh.customerid_valid = 1;
        CTC_ERROR_RETURN(ctc_nh_add_iloop(p_g_app_vlan_port_master->downlink_iloop_nhid[vdev_index], &flex_iloop_nh));

        /* Enable scl lookup for downlink iloop port */
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        scl_prop.use_logic_port_en = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_RETURN(ctc_port_set_scl_property(p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index], &scl_prop));
#endif
        fid = CTC_APP_VLAN_PORT_RSV_MCAST_GROUP_ID(vdev_index);
        p_g_app_vlan_port_master->default_bcast_fid[vdev_index] = fid;
        sal_memset(&user_vlan, 0, sizeof(user_vlan));
        user_vlan.vlan_id = fid;
        user_vlan.user_vlanptr = fid;
        user_vlan.fid = fid;
        CTC_SET_FLAG(user_vlan.flag, CTC_MAC_LEARN_USE_LOGIC_PORT);
        CTC_ERROR_RETURN(ctc_vlan_create_uservlan(&user_vlan));
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = fid;
        l2dflt.l2mc_grp_id = fid;
        CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
        CTC_ERROR_RETURN(ctc_l2_add_default_entry(&l2dflt));

        /* Alloc default deny learning vdev fid for match default scl*/
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;
        CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, &fid));
        p_g_app_vlan_port_master->default_deny_learning_fid[vdev_index] = fid;

        sal_memset(&user_vlan, 0, sizeof(user_vlan));
        user_vlan.vlan_id = fid;
        user_vlan.user_vlanptr = fid;
        user_vlan.fid = fid;
        CTC_SET_FLAG(user_vlan.flag, CTC_MAC_LEARN_USE_LOGIC_PORT);
        CTC_ERROR_RETURN(ctc_vlan_create_uservlan(&user_vlan));
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = fid;
        l2dflt.l2mc_grp_id = fid;
        CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
        CTC_ERROR_RETURN(ctc_l2_add_default_entry(&l2dflt));
        CTC_ERROR_RETURN(ctc_vlan_set_learning_en(fid, FALSE));

        sal_memset(&group_info, 0, sizeof(ctc_acl_group_info_t));
        group_info.dir = CTC_INGRESS;
        group_info.lchip = lchip;
        group_info.type  = CTC_ACL_GROUP_TYPE_NONE;
        group_info.priority = 0;
        ctc_acl_create_group(CTC_APP_VLAN_PORT_ACL_GROUP_ID, &group_info);

        sal_memset(&group_info, 0, sizeof(ctc_acl_group_info_t));
        group_info.dir = CTC_EGRESS;
        group_info.lchip = lchip;
        group_info.type  = CTC_ACL_GROUP_TYPE_NONE;
        group_info.priority = 0;
        ctc_acl_create_group(CTC_APP_VLAN_PORT_ACL_GROUP_ID+1, &group_info);
    }

    return CTC_E_NONE;
}

#ifdef DUET2
STATIC int32
_ctc_gbx_app_vlan_port_init_mac_limit_info(uint8 lchip)
{
    uint32 entry_id = 0;
    ctc_acl_entry_t acl_entry;
    ctc_field_key_t key_field;
    ctc_acl_field_action_t action_field;
    ctc_acl_to_cpu_t acl_cp_to_cpu;
    ctc_acl_group_info_t group_info;

    sal_memset(&group_info, 0, sizeof(group_info));
    group_info.dir = CTC_INGRESS;
    group_info.priority = CTC_APP_VLAN_PORT_GEM_LMT_ACL_PRI;
    CTC_ERROR_RETURN(ctc_acl_create_group(CTC_APP_VLAN_PORT_GEM_LMT_ACL_GRP, &group_info));
    /*Init acl for mac limit discard*/
    entry_id = CTC_ACL_MAX_USER_ENTRY_ID - 2;
    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    acl_entry.key_type = CTC_ACL_KEY_FWD;
    acl_entry.entry_id = entry_id;
    acl_entry.mode = 1;
    CTC_ERROR_RETURN(ctc_acl_add_entry(CTC_APP_VLAN_PORT_GEM_LMT_ACL_GRP, &acl_entry));

    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACDA_LKUP;
    key_field.data = 1;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_METADATA;
    key_field.data = ENCODE_ACL_METADATA(1);
    key_field.mask = 0xFFFF;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACSA_LKUP;
    key_field.data = 1;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACSA_HIT;
    key_field.data = 0;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));

    sal_memset(&action_field, 0, sizeof(action_field));
    action_field.type = CTC_ACL_FIELD_ACTION_DISCARD;
    CTC_ERROR_RETURN(ctc_acl_add_action_field(entry_id, &action_field));
    sal_memset(&action_field, 0, sizeof(action_field));
    action_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
    CTC_ERROR_RETURN(ctc_acl_add_action_field(entry_id, &action_field));
    sal_memset(&action_field, 0, sizeof(action_field));
    sal_memset(&acl_cp_to_cpu, 0, sizeof(acl_cp_to_cpu));
    acl_cp_to_cpu.mode = CTC_ACL_TO_CPU_MODE_CANCEL_TO_CPU;
    action_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
    action_field.ext_data = &acl_cp_to_cpu;
    CTC_ERROR_RETURN(ctc_acl_install_entry(entry_id));

    /*Init acl for mac limit redirect to cpu*/
    entry_id = CTC_ACL_MAX_USER_ENTRY_ID-1;
    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    acl_entry.key_type = CTC_ACL_KEY_FWD;
    acl_entry.entry_id = entry_id;
    acl_entry.mode = 1;
    CTC_ERROR_RETURN(ctc_acl_add_entry(CTC_APP_VLAN_PORT_GEM_LMT_ACL_GRP, &acl_entry));


    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACDA_LKUP;
    key_field.data = 1;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_METADATA;
    key_field.data = ENCODE_ACL_METADATA(3);
    key_field.mask = 0x3FFF;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACSA_LKUP;
    key_field.data = 1;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));
    sal_memset(&key_field, 0, sizeof(key_field));
    key_field.type = CTC_FIELD_KEY_MACSA_HIT;
    key_field.data = 0;
    key_field.mask = 1;
    CTC_ERROR_RETURN(ctc_acl_add_key_field(entry_id, &key_field));

    sal_memset(&action_field, 0, sizeof(action_field));
    action_field.type = CTC_ACL_FIELD_ACTION_DISCARD;
    CTC_ERROR_RETURN(ctc_acl_add_action_field(entry_id, &action_field));
    sal_memset(&action_field, 0, sizeof(action_field));
    action_field.type = CTC_ACL_FIELD_ACTION_DENY_LEARNING;
    CTC_ERROR_RETURN(ctc_acl_add_action_field(entry_id, &action_field));
    sal_memset(&action_field, 0, sizeof(action_field));
    sal_memset(&acl_cp_to_cpu, 0, sizeof(acl_cp_to_cpu));
    acl_cp_to_cpu.mode = CTC_ACL_TO_CPU_MODE_TO_CPU_COVER;
    acl_cp_to_cpu.cpu_reason_id = CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT;
    action_field.type = CTC_ACL_FIELD_ACTION_CP_TO_CPU;
    action_field.ext_data = &acl_cp_to_cpu;
    CTC_ERROR_RETURN(ctc_acl_add_action_field(entry_id, &action_field));
    CTC_ERROR_RETURN(ctc_acl_install_entry(entry_id));

    return CTC_E_NONE;
}
#endif

STATIC int32
_ctc_gbx_app_vlan_port_add_fdb_learn_with_def_fid(uint8 lchip, mac_addr_t mac_addr, uint16 vdev_id, uint16 pkt_svlan, uint16 pkt_cvlan, uint16 logic_port, uint16* p_fid)
{
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fid_db_t* p_port_fid = NULL;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_find = NULL;

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = vdev_id;
    port_fid.pkt_svlan = pkt_svlan;
    port_fid.pkt_cvlan = pkt_cvlan;
    port_fid.rsv = 1;
    CTC_ERROR_RETURN(ctc_spool_add(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL, &p_port_fid));
    p_port_fid->rsv = 1;

    p_nni_port_find = _ctc_gbx_app_vlan_port_find_nni_port(lchip, logic_port);
    if (NULL == p_nni_port_find)
    {
       _ctc_gbx_app_vlan_port_add_uplink_svlan_entry(lchip, logic_port, pkt_svlan, pkt_cvlan, p_port_fid->fid);
    }


    *p_fid = p_port_fid->fid;

    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add default fid learn : MAC:%.4x.%.4x.%.4x  SVLAN:%-5d\n",
                    sal_ntohs(*(unsigned short*)&mac_addr[0]),
                    sal_ntohs(*(unsigned short*)&mac_addr[2]),
                    sal_ntohs(*(unsigned short*)&mac_addr[4]),
                    pkt_svlan);


    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_remove_fdb_learn_with_def_fid(uint8 lchip, uint16 logic_port, ctc_app_vlan_port_fid_db_t* p_port_fid)
{
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fid_db_t* p_port_fid_tmp = NULL;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_find = NULL;

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid.vdev_id = p_port_fid->vdev_id;
    port_fid.pkt_svlan = p_port_fid->pkt_svlan;
    port_fid.pkt_cvlan = p_port_fid->pkt_cvlan;
    ctc_spool_remove(p_g_app_vlan_port_master->fid_spool, &port_fid, NULL);

    p_port_fid_tmp = ctc_spool_lookup(p_g_app_vlan_port_master->fid_spool, &port_fid);
    if (p_port_fid_tmp)
    {
        p_port_fid_tmp->rsv = 0;
    }

    p_nni_port_find = _ctc_gbx_app_vlan_port_find_nni_port(lchip, logic_port);
    if (NULL == p_nni_port_find)
    {
        _ctc_gbx_app_vlan_port_remove_uplink_svlan_entry(lchip, logic_port, p_port_fid->pkt_svlan, p_port_fid->fid);
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_hw_learning_sync(uint8 gchip, void* p_data)
{
    ctc_learning_cache_t* p_cache = (ctc_learning_cache_t*)p_data;
    ctc_l2_addr_t l2_addr;
    uint32 index = 0;
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;
    uint32 ad_index;
    ctc_app_vlan_port_db_t* p_vlan_port_find = NULL;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_find = NULL;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_find = NULL;
    uint8 lchip = 0;
    uint8 need_fid_add = 0;
    uint8 need_gem_port_add = 0;
    uint16 fid_new = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_app_vlan_port_fdb_node_t* p_fdb_node = NULL;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    CTC_APP_VLAN_PORT_LOCK(lchip);
    ctc_app_index_get_lchip_id(gchip, &lchip);
    for (index = 0; index < p_cache->entry_num; index++)
    {
        /* pizzabox */
        l2_addr.fid = p_cache->learning_entry[index].fid;
        l2_addr.flag = 0;

        l2_addr.mac[0] = (p_cache->learning_entry[index].mac_sa_32to47 & 0xff00) >> 8;
        l2_addr.mac[1] = (p_cache->learning_entry[index].mac_sa_32to47 & 0xff);
        l2_addr.mac[2] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff000000) >> 24;
        l2_addr.mac[3] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff0000) >> 16;
        l2_addr.mac[4] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff00) >> 8;
        l2_addr.mac[5] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff);

        if (p_cache->learning_entry[index].is_logic_port)
        {

            l2_addr.gport = p_cache->learning_entry[index].logic_port;

            pkt_svlan = p_cache->learning_entry[index].svlan_id;
            pkt_cvlan = p_cache->learning_entry[index].cvlan_id;

            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "SynC fdb, logic_port:%d, svlan:%d, cvlan:%d \n",
                            l2_addr.gport,
                            pkt_svlan,
                            pkt_cvlan);

            p_fdb_node = _ctc_gbx_app_vlan_port_fdb_hash_lkup(lchip, &l2_addr);
            if (p_fdb_node)
            {
                /*gem port mac limit aging*/
                p_vlan_port_find = _ctc_gbx_app_vlan_port_find_vlan_port_db(lchip, p_fdb_node->logic_port);
                if (p_vlan_port_find)
                {
                    _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_find->p_gem_port_db, FALSE, &need_gem_port_add);
                    p_vlan_port_find->count--;
                }
                else
                {
                    p_gem_port_find = _ctc_gbx_app_vlan_port_find_gem_port_db(lchip, p_fdb_node->logic_port);
                    if (p_gem_port_find)
                    {
                        _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_gem_port_find, FALSE, &need_gem_port_add);
                    }
                }
                _ctc_gbx_app_vlan_port_fdb_hash_remove(lchip, &l2_addr);
            }

            /*ONU*/
            p_vlan_port_find = _ctc_gbx_app_vlan_port_find_vlan_port_db(lchip, l2_addr.gport);
            if ((NULL != p_vlan_port_find) && (((pkt_svlan == p_vlan_port_find->match_svlan)
                && (pkt_cvlan == p_vlan_port_find->match_cvlan))
                || ((p_vlan_port_find->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX)
                && (p_vlan_port_find->fid == l2_addr.fid))))
            {
                if (l2_addr.fid != p_g_app_vlan_port_master->default_bcast_fid[p_vlan_port_find->vdev_id])
                {
                    nhid =  p_vlan_port_find->xlate_nhid;
                    ad_index = p_vlan_port_find->ad_index;
                    _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_find->p_gem_port_db, TRUE, &need_gem_port_add);
                    _ctc_gbx_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                    if (need_fid_add && need_gem_port_add)
                    {
                        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port find nhid:%u, ad_index:%u  !!!\n", nhid, ad_index);
                        if (p_vlan_port_find->p_gem_port_db && p_vlan_port_find->p_gem_port_db->station_move_action)
                        {
                            CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_SRC_DISCARD);
                        }
                        CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                        p_vlan_port_find->count++;
                        _ctc_gbx_app_vlan_port_fdb_hash_add(lchip, &l2_addr, ad_index);
                    }
                    else
                    {
                        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port mac limit fid %u gem_port %u!!!\n", need_fid_add, need_gem_port_add);
                    }
                    continue;
                }
            }

            /*PON*/
            p_gem_port_find = _ctc_gbx_app_vlan_port_find_gem_port_db(lchip, l2_addr.gport);
            if (p_gem_port_find)
            {
                if (l2_addr.fid == p_g_app_vlan_port_master->default_bcast_fid[p_gem_port_find->vdev_id])
                {
                    _ctc_gbx_app_vlan_port_add_fdb_learn_with_def_fid(lchip, l2_addr.mac, p_gem_port_find->vdev_id, pkt_svlan, pkt_cvlan, l2_addr.gport, &fid_new);
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
                    l2_addr.fid = fid_new;
                }

                port_fid.fid = l2_addr.fid;
                ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid);
                if (port_fid.is_flex)
                {
                    nhid = p_gem_port_find->flex_e2eloop_nhid;
                }
                else
                {
#ifdef GREATBELT
                    nhid = p_gem_port_find->e2eloop_nhid;
#else
                    nhid = p_gem_port_find->e2iloop_nhid;
#endif
                }
                ad_index = p_gem_port_find->ad_index;
                _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_gem_port_find, TRUE, &need_gem_port_add);
                _ctc_gbx_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                if (need_fid_add && need_gem_port_add)
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "GemPort find nhid:%d  ad_index:%d  !!!\n", nhid, ad_index);
                    if (p_gem_port_find->station_move_action)
                    {
                        CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_SRC_DISCARD);
                    }
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                    _ctc_gbx_app_vlan_port_fdb_hash_add(lchip, &l2_addr, ad_index);
                }
                else
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Gem port mac limit fid %u gem_port %u!!!\n", need_fid_add, need_gem_port_add);
                }

                continue;
            }

            /*NNI port*/
            p_nni_port_find = _ctc_gbx_app_vlan_port_find_nni_port(lchip, l2_addr.gport);
            if (p_nni_port_find)
            {
                CTC_APP_DBG_INFO("p_g_app_vlan_port_master->nni_logic_port:%d\n", p_nni_port_find->nni_logic_port);
                if (l2_addr.fid == p_g_app_vlan_port_master->default_bcast_fid[p_nni_port_find->vdev_id])
                {
                    _ctc_gbx_app_vlan_port_add_fdb_learn_with_def_fid(lchip, l2_addr.mac, p_nni_port_find->vdev_id, pkt_svlan, pkt_cvlan, l2_addr.gport, &fid_new);
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
                    l2_addr.fid = fid_new;
                }
                if (CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN))
                {
                    nhid = p_nni_port_find->nni_nh_id;
                }
                else
                {
                    nhid = p_g_app_vlan_port_master->nni_mcast_nhid[p_nni_port_find->vdev_id];
                }
                ad_index = p_nni_port_find->nni_ad_index;
                _ctc_gbx_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                if (need_fid_add)
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NNI find nhid:%d ad_index:%d  !!!\n", nhid, ad_index);
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                    _ctc_gbx_app_vlan_port_fdb_hash_add(lchip, &l2_addr, ad_index);
                }
                else
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NNI find mac limit fid %u!!!\n", need_fid_add);
                }
                continue;
            }

            /*ONU Vlan Range*/
            if (p_vlan_port_find && ((p_vlan_port_find->match_svlan_end != 0) && ( pkt_svlan >= p_vlan_port_find->match_svlan) && (pkt_svlan <= p_vlan_port_find->match_svlan_end)))
            {
                if (l2_addr.fid == p_g_app_vlan_port_master->default_bcast_fid[p_vlan_port_find->vdev_id])
                {
                    _ctc_gbx_app_vlan_port_add_fdb_learn_with_def_fid(lchip, l2_addr.mac, p_vlan_port_find->vdev_id, pkt_svlan, pkt_cvlan, l2_addr.gport, &fid_new);
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
                    l2_addr.fid = fid_new;
                }
                nhid = p_vlan_port_find->xlate_nhid;
                ad_index = p_vlan_port_find->ad_index;
                _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_find->p_gem_port_db, TRUE, &need_gem_port_add);
                _ctc_gbx_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                if (need_fid_add && need_gem_port_add)
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port find nhid:%u, ad_index:%u  !!!\n", nhid, ad_index);
                    CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                    _ctc_gbx_app_vlan_port_fdb_hash_add(lchip, &l2_addr, ad_index);
                }
                else
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port mac limit fid %u gem_port %u!!!\n", need_fid_add, need_gem_port_add);
                }
                continue;
            }
        }
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_gbx_app_vlan_port_hw_aging_sync(uint8 gchip, void* p_data)
{
    ctc_aging_fifo_info_t* p_fifo = (ctc_aging_fifo_info_t*)p_data;

    uint32 i = 0;
    int32 ret = CTC_E_NONE;
    ctc_l2_addr_t l2_addr;
    ctc_l2_addr_t l2_addr_rst;
    uint8 lchip = 0;
    uint8 chip_type = 0;
    uint8 need_fid_add = 0;
    uint8 need_gem_port_add = 0;
    ctc_app_vlan_port_fid_db_t port_fid;
    ctc_l2_fdb_query_t fdb_query;
    ctc_l2_fdb_query_rst_t fdb_rst;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_find = NULL;

    ctc_app_index_get_lchip_id(gchip, &lchip);
    /*Using Dma*/
    CTC_APP_VLAN_PORT_LOCK(lchip);
    for (i = 0; i < p_fifo->fifo_idx_num; i++)
    {
        sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

        chip_type = ctc_get_chip_type(lchip);
        switch (chip_type)
        {
#ifdef GREATBELT
            case CTC_CHIP_GREATBELT:
                {
                    uint32 index = 0;
                    index = p_fifo->aging_index_array[i];
                    ctc_l2_get_fdb_by_index(index, &l2_addr);
                }
                break;
#endif
#ifdef DUET2
            case CTC_CHIP_DUET2:
                {
                    ctc_aging_info_entry_t* p_entry = NULL;
                    p_entry = &(p_fifo->aging_entry[i]);
                    l2_addr.fid = p_entry->fid;
                    sal_memcpy(l2_addr.mac, p_entry->mac, sizeof(mac_addr_t));
                }
                break;
#endif
            default:
                CTC_APP_VLAN_PORT_UNLOCK(lchip);
                return CTC_E_INVALID_PARAM;
        }

        CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_REMOVE_DYNAMIC);

        sal_memset(&fdb_query, 0, sizeof(ctc_l2_fdb_query_t));
        sal_memset(&fdb_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));
        fdb_query.fid = l2_addr.fid;
        sal_memcpy(fdb_query.mac, l2_addr.mac, sizeof(mac_addr_t));
        fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        fdb_query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
        fdb_rst.buffer_len = sizeof(ctc_l2_addr_t);
        fdb_rst.buffer = &l2_addr_rst;
        ctc_l2_get_fdb_entry(&fdb_query, &fdb_rst);

        ret = CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
        _ctc_gbx_app_vlan_port_fdb_hash_remove(lchip, &l2_addr);
        l2_addr.gport = fdb_rst.buffer[0].gport;
        /*gem port mac limit aging*/
        p_vlan_port_db = _ctc_gbx_app_vlan_port_find_vlan_port_db(lchip, l2_addr.gport);
        if (p_vlan_port_db)
        {
            p_vlan_port_db->count--;
            _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_db->p_gem_port_db, FALSE, &need_gem_port_add);
        }
        else
        {
            p_gem_port_find = _ctc_gbx_app_vlan_port_find_gem_port_db(lchip, l2_addr.gport);
            if (p_gem_port_find)
            {
                _ctc_gbx_app_vlan_port_sync_gem_port_mac_limit(lchip, p_gem_port_find, FALSE, &need_gem_port_add);
            }
        }
        _ctc_gbx_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, FALSE, &need_fid_add);
        sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_db_t));
        port_fid.fid = l2_addr.fid;
        ret = _ctc_gbx_app_vlan_port_get_fid_by_fid(&port_fid);
        if (ret && port_fid.rsv)
        {
            sal_memset(&fdb_query, 0, sizeof(ctc_l2_fdb_query_t));
            fdb_query.fid = l2_addr.fid;
            fdb_query.gport = l2_addr.gport;
            fdb_query.use_logic_port = 1;
            fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
            fdb_query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
            ctc_l2_get_fdb_count(&fdb_query);
            if (0 == fdb_query.count)
            {
                _ctc_gbx_app_vlan_port_remove_fdb_learn_with_def_fid(lchip, l2_addr.gport, &port_fid);
            }
        }
        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Aging Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d\n",
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                        l2_addr.fid);

        if (ret)
        {
            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Aging fdb fail, ret:%d \n", ret);
        }
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_init(uint8 lchip, void *p_param)
{
    uint8 gchip = 0;
    uint8 chip_type = 0;
    uint16 policer_ptr = 0;
    uint16 vdev_index = 0;
    uint16 vdev_num = 0;
    ctc_app_vlan_port_init_cfg_t* p_app_init = NULL;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] ={0};

    if (NULL != p_g_app_vlan_port_master)
    {
         return CTC_E_INIT_FAIL;
    }

    p_app_init = (ctc_app_vlan_port_init_cfg_t*)p_param;
    if(NULL == p_app_init)
    {
        vdev_num = 1;
    }
    else
    {
        vdev_num = p_app_init->vdev_num;
    }
    if (vdev_num > CTC_APP_VLAN_PORT_MAX_VDEV_NUM || (vdev_num == 0))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_app_init->mcast_tunnel_vlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_app_init->bcast_tunnel_vlan);

    MALLOC_POINTER(ctc_app_vlan_port_master_t, p_g_app_vlan_port_master);
    if (NULL == p_g_app_vlan_port_master)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_g_app_vlan_port_master, 0, sizeof(ctc_app_vlan_port_master_t));
    p_g_app_vlan_port_master->vdev_num = vdev_num;
    p_g_app_vlan_port_master->mcast_tunnel_vlan = ((0 == p_app_init->mcast_tunnel_vlan)?CTC_APP_VLAN_PORT_MCAST_TUNNEL_ID:p_app_init->mcast_tunnel_vlan);
    p_g_app_vlan_port_master->bcast_tunnel_vlan = ((0 == p_app_init->bcast_tunnel_vlan)?CTC_APP_VLAN_PORT_BCAST_TUNNEL_ID:p_app_init->bcast_tunnel_vlan);
    p_g_app_vlan_port_master->flag = p_app_init->flag;

    /*GB function API callback*/
    chip_type = ctc_get_chip_type(lchip);
    switch (chip_type)
    {
#ifdef GREATBELT
        case CTC_CHIP_GREATBELT:
            p_g_app_vlan_port_master->ctc_global_set_xgpon_en = sys_greatbelt_global_set_xgpon_en;
            p_g_app_vlan_port_master->ctc_port_set_service_policer_en = sys_greatbelt_port_set_service_policer_en;
            p_g_app_vlan_port_master->ctc_nh_set_xgpon_en = sys_greatbelt_nh_set_xgpon_en;
            p_g_app_vlan_port_master->ctc_port_set_global_port = sys_greatbelt_port_set_global_port;
            p_g_app_vlan_port_master->ctc_nh_update_logic_port = sys_greatbelt_nh_update_logic_port;
            p_g_app_vlan_port_master->ctc_qos_policer_index_get = sys_greatbelt_qos_policer_index_get;
            p_g_app_vlan_port_master->ctc_qos_policer_reserv_service_hbwp = sys_greatbelt_qos_policer_reserv_service_hbwp;
            p_g_app_vlan_port_master->ctc_nh_set_mcast_bitmap_ptr = sys_greatbelt_nh_set_mcast_bitmap_ptr;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db = sys_greatbelt_l2_hw_sync_add_db;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db = sys_greatbelt_l2_hw_sync_remove_db;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_alloc_ad_idx = sys_greatbelt_l2_hw_sync_alloc_ad_idx;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_free_ad_idx = sys_greatbelt_l2_hw_sync_free_ad_idx;
            p_g_app_vlan_port_master->ctc_port_set_mac_rx_en = sys_greatbelt_port_set_mac_rx_en;
            p_g_app_vlan_port_master->ctc_port_set_logic_port_en = sys_greatbelt_port_set_logic_port_check_en;
            p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action = NULL;
            p_g_app_vlan_port_master->ctc_port_set_ingress_property = NULL;
            p_g_app_vlan_port_master->ctc_nh_update_xlate = NULL;
            p_g_app_vlan_port_master->ctc_l2_fdb_set_station_move_action = NULL;
            break;
#endif
#ifdef DUET2
        case CTC_CHIP_DUET2:
            p_g_app_vlan_port_master->ctc_global_set_xgpon_en = sys_usw_global_set_xgpon_en;
            p_g_app_vlan_port_master->ctc_port_set_service_policer_en = NULL;
            p_g_app_vlan_port_master->ctc_nh_set_xgpon_en = sys_usw_nh_set_xgpon_en;
            p_g_app_vlan_port_master->ctc_port_set_global_port = NULL;
            p_g_app_vlan_port_master->ctc_nh_update_logic_port = sys_usw_nh_update_logic_port;
            p_g_app_vlan_port_master->ctc_qos_policer_index_get = NULL;
            p_g_app_vlan_port_master->ctc_qos_policer_reserv_service_hbwp = NULL;
            p_g_app_vlan_port_master->ctc_nh_set_mcast_bitmap_ptr = sys_usw_nh_set_mcast_bitmap_ptr;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_add_db = sys_usw_l2_hw_sync_add_db;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_remove_db = sys_usw_l2_hw_sync_remove_db;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_alloc_ad_idx = sys_usw_l2_hw_sync_alloc_ad_idx;
            p_g_app_vlan_port_master->ctc_l2_hw_sync_free_ad_idx = sys_usw_l2_hw_sync_free_ad_idx;
            p_g_app_vlan_port_master->ctc_ftm_alloc_table_offset_from_position = sys_usw_ftm_alloc_table_offset_from_position;
            p_g_app_vlan_port_master->ctc_port_set_mac_rx_en = sys_usw_mac_set_mac_rx_en;
            p_g_app_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action = sys_usw_mac_security_set_fid_learn_limit_action;
            p_g_app_vlan_port_master->ctc_port_set_ingress_property = sys_usw_port_set_ingress_property;
            p_g_app_vlan_port_master->ctc_nh_update_xlate = sys_usw_egress_vlan_edit_nh_update;
            p_g_app_vlan_port_master->ctc_l2_fdb_set_station_move_action = sys_usw_l2_update_station_move_action;
            break;
#endif
        default:
            mem_free(p_g_app_vlan_port_master);
            p_g_app_vlan_port_master = NULL;
            return CTC_E_NOT_SUPPORT;
            break;
    }

    /*Check xgpon global support*/
    if (NULL == p_g_app_vlan_port_master->ctc_global_set_xgpon_en)
    {
        mem_free(p_g_app_vlan_port_master);
        p_g_app_vlan_port_master = NULL;
        return CTC_E_NOT_SUPPORT;
    }

    /*create mutex*/
    CTC_APP_VLAN_PORT_CREAT_LOCK(lchip);

    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_global_set_xgpon_en, lchip, vdev_num));
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_set_xgpon_en, lchip, TRUE, p_g_app_vlan_port_master->flag));

    ctc_get_gchip_id(lchip, &gchip);

    ctc_app_index_init(gchip);


    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_nh_set_mcast_bitmap_ptr, lchip, p_g_app_vlan_port_master->mcast_tunnel_vlan));
    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    p_g_app_vlan_port_master->mcast_max_group_num = capability[CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM];
    p_g_app_vlan_port_master->max_fid_num = (p_g_app_vlan_port_master->mcast_max_group_num<CTC_APP_VLAN_PORT_MAX_FID+1024)?(p_g_app_vlan_port_master->mcast_max_group_num-1024):CTC_APP_VLAN_PORT_MAX_FID;
    /*------------DB------------*/
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_init_db(lchip));

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_init_opf(lchip, vdev_num));

    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_init_port(lchip, vdev_num));

    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_qos_policer_reserv_service_hbwp, lchip));
    CTC_ERROR_RETURN(CTC_APP_VLAN_PORT_API_CALL(p_g_app_vlan_port_master->ctc_qos_policer_index_get, lchip,
                                                                            CTC_QOS_POLICER_TYPE_FLOW,
                                                                            0xFFFE,
                                                                            &policer_ptr));

    for (vdev_index=0; vdev_index<vdev_num; vdev_index++)
    {
        p_g_app_vlan_port_master->default_logic_dest_port[vdev_index] = policer_ptr/4;
        p_g_app_vlan_port_master->uni_inner_isolate_en[vdev_index] = 1;
    }

    if (CTC_CHIP_GREATBELT == chip_type)
    {
        ctc_app_index_set_chipset_type(gchip, CTC_APP_CHIPSET_CTC5160);
    }
    else if (CTC_CHIP_DUET2 == chip_type)
    {
        ctc_acl_hash_field_sel_t sel;
        ctc_app_index_set_chipset_type(gchip, CTC_APP_CHIPSET_CTC7148);

        sal_memset(&sel, 0, sizeof(sel));
        sel.hash_key_type = CTC_ACL_KEY_HASH_MAC;
        sel.u.mac.phy_port = 1;
        sel.u.mac.vlan_id = 1;
        sel.u.mac.tag_valid = 1;
        sel.field_sel_id = 0;

        ctc_acl_set_hash_field_sel(&sel);
    }

    ctc_interrupt_register_event_cb(CTC_EVENT_L2_SW_LEARNING, ctc_gbx_app_vlan_port_hw_learning_sync);
    ctc_interrupt_register_event_cb(CTC_EVENT_L2_SW_AGING, ctc_gbx_app_vlan_port_hw_aging_sync);
#ifdef DUET2
    CTC_ERROR_RETURN(_ctc_gbx_app_vlan_port_init_mac_limit_info(lchip));
#endif
    return CTC_E_NONE;
}

#define _______DUMP________

int32
_ctc_gbx_app_vlan_port_show_gem_port(ctc_app_vlan_port_gem_port_db_t *p_gem_port_db,
                                 uint32* i)
{
    CTC_PTR_VALID_CHECK(i);
    CTC_PTR_VALID_CHECK(p_gem_port_db);

    CTC_APP_DBG_DUMP("%-5d %5d %15d %15d %15d\n",
               *i,
               p_gem_port_db->port,
               p_gem_port_db->tunnel_value,
               p_gem_port_db->logic_port,
               p_gem_port_db->ref_cnt);
    (*i)++;

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_show_vlan_port(ctc_app_vlan_port_db_t *p_vlan_port_db,
                                  uint32* i)
{
   CTC_PTR_VALID_CHECK(i);
   CTC_PTR_VALID_CHECK(p_vlan_port_db);


   CTC_APP_DBG_DUMP("%-5d %5d %5d %15d %10d %10d %10d %15d\n",
              *i,
              p_vlan_port_db->vlan_port_id,
              p_vlan_port_db->port,
              p_vlan_port_db->tunnel_value,
              p_vlan_port_db->match_svlan,
              p_vlan_port_db->match_cvlan,
              p_vlan_port_db->match_svlan_end,
              p_vlan_port_db->logic_port);
    (*i)++;

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_uni_port(uint8 lchip, uint8 detail, uint32 port)
{
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;

    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, port);
    if (CTC_IS_LINKAGG_PORT(port))
    {
        return CTC_E_INVALID_GLOBAL_PORT;
    }
    lport = CTC_MAP_GPORT_TO_LPORT(port);

    p_uni_port = &p_g_app_vlan_port_master->p_port_pon[lport];

    CTC_APP_DBG_DUMP("port: 0x%x detail:\n", port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ref_cnt"          , p_uni_port->ref_cnt);

    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "iloop_port"       , p_uni_port->iloop_port);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "e2eloop_port"     , p_uni_port->e2eloop_port);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "onu_e2eloop_port" , p_uni_port->onu_e2eloop_port);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "mc_eloop_port"    , p_uni_port->mc_eloop_port);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "bc_eloop_port"    , p_uni_port->bc_eloop_port);

    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "iloop_nhid"       , p_uni_port->iloop_nhid);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "e2iloop_nhid"     , p_uni_port->e2iloop_nhid);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "mc_xlate_nhid"    , p_uni_port->mc_xlate_nhid);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "mc_e2iloop_nhid"  , p_uni_port->mc_e2iloop_nhid);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "bc_xlate_nhid"    , p_uni_port->bc_xlate_nhid);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "bc_e2iloop_nhid"  , p_uni_port->bc_e2iloop_nhid);

    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "bc_mc_installed"  , p_uni_port->bc_mc_installed);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "vlan_range_grp"   , p_uni_port->vlan_range_grp);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_nni_port(uint8 lchip, uint32 port)
{
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t *p_nni_port_db = NULL;

    CTC_APP_VLAN_PORT_GLOABL_PORT_CHECK(lchip, port);
    /* DB */
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = port;
    p_nni_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        return CTC_E_NOT_EXIST;
    }

    CTC_APP_DBG_DUMP("port: 0x%x detail:\n", port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "vDev ID"      , p_nni_port_db->vdev_id);
    CTC_APP_DBG_DUMP("%-40s : %10d\n" , "Logic Port"   , p_nni_port_db->nni_logic_port);
    CTC_APP_DBG_DUMP("%-40s : %10s\n" , "Rx Enable"       , p_nni_port_db->rx_en?"Enable":"Disable");

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_gem_port(uint8 lchip, uint8 detail, uint32 port, uint32 tunnel_value)
{

    if (0 == detail)
    {
        uint32 i = 1;
        CTC_APP_DBG_DUMP("%-5s %5s %15s %15s %15s\n", "No.", "port", "tunnel-value", "logic port", "ref cnt");
        CTC_APP_DBG_DUMP("-----------------------------------------------------------------\n");

        ctc_hash_traverse(p_g_app_vlan_port_master->gem_port_hash,
                          (hash_traversal_fn)_ctc_gbx_app_vlan_port_show_gem_port, &i);
    }
    else
    {
        ctc_app_vlan_port_gem_port_db_t gem_port_db;
        ctc_app_vlan_port_gem_port_db_t *p_gem_port_db = NULL;

        /* DB */
        sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
        gem_port_db.port = port;
        gem_port_db.tunnel_value = tunnel_value;
        p_gem_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->gem_port_hash, &gem_port_db);
        if (NULL == p_gem_port_db)
        {
            return CTC_E_NOT_EXIST;
        }

        CTC_APP_DBG_DUMP("port: 0x%x  tunnel-value:%d detail:\n", port, tunnel_value);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ref_cnt"      , p_gem_port_db->ref_cnt);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flag"         , p_gem_port_db->flag);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "logic_port"   , p_gem_port_db->logic_port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "xlate_nhid"   , p_gem_port_db->xlate_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "e2eloop_nhid" , p_gem_port_db->e2eloop_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_e2eloop_nhid" , p_gem_port_db->flex_e2eloop_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "onu_e2eloop_nhid" , p_gem_port_db->onu_eloop_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ingress_policer_id" , p_gem_port_db->ingress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egress_policer_id"  , p_gem_port_db->egress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egress_cos_idx"  , p_gem_port_db->egress_cos_idx);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ingress_stats_id" , p_gem_port_db->ingress_stats_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egress_stats_id"  , p_gem_port_db->egress_stats_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "logic_dest_port"    , p_gem_port_db->logic_dest_port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "station_move_action", p_gem_port_db->station_move_action);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "custom_id"          , p_gem_port_db->custom_id);
    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_vlan_port(uint8 lchip, uint8 detail, uint32 vlan_port_id)
{

    if (0 == detail)
    {
        uint32 i = 1;
        CTC_APP_DBG_DUMP("%-5s %5s %5s %15s %10s %10s %10s %15s\n", "No.", "id", "port", "tunnel-value", "svid", "cvid", "svid-end", "logic port");
        CTC_APP_DBG_DUMP("----------------------------------------------------------------------------------------------------------------------\n");

        ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_hash,
                          (hash_traversal_fn)_ctc_gbx_app_vlan_port_show_vlan_port, &i);
    }
    else
    {
        ctc_app_vlan_port_db_t vlan_port_db;
        ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
        sal_memset(&vlan_port_db, 0, sizeof(vlan_port_db));
        vlan_port_db.vlan_port_id = vlan_port_id;

        p_vlan_port_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_hash, &vlan_port_db);

        if (NULL == p_vlan_port_db)
        {
            return CTC_E_NOT_EXIST;
        }

        CTC_APP_DBG_DUMP("vlan-port-id: %d detail:\n", vlan_port_id);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "criteria"            , p_vlan_port_db->criteria);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "port"                , p_vlan_port_db->port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "tunnel_value"        , p_vlan_port_db->tunnel_value);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "match_svlan"         , p_vlan_port_db->match_svlan);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "match_cvlan"         , p_vlan_port_db->match_cvlan);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "match_svlan_end"     , p_vlan_port_db->match_svlan_end);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "match_cvlan_end"     , p_vlan_port_db->match_cvlan_end);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "logic_port"          , p_vlan_port_db->logic_port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "xlate_nhid"          , p_vlan_port_db->xlate_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egs_vlan_mapping_en" , p_vlan_port_db->egs_vlan_mapping_en);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "fid"                 , p_vlan_port_db->fid_mapping);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "pkt-svlan"           , p_vlan_port_db->pkt_svlan);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "pkt-cvlan"           , p_vlan_port_db->pkt_cvlan);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_add_vlan_xlate_nhid"   , p_vlan_port_db->flex_add_vlan_xlate_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_xlate_nhid"            , p_vlan_port_db->flex_xlate_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_logic_port_vlan"       , p_vlan_port_db->flex_logic_port_vlan);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_pon_uplink_iloop"   , p_vlan_port_db->pon_uplink_iloop_port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_pon_uplink_nhid"    , p_vlan_port_db->pon_uplink_e2iloop_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_pon_downlink_iloop" , p_vlan_port_db->pon_downlink_iloop_port);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "flex_pon_downlink_nhid"  , p_vlan_port_db->pon_downlink_e2iloop_nhid);

        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ingress_policer_id" , p_vlan_port_db->ingress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egress_policer_id"  , p_vlan_port_db->egress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "ingress_stats_id" , p_vlan_port_db->ingress_stats_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "egress_stats_id"  , p_vlan_port_db->egress_stats_id);
        CTC_APP_DBG_DUMP("%-40s : %10d\n" , "logic_dest_port"    , p_vlan_port_db->logic_dest_port);

    }

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_status(uint8 lchip)
{
    uint16 vdev_index = 0;

    CTC_APP_DBG_DUMP("%-40s : %d\n", "mcast tunnel vlan",  p_g_app_vlan_port_master->mcast_tunnel_vlan);
    CTC_APP_DBG_DUMP("%-40s : %d\n", "bcast tunnel vlan",  p_g_app_vlan_port_master->bcast_tunnel_vlan);
    CTC_APP_DBG_DUMP("%-40s : %s\n", "mutli nni en",    CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN)?"enable":"disable");
    CTC_APP_DBG_DUMP("%-40s : %s\n", "stats en",        CTC_FLAG_ISSET(p_g_app_vlan_port_master->flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN)?"enable":"disable");
    CTC_APP_DBG_DUMP("%-40s : %d\n", "max mcast group id",  p_g_app_vlan_port_master->mcast_max_group_num-1);
    CTC_APP_DBG_DUMP("%-40s : %d\n", "max fid",             p_g_app_vlan_port_master->max_fid_num-1);
    for (vdev_index=0; vdev_index<p_g_app_vlan_port_master->vdev_num; vdev_index++)
    {
        CTC_APP_DBG_DUMP("vDev ID:%d\n", vdev_index);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %d\n", "gem port",    p_g_app_vlan_port_master->gem_port_cnt[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "vlan port",   p_g_app_vlan_port_master->vlan_port_cnt[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "nni port",    p_g_app_vlan_port_master->nni_port_cnt[vdev_index]);

        CTC_APP_DBG_DUMP("\n");
        CTC_APP_DBG_DUMP("Resource:\n");
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %d\n", "downlink eloop port",    p_g_app_vlan_port_master->downlink_eloop_port[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "flex_1st_iloop_port",    p_g_app_vlan_port_master->flex_1st_iloop_port[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "flex_2nd_iloop_port",    p_g_app_vlan_port_master->flex_2nd_iloop_port[vdev_index]);

        CTC_APP_DBG_DUMP("%-40s : %d\n", "flex_1st_iloop_nhid",    p_g_app_vlan_port_master->flex_1st_iloop_nhid[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "flex_2nd_iloop_nhid",    p_g_app_vlan_port_master->flex_2nd_iloop_nhid[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "flex_2nd_e2iloop_nhid",  p_g_app_vlan_port_master->flex_2nd_e2iloop_nhid[vdev_index]);

        CTC_APP_DBG_DUMP("%-40s : %d\n", "default_logic_dest_port",p_g_app_vlan_port_master->default_logic_dest_port[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "nni mcast nhid",         p_g_app_vlan_port_master->nni_mcast_nhid[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %d\n", "downlink_iloop_port",    p_g_app_vlan_port_master->downlink_iloop_default_port[vdev_index]);
    }

    return CTC_E_NONE;
}

#define CTC_APP_VLAN_PORT_DUMP(param) CTC_APP_DBG_DUMP("%-40s :%10d\n", #param, vlan_port.param)
int32
ctc_gbx_app_vlan_port_show_logic_port(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_t vlan_port;

    CTC_APP_VLAN_PORT_INIT_CHECK();

    sal_memset(&vlan_port, 0, sizeof(vlan_port));

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_gbx_app_vlan_port_get_by_logic_port(lchip, logic_port, &vlan_port));
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_DUMP("logic port: %d detail:\n", logic_port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_VLAN_PORT_DUMP(vlan_port_id);
    CTC_APP_VLAN_PORT_DUMP(criteria);
    CTC_APP_DBG_DUMP("%-40s :%10x\n", "port", vlan_port.port);
    CTC_APP_VLAN_PORT_DUMP(match_tunnel_value);
    CTC_APP_VLAN_PORT_DUMP(match_svlan);
    CTC_APP_VLAN_PORT_DUMP(match_cvlan);
    CTC_APP_VLAN_PORT_DUMP(match_svlan_end);
    CTC_APP_VLAN_PORT_DUMP(match_cvlan_end);

    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.svid);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.new_svid);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.cvid);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.new_cvid);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.scos);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.new_scos);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.ccos);
    CTC_APP_VLAN_PORT_DUMP(ingress_vlan_action_set.new_ccos);

    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.svid);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.new_svid);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.cvid);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.new_cvid);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.scos);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.new_scos);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.ccos);
    CTC_APP_VLAN_PORT_DUMP(egress_vlan_action_set.new_ccos);

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show(uint8 lchip, uint8 type, uint8 detail, uint32 param0, uint32 param1)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    CTC_APP_VLAN_PORT_LOCK(lchip);
    switch(type)
    {
        case 0:
            ctc_gbx_app_vlan_port_show_status(lchip);
            break;
        case 1:
            ctc_gbx_app_vlan_port_show_uni_port(lchip, detail, param0);
            break;
        case 2:
            ctc_gbx_app_vlan_port_show_gem_port(lchip, detail, param0, param1);
            break;
        case 3:
            ctc_gbx_app_vlan_port_show_vlan_port(lchip, detail, param0);
            break;
        case 4:
            ctc_gbx_app_vlan_port_show_nni_port(lchip, param0);
            break;

        default:
           break;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
_ctc_gbx_app_vlan_port_show_sync_db(ctc_app_vlan_port_sync_db_t *p_sync_db,
                                  uint32* i)
{
    CTC_PTR_VALID_CHECK(i);
    CTC_PTR_VALID_CHECK(p_sync_db);

    CTC_APP_DBG_DUMP("%-10d %15d %10d %10d %15d %15d %15d\n",
              *i,
              p_sync_db->logic_port,
              p_sync_db->match_svlan,
              p_sync_db->match_cvlan,
              p_sync_db->oun_nhid,
              p_sync_db->pon_nhid,
              p_sync_db->ad_index);
    (*i)++;

    return CTC_E_NONE;
}

int32
ctc_gbx_app_vlan_port_show_sync_db(uint8 lchip, uint8 detail, uint32 logic_port, uint16 match_svlan, uint16 match_cvlan)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    CTC_APP_VLAN_PORT_LOCK(lchip);
    if (0 == detail)
    {
        uint32 i = 1;
        CTC_APP_DBG_DUMP("Sync DB count: %d\n", p_g_app_vlan_port_master->vlan_port_logic_vlan_hash->count);
        CTC_APP_DBG_DUMP("%-10s %15s %10s %10s %15s %15s %15s\n", "No.", "Lport", "svlan", "cvlan", "oun-nhid", "pon-nhid", "adindex");
        CTC_APP_DBG_DUMP("-------------------------------------------------------------------------------------------------------\n");

        ctc_hash_traverse(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash,
                          (hash_traversal_fn)_ctc_gbx_app_vlan_port_show_sync_db, &i);
    }
    else
    {
        ctc_app_vlan_port_sync_db_t sync_db;
        ctc_app_vlan_port_sync_db_t *p_sync_db = NULL;
        sal_memset(&sync_db, 0, sizeof(ctc_app_vlan_port_sync_db_t));

        sync_db.logic_port = logic_port;
        sync_db.match_svlan = match_svlan;
        sync_db.match_cvlan = match_cvlan;
        p_sync_db = ctc_hash_lookup(p_g_app_vlan_port_master->vlan_port_logic_vlan_hash, &sync_db);

        if (NULL == p_sync_db)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }

        CTC_APP_DBG_DUMP("%-15s %-10s %-10s %-15s %-15s %-15s\n", "Logic port", "match svlan", "match cvlan", "oun-nhid", "pon-nhid", "adindex");
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-15d" , p_sync_db->logic_port);
        CTC_APP_DBG_DUMP("%-10d" , p_sync_db->match_svlan);
        CTC_APP_DBG_DUMP("%-10d" , p_sync_db->match_cvlan);
        CTC_APP_DBG_DUMP("%-15d" , p_sync_db->oun_nhid);
        CTC_APP_DBG_DUMP("%-15d" , p_sync_db->pon_nhid);
        CTC_APP_DBG_DUMP("%-15d" , p_sync_db->ad_index);
        CTC_APP_DBG_DUMP("\n");
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

STATIC int32
_ctc_gbx_app_vlan_port_show_fid(void* node, void* user_data)
{
    ctc_app_vlan_port_fid_db_t* p_port_fid = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = (ctc_app_vlan_port_fid_db_t*)user_data;

    if (p_port_fid_temp->vdev_id == p_port_fid->vdev_id)
    {
        CTC_APP_DBG_DUMP("%-10d %10d %10d %10d %10d %10d\n",
              p_port_fid_temp->fid,
              p_port_fid->vdev_id,
              p_port_fid->pkt_svlan,
              p_port_fid->pkt_cvlan,
              p_port_fid->is_flex,
              p_port_fid->fid
             );
        p_port_fid_temp->fid++;
    }
    return CTC_E_NONE;
}


int32
ctc_gbx_app_vlan_port_show_fid(uint8 lchip, uint16 vdev_id)
{
    ctc_app_vlan_port_fid_db_t port_fid;
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(vdev_id);

    sal_memset(&port_fid, 0, sizeof(port_fid));
    port_fid.vdev_id = vdev_id;
    CTC_APP_VLAN_PORT_LOCK(lchip);

    CTC_APP_DBG_DUMP("%-10s %10s %10s %10s %10s %10s\n", "index", "vDev id", "Packet svlan", "Packet cvlan", "flex", "fid");
    CTC_APP_DBG_DUMP("---------------------------------------------------------------\n");

    ctc_spool_traverse(p_g_app_vlan_port_master->fid_spool,
                      (spool_traversal_fn)_ctc_gbx_app_vlan_port_show_fid, &port_fid);

    CTC_APP_DBG_DUMP("VLan mapping fid db: %d\n", port_fid.fid);
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#endif

