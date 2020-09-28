#if (FEATURE_MODE == 0)
/**
 @file sys_usw_stacking.c

 @date 2010-3-9

 @version v2.0

  This file contains stakcing sys layer function implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_stacking.h"
#include "ctc_packet.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_error.h"
#include "ctc_warmboot.h"
#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_ftm.h"
#include "sys_usw_register.h"
#include "sys_usw_common.h"
#include "sys_usw_port.h"
#include "sys_usw_linkagg.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_stacking.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "sys_usw_dmps.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

#define SYS_STK_TRUNK_MIN_ID 1

#define SYS_STK_TRUNK_VEC_BLOCK_NUM  64
#define SYS_STK_TRUNK_VEC_BLOCK_SIZE 8

#define SYS_STK_KEEPLIVE_VEC_BLOCK_SIZE 1024
#define SYS_STK_KEEPLIVE_MEM_MAX 3



#define SYS_STK_MAX_GCHIP     128
#define SYS_STK_PORT_BLOCK_PROFILE_NUM 8
#define SYS_STK_PORT_FWD_PROFILE_NUM 8

#define SYS_STK_INIT_CHECK(lchip) \
    do { \
        LCHIP_CHECK(lchip); \
        if (NULL == g_stacking_master[lchip]){ \
            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_STK_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(stacking, stacking, STK_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

#define SYS_STK_DBG_FUNC()           SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC,  "%s()\n", __FUNCTION__)
#define SYS_STK_DBG_DUMP(FMT, ...)  SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  FMT, ##__VA_ARGS__)
#define SYS_STK_DBG_INFO(FMT, ...)  SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO,  FMT, ##__VA_ARGS__)
#define SYS_STK_DBG_PARAM(FMT, ...) SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ##__VA_ARGS__)

#define STACKING_LOCK \
    if (g_stacking_master[lchip]->p_stacking_mutex) sal_mutex_lock(g_stacking_master[lchip]->p_stacking_mutex)
#define STACKING_UNLOCK \
    if (g_stacking_master[lchip]->p_stacking_mutex) sal_mutex_unlock(g_stacking_master[lchip]->p_stacking_mutex)

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

enum sys_stacking_out_encap_e
{
    SYS_STK_OUT_ENCAP_NONE                = 0,  /* 4'b0000: BridgeHeader with no ipv4/ipv6   */
    SYS_STK_OUT_ENCAP_IPV4                = 1,  /* 4'b0001: BridgeHeader with ipv4           */
    SYS_STK_OUT_ENCAP_IPV6                = 2  /* 4'b0002: BridgeHeader with ipv6           */
};
typedef enum sys_stacking_out_encap_e sys_stacking_out_encap_t;


enum sys_stacking_opf_type_e
{
    SYS_STK_OPF_MEM_BASE,
    SYS_STK_OPF_BLOCK,
    SYS_STK_OPF_FWD,
    SYS_STK_OPF_ABILITY,
    SYS_STK_OPF_MCAST_PROFILE,
    SYS_STK_OPF_MAX
};
typedef enum sys_stacking_opf_type_e sys_stacking_opf_type_t;


struct sys_stacking_trunk_info_s
{
    ctc_list_pointer_node_t list_head;
    uint32 trunk_id : 8;
    uint32 max_mem_cnt:8;
    uint32 mode:3;
    uint32 replace_en:1;
    uint32 rsv:12;
    ctc_stacking_hdr_encap_t encap_hdr;

    uint16   dsmet_offset;
    uint16   next_dsmet_offset;
    uint32   flag;
};
typedef struct sys_stacking_trunk_info_s sys_stacking_trunk_info_t;


struct sys_stacking_block_profile_s
{
    uint32 bitmap[SYS_STK_MAX_GCHIP][2];
	uint32 profile_id;
};
typedef struct sys_stacking_block_profile_s sys_stacking_block_profile_t;

struct sys_stacking_fwd_profile_s
{
    uint32 trunk[SYS_STK_MAX_GCHIP];
	uint32 profile_id;
};
typedef struct sys_stacking_fwd_profile_s sys_stacking_fwd_profile_t;


struct sys_stacking_keeplive_mem_s
{
    uint32    member;
    uint8    valid;
    uint8    rsv[3];
    uint16   dsmet_offset;
    uint16   next_dsmet_offset;
};
typedef struct sys_stacking_keeplive_mem_s sys_stacking_keeplive_mem_t;

struct sys_stacking_mcast_db_s
{
    uint8   type;
    uint8   rsv;
    uint16  id;

    uint32   head_met_offset;
    uint32   tail_met_offset;

    uint8 append_en;
    uint8 alloc_id;
    uint32 last_tail_offset;

    uint32   trunk_bitmap[CTC_STK_TRUNK_BMP_NUM];
    ctc_port_bitmap_t port_bitmap;
    uint32   cpu_port;
};
typedef struct sys_stacking_mcast_db_s sys_stacking_mcast_db_t;


struct sys_stacking_master_s
{
    uint8 stacking_en;
    uint8 fabric_mode;
	uint8 opf_type;
	uint8 trunk_mode;

    uint32 stacking_mcast_offset;

    uint8 mcast_mode;/*0: add trunk to mcast group auto; 1: add trunk to mcast group by user*/
    uint8 binding_trunk;/*0: binding port to trunk auto when add port to trunk; 1: binding by user*/
    uint8 bind_mcast_en;

    ctc_vector_t* p_trunk_vec;

	sys_stacking_block_profile_t* p_block_profile[SYS_STK_PORT_BLOCK_PROFILE_NUM];
	ctc_spool_t*   p_block_prof_spool;

    sys_stacking_fwd_profile_t* p_fwd_profile[SYS_STK_PORT_FWD_PROFILE_NUM];
    ctc_spool_t*   p_fwd_prof_spool;


    ctc_hash_t* mcast_hash;
    sys_stacking_mcast_db_t *p_default_prof_db;

    ctc_port_bitmap_t ing_port_en_bmp;               /*0:enable stk port when add trunk port, 1: enable stk port by API CTC_STK_PROP_STK_PORT_EN*/
    ctc_port_bitmap_t egr_port_en_bmp;               /*0:enable stk port when add trunk port, 1: enable stk port by API CTC_STK_PROP_STK_PORT_EN*/

    sal_mutex_t* p_stacking_mutex;
};
typedef struct sys_stacking_master_s sys_stacking_master_t;

#define SYS_STK_TRUNKID_RANGE_CHECK(val) \
    { \
        if ((val) < SYS_STK_TRUNK_MIN_ID || (val) > MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_ID)){ \
            return CTC_E_INVALID_PARAM; } \
        if ((g_stacking_master[lchip]->trunk_mode == 0) && (val >= MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS)/8))  \
        {                \
           return CTC_E_INVALID_PARAM;    \
        }                \
        if ((g_stacking_master[lchip]->trunk_mode == 1) && (val >= MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS)/32)) \
        {                \
           return CTC_E_INVALID_PARAM;    \
        }                \
    }

#define SYS_STK_CHIPID_CHECK(gchip) \
    { \
        if ((gchip) > MCHIP_CAP(SYS_CAP_GCHIP_CHIP_ID) || (gchip) == CTC_LINKAGG_CHIPID){ \
            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid global chip id \n");\
			return CTC_E_INVALID_CHIP_ID;\
 } \
    }

#define SYS_STK_DSCP_CHECK(dscp) \
    { \
        if ((dscp) > 63){ \
            return CTC_E_INVALID_PARAM; } \
    }

#define SYS_STK_COS_CHECK(dscp) \
    { \
        if ((dscp) > 7){ \
            return CTC_E_INVALID_PARAM; } \
    }

#define SYS_STK_LPORT_CHECK(lport) \
    { \
        if ((lport) >= MCHIP_CAP(SYS_CAP_STK_MAX_LPORT)){ \
            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk member port not valid\n");\
			return CTC_E_INVALID_PORT;\
 } \
    }


#define SYS_NH_NEXT_MET_ENTRY(lchip, dsmet_offset) ((SYS_NH_INVALID_OFFSET == dsmet_offset)?\
                                                         SYS_NH_INVALID_OFFSET : dsmet_offset)

sys_stacking_master_t* g_stacking_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
*
* Function
*
**
***************************************************************************/
extern int32
_sys_usw_port_set_glb_dest_port_by_dest_port(uint8 lchip, uint16 lport);
/***********************************************************
** SPOOL functions
************************************************************/
#define SPOOL ""

STATIC uint32
_sys_usw_stacking_block_profile_hash_make(sys_stacking_block_profile_t* backet)
{

    uint8* data                    = NULL;
    uint32   length                = 0;

    if (!backet)
    {
        return 0;
    }

    data = (uint8*)&backet->bitmap;
    length = sizeof(backet->bitmap);

    return ctc_hash_caculate(length, data);

}


STATIC bool
_sys_usw_stacking_block_profile_hash_compare(sys_stacking_block_profile_t* p_bucket,
                                               sys_stacking_block_profile_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (!sal_memcmp(p_bucket->bitmap, p_new->bitmap, sizeof(p_bucket->bitmap)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_stacking_block_profile_index_alloc(sys_stacking_block_profile_t* p_block_profile,
                                              uint8* lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t  opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[*lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_BLOCK;


    if (CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*lchip, &opf, 1, p_block_profile->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*lchip, &opf, 1, &value_32));
        p_block_profile->profile_id = value_32;
    }

    SYS_STK_DBG_INFO("stacking_block_profile_index_alloc :%d\n",p_block_profile->profile_id);


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_block_profile_index_free(sys_stacking_block_profile_t* p_block_profile, uint8* lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t  opf;

    value_32 = p_block_profile->profile_id;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[*lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_BLOCK;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, value_32));

    SYS_STK_DBG_INFO("stacking_block_profile_index_free :%d\n",value_32);
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_add_block_port_profile(uint8 lchip,
                                           ctc_stacking_stop_rchip_t *p_stop_rchip,
                                           sys_stacking_block_profile_t** pp_profile_get)
{
    uint32 cmd                     = 0;
    uint32 lport                   = 0;
    uint8 rchip                    = 0;
    uint32 port_block_sel          = 0;
	sys_stacking_block_profile_t* p_profile_old = NULL;
    sys_stacking_block_profile_t* p_new_profile = NULL;
    int32 ret = CTC_E_NONE;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_stop_rchip->src_gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexDstIsolateGroupSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_block_sel));

    p_new_profile = (sys_stacking_block_profile_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_block_profile_t));
    if (NULL == p_new_profile)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_new_profile, 0, sizeof(sys_stacking_block_profile_t));

    p_profile_old = g_stacking_master[lchip]->p_block_profile[port_block_sel];

    if (p_profile_old)
    {
        sal_memcpy(p_new_profile->bitmap, p_profile_old->bitmap, sizeof(p_new_profile->bitmap));
    }


	rchip = p_stop_rchip->rchip;
	p_new_profile->bitmap[rchip][0]= p_stop_rchip->pbm[0];
	p_new_profile->bitmap[rchip][1]= p_stop_rchip->pbm[1];

    CTC_ERROR_GOTO(ctc_spool_add(g_stacking_master[lchip]->p_block_prof_spool,
                                   p_new_profile,
                                   p_profile_old,
                                   pp_profile_get), ret, out);

    port_block_sel = (*pp_profile_get)->profile_id;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cFlexDstIsolateGroupSel_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &port_block_sel), ret, out);

    g_stacking_master[lchip]->p_block_profile[port_block_sel] = (*pp_profile_get);

out:
    if (p_new_profile)
    {
        mem_free(p_new_profile);
    }

    return ret;

}

STATIC int32
_sys_usw_stacking_add_static_block_port_profile(uint8 lchip)
{
    sys_stacking_block_profile_t* p_new_profile = NULL;
    int32 ret = CTC_E_NONE;

    p_new_profile = (sys_stacking_block_profile_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_block_profile_t));
    if (NULL == p_new_profile)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_new_profile, 0, sizeof(sys_stacking_block_profile_t));
    ret = ctc_spool_static_add(g_stacking_master[lchip]->p_block_prof_spool,
                                   p_new_profile);
    mem_free(p_new_profile);

    return ret;
}




STATIC uint32
_sys_usw_stacking_fwd_profile_hash_make(sys_stacking_fwd_profile_t* backet)
{
    uint8* data                    = NULL;
    uint32   length                = 0;

    if (!backet)
    {
        return 0;
    }

    data = (uint8*)&backet->trunk;
    length = sizeof(backet->trunk);

    return ctc_hash_caculate(length, data);
}


STATIC bool
_sys_usw_stacking_fwd_profile_hash_compare(sys_stacking_fwd_profile_t* p_bucket,
                                               sys_stacking_fwd_profile_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (!sal_memcmp(p_bucket->trunk, p_new->trunk, sizeof(p_bucket->trunk)))
    {
        return TRUE;
    }

    return FALSE;
}

STATIC int32
_sys_usw_stacking_fwd_profile_index_alloc(sys_stacking_fwd_profile_t* p_fwd_profile, uint8* lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t  opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[*lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_FWD;

    if (CTC_WB_ENABLE && CTC_WB_STATUS(*lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*lchip, &opf, 1, p_fwd_profile->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*lchip, &opf, 1, &value_32));
        p_fwd_profile->profile_id = value_32;
    }

    SYS_STK_DBG_INFO("stacking_fwd_profile_index_alloc :%d\n",p_fwd_profile->profile_id);



    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_fwd_profile_index_free(sys_stacking_fwd_profile_t* p_fwd_profile, uint8* lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t  opf;

    value_32 = p_fwd_profile->profile_id;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[*lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_FWD;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*lchip, &opf, 1, value_32));

    SYS_STK_DBG_INFO("stacking_fwd_profile_index_free :%d\n",value_32);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_add_fwd_port_profile(uint8 lchip,
                                           ctc_stacking_trunk_t *p_trunk,
                                           uint32* p_profile, bool is_del)
{
    uint32 cmd                     = 0;
    uint32 lport                   = 0;
    uint32 rchip                   = 0;
    uint32 port_fwd_sel            = 0;
    uint32 enable                  = 0;
	sys_stacking_fwd_profile_t* p_profile_get = NULL;
	sys_stacking_fwd_profile_t* p_profile_old = NULL;
    sys_stacking_fwd_profile_t new_profile;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->src_gport, lchip, lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

    sal_memset(&new_profile, 0, sizeof(new_profile));

    p_profile_old = g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel];

    if (p_profile_old)
    {
        sal_memcpy(new_profile.trunk, p_profile_old->trunk, sizeof(new_profile.trunk));
    }

	rchip = p_trunk->remote_gchip;
	new_profile.trunk[rchip]= is_del?0: p_trunk->trunk_id;

    CTC_ERROR_RETURN(ctc_spool_add(g_stacking_master[lchip]->p_fwd_prof_spool,
                                   &new_profile,
                                   p_profile_old,
                                   &p_profile_get));

    port_fwd_sel = p_profile_get->profile_id ;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

    SYS_STK_DBG_INFO("add_fwd_port_profile lport :%d, sel:%d\n",lport, port_fwd_sel);

    enable = 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSelEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &enable));

    g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel] = p_profile_get;
    *p_profile = p_profile_get->profile_id;

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_stacking_add_static_fwd_port_profile(uint8 lchip)
{
    sys_stacking_fwd_profile_t new_profile;

    sal_memset(&new_profile, 0, sizeof(new_profile));
    CTC_ERROR_RETURN(ctc_spool_static_add(g_stacking_master[lchip]->p_fwd_prof_spool,
                                   &new_profile));
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_add_trunk_port_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint32 field_val               = 0;
    uint32 port_fwd_sel            = 0;
    uint32 rchip                   = 0;



    CTC_ERROR_RETURN(_sys_usw_stacking_add_fwd_port_profile(lchip, p_trunk, &port_fwd_sel, FALSE));

    for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
    {
        if (g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel])
        {
            field_val = g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel]->trunk[rchip];
        }
        else
        {
            field_val = 0;
        }


        index = (port_fwd_sel << 7) + rchip;
        cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }



    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_remove_trunk_port_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint32 field_val               = 0;
    uint32 port_fwd_sel            = 0;
    uint32 rchip                   = 0;

    CTC_ERROR_RETURN(_sys_usw_stacking_add_fwd_port_profile(lchip, p_trunk, &port_fwd_sel, TRUE));

    for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
    {
        if (g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel])
        {
            field_val = g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel]->trunk[rchip];
        }
        else
        {
            field_val = 0;
        }

        index = (port_fwd_sel << 7) + rchip;
        cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
    }



    return CTC_E_NONE;
}

#if 0
STATIC int32 _sys_usw_stacking_trunk_traversal_fn (void* array_data, uint32 index, void* user_data)
{
    uint8* exist_trunk = (uint8*)user_data;

    exist_trunk[index] = ((sys_stacking_trunk_info_t*)array_data)->trunk_id;
    return 0;
}

STATIC int32
_sys_usw_stacking_add_all_rchip_to_trunk(uint8 lchip, uint8 trunk_id)
{
    uint32 cmd;
    uint32 field_val;
    uint16 loop1,loop2;
    uint8 exist_trunk[64] = {0};

    /*Fabric mode*/
    if (!g_stacking_master[lchip]->fabric_mode)
    {
        return CTC_E_NONE;
    }

    ctc_vector_traverse2(g_stacking_master[lchip]->p_trunk_vec, 0, _sys_usw_stacking_trunk_traversal_fn,(void*)exist_trunk);
    cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
    loop1 = 0;
    while(loop1 < MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
    {
        for(loop2=0; loop2 < g_stacking_master[lchip]->p_trunk_vec->used_cnt; loop2++)
        {
            field_val = exist_trunk[loop2];
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop1, cmd, &field_val));
            loop1++;
            if (loop1 >= MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
            {
                break;
            }
        }

        if (loop1 >= MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
        {
            break;
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_del_all_rchip_from_trunk(uint8 lchip, uint8 trunk_id)
{
    uint32 port_fwd_sel            = 0;
    uint16 rchip                   = 0;
    uint8 gchip                    = 0;
    uint16 lport                   = 0;
    uint32 cmd                     = 0;
    uint32 field_val               = 0;
    uint16 i,loop;
    uint8 exist_trunk[256] = {0};
    ctc_stacking_trunk_t trunk;

    /*Fabric mode*/
    if (g_stacking_master[lchip]->fabric_mode)
    {
        cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        if (0 == g_stacking_master[lchip]->p_trunk_vec->used_cnt)
        {
            for(i = 0; i< MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM);i++)
            {
                field_val = 0;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &field_val));
            }
        }
        else
        {
            ctc_vector_traverse2(g_stacking_master[lchip]->p_trunk_vec, 0, _sys_usw_stacking_trunk_traversal_fn,(void*)exist_trunk);
            i=0;
            while(i< MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
            {
                for(loop=0; loop < g_stacking_master[lchip]->p_trunk_vec->used_cnt;loop++)
                {
                    field_val = exist_trunk[loop];
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &field_val));
                    i++;
                    if (i >= MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
                    {
                        break;
                    }
                }

                if (i >= MCHIP_CAP(SYS_CAP_STK_SGMAC_GROUP_NUM))
                {
                    break;
                }
            }
        }
		return CTC_E_NONE;
    }

    /*Unicast rchip select path mode*/
    sal_memset(&trunk, 0, sizeof(trunk));

    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_STK_MAX_LPORT); lport++)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

        if (0 == port_fwd_sel || NULL == g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel])
        {
            continue;
        }

        for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
        {
            if (trunk_id != g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel]->trunk[rchip])
            {
                continue;
            }

            trunk.trunk_id = trunk_id;
            trunk.remote_gchip = rchip;
            sys_usw_get_gchip_id(lchip, &gchip);
            trunk.src_gport =  SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);

            CTC_ERROR_RETURN(_sys_usw_stacking_remove_trunk_port_rchip(lchip, &trunk));
        }

    }

    return CTC_E_NONE;
}
#endif

/***********************************************************
** TRUNK functions
************************************************************/
#define _____TRUNK_____ ""
STATIC int32
_sys_usw_stacking_get_member_ref_cnt(uint8 lchip, uint8 channel, uint16 *port_ref_cnt,uint8 excp_trunk_id)
{
    uint32 cmd                     = 0;
    uint32 cmd_r_group             = 0;
    uint16 mem_cnt                 = 0;
    uint16 mem_base                = 0;
    uint16 mem_idx                 = 0;
    uint8 mode                     = 0;
    uint32 mem_channel_id          = 0xFF;
    uint8 trunk_id                 = 0;
    DsLinkAggregateChannelGroup_m  ds_link_aggregate_sgmac_group;

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(ds_link_aggregate_sgmac_group));

    cmd_r_group = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);

    for(trunk_id = SYS_STK_TRUNK_MIN_ID; trunk_id < MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_ID); trunk_id++)
    {
        if(excp_trunk_id != 0xFF && (excp_trunk_id ==trunk_id) )
        {
          continue;
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd_r_group, &ds_link_aggregate_sgmac_group));
        mem_cnt     = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group);
        mem_base    = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group);
        mode        = GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group);

        if (0 == mem_cnt)
        {
            continue;
        }

        for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
        {
            if (mode == 0)
            {
                cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_base + mem_idx, cmd, &mem_channel_id));
            }
            else if(mode == 1)
            {
                cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f + mem_idx);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &mem_channel_id));
            }

            if (channel == mem_channel_id)
            {
                (*port_ref_cnt)++;
                break;
            }

        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_check_port_status(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile)
{
    uint32 depth                   = 0;
    uint16 index = 0;

    sys_usw_queue_get_port_depth(lchip, gport, &depth);
    while (depth)
    {
        sal_task_sleep(20);
        if ((index++) > 50)
        {
            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "the queue depth:(%d) \n", depth);
            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " HW operation timed out \n");
            return CTC_E_HW_TIME_OUT;
        }
        sys_usw_queue_get_port_depth(lchip, gport, &depth);
    }
    sal_task_sleep(20);
    return CTC_E_NONE;
}
#if 0
STATIC int32
_sys_usw_stacking_restore_port_status(uint8 lchip, uint32 gport, sys_qos_shape_profile_t* p_shp_profile)
{
    CTC_PTR_VALID_CHECK(p_shp_profile);
    /*enqdrop disable*/
    CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, gport, FALSE, p_shp_profile));

    return CTC_E_NONE;
}
#endif
STATIC int32
_sys_usw_stacking_get_trunk_info(uint8 lchip, uint8 trunk_id,
                                   uint16* cur_mem_cnt, uint16* mem_base, uint8* is_dlb)
{
    uint32 cmd = 0;
    DsLinkAggregateChannelGroup_m  ds_link_aggregate_sgmac_group;

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(ds_link_aggregate_sgmac_group));

    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));

	*cur_mem_cnt = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group);
	*mem_base    = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group);
    *is_dlb      = (GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group) == 1)?1:0;

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_stacking_trunk_alloc_mem_base(uint8 lchip,
                                         sys_stacking_trunk_info_t* p_sys_trunk,
                                         uint16 max_mem_cnt, uint16* mem_base)
{
    uint32 value_32 = 0;
	uint8  trunk_id = 0;
    sys_usw_opf_t  opf;

    trunk_id = p_sys_trunk->trunk_id;

    switch(g_stacking_master[lchip]->trunk_mode)
    {
    case 0:
        if (trunk_id >= MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS) / 8)
        {
           return CTC_E_INVALID_PARAM;
        }
        *mem_base = trunk_id*8;
		p_sys_trunk->max_mem_cnt = 8;
        break;

    case 1:
        if (trunk_id >= MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS) / 32)
        {
           return CTC_E_INVALID_PARAM;
        }

		*mem_base = trunk_id*32;


        if (p_sys_trunk->mode == CTC_STK_LOAD_DYNAMIC)
        {
            p_sys_trunk->max_mem_cnt = 32;
        }
        else
        {
            p_sys_trunk->max_mem_cnt = MCHIP_CAP(SYS_CAP_STK_TRUNK_STATIC_MAX_MEMBERS);
        }

        break;

    case 2:
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = g_stacking_master[lchip]->opf_type;
        opf.pool_index = SYS_STK_OPF_MEM_BASE;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, max_mem_cnt, &value_32));
        *mem_base = value_32;
        p_sys_trunk->max_mem_cnt = max_mem_cnt;
        break;

    default:
        return CTC_E_INVALID_PARAM;

    }


    SYS_STK_DBG_INFO("_sys_usw_stacking_trunk_alloc_mem_base :%d\n",*mem_base);

    return CTC_E_NONE;

}


STATIC int32
_sys_usw_stacking_trunk_free_mem_base(uint8 lchip,
                                        sys_stacking_trunk_info_t* p_sys_trunk,
                                        uint16 max_mem_cnt, uint16 mem_base)
{
    uint32 value_32 = 0;
    sys_usw_opf_t  opf;

    switch(g_stacking_master[lchip]->trunk_mode)
    {
    case 0:
    case 1:
        break;

    case 2:
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = g_stacking_master[lchip]->opf_type;
        opf.pool_index = SYS_STK_OPF_MEM_BASE;
        value_32 = mem_base;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, max_mem_cnt, value_32));
        break;

    default:
        return CTC_E_INVALID_PARAM;

    }

    SYS_STK_DBG_INFO("_sys_usw_stacking_trunk_free_mem_base :%d\n",mem_base);

    return CTC_E_NONE;

}




STATIC int32
_sys_usw_stacking_clear_flow_active(uint8 lchip, uint8 trunk_id)
{
    uint32 cmd_r                   = 0;
    uint32 cmd_w                   = 0;
    uint16 flow_num                = 0;
    uint32 flow_base               = 0;
    uint16 index                   = 0;
    DsLinkAggregateChannelGroup_m ds_link_aggregate_channel_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_channel_member;

    sal_memset(&ds_link_aggregate_channel_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    sal_memset(&ds_link_aggregate_channel_member, 0, sizeof(DsLinkAggregateChannelMember_m));

    cmd_r = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd_r, &ds_link_aggregate_channel_group));

    flow_base = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_channel_group);

    flow_num = 32;

    /* clear active */
    cmd_r = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
    cmd_w = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);

    for (index = 0; index < flow_num; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, flow_base+index, cmd_r, &ds_link_aggregate_channel_member));
        SetDsLinkAggregateChannelMember(V, active_f, &ds_link_aggregate_channel_member, 0);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, flow_base+index, cmd_w, &ds_link_aggregate_channel_member));
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_failover_add_member(uint8 lchip, uint8 trunk_id, uint32 gport)
{
    uint32 cmd                     = 0;
    uint32 channel_id              = 0;
    DsLinkAggregateChannel_m linkagg_channel;


    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x gport =  %d \n", trunk_id, gport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, u1_g2_linkAggregationChannelGroup_f, &linkagg_channel, trunk_id);
    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_failover_del_member(uint8 lchip, uint8 trunk_id, uint32 gport)
{
    uint32 cmd                     = 0;
    uint8 channel_id               = 0;
    DsLinkAggregateChannel_m linkagg_channel;


    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = 0x%x gport =  %d \n", trunk_id, gport);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);

    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 0);
    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 0);
    SetDsLinkAggregateChannel(V, u1_g2_linkAggregationChannelGroup_f, &linkagg_channel, 0);
    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_dlb_add_member_channel(uint8 lchip, uint8 trunk_id, uint16 channel_id)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "trunk_id = 0x%x channel_id =  %d \n", trunk_id, channel_id);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, linkChangeEn_f, &linkagg_channel, 0);
    SetDsLinkAggregateChannel(V, u1_g2_linkAggregationChannelGroup_f, &linkagg_channel, trunk_id);

    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_dlb_del_member_channel(uint8 lchip, uint8 trunk_id, uint8 channel_id)
{
    uint32 cmd = 0;
    DsLinkAggregateChannel_m linkagg_channel;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "trunk_id = 0x%x channel_id =  %d \n", trunk_id, channel_id);

    sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));

    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_dlb_add_member(uint8 lchip, uint8 trunk_id, uint8 port_index, uint8 channel_id)
{
    uint32 cmd = 0;
    uint32 field_val = channel_id;

    cmd = DRV_IOW(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f + port_index);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_val));

    CTC_ERROR_RETURN(_sys_usw_stacking_dlb_add_member_channel(lchip, trunk_id, channel_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_dlb_del_member(uint8 lchip, uint8 trunk_id, uint8 port_index, uint8 tail_index, uint8 channel_id)
{
    uint32 cmd                     = 0;
    uint32 temp_channel_id          = 0;
    uint32 field_value               = 0;


    /* copy the last one to the removed position */
    field_value = channel_id;
    cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+tail_index);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_value));

    cmd = DRV_IOW(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+port_index);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_value));

    /* set the last one to reserve */
    temp_channel_id = MCHIP_CAP(SYS_CAP_CHANID_DROP);
    cmd = DRV_IOW(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+tail_index);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &temp_channel_id));

    CTC_ERROR_RETURN(_sys_usw_stacking_dlb_del_member_channel(lchip, trunk_id, channel_id));

    return CTC_E_NONE;
}



