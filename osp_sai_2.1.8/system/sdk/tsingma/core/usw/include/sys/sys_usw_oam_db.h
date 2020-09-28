#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_db.h

 @date 2010-1-19

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_DBD_H
#define _SYS_USW_OAM_DBD_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include "sal.h"
#include "ctc_hash.h"
#include "ctc_linklist.h"
#include "ctc_vector.h"
#include "ctc_interrupt.h"
#include "ctc_register.h"
#include "sys_usw_chip.h"
#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

#define SYS_OAM_MIN_CCM_INTERVAL 1
#define SYS_OAM_MAX_CCM_INTERVAL 7

#define SYS_OAM_MIN_BFD_INTERVAL 1
#define SYS_OAM_MAX_BFD_INTERVAL 1023

#define SYS_OAM_MIN_BFD_UDP_PORT 49152
#define SYS_OAM_MAX_BFD_UDP_PORT 65535

#define SYS_OAM_MIN_BFD_DETECT_MULT 1
#define SYS_OAM_MAX_BFD_DETECT_MULT 15

#define SYS_OAM_MIN_MEP_ID 1

#define SYS_OAM_MIN_MEP_INDEX 2

#define SYS_OAM_BFD_INVALID_IPV6_NUM   0xFF

#define CTC_USW_OAM_DEFECT_NUM 20


/***************************************************************
*
*           OAM FUNCTION DB
*
*****************************************************************/
#define SYS_OAM_MAX_MAID_LEN 48

enum sys_oam_module_e
{
    SYS_OAM_CHAN,
    SYS_OAM_MAID,
    SYS_OAM_LMEP,
    SYS_OAM_RMEP,
    SYS_OAM_MIP,
    SYS_OAM_GLOB,
    SYS_OAM_STATS,
    SYS_OAM_MODULE_MAX
};
typedef enum sys_oam_module_e sys_oam_module_t;

enum sys_oam_op_e
{
    SYS_OAM_ADD,
    SYS_OAM_DEL,
    SYS_OAM_UPDATE,
    SYS_OAM_CONFIG,
    SYS_OAM_GET_INFO,
    SYS_OAM_OP_MAX
};
typedef enum sys_oam_op_e sys_oam_op_t;

enum sys_oam_check_type_e
{
    SYS_OAM_CHECK_MAID,
    SYS_OAM_CHECK_LMEP,
    SYS_OAM_CHECK_RMEP,
    SYS_OAM_CHECK_MEP_UPDATE,
    SYS_OAM_CHECK_MIP,
    SYS_OAM_CHECK_MAX
};
typedef enum sys_oam_check_type_e sys_oam_check_type_t;


enum sys_oam_key_alloc_e
{
    SYS_OAM_KEY_HASH,
    SYS_OAM_KEY_LTCAM,
    SYS_OAM_KEY_MAX
};
typedef enum sys_oam_key_alloc_e sys_oam_key_alloc_t;

enum sys_oam_tp_check_type_e
{
    SYS_OAM_TP_NONE_TYPE,
    SYS_OAM_TP_MEP_TYPE,
    SYS_OAM_TP_MIP_TYPE,
    SYS_OAM_TP_OAM_DISCARD_TYPE,
    SYS_OAM_TP_TO_CPU_TYPE,
    SYS_OAM_TP_DATA_DISCARD_TYPE
};
typedef enum sys_oam_tp_check_type_e sys_oam_tp_check_type_t;

