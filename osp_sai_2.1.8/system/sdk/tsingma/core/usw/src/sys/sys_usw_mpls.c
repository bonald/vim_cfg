#if (FEATURE_MODE == 0)
/**
 @file sys_usw_mpls.c

 @date 2010-03-12

 @version v2.0

 The file contains all mpls related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"

#include "ctc_mpls.h"
#include "ctc_spool.h"
#include "ctc_register.h"

#include "sys_usw_opf.h"

#include "sys_usw_chip.h"
#include "sys_usw_mpls.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_vlan.h"
#include "sys_usw_ftm.h"
#include "sys_usw_register.h"

#include "sys_usw_stats_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_common.h"

#include "drv_api.h"


/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
#define SYS_MPLS_TBL_BLOCK_SIZE         64
#define SYS_MPLS_MAX_L3VPN_VRF_ID       0xBFFE
#define SYS_MPLS_MAX_COLOR              (MAX_CTC_QOS_COLOR - 1)

#define SYS_MPLS_INIT_CHECK(lchip) \
    do \
    { \
        if ((lchip) >= sys_usw_get_local_chip_num(lchip)){  \
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");\
			return CTC_E_INVALID_CHIP_ID;\
 }\
        if (p_usw_mpls_master[lchip] == NULL){ \
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_MPLS_ILM_KEY_CHECK(p_ilm)\
    do\
    {\
        if ((p_ilm)->label > MCHIP_CAP(SYS_CAP_MPLS_MAX_LABEL)){   \
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Parameter \n");\
            return CTC_E_INVALID_PARAM; }\
    } while(0)

#define SYS_MPLS_DBG_DUMP(FMT, ...)                      \
    do                                                       \
    {                                                        \
        CTC_DEBUG_OUT_INFO(mpls, mpls, MPLS_SYS, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_MPLS_DUMP(FMT, ...)                    \
    do                                                       \
    {                                                        \
        CTC_DEBUG_OUT(mpls, mpls, MPLS_SYS, CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_MPLS_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(mpls, mpls, MPLS_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

#define SYS_MPLS_DUMP_ILM(p_mpls_ilm) \
    do { \
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "    p_mpls_ilm :: nh_id = %d, spaceid = %d , label = %d,\n" \
                         "    id_type = %d, 0-NULL,1-FLOW,2-VRF,3-SERVICE,4-APS,5-STATS,6-MAX,\n" \
                         "    label type = %d, 0-NORMAL LABEL,1-L3VPN LABEL,2-VPWS,3-VPLS,4-MAX,\n" \
                         "    tunnel model = %d, 0-UNIFORM,1-SHORT PIPE,2-PIPE,3-MAX,\n" \
                         "    pop = %d, cwen = %d, aps_select_protect_path = %d,  oam_en = %d,\n" \
                         "    pwid = %d, logic_port_type = %d,\n" \
                         "    flw_vrf_srv_aps = %d, flow_id   vrf_id   service_id   aps_select_grp_id\n\n", \
                         p_mpls_ilm->nh_id, p_mpls_ilm->spaceid, p_mpls_ilm->label, \
                         p_mpls_ilm->id_type, \
                         p_mpls_ilm->type, \
                         p_mpls_ilm->model, \
                         p_mpls_ilm->pop, p_mpls_ilm->cwen, p_mpls_ilm->aps_select_protect_path, p_mpls_ilm->oam_en, \
                         p_mpls_ilm->pwid, p_mpls_ilm->logic_port_type, \
                         p_mpls_ilm->flw_vrf_srv_aps.flow_id); \
    } while (0)

#define SYS_MPLS_DUMP_MPLS_DB_INFO(p_mpls_info) \
    do { \
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "    p_mpls_info :: key_offset = %d, spaceid = %d ,\n" \
                         "label = %d,nh_id = %d,pop = %d\n" \
                         "    id_type = %d, 0x0-NULL,0x1-FLOW,0x2-VRF,0x4-SERVICE,0x8-APS_SELECT,0x10-STATS,0xFFFF-MAX,\n" \
                         "    label type = %d, 0-NORMAL LABEL,1-L3VPN LABEL,2-VPWS,3-VPLS,4-MAX,\n" \
                         "    tunnel model = %d, 0-UNIFORM,1-SHORT PIPE,2-PIPE,3-MAX,\n" \
                         "    pwid = %d, logic_port_type = %d,\n" \
                         "    fid = %d, pw_mode = %d  0-Tagged PW    1-Raw PW,\n" \
                         "    learn_disable = %d, igmp_snooping_enable =%d, maclimit_enable=%d, service_aclqos_enable = %d,\n" \
                         "    cwen = %d, oam_en = %d,stats_ptr = %d,\n" \
                         "    flw_vrf_srv = %d, aps_group_id = %d, aps_select_ppath =%d\n\n", \
                         p_mpls_info->key_offset, p_mpls_info->spaceid, p_mpls_info->label, p_mpls_info->nh_id, p_mpls_info->pop, \
                         p_mpls_info->id_type, \
                         p_mpls_info->type, \
                         p_mpls_info->model, \
                         p_mpls_info->logic_src_port, p_mpls_info->logic_port_type, \
                         p_mpls_info->fid, p_mpls_info->pw_mode, \
                         p_mpls_info->learn_disable, p_mpls_info->igmp_snooping_enable, p_mpls_info->maclimit_enable, p_mpls_info->service_aclqos_enable, \
                         p_mpls_info->cwen, p_mpls_info->oam_en, p_mpls_info->stats_ptr, \
                         p_mpls_info->mpls_flow_policer_ptr, p_mpls_info->aps_group_id,  p_mpls_info->aps_select_ppath); \
    } while (0)

#define SYS_MPLS_DUMP_VPWS_PARAM(p_mpls_pw) \
    do { \
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "    p_mpls_pw :: label = %d, l2vpntype = %d,\n" \
                         "    pw_mode = %d,  \n" \
                         "    learn_disable = %d, maclimit_enable = %d, service_aclqos_enable = %d, igmp_snooping_enable = %d\n" \
                         "    pw_nh_id = %d\n\n", \
                         p_mpls_pw->label, p_mpls_pw->l2vpntype, p_mpls_pw->pw_mode, \
                         p_mpls_pw->learn_disable, p_mpls_pw->maclimit_enable, p_mpls_pw->service_aclqos_enable, p_mpls_pw->igmp_snooping_enable, \
                         p_mpls_pw->u.pw_nh_id); \
    } while (0)

#define SYS_MPLS_DUMP_VPLS_PARAM(p_mpls_pw) \
    do { \
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "    p_mpls_pw :: label = %d, l2vpntype = %d,\n" \
                         "    pw_mode = %d,  \n" \
                         "    learn_disable = %d, maclimit_enable = %d, service_aclqos_enable = %d, igmp_snooping_enable = %d\n" \
                         "    fid = %d, logic_src_port = %d, " \
                         "    logic_port_type= %d 0-H_VPLS 1-Normal_VPLS \n\n", \
                         p_mpls_pw->label, p_mpls_pw->l2vpntype, p_mpls_pw->pw_mode, \
                         p_mpls_pw->learn_disable, p_mpls_pw->maclimit_enable, p_mpls_pw->service_aclqos_enable, p_mpls_pw->igmp_snooping_enable, \
                         p_mpls_pw->u.vpls_info.fid, p_mpls_pw->u.vpls_info.logic_port, p_mpls_pw->u.vpls_info.logic_port_type); \
    } while (0)

struct sys_mpls_show_info_s
{
    uint32 space_id;
    uint32 count;
    uint8 lchip;
    uint8 is_tcam;
    uint8 rsv[2];
};
typedef struct sys_mpls_show_info_s sys_mpls_show_info_t;

struct sys_mpls_ilm_spool_s
{
    DsMpls_m spool_ad;
    uint32 ad_index;
    uint8 is_half;
    uint8 rsv[3];
};
typedef struct sys_mpls_ilm_spool_s sys_mpls_ilm_spool_t;

struct sys_mpls_ilm_tcam_s
{
    uint32 entry_id;
    DsMpls_m ad_info;
};
typedef struct sys_mpls_ilm_tcam_s sys_mpls_ilm_tcam_t;

struct sys_mpls_ilm_hash_s
{
    /*key*/
    uint32 label;
    uint16 spaceid:       8;
    uint16 is_interface:  1;
    uint16 rsv:           7;
    /*action*/
    uint32 nh_id;
    uint32 stats_id;
    uint8 model: 7;
    uint8 bind_dsfwd: 1;
    uint16 type:  4;
    uint16 id_type: 4;
    uint16 is_vpws_oam:   1;
    uint16 is_tp_oam:     1;
    uint16 rsv1:           6;
    uint32 ad_index;
    uint8  u2_type: 7;
    uint8 is_assignport: 1;
    uint8  u4_type;
    uint16 u2_bitmap;
    uint16 u4_bitmap;
    uint16 service_id;
    uint16 oam_id;   /*vpws l2vpn_id*/
    uint32 gport;    /*only used for resotre vpws oam ac port info*/
    uint8 is_tcam;
    uint8 rsv2;
};
typedef struct sys_mpls_ilm_hash_s sys_mpls_ilm_hash_t;

struct sys_mpls_ilm_s
{
    uint32 key_offset;
    uint32 ad_offset;          /*new ad offset*/
    DsMpls_m* p_new_ad;
    DsMpls_m* p_old_ad;
    uint16 aps_group_id;
    uint16 stats_ptr;
    uint32 flag;
    uint32 gport;
    uint8  update;
    sys_mpls_ilm_hash_t* p_cur_hash;
    ctc_mpls_property_t* p_prop_info;
    sys_mpls_ilm_spool_t* p_spool_ad;
};
typedef struct sys_mpls_ilm_s sys_mpls_ilm_t;

struct sys_mpls_inner_route_mac_s
{
    uint16 ref_cnt;
    mac_addr_t inner_route_mac;
};
typedef struct sys_mpls_inner_route_mac_s sys_mpls_inner_route_mac_t;

typedef int32 (* mpls_write_ilm_t)(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info, DsMpls_m* p_dsmpls);

struct sys_mpls_master_s
{
    sal_mutex_t* mutex;
    ctc_hash_t *p_mpls_hash;
    ctc_spool_t * ad_spool;
    uint32 cur_ilm_num;
    uint8  tcam_group[2];  /*create scl group once*/
    uint16  mpls_cur_tcam; /*current tcam entry number*/
    uint32 pop_ad_idx;
    uint32 decap_ad_idx;
    uint8  decap_mode;      /* 0:normal mode, other:iloop mode */
    uint8  decap_tcam_group;
};
typedef struct sys_mpls_master_s sys_mpls_master_t;

enum sys_mpls_oam_check_type_e
{
    SYS_MPLS_OAM_CHECK_TYPE_NONE,
    SYS_MPLS_OAM_CHECK_TYPE_MEP,
    SYS_MPLS_OAM_CHECK_TYPE_MIP,
    SYS_MPLS_OAM_CHECK_TYPE_DISCARD,
    SYS_MPLS_OAM_CHECK_TYPE_TOCPU,
    SYS_MPLS_OAM_CHECK_TYPE_DATA_DISCARD,
    SYS_MPLS_OAM_CHECK_TYPE_MAX
};
typedef enum sys_mpls_oam_check_type_e sys_mpls_oam_check_type_t;


enum sys_mpls_group_sub_type_e
{
       SYS_MPLS_GROUP_SUB_TYPE_TCAM0,
       SYS_MPLS_GROUP_SUB_TYPE_TCAM1,
       SYS_MPLS_GROUP_SUB_TYPE_LOOP,
       SYS_MPLS_GROUP_SUB_TYPE_MAX
};
typedef enum sys_mpls_group_sub_type_e sys_mpls_group_sub_type_t;

sys_mpls_master_t* p_usw_mpls_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_MPLS_CREAT_LOCK(lchip)                   \
    do                                            \
    {                                             \
        sal_mutex_create(&p_usw_mpls_master[lchip]->mutex); \
        if (NULL == p_usw_mpls_master[lchip]->mutex)  \
        { \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); \
        } \
    } while (0)

#define SYS_MPLS_LOCK(lchip) \
    if(p_usw_mpls_master[lchip]->mutex) sal_mutex_lock(p_usw_mpls_master[lchip]->mutex)

#define SYS_MPLS_UNLOCK(lchip) \
    if(p_usw_mpls_master[lchip]->mutex) sal_mutex_unlock(p_usw_mpls_master[lchip]->mutex)

#define CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_mpls_master[lchip]->mutex); \
            return rv; \
        } \
    } while (0)

#define SYS_MPLS_TABLE_ID  (DRV_IS_DUET2(lchip)?DsUserIdTunnelMplsHashKey_t: DsMplsLabelHashKey_t)


#define CTC_RETURN_MPLS_UNLOCK(lchip, op) \
    do \
    { \
        sal_mutex_unlock(p_usw_mpls_master[lchip]->mutex); \
        return (op); \
    } while (0)

STATIC int32 _sys_usw_mpls_ilm_common(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_sys_ilm);
STATIC sys_mpls_ilm_hash_t* _sys_usw_mpls_ilm_get_hash(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint8 is_tcam);
extern int32 sys_usw_mpls_wb_sync(uint8 lchip);
extern int32 sys_usw_mpls_wb_restore(uint8 lchip);
STATIC int32 _sys_usw_mpls_update_tcam_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* mpls_info, void* p_ad, uint8 scl_id);
STATIC int32 _sys_usw_mpls_ilm_set_default_action(uint8 lchip, sys_mpls_ilm_t* p_ilm);
extern int32 sys_usw_oam_add_oamid(uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid);
extern int32 sys_usw_oam_remove_oamid(uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid);
#define _________OPERATION_ON_HASHKEY_________

STATIC int32
_sys_usw_mpls_lookup_key(uint8 lchip, DsUserIdTunnelMplsHashKey_m* pkey, uint32* p_key_index)
{
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id    = SYS_MPLS_TABLE_ID;
    in.data      = pkey;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    if (out.is_conflict)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key conflict!!\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [MPLS] No resource \n");
			return CTC_E_HASH_CONFLICT;

    }

    *p_key_index = out.key_index;

    if (out.is_hit)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key exist!!\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

    }

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Key Index:%#x \n", *p_key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_alloc_key(uint8 lchip, DsUserIdTunnelMplsHashKey_m* pkey, uint32* p_key_index)
{
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    in.type = DRV_ACC_TYPE_ALLOC;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id    = SYS_MPLS_TABLE_ID;
    in.data      = pkey;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    if (out.is_conflict)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key conflict!!\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
			return CTC_E_HASH_CONFLICT;

    }

    *p_key_index = out.key_index;

    if (out.is_hit)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key exist!!\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

    }

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Alloc Key Index:%#x \n", *p_key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_add_key(uint8 lchip, DsUserIdTunnelMplsHashKey_m* p_key, uint32 key_index)
{
    drv_acc_in_t in;
    drv_acc_out_t out;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id    = SYS_MPLS_TABLE_ID;
    in.data      = p_key;
    in.index = key_index;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key index:%d\n", key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_remove_key(uint8 lchip, uint32 key_index)
{
    drv_acc_in_t in;
    drv_acc_out_t out;
    DsUserIdTunnelMplsHashKey_m key;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&key, 0, sizeof(DsUserIdTunnelMplsHashKey_m));
    sal_memset(&in, 0, sizeof(in));
    in.type = DRV_ACC_TYPE_DEL;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id    = SYS_MPLS_TABLE_ID;
    in.data      = &key;
    in.index = key_index;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key index:%d\n", key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_get_key_ad(uint8 lchip, uint32 key_index, uint32* p_ad_index)
{
    drv_acc_in_t in;
    drv_acc_out_t out;
    DsUserIdTunnelMplsHashKey_m key;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&key, 0, sizeof(DsUserIdTunnelMplsHashKey_m));
    sal_memset(&in, 0, sizeof(in));
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id    = SYS_MPLS_TABLE_ID;
    in.index = key_index;
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    sal_memcpy(&key, out.data, sizeof(key));

    *p_ad_index = GetDsUserIdTunnelMplsHashKey(V, dsAdIndex_f, &key);

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key index:%d\n", key_index);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key ad index:%d\n", *p_ad_index);

    return CTC_E_NONE;
}


#define _________OPERATION_ON_DSMPLS_________
STATIC int32
_sys_usw_mpls_write_dsmpls(uint8 lchip, uint32 ad_offset, DsMpls_m* p_dsmpls)
{
    uint32 cmd = 0;
    DsMplsHalf_m* p_half = NULL;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls AD index:%d\n", ad_offset);

    if (GetDsMpls(V, isHalf_f, p_dsmpls))
    {
        p_half = (DsMplsHalf_m*)p_dsmpls;
        cmd = DRV_IOW(DsMplsHalf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_half));
    }
    else
    {
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOW(DsMpls_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_dsmpls));
        }
        else
        {
            if (ad_offset >= 64)
            {
                cmd = DRV_IOW(DsMpls_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset - 64, cmd, p_dsmpls));
            }
            else
            {
                cmd = DRV_IOW(DsMplsHashCamAd_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_dsmpls));
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_read_dsmpls(uint8 lchip, uint32 ad_offset, DsMpls_m* p_dsmpls)
{
    uint32 cmd = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls AD index:%d\n", ad_offset);

    if ((ad_offset & 0x1) && DRV_IS_DUET2(lchip))
    {
        /*must half*/
        cmd = DRV_IOR(DsMplsHalf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_dsmpls));
    }
    else
    {
        /*maybe half or full, read as full ad*/
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOR(DsMpls_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_dsmpls));
        }
        else
        {
            if (ad_offset >= 64)
            {
                cmd = DRV_IOR(DsMpls_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset - 64, cmd, p_dsmpls));
            }
            else
            {
                cmd = DRV_IOR(DsMplsHashCamAd_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, p_dsmpls));
            }
        }
    }

    return CTC_E_NONE;
}

 #define _________DB_HASH_________

STATIC sys_mpls_ilm_hash_t*
_sys_usw_mpls_db_lookup_hash(uint8 lchip, sys_mpls_ilm_hash_t* p_db_hash_info)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "    spaceid = %d  label:%d is_tcam: %d\n",
                  p_db_hash_info->spaceid, p_db_hash_info->label, p_db_hash_info->is_tcam);

    return ctc_hash_lookup(p_usw_mpls_master[lchip]->p_mpls_hash, p_db_hash_info);
}

STATIC int32
_sys_usw_mpls_db_add_hash(uint8 lchip, sys_mpls_ilm_hash_t* p_db_hash_info)
{

    sys_mpls_ilm_hash_t* p_hash_ret_node = NULL;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    p_hash_ret_node = ctc_hash_insert(p_usw_mpls_master[lchip]->p_mpls_hash, p_db_hash_info);
    if (NULL == p_hash_ret_node)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [MPLS] No Memory \n");
        return CTC_E_NO_MEMORY;

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_db_remove_hash(uint8 lchip, sys_mpls_ilm_hash_t* p_db_hash_info)
{
    sys_mpls_ilm_hash_t* p_node = NULL;
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_node = ctc_hash_remove(p_usw_mpls_master[lchip]->p_mpls_hash, p_db_hash_info);
    if (p_node)
    {
        mem_free(p_node);
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_mpls_hash_make(sys_mpls_ilm_hash_t* backet)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!backet)
    {
        return 0;
    }

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"spaceid = %d, label = %d\n",
                             backet->spaceid, backet->label);
    return ctc_hash_caculate(sizeof(uint32)+sizeof(uint16), &(backet->label));

}


STATIC bool
_sys_usw_mpls_hash_cmp(sys_mpls_ilm_hash_t* p_ilm_data0,
                              sys_mpls_ilm_hash_t* p_ilm_data1)
{
    if (!p_ilm_data0 || !p_ilm_data1)
    {
        return FALSE;
    }

    if ((p_ilm_data0->spaceid == p_ilm_data1->spaceid)
        && (p_ilm_data0->label == p_ilm_data1->label)
        && (p_ilm_data0->is_interface == p_ilm_data1->is_interface))
    {
        return TRUE;
    }

    return FALSE;
}

#define _________DB_SPOOL_________

uint32
_sys_usw_mpls_ad_spool_hash_make(sys_mpls_ilm_spool_t* p_spool_node)
{
        return ctc_hash_caculate(sizeof(DsMpls_m), &p_spool_node->spool_ad);
}

int
_sys_usw_mpls_ad_spool_hash_cmp(sys_mpls_ilm_spool_t* p_ilm_data0,
                              sys_mpls_ilm_spool_t* p_ilm_data1)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_ilm_data0 || !p_ilm_data1)
    {
        return FALSE;
    }

    if (!sal_memcmp(&p_ilm_data0->spool_ad, &p_ilm_data1->spool_ad, sizeof(DsMpls_m)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_mpls_ad_spool_alloc_index(sys_mpls_ilm_spool_t* p_ilm, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    uint32 ad_index = 0;
    int32 ret = 0;
    uint32 entry_num = 0;
    uint8  dir = 0;
    uint8 multi = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is_half:%d\n", p_ilm->is_half);

    dir = 0;
    entry_num = p_ilm->is_half?1:2;
    multi = p_ilm->is_half?1:2;

    if(!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        ret = sys_usw_ftm_alloc_table_offset(lchip, DsMpls_t, dir, entry_num, multi, &ad_index);
    }

    if(ret)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spool alloc index failed,  line=%d\n",__LINE__);
        return CTC_E_NO_RESOURCE;
    }
    p_ilm->ad_index = ad_index;
    return CTC_E_NONE;
}

int32
_sys_usw_mpls_ad_spool_free_index(sys_mpls_ilm_spool_t* p_ilm, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    int32 ret = 0;
    uint32 entry_num = 0;
    uint8   dir = 0;
    entry_num = p_ilm->is_half?1:2;
    dir = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_ilm);

    ret = sys_usw_ftm_free_table_offset(lchip, DsMpls_t, dir, entry_num, p_ilm->ad_index);

    if(ret)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spool fress index failed,  line=%d\n",__LINE__);
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

#define _________SYS_CORE_________

/* for vpls and vpws loop*/
STATIC int32
_sys_usw_mpls_build_scl_key(uint8 lchip, uint32 entry_id, ctc_mpls_ilm_t* p_mpls_ilm, sys_scl_lkup_key_t* p_lkup_key)
{
    sys_mpls_ilm_hash_t* p_db_hash = NULL;
    ctc_mpls_ilm_t mpls_ilm;
    ctc_field_key_t field_key;

    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    field_key.type = CTC_FIELD_KEY_CUSTOMER_ID;
    field_key.data = p_mpls_ilm->label << 12;
    field_key.mask = 0xFFFFF000;
    if (NULL == p_lkup_key)
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
    }
    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    field_key.data = 1;
    if (NULL == p_lkup_key)
    {
        CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
    }

    mpls_ilm.label = p_mpls_ilm->label;
    mpls_ilm.spaceid = p_mpls_ilm->spaceid;
    p_db_hash = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, 0);
    if (p_db_hash)
    {
        sys_nh_info_dsnh_t nhinfo;
        ctc_field_port_t field_port;
        ctc_field_port_t port_mask;
        uint16 lport = 0;

        sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
        sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
        sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));

        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_db_hash->nh_id, &nhinfo, 0));
        if(SYS_NH_TYPE_ILOOP == nhinfo.nh_entry_type)
        {
            lport = ((nhinfo.dest_map&0x100)|(((nhinfo.dsnh_offset>>16)&0x1)<<7)|(nhinfo.dsnh_offset&0xFF));
            field_key.type = CTC_FIELD_KEY_PORT;
            field_port.gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(SYS_DECODE_DESTMAP_GCHIP(nhinfo.dest_map)  , lport);
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            port_mask.gport = 0x3FFF;
            field_key.ext_data = &field_port;
            field_key.ext_mask = &port_mask;
            if (NULL == p_lkup_key)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, p_lkup_key));
            }
        }
    }
    else
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

