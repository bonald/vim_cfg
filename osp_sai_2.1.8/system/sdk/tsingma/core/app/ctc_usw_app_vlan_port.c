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
#include "ctc_usw_app_vlan_port.h"

#if defined(TSINGMA)
ctc_app_vlan_port_api_t ctc_usw_app_vlan_port_api =
{
    ctc_usw_app_vlan_port_init,
    ctc_usw_app_vlan_port_get_nni,
    ctc_usw_app_vlan_port_destory_nni,
    ctc_usw_app_vlan_port_create_nni,
    ctc_usw_app_vlan_port_update_nni,
    ctc_usw_app_vlan_port_update,
    ctc_usw_app_vlan_port_get,
    ctc_usw_app_vlan_port_destory,
    ctc_usw_app_vlan_port_create,
    ctc_usw_app_vlan_port_update_gem_port,
    ctc_usw_app_vlan_port_get_gem_port,
    ctc_usw_app_vlan_port_destory_gem_port,
    ctc_usw_app_vlan_port_create_gem_port,
    ctc_usw_app_vlan_port_get_uni,
    ctc_usw_app_vlan_port_destory_uni,
    ctc_usw_app_vlan_port_create_uni,
    ctc_usw_app_vlan_port_get_by_logic_port,
    ctc_usw_app_vlan_port_get_global_cfg,
    ctc_usw_app_vlan_port_set_global_cfg,
    ctc_usw_app_vlan_port_get_fid_mapping_info,
    ctc_usw_app_vlan_port_alloc_fid,
    ctc_usw_app_vlan_port_free_fid,
};

#define CTC_APP_VLAN_PORT_CREAT_LOCK(lchip)                 \
    do                                                      \
    {                                                       \
        sal_mutex_create(&p_usw_vlan_port_master->mutex); \
        if (NULL == p_usw_vlan_port_master->mutex)        \
        {                                                   \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY);              \
        }                                                   \
    } while (0)

#define CTC_APP_VLAN_PORT_LOCK(lchip) \
    if(p_usw_vlan_port_master->mutex) sal_mutex_lock(p_usw_vlan_port_master->mutex)

#define CTC_APP_VLAN_PORT_UNLOCK(lchip) \
    if(p_usw_vlan_port_master->mutex) sal_mutex_unlock(p_usw_vlan_port_master->mutex)

#define CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(op)               \
    do                                                          \
    {                                                           \
        int32 rv = (op);                                        \
        if (rv < 0)                                             \
        {                                                       \
            sal_mutex_unlock(p_usw_vlan_port_master->mutex);  \
            return rv;                                          \
        }                                                       \
    } while (0)

#define CTC_APP_VLAN_PORT_VLAN_BITMAP    (CTC_MAX_VLAN_ID+1)/32
#define CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE 0xFFFFFFFF

#define CTC_APP_VLAN_PORT_DEFULAT_FID 1

#define CTC_APP_VLAN_PORT_VLAN_ID_CHECK(val) \
do { \
    if ((val) > CTC_MAX_VLAN_ID)\
    {\
        return CTC_E_BADID;\
    }\
}while(0)

#define CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(val) \
do { \
    if ((val) > 0xFFFF)\
    {\
        return CTC_E_BADID;\
    }\
}while(0)

#define CTC_APP_VLAN_PORT_VDEV_ID_CHECK(ID) \
do {\
    if ((ID) >= p_usw_vlan_port_master->vdev_num)\
        return CTC_E_INVALID_PARAM; \
}while(0)

#define CTC_APP_VLAN_PORT_INIT_CHECK() \
do {\
    if(NULL == p_usw_vlan_port_master) return CTC_E_NOT_INIT;\
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

enum ctc_app_vlan_port_type_e
{
    CTC_APP_VLAN_PORT_TYPE_NONE,
    CTC_APP_VLAN_PORT_TYPE_NNI_PORT,
    CTC_APP_VLAN_PORT_TYPE_UNI_PORT,
    CTC_APP_VLAN_PORT_TYPE_GEM_PORT,
    CTC_APP_VLAN_PORT_TYPE_VLAN_PORT,
    CTC_APP_VLAN_PORT_TYPE_MAX
};
typedef enum ctc_app_vlan_port_type_e ctc_app_vlan_port_type_t;

#define CTC_APP_DBG_FUNC()          CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__)
#define CTC_APP_DBG_INFO(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_ERROR(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_PARAM(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ##__VA_ARGS__)
#define CTC_APP_DBG_DUMP(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__)

#define CTC_APP_DBG_PARAM_ST(param) CTC_APP_DBG_PARAM("%-40s :%10u\n", #param, p_vlan_port->param)

#define CTC_APP_VLAN_PORT_RSV_MCAST_GROUP_ID(id) (id+1)
#define ENCODE_SCL_XC_ENTRY_ID(entry_id) (1 << 24 | (entry_id))
#define ENCODE_SCL_DICARD_ENTRY_ID(entry_id) (2 << 24 | (entry_id))
#define ENCODE_SCL_VLAN_ENTRY_ID(entry_id, offset) (3 << 24 | ((CTC_APP_VLAN_PORT_MAX_VDEV_NUM*entry_id)+ offset))
#define ENCODE_SCL_DOWNLINK_ENTRY_ID(entry_id, id) (4<<24 | (entry_id*CTC_APP_VLAN_PORT_MAX_VDEV_NUM) | id)
#define ENCODE_SCL_DOWNLINK_VLAN_RANGE_ENTRY_ID(fid, id) (5<<24 | (fid)<<13 | id)
#define ENCODE_SCL_UPLINK_ENTRY_ID(fid, port) (6<<24 | (port)<<13 |(fid))
#define CTC_APP_VLAN_PORT_SCL_GROUP_ID 1
#define ENCODE_MCAST_GROUP_ID(BASE, OFFSET) (BASE + OFFSET)
#define CTC_APP_VLAN_PORT_MAX_RSV_MC_GROUP_ID      5*1024

enum ctc_app_vlan_port_opf_e
{
    CTC_APP_VLAN_PORT_OPF_LOGIC_PORT,  /*logic port*/
    CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID,
    CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN,
    CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP,
    CTC_APP_VLAN_PORT_OPF_FID,
    CTC_APP_VLAN_PORT_OPF_MAX
};
typedef enum ctc_app_vlan_port_opf_e ctc_app_vlan_port_opf_t;

struct ctc_app_vlan_port_bpe_gem_port_s
{
    uint16 lport;                      /**< [TM] local port */
    uint16 gem_vlan;                   /**< [TM] gem port vlaue */
    uint16 logic_port;                 /**< [TM] Logic port */
    uint8  pass_through_en;            /**< [TM] pass through enable/disable */
    uint8  mac_security;            /**< [TM] mac security : refer to ctc_maclimit_action_t */
};
typedef struct ctc_app_vlan_port_bpe_gem_port_s ctc_app_vlan_port_bpe_gem_port_t;

struct ctc_app_vlan_port_fid_db_s
{
  uint16 vdev_id;              /**< [GB.D2] [in] virtual device Identification */
  uint16 pkt_svlan;            /**< [GB.D2] [in] svlan id in packet */
  uint16 pkt_cvlan;            /**< [GB.D2] [in] cvlan id in packet */
  uint16 mcast_id;             /**< [GB.D2] [in] mcast group base id */
  uint16 fid;                  /**< [GB.D2] [out] forwarding id */
  uint32 limit_count;
  uint32 limit_num;
  uint16 limit_action;
  uint16 is_pass_through;
};
typedef struct ctc_app_vlan_port_fid_db_s ctc_app_vlan_port_fid_db_t;

struct ctc_app_vlan_port_uni_db_s
{
    uint8 flex_pon_cnt;
    uint8 flex_onu_cnt;
    uint8 rsv;
    uint8 vdev_id;
    uint8 isolated_id;
    uint8 vlan_range_grp;
    uint16 ref_cnt;
    uint32 mc_xlate_nhid;
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
    ctc_hash_t* nni_port_hash;
    ctc_hash_t* gem_port_hash;
    ctc_hash_t* vlan_port_hash;
    ctc_hash_t* vlan_port_key_hash;
    ctc_spool_t* fid_spool;

    uint16 gem_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 vlan_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 nni_port_cnt[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];

    uint16 nni_logic_port[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 nni_mcast_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 uni_outer_isolate_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 uni_inner_isolate_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint8 unknown_mcast_drop_en[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 default_mcast_group_id[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 default_bcast_fid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 default_unknown_mcast_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 default_unknown_bcast_nhid[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint16 limit_count[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 limit_num[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 limit_action[CTC_APP_VLAN_PORT_MAX_VDEV_NUM];
    uint32 vlan_glb_vdev[CTC_APP_VLAN_PORT_VLAN_BITMAP];  /**<[TM] when bit set, the vlan will use global vdev vlan to add packet header*/
    uint16 vdev_num;
    uint16 vdev_base_vlan;
    uint16 mcast_max_group_num;
    uint32 glb_unknown_mcast_nhid;
    uint32 glb_unknown_bcast_nhid;
    uint32 bcast_value_logic_port;
    ctc_app_vlan_port_uni_db_t* p_port_pon;
    sal_mutex_t* mutex;

    int32 (*ctc_global_set_xgpon_en)(uint8 lchip, uint8 enable);
    int32 (*ctc_port_set_service_policer_en)(uint8 lchip, uint32 gport, bool enable);
    int32 (*ctc_nh_set_xgpon_en)(uint8 lchip, uint8 enable);
    int32 (*ctc_port_set_global_port)(uint8 lchip, uint8 lport, uint16 gport);
    int32 (*ctc_nh_update_logic_port)(uint8 lchip, uint32 nhid, uint32 logic_port);
    int32 (*ctc_qos_policer_index_get)(uint8 lchip, uint16 policer_id, uint8 sys_policer_type, uint8* p_is_bwp);
    int32 (*ctc_qos_policer_reserv_service_hbwp)(uint8 lchip);
    int32 (*ctc_nh_set_mcast_bitmap_ptr)(uint8 lchip, uint32 offset);
    int32 (*ctc_l2_hw_sync_add_db)(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
    int32 (*ctc_l2_hw_sync_remove_db)(uint8 lchip, ctc_l2_addr_t* l2_addr);
    int32 (*ctc_l2_hw_sync_alloc_ad_idx)(uint8 lchip, uint32* ad_index);
    int32 (*ctc_l2_hw_sync_free_ad_idx)(uint8 lchip, uint32 ad_index);
    int32 (*ctc_ftm_alloc_table_offset_from_position)(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);
    int32 (*ctc_bpe_enable_gem_port)(uint8 lchip, uint32 gport, uint32 enable);
    int32 (*ctc_bpe_add_gem_port)(uint8 lchip, void* p_gem_port);
    int32 (*ctc_bpe_remove_gem_port)(uint8 lchip, void* p_gem_port);
    int32 (*ctc_nh_add_gem_port)(uint8 lchip, uint16 logic_dest_port, uint16 gem_vlan);
    int32 (*ctc_nh_remove_gem_port)(uint8 lchip, uint16 logic_dest_port);
    int32 (*ctc_mac_security_set_fid_learn_limit_action)(uint8 lchip, uint16 fid, ctc_maclimit_action_t action);
    int32 (*ctc_vlan_set_fid_mapping_vdev_vlan)(uint8 lchip, uint16 fid, uint16 vdev_vlan);
    int32 (*ctc_bpe_port_vdev_en)(uint8 lchip, uint32 gport, uint32 enable);
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

    uint32 pon_downlink_nhid;      /*PON servie nexthop*/
    uint32 ad_index;

    uint16 logic_dest_port;
    uint16 ingress_policer_id;
    uint16 egress_policer_id;
    uint16 vdev_id;

    uint8 pass_trough_en;
    uint8 mac_security;
    uint32 limit_num;
    uint32 limit_count;
    uint16 limit_action;
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
    uint32 calc_key_len[0];

    uint32 vlan_port_id;

    ctc_app_vlan_action_set_t ingress_vlan_action_set;  /* Ingress vlan action */
    ctc_app_vlan_action_set_t egress_vlan_action_set;   /* Egress vlan action */

    uint32 ad_index;

    uint32 logic_port;
    uint32 xlate_nhid;
    uint16 pkt_svlan;
    uint16 pkt_cvlan;
    uint16 fid;
    uint8 egs_vlan_mapping_en;
    uint8 logic_port_b_en;
    uint8 igs_vlan_mapping_use_flex;

    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db;

    uint16 ingress_policer_id;
    uint16 egress_policer_id;
    uint16 vdev_id;
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

ctc_app_vlan_port_master_t* p_usw_vlan_port_master = NULL;

extern int32 sys_usw_global_set_xgpon_en(uint8 lchip, uint8 enable);
extern int32 sys_usw_nh_set_xgpon_en(uint8 lchip, uint8 enable);
extern int32 sys_usw_nh_update_logic_port(uint8 lchip, uint32 nhid, uint32 logic_port);
extern int32 sys_usw_qos_policer_id_check(uint8 lchip, uint16 policer_id, uint8 sys_policer_type, uint8* is_bwp);
extern int32 sys_usw_nh_set_mcast_bitmap_ptr(uint8 lchip, uint32 offset);
extern int32 sys_usw_l2_hw_sync_add_db(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index);
extern int32 sys_usw_l2_hw_sync_remove_db(uint8 lchip, ctc_l2_addr_t* l2_addr);
extern int32  sys_usw_l2_hw_sync_alloc_ad_idx(uint8 lchip, uint32* ad_index);
extern int32  sys_usw_l2_hw_sync_free_ad_idx(uint8 lchip, uint32 ad_index);
extern int32 sys_usw_ftm_alloc_table_offset_from_position(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset);
extern int32 sys_usw_l2_set_fid_mapping(uint8 lchip, uint16 fid, uint16 svlan);
extern int32 sys_usw_bpe_enable_gem_port(uint8 lchip, uint32 gport, uint32 enable);
extern int32 sys_usw_bpe_add_gem_port(uint8 lchip, void* p_gem_port);
extern int32 sys_usw_bpe_remove_gem_port(uint8 lchip, void* p_gem_port);
extern int32 sys_usw_nh_add_gem_port_l2edit_8w_outer(uint8 lchip, uint16 logic_dest_port, uint16 gem_vlan);
extern int32 sys_usw_nh_remove_gem_port_l2edit_8w_outer(uint8 lchip, uint16 logic_dest_port);
extern int32 sys_usw_mac_security_set_fid_learn_limit_action(uint8 lchip, uint16 fid, ctc_maclimit_action_t action);
extern int32 sys_usw_vlan_set_fid_mapping_vdev_vlan(uint8 lchip,  uint16 fid, uint16 vdev_vlan);
extern int32 sys_usw_bpe_port_vdev_en(uint8 lchip, uint32 gport, uint32 enable);
extern int32 _ctc_usw_app_vlan_port_traverse_fid(uint8 limit_action, uint8 vdev_id);

STATIC int32
_ctc_app_vlan_port_check(uint8 lchip, ctc_app_vlan_port_type_t vlan_port_type, uint32 gport, uint32 vlan_port_id)
{
    uint8 gchip_id = 0;
    uint8 gchip_id_invalid = 0;
    uint8 is_linkagg_port = CTC_IS_LINKAGG_PORT(gport);
    switch(vlan_port_type)
    {
        case CTC_APP_VLAN_PORT_TYPE_NNI_PORT:
            CTC_GLOBAL_PORT_CHECK(gport);
            CTC_ERROR_RETURN(ctc_get_gchip_id(lchip, &gchip_id));
            gchip_id_invalid = (gchip_id != CTC_MAP_GPORT_TO_GCHIP(gport)) && !is_linkagg_port;
            if (gchip_id_invalid)
            {
                CTC_APP_DBG_ERROR("Invalid gchip id!n");
                return CTC_E_INVALID_GLOBAL_PORT;
            }
            break;
        case CTC_APP_VLAN_PORT_TYPE_VLAN_PORT:
            if (vlan_port_id)
            {
                break;
            }
        case CTC_APP_VLAN_PORT_TYPE_UNI_PORT:
        case CTC_APP_VLAN_PORT_TYPE_GEM_PORT:
            CTC_GLOBAL_PORT_CHECK(gport);
            CTC_ERROR_RETURN(ctc_get_gchip_id(lchip, &gchip_id));
            gchip_id_invalid = (gchip_id != CTC_MAP_GPORT_TO_GCHIP(gport)) && !is_linkagg_port;
            if (is_linkagg_port || gchip_id_invalid)
            {
                CTC_APP_DBG_ERROR("Linkagg port is not support or invalid gchip id!n");
                return CTC_E_INVALID_GLOBAL_PORT;
            }
            break;
        default :
            CTC_APP_DBG_DUMP("Error use app vlan port check function!\n");
            return CTC_E_NONE;
    }

    return CTC_E_NONE;
}
int32
_ctc_usw_app_vlan_port_alloc_offset(uint8 lchip, uint32* p_offset)
{
    int32 ret = 0;

    ret = CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_alloc_ad_idx, lchip, p_offset);
    CTC_APP_DBG_INFO("Alloc ad index :%u\n", *p_offset);

    return  ret;
}

int32
_ctc_usw_app_vlan_port_free_offset(uint8 lchip, uint32 offset)
{
    int32 ret = 0;

    ret = CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_free_ad_idx, lchip, offset);
    CTC_APP_DBG_INFO("Free ad index :%u\n", offset);

    return  ret;
}

int32
_ctc_usw_app_vlan_port_alloc_nhid(uint8 lchip, uint32* p_nhid)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    ctc_get_gchip_id(lchip, &gchip_id);
    sal_memset(&app_index, 0, sizeof(app_index));
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    app_index.entry_num = 1;


    CTC_ERROR_RETURN(ctc_app_index_alloc(&app_index));

    *p_nhid = app_index.index;

    return 0;
}

int32
_ctc_usw_app_vlan_port_free_nhid(uint8 lchip, uint32 nhid)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    ctc_get_gchip_id(lchip, &gchip_id);
    sal_memset(&app_index, 0, sizeof(app_index));
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    app_index.entry_num = 1;
    app_index.index = nhid;

    ctc_app_index_free(&app_index);

    return 0;
}

int32
_ctc_usw_app_vlan_port_alloc_mcast_group_id(uint8 lchip, uint32  num, uint32* p_mcast_id)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    ctc_get_gchip_id(lchip, &gchip_id);
    sal_memset(&app_index, 0, sizeof(app_index));
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_MCAST_GROUP_ID;
    app_index.entry_num = num;


    CTC_ERROR_RETURN(ctc_app_index_alloc(&app_index));

    *p_mcast_id = app_index.index;

    return 0;
}

int32
_ctc_usw_app_vlan_port_free_mcast_group_id(uint8 lchip, uint32 num, uint32 mcast_id)
{
    ctc_app_index_t app_index;
    uint8 gchip_id = 0;

    ctc_get_gchip_id(lchip, &gchip_id);
    sal_memset(&app_index, 0, sizeof(app_index));
    app_index.gchip = gchip_id;
    app_index.index_type = CTC_APP_INDEX_TYPE_MCAST_GROUP_ID;
    app_index.entry_num = num;
    app_index.index = mcast_id;

    ctc_app_index_free(&app_index);

    return 0;
}

int32
_ctc_usw_app_vlan_port_match_gem_port(ctc_app_vlan_port_gem_port_db_t* p_gem_port_db,
                                   ctc_app_vlan_port_find_t* p_vlan_port_match)
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
_ctc_usw_app_vlan_port_find_gem_port_db(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_usw_vlan_port_master->gem_port_hash,
                      (hash_traversal_fn)_ctc_usw_app_vlan_port_match_gem_port, &vlan_port_find);

    return vlan_port_find.p_gem_port_db;
}

int32
_ctc_usw_app_vlan_port_match_nni_port(ctc_app_vlan_port_nni_port_db_t* p_nni_port_db,
                                   ctc_app_vlan_port_find_t* p_vlan_port_match)
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
_ctc_usw_app_vlan_port_find_nni_port(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_usw_vlan_port_master->nni_port_hash,
                      (hash_traversal_fn)_ctc_usw_app_vlan_port_match_nni_port, &vlan_port_find);

    return vlan_port_find.p_nni_port_db;
}

int32
_ctc_usw_app_vlan_port_match_vlan_port(ctc_app_vlan_port_db_t* p_vlan_port_db,
                                   ctc_app_vlan_port_find_t* p_vlan_port_match)
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
_ctc_usw_app_vlan_port_find_vlan_port_db(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_find_t vlan_port_find;

    sal_memset(&vlan_port_find, 0, sizeof(vlan_port_find));

    vlan_port_find.logic_port = logic_port;
    ctc_hash_traverse(p_usw_vlan_port_master->vlan_port_hash,
                      (hash_traversal_fn)_ctc_usw_app_vlan_port_match_vlan_port, &vlan_port_find);

    return vlan_port_find.p_vlan_port_db;
}

STATIC uint32
_ctc_usw_app_vlan_port_hash_nni_port_make(ctc_app_vlan_port_nni_port_db_t* p_nni_port_db)
{
    uint32 data;
    uint32 length = 0;

    data = p_nni_port_db->port;
    length = sizeof(uint32);

    return ctc_hash_caculate(length, (uint8*)&data);
}

STATIC bool
_ctc_usw_app_vlan_port_hash_nni_port_cmp(ctc_app_vlan_port_nni_port_db_t* p_data0,
                                     ctc_app_vlan_port_nni_port_db_t* p_data1)
{
    if (p_data0->port == p_data1->port)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_usw_app_vlan_port_hash_gem_port_make(ctc_app_vlan_port_gem_port_db_t* p_gem_port_db)
{
    uint32 data[2] = {0};
    uint32 length = 0;

    data[0] = p_gem_port_db->port;
    data[1] = p_gem_port_db->tunnel_value;
    length = sizeof(uint32)*2;

    return ctc_hash_caculate(length, (uint8*)data);
}

STATIC bool
_ctc_usw_app_vlan_port_hash_gem_port_cmp(ctc_app_vlan_port_gem_port_db_t* p_data0,
                                     ctc_app_vlan_port_gem_port_db_t* p_data1)
{
    if (p_data0->port == p_data1->port &&
        p_data0->tunnel_value == p_data1->tunnel_value)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_usw_app_vlan_port_hash_make(ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    uint32 data;
    uint32 length = 0;

    data = p_vlan_port_db->vlan_port_id;

    length = sizeof(uint32);

    return ctc_hash_caculate(length, (uint8*)&data);
}

STATIC bool
_ctc_usw_app_vlan_port_hash_cmp(ctc_app_vlan_port_db_t* p_data0,
                            ctc_app_vlan_port_db_t* p_data1)
{
    if (p_data0->vlan_port_id == p_data1->vlan_port_id)
    {
        return TRUE;
    }

    return FALSE;
}

STATIC uint32
_ctc_usw_app_vlan_port_key_hash_make(ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    uint32 length = 0;

    length = CTC_OFFSET_OF(ctc_app_vlan_port_db_t, calc_key_len);

    return ctc_hash_caculate(length, (uint8*)p_vlan_port_db);
}

STATIC bool
_ctc_usw_app_vlan_port_key_hash_cmp(ctc_app_vlan_port_db_t *p_data0,
                            ctc_app_vlan_port_db_t* p_data1)
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
_ctc_usw_app_vlan_port_fid_hash_make(ctc_app_vlan_port_fid_db_t* p_port_fid)
{
    uint32 length = 0;
    uint16 data[3] = {0};


    if (CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[p_port_fid->pkt_svlan/32], p_port_fid->pkt_svlan%32))
    {
        data[0] = p_port_fid->pkt_svlan;
        data[1] = 0;
        data[2] = 0;
    }
    else
    {
        data[0] = p_port_fid->pkt_svlan;
        data[1] = p_port_fid->pkt_cvlan;
        data[2] = p_port_fid->vdev_id;
    }

    length = sizeof(data);

    return ctc_hash_caculate(length, (uint8*)data);
}

STATIC bool
_ctc_usw_app_vlan_port_fid_hash_cmp(ctc_app_vlan_port_fid_db_t* p_data0,
                                       ctc_app_vlan_port_fid_db_t* p_data1)
{
    if (((p_data0->pkt_svlan == p_data1->pkt_svlan)
        && (p_data0->pkt_cvlan == p_data1->pkt_cvlan)
        && (p_data0->vdev_id == p_data1->vdev_id))
        || (CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[p_data0->pkt_svlan/32], p_data0->pkt_svlan%32)
        && (p_data0->pkt_svlan == p_data1->pkt_svlan)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_ctc_usw_app_vlan_port_add_downlink_scl_entry(uint8 lchip, uint16 vdev_id, uint16 pkt_svlan, uint16 pkt_cvlan, uint16 fid, uint8 vlan_range)
{
    ctc_scl_entry_t scl_entry;
    uint32 gid = 0;
    int32 ret = CTC_E_NONE;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = vlan_range?ENCODE_SCL_DOWNLINK_VLAN_RANGE_ENTRY_ID(fid, pkt_cvlan):ENCODE_SCL_DOWNLINK_ENTRY_ID(fid, vdev_id);
    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
    scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_LOGIC_PORT;
    scl_entry.key.u.hash_port_2vlan_key.gport = p_usw_vlan_port_master->nni_logic_port[vdev_id];
    scl_entry.key.u.hash_port_2vlan_key.svlan = pkt_svlan;
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    scl_entry.action.u.igs_action.fid = fid;

    scl_entry.key.u.hash_port_2vlan_key.cvlan = pkt_cvlan;
    gid = pkt_cvlan? CTC_SCL_GROUP_ID_HASH_PORT_2VLAN: (CTC_APP_VLAN_PORT_SCL_GROUP_ID+vdev_id);
    ret = ctc_scl_add_entry(gid, &scl_entry);
    if (CTC_E_HASH_CONFLICT == ret)
    {
        CTC_APP_DBG_INFO("downlink double vlan pkt_svlan %u pkt_cvlan %u entry_id %u confilct\n", pkt_svlan, pkt_cvlan, scl_entry.entry_id);
        scl_entry.resolve_conflict = 1;
        CTC_ERROR_RETURN(ctc_scl_add_entry(gid, &scl_entry));
    }
    else if (ret != CTC_E_NONE)
    {
        return ret;
    }
    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_0);
    CTC_APP_DBG_INFO("downlink double vlan pkt_svlan %u pkt_cvlan %u entry_id %u\n", pkt_svlan, pkt_cvlan, scl_entry.entry_id);

    return CTC_E_NONE;

roll_back_0:
    ctc_scl_remove_entry(scl_entry.entry_id);

    return ret;
}

STATIC int32
_ctc_usw_app_vlan_port_remove_downlink_scl_entry(uint8 lchip, uint16 vdev_id, uint16 pkt_cvlan, uint16 fid, uint8 vlan_range)
{
    uint32 entry_id = 0;

    entry_id = vlan_range?ENCODE_SCL_DOWNLINK_VLAN_RANGE_ENTRY_ID(fid, pkt_cvlan):ENCODE_SCL_DOWNLINK_ENTRY_ID(fid, vdev_id);
    ctc_scl_uninstall_entry(entry_id);
    ctc_scl_remove_entry(entry_id);

    return CTC_E_NONE;
}
STATIC bool
_ctc_usw_app_vlan_port_fid_build_index(ctc_app_vlan_port_fid_db_t* p_port_fid_db, uint8* p_lchip)
{
    uint8 lchip = 0;
    uint16 index = 0;
    uint16 vdev_vlan = 0;
    uint16 vdev_id = 0;
    uint16 vdev_max = 0;
    uint16 vlan_glb_vdev_en = 1;
    uint32 fid = 0;
    uint32 mcast_id = 0;
    uint32 nh_profile_id = 0;
    uint32 nh_id = 0;
    uint32 nh_mcast_id = 0;
    ctc_opf_t opf;
    ctc_l2dflt_addr_t l2dflt;
    ctc_mcast_nh_param_group_t mcast_group;
    int32 ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(p_port_fid_db);

    lchip = *p_lchip;
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;

    CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, &fid));
    p_port_fid_db->fid = fid;
    p_port_fid_db->limit_num = CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE;
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_mcast_group_id(lchip, 2, &mcast_id), ret, roll_back_0);
    p_port_fid_db->mcast_id = mcast_id;
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    CTC_ERROR_GOTO(ctc_l2_add_default_entry(&l2dflt), ret, roll_back_1);

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_mcast_id), ret, roll_back_2);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 1);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_mcast_id, &mcast_group), ret, roll_back_3);
    ctc_l2_set_fid_property(fid, CTC_L2_FID_PROP_BDING_MCAST_GROUP, ENCODE_MCAST_GROUP_ID(p_port_fid_db->mcast_id, 0));

    vlan_glb_vdev_en = CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[p_port_fid_db->pkt_svlan/32], p_port_fid_db->pkt_svlan%32);
    vdev_vlan = p_usw_vlan_port_master->vdev_base_vlan + (vlan_glb_vdev_en? p_usw_vlan_port_master->vdev_num: p_port_fid_db->vdev_id);
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_vlan_set_fid_mapping_vdev_vlan, lchip, fid, vdev_vlan);

    if (vlan_glb_vdev_en)
    {
        vdev_id = 0;
        vdev_max = p_usw_vlan_port_master->vdev_num;
    }
    else
    {
        vdev_id = p_port_fid_db->vdev_id;
        vdev_max = vdev_id+1;
    }

    /*Add MC member*/
    nh_profile_id = vlan_glb_vdev_en? p_usw_vlan_port_master->glb_unknown_mcast_nhid:p_usw_vlan_port_master->default_unknown_mcast_nhid[p_port_fid_db->vdev_id];
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 1);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_profile_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    CTC_ERROR_GOTO(ctc_nh_update_mcast(nh_mcast_id, &mcast_group), ret, roll_back_4);

    /*Add BC member*/
    nh_profile_id = vlan_glb_vdev_en? p_usw_vlan_port_master->glb_unknown_bcast_nhid:p_usw_vlan_port_master->default_unknown_bcast_nhid[p_port_fid_db->vdev_id];
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_profile_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_get_mcast_nh(ENCODE_MCAST_GROUP_ID(mcast_id, 0), &nh_id);
    CTC_ERROR_GOTO(ctc_nh_update_mcast(nh_id, &mcast_group), ret, roll_back_4);

    for (index = vdev_id; index<vdev_max; index++)
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_add_downlink_scl_entry(lchip, index, p_port_fid_db->pkt_svlan, p_port_fid_db->pkt_cvlan, fid, 0), ret, roll_back_5);
    }

    return CTC_E_NONE;
