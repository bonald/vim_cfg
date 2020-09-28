/**
   @file sys_usw_l2_fdb.c

   @date 2013-6-13

   @version v2.0

   The file implement   FDB/L2 unicast /mac filtering /mac security/L2 mcast functions
 */

#include "sal.h"
#include "ctc_const.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_error.h"



#include "ctc_l2.h"
#include "ctc_pdu.h"
#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_port.h"
#include "sys_usw_opf.h"
#include "sys_usw_ftm.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_learning_aging.h"
#include "sys_usw_aps.h"
#include "sys_usw_dma.h"
#include "sys_usw_vlan.h"
#include "sys_usw_stp.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_fdb_sort.h"
#include "drv_api.h"


#define FAIL(r)                 ((r) < 0)
#define FDB_ERR_RET_UL(r)       CTC_ERROR_RETURN_WITH_UNLOCK((r), pl2_usw_master[lchip]->l2_mutex)
#define FDB_SET_MAC(d, s)       sal_memcpy(d, s, sizeof(mac_addr_t))
#define FDB_SET_HW_MAC(d, s)    SYS_USW_SET_HW_MAC(d, s)
#define HASH_INDEX(index)    (index)
#define SYS_FDB_DISCARD_FWD_PTR    0
#define DISCARD_PORT               0xFFFF

#define SYS_L2_FDB_FLUSH_COUNT_SET(flush_fdb_cnt_per_loop)  \
    {                                                       \
        flush_fdb_cnt_per_loop = pl2_usw_master[lchip]->cfg_flush_cnt; \
    }

#define SYS_L2_FDB_FLUSH_COUNT_PROCESS(flush_fdb_cnt_per_loop) \
    {                                                          \
        if ((flush_fdb_cnt_per_loop) > 0)                      \
        {                                                      \
            (flush_fdb_cnt_per_loop)--;                        \
            if (0 == (flush_fdb_cnt_per_loop))                 \
            {                                                  \
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [FDB] Need to next flush \n");\
			return CTC_E_INVALID_CONFIG;\
              \
            }                                                  \
        }                                                      \
    }

#define SYS_FDB_DBG_OUT(level, FMT, ...) \
    CTC_DEBUG_OUT(l2, fdb, L2_FDB_SYS, level, FMT, ## __VA_ARGS__)

#define SYS_L2_DUMP_SYS_INFO(psys)                                        \
    {                                                                     \
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "@@@@@ MAC[%s]  FID[%d]\n"                       \
                         "     gport:0x%x,   key_index:0x%x\n" \
                         "     ad_index:0x%x\n",                          \
                         sys_output_mac((psys)->addr.l2_addr.mac), (psys)->addr.l2_addr.fid,        \
                         (psys)->addr.l2_addr.gport, (psys)->key_index, \
                         (psys)->ad_index);                               \
    }

#define SYS_IGS_VSI_PARAM_MAX    8
#define SYS_EGS_VSI_PARAM_MAX    2

#define SYS_L2_OP_FLAG(val, flag, is_set) \
    do\
    {\
         if(is_set) { \
            CTC_SET_FLAG((val),(flag)); \
         }else{ \
            CTC_UNSET_FLAG((val), (flag)); \
         }\
    }while(0)
typedef DsFibHost0MacHashKey_m   ds_key_t;

/****************************************************************************
 *
 * Global and Declaration
 *
 *****************************************************************************/

#define  _ENTRY_

#define CONTINUE_IF_FLAG_NOT_MATCH(query_flag, flag) \
    switch (query_flag)                              \
    {                                                \
    case CTC_L2_FDB_ENTRY_STATIC:                    \
        if (!flag.is_static)                         \
        {                                            \
            continue;                                \
        }                                            \
        break;                                       \
                                                     \
    case  CTC_L2_FDB_ENTRY_DYNAMIC:                  \
        if (flag.is_static)                          \
        {                                            \
            continue;                                \
        }                                            \
        break;                                       \
                                                     \
    case CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC:             \
        if (flag.remote_dynamic || flag.is_static)   \
        {                                            \
            continue;                                \
        }                                            \
        break;                                       \
                                                     \
    case CTC_L2_FDB_ENTRY_ALL:                       \
    default:                                         \
        break;                                       \
    }


#define RETURN_IF_FLAG_NOT_MATCH(query_flag, flag) \
    switch (query_flag)                            \
    {                                              \
    case CTC_L2_FDB_ENTRY_STATIC:                  \
        if (!CTC_FLAG_ISSET(flag, CTC_L2_FLAG_IS_STATIC))                       \
        {                                          \
            return 0;                              \
        }                                          \
        break;                                     \
                                                   \
    case  CTC_L2_FDB_ENTRY_DYNAMIC:                \
        if (CTC_FLAG_ISSET(flag, CTC_L2_FLAG_IS_STATIC))                        \
        {                                          \
            return 0;                              \
        }                                          \
        break;                                     \
    case CTC_L2_FDB_ENTRY_ALL:                     \
    default:                                       \
        break;                                     \
    }


#define SYS_FDB_FLUSH_FDB_MAC_LIMIT_FLAG      \
  ( DRV_FIB_ACC_FLUSH_DEC_DYNAMIC_PORT_LIMIT  \
  | DRV_FIB_ACC_FLUSH_DEC_DYNAMIC_VLAN_LIMIT  \
  | DRV_FIB_ACC_FLUSH_DEC_DYNAMIC_LIMIT  \
  | DRV_FIB_ACC_FLUSH_DEC_SYSTEM_LIMIT )

#define UTILITY
#define GET_FID_NODE          0
#define GET_FID_LIST          1

#define L2_COUNT_GLOBAL       0
#define L2_COUNT_PORT_LIST    1
#define L2_COUNT_FID_LIST     2

#define L2_TYPE_UC            0
#define L2_TYPE_MC            1
#define L2_TYPE_DF            2
#define L2_TYPE_TCAM          3

#define DEFAULT_ENTRY_INDEX(fid)            (pl2_usw_master[lchip]->dft_entry_base + (fid))
#define IS_DEFAULT_ENTRY_INDEX(ad_index)    ((ad_index) <= (pl2_usw_master[lchip]->dft_entry_base+pl2_usw_master[lchip]->cfg_max_fid))

#define L2_LOCK \
    if (pl2_usw_master[lchip] && pl2_usw_master[lchip]->l2_mutex) sal_mutex_lock(pl2_usw_master[lchip]->l2_mutex)

#define L2_UNLOCK \
    if (pl2_usw_master[lchip] && pl2_usw_master[lchip]->l2_mutex) sal_mutex_unlock(pl2_usw_master[lchip]->l2_mutex)

#define L2_DUMP_LOCK \
    if (pl2_usw_master[lchip] && pl2_usw_master[lchip]->l2_dump_mutex) sal_mutex_lock(pl2_usw_master[lchip]->l2_dump_mutex)

#define L2_DUMP_UNLOCK \
    if (pl2_usw_master[lchip] && pl2_usw_master[lchip]->l2_dump_mutex) sal_mutex_unlock(pl2_usw_master[lchip]->l2_dump_mutex)

#define SYS_L2_INIT_CHECK()        \
    {                              \
        LCHIP_CHECK(lchip);         \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (pl2_usw_master[lchip] == NULL)    \
        { SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
            return CTC_E_NOT_INIT;\
 } \
    }

#define SYS_L2_INDEX_CHECK(index)           \
do{ \
    uint32 hash_entry_num = 0;\
    sys_usw_ftm_query_table_entry_num(lchip, DsFibHost0MacHashKey_t, &hash_entry_num);\
    if ((index) >= (hash_entry_num+MCHIP_CAP(SYS_CAP_L2_FDB_CAM_NUM))) \
    { return CTC_E_INVALID_PARAM; }\
}while(0)

#define DONTCARE_FID    0xFFFF
#define DUMP_MAX_ENTRY  256
#define L2UC_INVALID_FID(fid)       (DONTCARE_FID != fid) && REGULAR_INVALID_FID(fid)

#define SYS_L2UC_FID_CHECK(fid)       \
    if (L2UC_INVALID_FID(fid))     \
    {                                 \
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [FDB] Invalid FID \n");\
        return CTC_E_BADID;\
 \
    }

#define IS_MCAST_DESTMAP(dm) ((dm >> 18) & 0x1)
#define SYS_L2_IS_LOGIC_PORT(fid) (CTC_FLAG_ISSET(_sys_usw_l2_get_fid_flag(lchip, (fid)),CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT))

/*32bits means: 31.is_sw; 30-11.hash_node id, refer to FDB_MAC_FID_HASH_SIZE; 0-10.hash list node number id*/
#define FDB_MAC_FID_SW_QUERY_IDX_DECODE(start_idx, is_sw, hash_node_id, list_node_id) \
do{\
    if (CTC_IS_BIT_SET(start_idx,31))\
    {\
        is_sw = 1;\
        list_node_id = (start_idx &0x7ff);\
        hash_node_id = ((0x7fffffff & start_idx) >> 11);\
    }\
    else\
    {\
        is_sw = 0;\
        list_node_id = 0;\
        hash_node_id = 0;\
    }\
}while(0)

#define FDB_MAC_FID_SW_QUERY_IDX_ENCODE(next_idx, hash_node_id, list_node_id) \
do{\
    next_idx = 0;\
    CTC_BIT_SET(next_idx,31);\
    next_idx |= ((hash_node_id << 11) | list_node_id);\
}while(0)

/****************************************************************************

         Struct Definition

*****************************************************************************/
typedef enum sys_l2_dsmac_base_type_e
{
    SYS_L2_DSMAC_BASE_GPORT=1,
    SYS_L2_DSMAC_BASE_TRUNK=2,
    SYS_L2_DSMAC_BASE_VPORT=0,
}sys_l2_dsmac_base_type_t;

typedef struct
{
    uint32 key_index;
    uint32 ad_index;

    uint8  conflict;
    uint8  hit;
    uint8  pending;   /* mac limit reach*/
    uint8  rsv0;
}mac_lookup_result_t;

struct sys_l2_ad_s
{
    DsMac_m       ds_mac;

    uint32        index;
};
typedef struct sys_l2_ad_s sys_l2_ad_t;

struct sys_l2_vport_2_nhid_s
{
    uint32 nhid;
    uint32 ad_idx;
};
typedef struct sys_l2_vport_2_nhid_s   sys_l2_vport_2_nhid_t;

/**
   @brief stored in vlan_fdb_list.
 */
typedef struct
{
    uint16         fid;
    uint16         flag;         /**<ctc_l2_dft_vlan_flag_t */
    uint16         mc_gid;
    uint16         cid;
    uint8          share_grp_en;
    uint8          rsv;
}sys_l2_fid_node_t;

struct sys_l2_node_s
{
    mac_addr_t mac;
    uint16     fid;

    uint32         flag;         /**< ctc_l2_flag_t */
    uint32         ecmp_valid :1;
    uint32         aps_valid  :1;
    uint32         bind_nh    :1;
    uint32         rsv_ad     :1;
    uint32         use_destmap_profile : 1;
    uint32         rsv        :27;
    uint32         nh_id;        /*only used for bind nexthop,else is zero*/
    sys_l2_ad_t*   adptr;        /**< if NULL, indicate not use spool */
    uint32         key_index;

};
typedef struct sys_l2_node_s   sys_l2_node_t;

typedef enum
{
    SYS_L2_ADDR_TYPE_UC,
    SYS_L2_ADDR_TYPE_MC,
    SYS_L2_ADDR_TYPE_DEFAULT
}sys_l2_addr_type_t;

struct mc_mem_s
{
    uint8    is_assign;
    uint8    with_nh;
    uint32   mem_port;
    uint32   nh_id;
    uint8    remote_chip;
} ;
typedef struct mc_mem_s   mc_mem_t;

struct sys_l2_flush_fdb_s
{
    mac_addr_t mac;
    uint16 fid;
    uint32 gport;
    uint8  flush_flag;
    uint8  flush_type;
    uint8  use_logic_port;
    uint8  lchip;
};

typedef struct sys_l2_flush_fdb_s sys_l2_flush_fdb_t;

typedef struct
{
    uint32 key_index[64];
    uint8  index_cnt;
    uint8  rsv[3];
}sys_l2_calc_index_t;

/* proved neccsary. like init default entry. */
typedef struct
{
    union {
        ctc_l2_addr_t  l2_addr;
        ctc_l2_mcast_addr_t l2mc_addr;
        ctc_l2dflt_addr_t l2df_addr;

    }addr;

    uint8         addr_type;     /**< refer to sys_l2_addr_type_t */

    uint32         key_valid:1;
    uint32         pending:1;
    uint32         rsv_ad_index:1;
    uint32         with_nh :1;       /**< used for build ad_index with nexthop */
    uint32         revert_default:1;
    uint32         merge_dsfwd:1;
    uint32         ecmp_valid:1; /*ecmp or ecmp interface*/
    uint32         aps_valid:1;

    uint32         mc_nh:1;
    uint32         is_logic_port:1; /*used for uc & binding nhid and logic port*/
    uint32         bind_nh:1;
    uint32         nh_ext:1;
    uint32         cloud_sec_en:1;
    uint32         by_pass_ingress_edit:1;
    uint32         is_exist:1;
    uint32         share_grp_en:1;
    uint32         adjust_len:8;

    uint32         use_destmap_profile:1;
    uint32         rsv:7;

    uint32        nhid;          /**< use it directly if with_nh is true */
    uint32        fwd_ptr;
    uint32        dsnh_offset;

    uint32        destmap;     /*for ecmp, means ecmp group id*/

    uint32        key_index;
    uint32        ad_index;

}sys_l2_info_t;

struct sys_l2_destmap_profile_fdb_s
{
    uint8 lchip;
    ctc_l2_fdb_query_t* pq;
    ctc_l2_fdb_query_rst_t* pr;
    sys_l2_detail_t* p_detail;
};
typedef struct sys_l2_destmap_profile_fdb_s sys_l2_destmap_profile_fdb_t;

#define SYS_L2_REG_SYN_ADD_DB_CB(lchip, add) pl2_usw_master[lchip]->sync_##add##_db = _sys_usw_l2_add_indrect;
#define SYS_L2_REG_SYN_DEL_DB_CB(lchip, remove) pl2_usw_master[lchip]->sync_##remove##_db = _sys_usw_l2_remove_indrect;


sys_l2_master_t* pl2_usw_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern int32 sys_usw_aging_get_aging_timer(uint8 lchip, uint8 domain_type, uint32 key_index, uint8* p_timer);

extern int32 sys_usw_nh_remove_all_members(uint8 lchip,  uint32 nhid);
extern int32 sys_usw_l2_fdb_wb_sync(uint8 lchip);

extern int32 sys_usw_l2_fdb_wb_restore(uint8 lchip);
STATIC int32
_sys_usw_l2_write_hw(uint8 lchip, sys_l2_info_t* psys);
STATIC int32 _sys_usw_l2_set_fid_property(uint8 lchip, uint16 fid_id, uint32 fid_prop, uint32 value);
/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
int32 sys_usw_l2_fdb_change_mem_cb(uint8 lchip, void*user_data);
int32
sys_usw_l2_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);
/* normal fdb hash opearation*/
uint32
_sys_usw_l2_fdb_hash_make(sys_l2_node_t* node)
{
    return ctc_hash_caculate(sizeof(mac_addr_t)+sizeof(uint16), &(node->mac));
}

bool
_sys_usw_l2_fdb_hash_compare(sys_l2_node_t* stored_node, sys_l2_node_t* lkup_node)
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
_sys_usw_l2_fdb_hash_add(uint8 lchip, sys_l2_node_t* p_fdb_node)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (NULL == ctc_hash_insert(pl2_usw_master[lchip]->fdb_hash, p_fdb_node))
    {
        return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_fdb_hash_remove(uint8 lchip, sys_l2_node_t* p_fdb_node)
{
    if (!ctc_hash_remove(pl2_usw_master[lchip]->fdb_hash, p_fdb_node))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [FDB] Hash remove failed \n");
        return CTC_E_NOT_EXIST;

    }

    return CTC_E_NONE;
}

STATIC sys_l2_node_t*
_sys_usw_l2_fdb_hash_lkup(uint8 lchip, mac_addr_t* mac, uint16 fid)
{
    sys_l2_node_t node;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&node, 0, sizeof(node));

    node.fid = fid;
    sal_memcpy(&node.mac, mac, sizeof(mac_addr_t));

    return ctc_hash_lookup(pl2_usw_master[lchip]->fdb_hash, &node);
}
STATIC void*
_sys_usw_l2_fid_node_lkup(uint8 lchip, uint16 fid)
{
    sys_l2_fid_node_t* p_lkup;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_lkup = ctc_vector_get(pl2_usw_master[lchip]->fid_vec, fid);

    return p_lkup;

}

STATIC uint16
_sys_usw_l2_get_fid_flag(uint8 lchip, uint16 fid)
{
    sys_l2_fid_node_t* p_fid_node = NULL;
    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, fid);
    return p_fid_node ? (p_fid_node->flag) : 0;
}

STATIC uint32
_sys_usw_l2_get_dsmac_base(uint8 lchip, sys_l2_dsmac_base_type_t type)
{
    uint32 cmd = DRV_IOR(FibAccelerationCtl_t, FibAccelerationCtl_dsMacBase0_f + type);
    uint32 dsmac_base = 0;
    int32 ret = 0;

    ret = DRV_IOCTL(lchip, 0, cmd, &dsmac_base);
    if(ret)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "read FibAccelerationCtl error\n");
    }

    return dsmac_base;
}

STATIC int32
_sys_usw_l2_decode_dsmac(uint8 lchip, sys_l2_info_t* psys, uint32 ad_index, void* p_ds_mac)
{
    uint32 destmap = 0;
    uint32 temp_flag = 0;
    uint32 temp_gport = 0;
    DsMac_m ds_mac;
    uint32  cmd = 0;
    uint16  ecmp_group_id = 0;

    CTC_PTR_VALID_CHECK(psys);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Read dsmac, ad_index:0x%x\n", ad_index);

    if (p_ds_mac == NULL)
    {
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &ds_mac));
    }
    else
    {
        sal_memcpy(&ds_mac, p_ds_mac, sizeof(DsMac_m));
    }

    psys->ad_index        = ad_index;
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_IS_STATIC, GetDsMac(V, isStatic_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_AGING_DISABLE, GetDsMac(V, isStatic_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_BIND_PORT, GetDsMac(V, srcMismatchDiscard_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_PROTOCOL_ENTRY, GetDsMac(V, protocolExceptionEn_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_COPY_TO_CPU, GetDsMac(V, macDaExceptionEn_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_SRC_DISCARD, GetDsMac(V, srcDiscard_f, &ds_mac));
    SYS_L2_OP_FLAG(temp_flag, CTC_L2_FLAG_SELF_ADDRESS, GetDsMac(V, localAddress_f, &ds_mac));

    /*decode addr type*/
    psys->addr_type = SYS_L2_ADDR_TYPE_UC;
    destmap = GetDsMac(V, adDestMap_f, &ds_mac);
    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_IS_STATIC) && /*mcast is static.*/
        (GetDsMac(V, nextHopPtrValid_f, &ds_mac) || CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_IS_STATIC)) &&
        IS_MCAST_DESTMAP(destmap))
    {
        if (IS_DEFAULT_ENTRY_INDEX(ad_index))
        {
            psys->addr_type = SYS_L2_ADDR_TYPE_DEFAULT;
            psys->addr.l2df_addr.l2mc_grp_id = (destmap &0xFFFF);
        }
        else
        {
            psys->addr_type = SYS_L2_ADDR_TYPE_MC;
            psys->addr.l2mc_addr.l2mc_grp_id = (destmap &0xFFFF);
        }
    }

    psys->is_logic_port = GetDsMac(V, learnSource_f, &ds_mac);
    psys->merge_dsfwd = GetDsMac(V, nextHopPtrValid_f, &ds_mac);
    if(psys->merge_dsfwd)
    {
        psys->aps_valid = GetDsMac(V, adApsBridgeEn_f, &ds_mac);
        destmap = GetDsMac(V, adDestMap_f, &ds_mac);

        psys->ecmp_valid = SYS_IS_ECMP_DESTMAP(destmap);
        if(psys->ecmp_valid)
        {
            psys->destmap = SYS_DECODE_ECMP_DESTMAP(destmap);
        }
        else
        {
            psys->destmap = destmap;
        }

        psys->dsnh_offset = GetDsMac(V, adNextHopPtr_f, &ds_mac);
    }
    else
    {
        ecmp_group_id = GetDsMac(V, ecmpGroupIdHighBits_f, &ds_mac) << 7 |
                            GetDsMac(V, tunnelPacketType_f, &ds_mac) << 4 |
                            GetDsMac(V, tunnelPayloadOffset_f, &ds_mac) ;

        psys->ecmp_valid = !GetDsMac(V, payloadSelect_f, &ds_mac) && (ecmp_group_id != 0);
        if(psys->ecmp_valid)
        {
            psys->destmap = ecmp_group_id ;
        }

        psys->fwd_ptr             = GetDsMac(V, dsFwdPtr_f, &ds_mac);
    }

    /* | default| tid| gport| vport|*/
    if (ad_index >= _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK))
    {
        psys->rsv_ad_index = 1;
    }
    else if (ad_index == pl2_usw_master[lchip]->ad_index_drop)
    {
        CTC_SET_FLAG(temp_flag, CTC_L2_FLAG_DISCARD);
    }
    else if ( !GetDsMac(V, nextHopPtrValid_f, &ds_mac) && (SYS_FDB_DISCARD_FWD_PTR == GetDsMac(V, dsFwdPtr_f, &ds_mac)))
    {
        CTC_SET_FLAG(temp_flag, CTC_L2_FLAG_DISCARD);
    }

    if(GetDsMac(V, srcDiscard_f, &ds_mac) &&  GetDsMac(V, macSaExceptionEn_f, &ds_mac))
    {
        CTC_SET_FLAG(temp_flag, CTC_L2_FLAG_SRC_DISCARD_TOCPU);
    }

    if(psys->addr_type == SYS_L2_ADDR_TYPE_UC)
    {
        psys->addr.l2_addr.flag = temp_flag;

        if (psys->is_logic_port)
        {
            temp_gport = GetDsMac(V, logicSrcPort_f, &ds_mac);
            psys->addr.l2_addr.is_logic_port = 1;
        }
        else
        {
            if ((psys->fwd_ptr == SYS_FDB_DISCARD_FWD_PTR) && (!GetDsMac(V, nextHopPtrValid_f , &ds_mac)))
            {
                temp_gport = DISCARD_PORT;
            }
            else
            {
                temp_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsMac(V, globalSrcPort_f, &ds_mac));
            }
        }
        psys->addr.l2_addr.gport = temp_gport;
    }
    else if(psys->addr_type == SYS_L2_ADDR_TYPE_MC)
    {
        psys->addr.l2mc_addr.flag = temp_flag;
    }

    psys->share_grp_en = !GetDsMac(V, fastLearningEn_f, &ds_mac);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_encode_dsmac(uint8 lchip, DsMac_m* p_dsmac, sys_l2_info_t* psys)
{
    uint8  gchip    = 0;
    uint8  cid = 0;
    uint16 group_id = 0;
    uint32   temp_flag = 0;
    uint32   cmd = 0;
    uint32   value = 0;
    uint8 xgpon_en = 0;

    CTC_PTR_VALID_CHECK(psys);
    CTC_PTR_VALID_CHECK(p_dsmac);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    sal_memset(p_dsmac, 0, sizeof(DsMac_m));

    /* l2 uc*/
    if (psys->addr_type == SYS_L2_ADDR_TYPE_UC)
    {
        cid = psys->addr.l2_addr.cid;
        temp_flag = psys->addr.l2_addr.flag;
        SetDsMac(V, sourcePortCheckEn_f, p_dsmac, 1);

        if (!CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_IS_STATIC) && (psys->addr.l2_addr.gport != DISCARD_PORT))
        {
            if (!DRV_IS_DUET2(lchip) || !xgpon_en || !CTC_FLAG_ISSET(temp_flag,CTC_L2_FLAG_BIND_PORT))
            {
                SetDsMac(V, srcMismatchLearnEn_f, p_dsmac, 1);
            }
        }

        if (CTC_FLAG_ISSET(temp_flag,CTC_L2_FLAG_BIND_PORT))
        {
            SetDsMac(V, srcMismatchDiscard_f, p_dsmac, 1);
        }

        if (psys->is_logic_port)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " DsMac.logic_port 0x%x\n", psys->addr.l2_addr.gport);
            SetDsMac(V, learnSource_f, p_dsmac, 1);
            SetDsMac(V, logicSrcPort_f, p_dsmac, psys->addr.l2_addr.gport);
            sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
            if (!DRV_IS_DUET2(lchip) && psys->mc_nh && xgpon_en)
            {
                SetDsMac(V, logicDestPortValid_f, p_dsmac, 1);
            }
            if (!DRV_IS_DUET2(lchip) && psys->addr.l2_addr.assign_port && xgpon_en)
            {
                uint16 policer_ptr = 0;
                sys_qos_policer_param_t policer_param;

                sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
                CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, psys->addr.l2_addr.assign_port,
                                                                      SYS_QOS_POLICER_TYPE_FLOW,
                                                                      &policer_param));
                policer_ptr = policer_param.policer_ptr;
                SetDsMac(V, macDaPolicerEn_f, p_dsmac, 1);
                SetDsMac(V, stpCheckDisable_f, p_dsmac, (policer_ptr>>12) & 0x1);
                SetDsMac(V, macDaExceptionEn_f, p_dsmac, (policer_ptr>>11) & 0x1);
                SetDsMac(V, exceptionSubIndex_f, p_dsmac, (policer_ptr>>9) & 0x3);
                SetDsMac(V, categoryIdValid_f, p_dsmac, (policer_ptr>>8) & 0x1);
                SetDsMac(V, categoryId_f, p_dsmac, (policer_ptr) & 0xFF);
            }
        }
        else
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " DsMac.gport 0x%x\n", psys->addr.l2_addr.gport);
            SetDsMac(V, learnSource_f, p_dsmac, 0);
            SetDsMac(V, globalSrcPort_f, p_dsmac, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(psys->addr.l2_addr.gport));
        }

        SetDsMac(V, isConversation_f, p_dsmac, 0);
        if (!psys->merge_dsfwd)
        {
            if (psys->ecmp_valid)
            {   /* ecmpGroupId(9,0) = {ecmpGroupIdHighBits(2,0),tunnelPacketType(2,0)tunnelPayloadOffset(3,0)}*/
                 /*-SetDsMac(V, u1_g1_ecmpEn_f, p_dsmac, 1);*/
                SetDsMac(V, ecmpGroupIdHighBits_f,   p_dsmac, ((psys->destmap>>7) & 0x7));
                SetDsMac(V, tunnelPacketType_f,      p_dsmac, ((psys->destmap>>4) & 0x7));
                SetDsMac(V, tunnelPayloadOffset_f,   p_dsmac,  (psys->destmap & 0xF));
                SetDsMac(V, dsFwdPtr_f, p_dsmac, psys->fwd_ptr);
            }
            else
            {
                SetDsMac(V, dsFwdPtr_f, p_dsmac, psys->fwd_ptr);
            }
        }
        else
        {
            uint8 adjust_len_idx = 0;
            SetDsMac(V, nextHopPtrValid_f, p_dsmac, 1);
            SetDsMac(V, adNextHopExt_f, p_dsmac, psys->nh_ext);
            SetDsMac(V, adDestMap_f, p_dsmac, psys->destmap);
            SetDsMac(V, adNextHopPtr_f, p_dsmac,psys->dsnh_offset);
            if(0 != psys->adjust_len)
            {
                sys_usw_lkup_adjust_len_index(lchip, psys->adjust_len, &adjust_len_idx);
                SetDsMac(V, adLengthAdjustType_f, p_dsmac, adjust_len_idx);
            }
            else
            {
                SetDsMac(V, adLengthAdjustType_f, p_dsmac, 0);
            }
            SetDsMac(V, adApsBridgeEn_f, p_dsmac, psys->aps_valid);
            SetDsMac(V, adCloudSecEn_f, p_dsmac, psys->cloud_sec_en);
            SetDsMac(V, adBypassIngressEdit_f, p_dsmac, psys->by_pass_ingress_edit);

        }
    }
    else if (psys->addr_type == SYS_L2_ADDR_TYPE_DEFAULT && psys->revert_default)
    {
        cid = psys->addr.l2df_addr.cid;
        SetDsMac(V, dsFwdPtr_f, p_dsmac, SYS_FDB_DISCARD_FWD_PTR);
        SetDsMac(V, protocolExceptionEn_f, p_dsmac, 1);
        /*TBD sub index base need consider more*/
        SetDsMac(V, unknownMcastExceptionSubIndexBase_f, p_dsmac, 63);
        SetDsMac(V, bcastExceptionSubIndexAddition_f, p_dsmac, 0);/*D2*/
        SetDsMac(V, unknownUcastExceptionSubIndexAddition_f, p_dsmac, 0);/*D2*/
    }
    else if (psys->addr_type == SYS_L2_ADDR_TYPE_DEFAULT && !psys->revert_default)
    {
        cid = psys->addr.l2df_addr.cid;
        if (CTC_FLAG_ISSET(psys->addr.l2df_addr.flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT))
        {
            SetDsMac(V, learnSource_f, p_dsmac, 1);
        }

        if (CTC_FLAG_ISSET(psys->addr.l2df_addr.flag, CTC_L2_DFT_VLAN_FLAG_LEARNING_DISABLE))
        {
            SetDsMac(V, conversationCheckEn_f, p_dsmac, 1);
        }
        else
        {
            SetDsMac(V, conversationCheckEn_f, p_dsmac, 0);
        }

        if (CTC_FLAG_ISSET(psys->addr.l2df_addr.flag, CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU))
        {
            SetDsMac(V, protocolExceptionEn_f, p_dsmac, 1);
        }

        if (CTC_FLAG_ISSET(psys->addr.l2df_addr.flag, CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTH))
        {
            SetDsMac(V, srcMismatchDiscard_f, p_dsmac, 1);
        }
        /*port mac auth also config this,so the global src port is 1008 (0x3f << 4), the mac auth can work normally*/
        SetDsMac(V, unknownMcastExceptionSubIndexBase_f, p_dsmac, 63);
        SetDsMac(V, bcastExceptionSubIndexAddition_f, p_dsmac, 0);/*D2*/
        SetDsMac(V, unknownUcastExceptionSubIndexAddition_f, p_dsmac, 0);/*D2*/
        if (CTC_FLAG_ISSET(psys->addr.l2df_addr.flag, CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH))
        {
            SetDsMac(V, srcDiscard_f, p_dsmac, 1);
            cmd = DRV_IOR(DsMac_t, DsMac_macSaExceptionEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, psys->ad_index, cmd, &value));
            SetDsMac(V, macSaExceptionEn_f, p_dsmac, value);
        }

        /* always use nexthop */
        SetDsMac(V, nextHopPtrValid_f, p_dsmac, 1);
        SetDsMac(V, adDestMap_f, p_dsmac, SYS_ENCODE_MCAST_IPE_DESTMAP(psys->addr.l2df_addr.l2mc_grp_id));
        SetDsMac(V, adNextHopPtr_f, p_dsmac, 0);
        SetDsMac(V, isConversation_f, p_dsmac, 0);

        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gchip=%d, group_id=0x%x\n", gchip, group_id);
    }
    else  /* (psys->flag.type_l2mc)*/
    {
        if (psys->is_exist)
        {
            cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, psys->ad_index, cmd, p_dsmac));
        }

        cid = psys->addr.l2mc_addr.cid;
        temp_flag = psys->addr.l2mc_addr.flag;
        /* always use nexthop */
        SetDsMac(V, nextHopPtrValid_f, p_dsmac, 1);
        if (psys->addr.l2mc_addr.l2mc_grp_id)
        {
            SetDsMac(V, adDestMap_f, p_dsmac, SYS_ENCODE_MCAST_IPE_DESTMAP(psys->addr.l2mc_addr.l2mc_grp_id));
        }
        SetDsMac(V, adNextHopPtr_f, p_dsmac, 0);
        SetDsMac(V, isConversation_f, p_dsmac, 0);

        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gchip=%d, group_id=0x%x\n", gchip, group_id);
    }

    if ((CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_IS_STATIC)) || (psys->addr_type == SYS_L2_ADDR_TYPE_DEFAULT && !psys->revert_default))
    {
        SetDsMac(V, isStatic_f, p_dsmac, 1);
    }

    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_SELF_ADDRESS))
    {
        SetDsMac(V, localAddress_f, p_dsmac, 1);
    }
    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_DISCARD))
    {
        SetDsMac(V, nextHopPtrValid_f, p_dsmac, 0);
        SetDsMac(V, dsFwdPtr_f, p_dsmac, SYS_FDB_DISCARD_FWD_PTR);
        SetDsMac(V, ecmpGroupIdHighBits_f,   p_dsmac, 0);
        SetDsMac(V, tunnelPacketType_f,      p_dsmac, 0);
        SetDsMac(V, tunnelPayloadOffset_f,   p_dsmac, 0);
    }

    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_SRC_DISCARD))
    {
        SetDsMac(V, srcDiscard_f, p_dsmac, 1);
        SetDsMac(V, macSaExceptionEn_f, p_dsmac, 0);
    }

    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_SRC_DISCARD_TOCPU))
    {
        SetDsMac(V, srcDiscard_f, p_dsmac, 1);
        SetDsMac(V, macSaExceptionEn_f, p_dsmac, 1);
    }

    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_COPY_TO_CPU))
    {
        SetDsMac(V, macDaExceptionEn_f, p_dsmac, 1);
        /* Reserved for normal MACDA copy_to_cpu */
        SetDsMac(V, exceptionSubIndex_f, p_dsmac, CTC_L2PDU_ACTION_INDEX_MACDA);
    }

    if (CTC_FLAG_ISSET(temp_flag, CTC_L2_FLAG_PROTOCOL_ENTRY))
    {
        SetDsMac(V, protocolExceptionEn_f, p_dsmac, 1);
    }

    SetDsMac(V, stormCtlEn_f, p_dsmac, 1);
    SetDsMac(V, fastLearningEn_f, p_dsmac, (psys->share_grp_en)?0:1);
    /*dsmac for cid*/
    if (cid)
    {
        SetDsMac(V, categoryIdValid_f, p_dsmac, 1);
        SetDsMac(V, categoryId_f, p_dsmac, cid);
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l2_acc_lookup_mac_fid(uint8 lchip, mac_addr_t mac, uint16 fid, mac_lookup_result_t* pr)
{
    drv_acc_in_t  in;
    drv_acc_out_t out;
    DsFibHost0MacHashKey_m fdb_key;
    hw_mac_addr_t   hw_mac = {0};
    uint8 xgpon_en = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    sal_memset(pr, 0, sizeof(mac_lookup_result_t));
    sal_memset(&fdb_key, 0, sizeof(DsFibHost0MacHashKey_m));

    FDB_SET_HW_MAC(hw_mac, mac);
    SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, fid);
    SetDsFibHost0MacHashKey(A, mappedMac_f, &fdb_key, hw_mac);
    in.tbl_id = DsFibHost0MacHashKey_t;
    in.data = &fdb_key;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_KEY;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if (xgpon_en)
    {
        out.ad_index = out.ad_index & 0xFFFF;
    }
    pr->ad_index  = out.ad_index;
    pr->key_index = out.key_index;
    pr->hit       = out.is_hit;
    pr->conflict  = out.is_conflict;
    pr->pending   = out.is_pending;
    if (pr->pending)
    {
        pr->hit       = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_fdb_calc_index(uint8 lchip, mac_addr_t mac, uint16 fid, sys_l2_calc_index_t* p_index)
{
    drv_acc_in_t  in;
    drv_acc_out_t out;
    uint8 hash_key[12] = {0};

    CTC_PTR_VALID_CHECK(p_index);
    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    hash_key[0] = 0x09;
    hash_key[2] = (fid & 0x3) << 6;
    hash_key[3] = (fid >> 2) & 0xFF;
    hash_key[4] = ((fid >> 10) & 0xF) | ((mac[5] & 0xF) << 4);
    hash_key[5] = (mac[5] >> 4) | ((mac[4] & 0xF) << 4);
    hash_key[6] = (mac[4] >> 4) | ((mac[3] & 0xF) << 4);
    hash_key[7] = (mac[3] >> 4) | ((mac[2] & 0xF) << 4);
    hash_key[8] = (mac[2] >> 4) | ((mac[1] & 0xF) << 4);
    hash_key[9] = (mac[1] >> 4) | ((mac[0] & 0xF) << 4);
    hash_key[10] = mac[0] >> 4;
    in.data = (void*)hash_key;
    in.type = DRV_ACC_TYPE_CALC;
    in.tbl_id    = DsFibHost0MacHashKey_t;
    in.op_type = DRV_ACC_OP_BY_KEY;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    p_index->index_cnt = out.query_cnt;
    sal_memcpy(p_index->key_index, out.data, p_index->index_cnt * sizeof(uint32));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l2_decode_dskey(uint8 lchip, sys_l2_info_t* psys, void* p_dskey)
{
    uint8 xgpon_en = 0;
    hw_mac_addr_t   hw_mac;
    uint8           timer = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    psys->addr.l2_addr.fid = GetDsFibHost0MacHashKey(V, vsiId_f, p_dskey);

    GetDsFibHost0MacHashKey(A, mappedMac_f, p_dskey, hw_mac);
    SYS_USW_SET_USER_MAC(psys->addr.l2_addr.mac, hw_mac);
    psys->ad_index  = GetDsFibHost0MacHashKey(V, dsAdIndex_f, p_dskey);
    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    if (xgpon_en)
    {
        psys->ad_index = psys->ad_index & 0xFFFF;
    }
    psys->key_valid = GetDsFibHost0MacHashKey(V, valid_f, p_dskey);
    psys->pending   = GetDsFibHost0MacHashKey(V, pending_f, p_dskey);
    CTC_ERROR_RETURN(sys_usw_aging_get_aging_timer(lchip, 1, psys->key_index, &timer));
    if(timer == 0)
    {
        CTC_SET_FLAG(psys->addr.l2_addr.flag, CTC_L2_FLAG_AGING_DISABLE);
    }

    return CTC_E_NONE;
}

/*
if ad_index == NULL, needn't to get ad_index
*/
STATIC uint8
_sys_usw_l2_black_hole_lookup(uint8 lchip, mac_addr_t mac, uint32* key_index, uint32* ad_index)
{
    uint32 index = 0;
    hw_mac_addr_t drv_mac;
    uint32 cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DRV_ENTRY_FLAG);
    DsFibMacBlackHoleHashKey_m fib_mac_hash_key;
    hw_mac_addr_t hw_mac;

    FDB_SET_HW_MAC(hw_mac, mac);

    for(index=0; index < SYS_L2_MAX_BLACK_HOLE_ENTRY; index++)
    {
        if(CTC_BMP_ISSET(pl2_usw_master[lchip]->mac_fdb_bmp, index))
        {
            DRV_IOCTL(lchip, index, cmd, &fib_mac_hash_key);

            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "read DsFibMacBlackHoleHashKey_t index:%d\n", index);

            if(!GetDsFibMacBlackHoleHashKey(V, valid_f, &fib_mac_hash_key))
            {
                continue;
            }

            GetDsFibMacBlackHoleHashKey(A, mappedMac_f, &fib_mac_hash_key, &drv_mac);
            if(!sal_memcmp(&hw_mac, &drv_mac, sizeof(hw_mac_addr_t)))
            {
                *key_index = index;
                break;
            }
        }
    }

    if(index != SYS_L2_MAX_BLACK_HOLE_ENTRY)
    {
        if(ad_index)
        {
            *ad_index = GetDsFibMacBlackHoleHashKey(V, dsAdIndex_f, &fib_mac_hash_key);
        }

        return 1;
    }

    *key_index = 0xFFFFFFFF;
    return 0;
}

STATIC int32
_sys_usw_l2_update_logic_dsmac(uint8 lchip, void* data, void* change_nh_param)
{
    sys_l2_vport_2_nhid_t* p_node = (sys_l2_vport_2_nhid_t*)data;
    uint32 cmd  = 0;
    DsMac_m   dsmac;
    uint32 fwd_offset = 0;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;

    if (!p_node)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "bind reserve nh update fail, nh & logic bind is Null!\n");
		return CTC_E_INVALID_PTR;
    }

    cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_node->ad_idx, cmd, &dsmac);

    if (!change_nh_param)
    {
        /*update ad from merge dsfwd to using dsfwd*/

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_node->nhid, &fwd_offset, 1));

        /*2. update ad*/
        SetDsMac(V, nextHopPtrValid_f, &dsmac, 0);
        SetDsMac(V, adDestMap_f, &dsmac, 0);
        SetDsMac(V, adNextHopPtr_f, &dsmac, 0);
        SetDsMac(V, adNextHopExt_f, &dsmac, 0);
        SetDsMac(V, adLengthAdjustType_f, &dsmac, 0);
        SetDsMac(V, adApsBridgeEn_f, &dsmac, 0);
        SetDsMac(V, dsFwdPtr_f, &dsmac, fwd_offset);
    }
    else
    {
        uint8 adjust_len_idx = 0;
        if(GetDsMac(V, nextHopPtrValid_f, &dsmac) == 0)
        {
            return CTC_E_NONE;
        }
        SetDsMac(V, adDestMap_f, &dsmac, p_dsnh_info->dest_map);
        SetDsMac(V, adNextHopPtr_f, &dsmac, p_dsnh_info->dsnh_offset);
        SetDsMac(V, adNextHopExt_f, &dsmac, p_dsnh_info->nexthop_ext);
        SetDsMac(V, adApsBridgeEn_f, &dsmac, p_dsnh_info->aps_en);
        if(0 != p_dsnh_info->adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
            SetDsMac(V, adLengthAdjustType_f, &dsmac, adjust_len_idx);
        }
        else
        {
            SetDsMac(V, adLengthAdjustType_f, &dsmac, 0);
        }
        if (!DRV_IS_DUET2(lchip) && (p_dsnh_info->dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG))
        {
            SetDsMac(V, adNextHopPtr_f, &dsmac, (p_dsnh_info->dsnh_offset&(~SYS_NH_DSNH_BY_PASS_FLAG)));
            SetDsMac(V, adBypassIngressEdit_f, &dsmac, 1);
        }

        if (!GetDsMac(V, learnSource_f, &dsmac))
        {
            SetDsMac(V, globalSrcPort_f, &dsmac, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_dsnh_info->gport));
        }
    }

    cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_node->ad_idx, cmd, &dsmac);

    return CTC_E_NONE;
}

 int32