/* for vpls and vpws */
STATIC int32
_sys_usw_mpls_build_scl_ad(uint8 lchip, uint32 entry_id, ctc_mpls_ilm_t* p_mpls_ilm)
{
    ctc_scl_field_action_t field_action;
    ctc_scl_logic_port_t logic_port;
    ctc_scl_aps_t scl_aps;
    ctc_scl_vlan_edit_t vlan_edit;

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_BOS_LABLE)
        || CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT)
        || CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT))
    {
        return CTC_E_FEATURE_NOT_SUPPORT;
    }

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    logic_port.logic_port = p_mpls_ilm->pwid;
    logic_port.logic_port_type = ((p_mpls_ilm->logic_port_type) ? 1 : 0);
    field_action.ext_data = &logic_port;
    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_SERVICE)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;
        field_action.data0 = p_mpls_ilm->flw_vrf_srv_aps.service_id ;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_SERVICE_ACL_EN))
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
        field_action.data0 = TRUE ;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_APS_SELECT)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        sal_memset(&scl_aps, 0, sizeof(ctc_scl_aps_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_APS;
        scl_aps.aps_select_group_id = p_mpls_ilm->aps_select_grp_id & 0x1FFF;
        scl_aps.is_working_path = (p_mpls_ilm->aps_select_protect_path?1:0);
        field_action.ext_data = &scl_aps;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_STATS)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_mpls_ilm->stats_id ;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (p_mpls_ilm->learn_disable)
    {
        sys_scl_wlan_client_t wlan_client;
        sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
        field_action.ext_data = &wlan_client;
        wlan_client.sta_roam_status = 1;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO) || p_mpls_ilm->qos_use_outer_info)
    {
        return CTC_E_FEATURE_NOT_SUPPORT;
    }


    /* p_mpls_pw->cwen , not support this feature when use userid */
    if (p_mpls_ilm->cwen)
    {
        return CTC_E_FEATURE_NOT_SUPPORT;
    }

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM))
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;
        field_action.data0 = p_mpls_ilm->l2vpn_oam_id;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    if (CTC_MPLS_L2VPN_RAW == p_mpls_ilm->pw_mode)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;
        vlan_edit.vlan_domain = CTC_SCL_VLAN_DOMAIN_CVLAN;
        field_action.ext_data = &vlan_edit;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    /* p_mpls_pw->pw_mode encapsule mode, will do by loopback port */
    /* p_mpls_pw->oam_en */
    /* p_mpls_pw->qos_use_outer_info */

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_build_vpws_scl_ad(uint8 lchip, uint32 entry_id, ctc_mpls_ilm_t* p_mpls_ilm)
{
    ctc_scl_field_action_t field_action;

    if (p_mpls_ilm->nh_id)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_mpls_ilm->nh_id;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_build_vpls_scl_ad(uint8 lchip, uint32 entry_id, ctc_mpls_ilm_t* p_mpls_ilm)
{
    ctc_scl_field_action_t field_action;

    if (p_mpls_ilm->fid)
    {
        sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        field_action.data0 = p_mpls_ilm->fid;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &field_action));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_mpls_add_l2vpn_pw_loop(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    ctc_scl_group_info_t group_info;
    ctc_scl_entry_t scl_entry;
    uint32 gid = 0;
    int32 ret = CTC_E_NONE;

    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.priority = 0;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_LOOP, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    if (0 == p_usw_mpls_master[lchip]->decap_tcam_group)
    {
       CTC_ERROR_RETURN(sys_usw_scl_create_group(lchip, gid, &group_info, 1));
       p_usw_mpls_master[lchip]->decap_tcam_group = 1;
    }
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.key_type = SYS_SCL_KEY_TCAM_MAC;
    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.mode = 1;
    scl_entry.resolve_conflict = 0;
    CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1));

    CTC_ERROR_GOTO(_sys_usw_mpls_build_scl_key(lchip, scl_entry.entry_id, p_mpls_ilm, NULL), ret, roll_back_0);
    CTC_ERROR_GOTO(_sys_usw_mpls_build_scl_ad(lchip, scl_entry.entry_id, p_mpls_ilm), ret, roll_back_0);
    if(CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type)
    {
        CTC_ERROR_GOTO(_sys_usw_mpls_build_vpws_scl_ad(lchip, scl_entry.entry_id, p_mpls_ilm), ret, roll_back_0);
    }
    else if (CTC_MPLS_LABEL_TYPE_VPLS == p_mpls_ilm->type)
    {
        CTC_ERROR_GOTO(_sys_usw_mpls_build_vpls_scl_ad(lchip, scl_entry.entry_id, p_mpls_ilm), ret, roll_back_0);
    }

    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, roll_back_0);

    return CTC_E_NONE;

roll_back_0:
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

    return ret;
}

int32
_sys_usw_mpls_remove_l2vpn_pw_loop(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint32 gid = 0;
    uint32 entry_id = 0;
    sys_scl_lkup_key_t lkup_key;

    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));

    if (0 == p_usw_mpls_master[lchip]->decap_tcam_group)
    {
        return CTC_E_NOT_EXIST;
    }
    lkup_key.key_type = SYS_SCL_KEY_TCAM_MAC;
    lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_LOOP, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    lkup_key.group_id = gid;

    CTC_ERROR_RETURN(_sys_usw_mpls_build_scl_key(lchip, 0, p_mpls_ilm, &lkup_key));

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    entry_id = lkup_key.entry_id;

    /* Install to hardware */
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, entry_id, 1) );

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_get_tcam_action_field(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_tcam_t* mpls_tcam, uint8 scl_id)
{
    ctc_scl_field_action_t field_action;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    uint32 gid[2] = {0};
    DsMpls_m ds_mpls_temp;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&ds_mpls_temp, 0, sizeof(DsMpls_m));

    if (1 < scl_id)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }
    gid[0] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM0, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    gid[1] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM1, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    lkup_key.group_priority = scl_id;

    lkup_key.group_id = gid[scl_id];
    lkup_key.resolve_conflict = 1;
    lkup_key.key_type = SYS_SCL_KEY_HASH_TUNNEL_MPLS;
    lkup_key.action_type = SYS_SCL_ACTION_MPLS;

    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL;
    field_key.mask = 0xfffff;
    field_key.data = p_mpls_ilm->label;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL_SPACE;
    field_key.mask = 0xfff;
    field_key.data = p_mpls_ilm->spaceid;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_IS_INTERFACEID;
    field_key.mask = 1;
    field_key.data = 0;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_MPLS;
    field_action.ext_data = &ds_mpls_temp;
    CTC_ERROR_RETURN(sys_usw_scl_get_action_field(lchip, lkup_key.entry_id, &field_action));
    mpls_tcam->entry_id = lkup_key.entry_id;
    sal_memcpy(&mpls_tcam->ad_info, field_action.ext_data, sizeof(DsMpls_m));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_add_tcam_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* mpls_info, uint8 scl_id)
{
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    ctc_scl_group_info_t group_info;
    ctc_scl_entry_t scl_entry;
    ds_t data;
    uint32 gid[2] = {0};
    int32 ret = CTC_E_NONE;
    uint8 create_default_grp = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&data, 0, sizeof(ds_t));
    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));

    if (mpls_info->update)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_update_tcam_ilm(lchip, p_mpls_ilm, mpls_info, NULL, scl_id));
        return CTC_E_NONE;
    }

    /*create group*/
    if (1 < scl_id)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }
    gid[0] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM0, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    gid[1] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM1, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
    group_info.priority = scl_id;
    if (0 == p_usw_mpls_master[lchip]->tcam_group[scl_id])
    {
       create_default_grp = 1;
       CTC_ERROR_RETURN(sys_usw_scl_create_group(lchip, gid[scl_id], &group_info, 1));
       p_usw_mpls_master[lchip]->tcam_group[scl_id] = 1;
    }

    /*add entry*/
    scl_entry.key_type = SYS_SCL_KEY_HASH_TUNNEL_MPLS;
    scl_entry.action_type = SYS_SCL_ACTION_MPLS;
    scl_entry.resolve_conflict = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid[scl_id], &scl_entry,1), ret, error_1);
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL;
    field_key.mask = 0xfffff;
    field_key.data = p_mpls_ilm->label;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL_SPACE;
    field_key.mask = 0xfff;
    field_key.data= p_mpls_ilm->spaceid;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_IS_INTERFACEID;
    field_key.data = 0;
    field_key.mask = 1;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);

    /*add ad*/
    mpls_info->p_new_ad = (DsMpls_m*)&data;
    CTC_ERROR_GOTO(_sys_usw_mpls_ilm_set_default_action(lchip, mpls_info), ret, error_2);
    CTC_ERROR_GOTO(_sys_usw_mpls_ilm_common(lchip, p_mpls_ilm, mpls_info), ret, error_2);
    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_MPLS;
    field_action.ext_data = &data;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
    /*install entry*/
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error_2);
    return ret;
    error_2:
        sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);
    error_1:
        if (create_default_grp)
        {
            sys_usw_scl_destroy_group(lchip, gid[scl_id], 1);
            p_usw_mpls_master[lchip]->tcam_group[scl_id] = 0;
        }
    return ret;
}

/*
Brief: if p_ad == NULL, means ad should construct by new interface, for mpls update function;
       else ad is constructed new interface just use directlt, for mpls set property function;
*/

STATIC int32
_sys_usw_mpls_update_tcam_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* mpls_info, void* p_ad, uint8 scl_id)
{
    ctc_scl_field_action_t field_action;
    ds_t data;
    sys_mpls_ilm_hash_t *p_mpls_hash;
    ctc_scl_entry_t* p_scl_entry = NULL;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    sys_mpls_ilm_tcam_t mpls_tcam;
    int32 ret = CTC_E_NONE;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&data, 0, sizeof(ds_t));
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&mpls_tcam, 0, sizeof(sys_mpls_ilm_tcam_t));

    p_mpls_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, 1);
    if (!p_mpls_hash)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (0 == scl_id)
    {
         CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 0));
    }
    else if (1 == scl_id)
    {
         CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 1));
    }
    else
    {
         SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
         return CTC_E_INVALID_PARAM;
    }

    if (!p_ad)
    {
        /*build ad info*/
        mpls_info->p_new_ad = (DsMpls_m*)&data;
        mpls_info->p_old_ad = &mpls_tcam.ad_info;
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_set_default_action(lchip, mpls_info));
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_common(lchip, p_mpls_ilm, mpls_info));
    }
    else
    {
        sal_memcpy(&data, p_ad, sizeof(DsMpls_m));
    }

    p_scl_entry = (ctc_scl_entry_t*)mem_malloc(MEM_MPLS_MODULE, sizeof(ctc_scl_entry_t));
    if (NULL == p_scl_entry)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_scl_entry, 0, sizeof(ctc_scl_entry_t));
    p_scl_entry->entry_id = mpls_tcam.entry_id;

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_MPLS;
    field_action.ext_data = &data;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, out);
    /*install entry*/
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, p_scl_entry->entry_id, 1), ret, out);

out:
    if (p_scl_entry)
    {
        mem_free(p_scl_entry);
    }

    return ret;
}

STATIC int32
_sys_usw_mpls_remove_tcam_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint8 scl_id)
{
    ctc_scl_entry_t scl_entry;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    uint32 gid = 0;
    sal_memset(&lkup_key, 0, sizeof(sys_scl_lkup_key_t));
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if (0 == scl_id)
    {
        gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM0, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
        lkup_key.group_priority = 0;
    }
    else if (1 == scl_id)
    {
        gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_MPLS, SYS_MPLS_GROUP_SUB_TYPE_TCAM1, CTC_FIELD_PORT_TYPE_NONE, 0, 0);
        lkup_key.group_priority = 1;
    }
    else
    {
         SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
         return CTC_E_INVALID_PARAM;
    }
    lkup_key.group_id = gid;
    lkup_key.resolve_conflict = 1;
    lkup_key.key_type = SYS_SCL_KEY_HASH_TUNNEL_MPLS;
    lkup_key.action_type = SYS_SCL_ACTION_MPLS;

    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL;
    field_key.mask = 0xfffff;
    field_key.data = p_mpls_ilm->label;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_LABEL_SPACE;
    field_key.mask = 0xfff;
    field_key.data = p_mpls_ilm->spaceid;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_MPLS_IS_INTERFACEID;
    field_key.mask = 1;
    field_key.data = 0;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));
    field_key.type = SYS_SCL_FIELD_KEY_COMMON;
    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key));
    scl_entry.entry_id = lkup_key.entry_id;
    CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1));
    CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_half_check(uint8 lchip, DsMpls_m* p_dsmpls)
{
    DsMplsHalf_m empty_half;
    DsMplsHalf_m* p_2nd_half = NULL;
    uint32 is_half = 0;

    if (DRV_IS_TSINGMA(lchip))
    {
        return CTC_E_NONE;
    }
    sal_memset(&empty_half, 0, sizeof(DsMplsHalf_m));
    p_2nd_half = (DsMplsHalf_m*)p_dsmpls + 1;
    if (!sal_memcmp(&empty_half, p_2nd_half, sizeof(DsMplsHalf_m)))
    {
        is_half = 1;
    }
    SetDsMpls(V, isHalf_f, p_dsmpls, is_half);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_get_ad_offset(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        _sys_usw_mpls_get_key_ad(lchip, p_ilm_info->key_offset, &p_ilm_info->ad_offset);
    }
    else
    {
        p_ilm_info->ad_offset = p_ilm_info->key_offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_map_pw_ctc2sys(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_ctc, ctc_mpls_ilm_t *p_sys)
{
    p_sys->label                   = p_ctc->label;
    p_sys->pwid= p_ctc->logic_port;

    p_sys->flw_vrf_srv_aps.service_id  = p_ctc->service_id;
    p_sys->aps_select_grp_id       = p_ctc->aps_select_grp_id;

    p_sys->id_type                 = p_ctc->id_type;
    p_sys->spaceid                 = p_ctc->space_id;
    p_sys->cwen                    = p_ctc->cwen;

    p_sys->decap                   = 1;
    p_sys->aps_select_protect_path = p_ctc->aps_select_protect_path;
    p_sys->logic_port_type         = p_ctc->u.vpls_info.logic_port_type;

    p_sys->oam_en                  = p_ctc->oam_en;
    p_sys->l2vpn_oam_id    = p_ctc->l2vpn_oam_id;
    p_sys->stats_id                = p_ctc->stats_id;

    p_sys->qos_use_outer_info      = p_ctc->qos_use_outer_info;

    p_sys->svlan_tpid_index        = p_ctc->svlan_tpid_index;

    p_sys->pw_mode                 = p_ctc->pw_mode;

    p_sys->gport                  = p_ctc->gport;
    p_sys->inner_pkt_type = p_ctc->inner_pkt_type;
    p_sys->learn_disable = p_ctc->learn_disable;

    if (p_ctc->service_aclqos_enable)
    {
        CTC_SET_FLAG(p_sys->flag, CTC_MPLS_ILM_FLAG_SERVICE_ACL_EN);
    }

    if (CTC_FLAG_ISSET(p_ctc->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT))
    {
        CTC_SET_FLAG(p_sys->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);
    }

    if (p_ctc->l2vpntype == CTC_MPLS_L2VPN_VPWS)
    {
        p_sys->type =  CTC_MPLS_LABEL_TYPE_VPWS;
        p_sys->nh_id                   = p_ctc->u.pw_nh_id;
    }
    else if (p_ctc->l2vpntype == CTC_MPLS_L2VPN_VPLS)
    {
        p_sys->type =  CTC_MPLS_LABEL_TYPE_VPLS;
        p_sys->fid                     = p_ctc->u.vpls_info.fid;
    }

    if (CTC_FLAG_ISSET(p_ctc->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM))
    {
        CTC_SET_FLAG(p_sys->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM);
    }

    if (CTC_FLAG_ISSET(p_ctc->flag, CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        CTC_SET_FLAG(p_sys->flag, CTC_MPLS_ILM_FLAG_USE_FLEX);
    }
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_mpls_ilm_write_key(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    DsUserIdTunnelMplsHashKey_m key;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "    spaceid = %d  label:%d,\n",
                  p_mpls_ilm->spaceid, p_mpls_ilm->label);


    sal_memset(&key, 0, sizeof(DsUserIdTunnelMplsHashKey_m));

    if (DRV_IS_DUET2(lchip))
    {
        SetDsUserIdTunnelMplsHashKey(V, hashType_f,      &key, USERIDHASHTYPE_TUNNELMPLS);
        SetDsUserIdTunnelMplsHashKey(V, isInterfaceId_f, &key, 0);
        SetDsUserIdTunnelMplsHashKey(V, labelSpace_f,    &key, p_mpls_ilm->spaceid);
        SetDsUserIdTunnelMplsHashKey(V, mplsLabel_f,     &key, p_mpls_ilm->label);
        SetDsUserIdTunnelMplsHashKey(V, dsAdIndex_f,     &key, p_ilm_info->ad_offset);
        SetDsUserIdTunnelMplsHashKey(V, valid_f,         &key, 1);
    }
    else
    {
        SetDsMplsLabelHashKey(V, isInterfaceId_f, &key, 0);
        SetDsMplsLabelHashKey(V, labelSpace_f,    &key, p_mpls_ilm->spaceid);
        SetDsMplsLabelHashKey(V, mplsLabel_f,     &key, p_mpls_ilm->label);
        SetDsMplsLabelHashKey(V, valid_f,         &key, 1);
    }

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Mpls Hash key index:%d\n", p_ilm_info->key_offset);

    return _sys_usw_mpls_add_key(lchip, &key, p_ilm_info->key_offset);
}

/*
    the function actually just look up the key index
*/
STATIC int32
_sys_usw_mpls_ilm_get_key_index(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    DsUserIdTunnelMplsHashKey_m key;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&key, 0, sizeof(DsUserIdTunnelMplsHashKey_m));

    if (DRV_IS_DUET2(lchip))
    {
        SetDsUserIdTunnelMplsHashKey(V, hashType_f,         &key, USERIDHASHTYPE_TUNNELMPLS);
        SetDsUserIdTunnelMplsHashKey(V, isInterfaceId_f,    &key, 0);
        SetDsUserIdTunnelMplsHashKey(V, labelSpace_f,       &key, p_mpls_ilm->spaceid);
        SetDsUserIdTunnelMplsHashKey(V, mplsLabel_f,        &key, p_mpls_ilm->label);
        SetDsUserIdTunnelMplsHashKey(V, dsAdIndex_f,        &key, 0);
        SetDsUserIdTunnelMplsHashKey(V, valid_f,            &key, 1);
    }
    else
    {
        SetDsMplsLabelHashKey(V, isInterfaceId_f, &key, 0);
        SetDsMplsLabelHashKey(V, labelSpace_f,    &key, p_mpls_ilm->spaceid);
        SetDsMplsLabelHashKey(V, mplsLabel_f,     &key, p_mpls_ilm->label);
        SetDsMplsLabelHashKey(V, valid_f,         &key, 1);
    }

    return _sys_usw_mpls_lookup_key(lchip, &key, &p_ilm_info->key_offset);
}

STATIC int32
_sys_usw_mpls_ilm_alloc_key_index(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    DsUserIdTunnelMplsHashKey_m key;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&key, 0, sizeof(DsUserIdTunnelMplsHashKey_m));

    if (DRV_IS_DUET2(lchip))
    {

        SetDsUserIdTunnelMplsHashKey(V, hashType_f,         &key, USERIDHASHTYPE_TUNNELMPLS);
        SetDsUserIdTunnelMplsHashKey(V, isInterfaceId_f,    &key, 0);
        SetDsUserIdTunnelMplsHashKey(V, labelSpace_f,       &key, p_mpls_ilm->spaceid);
        SetDsUserIdTunnelMplsHashKey(V, mplsLabel_f,        &key, p_mpls_ilm->label);
        SetDsUserIdTunnelMplsHashKey(V, dsAdIndex_f,        &key, 0);
        SetDsUserIdTunnelMplsHashKey(V, valid_f,            &key, 1);
    }
    else
    {
        SetDsMplsLabelHashKey(V, isInterfaceId_f, &key, 0);
        SetDsMplsLabelHashKey(V, labelSpace_f,    &key, p_mpls_ilm->spaceid);
        SetDsMplsLabelHashKey(V, mplsLabel_f,     &key, p_mpls_ilm->label);
        SetDsMplsLabelHashKey(V, valid_f,         &key, 1);
    }

    return _sys_usw_mpls_alloc_key(lchip, &key, &p_ilm_info->key_offset);
}

STATIC int32
_sys_usw_mpls_update_ad(uint8 lchip, void* data, void* change_nh_param)
{
    sys_mpls_ilm_hash_t* p_ilm_info = data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    DsMpls_m   dsmpls;
    uint32 cmdr;
    uint32 cmdw;
    uint32 fwd_offset = 0;
    uint8 is_half = 0;
    sys_mpls_ilm_spool_t dsmpls_new;
    sys_mpls_ilm_spool_t dsmpls_old;
    sys_mpls_ilm_spool_t* p_dsmpls_get =NULL;
    ctc_mpls_ilm_t mpls_ilm;
    sys_mpls_ilm_t ilm_info;
    uint32 ad_index_temp = 0;
    uint8 adjust_len_idx = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dsmpls, 0, sizeof(dsmpls));
    sal_memset(&dsmpls_new, 0, sizeof(sys_mpls_ilm_spool_t));
    sal_memset(&dsmpls_old, 0, sizeof(sys_mpls_ilm_spool_t));

    if (DRV_IS_DUET2(lchip))
    {
        ad_index_temp = p_ilm_info->ad_index;
    }
    else
    {
        ad_index_temp = (p_ilm_info->ad_index > 64) ? (p_ilm_info->ad_index - 64) : p_ilm_info->ad_index;
    }

    if (ad_index_temp & 0x1)
    {
        /*must half*/
        cmdr = DRV_IOR(DsMplsHalf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index_temp, cmdr, &dsmpls));
    }
    else
    {
        /*maybe half or full, read as full ad*/
        cmdr = DRV_IOR(DsMpls_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ad_index_temp, cmdr, &dsmpls));
    }
    if (GetDsMpls(V, isHalf_f, &dsmpls))
    {
        /*clear second half ad*/
        sal_memset(((uint32*)&dsmpls+3), 0, sizeof(DsMplsHalf_m));
    }
    is_half = GetDsMpls(V, isHalf_f, &dsmpls);
    dsmpls_old.is_half = is_half;
    sal_memcpy(&dsmpls_old.spool_ad, &dsmpls, sizeof(DsMpls_m));

    if (!p_dsnh_info)
    {
        /*update ad from merge dsfwd to using dsfwd*/
        /*clear u2*/
        SetDsMpls(V, adDestMap_f, &dsmpls, 0);
        SYS_SET_UNION_TYPE(p_ilm_info->u2_type,SYS_AD_UNION_G_2,DsMpls,2,&dsmpls, 0);
        /*clear u4*/
        SetDsMpls(V, adNextHopPtr_f, &dsmpls, 0);
        SYS_SET_UNION_TYPE(p_ilm_info->u4_type,SYS_AD_UNION_G_4,DsMpls,4,&dsmpls, 0);

        SYS_CHECK_UNION_BITMAP(p_ilm_info->u4_type, SYS_AD_UNION_G_1);

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_ilm_info->nh_id, &fwd_offset, 1));

        /*2. update ad*/
        SetDsMpls(V, nextHopPtrValid_f, &dsmpls, 0);
        SetDsMpls(V, adDestMap_f, &dsmpls, 0);
        SetDsMpls(V, adNextHopPtr_f, &dsmpls, 0);
        SetDsMpls(V, adLengthAdjustType_f, &dsmpls, 0);
        SetDsMpls(V, dsFwdPtr_f, &dsmpls, fwd_offset);
        SetDsMpls(V, dsFwdPtrSelect_f, &dsmpls, 1);

        SYS_SET_UNION_TYPE(p_ilm_info->u4_type,SYS_AD_UNION_G_1,DsMpls,4,&dsmpls, 1);

        p_ilm_info->bind_dsfwd = 0;
    }
    else
    {
        if(GetDsMpls(V, nextHopPtrValid_f, &dsmpls) == 0)
        {
             return CTC_E_NONE;
        }
        SetDsMpls(V, adApsBridgeEn_f, &dsmpls, p_dsnh_info->aps_en);
        SetDsMpls(V, adDestMap_f, &dsmpls, p_dsnh_info->dest_map);
        SetDsMpls(V, adNextHopExt_f, &dsmpls, p_dsnh_info->nexthop_ext);
        SetDsMpls(V, adNextHopPtr_f, &dsmpls, p_dsnh_info->dsnh_offset);
        if(0 != p_dsnh_info->adjust_len)
        {
             sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
             SetDsMpls(V, adLengthAdjustType_f, &dsmpls, adjust_len_idx);
        }
        else
        {
            SetDsMpls(V, adLengthAdjustType_f, &dsmpls, 0);
        }
        SYS_SET_UNION_TYPE(p_ilm_info->u2_type,SYS_AD_UNION_G_2,DsMpls,2,&dsmpls, 1);
        SYS_SET_UNION_TYPE(p_ilm_info->u4_type,SYS_AD_UNION_G_4,DsMpls,4,&dsmpls, 1);
    }

    _sys_usw_mpls_ilm_half_check(lchip, &dsmpls);
    is_half = GetDsMpls(V, isHalf_f, &dsmpls);

    if (DRV_IS_DUET2(lchip))
    {
        dsmpls_new.is_half = is_half;
        sal_memcpy(&dsmpls_new.spool_ad, &dsmpls, sizeof(DsMpls_m));

        /*update spool*/
        CTC_ERROR_RETURN(ctc_spool_add(p_usw_mpls_master[lchip]->ad_spool, &dsmpls_new, &dsmpls_old, &p_dsmpls_get));

        if ((p_dsmpls_get->ad_index) != (ad_index_temp))
        {
            mpls_ilm.spaceid = p_ilm_info->spaceid;
            mpls_ilm.label = p_ilm_info->label;
            _sys_usw_mpls_ilm_get_key_index(lchip, &mpls_ilm, &ilm_info);
            ilm_info.ad_offset = p_dsmpls_get->ad_index;
            _sys_usw_mpls_ilm_write_key(lchip, &mpls_ilm, &ilm_info);
        }
        ad_index_temp = p_dsmpls_get->ad_index;
    }

    if (is_half)
    {
        cmdw = DRV_IOW(DsMplsHalf_t, DRV_ENTRY_FLAG);
    }
    else
    {
        cmdw = DRV_IOW(DsMpls_t, DRV_ENTRY_FLAG);
    }

    DRV_IOCTL(lchip, ad_index_temp, cmdw, &dsmpls);

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "MPLS update_ad: p_ilm_info->ad_index:0x%x    \n", ad_index_temp);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_bind_nexthop(uint8 lchip, uint32 nh_id, sys_mpls_ilm_t* p_ilm_info, uint8 is_bind)
{
    sys_nh_update_dsnh_param_t update_dsnh;
	int32 ret = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    /* host route, write dsnh to dsmpls */
    update_dsnh.data = is_bind?p_ilm_info->p_cur_hash:NULL;
    update_dsnh.updateAd = is_bind?_sys_usw_mpls_update_ad:NULL;
    ret = sys_usw_nh_bind_dsfwd_cb(lchip, nh_id, &update_dsnh);
    if (CTC_E_NONE == ret)
    {
        if (is_bind)
        {
           p_ilm_info->p_cur_hash->bind_dsfwd = 1;
        }
        else
        {
           p_ilm_info->p_cur_hash->bind_dsfwd = 0;
        }
    }

    return ret;
}