enum sys_oam_exception_type_e
{
    SYS_OAM_EXCP_NONE                   = 0,
    SYS_OAM_EXCP_ETH_INVALID            = 1,
    SYS_OAM_EXCP_ETH_LB                 = 2,
    SYS_OAM_EXCP_ETH_LT                 = 3,
    SYS_OAM_EXCP_ETH_LM                 = 4,
    SYS_OAM_EXCP_ETH_DM                 = 5,
    SYS_OAM_EXCP_ETH_TST                = 6,
    SYS_OAM_EXCP_ETH_APS                = 7,
    SYS_OAM_EXCP_ETH_SCC                = 8,
    SYS_OAM_EXCP_ETH_MCC                = 9,
    SYS_OAM_EXCP_ETH_CSF                = 10,
    SYS_OAM_EXCP_ETH_BIG_CCM            = 11,
    SYS_OAM_EXCP_ETH_LEARN_CCM          = 12,
    SYS_OAM_EXCP_ETH_DEFECT             = 13,
    SYS_OAM_EXCP_PBX_OAM                = 14,
    SYS_OAM_EXCP_ETH_HIGH_VER           = 15,
    SYS_OAM_EXCP_ETH_TLV                = 16,
    SYS_OAM_EXCP_ETH_OTHER              = 17,
    SYS_OAM_EXCP_BFD_INVALID            = 18,
    SYS_OAM_EXCP_BFD_LEARN              = 19,
    SYS_OAM_EXCP_BIG_BFD                = 20,
    SYS_OAM_EXCP_BFD_TIMER_NEG          = 21,
    SYS_OAM_EXCP_TP_SCC                 = 22,
    SYS_OAM_EXCP_TP_MCC                 = 23,
    SYS_OAM_EXCP_TP_CSF                 = 24,
    SYS_OAM_EXCP_TP_LB                  = 25,
    SYS_OAM_EXCP_TP_DLM                 = 26,
    SYS_OAM_EXCP_TP_DM                  = 27,
    SYS_OAM_EXCP_TP_FM                  = 28,
    SYS_OAM_EXCP_BFD_OTHER              = 29,
    SYS_OAM_EXCP_BFD_DISC_MISMATCH      = 30,
    SYS_OAM_EXCP_ETH_SM                 = 31,
    SYS_OAM_EXCP_TP_CV                  = 32,
    SYS_OAM_EXCP_TWAMP                  = 33,
    SYS_OAM_EXCP_ETH_SC                 = 34,
    SYS_OAM_EXCP_ETH_LL                 = 35,
    SYS_OAM_EXCP_MAC_FAIL               = 36
};
typedef enum sys_oam_exception_type_e sys_oam_exception_type_t;

enum sys_oam_tbl_type_e
{
    SYS_OAM_TBL_MEP,
    SYS_OAM_TBL_MA,
    SYS_OAM_TBL_MANAME,
    SYS_OAM_TBL_LM,
    SYS_OAM_TBL_LOOKUP_KEY,
    SYS_OAM_TBL_BFDV6ADDR,
    SYS_OAM_TBL_MAX
};
typedef enum sys_oam_tbl_type_e sys_oam_tbl_type_t;

enum sys_oam_session_type_e
{
    SYS_OAM_SESSION_Y1731_ETH_P2P,
    SYS_OAM_SESSION_Y1731_ETH_P2MP,
    SYS_OAM_SESSION_Y1731_TP_SECTION,
    SYS_OAM_SESSION_Y1731_TP_MPLS,
    SYS_OAM_SESSION_BFD_IPv4,
    SYS_OAM_SESSION_BFD_IPv6,
    SYS_OAM_SESSION_BFD_MPLS,
    SYS_OAM_SESSION_BFD_TP_SECTION,
    SYS_OAM_SESSION_BFD_TP_MPLS,
    SYS_OAM_SESSION_BFD_MICRO,
    SYS_OAM_SESSION_MEP_ON_CPU,
    SYS_OAM_SESSION_MAX
};
typedef enum sys_oam_session_type_e sys_oam_session_type_t;

enum sys_oam_opf_type_e
{
    SYS_OAM_OPF_TYPE_MEP_LMEP,
    SYS_OAM_OPF_TYPE_MA,
    SYS_OAM_OPF_TYPE_MA_NAME,
    SYS_OAM_OPF_TYPE_LM_PROFILE,
    SYS_OAM_OPF_TYPE_LM,
    SYS_OAM_OPF_TYPE_MAX
};
typedef enum sys_oam_opf_type_e sys_oam_opf_type_t;

typedef int32 (* SYS_OAM_CALL_FUNC_P)(uint8 lchip, void* p_para);

typedef int32 (* SYS_OAM_CHECK_CALL_FUNC_P)(uint8 lchip, void* p_para, bool is_add);

typedef int32 (* SYS_OAM_DEFECT_PROCESS_FUNC)(uint8 lchip, void* p_data);

struct sys_oam_master_s
{
    SYS_OAM_CALL_FUNC_P* p_fun_table;

    SYS_OAM_CHECK_CALL_FUNC_P *p_check_fun_table;