sys_usw_l2_update_dsmac(uint8 lchip, void* data, void* change_nh_param)
{
    sys_l2_node_t* p_l2_info = data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    DsMac_m   dsmac;
    uint32 cmdr  = 0;
    uint32 cmdw  = 0;
    uint32 fwd_offset = 0;
    sys_nh_info_dsnh_t nh_info;
    sys_l2_ad_t         l2_ad;
    sys_l2_ad_t         l2_ad_old;
    sys_l2_ad_t*        p_ad_get = NULL;
    sys_l2_info_t  new_sys;
    int32 ret = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&dsmac, 0, sizeof(dsmac));

    /*if true funtion was called because nexthop update,if false funtion was called because using dsfwd*/
    if (p_dsnh_info)
    {
        L2_LOCK;
    }

    if (!p_l2_info || !p_l2_info->adptr)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "bind nh update to dsfwd fail, l2 info is Null!\n");
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
        if (p_dsnh_info)
        {
            L2_UNLOCK;
        }

        return CTC_E_INVALID_PTR;
    }

    sal_memset(&nh_info, 0, sizeof(nh_info));

    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo(lchip, p_l2_info->nh_id, &nh_info, 1), ret, error0);

    cmdr = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_l2_info->adptr->index, cmdr, &dsmac);

    l2_ad_old.index = p_l2_info->adptr->index;
    sal_memcpy(&l2_ad_old.ds_mac, &dsmac, sizeof(dsmac));

    if (!p_dsnh_info)
    {
        /*update ad from merge dsfwd to using dsfwd*/

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_l2_info->nh_id, &fwd_offset, 1));

        /*2. update ad*/
        SetDsMac(V, nextHopPtrValid_f, &dsmac, 0);
        SetDsMac(V, adDestMap_f, &dsmac, 0);
        SetDsMac(V, adNextHopPtr_f, &dsmac, 0);
        SetDsMac(V, adNextHopExt_f, &dsmac, 0);
        SetDsMac(V, adApsBridgeEn_f, &dsmac, 0);
        SetDsMac(V, adLengthAdjustType_f, &dsmac, 0);
        SetDsMac(V, dsFwdPtr_f, &dsmac, fwd_offset);
        p_l2_info->bind_nh = 0;
        SetDsMac(V, adCloudSecEn_f, &dsmac, 0);
    }
    else
    {
        uint8 adjust_len_idx = 0;
        if (GetDsMac(V, nextHopPtrValid_f, &dsmac) == 0)
        {
            L2_UNLOCK;
            return CTC_E_NONE;
        }

        sal_memset(&new_sys, 0, sizeof(sys_l2_info_t));
        new_sys.dsnh_offset = p_dsnh_info->dsnh_offset;
        if ((nh_info.dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG) && !DRV_IS_DUET2(lchip))
        {
            new_sys.dsnh_offset &= ~SYS_NH_DSNH_BY_PASS_FLAG;
            new_sys.by_pass_ingress_edit = 1;
        }
        SetDsMac(V, adDestMap_f, &dsmac, p_dsnh_info->dest_map);
        SetDsMac(V, adNextHopExt_f, &dsmac, p_dsnh_info->nexthop_ext);
        SetDsMac(V, adNextHopPtr_f, &dsmac, new_sys.dsnh_offset);
        SetDsMac(V, adApsBridgeEn_f, &dsmac, p_dsnh_info->aps_en);
        SetDsMac(V, adBypassIngressEdit_f, &dsmac, new_sys.by_pass_ingress_edit);
        if(0 != p_dsnh_info->adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
            SetDsMac(V, adLengthAdjustType_f, &dsmac, adjust_len_idx);
        }
        else
        {
            SetDsMac(V, adLengthAdjustType_f, &dsmac, 0);
        }

        if (!GetDsMac(V, learnSource_f, &dsmac))
        {
            SetDsMac(V, globalSrcPort_f, &dsmac, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_dsnh_info->gport));
        }
        SetDsMac(V, adCloudSecEn_f, &dsmac, nh_info.cloud_sec_en);
    }

    l2_ad.index = p_l2_info->adptr->index;
    sal_memcpy(&l2_ad.ds_mac, &dsmac, sizeof(dsmac));

    /*spool ad*/
    CTC_ERROR_GOTO(ctc_spool_add(pl2_usw_master[lchip]->ad_spool, (void*)&l2_ad, NULL, &p_ad_get), ret, error0);

    p_l2_info->adptr = p_ad_get;

    sal_memset(&new_sys, 0, sizeof(sys_l2_info_t));

    new_sys.addr.l2_addr.flag = p_l2_info->flag;
    new_sys.addr.l2_addr.fid = p_l2_info->fid;
    sal_memcpy(&new_sys.addr.l2_addr.mac, &p_l2_info->mac, sizeof(p_l2_info->mac));
    new_sys.addr.l2_addr.gport = p_dsnh_info?(p_dsnh_info->gport):nh_info.gport;
    new_sys.addr_type = SYS_L2_ADDR_TYPE_UC;
    new_sys.ecmp_valid =  p_l2_info->ecmp_valid;
    new_sys.ad_index = p_ad_get->index;
    new_sys.rsv_ad_index = 1;
    new_sys.is_logic_port = GetDsMac(V, learnSource_f, &dsmac);

    DRV_IOCTL(lchip, p_l2_info->adptr->index, cmdw, &dsmac);

    CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &new_sys), ret, error1);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "FDB update_da, ad_offset:0x%x \n", p_l2_info->adptr->index);

    /*Must free old node after write hw*/
    ctc_spool_remove(pl2_usw_master[lchip]->ad_spool,(void*)&l2_ad_old, &p_ad_get);

    if (p_dsnh_info)
    {
        L2_UNLOCK;
    }
    return CTC_E_NONE;

    error1:
    ctc_spool_remove(pl2_usw_master[lchip]->ad_spool,(void*)&l2_ad, &p_ad_get);
    error0:
    if (p_dsnh_info)
    {
        L2_UNLOCK;
    }
    return ret;

}

int32
_sys_usw_l2_bind_nexthop(uint8 lchip, void* p_l2_info, uint8 is_bind, uint8 is_logic)
{
    sys_nh_update_dsnh_param_t update_dsnh;
	int32 ret = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    if (is_logic)
    {
        sys_l2_vport_2_nhid_t* p_l2_node = (sys_l2_vport_2_nhid_t*)(p_l2_info);
        update_dsnh.data = is_bind?p_l2_info:NULL;
        update_dsnh.updateAd = is_bind?_sys_usw_l2_update_logic_dsmac:NULL;
        ret = sys_usw_nh_bind_dsfwd_cb(lchip, p_l2_node->nhid, &update_dsnh);
    }
    else
    {
        sys_l2_node_t* p_l2_node = (sys_l2_node_t*)(p_l2_info);
        update_dsnh.data = is_bind?p_l2_info:NULL;
        update_dsnh.updateAd = is_bind?sys_usw_l2_update_dsmac:NULL;
        ret = sys_usw_nh_bind_dsfwd_cb(lchip, p_l2_node->nh_id, &update_dsnh);
        if (CTC_E_NONE == ret)
        {
            p_l2_node->bind_nh = is_bind?1:0;
        }
    }

    return ret;
}

int32
_sys_usw_l2_get_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 *nhp_id)
{
    sys_l2_vport_2_nhid_t *p_node  = NULL;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    SYS_LOGIC_PORT_CHECK(logic_port);

    p_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
    if (NULL == p_node)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }
    else
    {
        *nhp_id = p_node->nhid;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2_get_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 *nhp_id)
{
    int32 ret = 0;
    L2_LOCK;
    ret = _sys_usw_l2_get_nhid_by_logic_port( lchip, logic_port, nhp_id);
    L2_UNLOCK;
    return ret;
}


/*Notice: here gport means dest port */
STATIC int32
_sys_usw_l2_map_nh_to_sys(uint8 lchip, uint32 nhid, sys_l2_info_t * psys, uint8 is_assign, uint32 gport, uint16 assign_gport)
{
    sys_nh_info_dsnh_t nh_info;
    uint8 have_dsfwd = 0;
    int32 ret = 0;
    uint32 bind_nh = 0;
    uint8 is_rsv = 0;

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

    ret = _sys_usw_l2_get_nhid_by_logic_port(lchip, psys->addr.l2_addr.gport, &bind_nh);
    if (((ret == CTC_E_NONE) && (bind_nh == nhid) && psys->is_logic_port ) || CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_DISCARD))
    {
        is_rsv = 1;
    }

    /*get nexthop info from nexthop id */
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info, 0));
    have_dsfwd = (nh_info.dsfwd_valid||(nh_info.merge_dsfwd == 2)) &&
        !is_assign && !nh_info.is_mcast && !is_rsv;

    if (!have_dsfwd && !nh_info.is_mcast && psys->with_nh && !is_assign)
    {
        /*reserve logic port ad do not bind nh, bind should */
        psys->bind_nh = is_rsv?0:1;
        psys->nhid = nhid;
    }

    if (!have_dsfwd  && !(nh_info.ecmp_valid || nh_info.is_ecmp_intf))
    {
        psys->merge_dsfwd = 1;
        psys->dsnh_offset = nh_info.dsnh_offset;
        psys->nh_ext = nh_info.nexthop_ext;
        psys->adjust_len = nh_info.adjust_len;
        if ((nh_info.dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG) && !DRV_IS_DUET2(lchip))
        {
            psys->dsnh_offset &= ~SYS_NH_DSNH_BY_PASS_FLAG;
            psys->by_pass_ingress_edit = 1;
        }
        if (is_assign)
        {
            psys->destmap = SYS_ENCODE_DESTMAP(SYS_MAP_CTC_GPORT_TO_GCHIP(assign_gport), SYS_MAP_CTC_GPORT_TO_DRV_LPORT(assign_gport));
        }
        else
        {
            psys->destmap = nh_info.dest_map;
        }
    }
    else if(nh_info.ecmp_valid)
    {
        psys->destmap = nh_info.ecmp_group_id;
    }
    else if(nh_info.is_ecmp_intf)
    {
        psys->merge_dsfwd = 1;
        psys->destmap = nh_info.dest_map;
        psys->dsnh_offset = nh_info.dsnh_offset;
        if ((nh_info.dsnh_offset & SYS_NH_DSNH_BY_PASS_FLAG) && !DRV_IS_DUET2(lchip))
        {
            psys->dsnh_offset &= ~SYS_NH_DSNH_BY_PASS_FLAG;
            psys->by_pass_ingress_edit = 1;
        }
        psys->nh_ext = nh_info.nexthop_ext;
    }
    else
    {
        sys_usw_nh_get_dsfwd_offset(lchip, nhid, &psys->fwd_ptr, 0);
        psys->merge_dsfwd = 0;
    }

    psys->ecmp_valid = nh_info.ecmp_valid || nh_info.is_ecmp_intf;
    psys->aps_valid = nh_info.aps_en;
    psys->mc_nh = nh_info.is_mcast;
    psys->cloud_sec_en = nh_info.cloud_sec_en;
    if(is_assign && !psys->is_logic_port)
    {
        psys->addr.l2_addr.gport  = assign_gport;
    }
    else
    {
        psys->addr.l2_addr.gport  = gport;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_get_reserved_index(uint8 lchip, sys_l2_info_t* psys)
{
    int32  ret              = CTC_E_NONE;
    uint8  is_local_chip    = 0;
    uint8  is_dynamic = 0;
    uint8  is_trunk         = 0;
    uint8  is_phy_port      = 0;
    uint8  is_logic_port    = 0;
    uint8  is_binding       = 0;
    uint32 ad_index         = 0;
    uint8  other_flag    = 0;
    uint8 gchip_id          = 0;

    psys->rsv_ad_index = 1;
    other_flag += CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_SRC_DISCARD);
    other_flag += CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_SRC_DISCARD_TOCPU); /*psys->flag.src_discard_to_cpu;*/
    other_flag += CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_COPY_TO_CPU);  /*psys->flag.copy_to_cpu;*/
    other_flag += CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_PROTOCOL_ENTRY);  /*psys->flag.protocol_entry;*/
    other_flag += CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_BIND_PORT);  /*psys->flag.bind_port;*/
    other_flag += psys->is_logic_port;
#if 0
    /* for discard fdb */
    if (CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_DISCARD) && (0 == other_flag))
    {
        ad_index = pl2_usw_master[lchip]->ad_index_drop;
        psys->ad_index = ad_index;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "build drop ucast ad_index:0x%x\n", ad_index);
        return CTC_E_NONE;
    }
#endif
    gchip_id = CTC_MAP_GPORT_TO_GCHIP(psys->addr.l2_addr.gport);
    is_local_chip = sys_usw_chip_is_local(lchip, gchip_id);

	/* is pure dynamic. */
    is_dynamic = (!CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC) && !CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_BIND_PORT));

    is_logic_port = psys->is_logic_port;
    is_trunk      = !is_logic_port && CTC_IS_LINKAGG_PORT(psys->addr.l2_addr.gport);
    is_phy_port   = !is_logic_port && !CTC_IS_LINKAGG_PORT(psys->addr.l2_addr.gport);

    if (is_logic_port)
    {
        uint32 nhid;
        ret        = _sys_usw_l2_get_nhid_by_logic_port(lchip, psys->addr.l2_addr.gport, &nhid);
        is_binding = !FAIL(ret);
    }

    if (is_dynamic && is_trunk && !psys->with_nh)
    {
        ad_index =  _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK) +
                   CTC_GPORT_LINKAGG_ID(psys->addr.l2_addr.gport); /* linkagg */
    }
    else if (is_dynamic && is_local_chip && is_phy_port && !psys->with_nh)
    {
        ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT) +
                   SYS_MAP_CTC_GPORT_TO_DRV_LPORT(psys->addr.l2_addr.gport); /* gport */
    }
    else if (is_dynamic  && is_logic_port && is_binding)
    {
        if (pl2_usw_master[lchip]->vp_alloc_ad_en)
        {
            sys_l2_vport_2_nhid_t *p_vp_node = NULL;
            p_vp_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, psys->addr.l2_addr.gport);
            if(NULL == p_vp_node)
            {
                return CTC_E_INVALID_PARAM;
            }
            ad_index = p_vp_node->ad_idx;
        }
        else
        {
            ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_VPORT) + psys->addr.l2_addr.gport;  /* logic port */
        }

    }
    else if (CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_REMOTE_DYNAMIC) && !is_local_chip && pl2_usw_master[lchip]->rchip_ad_rsv[gchip_id])
    {
        uint32 base = 0;
        uint32 cmd = 0;
        uint16 lport = CTC_MAP_GPORT_TO_LPORT(psys->addr.l2_addr.gport);
        if (lport >= pl2_usw_master[lchip]->rchip_ad_rsv[gchip_id])
        {
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOR(FibAccelerationRemoteChipAdBase_t, FibAccelerationRemoteChipAdBase_array_0_remoteChipAdBase_f + gchip_id);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &base));
        ad_index = lport + (base << 6);
    }
    else
    {
        psys->rsv_ad_index = 0;
    }

    /*for cid using, cannot using reserve ad*/
    if (psys->addr.l2_addr.cid)
    {
        psys->rsv_ad_index = 0;
    }

    psys->ad_index = ad_index;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_build_sys_info(uint8 lchip, uint8 type, void * l2, sys_l2_info_t * psys,
                                  uint8 with_nh, uint32 nhid)
{
    uint8 gport_assign = 0;
    uint32 gport = 0;
    sys_nh_info_com_t* p_nhinfo = NULL;
    sys_nh_info_misc_t* p_nh_misc_info = NULL;
    uint8 is_cpu_nh = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(l2);

    if ((L2_TYPE_UC == type) ||(L2_TYPE_TCAM == type))
    {
        ctc_l2_addr_t * l2_addr = (ctc_l2_addr_t *) l2;
        uint32 temp_nhid = 0;
        sys_nh_info_dsnh_t nh_info;
        uint16  fid_flag = 0;
        sal_memcpy(&(psys->addr.l2_addr), l2_addr, sizeof(ctc_l2_addr_t));
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
        gport_assign = (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT))?1:0;

        /*get fid flag*/
        fid_flag = _sys_usw_l2_get_fid_flag(lchip, l2_addr->fid);
        psys->is_logic_port = CTC_FLAG_ISSET(fid_flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT);

        if (!with_nh)
        {
            /* if not set discard, get dsfwdptr of port */
            if (!CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_DISCARD))
            {
                /*get nexthop id from gport*/
                CTC_ERROR_RETURN(sys_usw_l2_get_ucast_nh(lchip, l2_addr->gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &temp_nhid));
                _sys_usw_l2_map_nh_to_sys(lchip, temp_nhid, psys, gport_assign, l2_addr->gport, l2_addr->assign_port);

            }
            else
            {
                psys->fwd_ptr = SYS_FDB_DISCARD_FWD_PTR;
                psys->addr.l2_addr.gport  = DISCARD_PORT;
                psys->merge_dsfwd  = 0;
            }
        }
        else  /* with nexthop */
        {
            psys->with_nh = 1;

            if (nhid == SYS_NH_RESOLVED_NHID_FOR_TOCPU)
            {
                is_cpu_nh = 1;
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, &p_nhinfo));
                if (p_nhinfo->hdr.nh_entry_type == SYS_NH_TYPE_MISC)
                {
                    p_nh_misc_info = (sys_nh_info_misc_t*)p_nhinfo;
                    if ((p_nh_misc_info->misc_nh_type == CTC_MISC_NH_TYPE_TO_CPU)&&CTC_IS_CPU_PORT(p_nh_misc_info->gport))
                    {
                        is_cpu_nh = 1;
                    }
                }
            }

            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info, 0));
            gport = (psys->is_logic_port || nh_info.ecmp_valid || nh_info.aps_en || nh_info.is_mcast\
                                                            || nh_info.is_ecmp_intf)?l2_addr->gport:nh_info.gport;
            psys->use_destmap_profile = nh_info.use_destmap_profile;
            if (is_cpu_nh && !psys->is_logic_port)
            {
                gport = SYS_RSV_PORT_OAM_CPU_ID;
            }
            else if(nh_info.drop_pkt && !psys->is_logic_port)
            {
                gport = DISCARD_PORT;
            }
            _sys_usw_l2_map_nh_to_sys(lchip, nhid, psys, gport_assign, gport, l2_addr->assign_port);

            /*just for white list entry mismatch src port*/
            if (CTC_FLAG_ISSET(psys->addr.l2_addr.flag, CTC_L2_FLAG_WHITE_LIST_ENTRY))   /*psys->flag.white_list */
            {
                psys->addr.l2_addr.gport = SYS_RSV_PORT_DROP_ID;
            }
        }

        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "psys  gport 0x%x\n", psys->addr.l2_addr.gport);

        psys->addr_type = SYS_L2_ADDR_TYPE_UC;
    }
    else if (L2_TYPE_MC == type)  /* mc*/
    {
        ctc_l2_mcast_addr_t * l2mc_addr = (ctc_l2_mcast_addr_t *) l2;
        sal_memcpy(&(psys->addr.l2mc_addr), l2mc_addr, sizeof(ctc_l2_mcast_addr_t));
        /* map hw entry */
        CTC_SET_FLAG(psys->addr.l2mc_addr.flag, CTC_L2_FLAG_IS_STATIC);  /*psys->flag.is_static = 1;*/
        psys->addr_type = SYS_L2_ADDR_TYPE_MC;
        psys->share_grp_en = l2mc_addr->share_grp_en;
    }
    else if (L2_TYPE_DF == type)  /*df*/
    {
        ctc_l2dflt_addr_t * l2df_addr = (ctc_l2dflt_addr_t *) l2;
        sal_memcpy(&(psys->addr.l2df_addr), l2df_addr, sizeof(ctc_l2dflt_addr_t));
        /* map hw entry */
         /*CTC_SET_FLAG(psys->flag, CTC_L2_FLAG_IS_STATIC);*/
        psys->addr_type = SYS_L2_ADDR_TYPE_DEFAULT;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_build_sys_ad_info_by_index(uint8 lchip, uint32 ad_index, sys_l2_info_t* p_sys)
{
    return _sys_usw_l2_decode_dsmac(lchip, p_sys, ad_index, NULL);
}

STATIC int32
_sys_usw_l2_map_dma_to_entry(uint8 lchip, uint16 entry_num, void* p_data,
                        ctc_l2_addr_t* pctc, sys_l2_detail_t* pi, uint16* rel_cnt)
{
    uint16 index = 0;
    DmaFibDumpFifo_m* p_dump_info = NULL;
    hw_mac_addr_t                mac_sa   = { 0 };
    uint32 key_index= 0;
    uint32 ad_index = 0;
    sys_l2_info_t       sys;
    uint16 fid = 0;
    uint16 index1 = 0;
    uint8 xgpon_en = 0;
    uint8  timer = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    *rel_cnt = 0;

    for (index1 = 0; index1 < entry_num; index1++)
    {
        sal_memset(&sys, 0, sizeof(sys_l2_info_t));

        p_dump_info = (DmaFibDumpFifo_m*)p_data+index1;
        fid = GetDmaFibDumpFifo(V, vsiId_f, p_dump_info);

        GetDmaFibDumpFifo(A, mappedMac_f, p_dump_info, mac_sa);
        SYS_USW_SET_USER_MAC(pctc[index].mac, mac_sa);
        key_index = GetDmaFibDumpFifo(V, keyIndex_f, p_dump_info);
        ad_index = GetDmaFibDumpFifo(V, dsAdIndex_f, p_dump_info);

        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (xgpon_en)
        {
            ad_index = ad_index & 0xDFFFF;
        }
        /*read dsmac */
        _sys_usw_l2_build_sys_ad_info_by_index(lchip, ad_index, &sys);
        CTC_ERROR_RETURN(sys_usw_aging_get_aging_timer(lchip, 1, key_index, &timer));
        if(timer == 0)
        {
            CTC_SET_FLAG(sys.addr.l2_addr.flag, CTC_L2_FLAG_AGING_DISABLE);
        }

        pctc[index].fid = fid;
        pctc[index].gport = sys.addr.l2_addr.gport;
        pctc[index].flag = sys.addr.l2_addr.flag;

        pctc[index].is_logic_port = SYS_L2_IS_LOGIC_PORT(pctc[index].fid);
        if(pi)
        {
            pi[index].ad_index = ad_index;
            pi[index].key_index = key_index;
            pi[index].flag = sys.addr.l2_addr.flag;
            pi[index].pending = GetDmaFibDumpFifo(V, pending_f, p_dump_info);
        }
        if (GetDmaFibDumpFifo(V, pending_f, p_dump_info))
        {
            CTC_SET_FLAG(pctc[index].flag, CTC_L2_FLAG_PENDING_ENTRY);
        }
        (*rel_cnt)++;
        index++;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_l2_get_blackhole_entry(uint8 lchip,
                                   ctc_l2_fdb_query_t* pq,
                                   ctc_l2_fdb_query_rst_t* pr,
                                   uint8  only_get_cnt,
                                   sys_l2_detail_t* detail_info)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 max_entry_cnt = pr->buffer_len/sizeof(ctc_l2_addr_t);
    DsFibMacBlackHoleHashKey_m tcam_key;
    sys_l2_detail_t*     temp_detail = detail_info;
    ctc_l2_addr_t*       temp_addr = pr->buffer;
    sys_l2_info_t       l2_info;
    uint32 index = 0;
    uint32 end_index = 0;
    hw_mac_addr_t hw_mac = {0};
    uint16 max_blackhole_entry_num = MCHIP_CAP(SYS_CAP_L2_BLACK_HOLE_ENTRY);
    uint32 ad_index = 0;

    pq->count = 0;
    if (pr->start_index >= max_blackhole_entry_num)
    {
        pr->is_end = 1;
        return CTC_E_NONE;
    }
    if ((pq->query_flag != CTC_L2_FDB_ENTRY_ALL)
        && (pq->query_flag != CTC_L2_FDB_ENTRY_STATIC))
    {
        pr->is_end = 1;
        return CTC_E_NONE;
    }

    end_index = ((pr->start_index + max_entry_cnt) >= max_blackhole_entry_num)
                ? max_blackhole_entry_num : (pr->start_index + max_entry_cnt);

    for (index = pr->start_index; index < end_index; index++)
    {
        sal_memset(&tcam_key, 0, sizeof(tcam_key));
        cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &tcam_key), ret, error_roll);
        if (!GetDsFibMacBlackHoleHashKey(V, valid_f, &tcam_key))
        {
            continue;
        }
        pq->count++;
        if (only_get_cnt)
        {
            continue;
        }
        temp_addr->fid = DONTCARE_FID;
        GetDsFibMacBlackHoleHashKey(A, mappedMac_f, &tcam_key, hw_mac);
        SYS_USW_SET_USER_MAC(temp_addr->mac, hw_mac);
        ad_index = GetDsFibMacBlackHoleHashKey(V, dsAdIndex_f, &tcam_key);
        /*read dsmac */
        sal_memset(&l2_info, 0, sizeof(l2_info));
        _sys_usw_l2_build_sys_ad_info_by_index(lchip, ad_index, &l2_info);
        temp_addr->gport = l2_info.addr.l2_addr.gport;
        temp_addr->flag = l2_info.addr.l2_addr.flag;
        if (temp_detail)
        {
            temp_detail->ad_index = ad_index;
            temp_detail->key_index = index;
            temp_detail->flag = l2_info.addr.l2_addr.flag;
            temp_detail++;
        }
        temp_addr++;
    }
    if (end_index >= max_blackhole_entry_num)
    {
        pr->is_end = 1;
    }
    pr->next_query_index = end_index;
    return CTC_E_NONE;

error_roll:
    pr->next_query_index = index;
    pr->is_end = 1;
    return ret;
}