STATIC int32
_sys_usw_mpls_ilm_set_nhinfo(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    sys_nh_info_dsnh_t nhinfo;
    uint32  fwd_offset;
    uint32 nh_id = p_mpls_ilm->nh_id;
    uint8  have_dsfwd = 0;
    int32 ret = 0;
    uint8 vpws_oam_en = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));

    if ((TRUE == p_mpls_ilm->pop) && (p_mpls_ilm->nh_id == 0))
    {   /* there's no label in the label stack, drop the pkt */
        nh_id = 1;
    }

    if (p_ilm_info->p_cur_hash  && p_ilm_info->p_cur_hash->nh_id &&  p_ilm_info->p_cur_hash->bind_dsfwd  )
    {
        _sys_usw_mpls_bind_nexthop(lchip, p_ilm_info->p_cur_hash->nh_id, p_ilm_info, 0);
    }

    if ((CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM) && (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_VPWS)))
    {
        vpws_oam_en = 1;
    }

    p_ilm_info->flag = p_mpls_ilm->flag;
    p_ilm_info->gport = p_mpls_ilm->gport;

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nh_id, &nhinfo, 0));

    have_dsfwd = (nhinfo.dsfwd_valid ||(nhinfo.merge_dsfwd == 2) || nhinfo.ecmp_valid || vpws_oam_en ||
        CTC_FLAG_ISSET(p_mpls_ilm->id_type, CTC_MPLS_ID_VRF)) &&
        !CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);

    if(NULL != p_ilm_info->p_cur_hash)
    {
        have_dsfwd |= (p_ilm_info->p_cur_hash->u2_type == SYS_AD_UNION_G_1);
    }


    if (!have_dsfwd && !CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT) && p_ilm_info->p_cur_hash)
    {
        /*do not using dsfwd, do nexthop and ipuc bind*/
        ret = _sys_usw_mpls_bind_nexthop(lchip, nh_id, p_ilm_info, 1);
        if (CTC_E_IN_USE == ret)
        {
            /*nh have already bind, do unbind, using dsfwd*/
            have_dsfwd = 1;
        }
        else if (ret < 0)
        {
            /*Bind fail, using dsfwd*/
            have_dsfwd = 1;
        }
    }

    if (p_mpls_ilm->fid && have_dsfwd)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "fid and fwdptr have parameter conflict!\n");
        return CTC_E_PARAM_CONFLICT;
    }

    if (p_mpls_ilm->pop)
    {
        SetDsMpls(V, continuing_f, p_dsmpls, TRUE);
    }
    else
    {
        SetDsMpls(V, continuing_f, p_dsmpls, 0);
    }
    if (nhinfo.ecmp_valid)
    {
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u2_type, SYS_AD_UNION_G_1);
        SetDsMpls(V, ecmpGroupIdEn_f, p_dsmpls, 1);
        SetDsMpls(V, secondFid_f, p_dsmpls, nhinfo.ecmp_group_id);
        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,p_dsmpls, 1);
    }
    else if (have_dsfwd)
    {
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_1);
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, nh_id, &fwd_offset, 0));
        SetDsMpls(V, dsFwdPtrSelect_f, p_dsmpls, 1);
        SetDsMpls(V, dsFwdPtr_f, p_dsmpls, fwd_offset);
        SetDsMpls(V, nextHopPtrValid_f, p_dsmpls, 0);
        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_1,DsMpls,4,p_dsmpls, 1);
    }
    else
    {
        uint8 adjust_len_idx = 0;
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_4);
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u2_type, SYS_AD_UNION_G_2);

        /* for pop, if the config the nexthop, need to write the hw tbl */
        if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT))
        {
            SetDsMpls(V, adDestMap_f, p_dsmpls,
                SYS_ENCODE_DESTMAP(SYS_MAP_CTC_GPORT_TO_GCHIP(p_mpls_ilm->gport),
                                   SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_mpls_ilm->gport)));
        }
        else
        {
            SetDsMpls(V, adDestMap_f, p_dsmpls, nhinfo.dest_map);
        }
        SetDsMpls(V, adNextHopExt_f, p_dsmpls, nhinfo.nexthop_ext);
        SetDsMpls(V, adNextHopPtr_f,p_dsmpls, nhinfo.dsnh_offset);
        SetDsMpls(V, adApsBridgeEn_f, p_dsmpls, nhinfo.aps_en);
        SetDsMpls(V, nextHopPtrValid_f, p_dsmpls, 1);
        if(0 != nhinfo.adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, nhinfo.adjust_len, &adjust_len_idx);
            SetDsMpls(V, adLengthAdjustType_f, p_dsmpls, adjust_len_idx);
        }
        else
        {
            SetDsMpls(V, adLengthAdjustType_f, p_dsmpls, 0);
        }

        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_2,DsMpls,2,p_dsmpls, 1);
        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_4,DsMpls,4,p_dsmpls, 1);
    }

     p_ilm_info->p_cur_hash->nh_id = nh_id;
    return CTC_E_NONE;
}



STATIC sys_mpls_ilm_hash_t*
_sys_usw_mpls_ilm_get_hash(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint8 is_tcam)
{
    sys_mpls_ilm_hash_t db_hash_info;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&db_hash_info, 0, sizeof(sys_mpls_ilm_hash_t));

    db_hash_info.label = p_mpls_ilm->label;
    db_hash_info.spaceid = p_mpls_ilm->spaceid;
    db_hash_info.is_tcam = is_tcam;

    return _sys_usw_mpls_db_lookup_hash(lchip, &db_hash_info);
}

STATIC int32
_sys_usw_mpls_lookup_key_by_label(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint32* key_index, uint32* ad_index)
{
    int32 ret = CTC_E_NONE;
    sys_mpls_ilm_t ilm_info;
    ctc_mpls_ilm_t mpls_ilm;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "spaceid = %d  label:%d,\n",p_mpls_ilm->spaceid, p_mpls_ilm->label);

    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));

    mpls_ilm.spaceid = p_mpls_ilm->spaceid;
    mpls_ilm.label= p_mpls_ilm->label;

    ret = _sys_usw_mpls_ilm_get_key_index(lchip, &mpls_ilm, &ilm_info);
    if (CTC_E_EXIST != ret)
    {
        CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
    }

    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_get_ad_offset(lchip, &mpls_ilm, &ilm_info));
    *key_index = ilm_info.key_offset;
    *ad_index = ilm_info.ad_offset;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_map_ilm_sys2ctc(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_hash_t* p_db_hashkey)
{
    DsMpls_m* p_spool_ad = NULL;
    sys_mpls_ilm_t ilm_info;
    DsMpls_m dsmpls;
    sys_mpls_ilm_tcam_t mpls_tcam;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&mpls_tcam, 0, sizeof(sys_mpls_ilm_tcam_t));

    if (!p_db_hashkey->is_tcam)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_lookup_key_by_label(lchip, p_mpls_ilm,
                                        &ilm_info.key_offset, &ilm_info.ad_offset));
        CTC_ERROR_RETURN(_sys_usw_mpls_read_dsmpls(lchip, ilm_info.ad_offset, &dsmpls));
        p_spool_ad = &dsmpls;
    }
    else if (DRV_IS_DUET2(lchip))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_USE_FLEX);
        CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 0));
        sal_memcpy(&dsmpls, &mpls_tcam.ad_info, sizeof(DsMpls_m));
        p_spool_ad = &dsmpls;
    }

    if (GetDsMpls(V, vplsOam_f, p_spool_ad))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM);
    }

    if (GetDsMpls(V, serviceAclQosEn_f, p_spool_ad))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_SERVICE_ACL_EN);
    }

    if (GetDsMpls(V, aclQosUseOuterInfo_f, p_spool_ad))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO);
    }

    if (GetDsMpls(V, bosAction_f, p_spool_ad))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_BOS_LABLE);
    }

    if (1 == GetDsMpls(V, entropyLabelMode_f, p_spool_ad))
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT);
    }

    if (p_db_hashkey->is_assignport)
    {
        CTC_SET_FLAG(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);
    }

    p_mpls_ilm->pwid = GetDsMpls(V, logicSrcPort_f, p_spool_ad);
    p_mpls_ilm->id_type = p_db_hashkey->id_type;
    p_mpls_ilm->type = p_db_hashkey->type;
    p_mpls_ilm->model = p_db_hashkey->model;
    p_mpls_ilm->cwen = GetDsMpls(V, cwExist_f, p_spool_ad);
    p_mpls_ilm->pop = GetDsMpls(V, continuing_f, p_spool_ad);
    p_mpls_ilm->nh_id = p_db_hashkey->nh_id;
    p_mpls_ilm->decap = GetDsMpls(V, innerPacketLookup_f, p_spool_ad);
    p_mpls_ilm->pw_mode = GetDsMpls(V, outerVlanIsCVlan_f, p_spool_ad);
    p_mpls_ilm->stats_id = p_db_hashkey->stats_id;
    p_mpls_ilm->flw_vrf_srv_aps.service_id = p_db_hashkey->service_id;
    p_mpls_ilm->l2vpn_oam_id = p_db_hashkey->oam_id;

    if(p_mpls_ilm->id_type & CTC_MPLS_ID_FLOW)
    {
        /*p_mpls_ilm->flw_vrf_srv_aps.flow_id = p_ilm_data->flow_id;*/
    }

    if(p_mpls_ilm->id_type & CTC_MPLS_ID_VRF)
    {
        if ((CTC_MPLS_LABEL_TYPE_GATEWAY != p_db_hashkey->type) && (SYS_AD_UNION_G_2 == p_db_hashkey->u4_type))
        {
            p_mpls_ilm->vrf_id = GetDsMpls(V, fid_f, p_spool_ad);
        }
        else if (SYS_AD_UNION_G_1 == p_db_hashkey->u2_type)
        {
            p_mpls_ilm->vrf_id = GetDsMpls(V, secondFid_f, p_spool_ad);
        }
    }

    if(p_mpls_ilm->id_type & CTC_MPLS_ID_SERVICE)
    {
        /*p_mpls_ilm->flw_vrf_srv_aps.service_id = p_ilm_data->service_id;*/
    }

    if(p_mpls_ilm->id_type & CTC_MPLS_ID_APS_SELECT)
    {
        p_mpls_ilm->flw_vrf_srv_aps.aps_select_grp_id = GetDsMpls(V, apsSelectGroupId_f, p_spool_ad);
        p_mpls_ilm->aps_select_grp_id = GetDsMpls(V, apsSelectGroupId_f, p_spool_ad);
        p_mpls_ilm->aps_select_protect_path = GetDsMpls(V, apsSelectProtectingPath_f, p_spool_ad);
    }

    p_mpls_ilm->fid = (SYS_AD_UNION_G_2 == p_db_hashkey->u4_type) ? GetDsMpls(V, fid_f, p_spool_ad) : 0;
    p_mpls_ilm->out_intf_spaceid = (SYS_AD_UNION_G_3 == p_db_hashkey->u4_type) ? GetDsMpls(V, mplsLabelSpace_f, p_spool_ad) : 0;

    p_mpls_ilm->oam_en = GetDsMpls(V, vplsOam_f, p_spool_ad);
    p_mpls_ilm->qos_use_outer_info = GetDsMpls(V, aclQosUseOuterInfo_f, p_spool_ad);
    p_mpls_ilm->logic_port_type         = GetDsMpls(V, logicPortType_f, p_spool_ad);
    p_mpls_ilm->svlan_tpid_index = GetDsMpls(V, svlanTpidIndex_f, p_spool_ad);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_ilm_add_hash(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    sys_nh_info_dsnh_t nh_info;
    p_ilm_info->p_cur_hash->label = p_mpls_ilm->label;
    p_ilm_info->p_cur_hash->nh_id = p_mpls_ilm->nh_id;
    p_ilm_info->p_cur_hash->spaceid = p_mpls_ilm->spaceid;
    p_ilm_info->p_cur_hash->stats_id = p_mpls_ilm->stats_id;
    p_ilm_info->p_cur_hash->model = p_mpls_ilm->model;
    p_ilm_info->p_cur_hash->type = p_mpls_ilm->type;
    p_ilm_info->p_cur_hash->id_type = p_mpls_ilm->id_type;
    p_ilm_info->p_cur_hash->service_id = p_mpls_ilm->flw_vrf_srv_aps.service_id;
    p_ilm_info->p_cur_hash->is_assignport = CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);
    if ((CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (p_mpls_ilm->l2vpn_oam_id))
    {
        if (DRV_IS_DUET2(lchip))
        {
            p_ilm_info->p_cur_hash->is_vpws_oam = 1;
            sys_usw_nh_get_nhinfo(lchip, p_mpls_ilm->nh_id, &nh_info, 0);
            p_ilm_info->p_cur_hash->gport = nh_info.gport;
            sys_usw_oam_add_oamid(lchip, nh_info.gport, p_mpls_ilm->l2vpn_oam_id, p_mpls_ilm->label, p_mpls_ilm->spaceid);
        }
        p_ilm_info->p_cur_hash->oam_id = p_mpls_ilm->l2vpn_oam_id;
    }

    return _sys_usw_mpls_db_add_hash(lchip, p_ilm_info->p_cur_hash);
}

STATIC int32
_sys_usw_mpls_ilm_update_hash(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_hash_t* p_hash)
{
    sys_nh_info_dsnh_t nh_info;
    uint32 new_gport = 0;
    if (!p_hash)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
        return CTC_E_EXIST;
    }
    if ((CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (p_mpls_ilm->l2vpn_oam_id))
    {
        if (DRV_IS_DUET2(lchip))
        {
            sys_usw_nh_get_nhinfo(lchip, p_mpls_ilm->nh_id, &nh_info, 0);
            new_gport = nh_info.gport;
            if ((p_hash->is_vpws_oam) && ((new_gport != p_hash->gport) || (p_hash->oam_id != p_mpls_ilm->l2vpn_oam_id)))
            {
                sys_usw_oam_remove_oamid(lchip, p_hash->gport, p_hash->oam_id, p_hash->label, p_hash->spaceid);
            }
            sys_usw_oam_add_oamid(lchip, new_gport, p_mpls_ilm->l2vpn_oam_id, p_mpls_ilm->label, p_mpls_ilm->spaceid);
            p_hash->is_vpws_oam = 1;
            p_hash->gport = new_gport;
        }
        p_hash->oam_id = p_mpls_ilm->l2vpn_oam_id;
    }
    p_hash->nh_id = p_mpls_ilm->nh_id;
    p_hash->stats_id = p_mpls_ilm->stats_id;
    p_hash->model = p_mpls_ilm->model;
    p_hash->type = p_mpls_ilm->type;
    p_hash->id_type = p_mpls_ilm->id_type;
    p_hash->service_id = p_mpls_ilm->flw_vrf_srv_aps.service_id;
    p_hash->is_assignport = CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_remove_hash(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint8 is_tcam)
{
    sys_mpls_ilm_hash_t db_hash_info;
    sys_mpls_ilm_hash_t* p_cur_hash;

    sal_memset(&db_hash_info, 0, sizeof(sys_mpls_ilm_hash_t));

    db_hash_info.is_interface = 0;
    db_hash_info.label = p_mpls_ilm->label;
    db_hash_info.spaceid = p_mpls_ilm->spaceid;
    db_hash_info.is_tcam = is_tcam;

    p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, is_tcam);
    if (p_cur_hash && p_cur_hash->is_vpws_oam && DRV_IS_DUET2(lchip))
    {
        sys_usw_oam_remove_oamid(lchip, p_cur_hash->gport, p_cur_hash->oam_id, p_cur_hash->label, p_cur_hash->spaceid);
    }

    return _sys_usw_mpls_db_remove_hash(lchip, &db_hash_info);
}