int32
sys_usw_stacking_get_rsv_trunk_number(uint8 lchip, uint8* number)
{
    uint8 trunk_num = 0;
    uint8 trunk_idx = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    CTC_PTR_VALID_CHECK(number);

    if (NULL == g_stacking_master[lchip])
    {
        *number = 0;
        return CTC_E_NONE;
    }

    STACKING_LOCK;
    for (trunk_idx = SYS_STK_TRUNK_MIN_ID; trunk_idx < MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_ID); trunk_idx++)
    {
        p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_idx);
        if (NULL != p_sys_trunk)
        {
            trunk_num++;
        }
    }
    STACKING_UNLOCK;

    if (trunk_num == 0)
    {
        *number = 0;
        return CTC_E_NONE;
    }

    if (trunk_num > 2)
    {
        *number = 8;
    }
    else
    {
        *number = 2;
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_stacking_hdr_set(uint8 lchip, ctc_stacking_hdr_glb_t* p_hdr_glb)
{
    uint32 cmd  = 0;
    uint32 ipv6_sa[CTC_IPV6_ADDR_LEN] = {0};
    IpeHeaderAdjustCtl_m ipe_header_adjust_ctl;
    EpePktHdrCtl_m epe_pkt_hdr_ctl;

    SYS_STK_DSCP_CHECK(p_hdr_glb->ip_dscp);
    SYS_STK_COS_CHECK(p_hdr_glb->cos);

    if (!DRV_IS_DUET2(lchip))
    {
        if (p_hdr_glb->udp_en
        ||  p_hdr_glb->ether_type == 0x0800
        || p_hdr_glb->ether_type == 0x86DD)
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    sal_memset(&ipe_header_adjust_ctl, 0, sizeof(ipe_header_adjust_ctl));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));

    sal_memset(&epe_pkt_hdr_ctl, 0, sizeof(epe_pkt_hdr_ctl));
    cmd = DRV_IOR(EpePktHdrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_hdr_ctl));

    SetIpeHeaderAdjustCtl(V, headerMacDaCheckEn_f, &ipe_header_adjust_ctl, p_hdr_glb->mac_da_chk_en);
    SetIpeHeaderAdjustCtl(V, headerTpid_f, &ipe_header_adjust_ctl, p_hdr_glb->vlan_tpid);
    SetIpeHeaderAdjustCtl(V, headerEtherTypeCheckDisable_f, &ipe_header_adjust_ctl, p_hdr_glb->ether_type_chk_en?0:1);
    SetIpeHeaderAdjustCtl(V, headerEtherType_f, &ipe_header_adjust_ctl, p_hdr_glb->ether_type);
    SetIpeHeaderAdjustCtl(V, headerIpProtocol_f, &ipe_header_adjust_ctl, p_hdr_glb->ip_protocol);
    SetIpeHeaderAdjustCtl(V, headerUdpEn_f, &ipe_header_adjust_ctl, p_hdr_glb->udp_en);
    SetIpeHeaderAdjustCtl(V, udpSrcPort_f, &ipe_header_adjust_ctl, p_hdr_glb->udp_dest_port);
    SetIpeHeaderAdjustCtl(V, udpDestPort_f, &ipe_header_adjust_ctl, p_hdr_glb->udp_src_port);

    SetEpePktHdrCtl(V, tpid_f, &epe_pkt_hdr_ctl, p_hdr_glb->vlan_tpid);
    SetEpePktHdrCtl(V, headerEtherType_f, &epe_pkt_hdr_ctl, p_hdr_glb->ether_type);
    SetEpePktHdrCtl(V, headerIpProtocol_f, &epe_pkt_hdr_ctl, p_hdr_glb->ip_protocol);
    SetEpePktHdrCtl(V, headerTtl_f, &epe_pkt_hdr_ctl, p_hdr_glb->ip_ttl);
    SetEpePktHdrCtl(V, headerDscp_f, &epe_pkt_hdr_ctl, p_hdr_glb->ip_dscp);
    SetEpePktHdrCtl(V, headerCos_f, &epe_pkt_hdr_ctl, p_hdr_glb->cos);


    if (p_hdr_glb->is_ipv4)
    {
            SetEpePktHdrCtl(V, headerIpSa_f, &epe_pkt_hdr_ctl, p_hdr_glb->ipsa.ipv4);
    }
    else
    {
        ipv6_sa[0] = p_hdr_glb->ipsa.ipv6[3];
        ipv6_sa[1] = p_hdr_glb->ipsa.ipv6[2];
        ipv6_sa[2] = p_hdr_glb->ipsa.ipv6[1];
        ipv6_sa[3] = p_hdr_glb->ipsa.ipv6[0];
        SetEpePktHdrCtl(A, headerIpSa_f, &epe_pkt_hdr_ctl, ipv6_sa);
    }

    SetEpePktHdrCtl(V, headerUdpEn_f, &epe_pkt_hdr_ctl, p_hdr_glb->udp_en);
    SetEpePktHdrCtl(V, useUdpPayloadLength_f, &epe_pkt_hdr_ctl, 1);
    SetEpePktHdrCtl(V, useIpPayloadLength_f, &epe_pkt_hdr_ctl, 1);
    SetEpePktHdrCtl(V, udpSrcPort_f, &epe_pkt_hdr_ctl, p_hdr_glb->udp_src_port);
    SetEpePktHdrCtl(V, udpDestPort_f, &epe_pkt_hdr_ctl, p_hdr_glb->udp_dest_port);

    /*Encap header param*/
    cmd = DRV_IOW(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));

    cmd = DRV_IOW(EpePktHdrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_hdr_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_hdr_get(uint8 lchip, ctc_stacking_hdr_glb_t* p_hdr_glb)
{
    uint32 cmd  = 0;
    uint32 ipv6_sa[CTC_IPV6_ADDR_LEN] = {0};
    IpeHeaderAdjustCtl_m ipe_header_adjust_ctl;
    EpePktHdrCtl_m epe_pkt_hdr_ctl;

    sal_memset(&ipe_header_adjust_ctl, 0, sizeof(ipe_header_adjust_ctl));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_header_adjust_ctl));

    sal_memset(&epe_pkt_hdr_ctl, 0, sizeof(epe_pkt_hdr_ctl));
    cmd = DRV_IOR(EpePktHdrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_hdr_ctl));

    GetIpeHeaderAdjustCtl(V, headerMacDaCheckEn_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, headerTpid_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, headerEtherTypeCheckDisable_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, headerEtherType_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, headerIpProtocol_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, headerUdpEn_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, udpSrcPort_f, &ipe_header_adjust_ctl);
    GetIpeHeaderAdjustCtl(V, udpDestPort_f, &ipe_header_adjust_ctl);

    p_hdr_glb->mac_da_chk_en     = GetIpeHeaderAdjustCtl(V, headerMacDaCheckEn_f, &ipe_header_adjust_ctl);
    p_hdr_glb->vlan_tpid         = GetIpeHeaderAdjustCtl(V, headerTpid_f, &ipe_header_adjust_ctl);
    p_hdr_glb->ether_type_chk_en = GetIpeHeaderAdjustCtl(V, headerEtherTypeCheckDisable_f, &ipe_header_adjust_ctl) ? 0 : 1;
    p_hdr_glb->ether_type        = GetIpeHeaderAdjustCtl(V, headerEtherType_f, &ipe_header_adjust_ctl);

    p_hdr_glb->ip_protocol   = GetIpeHeaderAdjustCtl(V, headerIpProtocol_f, &ipe_header_adjust_ctl);
    p_hdr_glb->udp_en        = GetIpeHeaderAdjustCtl(V, headerUdpEn_f, &ipe_header_adjust_ctl);
    p_hdr_glb->udp_dest_port = GetIpeHeaderAdjustCtl(V, udpSrcPort_f, &ipe_header_adjust_ctl);
    p_hdr_glb->udp_src_port  = GetIpeHeaderAdjustCtl(V, udpDestPort_f, &ipe_header_adjust_ctl);

    p_hdr_glb->ip_protocol   = GetEpePktHdrCtl(V, headerIpProtocol_f, &epe_pkt_hdr_ctl);
    p_hdr_glb->ip_ttl        = GetEpePktHdrCtl(V, headerTtl_f, &epe_pkt_hdr_ctl);
    p_hdr_glb->ip_dscp       = GetEpePktHdrCtl(V, headerDscp_f, &epe_pkt_hdr_ctl);
    p_hdr_glb->cos           = GetEpePktHdrCtl(V, headerCos_f, &epe_pkt_hdr_ctl);


    p_hdr_glb->is_ipv4 = (p_hdr_glb->ether_type == 0x0800);

    GetEpePktHdrCtl(A, headerIpSa_f, &epe_pkt_hdr_ctl, ipv6_sa);

    if (p_hdr_glb->is_ipv4)
    {
        p_hdr_glb->ipsa.ipv4        = ipv6_sa[0];
    }
    else
    {
        sal_memcpy(p_hdr_glb->ipsa.ipv6, ipv6_sa, sizeof(ipv6_addr_t));
        p_hdr_glb->ipsa.ipv6[3] = ipv6_sa[0];
        p_hdr_glb->ipsa.ipv6[2] = ipv6_sa[1];
        p_hdr_glb->ipsa.ipv6[1] = ipv6_sa[2];
        p_hdr_glb->ipsa.ipv6[0] = ipv6_sa[3];
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_register_en(uint8 lchip, uint8 enable, uint8 mode, uint8 version)
{
    uint32 cmd                     = 0;
    uint32 value                   = 0;
    uint32 ds_nh_offset            = 0;
    uint32 value_mode              = 0;
    uint32 field_val               = 0;
    uint8 gchip_id                 = 0;
    uint8 sub_queue_id             = 0;
    uint8 grp_id                   = 0;
    IpeHeaderAdjustCtl_m  ipe_hdr_adjust_ctl;
    IpeFwdCtl_m           ipe_fwd_ctl;
    IpeLookupCtl_m        ipe_lookup_ctl;
    IpeLoopbackHeaderAdjustCtl_m ipe_loop_hdr_adj_ctl;
    MetFifoCtl_m          met_fifo_ctl;
    QWriteSgmacCtl_m      qwrite_sgmac_ctl;
	QWritePortIsolateCtl_m  qwrite_port_isolation_ctl;
    EpeHdrAdjustCtl_m     epe_hdr_adjust_ctl;
    EpeNextHopCtl_m       epe_next_hop_ctl;
    EpePktProcCtl_m       epe_pkt_proc_ctl;
    EpeHeaderEditCtl_m    epe_header_edit_ctl;
    EpePktHdrCtl_m        epe_pkt_hdr_ctl;
    BufferStoreCtl_m      buffer_store_ctl;
    EpeHdrAdjustChanCtl_m epe_hdr_adjust_chan_ctl;
    LagEngineCtl_m        lag_engine_ctl;
    IpeFwdCflexUplinkLagHashCtl_m uplink_lag_hash_ctl;
    IpeLegacyCFHeaderXlateCtl_m ipe_legacy_cfhdr_xlate_ctl;
    EpeLegacyCFHeaderXlateCtl_m epe_legacy_cfhdr_xlate_ctl;
    DsGlbDestPortMap_m port_map;

    uint32 packet_header_en[2] = {0};
     /*uint32 chip_discard[4] = {0};*/

    value = enable ? 1 : 0;
    value_mode = (mode == 1) ? 0 : value;
    sys_usw_get_gchip_id(lchip, &gchip_id);

    /*IPE Hdr Adjust Ctl*/
    sal_memset(&ipe_hdr_adjust_ctl, 0, sizeof(IpeHeaderAdjustCtl_m));
    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_hdr_adjust_ctl));
    SetIpeHeaderAdjustCtl(V, packetHeaderBypassAll_f, &ipe_hdr_adjust_ctl, value);
    SetIpeHeaderAdjustCtl(V, discardNonPacketHeader_f, &ipe_hdr_adjust_ctl, value_mode);
    SetIpeHeaderAdjustCtl(V, cFlexLookupDestMap_f, &ipe_hdr_adjust_ctl, SYS_RSV_PORT_SPINE_LEAF_PORT);
    SetIpeHeaderAdjustCtl(V, cFlexLookupDestMapMask_f, &ipe_hdr_adjust_ctl, 0x2001FF);

    /*SDK not support BrgHdr in CFlexHeader*/
    SetIpeHeaderAdjustCtl(V, cfHeaderLen_f, &ipe_hdr_adjust_ctl, 0);
	SetIpeHeaderAdjustCtl(V, packetHeaderAsCFlexHeader_f, &ipe_hdr_adjust_ctl, 0);

    SetIpeHeaderAdjustCtl(V, cFlexSkipHeaderParser_f, &ipe_hdr_adjust_ctl, 1);
	SetIpeHeaderAdjustCtl(V, forceSkipHeaderParser_f, &ipe_hdr_adjust_ctl, 0);
	SetIpeHeaderAdjustCtl(V, cflexDebugEn_f, &ipe_hdr_adjust_ctl, 1);

    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetIpeHeaderAdjustCtl(V, legacyCFHeaderEn_f, &ipe_hdr_adjust_ctl, 1);
        SetIpeHeaderAdjustCtl(V, cFlexPacketTypeMode_f, &ipe_hdr_adjust_ctl, (mode?1:0));
        SetIpeHeaderAdjustCtl(V, sourcePortMode_f, &ipe_hdr_adjust_ctl, 1);
        SetIpeHeaderAdjustCtl(V, stackingMacLearningMode_f, &ipe_hdr_adjust_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetIpeHeaderAdjustCtl(V, legacyCFHeaderEn_f, &ipe_hdr_adjust_ctl, 0);
        SetIpeHeaderAdjustCtl(V, cFlexPacketTypeMode_f, &ipe_hdr_adjust_ctl, 1);
        SetIpeHeaderAdjustCtl(V, sourcePortMode_f, &ipe_hdr_adjust_ctl, 0);
        SetIpeHeaderAdjustCtl(V, stackingMacLearningMode_f, &ipe_hdr_adjust_ctl, 1);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(IpeHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_hdr_adjust_ctl));


    field_val = 1;
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_bypassAllcFlexFwdSgGroupSelEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_bypassAllcFlexDstIsolateGroupSelEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_stackingLearningLogicPortEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    /*ingress edit*/
    sal_memset(&ipe_fwd_ctl, 0, sizeof(IpeFwdCtl_m));
    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));

    SetIpeFwdCtl(V, packetHeaderAsCFlexHeader_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, cfHeaderLen_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, logicSrcPort32kMode_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, cFlexIsolateType_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, globalPortIsolateType_f, &ipe_fwd_ctl, 0);


    SetIpeFwdCtl(V, uplinkReflectCheckEn_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, discardReplicationPenultimateHop_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, ingressEditNexthopExt_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, ingressEditNexthopExtSpan_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, cfHeaderNextHopExt_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, ptpCflexEnable_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, cflexVlanEditMode_f, &ipe_fwd_ctl, 1);

    SetIpeFwdCtl(V, cFlexSrcIsolateFwdTypeEn_f, &ipe_fwd_ctl, 0xFFFFFFFF);
    SetIpeFwdCtl(V, c2cCheckDisable_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, cFlexDstIsolateMcastEn_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, cFlexDstIsolateUcastEn_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, cFlexSrcIsolateMode_f, &ipe_fwd_ctl, 1);


    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetIpeFwdCtl(V, sourcePortMode_f, &ipe_fwd_ctl, 1);
        SetIpeFwdCtl(V, legacyCFHeaderEn_f, &ipe_fwd_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetIpeFwdCtl(V, sourcePortMode_f, &ipe_fwd_ctl, 0);
        SetIpeFwdCtl(V, legacyCFHeaderEn_f, &ipe_fwd_ctl, 0);
    }
    CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip,
    SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH,  &ds_nh_offset));
    SetIpeFwdCtl(V, ingressEditNexthopPtr_f, &ipe_fwd_ctl, ds_nh_offset);

    CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip,
    SYS_NH_RES_OFFSET_TYPE_MIRROR_NH,  &ds_nh_offset));
    SetIpeFwdCtl(V, ingressEditNexthopPtrSpan_f, &ipe_fwd_ctl, ds_nh_offset);

    CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_C2C_PKT, &sub_queue_id));
    grp_id = (sub_queue_id / MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM)) + 1;
    /*to cpu based on prio*/
    field_val =  SYS_ENCODE_EXCP_DESTMAP_GRP(gchip_id, grp_id);

    SetIpeFwdCtl(V, neighborDiscoveryDestMap_f, &ipe_fwd_ctl, field_val);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl));

    /*Ipe Lookup Ctl*/
    sal_memset(&ipe_lookup_ctl, 0, sizeof(IpeLookupCtl_m));
    cmd = DRV_IOR(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_lookup_ctl));
    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetIpeLookupCtl(V, stackingMacLearningMode_f, &ipe_lookup_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetIpeLookupCtl(V, stackingMacLearningMode_f, &ipe_lookup_ctl, 1);
    }
    cmd = DRV_IOW(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_lookup_ctl));

    sal_memset(&ipe_loop_hdr_adj_ctl, 0, sizeof(IpeLoopbackHeaderAdjustCtl_m));
    cmd = DRV_IOR(IpeLoopbackHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_loop_hdr_adj_ctl));
    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetIpeLoopbackHeaderAdjustCtl(V, legacyCFHeaderEn_f, &ipe_loop_hdr_adj_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetIpeLoopbackHeaderAdjustCtl(V, legacyCFHeaderEn_f, &ipe_loop_hdr_adj_ctl, 0);
    }
    cmd = DRV_IOW(IpeLoopbackHeaderAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_loop_hdr_adj_ctl));

    /*Buffer Store Ctl init*/
    sal_memset(&buffer_store_ctl, 0, sizeof(BufferStoreCtl_m));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));
     /*CTC_BIT_SET(chip_discard[gchip_id / 32], gchip_id % 32);*/
     /*SetBufferStoreCtl(A, discardSourceChip_f, &buffer_store_ctl, chip_discard);*/
    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));

    /*MetFifo Ctl init*/
    sal_memset(&met_fifo_ctl, 0, sizeof(MetFifoCtl_m));
    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));
    SetMetFifoCtl(V, stackingMcastEndEn_f, &met_fifo_ctl, value);
    SetMetFifoCtl(V, exceptionResetSrcSgmac_f, &met_fifo_ctl, value);
    SetMetFifoCtl(V, uplinkReflectCheckEn_f, &met_fifo_ctl, value);
    SetMetFifoCtl(V, forceReplicationFromFabric_f, &met_fifo_ctl, value);
    SetMetFifoCtl(V, stackingBroken_f, &met_fifo_ctl, 0);
    SetMetFifoCtl(V, discardMetLoop_f, &met_fifo_ctl, 0);
    SetMetFifoCtl(V, fromRemoteCpuPhyPortCheckEn_f, &met_fifo_ctl, 1);
    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    /*Qwrite Sgmac Ctl init*/
    sal_memset(&qwrite_sgmac_ctl, 0, sizeof(QWriteSgmacCtl_m));
    cmd = DRV_IOR(QWriteSgmacCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qwrite_sgmac_ctl));
    SetQWriteSgmacCtl(V, sgmacEn_f, &qwrite_sgmac_ctl, value);
    SetQWriteSgmacCtl(V, uplinkReflectCheckEn_f, &qwrite_sgmac_ctl, value_mode);
    SetQWriteSgmacCtl(V, discardUnkownSgmacGroup_f, &qwrite_sgmac_ctl, value);
    SetQWriteSgmacCtl(V, cFlexLagHashType_f, &qwrite_sgmac_ctl, 1);
    cmd = DRV_IOW(QWriteSgmacCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qwrite_sgmac_ctl));


    /*Qwrite Port Isolation Ctl init*/
    sal_memset(&qwrite_port_isolation_ctl, 0, sizeof(QWritePortIsolateCtl_m));
    cmd = DRV_IOR(QWritePortIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qwrite_port_isolation_ctl));
    SetQWritePortIsolateCtl(V, dstPhyPortMode_f, &qwrite_port_isolation_ctl, 1);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateC2cPacketEn_f, &qwrite_port_isolation_ctl, 0);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateFwdTypeEn1_f, &qwrite_port_isolation_ctl, 0);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateFwdTypeEn2_f, &qwrite_port_isolation_ctl, 0xF);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateMcastEn_f, &qwrite_port_isolation_ctl, 1);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateUcastEn_f, &qwrite_port_isolation_ctl, 0);
    SetQWritePortIsolateCtl(V, cFlexDstIsolateMode_f, &qwrite_port_isolation_ctl, 1);
    SetQWritePortIsolateCtl(V, overFlowDiscard_f, &qwrite_port_isolation_ctl, 0);
    cmd = DRV_IOW(QWritePortIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &qwrite_port_isolation_ctl));


    /*Epe Head Adjust Ctl init*/
    sal_memset(&epe_hdr_adjust_ctl, 0, sizeof(EpeHdrAdjustCtl_m));
    cmd = DRV_IOR(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_ctl));

    SetEpeHdrAdjustCtl(V, egressCpuRemoveByteType_f, &epe_hdr_adjust_ctl, 0);
    SetEpeHdrAdjustCtl(V, egressCpuNotStripPacket_f, &epe_hdr_adjust_ctl, 1);
    SetEpeHdrAdjustCtl(V, toRemoteCpuSkipPacketEdit_f, &epe_hdr_adjust_ctl, 1);
    SetEpeHdrAdjustCtl(V, toRemoteCpuAttachHeader_f, &epe_hdr_adjust_ctl, 0);
    SetEpeHdrAdjustCtl(V, cfHeaderLen_f, &epe_hdr_adjust_ctl, 0);
    SetEpeHdrAdjustCtl(V, packetHeaderAsCFlexHeader_f, &epe_hdr_adjust_ctl, 0);
    SetEpeHdrAdjustCtl(V, cFlexPtpApplyEgressAsymmetryDelay_f, &epe_hdr_adjust_ctl, 1);
    SetEpeHdrAdjustCtl(V, packetHeaderBypassAll_f, &epe_hdr_adjust_ctl, 1);
    SetEpeHdrAdjustCtl(V, cflexGlbDestPortEn_f, &epe_hdr_adjust_ctl, 1);

    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetEpeHdrAdjustCtl(V, cFlexPtpEnable_f, &epe_hdr_adjust_ctl, 0);
        SetEpeHdrAdjustCtl(V, legacyCFHeaderEn_f, &epe_hdr_adjust_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetEpeHdrAdjustCtl(V, cFlexPtpEnable_f, &epe_hdr_adjust_ctl, 1);
        SetEpeHdrAdjustCtl(V, legacyCFHeaderEn_f, &epe_hdr_adjust_ctl, 0);
    }
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_ctl));

    sal_memset(&epe_hdr_adjust_chan_ctl, 0, sizeof(EpeHdrAdjustChanCtl_m));
    cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl));
    GetEpeHdrAdjustChanCtl(A, packetHeaderEditIngress_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    {
        packet_header_en[0] = 0xFFFFFFFF;
        packet_header_en[1] = 0xFFFFFFFF;
    }
    SetEpeHdrAdjustChanCtl(A, packetHeaderEditIngress_f, &epe_hdr_adjust_chan_ctl, packet_header_en);

    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl));


    /*Epe Nexthop Ctl init*/
    sal_memset(&epe_next_hop_ctl, 0, sizeof(EpeNextHopCtl_m));
    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_next_hop_ctl));
    SetEpeNextHopCtl(V, cFlexDot1QEn1_f, &epe_next_hop_ctl, 1);
    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetEpeNextHopCtl(V, stackingDot1QEn_f, &epe_next_hop_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetEpeNextHopCtl(V, stackingDot1QEn_f, &epe_next_hop_ctl, 0);
    }
    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_next_hop_ctl));

    /*Epe Packet Process Ctl init*/
    sal_memset(&epe_pkt_proc_ctl, 0, sizeof(EpePktProcCtl_m));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_proc_ctl));
    SetEpePktProcCtl(V, cfPathSkipTimestamp_f, &epe_pkt_proc_ctl, 0);
    SetEpePktProcCtl(V, ingEditSkipVlanEdit_f, &epe_pkt_proc_ctl, 0);
    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_proc_ctl));

    /*Epe Header Edit Ctl init*/
    sal_memset(&epe_header_edit_ctl, 0, sizeof(EpeHeaderEditCtl_m));
    cmd = DRV_IOR(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_header_edit_ctl));
    SetEpeHeaderEditCtl(V, stackingEn_f, &epe_header_edit_ctl, value);
    SetEpeHeaderEditCtl(V, stackingCompatibleMode_f, &epe_header_edit_ctl, value);

    SetEpeHeaderEditCtl(V, headerVersion_f, &epe_header_edit_ctl, 2);
    SetEpeHeaderEditCtl(V, useEgressNewTpid_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, ecnCriticalPacketEn_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, ingressEditUseLogicPortSelect_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, extEgrEditValid_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, extCidValid_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, extLearningValid_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, extHeaderDisableBitmap_f, &epe_header_edit_ctl, 0);
    SetEpeHeaderEditCtl(V, cflexDebugEn_f, &epe_header_edit_ctl, 1);
    SetEpeHeaderEditCtl(V, spanPktUseSrcPortOnCFlex_f, &epe_header_edit_ctl, 1)

    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetEpeHeaderEditCtl(V, sourcePortMode_f, &epe_header_edit_ctl, 1);
        SetEpeHeaderEditCtl(V, legacyCFHeaderEn_f, &epe_header_edit_ctl, 1);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetEpeHeaderEditCtl(V, sourcePortMode_f, &epe_header_edit_ctl, 0);
        SetEpeHeaderEditCtl(V, legacyCFHeaderEn_f, &epe_header_edit_ctl, 0);
        SetEpeHeaderEditCtl(V, cfHeaderLearningEn_f, &epe_header_edit_ctl, 1);
    }
    cmd = DRV_IOW(EpeHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_header_edit_ctl));

    /*Epe Packet Header Ctl init*/
    sal_memset(&epe_pkt_hdr_ctl, 0, sizeof(EpePktHdrCtl_m));
    cmd = DRV_IOR(EpePktHdrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_hdr_ctl));
    SetEpePktHdrCtl(V, headerUdpEn_f, &epe_pkt_hdr_ctl, 0);
    SetEpePktHdrCtl(V, cFlexLookupDestMap_f, &epe_pkt_hdr_ctl, SYS_RSV_PORT_SPINE_LEAF_PORT);
    SetEpePktHdrCtl(V, cFlexLookupDestMapMask_f, &epe_pkt_hdr_ctl, 0x401FF);
    SetEpePktHdrCtl(V, cFlexPortEcnEn_f, &epe_pkt_hdr_ctl, 1);
    SetEpePktHdrCtl(V, skipHeaderEditingEn_f, &epe_pkt_hdr_ctl, 0);
    SetEpePktHdrCtl(V, skipHeaderDestMap_f, &epe_pkt_hdr_ctl, SYS_RSV_PORT_SPINE_LEAF_PORT);
    SetEpePktHdrCtl(V, skipHeaderDestMapMask_f, &epe_pkt_hdr_ctl, 0x401FF);
    SetEpePktHdrCtl(V, packetHeaderAsCFlexHeader_f, &epe_pkt_hdr_ctl, 0);
    SetEpePktHdrCtl(V, cFlexDisableMarkCnAcion_f, &epe_pkt_hdr_ctl, 0);
    cmd = DRV_IOW(EpePktHdrCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_hdr_ctl));

    /*Lag Engine Ctl init*/
    sal_memset(&lag_engine_ctl, 0, sizeof(LagEngineCtl_m));
    cmd = DRV_IOR(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));
    SetLagEngineCtl(V, sgmacEn_f, &lag_engine_ctl, value);
    SetLagEngineCtl(V, discardUnkownSgmacGroup_f, &lag_engine_ctl, value);
    SetLagEngineCtl(V, destChipSgMacSelMode_f, &lag_engine_ctl, 0);
    SetLagEngineCtl(V, chanLagMemberMoveCheckEn_f, &lag_engine_ctl, 1);


    if (CTC_STACKING_VERSION_1_0 == version)
    {
        SetLagEngineCtl(V, sgHashMode_f, &lag_engine_ctl, 0);
        SetLagEngineCtl(V, destChipSgMacSelMode_f, &lag_engine_ctl, 0);
    }
    else if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetLagEngineCtl(V, sgHashMode_f, &lag_engine_ctl, 1);
        SetLagEngineCtl(V, destChipSgMacSelMode_f, &lag_engine_ctl, 0);
    }

    /* spine-leaf mode init */
    field_val = mode?1:0;
    SetLagEngineCtl(V, fabricMode_f, &lag_engine_ctl, field_val);

    if (mode && sys_usw_chip_get_rchain_en())
    {
        SetLagEngineCtl(V, chipId_f, &lag_engine_ctl, 0x1f);
    }
    cmd = DRV_IOW(LagEngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lag_engine_ctl));

    /*to cpu*/
    if (CTC_STACKING_VERSION_1_0 == version)
    {
        sal_memset(&ipe_legacy_cfhdr_xlate_ctl, 0, sizeof(IpeLegacyCFHeaderXlateCtl_m));
        SetIpeLegacyCFHeaderXlateCtl(V, g_0_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        SetIpeLegacyCFHeaderXlateCtl(V, g_1_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 1);
        SetIpeLegacyCFHeaderXlateCtl(V, g_2_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        SetIpeLegacyCFHeaderXlateCtl(V, g_3_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        SetIpeLegacyCFHeaderXlateCtl(V, g_4_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        SetIpeLegacyCFHeaderXlateCtl(V, g_5_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        SetIpeLegacyCFHeaderXlateCtl(V, g_6_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 1);
        SetIpeLegacyCFHeaderXlateCtl(V, g_7_toCpu_f, &ipe_legacy_cfhdr_xlate_ctl, 0);
        cmd = DRV_IOW(IpeLegacyCFHeaderXlateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_legacy_cfhdr_xlate_ctl));

        sal_memset(&epe_legacy_cfhdr_xlate_ctl, 0, sizeof(EpeLegacyCFHeaderXlateCtl_m));
        SetEpeLegacyCFHeaderXlateCtl(V, g_0_queSelType_f, &epe_legacy_cfhdr_xlate_ctl, 0);
        SetEpeLegacyCFHeaderXlateCtl(V, g_1_queSelType_f, &epe_legacy_cfhdr_xlate_ctl, 6);
        cmd = DRV_IOW(EpeLegacyCFHeaderXlateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_legacy_cfhdr_xlate_ctl));
    }


    sal_memset(&uplink_lag_hash_ctl, 0, sizeof(IpeFwdCflexUplinkLagHashCtl_m));
    cmd = DRV_IOR(IpeFwdCflexUplinkLagHashCtl_t, DRV_ENTRY_FLAG);
    if (CTC_STACKING_VERSION_2_0 == version)
    {
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_0_destMapMask_f, &uplink_lag_hash_ctl, 0x7FFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_0_sourcePortMask_f, &uplink_lag_hash_ctl, 0xFFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_0_headerHashMask_f, &uplink_lag_hash_ctl, 0xFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_1_destMapMask_f, &uplink_lag_hash_ctl, 0x7FFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_1_sourcePortMask_f, &uplink_lag_hash_ctl, 0xFFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_1_headerHashMask_f, &uplink_lag_hash_ctl, 0xFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_destMapMask_f, &uplink_lag_hash_ctl, 0x7FFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_sourcePortMask_f, &uplink_lag_hash_ctl, 0xFFFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, gFwdType_2_headerHashMask_f, &uplink_lag_hash_ctl, 0xFF);
        SetIpeFwdCflexUplinkLagHashCtl(V, srcPortModeFabric_f, &uplink_lag_hash_ctl, 0xFF);

        /* cflexHashTypeFabric:
        0: HEAD HASH
        1: FLOW_HASH
        2: CRC8
        3: XOR8
		*/
        SetIpeFwdCflexUplinkLagHashCtl(V, cflexHashTypeFabric_f, &uplink_lag_hash_ctl, 2);
        SetIpeFwdCflexUplinkLagHashCtl(V, backToHeadHash_f, &uplink_lag_hash_ctl, 0);
        SetIpeFwdCflexUplinkLagHashCtl(V, srcPortModeFabric_f, &uplink_lag_hash_ctl, 1);

    }
    else if(CTC_STACKING_VERSION_1_0 == version)
    {
        SetIpeFwdCflexUplinkLagHashCtl(V, cflexHashTypeFabric_f, &uplink_lag_hash_ctl, 0);
        SetIpeFwdCflexUplinkLagHashCtl(V, backToHeadHash_f, &uplink_lag_hash_ctl, 1);
    }

    cmd = DRV_IOW(IpeFwdCflexUplinkLagHashCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &uplink_lag_hash_ctl));


    field_val = (CTC_STACKING_VERSION_2_0 == version)?1:0;
    cmd = DRV_IOW(EpeHdrAdjustCtl_t, EpeHdrAdjustCtl_cflexGlbDestPortEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*Disable lchip global port map*/
    SetDsGlbDestPortMap(V, glbDestPortBase_f, &port_map, 0);
    SetDsGlbDestPortMap(V, glbDestPortMin_f, &port_map, 1);
    SetDsGlbDestPortMap(V, glbDestPortMax_f, &port_map, 0);

    cmd = DRV_IOW(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);

    for (gchip_id = 0; gchip_id < SYS_STK_MAX_GCHIP; gchip_id++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, gchip_id, cmd, &port_map));
    }

    return CTC_E_NONE;
}