roll_back_5:
    for (; index>0; index--)
    {
        _ctc_usw_app_vlan_port_remove_downlink_scl_entry(lchip, index, p_port_fid_db->pkt_cvlan, fid, 0);
    }
    _ctc_usw_app_vlan_port_remove_downlink_scl_entry(lchip, 0, p_port_fid_db->pkt_cvlan, fid, 0);
roll_back_4:
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_vlan_set_fid_mapping_vdev_vlan, lchip, fid, 0);
    ctc_l2_set_fid_property(fid, CTC_L2_FID_PROP_BDING_MCAST_GROUP, 0);
    CTC_APP_DBG_ERROR("mcast nexthop id error!!\n");
    ctc_nh_remove_mcast(nh_mcast_id);
roll_back_3:
    CTC_APP_DBG_ERROR("free nhid !!\n");
    _ctc_usw_app_vlan_port_free_nhid(lchip, nh_mcast_id);
roll_back_2:
    CTC_APP_DBG_ERROR("remove default entry !!\n");
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    ctc_l2_remove_default_entry(&l2dflt);

roll_back_1:
    CTC_APP_DBG_ERROR("The mcast group error!\n");
    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 2, mcast_id);

roll_back_0:
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;

    ctc_opf_free_offset(&opf, 1, fid);

    return ret;
}

/* free index of HASH ad */
STATIC bool
_ctc_usw_app_vlan_port_fid_free_index(ctc_app_vlan_port_fid_db_t* p_port_fid_db, uint8* p_lchip)
{
    uint8 lchip = 0;
    uint16 index = 0;
    uint16 vdev_id = 0;
    uint16 vdev_max = 0;
    uint16 vlan_glb_vdev_en = 1;
    uint32 nh_id = 0;
    ctc_opf_t opf;
    ctc_l2dflt_addr_t l2dflt;
    ctc_mcast_nh_param_group_t mcast_group;

    CTC_PTR_VALID_CHECK(p_port_fid_db);

    lchip = *p_lchip;
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = p_port_fid_db->fid;
    l2dflt.l2mc_grp_id = ENCODE_MCAST_GROUP_ID(p_port_fid_db->mcast_id, 0);
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    ctc_l2_remove_default_entry(&l2dflt);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;
    ctc_opf_free_offset(&opf, 1, p_port_fid_db->fid);

    ctc_nh_get_mcast_nh(ENCODE_MCAST_GROUP_ID(p_port_fid_db->mcast_id, 1), &nh_id);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_port_fid_db->mcast_id, 1);
    ctc_nh_remove_mcast(nh_id);
    _ctc_usw_app_vlan_port_free_nhid(lchip, nh_id);

    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 2, p_port_fid_db->mcast_id);

    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_vlan_set_fid_mapping_vdev_vlan, lchip, p_port_fid_db->fid, 0);

    vlan_glb_vdev_en = CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[p_port_fid_db->pkt_svlan/32], p_port_fid_db->pkt_svlan%32);

    if (vlan_glb_vdev_en)
    {
        vdev_id = 0;
        vdev_max = p_usw_vlan_port_master->vdev_num;
    }
    else
    {
        vdev_id = p_port_fid_db->vdev_id;
        vdev_max = vdev_id+1;
    }

    for (index = vdev_id; index<vdev_max; index++)
    {
        _ctc_usw_app_vlan_port_remove_downlink_scl_entry(lchip, index, p_port_fid_db->pkt_cvlan, p_port_fid_db->fid, 0);
    }

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_get_by_logic_port(uint8 lchip, uint32 logic_port, ctc_app_vlan_port_t* p_vlan_port)
{
    ctc_app_vlan_port_db_t* p_vlan_port_find = NULL;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_find = NULL;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_find = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();

    p_vlan_port_find = _ctc_usw_app_vlan_port_find_vlan_port_db(lchip, logic_port);

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

        return CTC_E_NONE;
    }

    p_gem_port_find = _ctc_usw_app_vlan_port_find_gem_port_db(lchip, logic_port);
    if (p_gem_port_find)
    {
        p_vlan_port->port = p_gem_port_find->port;
        p_vlan_port->match_tunnel_value = p_gem_port_find->tunnel_value;
        p_vlan_port->logic_port = p_gem_port_find->logic_port;
        return CTC_E_NONE;
    }

    p_nni_port_find = _ctc_usw_app_vlan_port_find_nni_port(lchip, logic_port);
    if (p_nni_port_find)
    {
        p_vlan_port->port = p_nni_port_find->port;
        return CTC_E_NONE;
    }

    return CTC_E_NOT_EXIST;
}

STATIC int32
_ctc_usw_app_vlan_port_compare_fid(void* node, void* user_data)
{
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    ctc_app_vlan_port_fid_db_t* p_port_fid_db_data = (ctc_app_vlan_port_fid_db_t*)user_data;

    if (p_port_fid_db->fid == p_port_fid_db_data->fid)
    {
        p_port_fid_db_data->vdev_id = p_port_fid_db->vdev_id;
        p_port_fid_db_data->pkt_svlan = p_port_fid_db->pkt_svlan;
        p_port_fid_db_data->pkt_cvlan = p_port_fid_db->pkt_cvlan;
        p_port_fid_db_data->is_pass_through = p_port_fid_db->is_pass_through;
        p_port_fid_db_data->limit_num = p_port_fid_db->limit_num;
        p_port_fid_db_data->limit_count = p_port_fid_db->limit_count;
        p_port_fid_db_data->limit_action = p_port_fid_db->limit_action;
        p_port_fid_db_data->mcast_id = p_port_fid_db->mcast_id;
        return -1;
    }
    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_get_fid_by_fid(ctc_app_vlan_port_fid_db_t* p_port_fid_db)
{
    CTC_ERROR_RETURN(ctc_spool_traverse(p_usw_vlan_port_master->fid_spool,
                          (spool_traversal_fn)_ctc_usw_app_vlan_port_compare_fid, p_port_fid_db));

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_get_fid_mapping_info(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
{
    int ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_temp = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_port_fid);
    CTC_APP_VLAN_PORT_INIT_CHECK();
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
    if (port_fid_db.fid)
    {
        ret = _ctc_usw_app_vlan_port_get_fid_by_fid(&port_fid_db);
        if (!ret)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }
        p_port_fid->pkt_svlan = port_fid_db.pkt_svlan;
        p_port_fid->pkt_cvlan = port_fid_db.pkt_cvlan;
        p_port_fid->vdev_id = port_fid_db.vdev_id;
    }
    else
    {
        p_port_fid_temp = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);

        if (NULL == p_port_fid_temp)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NOT_EXIST;
        }
        p_port_fid->fid = p_port_fid_temp->fid;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_INFO("Fid: %u!\n", p_port_fid->fid);

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_alloc_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
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
    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_port_fid->vdev_id])
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

    p_port_fid_temp = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);
    if (p_port_fid_temp)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }

    ret = ctc_spool_add(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL, &p_port_fid_temp);
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
ctc_usw_app_vlan_port_free_fid(uint8 lchip, ctc_app_vlan_port_fid_t* p_port_fid)
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

    p_port_fid_temp = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);
    if (NULL == p_port_fid_temp)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    ret = ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);
    if (ret)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return ret;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_INFO("Fid: %u!\n", p_port_fid->fid);

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_get_uni_port(uint8 lchip, uint32 port, ctc_app_vlan_port_uni_db_t** pp_uni_port)
{
    uint16 lport = 0;

    lport = CTC_MAP_GPORT_TO_LPORT(port);
    *pp_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    return CTC_E_NONE;
}

#define _______DUMP________