STATIC int32
_sys_usw_mpls_get_ilm_property(uint8 lchip, ctc_mpls_property_t* p_prop_info, DsMpls_m* p_ad, sys_mpls_ilm_hash_t type_check)
{
    uint8 mac_limit_action_bitmap = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "property type:%u\n", p_prop_info->property_type);

    switch(p_prop_info->property_type)
    {
        case SYS_MPLS_ILM_QOS_DOMAIN:
            *((uint32*)p_prop_info->value) =  GetDsMpls(V, uMplsPhb_gElsp_mplsTcPhbPtr_f, p_ad);
            break;

        case SYS_MPLS_ILM_APS_SELECT:
        {
            ctc_mpls_ilm_t* mpls_ilm = (ctc_mpls_ilm_t*)p_prop_info->value;

            if (GetDsMpls(V, apsSelectValid_f, p_ad))
            {
                mpls_ilm->aps_select_grp_id = GetDsMpls(V, apsSelectGroupId_f, p_ad);
                mpls_ilm->aps_select_protect_path = GetDsMpls(V, apsSelectProtectingPath_f, p_ad);
            }
            else
            {
                mpls_ilm->aps_select_grp_id = 0xFFFF;
            }
        }
            break;

        case SYS_MPLS_ILM_OAM_CHK:
            *((uint32*)p_prop_info->value) =  GetDsMpls(V, oamCheckType_f, p_ad);
            break;

        case SYS_MPLS_ILM_OAM_LMEN:

            *((uint32*)p_prop_info->value) =  GetDsMpls(V, lmEn_f, p_ad);
            break;

        case SYS_MPLS_ILM_OAM_IDX:
            *((uint32*)p_prop_info->value) =  GetDsMpls(V, mplsOamIndex_f, p_ad);
            break;

        case SYS_MPLS_ILM_OAM_VCCV_BFD:

            *((uint32*)p_prop_info->value) =  GetDsMpls(V, cwExist_f, p_ad);
            break;

        case SYS_MPLS_ILM_ROUTE_MAC:
        {
            uint8 route_mac_index = 0;
            uint32 cmd = 0;
            DsRouterMacInner_m  inner_router_mac;
            mac_addr_t* user_mac = (mac_addr_t *)p_prop_info->value;
            uint32 hw_mac[2] = {0};

            route_mac_index = GetDsMpls(V, routerMacProfile_f, p_ad);
            cmd = DRV_IOR(DsRouterMacInner_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, route_mac_index, cmd, &inner_router_mac);
            GetDsRouterMacInner(A, routerMac_f, &inner_router_mac, hw_mac);
            SYS_USW_SET_USER_MAC(*user_mac, hw_mac);
        }
            break;

         case SYS_MPLS_ILM_DENY_LEARNING:
            *((uint32*)p_prop_info->value) =  GetDsMpls(V, denyLearning_f, p_ad);
            break;
         case SYS_MPLS_ILM_ARP_ACTION:
            if (type_check.u2_type != SYS_AD_UNION_G_1)
            {
                *((uint32*)p_prop_info->value) =  0xff;
                break;
            }
            if (GetDsMpls(V, arpCtlEn_f, p_ad))
            {
                *((uint32*)p_prop_info->value) =  GetDsMpls(V, arpExceptionType_f, p_ad);
            }
            else
            {
                *((uint32*)p_prop_info->value) =  0xff;
            }

            break;

         case SYS_MPLS_ILM_QOS_MAP:
        {
            uint8 tc_mode = 0;
            ctc_mpls_ilm_qos_map_t* qos_map = (ctc_mpls_ilm_qos_map_t *)p_prop_info->value;

            tc_mode = GetDsMpls(V, trustMplsTc_f, p_ad);

            if (tc_mode == SYS_MPLS_ILM_PHB_DISABLE)
            {
                qos_map->mode = CTC_MPLS_ILM_QOS_MAP_DISABLE;
            }
            else if (tc_mode == SYS_MPLS_ILM_PHB_RAW)
            {
                qos_map->mode = CTC_MPLS_ILM_QOS_MAP_ASSIGN;
                qos_map->priority = GetDsMpls(V, uMplsPhb_gRaw_mplsPrio_f, p_ad);
                qos_map->color = GetDsMpls(V, uMplsPhb_gRaw_mplsColor_f, p_ad);
            }
            else if (tc_mode == SYS_MPLS_ILM_PHB_ELSP)
            {
                qos_map->mode = CTC_MPLS_ILM_QOS_MAP_ELSP;
                qos_map->exp_domain = GetDsMpls(V, uMplsPhb_gElsp_mplsTcPhbPtr_f, p_ad);
            }
            else if (tc_mode == SYS_MPLS_ILM_PHB_LLSP)
            {
                qos_map->mode = CTC_MPLS_ILM_QOS_MAP_LLSP;
                qos_map->priority = GetDsMpls(V, uMplsPhb_gLlsp_mplsPrio_f, p_ad);
                qos_map->exp_domain = GetDsMpls(V, uMplsPhb_gLlsp_mplsTcPhbPtr_f, p_ad);
            }
        }
            break;
        case SYS_MPLS_STATS_EN:
        {
            *((uint32*)p_prop_info->value) = type_check.stats_id;
        }
            break;
        case SYS_MPLS_ILM_MAC_LIMIT_DISCARD_EN:
            if (DRV_IS_DUET2(lchip))
            {
                *((uint32*)p_prop_info->value) = (type_check.u4_type == SYS_AD_UNION_G_2) ? GetDsMpls(V, macSecurityDiscard_f, p_ad) : 0;
            }
            else
            {
                *((uint32*)p_prop_info->value) = GetDsMpls(V, logicPortMacSecurityDiscard_f, p_ad);
            }
            break;
        case SYS_MPLS_ILM_MAC_LIMIT_ACTION:
            mac_limit_action_bitmap = mac_limit_action_bitmap | (GetDsMpls(V, denyLearning_f, p_ad) << 0);
            mac_limit_action_bitmap = mac_limit_action_bitmap | (GetDsMpls(V, logicPortMacSecurityDiscard_f, p_ad) << 1);
            mac_limit_action_bitmap = mac_limit_action_bitmap | (GetDsMpls(V, logicPortSecurityExceptionEn_f, p_ad) << 2);
            switch (mac_limit_action_bitmap)
            {
                case 0:
                case 4:
                    *((uint32 *)p_prop_info->value) = CTC_MACLIMIT_ACTION_NONE;
                    break;
                case 1:
                case 5:
                    *((uint32 *)p_prop_info->value) = CTC_MACLIMIT_ACTION_FWD;
                    break;
                case 2:
                case 3:
                    *((uint32 *)p_prop_info->value) = CTC_MACLIMIT_ACTION_DISCARD;
                    break;
                case 6:
                case 7:
                    *((uint32 *)p_prop_info->value) = CTC_MACLIMIT_ACTION_TOCPU;
                    break;
                default:
                    break;
            }
            break;
        case SYS_MPLS_ILM_STATION_MOVE_DISCARD_EN:
            *((uint32 *)p_prop_info->value) = GetDsMpls(V, logicPortSecurityEn_f, p_ad);
            break;
        case SYS_MPLS_ILM_METADATA:
            *((uint32 *)p_prop_info->value) = GetDsMpls(V, metadata_f, p_ad);
            break;
        case SYS_MPLS_ILM_DCN_ACTION:
            *((uint32 *)p_prop_info->value) = GetDsMpls(V, dcnCheckType_f, p_ad);
            break;
        case SYS_MPLS_ILM_CID:
            *((uint32*)p_prop_info->value) = GetDsMpls(V, serviceId_f, p_ad);
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_cfg_policer(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls)
{
    uint8 type = 0;
    sys_qos_policer_param_t policer_param;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&policer_param, 0, sizeof(sys_qos_policer_param_t));

    if (p_mpls_ilm->policer_id != 0)
    {
        type = SYS_QOS_POLICER_TYPE_SERVICE;
        CTC_ERROR_RETURN(sys_usw_qos_policer_index_get(lchip, p_mpls_ilm->policer_id,
                                                              type,
                                                              &policer_param));
        if (policer_param.is_bwp)
        {
            SetDsMpls(V, policerPhbEn_f, p_dsmpls, 1);
        }
        else
        {
            SetDsMpls(V, policerPhbEn_f, p_dsmpls, 0);
        }
        SetDsMpls(V, policerLvlSel_f, p_dsmpls, policer_param.level);
        SetDsMpls(V, policerPtr_f, p_dsmpls, policer_param.policer_ptr);
    }
    else
    {
        SetDsMpls(V, policerLvlSel_f, p_dsmpls, 0);
        SetDsMpls(V, policerPtr_f, p_dsmpls, 0);
        SetDsMpls(V, policerPhbEn_f, p_dsmpls, 0);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_normal(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_set_nhinfo(lchip, p_mpls_ilm, p_dsmpls, p_ilm_info));

    /* for normal mpls pkt, we need the condition for pop mode than it'll decap! */
    /* php don't use decap pkt, without php must use inner pkt to transit */
    if (p_mpls_ilm->decap)
    {
        SetDsMpls(V, innerPacketLookup_f, p_dsmpls, TRUE);
        SetDsMpls(V, oamCheckType_f, p_dsmpls, 7);

        /* according to  rfc3270*/
        if (CTC_MPLS_TUNNEL_MODE_SHORT_PIPE != p_mpls_ilm->model)
        {
            SetDsMpls(V, aclQosUseOuterInfo_f, p_dsmpls, 1);
        }
    }

    if (CTC_MPLS_TUNNEL_MODE_UNIFORM != p_mpls_ilm->model)
    {
        SetDsMpls(V, useLabelTtl_f, p_dsmpls, TRUE);
        SetDsMpls(V, ttlUpdate_f, p_dsmpls, TRUE);
    }

    if (CTC_MPLS_INNER_IPV4 == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IPV4);
    }
    else if (CTC_MPLS_INNER_IPV6 == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IPV6);
    }
    else if (CTC_MPLS_INNER_IP == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IP);
    }

    SetDsMpls(V, offsetBytes_f, p_dsmpls, 1);
    SetDsMpls(V, logicSrcPort_f, p_dsmpls, p_mpls_ilm->pwid);
    /*
        note for entropy label and flow label this field must be 0
    */
    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_BOS_LABLE))
    {
        SetDsMpls(V, bosAction_f, p_dsmpls, 1);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_gateway(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (0 != p_mpls_ilm->nh_id)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_set_nhinfo(lchip, p_mpls_ilm, p_dsmpls, p_ilm_info));
    }
    else
    {
        /*
            nexthop is prior. If set, bridge or routing information
            will cover nexthop information
        */
        SetDsMpls(V, innerPacketLookup_f, p_dsmpls, TRUE);
        SetDsMpls(V, oamCheckType_f, p_dsmpls, 7);
    }

    SetDsMpls(V, useLabelTtl_f, p_dsmpls, TRUE);

    if (CTC_MPLS_L2VPN_RAW == p_mpls_ilm->pw_mode)
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, TRUE);
    }
    else
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, FALSE);
    }

    if (CTC_MPLS_INNER_IP == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IP);
    }
    else
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_ETH);
    }

    SetDsMpls(V, sBit_f, p_dsmpls, TRUE);
    SetDsMpls(V, sBitCheckEn_f, p_dsmpls, TRUE);

    if (p_mpls_ilm->fid)
    {
        /*
            0 is invalid fid id
        */
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_2);
        SetDsMpls(V, fidSelect_f, p_dsmpls, 1);
        SetDsMpls(V, dsFwdPtrSelect_f, p_dsmpls, 0);
        SetDsMpls(V, fidType_f, p_dsmpls, TRUE);
        SetDsMpls(V, fid_f, p_dsmpls, p_mpls_ilm->fid);

        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_2,DsMpls,4,p_dsmpls, 1);
    }

    SetDsMpls(V, logicSrcPort_f, p_dsmpls, p_mpls_ilm->pwid);
    SetDsMpls(V, logicPortType_f, p_dsmpls, p_mpls_ilm->logic_port_type);

    if (CTC_FLAG_ISSET(p_mpls_ilm->id_type, CTC_MPLS_ID_VRF))
    {
        SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u2_type, SYS_AD_UNION_G_1);
        SetDsMpls(V, secondFidEn_f, p_dsmpls, 1);
        SetDsMpls(V, secondFid_f, p_dsmpls, p_mpls_ilm->vrf_id);
        SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,p_dsmpls, 1);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_l3vpn(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_MPLS_TUNNEL_MODE_UNIFORM != p_mpls_ilm->model)
    {
        SetDsMpls(V, useLabelTtl_f, p_dsmpls, TRUE);
        SetDsMpls(V, ttlUpdate_f, p_dsmpls, TRUE);
    }

    if (CTC_MPLS_INNER_IPV4 == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IPV4);
    }
    else if (CTC_MPLS_INNER_IPV6 == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IPV6);
    }
    else if (CTC_MPLS_INNER_IP == p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_IP);
    }

    SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_2);

    SetDsMpls(V, offsetBytes_f, p_dsmpls, 1);
    SetDsMpls(V, sBit_f, p_dsmpls, TRUE);
    SetDsMpls(V, sBitCheckEn_f, p_dsmpls, TRUE);
    SetDsMpls(V, fidSelect_f, p_dsmpls, 1);
    SetDsMpls(V, dsFwdPtrSelect_f, p_dsmpls, 0);
    SetDsMpls(V, fidType_f, p_dsmpls, FALSE);
    SetDsMpls(V, fid_f, p_dsmpls, p_mpls_ilm->vrf_id);
    SetDsMpls(V, innerPacketLookup_f, p_dsmpls, TRUE);
    SetDsMpls(V, oamCheckType_f, p_dsmpls, 7);
    SetDsMpls(V, bosAction_f, p_dsmpls, 1);
    SetDsMpls(V, logicSrcPort_f, p_dsmpls, p_mpls_ilm->pwid);

    SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_2,DsMpls,4,p_dsmpls, 1);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_vpws(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    uint8 service_queue_mode = 0;
    uint16 old_service_id = 0;
    uint16 old_pwid = 0;
    uint32 mode = 0;
    uint16 value = 0;
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    old_service_id = p_ilm_info->p_cur_hash->service_id;
    old_pwid = GetDsMpls(V, logicSrcPort_f, p_ilm_info->p_old_ad);
    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_set_nhinfo(lchip, p_mpls_ilm, p_dsmpls, p_ilm_info));
    CTC_ERROR_RETURN(sys_usw_queue_get_service_queue_mode(lchip, &service_queue_mode));
    if (p_mpls_ilm->id_type & CTC_MPLS_ID_SERVICE)
    {
        CTC_ERROR_RETURN(sys_usw_qos_bind_service_logic_srcport(lchip, p_mpls_ilm->flw_vrf_srv_aps.service_id,p_mpls_ilm->pwid));
        if(p_mpls_ilm->pwid != old_pwid)
        {
            sys_usw_qos_unbind_service_logic_srcport(lchip, old_service_id,old_pwid);
        }
        if(service_queue_mode == 1)
        {
            if (DRV_IS_DUET2(lchip))
            {
                value = ((1<<7)|p_mpls_ilm->flw_vrf_srv_aps.service_id);
            }
            else
            {
                value = p_mpls_ilm->flw_vrf_srv_aps.service_id;
            }
            SetDsMpls(V, serviceId_f, p_dsmpls, value);
            SetDsMpls(V, serviceIdShareMode_f, p_dsmpls, 1);
        }
    }
    else
    {
        if(old_service_id != 0 && old_pwid != 0)
        {
            CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_srcport(lchip,old_service_id,old_pwid));
        }
        if(service_queue_mode == 1)
        {
            SetDsMpls(V, serviceId_f, p_dsmpls, 0);
        }
        SetDsMpls(V, serviceIdShareMode_f, p_dsmpls, 0);
    }

    if (CTC_MPLS_L2VPN_RAW == p_mpls_ilm->pw_mode)
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, TRUE);
    }
    else
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, FALSE);
    }

    if (CTC_MPLS_INNER_RAW != p_mpls_ilm->inner_pkt_type)
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_ETH);
    }
    else
    {
        SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_RESERVED);
    }

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_SERVICE_ACL_EN))
    {
        SetDsMpls(V, serviceAclQosEn_f, p_dsmpls, 1);
    }

    SetDsMpls(V, oamCheckType_f, p_dsmpls, 7);
    SetDsMpls(V, useLabelTtl_f, p_dsmpls, TRUE);
    SetDsMpls(V, sBit_f, p_dsmpls, TRUE);
    SetDsMpls(V, sBitCheckEn_f, p_dsmpls, TRUE);
    SetDsMpls(V, logicSrcPort_f, p_dsmpls, p_mpls_ilm->pwid);

    SetDsMpls(V, svlanTpidIndex_f, p_dsmpls, p_mpls_ilm->svlan_tpid_index);
    SetDsMpls(V, bosAction_f, p_dsmpls, 1);

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM))
    {
        SetDsMpls(V, vplsOam_f, p_dsmpls, 1);
        if (DRV_IS_DUET2(lchip))
        {
            sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
            if (0 == p_ilm_info->p_cur_hash->is_tp_oam)
            {
                SetDsMpls(V, mplsOamIndex_f, p_dsmpls, mode ? (p_mpls_ilm->l2vpn_oam_id + 2048) : p_mpls_ilm->l2vpn_oam_id);
            }
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_set_vpws_vpnid(lchip, p_mpls_ilm->nh_id, p_mpls_ilm->l2vpn_oam_id));
        }
    }

    if (p_usw_mpls_master[lchip]->decap_mode)
    {
        SetDsMpls(V, offsetBytes_f, p_dsmpls, 0);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_vpls(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, DsMpls_m* p_dsmpls, sys_mpls_ilm_t* p_ilm_info)
{
    uint8 service_queue_mode = 0;
    uint16 old_service_id = 0;
    uint16 old_pwid = 0;
    uint16 value = 0;
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    old_service_id = p_ilm_info->p_cur_hash->service_id;
    old_pwid = GetDsMpls(V, logicSrcPort_f, p_ilm_info->p_old_ad);
    SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_2);
    CTC_ERROR_RETURN(sys_usw_queue_get_service_queue_mode(lchip, &service_queue_mode));

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_SERVICE)
    {
        CTC_ERROR_RETURN(sys_usw_qos_bind_service_logic_srcport(lchip, p_mpls_ilm->flw_vrf_srv_aps.service_id,p_mpls_ilm->pwid));
        if(p_mpls_ilm->pwid != old_pwid)
        {
            sys_usw_qos_unbind_service_logic_srcport(lchip, old_service_id,old_pwid);
        }
        if(service_queue_mode == 1)
        {
            if (DRV_IS_DUET2(lchip))
            {
                value = ((1<<7)|p_mpls_ilm->flw_vrf_srv_aps.service_id);
            }
            else
            {
                value = p_mpls_ilm->flw_vrf_srv_aps.service_id;
            }
            SetDsMpls(V, serviceId_f, p_dsmpls, value);
            SetDsMpls(V, serviceIdShareMode_f, p_dsmpls, 1);
        }
    }
    else
    {
        if(old_service_id != 0 && old_pwid != 0)
        {
            CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_srcport(lchip,old_service_id,old_pwid));
        }
        if(service_queue_mode == 1)
        {
            SetDsMpls(V, serviceId_f, p_dsmpls, 0);
        }
        SetDsMpls(V, serviceIdShareMode_f, p_dsmpls, 0);
    }

    if (CTC_MPLS_L2VPN_RAW == p_mpls_ilm->pw_mode)
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, TRUE);
    }
    else
    {
        SetDsMpls(V, outerVlanIsCVlan_f, p_dsmpls, FALSE);
    }

    SetDsMpls(V, useLabelTtl_f, p_dsmpls, TRUE);
    SetDsMpls(V, innerPacketLookup_f, p_dsmpls, TRUE);
    SetDsMpls(V, oamCheckType_f, p_dsmpls, 7);
    SetDsMpls(V, sBit_f, p_dsmpls, TRUE);
    SetDsMpls(V, sBitCheckEn_f, p_dsmpls, TRUE);
    SetDsMpls(V, packetType_f, p_dsmpls, PKT_TYPE_ETH);
    SetDsMpls(V, fidSelect_f, p_dsmpls, 1);
    SetDsMpls(V, dsFwdPtrSelect_f, p_dsmpls, 0);
    SetDsMpls(V, fidType_f, p_dsmpls, TRUE);
    SetDsMpls(V, fid_f, p_dsmpls, p_mpls_ilm->fid);
    SetDsMpls(V, logicSrcPort_f, p_dsmpls, p_mpls_ilm->pwid);
    SetDsMpls(V, logicPortType_f, p_dsmpls, p_mpls_ilm->logic_port_type);
    SetDsMpls(V, svlanTpidIndex_f, p_dsmpls, p_mpls_ilm->svlan_tpid_index);
    SetDsMpls(V, bosAction_f, p_dsmpls, 1);

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_L2VPN_OAM))
    {
        SetDsMpls(V, vplsOam_f, p_dsmpls, 1);
    }

    if (p_usw_mpls_master[lchip]->decap_mode)
    {
        SetDsMpls(V, innerPacketLookup_f, p_dsmpls, FALSE);
        SetDsMpls(V, fidType_f, p_dsmpls, FALSE);
        SetDsMpls(V, fid_f, p_dsmpls, 0);
        SetDsMpls(V, offsetBytes_f, p_dsmpls, 0);
    }
    SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_2,DsMpls,4,p_dsmpls, 1);
    return CTC_E_NONE;
}

/* Encode dsmpls */
STATIC int32
_sys_usw_mpls_ilm_common(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_sys_ilm)
{
    DsMpls_m* p_dsmpls = NULL;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_dsmpls = p_sys_ilm->p_new_ad;
    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_cfg_policer(lchip, p_mpls_ilm, p_dsmpls));

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_APS_SELECT)
    {
        SetDsMpls(V, apsSelectValid_f, p_dsmpls, 1);
        if (p_mpls_ilm->aps_select_protect_path)
        {
            SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 1);
        }
        else
        {
            SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 0);
        }

        SetDsMpls(V, apsSelectGroupId_f, p_dsmpls, p_mpls_ilm->aps_select_grp_id & 0x1FFF);
    }
    else
    {
        SetDsMpls(V, apsSelectValid_f, p_dsmpls, 0);
        SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 0);
        SetDsMpls(V, apsSelectGroupId_f, p_dsmpls, 0);
    }

    if (p_mpls_ilm->id_type & CTC_MPLS_ID_STATS)
    {
        uint32 stats_ptr = 0;

        p_sys_ilm->p_cur_hash->stats_id = p_mpls_ilm->stats_id;
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_mpls_ilm->stats_id, &stats_ptr));
        SetDsMpls(V, statsPtr_f, p_dsmpls, stats_ptr);
    }
    else
    {
        /* for update stats information */
        p_sys_ilm->p_cur_hash->stats_id = 0;
        SetDsMpls(V, statsPtr_f, p_dsmpls, 0);
    }

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO))
    {
         SetDsMpls(V, aclQosUseOuterInfo_f, p_dsmpls, 1);
    }

    if (p_mpls_ilm->qos_use_outer_info)
    {
        SetDsMpls(V, aclQosUseOuterInfo_f, p_dsmpls, 1);
    }



    if (p_mpls_ilm->cwen)
    {
        SetDsMpls(V, cwExist_f, p_dsmpls, 1);
        SetDsMpls(V, offsetBytes_f, p_dsmpls, 2);
    }
    else
    {
        SetDsMpls(V, offsetBytes_f, p_dsmpls, 1);
    }

    if ((13 == p_mpls_ilm->label) && (0 == p_mpls_ilm->spaceid))
    {
        SetDsMpls(V, oamCheckType_f, p_dsmpls, 1);
    }

    SetDsMpls(V, useLabelTc_f, p_dsmpls,
                (p_mpls_ilm->trust_outer_exp) ? FALSE : TRUE);

    SetDsMpls(V, denyLearning_f, p_dsmpls,
                (p_mpls_ilm->learn_disable) ? TRUE : FALSE);

    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT))
    {
            SetDsMpls(V, entropyLabelMode_f, p_dsmpls, 1);
    }

    if (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_NORMAL)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_normal(lchip, p_mpls_ilm, p_dsmpls, p_sys_ilm));
    }
    else if (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_L3VPN)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_l3vpn(lchip, p_mpls_ilm, p_dsmpls, p_sys_ilm));
    }
    else if (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_VPWS)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_vpws(lchip, p_mpls_ilm, p_dsmpls, p_sys_ilm));
    }
    else if (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_VPLS)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_vpls(lchip, p_mpls_ilm, p_dsmpls, p_sys_ilm));
    }
    else if (p_mpls_ilm->type == CTC_MPLS_LABEL_TYPE_GATEWAY)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_gateway(lchip, p_mpls_ilm, p_dsmpls, p_sys_ilm));
    }

    _sys_usw_mpls_ilm_half_check(lchip, p_dsmpls);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_add_hw(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    int32 ret = CTC_E_NONE;
    uint8 is_half = 0;
    sys_mpls_ilm_spool_t dsmpls_new;
    sys_mpls_ilm_spool_t dsmpls_old;
    sys_mpls_ilm_spool_t* p_dsmpls_get =NULL;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dsmpls_new, 0, sizeof(sys_mpls_ilm_spool_t));
    sal_memset(&dsmpls_old, 0, sizeof(sys_mpls_ilm_spool_t));

    is_half = GetDsMpls(V, isHalf_f, p_ilm_info->p_new_ad);
    sal_memcpy(&dsmpls_new.spool_ad, p_ilm_info->p_new_ad, sizeof(DsMpls_m));
    dsmpls_new.is_half = is_half;
    is_half = GetDsMpls(V, isHalf_f, p_ilm_info->p_old_ad);
    sal_memcpy(&dsmpls_old.spool_ad, p_ilm_info->p_old_ad, sizeof(DsMpls_m));
    dsmpls_old.is_half = is_half;

    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(ctc_spool_add(p_usw_mpls_master[lchip]->ad_spool, &dsmpls_new, &dsmpls_old, &p_dsmpls_get));

        p_ilm_info->ad_offset = p_dsmpls_get->ad_index;
    }
    else
    {
        p_ilm_info->ad_offset = p_ilm_info->key_offset;
    }

    CTC_ERROR_GOTO(_sys_usw_mpls_write_dsmpls(lchip, p_ilm_info->ad_offset, &dsmpls_new.spool_ad), ret, EXIT_STEP1);
    CTC_ERROR_GOTO(_sys_usw_mpls_ilm_write_key(lchip, p_mpls_ilm, p_ilm_info), ret, EXIT_STEP1);


    if (p_ilm_info->p_cur_hash)
    {
        if (DRV_IS_DUET2(lchip))
        {
            p_ilm_info->p_cur_hash->ad_index = p_dsmpls_get->ad_index;
        }
        else
        {
            p_ilm_info->p_cur_hash->ad_index  = p_ilm_info->key_offset;
        }
    }

    goto EXIT;