    ctc_hash_t*  maid_hash;
    ctc_hash_t*  chan_hash;
    ctc_hash_t*  oamid_hash;

    ctc_vector_t* mep_vec;
    sal_mutex_t*  oam_mutex;

    uint8  maid_len_format;
    uint8  mep_index_alloc_by_sdk;
    uint8  tp_section_oam_based_l3if;
    uint8  oam_opf_type;

    uint32 oam_reource_num[SYS_OAM_TBL_MAX];
    uint32 oam_session_num[SYS_OAM_SESSION_MAX];

    void* ipv6_bmp;

    uint32* mep_defect_bitmap;
    uint32  rsv_maname_idx;
    uint32  sf_defect_bitmap;

    uint8 timer_update_disable;
    uint8 no_mep_resource;
    uint32 defect_to_rdi_bitmap0;
    uint32 defect_to_rdi_bitmap1;
};
typedef struct sys_oam_master_s sys_oam_master_t;


#define SYS_OAM_NO_MEP_OPERATION(lchip, master_gchip)\
(((FALSE == sys_usw_chip_is_local(lchip, master_gchip))\
        && (1 == g_oam_master[lchip]->mep_index_alloc_by_sdk))\
        || g_oam_master[lchip]->no_mep_resource)\


#define SYS_OAM_FUNC_TABLE(lchip, type, module, op) \
    *(SYS_OAM_CALL_FUNC_P*) \
    (g_oam_master[lchip]->p_fun_table + (type * (SYS_OAM_MODULE_MAX * SYS_OAM_OP_MAX) + module * SYS_OAM_OP_MAX + op))

#define SYS_OAM_CHECK_FUNC_TABLE(lchip, type, module) \
    *(SYS_OAM_CHECK_CALL_FUNC_P*) \
    (g_oam_master[lchip]->p_check_fun_table + ((type * (SYS_OAM_CHECK_MAX)) + module ))


#define SYS_OAM_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == g_oam_master[lchip]) \
        { \
            return CTC_E_NOT_INIT; \
        } \
    } while (0)

#define SYS_OAM_FUNC_TABLE_CHECK(p_func) \
    do { \
        if (NULL == p_func) \
        { \
            return CTC_E_NOT_SUPPORT; \
        } \
    } while (0)


#define OAM_LOCK(lchip) \
    do { \
        if (g_oam_master[lchip]->oam_mutex) \
        { \
            sal_mutex_lock(g_oam_master[lchip]->oam_mutex); \
        } \
    } while (0)

#define OAM_UNLOCK(lchip) \
    do { \
        if (g_oam_master[lchip]->oam_mutex) \
        { \
            sal_mutex_unlock(g_oam_master[lchip]->oam_mutex); \
        } \
    } while (0)


#define SYS_OAM_SESSION_CNT(lchip, id, add) \
    do { \
        if (id < SYS_OAM_SESSION_MAX) \
        {\
            (add ? g_oam_master[lchip]->oam_session_num[id]++ : g_oam_master[lchip]->oam_session_num[id]--); \
        }\
    } while (0)


#define SYS_OAM_TBL_CNT(lchip, id, add, step) \
    do { \
        if (id < SYS_OAM_TBL_MAX) \
        {\
            (add ? (g_oam_master[lchip]->oam_reource_num[id]+= step) : (g_oam_master[lchip]->oam_reource_num[id]-=step)); \
        }\
    } while (0)




/***************************************************************
*
*           OAM DB COM HEADER
*
*****************************************************************/

struct sys_oam_maid_com_s
{
    uint8    mep_type;
    uint8    maid_len;
    uint8    maid_entry_num;
    uint8    ref_cnt;
    uint32   maid_index;
    uint8    maid[SYS_OAM_MAX_MAID_LEN];
};
typedef struct sys_oam_maid_com_s sys_oam_maid_com_t;

struct sys_oam_key_com_s
{
    uint8       mep_type;
    uint8       key_alloc;  /*sys_oam_key_alloc_t*/
    uint8       key_exit;
    uint8       rsv;
    uint32      key_index;
};
typedef struct sys_oam_key_com_s sys_oam_key_com_t;

struct sys_oam_chan_com_s
{
    ctc_slist_t* lmep_list;
    uint8        mep_type;
    uint8        master_chipid;
    uint8        lm_num; /*lm number for ether, not include link*/
    uint8        rsv;
    uint16       lm_index_base; /*lm profile index when ether*/
    uint16       link_lm_index_base;
};
typedef struct sys_oam_chan_com_s sys_oam_chan_com_t;