STATIC int32
_ctc_usw_app_vlan_port_show_gem_port_all(ctc_app_vlan_port_gem_port_db_t* p_gem_port_db,
                                 uint32* i)
{
    CTC_PTR_VALID_CHECK(i);
    CTC_PTR_VALID_CHECK(p_gem_port_db);

    CTC_APP_DBG_DUMP("%-5u %5u %15u %15u %15u\n",
               *i,
               p_gem_port_db->port,
               p_gem_port_db->tunnel_value,
               p_gem_port_db->logic_port,
               p_gem_port_db->ref_cnt);
    (*i)++;

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_show_vlan_port_all(ctc_app_vlan_port_db_t* p_vlan_port_db,
                                  uint32* i)
{
   CTC_PTR_VALID_CHECK(i);
   CTC_PTR_VALID_CHECK(p_vlan_port_db);


   CTC_APP_DBG_DUMP("%-5u %5u %5u %15u %10u %10u %10u %15u\n",
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
_ctc_usw_app_vlan_port_show_uni_port(uint8 lchip, uint8 detail, uint32 port)
{
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_UNI_PORT, port, 0));
    lport = CTC_MAP_GPORT_TO_LPORT(port);

    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];
    if (0 == p_uni_port->isolated_id)
    {
        return CTC_E_NOT_EXIST;
    }

    CTC_APP_DBG_DUMP("port: 0x%x detail:\n", port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "uni index"        , p_uni_port->isolated_id);
    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "ref_cnt"          , p_uni_port->ref_cnt);
    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "mc_xlate_nhid"    , p_uni_port->mc_xlate_nhid);

    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "vlan_range_grp"   , p_uni_port->vlan_range_grp);

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_show_nni_port(uint8 lchip, uint32 port)
{
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;

    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_NNI_PORT, port, 0));

    /* DB */
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = port;
    p_nni_port_db = ctc_hash_lookup(p_usw_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        return CTC_E_NOT_EXIST;
    }

    CTC_APP_DBG_DUMP("port: 0x%x detail:\n", port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "vDev ID"      , p_nni_port_db->vdev_id);
    CTC_APP_DBG_DUMP("%-40s : %10u\n" , "Logic Port"   , p_nni_port_db->nni_logic_port);
    CTC_APP_DBG_DUMP("%-40s : %10s\n" , "Rx Enable"       , p_nni_port_db->rx_en?"Enable":"Disable");

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_show_gem_port(uint8 lchip, uint8 detail, uint32 port, uint32 tunnel_value)
{

    if (0 == detail)
    {
        uint32 i = 1;
        CTC_APP_DBG_DUMP("%-5s %5s %15s %15s %15s\n", "No.", "port", "tunnel-value", "logic port", "ref cnt");
        CTC_APP_DBG_DUMP("-----------------------------------------------------------------\n");

        ctc_hash_traverse(p_usw_vlan_port_master->gem_port_hash,
                          (hash_traversal_fn)_ctc_usw_app_vlan_port_show_gem_port_all, &i);
    }
    else
    {
        ctc_app_vlan_port_gem_port_db_t gem_port_db;
        ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;

        /* DB */
        sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
        gem_port_db.port = port;
        gem_port_db.tunnel_value = tunnel_value;
        p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
        if (NULL == p_gem_port_db)
        {
            return CTC_E_NOT_EXIST;
        }

        CTC_APP_DBG_DUMP("port: 0x%x  tunnel-value:%u detail:\n", port, tunnel_value);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "ref_cnt"      , p_gem_port_db->ref_cnt);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "flag"         , p_gem_port_db->flag);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "logic_port"   , p_gem_port_db->logic_port);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "pon downlink nhid"  , p_gem_port_db->pon_downlink_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "ingress_policer_id" , p_gem_port_db->ingress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "egress_policer_id"  , p_gem_port_db->egress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "logic_dest_port"    , p_gem_port_db->logic_dest_port);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "pass through"       , p_gem_port_db->pass_trough_en);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "mac security"       , p_gem_port_db->mac_security);
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_show_vlan_port(uint8 lchip, uint8 detail, uint32 vlan_port_id)
{

    if (0 == detail)
    {
        uint32 i = 1;
        CTC_APP_DBG_DUMP("%-5s %5s %5s %15s %10s %10s %10s %15s\n", "No.", "id", "port", "tunnel-value", "svid", "cvid", "svid-end", "logic port");
        CTC_APP_DBG_DUMP("----------------------------------------------------------------------------------------------------------------------\n");

        ctc_hash_traverse(p_usw_vlan_port_master->vlan_port_hash,
                          (hash_traversal_fn)_ctc_usw_app_vlan_port_show_vlan_port_all, &i);
    }
    else
    {
        ctc_app_vlan_port_db_t vlan_port_db;
        ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
        sal_memset(&vlan_port_db, 0, sizeof(vlan_port_db));
        vlan_port_db.vlan_port_id = vlan_port_id;

        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_hash, &vlan_port_db);

        if (NULL == p_vlan_port_db)
        {
            return CTC_E_NOT_EXIST;
        }

        CTC_APP_DBG_DUMP("vlan-port-id: %u detail:\n", vlan_port_id);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "criteria"            , p_vlan_port_db->criteria);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "port"                , p_vlan_port_db->port);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "tunnel_value"        , p_vlan_port_db->tunnel_value);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "match_svlan"         , p_vlan_port_db->match_svlan);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "match_cvlan"         , p_vlan_port_db->match_cvlan);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "match_svlan_end"     , p_vlan_port_db->match_svlan_end);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "match_cvlan_end"     , p_vlan_port_db->match_cvlan_end);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "logic_port"          , p_vlan_port_db->logic_port);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "xlate_nhid"          , p_vlan_port_db->xlate_nhid);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "egs_vlan_mapping_en" , p_vlan_port_db->egs_vlan_mapping_en);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "fid"                 , p_vlan_port_db->fid);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "pkt-svlan"           , p_vlan_port_db->pkt_svlan);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "pkt-cvlan"           , p_vlan_port_db->pkt_cvlan);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "xlate_nhid"          , p_vlan_port_db->xlate_nhid);

        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "ingress_policer_id" , p_vlan_port_db->ingress_policer_id);
        CTC_APP_DBG_DUMP("%-40s : %10u\n" , "egress_policer_id"  , p_vlan_port_db->egress_policer_id);
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_show_status(uint8 lchip)
{
    uint16 vdev_index = 0;

    CTC_APP_DBG_DUMP("%-40s : %u\n", "Mcast tunnel Value logic port",  p_usw_vlan_port_master->bcast_value_logic_port);
    for (vdev_index=0; vdev_index<p_usw_vlan_port_master->vdev_num; vdev_index++)
    {
        CTC_APP_DBG_DUMP("vDev ID:%u\n", vdev_index);
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %u\n", "gem port",    p_usw_vlan_port_master->gem_port_cnt[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %u\n", "vlan port",   p_usw_vlan_port_master->vlan_port_cnt[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %u\n", "nni port",    p_usw_vlan_port_master->nni_port_cnt[vdev_index]);

        CTC_APP_DBG_DUMP("\n");
        CTC_APP_DBG_DUMP("Resource:\n");
        CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
        CTC_APP_DBG_DUMP("%-40s : %u\n", "default bcast fid",           p_usw_vlan_port_master->default_bcast_fid[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %u\n", "nni mcast nhid",              p_usw_vlan_port_master->nni_mcast_nhid[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %u\n", "default mcast group id",      p_usw_vlan_port_master->default_mcast_group_id[vdev_index]);
        CTC_APP_DBG_DUMP("%-40s : %u\n", "default unknown mcast nhid",  p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index]);
    }

    return CTC_E_NONE;
}

#define CTC_APP_VLAN_PORT_DUMP(param) CTC_APP_DBG_DUMP("%-40s :%10u\n", #param, vlan_port.param)
int32
ctc_usw_app_vlan_port_show_logic_port(uint8 lchip, uint32 logic_port)
{
    ctc_app_vlan_port_t vlan_port;

    CTC_APP_VLAN_PORT_INIT_CHECK();

    sal_memset(&vlan_port, 0, sizeof(vlan_port));

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_usw_app_vlan_port_get_by_logic_port(lchip, logic_port, &vlan_port));
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    CTC_APP_DBG_DUMP("logic port: %u detail:\n", logic_port);
    CTC_APP_DBG_DUMP("---------------------------------------------------------------------------------------------\n");
    CTC_APP_VLAN_PORT_DUMP(vlan_port_id);
    CTC_APP_VLAN_PORT_DUMP(criteria);
    CTC_APP_DBG_DUMP("%-40s :%10u\n", "port", vlan_port.port);
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
ctc_usw_app_vlan_port_show(uint8 lchip, uint8 type, uint8 detail, uint32 param0, uint32 param1)
{
    int32 ret = CTC_E_NONE;

    CTC_APP_VLAN_PORT_INIT_CHECK();

    CTC_APP_VLAN_PORT_LOCK(lchip);
    switch(type)
    {
        case 0:
            ret = _ctc_usw_app_vlan_port_show_status(lchip);
            break;
        case 1:
            ret = _ctc_usw_app_vlan_port_show_uni_port(lchip, detail, param0);
            break;
        case 2:
            ret = _ctc_usw_app_vlan_port_show_gem_port(lchip, detail, param0, param1);
            break;
        case 3:
            ret = _ctc_usw_app_vlan_port_show_vlan_port(lchip, detail, param0);
            break;
        case 4:
            ret = _ctc_usw_app_vlan_port_show_nni_port(lchip, param0);
            break;

        default:
           break;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

STATIC int32
_ctc_usw_app_vlan_port_show_fid(void* node, void* user_data)
{
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    ctc_app_vlan_port_fid_db_t* p_port_fid_db_temp = (ctc_app_vlan_port_fid_db_t*)user_data;

    if (p_port_fid_db_temp->vdev_id == p_port_fid_db->vdev_id)
    {
        CTC_APP_DBG_DUMP("%-10u %10u %10u %10u %10u\n",
              p_port_fid_db_temp->fid,
              p_port_fid_db->vdev_id,
              p_port_fid_db->pkt_svlan,
              p_port_fid_db->pkt_cvlan,
              p_port_fid_db->fid
             );
        p_port_fid_db_temp->fid++;
    }
    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_show_fid(uint8 lchip, uint16 vdev_id)
{
    ctc_app_vlan_port_fid_db_t port_fid_db;
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(vdev_id);

    sal_memset(&port_fid_db, 0, sizeof(port_fid_db));
    port_fid_db.vdev_id = vdev_id;
    CTC_APP_VLAN_PORT_LOCK(lchip);

    CTC_APP_DBG_DUMP("%-10s %10s %10s %10s %10s\n", "index", "vDev id", "Packet svlan", "Packet cvlan", "fid");
    CTC_APP_DBG_DUMP("---------------------------------------------------------------\n");

    ctc_spool_traverse(p_usw_vlan_port_master->fid_spool,
                      (spool_traversal_fn)_ctc_usw_app_vlan_port_show_fid, &port_fid_db);

    CTC_APP_DBG_DUMP("VLan mapping fid db: %u\n", port_fid_db.fid);
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _____GLOBAL_PROPERTY_____ ""
#if 0
int32
_ctc_usw_app_vlan_port_set_xc_port_discard(uint8 lchip, uint32 src_gport, uint32 dst_gport, uint8 discard)
{
    int32 ret = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_scl_entry_t  scl_entry;

    if (discard)
    {
        sal_memset(&scl_entry, 0, sizeof(scl_entry));
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

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_EGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_XC;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(dst_gport, &scl_prop), ret, roll_back_2);
    }
    else
    {
        sal_memset(&scl_entry, 0, sizeof(scl_entry));
        scl_entry.entry_id = ENCODE_SCL_XC_ENTRY_ID(src_gport*100 + dst_gport);
        CTC_APP_DBG_INFO("EntryId :%u\n", scl_entry.entry_id );
        CTC_ERROR_GOTO(ctc_scl_uninstall_entry(scl_entry.entry_id), ret, roll_back_0);
        CTC_ERROR_GOTO(ctc_scl_remove_entry(scl_entry.entry_id), ret, roll_back_0);
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
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
#endif
int32
_ctc_usw_app_vlan_port_set_uni_inner_isolate(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    uint8 gchip = 0;
    uint32 lport = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    uint8 discard = 0;
    uint32 gport = 0;

    CTC_ERROR_RETURN(ctc_get_gchip_id(lchip, &gchip));

    discard = p_glb_cfg->value?1:0;

    if (p_usw_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id] == discard)
    {
        return CTC_E_NONE;
    }

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

        if (p_uni_port->vdev_id != p_glb_cfg->vdev_id)
        {
            continue;
        }
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
        CTC_APP_DBG_PARAM("%-40s :%10u\n", "gport", gport);
        CTC_ERROR_RETURN(ctc_port_set_reflective_bridge_en(gport, !discard));
    }

    p_usw_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id] = discard;

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_set_uni_outer_isolate(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    uint8 gchip = 0;
    uint32 lport = 0;
    ctc_port_restriction_t isolation;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    uint8 discard = 0;

    ctc_get_gchip_id(lchip, &gchip);

    discard = p_glb_cfg->value?1:0;

    if (p_usw_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id] == discard)
    {
        return CTC_E_NONE;
    }

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

        if ((0 == p_uni_port->isolated_id) || (p_uni_port->vdev_id != p_glb_cfg->vdev_id))
        {
            continue;
        }

        /***********************************************/
        /** PORT ISOLATION*/
        sal_memset(&isolation, 0, sizeof(isolation));
        isolation.dir = CTC_EGRESS;
        isolation.isolated_id = p_glb_cfg->value? p_uni_port->isolated_id: 0;
        isolation.mode = p_glb_cfg->value? CTC_PORT_RESTRICTION_PORT_ISOLATION: CTC_PORT_RESTRICTION_DISABLE;
        isolation.type = CTC_PORT_ISOLATION_ALL;
        CTC_ERROR_RETURN(ctc_port_set_restriction(CTC_MAP_LPORT_TO_GPORT(gchip, lport), &isolation));
        isolation.dir = CTC_INGRESS;
        CTC_ERROR_RETURN(ctc_port_set_restriction(CTC_MAP_LPORT_TO_GPORT(gchip, lport), &isolation));
    }

    p_usw_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id] = discard;

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_set_unknown_mcast_drop(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    uint8 drop_en  = 0;
    uint16 lport = 0;
    uint32 mcast_group_id = 0;
    uint32 mcast_group_nh_id = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_mcast_nh_param_group_t mcast_group;

    drop_en = p_glb_cfg->value?1:0;

    if (p_usw_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id] == drop_en)
    {
        return CTC_E_NONE;
    }

    for (lport = 0; lport < 128; lport++)
    {
        p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

        if ((0 == p_uni_port->isolated_id) || (p_uni_port->vdev_id != p_glb_cfg->vdev_id))
        {
            continue;
        }

        mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_glb_cfg->vdev_id], 3);
        mcast_group_nh_id = p_usw_vlan_port_master->default_unknown_mcast_nhid[p_glb_cfg->vdev_id];


        /*Add or remove MC member(drop_en: 0-add, others-remove)*/
        sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
        mcast_group.mc_grp_id = mcast_group_id;
        mcast_group.opcode = drop_en? CTC_NH_PARAM_MCAST_DEL_MEMBER: CTC_NH_PARAM_MCAST_ADD_MEMBER;
        mcast_group.mem_info.ref_nhid = p_uni_port->mc_xlate_nhid;
        mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        CTC_ERROR_RETURN(ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group));

        mcast_group_nh_id = p_usw_vlan_port_master->glb_unknown_mcast_nhid;
        ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);
    }

    p_usw_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id] = drop_en;

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_set_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
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
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_SYSTEM:
            learn_limit.limit_action = p_mac_learn->limit_action;
            learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
            learn_limit.limit_num = p_mac_learn->limit_num;
            CTC_ERROR_RETURN(ctc_mac_security_set_learn_limit(&learn_limit));
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV:
            if (p_mac_learn->limit_action == CTC_MACLIMIT_ACTION_TOCPU)
            {
                return CTC_E_INVALID_PARAM;
            }

            if ((p_mac_learn->limit_num == CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_usw_vlan_port_master->limit_num[p_glb_cfg->vdev_id] != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE))
            {
                _ctc_usw_app_vlan_port_traverse_fid(CTC_MACLIMIT_ACTION_NONE, p_glb_cfg->vdev_id);
            }
            else if ((p_mac_learn->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_usw_vlan_port_master->limit_num[p_glb_cfg->vdev_id] != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_usw_vlan_port_master->limit_action[p_glb_cfg->vdev_id] != p_mac_learn->limit_action))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (0 == p_mac_learn->limit_num)
            {
                _ctc_usw_app_vlan_port_traverse_fid(p_mac_learn->limit_action, p_glb_cfg->vdev_id);
            }

            p_usw_vlan_port_master->limit_num[p_glb_cfg->vdev_id] = p_mac_learn->limit_num;
            p_usw_vlan_port_master->limit_action[p_glb_cfg->vdev_id] = p_mac_learn->limit_action;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID:
            sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
            port_fid_db.fid = p_mac_learn->fid;
            ret = _ctc_usw_app_vlan_port_get_fid_by_fid(&port_fid_db);
            if (!ret)
            {
                return CTC_E_NOT_EXIST;
            }

            p_port_fid_db = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);
            if (NULL == p_port_fid_db)
            {
                return CTC_E_NOT_EXIST;
            }

            if ((p_mac_learn->limit_num == CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE))
            {
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, CTC_MACLIMIT_ACTION_NONE);
            }
            else if ((p_mac_learn->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_port_fid_db->limit_action != p_mac_learn->limit_action))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if (0 == p_mac_learn->limit_num)
            {
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, p_mac_learn->limit_action);
            }
            p_port_fid_db->limit_action = p_mac_learn->limit_action;
            p_port_fid_db->limit_num = p_mac_learn->limit_num;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT:
            sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
            gem_port_db.port = p_mac_learn->gport;
            gem_port_db.tunnel_value = p_mac_learn->tunnel_value;
            p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
            if (NULL == p_gem_port_db)
            {
                return CTC_E_NOT_EXIST;
            }

            if ((p_mac_learn->limit_num == CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE))
            {
                ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

                sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
                bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_db->port);
                bpe_gem_port.gem_vlan = p_gem_port_db->tunnel_value;
                bpe_gem_port.logic_port = p_gem_port_db->logic_port;
                bpe_gem_port.pass_through_en = p_gem_port_db->pass_trough_en;
                bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_NONE;
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
                p_gem_port_db->mac_security = CTC_MACLIMIT_ACTION_NONE;
            }
            else if ((p_mac_learn->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
                && (p_gem_port_db->limit_action != p_mac_learn->limit_action))
            {
                return CTC_E_NOT_SUPPORT;
            }

            if ((0 == p_mac_learn->limit_num) && (p_mac_learn->limit_action == CTC_MACLIMIT_ACTION_DISCARD))
            {
                ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

                sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
                bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_db->port);
                bpe_gem_port.gem_vlan = p_gem_port_db->tunnel_value;
                bpe_gem_port.logic_port = p_gem_port_db->logic_port;
                bpe_gem_port.pass_through_en = p_gem_port_db->pass_trough_en;
                bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_DISCARD;
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
                p_gem_port_db->mac_security = CTC_MACLIMIT_ACTION_DISCARD;
            }
            else if ((0 == p_mac_learn->limit_num) && (p_mac_learn->limit_action == CTC_MACLIMIT_ACTION_TOCPU))
            {
                ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

                sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
                bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_db->port);
                bpe_gem_port.gem_vlan = p_gem_port_db->tunnel_value;
                bpe_gem_port.logic_port = p_gem_port_db->logic_port;
                bpe_gem_port.pass_through_en = p_gem_port_db->pass_trough_en;
                bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_TOCPU;
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
                p_gem_port_db->mac_security = CTC_MACLIMIT_ACTION_TOCPU;
            }
            p_gem_port_db->limit_action = p_mac_learn->limit_action;
            p_gem_port_db->limit_num = p_mac_learn->limit_num;
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_fid_compare_svlan(void* node, void* user_data)
{
    ctc_app_vlan_port_fid_db_t* p_port_fid = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);
    uint16 svlan = *(uint16*)user_data;

    if (p_port_fid->pkt_svlan == svlan)
    {
        return CTC_E_EXIST;
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_traverse_fid_with_svlan(uint16 svlan)
{

    CTC_ERROR_RETURN(ctc_spool_traverse(p_usw_vlan_port_master->fid_spool,
                          (spool_traversal_fn)_ctc_usw_app_vlan_port_fid_compare_svlan, &svlan));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_set_vlan_glb_vdev_en(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    int32 ret = CTC_E_NONE;
    uint16 vlan_id = 0;

    CTC_VLAN_RANGE_CHECK(p_glb_cfg->vlan_id);
    vlan_id = p_glb_cfg->vlan_id;

    ret = _ctc_usw_app_vlan_port_traverse_fid_with_svlan(vlan_id);
    if (ret)
    {
        return CTC_E_EXIST;
    }

    if (p_glb_cfg->value)
    {
        CTC_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[vlan_id/32], vlan_id%32);
    }
    else
    {
        CTC_BIT_UNSET(p_usw_vlan_port_master->vlan_glb_vdev[vlan_id/32], vlan_id%32);
    }

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_set_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    CTC_PTR_VALID_CHECK(p_glb_cfg);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_glb_cfg->vdev_id);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    switch(p_glb_cfg->cfg_type)
    {
    case CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE:
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_set_uni_outer_isolate(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE:
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_set_uni_inner_isolate(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP:
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_set_unknown_mcast_drop(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT:
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_set_mac_learn_limit(lchip, p_glb_cfg));
        break;

    case CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN:
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_set_vlan_glb_vdev_en(lchip, p_glb_cfg));
        break;

    default:
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_INVALID_CONFIG;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_get_mac_learn_limit(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
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
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_SYSTEM:
            learn_limit.limit_action = p_mac_learn->limit_action;
            learn_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
            learn_limit.limit_num = p_mac_learn->limit_num;
            ctc_mac_security_get_learn_limit(&learn_limit);

            p_mac_learn->limit_count = learn_limit.limit_count;
            p_mac_learn->limit_num = learn_limit.limit_num;
            p_mac_learn->limit_action = learn_limit.limit_action;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV:
            p_mac_learn->limit_count = p_usw_vlan_port_master->limit_count[p_glb_cfg->vdev_id];
            p_mac_learn->limit_num = p_usw_vlan_port_master->limit_num[p_glb_cfg->vdev_id];
            p_mac_learn->limit_action = p_usw_vlan_port_master->limit_action[p_glb_cfg->vdev_id];
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID:
            sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
            port_fid_db.fid = p_mac_learn->fid;
            ret = _ctc_usw_app_vlan_port_get_fid_by_fid(&port_fid_db);
            if (!ret)
            {
                return CTC_E_NOT_EXIST;
            }

            p_port_fid_db = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);
            if (NULL == p_port_fid_db)
            {
                return CTC_E_NOT_EXIST;
            }
            p_mac_learn->limit_count = p_port_fid_db->limit_count;
            p_mac_learn->limit_num = p_port_fid_db->limit_num;
            p_mac_learn->limit_action = p_port_fid_db->limit_action;
            break;
        case CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT:
            sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
            gem_port_db.port = p_mac_learn->gport;
            gem_port_db.tunnel_value = p_mac_learn->tunnel_value;
            p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
            if (NULL == p_gem_port_db)
            {
                CTC_APP_VLAN_PORT_UNLOCK(lchip);
                return CTC_E_NOT_EXIST;
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

int32
ctc_usw_app_vlan_port_get_global_cfg(uint8 lchip, ctc_app_global_cfg_t* p_glb_cfg)
{
    uint16 vlan_id = 0;

    CTC_PTR_VALID_CHECK(p_glb_cfg);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_glb_cfg->vdev_id);

    if (p_glb_cfg->cfg_type == CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN)
    {
        CTC_VLAN_RANGE_CHECK(p_glb_cfg->vlan_id);
    }

    CTC_APP_VLAN_PORT_LOCK(lchip);
    switch(p_glb_cfg->cfg_type)
    {
        case CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE:
            p_glb_cfg->value = p_usw_vlan_port_master->uni_outer_isolate_en[p_glb_cfg->vdev_id];
            break;

        case CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE:
            p_glb_cfg->value = p_usw_vlan_port_master->uni_inner_isolate_en[p_glb_cfg->vdev_id];
            break;

        case CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP:
            p_glb_cfg->value = p_usw_vlan_port_master->unknown_mcast_drop_en[p_glb_cfg->vdev_id];
            break;
        case CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT:
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_mac_learn_limit(lchip, p_glb_cfg));
            break;

        case CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN:
            vlan_id = p_glb_cfg->vlan_id;
            p_glb_cfg->value = CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[vlan_id/32], vlan_id%32);
            break;

        default:
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_INVALID_CONFIG;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}


#define _____VLAN_PORT_____ ""

STATIC int32
_ctc_usw_app_vlan_port_vlan_edit(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port,
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
                *pkt_cvid = p_vlan_port->match_svlan_end;
            }
            else
            {
                *pkt_cvid = p_vlan_port->match_svlan;
            }
        }
    }
    else if(CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.svid)
    {
        if ((CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.cvid) || \
            (CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.cvid))
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
        if ((CTC_APP_VLAN_ACTION_ADD == p_vlan_port->ingress_vlan_action_set.cvid) || \
            (CTC_APP_VLAN_ACTION_REPLACE == p_vlan_port->ingress_vlan_action_set.cvid))
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
_ctc_usw_app_vlan_port_vlan_mapping(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port,
                               ctc_vlan_mapping_t* p_vlan_mapping, ctc_scl_vlan_edit_t* p_vlan_edit)
{
    CTC_APP_VLAN_PORT_INIT_CHECK();

    /*SVLAN op*/
    switch(p_vlan_port->ingress_vlan_action_set.svid)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->ingress_vlan_action_set.new_svid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            }
            else
            {
                p_vlan_edit->stag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->svid_new = p_vlan_port->ingress_vlan_action_set.new_svid;
            }
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->ingress_vlan_action_set.new_svid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_svid = p_vlan_port->ingress_vlan_action_set.new_svid;
            }
            else
            {
                p_vlan_edit->stag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_edit->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->svid_new = p_vlan_port->ingress_vlan_action_set.new_svid;
            }
            break;

        case CTC_APP_VLAN_ACTION_DEL:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_DEL;
            }
            else
            {
                p_vlan_edit->stag_op = CTC_VLAN_TAG_OP_DEL;
            }
            return CTC_E_INVALID_PARAM;
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_VALID;
            }
            else
            {
                p_vlan_edit->stag_op = CTC_VLAN_TAG_OP_VALID;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.cvid)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->ingress_vlan_action_set.new_cvid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            }
            else
            {
                p_vlan_edit->ctag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_edit->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->cvid_new = p_vlan_port->ingress_vlan_action_set.new_cvid;
            }
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->ingress_vlan_action_set.new_cvid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_cvid = p_vlan_port->ingress_vlan_action_set.new_cvid;
            }
            else
            {
                p_vlan_edit->ctag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_edit->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->cvid_new = p_vlan_port->ingress_vlan_action_set.new_cvid;
            }
            break;

        case CTC_APP_VLAN_ACTION_DEL:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_DEL;
            }
            else
            {
                p_vlan_edit->ctag_op = CTC_VLAN_TAG_OP_DEL;
            }
            return CTC_E_INVALID_PARAM;
            break;


         case CTC_APP_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_VALID;
            }
            else
            {
                p_vlan_edit->ctag_op = CTC_VLAN_TAG_OP_VALID;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.scos)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_scos = p_vlan_port->ingress_vlan_action_set.new_scos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
            }
            else
            {
                p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->scos_new = p_vlan_port->ingress_vlan_action_set.new_scos;
            }
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_scos = p_vlan_port->ingress_vlan_action_set.new_scos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->stag_op)
                {
                    p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            else
            {
                p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->scos_new = p_vlan_port->ingress_vlan_action_set.new_scos;
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_edit->stag_op)
                {
                    p_vlan_edit->stag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            break;

        case  CTC_APP_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            else
            {
                p_vlan_edit->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->ingress_vlan_action_set.ccos)
    {
        case CTC_APP_VLAN_ACTION_ADD:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_ccos = p_vlan_port->ingress_vlan_action_set.new_ccos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
            }
            else
            {
                p_vlan_edit->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->ccos_new = p_vlan_port->ingress_vlan_action_set.new_ccos;
            }
            break;

        case CTC_APP_VLAN_ACTION_REPLACE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_ccos = p_vlan_port->ingress_vlan_action_set.new_ccos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->ctag_op)
                {
                    p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            else
            {
                p_vlan_edit->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_edit->ccos_new = p_vlan_port->ingress_vlan_action_set.new_ccos;
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_edit->ctag_op)
                {
                    p_vlan_edit->ctag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            break;

         case CTC_APP_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            else
            {
                p_vlan_edit->ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_nh_xlate_mapping(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port,
                               ctc_vlan_egress_edit_info_t* p_xlate)
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
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
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
_ctc_usw_app_vlan_port_scl_xlate_mapping(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port,
                               ctc_scl_vlan_edit_t* p_xlate)
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

STATIC int32
_ctc_usw_app_vlan_port_add_vlan(uint8 lchip,
                           ctc_app_vlan_port_t* p_vlan_port,
                           ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    ctc_l2dflt_addr_t l2dflt;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /*Add vlan member*/
    if (NULL != p_gem_port_db)
    {
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = p_vlan_port_db->fid;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid;
        l2dflt.with_nh = 1;

        l2dflt.logic_port = p_vlan_port_db->logic_port;
        l2dflt.member.nh_id = p_vlan_port_db->xlate_nhid;
        CTC_ERROR_RETURN(ctc_l2_add_port_to_default_entry(&l2dflt));


        CTC_APP_DBG_INFO("Group(%u) add ONU nhid: %u\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_remove_vlan(uint8 lchip,
                           ctc_app_vlan_port_t* p_vlan_port,
                           ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    ctc_l2dflt_addr_t l2dflt;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /*Add vlan member*/
    if (NULL != p_gem_port_db)
    {
        sal_memset(&l2dflt, 0, sizeof(l2dflt));
        l2dflt.fid = p_vlan_port_db->fid;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid;
        l2dflt.with_nh = 1;

        l2dflt.logic_port = p_vlan_port_db->logic_port;
        l2dflt.member.nh_id = p_vlan_port_db->xlate_nhid;
        ctc_l2_remove_port_from_default_entry(&l2dflt);

        CTC_APP_DBG_INFO("Group(%u) remove ONU nhid: %u\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_downlink_scl_add(uint8 lchip,
                                    ctc_app_vlan_port_t* p_vlan_port,
                                    ctc_app_vlan_port_db_t* p_vlan_port_db,
                                    ctc_app_vlan_port_uni_db_t* p_uni_port)
{
    int32 ret = 0;
    ctc_vlan_range_info_t range_info;
    ctc_vlan_range_t vlan_range;
    ctc_scl_entry_t  scl_entry;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;

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
        scl_entry.key.u.hash_port_key.gport = p_vlan_port->port;
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
    CTC_SET_FLAG(scl_entry.action.u.egs_action.flag, CTC_SCL_EGS_ACTION_FLAG_VLAN_EDIT);

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_scl_xlate_mapping(lchip, p_vlan_port, &scl_entry.action.u.egs_action.vlan_edit), ret, roll_back_1);

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
    CTC_APP_DBG_INFO("entry_id %u\n", scl_entry.entry_id);

    return CTC_E_NONE;

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
_ctc_usw_app_vlan_port_downlink_scl_remove(uint8 lchip,
                                      ctc_app_vlan_port_db_t* p_vlan_port_db,
                                      ctc_app_vlan_port_uni_db_t* p_uni_port)
{
    ctc_vlan_range_info_t range_info;
    ctc_vlan_range_t vlan_range;
    uint32 entry_id = 0;

    sal_memset(&range_info, 0, sizeof(range_info));
    sal_memset(&vlan_range, 0, sizeof(vlan_range));

    entry_id = ENCODE_SCL_VLAN_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);
    CTC_ERROR_RETURN(ctc_scl_uninstall_entry(entry_id));
    CTC_ERROR_RETURN(ctc_scl_remove_entry(entry_id));

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
_ctc_usw_app_vlan_port_downlink_nexthop_add(uint8 lchip,
                                        ctc_app_vlan_port_t* p_vlan_port,
                                        ctc_app_vlan_port_db_t* p_vlan_port_db,
                                        ctc_app_vlan_port_uni_db_t* p_uni_port)
{
    int32 ret = 0;
    ctc_vlan_edit_nh_param_t vlan_xlate_nh;

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_offset(lchip, &p_vlan_port_db->ad_index), ret, roll_back_0);
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &p_vlan_port_db->xlate_nhid), ret, roll_back_1);

    /* Create xlate nh */
    sal_memset(&vlan_xlate_nh, 0, sizeof(vlan_xlate_nh));
    vlan_xlate_nh.dsnh_offset = p_vlan_port_db->pkt_svlan;
    vlan_xlate_nh.gport_or_aps_bridge_id = p_vlan_port_db->port;

    CTC_SET_FLAG(vlan_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_SERVICE_QUEUE_FLAG); /*logicRep*/

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_nh_xlate_mapping(lchip, p_vlan_port, &vlan_xlate_nh.vlan_edit_info), ret, roll_back_2);

    CTC_APP_DBG_INFO("nh xlate nhid: %u\n", p_vlan_port_db->xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_vlan_port_db->xlate_nhid, &vlan_xlate_nh), ret, roll_back_2);

    p_vlan_port_db->egs_vlan_mapping_en = 0;

    return CTC_E_NONE;

roll_back_2:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid);

roll_back_1:
    _ctc_usw_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index);

roll_back_0:
    return ret;
}

int32
_ctc_usw_app_vlan_port_downlink_nexthop_remove(uint8 lchip,
                                      ctc_app_vlan_port_db_t* p_vlan_port_db,
                                      ctc_app_vlan_port_uni_db_t* p_uni_port)
{
    ctc_nh_remove_xlate(p_vlan_port_db->xlate_nhid);

    _ctc_usw_app_vlan_port_free_nhid(lchip, p_vlan_port_db->xlate_nhid);
    _ctc_usw_app_vlan_port_free_offset(lchip, p_vlan_port_db->ad_index);

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_basic_create(uint8 lchip,
                             ctc_app_vlan_port_t* p_vlan_port,
                             ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    uint32 vlan_mapping_port = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    uint16 index = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_vlan_edit(lchip, p_vlan_port, &pkt_svlan, &pkt_cvlan), ret, roll_back_0);
    p_vlan_port_db->pkt_svlan = pkt_svlan;
    p_vlan_port_db->pkt_cvlan = pkt_cvlan;

    if ( p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        p_vlan_port_db->egs_vlan_mapping_en = 1;
    }

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_downlink_scl_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }
    else
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_downlink_nexthop_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }

    /* Add vlan mapping */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

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
        /* src port key */
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        vlan_mapping_port = p_gem_port_db->logic_port;
        vlan_mapping.scl_id = 0;

        /*known onu service*/
        if (p_vlan_port_db->logic_port_b_en)
        {
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
            vlan_mapping.logic_src_port = p_vlan_port_db->logic_port;
        }
    }
    else
    {
        vlan_mapping_port = p_vlan_port->port;
        vlan_mapping.scl_id = 1;
    }

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_vlan_mapping(lchip, p_vlan_port, &vlan_mapping, NULL), ret, roll_back_2);
    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = p_uni_port->vdev_id;
    port_fid_db.pkt_svlan = pkt_svlan;
    port_fid_db.pkt_cvlan = pkt_cvlan;

    CTC_ERROR_GOTO(ctc_spool_add(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL, &p_port_fid_db), ret, roll_back_2);
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_FID);
    vlan_mapping.u3.fid = p_port_fid_db->fid;
    p_vlan_port_db->fid = p_port_fid_db->fid;
    if (p_vlan_port->ingress_policer_id)
    {
        CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_SERVICE_POLICER_EN);
        vlan_mapping.policer_id = p_vlan_port->ingress_policer_id;
    }
    if (0 != p_vlan_port->match_svlan_end)
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
        p_vlan_port_db->igs_vlan_mapping_use_flex = 1;
    }
    ret = ctc_vlan_add_vlan_mapping(vlan_mapping_port, &vlan_mapping);
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

    if (p_vlan_port_db->match_svlan_end)
    {
        for (index = p_vlan_port_db->match_svlan; index < p_vlan_port_db->match_svlan_end; index++)
        {
            _ctc_usw_app_vlan_port_add_downlink_scl_entry(lchip, p_vlan_port_db->vdev_id, p_vlan_port_db->pkt_svlan, index, p_port_fid_db->fid, 1);
        }
    }

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_3:
    ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);

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
        _ctc_usw_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db, p_uni_port);
    }
    else
    {
        _ctc_usw_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

roll_back_0:

    return ret;
}