/* dump all means dump ucast + mcast*/
STATIC int32
_sys_usw_l2_get_entry_by_hw(uint8 lchip,
                                   ctc_l2_fdb_query_t* query_rq,
                                   ctc_l2_fdb_query_rst_t* query_rst,
                                   uint8  only_get_cnt,
                                   uint8  dump_all,
                                   sys_l2_detail_t* detail_info)
{
    drv_acc_in_t  in;
    drv_acc_out_t out;
    uint32 max_index = 0;
    uint32 threshold_cnt = query_rst->buffer_len/sizeof(ctc_l2_addr_t);
    void* p_data = NULL;
    uint16 entry_num = 0;
    uint16 rel_cnt = 0;
    DsFibHost0MacHashKey_m fdb_key;
    int32 ret = 0;
    DMA_DUMP_FUN_P p_dma_dump_cb = NULL;
    void* user_data = NULL;
    uint8 is_end = 0;
    ctc_l2_addr_t* p_buffer = NULL;
    dma_dump_cb_parameter_t pa;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((query_rq->query_type == CTC_L2_FDB_ENTRY_OP_BY_VID)
        && (query_rq->fid == DONTCARE_FID))
    {
        return _sys_usw_l2_get_blackhole_entry(lchip, query_rq, query_rst, only_get_cnt, detail_info);
    }


    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));

    sal_memset(&fdb_key, 0, sizeof(DsFibHost0MacHashKey_m));

    p_data = (DmaFibDumpFifo_m*)mem_malloc(MEM_FDB_MODULE, sizeof(DmaFibDumpFifo_m)*threshold_cnt);
    if (p_data == NULL)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    /* set request for fib acc to dump hash key by fid */
    SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, query_rq->fid);
    in.type = DRV_ACC_TYPE_DUMP;
    in.tbl_id = DsFibHost0MacHashKey_t;
    in.index = query_rst->start_index;
    in.query_cnt = threshold_cnt;
    in.data = &fdb_key;

    if (query_rq->query_type == CTC_L2_FDB_ENTRY_OP_ALL)
    {
        in.op_type = DRV_ACC_OP_BY_ALL;
    }
    else if (query_rq->query_type == CTC_L2_FDB_ENTRY_OP_BY_VID)
    {
        in.op_type = DRV_ACC_OP_BY_VLAN;
    }
    else if (query_rq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT)
    {
        in.op_type = DRV_ACC_OP_BY_PORT;
    }
    else if (query_rq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN)
    {
        in.op_type = DRV_ACC_OP_BY_PORT_VLAN;
    }
    else
    {
        mem_free(p_data);
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    if (query_rq->use_logic_port)
    {
        in.gport = query_rq->gport;
        CTC_BIT_SET(in.flag, DRV_ACC_USE_LOGIC_PORT);
    }
    else
    {
        in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(query_rq->gport);
    }

    CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_UCAST);
    if(dump_all)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_STATIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_MCAST);
    }
    else if (query_rq->query_flag == CTC_L2_FDB_ENTRY_STATIC)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_STATIC);
    }
    else if (query_rq->query_flag == CTC_L2_FDB_ENTRY_DYNAMIC)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
    }
    else if (query_rq->query_flag == CTC_L2_FDB_ENTRY_ALL)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_STATIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
    }
    else if(query_rq->query_type == CTC_L2_FDB_ENTRY_OP_ALL && query_rq->query_flag == CTC_L2_FDB_ENTRY_MCAST)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_STATIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_MCAST);
        CTC_BIT_UNSET(in.flag, DRV_ACC_ENTRY_UCAST);
    }
    else if(query_rq->query_type == CTC_L2_FDB_ENTRY_OP_ALL && query_rq->query_flag == CTC_L2_FDB_ENTRY_PENDING)
    {
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_STATIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
        CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_PENDING);
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsFibHost0MacHashKey_t, &max_index);
    in.query_end_index = max_index+32;
    if(in.index >= in.query_end_index)
    {
        mem_free(p_data);
        query_rq->count = 0;
        query_rst->is_end = 1;
        return CTC_E_NONE;
    }
    L2_DUMP_LOCK;
    /**/
    CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out),ret, error);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "in.query_cnt:%d, out.query_cnt:%d, out.is_full:%d out.is_end:%d\n", in.query_cnt, out.query_cnt, out.is_full, out.is_end);
    query_rq->count = 0;

    do
    {
        if (out.query_cnt == 0)
        {
            /* Even if count is 0, but Dma memory still may have invalid entry, need clear, Eg:the last invalid entry */
            query_rst->is_end = 1;
            ret = CTC_E_NONE;

            if (DRV_IS_DUET2(lchip))
            {
                ret += sys_usw_dma_wait_desc_done(lchip, SYS_DMA_HASHKEY_CHAN_ID);
                goto error;
            }
            else
            {
                goto error0;
            }
        }

        /* get dump result from dma interface */
        CTC_ERROR_GOTO(sys_usw_dma_get_dump_cb(lchip, (void**)&p_dma_dump_cb, &user_data),ret, error);
        pa.threshold = in.query_cnt;
        pa.entry_count = out.query_cnt;
        pa.fifo_full = out.is_full;
        CTC_ERROR_GOTO((*p_dma_dump_cb)(lchip, &pa, &entry_num, p_data),ret, error);
        rel_cnt = entry_num;
        if(!only_get_cnt)
        {
            p_buffer = query_rst->buffer+query_rq->count;
            _sys_usw_l2_map_dma_to_entry(lchip, entry_num, p_data, p_buffer, detail_info ? (detail_info+query_rq->count) : NULL, &rel_cnt);
        }

        query_rq->count += rel_cnt;

        /* check out status */
        if (out.is_full)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dma FIFO is Full, continue to dump!\n");
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "          last query end_indesx:0x%x\n", out.key_index);
            is_end = 0;
            in.query_cnt -= entry_num;
            in.index = out.key_index;
            CTC_ERROR_GOTO(drv_acc_api(lchip, &in, &out),ret, error);
        }
        else
        {
            if (!out.is_end)
            {
                query_rst->is_end = 0;
                query_rst->next_query_index = out.key_index + 1;
            }
            else
            {
                query_rst->is_end = 1;
            }

            is_end = 1;
        }
    }while(!is_end);

    mem_free(p_data);
    L2_DUMP_UNLOCK;
    return CTC_E_NONE;
error:
    sys_usw_dma_clear_chan_data(lchip, SYS_DMA_HASHKEY_CHAN_ID);

error0:
    mem_free(p_data);
    L2_DUMP_UNLOCK;
    return ret;
}

/**
   @brief Query fdb enery according to specified query condition

 */
STATIC int32
_sys_usw_l2_query_flush_check(uint8 lchip, void* p, uint8 is_flush)
{
    uint8  type           = 0;
    uint16 fid            = 0;
    uint32 gport          = 0;
    uint8  use_logic_port = 0;
    uint8  flag           = 0;
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    if (is_flush)
    {
        ctc_l2_flush_fdb_t* pf = (ctc_l2_flush_fdb_t *) p;
        type           = pf->flush_type;
        fid            = pf->fid;
        use_logic_port = pf->use_logic_port;
        gport          = pf->gport;
        flag           = pf->flush_flag;
    }
    else
    {
        ctc_l2_fdb_query_t* pq = (ctc_l2_fdb_query_t *) p;
        type           = pq->query_type;
        fid            = pq->fid;
        use_logic_port = pq->use_logic_port;
        gport          = pq->gport;
        flag           = pq->query_flag;
    }
    if (CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC == flag)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    if ((CTC_L2_FDB_ENTRY_PENDING == flag || CTC_L2_FDB_ENTRY_MCAST == flag) && CTC_L2_FDB_ENTRY_OP_ALL != type)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    if ((type == CTC_L2_FDB_ENTRY_OP_BY_VID) ||
        (type == CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN) ||
        (type == CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN))
    {
        SYS_L2UC_FID_CHECK(fid); /* allow 0xFFFF */
    }

    if ((type == CTC_L2_FDB_ENTRY_OP_BY_PORT) ||
        (type == CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN))
    {
        if (!use_logic_port)
        {
            SYS_GLOBAL_PORT_CHECK(gport);
        }
        else
        {
            SYS_LOGIC_PORT_CHECK(gport);
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l2_unbind_nhid_by_logic_port(uint8 lchip, uint16 logic_port)
{
    sys_l2_vport_2_nhid_t *p_node;

    uint32 cmd = 0;
    DsMac_m ds_mac;

    p_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
    if(p_node)
    {
        sal_memset(&ds_mac, 0, sizeof(DsMac_m));
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_node->ad_idx, cmd, &ds_mac));

        if (pl2_usw_master[lchip]->vp_alloc_ad_en)
        {
            sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, p_node->ad_idx);
        }

        _sys_usw_l2_bind_nexthop(lchip, p_node, 0, 1);
        ctc_vector_del(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
        mem_free(p_node);
    }
    else
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_l2_bind_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 nhp_id)
{
    sys_l2_info_t          sys;
    sys_l2_vport_2_nhid_t* p_node;
    DsMac_m                ds_mac;
    uint32                 cmd  = 0;
    int32                  ret = 0;
    uint8                  is_new_node = 0;
    uint32                 old_nh_id = 0;
    sys_nh_info_dsnh_t     nh_info;

    /*get nhinfo and check*/
    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhp_id, &nh_info, 0));

    p_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
    if (NULL == p_node)
    {
        MALLOC_ZERO(MEM_FDB_MODULE, p_node, sizeof(sys_l2_vport_2_nhid_t));
        if (NULL == p_node)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;

        }
        is_new_node = 1;
        if (pl2_usw_master[lchip]->vp_alloc_ad_en)
        {
            if (!pl2_usw_master[lchip]->cfg_hw_learn)
            {
                CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, 1, 1, &p_node->ad_idx), ret, error);
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc ad index:0x%x\n", p_node->ad_idx);
            }
            else
            {
                uint32 ad_index = 0;
                cmd = DRV_IOR(FibAccelerationCtl_t, FibAccelerationCtl_dsMacBase0_f);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ad_index), ret, error);
                ad_index += logic_port;
                CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset_from_position(lchip, DsMac_t, 0, 1, ad_index), ret, error);
                p_node->ad_idx = ad_index;
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc ad index:0x%x\n", ad_index);
            }
        }
        else
        {
            p_node->ad_idx = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_VPORT) + logic_port;
        }

        p_node->nhid = nhp_id;
        ctc_vector_add(pl2_usw_master[lchip]->vport2nh_vec, logic_port, p_node);
    }
    else
    {
        if (p_node->nhid == nhp_id)
        {
            return CTC_E_NONE;
        }
        old_nh_id = p_node->nhid;
        p_node->nhid = nhp_id;
    }

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    if (!nh_info.bind_data && !nh_info.dsfwd_valid)
    {
        CTC_ERROR_GOTO(_sys_usw_l2_bind_nexthop(lchip, p_node, 1, 1), ret, error);
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_nh_get_dsfwd_offset(lchip, nhp_id, &sys.fwd_ptr, 0), ret, error);
    }

    /* write binding logic-port dsmac */
    sal_memset(&ds_mac, 0, sizeof(DsMac_m));

    sys.is_logic_port = 1;
    sys.addr.l2_addr.gport = logic_port;
    CTC_ERROR_GOTO(_sys_usw_l2_map_nh_to_sys(lchip, nhp_id, &sys, 0, logic_port, 0), ret, error);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Bind nhid by logic port: logic_port:0x%x, merge Dsfwd:%d\n", logic_port, sys.merge_dsfwd);
    CTC_ERROR_GOTO(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, &sys), ret, error);

    cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_node->ad_idx, cmd, &ds_mac), ret, error);

    /*unbind old nexthop*/
    if(!is_new_node && (old_nh_id != nhp_id))
    {
        sys_l2_vport_2_nhid_t  temp_node;
        sal_memset(&temp_node, 0, sizeof(temp_node));
        temp_node.nhid = old_nh_id;
        _sys_usw_l2_bind_nexthop(lchip, &temp_node, 0, 1);
    }

    return ret;
error:
    if(is_new_node)
    {
        ctc_vector_del(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
        if (pl2_usw_master[lchip]->vp_alloc_ad_en)
        {
            sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, p_node->ad_idx);
        }

        mem_free(p_node);
    }
    else
    {
        p_node->nhid = old_nh_id;
    }
    return ret;
}
int32
_sys_usw_l2_reset_hw_cb(sys_l2_node_t* p_l2_node, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    sys_l2_info_t sys;

    if(p_l2_node && p_l2_node->flag & CTC_L2_FLAG_IS_STATIC)
    {
        sal_memset(&sys, 0, sizeof(sys));
        sys.addr_type = SYS_L2_ADDR_TYPE_UC;
        sys.rsv_ad_index = 1;

        sal_memcpy((uint8*)&(sys.addr.l2_addr.mac), (uint8*)&(p_l2_node->mac), sizeof(mac_addr_t));
        sys.addr.l2_addr.fid = p_l2_node->fid;
        sys.addr.l2_addr.flag = p_l2_node->flag;

        sys.ad_index = p_l2_node->adptr->index;
        _sys_usw_l2_build_sys_ad_info_by_index(lchip,  p_l2_node->adptr->index, &sys);
        _sys_usw_l2_write_hw(lchip, &sys);
    }

    return CTC_E_NONE;
}
int32 sys_usw_fdb_reset_hw(uint8 lchip, void* p_user_data)
{
    SYS_L2_INIT_CHECK();
    L2_LOCK;
    ctc_hash_traverse(pl2_usw_master[lchip]->fdb_hash, (hash_traversal_fn)_sys_usw_l2_reset_hw_cb, &lchip);
    L2_UNLOCK;

    return CTC_E_NONE;
}

#if 0
int32
sys_usw_l2_fdb_set_policer(uint8 lchip, uint16 logic_port, uint16 policer_id)
{
    uint32 cmd = 0;
    uint32 dsmac_index = 0;
    uint16 policer_ptr = 0;
    DsMac_m dsmac;
    sys_l2_vport_2_nhid_t* p_node;
    sys_qos_policer_param_t policer_param;

    p_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, logic_port);
    dsmac_index = p_node->ad_idx;

    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));
    CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, policer_id,
                                              SYS_QOS_POLICER_TYPE_FLOW,
                                              &policer_param));

    sal_memset(&dsmac, 0, sizeof(DsMac_t));
    cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsmac_index, cmd, &dsmac));

    policer_ptr = policer_param.policer_ptr;
    SetDsMac(V, macDaPolicerEn_f, &dsmac, 1);
    SetDsMac(V, stpCheckDisable_f, &dsmac, (policer_ptr>>12) & 0x1);
    SetDsMac(V, macDaExceptionEn_f, &dsmac, (policer_ptr>>11) & 0x1);
    SetDsMac(V, exceptionSubIndex_f, &dsmac, (policer_ptr>>9) & 0x3);
    SetDsMac(V, categoryIdValid_f, &dsmac, (policer_ptr>>8) & 0x1);
    SetDsMac(V, categoryId_f, &dsmac, (policer_ptr) & 0xFF);

    cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, dsmac_index, cmd, &dsmac));
    return CTC_E_NONE;
}
#endif

#define __FDB_ENTRY__

STATIC int32
_sys_usw_l2_get_fdb_conflict(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr, sys_l2_detail_t* pi)
{
    int32 ret = 0;
    uint8 index = 0;
    uint16 buf_cnt = pr->buffer_len/sizeof(ctc_l2_addr_t);
    uint16 cf_max = 0;
    uint16 cf_count = 0;
    uint32 key_index = 0;
    sys_l2_calc_index_t calc_index;
    sys_l2_detail_t* l2_detail = NULL;
    ctc_l2_addr_t* cf_addr = NULL;
    sys_l2_detail_t* l2_detail_temp = NULL;
    ctc_l2_addr_t* cf_addr_temp = NULL;
    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(pq);
    CTC_PTR_VALID_CHECK(pr);
    SYS_L2UC_FID_CHECK(pq->fid);

    if (pq->fid == DONTCARE_FID)
    {
        return CTC_E_NOT_SUPPORT;
    }
    pq->count = 0;
    sal_memset(&calc_index, 0, sizeof(sys_l2_calc_index_t));
    CTC_ERROR_RETURN(_sys_usw_l2_fdb_calc_index(lchip, pq->mac, pq->fid, &calc_index));

    cf_max = calc_index.index_cnt + MCHIP_CAP(SYS_CAP_L2_FDB_CAM_NUM);
    cf_addr = (ctc_l2_addr_t*)mem_malloc(MEM_FDB_MODULE, cf_max * sizeof(ctc_l2_addr_t));
    if (cf_addr == NULL)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(cf_addr, 0, cf_max * sizeof(ctc_l2_addr_t));

    l2_detail = (sys_l2_detail_t*)mem_malloc(MEM_FDB_MODULE, cf_max * sizeof(sys_l2_detail_t));
    if (l2_detail == NULL)
    {
        mem_free(cf_addr);
        return CTC_E_NO_MEMORY;
    }
    sal_memset(l2_detail, 0, cf_max * sizeof(sys_l2_detail_t));
    l2_detail_temp = l2_detail;
    cf_addr_temp = cf_addr;

    for (index = 0; index < cf_max; index++)
    {
        key_index = (index >= calc_index.index_cnt) ? (index - calc_index.index_cnt) : calc_index.key_index[index];
        ret = sys_usw_l2_get_fdb_by_index(lchip, key_index, cf_addr_temp, l2_detail_temp);
        if (ret < 0)
        {
            if (ret == CTC_E_NOT_EXIST)
            {
                ret = CTC_E_NONE;
                continue;
                 /*goto error_return;*/
            }
            else
            {
                goto error_return;
            }
        }
        cf_addr_temp++;
        cf_count++;
        l2_detail_temp++;
    }

    pr->next_query_index = pr->start_index + buf_cnt;
    if (pr->next_query_index >= cf_count)
    {
        pr->next_query_index = cf_count;
        pr->is_end = 1;
    }

    if (pr->start_index >= cf_count)
    {
        goto error_return;
    }
    pq->count = pr->next_query_index - pr->start_index;
    sal_memcpy(pr->buffer, cf_addr + pr->start_index, pq->count * sizeof(ctc_l2_addr_t));

    if (pi)
    {
        sal_memcpy(pi, l2_detail + pr->start_index, pq->count * sizeof(sys_l2_detail_t));
    }

    mem_free(cf_addr);
    mem_free(l2_detail);

    return CTC_E_NONE;

error_return:
    pr->is_end = 1;
    mem_free(cf_addr);
    mem_free(l2_detail);

    return ret;
}



STATIC int32
_sys_usw_l2_get_fdb_entry_detail_by_mac_vlan(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr, sys_l2_detail_t* p_detail)
{
    mac_lookup_result_t     rslt ;
    sys_l2_info_t           sys ;
    int32  ret = CTC_E_NONE;
    uint32  key_index = 0;
    uint32  ad_index = 0;

    sal_memset(&rslt, 0, sizeof(mac_lookup_result_t));
    if(pq->fid == DONTCARE_FID)
    {
        if(!_sys_usw_l2_black_hole_lookup(lchip, pq->mac, &key_index, &ad_index))
        {
            ad_index = 0XFFFFFFFF;
        }
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, pq->mac, pq->fid, &rslt));
        if(rslt.hit)
        {
            key_index = rslt.key_index;
            ad_index = rslt.ad_index;
        }
        else
        {
            ad_index = 0xFFFFFFFF;
        }
    }

    if(ad_index != 0xFFFFFFFF)
    {
        uint8   timer = 0;
        sal_memset(&sys, 0, sizeof(sys_l2_info_t));

        ret = _sys_usw_l2_build_sys_ad_info_by_index(lchip, ad_index, &sys);
        CTC_ERROR_RETURN(sys_usw_aging_get_aging_timer(lchip, 1, key_index, &timer));
        if(timer == 0)
        {
            CTC_SET_FLAG(sys.addr.l2_addr.flag, CTC_L2_FLAG_AGING_DISABLE);
        }

        /*check flag*/
        if( (CTC_L2_FDB_ENTRY_STATIC == pq->query_flag && !CTC_FLAG_ISSET(sys.addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC) )||
             (CTC_L2_FDB_ENTRY_DYNAMIC == pq->query_flag && CTC_FLAG_ISSET(sys.addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC)))
        {
            goto end;
        }

        if (rslt.pending)
        {
            CTC_SET_FLAG(sys.addr.l2_addr.flag, CTC_L2_FLAG_PENDING_ENTRY);
        }
        /*key*/
        sal_memcpy(&sys.addr.l2_addr.mac, &pq->mac, sizeof(mac_addr_t));
        sys.addr.l2_addr.fid = pq->fid;
        if(pr->buffer_len && pr->buffer)
        {
            sal_memcpy((uint8*)pr->buffer, (uint8*)&sys.addr.l2_addr, sizeof(ctc_l2_addr_t));
            pq->count = 1;
        }
        else
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "No buffer for get fdb entry!!!!!!!!!!!!!!!!\n");
        }


        if(p_detail)
        {
            p_detail->ad_index = ad_index;
            p_detail->key_index = key_index;
            p_detail->pending = rslt.pending;
            p_detail->flag = sys.addr.l2_addr.flag;

        }
    }

end:
    pr->is_end = 1;
    return ret;
}

STATIC int32
_sys_usw_l2_get_fdb_entry_detail_by_mac(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr, sys_l2_detail_t* p_detail)
{
    ctc_l2_fdb_query_t      temp_query ;
    ctc_l2_fdb_query_rst_t  temp_query_rst;
    sys_l2_detail_t*        temp_detail = NULL;
    uint32 max_entry_cnt = 0;
    uint32 loop = 0;
    int32  ret = CTC_E_NONE;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&temp_query, pq, sizeof(temp_query));
    sal_memcpy(&temp_query_rst, pr, sizeof(temp_query_rst));

    max_entry_cnt = pr->buffer_len/sizeof(ctc_l2_addr_t);
    temp_detail = (sys_l2_detail_t*)mem_malloc(MEM_FDB_MODULE, DUMP_MAX_ENTRY*sizeof(sys_l2_detail_t));
    if(!temp_detail)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    temp_query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    temp_query_rst.buffer = (ctc_l2_addr_t*)mem_malloc(MEM_FDB_MODULE, DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t));
    if(!temp_query_rst.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }
    temp_query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);
    temp_query_rst.next_query_index = pr->start_index;

    do
    {
        temp_query_rst.start_index = temp_query_rst.next_query_index;
        CTC_ERROR_GOTO(_sys_usw_l2_get_entry_by_hw(lchip, &temp_query, &temp_query_rst, 0, 0, temp_detail), ret ,error2);
        pr->is_end = temp_query_rst.is_end;

        for(loop=0; loop<temp_query.count && pq->count<max_entry_cnt;loop++)
        {
            if(sal_memcmp(&pq->mac, &(temp_query_rst.buffer+loop)->mac, sizeof(mac_addr_t)))
            {
                continue;
            }
            sal_memcpy((uint8*)((ctc_l2_addr_t*)pr->buffer+pq->count), (uint8*)((ctc_l2_addr_t*)temp_query_rst.buffer+loop), sizeof(ctc_l2_addr_t));
            if(p_detail)
            {
                sal_memcpy((uint8*)(p_detail+pq->count), (uint8*)(temp_detail+loop), sizeof(sys_l2_detail_t));
            }
            pq->count++;

        }

        /*The user's buffer is full but there are entries in hardware table*/
        if(pq->count == max_entry_cnt && loop < temp_query.count)
        {
			pr->next_query_index = (temp_detail+loop)->key_index;
            pr->is_end = 0;
            break;
        }

    }while(!temp_query_rst.is_end);

error2:
    mem_free(temp_query_rst.buffer);
error1:
    mem_free(temp_detail);
error0:
    return ret;
}

STATIC int32
_sys_usw_l2_get_dstmp_profile_cb(void* p_bucket_node, uint32 cur_hash_node_id, uint32 cur_list_id, void* user_data)
{
    ctc_hash_bucket_t* bucket_node = (ctc_hash_bucket_t*)p_bucket_node;
    sys_l2_node_t* p_l2_node = NULL;
    sys_l2_destmap_profile_fdb_t* user_info = (sys_l2_destmap_profile_fdb_t*)user_data;
    uint8 lchip = user_info->lchip;
    ctc_l2_addr_t* p_user_addr_buf = NULL;
    sys_l2_info_t l2_info_sys;
    int32 ret = CTC_E_NONE;

    sal_memset(&l2_info_sys, 0, sizeof(sys_l2_info_t));

    if (user_info->pr->buffer_len <  sizeof(ctc_l2_addr_t))
    {
         return CTC_E_NO_MEMORY;
    }

    p_l2_node = (sys_l2_node_t*)(bucket_node->data);
    if(p_l2_node && p_l2_node->use_destmap_profile)
    {
        /*get sw data to user_buf*/
        _sys_usw_l2_decode_dsmac(lchip, &l2_info_sys, p_l2_node->adptr->index, &(p_l2_node->adptr->ds_mac));

        if (user_info->pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_VID)
        {
            if(user_info->pq->fid != p_l2_node->fid)
            {
                ret =  CTC_E_NONE;
                goto done;
            }

        }
        else if (user_info->pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT)
        {
            if(l2_info_sys.is_logic_port != user_info->pq->use_logic_port || user_info->pq->gport != l2_info_sys.addr.l2_addr.gport)
            {
                ret =  CTC_E_NONE;
                goto done;
            }
        }
        else if (user_info->pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN)
        {
            if((user_info->pq->fid != p_l2_node->fid) || (user_info->pq->gport != l2_info_sys.addr.l2_addr.gport))
            {
                ret = CTC_E_NONE;
                goto done;
            }
        }

        p_user_addr_buf = user_info->pr->buffer;
        if (user_info->p_detail)
        {
            user_info->p_detail->key_index = p_l2_node->key_index;
            user_info->p_detail->ad_index = p_l2_node->adptr->index;
        }
        sal_memcpy(p_user_addr_buf , &l2_info_sys.addr.l2_addr, sizeof(ctc_l2_addr_t));
        sal_memcpy(p_user_addr_buf->mac , p_l2_node->mac, sizeof(mac_addr_t));
        p_user_addr_buf->fid = p_l2_node->fid;

        if (user_info->p_detail)
        {
            user_info->p_detail->pending = 0;
            user_info->p_detail->flag = p_user_addr_buf->flag;
        }

        (user_info->pq->count) ++;
        if (user_info->pr->buffer_len >= sizeof(ctc_l2_addr_t))
        {
            user_info->pr->buffer_len -= sizeof(ctc_l2_addr_t);
            user_info->pr->buffer ++;
            if (user_info->p_detail)
            {
                user_info->p_detail ++;
            }
        }
        else
        {
            /*user buf full*/
        }
    }

 done:
    if (bucket_node->next == NULL)
    {
        cur_hash_node_id +=  1;
        cur_list_id      =   0;
    }
    else
    {
        cur_hash_node_id += 0;
        cur_list_id      += 1;
    }
    FDB_MAC_FID_SW_QUERY_IDX_ENCODE(user_info->pr->next_query_index, cur_hash_node_id, cur_list_id);
    return ret;
}


STATIC int32
_sys_usw_l2_get_fdb_entry_detail(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr, sys_l2_detail_t* p_detail)
{
    int ret = CTC_E_NONE;
    sys_l2_destmap_profile_fdb_t user_data;
    uint32 sw_get_cnt = 0;
    uint8  is_sw_multi = 0;
    uint32 hash_node_id = 0;
    uint32 list_node_id = 0;
    ctc_l2_addr_t* p_user_buf_old = NULL;
    uint32  user_buf_len_old = 0;
    sys_l2_detail_t* p_temp_detail = NULL;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(pq->query_flag == CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        ret =  CTC_E_NOT_SUPPORT;
        goto done;

    }
    if ((pq->query_type != CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN)
        && (pq->query_flag == CTC_L2_FDB_ENTRY_CONFLICT))
    {
        ret =  CTC_E_NOT_SUPPORT;
        goto done;
    }

    if(pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_MAC)
    {
        ret = _sys_usw_l2_get_fdb_entry_detail_by_mac(lchip, pq, pr, p_detail);
        goto done;

    }
    else if(pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN)
    {
        if (pq->query_flag == CTC_L2_FDB_ENTRY_CONFLICT)
        {
            ret = _sys_usw_l2_get_fdb_conflict(lchip, pq, pr, p_detail);
            goto done;
        }
        else
        {
            ret = _sys_usw_l2_get_fdb_entry_detail_by_mac_vlan(lchip, pq, pr, p_detail);
            goto done;
        }
    }
    else
    {
        /*transfer info from pr->start_index*/
        FDB_MAC_FID_SW_QUERY_IDX_DECODE(pr->start_index, is_sw_multi, hash_node_id, list_node_id);
        p_user_buf_old = pr->buffer;
        user_buf_len_old = pr->buffer_len;
        /*get software*/
        if (((pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_VID) || (pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT) || (pq->query_type == CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN))
            &&((pr->start_index == 0) || (0 != is_sw_multi)))
        {
            pq->count = 0;
            user_data.lchip = lchip;
            user_data.pq = pq;
            user_data.pr = pr;
            user_data.p_detail = p_detail;
            ctc_hash_traverse3(pl2_usw_master[lchip]->fdb_hash, (hash_traversal_3_fn)_sys_usw_l2_get_dstmp_profile_cb, &user_data, hash_node_id, list_node_id);
            sw_get_cnt = pq->count;
        }

        /*get hardware*/
        if (pr->buffer_len >= sizeof(ctc_l2_addr_t))
        {
            pq->count = 0;
            pr->start_index = is_sw_multi ? 0 : pr->start_index ;
            p_temp_detail = p_detail ? p_detail + sw_get_cnt : p_detail;
            ret =  _sys_usw_l2_get_entry_by_hw(lchip, pq, pr, 0, 0, p_temp_detail);
            pq->count += sw_get_cnt;
        }
        pr->buffer = p_user_buf_old;
        pr->buffer_len = user_buf_len_old;
    }

done:
    return ret;
}


/* call back funtion fot trie sort*/
STATIC int32
sys_usw_l2_get_fdb_entry_cb(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr)
{
    CTC_PTR_VALID_CHECK(pq);
    CTC_PTR_VALID_CHECK(pr);

    pq->query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    pq->query_flag = CTC_L2_FDB_ENTRY_ALL;

    return _sys_usw_l2_get_entry_by_hw(lchip, pq, pr, 0,1, NULL);
}

int32
sys_usw_l2_get_fdb_entry(uint8 lchip, ctc_l2_fdb_query_t* pq,
                                ctc_l2_fdb_query_rst_t* pr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(pq);
    CTC_PTR_VALID_CHECK(pr);

    CTC_ERROR_RETURN(_sys_usw_l2_query_flush_check(lchip, pq, 0));

    pq->count = 0;
    pr->next_query_index = 0;
    pr->is_end = 0;
    return _sys_usw_l2_get_fdb_entry_detail(lchip, pq, pr, NULL);
}

#define _FDB_COUNT_
/**
 * get some type fdb count by specified port,not include the default entry num
 */

#define GET_STATIC_ENTRY_CNT \
do{ \
    query.query_flag = CTC_L2_FDB_ENTRY_STATIC;\
    query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);\
    query_rst.next_query_index = 0;\
    do\
    {\
        query_rst.start_index = query_rst.next_query_index;\
        CTC_ERROR_RETURN(_sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 1, 0, NULL));\
        static_entry_cnt += query.count;\
    }while(!query_rst.is_end);\
}while(0)

STATIC int32
_sys_usw_l2_get_count_by_port(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    ctc_l2_fdb_query_rst_t   query_rst ;
    ctc_l2_fdb_query_t   query  ;
    uint32   mac_limit_cnt = 0;
    uint32   static_entry_cnt = 0;
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&query, pq, sizeof(query));
    sal_memset(&query_rst, 0, sizeof(query_rst));

    /*use logic port*/
    if(pq->use_logic_port)
    {
        query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);
        query_rst.next_query_index = 0;
        do
        {
            query_rst.start_index = query_rst.next_query_index;
            CTC_ERROR_RETURN(_sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 1, 0, NULL));
            pq->count += query.count;
        }while(!query_rst.is_end);

        return CTC_E_NONE;
    }

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.tbl_id = DsMacLimitCount_t;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_PORT;
    in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(pq->gport);
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    mac_limit_cnt = out.query_cnt;

    switch (pq->query_flag)
    {
    case CTC_L2_FDB_ENTRY_STATIC:
        GET_STATIC_ENTRY_CNT;
        pq->count = static_entry_cnt;
        break;

    case  CTC_L2_FDB_ENTRY_DYNAMIC:
        if (pl2_usw_master[lchip]->static_fdb_limit)
        {
            GET_STATIC_ENTRY_CNT;
        }

        pq->count = mac_limit_cnt - static_entry_cnt;
        break;
    case CTC_L2_FDB_ENTRY_ALL:
        if (!pl2_usw_master[lchip]->static_fdb_limit)
        {
            GET_STATIC_ENTRY_CNT;
        }
        pq->count = mac_limit_cnt + static_entry_cnt;

    default:
        break;
    }

    return CTC_E_NONE;
}