struct sys_oam_lmep_s
{
    /*common*/
    ctc_slistnode_t head;
    ctc_slist_t* rmep_list;
    sys_oam_maid_com_t* p_sys_maid;
    sys_oam_chan_com_t* p_sys_chan;
    uint32 lmep_index;
    uint16 ma_index;
    uint8  mep_on_cpu;
    uint8  lm_cos;
    uint8  lm_cos_type;
    uint16 lm_index_base;   /*lm index*/
    uint32 nhid;
    uint32 flag;           /*BFD Local mep flag,refer to ctc_oam_bfd_lmep_flag_t; Y1731 Local mep flag,refer to ctc_oam_y1731_lmep_update_type_t */

    /*BFD*/
    uint32 local_discr;           /*for debug show*/
    uint32 mpls_in_label;
    uint8  spaceid;
    sal_systime_t tv_p;           /*used for pf process by sw, indicate rx pbit pkt timestamp*/
    sal_systime_t tv_f;           /*used for pf process by sw, indicate rx fbit pkt timestamp*/
    sal_systime_t tv_p_set;       /*record the TS when set pbit, the interval to set pbit must over 1 second, only used for master device*/

    /*Y1731*/
    uint8  tx_csf_type;   /*used for rmep*/
    uint8  md_level;
    uint8  is_up;
    uint8  vlan_domain;
    uint8  lm_index_alloced; /*used for ether*/
    uint8  lock_en;
}
;
typedef struct sys_oam_lmep_s sys_oam_lmep_t;

struct sys_oam_rmep_s
{
    ctc_slistnode_t head;
    sys_oam_maid_com_t* p_sys_maid;
    sys_oam_lmep_t* p_sys_lmep;
    uint32 rmep_index;
    uint32 flag;            /** BFD Remote mep flag, refer to ctc_oam_bfd_rmep_flag_t; Y1731 Remote mep flag, refer to ctc_oam_y1731_rmep_flag_t*/
    /*Y1731*/
    uint32  key_index;
    uint32  rmep_id;
    uint8   md_level;

};
typedef struct sys_oam_rmep_s sys_oam_rmep_t;

/***************************************************************
*
*           Eth OAM DB
*
*****************************************************************/
#define ETHERNET_OAM "ETHERNET OAM Begin"

#define SYS_OAM_MAX_MD_LEVEL 7
#define SYS_OAM_MAX_MEP_NUM_PER_CHAN 4

struct sys_oam_key_eth_s
{
    sys_oam_key_com_t com;

    uint8     use_fid;
    uint8     link_oam;

    uint16    gport;     /**<Ethoam*/
    uint16    vlan_id;   /**<Ethoam vlanID*/

    uint16    cvlan_id;   /**<Ethoam cvlanID*/
    uint16    l2vpn_oam_id;    /**< vpws oam id or vpls fid  */

    uint8      is_vpws;
    uint8      is_cvlan;

};
typedef struct sys_oam_key_eth_s sys_oam_key_eth_t;

struct sys_oam_chan_eth_s
{
    sys_oam_chan_com_t com;

    sys_oam_key_eth_t key;

    uint16  tp_oam_key_index[2];   /*duplicate tp oam key index*/
    uint8  up_mep_bitmap;
    uint8  down_mep_bitmap;
    uint8  mip_bitmap;
    uint8  lm_bitmap;

    uint8  lm_type;
    uint8  link_lm_type;
    uint8  tp_oam_key_valid[2];    /*duplicate tp oam key index exist*/

    uint8  link_lm_cos_type;
    uint8  link_lm_cos;
    uint8  lm_index_alloced; /*lm profile when ether*/
    uint8  link_lm_index_alloced;

    uint32 mep_index[SYS_OAM_MAX_MD_LEVEL + 1];
    uint16 lm_index[SYS_OAM_MAX_MD_LEVEL + 1];/*lm index for ether*/
    uint8  lm_cos[SYS_OAM_MAX_MD_LEVEL + 1];
    uint8  lm_cos_type[SYS_OAM_MAX_MD_LEVEL + 1];
};
typedef struct sys_oam_chan_eth_s sys_oam_chan_eth_t;