int32
_ctc_usw_app_vlan_port_basic_destroy(uint8 lchip,
                             ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    uint16 index = 0;
    uint32 vlan_mapping_port = 0;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    /* Remove vlan mapping */
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
    {
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        vlan_mapping_port =  p_gem_port_db->logic_port;
        vlan_mapping.scl_id = 0;
    }
    else
    {
        vlan_mapping_port = p_vlan_port_db->port;
        vlan_mapping.scl_id = 1;
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

    ctc_vlan_remove_vlan_mapping(vlan_mapping_port , &vlan_mapping);

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = p_uni_port->vdev_id;
    port_fid_db.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid_db.pkt_cvlan = p_vlan_port_db->pkt_cvlan;
    ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);

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

        ctc_vlan_remove_vlan_range(&range_info, &vlan_range);


        for (index = p_vlan_port_db->match_svlan; index < p_vlan_port_db->match_svlan_end; index++)
        {
            _ctc_usw_app_vlan_port_remove_downlink_scl_entry(lchip, p_vlan_port_db->vdev_id, index, p_vlan_port_db->fid, 1);
        }
    }

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        _ctc_usw_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db,  p_uni_port);
    }
    else
    {
        _ctc_usw_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db,  p_uni_port);
    }

    return ret;
}

int32
_ctc_usw_app_vlan_port_flex_acl_map_mac_key_field(uint8 lchip, uint32 entry_id, uint32 group_id, ctc_acl_mac_key_t* p_ctc_key)
{
    uint32 flag = 0;
    ctc_field_key_t key_field;
    ctc_scl_entry_t scl_entry;
    int32 ret = CTC_E_NONE;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    scl_entry.key_type = CTC_SCL_KEY_TCAM_MAC;
    scl_entry.mode = 1;
    scl_entry.action_type =  CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = entry_id;
    CTC_ERROR_RETURN(ctc_scl_add_entry(group_id, &scl_entry));

    flag = p_ctc_key->flag;
    /* mac da */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_MAC_DA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* mac sa */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_MAC_SA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* cvlan */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CVLAN))
    {
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_ctc_key->cvlan;
        key_field.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* ctag cos */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* svlan */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_SVLAN))
    {
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;
        key_field.data = p_ctc_key->svlan;
        key_field.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* stag cos */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* eth type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_ETH_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    /* l2 type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_L2_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    /* l3 type */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_L3_TYPE))
    {/*D2 not support*/
        ret = CTC_E_NOT_SUPPORT;
        goto roll_back_0;
    }

    /* ctag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* stag cfi */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_ARP_OP_CODE)
        || CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_IP_SA)
        || CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_IP_DA) )
    {/*D2 not support*/
        ret = CTC_E_NOT_SUPPORT;
        goto roll_back_0;
    }

    /*stag valid*/
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_STAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /*ctag valid*/
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_CTAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    /* vlan num */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_VLAN_NUM))
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    /* metadata */
    if (CTC_FLAG_ISSET(flag, CTC_ACL_MAC_KEY_FLAG_METADATA))
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    return CTC_E_NONE;

roll_back_0:
    ctc_scl_remove_entry(entry_id);

    return ret;
}

int32
_ctc_usw_app_vlan_port_flex_acl_map_ipv4_key_field(uint8 lchip, uint32 entry_id, uint32 group_id, ctc_acl_ipv4_key_t* p_ctc_key)
{
    uint32 flag = 0;
    uint32 sub_flag = 0;
    /*uint32 sub1_flag = 0;
    uint32 sub3_flag = 0;*/
    ctc_field_key_t key_field;
    ctc_scl_entry_t scl_entry;
    int32 ret = CTC_E_NONE;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
    scl_entry.mode = 1;
    scl_entry.action_type =  CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = entry_id;
    CTC_ERROR_RETURN(ctc_scl_add_entry(group_id, &scl_entry));

    /* flag */
    flag = p_ctc_key->flag;
    sub_flag = p_ctc_key->sub_flag;
    /*sub1_flag = p_ctc_key->sub1_flag;
    sub3_flag = p_ctc_key->sub3_flag;*/

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN))
    {
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_ctc_key->cvlan;
        key_field.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN))
    {
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;
        key_field.data = p_ctc_key->svlan;
        key_field.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L2_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    else if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        key_field.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    else
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = CTC_PARSER_L3_TYPE_IPV4;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA))
    {
        key_field.type = CTC_FIELD_KEY_IP_SA;
        key_field.data = p_ctc_key->ip_sa;
        key_field.mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA))
    {
        key_field.type = CTC_FIELD_KEY_IP_DA;
        key_field.data = p_ctc_key->ip_da;
        key_field.mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL))
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_DSCP))
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_PRECEDENCE))
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask & 0x7;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_FRAG))
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;/*useless*/
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_OPTION))
    {
        key_field.type = CTC_FIELD_KEY_IP_OPTIONS;
        key_field.data = p_ctc_key->ip_option;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ROUTED_PACKET))
    {
        key_field.type = CTC_FIELD_KEY_ROUTED_PKT;
        key_field.data = p_ctc_key->routed_packet;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_ECN))
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        key_field.mask = p_ctc_key->ecn_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_METADATA))
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_VLAN_NUM))
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_PKT_LEN_RANGE))
    {
        key_field.type = CTC_FIELD_KEY_IP_PKT_LEN_RANGE;
        key_field.data = p_ctc_key->pkt_len_min;
        key_field.mask = p_ctc_key->pkt_len_max;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV4_KEY_FLAG_IP_HDR_ERROR))
    {
        key_field.type = CTC_FIELD_KEY_IP_HDR_ERROR;
        key_field.data = p_ctc_key->ip_hdr_error;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT))
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT))
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_TCP_FLAGS))
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE)
        || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE))
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_IGMP;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_IGMP_TYPE;
        key_field.data = p_ctc_key->igmp_type;
        key_field.mask = p_ctc_key->igmp_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }


    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_OP_CODE))
    {
        key_field.type = CTC_FIELD_KEY_ARP_OP_CODE;
        key_field.data = p_ctc_key->arp_op_code;
        key_field.mask = p_ctc_key->arp_op_code_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_SENDER_IP))
    {
        key_field.type = CTC_FIELD_KEY_ARP_SENDER_IP;
        key_field.data = p_ctc_key->sender_ip;
        key_field.mask = p_ctc_key->sender_ip_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_TARGET_IP))
    {
        key_field.type = CTC_FIELD_KEY_ARP_TARGET_IP;
        key_field.data = p_ctc_key->target_ip;
        key_field.mask = p_ctc_key->target_ip_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ARP_PROTOCOL_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;
        key_field.data = p_ctc_key->arp_protocol_type;
        key_field.mask = p_ctc_key->arp_protocol_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_GRE_KEY))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_VNI))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_UDP;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_L4_USER_TYPE;
        key_field.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        key_field.mask = p_ctc_key->vni_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_NVGRE_KEY))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL_NUM))
    {
        key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        key_field.data = p_ctc_key->mpls_label_num;
        key_field.mask = p_ctc_key->mpls_label_num_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL0))
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL0;
        key_field.data = (p_ctc_key->mpls_label0 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 12) & 0xFFFFF;
        if(key_field.mask != 0)
        {
            CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        }
        key_field.type = CTC_FIELD_KEY_MPLS_EXP0;
        key_field.data = (p_ctc_key->mpls_label0 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 9) & 0x7;
        if(key_field.mask != 0)
        {
            CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        }
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT0;
        key_field.data = (p_ctc_key->mpls_label0 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label0_mask >> 8) & 0x1;
        if(key_field.mask != 0)
        {
            CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        }
        key_field.type = CTC_FIELD_KEY_MPLS_TTL0;
        key_field.data = p_ctc_key->mpls_label0 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label0_mask & 0xFF;
        if(key_field.mask != 0)
        {
            CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        }
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL1))
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL1;
        key_field.data = (p_ctc_key->mpls_label1 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 12) & 0xFFFFF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_EXP1;
        key_field.data = (p_ctc_key->mpls_label1 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 9) & 0x7;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT1;
        key_field.data = (p_ctc_key->mpls_label1 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label1_mask >> 8) & 0x1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_TTL1;
        key_field.data = p_ctc_key->mpls_label1 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label1_mask & 0xFF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub1_flag, CTC_ACL_IPV4_KEY_SUB1_FLAG_MPLS_LABEL2))
    {
        key_field.type = CTC_FIELD_KEY_MPLS_LABEL2;
        key_field.data = (p_ctc_key->mpls_label2 >> 12) & 0xFFFFF;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 12) & 0xFFFFF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_EXP2;
        key_field.data = (p_ctc_key->mpls_label2 >> 9) & 0x7;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 9) & 0x7;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_SBIT2;
        key_field.data = (p_ctc_key->mpls_label2 >> 8) & 0x1;
        key_field.mask = (p_ctc_key->mpls_label2_mask >> 8) & 0x1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
        key_field.type = CTC_FIELD_KEY_MPLS_TTL2;
        key_field.data = p_ctc_key->mpls_label2 & 0xFF;
        key_field.mask = p_ctc_key->mpls_label2_mask & 0xFF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub3_flag, CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_LEVEL))
    {
        key_field.type = CTC_FIELD_KEY_ETHER_OAM_LEVEL;
        key_field.data = p_ctc_key->ethoam_level;
        key_field.mask = p_ctc_key->ethoam_level_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(sub3_flag, CTC_ACL_IPV4_KEY_SUB3_FLAG_ETHOAM_OPCODE))
    {
        key_field.type = CTC_FIELD_KEY_ETHER_OAM_OP_CODE;
        key_field.data = p_ctc_key->ethoam_op_code;
        key_field.mask = p_ctc_key->ethoam_op_code_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    return CTC_E_NONE;

roll_back_0:
    ctc_scl_remove_entry(entry_id);

    return ret;
}

int32
_ctc_usw_app_vlan_port_flex_acl_map_ipv6_key_field(uint8 lchip, uint32 entry_id, uint32 group_id, ctc_acl_ipv6_key_t* p_ctc_key)
{
    uint32 flag = 0;
    uint32 sub_flag = 0;
    ctc_field_key_t key_field;
    ctc_scl_entry_t scl_entry;
    int32 ret = CTC_E_NONE;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&key_field, 0, sizeof(ctc_field_key_t));
    scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV6;
    scl_entry.mode = 1;
    scl_entry.action_type =  CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = entry_id;
    CTC_ERROR_RETURN(ctc_scl_add_entry(group_id, &scl_entry));

    flag = p_ctc_key->flag;
    sub_flag = p_ctc_key->sub_flag;
    key_field.type = CTC_FIELD_KEY_L3_TYPE;
    key_field.data = CTC_PARSER_L3_TYPE_IPV6;
    key_field.mask = 0xF;
    CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_DA;
        key_field.ext_data = p_ctc_key->mac_da;
        key_field.ext_mask = p_ctc_key->mac_da_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if(CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA))
    {
        key_field.type = CTC_FIELD_KEY_MAC_SA;
        key_field.ext_data = p_ctc_key->mac_sa;
        key_field.ext_mask = p_ctc_key->mac_sa_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN))
    {
        key_field.type = CTC_FIELD_KEY_CVLAN_ID;
        key_field.data = p_ctc_key->cvlan;
        key_field.mask = p_ctc_key->cvlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_COS;
        key_field.data = p_ctc_key->ctag_cos;
        key_field.mask = p_ctc_key->ctag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_SVLAN))
    {
        key_field.type = CTC_FIELD_KEY_SVLAN_ID;
        key_field.data = p_ctc_key->svlan;
        key_field.mask = p_ctc_key->svlan_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS))
    {
        key_field.type = CTC_FIELD_KEY_STAG_COS;
        key_field.data = p_ctc_key->stag_cos;
        key_field.mask = p_ctc_key->stag_cos_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ETH_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        key_field.data = p_ctc_key->eth_type;
        key_field.mask = p_ctc_key->eth_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L2_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L2_TYPE;
        key_field.data = p_ctc_key->l2_type;
        key_field.mask = p_ctc_key->l2_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = p_ctc_key->l3_type;
        key_field.mask = p_ctc_key->l3_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    else
    {
        key_field.type = CTC_FIELD_KEY_L3_TYPE;
        key_field.data = CTC_PARSER_L3_TYPE_IPV6;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA))
    {
        key_field.type = CTC_FIELD_KEY_IPV6_SA;
        key_field.ext_data = p_ctc_key->ip_sa;
        key_field.ext_mask = p_ctc_key->ip_sa_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA))
    {
        key_field.type = CTC_FIELD_KEY_IPV6_DA;
        key_field.ext_data = p_ctc_key->ip_da;
        key_field.ext_mask = p_ctc_key->ip_da_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_L4_PROTOCOL))
    {
        key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        key_field.data = p_ctc_key->l4_protocol;
        key_field.mask = p_ctc_key->l4_protocol_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_DSCP))
    {
        key_field.type = CTC_FIELD_KEY_IP_DSCP;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_PRECEDENCE))
    {
        key_field.type = CTC_FIELD_KEY_IP_PRECEDENCE;
        key_field.data = p_ctc_key->dscp;
        key_field.mask = p_ctc_key->dscp_mask & 0x7;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_FRAG))
    {
        key_field.type = CTC_FIELD_KEY_IP_FRAG;
        key_field.data = p_ctc_key->ip_frag;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_OPTION))
    {
        key_field.type = CTC_FIELD_KEY_IP_OPTIONS;
        key_field.data = p_ctc_key->ip_option;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ROUTED_PACKET))
    {
        key_field.type = CTC_FIELD_KEY_ROUTED_PKT;
        key_field.data = p_ctc_key->routed_packet;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_FLOW_LABEL))
    {
        key_field.type = CTC_FIELD_KEY_IPV6_FLOW_LABEL;
        key_field.data = p_ctc_key->flow_label;
        key_field.mask = p_ctc_key->flow_label_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_CFI;
        key_field.data = p_ctc_key->ctag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_CFI))
    {
        key_field.type = CTC_FIELD_KEY_STAG_CFI;
        key_field.data = p_ctc_key->stag_cfi;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_STAG_VALID;
        key_field.data = p_ctc_key->stag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID))
    {
        key_field.type = CTC_FIELD_KEY_CTAG_VALID;
        key_field.data = p_ctc_key->ctag_valid;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_ECN))
    {
        key_field.type = CTC_FIELD_KEY_IP_ECN;
        key_field.data = p_ctc_key->ecn;
        key_field.mask = p_ctc_key->ecn_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_METADATA))
    {
        key_field.type = CTC_FIELD_KEY_METADATA;
        key_field.data = p_ctc_key->metadata;
        key_field.mask = p_ctc_key->metadata_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_UDF))
    {
        return CTC_E_NOT_SUPPORT;
    }
#endif
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_VLAN_NUM))
    {
        key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        key_field.data = p_ctc_key->vlan_num;
        key_field.mask = p_ctc_key->vlan_num_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_PKT_LEN_RANGE))
    {
        key_field.type = CTC_FIELD_KEY_IP_PKT_LEN_RANGE;
        key_field.data = p_ctc_key->pkt_len_min;
        key_field.mask = p_ctc_key->pkt_len_max;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if (CTC_FLAG_ISSET(flag, CTC_ACL_IPV6_KEY_FLAG_IP_HDR_ERROR))
    {
        key_field.type = CTC_FIELD_KEY_IP_HDR_ERROR;
        key_field.data = p_ctc_key->ip_hdr_error;
        key_field.mask = 1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    /*The sequence is according to ctc_acl_ipv6_key_sub_flag_t */
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT))
    {
        if(p_ctc_key->l4_src_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_SRC_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_src_port_0;
        key_field.mask = p_ctc_key->l4_src_port_1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if (CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT))
    {
        if(p_ctc_key->l4_dst_port_use_mask)
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        }
        else
        {
            key_field.type = CTC_FIELD_KEY_L4_DST_PORT_RANGE;
        }
        key_field.data = p_ctc_key->l4_dst_port_0;
        key_field.mask = p_ctc_key->l4_dst_port_1;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#if 0
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_TCP_FLAGS))
    {
        key_field.type = CTC_FIELD_KEY_TCP_FLAGS;
        key_field.data = p_ctc_key->tcp_flags;
        if(p_ctc_key->tcp_flags_match_any)
        {
            key_field.mask = ~(p_ctc_key->tcp_flags);
        }
        else
        {
            key_field.mask = 0x3F;
        }
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_TYPE)
        || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_ICMP_CODE))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_ICMP;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_TYPE))
    {
        key_field.type = CTC_FIELD_KEY_ICMP_TYPE;
        key_field.data = p_ctc_key->icmp_type;
        key_field.mask = p_ctc_key->icmp_type_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_ICMP_CODE))
    {
        key_field.type = CTC_FIELD_KEY_ICMP_CODE;
        key_field.data = p_ctc_key->icmp_code;
        key_field.mask = p_ctc_key->icmp_code_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY)
        || CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY))
    {
    key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_GRE;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_GRE_KEY))
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_NVGRE_KEY))
    {
        key_field.type = CTC_FIELD_KEY_GRE_KEY;
        key_field.data = p_ctc_key->gre_key;
        key_field.mask = p_ctc_key->gre_key_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }

    if(CTC_FLAG_ISSET(sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_VNI))
    {
        key_field.type = CTC_FIELD_KEY_L4_TYPE;
        key_field.data = CTC_PARSER_L4_TYPE_UDP;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_L4_USER_TYPE;
        key_field.data = CTC_PARSER_L4_USER_TYPE_UDP_VXLAN;
        key_field.mask = 0xF;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);

        key_field.type = CTC_FIELD_KEY_VN_ID;
        key_field.data = p_ctc_key->vni;
        key_field.mask = p_ctc_key->vni_mask;
        CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &key_field), ret, roll_back_0);
    }
#endif
    return CTC_E_NONE;

roll_back_0:
    ctc_scl_remove_entry(entry_id);

    return ret;
}