uint32
_sys_usw_stacking_mcast_hash_make(sys_stacking_mcast_db_t* backet)
{
    uint32 val = 0;
    uint8* data = NULL;
    uint8   length = 0;


    if (!backet)
    {
        return 0;
    }

    val = (backet->type << 24) | (backet->id);

    data = (uint8*)&val;
    length = sizeof(uint32);

    return ctc_hash_caculate(length, data);
}

/**
 @brief  hash comparison hook.
*/
bool
_sys_usw_stacking_mcast_hash_cmp(sys_stacking_mcast_db_t* backet1,
                                sys_stacking_mcast_db_t* backet2)
{

    if (!backet1 || !backet2)
    {
        return FALSE;
    }

    if ((backet1->type == backet2->type) &&
        (backet1->id == backet2->id))
    {
        return TRUE;
    }

    return FALSE;
}

int32
sys_usw_stacking_mcast_group_create(uint8 lchip,
                                           uint8 type,
                                           uint16 id,
                                           sys_stacking_mcast_db_t **pp_mcast_db,
                                           uint8 append_en)
{
    int32  ret                     = CTC_E_NONE;
    uint32 new_met_offset          = 0;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;
    sys_nh_param_dsmet_t dsmet_para;


    /*build node*/
    p_mcast_db = (sys_stacking_mcast_db_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_mcast_db_t));
    if (NULL == p_mcast_db)
    {
        ret =  CTC_E_NO_MEMORY;
        goto Error0;
    }

    sal_memset(p_mcast_db, 0, sizeof(sys_stacking_mcast_db_t));
    p_mcast_db->type = type;
    p_mcast_db->id   = id;

    if (append_en)
    {
        p_mcast_db->last_tail_offset = g_stacking_master[lchip]->stacking_mcast_offset;
    }
    else
    {
        p_mcast_db->last_tail_offset = SYS_NH_INVALID_OFFSET;
    }

    sal_memset(&dsmet_para, 0, sizeof(sys_nh_param_dsmet_t));
    dsmet_para.next_met_entry_ptr = SYS_NH_NEXT_MET_ENTRY(lchip, p_mcast_db->last_tail_offset);

    if (type == 0) /* mcast profile */
    {
        /*alloc resource*/
        CTC_ERROR_GOTO(sys_usw_nh_alloc(lchip, SYS_NH_ENTRY_TYPE_MET, 1, &new_met_offset),
                       ret, Error1);

        dsmet_para.met_offset = new_met_offset;
        dsmet_para.dest_id = 0 ;
        dsmet_para.remote_chip = 1;
        CTC_ERROR_GOTO(sys_usw_nh_add_dsmet(lchip, &dsmet_para), ret, Error2);
        /*add db*/
        p_mcast_db->head_met_offset = new_met_offset;
        p_mcast_db->tail_met_offset = p_mcast_db->last_tail_offset;
        p_mcast_db->append_en = append_en;
    }
    else/* keeplive */
    {
        new_met_offset = id;

        dsmet_para.met_offset = new_met_offset;
        dsmet_para.dest_id = SYS_RSV_PORT_DROP_ID ;
        dsmet_para.end_local_rep = 1;
        CTC_ERROR_GOTO(sys_usw_nh_add_dsmet(lchip, &dsmet_para), ret, Error1);

        /*add db*/
        p_mcast_db->head_met_offset = new_met_offset;
        p_mcast_db->tail_met_offset = new_met_offset;
    }

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "head_met_offset :%d\n ", new_met_offset);
    if(NULL == ctc_hash_insert(g_stacking_master[lchip]->mcast_hash, p_mcast_db))
    {
        ret = CTC_E_NO_MEMORY;
        goto Error2;
    }

    *pp_mcast_db = p_mcast_db;

    return CTC_E_NONE;


    Error2:

    if (type == 0)
    {
        sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, new_met_offset);
    }

    Error1:

    if (p_mcast_db)
    {
        mem_free(p_mcast_db);
    }

    Error0:

    return ret;

}



int32
sys_usw_stacking_mcast_group_destroy(uint8 lchip,
                                            uint8 type,
                                            uint16 id,
                                            sys_stacking_mcast_db_t *p_mcast_db)
{
    sys_nh_param_dsmet_t dsmet;

    sal_memset(&dsmet, 0, sizeof(sys_nh_param_dsmet_t));
    dsmet.next_met_entry_ptr = SYS_NH_INVALID_OFFSET;
    dsmet.dest_id = 0 ;
    dsmet.remote_chip = 1;
    dsmet.met_offset = p_mcast_db->head_met_offset;
    CTC_ERROR_RETURN(sys_usw_nh_add_dsmet(lchip, &dsmet));

    if (type == 0)/* mcast profile */
    {
        /*free resource*/
        CTC_ERROR_RETURN(sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, p_mcast_db->head_met_offset));
    }

    ctc_hash_remove(g_stacking_master[lchip]->mcast_hash, p_mcast_db);

    mem_free(p_mcast_db);

    return CTC_E_NONE;
}

int32
sys_usw_stacking_mcast_group_add_member(uint8 lchip,
                                               sys_stacking_mcast_db_t *p_mcast_db,
                                               uint16 dest_id, uint8 is_remote)
{
    int32  ret                     = CTC_E_NONE;
    uint32 new_met_offset          = 0;
    uint32 cmd                     = 0;
    DsMetEntry3W_m dsmet3w;
    sys_nh_param_dsmet_t dsmet;
    uint32 dsnh_offset             = 0;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dest_id = %d, head:%d, is_remote:%u \n", dest_id, p_mcast_db->head_met_offset, is_remote);

    if (!is_remote)
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BRIDGE_NH, &dsnh_offset));
    }

    if (p_mcast_db->tail_met_offset == p_mcast_db->last_tail_offset)
    {

        sal_memset(&dsmet, 0, sizeof(sys_nh_param_dsmet_t));
        dsmet.next_met_entry_ptr = SYS_NH_NEXT_MET_ENTRY(lchip, p_mcast_db->last_tail_offset);
        dsmet.dest_id = dest_id ;
        dsmet.remote_chip = is_remote;
        dsmet.met_offset = p_mcast_db->head_met_offset;
        dsmet.next_hop_ptr = dsnh_offset;
        CTC_ERROR_RETURN(sys_usw_nh_add_dsmet(lchip, &dsmet));

        p_mcast_db->tail_met_offset = p_mcast_db->head_met_offset;

        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "update head_met_offset :%d, trunk-id:%d \n ", new_met_offset, dest_id);

    }
    else
    {

        /*new mcast offset*/
        CTC_ERROR_RETURN(sys_usw_nh_alloc(lchip, SYS_NH_ENTRY_TYPE_MET, 1, &new_met_offset));
        sal_memset(&dsmet, 0, sizeof(dsmet));
        dsmet.next_met_entry_ptr = SYS_NH_NEXT_MET_ENTRY(lchip, p_mcast_db->last_tail_offset);
        dsmet.dest_id = dest_id ;
        dsmet.remote_chip = is_remote;
        dsmet.met_offset = new_met_offset;
        dsmet.next_hop_ptr = dsnh_offset;
        CTC_ERROR_GOTO(sys_usw_nh_add_dsmet(lchip, &dsmet),ret, Error0);


        cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->tail_met_offset, cmd, &dsmet3w),
                       ret, Error0);

        SetDsMetEntry3W(V, nextMetEntryPtr_f,  &dsmet3w, SYS_NH_NEXT_MET_ENTRY(lchip, new_met_offset));

        cmd = DRV_IOW(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->tail_met_offset, cmd, &dsmet3w),
                       ret, Error0);

        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add new_met_offset :%d, pre_offset:%d, dest_id:%d\n ", new_met_offset, p_mcast_db->tail_met_offset, dest_id);

        p_mcast_db->tail_met_offset = new_met_offset;

    }

    if (is_remote)
    {
        CTC_BIT_SET(p_mcast_db->trunk_bitmap[dest_id / 32], dest_id % 32);
    }
    else
    {
        CTC_BIT_SET(p_mcast_db->port_bitmap[dest_id / 32], dest_id % 32);
    }
    return CTC_E_NONE;

    Error0:
    sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, new_met_offset);

    return ret;


}

int32
sys_usw_stacking_mcast_group_remove_member(uint8 lchip,
                                                  sys_stacking_mcast_db_t *p_mcast_db,
                                                  uint16 dest_id, uint8 is_remote)
{
    uint16 trunk_id_hw             = 0;
    uint16 remote_chip             = 0;
    uint32 cmd                     = 0;
    uint32 head_met_offset         = 0;
    uint32 pre_met_offset          = 0;
    uint32 cur_met_offset          = 0;
    uint32 next_met_offset         = 0;
    DsMetEntry3W_m dsmet3w;
    sys_nh_param_dsmet_t dsmet;
    uint8   is_found               = 0;


    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dest_id = %d, head:%d, is_remote:%u \n", dest_id, p_mcast_db->head_met_offset, is_remote);

    head_met_offset = p_mcast_db->head_met_offset;
    cur_met_offset  = head_met_offset;
    next_met_offset = head_met_offset;


    do
    {
        pre_met_offset = cur_met_offset;
        cur_met_offset = next_met_offset;

        cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, cur_met_offset, cmd, &dsmet3w));

        trunk_id_hw = GetDsMetEntry3W(V, ucastId_f,  &dsmet3w);
        remote_chip =  GetDsMetEntry3W(V, remoteChip_f,  &dsmet3w);
        next_met_offset = GetDsMetEntry3W(V, nextMetEntryPtr_f,  &dsmet3w);

        if ((remote_chip == is_remote) && (trunk_id_hw == dest_id))
        {
            is_found = 1;
            break;
        }

    }
    while (next_met_offset != p_mcast_db->last_tail_offset);

    if (!is_found)
    {
        return  CTC_E_ENTRY_NOT_EXIST;
    }

    if (cur_met_offset == head_met_offset)
    {
        /*remove is first met*/

        if (next_met_offset == p_mcast_db->last_tail_offset)
        {
            /*remove last node, */

            sal_memset(&dsmet, 0, sizeof(sys_nh_param_dsmet_t));
            dsmet.next_met_entry_ptr = SYS_NH_NEXT_MET_ENTRY(lchip, p_mcast_db->last_tail_offset);
            dsmet.dest_id = 0 ;
            dsmet.remote_chip = 1;
            dsmet.met_offset = head_met_offset;
            CTC_ERROR_RETURN(sys_usw_nh_add_dsmet(lchip, &dsmet));

            p_mcast_db->tail_met_offset = p_mcast_db->last_tail_offset;

            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update first to drop, head_met_offset: %d, dest_id:%d, is_remote:%u\n ",
                            head_met_offset, dest_id, is_remote);

            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tail:%d\n ",  p_mcast_db->tail_met_offset);
        }
        else
        {

            /*nexthop change to first*/
            cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN( DRV_IOCTL(lchip, next_met_offset, cmd, &dsmet3w));

            /*nexthop change to first*/
            cmd = DRV_IOW(DsMetEntry3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, cur_met_offset, cmd, &dsmet3w));

            /*free currect offset*/
            CTC_ERROR_RETURN(sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, next_met_offset));


            if (p_mcast_db->tail_met_offset == next_met_offset)
            {
                p_mcast_db->tail_met_offset = cur_met_offset;
            }


            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Replace first head_met_offset: %d, dest_id:%d, is_remote:%u\n ",
                            head_met_offset, dest_id, is_remote);

            SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free Met offset:%d, tail:%d\n ",  next_met_offset, p_mcast_db->tail_met_offset);

        }
    }
    else
    {
        cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, pre_met_offset, cmd, &dsmet3w));

        SetDsMetEntry3W(V, nextMetEntryPtr_f,  &dsmet3w, SYS_NH_NEXT_MET_ENTRY(lchip, next_met_offset));

        cmd = DRV_IOW(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, pre_met_offset, cmd, &dsmet3w));

        /*free currect offset*/
        CTC_ERROR_RETURN(sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, cur_met_offset));

        if (p_mcast_db->tail_met_offset == cur_met_offset)
        {
            p_mcast_db->tail_met_offset = pre_met_offset;
        }

        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove middle pre: %d, cur:%d, next:%d, dest_id:%d\n ",
                        pre_met_offset, cur_met_offset, next_met_offset,  dest_id);

        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free Met offset:%d, tail:%d\n ",  cur_met_offset, p_mcast_db->tail_met_offset);


    }

    if (is_remote)
    {
        CTC_BIT_UNSET(p_mcast_db->trunk_bitmap[dest_id / 32], dest_id % 32);
    }

    return CTC_E_NONE;
}



int32
_sys_usw_stacking_encap_header_enable(uint8 lchip, uint16 chan_id, ctc_stacking_hdr_encap_t* p_encap, uint8 dir)
{
    uint32 cmd                     = 0;
    uint8 mux_type                 = 0;
    uint32 field_val               = 0;
    uint8 out_encap_type           = 0;
    hw_mac_addr_t mac;
    DsPacketHeaderEditTunnel_m  ds_packet_header_edit_tunnel;

    switch (p_encap->hdr_flag)
    {
    case CTC_STK_HDR_WITH_NONE:
        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL);
        out_encap_type = SYS_STK_OUT_ENCAP_NONE;
        break;

    case CTC_STK_HDR_WITH_L2:
        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2);
        out_encap_type = SYS_STK_OUT_ENCAP_NONE;
        break;

    case CTC_STK_HDR_WITH_L2_AND_IPV4:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }
        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4);
        out_encap_type = SYS_STK_OUT_ENCAP_IPV4;
        break;

    case CTC_STK_HDR_WITH_L2_AND_IPV6:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }

        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6);
        out_encap_type = SYS_STK_OUT_ENCAP_IPV6;
        break;

    case CTC_STK_HDR_WITH_IPV4:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }

        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV4);
        out_encap_type = SYS_STK_OUT_ENCAP_IPV4;
        break;

    case CTC_STK_HDR_WITH_IPV6:
        if (!DRV_IS_DUET2(lchip))
        {
            return CTC_E_NOT_SUPPORT;
        }

        mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV6);
        out_encap_type = SYS_STK_OUT_ENCAP_IPV6;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

 if ((dir == CTC_INGRESS) || (dir == CTC_BOTH_DIRECTION))
 {
     /*mux type*/
     cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
     field_val = mux_type;
     CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
 }

if ((dir == CTC_EGRESS) || (dir == CTC_BOTH_DIRECTION))
{
    EpeHdrAdjustChanCtl_m    epe_hdr_adjust_chan_ctl;
    BufRetrvChanStackingEn_m stacking_en;
    uint32 packet_header_en[2] = {0};

    sal_memset(&ds_packet_header_edit_tunnel, 0, sizeof(ds_packet_header_edit_tunnel));
    cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl) );
    GetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    CTC_BIT_SET(packet_header_en[chan_id>>5], chan_id&0x1F);
    SetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl) );

    sal_memset(&stacking_en, 0, sizeof(stacking_en));
    sal_memset(packet_header_en, 0, sizeof(packet_header_en));
    cmd = DRV_IOR(BufRetrvChanStackingEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stacking_en) );
    GetBufRetrvChanStackingEn(A, chanStackingEn_f, &stacking_en, packet_header_en);
    CTC_BIT_SET(packet_header_en[chan_id>>5], chan_id&0x1F);
    SetBufRetrvChanStackingEn(A, chanStackingEn_f, &stacking_en, packet_header_en);
    cmd = DRV_IOW(BufRetrvChanStackingEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stacking_en) );

   if (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2) == mux_type ||
       DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4) == mux_type ||
   DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6) == mux_type)
   {
       SetDsPacketHeaderEditTunnel(V, packetHeaderL2En_f , &ds_packet_header_edit_tunnel, 1);
       SetDsPacketHeaderEditTunnel(V, vlanIdValid_f, &ds_packet_header_edit_tunnel, p_encap->vlan_valid);


       if (p_encap->vlan_valid)
       {
           SetDsPacketHeaderEditTunnel(V, vlanId_f , &ds_packet_header_edit_tunnel, p_encap->vlan_id);
       }

       SYS_USW_SET_HW_MAC(mac, p_encap->mac_da)
       SetDsPacketHeaderEditTunnel(A, macDa_f, &ds_packet_header_edit_tunnel, mac);
       SYS_USW_SET_HW_MAC(mac, p_encap->mac_sa)
       SetDsPacketHeaderEditTunnel(A, macSa_f , &ds_packet_header_edit_tunnel, mac);
   }
   SetDsPacketHeaderEditTunnel(V, packetHeaderL3Type_f , &ds_packet_header_edit_tunnel, out_encap_type);


   if (SYS_STK_OUT_ENCAP_IPV4 == out_encap_type)
   {
       SetDsPacketHeaderEditTunnel(A, ipDa_f  , &ds_packet_header_edit_tunnel, &p_encap->ipda.ipv4);
   }

    if (SYS_STK_OUT_ENCAP_IPV6 == out_encap_type)
    {
        uint32 ipv6_da[CTC_IPV6_ADDR_LEN];
        /* ipda, use little india for DRV_SET_FIELD_A */
        ipv6_da[0] = p_encap->ipda.ipv6[3];
        ipv6_da[1] = p_encap->ipda.ipv6[2];
        ipv6_da[2] = p_encap->ipda.ipv6[1];
        ipv6_da[3] = p_encap->ipda.ipv6[0];
        SetDsPacketHeaderEditTunnel(A, ipDa_f  , &ds_packet_header_edit_tunnel, &ipv6_da);
    }

    cmd = DRV_IOW(DsPacketHeaderEditTunnel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ds_packet_header_edit_tunnel));
}
    return CTC_E_NONE;
}

int32
_sys_usw_stacking_encap_header_disable(uint8 lchip, uint16 chan_id, uint8 dir)
{
    uint32 cmd                     = 0;
    uint8 mux_type                 = 0;
    uint32 field_val               = 0;

    DsPacketHeaderEditTunnel_m  ds_packet_header_edit_tunnel;

    mux_type = DRV_ENUM(DRV_STK_MUX_TYPE_HDR_REGULAR_PORT);

if ((dir == CTC_INGRESS) || (dir == CTC_BOTH_DIRECTION))
{
    /*mux type*/
    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    field_val = mux_type;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
}

if ((dir == CTC_EGRESS) || (dir == CTC_BOTH_DIRECTION))
{
    EpeHdrAdjustChanCtl_m    epe_hdr_adjust_chan_ctl;
    BufRetrvChanStackingEn_m stacking_en;
    uint32 packet_header_en[2] = {0};

    sal_memset(&ds_packet_header_edit_tunnel, 0, sizeof(ds_packet_header_edit_tunnel));
    cmd = DRV_IOW(DsPacketHeaderEditTunnel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ds_packet_header_edit_tunnel));

    sal_memset(packet_header_en, 0, sizeof(packet_header_en));
    cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl) );
    GetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    CTC_BIT_UNSET(packet_header_en[chan_id>>5], chan_id&0x1F);
    SetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    cmd = DRV_IOW(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl) );

    sal_memset(&stacking_en, 0, sizeof(stacking_en));
    sal_memset(packet_header_en, 0, sizeof(packet_header_en));
    cmd = DRV_IOR(BufRetrvChanStackingEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stacking_en) );
    GetBufRetrvChanStackingEn(A, chanStackingEn_f, &stacking_en, packet_header_en);
    CTC_BIT_UNSET(packet_header_en[chan_id>>5], chan_id&0x1F);
    SetBufRetrvChanStackingEn(A, chanStackingEn_f, &stacking_en, packet_header_en);
    cmd = DRV_IOW(BufRetrvChanStackingEn_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stacking_en) );
}
    return CTC_E_NONE;
}

STATIC uint8
_sys_usw_stacking_is_trunk_have_member(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 packet_header_en[2] = {0};
    EpeHdrAdjustChanCtl_m epe_hdr_adjust_chan_ctl;

	cmd = DRV_IOR(EpeHdrAdjustChanCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_hdr_adjust_chan_ctl));

    GetEpeHdrAdjustChanCtl(A, packetHeaderEn_f, &epe_hdr_adjust_chan_ctl, packet_header_en);
    if (packet_header_en[0] || packet_header_en[1])
    {
        return 1;
    }

    return 0;
}