EXIT_STEP1:
    if (p_ilm_info->update && DRV_IS_DUET2(lchip))
    {
        ctc_spool_add(p_usw_mpls_master[lchip]->ad_spool, &dsmpls_old, &dsmpls_new, &p_dsmpls_get);
    }
    else if (DRV_IS_DUET2(lchip))
    {
        ctc_spool_remove(p_usw_mpls_master[lchip]->ad_spool, p_dsmpls_get, NULL);
    }
    return ret;
EXIT:
    return ret;
}

STATIC int32
_sys_usw_mpls_add_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    int32 ret = CTC_E_NONE;
    sys_mpls_ilm_t ilm_info;
    DsMpls_m dsmpls;
    DsMpls_m new_dsmpls;
    sys_mpls_ilm_tcam_t tcam_ilm;
    uint8 alloc_flag = 0;
    uint8 is_tcam;
    sys_nh_info_dsnh_t nhinfo;
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&new_dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&tcam_ilm, 0, sizeof(sys_mpls_ilm_tcam_t));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));

    if (p_usw_mpls_master[lchip]->decap_mode)
    {
        if (p_mpls_ilm->nh_id)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_mpls_ilm->nh_id, &nhinfo, 0));
        }
        if (((CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) || (CTC_MPLS_LABEL_TYPE_VPLS == p_mpls_ilm->type))
            && (SYS_NH_TYPE_ILOOP != nhinfo.nh_entry_type))
        {
            CTC_ERROR_RETURN(_sys_usw_mpls_add_l2vpn_pw_loop(lchip, p_mpls_ilm));
            return CTC_E_NONE;
        }
    }

    /* lookup for mpls entrise in db */
    is_tcam = (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) ? 1 : 0;
    ilm_info.p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip,p_mpls_ilm, is_tcam);
    if(ilm_info.p_cur_hash == NULL)
    {
        ilm_info.update  = 0;
        ilm_info.p_cur_hash = mem_malloc(MEM_MPLS_MODULE, sizeof(sys_mpls_ilm_hash_t));
        if(!ilm_info.p_cur_hash)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
        sal_memset(ilm_info.p_cur_hash,0,sizeof(sys_mpls_ilm_hash_t));
    }
    else
    {
        ilm_info.update  = 1;
        if (is_tcam != ilm_info.p_cur_hash->is_tcam)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No support hash or tcam update to anthor\n");
            return CTC_E_NOT_SUPPORT;
        }
    }

    /*Using Tcam resolve hash conflict*/
    /*hardware*/
    if (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX)
    {
        CTC_ERROR_GOTO(_sys_usw_mpls_add_tcam_ilm(lchip, p_mpls_ilm, &ilm_info, 0), ret, error0);
        CTC_ERROR_GOTO(_sys_usw_mpls_add_tcam_ilm(lchip, p_mpls_ilm, &ilm_info, 1), ret, error1);
    }
    else
    {
        /* if hash conflict, it will discover here */
        ret = _sys_usw_mpls_ilm_get_key_index(lchip, p_mpls_ilm, &ilm_info);
        if (CTC_E_HASH_CONFLICT == ret)
        {
            goto error0;
        }
        else if (CTC_E_EXIST == ret)
        {
            CTC_ERROR_GOTO(_sys_usw_mpls_lookup_key_by_label(lchip,p_mpls_ilm,
                                        &ilm_info.key_offset, &ilm_info.ad_offset), ret, error0);
            CTC_ERROR_GOTO(_sys_usw_mpls_read_dsmpls(lchip,ilm_info.ad_offset, &dsmpls), ret, error0);
            ilm_info.p_old_ad = &dsmpls;
            if (GetDsMpls(V, isHalf_f, &dsmpls))
            {
                /*clear second half ad*/
                sal_memset(((uint32*)&dsmpls+3), 0, sizeof(DsMplsHalf_m));
            }
        }
        else
        {
            /* if  entry not exit , need alloc key index for the ilm */
            CTC_ERROR_GOTO(_sys_usw_mpls_ilm_alloc_key_index(lchip,p_mpls_ilm, &ilm_info),ret, error0);
            ilm_info.p_old_ad = &dsmpls;
            alloc_flag = 1;
        }


        /*get default action*/
        ilm_info.p_new_ad = &new_dsmpls;
        CTC_ERROR_GOTO(_sys_usw_mpls_ilm_set_default_action(lchip, &ilm_info), ret, error2);

        /* business operation */
        CTC_ERROR_GOTO(_sys_usw_mpls_ilm_common(lchip,p_mpls_ilm, &ilm_info), ret, error2);

        /* write mpls ilm entry */
        CTC_ERROR_GOTO(_sys_usw_mpls_ilm_add_hw(lchip,p_mpls_ilm, &ilm_info), ret, error2);
    }

    /*If have mpls hash, update mpls hash db*/
    ilm_info.p_cur_hash->ad_index = ilm_info.ad_offset;

    if (ilm_info.update)
    {
        CTC_ERROR_GOTO(_sys_usw_mpls_ilm_update_hash(lchip, p_mpls_ilm, ilm_info.p_cur_hash),ret, error2);
    }
    else
    {
        if (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX)
        {
            ilm_info.p_cur_hash->is_tcam = 1;
            p_usw_mpls_master[lchip]->mpls_cur_tcam++;
        }
        else
        {
            ilm_info.p_cur_hash->is_tcam = 0;
        }
        p_usw_mpls_master[lchip]->cur_ilm_num++;
        CTC_ERROR_GOTO(_sys_usw_mpls_ilm_add_hash(lchip, p_mpls_ilm, &ilm_info), ret, error2);
    }
    return CTC_E_NONE;

error2:
    if (alloc_flag)
    {
        _sys_usw_mpls_remove_key(lchip, ilm_info.key_offset);
    }
    if ((p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) && (DRV_IS_DUET2(lchip)))
    {
        _sys_usw_mpls_remove_tcam_ilm(lchip, p_mpls_ilm, 1);
    }
error1:
    if ((p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) && (DRV_IS_DUET2(lchip)))
    {
        _sys_usw_mpls_remove_tcam_ilm(lchip, p_mpls_ilm, 0);
    }
error0:
    if (!ilm_info.update)
    {
        if (ilm_info.p_cur_hash->bind_dsfwd)
        {
            _sys_usw_mpls_bind_nexthop(lchip, ilm_info.p_cur_hash->nh_id, &ilm_info, 0);
        }
        mem_free(ilm_info.p_cur_hash);
    }
    return ret;

}