int32
_ctc_usw_app_vlan_port_flex_acl_mapp_to_vlan_mapping(uint8 lchip, uint32 entry_id, uint32 group_id, ctc_acl_key_t* p_flex_key, uint8* tcam_type)
{

    switch (p_flex_key->type)
    {
        case CTC_ACL_KEY_MAC:
            CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_flex_acl_map_mac_key_field(lchip, entry_id, group_id, &p_flex_key->u.mac_key));
            *tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
            break;
        case CTC_ACL_KEY_IPV4:
            CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_flex_acl_map_ipv4_key_field(lchip, entry_id, group_id, &p_flex_key->u.ipv4_key));
            *tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
            break;
        case CTC_ACL_KEY_IPV6:
            CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_flex_acl_map_ipv6_key_field(lchip, entry_id, group_id, &p_flex_key->u.ipv6_key));
            *tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_IP;
            break;
        default:
            return CTC_E_NOT_SUPPORT;
            break;
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_flex_create(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port, ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    int32 ret = 0;
    uint8 tcam_type = 0;
    uint16 lport = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    uint32 entry_id = 0;
    uint32 group_id = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;
    ctc_port_scl_property_t scl_prop;

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;


    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_vlan_edit(lchip, p_vlan_port, &pkt_svlan, &pkt_cvlan), ret, roll_back_0);
    p_vlan_port_db->pkt_svlan = pkt_svlan;
    p_vlan_port_db->pkt_cvlan = pkt_cvlan;

    if ( p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_FLEX)
    {
        p_vlan_port_db->egs_vlan_mapping_en = 1;
    }

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        group_id = CTC_SCL_GROUP_ID_TCAM3;
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_downlink_scl_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }
    else
    {
        group_id = CTC_SCL_GROUP_ID_TCAM2;
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_downlink_nexthop_add(lchip, p_vlan_port, p_vlan_port_db,  p_uni_port), ret, roll_back_0);
    }

    entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_flex_acl_mapp_to_vlan_mapping(lchip, entry_id, group_id, &p_vlan_port->flex_key, &tcam_type), ret, roll_back_1);
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
    field_key.type = CTC_FIELD_KEY_PORT;
    if (p_vlan_port_db->egs_vlan_mapping_en)
    {
        field_port.gport = p_vlan_port->port;
        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
    }
    else
    {
        field_port.logic_port = p_gem_port_db->logic_port;
        field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
    }
    field_key.ext_data = &field_port;
    CTC_ERROR_GOTO(ctc_scl_add_key_field(entry_id, &field_key), ret, roll_back_2);

    if (p_vlan_port_db->logic_port_b_en)
    {
        ctc_scl_logic_port_t logic_dest_port;
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        sal_memset(&logic_dest_port, 0, sizeof(ctc_scl_logic_port_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
        logic_dest_port.logic_port = p_vlan_port_db->logic_port;
        logic_dest_port.logic_port_type = 0;
        field_action.ext_data = &logic_dest_port;
        CTC_ERROR_GOTO(ctc_scl_add_action_field(entry_id, &field_action), ret, roll_back_2);
    }

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
    sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_vlan_mapping(lchip, p_vlan_port, NULL, &vlan_edit), ret, roll_back_2);
    field_action.ext_data = &vlan_edit;
    CTC_ERROR_GOTO(ctc_scl_add_action_field(entry_id, &field_action), ret, roll_back_2);

    if (p_vlan_port->ingress_policer_id)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_vlan_port->ingress_policer_id;
        field_action.data1 = 1;
        CTC_ERROR_GOTO(ctc_scl_add_action_field(entry_id, &field_action), ret, roll_back_2);
    }

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = p_uni_port->vdev_id;
    port_fid_db.pkt_svlan = pkt_svlan;
    port_fid_db.pkt_cvlan = pkt_cvlan;
    CTC_ERROR_GOTO(ctc_spool_add(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL, &p_port_fid_db), ret, roll_back_2);
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
    field_action.data0 = p_port_fid_db->fid;
    CTC_ERROR_GOTO(ctc_scl_add_action_field(entry_id, &field_action), ret, roll_back_3);
    p_vlan_port_db->fid = p_port_fid_db->fid;

    CTC_ERROR_GOTO(ctc_scl_install_entry(entry_id), ret, roll_back_3);

    /** Enable scl lookup double vlan for onu and pon qinq */
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    ctc_port_get_scl_property(p_vlan_port_db->port, &scl_prop);
    if (tcam_type!=scl_prop.tcam_type && CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT!=scl_prop.tcam_type)
    {
        CTC_APP_DBG_ERROR("Tcam type must keep the same when update!\n");
        ret = CTC_E_INVALID_PARAM;
        goto roll_back_4;
    }

    if ((0 == p_uni_port->flex_pon_cnt && p_vlan_port_db->egs_vlan_mapping_en) || \
        (0 == p_uni_port->flex_onu_cnt && !(p_vlan_port_db->egs_vlan_mapping_en)))
    {
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = p_vlan_port_db->egs_vlan_mapping_en?3:2;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.use_logic_port_en = p_vlan_port_db->egs_vlan_mapping_en?0:1;
        scl_prop.tcam_type = tcam_type;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(p_vlan_port_db->port, &scl_prop), ret, roll_back_4);
    }
    p_vlan_port_db->egs_vlan_mapping_en? p_uni_port->flex_pon_cnt++: p_uni_port->flex_onu_cnt++;
    CTC_APP_DBG_INFO("flex pon cnt: %d, flex onu cnt: %d\n", p_uni_port->flex_pon_cnt, p_uni_port->flex_onu_cnt);

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_4:
    ctc_scl_uninstall_entry(entry_id);

roll_back_3:
    ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);

roll_back_2:
    ctc_scl_remove_entry(entry_id);

roll_back_1:
    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        _ctc_usw_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db, p_uni_port);
    }
    else
    {
        _ctc_usw_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

roll_back_0:

    return ret;
}

int32
_ctc_usw_app_vlan_port_flex_destroy(uint8 lchip, ctc_app_vlan_port_db_t* p_vlan_port_db)
{
    uint16 lport = 0;
    uint32 entry_id = 0;
    ctc_app_vlan_port_uni_db_t *p_uni_port = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_port_scl_property_t scl_prop;

    lport = CTC_MAP_GPORT_TO_LPORT(p_vlan_port_db->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    /** Enable scl lookup double vlan for onu and pon qinq */
    p_vlan_port_db->egs_vlan_mapping_en? p_uni_port->flex_pon_cnt--: p_uni_port->flex_onu_cnt--;
    if ((0==p_uni_port->flex_pon_cnt && p_vlan_port_db->egs_vlan_mapping_en) || \
        (0==p_uni_port->flex_onu_cnt && !(p_vlan_port_db->egs_vlan_mapping_en)))
    {
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = p_vlan_port_db->egs_vlan_mapping_en?3:2;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        scl_prop.use_logic_port_en = p_vlan_port_db->egs_vlan_mapping_en?0:1;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(p_vlan_port_db->port, &scl_prop);
    }
    CTC_APP_DBG_INFO("flex pon cnt: %d, flex onu cnt: %d\n", p_uni_port->flex_pon_cnt, p_uni_port->flex_onu_cnt);

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = p_uni_port->vdev_id;
    port_fid_db.pkt_svlan = p_vlan_port_db->pkt_svlan;
    port_fid_db.pkt_cvlan = p_vlan_port_db->pkt_cvlan;

    ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);
    entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0);
    ctc_scl_uninstall_entry(entry_id);
    ctc_scl_remove_entry(entry_id);

    if (1 == p_vlan_port_db->egs_vlan_mapping_en)
    {
        _ctc_usw_app_vlan_port_downlink_scl_remove(lchip, p_vlan_port_db, p_uni_port);
    }
    else
    {
        _ctc_usw_app_vlan_port_downlink_nexthop_remove(lchip, p_vlan_port_db, p_uni_port);
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_check_policer_id(uint8 lchip, uint16 policer_id, uint8 dir)
{
    uint8   sys_policer_type = 2;       /*SYS_QOS_POLICER_TYPE_FLOW*/
    uint8   is_bwp = 0;
    int32   ret = CTC_E_NONE;

    if (policer_id)
    {
        ret = CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_qos_policer_index_get, lchip, policer_id, sys_policer_type, &is_bwp);
        if (!is_bwp && (CTC_EGRESS == dir))      /*egress only support hbwp policer*/
        {
            CTC_APP_DBG_ERROR("Only Policer level 0 and hbwp is support in downstream !\n");
            return CTC_E_NOT_SUPPORT;
        }
    }

    return ret;
}

int32
ctc_usw_app_vlan_port_create(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    int32 ret = 0;
    uint8 onu_sevice_en = 0;
    uint32 logic_port = 0;
    uint32 vlan_port_id = 0;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_opf_t opf;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_svlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_cvlan);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_svlan_end);
    CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_vlan_port->match_cvlan_end);
    CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(p_vlan_port->match_tunnel_value);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_VLAN_PORT, p_vlan_port->port, p_vlan_port->vlan_port_id));

    /* Debug */
    CTC_APP_DBG_FUNC();
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
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_vlan_port->port, &p_uni_port));

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_uni_port->vdev_id])
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    if ((p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN)
        || (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX))
    {
        onu_sevice_en = 1; /*gem port onu base vlan edit*/
    }
    else if((p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
        || (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_FLEX))
    {
         onu_sevice_en = 0; /*port base vlan edit*/
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
        p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);

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

    p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);

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
    CTC_APP_DBG_INFO("OPF vlan port id: %u\n", vlan_port_id);

    /* Build node */
    MALLOC_POINTER(ctc_app_vlan_port_db_t, p_vlan_port_db);
    if (NULL == p_vlan_port_db)
    {
        goto roll_back_1;
    }

    sal_memset(p_vlan_port_db, 0, sizeof(ctc_app_vlan_port_db_t));
    p_vlan_port_db->vdev_id = p_uni_port->vdev_id;
    p_vlan_port_db->vlan_port_id = vlan_port_id;
    p_vlan_port_db->ingress_policer_id = p_vlan_port->ingress_policer_id;
    p_vlan_port_db->egress_policer_id = p_vlan_port->egress_policer_id;

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_check_policer_id(lchip, p_vlan_port->ingress_policer_id, CTC_INGRESS),ret, roll_back_2);
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_check_policer_id(lchip, p_vlan_port->egress_policer_id, CTC_EGRESS),ret, roll_back_2);
    p_vlan_port_db->criteria = p_vlan_port->criteria;
    p_vlan_port_db->port = p_vlan_port->port;
    p_vlan_port_db->tunnel_value = p_vlan_port->match_tunnel_value;
    p_vlan_port_db->match_svlan = p_vlan_port->match_svlan;
    p_vlan_port_db->match_svlan_end = p_vlan_port->match_svlan_end;
    p_vlan_port_db->match_cvlan = p_vlan_port->match_cvlan;
    p_vlan_port_db->match_cvlan_end = p_vlan_port->match_cvlan_end;
    sal_memcpy(&p_vlan_port_db->flex_key, &p_vlan_port->flex_key, sizeof(ctc_acl_key_t));
    sal_memcpy(&p_vlan_port_db->ingress_vlan_action_set, &p_vlan_port->ingress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));
    sal_memcpy(&p_vlan_port_db->egress_vlan_action_set, &p_vlan_port->egress_vlan_action_set, sizeof(ctc_app_vlan_action_set_t));

    p_vlan_port_db->p_gem_port_db = p_gem_port_db;

    if ((CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX == p_vlan_port->criteria)
        || (CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN == p_vlan_port->criteria)) /*First ONU service use the gem port logic port*/
    {
        if (0 == p_gem_port_db->ref_cnt)
        {
            p_vlan_port_db->logic_port = p_gem_port_db->logic_port;
            p_vlan_port_db->logic_port_b_en = 1;
        }
        else
        {
            /* OPF --> vlan port loigcPort B*/
            sal_memset(&opf, 0, sizeof(opf));
            opf.pool_type = CTC_OPF_VLAN_PORT;
            opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
            CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_2);
            p_vlan_port_db->logic_port = logic_port;
            p_vlan_port_db->logic_port_b_en = 1;
            CTC_APP_DBG_INFO("OPF vlan logic port: %u\n", logic_port);
            /* Create l2editflex for add gem tunnel vlan */
            CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_add_gem_port, lchip, logic_port, p_gem_port_db->tunnel_value), ret, roll_back_3);
        }
    }

    /* VLAN edit */
    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_basic_create(lchip, p_vlan_port, p_vlan_port_db),ret, roll_back_4);
    }
    else
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_flex_create(lchip, p_vlan_port, p_vlan_port_db),ret, roll_back_4);
    }

    /*Add vlan*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_add_vlan(lchip, p_vlan_port, p_vlan_port_db), ret, roll_back_5);

    ctc_hash_insert(p_usw_vlan_port_master->vlan_port_hash, p_vlan_port_db);
    ctc_hash_insert(p_usw_vlan_port_master->vlan_port_key_hash, p_vlan_port_db);
    p_usw_vlan_port_master->vlan_port_cnt[p_uni_port->vdev_id]++;

    if(onu_sevice_en)
    {
        p_gem_port_db->ref_cnt++;
    }

    p_vlan_port->vlan_port_id = vlan_port_id;
    p_vlan_port->logic_port = p_vlan_port_db->logic_port;

    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
roll_back_5:
    /* VLAN edit */
    if (p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        _ctc_usw_app_vlan_port_basic_destroy(lchip, p_vlan_port_db);
    }
    else
    {
        _ctc_usw_app_vlan_port_flex_destroy(lchip, p_vlan_port_db);
    }

roll_back_4:
    if (p_vlan_port_db->logic_port_b_en && (p_vlan_port_db->logic_port != p_gem_port_db->logic_port))
    {
        CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_remove_gem_port, lchip, logic_port);
    }

roll_back_3:
    if (p_vlan_port_db->logic_port_b_en && (p_vlan_port_db->logic_port != p_gem_port_db->logic_port))
    {
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        ctc_opf_free_offset(&opf, 1, logic_port);
    }

roll_back_2:
    mem_free(p_vlan_port_db);
    p_vlan_port_db = NULL;

roll_back_1:
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    ctc_opf_free_offset(&opf, 1, vlan_port_id);

roll_back_0:
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_usw_app_vlan_port_destory(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    uint32 logic_port = 0;
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_opf_t opf;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_VLAN_PORT, p_vlan_port->port, p_vlan_port->vlan_port_id));

    /* Debug */
    CTC_APP_DBG_FUNC();
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
        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
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

        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    p_gem_port_db = p_vlan_port_db->p_gem_port_db;

    logic_port = p_vlan_port_db->logic_port;

    /*Add vlan*/
    _ctc_usw_app_vlan_port_remove_vlan(lchip, p_vlan_port, p_vlan_port_db);

    /* VLAN edit */
    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN)
    {
        _ctc_usw_app_vlan_port_basic_destroy(lchip, p_vlan_port_db);
    }
    else
    {
        _ctc_usw_app_vlan_port_flex_destroy(lchip, p_vlan_port_db);
    }

    /* Free opf */
    sal_memset(&opf, 0, sizeof(opf));

    if (p_vlan_port_db->logic_port_b_en && (p_vlan_port_db->logic_port != p_gem_port_db->logic_port))
    {
        CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_remove_gem_port, lchip, logic_port);

        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_opf_free_offset(&opf, 1, logic_port));
    }

    /* Free vlan port id*/
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_opf_free_offset(&opf, 1, p_vlan_port_db->vlan_port_id));

    ctc_hash_remove(p_usw_vlan_port_master->vlan_port_hash, p_vlan_port_db);
    ctc_hash_remove(p_usw_vlan_port_master->vlan_port_key_hash, p_vlan_port_db);

    p_usw_vlan_port_master->vlan_port_cnt[p_uni_port->vdev_id]--;

    if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN ||
        p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX)
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
ctc_usw_app_vlan_port_get(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_VLAN_PORT, p_vlan_port->port, p_vlan_port->vlan_port_id));

    /* Debug */
    CTC_APP_DBG_FUNC();
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
        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
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

        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    p_vlan_port->vlan_port_id = p_vlan_port_db->vlan_port_id;
    p_vlan_port->logic_port = p_vlan_port_db->logic_port;
    p_vlan_port->flex_nhid = p_vlan_port_db->xlate_nhid;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_update(uint8 lchip, ctc_app_vlan_port_t* p_vlan_port)
{
    uint32 vlan_mapping_port = 0;
    ctc_app_vlan_port_db_t vlan_port_db_tmp;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_qos_policer_t policer;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_scl_field_action_t field_action;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vlan_port);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_VLAN_PORT, p_vlan_port->port, p_vlan_port->vlan_port_id));

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

    if (p_vlan_port->update_type == CTC_APP_VLAN_PORT_UPDATE_NONE)
    {
        return CTC_E_NOT_SUPPORT;
    }

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* DB */
    sal_memset(&vlan_port_db_tmp, 0, sizeof(vlan_port_db_tmp));
    sal_memset(&policer, 0, sizeof(policer));

    if (p_vlan_port->vlan_port_id)
    {
        vlan_port_db_tmp.vlan_port_id = p_vlan_port->vlan_port_id;
        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_hash, &vlan_port_db_tmp);
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

        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_key_hash, &vlan_port_db_tmp);
    }

    if (NULL == p_vlan_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_vlan_port_db->port, &p_uni_port));

    /*-----------------------------------------*/
    /*Ingress policetr*/
    if (p_vlan_port->update_type == CTC_APP_VLAN_PORT_UPDATE_IGS_POLICER)
    {
        if (p_vlan_port->ingress_policer_id == p_vlan_port_db->ingress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_check_policer_id(lchip, p_vlan_port->ingress_policer_id, CTC_INGRESS));

        /*-----------------------------------------*/
        /*Update ingress policetr*/

        p_gem_port_db = p_vlan_port_db->p_gem_port_db;

        /* Add vlan mapping */
        sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));

        if ((p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN)
            || (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN))
        {
            if (p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN )
            {
                vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
                vlan_mapping_port =  p_gem_port_db->logic_port;
                vlan_mapping.scl_id = 0;
            }
            else
            {
                vlan_mapping_port = p_vlan_port_db->port;
                vlan_mapping.scl_id = 1;
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
                CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
            }

            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_get_vlan_mapping(vlan_mapping_port , &vlan_mapping));

            vlan_mapping.policer_id = p_vlan_port->ingress_policer_id;

            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_vlan_update_vlan_mapping(vlan_mapping_port , &vlan_mapping));
        }
        else
        {
            sal_memset(&field_action, 0, sizeof(field_action));
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
            field_action.data0 = p_vlan_port->ingress_policer_id;
            field_action.data1 = 1;
            if (p_vlan_port->ingress_policer_id)
            {
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_add_action_field(ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0), &field_action));
            }
            else
            {
                CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_scl_remove_action_field(ENCODE_SCL_UPLINK_ENTRY_ID(p_vlan_port_db->vlan_port_id, 0), &field_action));
            }
        }
        p_vlan_port_db->ingress_policer_id  = p_vlan_port->ingress_policer_id;
    }

    /*-----------------------------------------*/
    /*Egress policetr*/
    if (p_vlan_port->update_type == CTC_APP_VLAN_PORT_UPDATE_EGS_POLICER)
    {
        if (p_vlan_port->egress_policer_id == p_vlan_port_db->egress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_check_policer_id(lchip, p_vlan_port->egress_policer_id, CTC_EGRESS));

        p_vlan_port_db->egress_policer_id  = p_vlan_port->egress_policer_id;

    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _____GEM_PORT_____ ""

int32
_ctc_usw_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                     ctc_app_vlan_port_gem_port_db_t* p_gem_port_db)
{
    uint16 lport = 0;
    int32 ret = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_offset(lchip, &p_gem_port_db->ad_index), ret, roll_back_0);

    lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    /***********************************************
    **  from ONU to uplink
    ***********************************************/
    /* Add Gem port Hash key */
    sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
    bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    bpe_gem_port.gem_vlan = p_gem_port->tunnel_value;
    bpe_gem_port.logic_port = p_gem_port_db->logic_port;
    bpe_gem_port.pass_through_en = p_gem_port->pass_trough_en;
    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port), ret, roll_back_1);

    /***********************************************
    **  from uplink to ONU
    ***********************************************/
    /* Create xlate nh for leaning */
    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_add_gem_port, lchip, p_gem_port_db->logic_port, p_gem_port->tunnel_value), ret, roll_back_2);

    /* Create xlate nh */
    CTC_ERROR_GOTO(ctc_nh_get_l2uc(p_gem_port_db->port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &p_gem_port_db->pon_downlink_nhid), ret, roll_back_3);

    CTC_APP_DBG_INFO("add gem port pon downlink nhid: %u\n", p_gem_port_db->pon_downlink_nhid);

    /*PON port refcnt ++*/
    p_uni_port->ref_cnt++;

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
roll_back_3:
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_remove_gem_port, lchip, p_gem_port_db->logic_port);

roll_back_2:
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_remove_gem_port, lchip, (void*)&bpe_gem_port);

roll_back_1:
   _ctc_usw_app_vlan_port_free_offset(lchip, p_gem_port_db->ad_index);

roll_back_0:
    return ret;
}

int32
_ctc_usw_app_vlan_port_destroy_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port,
                                     ctc_app_vlan_port_gem_port_db_t* p_gem_port_db)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

    lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

     /***********************************************
     **  from uplink to ONU
     ***********************************************/

    if (p_gem_port_db->pon_downlink_nhid)
    {
        /* Create xlate nh */
        /*ctc_nh_remove_xlate(p_gem_port_db->pon_downlink_nhid);*/
    }
    CTC_APP_DBG_INFO("Remove gem port pon downlink nhid: %u\n", p_gem_port_db->pon_downlink_nhid);

    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_remove_gem_port, lchip, p_gem_port_db->logic_port);

    /***********************************************
    **  from ONU to uplink
    ***********************************************/
    sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
    bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port->port);
    bpe_gem_port.gem_vlan = p_gem_port->tunnel_value;
    bpe_gem_port.logic_port = p_gem_port_db->logic_port;
    bpe_gem_port.pass_through_en = p_gem_port->pass_trough_en;
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_remove_gem_port, lchip, (void*)&bpe_gem_port);

    _ctc_usw_app_vlan_port_free_offset(lchip, p_gem_port_db->ad_index);

    /*PON port refcnt ++*/
    p_uni_port->ref_cnt--;

    return ret;
}

int32
ctc_usw_app_vlan_port_create_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    int32 ret = 0;
    uint32 logic_port = 0;
    ctc_opf_t opf;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_GEM_PORT, p_gem_port->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port",         p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "tunnel_value", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));
    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
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
    p_gem_port_db->pass_trough_en = p_gem_port->pass_trough_en?1:0;
    p_gem_port_db->ingress_policer_id = p_gem_port->ingress_policer_id;
    p_gem_port_db->egress_policer_id = p_gem_port->egress_policer_id;
    p_gem_port_db->limit_num = CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE;

    /* OPF --> logicPort A */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, free_meomory);
    p_gem_port_db->logic_port = logic_port;
    p_gem_port->logic_port = logic_port;
    CTC_APP_DBG_INFO("OPF gem logic port: %u\n", logic_port);

    /*Create gem port sevice*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_create_gem_port(lchip, p_gem_port, p_gem_port_db), ret, roll_back_0);

    /* DB & STATS */
    ctc_hash_insert(p_usw_vlan_port_master->gem_port_hash, p_gem_port_db);
    p_usw_vlan_port_master->gem_port_cnt[p_gem_port_db->vdev_id]++;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;

   /*-----------------------------------------------------------
   *** rool back
   -----------------------------------------------------------*/