STATIC int32
_sys_usw_stacking_all_mcast_add_default_profile(void* array_data, uint32 group_id, void* user_data)
{
    uint8 lchip                    = 0;
    uint32 cmd                     = 0;
    uint32 cur_met_offset          = 0;
    uint32 next_met_offset         = 0;
    sys_nh_info_mcast_t* p_mcast_db;

    lchip = *((uint8*) user_data);
    cur_met_offset  = group_id;
    next_met_offset = cur_met_offset;
    p_mcast_db = (sys_nh_info_mcast_t*)array_data;

    do
    {
        cur_met_offset = next_met_offset;

        cmd = DRV_IOR(DsMetEntry3W_t, DsMetEntry3W_nextMetEntryPtr_f);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, cur_met_offset, cmd, &next_met_offset));

        if (next_met_offset == g_stacking_master[lchip]->stacking_mcast_offset)
        {
           goto done;
        }

    }
    while (next_met_offset != SYS_NH_INVALID_OFFSET);

    next_met_offset = SYS_NH_NEXT_MET_ENTRY(lchip, g_stacking_master[lchip]->stacking_mcast_offset);
    cmd = DRV_IOW(DsMetEntry3W_t, DsMetEntry3W_nextMetEntryPtr_f);
    CTC_ERROR_RETURN( DRV_IOCTL(lchip, cur_met_offset, cmd, &next_met_offset));
done:
    if (!CTC_FLAG_ISSET(p_mcast_db->mcast_flag, SYS_NH_INFO_MCAST_FLAG_MEM_PROFILE)
        && !CTC_FLAG_ISSET(p_mcast_db->mcast_flag, SYS_NH_INFO_MCAST_FLAG_STK_MEM_PROFILE))
    {
        p_mcast_db->profile_met_offset = g_stacking_master[lchip]->stacking_mcast_offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_all_mcast_remove_default_profile(void* array_data, uint32 group_id, void* user_data)
{
    uint8 lchip                    = 0;
    uint32 cmd                     = 0;
    uint32 cur_met_offset          = 0;
    uint32 next_met_offset         = 0;
    sys_nh_info_mcast_t* p_mcast_db;

    lchip = *((uint8*) user_data);
    cur_met_offset  = group_id;
    next_met_offset = cur_met_offset;
    p_mcast_db = (sys_nh_info_mcast_t*)array_data;

    do
    {
        cur_met_offset = next_met_offset;

        cmd = DRV_IOR(DsMetEntry3W_t, DsMetEntry3W_nextMetEntryPtr_f);
        CTC_ERROR_RETURN( DRV_IOCTL(lchip, cur_met_offset, cmd, &next_met_offset));


        if (next_met_offset == g_stacking_master[lchip]->stacking_mcast_offset)
        {
            break;
        }

    }
    while (next_met_offset != SYS_NH_INVALID_OFFSET);


    if (next_met_offset !=  g_stacking_master[lchip]->stacking_mcast_offset)
    {
        return CTC_E_NONE;
    }

    next_met_offset = SYS_NH_INVALID_OFFSET;
    cmd = DRV_IOW(DsMetEntry3W_t, DsMetEntry3W_nextMetEntryPtr_f);
    CTC_ERROR_RETURN( DRV_IOCTL(lchip, cur_met_offset, cmd, &next_met_offset));
    if (!CTC_FLAG_ISSET(p_mcast_db->mcast_flag, SYS_NH_INFO_MCAST_FLAG_MEM_PROFILE)
        && !CTC_FLAG_ISSET(p_mcast_db->mcast_flag, SYS_NH_INFO_MCAST_FLAG_STK_MEM_PROFILE))
    {
        p_mcast_db->profile_met_offset = 0;
    }
    return CTC_E_NONE;
}


int32
sys_usw_stacking_all_mcast_bind_en(uint8 lchip, uint8 enable)
{
    vector_traversal_fn2 fn = NULL;

    SYS_STK_INIT_CHECK(lchip);

    if (enable)
    {
        fn = _sys_usw_stacking_all_mcast_add_default_profile;
    }
    else
    {
        fn = _sys_usw_stacking_all_mcast_remove_default_profile;
    }

    sys_usw_nh_traverse_mcast_db( lchip,  fn,  &lchip);

    g_stacking_master[lchip]->bind_mcast_en = enable?1:0;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_set_stacking_port_en(uint8 lchip, sys_stacking_trunk_info_t* p_trunk, uint32 gport, uint8 enable)
{
    uint32 field_val;
    uint32 cmd;
    uint16 lport;
    uint16 chan_id;
    uint8 enq_mode = 0;
    int32 ret = 0;
    uint8 direction = CTC_BOTH_DIRECTION;
    uint8 ingr_en = 0;
    uint8 egr_en = 0;

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    sys_usw_queue_get_enq_mode(lchip, &enq_mode);
    if (0 == enq_mode)
    {
        if (enable)
        {
            ret = sys_usw_queue_add_for_stacking(lchip, gport);
        }
        else
        {
            ret = sys_usw_queue_remove_for_stacking(lchip, gport);
        }
        if(ret != CTC_E_NONE && ret != CTC_E_EXIST)
        {
          return ret;
        }
    }

    field_val = enable? (g_stacking_master[lchip]->fabric_mode?1:2) : 0;
    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_cFlexLookupMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

    /*set not edit packet on stacking port*/
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    field_val = enable ? 1 :0;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_ctagOperationDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_stagOperationDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
    field_val = enable ? 0 :1;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultVlanId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    ingr_en = CTC_BMP_ISSET(g_stacking_master[lchip]->ing_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(gport));
    egr_en = CTC_BMP_ISSET(g_stacking_master[lchip]->egr_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(gport));
    if (enable)
    {
        /*Encap Output packet Heaer enable*/
        if (ingr_en && egr_en)
        {
            goto end;
        }

        direction = (ingr_en)?CTC_EGRESS:(egr_en?CTC_INGRESS:CTC_BOTH_DIRECTION);
        CTC_ERROR_RETURN(_sys_usw_stacking_encap_header_enable(lchip, chan_id, &p_trunk->encap_hdr, direction));
    }
    else
    {
        if (ingr_en && egr_en)
        {
            goto end;
        }
        direction = (ingr_en)?CTC_EGRESS:(egr_en?CTC_INGRESS:CTC_BOTH_DIRECTION);
        CTC_ERROR_RETURN(_sys_usw_stacking_encap_header_disable( lchip, chan_id, direction));
    }
end:
    if (0 == g_stacking_master[lchip]->binding_trunk)
    {
        field_val = enable ? p_trunk->trunk_id :0;
        cmd = DRV_IOW(DsSrcSgmacGroup_t, DsSrcSgmacGroup_sgmacGroupId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_change_stacking_port(uint8 lchip, sys_stacking_trunk_info_t* p_sys_trunk,
                                       uint32 gport,
                                       uint8 enable )
{
    uint16 chan_id              = SYS_GET_CHANNEL_ID(lchip, gport);
    uint16 port_ref_cnt         = 0;
    sys_qos_shape_profile_t shp_profile;
    int32 ret = 0;

    sal_memset(&shp_profile,0,sizeof(shp_profile));
    /*for restore shape profile*/
    _sys_usw_stacking_get_member_ref_cnt(lchip, chan_id, &port_ref_cnt, p_sys_trunk->trunk_id);
    if (port_ref_cnt > 0)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN( sys_usw_queue_get_profile_from_hw(lchip, gport, &shp_profile));
    CTC_ERROR_RETURN(sys_usw_queue_set_port_drop_en(lchip, gport, 1, &shp_profile));
    CTC_ERROR_GOTO(_sys_usw_stacking_check_port_status(lchip, gport, &shp_profile), ret, error_proc1);

    CTC_ERROR_GOTO(_sys_usw_stacking_set_stacking_port_en(lchip, p_sys_trunk, gport, enable), ret, error_proc1);

    CTC_ERROR_GOTO( sys_usw_queue_set_port_drop_en(lchip, gport, 0, &shp_profile), ret, error_proc2);

    return CTC_E_NONE;
error_proc2:
    _sys_usw_stacking_set_stacking_port_en(lchip, p_sys_trunk, gport, !enable);
error_proc1:
    sys_usw_queue_set_port_drop_en(lchip, gport, 0, &shp_profile);
    return ret;

}
int32
_sys_usw_stacking_create_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    int32  ret                     = CTC_E_NONE;
    uint32 cmd                     = 0;
    uint8  load_mode                    = 0;
    uint8  trunk_id                 = p_trunk->trunk_id;
    uint16 mem_base                = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    DsLinkAggregateChannelGroup_m  ds_link_aggregate_sgmac_group;

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL != p_sys_trunk)
    {
        return CTC_E_EXIST;
    }

    p_sys_trunk = (sys_stacking_trunk_info_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_trunk_info_t));
    if (NULL == p_sys_trunk)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_sys_trunk, 0, sizeof(sys_stacking_trunk_info_t));
    p_sys_trunk->trunk_id = trunk_id;
    p_sys_trunk->flag = p_trunk->flag;
    p_sys_trunk->mode = p_trunk->load_mode;
    /*Alloc member base*/
    CTC_ERROR_GOTO(_sys_usw_stacking_trunk_alloc_mem_base(lchip, p_sys_trunk, p_trunk->max_mem_cnt, &mem_base),
                   ret, Error1);


    if (0 == g_stacking_master[lchip]->mcast_mode)
    {
        CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_add_member(lchip,
                                                                      g_stacking_master[lchip]->p_default_prof_db,
                                                                      p_sys_trunk->trunk_id, 1), ret, Error1);
    }


    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(ds_link_aggregate_sgmac_group));
    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group, mem_base);
    load_mode = (CTC_STK_LOAD_DYNAMIC == p_trunk->load_mode) ? 1 : 0;
    SetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group, load_mode);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggFlowNum_f, &ds_link_aggregate_sgmac_group, 3);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group, 0);

    if (CTC_FLAG_ISSET(p_trunk->flag, CTC_STACKING_TRUNK_LB_HASH_OFFSET_VALID))
    {
        SetDsLinkAggregateChannelGroup(V, hashCfgPriorityIsHigher_f, &ds_link_aggregate_sgmac_group, 1);
        SetDsLinkAggregateChannelGroup(V, hashSelect_f, &ds_link_aggregate_sgmac_group, p_trunk->lb_hash_offset / 16);
        SetDsLinkAggregateChannelGroup(V, hashOffset_f, &ds_link_aggregate_sgmac_group, p_trunk->lb_hash_offset % 16);
    }

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group), ret, Error2);

    sal_memcpy(&p_sys_trunk->encap_hdr, &p_trunk->encap_hdr, sizeof(ctc_stacking_hdr_encap_t));

    ctc_vector_add(g_stacking_master[lchip]->p_trunk_vec, trunk_id, p_sys_trunk);

    if (g_stacking_master[lchip]->fabric_mode)
    {
        uint16 tmp_idx = 0;
        uint32 field_val = trunk_id;
        for (tmp_idx = 0; tmp_idx <= 255; tmp_idx++)
        {
            cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
            DRV_IOCTL(lchip, tmp_idx, cmd, &field_val);
        }
    }

    return CTC_E_NONE;

    Error2:
    if (0 == g_stacking_master[lchip]->mcast_mode)
    {
        sys_usw_stacking_mcast_group_remove_member(lchip, g_stacking_master[lchip]->p_default_prof_db,
                                                          p_sys_trunk->trunk_id, 1);
    }

    Error1:
    mem_free(p_sys_trunk);
    return ret;

}

int32
_sys_usw_stacking_destroy_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint32 cmd                     = 0;
    uint8 trunk_id                 = p_trunk->trunk_id;
    uint32 mem_idx                 = 0;
    uint16 gport                   = 0;
    uint16 chan_id                 = 0;
    uint16 mem_base                = 0;
    uint16 mem_cnt                 = 0;
    uint8  gchip_id                = 0;
    uint8  is_dlb                  = 0;

    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    DsLinkAggregateChannelGroup_m ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
        return CTC_E_NOT_EXIST;

    }
    sys_usw_get_gchip_id(lchip, &gchip_id);
    /*cancel ucast path*/

    CTC_ERROR_RETURN(_sys_usw_stacking_get_trunk_info(lchip, trunk_id, &mem_cnt, &mem_base, &is_dlb));

    for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
    {
        cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_idx + mem_base, cmd, &ds_link_aggregate_sgmac_member));
        chan_id = GetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member);
        gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_GET_LPORT_ID_WITH_CHAN(lchip, chan_id));

        _sys_usw_stacking_change_stacking_port(lchip,p_sys_trunk,gport,FALSE);
    }

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(DsLinkAggregateChannelMember_m));

    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group, mem_base);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group, 0);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group, 0);
    SetDsLinkAggregateChannelGroup(V, channelLinkAggFlowNum_f, &ds_link_aggregate_sgmac_group, 0);
    DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group);

    if (0 == g_stacking_master[lchip]->mcast_mode)
    {
        sys_usw_stacking_mcast_group_remove_member(lchip,
                                                          g_stacking_master[lchip]->p_default_prof_db,
                                                          p_sys_trunk->trunk_id, 1);
    }

    if ((1 == g_stacking_master[lchip]->bind_mcast_en)
    && (0 == _sys_usw_stacking_is_trunk_have_member(lchip)))
    {
        sys_usw_stacking_all_mcast_bind_en(lchip, 0);
    }

    /*Free trunk mem base*/
    _sys_usw_stacking_trunk_free_mem_base(lchip, p_sys_trunk,
                                            p_sys_trunk->max_mem_cnt, mem_base);

    ctc_vector_del(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    mem_free(p_sys_trunk);

    return CTC_E_NONE;
}

STATIC bool
_sys_usw_stacking_find_trunk_port(uint8 lchip, uint8 trunk_id, uint16 chan_id, uint32* index)
{
    uint32 mem_index               = 0;
    uint16 mem_base                = 0;
    uint16 mem_cnt                 = 0;
    uint32 cmd                     = 0;
    uint32 tmp_chan_id             = 0;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;
    uint8  is_dlb                  = 0;

    CTC_PTR_VALID_CHECK(index);

    CTC_ERROR_RETURN(_sys_usw_stacking_get_trunk_info(lchip, trunk_id, &mem_cnt, &mem_base, &is_dlb));

    sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(DsLinkAggregateChannelMember_m));

    if (!is_dlb)
    {
        cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        for (mem_index = 0; mem_index < mem_cnt; mem_index++)
        {
             CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_index + mem_base, cmd, &ds_link_aggregate_sgmac_member));
            tmp_chan_id = GetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member);
            if (tmp_chan_id == chan_id)
            {
                *index = mem_index;
                return TRUE;
            }
        }
    }
    else
    {
        cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        for (mem_index = 0; mem_index < mem_cnt; mem_index++)
        {
            cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+mem_index);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &tmp_chan_id));
            if (tmp_chan_id == chan_id)
            {
                *index = mem_index;
                return TRUE;
            }
        }
    }

    return FALSE;
}


int32
_sys_usw_stacking_add_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint16 mem_cnt                 = 0;
    uint16 mem_base                = 0;
    uint32 cmd                     = 0;
    uint8 trunk_id                 = p_trunk->trunk_id;
    uint32 gport                   = p_trunk->gport;
    uint32 index                   = 0;
    uint16 chan_id                 = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    DsLinkAggregateChannelGroup_m ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;

    int32 ret = 0;
    uint8  is_dlb                  = 0;


    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == chan_id)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
         return CTC_E_NOT_EXIST;

    }
    if(p_sys_trunk->replace_en)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Only support replace API operation \n");
        return CTC_E_PARAM_CONFLICT;
    }

    if (TRUE == _sys_usw_stacking_find_trunk_port(lchip, trunk_id, chan_id, &index))
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk member port exist\n");
        return CTC_E_EXIST;
    }

    SYS_STK_DBG_INFO("Add uplink gport :%d, chan_id:%d\n", gport, chan_id);

    CTC_ERROR_RETURN(_sys_usw_stacking_get_trunk_info(lchip, trunk_id, &mem_cnt, &mem_base, &is_dlb));

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));

    if (((MCHIP_CAP(SYS_CAP_STK_TRUNK_DLB_MAX_MEMBERS) == mem_cnt) && (CTC_STK_LOAD_DYNAMIC == p_sys_trunk->mode))
        || ((p_sys_trunk->max_mem_cnt == mem_cnt) && (CTC_STK_LOAD_DYNAMIC != p_sys_trunk->mode)))
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk exceed max member port\n");
        return CTC_E_INVALID_CONFIG;
    }
   /*for restore shape profile*/
    CTC_ERROR_RETURN(_sys_usw_stacking_change_stacking_port(lchip,p_sys_trunk,gport,TRUE));

    if (p_sys_trunk->mode == CTC_STK_LOAD_DYNAMIC)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_dlb_add_member(lchip, trunk_id, mem_cnt, chan_id), ret, end);
    }
    else if (CTC_STK_LOAD_STATIC_FAILOVER == p_sys_trunk->mode)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_failover_add_member(lchip, trunk_id, gport), ret, end);
    }

    if(CTC_FLAG_ISSET(p_sys_trunk->flag, CTC_STACKING_TRUNK_MEM_ASCEND_ORDER))
    {
        uint8 temp_mem_idx = mem_cnt;
        DsLinkAggregateChannelMember_m  hw_member;
        uint32 cmdr = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        uint32 cmdw = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);

        for(; temp_mem_idx > 0; temp_mem_idx--)
        {
            DRV_IOCTL(lchip, temp_mem_idx+mem_base-1, cmdr, &hw_member);
            if(chan_id >= GetDsLinkAggregateChannelMember(V, channelId_f, &hw_member))
            {
                break;
            }
            DRV_IOCTL(lchip, temp_mem_idx+mem_base, cmdw, &hw_member);
        }

        index = temp_mem_idx;
    }
    else
    {
        index = mem_cnt;
    }

    if (!is_dlb)
    {
        sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));
        cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
        SetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member, chan_id);
        DRV_IOCTL(lchip, mem_base+index, cmd, &ds_link_aggregate_sgmac_member);
    }

    /*update trunk members*/
    mem_cnt++;

    if ((0 == g_stacking_master[lchip]->bind_mcast_en))
    {
        sys_usw_stacking_all_mcast_bind_en(lchip, 1);
    }

    /*for the first member in dlb mode ,need flush active */
    if ((CTC_STK_LOAD_DYNAMIC == p_sys_trunk->mode) && (1 == mem_cnt))
    {
       _sys_usw_stacking_clear_flow_active(lchip, trunk_id);
    }

    SetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group, mem_cnt);

    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group);


    return CTC_E_NONE;

end:
   _sys_usw_stacking_change_stacking_port(lchip,p_sys_trunk,gport,FALSE);
    return ret;
}

int32
_sys_usw_stacking_remove_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint32 cmd                     = 0;
    uint8 trunk_id                 = 0;
    uint32 gport                   = p_trunk->gport;
    uint16 last_lport              = 0;
    uint32 mem_index               = 0;
    uint32 index                   = 0;
    uint32 field_val               = 0;
    uint16 chan_id                 = 0;
    uint16 chan_tmp                = 0;
    uint16 mem_cnt                 = 0;
    uint16 mem_base                = 0;
    uint8 mode                     = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    DsLinkAggregateChannelGroup_m ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;
    uint8  is_dlb                  = 0;


    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    trunk_id = p_trunk->trunk_id;
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == chan_id)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
			return CTC_E_NOT_EXIST;

    }
    if(p_sys_trunk->replace_en)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Only support replace API operation \n");
        return CTC_E_PARAM_CONFLICT;
    }
    if (FALSE == _sys_usw_stacking_find_trunk_port(lchip, trunk_id, chan_id, &mem_index))
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk member port not exist\n");
			return CTC_E_NOT_EXIST;
    }

    SYS_STK_DBG_INFO("Remove gport %d, mem_index = %d\n", gport, mem_index);

    CTC_ERROR_RETURN(_sys_usw_stacking_get_trunk_info(lchip, trunk_id, &mem_cnt, &mem_base, &is_dlb));

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));

    mode = GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group);

    if ((mem_index < (mem_cnt - 1)) && (mode != CTC_STK_LOAD_DYNAMIC))
    {   /*Need replace this member using last member*/
        sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));
        if (CTC_FLAG_ISSET(p_sys_trunk->flag, CTC_STACKING_TRUNK_MEM_ASCEND_ORDER))
        {
            uint8 tmp_index = mem_index;
            DsLinkAggregateChannelMember_m  hw_member;
            uint32 cmdr = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            uint32 cmdw = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);

            for(;tmp_index < mem_cnt-1; tmp_index++)
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+mem_base+1, cmdr, &hw_member));
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_index+mem_base, cmdw, &hw_member));
            }
        }
        else
        {
            index = mem_base + mem_cnt - 1;
            cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_link_aggregate_sgmac_member));
            chan_tmp = GetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member);
            if (0xFF == chan_tmp)
            {
                SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
                return CTC_E_INVALID_PORT;

            }

            sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));
            SetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member, chan_tmp);

            SYS_STK_DBG_INFO("Need replace this member using last member drv lport:%d, chan:%d\n", last_lport, chan_tmp);

            index = mem_base + mem_index;
            cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_link_aggregate_sgmac_member));
        }
    }

    /*update trunk members*/
    mem_cnt--;
    field_val = mem_cnt;
    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DsLinkAggregateChannelGroup_channelLinkAggMemNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_val));

    if (p_sys_trunk->mode == CTC_STK_LOAD_DYNAMIC)
    {
        _sys_usw_stacking_dlb_del_member(lchip, trunk_id, mem_index, mem_cnt, chan_id);
    }
    else if (CTC_STK_LOAD_STATIC_FAILOVER == p_sys_trunk->mode)
    {
        _sys_usw_stacking_failover_del_member(lchip, trunk_id, gport);
    }

    if ((1 == g_stacking_master[lchip]->bind_mcast_en)
    && (0 == _sys_usw_stacking_is_trunk_have_member(lchip)))
    {
        sys_usw_stacking_all_mcast_bind_en(lchip, 0);
    }

    /*for restore shape profile*/
    CTC_ERROR_RETURN(_sys_usw_stacking_change_stacking_port(lchip,p_sys_trunk,gport,FALSE));

   return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_find_no_dupicated(uint8 lchip, uint32* new_members, uint8 new_cnt,
                                          uint32* old_members, uint8 old_cnt,
                                          uint32* find_members, uint8* find_cnt)
{
    uint16 count = 0;
    uint16 loop, loop1;
    uint8  is_new_member;
    uint8  duplicated_member;
    uint32  temp_port;

    for(loop=0;loop<new_cnt; loop++)
    {
        if (0xFF == SYS_GET_CHANNEL_ID(lchip, new_members[loop]))
        {
           return CTC_E_INVALID_LOCAL_PORT;
        }
        temp_port = new_members[loop];
        is_new_member = 1;
        for(loop1=0; loop1 < old_cnt; loop1++)
        {
            if(temp_port == old_members[loop1])
            {
                is_new_member = 0;
                break;
            }
        }
        if(is_new_member)
        {
            duplicated_member = 0;
            for(loop1=0; loop1 < count;loop1++)
            {
                if(temp_port == find_members[loop1])
                {
                    duplicated_member = 1;
                    break;
                }
            }

            if(!duplicated_member)
            {
                find_members[count++] = temp_port;
            }
        }
    }
    *find_cnt = count;
    return CTC_E_NONE;
}

int32
_sys_usw_stacking_replace_trunk_ports(uint8 lchip, uint8 trunk_id, uint32* gports, uint8 mem_ports)
{
    int32  ret = 0;
    uint32 cmd;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    uint32 fld_val = 0;
    uint8  loop = 0;
    uint8  add_loop = 0;
    uint8  del_loop = 0;
    uint16 mem_base = 0;
    uint8  add_ports_num;
    uint8  del_ports_num;
    uint32 add_ports[32] = {0};
    uint32 del_ports[32] = {0};
    uint32 old_ports[32] = {0};
    uint8  old_ports_num;
    uint16 cur_mem_cnt = 0;
    uint16 chan_id = 0;
    uint8  is_dlb                  = 0;

    SYS_STK_INIT_CHECK(lchip);

    CTC_PTR_VALID_CHECK(gports);
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    SYS_STK_DBG_FUNC();
    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        return CTC_E_STK_TRUNK_NOT_EXIST;
    }

    if (CTC_STK_LOAD_STATIC != p_sys_trunk->mode)
    {

        return CTC_E_NOT_SUPPORT;
    }

    sys_usw_stacking_get_member_ports(lchip, trunk_id, &old_ports[0],&old_ports_num);

    if (mem_ports > p_sys_trunk->max_mem_cnt)
    {

        return CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT;
    }

    /*1.find add ports */
    /*2.find delete ports */
    CTC_ERROR_GOTO(_sys_usw_stacking_find_no_dupicated(lchip, gports, mem_ports, old_ports, old_ports_num, add_ports, &add_ports_num), ret, error_proc);
    CTC_ERROR_GOTO(_sys_usw_stacking_find_no_dupicated(lchip, old_ports, old_ports_num, gports, mem_ports, del_ports, &del_ports_num), ret, error_proc);

    /*3. set delete ports, change stacking port to normal port*/
    for (del_loop = 0; del_loop < del_ports_num; del_loop++)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_change_stacking_port(lchip, p_sys_trunk, del_ports[del_loop], FALSE), ret, error_proc);
    }

    /*4. set add ports, change normal port to stacking port */
    for (add_loop = 0; add_loop < add_ports_num; add_loop++)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_change_stacking_port(lchip, p_sys_trunk, add_ports[add_loop], TRUE), ret, error_proc);
    }

    CTC_ERROR_GOTO(_sys_usw_stacking_get_trunk_info(lchip, p_sys_trunk->trunk_id, &cur_mem_cnt, &mem_base, &is_dlb), ret, error_proc);

    /*5. replace per port,don't need rollback !!!!!!!!!!!!!!*/
    for (loop = 0; loop < mem_ports; loop++)
    {
        chan_id = SYS_GET_CHANNEL_ID(lchip,  gports[loop]);
        fld_val = chan_id;
        cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
        DRV_IOCTL(lchip, mem_base + loop, cmd, &fld_val);

        fld_val = p_sys_trunk->trunk_id;
        cmd = DRV_IOW(DsLinkAggregateChannel_t, DsLinkAggregateChannel_u1_g2_linkAggregationChannelGroup_f);
        DRV_IOCTL(lchip, chan_id, cmd, &fld_val);
    }

    fld_val = mem_ports;
    cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DsLinkAggregateChannelGroup_channelLinkAggMemNum_f);
    DRV_IOCTL(lchip, trunk_id, cmd, &fld_val);

    if (!g_stacking_master[lchip]->bind_mcast_en && mem_ports)
    {
        sys_usw_stacking_all_mcast_bind_en(lchip, 1);
    }

    if ((1 == g_stacking_master[lchip]->bind_mcast_en)
        && (0 == _sys_usw_stacking_is_trunk_have_member(lchip)))
    {
        sys_usw_stacking_all_mcast_bind_en(lchip, 0);
    }

    p_sys_trunk->replace_en = 1;

    return CTC_E_NONE;
error_proc:

    for (loop = 0; loop < add_loop; loop++)
    {
        _sys_usw_stacking_change_stacking_port(lchip, p_sys_trunk, add_ports[loop], FALSE);
    }

    /*4. set delete ports  drop en = 1 and switch port to normal port*/
    for (loop = 0; loop < del_loop; loop++)
    {
        _sys_usw_stacking_change_stacking_port(lchip, p_sys_trunk, del_ports[loop], TRUE);
    }

    return ret;
}