STATIC int32
_sys_usw_mpls_update_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    int32 ret = CTC_E_NONE;
    sys_mpls_ilm_t ilm_info;
    DsMpls_m dsmpls;
    DsMpls_m new_dsmpls;
    uint8 is_tcam;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&new_dsmpls, 0, sizeof(DsMpls_m));

    if (p_usw_mpls_master[lchip]->p_mpls_hash)
    {
         /* lookup for mpls entrise in db */
        is_tcam = (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) ? 1 : 0;
        ilm_info.p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip,p_mpls_ilm, is_tcam);
        if(ilm_info.p_cur_hash == NULL)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Entry not exist");
            return CTC_E_NOT_EXIST;
        }
        else if (is_tcam != ilm_info.p_cur_hash->is_tcam)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No support hash or tcam update to anthor\n");
            return CTC_E_NOT_SUPPORT;
        }
    }
    ilm_info.update = 1;
    /*Using Tcam resolve hash conflict*/
    if (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_update_tcam_ilm(lchip, p_mpls_ilm, &ilm_info, NULL, 0));
        CTC_ERROR_RETURN(_sys_usw_mpls_update_tcam_ilm(lchip, p_mpls_ilm, &ilm_info, NULL, 1));
    }
    else
    {
        /* if hash conflict, it will discover here */
        ret = _sys_usw_mpls_ilm_get_key_index(lchip,p_mpls_ilm, &ilm_info);
        if (CTC_E_EXIST != ret)
        {
            CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
        }
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_get_ad_offset(lchip,p_mpls_ilm, &ilm_info));
        CTC_ERROR_RETURN(_sys_usw_mpls_read_dsmpls(lchip,ilm_info.ad_offset, &dsmpls));
        ilm_info.p_old_ad = &dsmpls;
        ilm_info.p_new_ad = &new_dsmpls;

         /*get default action*/
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_set_default_action(lchip, &ilm_info));
         /* business operation */
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_common(lchip, p_mpls_ilm, &ilm_info));

        /* write mpls ilm entry */
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_add_hw(lchip,p_mpls_ilm, &ilm_info));
    }
    /*If have mpls hash, update mpls hash*/
    if (p_usw_mpls_master[lchip]->p_mpls_hash)
    {
        /*update hash*/
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_update_hash(lchip, p_mpls_ilm, ilm_info.p_cur_hash));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_remove_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    int32 ret = CTC_E_NONE;
    sys_mpls_ilm_t ilm_info;
    DsMpls_m dsmpls;
    sys_mpls_ilm_spool_t dsmpls_spool;
    uint8 is_tcam;
    uint16 logic_port_temp = 0;
    uint16 service_id_temp = 0;
    sys_mpls_ilm_tcam_t mpls_tcam_temp;
    sys_mpls_ilm_hash_t *p_mpls_hash;

    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&mpls_tcam_temp, 0, sizeof(sys_mpls_ilm_tcam_t));

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "label:%u, spaceid:%u\n", p_mpls_ilm->label,p_mpls_ilm->spaceid);

    if (p_usw_mpls_master[lchip]->decap_mode)
    {
        ret = _sys_usw_mpls_remove_l2vpn_pw_loop(lchip, p_mpls_ilm);
        if (CTC_E_NONE == ret)
        {
            return CTC_E_NONE;
        }
    }
    is_tcam = (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) ? 1 : 0;
    p_mpls_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, is_tcam);
    if (!p_mpls_hash)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_mpls_hash->is_tcam)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam_temp, 0));
        logic_port_temp = GetDsMpls(V, logicSrcPort_f, &mpls_tcam_temp.ad_info);
        CTC_ERROR_RETURN(_sys_usw_mpls_remove_tcam_ilm(lchip, p_mpls_ilm, 0));
        CTC_ERROR_RETURN(_sys_usw_mpls_remove_tcam_ilm(lchip, p_mpls_ilm, 1));
    }
    else
    {
        ret = _sys_usw_mpls_ilm_get_key_index(lchip, p_mpls_ilm, &ilm_info);
        if (CTC_E_EXIST != ret)
        {
            CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
        }

        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_get_ad_offset(lchip,p_mpls_ilm, &ilm_info));
        CTC_ERROR_RETURN(_sys_usw_mpls_read_dsmpls(lchip,ilm_info.ad_offset, &dsmpls));
        if (GetDsMpls(V, isHalf_f, &dsmpls))
        {
            /*clear second half ad*/
            sal_memset(((uint32*)&dsmpls+3), 0, sizeof(DsMplsHalf_m));
        }

        dsmpls_spool.is_half = GetDsMpls(V, isHalf_f, &dsmpls);
        dsmpls_spool.ad_index = ilm_info.ad_offset;
        logic_port_temp = GetDsMpls(V, logicSrcPort_f, &dsmpls);
        sal_memcpy(&dsmpls_spool.spool_ad, &dsmpls, sizeof(DsMpls_m));
        CTC_ERROR_RETURN(_sys_usw_mpls_remove_key(lchip,ilm_info.key_offset));

        if(DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN(ctc_spool_remove(p_usw_mpls_master[lchip]->ad_spool, &dsmpls_spool, NULL));
        }

    }

    if (p_mpls_hash->is_tcam)
    {
        p_usw_mpls_master[lchip]->mpls_cur_tcam--;
    }

    ilm_info.p_cur_hash = p_mpls_hash;
    if (ilm_info.p_cur_hash)
    {
        if (ilm_info.p_cur_hash->bind_dsfwd)
        {
            _sys_usw_mpls_bind_nexthop(lchip, ilm_info.p_cur_hash->nh_id, &ilm_info, 0);
        }
    }
    service_id_temp = ilm_info.p_cur_hash->service_id;
    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_remove_hash(lchip, p_mpls_ilm, is_tcam));
    (p_usw_mpls_master[lchip]->cur_ilm_num) --;

    sys_usw_qos_unbind_service_logic_srcport(lchip, service_id_temp,logic_port_temp);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_set_ilm_property_aps(uint8 lchip,
                                ctc_mpls_property_t *p_prop_info,
                                sys_mpls_ilm_t* p_ilm_info,
                                DsMpls_m* p_dsmpls)
{
    uint32 max_index = 0;
    ctc_mpls_ilm_t* p_mpls_ilm = (ctc_mpls_ilm_t*)p_prop_info->value;

    if ((p_mpls_ilm->aps_select_grp_id) != 0xFFFF)
    {
        sys_usw_ftm_query_table_entry_num(lchip, DsApsBridge_t, &max_index);

        /* no aps -> aps */
        if ((p_mpls_ilm->aps_select_grp_id) >= max_index)
        {
            CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
        }
        SetDsMpls(V, apsSelectValid_f, p_dsmpls, 1);
        if (p_mpls_ilm->aps_select_protect_path)
        {
            SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 1);
        }
        else
        {
            SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 0);
        }

        SetDsMpls(V, apsSelectGroupId_f, p_dsmpls, p_mpls_ilm->aps_select_grp_id & 0x1FFF);
        CTC_SET_FLAG(p_ilm_info->p_cur_hash->id_type, CTC_MPLS_ID_APS_SELECT);
        SetDsMpls(V, isHalf_f, p_dsmpls, 0);
    }
    else
    {
        /* aps -> no aps */

        SetDsMpls(V, apsSelectValid_f, p_dsmpls, 0);
        SetDsMpls(V, apsSelectProtectingPath_f, p_dsmpls, 0);
        SetDsMpls(V, apsSelectGroupId_f, p_dsmpls, 0);

        CTC_UNSET_FLAG(p_ilm_info->p_cur_hash->id_type, CTC_MPLS_ID_APS_SELECT);
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_mpls_set_ilm_bind_route_mac(uint8 lchip, mac_addr_t* p_mac_addr, sys_mpls_ilm_t* p_ilm_info, DsMpls_m* p_dsmpls)
{
    uint8 route_mac_index = 0;

    SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u2_type, SYS_AD_UNION_G_1);

    route_mac_index = GetDsMpls(V, routerMacProfile_f, p_dsmpls);
    if (route_mac_index) /*update*/
    {
        CTC_ERROR_RETURN(sys_usw_l3if_unbinding_inner_router_mac(lchip, route_mac_index));
        CTC_ERROR_RETURN(sys_usw_l3if_binding_inner_router_mac(lchip, p_mac_addr, &route_mac_index));
    }
    else /*add*/
    {
        CTC_ERROR_RETURN(sys_usw_l3if_binding_inner_router_mac(lchip, p_mac_addr, &route_mac_index));
    }

    SetDsMpls(V, routerMacProfile_f, p_dsmpls, route_mac_index);

    SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,p_dsmpls, 1);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_set_ilm_unbind_route_mac(uint8 lchip, mac_addr_t* p_mac_addr, sys_mpls_ilm_t* p_ilm_info, DsMpls_m* p_dsmpls)
{
    uint8 route_mac_index = 0;

    route_mac_index = GetDsMpls(V, routerMacProfile_f, p_dsmpls);
    CTC_ERROR_RETURN(sys_usw_l3if_unbinding_inner_router_mac(lchip, route_mac_index));
    SetDsMpls(V,  routerMacProfile_f, p_dsmpls, 0);

    SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,p_dsmpls, 0);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_set_ilm_property_route_mac(uint8 lchip, mac_addr_t* p_mac_addr, sys_mpls_ilm_t* p_ilm_info, DsMpls_m* p_dsmpls)
{
    uint8 mac_0[6] = {0};
    uint8 mac_f[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

    /*
        1. check mac 0.0.0 or ffff.ffff.ffff
        2. if mac invalid ,unbind the reference
        3. if mac valid, bind the reference
    */

    if ((sal_memcmp(p_mac_addr, &mac_0, sizeof(mac_addr_t)))
        && (sal_memcmp(p_mac_addr, &mac_f, sizeof(mac_addr_t))))
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_set_ilm_bind_route_mac(lchip, p_mac_addr, p_ilm_info, p_dsmpls));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_set_ilm_unbind_route_mac(lchip, p_mac_addr, p_ilm_info, p_dsmpls));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_encode_dsmpls(uint8 lchip, ctc_mpls_property_t* p_prop_info, sys_mpls_ilm_t* p_ilm_info, DsMpls_m* new_dsmpls)
{
    uint32 field = 0;
    ctc_mpls_ilm_qos_map_t* qos_map;
    uint32 stats_ptr = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    switch(p_prop_info->property_type)
    {
        case SYS_MPLS_ILM_QOS_DOMAIN:

            field = *((uint32 *)p_prop_info->value);
            if ( (field > MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX)) &&  (0xFF != field))
            {
                CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
            }
            field = (0xFF == *((uint32 *)p_prop_info->value)) ? 0:(*((uint32 *)p_prop_info->value));
            SetDsMpls(V, uMplsPhb_gElsp_mplsTcPhbPtr_f, new_dsmpls, field);
            field = (0xFF == *((uint32 *)p_prop_info->value)) ? 0:SYS_MPLS_ILM_PHB_ELSP;
            SetDsMpls(V, trustMplsTc_f, new_dsmpls, field);
            break;

        case SYS_MPLS_ILM_APS_SELECT:

            _sys_usw_mpls_set_ilm_property_aps(lchip,
                        p_prop_info,
                        p_ilm_info,
                        new_dsmpls);
            break;
        case SYS_MPLS_ILM_OAM_LMEN:

            field = *((uint32 *)p_prop_info->value) ? 1 : 0;
            SetDsMpls(V, lmEn_f, new_dsmpls, field);

            break;
        case SYS_MPLS_ILM_OAM_IDX:
            field = *((uint32 *)p_prop_info->value);
            if (0x1fff < field)
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam index error \n");
                return CTC_E_INVALID_PARAM;
            }
            SetDsMpls(V, mplsOamIndex_f, new_dsmpls, field);

            break;
        case SYS_MPLS_ILM_OAM_CHK:
            field = *((uint32 *)p_prop_info->value);
            if ((field > MCHIP_CAP(SYS_CAP_MPLS_MAX_OAM_CHK_TYPE)) && (field != 7))
            {
                CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
            }
            SetDsMpls(V, oamCheckType_f, new_dsmpls, field);
            if (SYS_MPLS_OAM_CHECK_TYPE_MEP == field)
            {
                p_ilm_info->p_cur_hash->is_tp_oam = 1;
            }
            else
            {
                p_ilm_info->p_cur_hash->is_tp_oam = 0;
            }
            break;
        case SYS_MPLS_ILM_OAM_VCCV_BFD:
            field = *((uint32 *)p_prop_info->value) ? 1 : 0;
            SetDsMpls(V, cwExist_f, new_dsmpls, field);

            break;
        case SYS_MPLS_ILM_ROUTE_MAC:

            CTC_ERROR_RETURN(_sys_usw_mpls_set_ilm_property_route_mac(lchip,
                            (mac_addr_t *)p_prop_info->value, p_ilm_info, new_dsmpls));
            break;

        case SYS_MPLS_ILM_LLSP:

            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "API or some feature is not supported \n");
            return CTC_E_NOT_SUPPORT;

            break;
         case SYS_MPLS_ILM_DENY_LEARNING:
            field = *((uint32 *)p_prop_info->value) ? 1 : 0;
            SetDsMpls(V, denyLearning_f, new_dsmpls, field);
            break;
         case SYS_MPLS_ILM_ARP_ACTION:
            field = *((uint32 *)p_prop_info->value);
            if (field == 0xff)
            {
                SetDsMpls(V, arpCtlEn_f, new_dsmpls, 0);
                SetDsMpls(V, arpExceptionType_f, new_dsmpls, 0);
                SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,new_dsmpls, 0);
            }
            else
            {
                if (MAX_CTC_EXCP_TYPE <= field)
                {
                    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "arp action error \n");
                    return CTC_E_INVALID_PARAM;
                }
                SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u2_type, SYS_AD_UNION_G_1);
                SetDsMpls(V, arpCtlEn_f, new_dsmpls, 1);
                SetDsMpls(V, arpExceptionType_f, new_dsmpls, field);
                SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u2_type,SYS_AD_UNION_G_1,DsMpls,2,new_dsmpls, 1);
            }
            break;
         case SYS_MPLS_ILM_QOS_MAP:
            qos_map = ((ctc_mpls_ilm_qos_map_t *)p_prop_info->value);
            if (SYS_MPLS_MAX_COLOR < qos_map->color || MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX) < qos_map->priority || MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX) < qos_map->exp_domain)
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
                return CTC_E_INVALID_PARAM;
            }
            if (CTC_MPLS_ILM_QOS_MAP_DISABLE == qos_map->mode)
            {
                SetDsMpls(V, uMplsPhb_gRaw_mplsPrio_f, new_dsmpls, 0);
                SetDsMpls(V, uMplsPhb_gRaw_mplsColor_f, new_dsmpls, 0);
                SetDsMpls(V, uMplsPhb_gElsp_mplsTcPhbPtr_f, new_dsmpls, 0);
                SetDsMpls(V, uMplsPhb_gLlsp_mplsPrio_f, new_dsmpls, 0);
                SetDsMpls(V, uMplsPhb_gLlsp_mplsTcPhbPtr_f, new_dsmpls, 0);
                SetDsMpls(V, trustMplsTc_f, new_dsmpls, SYS_MPLS_ILM_PHB_DISABLE);
            }
            else if (CTC_MPLS_ILM_QOS_MAP_ASSIGN == qos_map->mode)
            {
                SetDsMpls(V, uMplsPhb_gRaw_mplsPrio_f, new_dsmpls, qos_map->priority);
                SetDsMpls(V, uMplsPhb_gRaw_mplsColor_f, new_dsmpls, qos_map->color);
                SetDsMpls(V, trustMplsTc_f, new_dsmpls, SYS_MPLS_ILM_PHB_RAW);
            }
            else if (CTC_MPLS_ILM_QOS_MAP_ELSP == qos_map->mode)
            {
                SetDsMpls(V, uMplsPhb_gElsp_mplsTcPhbPtr_f, new_dsmpls, qos_map->exp_domain);
                SetDsMpls(V, trustMplsTc_f, new_dsmpls, SYS_MPLS_ILM_PHB_ELSP);
            }
            else if (CTC_MPLS_ILM_QOS_MAP_LLSP == qos_map->mode)
            {
                SetDsMpls(V, uMplsPhb_gLlsp_mplsPrio_f, new_dsmpls, qos_map->priority);
                SetDsMpls(V, uMplsPhb_gLlsp_mplsTcPhbPtr_f, new_dsmpls, qos_map->exp_domain);
                SetDsMpls(V, trustMplsTc_f, new_dsmpls, SYS_MPLS_ILM_PHB_LLSP);
            }
            else
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "API or some feature is not supported \n");
                return CTC_E_NOT_SUPPORT;
            }
            break;
        case SYS_MPLS_STATS_EN:
            p_ilm_info->p_cur_hash->stats_id = *((uint32*)p_prop_info->value);
            if (!(p_ilm_info->p_cur_hash->stats_id))
            {
                SetDsMpls(V, statsPtr_f, new_dsmpls, 0);
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_ilm_info->p_cur_hash->stats_id, &stats_ptr));
                SetDsMpls(V, statsPtr_f, new_dsmpls, stats_ptr);
            }
            break;
        case SYS_MPLS_ILM_MAC_LIMIT_DISCARD_EN:
            field = *((uint32 *)p_prop_info->value) ? 1 : 0;
            if (DRV_IS_DUET2(lchip))
            {
                SYS_CHECK_UNION_BITMAP(p_ilm_info->p_cur_hash->u4_type, SYS_AD_UNION_G_2);
                SetDsMpls(V, macSecurityDiscard_f, new_dsmpls, field);
                SYS_SET_UNION_TYPE(p_ilm_info->p_cur_hash->u4_type,SYS_AD_UNION_G_2,DsMpls,4,new_dsmpls, field ? 1 : 0);
            }
            else
            {
                SetDsMpls(V, logicPortMacSecurityDiscard_f, new_dsmpls, field);
            }
            break;
        case SYS_MPLS_ILM_MAC_LIMIT_ACTION:
            if (!DRV_FLD_IS_EXISIT(DsMpls_t, DsMpls_logicPortMacSecurityDiscard_f)||
                !DRV_FLD_IS_EXISIT(DsMpls_t, DsMpls_logicPortSecurityExceptionEn_f))
            {
                CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
            }
            field = *((uint32 *)p_prop_info->value);
            SetDsMpls(V, denyLearning_f, new_dsmpls, 0);
            SetDsMpls(V, logicPortMacSecurityDiscard_f, new_dsmpls, 0);
            SetDsMpls(V, logicPortSecurityExceptionEn_f, new_dsmpls, 0);
            switch (field)
            {
                case CTC_MACLIMIT_ACTION_NONE:
                    break;
                case CTC_MACLIMIT_ACTION_FWD:
                    SetDsMpls(V, denyLearning_f, new_dsmpls, 1);
                    break;
                case CTC_MACLIMIT_ACTION_DISCARD:
                    SetDsMpls(V, logicPortMacSecurityDiscard_f, new_dsmpls, 1);
                    break;
                case CTC_MACLIMIT_ACTION_TOCPU:
                    SetDsMpls(V, logicPortMacSecurityDiscard_f, new_dsmpls, 1);
                    SetDsMpls(V, logicPortSecurityExceptionEn_f, new_dsmpls, 1);
                    break;
                default:
                    break;
            }

            break;
        case SYS_MPLS_ILM_STATION_MOVE_DISCARD_EN:
            if (!DRV_FLD_IS_EXISIT(DsMpls_t, DsMpls_logicPortSecurityEn_f))
            {
                CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
            }
            field = *((uint32 *)p_prop_info->value) ? 1 : 0;
            SetDsMpls(V, logicPortSecurityEn_f, new_dsmpls, field);
            break;
        case SYS_MPLS_ILM_METADATA:
            if (!DRV_FLD_IS_EXISIT(DsMpls_t, DsMpls_metadata_f))
            {
                CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
            }
            field = *((uint32 *)p_prop_info->value);
            if (0xf < field)
            {
                CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
            }
            SetDsMpls(V, metadata_f, new_dsmpls, field);
            break;
        case SYS_MPLS_ILM_DCN_ACTION:
            field = *((uint32 *)p_prop_info->value);
            if (0x2 < field)
            {
                CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
            }
            SetDsMpls(V, dcnCheckType_f, new_dsmpls, field);
            break;
        case SYS_MPLS_ILM_CID:
            field = *((uint32 *)p_prop_info->value);
            CTC_MAX_VALUE_CHECK(field, 255);
            if (GetDsMpls(V, serviceIdShareMode_f, new_dsmpls))
            {
                return CTC_E_INVALID_CONFIG;
            }
            SetDsMpls(V, serviceId_f, new_dsmpls, field);
            break;
        default:
            break;
    }

    _sys_usw_mpls_ilm_half_check(lchip, new_dsmpls);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_set_default_action(uint8 lchip, sys_mpls_ilm_t* p_ilm)
{
    uint8 pro_idx = 0;
    ctc_mpls_property_t mpls_prop;
    ctc_mpls_ilm_qos_map_t qos_map;
    mac_addr_t user_mac = {0};
    ctc_mpls_ilm_t aps_sel;
    uint32 value = 0;
    sys_mpls_ilm_hash_t hash_tmp;

    if (!p_ilm->update)
    {
        return CTC_E_NONE;
    }

    sal_memcpy(&hash_tmp, p_ilm->p_cur_hash, sizeof(hash_tmp));

    p_ilm->p_cur_hash->u2_bitmap = 0;
    p_ilm->p_cur_hash->u4_bitmap = 0;
    p_ilm->p_cur_hash->u2_type = 0;
    p_ilm->p_cur_hash->u4_type = 0;

    sal_memset(&qos_map, 0, sizeof(ctc_mpls_ilm_qos_map_t));
    sal_memset(&aps_sel, 0, sizeof(ctc_mpls_ilm_t));

    /*get prop*/
    for (pro_idx = SYS_MPLS_ILM_QOS_DOMAIN; pro_idx < SYS_MPLS_ILM_MAX; pro_idx++)
    {
        if (SYS_MPLS_ILM_LLSP == pro_idx)
        {
            continue;
        }
        mpls_prop.property_type = pro_idx;
        mpls_prop.space_id = p_ilm->p_cur_hash->spaceid;
        mpls_prop.label = p_ilm->p_cur_hash->label;
        mpls_prop.use_flex = p_ilm->p_cur_hash->is_tcam;

        if (pro_idx == SYS_MPLS_ILM_QOS_MAP)
        {
            mpls_prop.value = &qos_map;
        }
        else if (pro_idx == SYS_MPLS_ILM_ROUTE_MAC)
        {
            mpls_prop.value = &user_mac[0];
        }
        else if (pro_idx == SYS_MPLS_ILM_APS_SELECT)
        {
            mpls_prop.value = &aps_sel;
        }
        else
        {
            mpls_prop.value = &value;
        }
        CTC_ERROR_RETURN(_sys_usw_mpls_get_ilm_property(lchip, &mpls_prop, p_ilm->p_old_ad, hash_tmp));

        if ((SYS_MPLS_STATS_EN == pro_idx) ||
            ((SYS_MPLS_ILM_ROUTE_MAC == pro_idx) && (hash_tmp.u2_type != SYS_AD_UNION_G_1)) ||
            (SYS_MPLS_ILM_OAM_VCCV_BFD == pro_idx) || (SYS_MPLS_ILM_APS_SELECT == pro_idx)||
            (((SYS_MPLS_ILM_MAC_LIMIT_ACTION == pro_idx)||(SYS_MPLS_ILM_STATION_MOVE_DISCARD_EN == pro_idx)||(SYS_MPLS_ILM_METADATA == pro_idx))
            && (DRV_IS_DUET2(lchip))))
        {
            continue;
        }
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_encode_dsmpls(lchip, &mpls_prop, p_ilm, p_ilm->p_new_ad));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_set_ilm_property(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, sys_mpls_ilm_t* p_ilm_info)
{
    int32 ret = CTC_E_NONE;
    ctc_mpls_property_t* p_prop_info = p_ilm_info->p_prop_info;
    DsMpls_m dsmpls;
    DsMpls_m new_dsmpls;
    sys_mpls_ilm_tcam_t mpls_tcam;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "property type:%u\n", p_prop_info->property_type);

    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&new_dsmpls, 0, sizeof(DsMpls_m));
    sal_memset(&mpls_tcam, 0, sizeof(sys_mpls_ilm_tcam_t));

    p_ilm_info->p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, p_ilm_info->p_prop_info->use_flex);
    if (NULL == p_ilm_info->p_cur_hash)
    {
        CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
    }

    if (p_usw_mpls_master[lchip]->decap_mode)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (!(p_ilm_info->p_prop_info->use_flex))
    {
        /* lookup for mpls entrise */
        ret = _sys_usw_mpls_ilm_get_key_index(lchip, p_mpls_ilm, p_ilm_info);
        if (CTC_E_EXIST != ret)
        {
            /* this is a serious problem that soft tbl and hard tbl are not coincident */
            CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
        }

        p_ilm_info->p_new_ad = &new_dsmpls;
        p_ilm_info->p_old_ad = &dsmpls;

        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_get_ad_offset(lchip, p_mpls_ilm, p_ilm_info));

        CTC_ERROR_RETURN(_sys_usw_mpls_read_dsmpls(lchip, p_ilm_info->ad_offset, &dsmpls));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 0));
        sal_memcpy(&dsmpls, &mpls_tcam.ad_info, sizeof(DsMpls_m));
    }

    if (GetDsMpls(V, isHalf_f, &dsmpls))
    {
        /*clear second half ad*/
        sal_memset(((uint32*)&dsmpls+3), 0, sizeof(DsMplsHalf_m));
    }

    sal_memcpy(&new_dsmpls, &dsmpls, sizeof(DsMpls_m));

    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_encode_dsmpls(lchip, p_prop_info, p_ilm_info, &new_dsmpls));

    p_mpls_ilm->id_type = p_ilm_info->p_cur_hash->id_type;

    /*Using Tcam resolve hash conflict*/
    if (p_ilm_info->p_prop_info->use_flex)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_update_tcam_ilm(lchip, p_mpls_ilm, p_ilm_info, &new_dsmpls, 0));
        CTC_ERROR_RETURN(_sys_usw_mpls_update_tcam_ilm(lchip, p_mpls_ilm, p_ilm_info, &new_dsmpls, 1));
    }
    else
    {
        p_ilm_info->update = 1;
        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_add_hw(lchip, p_mpls_ilm, p_ilm_info));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_ilm_data_check_mask(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint32 max_index = 0;
    uint8 service_queue_mode = 0;

    sys_usw_ftm_query_table_entry_num(0, DsApsBridge_t, &max_index);
    CTC_ERROR_RETURN(sys_usw_queue_get_service_queue_mode(lchip, &service_queue_mode));
    /* data check */
    if ((p_mpls_ilm)->id_type >= CTC_MPLS_MAX_ID)
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->id_type & CTC_MPLS_ID_SERVICE
        && (p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_VPLS
        && (p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_VPWS)
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->id_type & CTC_MPLS_ID_APS_SELECT
        && ((p_mpls_ilm)->flw_vrf_srv_aps.aps_select_grp_id >= max_index
        || (p_mpls_ilm)->aps_select_grp_id >= max_index))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Aps] Invalid group id \n");
        return CTC_E_BADID;
    }
    if ((p_mpls_ilm)->id_type & CTC_MPLS_ID_SERVICE &&
        ((p_mpls_ilm)->flw_vrf_srv_aps.service_id == 0 ||((service_queue_mode == 1)&&(p_mpls_ilm)->flw_vrf_srv_aps.service_id >= MCHIP_CAP(SYS_CAP_SERVICE_ID_NUM))))
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->id_type & CTC_MPLS_ID_VRF
        && ((p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_L3VPN
        && p_mpls_ilm->type != CTC_MPLS_LABEL_TYPE_GATEWAY))
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->id_type & CTC_MPLS_ID_VRF &&
        (p_mpls_ilm)->flw_vrf_srv_aps.vrf_id > SYS_MPLS_MAX_L3VPN_VRF_ID)
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->type >= CTC_MPLS_MAX_LABEL_TYPE)
    {
        return CTC_E_INVALID_PARAM;
    }
    if ((p_mpls_ilm)->model >= CTC_MPLS_MAX_TUNNEL_MODE)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_mpls_ilm->svlan_tpid_index) > MCHIP_CAP(SYS_CAP_MPLS_MAX_TPID_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* data mask */
    if ((p_mpls_ilm)->type == CTC_MPLS_LABEL_TYPE_VPWS
        || (p_mpls_ilm)->type == CTC_MPLS_LABEL_TYPE_VPLS)
    {
        (p_mpls_ilm)->model = CTC_MPLS_TUNNEL_MODE_PIPE;
    }
    if ((p_mpls_ilm)->type == CTC_MPLS_LABEL_TYPE_L3VPN)
    {
        (p_mpls_ilm)->cwen = FALSE;
    }
    if ((p_mpls_ilm->pwid > MCHIP_CAP(SYS_CAP_MPLS_VPLS_SRC_PORT_NUM))
            && (p_mpls_ilm->type != CTC_MPLS_LABEL_TYPE_VPWS)
            && (p_mpls_ilm->type != CTC_MPLS_LABEL_TYPE_NORMAL)
            && (p_mpls_ilm->type != CTC_MPLS_LABEL_TYPE_L3VPN))
    {
        (p_mpls_ilm)->pwid = 0xffff;
    }
    if ((p_mpls_ilm)->cwen)
    {
        (p_mpls_ilm)->cwen = TRUE;
    }
    if ((p_mpls_ilm)->logic_port_type)
    {
        (p_mpls_ilm)->logic_port_type = TRUE;
    }

    if ((p_mpls_ilm->fid) && (p_mpls_ilm->nh_id))
    {
        /*
            fid and nexthop share hw tbl feild
        */
        return CTC_E_INVALID_PARAM;
    }
    if((1 == p_mpls_ilm->nh_id || 2 == p_mpls_ilm->nh_id) && CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_free_node_data(void* node_data, void* user_data)
{
    if (node_data)
    {
        mem_free(node_data);
    }
    return 1;
}

STATIC int32
_sys_usw_mpls_init_gal(uint8 lchip, uint8 is_add)
{
    ctc_mpls_ilm_t mpls_param;

    SYS_MPLS_INIT_CHECK(lchip);

    sal_memset(&mpls_param, 0, sizeof(ctc_mpls_ilm_t));
    mpls_param.label = 13;
    mpls_param.pop = 1;
    if (is_add)
    {
        CTC_ERROR_RETURN(_sys_usw_mpls_add_ilm(lchip, &mpls_param));
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_get_ilm(uint8 lchip, uint32 nh_id[SYS_USW_MAX_ECPN], ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 i = 0;
    sys_mpls_ilm_hash_t* p_db_hash = NULL;
    sys_nh_info_dsnh_t nh_info;
    int32 ret = 0;
    uint8 is_tcam = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nh id[]:%d,%d,%d,%d,%d,%d,%d,%d\n",
                     nh_id[0], nh_id[1], nh_id[2], nh_id[3], nh_id[4], nh_id[5], nh_id[6], nh_id[7]);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "label: %d\nspace: %d\nflag: %d\n",
                     p_mpls_ilm->label, p_mpls_ilm->spaceid, p_mpls_ilm->flag);

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    for (i = 0; i < SYS_USW_MAX_ECPN; i++)
    {
        nh_id[i] = CTC_INVLD_NH_ID;
    }
    /* lookup for ipuc entrise */
    is_tcam = (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) ? 1 : 0;
    p_db_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, is_tcam);
    if (!p_db_hash)
    {
        return CTC_E_NOT_EXIST;
    }

    _sys_usw_mpls_map_ilm_sys2ctc(lchip, p_mpls_ilm ,p_db_hash);

    if ((0 == p_db_hash->nh_id) && (1 == p_mpls_ilm->pop))
    {
        p_db_hash->nh_id = 1;
        p_mpls_ilm->nh_id = 1;
    }

    if ((CTC_MPLS_LABEL_TYPE_VPLS != p_db_hash->type)
        && (CTC_MPLS_LABEL_TYPE_L3VPN != p_db_hash->type)
        && (CTC_MPLS_LABEL_TYPE_GATEWAY != p_db_hash->type))
    {
        ret = sys_usw_nh_get_nhinfo(lchip, p_db_hash->nh_id, &nh_info, 0);
        if (ret == CTC_E_NOT_EXIST)
        {
            return CTC_E_NONE;
        }
    }

    if (nh_info.ecmp_valid)
    {
        for (i = 0; i < nh_info.valid_cnt; i++)
        {
            nh_id[i] = nh_info.nh_array[i];
        }
    }
    else
    {
        nh_id[0] = p_db_hash->nh_id;
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_MPLS_INIT_CHECK(lchip);

    specs_info->used_size = p_usw_mpls_master[lchip]->cur_ilm_num;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_global_register_init(uint8 lchip)
{
    uint32 field_val = 0;
    uint32 cmd = 0;
    ParserMplsCtl_m parser_ctl;
    IpeMplsCtl_m ipe_mpls_ctl;

    field_val = 0;
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_globalLabelSpace_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_metadataBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    field_val = 2;
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_serviceIdShareMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    /*hash condlict using scl tcam lookup1*/
    field_val = 1;
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_mplsTcamLookupEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_sectionDcnExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_sectionDcnDiscardEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_useFirstLabelTc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_serviceAclQosMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOR(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &parser_ctl));
    SetParserMplsCtl(V, parseInnerEthEn_f, &parser_ctl, 1);
    SetParserMplsCtl(V, cwExist_f, &parser_ctl, 1);
    SetParserMplsCtl(V, achIdentification_f, &parser_ctl, 1);
    cmd = DRV_IOW(ParserMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &parser_ctl));

    /*flex edit init*/
    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_mplsOfModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_mplsOfModeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    /*Inner hash*/
    cmd = DRV_IOW(IpeTunnelIdCtl_t, IpeTunnelIdCtl_achIdentification_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    /*set trust dscp at interface*/
    cmd = DRV_IOW(IpePktProcPhbCtl_t, IpePktProcPhbCtl_routedPacketMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    /*Enable station move discard*/
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_portSecurityType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_trustMplsTcSkipLabelTcAssignment_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &field_val));

    /*dcn mcc scc init*/
    cmd = DRV_IOR(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ipe_mpls_ctl));
    SetIpeMplsCtl(V, achDcnChannelType0_f, &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V, achDcnChannelType1_f, &ipe_mpls_ctl, 2);
    SetIpeMplsCtl(V, mccChannelTypeEn_f, &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V, sccChannelTypeEn_f, &ipe_mpls_ctl, 1);

    /*default to set dcn pid ipv4 and ipv4*/
    SetIpeMplsCtl(V, dcnPidIpv4En_f, &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V, dcnPidIpv6En_f, &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V, dcnPid0_f, &ipe_mpls_ctl, 0x21);
    SetIpeMplsCtl(V, dcnPid1_f, &ipe_mpls_ctl, 0x57);
    SetIpeMplsCtl(V, dcnPidPacketType0_f, &ipe_mpls_ctl, 0x1);
    SetIpeMplsCtl(V, dcnPidPacketType1_f, &ipe_mpls_ctl, 0x3);
    cmd = DRV_IOW(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ipe_mpls_ctl));

    return CTC_E_NONE;
}

#define ________API_____________

int32
sys_usw_mpls_set_decap_mode(uint8 lchip, uint8 mode)
{
    SYS_MPLS_INIT_CHECK(lchip);

    SYS_MPLS_LOCK(lchip);
    p_usw_mpls_master[lchip]->decap_mode = (mode ? 1 : 0);
    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_mpls_get_oam_info(uint8 lchip, sys_mpls_oam_t* mpls_oam)
{
    sys_mpls_ilm_hash_t* mpls_db = NULL;
    ctc_mpls_ilm_t mpls_ilm;
    uint32 cmd = 0;
    uint32 mepidx_in_key = 0;

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    SYS_MPLS_LOCK(lchip);

    mpls_ilm.label = mpls_oam->label;
    mpls_ilm.spaceid = mpls_oam->spaceid;
    mpls_db = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, 0);
    if (NULL == mpls_db)
    {
        SYS_MPLS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    if(!mpls_oam->oam_type && mpls_db->is_vpws_oam)
    {
        sys_nh_info_dsnh_t nh_info;
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
        sys_usw_nh_get_nhinfo(lchip, mpls_db->nh_id, &nh_info, 0);
        mpls_oam->gport = nh_info.gport;
        mpls_oam->oamid = mpls_db->oam_id;
    }
    else if(mpls_oam->oam_type && mpls_db->is_tp_oam)
    {
       cmd = DRV_IOR(DsMpls_t, DsMpls_mplsOamIndex_f);
       DRV_IOCTL(lchip, mpls_db->ad_index, cmd, &mepidx_in_key);
       mpls_oam->oamid = mepidx_in_key;
    }
    else
    {
       SYS_MPLS_UNLOCK(lchip);
       return CTC_E_NOT_EXIST;
    }

    SYS_MPLS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_mpls_swap_oam_info(uint8 lchip, uint32 label, uint8 spaceid, uint32 new_label, uint8 new_spaceid)
{
    sys_mpls_ilm_hash_t* mpls_db_old = NULL;
    sys_mpls_ilm_hash_t* mpls_db_new = NULL;
    ctc_mpls_ilm_t mpls_ilm;
    uint32 cmd = 0;
    DsMpls_m   dsmpls;
    uint32 mep_index_in_key = 0;
    uint32 oam_check_type = 0;
    uint32 lm_en = 0;
    uint32 mode = 0;
    uint32 mepidx_in_key = 0;
    uint32 reset = 0;
    sys_mpls_ilm_t ilm_info;
    ctc_mpls_property_t mpls_pro_info;
    uint32 nh_id[SYS_USW_MAX_ECPN] = {0};

    SYS_MPLS_INIT_CHECK(lchip);
    SYS_MPLS_LOCK(lchip);
    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    /*old label*/
    mpls_ilm.label = label;
    mpls_ilm.spaceid = spaceid;
    mpls_db_old = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, 0);
    if (NULL == mpls_db_old)
    {
        SYS_MPLS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    sal_memset(&dsmpls, 0, sizeof(DsMpls_m));
    if (mpls_db_old->ad_index >= 64)
    {
        cmd = DRV_IOR(DsMpls_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, DRV_IOCTL(lchip, mpls_db_old->ad_index - 64, cmd, &dsmpls));
    }
    else
    {
        cmd = DRV_IOR(DsMplsHashCamAd_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, DRV_IOCTL(lchip, mpls_db_old->ad_index, cmd, &dsmpls));
    }
    mep_index_in_key = GetDsMpls(V, mplsOamIndex_f, &dsmpls);
    oam_check_type = GetDsMpls(V, oamCheckType_f, &dsmpls);
    lm_en = GetDsMpls(V, lmEn_f, &dsmpls);
    if (mpls_db_old->is_vpws_oam && mpls_db_old->oam_id && DRV_IS_DUET2(lchip))
    {
        sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
        mepidx_in_key = mode ? (mpls_db_old->oam_id + 2048) : mpls_db_old->oam_id;
    }
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_get_ilm(lchip, nh_id, &mpls_ilm));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
    mpls_pro_info.label = label;
    mpls_pro_info.space_id = spaceid;
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_IDX;
    mpls_pro_info.value = (void*)(&mepidx_in_key);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
    mpls_pro_info.value = (void*)(&reset);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
    mpls_pro_info.value = (void*)(&reset);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));

    /*new label*/
    mpls_ilm.label = new_label;
    mpls_ilm.spaceid = new_spaceid;
    mpls_db_new = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, 0);
    if (NULL == mpls_db_new)
    {
        SYS_MPLS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_get_ilm(lchip, nh_id, &mpls_ilm));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
    mpls_pro_info.label = new_label;
    mpls_pro_info.space_id = new_spaceid;
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_IDX;
    mpls_pro_info.value = (void*)(&mep_index_in_key);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
    mpls_pro_info.value = (void*)(&oam_check_type);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));
    mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
    mpls_pro_info.value = (void*)(&lm_en);
    ilm_info.p_prop_info = &mpls_pro_info;
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));

    SYS_MPLS_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