roll_back_0:
    ctc_opf_free_offset(&opf, 1, logic_port);

free_meomory:
    mem_free(p_gem_port_db);
    p_gem_port_db = NULL;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_usw_app_vlan_port_destory_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    uint32 logic_port = 0;
    ctc_opf_t opf;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_GEM_PORT, p_gem_port->port, 0));

    /* Debug */
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "tunnel_value", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    _ctc_usw_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port);

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
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
    CTC_APP_DBG_INFO("Gem logic port: %u\n", logic_port);

    /*Create gem port sevice*/
    _ctc_usw_app_vlan_port_destroy_gem_port(lchip, p_gem_port, p_gem_port_db);

    /* Free OPF */
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    ctc_opf_free_offset(&opf, 1, logic_port);

    ctc_hash_remove(p_usw_vlan_port_master->gem_port_hash, p_gem_port_db);
    p_usw_vlan_port_master->gem_port_cnt[p_uni_port->vdev_id]--;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    mem_free(p_gem_port_db);
    p_gem_port_db = NULL;

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_update_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    int32 ret = 0;
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_GEM_PORT, p_gem_port->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if(CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT == p_gem_port->update_type ||
        CTC_APP_GEM_PORT_UPDATE_UNBIND_VLAN_PORT == p_gem_port->update_type)
    {
        ctc_app_vlan_port_db_t vlan_port_db;
        ctc_app_vlan_port_db_t *p_vlan_port_db = NULL;
        ctc_l2dflt_addr_t l2dflt;

        /* DB */
        sal_memset(&vlan_port_db, 0, sizeof(vlan_port_db));
        vlan_port_db.vlan_port_id = p_gem_port->vlan_port_id;

        p_vlan_port_db = ctc_hash_lookup(p_usw_vlan_port_master->vlan_port_hash, &vlan_port_db);

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
        l2dflt.fid = p_vlan_port_db->fid;
        l2dflt.l2mc_grp_id = p_vlan_port_db->fid;
        l2dflt.logic_port = p_gem_port_db->logic_port;
        l2dflt.with_nh = 1;
        l2dflt.member.nh_id = p_gem_port_db->pon_downlink_nhid;

        CTC_APP_DBG_INFO("p_vlan_port_db->logic_port:%u\n", p_vlan_port_db->logic_port);

        if (CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT == p_gem_port->update_type )
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_add_port_to_default_entry(&l2dflt));
            CTC_APP_DBG_INFO("Group(%u) add ONU nhid: %u\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
        }
        else
        {
            CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(ctc_l2_remove_port_from_default_entry(&l2dflt));
            CTC_APP_DBG_INFO("Group(%u) remove ONU nhid: %u\n", l2dflt.l2mc_grp_id, l2dflt.member.nh_id);
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
        if (p_gem_port->egress_policer_id == p_gem_port_db->egress_policer_id)
        {
            CTC_APP_VLAN_PORT_UNLOCK(lchip);
            return CTC_E_NONE;
        }

        /*-----------------------------------------*/
        /*Egress policetr*/
        p_gem_port_db->egress_policer_id = p_gem_port->egress_policer_id;
    }
    else if (CTC_APP_GEM_PORT_UPDATE_PASS_THROUGH_EN == p_gem_port->update_type)
    {
        sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
        bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_db->port);
        bpe_gem_port.gem_vlan = p_gem_port_db->tunnel_value;
        bpe_gem_port.logic_port = p_gem_port_db->logic_port;
        bpe_gem_port.pass_through_en = p_gem_port->pass_trough_en;
        bpe_gem_port.mac_security = p_gem_port_db->mac_security;
        CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port));
        p_gem_port_db->pass_trough_en = p_gem_port->pass_trough_en;
    }
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
ctc_usw_app_vlan_port_get_gem_port(uint8 lchip, ctc_app_gem_port_t* p_gem_port)
{
    ctc_app_vlan_port_gem_port_db_t gem_port_db;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_db = NULL;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_gem_port);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_TUNNEL_VLAN_ID_CHECK(p_gem_port->tunnel_value);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_GEM_PORT, p_gem_port->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_gem_port->port);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_gem_port->tunnel_value);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check pon port */
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_gem_port->port, &p_uni_port));

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    /* DB */
    sal_memset(&gem_port_db, 0, sizeof(gem_port_db));
    gem_port_db.port = p_gem_port->port;
    gem_port_db.tunnel_value = p_gem_port->tunnel_value;
    p_gem_port_db = ctc_hash_lookup(p_usw_vlan_port_master->gem_port_hash, &gem_port_db);
    if (NULL == p_gem_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    p_gem_port->logic_port = p_gem_port_db->logic_port;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;
}

#define _____UNI_PORT_____ ""

/*
Each pon port: xlate nhid
*/
int32
_ctc_usw_app_vlan_port_create_mc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_vlan_edit_nh_param_t gem_xlate_nh;

    /* Save MC e2iloop nhid to port DB*/
    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &p_uni_port->mc_xlate_nhid));

    /* Create xlate nh */
    sal_memset(&gem_xlate_nh, 0, sizeof(gem_xlate_nh));
    gem_xlate_nh.gport_or_aps_bridge_id = port;
    gem_xlate_nh.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
    gem_xlate_nh.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    gem_xlate_nh.vlan_edit_info.output_svid = CTC_APP_VLAN_PORT_MCAST_TUNNEL_ID;
    gem_xlate_nh.vlan_edit_info.edit_flag = CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID;
    CTC_SET_FLAG(gem_xlate_nh.vlan_edit_info.flag, CTC_VLAN_NH_LENGTH_ADJUST_EN); /*Use Resever offset*/

    CTC_APP_DBG_INFO("nh xlate nhid: %u\n", p_uni_port->mc_xlate_nhid);
    CTC_ERROR_GOTO(ctc_nh_add_xlate(p_uni_port->mc_xlate_nhid, &gem_xlate_nh), ret, roll_back_1);

    return CTC_E_NONE;

    /*-----------------------------------------------------------
    *** rool back
    -----------------------------------------------------------*/
roll_back_1:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_uni_port->mc_xlate_nhid);

    return ret;
}

/*
Each pon port: xlate nhid
*/
int32
_ctc_usw_app_vlan_port_destroy_mc_gem_port(uint8 lchip, uint32 port)
{
    int32 ret = 0;
    uint16 lport = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    lport = CTC_MAP_GPORT_TO_LPORT(port);
    p_uni_port = &p_usw_vlan_port_master->p_port_pon[lport];

    /* Remove  xlate nh */
    CTC_ERROR_RETURN(ctc_nh_remove_xlate(p_uni_port->mc_xlate_nhid));

    CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_free_nhid(lchip, p_uni_port->mc_xlate_nhid));

    p_uni_port->mc_xlate_nhid = 0;

    return ret;
}


int32
ctc_usw_app_vlan_port_create_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    int32 ret = 0;
    uint32 vlan_range_grp = 0;
    uint32 nh_id = 0;
    uint32 mcast_group_id = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_port_scl_property_t scl_prop;
    ctc_opf_t opf;
    ctc_port_restriction_t isolation;
    ctc_port_isolation_t port_isolation;
    ctc_scl_default_action_t def_action;
    ctc_mcast_nh_param_group_t mcast_group;
    ctc_vlan_range_info_t vlan_range;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_UNI_PORT, p_uni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));
    if (0 != p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_EXIST;
    }
    /***********************************************/
    /** Alloc new uni port isolated id*/
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &vlan_range_grp), ret, roll_back_0);
    p_uni_port->vlan_range_grp = vlan_range_grp+1;
    CTC_APP_DBG_INFO("OPF isolated id: %u\n", p_uni_port->vlan_range_grp);

    /***********************************************/
    /** ponit the uni port vdev_id*/
    p_uni_port->vdev_id = p_uni->vdev_id;

    /***********************************************/
    /** Enable scl lookup double vlan for onu and pon qinq */
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
    scl_prop.hash_vlan_range_dis = 1;
    CTC_ERROR_GOTO(ctc_port_set_scl_property(p_uni->port, &scl_prop), ret, roll_back_1);

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
    scl_prop.hash_vlan_range_dis = 1;
    CTC_ERROR_GOTO(ctc_port_set_scl_property(p_uni->port, &scl_prop), ret, roll_back_2);
    CTC_ERROR_GOTO(ctc_port_set_default_vlan(p_uni->port, p_usw_vlan_port_master->default_bcast_fid[p_uni_port->vdev_id]), ret, roll_back_3);

    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = p_uni->port;
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    def_action.action.u.igs_action.fid = p_usw_vlan_port_master->default_bcast_fid[p_uni_port->vdev_id];
    CTC_ERROR_GOTO(ctc_scl_set_default_action(&def_action), ret, roll_back_3);

    /***********************************************/
    /** Enable egress scl lookup double vlan for pon service do vlan xlate*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_GOTO(ctc_port_set_scl_property(p_uni->port, &scl_prop), ret, roll_back_4);

    /***********************************************/
    /** Uni port property*/
    CTC_ERROR_GOTO(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_PORT_EN, 1), ret, roll_back_5);
    CTC_ERROR_GOTO(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE), ret, roll_back_5);
    CTC_ERROR_GOTO(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_LEARNING_EN, 1), ret, roll_back_5);
    CTC_ERROR_GOTO(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0), ret, roll_back_5);
    CTC_ERROR_GOTO(ctc_port_set_property(p_uni->port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1), ret, roll_back_5);

    /***********************************************/
    /** BC/MC PORT property*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_create_mc_gem_port(lchip, p_uni->port), ret, roll_back_5);

    /*Add BC member*/
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.destid = p_uni->port;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
    mcast_group.mem_info.logic_dest_port = p_usw_vlan_port_master->bcast_value_logic_port;
    nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_uni->vdev_id];
    CTC_ERROR_GOTO(ctc_nh_update_mcast(nh_id, &mcast_group), ret, roll_back_6);
    nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(nh_id, &mcast_group);

    /*Add MC member*/
    if (0 == p_usw_vlan_port_master->unknown_mcast_drop_en[p_uni->vdev_id])
    {
        mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 3);
        sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
        mcast_group.mc_grp_id = mcast_group_id;
        mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
        mcast_group.mem_info.ref_nhid = p_uni_port->mc_xlate_nhid;
        mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        nh_id = p_usw_vlan_port_master->default_unknown_mcast_nhid[p_uni->vdev_id];
        CTC_ERROR_GOTO(ctc_nh_update_mcast( nh_id, &mcast_group), ret, roll_back_7);
        nh_id = p_usw_vlan_port_master->glb_unknown_mcast_nhid;
        ctc_nh_update_mcast(nh_id, &mcast_group);
    }

    /* Vlan range enalbe*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_INGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    ctc_vlan_create_vlan_range_group(&vlan_range, TRUE);
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, TRUE);
    p_uni_port->vlan_range_grp = vlan_range_grp;

    /* Egress Vlan range enalbe*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_EGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    ctc_vlan_create_vlan_range_group(&vlan_range, FALSE);
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, TRUE);

    /***********************************************/
    /** PORT ISOLATION*/
    p_uni_port->isolated_id = 1;
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_EGRESS;
    isolation.isolated_id  = p_uni_port->isolated_id;
    isolation.mode = CTC_PORT_RESTRICTION_DISABLE;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_GOTO(ctc_port_set_restriction(p_uni->port, &isolation), ret, roll_back_8);
    isolation.dir = CTC_INGRESS;
    CTC_ERROR_GOTO(ctc_port_set_restriction(p_uni->port, &isolation), ret, roll_back_9);

    sal_memset(&port_isolation, 0, sizeof(ctc_port_isolation_t));
    port_isolation.gport = p_uni_port->isolated_id;
    port_isolation.use_isolation_id = 1;
    port_isolation.pbm[0] = 1<<(p_uni_port->isolated_id);
    port_isolation.isolation_pkt_type = CTC_PORT_ISOLATION_ALL;
    CTC_ERROR_GOTO(ctc_port_set_isolation(lchip, &port_isolation), ret, roll_back_10);

    /***********************************************/
    /** enable uni port mux gem */
    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_enable_gem_port, lchip, p_uni->port, TRUE), ret, roll_back_11);

    /***********************************************/
    /** Return param*/
    p_uni->mc_nhid = p_uni_port->mc_xlate_nhid;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return CTC_E_NONE;

roll_back_11:
    sal_memset(&port_isolation, 0, sizeof(ctc_port_isolation_t));
    port_isolation.gport = p_uni_port->isolated_id;
    port_isolation.use_isolation_id = 0;
    port_isolation.pbm[0] = 0;
    port_isolation.isolation_pkt_type = CTC_PORT_ISOLATION_ALL;
    ctc_port_set_isolation(lchip, &port_isolation);
roll_back_10:
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_INGRESS;
    isolation.isolated_id  = 0;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    ctc_port_set_restriction(p_uni->port, &isolation);

roll_back_9:
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_EGRESS;
    isolation.isolated_id  = 0;
    isolation.mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    ctc_port_set_restriction(p_uni->port, &isolation);

roll_back_8:
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 3);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = p_uni_port->mc_xlate_nhid;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_id = p_usw_vlan_port_master->default_unknown_mcast_nhid[p_uni->vdev_id];
    ctc_nh_update_mcast(nh_id, &mcast_group);
    nh_id = p_usw_vlan_port_master->glb_unknown_mcast_nhid;
    ctc_nh_update_mcast(nh_id, &mcast_group);

    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_EGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, FALSE);
    ctc_vlan_destroy_vlan_range_group(&vlan_range);

    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_INGRESS;
    vlan_range.vrange_grpid = vlan_range_grp;
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, FALSE);
    ctc_vlan_destroy_vlan_range_group(&vlan_range);

roll_back_7:
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.destid = p_uni->port;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
    mcast_group.mem_info.logic_dest_port = p_usw_vlan_port_master->bcast_value_logic_port;
    nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_uni->vdev_id];
    ctc_nh_update_mcast(nh_id, &mcast_group);
    nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(nh_id, &mcast_group);

roll_back_6:
    _ctc_usw_app_vlan_port_destroy_mc_gem_port(lchip, p_uni->port);

roll_back_5:
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);

    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 1);
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);

roll_back_4:
    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = p_uni->port;
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    ctc_scl_set_default_action(&def_action);
    ctc_port_set_default_vlan(p_uni->port, 1);

roll_back_3:
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);

roll_back_2:
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);

roll_back_1:
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    ctc_opf_free_offset(&opf, 1, vlan_range_grp);
    p_uni_port->isolated_id = 0;

roll_back_0:
    CTC_APP_VLAN_PORT_UNLOCK(lchip);
    return ret;
}

int32
ctc_usw_app_vlan_port_destory_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    int32 ret = 0;
    uint32 nh_id = 0;
    uint32 mcast_group_id = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;
    ctc_port_scl_property_t scl_prop;
    ctc_opf_t opf;
    ctc_port_restriction_t isolation;
    ctc_scl_default_action_t def_action;
    ctc_mcast_nh_param_group_t mcast_group;
    ctc_vlan_range_info_t vlan_range;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_UNI_PORT, p_uni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_DBG_ERROR("The Uni port no exist\n");
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    if (0 != p_uni_port->ref_cnt)
    {
        CTC_APP_DBG_ERROR("The Gem port in use, must free all gem ports in the uni port\n");
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_IN_USE;
    }


    /***********************************************/
    /** disable uni port mux gem */
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_enable_gem_port, lchip, p_uni->port, FALSE);

    /***********************************************/
    /** Port isolateion */
    sal_memset(&isolation, 0, sizeof(isolation));
    isolation.dir = CTC_EGRESS;
    isolation.isolated_id  = 0;
    isolation.mode = CTC_PORT_RESTRICTION_DISABLE;
    isolation.type = CTC_PORT_ISOLATION_ALL;
    ctc_port_set_restriction(p_uni->port, &isolation);

    isolation.dir = CTC_INGRESS;
    ctc_port_set_restriction(p_uni->port, &isolation);

    /***********************************************/
    /** Remove bc/mc nhid from vlan */

    /* Remove BC member*/
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.destid = p_uni->port;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
    mcast_group.mem_info.logic_dest_port = p_usw_vlan_port_master->bcast_value_logic_port;
    nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_uni->vdev_id];
    ctc_nh_update_mcast(nh_id, &mcast_group);
    nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(nh_id, &mcast_group);

    /* Remove MC member */
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_uni->vdev_id], 3);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = p_uni_port->mc_xlate_nhid;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    nh_id = p_usw_vlan_port_master->default_unknown_mcast_nhid[p_uni->vdev_id];
    ctc_nh_update_mcast(nh_id, &mcast_group);
    nh_id = p_usw_vlan_port_master->glb_unknown_mcast_nhid;
    ctc_nh_update_mcast(nh_id, &mcast_group);

    _ctc_usw_app_vlan_port_destroy_mc_gem_port(lchip, p_uni->port);

    /***********************************************/
    /** Disable egress scl pon service do vlan xlate */
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_EGRESS;
    scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);

    /***********************************************/
    /** Disable ingress scl onu and pon qinq */
    sal_memset(&def_action, 0, sizeof(def_action));
    def_action.gport = p_uni->port;
    def_action.action.type =  CTC_SCL_ACTION_INGRESS;
    ctc_scl_set_default_action(&def_action);

    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    scl_prop.hash_vlan_range_dis = 0;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = 0;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    scl_prop.hash_vlan_range_dis = 0;
    ctc_port_set_scl_property(p_uni->port, &scl_prop);

    /* Vlan range disable*/
    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_EGRESS;
    vlan_range.vrange_grpid = p_uni_port->vlan_range_grp;
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, FALSE);
    ctc_vlan_destroy_vlan_range_group(&vlan_range);

    sal_memset(&vlan_range, 0, sizeof(vlan_range));
    vlan_range.direction = CTC_INGRESS;
    vlan_range.vrange_grpid = p_uni_port->vlan_range_grp;
    ctc_port_set_vlan_range(p_uni->port, &vlan_range, FALSE);
    ctc_vlan_destroy_vlan_range_group(&vlan_range);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    ctc_opf_free_offset(&opf, 1, p_uni_port->vlan_range_grp);
    p_uni_port->isolated_id = 0;
    /***********************************************/
    /** Untag svlan enable*/
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 1);
    ctc_port_set_property(p_uni->port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);

    sal_memset(p_uni_port, 0, sizeof(ctc_app_vlan_port_uni_db_t));
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_usw_app_vlan_port_get_uni(uint8 lchip, ctc_app_uni_t* p_uni)
{
    int32 ret = 0;
    ctc_app_vlan_port_uni_db_t* p_uni_port = NULL;

    /* CHECK */
    CTC_PTR_VALID_CHECK(p_uni);
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_uni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_UNI_PORT, p_uni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id",      p_uni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port",         p_uni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    CTC_ERROR_RETURN_APP_VLAN_PORT_UNLOCK(_ctc_usw_app_vlan_port_get_uni_port(lchip, p_uni->port, &p_uni_port));

    if (0 == p_uni_port->isolated_id)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    p_uni->vdev_id = p_uni_port->vdev_id;
    p_uni->mc_nhid = p_uni_port->mc_xlate_nhid;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

#define _____NNI_PORT_____ ""

int32
ctc_usw_app_vlan_port_create_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    int32 ret = 0;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint16 mem_count = 0;
    uint8 mem_index = 0;
    uint32 logic_port = 0;
    uint32 nh_id = 0;
    uint32* p_gports = NULL;
    uint32 temp_gport = 0;
    uint32 mcast_nh_id = 0;
    uint32 mcast_group_nh_id = 0;
    uint32 mcast_group_id = 0;
    uint8 new_logic_port = 0;
    ctc_opf_t opf;
    ctc_port_scl_property_t scl_prop;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;
    ctc_scl_default_action_t def_action;
    ctc_mcast_nh_param_group_t mcast_group;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_NNI_PORT, p_nni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check the nni exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_usw_vlan_port_master->nni_port_hash, &nni_port_db);
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
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*max_num);
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

    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_nni->vdev_id])
    {
        /* OPF */
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index= CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_0);
        CTC_APP_DBG_INFO("OPF nni logic port: %u\n", logic_port);
        new_logic_port = 1;
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &mcast_nh_id), ret, roll_back_1);
        sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
        mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 2);
        CTC_ERROR_GOTO(ctc_nh_add_mcast(mcast_nh_id, &mcast_group), ret, roll_back_2);
    }
    else
    {
        logic_port = p_usw_vlan_port_master->nni_logic_port[p_nni->vdev_id];
        mcast_nh_id = p_usw_vlan_port_master->nni_mcast_nhid[p_nni->vdev_id];
    }

    /* linkagg nni port property should set by user */
    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];

        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_NONE), ret, roll_back_3);
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1), ret, roll_back_3);
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0), ret, roll_back_3);
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 1), ret, roll_back_3);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        scl_prop.use_logic_port_en = 1;
        scl_prop.hash_mcast_dis = 1;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(temp_gport, &scl_prop), ret, roll_back_3);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
        scl_prop.use_logic_port_en = 1;
        scl_prop.hash_mcast_dis = 0;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        CTC_ERROR_GOTO(ctc_port_set_scl_property(temp_gport, &scl_prop), ret, roll_back_3);

        /* Set port logic_port */
        CTC_ERROR_GOTO(ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, logic_port), ret, roll_back_3);
        CTC_ERROR_GOTO(ctc_port_set_default_vlan(temp_gport, p_usw_vlan_port_master->default_bcast_fid[p_nni->vdev_id]), ret, roll_back_3);
        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.scl_id = 0;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        CTC_SET_FLAG(def_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
        def_action.action.u.igs_action.fid = p_usw_vlan_port_master->default_bcast_fid[p_nni->vdev_id];
        CTC_ERROR_GOTO(ctc_scl_set_default_action(&def_action), ret, roll_back_3);
        def_action.scl_id = 1;
        CTC_ERROR_GOTO(ctc_scl_set_default_action(&def_action), ret, roll_back_3);

        /*set add volt id*/
        if (p_usw_vlan_port_master->vdev_num >1)
        {
            CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_port_vdev_en, lchip, temp_gport, TRUE);
        }

        if (!p_nni->rx_en)
        {
            CTC_ERROR_GOTO(ctc_port_set_receive_en(temp_gport, FALSE), ret, roll_back_3);
        }
    }

    p_nni->logic_port = logic_port;

    /* Add nni member to nni mcast group */
    CTC_ERROR_GOTO(ctc_nh_get_l2uc(p_nni->port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id), ret, roll_back_4);
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 2);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    CTC_ERROR_GOTO(ctc_nh_update_mcast(mcast_nh_id, &mcast_group), ret, roll_back_4);

    /*Add nni member*/
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    mcast_group_nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_nni->vdev_id];
    CTC_ERROR_GOTO(ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group), ret, roll_back_5);
    mcast_group_nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);

    p_usw_vlan_port_master->nni_logic_port[p_nni->vdev_id] = logic_port;
    p_usw_vlan_port_master->nni_mcast_nhid[p_nni->vdev_id] = mcast_nh_id;

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_offset(lchip, &p_nni_port_db->nni_ad_index), ret, roll_back_6);

    p_nni_port_db->port = p_nni->port;
    p_nni_port_db->vdev_id = p_nni->vdev_id;
    p_nni_port_db->nni_logic_port = logic_port;
    p_nni_port_db->nni_nh_id = nh_id;
    p_nni_port_db->rx_en = p_nni->rx_en;
    ctc_hash_insert(p_usw_vlan_port_master->nni_port_hash, (void*)p_nni_port_db);

    p_usw_vlan_port_master->nni_port_cnt[p_nni->vdev_id]++;
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    mem_free(p_gports);

    return CTC_E_NONE;