/**
 * get some type fdb count by specified vid,not include the default entry num
 */
STATIC int32
_sys_usw_l2_get_count_by_fid(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    ctc_l2_fdb_query_rst_t   query_rst ;
    ctc_l2_fdb_query_t   query ;
    uint32   mac_limit_cnt = 0;
    uint32   static_entry_cnt = 0;


    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    sal_memcpy(&query, pq, sizeof(query));
    sal_memset(&query_rst, 0, sizeof(query_rst));



    if((pq->fid <= CTC_MAX_VLAN_ID) && (CTC_L2_FDB_ENTRY_STATIC != pq->query_flag))
    {
        drv_acc_in_t in;
        drv_acc_out_t out;
        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));

        in.tbl_id = DsMacLimitCount_t;
        in.type = DRV_ACC_TYPE_LOOKUP;
        in.op_type = DRV_ACC_OP_BY_VLAN;
        in.data = &(pq->fid);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
        mac_limit_cnt = out.query_cnt;

        switch (pq->query_flag)
        {
        case  CTC_L2_FDB_ENTRY_DYNAMIC:
            if (pl2_usw_master[lchip]->static_fdb_limit)
            {
                GET_STATIC_ENTRY_CNT;
            }

            pq->count = mac_limit_cnt - static_entry_cnt;
            break;

        case CTC_L2_FDB_ENTRY_ALL:
            if (!pl2_usw_master[lchip]->static_fdb_limit)
            {
                GET_STATIC_ENTRY_CNT;
            }
            pq->count = mac_limit_cnt + static_entry_cnt;

        default:
            break;
        }

    }
    else
    {
        sal_memcpy(&query, pq, sizeof(query));

        query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);
        query_rst.next_query_index = 0;
        do
        {
            query_rst.start_index = query_rst.next_query_index;
            CTC_ERROR_RETURN(_sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 1, 0, NULL));
            pq->count += query.count;
        }while(!query_rst.is_end);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_get_count_by_port_fid(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    ctc_l2_fdb_query_rst_t   query_rst ;
    ctc_l2_fdb_query_t   query;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&query, pq, sizeof(query));
    sal_memset(&query_rst, 0, sizeof(query_rst));
    query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);
    query_rst.next_query_index = 0;
    do
    {
        query_rst.start_index = query_rst.next_query_index;
        CTC_ERROR_RETURN(_sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 1, 0, NULL));
        pq->count += query.count;
    }while(!query_rst.is_end);

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_l2_get_count_by_mac(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    int32 ret = CTC_E_NONE;
    uint32 loop = 0;
    ctc_l2_fdb_query_rst_t   query_rst;
    ctc_l2_fdb_query_t       query;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&query, pq, sizeof(query));
    sal_memset(&query_rst, 0, sizeof(query_rst));

    query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    query_rst.buffer = (ctc_l2_addr_t*)mem_malloc(MEM_FDB_MODULE, DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t));
    if (NULL == query_rst.buffer)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " no memory \n ");
        return CTC_E_NO_MEMORY;
    }
    query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);
    query_rst.next_query_index = 0;
    do
    {
        query_rst.start_index = query_rst.next_query_index;
        CTC_ERROR_GOTO(_sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 0, -0, NULL),ret, error);

        for(loop=0; loop<query.count;loop++)
        {
            if(!sal_memcmp(&pq->mac, &(query_rst.buffer+loop)->mac, sizeof(mac_addr_t)))
            {
                pq->count++;
            }
        }
    }while(!query_rst.is_end);

error:
    mem_free(query_rst.buffer);
    return ret;

}


/**
   @brief only count unicast FDB entries
 */
STATIC int32
_sys_usw_l2_get_count_by_all(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    ctc_l2_fdb_query_rst_t   query_rst;
    ctc_l2_fdb_query_t   query;
    uint32   static_entry_cnt = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memcpy(&query, pq, sizeof(query));
    sal_memset(&query_rst, 0, sizeof(query_rst));

    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));

    in.tbl_id = DsMacLimitCount_t;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_ALL;

    switch (pq->query_flag)
    {
    case CTC_L2_FDB_ENTRY_STATIC:
        GET_STATIC_ENTRY_CNT;
        pq->count = static_entry_cnt;
        break;

    case  CTC_L2_FDB_ENTRY_DYNAMIC:
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

        pq->count = out.query_cnt;
        break;
    case CTC_L2_FDB_ENTRY_ALL:
        CTC_BIT_SET(in.flag, DRV_ACC_STATIC_MAC_LIMIT_CNT);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

        pq->count = out.query_cnt;

    default:
        break;
    }

    return CTC_E_NONE;
}

/**
 * get some type fdb count, not include the default entry num
 */
int32
sys_usw_l2_get_fdb_count(uint8 lchip, ctc_l2_fdb_query_t* pq)
{
    mac_lookup_result_t rslt;
    sys_l2_info_t sys;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(pq);
    CTC_MAX_VALUE_CHECK(pq->query_flag, MAX_CTC_L2_FDB_ENTRY_OP - 1);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_l2_query_flush_check(lchip, pq, 0));

    pq->count = 0;

    switch (pq->query_type)
    {
    case CTC_L2_FDB_ENTRY_OP_BY_VID:

        return _sys_usw_l2_get_count_by_fid(lchip, pq);

    case CTC_L2_FDB_ENTRY_OP_BY_PORT:
        return _sys_usw_l2_get_count_by_port(lchip, pq);

    case CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN:
        return _sys_usw_l2_get_count_by_port_fid(lchip, pq);
    case CTC_L2_FDB_ENTRY_OP_BY_MAC:
        return _sys_usw_l2_get_count_by_mac(lchip, pq);
    case CTC_L2_FDB_ENTRY_OP_ALL:
        return _sys_usw_l2_get_count_by_all(lchip, pq);
        break;

    case CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN:
        CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, pq->mac, pq->fid, &rslt));
        sal_memset(&sys, 0, sizeof(sys_l2_info_t));
        _sys_usw_l2_build_sys_ad_info_by_index(lchip, rslt.ad_index, &sys);
        RETURN_IF_FLAG_NOT_MATCH(pq->query_flag, sys.addr.l2_addr.flag);
        pq->count = (rslt.hit);

        break;

    default:
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    return CTC_E_NONE;
}

#define __SPOOL__
/* share pool for uc only. and us distinguished by fwdptr */
uint32
_sys_usw_l2_ad_spool_hash_make(sys_l2_ad_t* p_ad)
{
        return ctc_hash_caculate(sizeof(p_ad->ds_mac), &p_ad->ds_mac);
}

int
_sys_usw_l2_ad_spool_hash_cmp(sys_l2_ad_t* p_ad1, sys_l2_ad_t* p_ad2)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_ad1 || !p_ad2)
    {
        return FALSE;
    }

    if (!sal_memcmp(&p_ad1->ds_mac, &p_ad2->ds_mac, sizeof(DsMac_m)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_l2_ad_spool_alloc_index(sys_l2_ad_t* p_ad, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    uint32 ad_index = 0;
    int32 ret = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        ret = sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 1, 1, 1, &ad_index);
        if(ret)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spool alloc index failed,  line=%d\n",__LINE__);
            return -1;
        }
        p_ad->index = ad_index;
    }

    return 0;
}

int32
_sys_usw_l2_ad_spool_free_index(sys_l2_ad_t* p_ad, void* user_data)
{


    uint8 lchip = *(uint8*)user_data;
    uint32 ad_index = p_ad->index;
    int32 ret = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_ad);

    ret = sys_usw_ftm_free_table_offset(lchip, DsMac_t, 1, 1, ad_index);

    if(ret)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spool fress index failed,  line=%d\n",__LINE__);
        return -1;
    }

    return 0;
}

int32
sys_usw_l2_hw_sync_add_db(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index)
{
    pl2_usw_master[lchip]->sync_add_db(lchip, l2_addr, nhid, index);
    return CTC_E_NONE;
}

int32
sys_usw_l2_hw_sync_remove_db(uint8 lchip, ctc_l2_addr_t* l2_addr)
{
    pl2_usw_master[lchip]->sync_remove_db(lchip, l2_addr);
    return CTC_E_NONE;
}

int32
sys_usw_l2_hw_sync_alloc_ad_idx(uint8 lchip, uint32* ad_index)
{
    return sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, 1, 1, ad_index);
}

int32
sys_usw_l2_hw_sync_free_ad_idx(uint8 lchip, uint32 ad_index)
{
    return sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, ad_index);
}


int32
_sys_usw_l2_flush_by_hw(uint8 lchip, ctc_l2_flush_fdb_t* p_flush, uint8 mac_limit_en, uint8 is_logic)
{
    drv_acc_in_t  acc_in;
    drv_acc_out_t acc_out;
    DsFibHost0MacHashKey_m fdb_key;
    hw_mac_addr_t   hw_mac = {0};

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();

    sal_memset(&acc_in, 0, sizeof(acc_in));
    sal_memset(&acc_out, 0, sizeof(acc_out));
    sal_memset(&fdb_key, 0, sizeof(DsFibHost0MacHashKey_m));

    if (CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC == p_flush->flush_flag)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    if (is_logic)
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_USE_LOGIC_PORT);
    }

    acc_in.type = DRV_ACC_TYPE_FLUSH;

    switch (p_flush->flush_type)
    {
    /* maybe need to add flush by port +vid */

    case CTC_L2_FDB_ENTRY_OP_BY_VID:

        acc_in.op_type = DRV_ACC_OP_BY_VLAN;
        SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, p_flush->fid);

        break;

    case CTC_L2_FDB_ENTRY_OP_BY_PORT:
        acc_in.op_type = DRV_ACC_OP_BY_PORT;

        if (p_flush->use_logic_port)
        {
            CTC_BIT_SET(acc_in.flag, DRV_ACC_USE_LOGIC_PORT);
            acc_in.gport = p_flush->gport;
        }
        else
        {
            acc_in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_flush->gport);
        }

        break;

    case CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN:
        acc_in.op_type =  DRV_ACC_OP_BY_PORT_VLAN;
        SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, p_flush->fid);
        if (p_flush->use_logic_port)
        {
            CTC_BIT_SET(acc_in.flag, DRV_ACC_USE_LOGIC_PORT);
            acc_in.gport = p_flush->gport;
        }
        else
        {
            acc_in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_flush->gport);
        }
        break;

    case CTC_L2_FDB_ENTRY_OP_BY_MAC:
        acc_in.op_type = DRV_ACC_OP_BY_MAC;
        FDB_SET_HW_MAC(hw_mac, p_flush->mac);
        SetDsFibHost0MacHashKey(A, mappedMac_f, &fdb_key, hw_mac);
        break;


    case CTC_L2_FDB_ENTRY_OP_ALL:
        acc_in.op_type = DRV_ACC_OP_BY_ALL;
        break;

    default:
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    /* add for mac limit*/
    if (0 != mac_limit_en)
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT);
        if (pl2_usw_master[lchip]->static_fdb_limit)
        {
            CTC_BIT_SET(acc_in.flag, DRV_ACC_STATIC_MAC_LIMIT_CNT);
        }
    }

    acc_in.tbl_id = DsFibHost0MacHashKey_t;
    acc_in.data = &fdb_key;

    switch (p_flush->flush_flag)
    {
        case CTC_L2_FDB_ENTRY_STATIC:
            CTC_BIT_SET(acc_in.flag, DRV_ACC_ENTRY_STATIC);
            break;

        case CTC_L2_FDB_ENTRY_DYNAMIC:
        case CTC_L2_FDB_ENTRY_ALL:
            CTC_BIT_SET(acc_in.flag, DRV_ACC_ENTRY_DYNAMIC);
            break;
        default:
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }


    CTC_ERROR_RETURN(drv_acc_api(lchip, &acc_in, &acc_out));

    return CTC_E_NONE;
}

int32
sys_usw_l2_delete_hw_by_mac_fid(uint8 lchip, mac_addr_t mac, uint16 fid, uint8 mac_limit_en, uint8 is_logic, uint8 is_static)
{
    /*remove fail not return for syn with system*/
    hw_mac_addr_t    hw_mac;
    drv_acc_in_t   acc_in;
    drv_acc_out_t  acc_out;
    DsFibHost0MacHashKey_m fdb_key ;

    SYS_L2_INIT_CHECK();

    sal_memset(&acc_in, 0, sizeof(acc_in));
    sal_memset(&acc_out, 0, sizeof(acc_out));
    sal_memset(&fdb_key, 0, sizeof(fdb_key));

    acc_in.type = DRV_ACC_TYPE_DEL;
    acc_in.op_type = DRV_ACC_OP_BY_KEY;
    FDB_SET_HW_MAC(hw_mac, mac);
    SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, fid);
    SetDsFibHost0MacHashKey(A, mappedMac_f, &fdb_key, hw_mac);

    acc_in.tbl_id = DsFibHost0MacHashKey_t;
    acc_in.data = &fdb_key;

    /* add for mac limit*/
    if (0 != mac_limit_en)
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT);
        if (pl2_usw_master[lchip]->static_fdb_limit)
        {
            CTC_BIT_SET(acc_in.flag, DRV_ACC_STATIC_MAC_LIMIT_CNT);
        }
    }

    if (is_logic)
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_USE_LOGIC_PORT);
    }

    if(is_static)
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_ENTRY_STATIC);

    }
    else
    {
        CTC_BIT_SET(acc_in.flag, DRV_ACC_ENTRY_DYNAMIC);
    }

    CTC_ERROR_RETURN(drv_acc_api(lchip, &acc_in, &acc_out));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_flush_fdb_cb(sys_l2_node_t* p_l2_node, sys_l2_flush_fdb_t* pf)
{
    uint8    is_dynamic = 0;
    uint8    mac_limit_en = 0;
    uint8    is_logic = 0;
    uint8    lchip = pf->lchip;
    uint16   gport = 0;
    uint8    learn_source = 0;
    /*system rsverved*/
    if(CTC_FLAG_ISSET(p_l2_node->flag, CTC_L2_FLAG_SYSTEM_RSV))
    {
        return 0;
    }

    is_logic = pf->use_logic_port;
    learn_source = GetDsMac(V,learnSource_f, &p_l2_node->adptr->ds_mac);
    switch(pf->flush_type)
    {
        case CTC_L2_FDB_ENTRY_OP_BY_MAC:
            if(sal_memcmp(&p_l2_node->mac, &pf->mac,sizeof(mac_addr_t)))
            {
                return 0;
            }
            break;

        case CTC_L2_FDB_ENTRY_OP_BY_VID:
            if(pf->fid != p_l2_node->fid)
            {
                return 0;
            }
            break;
        case CTC_L2_FDB_ENTRY_OP_BY_PORT:
            if(learn_source)
            {
                gport = GetDsMac(V,logicSrcPort_f, &p_l2_node->adptr->ds_mac);
            }
            else
            {
                gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsMac(V, globalSrcPort_f, &p_l2_node->adptr->ds_mac));
            }
            /*SYSTEM modified by yangl for sdk bug 108347*/
            if((learn_source != is_logic) || (pf->gport != gport))
            {
                return 0;
            }
            break;
        case CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN:
            if(learn_source)
            {
                gport = GetDsMac(V,logicSrcPort_f, &p_l2_node->adptr->ds_mac);
            }
            else
            {
                gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(GetDsMac(V, globalSrcPort_f, &p_l2_node->adptr->ds_mac));
            }
            /*SYSTEM modified by yangl for sdk bug 108347*/
            if((learn_source != is_logic) || (pf->gport != gport) ||( pf->fid != p_l2_node->fid ))
            {
                return 0;
            }
            break;

        case CTC_L2_FDB_ENTRY_OP_ALL:
            break;
        default:
            return 0;

    }
    is_dynamic = !CTC_FLAG_ISSET(p_l2_node->flag, CTC_L2_FLAG_IS_STATIC) && !(p_l2_node->ecmp_valid) ;
    mac_limit_en = (is_dynamic || pl2_usw_master[lchip]->static_fdb_limit)?1:0;

    if((pf->flush_flag == CTC_L2_FDB_ENTRY_STATIC && is_dynamic) ||
                        (pf->flush_flag == CTC_L2_FDB_ENTRY_DYNAMIC && !is_dynamic))
    {
        return 0 ;
    }

    sys_usw_l2_delete_hw_by_mac_fid(lchip, p_l2_node->mac, p_l2_node->fid, mac_limit_en, is_logic, CTC_FLAG_ISSET(p_l2_node->flag, CTC_L2_FLAG_IS_STATIC));

    if(p_l2_node->adptr)
    {
        if (p_l2_node->bind_nh)
        {
            _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 0, 0);
        }
        if (p_l2_node->rsv_ad)
        {
            mem_free(p_l2_node->adptr);
        }
        else
        {
            ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, p_l2_node->adptr, NULL);
        }
    }
    mem_free(p_l2_node);
    return 1;
}

STATIC int32
_sys_usw_l2_unbinding_logic_ecmp(uint8 lchip, ctc_l2_flush_fdb_t* pf, uint8 unbinding, void* p_mac)
{
    sys_l2_fid_node_t* p_fid_node = NULL;
    sys_l2_vport_2_nhid_t* p_vport_node = NULL;
    uint32                 cmd     = 0;
    sys_nh_info_dsnh_t p_nhinfo;
    DsMac_m              ds_mac;

    sal_memset(&p_nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&ds_mac, 0, sizeof(ds_mac));

    if (!pl2_usw_master[lchip]->cfg_hw_learn || (0 == pf->use_logic_port) || (CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN != pf->flush_type))
    {
        return CTC_E_NONE;
    }

    p_vport_node = ctc_vector_get(pl2_usw_master[lchip]->vport2nh_vec, pf->gport);
    p_fid_node = ctc_vector_get(pl2_usw_master[lchip]->fid_vec, pf->fid);
    if ((NULL == p_vport_node)||(NULL == p_fid_node))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_vport_node->nhid, &p_nhinfo, 0));
    if (!p_nhinfo.ecmp_valid)
    {
        return CTC_E_NONE;
    }

    if (unbinding)/* set to vlan default entry when flushing*/
    {
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_vport_node->ad_idx, cmd, &ds_mac));
        sal_memcpy(p_mac, &ds_mac, sizeof(DsMac_m));

        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, DEFAULT_ENTRY_INDEX(pf->fid), cmd, &ds_mac));
        SetDsMac(V, logicSrcPort_f, &ds_mac, pf->gport);
        SetDsMac(V, isStatic_f, &ds_mac, 0);
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_vport_node->ad_idx, cmd, &ds_mac));
    }
    else /* recover to nexthop after flush*/
    {
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_vport_node->ad_idx, cmd, p_mac));
    }

    return CTC_E_NONE;
}
int32
_sys_usw_l2_flush_fdb_by_fid_cb(void* array_data, void* user_data)
{
    sys_l2_fid_node_t *p_fid = (sys_l2_fid_node_t *)array_data;
    sys_l2_flush_fdb_t* pf = (sys_l2_flush_fdb_t*)user_data;
    ctc_l2_flush_fdb_t flush;
    if(!CTC_FLAG_ISSET(p_fid->flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT))
    {
        return CTC_E_NONE;
    }

    sal_memset(&flush, 0, sizeof(flush));
    flush.flush_flag = pf->flush_flag;
    flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
    flush.fid = p_fid->fid;

    return _sys_usw_l2_flush_by_hw(pf->lchip, &flush, 1, 1);
}
int32
sys_usw_l2_flush_fdb(uint8 lchip, ctc_l2_flush_fdb_t* pf)
{
    int32                      ret = CTC_E_NONE;
    ctc_learning_action_info_t learning_action = { 0 };
    sys_l2_flush_fdb_t   sys_flush;
    ctc_learning_action_t action = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(pf);
    CTC_ERROR_RETURN(_sys_usw_l2_query_flush_check(lchip, pf, 1));

    if (pl2_usw_master[lchip]->cfg_hw_learn) /* disable learn*/
    {
        sys_usw_learning_get_action(lchip, &learning_action);
        action = learning_action.action;
        if (CTC_LEARNING_ACTION_MAC_TABLE_FULL != action)
        {
            learning_action.action = CTC_LEARNING_ACTION_MAC_TABLE_FULL;
            learning_action.value  = 1;
            CTC_ERROR_RETURN(sys_usw_learning_set_action(lchip, &learning_action));
        }
    }

    if(CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN == pf->flush_type)
    {
        ctc_l2_addr_t   l2_addr;
        ctc_l2_fdb_query_t query;
        ctc_l2_fdb_query_rst_t result;

        sal_memset(&query, 0, sizeof(query));
        sal_memset(&result, 0, sizeof(result));
        query.fid = pf->fid;
        sal_memcpy(query.mac, pf->mac, sizeof(mac_addr_t));
        query.query_flag = pf->flush_flag;
        query.query_type = pf->flush_type;
        result.buffer = &l2_addr;
        result.buffer_len = sizeof(l2_addr);
        CTC_ERROR_GOTO(_sys_usw_l2_get_fdb_entry_detail_by_mac_vlan(lchip, &query, &result, NULL), ret, end);
        if(query.count)
        {
            sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));
            l2_addr.fid = pf->fid;
            sal_memcpy(l2_addr.mac, pf->mac, sizeof(mac_addr_t));
            ret =  sys_usw_l2_remove_fdb(lchip, &l2_addr);
        }
        goto end;
    }
    else if( CTC_L2_FDB_ENTRY_PENDING == pf->flush_flag)
    {
        ctc_l2_fdb_query_t query;
        ctc_l2_fdb_query_rst_t query_rst;
        uint32 loop = 0;
        uint16 fid = 0;
        uint8  is_logic = 0;
        mac_addr_t mac = {0};

        sal_memset(&query, 0, sizeof(query));
        sal_memset(&query_rst, 0, sizeof(query_rst));
        query.query_flag = CTC_L2_FDB_ENTRY_PENDING;
        query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;

        MALLOC_ZERO(MEM_FDB_MODULE, query_rst.buffer, DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t));
        if(NULL == query_rst.buffer)
        {
            ret =  CTC_E_NO_MEMORY;
            goto end;
        }

        query_rst.buffer_len = DUMP_MAX_ENTRY*sizeof(ctc_l2_addr_t);

        L2_LOCK;
        do
        {
            /* 1. dump all pending entry*/
            query.count = 0;
            query_rst.start_index = query_rst.next_query_index ;
            if((ret = _sys_usw_l2_get_entry_by_hw(lchip, &query, &query_rst, 0, 0, NULL)) < 0)
            {
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "flush pending fdb error\n");
                mem_free(query_rst.buffer);
                L2_UNLOCK;
                goto end;
            }

            /* 2. delete entry one by one*/
            for(loop=0; loop<query.count;loop++)
            {
                fid = (query_rst.buffer+loop)->fid;
                is_logic = SYS_L2_IS_LOGIC_PORT(fid);
                sal_memcpy(&mac, &((query_rst.buffer+loop)->mac), sizeof(mac_addr_t));

                sys_usw_l2_delete_hw_by_mac_fid(lchip, mac, fid, 1,  is_logic, 0);
            }
        }while(!query_rst.is_end);

        mem_free(query_rst.buffer);
        L2_UNLOCK;
        goto end;

    }

    sys_flush.fid = pf->fid;
    sys_flush.flush_flag = pf->flush_flag;
    sys_flush.flush_type = pf->flush_type;
    sys_flush.lchip = lchip;
    sal_memcpy(&sys_flush.mac, pf->mac, sizeof(mac_addr_t));
    sys_flush.gport = pf->gport;
    sys_flush.use_logic_port = pf->use_logic_port;
    L2_LOCK;
    ctc_hash_traverse_remove(pl2_usw_master[lchip]->fdb_hash, (hash_traversal_fn)_sys_usw_l2_flush_fdb_cb, &sys_flush);
    if(pf->flush_flag != CTC_L2_FDB_ENTRY_STATIC)
    {
        DsMac_m ds_mac;
        _sys_usw_l2_unbinding_logic_ecmp(lchip, pf, 1, &ds_mac);
        ret = _sys_usw_l2_flush_by_hw(lchip, pf, 1, pf->use_logic_port);
        _sys_usw_l2_unbinding_logic_ecmp(lchip, pf, 0, &ds_mac);
    }

    if(DRV_IS_TSINGMA(lchip) && pf->flush_type == CTC_L2_FDB_ENTRY_OP_ALL)
    {
        ctc_vector_traverse(pl2_usw_master[lchip]->fid_vec, _sys_usw_l2_flush_fdb_by_fid_cb, &sys_flush);
    }
    L2_UNLOCK;
end:
    if ( pl2_usw_master[lchip]->cfg_hw_learn) /* enable learn */
    {
        if (CTC_LEARNING_ACTION_MAC_TABLE_FULL != action)
        {
            learning_action.action = CTC_LEARNING_ACTION_MAC_TABLE_FULL;
            learning_action.value  = 0;
            CTC_ERROR_RETURN(sys_usw_learning_set_action(lchip, &learning_action));
        }
    }
    return ret;
}


#define _FDB_HW_
STATIC int32
_sys_usw_l2_build_tcam_key(uint8 lchip, mac_addr_t mac, uint32 ad_index, void* mackey)
{
    hw_mac_addr_t hw_mac;

    CTC_PTR_VALID_CHECK(mackey);
    sal_memset(mackey, 0, sizeof(DsFibMacBlackHoleHashKey_m));

    FDB_SET_HW_MAC(hw_mac, mac);
    SetDsFibMacBlackHoleHashKey(A, mappedMac_f, mackey, hw_mac);
    SetDsFibMacBlackHoleHashKey(V, valid_f, mackey, 1);
    SetDsFibMacBlackHoleHashKey(V, dsAdIndex_f, mackey, ad_index);

    return CTC_E_NONE;

}
STATIC int32
_sys_usw_l2_write_hw(uint8 lchip, sys_l2_info_t* psys)
{
    uint32   cmd  = 0;
    uint16   fid  = 0;
    DsMac_m ds_mac;
    mac_addr_t mac_addr = {0};
    uint32     flag     = 0;
    uint32     gport = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!psys->rsv_ad_index)
    {
        /* write Ad */
        CTC_ERROR_RETURN(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, psys));
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, psys->ad_index, cmd, &ds_mac));
    }

    switch(psys->addr_type)
    {
        case SYS_L2_ADDR_TYPE_UC:
            fid = psys->addr.l2_addr.fid;
            flag = psys->addr.l2_addr.flag;
            gport = psys->addr.l2_addr.gport;
            sal_memcpy(mac_addr, psys->addr.l2_addr.mac, sizeof(mac_addr));
            break;
        case SYS_L2_ADDR_TYPE_MC:
            fid = psys->addr.l2mc_addr.fid;
            flag = psys->addr.l2mc_addr.flag;
            sal_memcpy(mac_addr, psys->addr.l2mc_addr.mac, sizeof(mac_addr));
            break;
        default:
            break;
    }

    if (fid == DONTCARE_FID) /* tcam */
    {
        DsFibMacBlackHoleHashKey_m tcam_key;
        _sys_usw_l2_build_tcam_key(lchip, psys->addr.l2_addr.mac, psys->ad_index, &tcam_key);
        cmd = DRV_IOW(DsFibMacBlackHoleHashKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, psys->key_index, cmd, &tcam_key));
    }
    else if (!(psys->addr_type == SYS_L2_ADDR_TYPE_DEFAULT)) /* uc or mc */
    {
        drv_acc_in_t  in;
        drv_acc_out_t out;
        uint8 is_dynamic;
        DsFibHost0MacHashKey_m fdb_key;
        hw_mac_addr_t   hw_mac = {0};

        sal_memset(&in, 0, sizeof(in));
        sal_memset(&out, 0, sizeof(out));
        sal_memset(&fdb_key, 0, sizeof(DsFibHost0MacHashKey_m));
        /* write Key */
        FDB_SET_HW_MAC(hw_mac, mac_addr);
        SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, fid);
        SetDsFibHost0MacHashKey(A, mappedMac_f, &fdb_key, hw_mac);
        SetDsFibHost0MacHashKey(V, dsAdIndex_f, &fdb_key, psys->ad_index);
        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_KEY;
        in.tbl_id = DsFibHost0MacHashKey_t;
        in.data = &fdb_key;

        /*for white list entry do not aging */
        if (CTC_FLAG_ISSET(flag,CTC_L2_FLAG_WHITE_LIST_ENTRY)|| CTC_FLAG_ISSET(flag,CTC_L2_FLAG_AGING_DISABLE))
        {
            in.timer_index = 0;
        }
        else if (!CTC_FLAG_ISSET(flag,CTC_L2_FLAG_IS_STATIC)) /* pending entry already set aging status.*/
        {
            in.timer_index = SYS_AGING_TIMER_INDEX_MAC;
            CTC_BIT_SET(in.flag, DRV_ACC_AGING_EN);
        }

        is_dynamic = (!CTC_FLAG_ISSET(flag,CTC_L2_FLAG_IS_STATIC) &&  !(psys->ecmp_valid) && (psys->addr_type != SYS_L2_ADDR_TYPE_MC));

        if (is_dynamic || pl2_usw_master[lchip]->static_fdb_limit)
        {
            CTC_BIT_SET(in.flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT);

            if(is_dynamic)
            {
                CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
            }

            if (psys->is_logic_port)
            {
                CTC_BIT_SET(in.flag, DRV_ACC_USE_LOGIC_PORT);
            }

            in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);

            if (pl2_usw_master[lchip]->static_fdb_limit)
            {
                CTC_BIT_SET(in.flag, DRV_ACC_STATIC_MAC_LIMIT_CNT);
            }
        }

        CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
        if (out.is_conflict)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Hash Conflict\n");
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "MAC:%s, FID:%d\n", sys_output_mac(mac_addr), fid);
            return CTC_E_HASH_CONFLICT;
        }

        if (out.is_full)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [FDB] Security violation \n");
            return CTC_E_NO_RESOURCE;

        }
        psys->key_index = out.key_index;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2_dump_l2master(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_L2_INIT_CHECK();
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ad_index_drop              0x%x \n" \
                     "dft_entry_base             0x%x \n" \
                     "max_fid_value              0x%x \n" \
                     "hw_learn_en                0x%x \n" ,
                     pl2_usw_master[lchip]->ad_index_drop,       \
                     pl2_usw_master[lchip]->dft_entry_base,      \
                     pl2_usw_master[lchip]->cfg_max_fid,         \
                     pl2_usw_master[lchip]->cfg_hw_learn);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "hw_aging_en                0x%x \n", pl2_usw_master[lchip]->cfg_hw_learn);
    if (!pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "base_vport                 0x%x \n" , _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_VPORT));
    }
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
             "base_gport                 0x%x \n" \
             "base_trunk                 0x%x \n" \
             "logic_port_num             0x%x \n",
             _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT),   \
             _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK),   \
             pl2_usw_master[lchip]->cfg_vport_num);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l2_add_tcam(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 with_nh, uint32 nhid)
{
    int32          ret    = 0;
    uint32         key_index = 0;
    uint32         ad_index = 0;
    uint8         is_exist = 0;
    sys_l2_info_t  sys;
    uint32        cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DsFibMacBlackHoleHashKey_dsAdIndex_f);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys, 0, sizeof(sys));

    L2_LOCK;
    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_TCAM, l2_addr, &sys, with_nh, nhid), ret, error_0);

    /* 1.  lookup tcam_hash */
    is_exist = _sys_usw_l2_black_hole_lookup(lchip, l2_addr->mac, &key_index, NULL);
    if (!is_exist)
    {
        for(key_index=0; key_index<SYS_L2_MAX_BLACK_HOLE_ENTRY; key_index++)
        {
            if(!CTC_BMP_ISSET(pl2_usw_master[lchip]->mac_fdb_bmp, key_index))
            {
                CTC_BMP_SET(pl2_usw_master[lchip]->mac_fdb_bmp, key_index);
                break;
            }
        }

        if(key_index == SYS_L2_MAX_BLACK_HOLE_ENTRY)
        {
            ret =  CTC_E_NO_RESOURCE;
            goto error_0;
        }

        CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, 1, 1, &ad_index), ret, error_1);

        sys.key_index  = key_index;
        sys.ad_index   = ad_index;
        CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &sys), ret, error_2);

    }
    else
    {
        DsMac_m ds_mac;
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &sys.ad_index), ret, error_0);
        sys.key_index = key_index;
        CTC_ERROR_GOTO(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, &sys), ret, error_0);
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, sys.ad_index, cmd, &ds_mac), ret, error_0);

    }

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key index:%d, ad_index:%d\n",sys.key_index,sys.ad_index);

    L2_UNLOCK;
    return CTC_E_NONE;

error_2:
    sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, ad_index);
error_1:
    CTC_BMP_UNSET(pl2_usw_master[lchip]->mac_fdb_bmp, key_index);
error_0:
    L2_UNLOCK;
    return ret;
}

STATIC int32
_sys_usw_l2_remove_tcam(uint8 lchip, ctc_l2_addr_t* l2_addr)
{
    sys_l2_info_t  sys;
    uint32         cmd   = 0;
    DsFibMacBlackHoleHashKey_m empty;
    int32 ret = 0;
    uint8 is_exist = 0;
    uint32 ad_index = 0;

    L2_LOCK;

    sal_memcpy(&sys.addr.l2_addr, l2_addr, sizeof(ctc_l2_addr_t));

    is_exist = _sys_usw_l2_black_hole_lookup(lchip, l2_addr->mac, &sys.key_index, &ad_index);
    if (is_exist)
    {
         /*write key*/
        sal_memset(&empty, 0, sizeof(empty));
        cmd = DRV_IOW(DsFibMacBlackHoleHashKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, sys.key_index, cmd, &empty), ret, error);

         /*free ds mac*/
        sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, ad_index);

         /*free bitmap*/
        CTC_BMP_UNSET(pl2_usw_master[lchip]->mac_fdb_bmp, sys.key_index);
    }
    else
    {
        ret = CTC_E_NOT_EXIST;
    }