sys_usw_mpls_add_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 service_queue_egress_en = 0;
    uint32 mode = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_ilm);
    SYS_MPLS_ILM_KEY_CHECK(p_mpls_ilm);
    CTC_MAX_VALUE_CHECK(p_mpls_ilm->pwid, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_data_check_mask(lchip, p_mpls_ilm));
    CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT) && service_queue_egress_en)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }
    /* check vrf id */
    if (CTC_FLAG_ISSET(p_mpls_ilm->id_type, CTC_MPLS_ID_VRF))
    {
        if (0 != p_mpls_ilm->flw_vrf_srv_aps.vrf_id)
        {
           p_mpls_ilm->vrf_id = p_mpls_ilm->flw_vrf_srv_aps.vrf_id;
        }

        SYS_VRFID_CHECK(p_mpls_ilm->vrf_id);
    }

    /*check fid*/
    if (p_mpls_ilm->fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Fid error \n");
        return CTC_E_INVALID_PARAM;
    }
    sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
    if ((0x1fff < p_mpls_ilm->l2vpn_oam_id)
        || ((CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (0xFFF < p_mpls_ilm->l2vpn_oam_id))
        || (mode && DRV_IS_DUET2(lchip) && (CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (0x7ff < p_mpls_ilm->l2vpn_oam_id)))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam index error \n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_MPLS_LOCK(lchip);
    if (p_usw_mpls_master[lchip]->cur_ilm_num >= SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [MPLS] No resource \n");
        SYS_MPLS_UNLOCK(lchip);
        return CTC_E_NO_RESOURCE;
    }
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_add_ilm(lchip,p_mpls_ilm));

    SYS_MPLS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_mpls_update_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    uint8 service_queue_egress_en = 0;
    uint32 mode = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_ilm);
    SYS_MPLS_ILM_KEY_CHECK(p_mpls_ilm);
    CTC_MAX_VALUE_CHECK(p_mpls_ilm->pwid, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    SYS_GLOBAL_PORT_CHECK(p_mpls_ilm->gport);
    CTC_ERROR_RETURN(_sys_usw_mpls_ilm_data_check_mask(lchip, p_mpls_ilm));
    SYS_MPLS_DUMP_ILM(p_mpls_ilm);

    CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
    if (CTC_FLAG_ISSET(p_mpls_ilm->flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT) && service_queue_egress_en)
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }
    /* check vrf id */
    if (CTC_FLAG_ISSET(p_mpls_ilm->id_type, CTC_MPLS_ID_VRF))
    {
        if (0 != p_mpls_ilm->flw_vrf_srv_aps.vrf_id)
        {
           p_mpls_ilm->vrf_id = p_mpls_ilm->flw_vrf_srv_aps.vrf_id;
        }

        SYS_VRFID_CHECK(p_mpls_ilm->vrf_id);
    }

    /*check fid*/
    if (p_mpls_ilm->fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Fid error \n");
        return CTC_E_INVALID_PARAM;
    }

    sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
    if ((0x1fff < p_mpls_ilm->l2vpn_oam_id)
        || ((CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (0xFFF < p_mpls_ilm->l2vpn_oam_id))
        || (mode && DRV_IS_DUET2(lchip) && (CTC_MPLS_LABEL_TYPE_VPWS == p_mpls_ilm->type) && (0x7ff < p_mpls_ilm->l2vpn_oam_id)))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam index error \n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_MPLS_LOCK(lchip);

    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_update_ilm(lchip,p_mpls_ilm));

    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}

/**
 @brief function of remove mpls ilm entry

 @param[in] p_ipuc_param, parameters used to remove mpls ilm entry

 @return CTC_E_XXX
 */
int32
sys_usw_mpls_del_ilm(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_ilm);
    SYS_MPLS_ILM_KEY_CHECK(p_mpls_ilm);
    SYS_MPLS_DUMP_ILM(p_mpls_ilm);

    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }
    SYS_MPLS_LOCK(lchip);

    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_remove_ilm(lchip,p_mpls_ilm));

    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_mpls_get_ilm(uint8 lchip, uint32 nh_id[SYS_USW_MAX_ECPN], ctc_mpls_ilm_t* p_mpls_ilm)
{
    int32 ret = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_ilm);
    SYS_MPLS_DUMP_ILM(p_mpls_ilm);
    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }
    SYS_MPLS_LOCK(lchip);
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_get_ilm(lchip, nh_id, p_mpls_ilm));
    SYS_MPLS_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_mpls_set_ilm_property(uint8 lchip, ctc_mpls_property_t* p_mpls_pro_info)
{
    sys_mpls_ilm_t ilm_info;
    ctc_mpls_ilm_t mpls_ilm;
    uint32 nh_id[SYS_USW_MAX_ECPN] = {0};

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_pro_info);
    if (!(DRV_IS_DUET2(lchip)) && p_mpls_pro_info->use_flex)
    {
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));

    mpls_ilm.label = p_mpls_pro_info->label;
    mpls_ilm.spaceid = p_mpls_pro_info->space_id;
    ilm_info.p_prop_info = p_mpls_pro_info;

    if (ilm_info.p_prop_info->use_flex)
    {
        CTC_SET_FLAG(mpls_ilm.flag, CTC_MPLS_ILM_FLAG_USE_FLEX);
    }

    SYS_MPLS_LOCK(lchip);
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_get_ilm(lchip, nh_id, &mpls_ilm));
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_set_ilm_property(lchip, &mpls_ilm, &ilm_info));
    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_mpls_add_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    ctc_mpls_ilm_t sys_mpls_param;
    uint32 max_index = 0;
    uint32 mode = 0;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_pw);
    SYS_MPLS_ILM_KEY_CHECK(p_mpls_pw);
    CTC_MAX_VALUE_CHECK(p_mpls_pw->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_pw->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }
    sys_usw_ftm_query_table_entry_num(0, DsApsBridge_t, &max_index);

    if ((CTC_MPLS_L2VPN_VPWS != p_mpls_pw->l2vpntype) &&
        (CTC_MPLS_L2VPN_VPLS != p_mpls_pw->l2vpntype))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_mpls_pw->igmp_snooping_enable)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    /*check fid*/
    if (p_mpls_pw->u.vpls_info.fid > MCHIP_CAP(SYS_CAP_SPEC_MAX_FID))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Fid error \n");
        return CTC_E_INVALID_PARAM;
    }
    sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
    if ((0x1fff < p_mpls_pw->l2vpn_oam_id)
        || ((CTC_MPLS_L2VPN_VPWS == p_mpls_pw->l2vpntype) && (0xFFF < p_mpls_pw->l2vpn_oam_id))
        || (mode && DRV_IS_DUET2(lchip) && (CTC_MPLS_L2VPN_VPWS == p_mpls_pw->l2vpntype) && (0x7ff < p_mpls_pw->l2vpn_oam_id)))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Oam index error \n");
        return CTC_E_INVALID_PARAM;
    }

     if (p_mpls_pw->id_type & CTC_MPLS_ID_APS_SELECT
        && p_mpls_pw->aps_select_grp_id >= max_index)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Aps] Invalid group id \n");
        return CTC_E_BADID;
    }

    if ((p_mpls_pw->svlan_tpid_index) > MCHIP_CAP(SYS_CAP_MPLS_MAX_TPID_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&sys_mpls_param, 0 , sizeof(ctc_mpls_ilm_t));
    CTC_ERROR_RETURN(_sys_usw_mpls_map_pw_ctc2sys(lchip, p_mpls_pw, &sys_mpls_param));
    SYS_MPLS_LOCK(lchip);
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_add_ilm(lchip,&sys_mpls_param));
    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}


int32
sys_usw_mpls_del_l2vpn_pw(uint8 lchip, ctc_mpls_l2vpn_pw_t* p_mpls_pw)
{
    ctc_mpls_ilm_t sys_mpls_param;

    sal_memset(&sys_mpls_param, 0, sizeof(ctc_mpls_ilm_t));

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MPLS_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mpls_pw);
    SYS_MPLS_ILM_KEY_CHECK(p_mpls_pw);

    if (!(DRV_IS_DUET2(lchip)) && (p_mpls_pw->flag & CTC_MPLS_ILM_FLAG_USE_FLEX))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if ((CTC_MPLS_L2VPN_VPWS != p_mpls_pw->l2vpntype) &&
        (CTC_MPLS_L2VPN_VPLS != p_mpls_pw->l2vpntype))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_mpls_map_pw_ctc2sys(lchip, p_mpls_pw, &sys_mpls_param));

    SYS_MPLS_LOCK(lchip);
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, _sys_usw_mpls_remove_ilm(lchip,&sys_mpls_param));
    SYS_MPLS_UNLOCK(lchip);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mpls_traverse_fprintf_pool(void *node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    uint32*                     mode        = (uint32 *)user_date->value2;

    if(*mode == 1)
    {
        sys_mpls_ilm_spool_t *node_profile = (sys_mpls_ilm_spool_t *)(((ctc_spool_node_t*)node)->data);
        SYS_DUMP_DB_LOG(p_file, "%-9u%-10u%-10u%-13u\n",*cnt, node_profile->ad_index, node_profile->is_half, ((ctc_spool_node_t*)node)->ref_cnt);
        /*dump table DsMpls*/
        SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[0]", ((uint32*)&node_profile->spool_ad)[0]);
        SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[1]", ((uint32*)&node_profile->spool_ad)[1]);
        SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[2]", ((uint32*)&node_profile->spool_ad)[2]);
        if (!node_profile->is_half)
        {
            SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[3]", ((uint32*)&node_profile->spool_ad)[3]);
            SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[4]", ((uint32*)&node_profile->spool_ad)[4]);
            SYS_DUMP_DB_LOG(p_file, "%-12s:0x%-10x\n", "Word[5]", ((uint32*)&node_profile->spool_ad)[5]);
        }
    }
    (*cnt)++;

    return CTC_E_NONE;
}

int32
sys_usw_mpls_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 1;
    uint32 mode = 0;

    SYS_MPLS_INIT_CHECK(lchip);
    SYS_MPLS_LOCK(lchip);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# MPLS");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","cur_ilm_num", p_usw_mpls_master[lchip]->cur_ilm_num);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","mpls_cur_tcam", p_usw_mpls_master[lchip]->mpls_cur_tcam);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","pop_ad_idx", p_usw_mpls_master[lchip]->pop_ad_idx);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","decap_ad_idx", p_usw_mpls_master[lchip]->decap_ad_idx);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","decap_mode", p_usw_mpls_master[lchip]->decap_mode);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","decap_tcam_group", p_usw_mpls_master[lchip]->decap_tcam_group);
    SYS_DUMP_DB_LOG(p_f, "%-30s:[%u:%u][%u:%u]\n","tcam_group", 0, p_usw_mpls_master[lchip]->tcam_group[0], 1, p_usw_mpls_master[lchip]->tcam_group[1]);

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    mode = 1;
    cb_data.value2 = &mode;
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","mpls_ad_pool(DsMpls)");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-10s%-10s%-13s\n","Node","ad_index","is_half", "refcnt");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    ctc_spool_traverse(p_usw_mpls_master[lchip]->ad_spool, (spool_traversal_fn)_sys_usw_mpls_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    SYS_MPLS_UNLOCK(lchip);

    return ret;
}



int32
sys_usw_mpls_init(uint8 lchip, ctc_mpls_init_t* p_mpls_info)
{
    uint32 ftm_mpls_num = 0;
    int32 ret = 0;
    ctc_spool_t spool;
    uint8  work_status = 0;

    if(NULL != p_usw_mpls_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }

    /* get num of mpls resource */
    sys_usw_ftm_query_table_entry_num(lchip, DsMpls_t, &ftm_mpls_num);
    if ((0 == SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS))
        || (0 == ftm_mpls_num))
    {
        return CTC_E_NONE;
    }

    p_usw_mpls_master[lchip] = mem_malloc(MEM_MPLS_MODULE, sizeof(sys_mpls_master_t));
    if (NULL == p_usw_mpls_master[lchip])
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_mpls_master[lchip], 0, sizeof(sys_mpls_master_t));

    p_usw_mpls_master[lchip]->decap_mode = 0;
    p_usw_mpls_master[lchip]->p_mpls_hash  = ctc_hash_create(
            512,
            ftm_mpls_num/512,
            (hash_key_fn)_sys_usw_mpls_hash_make,
            (hash_cmp_fn)_sys_usw_mpls_hash_cmp);
    if (NULL == p_usw_mpls_master[lchip]->p_mpls_hash)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
        ret = CTC_E_NO_MEMORY;
        goto error_0;
    }

    if (DRV_IS_DUET2(lchip))
    {
        spool.lchip = lchip;
        spool.block_num = 512;
        spool.block_size = ftm_mpls_num / 512;
        spool.max_count = ftm_mpls_num;
        spool.user_data_size = sizeof(sys_mpls_ilm_spool_t);
        spool.spool_key = (hash_key_fn) _sys_usw_mpls_ad_spool_hash_make;
        spool.spool_cmp = (hash_cmp_fn) _sys_usw_mpls_ad_spool_hash_cmp;
        spool.spool_alloc = (spool_alloc_fn)_sys_usw_mpls_ad_spool_alloc_index;
        spool.spool_free = (spool_free_fn)_sys_usw_mpls_ad_spool_free_index;
        p_usw_mpls_master[lchip]->ad_spool = ctc_spool_create(&spool);
        if (NULL == p_usw_mpls_master[lchip]->ad_spool)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "No Memory\n");
            ret = CTC_E_NO_MEMORY;
            goto error_1;
        }
    }
    CTC_ERROR_GOTO(_sys_usw_mpls_global_register_init(lchip), ret, error_2);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_MPLS, sys_usw_mpls_wb_sync), ret, error_2);

    sal_mutex_create(&p_usw_mpls_master[lchip]->mutex);
    if (NULL == p_usw_mpls_master[lchip]->mutex)
    {
        ret = CTC_E_NO_MEMORY;
        goto error_2;
    }

    CTC_ERROR_GOTO(sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_MPLS,  _sys_usw_mpls_ftm_cb), ret, error_3);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_MPLS, sys_usw_mpls_dump_db), ret, error_3);
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (work_status != CTC_FTM_MEM_CHANGE_RECOVER))
    {
        CTC_ERROR_GOTO(sys_usw_mpls_wb_restore(lchip), ret, error_3);
        return ret; /*Note: return for WB Reloading!!!*/
    }
    CTC_ERROR_GOTO(_sys_usw_mpls_init_gal(lchip, 1), ret, error_3);
    CTC_ERROR_GOTO(sys_usw_ftm_register_rsv_key_cb(lchip, SYS_FTM_HASH_KEY_MPLS, _sys_usw_mpls_init_gal), ret, error_3);

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return ret;
    error_3:
        sal_mutex_destroy(p_usw_mpls_master[lchip]->mutex);
    error_2:
        if (p_usw_mpls_master[lchip]->ad_spool)
        {
            ctc_spool_free(p_usw_mpls_master[lchip]->ad_spool);
        }
    error_1:
        ctc_hash_traverse_remove(p_usw_mpls_master[lchip]->p_mpls_hash, (hash_traversal_fn)_sys_usw_mpls_free_node_data, NULL);
        ctc_hash_free(p_usw_mpls_master[lchip]->p_mpls_hash);
    error_0:
        mem_free(p_usw_mpls_master[lchip]);
    return ret;

}


int32
sys_usw_mpls_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_mpls_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*free hash data*/
    ctc_hash_traverse_remove(p_usw_mpls_master[lchip]->p_mpls_hash, (hash_traversal_fn)_sys_usw_mpls_free_node_data, NULL);
    ctc_hash_free(p_usw_mpls_master[lchip]->p_mpls_hash);

    if (p_usw_mpls_master[lchip]->ad_spool)
    {
        ctc_spool_free(p_usw_mpls_master[lchip]->ad_spool);
    }

    if (p_usw_mpls_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_mpls_master[lchip]->mutex);
    }
    mem_free(p_usw_mpls_master[lchip]);
    return CTC_E_NONE;
}

#define ___________WB________________

STATIC int32
_sys_usw_mpls_wb_hash_mapping_info(sys_wb_mpls_ilm_hash_t *p_wb_mpls_info, sys_mpls_ilm_hash_t *p_mpls_info, uint8 sync)
{
    if (sync)
    {
        p_wb_mpls_info->ad_index = p_mpls_info->ad_index;
        p_wb_mpls_info->bind_dsfwd = p_mpls_info->bind_dsfwd;
        p_wb_mpls_info->id_type = p_mpls_info->id_type;
        p_wb_mpls_info->is_interface = p_mpls_info->is_interface;
        p_wb_mpls_info->label = p_mpls_info->label;
        p_wb_mpls_info->model = p_mpls_info->model;
        p_wb_mpls_info->nh_id = p_mpls_info->nh_id;
        p_wb_mpls_info->spaceid = p_mpls_info->spaceid;
        p_wb_mpls_info->stats_id = p_mpls_info->stats_id;
        p_wb_mpls_info->type = p_mpls_info->type;
        p_wb_mpls_info->u2_bitmap = p_mpls_info->u2_bitmap;
        p_wb_mpls_info->u2_type = p_mpls_info->u2_type;
        p_wb_mpls_info->u4_bitmap = p_mpls_info->u4_bitmap;
        p_wb_mpls_info->u4_type = p_mpls_info->u4_type;
        p_wb_mpls_info->is_assignport = p_mpls_info->is_assignport;
        p_wb_mpls_info->is_tcam = p_mpls_info->is_tcam;
        p_wb_mpls_info->service_id = p_mpls_info->service_id;
        p_wb_mpls_info->oam_id = p_mpls_info->oam_id;
        p_wb_mpls_info->is_vpws_oam = p_mpls_info->is_vpws_oam;
        p_wb_mpls_info->is_tp_oam = p_mpls_info->is_tp_oam;
        p_wb_mpls_info->gport = p_mpls_info->gport;
    }
    else
    {
        p_mpls_info->ad_index = p_wb_mpls_info->ad_index;
        p_mpls_info->bind_dsfwd = p_wb_mpls_info->bind_dsfwd;
        p_mpls_info->id_type = p_wb_mpls_info->id_type;
        p_mpls_info->is_interface = p_wb_mpls_info->is_interface;
        p_mpls_info->label = p_wb_mpls_info->label;
        p_mpls_info->model = p_wb_mpls_info->model;
        p_mpls_info->nh_id = p_wb_mpls_info->nh_id;
        p_mpls_info->spaceid = p_wb_mpls_info->spaceid;
        p_mpls_info->stats_id = p_wb_mpls_info->stats_id;
        p_mpls_info->type = p_wb_mpls_info->type;
        p_mpls_info->u2_bitmap = p_wb_mpls_info->u2_bitmap;
        p_mpls_info->u2_type = p_wb_mpls_info->u2_type;
        p_mpls_info->u4_bitmap = p_wb_mpls_info->u4_bitmap;
        p_mpls_info->u4_type = p_wb_mpls_info->u4_type;
        p_mpls_info->is_assignport = p_wb_mpls_info->is_assignport & 0x1;
        p_mpls_info->is_tcam = p_wb_mpls_info->is_tcam;
        p_mpls_info->service_id = p_wb_mpls_info->service_id;
        p_mpls_info->oam_id = p_wb_mpls_info->oam_id;
        p_mpls_info->is_vpws_oam = p_wb_mpls_info->is_vpws_oam;
        p_mpls_info->is_tp_oam = p_wb_mpls_info->is_tp_oam;
        p_mpls_info->gport = p_wb_mpls_info->gport;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_mpls_sync_hash_func(sys_mpls_ilm_hash_t *p_mpls_info, void *user_data)
{
   uint16 max_entry_cnt = 0;
   sys_wb_mpls_ilm_hash_t *p_wb_mpls_info;
   ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(user_data);

   max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);
   p_wb_mpls_info = (sys_wb_mpls_ilm_hash_t *)wb_data->buffer + wb_data->valid_cnt;
   sal_memset(p_wb_mpls_info, 0, sizeof(sys_wb_mpls_ilm_hash_t));
   CTC_ERROR_RETURN(_sys_usw_mpls_wb_hash_mapping_info(p_wb_mpls_info, p_mpls_info, 1));
   if (++wb_data->valid_cnt == max_entry_cnt)
   {
       CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
       wb_data->valid_cnt = 0;
   }
   return CTC_E_NONE;
}

int32
sys_usw_mpls_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_mpls_master_t *p_wb_mpls_master = NULL;
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    wb_data.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_MPLS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*sync master*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_mpls_master_t, CTC_FEATURE_MPLS, SYS_WB_APPID_MPLS_SUBID_MASTER);

    p_wb_mpls_master = (sys_wb_mpls_master_t*)wb_data.buffer;

    p_wb_mpls_master->cur_ilm_num = p_usw_mpls_master[lchip]->cur_ilm_num;
    p_wb_mpls_master->decap_ad_idx = p_usw_mpls_master[lchip]->decap_ad_idx;
    p_wb_mpls_master->pop_ad_idx = p_usw_mpls_master[lchip]->pop_ad_idx;
    p_wb_mpls_master->mpls_cur_tcam = p_usw_mpls_master[lchip]->mpls_cur_tcam;
    p_wb_mpls_master->mpls_tcam0 = p_usw_mpls_master[lchip]->tcam_group[0];
    p_wb_mpls_master->mpls_tcam1 = p_usw_mpls_master[lchip]->tcam_group[1];
    p_wb_mpls_master->decap_mode = p_usw_mpls_master[lchip]->decap_mode;
    p_wb_mpls_master->decap_tcam_group = p_usw_mpls_master[lchip]->decap_tcam_group;
    p_wb_mpls_master->lchip = lchip;
    p_wb_mpls_master->version = SYS_WB_VERSION_MPLS;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*sync hash*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_mpls_ilm_hash_t, CTC_FEATURE_MPLS, SYS_WB_APPID_MPLS_SUBID_ILM_HASH);
    wb_data.valid_cnt = 0;
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_mpls_master[lchip]->p_mpls_hash, (hash_traversal_fn) _sys_usw_mpls_sync_hash_func, (void *)&wb_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
         CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
         wb_data.valid_cnt = 0;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