roll_back_6:
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    mcast_group_nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_nni->vdev_id];
    ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);
    mcast_group_nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);

roll_back_5:
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 2);
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_update_mcast(mcast_nh_id, &mcast_group);

roll_back_4:
    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];

        ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 1);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 0);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        /* Set port logic_port */
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, 0);

        /* Set nni port scl default action, e2iloop*/
        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.scl_id = 0;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        ctc_scl_set_default_action(&def_action);
        def_action.scl_id = 1;
        ctc_scl_set_default_action(&def_action);
        ctc_port_set_default_vlan(temp_gport, 1);

        /*set add volt id*/
        if (p_usw_vlan_port_master->vdev_num >1)
        {
            CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_port_vdev_en, lchip, temp_gport, FALSE);
        }

        if (!p_nni->rx_en)
        {
            ctc_port_set_receive_en(temp_gport, TRUE);
        }
    }

roll_back_3:
    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_nni->vdev_id])
    {
        ctc_nh_remove_mcast(mcast_nh_id);
    }
roll_back_2:
    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_nni->vdev_id])
    {
        _ctc_usw_app_vlan_port_free_nhid(lchip, mcast_nh_id);
    }
roll_back_1:
    if (new_logic_port)
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
ctc_usw_app_vlan_port_destory_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    int32 ret = CTC_E_NONE;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint16 mem_count = 0;
    uint8 mem_index = 0;
    uint32 nh_id = 0;
    uint32 mcast_group_id = 0;
    uint32 mcast_group_nh_id = 0;
    uint32* p_gports = NULL;
    uint32 temp_gport = 0;
    ctc_opf_t opf;
    ctc_port_scl_property_t scl_prop;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;
    ctc_mcast_nh_param_group_t mcast_group;
    ctc_scl_default_action_t def_action;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_NNI_PORT, p_nni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);
    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_usw_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        CTC_APP_VLAN_PORT_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    _ctc_usw_app_vlan_port_free_offset(lchip, p_nni_port_db->nni_ad_index);

    CTC_APP_DBG_INFO("NNI logic port: %u\n", p_nni_port_db->nni_logic_port);

    ctc_nh_get_l2uc(p_nni->port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id);

    /*Add nni member*/
    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 4);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    mcast_group_nh_id = p_usw_vlan_port_master->default_unknown_bcast_nhid[p_nni->vdev_id];
    ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);
    mcast_group_nh_id = p_usw_vlan_port_master->glb_unknown_bcast_nhid;
    ctc_nh_update_mcast(mcast_group_nh_id, &mcast_group);

    mcast_group_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[p_nni->vdev_id], 2);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = mcast_group_id;
    mcast_group.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_update_mcast(p_usw_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id], &mcast_group);

    ctc_linkagg_get_max_mem_num(&max_num);
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*max_num);
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

    for (mem_index=0; mem_index<mem_count; mem_index++)
    {
        temp_gport = p_gports[mem_index];
        /* Clear port logic_port*/
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT_CHECK_EN, 0);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_LOGIC_PORT, 0);

        /* Clear port cfg*/
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, CTC_PORT_UNTAG_PVID_TYPE_SVLAN);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0);
        ctc_port_set_property(temp_gport, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 1);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = 1;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        ctc_port_set_scl_property(temp_gport, &scl_prop);

        sal_memset(&def_action, 0, sizeof(def_action));
        def_action.gport = temp_gport;
        def_action.scl_id = 0;
        def_action.action.type =  CTC_SCL_ACTION_INGRESS;
        ctc_scl_set_default_action(&def_action);
        def_action.scl_id = 1;
        ctc_scl_set_default_action(&def_action);

        if (p_usw_vlan_port_master->vdev_num >1)
        {
            CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_port_vdev_en, lchip, temp_gport, FALSE);
        }

        if (!p_nni->rx_en)
        {
            ctc_port_set_receive_en(temp_gport, TRUE);
        }
    }

    p_usw_vlan_port_master->nni_port_cnt[p_nni_port_db->vdev_id]--;
    if (0 == p_usw_vlan_port_master->nni_port_cnt[p_nni_port_db->vdev_id])
    {
        /* Free opf offset */
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type = CTC_OPF_VLAN_PORT;
        opf.pool_index= CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
        ctc_opf_free_offset(&opf, 1, p_nni_port_db->nni_logic_port);
        p_usw_vlan_port_master->nni_logic_port[p_nni_port_db->vdev_id] = 0;

        ctc_nh_remove_mcast(p_usw_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id]);
        _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id]);
        p_usw_vlan_port_master->nni_mcast_nhid[p_nni_port_db->vdev_id] = 0;
    }

    /*remove nni port db from hash*/
    ctc_hash_remove(p_usw_vlan_port_master->nni_port_hash, (void*)p_nni_port_db);
    mem_free(p_nni_port_db);
roll_back_0:
    mem_free(p_gports);
    CTC_APP_VLAN_PORT_UNLOCK(lchip);

    return ret;
}

int32
ctc_usw_app_vlan_port_update_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    int32 ret = CTC_E_NONE;
    uint16 max_num = 0;
    uint8 linkagg_id = 0;
    uint16 mem_count = 0;
    uint8 mem_index = 0;
    uint32 value = 0;
    uint32* p_gports = NULL;
    uint32 temp_gport = 0;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_NNI_PORT, p_nni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_usw_vlan_port_master->nni_port_hash, &nni_port_db);
    if (NULL == p_nni_port_db)
    {
        ret =  CTC_E_NOT_EXIST;
        goto roll_back_0;
    }

    ctc_linkagg_get_max_mem_num(&max_num);
    p_gports = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*max_num);
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
ctc_usw_app_vlan_port_get_nni(uint8 lchip, ctc_app_nni_t* p_nni)
{
    uint32 logic_port = 0;
    ctc_app_vlan_port_nni_port_db_t nni_port_db;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_db = NULL;

    /* CHECK */
    CTC_APP_VLAN_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_nni);
    CTC_APP_VLAN_PORT_VDEV_ID_CHECK(p_nni->vdev_id);
    CTC_ERROR_RETURN(_ctc_app_vlan_port_check(lchip, CTC_APP_VLAN_PORT_TYPE_NNI_PORT, p_nni->port, 0));

    /* Debug */
    CTC_APP_DBG_FUNC();
    CTC_APP_DBG_PARAM("-------------------------------------------------\n");
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "vDev id", p_nni->vdev_id);
    CTC_APP_DBG_PARAM("%-40s :%10u\n", "port", p_nni->port);

    CTC_APP_VLAN_PORT_LOCK(lchip);

    /* Check the nni not exitst*/
    sal_memset(&nni_port_db, 0, sizeof(nni_port_db));
    nni_port_db.port = p_nni->port;
    p_nni_port_db = ctc_hash_lookup(p_usw_vlan_port_master->nni_port_hash, &nni_port_db);
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
_ctc_usw_app_vlan_port_sync_gem_port_mac_limit(uint8 lchip, ctc_app_vlan_port_gem_port_db_t* p_gem_port_find, uint8 enable, uint8* need_add)
{
    ctc_app_vlan_port_bpe_gem_port_t bpe_gem_port;

    if (p_gem_port_find)
    {
        *need_add = TRUE;
        sal_memset(&bpe_gem_port, 0, sizeof(ctc_app_vlan_port_bpe_gem_port_t));
        bpe_gem_port.lport = CTC_MAP_GPORT_TO_LPORT(p_gem_port_find->port);
        bpe_gem_port.gem_vlan = p_gem_port_find->tunnel_value;
        bpe_gem_port.logic_port = p_gem_port_find->logic_port;
        bpe_gem_port.pass_through_en = p_gem_port_find->pass_trough_en;
        if (enable && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num))
        {
            *need_add = FALSE;
        }
        else if ((!enable) && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num))
        {
            bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_NONE;
            CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
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
                p_gem_port_find->limit_count--;
            }

            if (enable && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num)
                && (p_gem_port_find->limit_action == CTC_MACLIMIT_ACTION_DISCARD))
            {
                bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_DISCARD;
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
                p_gem_port_find->mac_security = CTC_MACLIMIT_ACTION_DISCARD;
            }
            else if (enable && (p_gem_port_find->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE) && (p_gem_port_find->limit_count == p_gem_port_find->limit_num)
                && (p_gem_port_find->limit_action == CTC_MACLIMIT_ACTION_TOCPU))
            {
                bpe_gem_port.mac_security = CTC_MACLIMIT_ACTION_TOCPU;
                CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_bpe_add_gem_port, lchip, (void*)&bpe_gem_port);
                p_gem_port_find->mac_security = CTC_MACLIMIT_ACTION_TOCPU;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_set_fid_mac_limit_action(void* node, void* user_data)
{
    uint8 limit_action = 0;
    uint8 vdev_id = 0;
    uint16 data = 0;
    uint8 lchip = 0;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = (ctc_app_vlan_port_fid_db_t*)(((ctc_spool_node_t*)node)->data);

    data = *(uint16*)user_data;

    limit_action = data & 0xFF;
    vdev_id = (data>>8)& 0xFF;

    if (p_port_fid_db->vdev_id == vdev_id)
    {
        CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, limit_action);
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_traverse_fid(uint8 limit_action, uint8 vdev_id)
{
    uint16 data = 0;

    data = limit_action | (vdev_id<<8);
    CTC_ERROR_RETURN(ctc_spool_traverse(p_usw_vlan_port_master->fid_spool,
                          (spool_traversal_fn)_ctc_usw_app_vlan_port_set_fid_mac_limit_action, &data));

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_sync_vdev_mac_limit(uint8 lchip, uint8 vdev_id, uint8 enable, uint8* need_add)
{

    *need_add = TRUE;
    if (enable && (p_usw_vlan_port_master->limit_num[vdev_id] != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_usw_vlan_port_master->limit_count[vdev_id] == p_usw_vlan_port_master->limit_num[vdev_id]))
    {
        *need_add = FALSE;
    }
    else if ((!enable) && (p_usw_vlan_port_master->limit_num[vdev_id] != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_usw_vlan_port_master->limit_count[vdev_id] == p_usw_vlan_port_master->limit_num[vdev_id]))
    {
        _ctc_usw_app_vlan_port_traverse_fid(CTC_MACLIMIT_ACTION_NONE, vdev_id);
    }

    if (*need_add)
    {
        if (enable)
        {
            p_usw_vlan_port_master->limit_count[vdev_id]++;
        }
        else
        {
            p_usw_vlan_port_master->limit_count[vdev_id]--;
        }

        if (enable && (p_usw_vlan_port_master->limit_num[vdev_id] != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
            && (p_usw_vlan_port_master->limit_count[vdev_id] == p_usw_vlan_port_master->limit_num[vdev_id]))
        {
            _ctc_usw_app_vlan_port_traverse_fid(p_usw_vlan_port_master->limit_action[vdev_id], vdev_id);
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_ctc_usw_app_vlan_port_sync_fid_mac_limit(uint8 lchip, uint16 fid, uint8 enable, uint8* need_add)
{
    int32 ret = CTC_E_NONE;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));

    *need_add = TRUE;
    port_fid_db.fid = fid;
    ret = _ctc_usw_app_vlan_port_get_fid_by_fid(&port_fid_db);
    if (!ret)
    {
        return CTC_E_NOT_EXIST;
    }

    p_port_fid_db = ctc_spool_lookup(p_usw_vlan_port_master->fid_spool, &port_fid_db);
    if (NULL == p_port_fid_db)
    {
        return CTC_E_NOT_EXIST;
    }

    if (!CTC_IS_BIT_SET(p_usw_vlan_port_master->vlan_glb_vdev[p_port_fid_db->pkt_svlan/32], p_port_fid_db->pkt_svlan%32))
    {
        _ctc_usw_app_vlan_port_sync_vdev_mac_limit(lchip, p_port_fid_db->vdev_id, enable, need_add);
    }

    if (enable && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_port_fid_db->limit_count == p_port_fid_db->limit_num))
    {
        *need_add = FALSE;
    }
    else if ((!enable) && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
        && (p_port_fid_db->limit_count == (p_port_fid_db->limit_num)))
    {
        CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, CTC_MACLIMIT_ACTION_NONE);
    }

    if (*need_add)
    {
        if (enable)
        {
            p_port_fid_db->limit_count++;
        }
        else
        {
            p_port_fid_db->limit_count--;
        }

        if (enable && (p_port_fid_db->limit_num != CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE)
            && (p_port_fid_db->limit_count == p_port_fid_db->limit_num))
        {
            CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action, lchip, p_port_fid_db->fid, p_port_fid_db->limit_action);
        }
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_add_uplink_svlan_entry(uint8 lchip, uint16 port, uint16 pkt_svlan, uint16 pkt_cvlan, uint16 fid)
{
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(fid, port);
    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
    scl_entry.key.u.hash_port_2vlan_key.dir = CTC_INGRESS;
    scl_entry.key.u.hash_port_2vlan_key.gport_type = CTC_SCL_GPROT_TYPE_LOGIC_PORT;
    scl_entry.key.u.hash_port_2vlan_key.gport = port;
    scl_entry.key.u.hash_port_2vlan_key.svlan = pkt_svlan;
    scl_entry.key.u.hash_port_2vlan_key.cvlan = pkt_cvlan;

    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    CTC_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_FID);
    scl_entry.action.u.igs_action.fid = fid;
    ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_2VLAN, &scl_entry);
    if (ret)
    {
        if (ret == CTC_E_EXIST)
        {
            ret = CTC_E_NONE;
        }
        goto roll_back_0;
    }
    CTC_ERROR_GOTO(ctc_scl_install_entry(scl_entry.entry_id), ret, roll_back_0);
    CTC_APP_DBG_INFO("add uplink svlan port %u pkt_svlan %u pkt_cvlan %u fid %u entry_id %u\n", port, pkt_svlan, pkt_cvlan, fid, scl_entry.entry_id);

    return CTC_E_NONE;
roll_back_0:
    ctc_scl_remove_entry(scl_entry.entry_id);

    return ret;
}

int32
_ctc_usw_app_vlan_port_remove_uplink_svlan_entry(uint8 lchip, uint16 port, uint16 fid)
{
    uint32 entry_id = 0;

    entry_id = ENCODE_SCL_UPLINK_ENTRY_ID(fid, port);
    ctc_scl_uninstall_entry(entry_id);
    ctc_scl_remove_entry(entry_id);
    CTC_APP_DBG_INFO("remove uplink port %u fid %u entry_id %u\n", port, fid, entry_id);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_add_fdb_learn_with_def_fid(uint8 lchip, uint16 vdev_id, uint16 pkt_svlan, uint16 pkt_cvlan, uint16 logic_port, uint16* p_fid)
{
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_app_vlan_port_fid_db_t* p_port_fid_db = NULL;

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = vdev_id;
    port_fid_db.pkt_svlan = pkt_svlan;
    port_fid_db.pkt_cvlan = pkt_cvlan;
    port_fid_db.is_pass_through = 1;
    CTC_ERROR_RETURN(ctc_spool_add(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL, &p_port_fid_db));
    p_port_fid_db->is_pass_through = 1;

    if (logic_port != p_usw_vlan_port_master->nni_logic_port[vdev_id])
    {
        _ctc_usw_app_vlan_port_add_uplink_svlan_entry(lchip, logic_port, pkt_svlan, pkt_cvlan, p_port_fid_db->fid);
    }

    *p_fid = p_port_fid_db->fid;

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_remove_fdb_learn_with_def_fid(uint8 lchip, uint16 logic_port, ctc_app_vlan_port_fid_db_t* p_port_fid_db)
{
    ctc_app_vlan_port_fid_db_t port_fid_db;

    sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
    port_fid_db.vdev_id = p_port_fid_db->vdev_id;
    port_fid_db.pkt_svlan = p_port_fid_db->pkt_svlan;
    port_fid_db.pkt_cvlan = p_port_fid_db->pkt_cvlan;
    ctc_spool_remove(p_usw_vlan_port_master->fid_spool, &port_fid_db, NULL);

    if (logic_port != p_usw_vlan_port_master->nni_logic_port[p_port_fid_db->vdev_id])
    {
        _ctc_usw_app_vlan_port_remove_uplink_svlan_entry(lchip, logic_port, p_port_fid_db->fid);
    }

    return CTC_E_NONE;
}

int32
_ctc_usw_app_vlan_port_hw_learning_sync(uint8 gchip, void* p_data)
{
    uint8 lchip = 0;
    uint8 need_fid_add = 0;
    uint8 need_gem_port_add = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    uint32 index = 0;
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;
    uint32 ad_index;
    ctc_learning_cache_t* p_cache = (ctc_learning_cache_t*)p_data;
    ctc_l2_addr_t l2_addr;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_find = NULL;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;
    ctc_app_vlan_port_nni_port_db_t* p_nni_port_find = NULL;
    uint16 fid_new = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

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

            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add fdb, logic_port:%u, svlan:%u, cvlan:%u \n",
                            l2_addr.gport,
                            pkt_svlan,
                            pkt_cvlan);

            /*ONU*/
            p_vlan_port_db = _ctc_usw_app_vlan_port_find_vlan_port_db(lchip, l2_addr.gport);
            if ((NULL != p_vlan_port_db) && (((pkt_svlan == p_vlan_port_db->match_svlan)
                && (pkt_cvlan == p_vlan_port_db->match_cvlan))
                || ((p_vlan_port_db->match_svlan_end != 0)        /*ONU Vlan Range*/
                && ( pkt_svlan >= p_vlan_port_db->match_svlan)
                && (pkt_svlan <= p_vlan_port_db->match_svlan_end))
                || ((p_vlan_port_db->criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX)
                && (p_vlan_port_db->fid == l2_addr.fid))))
            {
                if (l2_addr.fid != p_usw_vlan_port_master->default_bcast_fid[p_vlan_port_db->vdev_id])
                {
                    nhid =  p_vlan_port_db->xlate_nhid;
                    ad_index = p_vlan_port_db->ad_index;
                    /*mask_valid represent the need set logic_dest_port_valid*/
                    l2_addr.mask_valid = 1;
                    l2_addr.assign_port = p_vlan_port_db->egress_policer_id;

                    _ctc_usw_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_db->p_gem_port_db, TRUE, &need_gem_port_add);

                    _ctc_usw_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                    if (need_fid_add && need_gem_port_add)
                    {
                        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port find nhid:%u, ad_index:%u  !!!\n", nhid, ad_index);
                        CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                    }
                    else
                    {
                        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Vlan port mac limit fid %u gem_port %u!!!\n", need_fid_add, need_gem_port_add);
                    }
                    continue;
                }
            }

            /*PON*/
            p_gem_port_find = _ctc_usw_app_vlan_port_find_gem_port_db(lchip, l2_addr.gport);
            if (p_gem_port_find)
            {
                if (l2_addr.fid == p_usw_vlan_port_master->default_bcast_fid[p_gem_port_find->vdev_id])
                {
                    _ctc_usw_app_vlan_port_add_fdb_learn_with_def_fid(lchip, p_gem_port_find->vdev_id, pkt_svlan, pkt_cvlan,  l2_addr.gport, &fid_new);
                    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
                    l2_addr.fid = fid_new;
                }

                nhid = p_gem_port_find->pon_downlink_nhid;
                ad_index = p_gem_port_find->ad_index;
                l2_addr.mask_valid = 1;
                l2_addr.cid = 1; /*cid represent pon*/

                _ctc_usw_app_vlan_port_sync_gem_port_mac_limit(lchip, p_gem_port_find, TRUE, &need_gem_port_add);
                _ctc_usw_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                if (need_fid_add && need_gem_port_add)
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "GemPort find nhid:%d  ad_index:%d  !!!\n", nhid, ad_index);
                    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                }
                else
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "GemPort find mac limit fid %u gem_port %u!!!\n", need_fid_add, need_gem_port_add);
                }
                continue;
            }

            /*NNI port*/
            p_nni_port_find = _ctc_usw_app_vlan_port_find_nni_port(lchip, l2_addr.gport);
            if (p_nni_port_find)
            {
                if (l2_addr.fid == p_usw_vlan_port_master->default_bcast_fid[p_nni_port_find->vdev_id])
                {
                    _ctc_usw_app_vlan_port_add_fdb_learn_with_def_fid(lchip, p_nni_port_find->vdev_id, pkt_svlan, pkt_cvlan,  l2_addr.gport, &fid_new);
                    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
                    l2_addr.fid = fid_new;
                }

                CTC_APP_DBG_INFO("p_usw_vlan_port_master->nni_logic_port:%u\n", p_nni_port_find->nni_logic_port);
                nhid = p_usw_vlan_port_master->nni_mcast_nhid[p_nni_port_find->vdev_id];
                ad_index = p_nni_port_find->nni_ad_index;
                _ctc_usw_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, TRUE, &need_fid_add);
                if (need_fid_add)
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NNI find nhid:%u ad_index:%u  !!!\n", nhid, ad_index);
                    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_add_db, lchip, &l2_addr, nhid, ad_index);
                }
                else
                {
                    CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "NNI find mac limit fid %u!!!\n", need_fid_add);
                }
                continue;
            }
        }
    }

    return ret;
}