#define ___TRUNK__API___
int32
sys_usw_stacking_create_trunk(uint8 lchip, ctc_stacking_trunk_t * p_trunk)
{
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);

    if (p_trunk->encap_hdr.vlan_valid)
    {
        CTC_VLAN_RANGE_CHECK(p_trunk->encap_hdr.vlan_id);
    }

    SYS_STK_TRUNKID_RANGE_CHECK(p_trunk->trunk_id);

    if (p_trunk->load_mode >= CTC_STK_LOAD_MODE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((0 == g_stacking_master[lchip]->trunk_mode)
		&& (CTC_STK_LOAD_DYNAMIC == p_trunk->load_mode))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (2 == g_stacking_master[lchip]->trunk_mode)
    {
        if (CTC_STK_LOAD_DYNAMIC == p_trunk->load_mode)
        {
            p_trunk->max_mem_cnt = 32;
        }
        else
        {
            if (p_trunk->max_mem_cnt > MCHIP_CAP(SYS_CAP_STK_TRUNK_STATIC_MAX_MEMBERS) ||  0 == p_trunk->max_mem_cnt)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }
    if(CTC_FLAG_ISSET(p_trunk->flag, CTC_STACKING_TRUNK_MEM_ASCEND_ORDER) && p_trunk->load_mode != CTC_STK_LOAD_STATIC)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Only static mode support \n");
        return CTC_E_INVALID_PARAM;
    }

    if (CTC_FLAG_ISSET(p_trunk->flag, CTC_STACKING_TRUNK_LB_HASH_OFFSET_VALID))
    {
        CTC_MAX_VALUE_CHECK(p_trunk->lb_hash_offset, 127)
    }

    if (!DRV_IS_DUET2(lchip))
    {
       if(p_trunk->encap_hdr.hdr_flag != CTC_STK_HDR_WITH_NONE &&
        p_trunk->encap_hdr.hdr_flag != CTC_STK_HDR_WITH_L2)
       {
           SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Header encap not support \n");
           return CTC_E_INVALID_PARAM;
       }
    }

    STACKING_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_stacking_create_trunk(lchip, p_trunk), g_stacking_master[lchip]->p_stacking_mutex);
    STACKING_UNLOCK;
    return CTC_E_NONE;
}
int32
sys_usw_stacking_destroy_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    SYS_STK_TRUNKID_RANGE_CHECK(p_trunk->trunk_id);

    STACKING_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_stacking_destroy_trunk(lchip, p_trunk), g_stacking_master[lchip]->p_stacking_mutex);
    STACKING_UNLOCK;

    return CTC_E_NONE;
}
int32
sys_usw_stacking_add_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint16 lport;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_INIT_CHECK(lchip);

    CTC_PTR_VALID_CHECK(p_trunk);
    SYS_STK_TRUNKID_RANGE_CHECK(p_trunk->trunk_id);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);

    STACKING_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_stacking_add_trunk_port(lchip, p_trunk), g_stacking_master[lchip]->p_stacking_mutex);
    STACKING_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_stacking_remove_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint16 lport;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);

    SYS_STK_TRUNKID_RANGE_CHECK(p_trunk->trunk_id);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);

    STACKING_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_stacking_remove_trunk_port(lchip, p_trunk), g_stacking_master[lchip]->p_stacking_mutex);
    STACKING_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_stacking_replace_trunk_ports(uint8 lchip, uint8 trunk_id, uint32* gports, uint8 mem_ports)
{
    int32 ret = 0;

    SYS_STK_INIT_CHECK(lchip);
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    STACKING_LOCK;
    ret = _sys_usw_stacking_replace_trunk_ports(lchip, trunk_id, gports, mem_ports);
    STACKING_UNLOCK;

    return ret;
}
int32
sys_usw_stacking_get_member_ports(uint8 lchip, uint8 trunk_id, uint32* p_gports, uint8* cnt)
{
	uint32 field_val = 0;
	uint16 mem_cnt = 0;
	uint16 mem_base = 0;
	uint32 cmd = 0;
	uint16 mem_idx = 0;
    uint8 gchip_id = 0;
    DsLinkAggregateChannelGroup_m ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_gports);
    CTC_PTR_VALID_CHECK(cnt);

    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    sys_usw_get_gchip_id(lchip, &gchip_id);


    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));


	mem_cnt = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group);
	mem_base    = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group);

    sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));

    if (GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group) != 1)
    {
        cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
        for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_base + mem_idx, cmd, &field_val));
            p_gports[mem_idx] =  CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_GET_LPORT_ID_WITH_CHAN(lchip, field_val));
        }
    }
    else
    {
        for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
        {
            cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+mem_idx);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_val));
            p_gports[mem_idx] = CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_GET_LPORT_ID_WITH_CHAN(lchip, field_val));
        }
    }
    *cnt = mem_cnt;

    return CTC_E_NONE;
}


int32
sys_usw_stacking_add_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    int32 ret                      = 0;
    uint8 trunk_id                 = 0;
    uint8 rgchip                   = 0;
    uint8 gchip                    = 0;
    uint32 index                   = 0;
    ctc_stacking_trunk_t trunk;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    trunk_id = p_trunk->trunk_id;
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);
    rgchip    = p_trunk->remote_gchip;
    SYS_STK_CHIPID_CHECK(rgchip);

    /*if gchip is local, return */
    if (p_trunk->extend.enable)
    {
        if (rgchip == p_trunk->extend.gchip)
        {
            return CTC_E_NONE;
        }
    }
    else
    {
        if (sys_usw_chip_is_local(lchip, rgchip))
        {
            return CTC_E_NONE;
        }
    }


    if (p_trunk->select_mode == 1)
    {
        uint32 lport = 0;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->src_gport, lchip, lport);
    }

    STACKING_LOCK;

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
        ret = CTC_E_NOT_EXIST;
		goto unlock;
    }

    if (p_trunk->select_mode == 1)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_add_trunk_port_rchip(lchip, p_trunk), ret, unlock);
    }
    else
    {
        uint32 cmd                     = 0;
        uint32 field_val               = 0;

        sal_memcpy(&trunk, p_trunk, sizeof(trunk));
        for (index = 0; index < MCHIP_CAP(SYS_CAP_STK_MAX_LPORT); index++)
        {
            sys_usw_get_gchip_id(lchip, &gchip);
            trunk.src_gport =  SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, index);
            CTC_ERROR_GOTO(_sys_usw_stacking_add_trunk_port_rchip(lchip, &trunk), ret, unlock);
        }

        index = (0 << 7) + p_trunk->remote_gchip;
        field_val = p_trunk->trunk_id;
        if (!g_stacking_master[lchip]->fabric_mode)
        {
            cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &field_val), ret, unlock);
        }
    }

    STACKING_UNLOCK;
    return CTC_E_NONE;

    unlock:
    STACKING_UNLOCK;
    return ret;
}

int32
sys_usw_stacking_remove_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    int32 ret                      = 0;
    uint8 trunk_id                 = 0;
    uint8 rgchip                   = 0;
    uint8 gchip                    = 0;
    uint32 index                   = 0;
    ctc_stacking_trunk_t trunk;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);

    trunk_id = p_trunk->trunk_id;
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    rgchip    = p_trunk->remote_gchip;
    SYS_STK_CHIPID_CHECK(rgchip);

    /*if gchip is local, return */
    if (p_trunk->extend.enable)
    {
        if (rgchip == p_trunk->extend.gchip)
        {
            return CTC_E_NONE;
        }
    }
    else
    {
        if (sys_usw_chip_is_local(lchip, rgchip))
        {
            return CTC_E_NONE;
        }
    }


    if (p_trunk->select_mode == 1)
    {
        uint32 lport = 0;
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->src_gport, lchip, lport);
    }

    STACKING_LOCK;

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
        ret = CTC_E_NOT_EXIST;
		goto unlock;

    }

    if (p_trunk->select_mode == 1)
    {
        CTC_ERROR_GOTO(_sys_usw_stacking_remove_trunk_port_rchip(lchip, p_trunk), ret, unlock);
    }
    else
    {
        uint32 cmd                     = 0;
        uint32 field_val               = 0;

        sal_memcpy(&trunk, p_trunk, sizeof(trunk));
        for (index = 0; index < MCHIP_CAP(SYS_CAP_STK_MAX_LPORT); index++)
        {
            sys_usw_get_gchip_id(lchip, &gchip);
            trunk.src_gport =  SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, index);
            CTC_ERROR_GOTO(_sys_usw_stacking_remove_trunk_port_rchip(lchip, &trunk), ret, unlock);
        }

        index = (0 << 7) + p_trunk->remote_gchip;
        field_val = 0;
        cmd = DRV_IOW(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &field_val), ret, unlock);

    }

    STACKING_UNLOCK;
    return CTC_E_NONE;

    unlock:
    STACKING_UNLOCK;
    return ret;
}


int32
sys_usw_stacking_get_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    SYS_STK_CHIPID_CHECK(p_trunk->remote_gchip);

    if (p_trunk->select_mode == 0)
    {
        cmd = DRV_IOR(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_trunk->remote_gchip, cmd, &field_val));
    }
    else
    {
        uint32 port_fwd_sel = 0;
        uint32 lport = 0;
		uint32 index = 0;

        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_trunk->src_gport, lchip, lport);

        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

        index = (port_fwd_sel << 7) + p_trunk->remote_gchip;
        cmd = DRV_IOR(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));


    }

    STACKING_LOCK;

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, field_val);
    if (NULL == p_sys_trunk)
    {
        STACKING_UNLOCK;
        return CTC_E_STK_TRUNK_NOT_EXIST;
    }

    STACKING_UNLOCK;

    p_trunk->trunk_id = field_val;

    return CTC_E_NONE;
}


/***********************************************************
** LOOP break functions
************************************************************/
#define _____LOOP_BREAK_____ ""


STATIC int32
_sys_usw_stacking_set_discard_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd       = 0;
    MetFifoCtl_m met_fifo_ctl;
    uint32 field_val[4] = {0};

    sal_memset(&met_fifo_ctl, 0, sizeof(MetFifoCtl_m));



    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    if (p_stop_rchip->mode == CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP)
    {
        GetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);
        field_val[0] = p_stop_rchip->rchip_bitmap;
        SetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);
    }
    else if(p_stop_rchip->mode == CTC_STK_STOP_MODE_DISCARD_RCHIP)
    {
        uint32 rgchip = p_stop_rchip->rchip;

        SYS_STK_CHIPID_CHECK(rgchip);

        GetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);

        if (p_stop_rchip->discard)
        {
            CTC_BIT_SET(field_val[rgchip / 32], rgchip % 32);
        }
        else
        {
            CTC_BIT_UNSET(field_val[rgchip / 32], rgchip % 32);
        }

        SetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);
    }

    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_get_discard_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd       = 0;
    uint32 field_val[4] = {0};
    MetFifoCtl_m met_fifo_ctl;

    sal_memset(&met_fifo_ctl, 0, sizeof(MetFifoCtl_m));

    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));


    if (p_stop_rchip->mode == CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP)
    {
        GetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);
        p_stop_rchip->rchip_bitmap = field_val[0];
    }
    else if(p_stop_rchip->mode == CTC_STK_STOP_MODE_DISCARD_RCHIP)
    {
        uint32 rgchip = p_stop_rchip->rchip;

        SYS_STK_CHIPID_CHECK(rgchip);

        GetMetFifoCtl(A, endRemoteChip_f, &met_fifo_ctl, field_val);

        p_stop_rchip->discard =  CTC_IS_BIT_SET(field_val[rgchip / 32], rgchip % 32);
    }



    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_set_discard_port_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd                     = 0;
    uint32 lport                   = 0;
    uint32 index                   = 0;
    uint32 bitmap[2] = {0};
    DsCFlexSrcPortBlockMask_m block;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stop_rchip);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_stop_rchip->src_gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);
    SYS_STK_CHIPID_CHECK(p_stop_rchip->rchip);

	index = p_stop_rchip->rchip;

    sal_memset(&block, 0, sizeof(block));
    cmd = DRV_IOR(DsCFlexSrcPortBlockMask_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &block));
    GetDsCFlexSrcPortBlockMask(A, srcPortBitMap_f, &block, bitmap);

    if (p_stop_rchip->discard)
    {
		CTC_BMP_SET(bitmap, lport);
    }
    else
    {
		CTC_BMP_UNSET(bitmap, lport);
    }

    SetDsCFlexSrcPortBlockMask(A, srcPortBitMap_f, &block, bitmap);


    cmd = DRV_IOW(DsCFlexSrcPortBlockMask_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &block));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_get_discard_port_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd                     = 0;
    uint32 lport                   = 0;
    uint32 index                   = 0;
    uint32 bitmap[2] = {0};
    DsCFlexSrcPortBlockMask_m block;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stop_rchip);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_stop_rchip->src_gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);
    SYS_STK_CHIPID_CHECK(p_stop_rchip->rchip);

	index = p_stop_rchip->rchip;

    sal_memset(&block, 0, sizeof(block));
    cmd = DRV_IOR(DsCFlexSrcPortBlockMask_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &block));
    GetDsCFlexSrcPortBlockMask(A, srcPortBitMap_f, &block, bitmap);

    if (CTC_BMP_ISSET(bitmap, lport))
    {
		p_stop_rchip->discard = 1;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_set_block_port_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint8  loop_chip;

    sys_stacking_block_profile_t*  p_profile_get = NULL;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stop_rchip);
    SYS_STK_CHIPID_CHECK(p_stop_rchip->rchip);

    CTC_ERROR_RETURN(_sys_usw_stacking_add_block_port_profile(lchip, p_stop_rchip, &p_profile_get));

    cmd = DRV_IOW(DsCFlexDstChannelBlockMask_t, DsCFlexDstChannelBlockMask_dstPortBitMap_f);
    for(loop_chip=0; loop_chip < SYS_STK_MAX_GCHIP; loop_chip++)
    {
        index = (loop_chip<<3) + p_profile_get->profile_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_profile_get->bitmap[loop_chip]));
    }

    return CTC_E_NONE;
}



STATIC int32
_sys_usw_stacking_get_block_port_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint32 lport                   = 0;
    uint32 port_block_sel          = 0;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stop_rchip);
    SYS_STK_CHIPID_CHECK(p_stop_rchip->rchip);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_stop_rchip->src_gport, lchip, lport);
    SYS_STK_LPORT_CHECK(lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexDstIsolateGroupSel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_block_sel));

	index = (p_stop_rchip->rchip<<3) + port_block_sel;

    cmd = DRV_IOR(DsCFlexDstChannelBlockMask_t, DsCFlexDstChannelBlockMask_dstPortBitMap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_stop_rchip->pbm));


    return CTC_E_NONE;
}



STATIC int32
_sys_usw_stacking_set_stop_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{

    switch(p_stop_rchip->mode)
    {
    case CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP:
    case CTC_STK_STOP_MODE_DISCARD_RCHIP:
        CTC_ERROR_RETURN(_sys_usw_stacking_set_discard_rchip(lchip, p_stop_rchip));
        break;


    case CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT:
        CTC_ERROR_RETURN(_sys_usw_stacking_set_discard_port_rchip(lchip, p_stop_rchip));
        break;

    case CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT:
        CTC_ERROR_RETURN(_sys_usw_stacking_set_block_port_rchip(lchip, p_stop_rchip));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_get_stop_rchip(uint8 lchip, ctc_stacking_stop_rchip_t *p_stop_rchip)
{
    switch(p_stop_rchip->mode)
    {
    case CTC_STK_STOP_MODE_DISCARD_RCHIP_BITMAP:
    case CTC_STK_STOP_MODE_DISCARD_RCHIP:
        CTC_ERROR_RETURN(_sys_usw_stacking_get_discard_rchip(lchip, p_stop_rchip));
        break;

    case CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT:
        CTC_ERROR_RETURN(_sys_usw_stacking_get_discard_port_rchip(lchip, p_stop_rchip));
        break;

    case CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT:
        CTC_ERROR_RETURN(_sys_usw_stacking_get_block_port_rchip(lchip, p_stop_rchip));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_set_break_en(uint8 lchip, uint32 enable)
{
    uint32 cmd       = 0;
    uint32 field_val = 0;

    field_val = enable;
    cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_stackingBroken_f);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_get_break_en(uint8 lchip, uint32* enable)
{
    uint32 cmd       = 0;
    uint32 field_val = 0;

    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_stackingBroken_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    *enable = field_val;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_set_full_mesh_en(uint8 lchip, uint32 enable)
{
    uint32 cmd       = 0;
    uint32 field_val = 0;

    field_val = enable ? 0 : 1;
    cmd = DRV_IOW(MetFifoCtl_t, MetFifoCtl_forceReplicationFromFabric_f);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_get_full_mesh_en(uint8 lchip, uint32* enable)
{
    uint32 cmd       = 0;
    uint32 field_val = 0;

    cmd = DRV_IOR(MetFifoCtl_t, MetFifoCtl_forceReplicationFromFabric_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    *enable = field_val ? 0 : 1;

    return CTC_E_NONE;
}

int32
sys_usw_stacking_set_global_cfg(uint8 lchip, ctc_stacking_glb_cfg_t* p_glb_cfg)
{

    CTC_ERROR_RETURN(_sys_usw_stacking_hdr_set(lchip, &p_glb_cfg->hdr_glb));

    return CTC_E_NONE;
}

int32
sys_usw_stacking_get_global_cfg(uint8 lchip, ctc_stacking_glb_cfg_t* p_glb_cfg)
{


    CTC_ERROR_RETURN(_sys_usw_stacking_hdr_get(lchip, &p_glb_cfg->hdr_glb));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_stacking_set_break_trunk_id(uint8 lchip, ctc_stacking_mcast_break_point_t *p_break_point)
{
    uint32 cmd = 0;
    MetFifoCtl_m met_fifo_ctl;
    uint32 source_bitmap[2] = {0};

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_break_point);
    SYS_STK_TRUNKID_RANGE_CHECK(p_break_point->trunk_id)

    sal_memset(&met_fifo_ctl, 0, sizeof(met_fifo_ctl));
    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));
    GetMetFifoCtl(A, sgmacDiscardSourceReplication_f, &met_fifo_ctl, source_bitmap);

    if (p_break_point->enable)
    {
        if (p_break_point->trunk_id >= 32)
        {
            CTC_BIT_SET(source_bitmap[1], (p_break_point->trunk_id - 32));
        }
        else
        {
            CTC_BIT_SET(source_bitmap[0], p_break_point->trunk_id );
        }
    }
    else
    {
        if (p_break_point->trunk_id >= 32)
        {
            CTC_BIT_UNSET(source_bitmap[1], (p_break_point->trunk_id - 32));
        }
        else
        {
            CTC_BIT_UNSET(source_bitmap[0], p_break_point->trunk_id );
        }
    }

    SetMetFifoCtl(A, sgmacDiscardSourceReplication_f, &met_fifo_ctl, source_bitmap);
    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    return CTC_E_NONE;
}


int32
sys_usw_stacking_map_gport(uint8 lchip, uint32 gport, uint16* p_index)
{
    uint16 lport                   = 0;
    uint8 rchip                    = 0;
    uint32 min                     = 0;
    uint32 max                     = 0;
    uint32 base                    = 0;
    uint32 cmd                     = 0;
	DsGlbDestPortMap_m port_map;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_index);

	rchip = CTC_MAP_GPORT_TO_GCHIP(gport);
	lport = CTC_MAP_GPORT_TO_LPORT(gport);

    SYS_STK_CHIPID_CHECK(rchip);


    cmd = DRV_IOR(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, rchip, cmd, &port_map));

    base = GetDsGlbDestPortMap(V, glbDestPortBase_f, &port_map);
    min  = GetDsGlbDestPortMap(V, glbDestPortMin_f, &port_map);
    max  = GetDsGlbDestPortMap(V, glbDestPortMax_f, &port_map);

    if ( lport > max  ||  lport < min)
    {
        return CTC_E_INVALID_CHIP_ID;
    }

    *p_index = base + lport;

     return CTC_E_NONE;

}


int32
sys_usw_stacking_alloc_glb_destport(uint8 lchip, uint16 max_lport, ctc_stacking_rchip_port_t * p_rchip_port)
{
    int32  ret                     = 0;
    uint32 base                    = 0;
    uint32 max                     = 0;
    uint32 min                     = 0;
    uint32 cmd                     = 0;
    uint16 loop1                   = 0;
	uint32 field_val               = 0;
    sys_usw_opf_t  opf;

    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_legacyCFHeaderEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (1 == field_val)
    {
        return CTC_E_NONE;
    }

    min = 0;
    max = max_lport;
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortMin_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &min));
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortMax_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &max));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_ABILITY;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, max_lport + 1, &base));

    SYS_STK_DBG_INFO("Alloc glbDestPortBase :%d, num:%d\n", base,  max_lport + 1);
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortBase_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &base), ret, Error0);

    for (loop1 = 0; loop1 <= max_lport; loop1++)
    {
        CTC_ERROR_GOTO(sys_usw_port_glb_dest_port_init(lchip, loop1 + base), ret, Error0);
    }

    return CTC_E_NONE;


    Error0:
    sys_usw_opf_free_offset(lchip, &opf, max_lport + 1, base);
    return ret;

}



int32
sys_usw_stacking_free_glb_destport(uint8 lchip, uint16 max_lport, ctc_stacking_rchip_port_t * p_rchip_port)
{
    sys_usw_opf_t  opf;
    uint32 base                    = 0;
    uint32 max                     = 0;
    uint32 min                     = 0;
    uint32 cmd                     = 0;
	uint32 field_val               = 0;

    min = 1;
    max = 0;
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortMin_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &min));
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortMax_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &max));


    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_legacyCFHeaderEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    if (1 == field_val)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &base));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_stacking_master[lchip]->opf_type;
    opf.pool_index = SYS_STK_OPF_ABILITY;
    SYS_STK_DBG_INFO("Free glbDestPortBase :%d num:%d\n", base, max_lport + 1);

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, max_lport + 1, base));



    base = 0;
    cmd = DRV_IOW(DsGlbDestPortMap_t, DsGlbDestPortMap_glbDestPortBase_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &base));


    return CTC_E_NONE;

}


extern int32
sys_usw_l2_reserve_rchip_ad_index(uint8 lchip, uint8 gchip, uint16 max_port_num);
extern int32
sys_usw_l2_free_rchip_ad_index(uint8 lchip, uint8 gchip, uint16 max_port_num);


int32
sys_usw_stacking_set_rchip_port(uint8 lchip, ctc_stacking_rchip_port_t * p_rchip_port)
{
	uint16 loop1                   = 0;
    uint16 loop2                   = 0;
    uint16 lport                   = 0;
    uint16 max_lport               = 0;
    uint32 gport                   = 0;
    uint8 gchip_id                 = 0;
	uint16 valid_cnt               = 0;
	uint32 cmd                     = 0;
    uint16 max                     = 0;
    uint16 min                     = 0;
    DsGlbDestPortMap_m             port_map;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rchip_port);
    SYS_STK_CHIPID_CHECK(p_rchip_port->rchip);

    sys_usw_get_gchip_id(lchip, &gchip_id);

    if (p_rchip_port->rchip == gchip_id)
    {
        return CTC_E_NONE;
    }

    for (loop1 = 0; loop1 < sizeof(ctc_port_bitmap_t) / sizeof(uint32); loop1++)
    {
        for (loop2 = 0; loop2 < CTC_UINT32_BITS; loop2++)
        {
            lport = loop1 * CTC_UINT32_BITS + loop2;

            gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(p_rchip_port->rchip, lport);

            if (CTC_IS_BIT_SET(p_rchip_port->pbm[loop1], loop2))
            {
                valid_cnt++;
                max_lport = lport > max_lport?lport: max_lport;
                sys_usw_brguc_nh_create(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC);
            }
            else
            {
                sys_usw_brguc_nh_delete(lchip, gport);
            }
        }
    }


    cmd = DRV_IOR(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &port_map));

    max    = GetDsGlbDestPortMap(V, glbDestPortMax_f, &port_map);
    min    = GetDsGlbDestPortMap(V, glbDestPortMin_f, &port_map);

    if (valid_cnt)
    {
        if (min != 1 || max != 0)
        {
            CTC_ERROR_RETURN(sys_usw_l2_free_rchip_ad_index(lchip, p_rchip_port->rchip, max + 1));
            CTC_ERROR_RETURN(sys_usw_stacking_free_glb_destport(lchip, max, p_rchip_port));
        }

        CTC_ERROR_RETURN(sys_usw_l2_reserve_rchip_ad_index(lchip, p_rchip_port->rchip, max_lport + 1));
        CTC_ERROR_RETURN(sys_usw_stacking_alloc_glb_destport(lchip, max_lport, p_rchip_port));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_l2_free_rchip_ad_index(lchip, p_rchip_port->rchip, max + 1));
        CTC_ERROR_RETURN(sys_usw_stacking_free_glb_destport(lchip, max, p_rchip_port));
    }

    return CTC_E_NONE;



}

int32
sys_usw_stacking_get_rchip_port(uint8 lchip, ctc_stacking_rchip_port_t* p_rchip_port)
{
    uint32 cmd                     = 0;
    uint8 gchip_id                 = 0;
	DsGlbDestPortMap_m port_map;
    uint16 max                     = 0;
    uint16 min                     = 0;
	uint16 loop1                   = 0;

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_rchip_port);
    SYS_STK_CHIPID_CHECK(p_rchip_port->rchip);

    sys_usw_get_gchip_id(lchip, &gchip_id);
    if (p_rchip_port->rchip == gchip_id)
    {
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rchip_port->rchip, cmd, &port_map));

    max    = GetDsGlbDestPortMap(V, glbDestPortMax_f, &port_map);
    min    = GetDsGlbDestPortMap(V, glbDestPortMin_f, &port_map);


    for (loop1 = min; loop1 < max + 1; loop1++)
    {
        CTC_BMP_SET(p_rchip_port->pbm, loop1);
    }

    return CTC_E_NONE;
}



int32
sys_usw_stacking_binding_trunk(uint8 lchip, uint32 gport, uint8 trunk_id)/*trunk_id = 0 means unbinding*/
{
    uint32 field_val = 0;
    uint8 chan_id = 0;
    uint32 cmd = 0;
	uint32 lport = 0;

    SYS_STK_INIT_CHECK(lchip);
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xff == chan_id)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid port \n");
        return CTC_E_INVALID_PORT;
    }

    field_val = trunk_id;
    cmd = DRV_IOW(DsSrcSgmacGroup_t, DsSrcSgmacGroup_sgmacGroupId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    g_stacking_master[lchip]->binding_trunk = 1; /*change to 1 once call by user*/

    return CTC_E_NONE;
}

int32
sys_usw_stacking_get_binding_trunk(uint8 lchip, uint32 gport, uint8* trunk_id)/*trunk_id = 0 means unbinding*/
{
    uint32 field_val = 0;
    uint8 chan_id = 0;
    uint32 cmd = 0;
	uint32 lport = 0;

    SYS_STK_INIT_CHECK(lchip);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xff == chan_id)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid port \n");
        return CTC_E_INVALID_PORT;
    }

    cmd = DRV_IOR(DsSrcSgmacGroup_t, DsSrcSgmacGroup_sgmacGroupId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
	*trunk_id = field_val;

    return CTC_E_NONE;
}