struct sys_oam_id_s
{
    /*key*/
    uint32 oam_id;
    uint32 gport;
    /*ad*/
    sys_oam_chan_eth_t* p_sys_chan_eth; /*VPWS OAM*/
    uint32 label[2];
    uint8  space_id[2];
    uint8  rsv[2];
};
typedef struct sys_oam_id_s sys_oam_id_t;

/***************************************************************
*
*           MPLS-TP Y.1731 OAM DB
*
*****************************************************************/
#define MPLS_TP_Y1731 "MPLS-TP Y.1731 Begin"

struct sys_oam_key_tp_s
{
    sys_oam_key_com_t com;

    uint8     section_oam;
    uint8     mip_en;
    uint16    gport_l3if_id;
    uint32    label;
    uint8     spaceid;
    uint8     resv[3];
};
typedef struct sys_oam_key_tp_s sys_oam_key_tp_t;

struct sys_oam_chan_tp_s
{
    sys_oam_chan_com_t com;

    sys_oam_key_tp_t key;

    uint8  lm_type;
    uint8  lm_cos_type;
    uint8  lm_cos;
    uint8  lm_index_alloced;

    uint32 mep_index;

    uint16 mep_index_in_key;
    uint8  mep_on_cpu;
    uint8  rsv;
};
typedef struct sys_oam_chan_tp_s sys_oam_chan_tp_t;


/***************************************************************
*
*           BFD OAM DB
*
*****************************************************************/
struct sys_oam_key_bfd_s
{
    sys_oam_key_com_t com;

    uint32    my_discr;
    uint8     is_sbfd_reflector;
    uint8     rsv[3];
};
typedef struct sys_oam_key_bfd_s sys_oam_key_bfd_t;

struct sys_oam_chan_bfd_s
{
    sys_oam_chan_com_t com;

    sys_oam_key_bfd_t key;

    uint32 mep_index;
};
typedef struct sys_oam_chan_bfd_s sys_oam_chan_bfd_t;

/***************************************************************
*
*           MISC OAM
*
*****************************************************************/
struct sys_oam_nhop_info_s
{
    uint32  dsnh_offset;

    uint8   nexthop_is_8w;
    uint8   rsv[3];

    uint32  dest_map;

    uint8   aps_bridge_en;
    uint8   nh_entry_type;
    uint8   mep_on_tunnel;
    uint8   have_l2edit;
};
typedef struct sys_oam_nhop_info_s sys_oam_nhop_info_t;

struct sys_oam_lm_com_s
{
    uint8 lm_type;
    uint8 lm_cos_type;
    uint16 lm_index;
};
typedef struct sys_oam_lm_com_s sys_oam_lm_com_t;

struct sys_oam_defect_info_s
{
    char  *defect_name;
    uint8 to_rdi;
    uint8 to_event;
};
typedef struct sys_oam_defect_info_s sys_oam_defect_info_t;

/****************************************************************************
 *
* Function
*
****************************************************************************/

/*MAID DB API*/
sys_oam_maid_com_t*
_sys_usw_oam_maid_lkup(uint8 lchip, ctc_oam_maid_t* p_ctc_maid);

sys_oam_maid_com_t*
_sys_usw_oam_maid_build_node(uint8 lchip, ctc_oam_maid_t* p_maid_param);