done:
    if (wb_data.buffer )
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_mpls_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t    wb_query;
    sys_wb_mpls_master_t wb_mpls_master = {0};
    sys_wb_mpls_ilm_hash_t wb_ilm_hash = {0};
    sys_mpls_ilm_hash_t* p_ilm_hash = NULL;
    sys_mpls_ilm_spool_t p_ilm_spool;
    sys_mpls_ilm_spool_t* p_ilm_spool_get = NULL;
    uint32 entry_cnt = 0;
    uint32 cmd = 0;
    uint32 cmd1 = DRV_IOR(DsMpls_t, DRV_ENTRY_FLAG);
    uint32 cmd2 = DRV_IOR(DsMplsHalf_t, DRV_ENTRY_FLAG);
    sys_nh_update_dsnh_param_t update_dsnh;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&p_ilm_spool, 0, sizeof(p_ilm_spool));
    /*restore master*/
    wb_query.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_MPLS_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_mpls_master_t, CTC_FEATURE_MPLS, SYS_WB_APPID_MPLS_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query mpls master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)&wb_mpls_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_MPLS, wb_mpls_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    p_usw_mpls_master[lchip]->cur_ilm_num = wb_mpls_master.cur_ilm_num;
    p_usw_mpls_master[lchip]->decap_ad_idx = wb_mpls_master.decap_ad_idx;
    p_usw_mpls_master[lchip]->pop_ad_idx = wb_mpls_master.pop_ad_idx;
    p_usw_mpls_master[lchip]->mpls_cur_tcam = wb_mpls_master.mpls_cur_tcam;
    p_usw_mpls_master[lchip]->tcam_group[0] = wb_mpls_master.mpls_tcam0;
    p_usw_mpls_master[lchip]->tcam_group[1] = wb_mpls_master.mpls_tcam1;
    p_usw_mpls_master[lchip]->decap_mode = wb_mpls_master.decap_mode;
    p_usw_mpls_master[lchip]->decap_tcam_group = wb_mpls_master.decap_tcam_group;

    /*restore hash*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_mpls_ilm_hash_t, CTC_FEATURE_MPLS, SYS_WB_APPID_MPLS_SUBID_ILM_HASH);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
    sal_memcpy((uint8*)&wb_ilm_hash, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
    entry_cnt++;

    p_ilm_hash = mem_malloc(MEM_MPLS_MODULE, sizeof(sys_mpls_ilm_hash_t));
    if (NULL == p_ilm_hash)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_ilm_hash, 0, sizeof(sys_mpls_ilm_hash_t));
    CTC_ERROR_GOTO(_sys_usw_mpls_wb_hash_mapping_info(&wb_ilm_hash, p_ilm_hash, 0), ret, done);

    if (p_ilm_hash->bind_dsfwd)
    {
        update_dsnh.data = p_ilm_hash;
        update_dsnh.updateAd = _sys_usw_mpls_update_ad;
        sys_usw_nh_bind_dsfwd_cb(lchip, p_ilm_hash->nh_id, &update_dsnh);
    }

    if (DRV_IS_DUET2(lchip))
    {
        if (!p_ilm_hash->ad_index)
        {
            cmd = cmd2;
        }
        else
        {
            cmd = (p_ilm_hash->ad_index % 2) ? cmd2 : cmd1;
        }
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_ilm_hash->ad_index, cmd, &(p_ilm_spool.spool_ad)), ret, done);
        _sys_usw_mpls_ilm_half_check(lchip, &(p_ilm_spool.spool_ad));
        p_ilm_spool.is_half = GetDsMpls(V, isHalf_f, &(p_ilm_spool.spool_ad));
        p_ilm_spool.ad_index = p_ilm_hash->ad_index;

        CTC_ERROR_GOTO(ctc_spool_add(p_usw_mpls_master[lchip]->ad_spool, &p_ilm_spool, NULL, p_ilm_spool_get), ret, done);

        sal_memset(&p_ilm_spool, 0, sizeof(p_ilm_spool));

        if (NULL == ctc_hash_insert(p_usw_mpls_master[lchip]->p_mpls_hash, p_ilm_hash))
        {
            ctc_spool_remove(p_usw_mpls_master[lchip]->ad_spool, p_ilm_spool_get, NULL);
            ret = CTC_E_NO_MEMORY;
            goto done;
        }

    }
    else
    {
        if (NULL == ctc_hash_insert(p_usw_mpls_master[lchip]->p_mpls_hash, p_ilm_hash))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
    }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    if (p_ilm_hash)
    {
        mem_free(p_ilm_hash);
    }

   return ret;
}


#define ___________DEBUG________________

STATIC int32
_sys_usw_show_ilm_info_brief(sys_mpls_ilm_hash_t* p_ilm_hash, void* p_data)
{
    uint8 lchip = 0;
    ctc_mpls_ilm_t mpls_ilm;
    sys_mpls_ilm_t ilm_info;
    sys_mpls_show_info_t* p_space_info = NULL;
    char *arr_type[] = {
            "Normal",
            "L3VPN",
            "VPWS",
            "VPLS",
            "GATEWAY"
        };
    char *arr_encap[] = {
            "Tagged",
            "Raw",
        };

    CTC_PTR_VALID_CHECK(p_data);

    p_space_info = (sys_mpls_show_info_t*)p_data;
    lchip = p_space_info->lchip;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));

    mpls_ilm.spaceid = p_ilm_hash->spaceid;
    mpls_ilm.label = p_ilm_hash->label;

    ilm_info.p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, p_ilm_hash->is_tcam);
    if (NULL == ilm_info.p_cur_hash)
    {
        /* continue traverse the hash */
        return CTC_E_NONE;
    }


    _sys_usw_mpls_map_ilm_sys2ctc(lchip, &mpls_ilm ,ilm_info.p_cur_hash);

    if ((0 == ilm_info.p_cur_hash->nh_id) && (1 == mpls_ilm.pop))
    {
        ilm_info.p_cur_hash->nh_id = 1;
    }

    /** print one entry */
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d%-10d%-10s",
                p_ilm_hash->spaceid, p_ilm_hash->label, arr_type[ilm_info.p_cur_hash->type]);

    if (0 == ilm_info.p_cur_hash->nh_id)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", ilm_info.p_cur_hash->nh_id);
    }

    if ((CTC_MPLS_LABEL_TYPE_VPWS == mpls_ilm.type)
        || (CTC_MPLS_LABEL_TYPE_VPLS == mpls_ilm.type)
        || (CTC_MPLS_LABEL_TYPE_GATEWAY == mpls_ilm.type))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", arr_encap[mpls_ilm.pw_mode]);
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }
    if ((0xFFFF == mpls_ilm.fid) || (0 == mpls_ilm.fid) || (0 != ilm_info.p_cur_hash->nh_id))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", mpls_ilm.fid);
    }

    if (CTC_FLAG_ISSET(mpls_ilm.id_type, CTC_MPLS_ID_VRF))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u",mpls_ilm.vrf_id);
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", "-");
    }

    if ((MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT) < mpls_ilm.pwid) || (0 == mpls_ilm.pwid))
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", mpls_ilm.pwid);
    }

    if (ilm_info.p_cur_hash->is_tcam)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s\n", "Yes");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s\n", "No");
    }
    /** end */

    ++(p_space_info->count);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_show_ilm_info_detail(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    sys_mpls_ilm_hash_t* p_cur_hash = NULL;
    uint32 ret = CTC_E_NONE;
    DsMpls_m dsmpls;
    uint8 is_half = 0;
    uint8 is_tcam;
    sys_mpls_ilm_tcam_t mpls_tcam;

    char* arr_type[] = {
            "Normal",
            "L3VPN",
            "VPWS",
            "VPLS",
            "GATEWAY"
        };
    char* arr_model[] = {
            "uniform",
            "short pipe",
            "pipe",
        };
    char* arr_encap[] = {
            "Tagged",
            "Raw",
        };
    sys_mpls_ilm_t ilm_info;

    sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
    is_tcam = (p_mpls_ilm->flag & CTC_MPLS_ILM_FLAG_USE_FLEX) ? 1 : 0;
    ilm_info.p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip, p_mpls_ilm, is_tcam);
    if (NULL == ilm_info.p_cur_hash)
    {
        /* continue traverse the hash */
        return CTC_E_NONE;
    }
    p_cur_hash = ilm_info.p_cur_hash;

    if (!p_cur_hash->is_tcam)
    {
        ret = _sys_usw_mpls_ilm_get_key_index(lchip, p_mpls_ilm, &ilm_info);
        if (CTC_E_EXIST != ret)
        {
            CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
        }

        CTC_ERROR_RETURN(_sys_usw_mpls_ilm_get_ad_offset(lchip, p_mpls_ilm, &ilm_info));

        _sys_usw_mpls_read_dsmpls(lchip, ilm_info.ad_offset, &dsmpls);
        is_half = GetDsMpls(V, isHalf_f, &dsmpls);

        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------Detail Information-------------------\r\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Label", p_mpls_ilm->label);
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Space Id", p_mpls_ilm->spaceid);

        if (ilm_info.p_cur_hash->is_interface)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Is Interface", "Yes");
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Is Interface", "No");
        }
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Type", arr_type[p_cur_hash->type]);

        if (0 != p_cur_hash->nh_id)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Nexthop Id", p_cur_hash->nh_id);
        }

        if (ilm_info.p_cur_hash->is_assignport)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Is assign port", "Yes");
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Is assign port", "No");
        }

        if (!is_half)
        {
            if (CTC_MPLS_ILM_OAM_MP_CHK_TYPE_OAM_MEP == GetDsMpls(V, oamCheckType_f, &dsmpls))
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "OAMTYPE", "MEP");
            }
            else if (CTC_MPLS_ILM_OAM_MP_CHK_TYPE_OAM_MIP == GetDsMpls(V, oamCheckType_f, &dsmpls))
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "OAMTYPE", "MIP");
            }
        }

        switch(p_cur_hash->type)
        {
            case CTC_MPLS_LABEL_TYPE_NORMAL:
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Model", arr_model[p_cur_hash->model]);

                break;
            case CTC_MPLS_LABEL_TYPE_L3VPN:
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Model", arr_model[p_cur_hash->model]);
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "VRF", p_mpls_ilm->vrf_id);
                break;
            case CTC_MPLS_LABEL_TYPE_VPWS:
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Encap Mode", arr_encap[p_mpls_ilm->pw_mode]);
                break;
            case CTC_MPLS_LABEL_TYPE_VPLS:
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Fid", p_mpls_ilm->fid);
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Logic Port", p_mpls_ilm->pwid);
                if (p_mpls_ilm->logic_port_type)
                {
                    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "HVPLS", "No");
                }
                else
                {
                    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "HVPLS", "Yes");
                }

                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Encap Mode", arr_encap[p_mpls_ilm->pw_mode]);
                break;
            default:
                break;
        }

        if (p_mpls_ilm->id_type & CTC_MPLS_ID_APS_SELECT)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "APS Group ID", p_mpls_ilm->aps_select_grp_id);
            if (p_mpls_ilm->aps_select_protect_path)
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "APS Path Type", "protecting");
            }
            else
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "APS Path Type", "working");
            }
        }

        if (p_mpls_ilm->decap)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Decap", "Yes");
        }

        if (p_mpls_ilm->cwen)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Control Word", p_mpls_ilm->cwen);
        }

        if (0 != p_cur_hash->stats_id)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Stats Id", p_cur_hash->stats_id);
        }

        if (0 != p_mpls_ilm->oam_en)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "OAM", "Yes");
        }

        if (0 != p_mpls_ilm->l2vpn_oam_id)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Oam Id", p_mpls_ilm->l2vpn_oam_id);
        }

        if (p_mpls_ilm->id_type & CTC_MPLS_ID_SERVICE)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Service id", p_mpls_ilm->flw_vrf_srv_aps.service_id);
        }
        if (DRV_IS_DUET2(lchip))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "DsUserIdTunnelMplsHashKey", ilm_info.key_offset);
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", is_half?"DsMplsHalf":"DsMpls", ilm_info.ad_offset);
        }
        else
        {
            if (ilm_info.key_offset > 63)
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "DsMplsLabelHashKey", (ilm_info.key_offset));
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "DsMpls", (ilm_info.ad_offset - 64));
            }
            else
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "DsMplsLabelHashKey", ilm_info.key_offset);
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "DsMplsHashCamAd", ilm_info.ad_offset);
            }
        }

        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\r\n");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------Detail Information-------------------\r\n");
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Label", p_mpls_ilm->label);
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6u \n", "Space Id", p_mpls_ilm->spaceid);

        if ((0 != p_cur_hash->is_tcam) && (DRV_IS_DUET2(lchip)))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%-6s \n", "Use Tcam", "Yes");
            CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 0));
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-6x \n", "Scl id 0 entry Id", mpls_tcam.entry_id);
            CTC_ERROR_RETURN(_sys_usw_mpls_get_tcam_action_field(lchip, p_mpls_ilm, &mpls_tcam, 1));
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:0x%-6x \n", "Scl id 1 entry Id", mpls_tcam.entry_id);
        }
    }

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------\r\n");

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_show_ilm_info(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm)
{
    sys_mpls_show_info_t space_info;
    int32 ret = CTC_E_NONE;

    sal_memset(&space_info, 0, sizeof(sys_mpls_show_info_t));

    space_info.lchip = lchip;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
                    "SPACE", "LABEL", "TYPE", "NHID", "PW_Mode", "FID", "VRF", "LOGIC_PORT", "   USE TCAM");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");

    ret = ctc_hash_traverse_through(p_usw_mpls_master[lchip]->p_mpls_hash,
                (hash_traversal_fn)_sys_usw_show_ilm_info_brief,
                &space_info);

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Count:%u\n", space_info.count);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------------------\n");

    return ret;
}

STATIC int32
_sys_usw_show_ilm_info_by_space(sys_mpls_ilm_hash_t* p_ilm_hash, void* p_data)
{
    uint8 lchip = 0;
    sys_mpls_ilm_t ilm_info;
    ctc_mpls_ilm_t mpls_ilm;
    sys_nh_info_dsnh_t nh_info;
    sys_mpls_show_info_t* p_space_info = NULL;
    uint8 i = 0;
    uint32 nh_id[SYS_USW_MAX_ECPN] = {0};
    char *arr_type[] = {
            "Normal",
            "L3VPN",
            "VPWS",
            "VPLS",
            "GATEWAY"
        };
    char *arr_encap[] = {
            "Tagged",
            "Raw",
        };

    if (NULL == p_data)
    {
        return CTC_E_NONE;
    }

    p_space_info = (sys_mpls_show_info_t*)p_data;
    lchip = p_space_info->lchip;

    if (p_ilm_hash->spaceid == p_space_info->space_id)
    {
        sal_memset(&ilm_info, 0, sizeof(sys_mpls_ilm_t));
        sal_memset(&mpls_ilm, 0, sizeof(ctc_mpls_ilm_t));
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

        for (i = 0; i < SYS_USW_MAX_ECPN; i++)
        {
            nh_id[i] = CTC_INVLD_NH_ID;
        }

        mpls_ilm.label = p_ilm_hash->label;
        mpls_ilm.spaceid = p_ilm_hash->spaceid;
        ilm_info.p_cur_hash = _sys_usw_mpls_ilm_get_hash(lchip, &mpls_ilm, p_ilm_hash->is_tcam);
        if (NULL == ilm_info.p_cur_hash)
        {
            return CTC_E_NONE;
        }

        _sys_usw_mpls_map_ilm_sys2ctc(lchip, &mpls_ilm ,ilm_info.p_cur_hash);

        if ((0 == ilm_info.p_cur_hash->nh_id) && (1 == mpls_ilm.pop))
        {
            ilm_info.p_cur_hash->nh_id = 1;
        }

        if ((CTC_MPLS_LABEL_TYPE_VPLS != ilm_info.p_cur_hash->type)
            && (CTC_MPLS_LABEL_TYPE_L3VPN != ilm_info.p_cur_hash->type)
            && (CTC_MPLS_LABEL_TYPE_GATEWAY != ilm_info.p_cur_hash->type))
        {
            sys_usw_nh_get_nhinfo(lchip, ilm_info.p_cur_hash->nh_id, &nh_info, 0);
        }

        if (nh_info.ecmp_valid)
        {
            for (i = 0; i < nh_info.valid_cnt; i++)
            {
                nh_id[i] = nh_info.nh_array[i];
            }
        }
        else
        {
            nh_id[0] = ilm_info.p_cur_hash->nh_id;
        }

        /** print one entry */
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10d %-10s",
                    p_ilm_hash->label, arr_type[ilm_info.p_cur_hash->type]);

        if (0 == ilm_info.p_cur_hash->nh_id)
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", "-");
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10u", nh_id[0]);
        }

        if ((CTC_MPLS_LABEL_TYPE_VPWS == mpls_ilm.type)
            || (CTC_MPLS_LABEL_TYPE_VPLS == mpls_ilm.type)
            || (CTC_MPLS_LABEL_TYPE_GATEWAY == mpls_ilm.type))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", arr_encap[mpls_ilm.pw_mode]);
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", "-");
        }
        if ((0xFFFF == mpls_ilm.fid) || (0 == mpls_ilm.fid) || (0 != ilm_info.p_cur_hash->nh_id))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", "-");
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10u", mpls_ilm.fid);
        }

        if (CTC_FLAG_ISSET(mpls_ilm.id_type, CTC_MPLS_ID_VRF))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u",mpls_ilm.vrf_id);
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s", "-");
        }

        if ((MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT) < mpls_ilm.pwid) ||(0 == mpls_ilm.pwid))
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s\n", "-");
        }
        else
        {
            SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10u\n", mpls_ilm.pwid);
        }
        /** end */

        ++(p_space_info->count);
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_show_mpls_ilm_all(sys_mpls_ilm_hash_t *p_mpls_info, void *user_data)
{
    int32 ret;
    uint8 lchip = 0;
    uint32 nh_id[SYS_USW_MAX_ECPN];
    ctc_mpls_ilm_t p_mpls_ilm;

    sal_memset(&p_mpls_ilm, 0, sizeof(p_mpls_ilm));
    p_mpls_ilm.label = p_mpls_info->label;
    p_mpls_ilm.spaceid = p_mpls_info->spaceid;
    if (p_mpls_info->is_tcam)
    {
        p_mpls_ilm.flag = p_mpls_ilm.flag | CTC_MPLS_ILM_FLAG_USE_FLEX;
    }
    ret = _sys_usw_mpls_get_ilm(lchip, nh_id, &p_mpls_ilm);
    if (ret < 0)
    {
       SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "% Entry not exist");
       return CTC_E_NONE;
    }
    ret = _sys_usw_show_ilm_info_detail(lchip, &p_mpls_ilm);
    return ret;
}

STATIC int32
_sys_usw_mpls_ilm_show_by_space(uint8 lchip, uint8 space_id)
{
    sys_mpls_show_info_t space_info;
    int32 ret = CTC_E_NONE;

    sal_memset(&space_info, 0, sizeof(sys_mpls_show_info_t));

    space_info.space_id = space_id;
    space_info.lchip = lchip;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Ilms In Space %d\n", space_id);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
                    "LABEL", "TYPE", "NHID", "PW_Mode", "FID", "VRF", "LOGIC_PORT");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");
    ret = ctc_hash_traverse_through(p_usw_mpls_master[lchip]->p_mpls_hash,
                        (hash_traversal_fn)_sys_usw_show_ilm_info_by_space,
                        &space_info);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " Count %d\n", space_info.count);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------\n");

    return ret;
}

int32
sys_usw_mpls_ilm_show(uint8 lchip, ctc_mpls_ilm_t* p_mpls_ilm, uint8  show_type, uint8 ilm_all)
{
    int32 ret = CTC_E_NONE;
    uint32 nh_id[SYS_USW_MAX_ECPN];
    ctc_mpls_ilm_t mpls_ilm;

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_mpls_ilm);
    SYS_MPLS_INIT_CHECK(lchip);
    SYS_MPLS_DUMP_ILM(p_mpls_ilm);
    sal_memset(&mpls_ilm, 0 ,sizeof(mpls_ilm));
    ctc_debug_set_flag("mpls", "mpls", 1, CTC_DEBUG_LEVEL_DUMP, TRUE);
    SYS_MPLS_LOCK(lchip);
    if (1 == show_type)
    {
        if (ilm_all)
        {
            ret = ctc_hash_traverse(p_usw_mpls_master[lchip]->p_mpls_hash, (hash_traversal_fn) _sys_usw_show_mpls_ilm_all, (void *)&mpls_ilm);
        }
        else
        {
            ret = _sys_usw_mpls_get_ilm(lchip, nh_id, p_mpls_ilm);
            if (ret < 0)
            {
                SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", "% Entry not exist");
                SYS_MPLS_UNLOCK(lchip);
                return CTC_E_NONE;
            }
            ret = _sys_usw_show_ilm_info_detail(lchip, p_mpls_ilm);
        }
    }
    else if (2 == show_type)
    {
        ret = _sys_usw_mpls_ilm_show_by_space(lchip, p_mpls_ilm->spaceid);
    }
    else
    {
        ret = _sys_usw_show_ilm_info(lchip, p_mpls_ilm);
    }

    ctc_debug_set_flag("mpls", "mpls", 1, CTC_DEBUG_LEVEL_INFO, FALSE);
    SYS_MPLS_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_mpls_sqn_show(uint8 lchip, uint8 index)
{
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

}

int32
sys_usw_show_mpls_status(uint8 lchip)
{
    uint32 field_val = 0;
    uint32 cmd;

    SYS_MPLS_INIT_CHECK(lchip);
    SYS_MPLS_LOCK(lchip);
    cmd = DRV_IOR(IpeMplsCtl_t, IpeMplsCtl_globalLabelSpace_f);
    CTC_ERROR_RETURN_MPLS_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd , &field_val));

    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------Mpls Status---------------------\r\n");
    if (!field_val)
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6s \n", "Global Space", "Yes");
    }
    else
    {
        SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6s \n", "Global Space", "No");
    }
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Mpls Space Count", CTC_MPLS_SPACE_NUMBER);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Mpls Key Count", SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS));
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Used Key Count", p_usw_mpls_master[lchip]->cur_ilm_num);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Used Tcam Key Count", p_usw_mpls_master[lchip]->mpls_cur_tcam);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Mpls Pop Ad Index", p_usw_mpls_master[lchip]->pop_ad_idx);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6u \n", "Mpls Decap Ad Index", p_usw_mpls_master[lchip]->decap_ad_idx);
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-30s:%6s \n", "Decap mode", p_usw_mpls_master[lchip]->decap_mode?"Iloop":"Normal");
    SYS_MPLS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------\r\n");
    SYS_MPLS_UNLOCK(lchip);
    return CTC_E_NONE;
}

#endif