error:

    L2_UNLOCK;

    return ret;
}

STATIC int32
_sys_usw_l2_fdb_check(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 with_nh, uint32 nhid)
{
    uint32 temp_flag = 0;

    SYS_USW_CID_CHECK(lchip,l2_addr->cid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
     /* 1. check flag supported*/
    if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_WHITE_LIST_ENTRY) && ((!with_nh) || CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_IS_STATIC)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (!with_nh && CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_UCAST_DISCARD))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

     /* 2. check parameter*/
    if (!with_nh && !CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_DISCARD))
    {
        SYS_GLOBAL_PORT_CHECK(l2_addr->gport);
    }

    if (!CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_IS_STATIC))
    {
        temp_flag = l2_addr->flag;
        CTC_UNSET_FLAG(temp_flag, CTC_L2_FLAG_AGING_DISABLE);
        CTC_UNSET_FLAG(temp_flag, CTC_L2_FLAG_REMOTE_DYNAMIC);
        CTC_UNSET_FLAG(temp_flag, CTC_L2_FLAG_BIND_PORT);
        CTC_UNSET_FLAG(temp_flag, CTC_L2_FLAG_WHITE_LIST_ENTRY);
        CTC_UNSET_FLAG(temp_flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT);
        if (temp_flag) /* dynamic entry has other flag */
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    if (with_nh)
    {
        if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT))
        {
            SYS_GLOBAL_PORT_CHECK(l2_addr->gport);
        }
    }

    if(with_nh && (1 == nhid || 2 == nhid) && CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}
/**
   @brief add fdb entry
 */
int32
sys_usw_l2_add_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 with_nh, uint32 nhid)
{
    sys_l2_info_t       new_sys;
    int32               ret         = 0;
    sys_l2_fid_node_t   * fid    = NULL;
    uint32              fid_flag = 0;
    mac_lookup_result_t result = {0};
    sys_l2_info_t       old_sys;
    sys_l2_node_t*      p_l2_node_old;
    sys_l2_node_t*      p_l2_node  = NULL;
    uint8               is_overwrite = 0;
    sys_l2_ad_t         l2_ad;
    sys_l2_ad_t*        p_old_ad = NULL;
    uint8  bind_flag = 0;
    uint8  need_has_sw = 0;
    uint8   old_rsv_ad = 0;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);

    sal_memset(&new_sys, 0, sizeof(sys_l2_info_t));
    sal_memset(&old_sys, 0, sizeof(sys_l2_info_t));

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s ,fid :%d ,flag:0x%x, gport:0x%x\n",
                      sys_output_mac(l2_addr->mac), l2_addr->fid, l2_addr->flag, l2_addr->gport);

    if (l2_addr->fid ==  DONTCARE_FID) /* is_tcam is set when system mac. and system mac can update */
    {
        return _sys_usw_l2_add_tcam(lchip, l2_addr, with_nh, nhid);
    }

    CTC_ERROR_RETURN(_sys_usw_l2_fdb_check(lchip, l2_addr,with_nh, nhid));
    L2_LOCK;
     /* 3. check fid created*/
    fid      = _sys_usw_l2_fid_node_lkup(lchip, l2_addr->fid);
    fid_flag = (fid) ? fid->flag : 0;

    if (!with_nh && (CTC_FLAG_ISSET(fid_flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT)))
    {
        ret = CTC_E_INVALID_PARAM;
        goto error_0;
    }

    if (SYS_L2_IS_LOGIC_PORT(l2_addr->fid))
    {
        SYS_LOGIC_PORT_CHECK_WITH_UNLOCK(l2_addr->gport, pl2_usw_master[lchip]->l2_mutex);
    }

     /* 4. map ctc to sys  _sys_usw_l2_build_sys_info*/

    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_UC, l2_addr, &new_sys, with_nh, nhid), ret, error_0);

     /* 5. lookup hw, check result*/
    CTC_ERROR_GOTO(_sys_usw_l2_acc_lookup_mac_fid
                    (lchip, l2_addr->mac, l2_addr->fid, &result), ret, error_0);
    new_sys.key_index = result.key_index;

    if (result.conflict)
    {
        ret = CTC_E_HASH_CONFLICT;
        goto error_0;
    }

     /* 6. if hit means entry already exist, pending entry has clear hit flag in driver*/
    is_overwrite = result.hit;

    if (result.pending)
    {
        new_sys.pending = 1;
    }

    if (is_overwrite)
    {
        CTC_ERROR_GOTO(_sys_usw_l2_build_sys_ad_info_by_index(lchip, result.ad_index, &old_sys), ret, error_0);
        /* l2 mc shall not ovewrite */
        if (old_sys.addr_type == SYS_L2_ADDR_TYPE_MC)
        {
            ret = CTC_E_EXIST;
            goto error_0;
        }

        /* dynamic cannot rewrite static */
        if (!CTC_FLAG_ISSET(new_sys.addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC) &&  CTC_FLAG_ISSET(old_sys.addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC))
        {
            ret = CTC_E_EXIST;
            goto error_0;
        }
    }

    p_l2_node_old = _sys_usw_l2_fdb_hash_lkup(lchip, &l2_addr->mac, l2_addr->fid);

    if(p_l2_node_old)
    {
        p_old_ad = p_l2_node_old->adptr;
        old_rsv_ad = p_l2_node_old->rsv_ad;
    }
    /*build dsmac index*/
    CTC_ERROR_GOTO(_sys_usw_l2_get_reserved_index(lchip, &new_sys), ret, error_0);

    need_has_sw = !new_sys.rsv_ad_index || new_sys.ecmp_valid || (new_sys.use_destmap_profile && DRV_IS_DUET2(lchip));

    if (need_has_sw)
    {
        sys_l2_ad_t*         p_ad = NULL;
        sal_memset(&l2_ad, 0, sizeof(l2_ad));

        if(p_l2_node_old)
        {
            p_l2_node_old->flag = l2_addr->flag;
            p_l2_node_old->ecmp_valid = new_sys.ecmp_valid;
            p_l2_node_old->use_destmap_profile = new_sys.use_destmap_profile;
            p_l2_node_old->aps_valid = new_sys.aps_valid;
            p_l2_node_old->rsv_ad    = new_sys.rsv_ad_index;
            p_l2_node_old->key_index = new_sys.key_index;
            p_l2_node = p_l2_node_old;

            /*update to unbind nexthop*/
            if (p_l2_node->bind_nh)
            {
                _sys_usw_l2_bind_nexthop(lchip, p_l2_node_old, 0, 0);
            }

        }
        else
        {
            MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node, sizeof(sys_l2_node_t));
            if(NULL == p_l2_node)
            {
                ret = CTC_E_NO_MEMORY;
                goto error_0;
            }

            sal_memcpy(&p_l2_node->mac, &l2_addr->mac, sizeof(mac_addr_t));
            p_l2_node->fid  = l2_addr->fid;
            p_l2_node->flag = l2_addr->flag;
            p_l2_node->ecmp_valid = new_sys.ecmp_valid;
            p_l2_node->use_destmap_profile = new_sys.use_destmap_profile;
            p_l2_node->key_index = new_sys.key_index;
            p_l2_node->aps_valid = new_sys.aps_valid;
            p_l2_node->rsv_ad    = new_sys.rsv_ad_index;
        }

        p_l2_node->nh_id = new_sys.nhid;


        if (new_sys.bind_nh)
        {
            /*do not using dsfwd, do nexthop and l2 bind*/
            ret = _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 1, 0);
            if (CTC_E_IN_USE == ret)
            {
                /*nh have already bind, do unbind, using dsfwd*/
                sys_usw_nh_get_dsfwd_offset(lchip, nhid, &new_sys.fwd_ptr, 0);
                new_sys.merge_dsfwd = 0;
                new_sys.bind_nh = 0;
            }
            else if (ret < 0)
            {
                /*Bind fail, using dsfwd*/
                sys_usw_nh_get_dsfwd_offset(lchip, nhid, &new_sys.fwd_ptr, 0);
                new_sys.merge_dsfwd = 0;
                new_sys.bind_nh = 0;
            }
            else
            {
                /*indicate bind success*/
                bind_flag = 1;
            }
        }

        CTC_ERROR_GOTO(_sys_usw_l2_encode_dsmac(lchip, &l2_ad.ds_mac, &new_sys), ret, error_0);
        if (p_l2_node->rsv_ad)
        {
            if (!p_l2_node->adptr )
            {
                MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node->adptr, sizeof(sys_l2_ad_t));
                if(NULL == p_l2_node->adptr)
                {
                    ret = CTC_E_NO_MEMORY;
                    goto error_0;
                }
            }
            sal_memcpy(&p_l2_node->adptr->ds_mac, &l2_ad.ds_mac, sizeof(DsMac_m));
            p_l2_node->adptr->index = new_sys.ad_index;
        }
        else
        {
            if (old_rsv_ad && p_l2_node->adptr)
            {
                mem_free(p_old_ad);
            }
            CTC_ERROR_GOTO(ctc_spool_add(pl2_usw_master[lchip]->ad_spool, &l2_ad, NULL, &p_ad), ret, error_0);
            p_l2_node->adptr = p_ad;
            new_sys.ad_index = p_ad->index;
        }

        if (!p_l2_node_old)
        {
            CTC_ERROR_GOTO(_sys_usw_l2_fdb_hash_add(lchip, p_l2_node), ret, error_1);
        }

    }
    else if(p_l2_node_old)
    {
         /*CTC_ERROR_GOTO(ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, p_l2_node_old->adptr, NULL), ret, error_0);*/
        CTC_ERROR_GOTO(_sys_usw_l2_fdb_hash_remove(lchip, p_l2_node_old), ret, error_0);
        if (p_l2_node_old->rsv_ad)
        {
            mem_free(p_old_ad);
        }
        mem_free(p_l2_node_old);
    }

    /* write hw */
    CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &new_sys), ret, error_2);

    SYS_L2_DUMP_SYS_INFO(&new_sys);

    /*Note: MUST free old resource after write hardware*/
    if(p_old_ad && !old_rsv_ad)
    {
        CTC_ERROR_GOTO(ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, p_old_ad, NULL), ret, error_2);
    }
    L2_UNLOCK;
    return ret;

error_2:
    if(p_l2_node)
    {
        _sys_usw_l2_fdb_hash_remove(lchip, p_l2_node);
    }
error_1:
    if(need_has_sw)
    {
        if (p_l2_node->rsv_ad)
        {
            mem_free(p_l2_node->adptr);
        }
        else
        {
            ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, &l2_ad, NULL);
        }
    }

error_0:
    if (bind_flag)
    {
        _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 0, 0);
    }

    if (p_l2_node)
    {
        mem_free(p_l2_node);
    }
    L2_UNLOCK;
    return ret;
}

int32
_sys_usw_l2_replace_match_fdb(uint8 lchip, ctc_l2_replace_t* p_replace)
{
    int32               ret = 0;
    uint32 cmd = 0;
    sys_l2_ad_t         l2_ad;
    uint8               bind_flag = 0;
    mac_lookup_result_t match_result = {0};
    ctc_l2_addr_t*     l2_addr = &p_replace->l2_addr;
    ctc_l2_addr_t*     match_addr = &p_replace->match_addr;
    uint8              with_nh = (p_replace->nh_id ? 1 : 0);
    uint32             nh_id   = p_replace->nh_id;
    sys_l2_info_t       new_sys;
    sys_l2_info_t       old_sys;
    sys_l2_fid_node_t   * fid    = NULL;
    uint32              fid_flag = 0;
    uint32             match_key_index = 0;
    uint8              match_hit = 0;
    DsMac_m            old_dsmac;
    sys_l2_node_t       * p_l2_node = NULL;
    sys_l2_node_t       * p_match_node = NULL;
    uint8 old_is_logic = 0;
    uint8 old_is_dynamic = 0;
    uint8 mac_limit_en = 0;
    uint8 need_has_sw = 0;

    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(match_addr->fid);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&new_sys, 0, sizeof(sys_l2_info_t));
    sal_memset(&old_sys, 0, sizeof(sys_l2_info_t));

    CTC_ERROR_RETURN(_sys_usw_l2_fdb_check(lchip, l2_addr,with_nh, nh_id));
    if ((l2_addr->fid ==  DONTCARE_FID) || (match_addr->fid ==  DONTCARE_FID))
    {
        return CTC_E_NOT_SUPPORT;
    }
    L2_LOCK;

     /*if hit or pending, remove hw first*/
    CTC_ERROR_GOTO(_sys_usw_l2_acc_lookup_mac_fid
        (lchip, match_addr->mac, match_addr->fid, &match_result), ret, error_0);
    if (match_result.hit || match_result.pending)
    {
         /*decode match key info*/
        uint8 index = 0;
        uint32 key_index = 0;
        uint8 cf_max = 0;
        sys_l2_calc_index_t calc_index;
        sal_memset(&old_dsmac, 0, sizeof(DsMac_m));
        sal_memset(&calc_index, 0, sizeof(sys_l2_calc_index_t));

        old_sys.key_index = match_result.key_index;
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, match_result.ad_index, cmd, &old_dsmac), ret, error_0);
        CTC_ERROR_GOTO(_sys_usw_l2_build_sys_ad_info_by_index(lchip, match_result.ad_index, &old_sys), ret, error_0);
        match_key_index = match_result.key_index;
         /*check the hash equal*/
        CTC_ERROR_GOTO(_sys_usw_l2_fdb_calc_index(lchip, l2_addr->mac, l2_addr->fid, &calc_index), ret, error_0);
        cf_max = calc_index.index_cnt + MCHIP_CAP(SYS_CAP_L2_FDB_CAM_NUM);
        for (index = 0; index < cf_max; index++)
        {
            key_index = (index >= calc_index.index_cnt) ? (index - calc_index.index_cnt) : calc_index.key_index[index];
            if (match_key_index == key_index)
            {
                match_hit = 1;
                break;
            }
        }
        if (!match_hit)
        {
            ret = CTC_E_INVALID_CONFIG;
            goto  error_0;
        }

        p_match_node = _sys_usw_l2_fdb_hash_lkup(lchip, &match_addr->mac, match_addr->fid);

        old_is_logic = old_sys.is_logic_port;

        old_is_dynamic = (!CTC_FLAG_ISSET(old_sys.addr.l2_addr.flag,CTC_L2_FLAG_IS_STATIC) &&  !(old_sys.ecmp_valid) && old_sys.addr_type != SYS_L2_ADDR_TYPE_MC);

        mac_limit_en = (old_is_dynamic || pl2_usw_master[lchip]->static_fdb_limit)?1:0;

    }

     /* check fid created*/
    fid      = _sys_usw_l2_fid_node_lkup(lchip, l2_addr->fid);
    fid_flag = (fid) ? fid->flag : 0;

    if (!with_nh && (CTC_FLAG_ISSET(fid_flag, CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT)))
    {
        ret = CTC_E_INVALID_PARAM;
        goto error_0;
    }

    if (SYS_L2_IS_LOGIC_PORT(l2_addr->fid))
    {
        SYS_LOGIC_PORT_CHECK_WITH_UNLOCK(l2_addr->gport, pl2_usw_master[lchip]->l2_mutex);
    }

     /*map ctc to sys*/
    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_UC, l2_addr, &new_sys, with_nh, nh_id), ret, error_0);

    /*build dsmac index*/
    CTC_ERROR_GOTO(_sys_usw_l2_get_reserved_index(lchip, &new_sys), ret, error_0);

    need_has_sw = !new_sys.rsv_ad_index || new_sys.ecmp_valid;
    if(need_has_sw)
    {
        sys_l2_ad_t*         p_ad = NULL;
        sal_memset(&l2_ad, 0, sizeof(l2_ad));
        MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node, sizeof(sys_l2_node_t));
        if(NULL == p_l2_node)
        {
            ret = CTC_E_NO_MEMORY;
            goto error_0;
        }

        sal_memcpy(&p_l2_node->mac, &l2_addr->mac, sizeof(mac_addr_t));
        p_l2_node->fid  = l2_addr->fid;
        p_l2_node->flag = l2_addr->flag;
        p_l2_node->ecmp_valid = new_sys.ecmp_valid;
        p_l2_node->aps_valid = new_sys.aps_valid;
        p_l2_node->nh_id = new_sys.nhid;
        p_l2_node->rsv_ad = new_sys.rsv_ad_index;

        if (new_sys.bind_nh)
        {
            /*do not using dsfwd, do nexthop and l2 bind*/
            ret = _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 1, 0);
            if (CTC_E_IN_USE == ret)
            {
                /*nh have already bind, do unbind, using dsfwd*/
                sys_usw_nh_get_dsfwd_offset(lchip, nh_id, &new_sys.fwd_ptr, 0);
                new_sys.merge_dsfwd = 0;
                new_sys.bind_nh = 0;
            }
            else if (ret < 0)
            {
                /*Bind fail, using dsfwd*/
                sys_usw_nh_get_dsfwd_offset(lchip, nh_id, &new_sys.fwd_ptr, 0);
                new_sys.merge_dsfwd = 0;
                new_sys.bind_nh = 0;
            }
            else
            {
                /*indicate bind success*/
                bind_flag = 1;
            }
        }

        CTC_ERROR_GOTO(_sys_usw_l2_encode_dsmac(lchip, &l2_ad.ds_mac, &new_sys), ret, error_0);
        if (p_l2_node->rsv_ad)
        {
            MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node->adptr, sizeof(sys_l2_ad_t));
            if(NULL == p_l2_node->adptr)
            {
                ret = CTC_E_NO_MEMORY;
                goto error_0;
            }
            sal_memcpy(&p_l2_node->adptr->ds_mac, &l2_ad.ds_mac, sizeof(DsMac_m));
            p_l2_node->adptr->index = new_sys.ad_index;
        }
        else
        {
            CTC_ERROR_GOTO(ctc_spool_add(pl2_usw_master[lchip]->ad_spool, &l2_ad, NULL, &p_ad), ret, error_0);
            p_l2_node->adptr = p_ad;
            new_sys.ad_index = p_ad->index;
        }

        CTC_ERROR_GOTO(_sys_usw_l2_fdb_hash_add(lchip, p_l2_node), ret, error_1);
    }

    /* remove old hw entry */
    if (match_hit)
    {
        CTC_ERROR_GOTO(sys_usw_l2_delete_hw_by_mac_fid
                        (lchip, match_addr->mac, match_addr->fid, mac_limit_en, old_is_logic, !old_is_dynamic), ret, error_2);
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Remove ucast fdb: key_index:0x%x, ad_index:0x%x\n",
                     match_result.key_index, match_result.ad_index);
    }

    /* write new hw */
    CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &new_sys), ret, error_3);

    SYS_L2_DUMP_SYS_INFO(&new_sys);

    /*remove old sw*/
    if(p_match_node)
    {
        if (p_match_node->bind_nh)
        {
            _sys_usw_l2_bind_nexthop(lchip, p_match_node, 0, 0);
        }
        _sys_usw_l2_fdb_hash_remove(lchip, p_match_node);
        if(p_match_node->adptr)
        {
            if (p_match_node->rsv_ad)
            {
                mem_free(p_match_node->adptr);
            }
            else
            {
                ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, p_match_node->adptr, NULL);
            }
        }
        mem_free(p_match_node);
    }
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "replace match hit:%d\n", match_hit);
    L2_UNLOCK;
    return CTC_E_NONE;

 error_3:
    if (match_hit)
    {
        FDB_SET_MAC(old_sys.addr.l2_addr.mac, match_addr->mac);
        old_sys.addr.l2_addr.fid = match_addr->fid;
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, old_sys.ad_index, cmd, &old_dsmac);
        old_sys.rsv_ad_index = 1;
        _sys_usw_l2_write_hw(lchip, &old_sys);
    }
error_2:
    if(p_l2_node)
    {
        _sys_usw_l2_fdb_hash_remove(lchip, p_l2_node);
    }
error_1:
    if(need_has_sw)
    {
        if (p_l2_node->rsv_ad)
        {
            mem_free(p_l2_node->adptr);
        }
        else
        {
            ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, &l2_ad, NULL);
        }
    }

error_0:
    if (bind_flag)
    {
        _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 0, 0);
    }

    if (p_l2_node)
    {
        mem_free(p_l2_node);
    }
    L2_UNLOCK;
    return ret;
}

int32
sys_usw_l2_replace_fdb(uint8 lchip, ctc_l2_replace_t* p_replace)
{
    mac_lookup_result_t result = {0};

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_replace);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s, fid:%d, flag:0x%x, gport:0x%x\n",
                      sys_output_mac(p_replace->l2_addr.mac), p_replace->l2_addr.fid,
                      p_replace->l2_addr.flag, p_replace->l2_addr.gport);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "match-mac:%s, match-fid:%d\n", sys_output_mac(p_replace->match_addr.mac), p_replace->match_addr.fid);

    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid
        (lchip, p_replace->l2_addr.mac, p_replace->l2_addr.fid, &result));
    if (result.hit || result.pending)
    {
        CTC_ERROR_RETURN(sys_usw_l2_add_fdb(lchip, &p_replace->l2_addr, p_replace->nh_id ? 1 : 0, p_replace->nh_id));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_l2_replace_match_fdb(lchip, p_replace));
    }

    return CTC_E_NONE;
}


int32
sys_usw_l2_remove_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr)
{
    sys_l2_info_t       sys;
    mac_lookup_result_t rslt;
    sys_l2_node_t*      p_l2_node  = NULL;
    uint8               mac_limit_en = 0;
    int32               ret = CTC_E_NONE;
    uint8               is_logic = 0;
    uint8               is_dynamic = 0;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac: %s,fid: %d  \n",
                      sys_output_mac(l2_addr->mac),
                      l2_addr->fid);

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    sal_memcpy(&sys.addr.l2_addr, l2_addr, sizeof(ctc_l2_addr_t));

    if (l2_addr->fid ==  DONTCARE_FID)
    {
        return _sys_usw_l2_remove_tcam(lchip, l2_addr);
    }

    L2_LOCK;
    CTC_ERROR_GOTO(_sys_usw_l2_acc_lookup_mac_fid
                    (lchip, l2_addr->mac, l2_addr->fid, &rslt), ret, error_0);

    if (rslt.hit)
    {
        sys.key_index = rslt.key_index;
        CTC_ERROR_GOTO(_sys_usw_l2_build_sys_ad_info_by_index(lchip, rslt.ad_index, &sys), ret, error_0);
        if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_REMOVE_DYNAMIC) && CTC_FLAG_ISSET(sys.addr.l2_addr.flag, CTC_L2_FLAG_IS_STATIC))
        {
            ret = CTC_E_NONE;
            goto error_0;
        }
    }
    else
    {
         /*SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n", sys_output_mac(l2_addr->mac), l2_addr->fid);*/
        ret =  CTC_E_NOT_EXIST;
        goto error_0;
    }

    is_logic = sys.is_logic_port;

    is_dynamic = (!CTC_FLAG_ISSET(sys.addr.l2_addr.flag,CTC_L2_FLAG_IS_STATIC) &&  !(sys.ecmp_valid) && sys.addr_type != SYS_L2_ADDR_TYPE_MC);

    mac_limit_en = (is_dynamic || pl2_usw_master[lchip]->static_fdb_limit)?1:0;

    /* remove hw entry */
    CTC_ERROR_GOTO(sys_usw_l2_delete_hw_by_mac_fid
                    (lchip, l2_addr->mac, l2_addr->fid, mac_limit_en, is_logic, !is_dynamic), ret, error_0);


    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove ucast fdb: key_index:0x%x, ad_index:0x%x\n",
                     rslt.key_index, rslt.ad_index);

    /* delete soft table*/
    p_l2_node = _sys_usw_l2_fdb_hash_lkup(lchip, &l2_addr->mac, l2_addr->fid);
    if(p_l2_node)
    {
        if (p_l2_node->bind_nh)
        {
            _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 0, 0);
        }

        CTC_ERROR_GOTO(_sys_usw_l2_fdb_hash_remove(lchip, p_l2_node), ret, error_0);
        if(p_l2_node->adptr)
        {
            if (p_l2_node->rsv_ad)
            {
                mem_free(p_l2_node->adptr);
            }
            else
            {
                CTC_ERROR_GOTO(ctc_spool_remove(pl2_usw_master[lchip]->ad_spool, p_l2_node->adptr, NULL), ret, error_0);
            }
        }
        mem_free(p_l2_node);
    }

error_0:

    L2_UNLOCK;
    return ret;
}

/*only remove pending fdb */
int32
sys_usw_l2_remove_pending_fdb(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 ad_idx)
{
    mac_lookup_result_t rslt;
    int32               ret = 0;

    SYS_L2_INIT_CHECK();

    sal_memset(&rslt, 0, sizeof(mac_lookup_result_t));

    L2_LOCK;

    CTC_ERROR_GOTO(_sys_usw_l2_acc_lookup_mac_fid
                    (lchip, l2_addr->mac, l2_addr->fid, &rslt), ret, error_0);

    if (!rslt.pending)
    {
        L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Not Pending,mac: %s,fid: %d  \n", sys_output_mac(l2_addr->mac), l2_addr->fid);
        return CTC_E_NONE;
    }

    if (rslt.ad_index != ad_idx)
    {
         L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Pending Aging But ad not match, should not remove mac: %s,fid: %d  ad_old:0x%x, ad_new:0x%x\n",
            sys_output_mac(l2_addr->mac), l2_addr->fid, ad_idx, rslt.ad_index);
        return CTC_E_NONE;
    }

    ret = sys_usw_l2_delete_hw_by_mac_fid(lchip, l2_addr->mac, l2_addr->fid, 1, SYS_L2_IS_LOGIC_PORT(l2_addr->fid), 0);

error_0:

    L2_UNLOCK;
    return ret;
}

int32
sys_usw_l2_set_station_move(uint8 lchip, uint32 gport, uint8 type, uint32 value)
{
    uint32 cmd = 0;
    uint32 ad_index = 0;
    uint32 field_val = 0;
    uint16 lport = 0;
    uint8  gchip = 0;
    uint8  is_local_chip = 0;

    SYS_L2_INIT_CHECK();

    if (MAX_SYS_L2_STATION_MOVE_OP <= type)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_GLOBAL_PORT_CHECK(gport);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        field_val = gport;
    }
    else
    {
        gchip = CTC_MAP_GPORT_TO_GCHIP(gport);
        SYS_GLOBAL_CHIPID_CHECK(gchip);
        lport = CTC_MAP_GPORT_TO_LPORT(gport);
        is_local_chip = sys_usw_chip_is_local(lchip, gchip);
        if (is_local_chip)
        {
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_globalSrcPort_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
        }
    }

    if (SYS_DRV_IS_LINKAGG_PORT(field_val) || CTC_IS_LINKAGG_PORT(gport))
    {

        ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK) + (field_val & CTC_LINKAGGID_MASK); /* linkagg */
    }
    else if (is_local_chip)
    {
        ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT) + lport; /* gport */
    }
    else if (pl2_usw_master[lchip]->rchip_ad_rsv[gchip])
    {
        uint32 base = 0;
        uint32 cmd = 0;
        uint16 lport = CTC_MAP_GPORT_TO_LPORT(gport);
        if (lport >= pl2_usw_master[lchip]->rchip_ad_rsv[gchip])
        {
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOR(FibAccelerationRemoteChipAdBase_t, FibAccelerationRemoteChipAdBase_array_0_remoteChipAdBase_f + gchip);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &base));
        ad_index = lport + (base << 6);
    }
    else
    {
        return CTC_E_NONE;
    }

    /* update ad */
    if (SYS_L2_STATION_MOVE_OP_PRIORITY == type)
    {
        field_val = (value == 0) ? 1 : 0;
        cmd = DRV_IOW(DsMac_t, DsMac_srcMismatchLearnEn_f);
    }
    else if (SYS_L2_STATION_MOVE_OP_ACTION == type)
    {
       field_val = (value != CTC_PORT_STATION_MOVE_ACTION_TYPE_FWD) ? 1 : 0;
       cmd = DRV_IOW(DsMac_t, DsMac_srcMismatchDiscard_f);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &field_val));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gchip:%d ad_index: 0x%x\n",gchip, ad_index);
    return CTC_E_NONE;
}

int32
sys_usw_l2_get_station_move(uint8 lchip, uint32 gport, uint8 type, uint32* value)
{
    uint32 cmd = 0;
    uint32 ad_index = 0;
    uint32 field_val = 0;
    uint16 lport = 0;

    SYS_L2_INIT_CHECK();

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_globalSrcPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    if (SYS_DRV_IS_LINKAGG_PORT(field_val))
    {
        ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK) + (field_val & SYS_DRV_LOCAL_PORT_MASK); /* linkagg */
    }
    else
    {
        ad_index = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT) + lport; /* gport */
    }

    /* update ad */
    if (SYS_L2_STATION_MOVE_OP_PRIORITY == type)
    {
        cmd = DRV_IOR(DsMac_t, DsMac_srcMismatchLearnEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &field_val));
        *value = field_val ? 0 : 1;
    }
    else if (SYS_L2_STATION_MOVE_OP_ACTION == type)
    {
        cmd = DRV_IOR(DsMac_t, DsMac_srcMismatchDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &field_val));
        *value = field_val ? CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD : CTC_PORT_STATION_MOVE_ACTION_TYPE_FWD;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_add_indrect(uint8 lchip, ctc_l2_addr_t* l2_addr, uint32 nhid, uint32 index)
{
    drv_acc_in_t  in;
    drv_acc_out_t out;
    sys_l2_info_t sys;
    DsMac_m ds_mac;
    uint32 cmd = 0;
    uint32 ad_index =0;
    DsFibHost0MacHashKey_m fdb_key;
    hw_mac_addr_t   hw_mac = {0};
    sys_nh_info_dsnh_t nhinfo;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s ,fid :%d ,flag:0x%x, gport:0x%x\n",
                      sys_output_mac(l2_addr->mac), l2_addr->fid, l2_addr->flag, l2_addr->gport);

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    sal_memset(&ds_mac, 0, sizeof(DsMac_m));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sys.addr_type = SYS_L2_ADDR_TYPE_UC;
    sys.is_logic_port = 1;
    sys.addr.l2_addr.gport = l2_addr->gport;
    sys.addr.l2_addr.assign_port = l2_addr->assign_port;   /*policer id*/
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nhinfo, 0));
    sys.nh_ext = nhinfo.nexthop_ext;
    sys.dsnh_offset = nhinfo.dsnh_offset;
    sys.destmap = nhinfo.dest_map;
    sys.merge_dsfwd = 1;
    if (l2_addr->mask_valid)
    {
        sys.mc_nh = 1;
    }
    if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_SRC_DISCARD))
    {
        CTC_SET_FLAG(sys.addr.l2_addr.flag ,CTC_L2_FLAG_BIND_PORT);
    }
    else
    {
        CTC_UNSET_FLAG(sys.addr.l2_addr.flag ,CTC_L2_FLAG_BIND_PORT);
    }

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "logic_port:0x%x, nhid:0x%x\n", l2_addr->gport, nhid);

    cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, &sys));
    ad_index = index;

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &ds_mac));

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    sal_memset(&fdb_key, 0, sizeof(DsFibHost0MacHashKey_m));

    FDB_SET_HW_MAC(hw_mac, l2_addr->mac);
    SetDsFibHost0MacHashKey(V, vsiId_f, &fdb_key, l2_addr->fid);
    SetDsFibHost0MacHashKey(A, mappedMac_f, &fdb_key, hw_mac);
    SetDsFibHost0MacHashKey(V, dsAdIndex_f, &fdb_key, ad_index);
    in.type = DRV_ACC_TYPE_ADD;
    in.tbl_id = DsFibHost0MacHashKey_t;
    in.data = &fdb_key;
    in.index = ad_index;
    CTC_BIT_SET(in.flag, DRV_ACC_ENTRY_DYNAMIC);
    CTC_BIT_SET(in.flag, DRV_ACC_UPDATE_MAC_LIMIT_CNT);
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_KEY;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    if (out.is_conflict)
    {
        return CTC_E_FDB_HASH_CONFLICT;
    }

    if (CTC_FLAG_ISSET(l2_addr->flag, CTC_L2_FLAG_AGING_DISABLE))
    {
        CTC_ERROR_RETURN(sys_usw_aging_set_aging_status(lchip, 0, out.key_index, 0, 0));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_aging_set_aging_status(lchip, 0, out.key_index, 1, 1));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_remove_indrect(uint8 lchip, ctc_l2_addr_t* l2_addr)
{
    mac_lookup_result_t rslt;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac: %s,fid: %d  \n",
                      sys_output_mac(l2_addr->mac),
                      l2_addr->fid);

    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2_addr->mac, l2_addr->fid, &rslt));

    if (!rslt.hit && !rslt.pending)
    {
       return CTC_E_NOT_EXIST;
    }

    CTC_ERROR_RETURN(sys_usw_l2_delete_hw_by_mac_fid(lchip, l2_addr->mac, l2_addr->fid, 1, l2_addr->is_logic_port, 0));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "sync  ucast fdb: key_index:0x%x, ad_index:0x%x\n", rslt.key_index, rslt.ad_index);

    return CTC_E_NONE;
}

int32
sys_usw_l2_update_station_move_action(uint8 lchip, uint32 ad_index, uint32 type)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    field_val = (type == CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD)?1:0;
    cmd = DRV_IOW(DsMac_t, DsMac_srcMismatchDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &field_val));

    field_val = (type == CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD)?0:1;
    cmd = DRV_IOW(DsMac_t, DsMac_srcMismatchLearnEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &field_val));

    return CTC_E_NONE;
}