int32
_sys_usw_oam_maid_free_node(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_build_index(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_free_index(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_add_to_db(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_del_from_db(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_add_to_asic(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

int32
_sys_usw_oam_maid_del_from_asic(uint8 lchip, sys_oam_maid_com_t* p_sys_maid);

/*CHAN DB API*/
sys_oam_chan_com_t*
_sys_usw_oam_chan_lkup(uint8 lchip, sys_oam_chan_com_t* p_sys_chan);

int32
_sys_usw_oam_chan_build_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan);

int32
_sys_usw_oam_chan_free_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan);

int32
_sys_usw_oam_chan_add_to_db(uint8 lchip, sys_oam_chan_com_t* p_sys_chan);

int32
_sys_usw_oam_chan_del_from_db(uint8 lchip, sys_oam_chan_com_t* p_sys_chan);

int32
_sys_usw_oam_lm_build_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint8 is_link_sec, uint8 md_level);

int32
_sys_usw_oam_lm_free_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint8 is_link_sec, uint8 md_level);

/*LMEP DB API*/
sys_oam_lmep_t*
_sys_usw_oam_lmep_lkup(uint8 lchip, sys_oam_chan_com_t* p_sys_chan,
                             sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_add_eth_to_asic(uint8 lchip, ctc_oam_y1731_lmep_t* p_eth_lmep, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_add_tp_y1731_to_asic(uint8 lchip, ctc_oam_y1731_lmep_t* p_tp_y1731_lmep, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep);

int32
_sys_usw_oam_lmep_update_eth_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_oam_lmep_update_tp_y1731_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_oam_lmep_update_master_chip(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint32 master_chipid);

int32
_sys_usw_oam_lmep_update_label(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, ctc_oam_tp_key_t* tp_oam_key);
/*RMEP DB API*/
sys_oam_rmep_t*
_sys_usw_oam_rmep_lkup(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, sys_oam_rmep_t* p_sys_rmep);
int32
_sys_usw_oam_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_add_eth_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_add_tp_y1731_to_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);


int32
_sys_usw_oam_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep);

int32
_sys_usw_oam_rmep_update_eth_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep, ctc_oam_update_t* p_rmep_param);

int32
_sys_usw_oam_rmep_update_tp_y1731_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep, ctc_oam_update_t* p_rmep_param);

int32
_sys_usw_oam_get_mep_entry_num(uint8 lchip);

int32
_sys_usw_oam_get_mpls_entry_num(uint8 lchip);

uint8
_sys_usw_bfd_csf_convert(uint8 lchip, uint8 type, bool to_asic);

int32
_sys_usw_oam_get_nexthop_info(uint8 lchip, uint32 nhid, uint32 b_protection, sys_oam_nhop_info_t* p_nhop_info);

int32
_sys_usw_oam_register_init(uint8 lchip, ctc_oam_global_t* p_oam_glb);
int32
_sys_usw_bfd_update_nh_process(uint8 lchip, uint32 nhid, mac_addr_t mac);
int32
_sys_usw_oam_defect_read_defect_status(uint8 lchip, void* p_defect_info);
int32
_sys_usw_oam_get_defect_info(uint8 lchip, void* p_defect_info);
int32
_sys_usw_oam_get_mep_info(uint8 lchip, ctc_oam_mep_info_t* mep_info);

int32
_sys_usw_oam_get_stats_info(uint8 lchip, sys_oam_lm_com_t* p_sys_oam_lm, ctc_oam_stats_info_t* p_stat_info);

int32
_sys_usw_oam_build_defect_event(uint8 lchip, ctc_oam_mep_info_t* p_mep_info, ctc_oam_event_t* p_event);

int32
_sys_usw_oam_db_init(uint8 lchip, ctc_oam_global_t* p_com_glb);

int32
_sys_usw_oam_db_deinit(uint8 lchip);

int32
sys_usw_oam_get_session_type(uint8 lchip, sys_oam_lmep_t *p_oam_lmep, uint32 *session_type);

int32
sys_usw_oam_get_session_num(uint8 lchip, uint32* used_session);

int32
sys_usw_oam_get_defect_type_config(uint8 lchip ,ctc_oam_defect_t defect, sys_oam_defect_info_t *p_defect);

int32
_sys_usw_oam_get_rdi_defect_type(uint8 lchip, uint32 defect, uint8* defect_type, uint8* defect_sub_type);

int32
sys_usw_oam_wb_sync(uint8 lchip);

int32
sys_usw_oam_wb_restore(uint8 lchip);
sys_oam_id_t*
_sys_usw_oam_oamid_lkup(uint8 lchip, uint32 gport, uint32 oam_id);
int32
_sys_usw_oam_oamid_add_to_db(uint8 lchip, sys_oam_id_t* p_sys_oamid);
int32
_sys_usw_oam_oamid_del_from_db(uint8 lchip, sys_oam_id_t* p_sys_oamid);
extern int32
sys_usw_oam_add_oamid(uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid);
extern int32
sys_usw_oam_remove_oamid(uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid);
extern int32
_sys_usw_oam_update_vpws_key(uint8 lchip, uint32 gport, uint16 oamid, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add);
extern int32
sys_usw_oam_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);

#ifdef __cplusplus
}
#endif

#endif

#endif