int32
sys_usw_stacking_set_property(uint8 lchip, ctc_stacking_property_t* p_prop)
{
    int32 ret    = 0;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_prop);

    STACKING_LOCK;


    switch (p_prop->prop_type)
    {

    case CTC_STK_PROP_GLOBAL_CONFIG:
        {
            ctc_stacking_glb_cfg_t* p_glb_cfg = NULL;
            p_glb_cfg = (ctc_stacking_glb_cfg_t *)p_prop->p_value;
            CTC_ERROR_GOTO(sys_usw_stacking_set_global_cfg(lchip, p_glb_cfg), ret, unlock);
        }
        break;

    case CTC_STK_PROP_MCAST_STOP_RCHIP:
        {
            ctc_stacking_stop_rchip_t* p_stop_rchip = NULL;
            p_stop_rchip = (ctc_stacking_stop_rchip_t *)p_prop->p_value;
            CTC_ERROR_GOTO(_sys_usw_stacking_set_stop_rchip(lchip, p_stop_rchip), ret, unlock);
        }
        break;

    case CTC_STK_PROP_BREAK_EN:
        CTC_ERROR_GOTO(_sys_usw_stacking_set_break_en(lchip, p_prop->value), ret, unlock);
        break;

    case CTC_STK_PROP_FULL_MESH_EN:
        CTC_ERROR_GOTO(_sys_usw_stacking_set_full_mesh_en(lchip, p_prop->value), ret, unlock);
        break;

    case CTC_STK_PROP_MCAST_BREAK_POINT:
        {
            ctc_stacking_mcast_break_point_t *p_break_point = NULL;
            p_break_point = (ctc_stacking_mcast_break_point_t *)p_prop->p_value;
            CTC_ERROR_GOTO(_sys_usw_stacking_set_break_trunk_id(lchip, p_break_point), ret, unlock);
        }
        break;

    case CTC_STK_PROP_PORT_BIND_TRUNK:
        {
            ctc_stacking_bind_trunk_t *p_bind_trunk = NULL;
            p_bind_trunk = (ctc_stacking_bind_trunk_t *)p_prop->p_value;
            if(!p_bind_trunk)
            {
                STACKING_UNLOCK;
                return CTC_E_INVALID_PTR;
            }
            CTC_ERROR_GOTO(sys_usw_stacking_binding_trunk(lchip, p_bind_trunk->gport, p_bind_trunk->trunk_id), ret, unlock);
        }
        break;

    case CTC_STK_PROP_RCHIP_PORT_EN:
        {
            ctc_stacking_rchip_port_t *p_rchip_port = NULL;
            p_rchip_port = (ctc_stacking_rchip_port_t *)p_prop->p_value;
            CTC_ERROR_GOTO(sys_usw_stacking_set_rchip_port(lchip, p_rchip_port), ret, unlock);
        }
        break;
    case CTC_STK_PROP_STK_PORT_EN:
        {
            ctc_stacking_port_cfg_t* p_stk_port;
            ctc_stacking_hdr_encap_t hdr_encap;
            uint8 chan_id = 0;
            p_stk_port = (ctc_stacking_port_cfg_t *)p_prop->p_value;
            if ((p_stk_port->direction > CTC_BOTH_DIRECTION) || (p_stk_port->hdr_type > CTC_STK_HDR_WITH_IPV6))
            {
                STACKING_UNLOCK;
    			return CTC_E_INVALID_PARAM;
            }

            chan_id = SYS_GET_CHANNEL_ID(lchip, p_stk_port->gport);
            if (0xFF == chan_id)
            {
                SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
                STACKING_UNLOCK;
    			return CTC_E_INVALID_PORT;
            }

            sal_memset(&hdr_encap, 0, sizeof(ctc_stacking_hdr_encap_t));
            hdr_encap.hdr_flag = p_stk_port->hdr_type;
            if (p_stk_port->enable)
            {
                CTC_ERROR_GOTO(_sys_usw_stacking_encap_header_enable(lchip, chan_id, &hdr_encap, p_stk_port->direction), ret, unlock);
                if ((p_stk_port->direction == CTC_INGRESS) || (p_stk_port->direction == CTC_BOTH_DIRECTION))
                {
                    CTC_BMP_SET(g_stacking_master[lchip]->ing_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(p_stk_port->gport));
                }

                if ((p_stk_port->direction == CTC_EGRESS) || (p_stk_port->direction == CTC_BOTH_DIRECTION))
                {
                    CTC_BMP_SET(g_stacking_master[lchip]->egr_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(p_stk_port->gport));
                }
            }
            else
            {
                CTC_ERROR_GOTO(_sys_usw_stacking_encap_header_disable(lchip, chan_id, p_stk_port->direction), ret, unlock);
                if ((p_stk_port->direction == CTC_INGRESS) || (p_stk_port->direction == CTC_BOTH_DIRECTION))
                {
                    CTC_BMP_UNSET(g_stacking_master[lchip]->ing_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(p_stk_port->gport));
                }

                if ((p_stk_port->direction == CTC_EGRESS) || (p_stk_port->direction == CTC_BOTH_DIRECTION))
                {
                    CTC_BMP_UNSET(g_stacking_master[lchip]->egr_port_en_bmp, CTC_MAP_GPORT_TO_LPORT(p_stk_port->gport));
                }
            }
        }
        break;
    default:
		ret = CTC_E_NOT_SUPPORT;
		goto unlock;
    }

    STACKING_UNLOCK;
    return CTC_E_NONE;

    unlock:
    STACKING_UNLOCK;
	return ret;

}

int32
sys_usw_stacking_get_property(uint8 lchip, ctc_stacking_property_t* p_prop)
{
    int32 ret    = 0;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_prop);

    STACKING_LOCK;

    switch (p_prop->prop_type)
    {

    case CTC_STK_PROP_GLOBAL_CONFIG:
        CTC_ERROR_GOTO(sys_usw_stacking_get_global_cfg(lchip, (ctc_stacking_glb_cfg_t *)p_prop->p_value), ret, unlock);
        break;

    case CTC_STK_PROP_MCAST_STOP_RCHIP:
        CTC_ERROR_GOTO(_sys_usw_stacking_get_stop_rchip(lchip, (ctc_stacking_stop_rchip_t *)p_prop->p_value), ret, unlock);
        break;

    case CTC_STK_PROP_BREAK_EN:
        CTC_ERROR_GOTO(_sys_usw_stacking_get_break_en(lchip, &p_prop->value), ret, unlock);
        break;

    case CTC_STK_PROP_FULL_MESH_EN:
        CTC_ERROR_GOTO(_sys_usw_stacking_get_full_mesh_en(lchip, &p_prop->value), ret, unlock);
        break;

    case CTC_STK_PROP_PORT_BIND_TRUNK:
        {
            ctc_stacking_bind_trunk_t *p_bind_trunk = NULL;
            p_bind_trunk = (ctc_stacking_bind_trunk_t *)p_prop->p_value;

            if(!p_bind_trunk)
            {
                STACKING_UNLOCK;
                return CTC_E_INVALID_PTR;
            }
            CTC_ERROR_GOTO(sys_usw_stacking_get_binding_trunk(lchip, p_bind_trunk->gport, &p_bind_trunk->trunk_id), ret, unlock);
        }
        break;

    case CTC_STK_PROP_RCHIP_PORT_EN:
        CTC_ERROR_GOTO(sys_usw_stacking_get_rchip_port(lchip, (ctc_stacking_rchip_port_t*)p_prop->p_value), ret, unlock);
        break;
    case CTC_STK_PROP_STK_PORT_EN:
        {
            ctc_stacking_port_cfg_t* p_stk_port;
            uint32 cmd = 0;
            uint32 field_val = 0;
            uint8 chan_id = 0;
            p_stk_port = (ctc_stacking_port_cfg_t *)p_prop->p_value;
            if (p_stk_port->direction >= CTC_BOTH_DIRECTION)
            {
                ret = CTC_E_INVALID_PARAM;
                goto unlock;
            }
            chan_id = SYS_GET_CHANNEL_ID(lchip, p_stk_port->gport);
            if (0xFF == chan_id)
            {
                SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
    			ret =  CTC_E_INVALID_PORT;
                goto unlock;
            }

            if (p_stk_port->direction == CTC_INGRESS)
            {
                cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &field_val), ret, unlock);
                p_stk_port->hdr_type = (field_val==DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2))?CTC_STK_HDR_WITH_L2:0;
                p_stk_port->enable = field_val?1:0;
            }
            else
            {
                BufRetrvChanStackingEn_m stacking_en;
                uint32 packet_header_en[2] = {0};
                cmd = DRV_IOR(BufRetrvChanStackingEn_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &stacking_en), ret, unlock);
                GetBufRetrvChanStackingEn(A, chanStackingEn_f, &stacking_en, packet_header_en);
                p_stk_port->enable = CTC_IS_BIT_SET(packet_header_en[chan_id>>5], (chan_id&0x1F))?1:0;
                cmd = DRV_IOR(DsPacketHeaderEditTunnel_t, DsPacketHeaderEditTunnel_packetHeaderL2En_f);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, chan_id, cmd, &field_val), ret, unlock);
                p_stk_port->hdr_type = field_val?CTC_STK_HDR_WITH_L2:0;
            }
        }
        break;
    default:
		ret = CTC_E_NOT_SUPPORT;
		goto unlock;
    }

    STACKING_UNLOCK;
    return CTC_E_NONE;

    unlock:
    STACKING_UNLOCK;
	return ret;

}


STATIC int32
_sys_usw_stacking_set_enable(uint8 lchip, uint8 enable, uint8 mode, uint8 version)
{
    CTC_ERROR_RETURN(_sys_usw_stacking_register_en(lchip, enable, mode, version));
    return CTC_E_NONE;
}
#if 0
int32
sys_usw_stacking_get_enable(uint8 lchip, bool* enable, uint32* stacking_mcast_offset)
{
    uint8 trunk_idx = 0;
    uint8 trunk_num = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;

    CTC_PTR_VALID_CHECK(enable);
    CTC_PTR_VALID_CHECK(stacking_mcast_offset);

    if (NULL == g_stacking_master[lchip])
    {
        *enable = FALSE;
        return CTC_E_NONE;
    }

    STACKING_LOCK;

    for (trunk_idx = SYS_STK_TRUNK_MIN_ID; trunk_idx < MCHIP_CAP(SYS_CAP_STK_TRUNK_MAX_ID); trunk_idx++)
    {
        p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_idx);
        if (NULL != p_sys_trunk)
        {
            trunk_num++;
        }
    }

    STACKING_UNLOCK;

    *enable = (g_stacking_master[lchip]->stacking_en && trunk_num)? TRUE : FALSE;
    *stacking_mcast_offset = g_stacking_master[lchip]->stacking_mcast_offset;


    return CTC_E_NONE;
}
#endif
int32
sys_usw_stacking_get_stkhdr_len(uint8 lchip,
                                  uint32 gport,
                                  uint8* p_packet_head_en,
                                  uint8* p_legacy_en,
                                  uint16* p_cloud_hdr_len)
{
    uint32 cmd                     = 0;
    uint32 field_val               = 0;
    uint8 vlan_valid               = 0;
    uint8 header_l3_offset         = 0;
    uint8 packet_header_len        = 0;
    uint8 mux_type                 = 0;
    uint8 udp_en                   = 0;
    uint8 legacy_en                = 0;
    uint8 chan_id                  = 0;

    CTC_PTR_VALID_CHECK(p_packet_head_en);
    CTC_PTR_VALID_CHECK(p_legacy_en);
    CTC_PTR_VALID_CHECK(p_cloud_hdr_len);

    if (NULL == g_stacking_master[lchip])
    {
        *p_cloud_hdr_len = 0;
        return CTC_E_NONE;
    }

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == chan_id)
    {
        *p_cloud_hdr_len = 0;
        return DRV_E_NONE;
    }


    /*mux type*/
    cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    mux_type = field_val;

    cmd = DRV_IOR(DsPacketHeaderEditTunnel_t, DsPacketHeaderEditTunnel_vlanIdValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    vlan_valid = field_val;

    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_headerUdpEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    udp_en = field_val;

    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_legacyCFHeaderEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    legacy_en = field_val;

    /* PACKET_HEADER_DECISION */
    if (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL) == mux_type ||
        DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2) == mux_type ||
        DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4) == mux_type ||
        DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6) == mux_type ||
        DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV4) == mux_type ||
        DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV6) == mux_type)
    { /* With PacketHeader */

        *p_packet_head_en = 1;

        if (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITHOUT_TUNNEL) == mux_type)
        {
            /* without L2/L3 tunnel header, do not need update tunnel header */
        }
        else
        { /* with L2/L3 tunnel header */
            if (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2) == mux_type ||
                DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4) == mux_type ||
                DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6) == mux_type)
            { /* with L2 Header */
                header_l3_offset = 14 + (vlan_valid << 2);
            }
            else
            { /* without L2 Header */
                header_l3_offset = 0;
            }

            packet_header_len += header_l3_offset;

            if (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2) == mux_type)  /* only L2 Header */
            {
                /*only add l2 header length over stacking header*/
            }
            else
            {
                if ((DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV4) == mux_type) ||
                    (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV4) == mux_type))  /* IPv4 */
                {
                    /*add l2 header + ip header + udp header length over stacking header*/
                    packet_header_len += (udp_en) ? 28 : 20;

                }
                else if ((DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_L2_AND_IPV6) == mux_type) ||
                         (DRV_ENUM(DRV_STK_MUX_TYPE_HDR_WITH_IPV6) == mux_type)) /* IPv6 */
                {
                    /*add l2 header + ip header + udp header length over stacking header*/
                    packet_header_len += (udp_en) ? 48 : 40;
                }
            }
        }

        *p_cloud_hdr_len = packet_header_len;
		*p_legacy_en = legacy_en;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_show_trunk_path(uint8 lchip, uint8 trunk_id)
{
    uint32 port_fwd_sel            = 0;
    uint16 rchip                   = 0;
    uint16 lport                   = 0;
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint32 field_val               = 0;
	uint32 bmp[8] = {0};
	uint8  i = 0;
	uint8  is_bit_set = 0;

    SYS_STK_DBG_DUMP("\ntrunk select path \n");
    SYS_STK_DBG_DUMP("%-10s %10s\n", "pbm", "rchip");
    SYS_STK_DBG_DUMP("======================================\n");

    for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
    {
        for (lport = 0; lport < 256; lport++)
        {
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

            index = (port_fwd_sel << 7) + rchip;
			cmd = DRV_IOR(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

            if (trunk_id != field_val)
            {
                continue;
            }

            CTC_BMP_SET(bmp, lport);
			is_bit_set = 1;
        }

        if (is_bit_set == 0)
        {
            continue;
        }


        for (i = 0; i < 8; i++)
        {
            if (i == 0)
            {
                SYS_STK_DBG_DUMP("0x%08x %10d\n", bmp[i],  rchip);
            }
            else
            {
                SYS_STK_DBG_DUMP("0x%08x\n", bmp[i]);
            }

            bmp[i] = 0;
        }

		is_bit_set = 0;
        SYS_STK_DBG_DUMP("\n");

    }

    return CTC_E_NONE;
}


int32
sys_usw_stacking_show_trunk_info(uint8 lchip, uint8 trunk_id)
{
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    uint32 mem_idx                 = 0;
    uint32 cmd                     = 0;
    uint8 load_mode                = 0;
    uint16 mem_base                = 0;
    uint16 mem_cnt                 = 0;
    uint32 field_val               = 0;
    uint32 gport                   = 0;
    uint8 gchip_id                 = 0;
    uint8 trunk_mode               = 0;
    char* mode[4] = {"Static", "Dynamic", "Failover", "Reserved"};
    DsLinkAggregateChannelGroup_m ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;

    SYS_STK_INIT_CHECK(lchip);
    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);


    STACKING_LOCK;
    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
        STACKING_UNLOCK;
        return CTC_E_NOT_EXIST;

    }
    trunk_mode = p_sys_trunk->mode;
    STACKING_UNLOCK;


    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));

    load_mode = GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group);
	mem_cnt = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group);
	mem_base    = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group);

    SYS_STK_DBG_DUMP("stacking trunk info below:\n");

    SYS_STK_DBG_DUMP("--------------------------------\n");
    SYS_STK_DBG_DUMP("trunk member ports\n");
    SYS_STK_DBG_DUMP("--------------------------------\n");
    SYS_STK_DBG_DUMP("trunk id:%d, mode:%s, count:%d\n", trunk_id, mode[trunk_mode],mem_cnt);
    sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip_id));
    if (load_mode != 1)
    {
        cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
        for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_base+mem_idx, cmd, &field_val));
            gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_GET_LPORT_ID_WITH_CHAN(lchip, field_val));
            SYS_STK_DBG_DUMP("Member port:0x%x\n", gport);
        }
    }
    else
    {
        for (mem_idx = 0; mem_idx < mem_cnt; mem_idx++)
        {
            cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f+mem_idx);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &field_val));
            gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, SYS_GET_LPORT_ID_WITH_CHAN(lchip, field_val));
            SYS_STK_DBG_DUMP("Member port:0x%x\n", gport);
        }
    }

    _sys_usw_stacking_show_trunk_path(lchip, trunk_id);

    return CTC_E_NONE;
}

int32
sys_usw_stacking_failover_detect(uint8 lchip, uint8 linkdown_chan)
{

    return CTC_E_NONE;
}





#define _____MCAST_TRUNK_GROUP_____ ""

int32
sys_usw_stacking_set_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    int32 ret = CTC_E_NONE;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_INIT_CHECK(lchip);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "profile id: %d \n", p_mcast_profile->mcast_profile_id);

    if (p_mcast_profile->mcast_profile_id >= 256)
    {
        return CTC_E_INVALID_PARAM;
    }

    STACKING_LOCK;

    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 0;
    mcast_db.id   = p_mcast_profile->mcast_profile_id;

    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    switch(p_mcast_profile->type)
    {
    case CTC_STK_MCAST_PROFILE_CREATE:
        {
            uint16 mcast_profile_id = 0;

            if (0 == p_mcast_profile->mcast_profile_id)
            {
                sys_usw_opf_t  opf;
                /*alloc profile id*/
                uint32 value_32 = 0;

                sal_memset(&opf, 0, sizeof(opf));
                opf.pool_type  = g_stacking_master[lchip]->opf_type;
                opf.pool_index = SYS_STK_OPF_MCAST_PROFILE;
                CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, 1, &value_32),
                                                                       ret, Error);
                mcast_profile_id = value_32;

            }
            else
            {
                if (NULL != p_mcast_db)
                {
                    ret = CTC_E_ENTRY_EXIST;
                    goto Error;
                }

                mcast_profile_id = p_mcast_profile->mcast_profile_id;
            }

            ret = sys_usw_stacking_mcast_group_create(lchip, 0,
                                                              mcast_profile_id,
                                                              &p_mcast_db,
                                                              p_mcast_profile->append_en);

            if (CTC_E_NONE != ret) /*roolback*/
            {
                if (0 == p_mcast_profile->mcast_profile_id)
                {
                    sys_usw_opf_t  opf;
                    sal_memset(&opf, 0, sizeof(opf));
                    opf.pool_type  = g_stacking_master[lchip]->opf_type;
                    opf.pool_index = SYS_STK_OPF_MCAST_PROFILE;
                    sys_usw_opf_free_offset(lchip, &opf, 1, mcast_profile_id);
                }
                goto Error;
            }

            if (0 == p_mcast_profile->mcast_profile_id)
            {
                if (NULL != p_mcast_db)
                {
                    p_mcast_db->alloc_id = 1;
                }

                p_mcast_profile->mcast_profile_id = mcast_profile_id;
            }
        }

        break;

    case CTC_STK_MCAST_PROFILE_DESTROY:
        {
            uint8 alloc_id;
            if (0 == p_mcast_profile->mcast_profile_id)
            {
                ret = CTC_E_NONE;
                goto Error;
            }

            if (NULL == p_mcast_db)
            {
                ret = CTC_E_NOT_EXIST;
                goto Error;
            }
            alloc_id = p_mcast_db->alloc_id;
            CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_destroy(lchip, 0,
                                                                       p_mcast_profile->mcast_profile_id, p_mcast_db),
                                                                       ret, Error);

            if (1 == alloc_id)
            {
                /*alloc profile id*/
                sys_usw_opf_t  opf;

                sal_memset(&opf, 0, sizeof(opf));
                opf.pool_type  = g_stacking_master[lchip]->opf_type;
                opf.pool_index = SYS_STK_OPF_MCAST_PROFILE;
                CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf, 1, p_mcast_profile->mcast_profile_id),
                                                                       ret, Error);
            }
        }

        break;

    case CTC_STK_MCAST_PROFILE_ADD:
        {
            uint32 bitmap[2] = {0};
            uint8 trunk_id = 0;

            if (NULL == p_mcast_db)
            {
                ret = CTC_E_NOT_EXIST;
                goto Error;
            }

            bitmap[0] = p_mcast_profile->trunk_bitmap[0] & (p_mcast_profile->trunk_bitmap[0] ^ p_mcast_db->trunk_bitmap[0]);
            bitmap[1] = p_mcast_profile->trunk_bitmap[1] & (p_mcast_profile->trunk_bitmap[1] ^ p_mcast_db->trunk_bitmap[1]);
            for (trunk_id = 1; trunk_id <= 63; trunk_id++)
            {
                if (!CTC_IS_BIT_SET(bitmap[trunk_id / 32], trunk_id % 32))
                {
                    continue;
                }

                ret =   sys_usw_stacking_mcast_group_add_member(lchip, p_mcast_db, trunk_id, 1);

                if (CTC_E_NONE != ret) /*roolback*/
                {
                    uint8 tmp_trunk_id = 0;

                    for (tmp_trunk_id = 1; tmp_trunk_id < trunk_id; tmp_trunk_id++)
                    {
                        if (!CTC_IS_BIT_SET(bitmap[tmp_trunk_id / 32], tmp_trunk_id % 32))
                        {
                            continue;
                        }
                        sys_usw_stacking_mcast_group_remove_member(lchip, p_mcast_db, tmp_trunk_id, 1);
                    }

                    goto Error;
                }

            }
        }

        break;

    case CTC_STK_MCAST_PROFILE_REMOVE:
        {
            uint32 bitmap[2] = {0};
            uint8 trunk_id = 0;

            if (NULL == p_mcast_db)
            {
                ret = CTC_E_NOT_EXIST;
                goto Error;
            }

            bitmap[0] = p_mcast_profile->trunk_bitmap[0] & p_mcast_db->trunk_bitmap[0];
            bitmap[1] = p_mcast_profile->trunk_bitmap[1] & p_mcast_db->trunk_bitmap[1];

            for (trunk_id = 1; trunk_id <= 63; trunk_id++)
            {
                if (!CTC_IS_BIT_SET(bitmap[trunk_id / 32], trunk_id % 32))
                {
                    continue;
                }
                ret = sys_usw_stacking_mcast_group_remove_member(lchip, p_mcast_db, trunk_id, 1);

                if (CTC_E_NONE != ret)
                {
                    goto Error;
                }
            }
        }

        break;
    }

    STACKING_UNLOCK;
    return CTC_E_NONE;

    Error:
    STACKING_UNLOCK;
    return ret;
}


int32
sys_usw_stacking_get_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_INIT_CHECK(lchip);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "profile id: %d \n", p_mcast_profile->mcast_profile_id);

    if (p_mcast_profile->mcast_profile_id >= 256)
    {
        return CTC_E_INVALID_PARAM;
    }


    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 0;
    mcast_db.id   = p_mcast_profile->mcast_profile_id;

    STACKING_LOCK;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    if (NULL == p_mcast_db)
    {
        STACKING_UNLOCK;
        return CTC_E_NOT_EXIST;
    }

    p_mcast_profile->trunk_bitmap[0] = p_mcast_db->trunk_bitmap[0];
    p_mcast_profile->trunk_bitmap[1] = p_mcast_db->trunk_bitmap[1];

    STACKING_UNLOCK;
    return CTC_E_NONE;
}


int32
sys_usw_stacking_create_keeplive_group(uint8 lchip, uint16 group_id)
{
    int32 ret = CTC_E_NONE;
    uint32 group_size = 0;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STK_DBG_PARAM("create group:0x%x\n", group_id);

    SYS_STK_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_ftm_get_dyn_entry_size(lchip, SYS_FTM_DYN_ENTRY_GLB_MET, &group_size));

    if (group_id >= group_size)
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_nh_check_glb_met_offset(lchip, group_id, 1, TRUE));
    CTC_ERROR_RETURN(sys_usw_nh_set_glb_met_offset(lchip, group_id, 1, TRUE));
    STACKING_LOCK;

    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 1;
    mcast_db.id   = group_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    if (NULL != p_mcast_db)
    {
        ret =  CTC_E_ENTRY_EXIST;
        goto Error;
    }

    CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_create( lchip,  1, group_id, &p_mcast_db, 0),
                   ret, Error);

    STACKING_UNLOCK;
    return CTC_E_NONE;

Error:
    STACKING_UNLOCK;
    sys_usw_nh_set_glb_met_offset(lchip, group_id, 1, FALSE);
    return ret;

}


int32
sys_usw_stacking_destroy_keeplive_group(uint8 lchip, uint16 group_id)
{
    int32 ret = CTC_E_NONE;
    uint32 group_size = 0;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);

    SYS_STK_DBG_PARAM("destroy group:0x%x\n", group_id);

    CTC_ERROR_RETURN(sys_usw_ftm_get_dyn_entry_size(lchip, SYS_FTM_DYN_ENTRY_GLB_MET, &group_size));

    if (group_id >= group_size)
    {
        return CTC_E_INVALID_PARAM;
    }

    STACKING_LOCK;

    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 1;
    mcast_db.id   = group_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    if (NULL == p_mcast_db)
    {
        ret =  CTC_E_NOT_EXIST;
        goto Error;
    }

    CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_destroy( lchip,  1, group_id, p_mcast_db),
                   ret, Error);

    STACKING_UNLOCK;
    sys_usw_nh_set_glb_met_offset(lchip, group_id, 1, FALSE);
    return CTC_E_NONE;

Error:
    STACKING_UNLOCK;
    return ret;


}


int32
sys_usw_stacking_keeplive_add_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{
    int32 ret = CTC_E_NONE;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);


    SYS_STK_DBG_PARAM("trunk:0x%x, gport:0x%x, type:%d, group:0x%x\n",
                      p_keeplive->trunk_id, p_keeplive->cpu_gport, p_keeplive->mem_type, p_keeplive->group_id);


    SYS_STK_INIT_CHECK(lchip);

    if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_TRUNK)
    {
        SYS_STK_TRUNKID_RANGE_CHECK(p_keeplive->trunk_id);
    }

    STACKING_LOCK;

    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 1;
    mcast_db.id   = p_keeplive->group_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    if (NULL == p_mcast_db)
    {
        ret =  CTC_E_NOT_EXIST;
        goto Error;
    }


    if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_TRUNK)
    {
        uint16 trunk_id = 0;

        trunk_id = p_keeplive->trunk_id;
        SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

        if (CTC_IS_BIT_SET(p_mcast_db->trunk_bitmap[trunk_id / 32], trunk_id % 32))
        {
            ret = CTC_E_MEMBER_EXIST;
            goto Error;
        }

        CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_add_member(lchip, p_mcast_db, trunk_id, 1),
                       ret, Error);

    }
    else if (!CTC_IS_CPU_PORT(p_keeplive->cpu_gport))
    {
        uint16 dest_id = 0;
        if (FALSE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(p_keeplive->cpu_gport)))
        {
            ret = CTC_E_INVALID_PORT;
            goto Error;
        }
        dest_id = CTC_MAP_GPORT_TO_LPORT(p_keeplive->cpu_gport);
        CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_add_member(lchip, p_mcast_db, dest_id, 0),
                       ret, Error);
    }
    else
    {
        uint16 dest_id = 0;
        uint32 dsnh_offset = 0;
        uint32 cmd = 0;
		uint8 gchip_id = 0;
		uint8 sub_queue_id = 0;
        uint8 grp_id = 0;
        DsMetEntry3W_m dsmet3w;

        sys_usw_get_gchip_id(lchip, &gchip_id);
        CTC_ERROR_GOTO(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_C2C_PKT, &sub_queue_id), ret, Error);

        /*p_keeplive->cpu_gport(0) mean hign prio reason grp*/
        if (CTC_IS_BIT_SET(p_keeplive->cpu_gport, 0))
        {
            grp_id = (sub_queue_id / MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM)) + 1;
        }
        else
        {
            grp_id = (sub_queue_id / MCHIP_CAP(SYS_CAP_REASON_GRP_QUEUE_NUM));
        }

        /*to cpu based on prio*/
        dest_id =  SYS_ENCODE_EXCP_DESTMAP_MET_GRP(gchip_id, grp_id);
        dsnh_offset = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_C2C_PKT, 0);

        cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->head_met_offset, cmd, &dsmet3w),
                       ret, Error);

        SetDsMetEntry3W(V, ucastId_f,  &dsmet3w, dest_id);
        SetDsMetEntry3W(V, replicationCtl_f,  &dsmet3w, (dsnh_offset << 5));

        cmd = DRV_IOW(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->head_met_offset, cmd, &dsmet3w),
                       ret, Error);
        p_mcast_db->cpu_port = p_keeplive->cpu_gport;
    }