#define _REMOVE_GET_BY_INDEX_
/* index is key_index, not aging_ptr*/
int32
sys_usw_l2_remove_fdb_by_index(uint8 lchip, uint32 index)
{
    ctc_l2_addr_t l2_addr;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    SYS_L2_INDEX_CHECK(index);

    CTC_ERROR_RETURN(sys_usw_l2_get_fdb_by_index(lchip, index, &l2_addr, NULL));
    CTC_ERROR_RETURN(sys_usw_l2_remove_fdb(lchip, &l2_addr));

    return CTC_E_NONE;
}


int32
sys_usw_l2_get_fdb_by_index(uint8 lchip, uint32 key_index, ctc_l2_addr_t* l2_addr, sys_l2_detail_t* pi)
{
    sys_l2_info_t     sys;
    drv_acc_in_t  in;
    drv_acc_out_t out;
    uint8         timer = 0;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2_INDEX_CHECK(key_index);

    in.type = DRV_ACC_TYPE_LOOKUP;
    in.tbl_id    = DsFibHost0MacHashKey_t;
    in.index = key_index ;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    sys.key_index = key_index;
    _sys_usw_l2_decode_dskey(lchip, &sys, &out.data);

    if (!sys.key_valid)  /*not valid*/
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }

    /* decode and get gport, flag */
    _sys_usw_l2_build_sys_ad_info_by_index(lchip, sys.ad_index, &sys);
    CTC_ERROR_RETURN(sys_usw_aging_get_aging_timer(lchip, 1, key_index, &timer));
    if(timer == 0)
    {
        CTC_SET_FLAG(sys.addr.l2_addr.flag, CTC_L2_FLAG_AGING_DISABLE);
    }
     /*_sys_usw_l2_unmap_flag(lchip, &l2_addr->flag, &sys.flag);*/

    sal_memcpy(l2_addr, &sys.addr.l2_addr, sizeof(ctc_l2_addr_t));

    if (sys.pending)
    {
        CTC_SET_FLAG(l2_addr->flag, CTC_L2_FLAG_PENDING_ENTRY);
    }
    if (pi)
    {
        pi->key_index   = key_index;
        pi->ad_index    = sys.ad_index;
        pi->flag        = sys.addr.l2_addr.flag;
        pi->pending     = sys.pending;
    }

    return CTC_E_NONE;
}

#define _HW_SYNC_UP_
    #if 0
/**
   @brief update soft tables after hardware learn aging
 */
int32
sys_usw_l2_sync_hw_info(uint8 lchip, void* pf)
{

    uint8                    index       = 0;
    sys_l2_info_t            sys;
    sys_learning_aging_data_t data;
    sys_learning_aging_info_t* p_info = NULL;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(pf);
    p_info = (sys_learning_aging_info_t*) pf;
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Hw total_num  %d\n", p_info->entry_num);

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    for (index = 0; index < p_info->entry_num; index++)
    {
        sal_memcpy(&data, &(p_info->data[index]), sizeof(data));
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "   index  %d\n"\
                         "   mac [%s] \n"\
                         "   fid [%d] \n"\
                         "   is_aging  [%d]\n",
                         index,
                         sys_output_mac(data.mac),
                         data.fid,
                         data.is_aging);

        if (!data.is_hw)
        {
            continue;
        }

        if (data.is_aging)
        {
            _sys_usw_l2_build_sys_ad_info_by_index(lchip, data.ad_index, &sys);
            L2_LOCK;
#if 0
            if (pl2_usw_master[lchip]->cfg_has_sw)
            {
                p_l2_node = _sys_usw_l2_fdb_hash_lkup(lchip, data.mac, data.fid);
                if (!p_l2_node)
                {
                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " lookup sw failed!!\n");
                    L2_UNLOCK;
                    continue;
                }

                FDB_ERR_RET_UL(_sys_usw_l2uc_remove_sw(lchip, p_l2_node));
                FDB_ERR_RET_UL(_sys_usw_l2_free_dsmac_index(lchip, &sys, NULL, &freed));

                if(freed)
                {
                    mem_free(p_l2_node->adptr);
                }
                mem_free(p_l2_node);
            }
#endif

            _sys_usw_l2_update_count(lchip, &sys.flag, -1, L2_COUNT_GLOBAL, &data.key_index);
            L2_UNLOCK;
        }
        else  /* learning */
        {
            if ((data.fid != 0) && (data.fid != 0xFFFF)
                && (!pl2_usw_master[lchip] || (data.fid >= pl2_usw_master[lchip]->cfg_max_fid)))
            {
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "learning fid check failed ! ---continue---next one!\n");
                continue;
            }

            L2_LOCK;
#if 0
            if (pl2_usw_master[lchip]->cfg_has_sw)
            {
                p_l2_node = _sys_usw_l2_fdb_hash_lkup(lchip, data.mac, data.fid);
                if (p_l2_node) /* update  */
                {
                    /* update learning port */
                    ret = _sys_usw_l2_port_list_remove(lchip, p_l2_node);
                    if (ret < 0)
                    {
                        L2_UNLOCK;
                        continue;
                    }

                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "HW learning found old one: \n");
                    FDB_ERR_RET_UL(_sys_usw_l2_fdb_get_gport_by_adindex(lchip, data.ad_index, &(p_l2_node->adptr->gport), &is_logic));
                    p_l2_node->key.fid = data.fid;
                    FDB_SET_MAC(p_l2_node->key.mac, data.mac);
                    p_l2_node->key_index    = data.key_index;
                    p_l2_node->adptr->index = data.ad_index;
                    if (is_logic)
                    {
                        p_l2_node->adptr->flag.logic_port = 1;
                    }

                    FDB_ERR_RET_UL(_sys_usw_l2_port_list_add(lchip, p_l2_node));
                }
                else
                {
                    MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node, sizeof(sys_l2_node_t));
                    if (NULL == p_l2_node)
                    {
                        L2_UNLOCK;
                        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

                    }

                    FDB_ERR_RET_UL(_sys_usw_l2_fdb_get_gport_by_adindex(lchip, data.ad_index, &(p_l2_node->adptr->gport), &is_logic));
                    p_l2_node->key.fid = data.fid;
                    FDB_SET_MAC(p_l2_node->key.mac, data.mac);
                    p_l2_node->key_index    = data.key_index;
                    p_l2_node->adptr->index = data.ad_index;
                    if (is_logic)
                    {
                        p_l2_node->adptr->flag.logic_port = 1;
                    }
                    FDB_ERR_RET_UL(_sys_usw_l2uc_add_sw(lchip, p_l2_node));
                }
            }
#endif
            _sys_usw_l2_update_count(lchip, &sys.flag, 1, L2_COUNT_GLOBAL, &data.key_index);
            L2_UNLOCK;
        }
    }

    return CTC_E_NONE;
}
    #endif
#define _MC_DEFAULT_

/*this function is used by l2mc & l2 default entry*/
STATIC int32
_sys_usw_l2_update_nh(uint8 lchip, uint16 group_id, mc_mem_t* member, uint8 is_add)
{
    int32                      ret        = CTC_E_NONE;
    uint8                      aps_brg_en = 0;
    uint8                      dest_chipid = 0;
    uint32                     m_nhid     = member->nh_id;
    uint32                     m_gport    = member->mem_port;
    uint8                      use_destmap_profile = 0;
    uint8                      is_assign = member->is_assign;
    uint8                      with_nh = member->with_nh;
    uint8                      xgpon_en = 0;

    sys_nh_param_mcast_group_t nh_mc;
    sal_memset(&nh_mc, 0, sizeof(nh_mc));

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

     /*mcast nexthop id*/
    CTC_ERROR_RETURN(sys_usw_nh_get_mcast_nh(lchip, group_id, &nh_mc.nhid));
    nh_mc.dsfwd_valid = 0;
    nh_mc.opcode      = is_add ? SYS_NH_PARAM_MCAST_ADD_MEMBER :
                        SYS_NH_PARAM_MCAST_DEL_MEMBER;

    L2_LOCK;
    if (with_nh)
    {
        sys_nh_info_dsnh_t nh_info;
        sal_memset(&nh_info, 0, sizeof(nh_info));
        CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo(lchip, m_nhid, &nh_info, 0), ret, error_0);

        if(nh_info.is_ecmp_intf || nh_info.ecmp_valid)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Mcast or vlan default entry not support ecmp member\n");
            goto error_1;
        }
        aps_brg_en = nh_info.aps_en;

        nh_mc.mem_info.ref_nhid    = m_nhid;
        use_destmap_profile = nh_info.use_destmap_profile;
        if (aps_brg_en && is_assign)
        {
            L2_UNLOCK;
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;

        }

        if (is_assign)
        {
            /*Assign mode, dest id using user assign*/
            dest_chipid =  CTC_MAP_GPORT_TO_GCHIP(m_gport);
            nh_mc.mem_info.destid = CTC_MAP_GPORT_TO_LPORT(m_gport);
            nh_mc.mem_info.member_type = SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH_DESTID;
        }
        else
        {
            dest_chipid = CTC_MAP_GPORT_TO_GCHIP(nh_info.gport);
            nh_mc.mem_info.destid = aps_brg_en?nh_info.gport:CTC_MAP_GPORT_TO_LPORT(nh_info.gport);
            nh_mc.mem_info.member_type = aps_brg_en ? SYS_NH_PARAM_BRGMC_MEM_LOCAL_WITH_APS_BRIDGE :
                                                      SYS_NH_PARAM_MCAST_MEM_LOCAL_WITH_NH;
        }
        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (!DRV_IS_DUET2(lchip) && xgpon_en && member->mem_port)
        {
            nh_mc.mem_info.is_logic_port = 1;
            nh_mc.mem_info.logic_port = member->mem_port;
        }

    }
    else
    {
        SYS_GPORT_CHECK_WITH_UNLOCK(m_gport, pl2_usw_master[lchip]->l2_mutex);

        dest_chipid =  CTC_MAP_GPORT_TO_GCHIP(m_gport);
        nh_mc.mem_info.destid = CTC_MAP_GPORT_TO_LPORT(m_gport);
		nh_mc.mem_info.member_type = member->remote_chip? SYS_NH_PARAM_MCAST_MEM_REMOTE : SYS_NH_PARAM_BRGMC_MEM_LOCAL;
    }

    if (!member->remote_chip && (aps_brg_en ||  CTC_LINKAGG_CHIPID == dest_chipid))
    {
        nh_mc.mem_info.is_linkagg = !aps_brg_en;
        nh_mc.mem_info.lchip = lchip;
        nh_mc.mem_info.is_destmap_profile = use_destmap_profile;
        CTC_ERROR_GOTO(sys_usw_mcast_nh_update(lchip, &nh_mc), ret, error_1);
    }
    else
    {

        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if ((!xgpon_en) && (FALSE == sys_usw_chip_is_local(lchip, dest_chipid)))
        {
            ret = CTC_E_NONE;
            goto error_0;
        }

        nh_mc.mem_info.lchip      = lchip;
        nh_mc.mem_info.is_linkagg = 0;
        nh_mc.mem_info.is_destmap_profile = use_destmap_profile;
        CTC_ERROR_GOTO(sys_usw_mcast_nh_update(lchip, &nh_mc), ret, error_1);
    }

    L2_UNLOCK;
    return CTC_E_NONE;

 error_1:
    ret = CTC_E_INVALID_CONFIG;
 error_0:
    L2_UNLOCK;
    return ret;
}


STATIC int32
_sys_usw_l2_update_l2mc_nh(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr, uint8 is_add)
{
    mac_lookup_result_t rslt;
    int32 ret = 0;
    sys_l2_info_t  sys;
    mc_mem_t  mc_member;
    /*uint8 xgpon_en = 0;
    uint8 aps_brg_en = 0;
    uint32 dest_id    = 0*/;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2mc_addr);
    SYS_L2_FID_CHECK(l2mc_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s,fid :%d, member_nd:%d, member_port:%d \n",
                      sys_output_mac(l2mc_addr->mac),
                      l2mc_addr->fid, l2mc_addr->member.nh_id, l2mc_addr->member.mem_port);

    if (CTC_FLAG_ISSET(l2mc_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT) && !(l2mc_addr->with_nh))
    {
        return CTC_E_INVALID_PARAM;
    }

    L2_LOCK;

    ret = _sys_usw_l2_acc_lookup_mac_fid(lchip, l2mc_addr->mac, l2mc_addr->fid, &rslt);
    if (ret < 0)
    {
        L2_UNLOCK;
        return ret;
    }

    if (!rslt.hit)
    {
        L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    _sys_usw_l2_build_sys_ad_info_by_index(lchip, rslt.ad_index, &sys);

    if(sys.addr_type == SYS_L2_ADDR_TYPE_MC)
    {

        /*sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (DRV_IS_DUET2(lchip) && xgpon_en)
        {
            if (l2mc_addr->with_nh)
            {
                FDB_ERR_RET_UL(sys_usw_nh_get_port(lchip, l2mc_addr->member.nh_id, &aps_brg_en, &dest_id));
                l2mc_addr->with_nh = 0;
                l2mc_addr->member.mem_port = dest_id;
            }
        }*/

        mc_member.is_assign = CTC_FLAG_ISSET(l2mc_addr->flag, CTC_L2_FLAG_ASSIGN_OUTPUT_PORT);
        mc_member.mem_port = l2mc_addr->member.mem_port;
        mc_member.nh_id = l2mc_addr->member.nh_id;
        mc_member.with_nh = l2mc_addr->with_nh;
               mc_member.remote_chip = l2mc_addr->remote_chip;
        L2_UNLOCK;
        return _sys_usw_l2_update_nh(lchip, sys.addr.l2mc_addr.l2mc_grp_id, &mc_member, is_add);
    }
    else
    {
        L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_update_default_nh(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr, uint8 is_add)
{
    sys_l2_fid_node_t * p_fid_node = NULL;
    mc_mem_t          mc_member;
    uint8 xgpon_en = 0;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2dflt_addr);
    SYS_L2_FID_CHECK(l2dflt_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d \n", l2dflt_addr->fid);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "member port:0x%x\n", l2dflt_addr->member.mem_port);


    if (CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_ASSIGN_OUTPUT_PORT) && !(l2dflt_addr->with_nh))
    {
        return CTC_E_INVALID_PARAM;
    }

    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
    L2_LOCK;
    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, l2dflt_addr->fid);
    if (NULL == p_fid_node)
    {
        FDB_ERR_RET_UL(CTC_E_NOT_EXIST);
    }

    mc_member.is_assign = CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_ASSIGN_OUTPUT_PORT);
    mc_member.with_nh = l2dflt_addr->with_nh;
    mc_member.nh_id = l2dflt_addr->member.nh_id;
    mc_member.mem_port = l2dflt_addr->member.mem_port;
    if (!DRV_IS_DUET2(lchip) && xgpon_en && mc_member.with_nh)
    {
        mc_member.mem_port = l2dflt_addr->logic_port;
    }
	mc_member.remote_chip = l2dflt_addr->remote_chip;
    L2_UNLOCK;

    return _sys_usw_l2_update_nh(lchip, p_fid_node->mc_gid, &mc_member, is_add);
}
#if 0
STATIC int32
_sys_usw_l2mcast_remove_all_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

}
#endif
int32
sys_usw_l2mcast_add_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return _sys_usw_l2_update_l2mc_nh(lchip, l2mc_addr, 1);
}
int32
sys_usw_l2mcast_remove_member(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return _sys_usw_l2_update_l2mc_nh(lchip, l2mc_addr, 0);
}

int32
sys_usw_l2mcast_add_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr)
{
    sys_nh_param_mcast_group_t nh_mcast_group;

    int32                      ret     = CTC_E_NONE;
    mac_lookup_result_t        result;
    sys_l2_info_t              new_sys;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2mc_addr);

    SYS_L2_FID_CHECK(l2mc_addr->fid);
    SYS_USW_CID_CHECK(lchip,l2mc_addr->cid);

    if (SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_L2MC) <= pl2_usw_master[lchip]->l2mc_count)
    {
        return CTC_E_NO_RESOURCE;
    }

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s, fid :%d, l2mc_group_id: %d  \n",
                      sys_output_mac(l2mc_addr->mac),
                      l2mc_addr->fid,
                      l2mc_addr->l2mc_grp_id);

    sal_memset(&new_sys, 0, sizeof(sys_l2_info_t));
    sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));

    L2_LOCK;
     /*  lookup hw, check result*/
    CTC_ERROR_GOTO(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2mc_addr->mac, l2mc_addr->fid, &result), ret, error_0);
    if (result.conflict)
    {
        ret = CTC_E_HASH_CONFLICT;
        goto error_0;
    }

    if (result.hit && !l2mc_addr->share_grp_en)
    {
        ret = CTC_E_EXIST;
        goto error_0;
    }
    new_sys.is_exist = result.hit;

    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_MC, l2mc_addr, &new_sys, 0, 0), ret, error_0);

    if (l2mc_addr->share_grp_en)
    {
        if (l2mc_addr->l2mc_grp_id)
        {
            CTC_ERROR_GOTO(sys_usw_nh_get_mcast_nh(lchip, l2mc_addr->l2mc_grp_id, &nh_mcast_group.nhid), ret, error_0);
        }
        else if (!result.hit)
        {
            CTC_SET_FLAG(new_sys.addr.l2_addr.flag, CTC_L2_FLAG_DISCARD);
        }
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_mcast_nh_create(lchip, l2mc_addr->l2mc_grp_id, &nh_mcast_group), ret, error_0);
    }

    if (result.hit)
    {
        new_sys.ad_index = result.ad_index;
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, 1, 1, &new_sys.ad_index), ret, error_1);
    }

    /* CTC_ERROR_GOTO(_sys_usw_l2_mc2nhop_add(lchip, l2mc_addr->l2mc_grp_id, nh_mcast_group.nhid),ret, error_2);*/

     /*4. write hw*/
     CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &new_sys), ret, error_2);

     SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add mcast fdb: chip_id:%d, key_index:0x%x, ad_index:0x%x, nexthop_id:0x%x, ds_fwd_offset:0x%x\n",
                         lchip, new_sys.key_index, (uint32) new_sys.ad_index, nh_mcast_group.nhid, new_sys.fwd_ptr);

    if (!result.hit)
    {
        /* 5. update index*/
        pl2_usw_master[lchip]->l2mc_count += 1;
    }

    L2_UNLOCK;
    return ret;

     /*_sys_usw_l2_mc2nhop_remove(lchip, l2mc_addr->l2mc_grp_id);*/
 error_2:
    if(!result.hit)
    {
        sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, new_sys.ad_index);
    }
 error_1:
    if(!l2mc_addr->share_grp_en)
    {
        sys_usw_mcast_nh_delete(lchip, nh_mcast_group.nhid);
    }
 error_0:
    L2_UNLOCK;
    return ret;
}


int32
sys_usw_l2mcast_remove_addr(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr)
{
    sys_l2_info_t       sys;
    mac_lookup_result_t rslt;
    uint32 nhp_id = 0;
    int32 ret = CTC_E_NONE;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2mc_addr);
    SYS_L2_FID_CHECK(l2mc_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac:%s,fid :%d  \n",
                      sys_output_mac(l2mc_addr->mac),
                      l2mc_addr->fid);


    sal_memset(&sys, 0, sizeof(sys_l2_info_t));

    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2mc_addr->mac, l2mc_addr->fid, &rslt));

    if (rslt.hit)
    {
        _sys_usw_l2_build_sys_ad_info_by_index(lchip, rslt.ad_index, &sys);
    }
    else
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    L2_LOCK;
    if(sys.addr_type == SYS_L2_ADDR_TYPE_MC)
    {
        sys_usw_nh_get_mcast_nh(lchip, sys.addr.l2mc_addr.l2mc_grp_id, &nhp_id);
    }
    else
    {
        L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    if (CTC_FLAG_ISSET(l2mc_addr->flag, CTC_L2_FLAG_KEEP_EMPTY_ENTRY))
    {
        sys_usw_nh_remove_all_members(lchip, nhp_id);
        L2_UNLOCK;
        return CTC_E_NONE;
    }

    /* remove hw entry */
    ret = sys_usw_l2_delete_hw_by_mac_fid(lchip, l2mc_addr->mac, l2mc_addr->fid,0,0,0);
    if (ret)
    {
        L2_UNLOCK;
        return ret;
    }
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove mcast fdb: key_index:0x%x, ad_index:0x%x\n",
                    rslt.key_index, rslt.ad_index);

    /* must lookup success. */
    if(0 != nhp_id)
    {
        if (!sys.share_grp_en)
        {
            /* remove sw entry */
            sys_usw_mcast_nh_delete(lchip, nhp_id);
        }
        sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, rslt.ad_index);

    }
    else
    {
        if (!sys.share_grp_en)
        {
            L2_UNLOCK;
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            return CTC_E_NOT_EXIST;
        }
        else
        {
            sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, rslt.ad_index);
        }
    }

    pl2_usw_master[lchip]->l2mc_count--;
    L2_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_l2_add_port_to_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return _sys_usw_l2_update_default_nh(lchip, l2dflt_addr, 1);
}

int32
sys_usw_l2_remove_port_from_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return _sys_usw_l2_update_default_nh(lchip, l2dflt_addr, 0);
}

int32
sys_usw_l2_add_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    sys_l2_info_t              sys;
    int32                      ret     = CTC_E_NONE;
    sys_nh_param_mcast_group_t nh_mcast_group;
    sys_l2_fid_node_t          * p_fid_node = NULL;
    uint32                     nh_id = 0;
    uint32                     max_ex_nhid = 0;
    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2dflt_addr);
    SYS_L2_FID_CHECK(l2dflt_addr->fid);


    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d ,l2mc_grp_id:%d \n", l2dflt_addr->fid, l2dflt_addr->l2mc_grp_id);

    SYS_USW_CID_CHECK(lchip,l2dflt_addr->cid);

    if (CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
  /* not support. */
    }

    if (CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
  /* not support. */
    }

    L2_LOCK;
    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_DF, l2dflt_addr, &sys, 0, 0), ret, error_0);

    sys.ad_index = DEFAULT_ENTRY_INDEX(l2dflt_addr->fid);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "build hash mcast ad_index:0x%x\n", sys.ad_index);

    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, l2dflt_addr->fid);
    if (NULL == p_fid_node)
    {
        MALLOC_ZERO(MEM_FDB_MODULE, p_fid_node, sizeof(sys_l2_fid_node_t));
        if (!p_fid_node)
        {
            ret = CTC_E_NO_MEMORY;
            goto error_0;
        }

        /* 1,create mcast group */
        if(l2dflt_addr->share_grp_en)
        {
            CTC_ERROR_GOTO(sys_usw_nh_get_mcast_nh(lchip, l2dflt_addr->l2mc_grp_id, &nh_id), ret, error_1);
            sys_usw_nh_get_max_external_nhid(lchip, &max_ex_nhid);
            if (nh_id > max_ex_nhid)
            {
                ret = CTC_E_NOT_SUPPORT;
                goto  error_1;
            }
        }
        else
        {
            CTC_ERROR_GOTO(sys_usw_mcast_nh_create(lchip, l2dflt_addr->l2mc_grp_id, &nh_mcast_group), ret, error_1);
        }

        /* map fid node.*/
        p_fid_node->fid    = l2dflt_addr->fid;
        p_fid_node->flag   = l2dflt_addr->flag;
        p_fid_node->mc_gid = l2dflt_addr->l2mc_grp_id;
        p_fid_node->share_grp_en = l2dflt_addr->share_grp_en;
        p_fid_node->cid = l2dflt_addr->cid;
        ret                = ctc_vector_add(pl2_usw_master[lchip]->fid_vec, l2dflt_addr->fid, p_fid_node);
        if (FAIL(ret))
        {
            goto error_2;
        }
    }
    else
    {
        if(p_fid_node->mc_gid != l2dflt_addr->l2mc_grp_id)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "default entry has been created, l2mc group id cann't be changed\n");
            ret = CTC_E_EXIST;
        }
        else
        {
            ret = CTC_E_NONE;  /* already created. */

        }
        goto error_0;
    }

    /* write hw */
    ret = _sys_usw_l2_write_hw(lchip, &sys);
    if (FAIL(ret))
    {
        goto error_3;
    }

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add default entry: chip_id:%d, ad_index:0x%x, group_id: 0x%x\n", lchip, sys.ad_index,
                         sys.addr.l2df_addr.l2mc_grp_id);

    pl2_usw_master[lchip]->def_count++;
    L2_UNLOCK;
    return CTC_E_NONE;

 error_3:
    ctc_vector_del(pl2_usw_master[lchip]->fid_vec, l2dflt_addr->fid);
 error_2:
    if (!p_fid_node->share_grp_en)
    {
        sys_usw_mcast_nh_delete(lchip, nh_mcast_group.nhid);
    }
 error_1:
    mem_free(p_fid_node);
 error_0:
    L2_UNLOCK;
    return ret;
}


int32
sys_usw_l2_remove_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    sys_l2_fid_node_t * p_fid_node = NULL;
    sys_l2_info_t     sys;
    int32             ret = 0;
    uint32            nh_id = 0;
    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2dflt_addr);
    SYS_L2_FID_CHECK(l2dflt_addr->fid);

    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d  \n", l2dflt_addr->fid);

    L2_LOCK;

    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, l2dflt_addr->fid);
    if (NULL == p_fid_node)
    {
        ret = CTC_E_NOT_EXIST;
        goto error_0;
    }

    sys.ad_index          = DEFAULT_ENTRY_INDEX(l2dflt_addr->fid);
    sys.addr_type  = SYS_L2_ADDR_TYPE_DEFAULT;
    sys.revert_default    = 1;

    /* remove hw */
    CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &sys), ret, error_0);
    sys_usw_nh_get_mcast_nh(lchip, p_fid_node->mc_gid, &nh_id);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove default entry: chip_id:%d, ad_index:0x%x nexthop_id 0x%x\n" \
                                            , lchip, DEFAULT_ENTRY_INDEX(p_fid_node->fid), nh_id);
    if (!p_fid_node->share_grp_en)
    {
        sys_usw_mcast_nh_delete(lchip, nh_id);
    }
    /* remove sw*/
    ctc_vector_del(pl2_usw_master[lchip]->fid_vec, p_fid_node->fid);
    mem_free(p_fid_node);

    pl2_usw_master[lchip]->def_count--;

 error_0:
    L2_UNLOCK;
    return ret;
}

#define _FDB_SHOW_


int32
sys_usw_l2_show_status(uint8 lchip)
{
    uint32 ad_cnt = 0;
    uint32 rsv_ad_num = 0;
    uint32 fdb_num = 0;
    uint32 running_num = 0;
    uint32 dynamic_running_num = 0;
    uint32 cmd = 0;
    ctc_l2_fdb_query_t query;
    MacLimitSystem_m mac_limit_system;
    uint32 alloc_ad_cnt = 0;
    uint16  loop1 = 0;
    uint16  loop2 = 0;
    uint16 used_count = 0;

    SYS_L2_INIT_CHECK();

    sal_memset(&query, 0, sizeof(ctc_l2_fdb_query_t));
    sal_memset(&mac_limit_system, 0, sizeof(MacLimitSystem_m));

    sys_usw_ftm_query_table_entry_num(lchip, DsMac_t, &ad_cnt);
    rsv_ad_num = SYS_USW_MAX_LINKAGG_GROUP_NUM + SYS_USW_MAX_PORT_NUM_PER_CHIP + (pl2_usw_master[lchip]->cfg_max_fid+1);

    if (!pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        rsv_ad_num += pl2_usw_master[lchip]->cfg_vport_num;
    }

    fdb_num = sys_usw_ftm_get_spec(lchip, CTC_FTM_SPEC_MAC);

    cmd = DRV_IOR(MacLimitSystem_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_limit_system));
    GetMacLimitSystem(A, profileRunningCount_f, &mac_limit_system, &running_num);
    GetMacLimitSystem(A, dynamicRunningCount_f, &mac_limit_system, &dynamic_running_num);

     /*get alloc ad count*/
    sys_usw_ftm_get_alloced_table_count(lchip, DsMac_t, &alloc_ad_cnt);
    L2_LOCK;
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------Work Mode---------------------\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6s \n", "Hardware Learning", pl2_usw_master[lchip]->cfg_hw_learn?"Y":"N");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------L2 FDB Resource---------------------\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Key Resource:\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Total Key count", fdb_num);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Used Key count", running_num+pl2_usw_master[lchip]->l2mc_count);


    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--Static", running_num-dynamic_running_num);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--Dynamic", dynamic_running_num);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--L2MC", pl2_usw_master[lchip]->l2mc_count);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ad Resource:\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Total Ad count", ad_cnt);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Used Ad count", alloc_ad_cnt);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--Reserved ", rsv_ad_num);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%3u(%u) \n", "--Drop(index)", 1, pl2_usw_master[lchip]->ad_index_drop);


    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--Dynamic", alloc_ad_cnt-rsv_ad_num-1);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ad Base:\n");
    if (!pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-4x \n", "Logic Port", _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_VPORT));
    }
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-4x \n", "Port", _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-4x \n", "Linkagg", _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-4x \n", "Default Entry", pl2_usw_master[lchip]->dft_entry_base);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Other Resource:\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Default entry count", pl2_usw_master[lchip]->def_count);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "FID Number", pl2_usw_master[lchip]->cfg_max_fid+1);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Logic Port Number", pl2_usw_master[lchip]->cfg_vport_num);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "BlackHole Resource:\n")
    for(loop1=0; loop1<SYS_L2_MAX_BLACK_HOLE_ENTRY/32; loop1++)
    {
        for(loop2=0; loop2 < 32; loop2++)
        {
            if(CTC_IS_BIT_SET(pl2_usw_master[lchip]->mac_fdb_bmp[loop1], loop2))
            {
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "----key index", loop1*32 + loop2);
                used_count++;
            }
        }
    }
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "--Total entry count", used_count);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

     L2_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_l2_dump_default_entry(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    uint32 nh_id                  = 0;
    sys_l2_fid_node_t* p_fid_node = NULL;

    SYS_L2_INIT_CHECK();

    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, l2dflt_addr->fid);
    if (NULL == p_fid_node)
    {
        CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
    }

    sys_usw_nh_get_mcast_nh(lchip, p_fid_node->mc_gid, &nh_id);
    sys_usw_nh_dump(lchip, nh_id, FALSE);
    return CTC_E_NONE;
}

int32
sys_usw_l2_dump_all_default_entry(uint8 lchip)
{
    sys_l2_fid_node_t* p_fid_node = NULL;
    uint16 fid = 0;
    uint16 pre_entry = 0;
    uint16 last_entry = 0;
    uint8 first_flag = 0;
    uint8 find_first = 0;
    uint8 seq_flag = 0;
    uint16 count = 0;
    uint16 block_cnt = 0;
    char buf[20] = {0};
    char tmp_buf[10] = {0};

    SYS_L2_INIT_CHECK();

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Fid list:\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------\n");

    for (fid = 1; fid <= pl2_usw_master[lchip]->cfg_max_fid; fid++)
    {
        p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, fid);
        if (NULL == p_fid_node)
        {
            continue;
        }

        count++;
        last_entry = fid;

        if (find_first == 0)
        {
            first_flag = 1;
        }

        if (first_flag)
        {
            sal_sprintf(buf, "%d",fid);
            find_first = 1;
            first_flag = 0;
            pre_entry = fid;
        }
        else
        {
            /*current fid is not sequenced by last*/
            if (fid != (pre_entry+1))
            {
                /*already have sequence fid list*/
                if (seq_flag)
                {
                    /*print last entry, and seperate remark*/
                    sal_sprintf(tmp_buf, "%d,",pre_entry);
                    sal_strcat((char*)buf, (char*)tmp_buf);
                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s ", (char*)buf);
                    block_cnt++;
                    if ((block_cnt%6) == 0)
                    {
                        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
                    }
                    /*print seperate remark and current fid*/
                    sal_sprintf(buf, "%d",fid);
                }
                else
                {
                    /*print seperate remark and current fid*/
                    sal_strcat((char*)buf, ",");
                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s ", (char*)buf);
                    block_cnt++;

                    if ((block_cnt%6) == 0)
                    {
                        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
                    }
                    sal_sprintf(buf, "%d",fid);
                }

                seq_flag = 0;
            }
            else
            {
                /*already have sequence fid list*/
                if (seq_flag)
                {
                    /*do nothing*/
                }
                else
                {
                    sal_strcat((char*)buf, "~");
                    seq_flag = 1;
                }
            }

            pre_entry = fid;
        }
    }

    if (seq_flag)
    {
        sal_sprintf(tmp_buf, "%d",last_entry);
        sal_strcat((char*)buf, (char*)tmp_buf);
    }

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s ", (char*)buf);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total Count:%d\n", count);

    return CTC_E_NONE;
}

int32
sys_usw_l2_dump_l2mc_entry(uint8 lchip, ctc_l2_mcast_addr_t* l2mc_addr, bool detail)
{
    mac_lookup_result_t rslt;
    sys_l2_info_t       sys;
    uint32              nhp_id = 0;

    SYS_L2_INIT_CHECK();

    sal_memset(&rslt, 0, sizeof(mac_lookup_result_t));
    sal_memset(&sys, 0, sizeof(sys_l2_info_t));
    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2mc_addr->mac, l2mc_addr->fid, &rslt));

    if (rslt.hit)
    {
        _sys_usw_l2_build_sys_ad_info_by_index(lchip, rslt.ad_index, &sys);
    }
    else
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }
    L2_LOCK;

    if(sys.addr_type == SYS_L2_ADDR_TYPE_MC)
    {
         /*_sys_usw_l2_get_nhop_by_mc_grp_id(lchip, sys.mc_gid, &nhp_id);*/
        sys_usw_nh_get_mcast_nh(lchip, sys.addr.l2mc_addr.l2mc_grp_id, &nhp_id);
    }
    else
    {
        L2_UNLOCK;
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }
    L2_UNLOCK;

    sys_usw_nh_dump(lchip, nhp_id, detail);
    return CTC_E_NONE;
}