int32
_ctc_usw_app_vlan_port_hw_aging_sync(uint8 gchip, void* p_data)
{
    ctc_aging_fifo_info_t* p_fifo = (ctc_aging_fifo_info_t*)p_data;
    uint8 need_fid_add = 0;
    uint8 need_gem_port_add = 0;
    uint32 i = 0;
    int32 ret = CTC_E_NONE;
    ctc_l2_addr_t l2_addr;
    ctc_l2_addr_t l2_addr_rst;
    uint8 lchip = 0;
    ctc_app_vlan_port_gem_port_db_t* p_gem_port_find = NULL;
    ctc_app_vlan_port_fid_db_t port_fid_db;
    ctc_l2_fdb_query_t fdb_query;
    ctc_l2_fdb_query_rst_t fdb_rst;
    ctc_app_vlan_port_db_t* p_vlan_port_db = NULL;

    sal_memset(&l2_addr_rst, 0, sizeof(ctc_l2_addr_t));

    ctc_app_index_get_lchip_id(gchip, &lchip);
    /*Using Dma*/
    for (i = 0; i < p_fifo->fifo_idx_num; i++)
    {
        ctc_aging_info_entry_t* p_entry = NULL;

        sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

        p_entry = &(p_fifo->aging_entry[i]);
        l2_addr.fid = p_entry->fid;
        sal_memcpy(l2_addr.mac, p_entry->mac, sizeof(mac_addr_t));

        CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_REMOVE_DYNAMIC);
        sal_memset(&fdb_query, 0, sizeof(ctc_l2_fdb_query_t));
        sal_memset(&fdb_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));
        fdb_query.fid = l2_addr.fid;
        sal_memcpy(fdb_query.mac, p_entry->mac, sizeof(mac_addr_t));
        fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        fdb_query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
        fdb_rst.buffer_len = sizeof(ctc_l2_addr_t);
        fdb_rst.buffer = &l2_addr_rst;
        ctc_l2_get_fdb_entry(&fdb_query, &fdb_rst);

        ret = CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_l2_hw_sync_remove_db, lchip, &l2_addr);
        l2_addr.gport = fdb_rst.buffer[0].gport;
        p_vlan_port_db = _ctc_usw_app_vlan_port_find_vlan_port_db(lchip, l2_addr.gport);
        if (p_vlan_port_db)
        {
            _ctc_usw_app_vlan_port_sync_gem_port_mac_limit(lchip, p_vlan_port_db->p_gem_port_db, FALSE, &need_gem_port_add);
        }
        else
        {
            p_gem_port_find = _ctc_usw_app_vlan_port_find_gem_port_db(lchip, l2_addr.gport);
            if (p_gem_port_find)
            {
                _ctc_usw_app_vlan_port_sync_gem_port_mac_limit(lchip, p_gem_port_find, FALSE, &need_gem_port_add);
            }
        }
        _ctc_usw_app_vlan_port_sync_fid_mac_limit(lchip, l2_addr.fid, FALSE, &need_fid_add);

        sal_memset(&port_fid_db, 0, sizeof(ctc_app_vlan_port_fid_db_t));
        port_fid_db.fid = l2_addr.fid;
        ret = _ctc_usw_app_vlan_port_get_fid_by_fid(&port_fid_db);
        if (ret && port_fid_db.is_pass_through)
        {
            sal_memset(&fdb_query, 0, sizeof(ctc_l2_fdb_query_t));
            fdb_query.fid = l2_addr.fid;
            fdb_query.gport = l2_addr.gport;
            fdb_query.use_logic_port = 1;
            fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
            fdb_query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
            ctc_l2_get_fdb_count(&fdb_query);
            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "fid: %d, logic port: %d \n", fdb_query.fid, fdb_query.gport);

            if (0 == fdb_query.count)
            {
                _ctc_usw_app_vlan_port_remove_fdb_learn_with_def_fid(lchip, l2_addr.gport, &port_fid_db);
            }
        }

        CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Aging Sync: MAC:%.4x.%.4x.%.4x  FID:%-5u\n",
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                        sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                        l2_addr.fid);

        if (ret)
        {
            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Aging fdb fail, ret:%u \n", ret);
        }
    }

    return CTC_E_NONE;
}

#define _______INIT________
STATIC int32
_ctc_usw_app_vlan_port_free_db(uint8 lchip)
{
    mem_free(p_usw_vlan_port_master->p_port_pon);

    ctc_spool_free(p_usw_vlan_port_master->fid_spool);

    ctc_hash_free(p_usw_vlan_port_master->vlan_port_key_hash);

    ctc_hash_free(p_usw_vlan_port_master->vlan_port_hash);

    ctc_hash_free(p_usw_vlan_port_master->gem_port_hash);

    ctc_hash_free(p_usw_vlan_port_master->nni_port_hash);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_init_db(uint8 lchip)
{
    ctc_spool_t spool;
    CTC_PTR_VALID_CHECK(p_usw_vlan_port_master);

    p_usw_vlan_port_master->nni_port_hash = ctc_hash_create(1,20,
                                                              (hash_key_fn) _ctc_usw_app_vlan_port_hash_nni_port_make,
                                                              (hash_cmp_fn) _ctc_usw_app_vlan_port_hash_nni_port_cmp);

    p_usw_vlan_port_master->gem_port_hash = ctc_hash_create(10,100,
                                                              (hash_key_fn) _ctc_usw_app_vlan_port_hash_gem_port_make,
                                                              (hash_cmp_fn) _ctc_usw_app_vlan_port_hash_gem_port_cmp);

    p_usw_vlan_port_master->vlan_port_hash = ctc_hash_create(10,100,
                                                               (hash_key_fn) _ctc_usw_app_vlan_port_hash_make,
                                                               (hash_cmp_fn) _ctc_usw_app_vlan_port_hash_cmp);

    p_usw_vlan_port_master->vlan_port_key_hash = ctc_hash_create(10,100,
                                                               (hash_key_fn) _ctc_usw_app_vlan_port_key_hash_make,
                                                               (hash_cmp_fn) _ctc_usw_app_vlan_port_key_hash_cmp);

    if ((NULL == p_usw_vlan_port_master->nni_port_hash) || (NULL == p_usw_vlan_port_master->gem_port_hash)
        || (NULL == p_usw_vlan_port_master->vlan_port_hash) || (NULL == p_usw_vlan_port_master->vlan_port_key_hash))
    {
        goto roll_back_0;
    }

    sal_memset(&spool, 0, sizeof(ctc_spool_t));

    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = 4*1024;
    spool.max_count = 4*1024;
    spool.user_data_size = sizeof(ctc_app_vlan_port_fid_db_t);
    spool.spool_key = (hash_key_fn)_ctc_usw_app_vlan_port_fid_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_ctc_usw_app_vlan_port_fid_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_ctc_usw_app_vlan_port_fid_build_index;
    spool.spool_free = (spool_free_fn)_ctc_usw_app_vlan_port_fid_free_index;
    p_usw_vlan_port_master->fid_spool = ctc_spool_create(&spool);
    if (NULL == p_usw_vlan_port_master->vlan_port_hash)
    {
        goto roll_back_0;
    }

    p_usw_vlan_port_master->p_port_pon = (ctc_app_vlan_port_uni_db_t*)mem_malloc(MEM_SYSTEM_MODULE, 512*sizeof(ctc_app_vlan_port_uni_db_t));
    if (NULL == p_usw_vlan_port_master->p_port_pon)
    {
        goto roll_back_1;
    }

    sal_memset( p_usw_vlan_port_master->p_port_pon, 0, 512*sizeof(ctc_app_vlan_port_uni_db_t));

    /*default all vlan support slice*/
    sal_memset(p_usw_vlan_port_master->vlan_glb_vdev, 0, sizeof(uint32)*CTC_APP_VLAN_PORT_VLAN_BITMAP);

    return CTC_E_NONE;

roll_back_1:
    ctc_spool_free(p_usw_vlan_port_master->fid_spool);

roll_back_0:
    if (p_usw_vlan_port_master->vlan_port_key_hash)
    {
        ctc_hash_free(p_usw_vlan_port_master->vlan_port_key_hash);
    }

    if (p_usw_vlan_port_master->vlan_port_hash)
    {
        ctc_hash_free(p_usw_vlan_port_master->vlan_port_hash);
    }

    if (p_usw_vlan_port_master->gem_port_hash)
    {
        ctc_hash_free(p_usw_vlan_port_master->gem_port_hash);
    }

    if (p_usw_vlan_port_master->nni_port_hash)
    {
        ctc_hash_free(p_usw_vlan_port_master->nni_port_hash);
    }

    return CTC_E_NO_MEMORY;
}

STATIC int32
_ctc_usw_app_vlan_port_free_opf(uint8 lchip)
{
    uint8 index = 0;

    for (index=0; index<CTC_APP_VLAN_PORT_OPF_MAX; index++)
    {
        ctc_opf_free(CTC_OPF_VLAN_PORT, index);
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_init_opf(uint8 lchip, uint16 vdev_num)
{
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    ctc_opf_t opf;

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    ctc_opf_init(CTC_OPF_VLAN_PORT, CTC_APP_VLAN_PORT_OPF_MAX);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    ctc_opf_init_offset(&opf, 1, capability[CTC_GLOBAL_CAPABILITY_LOGIC_PORT_NUM]-1);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT_ID;
    ctc_opf_init_offset(&opf, 1, 64*1024);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FLEX_LOGIC_VLAN;
    ctc_opf_init_offset(&opf, 1, 4095);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_VLAN_RANGE_GRP;
    ctc_opf_init_offset(&opf, 0, 16);

    opf.pool_index = CTC_APP_VLAN_PORT_OPF_FID;
    ctc_opf_init_offset(&opf, vdev_num+1, capability[CTC_GLOBAL_CAPABILITY_MAX_FID]);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_free_port(uint8 lchip, uint8 vdev_index)
{
    uint32 nh_id = 0;
    ctc_l2dflt_addr_t l2dflt;

    ctc_scl_destroy_group(CTC_APP_VLAN_PORT_SCL_GROUP_ID+vdev_index);

    ctc_nh_get_mcast_nh(ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[vdev_index], 1), &nh_id);
    ctc_nh_remove_mcast(nh_id);

    _ctc_usw_app_vlan_port_free_nhid(lchip, nh_id);

    ctc_nh_remove_mcast(p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index]);

    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index]);

    ctc_nh_remove_mcast(p_usw_vlan_port_master->default_unknown_bcast_nhid[vdev_index]);

    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->default_unknown_bcast_nhid[vdev_index]);


    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = p_usw_vlan_port_master->default_bcast_fid[vdev_index];
    l2dflt.l2mc_grp_id = ENCODE_MCAST_GROUP_ID(p_usw_vlan_port_master->default_mcast_group_id[vdev_index], 0);
    ctc_l2_remove_default_entry(&l2dflt);

    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 5, p_usw_vlan_port_master->default_mcast_group_id[vdev_index]);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_free_global_vdev(uint8 lchip, uint32 mcast_id)
{
    ctc_nh_remove_mcast(p_usw_vlan_port_master->glb_unknown_mcast_nhid);

    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->glb_unknown_mcast_nhid);

    ctc_nh_remove_mcast(p_usw_vlan_port_master->glb_unknown_bcast_nhid);

    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->glb_unknown_bcast_nhid);

    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 2, mcast_id);

    return CTC_E_NONE;
}

STATIC int32
_ctc_usw_app_vlan_port_init_global_vdev(uint8 lchip, uint32* p_mcast_id)
{
    uint32 nh_profile_id = 0;
    uint32 mcast_id = 0;
    ctc_mcast_nh_param_group_t mcast_group;
    int32 ret = CTC_E_NONE;
    ctc_opf_t opf;
    uint32 logic_port = 0;

    CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_alloc_mcast_group_id(lchip, 2, &mcast_id));

    /*alloc unknown ucast profile*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_profile_id), ret, roll_back_0);
    p_usw_vlan_port_master->glb_unknown_bcast_nhid = nh_profile_id;
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.is_profile = 1;
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_profile_id, &mcast_group), ret, roll_back_1);

    /*alloc unknown mcast profile*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_profile_id), ret, roll_back_2);
    p_usw_vlan_port_master->glb_unknown_mcast_nhid = nh_profile_id;
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.is_profile = 1;
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 1);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_profile_id, &mcast_group), ret, roll_back_3);

    *p_mcast_id = mcast_id;

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    CTC_ERROR_GOTO(ctc_opf_alloc_offset(&opf, 1, &logic_port), ret, roll_back_4);

    /* Create xlate nh for leaning */
    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_add_gem_port, lchip, logic_port, CTC_APP_VLAN_PORT_BCAST_TUNNEL_ID), ret, roll_back_5);
    p_usw_vlan_port_master->bcast_value_logic_port = logic_port;
    return CTC_E_NONE;

roll_back_5:
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_VLAN_PORT;
    opf.pool_index = CTC_APP_VLAN_PORT_OPF_LOGIC_PORT;
    ctc_opf_free_offset(&opf, 1, logic_port);

roll_back_4:
    ctc_nh_remove_mcast(nh_profile_id);

roll_back_3:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->glb_unknown_mcast_nhid);

roll_back_2:
    ctc_nh_remove_mcast(p_usw_vlan_port_master->glb_unknown_bcast_nhid);

roll_back_1:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->glb_unknown_bcast_nhid);

roll_back_0:
    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 2, mcast_id);

    return ret;
}


STATIC int32
_ctc_usw_app_vlan_port_init_port(uint8 lchip, uint8 vdev_index)
{
    uint32 fid = 0;
    uint32 nh_id = 0;
    uint32 nh_profile_id = 0;
    ctc_l2dflt_addr_t l2dflt;
    ctc_mcast_nh_param_group_t mcast_group;
    uint32 mcast_id = 0;
    ctc_scl_group_info_t group_info;
    int32 ret = CTC_E_NONE;

    fid = CTC_APP_VLAN_PORT_RSV_MCAST_GROUP_ID(vdev_index);
    p_usw_vlan_port_master->default_bcast_fid[vdev_index] = fid;

    CTC_ERROR_RETURN(_ctc_usw_app_vlan_port_alloc_mcast_group_id(lchip, 5, &mcast_id));
    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    CTC_SET_FLAG(l2dflt.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);
    CTC_ERROR_GOTO(ctc_l2_add_default_entry(&l2dflt), ret, roll_back_0);

    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_vlan_set_fid_mapping_vdev_vlan, lchip, fid, p_usw_vlan_port_master->vdev_base_vlan+vdev_index);

    /*alloc unknown ucast profile*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_profile_id), ret, roll_back_1);
    p_usw_vlan_port_master->default_unknown_bcast_nhid[vdev_index] = nh_profile_id;
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.is_profile = 1;
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 4);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_profile_id, &mcast_group), ret, roll_back_2);

    /* add unknown ucast profile to default unknown mcast */
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 0);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_profile_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_get_mcast_nh(ENCODE_MCAST_GROUP_ID(mcast_id, 0), &nh_id);
    ctc_nh_update_mcast(nh_id, &mcast_group);

    /*alloc unknown mcast profile*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_profile_id), ret, roll_back_3);
    p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index] = nh_profile_id;
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.is_profile = 1;
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 3);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_profile_id, &mcast_group), ret, roll_back_4);

    /* Alloc default unknown mcast nhid */
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_nhid(lchip, &nh_id), ret, roll_back_5);
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 1);
    CTC_ERROR_GOTO(ctc_nh_add_mcast(nh_id, &mcast_group), ret, roll_back_6);
    p_usw_vlan_port_master->default_mcast_group_id[vdev_index] = mcast_id;
    ctc_l2_set_fid_property(fid, CTC_L2_FID_PROP_BDING_MCAST_GROUP, ENCODE_MCAST_GROUP_ID(mcast_id, 0));

    /* add unknown mcast profile to default unknown mcast */
    sal_memset(&mcast_group, 0, sizeof(ctc_mcast_nh_param_group_t));
    mcast_group.mc_grp_id = ENCODE_MCAST_GROUP_ID(mcast_id, 1);
    mcast_group.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    mcast_group.mem_info.ref_nhid = nh_profile_id;
    mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
    ctc_nh_update_mcast(nh_id, &mcast_group);

    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.type = CTC_SCL_GROUP_TYPE_NONE;
    /* use default priority for inter module */
    group_info.priority = 1;
    CTC_ERROR_GOTO(ctc_scl_create_group(CTC_APP_VLAN_PORT_SCL_GROUP_ID+vdev_index, &group_info), ret, roll_back_7);

    p_usw_vlan_port_master->limit_num[vdev_index] = CTC_APP_VLAN_PORT_LIMIT_NUM_DISABLE;

    return CTC_E_NONE;

roll_back_7:
    ctc_nh_remove_mcast(nh_id);

roll_back_6:
    _ctc_usw_app_vlan_port_free_nhid(lchip, nh_id);

roll_back_5:
    ctc_nh_remove_mcast(p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index]);

roll_back_4:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->default_unknown_mcast_nhid[vdev_index]);

roll_back_3:
    ctc_nh_remove_mcast(p_usw_vlan_port_master->default_unknown_bcast_nhid[vdev_index]);

roll_back_2:
    _ctc_usw_app_vlan_port_free_nhid(lchip, p_usw_vlan_port_master->default_unknown_bcast_nhid[vdev_index]);

roll_back_1:
    ctc_l2_remove_default_entry(&l2dflt);

roll_back_0:
    _ctc_usw_app_vlan_port_free_mcast_group_id(lchip, 5, mcast_id);

    return ret;
}

int32
ctc_usw_app_vlan_port_init(uint8 lchip, void* p_param)
{
    uint8 gchip = 0;
    uint8 index = 0;
    uint16 vdev_index = 0;
    uint16 vdev_num = 0;
    uint16 vdev_base_vlan = 0;
    uint32 mcast_id = 0;
    ctc_app_vlan_port_init_cfg_t* p_app_init = NULL;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] ={0};
    int32 ret = CTC_E_NONE;

    if (NULL != p_usw_vlan_port_master)
    {
         return CTC_E_INIT_FAIL;
    }

    p_app_init = (ctc_app_vlan_port_init_cfg_t*)p_param;
    if(NULL == p_app_init)
    {
        vdev_num = 1;
        vdev_base_vlan = 0;
    }
    else
    {
        CTC_APP_VLAN_PORT_VLAN_ID_CHECK(p_app_init->vdev_base_vlan + p_app_init->vdev_num);
        if ((p_app_init->vdev_num == 1 && p_app_init->vdev_base_vlan) || (p_app_init->vdev_num > 1 && p_app_init->vdev_base_vlan == 0))
        {
            CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "%s\n", \
            p_app_init->vdev_num == 1? "Vdev base vlan not support when vdev num is 1 .": "Vdev base vlan can not be 0 when vdev num is larger than 1");
            return CTC_E_INVALID_PARAM;
        }
        vdev_num = p_app_init->vdev_num;
        vdev_base_vlan = p_app_init->vdev_base_vlan;
    }
    if ((vdev_num > CTC_APP_VLAN_PORT_MAX_VDEV_NUM) || (vdev_num == 0))
    {
        return CTC_E_INVALID_PARAM;
    }

    MALLOC_POINTER(ctc_app_vlan_port_master_t, p_usw_vlan_port_master);
    if (NULL == p_usw_vlan_port_master)
    {
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_vlan_port_master, 0, sizeof(ctc_app_vlan_port_master_t));
    p_usw_vlan_port_master->vdev_num = vdev_num;
    p_usw_vlan_port_master->vdev_base_vlan = vdev_base_vlan;

    p_usw_vlan_port_master->ctc_global_set_xgpon_en = sys_usw_global_set_xgpon_en;
    p_usw_vlan_port_master->ctc_port_set_service_policer_en = NULL;
    p_usw_vlan_port_master->ctc_nh_set_xgpon_en = sys_usw_nh_set_xgpon_en;
    p_usw_vlan_port_master->ctc_port_set_global_port = NULL;
    p_usw_vlan_port_master->ctc_nh_update_logic_port = sys_usw_nh_update_logic_port;
    p_usw_vlan_port_master->ctc_qos_policer_index_get = sys_usw_qos_policer_id_check;
    p_usw_vlan_port_master->ctc_qos_policer_reserv_service_hbwp = NULL;
    p_usw_vlan_port_master->ctc_nh_set_mcast_bitmap_ptr = sys_usw_nh_set_mcast_bitmap_ptr;
    p_usw_vlan_port_master->ctc_l2_hw_sync_add_db = sys_usw_l2_hw_sync_add_db;
    p_usw_vlan_port_master->ctc_l2_hw_sync_remove_db = sys_usw_l2_hw_sync_remove_db;
    p_usw_vlan_port_master->ctc_l2_hw_sync_alloc_ad_idx = sys_usw_l2_hw_sync_alloc_ad_idx;
    p_usw_vlan_port_master->ctc_l2_hw_sync_free_ad_idx = sys_usw_l2_hw_sync_free_ad_idx;
    p_usw_vlan_port_master->ctc_ftm_alloc_table_offset_from_position = sys_usw_ftm_alloc_table_offset_from_position;
    p_usw_vlan_port_master->ctc_bpe_enable_gem_port = sys_usw_bpe_enable_gem_port;
    p_usw_vlan_port_master->ctc_bpe_add_gem_port = sys_usw_bpe_add_gem_port;
    p_usw_vlan_port_master->ctc_bpe_remove_gem_port = sys_usw_bpe_remove_gem_port;
    p_usw_vlan_port_master->ctc_nh_add_gem_port = sys_usw_nh_add_gem_port_l2edit_8w_outer;
    p_usw_vlan_port_master->ctc_nh_remove_gem_port = sys_usw_nh_remove_gem_port_l2edit_8w_outer;
    p_usw_vlan_port_master->ctc_mac_security_set_fid_learn_limit_action = sys_usw_mac_security_set_fid_learn_limit_action;
    p_usw_vlan_port_master->ctc_vlan_set_fid_mapping_vdev_vlan = sys_usw_vlan_set_fid_mapping_vdev_vlan;
    p_usw_vlan_port_master->ctc_bpe_port_vdev_en = sys_usw_bpe_port_vdev_en;
    /*create mutex*/
    CTC_APP_VLAN_PORT_CREAT_LOCK(lchip);

    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_global_set_xgpon_en, lchip, TRUE), ret, roll_back_0);
    CTC_ERROR_GOTO(CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_set_xgpon_en, lchip, TRUE), ret, roll_back_1);

    CTC_ERROR_GOTO(ctc_get_gchip_id(lchip, &gchip), ret, roll_back_2);

    ctc_app_index_init(gchip);

    CTC_ERROR_GOTO(ctc_app_index_set_chipset_type(gchip, CTC_APP_CHIPSET_CTC5160), ret, roll_back_2);

    CTC_ERROR_GOTO(ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability), ret, roll_back_2);
    p_usw_vlan_port_master->mcast_max_group_num = capability[CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM];

    /*------------DB------------*/
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_init_db(lchip), ret, roll_back_2);

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_init_opf(lchip, vdev_num), ret, roll_back_3);

    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_init_global_vdev(lchip, &mcast_id), ret, roll_back_4);

    for (vdev_index=0; vdev_index<vdev_num; vdev_index++)
    {
        CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_init_port(lchip, vdev_index), ret, roll_back_5);
        p_usw_vlan_port_master->uni_inner_isolate_en[vdev_index] = 1;
    }
    CTC_ERROR_GOTO(_ctc_usw_app_vlan_port_alloc_mcast_group_id(lchip, CTC_APP_VLAN_PORT_MAX_RSV_MC_GROUP_ID, &mcast_id), ret, roll_back_6);

    ctc_interrupt_register_event_cb(0, _ctc_usw_app_vlan_port_hw_learning_sync);
    ctc_interrupt_register_event_cb(1, _ctc_usw_app_vlan_port_hw_aging_sync);

    return CTC_E_NONE;

roll_back_6:
    for (index = 0; index < vdev_index; index++)
    {
        _ctc_usw_app_vlan_port_free_port(lchip, index);
    }

roll_back_5:
    _ctc_usw_app_vlan_port_free_global_vdev(lchip, mcast_id);

roll_back_4:
    _ctc_usw_app_vlan_port_free_opf(lchip);

roll_back_3:
    _ctc_usw_app_vlan_port_free_db(lchip);

roll_back_2:
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_nh_set_xgpon_en, lchip, FALSE);

roll_back_1:
    CTC_APP_VLAN_PORT_API_CALL(p_usw_vlan_port_master->ctc_global_set_xgpon_en, lchip, FALSE);

roll_back_0:
    FREE_POINTER(p_usw_vlan_port_master);

    return ret;
}

#endif