Error:
    STACKING_UNLOCK;
    return ret;
}


int32
sys_usw_stacking_keeplive_remove_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{
    int32 ret = CTC_E_NONE;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);


    SYS_STK_DBG_PARAM("trunk:0x%x, gport:0x%x, type:%d, group:0x%x\n",
                      p_keeplive->trunk_id, p_keeplive->cpu_gport, p_keeplive->mem_type, p_keeplive->group_id);

    if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_TRUNK)
    {
        SYS_STK_TRUNKID_RANGE_CHECK(p_keeplive->trunk_id);
    }

    STACKING_LOCK;

    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 1;
    mcast_db.id   = p_keeplive->group_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);

    if (NULL == p_mcast_db)
    {
        ret = CTC_E_NOT_EXIST;
        goto Error;
    }


    if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_TRUNK)
    {
        uint16 trunk_id = 0;

        trunk_id = p_keeplive->trunk_id;

        if (!CTC_IS_BIT_SET(p_mcast_db->trunk_bitmap[trunk_id / 32], trunk_id % 32))
        {
            ret =  CTC_E_MEMBER_NOT_EXIST;
            goto Error;
        }

        CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_remove_member(lchip, p_mcast_db, p_keeplive->trunk_id, 1),
                         ret, Error);
    }
    else if (!CTC_IS_CPU_PORT(p_keeplive->cpu_gport))
    {
        uint16 dest_id = 0;
        if (FALSE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(p_keeplive->cpu_gport)))
        {
            ret = CTC_E_INVALID_PORT;
            goto Error;
        }
        dest_id = CTC_MAP_GPORT_TO_LPORT(p_keeplive->cpu_gport);
        CTC_ERROR_GOTO(sys_usw_stacking_mcast_group_remove_member(lchip, p_mcast_db, dest_id, 0),
                       ret, Error);
    }
    else
    {
        uint16 dest_id                 = 0;
        uint32 dsnh_offset             = 0;
        uint32 cmd                     = 0;
        DsMetEntry3W_m dsmet3w;

        dest_id  = SYS_RSV_PORT_DROP_ID;
        dsnh_offset = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_C2C_PKT, 0);

        cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->head_met_offset, cmd, &dsmet3w),
                         ret, Error);

        SetDsMetEntry3W(V, ucastId_f,  &dsmet3w, dest_id);
        SetDsMetEntry3W(V, replicationCtl_f,  &dsmet3w, (dsnh_offset << 5));

        cmd = DRV_IOW(DsMetEntry3W_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO( DRV_IOCTL(lchip, p_mcast_db->head_met_offset, cmd, &dsmet3w),
                         ret, Error);

    }

Error:
    STACKING_UNLOCK;
    return ret;
}

int32
sys_usw_stacking_keeplive_get_members(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{
    int32 ret = CTC_E_NONE;
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STK_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);

    SYS_STK_DBG_PARAM("group:0x%x mem_type:%d\n", p_keeplive->group_id, p_keeplive->mem_type);

    STACKING_LOCK;
    /*lookup*/
    sal_memset(&mcast_db, 0, sizeof(mcast_db));
    mcast_db.type = 1;
    mcast_db.id   = p_keeplive->group_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);
    if (NULL == p_mcast_db)

{
        ret = CTC_E_NOT_EXIST;
        goto EXIT;
    }

    if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_TRUNK)
    {
        sal_memcpy(p_keeplive->trunk_bitmap, p_mcast_db->trunk_bitmap, sizeof(p_keeplive->trunk_bitmap));
        p_keeplive->trunk_bmp_en = 1;
    }
    else
    {
        uint32 cmd = 0;
        uint16 dest_id                 = 0;
        uint32 dsnh_offset             = 0;
        DsMetEntry3W_m dsmet3w;
        uint8 gchip = 0;
        uint32 met_offset = p_mcast_db->head_met_offset;

        while(met_offset != 0xffff)
        {
            sal_memset(&dsmet3w, 0, sizeof(dsmet3w));
            cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, met_offset, cmd, &dsmet3w),
                             ret, EXIT);
            dest_id = GetDsMetEntry3W(V, ucastId_f, &dsmet3w);
            dsnh_offset = GetDsMetEntry3W(V, replicationCtl_f, &dsmet3w);
            met_offset  = GetDsMetEntry3W(V, nextMetEntryPtr_f, &dsmet3w);

            if (GetDsMetEntry3W(V, remoteChip_f, &dsmet3w) || (dest_id == SYS_RSV_PORT_DROP_ID))
            {
                continue;
            }

            if ((dest_id != SYS_RSV_PORT_DROP_ID)
             && ((dsnh_offset >> 5) == CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_C2C_PKT, 0)))
            {
                 sys_usw_get_gchip_id(lchip, &gchip);
                 p_keeplive->cpu_gport = CTC_LPORT_CPU | ((gchip & CTC_GCHIP_MASK) << CTC_LOCAL_PORT_LENGTH);
            }
            else if ((dsnh_offset >> 5) != CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_C2C_PKT, 0))
            {
                sys_usw_get_gchip_id(lchip, &gchip);
                p_keeplive->cpu_gport = dest_id | ((gchip & CTC_GCHIP_MASK) << CTC_LOCAL_PORT_LENGTH);
            }
        }
    }

EXIT:
    STACKING_UNLOCK;
    return ret;
}


int32
sys_usw_stacking_get_mcast_profile_met_offset(uint8 lchip,  uint16 mcast_profile_id, uint32 *p_stacking_met_offset)
{
    sys_stacking_mcast_db_t mcast_db;
    sys_stacking_mcast_db_t *p_mcast_db = NULL;

    CTC_PTR_VALID_CHECK(p_stacking_met_offset);

    if (NULL == g_stacking_master[lchip])
    {
        *p_stacking_met_offset = 0;
        return CTC_E_NONE;
    }

    if (0 == mcast_profile_id)
    {
        *p_stacking_met_offset = g_stacking_master[lchip]->bind_mcast_en?
        g_stacking_master[lchip]->stacking_mcast_offset : 0;
        return CTC_E_NONE;
    }


    STACKING_LOCK;

    sal_memset(&mcast_db, 0, sizeof(mcast_db));

    mcast_db.type = 0;
    mcast_db.id   = mcast_profile_id;
    p_mcast_db = ctc_hash_lookup(g_stacking_master[lchip]->mcast_hash, &mcast_db);


    if (NULL == p_mcast_db)
    {
        STACKING_UNLOCK;
        return CTC_E_NOT_EXIST;
    }

    *p_stacking_met_offset = p_mcast_db->head_met_offset;

    STACKING_UNLOCK;
    return CTC_E_NONE;
}






#define _____SYS_API_____ ""

int32
sys_usw_stacking_mcast_mode(uint8 lchip, uint8 mcast_mode)
{
    SYS_STK_INIT_CHECK(lchip);

    g_stacking_master[lchip]->mcast_mode = mcast_mode;
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stacking_update_load_mode(uint8 lchip, uint8 trunk_id, ctc_stacking_load_mode_t load_mode)
{
    uint8 mem_index                = 0;
    uint32 chan_id                 = 0;
    uint32 cmd                     = 0;
    uint32 index                   = 0;
    uint16 mem_cnt                 = 0;
    uint16 mem_base                = 0;
    uint8 mode                     = 0;
    uint8 old_load_mode            = 0;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    DsLinkAggregateChannelGroup_m  ds_link_aggregate_sgmac_group;
    DsLinkAggregateChannelMember_m ds_link_aggregate_sgmac_member;
    DsLinkAggregateChannel_m linkagg_channel;

    SYS_STK_INIT_CHECK(lchip);

    SYS_STK_TRUNKID_RANGE_CHECK(trunk_id);

    if (load_mode >= CTC_STK_LOAD_MODE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (1 != g_stacking_master[lchip]->trunk_mode)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] need switch static/dynmaic when trunk mode 1 \n");
        return CTC_E_INVALID_CONFIG;
    }

    p_sys_trunk = ctc_vector_get(g_stacking_master[lchip]->p_trunk_vec, trunk_id);
    if (NULL == p_sys_trunk)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [STACKING] Trunk not exist \n");
        return CTC_E_NOT_EXIST;
    }


    if (p_sys_trunk->mode == load_mode)
    {
        return CTC_E_NONE;
    }

    sal_memset(&ds_link_aggregate_sgmac_group, 0, sizeof(DsLinkAggregateChannelGroup_m));
    cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));


	mem_cnt     = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemNum_f, &ds_link_aggregate_sgmac_group);
    mem_base    = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &ds_link_aggregate_sgmac_group);
    mode        = GetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group);

    if (mode == 1)
    {
        old_load_mode = CTC_STK_LOAD_DYNAMIC;
    }
    else if(mode == 0)
    {
        old_load_mode = CTC_STK_LOAD_STATIC;
    }

    if (CTC_STK_LOAD_DYNAMIC == load_mode)
    {
        /* static/failover update to dynamic */

        p_sys_trunk->max_mem_cnt = 32;
		p_sys_trunk->mode = CTC_STK_LOAD_DYNAMIC;

        if (mem_cnt > MCHIP_CAP(SYS_CAP_STK_TRUNK_DLB_MAX_MEMBERS))
        {
            return CTC_E_INVALID_CONFIG;
        }

        /* 1.update member to dynamic */
        for (mem_index = 0; mem_index < mem_cnt; mem_index++)
        {
            cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_base + mem_index, cmd, &chan_id));

            CTC_ERROR_RETURN(_sys_usw_stacking_dlb_add_member(lchip, trunk_id, mem_index, chan_id));
        }

        CTC_ERROR_RETURN(_sys_usw_stacking_clear_flow_active(lchip, trunk_id));


        /* 2.update group to dynamic */
        SetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group, 1);
        SetDsLinkAggregateChannelGroup(V, channelLinkAggFlowNum_f, &ds_link_aggregate_sgmac_group, 3);

        cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));

    }
    else
    {

        p_sys_trunk->max_mem_cnt = MCHIP_CAP(SYS_CAP_STK_TRUNK_STATIC_MAX_MEMBERS);
		p_sys_trunk->mode = load_mode;


        /* failover/dynamic update to static; static/dynamic update to failover */
        if (old_load_mode == CTC_STK_LOAD_DYNAMIC)
        {
            /* 1.update group to static */
            SetDsLinkAggregateChannelGroup(V, channelLinkAggLbMode_f, &ds_link_aggregate_sgmac_group, 0);

            cmd = DRV_IOW(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, trunk_id, cmd, &ds_link_aggregate_sgmac_group));
        }

        /* 2.update member to static */
        for (mem_index = 0; mem_index < mem_cnt; mem_index++)
        {
            if (old_load_mode == CTC_STK_LOAD_DYNAMIC)
            {
                cmd = DRV_IOR(DsLinkAggregateChannelMemberSet_t, DsLinkAggregateChannelMemberSet_array_0_channelId_f + mem_index);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip,  trunk_id, cmd, &chan_id));
            }
            else
            {
                cmd = DRV_IOR(DsLinkAggregateChannelMember_t, DsLinkAggregateChannelMember_channelId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, mem_base + mem_index, cmd, &chan_id));
            }

            if (old_load_mode == CTC_STK_LOAD_DYNAMIC)
            {
                sal_memset(&ds_link_aggregate_sgmac_member, 0, sizeof(ds_link_aggregate_sgmac_member));
                cmd = DRV_IOW(DsLinkAggregateChannelMember_t, DRV_ENTRY_FLAG);
                SetDsLinkAggregateChannelMember(V, channelId_f, &ds_link_aggregate_sgmac_member, chan_id);
                index = mem_base + mem_index;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_link_aggregate_sgmac_member));
            }

            if (CTC_STK_LOAD_STATIC == load_mode)  /* failover/dynamic update to static */
            {
                CTC_ERROR_RETURN(_sys_usw_stacking_dlb_del_member_channel(lchip, trunk_id, chan_id));
            }
            else if (CTC_STK_LOAD_STATIC_FAILOVER == load_mode) /* static/dynamic update to failover */
            {
                sal_memset(&linkagg_channel, 0, sizeof(linkagg_channel));
                cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &linkagg_channel));
                SetDsLinkAggregateChannel(V, groupType_f, &linkagg_channel, 1);
                SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
                SetDsLinkAggregateChannel(V, u1_g2_linkAggregationChannelGroup_f, &linkagg_channel, trunk_id);

                cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &linkagg_channel));
            }
        }
    }


    return CTC_E_NONE;
}

int32
sys_usw_stacking_update_load_mode(uint8 lchip, uint8 trunk_id, ctc_stacking_load_mode_t load_mode)
{
    STACKING_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_stacking_update_load_mode(lchip, trunk_id, load_mode),
                                 g_stacking_master[lchip]->p_stacking_mutex);
    STACKING_UNLOCK;

    return CTC_E_NONE;
}


int32
sys_usw_stacking_set_trunk_mode(uint8 lchip, uint8 mode)
{
    ctc_vector_t* p_trunk_vec = NULL;;

    SYS_STK_INIT_CHECK(lchip);
    p_trunk_vec = g_stacking_master[lchip]->p_trunk_vec;
    if (!p_trunk_vec)
    {
        return CTC_E_NOT_INIT;
    }

    if (p_trunk_vec->used_cnt)
    {
        return CTC_E_IN_USE;
    }

    if (mode >= CTC_STACKING_TRUNK_MODE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    g_stacking_master[lchip]->trunk_mode = mode;

    return CTC_E_NONE;
}

#define _____DEBUG_SHOW_____ ""



int32
sys_usw_stacking_show_status(uint8 lchip)
{
    uint32 field_val = 0;
    uint32 cmd = 0;

    SYS_STK_INIT_CHECK(lchip);

    SYS_STK_DBG_DUMP("Global configure:\n");
    SYS_STK_DBG_DUMP("--------------------------------------------------------------------\n");
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Fabric mode",          g_stacking_master[lchip]->fabric_mode);
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Trunk mcast mode",     g_stacking_master[lchip]->mcast_mode);
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Trunk member mode",    g_stacking_master[lchip]->trunk_mode);
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Default mcast offset", g_stacking_master[lchip]->stacking_mcast_offset);

    cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_legacyCFHeaderEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Header version",  (1 == field_val)? CTC_STACKING_VERSION_1_0:CTC_STACKING_VERSION_2_0);

    SYS_STK_DBG_DUMP("\nTrunk path select profile:\n");
    SYS_STK_DBG_DUMP("--------------------------------------------------------------------\n");
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Total profile count",  g_stacking_master[lchip]->p_fwd_prof_spool->max_count);
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Used count",           g_stacking_master[lchip]->p_fwd_prof_spool->count);


    SYS_STK_DBG_DUMP("\nTrunk port block profile:\n");
    SYS_STK_DBG_DUMP("--------------------------------------------------------------------\n");
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Total profile count",  g_stacking_master[lchip]->p_block_prof_spool->max_count);
    SYS_STK_DBG_DUMP("%-30s :%10d\n", "Used count",           g_stacking_master[lchip]->p_block_prof_spool->count);

    return CTC_E_NONE;
}



#define _____WARM_BOOT_____ ""


STATIC int32
_sys_usw_stacking_wb_restore_block_profile(uint8 lchip)
{
    int32 ret                      = 0;
    uint32 cmd                     = 0;
    uint16 lport                   = 0;
    uint32 port_block_sel          = 0;
    uint8 rchip                    = 0;
    uint16 index                   = 0;
    sys_stacking_block_profile_t* p_profile_get = NULL;
    sys_stacking_block_profile_t* p_profile_old = NULL;
    sys_stacking_block_profile_t* p_new_profile = NULL;

    p_new_profile = (sys_stacking_block_profile_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_block_profile_t));
    if (NULL == p_new_profile)
    {
        return CTC_E_NO_MEMORY;
    }
    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_STK_MAX_LPORT); lport++)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexDstIsolateGroupSel_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, lport, cmd, &port_block_sel), ret, out);

        if (0 == port_block_sel)
        {
            continue;
        }

        sal_memset(p_new_profile, 0, sizeof(sys_stacking_block_profile_t));

        p_new_profile->profile_id = port_block_sel;

        for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
        {
            index = (rchip << 3) + port_block_sel;
            cmd = DRV_IOR(DsCFlexDstChannelBlockMask_t, DsCFlexDstChannelBlockMask_dstPortBitMap_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, p_new_profile->bitmap[rchip]), ret, out);
        }

        CTC_ERROR_GOTO(ctc_spool_add(g_stacking_master[lchip]->p_block_prof_spool,
                                       p_new_profile,
                                       p_profile_old,
                                       &p_profile_get), ret, out);


        g_stacking_master[lchip]->p_block_profile[port_block_sel] = p_profile_get;

    }

out:
    if (p_new_profile)
    {
        mem_free(p_new_profile);
    }

    return ret;

}

STATIC int32
_sys_usw_stacking_wb_restore_fwd_profile(uint8 lchip)
{
    int32 ret                      = 0;
    uint32 cmd                     = 0;
    uint16 lport                   = 0;
    uint32 port_fwd_sel            = 0;
    uint8 rchip                    = 0;
    uint16 index                   = 0;
    uint32 field_val               = 0;
    sys_stacking_fwd_profile_t* p_profile_get = NULL;
    sys_stacking_fwd_profile_t* p_profile_old = NULL;
    sys_stacking_fwd_profile_t new_profile;


    for (lport = 0; lport < MCHIP_CAP(SYS_CAP_STK_MAX_LPORT); lport++)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cFlexFwdSgGroupSel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_fwd_sel));

        if (0 == port_fwd_sel)
        {
            continue;
        }

        sal_memset(&new_profile, 0, sizeof(new_profile));

        new_profile.profile_id = port_fwd_sel;

        for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
        {
            index = (port_fwd_sel << 7) + rchip;
            cmd = DRV_IOR(DsSgmacMap_t, DsSgmacMap_linkAggregationChannelGroup_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
			new_profile.trunk[rchip] = field_val;
        }

        CTC_ERROR_RETURN(ctc_spool_add(g_stacking_master[lchip]->p_fwd_prof_spool,
                                       &new_profile,
                                       p_profile_old,
                                       &p_profile_get));


        g_stacking_master[lchip]->p_fwd_profile[port_fwd_sel] = p_profile_get;

    }



    return ret;

}


STATIC int32
_sys_usw_stacking_wb_restore_glb_port(uint8 lchip)
{
    int32 ret                      = 0;
    uint32 cmd                     = 0;
    uint16 max                     = 0;
    uint16 min                     = 0;
    uint32 base                    = 0;
	uint8  rchip                   = 0;
	sys_usw_opf_t  opf;
	DsGlbDestPortMap_m port_map;
    uint8 gchip_id;

    for (rchip = 0; rchip < SYS_STK_MAX_GCHIP; rchip++)
    {
        sys_usw_get_gchip_id(lchip, &gchip_id);
        if(rchip == gchip_id)
        {
            continue;
        }

        cmd = DRV_IOR(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, rchip, cmd, &port_map));

        base   = GetDsGlbDestPortMap(V, glbDestPortBase_f, &port_map);
        max    = GetDsGlbDestPortMap(V, glbDestPortMax_f, &port_map);
        min    = GetDsGlbDestPortMap(V, glbDestPortMin_f, &port_map);

        if (min == 1 && max == 0)
        {
            continue;
        }

        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type  = g_stacking_master[lchip]->opf_type;
        opf.pool_index = SYS_STK_OPF_ABILITY;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, max + 1, base));

    }

    return ret;
}


int32
_sys_usw_stacking_wb_traverse_sync_trunk_node(void *data,uint32 vec_index, void *user_data)
{
    int32 ret = 0;
    sys_stacking_trunk_info_t *p_sys_trunk = (sys_stacking_trunk_info_t*)data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t*)user_data;
    sys_wb_stacking_trunk_t   *p_wb_trunk_node = NULL;
    uint16 max_buffer_cnt = wb_data->buffer_len / (wb_data->key_len + wb_data->data_len);

    p_wb_trunk_node = (sys_wb_stacking_trunk_t *)wb_data->buffer + wb_data->valid_cnt;

    p_wb_trunk_node->trunk_id = vec_index;
    p_wb_trunk_node->max_mem_cnt = p_sys_trunk->max_mem_cnt;
    p_wb_trunk_node->replace_en = p_sys_trunk->replace_en;
    p_wb_trunk_node->mode = p_sys_trunk->mode;
    p_wb_trunk_node->encap_hdr.hdr_flag = p_sys_trunk->encap_hdr.hdr_flag;
    p_wb_trunk_node->encap_hdr.vlan_valid = p_sys_trunk->encap_hdr.vlan_valid;
    p_wb_trunk_node->encap_hdr.vlan_id = p_sys_trunk->encap_hdr.vlan_id;
    sal_memcpy(p_wb_trunk_node->encap_hdr.mac_da, p_sys_trunk->encap_hdr.mac_da, sizeof(mac_addr_t));
    sal_memcpy(p_wb_trunk_node->encap_hdr.mac_sa, p_sys_trunk->encap_hdr.mac_sa, sizeof(mac_addr_t));
    sal_memcpy(&p_wb_trunk_node->encap_hdr.ipda, &p_sys_trunk->encap_hdr.ipda, sizeof(ipv6_addr_t));

    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

extern int32
sys_usw_nh_offset_alloc_from_position(uint8 lchip, sys_nh_entry_table_type_t entry_type,
                                            uint32 entry_num, uint32 start_offset);

int32
_sys_usw_stacking_wb_mapping_mcast(uint8 lchip, sys_wb_stacking_mcast_db_t *p_wb_mcast_info, sys_stacking_mcast_db_t *p_mcast_info, uint8 sync)
{

    if (sync)
    {
        p_wb_mcast_info->type = p_mcast_info->type;
        p_wb_mcast_info->id = p_mcast_info->id;
        p_wb_mcast_info->head_met_offset = p_mcast_info->head_met_offset;
        p_wb_mcast_info->tail_met_offset = p_mcast_info->tail_met_offset;
        p_wb_mcast_info->append_en = p_mcast_info->append_en;
        p_wb_mcast_info->alloc_id = p_mcast_info->alloc_id;
        p_wb_mcast_info->last_tail_offset = p_mcast_info->last_tail_offset;
        sal_memcpy(p_wb_mcast_info->trunk_bitmap, p_mcast_info->trunk_bitmap, sizeof(uint32)*CTC_STK_TRUNK_BMP_NUM);
        sal_memcpy(p_wb_mcast_info->port_bitmap, p_mcast_info->port_bitmap, sizeof(ctc_port_bitmap_t));
        p_wb_mcast_info->cpu_port = p_mcast_info->cpu_port;
    }
    else
    {

        p_mcast_info->type = p_wb_mcast_info->type;
        p_mcast_info->id = p_wb_mcast_info->id;
        p_mcast_info->head_met_offset = p_wb_mcast_info->head_met_offset;
        p_mcast_info->tail_met_offset = p_wb_mcast_info->tail_met_offset;
        p_mcast_info->append_en = p_wb_mcast_info->append_en;
        p_mcast_info->alloc_id = p_wb_mcast_info->alloc_id;
        p_mcast_info->last_tail_offset = p_wb_mcast_info->last_tail_offset;
        sal_memcpy(p_mcast_info->trunk_bitmap, p_wb_mcast_info->trunk_bitmap, sizeof(uint32)*SYS_WB_STK_TRUNK_BMP_NUM);
        sal_memcpy(p_mcast_info->port_bitmap, p_wb_mcast_info->port_bitmap, sizeof(ctc_port_bitmap_t));
        p_mcast_info->cpu_port = p_wb_mcast_info->cpu_port;
    }

    return CTC_E_NONE;
}