int32
sys_usw_l2_get_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    sys_l2_fid_node_t* p_fid_node = NULL;
    int32            ret          = 0;
    uint16           fid = 0;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2dflt_addr);
    SYS_L2_FID_CHECK(l2dflt_addr->fid);

    fid = l2dflt_addr->fid;
    sal_memset(l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d\n", l2dflt_addr->fid);

    L2_LOCK;

    p_fid_node = _sys_usw_l2_fid_node_lkup(lchip, fid);
    if (NULL == p_fid_node)
    {
        ret = CTC_E_NOT_EXIST;
        goto error_0;
    }

    l2dflt_addr->flag        = p_fid_node->flag;
    l2dflt_addr->fid         = p_fid_node->fid;
    l2dflt_addr->l2mc_grp_id = p_fid_node->mc_gid;

 error_0:
    L2_UNLOCK;
    return ret;
}

int32
sys_usw_l2_set_nhid_by_logic_port(uint8 lchip, uint16 logic_port, uint32 nhp_id)
{
    uint32 ret= 0;

    SYS_L2_INIT_CHECK();
    SYS_LOGIC_PORT_CHECK(logic_port);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    L2_LOCK;
    if(0 == nhp_id)
    {
        ret = _sys_usw_l2_unbind_nhid_by_logic_port(lchip, logic_port);
    }
    else
    {
        ret =_sys_usw_l2_bind_nhid_by_logic_port(lchip, logic_port, nhp_id);
    }
    L2_UNLOCK;
    return ret;
}



#define FDB_LOGIC_PORT_VEC_B    1024
#define FDB_MAC_FID_HASH_SIZE   8192
#define FDB_MAC_FID_HASH_B      64
#define FDB_DFT_HASH_B          64

bool
sys_usw_l2_fdb_trie_sort_en(uint8 lchip)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (pl2_usw_master[lchip] && pl2_usw_master[lchip]->trie_sort_en)
    {
        return TRUE;
    }

    return FALSE;
}
/**internal test**/
int32
sys_usw_l2_fdb_set_trie_sort(uint8 lchip, uint8 enable)
{
    SYS_L2_INIT_CHECK();

    pl2_usw_master[lchip]->trie_sort_en = enable;

    return CTC_E_NONE;
}

int32
sys_usw_l2_set_static_mac_limit(uint8 lchip, uint8 static_limit_en)
{

    SYS_L2_INIT_CHECK();
    pl2_usw_master[lchip]->static_fdb_limit = static_limit_en;
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_l2_init_sw(uint8 lchip)
{
    uint32 ds_mac_size = 0;
    ctc_spool_t spool;
    int    ret = CTC_E_NONE;

    sal_memset(&spool, 0, sizeof(ctc_spool_t));

    pl2_usw_master[lchip]->fdb_hash = ctc_hash_create(CTC_VEC_BLOCK_NUM(FDB_MAC_FID_HASH_SIZE, FDB_MAC_FID_HASH_B),
                                           FDB_MAC_FID_HASH_B,
                                           (hash_key_fn) _sys_usw_l2_fdb_hash_make,
                                           (hash_cmp_fn) _sys_usw_l2_fdb_hash_compare);
    if (!pl2_usw_master[lchip]->fdb_hash)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
		goto error_0;
    }

    /* alloc fdb mac hash size  */

    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsMac_t, &ds_mac_size), ret, error_0);

    spool.lchip = lchip;
    spool.block_num = CTC_VEC_BLOCK_NUM(ds_mac_size, FDB_MAC_FID_HASH_B);
    spool.block_size = FDB_MAC_FID_HASH_B;
    spool.max_count = ds_mac_size;
    spool.user_data_size = sizeof(sys_l2_ad_t);
    spool.spool_key = (hash_key_fn) _sys_usw_l2_ad_spool_hash_make;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_l2_ad_spool_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_l2_ad_spool_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_l2_ad_spool_free_index;
    pl2_usw_master[lchip]->ad_spool = ctc_spool_create(&spool);
    if (!pl2_usw_master[lchip]->ad_spool)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error_0;
    }

    pl2_usw_master[lchip]->fid_vec = ctc_vector_init(CTC_VEC_BLOCK_NUM(pl2_usw_master[lchip]->cfg_max_fid + 1, FDB_DFT_HASH_B),
                                          FDB_DFT_HASH_B);
    if (!pl2_usw_master[lchip]->fid_vec)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error_0;
    }

    if (pl2_usw_master[lchip]->cfg_vport_num)
    {
        pl2_usw_master[lchip]->vport2nh_vec = ctc_vector_init(CTC_VEC_BLOCK_NUM(pl2_usw_master[lchip]->cfg_vport_num, FDB_LOGIC_PORT_VEC_B),
                                                   FDB_LOGIC_PORT_VEC_B);
        if (!pl2_usw_master[lchip]->vport2nh_vec)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            ret = CTC_E_NO_MEMORY;
            goto error_0;
        }
    }

    SYS_L2_REG_SYN_ADD_DB_CB(lchip, add);
    SYS_L2_REG_SYN_DEL_DB_CB(lchip, remove);

    return CTC_E_NONE;
error_0:
    if (pl2_usw_master[lchip]->fid_vec)
    {
        ctc_vector_release(pl2_usw_master[lchip]->fid_vec);
    }
    if (pl2_usw_master[lchip]->ad_spool)
    {
        ctc_spool_free(pl2_usw_master[lchip]->ad_spool);
    }
    if (pl2_usw_master[lchip]->fdb_hash)
    {
        ctc_hash_free(pl2_usw_master[lchip]->fdb_hash);
    }
    return ret;
}

STATIC int32
_sys_usw_l2_init_default_entry(uint8 lchip, uint16 default_entry_num)
{
    uint32                         cmd          = 0;
    uint32                         start_offset = 0;
    uint32                         fld_value    = 0;
    uint16                         fid          = 0;
    sys_l2_info_t                  sys;
    FibEngineLookupResultCtl_m fib_engine_lookup_result_ctl;

    /* get the start offset of DaMac */
    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, default_entry_num, 1, &start_offset));

    /*here must be multiple of 256*/
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Start Offset:%d\n", start_offset);
    pl2_usw_master[lchip]->dft_entry_base = start_offset;


    fld_value = ((pl2_usw_master[lchip]->dft_entry_base >> 8) & 0xFF);

    sal_memset(&fib_engine_lookup_result_ctl, 0, sizeof(fib_engine_lookup_result_ctl));

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl));

    DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                        \
                        FibEngineLookupResultCtl_gMacDaLookupResultCtl_defaultEntryBase_f, \
                        &fib_engine_lookup_result_ctl, fld_value);

    DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                      \
                        FibEngineLookupResultCtl_gMacDaLookupResultCtl_defaultEntryEn_f, \
                        &fib_engine_lookup_result_ctl, TRUE);

    DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                        \
                        FibEngineLookupResultCtl_gMacSaLookupResultCtl_defaultEntryBase_f, \
                        &fib_engine_lookup_result_ctl, fld_value);

    DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                      \
                        FibEngineLookupResultCtl_gMacSaLookupResultCtl_defaultEntryEn_f, \
                        &fib_engine_lookup_result_ctl, TRUE);

    cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl));

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dft_entry_base = 0x%x \n", pl2_usw_master[lchip]->dft_entry_base);


    sal_memset(&sys, 0, sizeof(sys));
    sys.addr_type = SYS_L2_ADDR_TYPE_DEFAULT;
    sys.revert_default    = 1;

    for (fid = 0; fid < pl2_usw_master[lchip]->cfg_max_fid; fid++)
    {
        sys.ad_index = DEFAULT_ENTRY_INDEX(fid);
        CTC_ERROR_RETURN(_sys_usw_l2_write_hw(lchip, &sys));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_set_global_cfg(uint8 lchip, ctc_l2_fdb_global_cfg_t* pcfg, uint8 vp_alloc_en)
{
    CTC_PTR_VALID_CHECK(pcfg);

    /* allocate logic_port_2_nh vec, only used for logic_port use hw learning */
    /* set the num of logic port */
    pl2_usw_master[lchip]->cfg_vport_num = pcfg->logic_port_num;
    pl2_usw_master[lchip]->cfg_hw_learn  = pcfg->hw_learn_en;
    pl2_usw_master[lchip]->trie_sort_en  = pcfg->trie_sort_en;
    if (pl2_usw_master[lchip]->trie_sort_en)
    {
        sys_usw_fdb_sort_register_fdb_entry_cb(lchip, sys_usw_l2_get_fdb_entry_cb);
        sys_usw_fdb_sort_register_trie_sort_cb(lchip, sys_usw_l2_fdb_trie_sort_en);
    }

    sys_usw_l2_set_static_mac_limit(lchip, pcfg->static_fdb_limit_en);

    if (pcfg->default_entry_rsv_num)
    {
        pl2_usw_master[lchip]->cfg_max_fid = pcfg->default_entry_rsv_num - 1;
    }
    else
    {
        pl2_usw_master[lchip]->cfg_max_fid = 5 * 1024 - 1;     /*default num 5K*/
    }

    /* add default entry */
    CTC_ERROR_RETURN(_sys_usw_l2_init_default_entry(lchip, pl2_usw_master[lchip]->cfg_max_fid + 1));

    /* stp init */
    CTC_ERROR_RETURN(sys_usw_stp_init(lchip, pcfg->stp_mode));

    /*default is 0*/
    pl2_usw_master[lchip]->vp_alloc_ad_en = vp_alloc_en;
    if (pl2_usw_master[lchip]->vp_alloc_ad_en && !pl2_usw_master[lchip]->cfg_hw_learn)
    {
        uint32 cmd = 0;
        uint32 value = 1;
        DsMac_m dsmac;
        sal_memset(&dsmac, 0, sizeof(dsmac));
        cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_logicSrcPortAdType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsmac));
        SetDsMac(V, learnSource_f, &dsmac, 1);
        SetDsMac(V, fastLearningEn_f, &dsmac, 1);
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dsmac));
    }

	/*Configure Max logic port*/
    {
        uint32 cmd = 0;
        uint32 value = 0;

		value = vp_alloc_en?0xffff:pcfg->logic_port_num;
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_maxLearningLogicSrcPort_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_set_register(uint8 lchip, uint8 hw_en)
{
    uint32                 cmd   = 0;
    FibAccelerationCtl_m fib_acceleration_ctl;
 /*-    FibAccDrainEnable_m fib_drain_ctl;*/
    uint32 value = 0;
    uint32 cmd1 = 0;
    uint32 cmd2 = 0;
    uint32 cmd3 = 0;
    uint32 cmd4 = 0;
    uint32 cmd5 = 0;
    uint32 cmd6 = 0;
    uint32 field_value = 0;
    uint8 lgchip = 0;
    uint8 hw_sync_en = 0;
    sal_memset(&fib_acceleration_ctl, 0, sizeof(fib_acceleration_ctl));
   /*-TBD  sal_memset(&fib_drain_ctl, 0, sizeof(FibAccDrainEnable_m));*/

    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));

    field_value = pl2_usw_master[lchip]->cfg_hw_learn?1:0;
    SetFibAccelerationCtl(V, fastAgingIgnoreFifoFull_f, &fib_acceleration_ctl, field_value);
    SetFibAccelerationCtl(V, fastLearningIgnoreFifoFull_f, &fib_acceleration_ctl, field_value);

    /* for hw_aging gb alwasy set 1 */
    field_value = pl2_usw_master[lchip]->cfg_hw_learn;
    cmd = DRV_IOW(IpeAgingCtl_t, IpeAgingCtl_hwAgingEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));


    /* always set hardware aging. like the bit in dsaging. */
    SetFibAccelerationCtl(V, hardwareAgingEn_f, &fib_acceleration_ctl, 1);
    sys_usw_get_gchip_id(lchip, &lgchip);
    SetFibAccelerationCtl(V, learningOnStacking_f, &fib_acceleration_ctl, hw_en);
    SetFibAccelerationCtl(V, chipId_f, &fib_acceleration_ctl, lgchip);
    cmd = DRV_IOW(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));



#if 0
    /* set IpeLearningCtl.macTableFull always 1 to support while-list function */
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_macTableFull_f);
    value = 1;
    if(TRUE)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
#endif


    value = hw_en;
    cmd1 = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_learningOnStacking_f);
    cmd2 = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_learningOnStacking_f);
    cmd3 = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_learningOnStacking_f);
    cmd4 = DRV_IOW(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_learningOnStacking_f);
    cmd5 = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_learningOnStacking_f);
    cmd6 = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_stackingLearningType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &value));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd2, &value));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd3, &value));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd4, &value));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd5, &value));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd6, &value));
#if 0
    /*Set acc priority*/
    cmd = DRV_IOR(FibAccDrainEnable_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_drain_ctl));
    SetFibAccDrainEnable(V, forceHighPriorityEn_f, &fib_drain_ctl, 1);
    cmd = DRV_IOW(FibAccDrainEnable_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_drain_ctl));
#endif
    CTC_ERROR_RETURN(sys_usw_dma_get_hw_learning_sync(lchip, &hw_sync_en));
    CTC_ERROR_RETURN(sys_usw_learning_aging_set_hw_sync(lchip, hw_sync_en));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_reserve_port_dsmac(uint8 lchip, uint8 gchip, uint32 ad_base, uint32 max_port_num)
{
    uint32        drv_lport = 0;
    uint32        cmd   = 0;
    DsMac_m      macda;
    uint16        dest_gport = 0;
    sys_l2_info_t sys;
    uint32        nhid = 0;

    sal_memset(&macda, 0, sizeof(DsMac_m));
    sal_memset(&sys, 0, sizeof(sys_l2_info_t));

    cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);

    for (drv_lport = 0; drv_lport < max_port_num; drv_lport++)
    {
        dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, drv_lport);

        /*get nexthop id from gport*/
        sys_usw_l2_get_ucast_nh(lchip, dest_gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nhid);

        _sys_usw_l2_map_nh_to_sys(lchip, nhid, &sys, 0, dest_gport, 0);

        CTC_ERROR_RETURN(_sys_usw_l2_encode_dsmac(lchip, &macda, &sys));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_base + drv_lport, cmd, &macda));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_reserve_ad_index(uint8 lchip)
{
    uint32 rsv_ad_num = 0;
    uint32 offset     = 0;
    uint8 gchip = 0;
    uint32 cmd = 0;
    FibAccelerationCtl_m fib_acceleration_ctl;

    rsv_ad_num = SYS_USW_MAX_LINKAGG_GROUP_NUM + SYS_USW_MAX_PORT_NUM_PER_CHIP;

    if (!pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
         /*only hw learning should reserve AD for logic port*/
        rsv_ad_num += pl2_usw_master[lchip]->cfg_vport_num;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 1, rsv_ad_num, 1, &offset));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " logic_port_num: 0x%x  rsv_ad_num:0x%x  offset:0x%x\n",
                     pl2_usw_master[lchip]->cfg_vport_num, rsv_ad_num, offset);

    sal_memset(&fib_acceleration_ctl, 0, sizeof(fib_acceleration_ctl));
    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));

    if (pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        if (pl2_usw_master[lchip]->cfg_hw_learn)
        {
            /*base from default entry+drop rsv*/
            SetFibAccelerationCtl(V, dsMacBase0_f, &fib_acceleration_ctl, (pl2_usw_master[lchip]->dft_entry_base+pl2_usw_master[lchip]->cfg_max_fid+2)); /*logic port base*/
        }
    }
    else
    {
        SetFibAccelerationCtl(V, dsMacBase0_f, &fib_acceleration_ctl, offset + SYS_USW_MAX_LINKAGG_GROUP_NUM + SYS_USW_MAX_PORT_NUM_PER_CHIP); /*logic port*/
    }
    SetFibAccelerationCtl(V, dsMacBase1_f, &fib_acceleration_ctl, offset + SYS_USW_MAX_LINKAGG_GROUP_NUM);      /*gport*/
    SetFibAccelerationCtl(V, dsMacBase2_f, &fib_acceleration_ctl, offset);                  /*linkagg*/

    cmd = DRV_IOW(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));
     /*_sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_VPORT) = offset + SYS_USW_MAX_LINKAGG_GROUP_NUM + SYS_USW_MAX_PORT_NUM_PER_CHIP;*/
     /*_sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT) = offset + SYS_USW_MAX_LINKAGG_GROUP_NUM;*/
     /*_sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK) = offset;*/

    sys_usw_get_gchip_id(lchip, &gchip);
    CTC_ERROR_RETURN(_sys_usw_l2_reserve_port_dsmac(lchip, gchip,  offset + SYS_USW_MAX_LINKAGG_GROUP_NUM, SYS_PHY_PORT_NUM_PER_SLICE));


    return CTC_E_NONE;
}

/* for stacking */
int32
sys_usw_l2_reserve_rchip_ad_index(uint8 lchip, uint8 gchip, uint16 max_port_num)
{
    uint32 rsv_ad_num = 0;
    uint32 offset     = 0;
    uint32 value = 0;

    uint32 cmd = 0;
    FibAccelerationRemoteChipAdBase_m fib_acceleration_ctl;

    SYS_GLOBAL_CHIPID_CHECK(gchip);
    SYS_L2_INIT_CHECK();


    /*if gchip is local, return */
    if (sys_usw_chip_is_local(lchip, gchip))
    {
        return CTC_E_NONE;
    }

    rsv_ad_num = max_port_num;
    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 1, rsv_ad_num, 64, &offset));

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Alloc  rsv_ad_num:0x%x  offset:0x%x\n", rsv_ad_num, offset);

    value = (offset>>6);

    CTC_ERROR_RETURN(_sys_usw_l2_reserve_port_dsmac(lchip, gchip, offset, max_port_num));

    L2_LOCK;
    pl2_usw_master[lchip]->rchip_ad_rsv[gchip] = rsv_ad_num;
    L2_UNLOCK;

    cmd = DRV_IOR(FibAccelerationRemoteChipAdBase_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));
    DRV_IOW_FIELD(lchip, FibAccelerationRemoteChipAdBase_t, FibAccelerationRemoteChipAdBase_array_0_remoteChipAdBase_f+gchip,
        &value, &fib_acceleration_ctl);
    cmd = DRV_IOW(FibAccelerationRemoteChipAdBase_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsMac rsv num for rchip:%d, base:0x%x\n",rsv_ad_num,offset);
    return CTC_E_NONE;
}

/* for stacking */
int32
sys_usw_l2_free_rchip_ad_index(uint8 lchip, uint8 gchip, uint16 max_port_num)
{
    uint32 rsv_ad_num = 0;
    uint32 offset     = 0;

    uint32 cmd = 0;
    FibAccelerationRemoteChipAdBase_m fib_acceleration_ctl;

    SYS_GLOBAL_CHIPID_CHECK(gchip);
    SYS_L2_INIT_CHECK();

    /*if gchip is local, return */
    if (sys_usw_chip_is_local(lchip, gchip))
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(FibAccelerationRemoteChipAdBase_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));

    offset = GetFibAccelerationRemoteChipAdBase(V, array_0_remoteChipAdBase_f + gchip, &fib_acceleration_ctl) << 6;

    rsv_ad_num = pl2_usw_master[lchip]->rchip_ad_rsv[gchip];
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Free  rsv_ad_num:0x%x  offset:0x%x\n", rsv_ad_num, offset);

    if (0 == offset)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(lchip, DsMac_t, 1, rsv_ad_num, offset));
    offset = 0;
    L2_LOCK;
    pl2_usw_master[lchip]->rchip_ad_rsv[gchip] = 0;
    L2_UNLOCK;

    DRV_IOW_FIELD(lchip, FibAccelerationRemoteChipAdBase_t, FibAccelerationRemoteChipAdBase_array_0_remoteChipAdBase_f + gchip,
                  &offset, &fib_acceleration_ctl);
    cmd = DRV_IOW(FibAccelerationRemoteChipAdBase_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acceleration_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2_set_hw_learn(uint8 lchip, uint8 hw_en)
{
    uint32 cmd      = 0;
    uint32 value    = 0;

    /* a nicer solution. */
    value = hw_en ? 1:0;
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_fastLearningEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);

    pl2_usw_master[lchip]->cfg_hw_learn  = hw_en;

    return _sys_usw_l2_set_register(lchip, hw_en);
}
int32
sys_usw_l2_set_hw_learn(uint8 lchip, uint8 hw_en)
{
    SYS_L2_INIT_CHECK();
    LCHIP_CHECK(lchip);

    if(pl2_usw_master[lchip]->cfg_hw_learn == hw_en)
    {
        return CTC_E_NONE;
    }
    if(pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return _sys_usw_l2_set_hw_learn(lchip, hw_en);
}

int32
sys_usw_l2_mc_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_L2_INIT_CHECK();

    specs_info->used_size = pl2_usw_master[lchip]->l2mc_count;

    return CTC_E_NONE;
}

int32
sys_usw_l2_fdb_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    uint32 cmd = 0;
    uint32 value = 0;

    cmd = DRV_IOR(MacLimitSystem_t, MacLimitSystem_profileRunningCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    specs_info->used_size = value;

    return CTC_E_NONE;
}
int32
_sys_usw_l2_fdb_init(uint8 lchip, void* l2_fdb_global_cfg, uint8 vp_alloc_en)
{
    uint32 hash_entry_num = 0;
    int32  ret            = 0;
    uint32 spec_fdb = 0;
    uint32 cmd = 0;
    sys_l2_ad_t l2_ad;
    uint16 fid = 0;
    uint8  work_status = 0;
    if (pl2_usw_master[lchip]) /*already init */
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    CTC_MAX_VALUE_CHECK(((ctc_l2_fdb_global_cfg_t*)l2_fdb_global_cfg)->logic_port_num, DRV_IS_DUET2(lchip)?0x7FFF:0xFFFF);

    /* get num of Hash Key */
    sys_usw_ftm_query_table_entry_num(lchip, DsFibHost0MacHashKey_t, &hash_entry_num);

    if (0 == hash_entry_num)
    {
        return CTC_E_NO_RESOURCE;
    }

    MALLOC_ZERO(MEM_FDB_MODULE, pl2_usw_master[lchip], sizeof(sys_l2_master_t));
    if (!pl2_usw_master[lchip])
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    ret = sal_mutex_create(&(pl2_usw_master[lchip]->l2_mutex));
    if (ret || !(pl2_usw_master[lchip]->l2_mutex))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;

    }

    ret = sal_mutex_create(&(pl2_usw_master[lchip]->l2_dump_mutex));
    if (ret || !(pl2_usw_master[lchip]->l2_dump_mutex))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;

    }


    CTC_ERROR_RETURN(_sys_usw_l2_set_global_cfg(lchip, l2_fdb_global_cfg, vp_alloc_en));

    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsMac_t, 0, 1, 1, &(pl2_usw_master[lchip]->ad_index_drop)));

    CTC_ERROR_RETURN(_sys_usw_l2_reserve_ad_index(lchip));

    CTC_ERROR_RETURN(_sys_usw_l2_init_sw(lchip));
    CTC_ERROR_RETURN(_sys_usw_l2_set_hw_learn(lchip, pl2_usw_master[lchip]->cfg_hw_learn));

    /* add static spool for drop ad*/
    sal_memset(&l2_ad, 0, sizeof(l2_ad));
    SetDsMac(V, dsFwdPtr_f, &l2_ad.ds_mac, SYS_FDB_DISCARD_FWD_PTR);
    SetDsMac(V, protocolExceptionEn_f, &l2_ad.ds_mac, 1);
    SetDsMac(V, isStatic_f, &l2_ad.ds_mac, 1);
    SetDsMac(V, stormCtlEn_f, &l2_ad.ds_mac, 1);
    SetDsMac(V, fastLearningEn_f, &l2_ad.ds_mac, 1);
    l2_ad.index = pl2_usw_master[lchip]->ad_index_drop;
    CTC_ERROR_RETURN(ctc_spool_static_add(pl2_usw_master[lchip]->ad_spool, &l2_ad))

    CTC_ERROR_RETURN(sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_L2MC,
                                                sys_usw_l2_mc_ftm_cb));

    CTC_ERROR_RETURN(sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_MAC,
                                                sys_usw_l2_fdb_ftm_cb));

    spec_fdb = sys_usw_ftm_get_spec(lchip, CTC_FTM_SPEC_MAC);
    cmd = DRV_IOW(MacLimitSystem_t, MacLimitSystem_profileMaxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &spec_fdb));

    for(fid = pl2_usw_master[lchip]->cfg_max_fid+1;fid <= MCHIP_CAP(SYS_CAP_FID_NUM); fid++)
    {
        _sys_usw_l2_set_fid_property(lchip, fid, CTC_L2_FID_PROP_LEARNING, 0);
    }
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_l2_fdb_wb_restore(lchip));
    }

    hash_entry_num = 1;
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_logicPortIgnoreMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &hash_entry_num));
    /* set chip_capability */
    MCHIP_CAP(SYS_CAP_SPEC_LOGIC_PORT_NUM) = pl2_usw_master[lchip]->cfg_vport_num;
    MCHIP_CAP(SYS_CAP_SPEC_MAX_FID)        = pl2_usw_master[lchip]->cfg_max_fid;

    sys_usw_nh_vxlan_vni_init(lchip);
    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_L2, sys_usw_l2_fdb_wb_sync));
    drv_ser_register_hw_reset_cb(lchip, DRV_SER_HW_RESET_CB_TYPE_FDB, sys_usw_fdb_reset_hw);
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_L2, sys_usw_l2_dump_db));
    CTC_ERROR_RETURN(sys_usw_ftm_register_change_mem_cb(lchip, sys_usw_l2_fdb_change_mem_cb));
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;
}
int32
sys_usw_l2_fdb_init(uint8 lchip, void* l2_fdb_global_cfg)
{
    return _sys_usw_l2_fdb_init(lchip, l2_fdb_global_cfg, 0);
}
#define __DEINIT__
STATIC int32
_sys_usw_l2_fdb_free_node_data(void* node_data, void* user_data)
{
    if (node_data)
    {
        mem_free(node_data);
    }

    return 1;
}

STATIC int32
_sys_usw_l2_free_node_data(void* node_data, void* user_data)
{
    if (node_data)
    {
        mem_free(node_data);
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2_fdb_deinit(uint8 lchip)
{
    uint32 rsv_ad_num = 0;
    LCHIP_CHECK(lchip);
    if (NULL == pl2_usw_master[lchip])
    {
        return CTC_E_NONE;
    }

    L2_DUMP_LOCK;

    sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, 1, pl2_usw_master[lchip]->ad_index_drop);
    rsv_ad_num = SYS_USW_MAX_LINKAGG_GROUP_NUM + SYS_USW_MAX_PORT_NUM_PER_CHIP;
    if (!pl2_usw_master[lchip]->vp_alloc_ad_en)
    {
        rsv_ad_num += pl2_usw_master[lchip]->cfg_vport_num;
    }
    sys_usw_ftm_free_table_offset(lchip, DsMac_t, 1, rsv_ad_num, _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK));
    sys_usw_ftm_free_table_offset(lchip, DsMac_t, 0, pl2_usw_master[lchip]->cfg_max_fid+1, pl2_usw_master[lchip]->dft_entry_base);

    /*free ad data*/
    ctc_spool_free(pl2_usw_master[lchip]->ad_spool);

    /*free fdb hash data*/
    ctc_hash_traverse_remove(pl2_usw_master[lchip]->fdb_hash, (hash_traversal_fn)_sys_usw_l2_fdb_free_node_data, NULL);
    ctc_hash_free(pl2_usw_master[lchip]->fdb_hash);

    if (pl2_usw_master[lchip]->cfg_vport_num)
    {
        /*free vport data*/
        ctc_vector_traverse(pl2_usw_master[lchip]->vport2nh_vec, (vector_traversal_fn)_sys_usw_l2_free_node_data, NULL);
        ctc_vector_release(pl2_usw_master[lchip]->vport2nh_vec);
    }

    /*free fid data*/
    ctc_vector_traverse(pl2_usw_master[lchip]->fid_vec, (vector_traversal_fn)_sys_usw_l2_free_node_data, NULL);
    ctc_vector_release(pl2_usw_master[lchip]->fid_vec);

    /*stp deinit*/
    sys_usw_stp_deinit(lchip);
    L2_DUMP_UNLOCK;
    sal_mutex_destroy(pl2_usw_master[lchip]->l2_dump_mutex);
    sal_mutex_destroy(pl2_usw_master[lchip]->l2_mutex);
    mem_free(pl2_usw_master[lchip]);

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_l2_set_fid_property(uint8 lchip, uint16 fid_id, uint32 fid_prop, uint32 value)
{
    uint32 field_id_0 = 0;
    uint32 field_id   = 0;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 tableid    = DsVsi_t;
    uint8  step       = DsVsi_array_1_bcastDiscard_f - DsVsi_array_0_bcastDiscard_f;
    uint32 temp_value = 0;
    DsMac_m ds_mac;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (fid_prop)
    {
    case CTC_L2_FID_PROP_LEARNING:
        field_id_0 = DsVsi_array_0_vsiLearningDisable_f;
        value      = (value) ? 0 : 1;
        break;

    case CTC_L2_FID_PROP_IGMP_SNOOPING:
        field_id_0 = DsVsi_array_0_igmpSnoopEn_f;
        value      = (value) ? 1 : 0;
        break;

    case CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST:
        field_id_0 = DsVsi_array_0_mcastDiscard_f;
        value      = (value) ? 1 : 0;
        break;

    case CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST:
        field_id_0 = DsVsi_array_0_ucastDiscard_f;
        value      = (value) ? 1 : 0;
        break;
    case CTC_L2_FID_PROP_DROP_BCAST:
        field_id_0 = DsVsi_array_0_bcastDiscard_f;
        value      = (value) ? 1 : 0;
        break;

    case CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU:
        value      = (value) ? 1 : 0;
        cmd = DRV_IOW(DsMac_t, DsMac_unknownMcastException_f);
        break;

    case CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU:
        value      = (value) ? 1 : 0;
        cmd = DRV_IOW(DsMac_t, DsMac_unknownUcastException_f);
        break;

    case CTC_L2_FID_PROP_BCAST_COPY_TO_CPU:
        value      = (value) ? 1 : 0;
        cmd = DRV_IOW(DsMac_t, DsMac_bcastException_f);
        break;

    case CTC_L2_FID_PROP_IGS_STATS_EN:
    case CTC_L2_FID_PROP_EGS_STATS_EN:
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

        break;
    case CTC_L2_FID_PROP_BDING_MCAST_GROUP:
        if(DRV_IS_DUET2(lchip))
        {
            return CTC_E_INVALID_PARAM;
        }
        sal_memset(&ds_mac, 0, sizeof(DsMac_m));
        index = DEFAULT_ENTRY_INDEX(fid_id);
        cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_mac));
        temp_value = value?1:0;
        SetDsMac(V, unknownUcastFwdEn_f, &ds_mac, 0);
        SetDsMac(V, unknownUcastNextHopEn_f, &ds_mac, 0);
        SetDsMac(V, unknownMcastFwdEn_f, &ds_mac, temp_value);
        SetDsMac(V, unknownMcastNextHopEn_f, &ds_mac, 0);
        SetDsMac(V, adDestMap_f, &ds_mac, SYS_ENCODE_MCAST_IPE_DESTMAP(value));
        SetDsMac(V, adNextHopPtr_f, &ds_mac, 0);
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_mac));
        return CTC_E_NONE;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if(cmd)
    {
        index = DEFAULT_ENTRY_INDEX(fid_id);
    }
    else
    {
        field_id = field_id_0 + (fid_id & 0x3) * step;
        cmd      = DRV_IOW(tableid, field_id);
        index    = (fid_id >> 2) & 0xFFF;
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    return CTC_E_NONE;

}
int32
sys_usw_l2_set_fid_property(uint8 lchip, uint16 fid_id, uint32 fid_prop, uint32 value)
{
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    SYS_L2_FID_CHECK(fid_id);
    CTC_ERROR_RETURN(_sys_usw_l2_set_fid_property(lchip, fid_id, fid_prop, value));

    return CTC_E_NONE;
}
int32
sys_usw_l2_get_fid_property(uint8 lchip, uint16 fid_id, uint32 fid_prop, uint32* value)
{
    uint32 field_id_0 = 0;
    uint32 field_id   = 0;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint8  step       = DsVsi_array_1_bcastDiscard_f - DsVsi_array_0_bcastDiscard_f;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_L2_INIT_CHECK();
    SYS_L2_FID_CHECK(fid_id);
    CTC_PTR_VALID_CHECK(value);

    switch (fid_prop)
    {
    case CTC_L2_FID_PROP_LEARNING:
        field_id_0 = DsVsi_array_0_vsiLearningDisable_f;
        break;

    case CTC_L2_FID_PROP_IGMP_SNOOPING:
        field_id_0 = DsVsi_array_0_igmpSnoopEn_f;
        break;

    case CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST:
        field_id_0 = DsVsi_array_0_mcastDiscard_f;
        break;

    case CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST:
        field_id_0 = DsVsi_array_0_ucastDiscard_f;
        break;
    case CTC_L2_FID_PROP_DROP_BCAST:
        field_id_0 = DsVsi_array_0_bcastDiscard_f;
        break;
    case CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU:
        cmd = DRV_IOR(DsMac_t, DsMac_unknownMcastException_f);
        break;

    case CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU:
        cmd = DRV_IOR(DsMac_t, DsMac_unknownUcastException_f);
        break;

    case CTC_L2_FID_PROP_BCAST_COPY_TO_CPU:
        cmd = DRV_IOR(DsMac_t, DsMac_bcastException_f);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if(cmd)
    {
        index = DEFAULT_ENTRY_INDEX(fid_id);
    }
    else
    {
        field_id = field_id_0 + (fid_id & 0x3) * step;
        cmd      = DRV_IOR(DsVsi_t, field_id);
        index    = (fid_id >> 2) & 0xFFF;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, value));
    if (CTC_L2_FID_PROP_LEARNING == fid_prop)
    {
        *value = !(*value);
    }

    return CTC_E_NONE;
}


int32
sys_usw_l2_set_default_entry_features(uint8 lchip, ctc_l2dflt_addr_t* l2dflt_addr)
{
    sys_l2_fid_node_t* fid   = NULL;
    int32            ret     = 0;
    sys_l2_info_t    sys;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2dflt_addr);
    SYS_L2_FID_CHECK(l2dflt_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "fid :%d\n", l2dflt_addr->fid);
    sal_memset(&sys, 0, sizeof(sys_l2_info_t));

    if (CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    if (CTC_FLAG_ISSET(l2dflt_addr->flag, CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP))
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    L2_LOCK;

    fid = _sys_usw_l2_fid_node_lkup(lchip, l2dflt_addr->fid);
    if (NULL == fid)
    {
        ret = CTC_E_NOT_EXIST;
        goto error_0;
    }

    l2dflt_addr->l2mc_grp_id = fid->mc_gid;

    CTC_ERROR_GOTO(_sys_usw_l2_build_sys_info(lchip, L2_TYPE_DF, l2dflt_addr, &sys, 0, 0), ret, error_0);
    sys.ad_index          = DEFAULT_ENTRY_INDEX(l2dflt_addr->fid);

    CTC_ERROR_GOTO(_sys_usw_l2_write_hw(lchip, &sys), ret, error_0);

    /*add hardware success, need change software*/
    fid->flag = l2dflt_addr->flag;

 error_0:
    L2_UNLOCK;
    return ret;
}

int32
sys_usw_l2_set_dsmac_for_linkagg_hw_learning(uint8 lchip, uint32 gport, bool b_add)
{
    uint16          dest_id = 0;
    DsMac_m        ds_mac;
    uint32          cmd         = 0;
    uint32          ds_mac_base = 0;
    sys_l2_info_t   sys;
    uint32 nhid = 0;

    SYS_L2_INIT_CHECK();
    sal_memset(&ds_mac, 0, sizeof(DsMac_m));
    sal_memset(&sys, 0, sizeof(sys));

    dest_id = CTC_MAP_GPORT_TO_LPORT(gport);

    ds_mac_base = _sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_TRUNK);

    if (b_add)
    {
        /*get nexthop id from gport*/
        sys_usw_l2_get_ucast_nh(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nhid);

        _sys_usw_l2_map_nh_to_sys(lchip, nhid, &sys, 0, gport, 0);

        /* write Ad */
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, &sys));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ds_mac_base + dest_id, cmd, &ds_mac));

    }

    /* no need consider delete. actually the dsmac should be set at the initial process.
     * the reason not doing that is because linkagg nexthop is not created yet, back then.
     * Unless nexthop reserved linkagg nexthop, fdb cannot do it at the initial process.
     */

    return CTC_E_NONE;
}

int32
sys_usw_l2_set_dsmac_for_bpe(uint8 lchip, uint32 gport, bool b_add)
{
    uint16          drv_lport = 0;
    DsMac_m        ds_mac;
    uint32          cmd         = 0;
    uint32          ds_mac_base = 0;
    sys_l2_info_t   sys;
    uint32 nhid = 0;

    SYS_L2_INIT_CHECK();
    sal_memset(&sys, 0, sizeof(sys));

    drv_lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    ds_mac_base = (_sys_usw_l2_get_dsmac_base(lchip, SYS_L2_DSMAC_BASE_GPORT));

    if (b_add)
    {
        /*get nexthop id from gport*/
        sys_usw_l2_get_ucast_nh(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nhid);

        _sys_usw_l2_map_nh_to_sys(lchip, nhid, &sys, 0, gport, 0);

        /* write Ad */
        cmd = DRV_IOW(DsMac_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(_sys_usw_l2_encode_dsmac(lchip, &ds_mac, &sys));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ds_mac_base + drv_lport, cmd, &ds_mac));

    }


    return CTC_E_NONE;
}
int32
sys_usw_l2_default_entry_set_mac_auth(uint8 lchip, uint16 fid, bool enable)
{
    uint16 ad_index = 0;
    uint32 cmd = 0;
    uint32 value = (enable ? 1 : 0 );

    SYS_L2_INIT_CHECK();
    ad_index = DEFAULT_ENTRY_INDEX(fid);
    cmd = DRV_IOW(DsMac_t, DsMac_macSaExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &value));

    return CTC_E_NONE;
}

int32
sys_usw_l2_default_entry_get_mac_auth(uint8 lchip, uint16 fid, bool* enable)
{
    uint16 ad_index = 0;
    uint32 cmd = 0;
    uint32  value = 0;

    SYS_L2_INIT_CHECK();
    ad_index = DEFAULT_ENTRY_INDEX(fid);
    cmd = DRV_IOR(DsMac_t, DsMac_macSaExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index, cmd, &value));
    *enable = value ? TRUE : FALSE;

    return CTC_E_NONE;
}
int32
sys_usw_l2_fdb_set_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8 hit)
{
    mac_lookup_result_t rslt;
    uint8 timer = 0;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mac: %s,fid: %d  \n",
                      sys_output_mac(l2_addr->mac),
                      l2_addr->fid);
    sal_memset(&rslt, 0, sizeof(mac_lookup_result_t));

    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2_addr->mac, l2_addr->fid, &rslt));
    if (!rslt.hit)
    {
        return CTC_E_NOT_EXIST;
    }
    hit = hit ? 1:0;
    CTC_ERROR_RETURN(sys_usw_aging_get_aging_timer(lchip, 1, rslt.key_index, &timer));
    CTC_ERROR_RETURN(sys_usw_aging_set_aging_status(lchip, 1, rslt.key_index, timer, hit));

    return CTC_E_NONE;
}

int32
sys_usw_l2_fdb_get_entry_hit(uint8 lchip, ctc_l2_addr_t* l2_addr, uint8* hit)
{
    mac_lookup_result_t rslt;

    SYS_L2_INIT_CHECK();
    CTC_PTR_VALID_CHECK(l2_addr);
    SYS_L2UC_FID_CHECK(l2_addr->fid);
    CTC_PTR_VALID_CHECK(hit);

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&rslt, 0, sizeof(mac_lookup_result_t));
    CTC_ERROR_RETURN(_sys_usw_l2_acc_lookup_mac_fid(lchip, l2_addr->mac, l2_addr->fid, &rslt));
    if (!rslt.hit)
    {
        return CTC_E_NOT_EXIST;
    }
    CTC_ERROR_RETURN(sys_usw_aging_get_aging_status(lchip, 1, rslt.key_index, hit));

    return CTC_E_NONE;
}
int32
sys_usw_l2_dump_fdb_entry_detail(uint8 lchip, ctc_l2_fdb_query_t* pq,
        uint16 entry_cnt, uint8 need_check, void* p_file)
{
    sys_l2_detail_t* detail_info = NULL;
    uint16 index = 0;
    uint16 pending = 0;
    ctc_l2_fdb_query_rst_t query_rst;
    uint32 total_count = 0;
    sal_file_t fp = NULL;
    char* char_buffer = NULL;
    uint8 hit = 0;
    int ret = CTC_E_NONE;

    SYS_L2_INIT_CHECK();
    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(pq);

    sal_memset(&query_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));

    if (!(pl2_usw_master[lchip]->cfg_hw_learn) &&(pq->query_hw)&&need_check)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    if (p_file)
    {
        fp = (sal_file_t)p_file;
        char_buffer = (char*)mem_malloc(MEM_FDB_MODULE, 128);
        if (NULL == char_buffer)
        {
            SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            ret = CTC_E_NO_MEMORY;
            goto end;
        }

        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Please read entry-file after seeing \"FDB entry have been writen!\"\n");
    }

    /* alloc memory for detail struct */
    detail_info = (sys_l2_detail_t*)mem_malloc(MEM_FDB_MODULE, sizeof(sys_l2_detail_t) * entry_cnt);
    if (NULL == detail_info)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto end;
    }

    query_rst.buffer_len = sizeof(ctc_l2_addr_t) * entry_cnt;
    query_rst.buffer = (ctc_l2_addr_t*)mem_malloc(MEM_FDB_MODULE, query_rst.buffer_len);
    if (NULL == query_rst.buffer)
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto end;
    }

    sal_memset(query_rst.buffer, 0, query_rst.buffer_len);

    if (p_file)
    {
        sal_fprintf(fp, "%-10s: DsFibHost0MacHashKey \n", "Key Table");
        sal_fprintf(fp, "%-10s: DsMac \n", "Ad Table");
        sal_fprintf(fp, "\n");

        sal_fprintf(fp, "%8s  %8s  %8s  %11s    %4s  %6s  %6s  %4s\n", "KEY-INDEX", "AD-INDEX", "MAC", "FID", "GPORT", "Static", "Pending", "Hit");
        sal_fprintf(fp, "------------------------------------------------------------------------------\n");
    }
    else
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s: DsFibHost0MacHashKey \n", "Key Table");
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s: DsMac \n", "Ad Table");
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%8s  %8s  %8s  %11s    %4s  %6s  %6s  %4s\n", "KEY-INDEX", "AD-INDEX", "MAC", "FID", "GPORT", "Static", "Pending", "Hit");
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------\n");
    }

    do
    {
        pq->count = 0;
        sal_memset(detail_info, 0, sizeof(sys_l2_detail_t) * entry_cnt);
        query_rst.start_index = query_rst.next_query_index;

        CTC_ERROR_GOTO(_sys_usw_l2_get_fdb_entry_detail(lchip, pq, &query_rst, detail_info), ret, end);
        for (index = 0; index < pq->count; index++)
        {
            hit = 0;
            if (query_rst.buffer[index].fid != DONTCARE_FID)
            {
                sys_usw_aging_get_aging_status(lchip, SYS_AGING_DOMAIN_MAC_HASH, detail_info[index].key_index, &hit);
            }

            if (p_file)
            {
                sal_sprintf(char_buffer, "0x%-8x", detail_info[index].key_index);
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "0x%-8x", detail_info[index].ad_index);
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%.4x.%.4x.%.4x%4s ", sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                            " ");

                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%.4d  ", query_rst.buffer[index].fid);
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "0x%.4x  ", query_rst.buffer[index].gport);
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%3s     ",
                                          (CTC_FLAG_ISSET(detail_info[index].flag, CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No"));
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%5s     ", (detail_info[index].pending) ? "Y" : "N");
                sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%3s     \n", (hit) ? "Y" : "N");

                sal_fprintf(fp, "%s", char_buffer);
            }
            else
            {
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x", detail_info[index].key_index);
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x", detail_info[index].ad_index);
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%.4x.%.4x.%.4x%4s ", sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                            " ");

                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%.4d  ", query_rst.buffer[index].fid);

                if (SYS_RSV_PORT_OAM_CPU_ID == query_rst.buffer[index].gport && !query_rst.buffer[index].is_logic_port)
                {
                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %s   ", "CPU");
                }
                else
                {
                    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%.4x  ", query_rst.buffer[index].gport);
                }
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%3s     ", CTC_FLAG_ISSET(detail_info[index].flag, CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No");
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%5s     ", (detail_info[index].pending) ? "Y" : "N");
                SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%3s     \n", (hit) ? "Y" : "N");
            }

            if (detail_info[index].pending)
            {
                pending ++;
            }

            total_count++;
            sal_task_sleep(100);

            sal_memset(&detail_info[index], 0, sizeof(sys_l2_detail_t));
        }

    }
    while (query_rst.is_end == 0);

    if (p_file)
    {
        sal_fprintf(fp,"------------------------------------------------------------------------------\n");
        sal_fprintf(fp,"Total Entry Num: %d\n", total_count);
        sal_fprintf(fp,"Pending Entry Num: %d\n", pending);
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "FDB entry have been writen\n");
    }
    else
    {
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------\n");
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Total Entry Num: %d\n", total_count);
        SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Pending Entry Num: %d\n", pending);
    }
end:
    if (detail_info)
    {
        mem_free(detail_info);
    }
    if (char_buffer)
    {
        mem_free(char_buffer);
    }
    if(query_rst.buffer)
    {
        mem_free(query_rst.buffer);
    }
    return ret;
}

#define __WARM_BOOT__
int32
_sys_usw_l2_wb_mapping_fid(sys_wb_l2_fid_node_t *p_wb_fid, sys_l2_fid_node_t *p_fid, uint8 sync)
{
    if (sync)
    {
        p_wb_fid->fid = p_fid->fid;
        p_wb_fid->flag = p_fid->flag;
        p_wb_fid->mc_gid = p_fid->mc_gid;
        p_wb_fid->share_grp_en = p_fid->share_grp_en;
        p_wb_fid->cid = p_fid->cid;
    }
    else
    {
        p_fid->fid = p_wb_fid->fid;
        p_fid->flag = p_wb_fid->flag;
        p_fid->mc_gid = p_wb_fid->mc_gid;
        p_fid->share_grp_en = p_wb_fid->share_grp_en;
        p_fid->cid = p_wb_fid->cid;
    }

    return CTC_E_NONE;
}
int32
_sys_usw_l2_wb_sync_fid_func(void* array_data, uint32 index, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_l2_fid_node_t *p_fid = (sys_l2_fid_node_t *)array_data;
    sys_wb_l2_fid_node_t  *p_wb_fid;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    uint8 lchip;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(traversal_data->value2 && p_fid->fid >= 4096)
    {
        return CTC_E_NONE;
    }
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);
    p_wb_fid = (sys_wb_l2_fid_node_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_fid, 0, sizeof(sys_wb_l2_fid_node_t));

    CTC_ERROR_RETURN(_sys_usw_l2_wb_mapping_fid(p_wb_fid, p_fid, 1));
    p_wb_fid->recover_hw_en = traversal_data->value2;

    if(traversal_data->value2)
    {
        ctc_nh_info_t nh_info;
        uint32 nh_id;
        uint16 loop;
        uint16 loop1;
        ctc_mcast_nh_param_member_t* p_member;

        lchip = traversal_data->value1;
        sal_memset(&nh_info, 0, sizeof(nh_info));
        nh_info.buffer = mem_malloc(MEM_FDB_MODULE, 100*sizeof(ctc_mcast_nh_param_member_t));
        if(NULL == nh_info.buffer)
        {
            return CTC_E_NO_MEMORY;
        }
        nh_info.buffer_len = 100*sizeof(ctc_mcast_nh_param_member_t);

        CTC_ERROR_RETURN(sys_usw_nh_get_mcast_nh(lchip, p_fid->mc_gid, &nh_id));
        do{
            CTC_ERROR_RETURN(sys_usw_nh_get_mcast_member(lchip, nh_id, &nh_info));
            for(loop=0; loop < nh_info.valid_number; loop++)
            {
                p_member = nh_info.buffer+loop;
                if(p_member->member_type != CTC_NH_PARAM_MEM_BRGMC_LOCAL)
                {
                    continue;
                }

                if(p_member->gchip_id == 0x1F && p_member->port_bmp_en)
                {
                    for(loop1=0; loop1 < CTC_PORT_BITMAP_IN_WORD; loop1++)
                    {
                        p_wb_fid->linkagg_bmp[loop1] |= p_member->port_bmp[loop1];
                    }
                }
                else if(p_member->port_bmp_en)
                {
                    for(loop1=0; loop1 < CTC_PORT_BITMAP_IN_WORD; loop1++)
                    {
                        p_wb_fid->port_bmp[loop1] |= p_member->port_bmp[loop1];
                    }
                }
             }

            nh_info.start_index = nh_info.next_query_index;
        }while(!nh_info.is_end);
    }
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;

}
int32
_sys_usw_l2_wb_sync_vport2nh_func(void* array_data, uint32 index, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_l2_vport_2_nhid_t *p_v2nhid = (sys_l2_vport_2_nhid_t *)array_data;
    sys_wb_l2_vport_2_nhid_t  *p_wb_v2nhid;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_v2nhid = (sys_wb_l2_vport_2_nhid_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_v2nhid, 0, sizeof(sys_wb_l2_vport_2_nhid_t));

    p_wb_v2nhid->vport = index;
    p_wb_v2nhid->nhid = p_v2nhid->nhid;
    p_wb_v2nhid->ad_idx = p_v2nhid->ad_idx;
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32 _sys_usw_l2_wb_mapping_fdb(uint8 lchip, sys_wb_l2_fdb_t* p_wb_fdb, sys_l2_node_t* p_l2_node, uint8 is_sync)
{
    uint32 cmd = DRV_IOR(DsMac_t, DRV_ENTRY_FLAG);
    sys_l2_ad_t l2_ad;
    sys_l2_ad_t* p_l2_ad_get = NULL;
    if(is_sync)
    {
        sal_memcpy(&p_wb_fdb->mac, &p_l2_node->mac, sizeof(mac_addr_t));
        p_wb_fdb->fid = p_l2_node->fid;
        p_wb_fdb->flag = p_l2_node->flag;
        p_wb_fdb->ad_index = p_l2_node->adptr->index;
        p_wb_fdb->bind_nh = p_l2_node->bind_nh;
        p_wb_fdb->nh_id = p_l2_node->nh_id;
        p_wb_fdb->rsv_ad = p_l2_node->rsv_ad;
    }
    else
    {
        sys_nh_info_dsnh_t nh_info;

        sal_memset(&l2_ad, 0, sizeof(l2_ad));
        sal_memcpy(&p_l2_node->mac, &p_wb_fdb->mac, sizeof(mac_addr_t));

        p_l2_node->fid = p_wb_fdb->fid;
        p_l2_node->flag = p_wb_fdb->flag;
        p_l2_node->bind_nh = p_wb_fdb->bind_nh;
        p_l2_node->nh_id = p_wb_fdb->nh_id;
        p_l2_node->rsv_ad = p_wb_fdb->rsv_ad;

        sal_memset(&nh_info, 0, sizeof(nh_info));
        if (p_l2_node->nh_id)
        {
            sys_usw_nh_get_nhinfo(lchip, p_l2_node->nh_id, &nh_info, 0);
        }
        p_l2_node->ecmp_valid = nh_info.ecmp_valid || nh_info.is_ecmp_intf;
        p_l2_node->aps_valid = nh_info.aps_en;

        l2_ad.index = p_wb_fdb->ad_index;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, l2_ad.index, cmd, &(l2_ad.ds_mac)));
        if (p_l2_node->rsv_ad)
        {
            MALLOC_ZERO(MEM_FDB_MODULE, p_l2_node->adptr, sizeof(sys_l2_ad_t));
            if(NULL == p_l2_node->adptr)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memcpy(&p_l2_node->adptr->ds_mac, &l2_ad.ds_mac, sizeof(DsMac_m));
        }
        else
        {
            CTC_ERROR_RETURN(ctc_spool_add(pl2_usw_master[lchip]->ad_spool, &l2_ad, NULL, &p_l2_ad_get));
            p_l2_node->adptr = p_l2_ad_get;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_l2_wb_sync_fdb_func(sys_l2_node_t *p_node, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_l2_fdb_t  *p_wb_fdb;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(traversal_data->data);
    uint8 lchip = (uint8)traversal_data->value1;

    SYS_FDB_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_fdb = (sys_wb_l2_fdb_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_fdb, 0, sizeof(sys_wb_l2_fdb_t));

    CTC_ERROR_RETURN(_sys_usw_l2_wb_mapping_fdb(lchip, p_wb_fdb, p_node, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}


int32
sys_usw_l2_fdb_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_l2_master_t  *p_wb_l2_master;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);

    wb_data.buffer = mem_malloc(MEM_FDB_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(&user_data, 0, sizeof(user_data));
    if(3 ==  work_status)
    {
        /*syncup fdb fid*/
        CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l2_fid_node_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_FID_NODE);
        user_data.data = &wb_data;
        user_data.value1 = lchip;
        user_data.value2 = 1;
        CTC_ERROR_GOTO(ctc_vector_traverse2(pl2_usw_master[lchip]->fid_vec, 0, _sys_usw_l2_wb_sync_fid_func, (void *)&user_data), ret, done);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
        goto done;
    }

    /*syncup fdb_matser*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l2_master_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_MASTER);

    p_wb_l2_master = (sys_wb_l2_master_t *)wb_data.buffer;

    p_wb_l2_master->lchip = lchip;
    p_wb_l2_master->l2mc_count = pl2_usw_master[lchip]->l2mc_count;
    p_wb_l2_master->def_count = pl2_usw_master[lchip]->def_count;
    p_wb_l2_master->ad_index_drop = pl2_usw_master[lchip]->ad_index_drop;
    p_wb_l2_master->cfg_vport_num = pl2_usw_master[lchip]->cfg_vport_num;
    p_wb_l2_master->cfg_hw_learn = pl2_usw_master[lchip]->cfg_hw_learn;
    p_wb_l2_master->static_fdb_limit = pl2_usw_master[lchip]->static_fdb_limit;
    p_wb_l2_master->cfg_max_fid = pl2_usw_master[lchip]->cfg_max_fid;
    p_wb_l2_master->vp_alloc_ad_en = pl2_usw_master[lchip]->vp_alloc_ad_en;
    sal_memcpy(p_wb_l2_master->rchip_ad_rsv, pl2_usw_master[lchip]->rchip_ad_rsv, SYS_USW_MAX_GCHIP_CHIP_ID*sizeof(uint16));
    p_wb_l2_master->version = SYS_WB_VERSION_L2;
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup fdb fid*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l2_fid_node_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_FID_NODE);
    user_data.data = &wb_data;
    user_data.value1 = lchip;

    CTC_ERROR_GOTO(ctc_vector_traverse2(pl2_usw_master[lchip]->fid_vec, 0, _sys_usw_l2_wb_sync_fid_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*sync nhid of logic port*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l2_vport_2_nhid_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_VPORT_2_NHID);
    user_data.data = &wb_data;

    CTC_ERROR_GOTO(ctc_vector_traverse2(pl2_usw_master[lchip]->vport2nh_vec, 0, _sys_usw_l2_wb_sync_vport2nh_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*syncup fdb*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_l2_fdb_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_FDB);
    user_data.data = &wb_data;
    user_data.value1 = lchip;

    CTC_ERROR_GOTO(ctc_hash_traverse(pl2_usw_master[lchip]->fdb_hash, (hash_traversal_fn) _sys_usw_l2_wb_sync_fdb_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_l2_fdb_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_l2_master_t  wb_l2_master;
    sys_wb_l2_master_t  *p_wb_l2_master = &wb_l2_master;
    sys_l2_node_t *p_l2_node = NULL;
    sys_wb_l2_fdb_t  wb_fdb;
    sys_wb_l2_fdb_t  *p_wb_fdb = &wb_fdb;
    sys_l2_fid_node_t *p_fid;
    sys_wb_l2_fid_node_t  wb_fid;
    sys_wb_l2_fid_node_t *p_wb_fid = &wb_fid;
    sys_l2_vport_2_nhid_t *p_v2nh;
    sys_wb_l2_vport_2_nhid_t  wb_v2nh;
    sys_wb_l2_vport_2_nhid_t *p_wb_v2nh = &wb_v2nh;
    uint32  loop = 0;
    uint32  cmd = 0;
    uint32  value = 0;
    DsFibMacBlackHoleHashKey_m DsFibMacBlackHoleHashKey;
    sys_nh_info_dsnh_t nh_info;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_FDB_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /* set default value*/
    sal_memset(p_wb_l2_master, 0, sizeof(sys_wb_l2_master_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_l2_master_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore fdb_master*/
    if (wb_query.valid_cnt == 1)
    {
        sal_memcpy((uint8*)p_wb_l2_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);

        if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_L2, p_wb_l2_master->version))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto done;
        }

        if ( (pl2_usw_master[lchip]->cfg_vport_num > p_wb_l2_master->cfg_vport_num)
            || (pl2_usw_master[lchip]->cfg_hw_learn != p_wb_l2_master->cfg_hw_learn)
        || (pl2_usw_master[lchip]->static_fdb_limit != p_wb_l2_master->static_fdb_limit)
        || (pl2_usw_master[lchip]->cfg_max_fid > p_wb_l2_master->cfg_max_fid)
        || (pl2_usw_master[lchip]->vp_alloc_ad_en != p_wb_l2_master->vp_alloc_ad_en))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto done;
        }
        pl2_usw_master[lchip]->l2mc_count = p_wb_l2_master->l2mc_count;
        pl2_usw_master[lchip]->def_count = p_wb_l2_master->def_count;
        pl2_usw_master[lchip]->ad_index_drop = p_wb_l2_master->ad_index_drop;
        sal_memcpy(pl2_usw_master[lchip]->rchip_ad_rsv, p_wb_l2_master->rchip_ad_rsv, SYS_USW_MAX_GCHIP_CHIP_ID*sizeof(uint16));

        cmd = DRV_IOR(DsFibMacBlackHoleHashKey_t, DRV_ENTRY_FLAG);
        for (loop = 0; loop < SYS_L2_MAX_BLACK_HOLE_ENTRY; loop++)
        {
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &DsFibMacBlackHoleHashKey), ret, done);
            value = GetDsFibMacBlackHoleHashKey(V, valid_f, &DsFibMacBlackHoleHashKey);
            if (value)
            {
                CTC_BMP_SET(pl2_usw_master[lchip]->mac_fdb_bmp, loop);
            }
        }
    }




    /*restore fdb */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_l2_fdb_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_FDB);
    /* set default value*/
    sal_memset(p_wb_fdb, 0, sizeof(sys_wb_l2_fdb_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_fdb, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_l2_node = mem_malloc(MEM_FDB_MODULE, sizeof(sys_l2_node_t));
        if (NULL == p_l2_node)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_l2_node, 0, sizeof(sys_l2_node_t));

        ret = _sys_usw_l2_wb_mapping_fdb(lchip, p_wb_fdb, p_l2_node, 0);
        if (ret)
        {
            continue;
        }

        if (p_l2_node->bind_nh)
        {
            _sys_usw_l2_bind_nexthop(lchip, p_l2_node, 1, 0);
        }
        /*add to soft table*/
        ctc_hash_insert(pl2_usw_master[lchip]->fdb_hash, p_l2_node);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

     /*restore fdb fid*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_l2_fid_node_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_FID_NODE);
    /* set default value*/
    sal_memset(p_wb_fid, 0, sizeof(sys_wb_l2_fid_node_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_fid, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        if(p_wb_fid->recover_hw_en)
        {
            ctc_l2dflt_addr_t l2dflt;
            uint16            loop;
            uint8             gchip;
            sal_memset(&l2dflt, 0, sizeof(l2dflt));

            l2dflt.fid = p_wb_fid->fid;
            l2dflt.cid = p_wb_fid->cid;
            l2dflt.flag = p_wb_fid->flag;
            l2dflt.l2mc_grp_id = p_wb_fid->mc_gid;
            l2dflt.share_grp_en = p_wb_fid->share_grp_en;
            CTC_ERROR_GOTO(sys_usw_l2_add_default_entry(lchip, &l2dflt), ret, done);

            CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, done);
            for(loop=0; loop < MAX_PORT_NUM_PER_CHIP; loop++)
            {
                if(CTC_BMP_ISSET(p_wb_fid->port_bmp, loop))
                {
                    l2dflt.member.mem_port = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
                    CTC_ERROR_GOTO(sys_usw_l2_add_port_to_default_entry(lchip, &l2dflt), ret, done);
                }
            }
            for(loop=0; loop < MAX_PORT_NUM_PER_CHIP; loop++)
            {
                if(CTC_BMP_ISSET(p_wb_fid->linkagg_bmp, loop))
                {
                    l2dflt.member.mem_port = CTC_MAP_LPORT_TO_GPORT(0x1F, loop);
                    CTC_ERROR_GOTO(sys_usw_l2_add_port_to_default_entry(lchip, &l2dflt), ret, done);
                }
            }
            continue;
        }
        p_fid = mem_malloc(MEM_FDB_MODULE, sizeof(sys_l2_fid_node_t));
        if (NULL == p_fid)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_fid, 0, sizeof(sys_l2_fid_node_t));

        ret = _sys_usw_l2_wb_mapping_fid(p_wb_fid, p_fid, 0);
        if (ret)
        {
            continue;
        }

        /*add to soft table*/
        ctc_vector_add(pl2_usw_master[lchip]->fid_vec, p_fid->fid, p_fid);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore nhid of logic port*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_l2_vport_2_nhid_t, CTC_FEATURE_L2, SYS_WB_APPID_L2_SUBID_VPORT_2_NHID);
    /* set default value*/
    sal_memset(p_wb_v2nh, 0, sizeof(sys_wb_l2_vport_2_nhid_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_v2nh, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_v2nh = mem_malloc(MEM_FDB_MODULE, sizeof(sys_l2_vport_2_nhid_t));
        if (NULL == p_v2nh)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_v2nh, 0, sizeof(sys_l2_vport_2_nhid_t));

        p_v2nh->ad_idx = p_wb_v2nh->ad_idx;
        p_v2nh->nhid = p_wb_v2nh->nhid;

        /*recover nh bind */
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
        sys_usw_nh_get_nhinfo(lchip, p_v2nh->nhid, &nh_info, 0);
        if (!nh_info.bind_data && !nh_info.dsfwd_valid)
        {
            _sys_usw_l2_bind_nexthop(lchip, p_v2nh, 1, 1);
        }

        /*add to soft table*/
        ctc_vector_add(pl2_usw_master[lchip]->vport2nh_vec, p_wb_v2nh->vport, p_v2nh);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}
STATIC int32
_sys_usw_l2_dump_spool_db(void *node, void* user_date)
{
    sys_traverse_t* p_traverse = (sys_traverse_t*)user_date;
    uint32*   p_spool_cnt = (uint32*)p_traverse->data1;
    sal_file_t p_f = (sal_file_t)p_traverse->data;
    ctc_spool_node_t* p_node = (ctc_spool_node_t*)node;
    sys_l2_ad_t* l2_ad = (sys_l2_ad_t*)p_node->data;

    if(0xFFFFFFFF == p_node->ref_cnt)
    {
        SYS_DUMP_DB_LOG(p_f, "%-20u %-20u %-20s\n", *p_spool_cnt, l2_ad->index, "Static");
    }
    else
    {
        SYS_DUMP_DB_LOG(p_f, "%-20u %-20u %-20u\n", *p_spool_cnt, l2_ad->index, p_node->ref_cnt);
    }
    (*p_spool_cnt)++;
    return CTC_E_NONE;
}

int32
sys_usw_l2_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint32 loop = 0;
    uint8  first_flag = 0;
    sys_traverse_t  param;
    uint32  spool_cnt = 1;

    SYS_L2_INIT_CHECK();
    L2_LOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# L2");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","L2 mcast count", pl2_usw_master[lchip]->l2mc_count);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Default entry count", pl2_usw_master[lchip]->def_count);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Ad index for discard", pl2_usw_master[lchip]->ad_index_drop);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Default entry base", pl2_usw_master[lchip]->dft_entry_base);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Logic port number for learning", pl2_usw_master[lchip]->cfg_vport_num);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Hardware learning enable", pl2_usw_master[lchip]->cfg_hw_learn);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Mac limit include static fdb", pl2_usw_master[lchip]->static_fdb_limit);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Max fid value", pl2_usw_master[lchip]->cfg_max_fid);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Enable trie sort", pl2_usw_master[lchip]->trie_sort_en);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Reserve ad for logic port", !pl2_usw_master[lchip]->vp_alloc_ad_en);

    SYS_DUMP_DB_LOG(p_f, "%-36s:\n","Bitmap for mac based fdb");
    for(loop=0; loop < SYS_L2_MAX_BLACK_HOLE_ENTRY/32; loop++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-32s%-4u: 0x%0x\n","", loop, pl2_usw_master[lchip]->mac_fdb_bmp[loop]);
    }

    for(loop=0; loop < SYS_USW_MAX_GCHIP_CHIP_ID; loop++)
    {
        if(0 == pl2_usw_master[lchip]->rchip_ad_rsv[loop])
        {
            continue;
        }

        if(0 == first_flag)
        {
            SYS_DUMP_DB_LOG(p_f, "%-36s:\n","Reserved ad base for remote chip");
            first_flag = 0;

        }
        SYS_DUMP_DB_LOG(p_f, "%-36sgchip%-4u : %-10u","", loop, pl2_usw_master[lchip]->rchip_ad_rsv[loop]);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "\n");

    sal_memset(&param, 0, sizeof(param));
    param.data = (void*)p_f;
    param.data1 = (void*)&spool_cnt;

    SYS_DUMP_DB_LOG(p_f, "Spool type: %-15s\n","L2 DsMac Spool");
    SYS_DUMP_DB_LOG(p_f, "%-20s: DsMac \n", "Key table");
    SYS_DUMP_DB_LOG(p_f, "%-20s %-20s %-20s\n", "Node","Index", "Ref count");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------");
    ctc_spool_traverse(pl2_usw_master[lchip]->ad_spool, (spool_traversal_fn)_sys_usw_l2_dump_spool_db , (void*)(&param));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "----------------------------------------------------");

    SYS_DUMP_DB_LOG(p_f, "\n");
    L2_UNLOCK;
    sys_usw_learning_aging_dump_db(lchip, p_f, p_dump_param);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return CTC_E_NONE;
}

int32 sys_usw_l2_fdb_change_mem_cb(uint8 lchip, void*user_data)
{
    int32 ret = 0;
    ctc_l2_flush_fdb_t flush;
    sal_memset(&flush, 0, sizeof(flush));

    flush.flush_flag = CTC_L2_FDB_ENTRY_PENDING;
    flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    ret = sys_usw_l2_flush_fdb(lchip, &flush);

    flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;
    ret = ret ? ret : sys_usw_l2_flush_fdb(lchip, &flush);
    return ret;
}