int32
_sys_usw_stacking_wb_sync_mcast_func(sys_stacking_mcast_db_t *p_mcast_info, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_stacking_mcast_db_t  *p_wb_mcast_info;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    uint8 lchip = (uint8)(data->value1);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_mcast_info = (sys_wb_stacking_mcast_db_t *)wb_data->buffer + wb_data->valid_cnt;

    CTC_ERROR_RETURN(_sys_usw_stacking_wb_mapping_mcast(lchip, p_wb_mcast_info, p_mcast_info, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_stacking_recover_mcast_by_wb(uint8 lchip, sys_wb_stacking_mcast_db_t* p_wb_mcast)
{
    uint32 loop = 0;
    uint8  gchip;

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    if(p_wb_mcast->type)
    {
        ctc_stacking_keeplive_t keep_live;
        if(p_wb_mcast->id != 0)
        {
            CTC_ERROR_RETURN(sys_usw_stacking_create_keeplive_group(lchip, p_wb_mcast->id));
        }
        sal_memset(&keep_live, 0, sizeof(keep_live));
        keep_live.group_id = p_wb_mcast->id;

        keep_live.mem_type = CTC_STK_KEEPLIVE_MEMBER_PORT;
        for(loop=0; loop<MAX_PORT_NUM_PER_CHIP; loop++)
        {
            if(CTC_BMP_ISSET(p_wb_mcast->port_bitmap, loop))
            {
                keep_live.cpu_gport = CTC_MAP_LPORT_TO_GPORT(gchip, loop);
                CTC_ERROR_RETURN(sys_usw_stacking_keeplive_add_member(lchip, &keep_live));
            }
        }

        keep_live.mem_type = CTC_STK_KEEPLIVE_MEMBER_TRUNK;
        for(loop=0; loop<=CTC_STK_MAX_TRUNK_ID; loop++)
        {
            if(CTC_BMP_ISSET(p_wb_mcast->trunk_bitmap, loop))
            {
                keep_live.trunk_id = loop;
                CTC_ERROR_RETURN(sys_usw_stacking_keeplive_add_member(lchip, &keep_live));
            }
        }
        keep_live.mem_type = CTC_STK_KEEPLIVE_MEMBER_PORT;
        if(p_wb_mcast->cpu_port)
        {
            keep_live.cpu_gport = p_wb_mcast->cpu_port;
            CTC_ERROR_RETURN(sys_usw_stacking_keeplive_add_member(lchip, &keep_live));
        }
    }
    else
    {
        ctc_stacking_trunk_mcast_profile_t mcast_profile;
        sal_memset(&mcast_profile, 0, sizeof(mcast_profile));

        /*default mcast profile has been create, only need recover member*/
        if(p_wb_mcast->id != 0)
        {
            mcast_profile.mcast_profile_id = (p_wb_mcast->alloc_id ? 0 : p_wb_mcast->id);
            mcast_profile.append_en = p_wb_mcast->append_en;
            mcast_profile.type = CTC_STK_MCAST_PROFILE_CREATE;
            CTC_ERROR_RETURN(sys_usw_stacking_set_trunk_mcast_profile(lchip, &mcast_profile));
        }

        mcast_profile.type = CTC_STK_MCAST_PROFILE_ADD;
        sal_memcpy(mcast_profile.trunk_bitmap, p_wb_mcast->trunk_bitmap, sizeof(uint32)*SYS_WB_STK_TRUNK_BMP_NUM);
        CTC_ERROR_RETURN(sys_usw_stacking_set_trunk_mcast_profile(lchip, &mcast_profile));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_stacking_wb_restore_mcast_func(uint8 lchip, sys_wb_stacking_mcast_db_t* p_wb_stacking_mcast)
{
    sys_stacking_mcast_db_t* p_sys_mcast = NULL;
    int32 ret = 0;
    DsMetEntry3W_m  met_entry;
    uint32 cmd = DRV_IOR(DsMetEntry3W_t, DRV_ENTRY_FLAG);
    uint16 next_met_offset;

    p_sys_mcast = mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_mcast_db_t));
    if (NULL == p_sys_mcast)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_sys_mcast, 0, sizeof(sys_stacking_mcast_db_t));

    ret = _sys_usw_stacking_wb_mapping_mcast(lchip, p_wb_stacking_mcast, p_sys_mcast, 0);
    if (ret)
    {
        mem_free(p_sys_mcast);
        return CTC_E_NO_MEMORY;
    }
    /*mcast profile alloc id*/
    if(p_sys_mcast->alloc_id)
    {
        sys_usw_opf_t  opf;
        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type  = g_stacking_master[lchip]->opf_type;
        opf.pool_index = SYS_STK_OPF_MCAST_PROFILE;
        sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_mcast->id);
    }

    /*mcast group or keep alive group alloc head node*/
    if(p_sys_mcast->type)
    {
        CTC_ERROR_GOTO(sys_usw_nh_set_glb_met_offset(lchip, p_sys_mcast->id, 1, TRUE), ret, error0);
    }
    else
    {
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_MET, 1, p_sys_mcast->head_met_offset), ret, error0);
    }

    /*mcast group or keep alive group alloc middle node*/
    DRV_IOCTL(lchip, p_sys_mcast->head_met_offset, cmd, &met_entry);
    next_met_offset = GetDsMetEntry3W(V, nextMetEntryPtr_f,  &met_entry);

    while(next_met_offset != p_sys_mcast->last_tail_offset)
    {
        sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_MET, 1, next_met_offset);

        DRV_IOCTL(lchip, next_met_offset, cmd, &met_entry);
        next_met_offset = GetDsMetEntry3W(V, nextMetEntryPtr_f,  &met_entry);
    }


    if(NULL == ctc_hash_insert(g_stacking_master[lchip]->mcast_hash, p_sys_mcast))
    {
        mem_free(p_sys_mcast);
        return CTC_E_NO_MEMORY;
    }
    if(p_sys_mcast->id == 0 && p_sys_mcast->type == 0)
    {
        g_stacking_master[lchip]->p_default_prof_db = p_sys_mcast;
    }

    return CTC_E_NONE;

error0:
    mem_free(p_sys_mcast);
    return ret;
}

int32
sys_usw_stacking_wb_sync(uint8 lchip)
{
    int32 ret                      = CTC_E_NONE;
    uint8 work_status = 0;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_stacking_master_t *p_wb_stacking_master;

    wb_data.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_STK_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);


    /*Sync master*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_stacking_master_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_MASTER);
    p_wb_stacking_master = (sys_wb_stacking_master_t  *)wb_data.buffer;

    p_wb_stacking_master->lchip = lchip;
    p_wb_stacking_master->stacking_mcast_offset = g_stacking_master[lchip]->stacking_mcast_offset;
    p_wb_stacking_master->binding_trunk = g_stacking_master[lchip]->binding_trunk;
    p_wb_stacking_master->bind_mcast_en = g_stacking_master[lchip]->bind_mcast_en;
    p_wb_stacking_master->version = SYS_WB_VERSION_STACKING;
    sal_memcpy(&p_wb_stacking_master->ing_port_en_bmp, &g_stacking_master[lchip]->ing_port_en_bmp, sizeof(ctc_port_bitmap_t));
    sal_memcpy(&p_wb_stacking_master->egr_port_en_bmp, &g_stacking_master[lchip]->egr_port_en_bmp, sizeof(ctc_port_bitmap_t));

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);


    /*Sync trunk*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_stacking_trunk_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_TRUNK);
        ctc_vector_traverse2(g_stacking_master[lchip]->p_trunk_vec,0,_sys_usw_stacking_wb_traverse_sync_trunk_node,&wb_data);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*Sync mcast_hash*/
    CTC_ERROR_GOTO(sys_usw_ftm_get_working_status(lchip, &work_status), ret, done);
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_stacking_mcast_db_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_MCAST_DB);
    user_data.data = &wb_data;
    user_data.value1 = lchip;
    user_data.value2 = (work_status == CTC_FTM_MEM_CHANGE_RECOVER) ? 1:0;
    CTC_ERROR_GOTO(ctc_hash_traverse(g_stacking_master[lchip]->mcast_hash, (hash_traversal_fn) _sys_usw_stacking_wb_sync_mcast_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }



done:
    if (wb_data.buffer )
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32 sys_usw_stacking_wb_restore(uint8 lchip)
{
    int32 ret = 0;
    uint32 entry_cnt = 0;
    uint32 cmd = 0;
    uint8  work_status = 0;

    ctc_wb_query_t    wb_query;
    sys_stacking_trunk_info_t* p_sys_trunk = NULL;
    sys_wb_stacking_master_t   wb_stacking_master;
    sys_wb_stacking_master_t*  p_wb_stacking_master = &wb_stacking_master;
    sys_wb_stacking_trunk_t    wb_stacking_trunk;
    sys_wb_stacking_trunk_t*   p_wb_stacking_trunk = &wb_stacking_trunk;
    sys_wb_stacking_mcast_db_t  wb_stacking_mcast;
    sys_wb_stacking_mcast_db_t* p_wb_stacking_mcast = &wb_stacking_mcast;

    DsLinkAggregateChannelGroup_m  aggregate_group;
    uint32 mem_base = 0;
    sys_usw_opf_t  opf;

    wb_query.buffer = mem_malloc(MEM_STK_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_STK_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    sys_usw_ftm_get_working_status(lchip, &work_status);
    /*Restore Master*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_stacking_master_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_MASTER);
    sal_memset(p_wb_stacking_master, 0, sizeof(sys_wb_stacking_master_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_stacking_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_STACKING, p_wb_stacking_master->version))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto done;
        }

        if(work_status != 3)
        {
            g_stacking_master[lchip]->stacking_mcast_offset = p_wb_stacking_master->stacking_mcast_offset;
        }
        g_stacking_master[lchip]->binding_trunk = p_wb_stacking_master->binding_trunk;
        g_stacking_master[lchip]->bind_mcast_en = p_wb_stacking_master->bind_mcast_en;
    sal_memcpy(&g_stacking_master[lchip]->ing_port_en_bmp, &p_wb_stacking_master->ing_port_en_bmp, sizeof(ctc_port_bitmap_t));
    sal_memcpy(&g_stacking_master[lchip]->egr_port_en_bmp, &p_wb_stacking_master->egr_port_en_bmp, sizeof(ctc_port_bitmap_t));
    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*Restore trunk*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_stacking_trunk_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_TRUNK);
    sal_memset(p_wb_stacking_trunk, 0, sizeof(sys_wb_stacking_trunk_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_stacking_trunk, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_sys_trunk = (sys_stacking_trunk_info_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_trunk_info_t));
        if (NULL == p_sys_trunk)
        {
            ret =  CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_sys_trunk, 0, sizeof(sys_stacking_trunk_info_t));

        p_sys_trunk->trunk_id = p_wb_stacking_trunk->trunk_id;
        p_sys_trunk->max_mem_cnt = p_wb_stacking_trunk->max_mem_cnt;
        p_sys_trunk->replace_en = p_wb_stacking_trunk->replace_en;
        p_sys_trunk->mode = p_wb_stacking_trunk->mode;

        p_sys_trunk->encap_hdr.hdr_flag = p_wb_stacking_trunk->encap_hdr.hdr_flag;
        p_sys_trunk->encap_hdr.vlan_valid = p_wb_stacking_trunk->encap_hdr.vlan_valid;
        p_sys_trunk->encap_hdr.vlan_id = p_wb_stacking_trunk->encap_hdr.vlan_id;
        sal_memcpy(p_sys_trunk->encap_hdr.mac_da, p_wb_stacking_trunk->encap_hdr.mac_da, sizeof(mac_addr_t));
        sal_memcpy(p_sys_trunk->encap_hdr.mac_sa, p_wb_stacking_trunk->encap_hdr.mac_sa, sizeof(mac_addr_t));
        sal_memcpy(&p_sys_trunk->encap_hdr.ipda, &p_wb_stacking_trunk->encap_hdr.ipda, sizeof(ipv6_addr_t));

        if (2 == g_stacking_master[lchip]->trunk_mode)
        {
            sal_memset(&aggregate_group, 0, sizeof(aggregate_group));
            cmd = DRV_IOR(DsLinkAggregateChannelGroup_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_trunk->trunk_id, cmd, &aggregate_group));
            mem_base = GetDsLinkAggregateChannelGroup(V, channelLinkAggMemBase_f, &aggregate_group);

            sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
            opf.pool_type  = g_stacking_master[lchip]->opf_type;
            opf.pool_index = SYS_STK_OPF_MEM_BASE;
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_sys_trunk->max_mem_cnt, mem_base));
        }

        ctc_vector_add(g_stacking_master[lchip]->p_trunk_vec,  p_sys_trunk->trunk_id, p_sys_trunk);

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*Restore  mcast*/
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_stacking_mcast_db_t, CTC_FEATURE_STACKING, SYS_WB_APPID_STACKING_SUBID_MCAST_DB);
    sal_memset(p_wb_stacking_mcast, 0, sizeof(sys_wb_stacking_mcast_db_t));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_stacking_mcast, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
        {
            drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
            ret = _sys_usw_stacking_recover_mcast_by_wb(lchip, p_wb_stacking_mcast);
            drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
            if(ret)
            {
                goto done;
            }
        }
        else
        {
            CTC_ERROR_GOTO(_sys_usw_stacking_wb_restore_mcast_func(lchip, p_wb_stacking_mcast), ret, done);
        }
    CTC_WB_QUERY_ENTRY_END((&wb_query));


   /********************************
   Restore from hardware
   **********************************/
   CTC_ERROR_GOTO(_sys_usw_stacking_wb_restore_block_profile(lchip), ret, done);
   CTC_ERROR_GOTO(_sys_usw_stacking_wb_restore_fwd_profile(lchip), ret, done);
   CTC_ERROR_GOTO(_sys_usw_stacking_wb_restore_glb_port(lchip), ret, done);


done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

   return ret;
}

STATIC int32
_sys_usw_stacking_traverse_fprintf_pool(void* node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    uint32 j = 0;
    uint32 type = *((uint32 *)(user_date->value2));

    if (type == 0)
    {
        sys_stacking_block_profile_t*   p_block_db =  NULL;
        p_block_db = (sys_stacking_block_profile_t*)((ctc_spool_node_t*)node)->data;
        if(((ctc_spool_node_t*)node)->ref_cnt == 0xFFFFFFFF)
        {
            SYS_DUMP_DB_LOG(p_file, "%-10s:%-5ustatic\n","Node", *cnt);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_file, "%-10s:%-5u,     %-10s:%-5u\n","Node", *cnt, "refcnt", ((ctc_spool_node_t*)node)->ref_cnt);
        }
        SYS_DUMP_DB_LOG(p_file, "%-10s\n","----------------------------------------");
        SYS_DUMP_DB_LOG(p_file, "%-20s:%u\n","profile_id", p_block_db->profile_id);
        for (j = 0; j < SYS_STK_MAX_GCHIP; j++)
        {
            SYS_DUMP_DB_LOG(p_file, "[%04x,%04x] ", p_block_db->bitmap[j][0], p_block_db->bitmap[j][1]);
            if (((j+1)%16) == 0)
            {
                SYS_DUMP_DB_LOG(p_file, "\n");
            }
        }
    }
    else
    {
        sys_stacking_fwd_profile_t*   p_fwd_db =  NULL;

        p_fwd_db = (sys_stacking_fwd_profile_t*)((ctc_spool_node_t*)node)->data;
        if(((ctc_spool_node_t*)node)->ref_cnt == 0xFFFFFFFF)
        {
            SYS_DUMP_DB_LOG(p_file, "%-10s:%-5ustatic\n","Node", *cnt);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_file, "%-10s:%-5u,     %-10s:%-5u\n","Node", *cnt, "refcnt", ((ctc_spool_node_t*)node)->ref_cnt);
        }
        SYS_DUMP_DB_LOG(p_file, "%-10s\n","----------------------------------------");
        SYS_DUMP_DB_LOG(p_file, "%-20s:%u\n","profile_id", p_fwd_db->profile_id);
        for (j = 0; j < SYS_STK_MAX_GCHIP; j++)
        {
            SYS_DUMP_DB_LOG(p_file, "[%02u] ", p_fwd_db->trunk[j]);
            if (((j+1)%16) == 0)
            {
                SYS_DUMP_DB_LOG(p_file, "\n");
            }
        }
    }

    return 0;
}

int32
sys_usw_stacking_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 i = 0;
    uint32 j = 0;
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 0;
    uint32 spool_type = 0;
    uint16 lport = 0;
    uint8  gchip = 0;

    SYS_STK_INIT_CHECK(lchip);
    STACKING_LOCK;

    sys_usw_get_gchip_id(lchip, &gchip);

    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Stacking");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","stacking_en",          g_stacking_master[lchip]->stacking_en);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","fabric_mode",          g_stacking_master[lchip]->fabric_mode);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","trunk_mode",           g_stacking_master[lchip]->trunk_mode);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","stacking_mcast_offset",g_stacking_master[lchip]->stacking_mcast_offset);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","mcast_mode",           g_stacking_master[lchip]->mcast_mode);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","binding_trunk",        g_stacking_master[lchip]->binding_trunk);
    SYS_DUMP_DB_LOG(p_f, "%-35s:%u\n","bind_mcast_en",        g_stacking_master[lchip]->bind_mcast_en);
    SYS_DUMP_DB_LOG(p_f, "  %-35s\n","p_block_profile");
    for (i = 0; i < SYS_STK_PORT_BLOCK_PROFILE_NUM; i++)
    {
        if (!g_stacking_master[lchip]->p_block_profile[i])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "  %-35s:%u\n","profile_id", g_stacking_master[lchip]->p_block_profile[i]->profile_id);
        SYS_DUMP_DB_LOG(p_f, "  %-35s:\n","bitmap");
        for (j = 0; j < SYS_STK_MAX_GCHIP; j++)
        {
            SYS_DUMP_DB_LOG(p_f, "    [%-4x,%-4x] ", g_stacking_master[lchip]->p_block_profile[i]->bitmap[j][0],
                g_stacking_master[lchip]->p_block_profile[i]->bitmap[j][1]);
            if (((j+1)%16) == 0)
            {
                SYS_DUMP_DB_LOG(p_f, "\n");
            }
        }
    }
    SYS_DUMP_DB_LOG(p_f, "  %-35s\n","p_fwd_profile");
    for (i = 0; i < SYS_STK_PORT_FWD_PROFILE_NUM; i++)
    {
        if (!g_stacking_master[lchip]->p_fwd_profile[i])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "  %-35s:%u\n","profile_id", g_stacking_master[lchip]->p_fwd_profile[i]->profile_id);
        SYS_DUMP_DB_LOG(p_f, "  %-35s:\n","trunk");
        for (j = 0; j < SYS_STK_MAX_GCHIP; j++)
        {
            SYS_DUMP_DB_LOG(p_f, "    [%-2u] ", g_stacking_master[lchip]->p_fwd_profile[i]->trunk[j]);
            if (((j+1)%16) == 0)
            {
                SYS_DUMP_DB_LOG(p_f, "\n");
            }
        }
    }

    SYS_DUMP_DB_LOG(p_f, "%-35s\n","Ingress port enable");
    SYS_DUMP_DB_LOG(p_f, "%-35s\n","Gport:");
    for (i = 0; i < sizeof(ctc_port_bitmap_t) / sizeof(uint32); i++)
    {
        for (j = 0; j < CTC_UINT32_BITS; j++)
        {
            lport = i * CTC_UINT32_BITS + j;
            if (CTC_BMP_ISSET(g_stacking_master[lchip]->ing_port_en_bmp, lport))
            {
                SYS_DUMP_DB_LOG(p_f, "    [0x%.4x] ", CTC_MAP_LPORT_TO_GPORT(gchip, lport));
            }
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-35s\n","Egress port enable");
    SYS_DUMP_DB_LOG(p_f, "%-35s\n","Gport:");
    for (i = 0; i < sizeof(ctc_port_bitmap_t) / sizeof(uint32); i++)
    {
        for (j = 0; j < CTC_UINT32_BITS; j++)
        {
            lport = i * CTC_UINT32_BITS + j;
            if (CTC_BMP_ISSET(g_stacking_master[lchip]->egr_port_en_bmp, lport))
            {
                SYS_DUMP_DB_LOG(p_f, "    [0x%.4x] ", CTC_MAP_LPORT_TO_GPORT(gchip, lport));
            }
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, g_stacking_master[lchip]->opf_type, p_f);

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    cb_data.value1 = &num_cnt;
    cb_data.value2 = &spool_type;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","p_block_prof_spool");
    ctc_spool_traverse(g_stacking_master[lchip]->p_block_prof_spool, (spool_traversal_fn)_sys_usw_stacking_traverse_fprintf_pool , (void*)(&cb_data));

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    num_cnt = 0;
    spool_type = 1;
    cb_data.value0 = p_f;
    cb_data.value1 = &num_cnt;
    cb_data.value2 = &spool_type;
    SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","p_block_prof_spool");
    ctc_spool_traverse(g_stacking_master[lchip]->p_block_prof_spool, (spool_traversal_fn)_sys_usw_stacking_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    STACKING_UNLOCK;

    return ret;
}


int32
sys_usw_stacking_init(uint8 lchip, void* p_cfg)
{
    uint8 work_status = 0;
    int32 ret                      = CTC_E_NONE;
    ctc_spool_t spool;
	sys_usw_opf_t opf;
    ctc_stacking_glb_cfg_t* p_glb_cfg = (ctc_stacking_glb_cfg_t*)p_cfg;

    if (g_stacking_master[lchip])
    {
        return CTC_E_NONE;
    }

    g_stacking_master[lchip] = (sys_stacking_master_t*)mem_malloc(MEM_STK_MODULE, sizeof(sys_stacking_master_t));
    if (NULL == g_stacking_master[lchip])
    {
        ret = CTC_E_NO_MEMORY;
        goto ERROR_FREE_MEM;
    }

    sal_memset(g_stacking_master[lchip], 0, sizeof(sys_stacking_master_t));


    g_stacking_master[lchip]->p_trunk_vec = ctc_vector_init(SYS_STK_TRUNK_VEC_BLOCK_NUM, SYS_STK_TRUNK_VEC_BLOCK_SIZE);
    if (NULL == g_stacking_master[lchip]->p_trunk_vec)
    {
        ret = CTC_E_NO_MEMORY;
        goto ERROR_FREE_MEM;
    }

    g_stacking_master[lchip]->mcast_hash  = ctc_hash_create(
            16,
            16,
            (hash_key_fn)_sys_usw_stacking_mcast_hash_make,
            (hash_cmp_fn)_sys_usw_stacking_mcast_hash_cmp);

    if (NULL == g_stacking_master[lchip]->mcast_hash)
    {
        ret = CTC_E_NO_MEMORY;
        goto ERROR_FREE_MEM;
    }

    /* set register cb api */
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_STK_GET_STKHDR_LEN, sys_usw_stacking_get_stkhdr_len), ret, ERROR_FREE_MEM);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_STK_GET_RSV_TRUNK_NUM, sys_usw_stacking_get_rsv_trunk_number), ret, ERROR_FREE_MEM);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_STK_GET_MCAST_PROFILE_MET_OFFSET, sys_usw_stacking_get_mcast_profile_met_offset), ret, ERROR_FREE_MEM);

    /*init opf*/
    ret = sys_usw_opf_init(lchip, &g_stacking_master[lchip]->opf_type, SYS_STK_OPF_MAX, "opf-stacking-profile");
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

	sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
	opf.pool_type  = g_stacking_master[lchip]->opf_type;

	opf.pool_index = SYS_STK_OPF_MEM_BASE;
	ret = sys_usw_opf_init_offset(lchip, &opf, 0 , MCHIP_CAP(SYS_CAP_STK_TRUNK_MEMBERS));
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

	opf.pool_index = SYS_STK_OPF_BLOCK;
	ret = sys_usw_opf_init_offset(lchip, &opf, 1 , SYS_STK_PORT_BLOCK_PROFILE_NUM-1);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

	opf.pool_index = SYS_STK_OPF_FWD;
    ret = sys_usw_opf_init_offset(lchip, &opf, 1 , SYS_STK_PORT_FWD_PROFILE_NUM-1);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

    /*0 ~ 255 is reserved for local chip*/
	opf.pool_index = SYS_STK_OPF_ABILITY;
    ret = sys_usw_opf_init_offset(lchip, &opf, 256 , MCHIP_CAP(SYS_CAP_STK_GLB_DEST_PORT_NUM));
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

    opf.pool_index = SYS_STK_OPF_MCAST_PROFILE;
    ret = sys_usw_opf_init_offset(lchip, &opf, 1 , (256-1));
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }


    /*init block profile*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_STK_PORT_BLOCK_PROFILE_NUM;
    spool.max_count = SYS_STK_PORT_BLOCK_PROFILE_NUM;
    spool.user_data_size = sizeof(sys_stacking_block_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_stacking_block_profile_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_stacking_block_profile_hash_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_stacking_block_profile_index_alloc;
    spool.spool_free= (spool_free_fn)_sys_usw_stacking_block_profile_index_free;
    g_stacking_master[lchip]->p_block_prof_spool = ctc_spool_create(&spool);
    ret = _sys_usw_stacking_add_static_block_port_profile(lchip);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }
    /*init fwd profile*/
    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = SYS_STK_PORT_FWD_PROFILE_NUM;
    spool.max_count = SYS_STK_PORT_FWD_PROFILE_NUM;
    spool.user_data_size = sizeof(sys_stacking_fwd_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_stacking_fwd_profile_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_stacking_fwd_profile_hash_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_stacking_fwd_profile_index_alloc;
    spool.spool_free= (spool_free_fn)_sys_usw_stacking_fwd_profile_index_free;
    g_stacking_master[lchip]->p_fwd_prof_spool = ctc_spool_create(&spool);
    ret = _sys_usw_stacking_add_static_fwd_port_profile(lchip);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

    /*default mcast group & keeplive group restore from database*/
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER))
    {
        if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
        {
            drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
        }
        ret =  sys_usw_stacking_mcast_group_create(lchip, 0, 0, &g_stacking_master[lchip]->p_default_prof_db, 0);
        if (NULL == g_stacking_master[lchip]->p_default_prof_db)
        {
            goto ERROR_FREE_MEM;
        }
        g_stacking_master[lchip]->stacking_mcast_offset = g_stacking_master[lchip]->p_default_prof_db->head_met_offset;

        ret = sys_usw_stacking_create_keeplive_group(lchip, 0);
        if (CTC_E_NONE != ret)
        {
            goto ERROR_FREE_MEM;
        }
        if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
        {
            drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
        }
    }

    ret = _sys_usw_stacking_hdr_set(lchip, &p_glb_cfg->hdr_glb);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

    ret = _sys_usw_stacking_set_enable(lchip, TRUE, p_glb_cfg->fabric_mode, p_glb_cfg->version);
    if (CTC_E_NONE != ret)
    {
        goto ERROR_FREE_MEM;
    }

    ret = sal_mutex_create(&(g_stacking_master[lchip]->p_stacking_mutex));
    if (ret || !(g_stacking_master[lchip]->p_stacking_mutex))
    {
        goto ERROR_FREE_MEM;
    }

    g_stacking_master[lchip]->stacking_en = 1;
    g_stacking_master[lchip]->fabric_mode = p_glb_cfg->fabric_mode;
    g_stacking_master[lchip]->trunk_mode = p_glb_cfg->trunk_mode;
    g_stacking_master[lchip]->mcast_mode = p_glb_cfg->mcast_mode;
    g_stacking_master[lchip]->bind_mcast_en = 0;
    CTC_ERROR_RETURN(sys_usw_port_register_rchip_get_gport_idx_cb(lchip, sys_usw_stacking_map_gport));
    if(CTC_STACKING_VERSION_2_0 == p_glb_cfg->version)
    {
        uint8  gchip;
        uint16 lport;
        DsGlbDestPortMap_m glb_dest_port_map;
        uint32 cmd = DRV_IOW(DsGlbDestPortMap_t, DRV_ENTRY_FLAG);
        sal_memset(&glb_dest_port_map, 0, sizeof(glb_dest_port_map));
        SetDsGlbDestPortMap(V, glbDestPortBase_f, &glb_dest_port_map, 0);
        SetDsGlbDestPortMap(V, glbDestPortMin_f, &glb_dest_port_map, 0);
        SetDsGlbDestPortMap(V, glbDestPortMax_f, &glb_dest_port_map, 0xFF);

        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, gchip, cmd, &glb_dest_port_map));

         /*reinit port for stacking version 2.0 need config global dest port*/
        for(lport=0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
        {
            _sys_usw_port_set_glb_dest_port_by_dest_port(lchip, lport);
        }

    }

    /*warm boot*/
    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_STACKING, sys_usw_stacking_wb_sync));

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_stacking_wb_restore(lchip));
    }

    /* dump-db register */
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_STACKING, sys_usw_stacking_dump_db));


    return CTC_E_NONE;

ERROR_FREE_MEM:

    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
        drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    if (g_stacking_master[lchip]->stacking_mcast_offset)
    {
        sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_MET, 1, g_stacking_master[lchip]->stacking_mcast_offset);
    }

    if (g_stacking_master[lchip]->p_stacking_mutex)
    {
        sal_mutex_destroy(g_stacking_master[lchip]->p_stacking_mutex);
    }

    if (NULL != g_stacking_master[lchip])
    {
        mem_free(g_stacking_master[lchip]);
    }

    return ret;
}

STATIC int32
_sys_usw_stacking_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
sys_usw_stacking_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == g_stacking_master[lchip])
    {
        return CTC_E_NONE;
    }
    /*free trunk vector*/
    ctc_vector_traverse(g_stacking_master[lchip]->p_trunk_vec, (vector_traversal_fn)_sys_usw_stacking_free_node_data, NULL);
    ctc_vector_release(g_stacking_master[lchip]->p_trunk_vec);

    ctc_hash_traverse(g_stacking_master[lchip]->mcast_hash, (hash_traversal_fn)_sys_usw_stacking_free_node_data, NULL);
    ctc_hash_free(g_stacking_master[lchip]->mcast_hash);


    /*free spool data*/
    ctc_spool_free(g_stacking_master[lchip]->p_block_prof_spool);
    ctc_spool_free(g_stacking_master[lchip]->p_fwd_prof_spool);

    sys_usw_opf_deinit(lchip, g_stacking_master[lchip]->opf_type);

    sal_mutex_destroy(g_stacking_master[lchip]->p_stacking_mutex);
    mem_free(g_stacking_master[lchip]);

    return CTC_E_NONE;

}

#endif

