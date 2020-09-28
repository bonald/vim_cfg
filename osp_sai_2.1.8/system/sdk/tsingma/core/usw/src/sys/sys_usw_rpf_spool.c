#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_rpf_spool.c

 @date 2012-09-17

 @version v2.0

 The file contains all rpf share pool related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_debug.h"
#include "ctc_spool.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_ftm.h"
#include "sys_usw_l3if.h"
#include "sys_usw_rpf_spool.h"

#include "drv_api.h"

#define SYS_USW_MAX_RPF_PROFILE_NUM      512
struct sys_rpf_master_s
{
    uint8   opf_type_rpf;
    uint8   rsv;
    uint16  ipmc_rpf_profile_num;
    ctc_spool_t*      rpf_spool;
};
typedef struct sys_rpf_master_s sys_rpf_master_t;

struct sys_rpf_traverse_s
{
    uint8    is_ipmc;
    uint32   ref_cnt;
    uint8  profile_valid[SYS_USW_MAX_RPF_PROFILE_NUM];
};
typedef struct sys_rpf_traverse_s sys_rpf_traverse_t;



static sys_rpf_master_t* p_usw_rpf_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

STATIC int32
_sys_usw_rpf_sort_profile_with_check(uint8 lchip, sys_rpf_info_t* p_rpf_info)
{
    uint8   loop1 = 0;
    uint8   loop2 = 0;
    uint16  rpf_intf = 0;
    uint32  cmd = 0;
    uint32  rpf_chk_port = 0;
    uint16  rpf_intf_array[SYS_USW_MAX_RPF_IF_NUM];

    sal_memcpy(rpf_intf_array,p_rpf_info->rpf_intf,sizeof(rpf_intf_array));

    cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rpf_chk_port));
    /*get valid rpf_intf*/
    for (loop1 = 0; loop1 < p_rpf_info->rpf_intf_cnt; loop1++)
    {
        if (rpf_chk_port)
        {
            SYS_GLOBAL_PORT_CHECK(rpf_intf_array[loop1]);
            p_rpf_info->rpf_intf[loop2] = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(rpf_intf_array[loop1]);
        }
        else
        {
            if (rpf_intf_array[loop1] == SYS_L3IF_INVALID_L3IF_ID || rpf_intf_array[loop1] > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1))
            {
                SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [L3if] Invalid interface ID! \n");
                return CTC_E_BADID;
            }
            p_rpf_info->rpf_intf[loop2] = rpf_intf_array[loop1];
        }
        loop2++;
    }
    p_rpf_info->rpf_intf_cnt = loop2;

     /*Bubble sort*/
    for (loop1 = 0; loop1 < p_rpf_info->rpf_intf_cnt; loop1++)
    {
        for (loop2 = loop1 + 1; loop2 < p_rpf_info->rpf_intf_cnt; loop2++)
        {
            if (p_rpf_info->rpf_intf[loop1] > p_rpf_info->rpf_intf[loop2])
            {
                rpf_intf = p_rpf_info->rpf_intf[loop2];
                p_rpf_info->rpf_intf[loop2] = p_rpf_info->rpf_intf[loop1];
                p_rpf_info->rpf_intf[loop1] = rpf_intf;
            }
        }
    }
    /*Remove duplicate elements */
    loop1 =1;
    loop2 = 1;
    for (loop1 = 1; loop1 < p_rpf_info->rpf_intf_cnt; loop1++)
    {
        if (p_rpf_info->rpf_intf[loop2-1] != p_rpf_info->rpf_intf[loop1])
        {
            p_rpf_info->rpf_intf[loop2++] = p_rpf_info->rpf_intf[loop1];
        }
    }
    p_rpf_info->rpf_intf_cnt = p_rpf_info->rpf_intf_cnt ? loop2 : 0;

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_rpf_intf_map_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info, sys_rpf_profile_t *p_rpf_profile)
{
    uint8 loop = 0;

    for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM; loop++)
    {
        if (loop < p_rpf_info->rpf_intf_cnt)
        {
            SetDsRpf(V, array_0_rpfIfId_f + (loop*2),      &p_rpf_profile->rpf_intf, p_rpf_info->rpf_intf[loop]);
            SetDsRpf(V, array_0_rpfIfIdValid_f + (loop*2), &p_rpf_profile->rpf_intf, 1);
        }
        else
        {
            SetDsRpf(V, array_0_rpfIfIdValid_f + (loop*2), &p_rpf_profile->rpf_intf, 0);
        }
    }


    return CTC_E_NONE;
}
STATIC uint32
_sys_usw_rpf_make(sys_rpf_profile_t* p_rpf_profile)
{
     return ctc_hash_caculate(sizeof(p_rpf_profile->rpf_intf), (void*)&p_rpf_profile->rpf_intf);
}
/* IPUC and IPMC share rpf profile,but ipuc or ipmc has at least
256 (ipmc is 255) profile, total up to 511. */
STATIC bool
_sys_usw_rpf_compare(sys_rpf_profile_t* p_bucket, sys_rpf_profile_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (!sal_memcmp(&p_bucket->rpf_intf, &p_new->rpf_intf, sizeof(DsRpf_m))
		&& (p_bucket->is_ipmc == p_new->is_ipmc))
    {
        return TRUE;
    }

    return FALSE;
}
/*rpf profile minimal 256(ipmc is 255) profile,but total up to 511*/
STATIC int32
_sys_usw_rpf_alloc_index(sys_rpf_profile_t* p_bucket, uint8 *p_lchip)
{
    sys_usw_opf_t opf;
    uint32 value_32 = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = p_usw_rpf_master[*p_lchip]->opf_type_rpf;
    opf.pool_index = p_bucket->is_ipmc ? 0 : 1;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_bucket->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_bucket->profile_id = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_rpf_free_index(sys_rpf_profile_t* p_bucket, uint8 *p_lchip)
{
    sys_usw_opf_t opf;
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = p_usw_rpf_master[*p_lchip]->opf_type_rpf;
    opf.pool_index = p_bucket->is_ipmc ? 0 : 1;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, (uint32)p_bucket->profile_id));

    return CTC_E_NONE;
}

int32
sys_usw_rpf_remove_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info)
{
    int32 ret = 0;
    uint32 cmd = 0;
    sys_rpf_profile_t profile;

    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&profile, 0, sizeof(sys_rpf_profile_t));

    if(p_rpf_info->mode == SYS_RPF_CHK_MODE_IFID
		|| (p_rpf_info->mode == SYS_RPF_CHK_MODE_PROFILE && p_rpf_info->rpf_id == 0))
    {
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"),  "One Interface", p_rpf_info->rpf_id);
        return CTC_E_NONE;
    }

    cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rpf_info->rpf_id, cmd, (void*)&profile.rpf_intf));

    profile.profile_id = p_rpf_info->rpf_id;
    profile.is_ipmc =  (p_rpf_info->rpf_id < p_usw_rpf_master[lchip]->ipmc_rpf_profile_num);

    ret = ctc_spool_remove(p_usw_rpf_master[lchip]->rpf_spool, &profile, NULL);
    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"), "Profile",p_rpf_info->rpf_id);

    return ret;
}

int32
sys_usw_rpf_add_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    sys_rpf_profile_t  new_profile;
    sys_rpf_profile_t* out_profile = NULL;

    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rpf_info);
    sal_memset(&new_profile, 0, sizeof(sys_rpf_profile_t));

    CTC_ERROR_RETURN(_sys_usw_rpf_sort_profile_with_check(lchip, p_rpf_info));

    if(p_rpf_info->rpf_intf_cnt <= 1)
    {
        p_rpf_info->mode = SYS_RPF_CHK_MODE_IFID ;
        p_rpf_info->rpf_id = p_rpf_info->rpf_intf_cnt ? p_rpf_info->rpf_intf[0] :0xFFFF;
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"), "One Interface", p_rpf_info->rpf_id);

        return CTC_E_NONE;
    }
    else
    {
        p_rpf_info->mode = SYS_RPF_CHK_MODE_PROFILE;
    }

    _sys_usw_rpf_intf_map_profile(lchip, p_rpf_info, &new_profile);
    new_profile.is_ipmc = p_rpf_info->is_ipmc;

    ret = ctc_spool_add(p_usw_rpf_master[lchip]->rpf_spool, &new_profile, NULL, &out_profile);
    if(ret == CTC_E_NONE)
    {
        p_rpf_info->rpf_id = out_profile->profile_id;

        cmd = DRV_IOW(DsRpf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, out_profile->profile_id, cmd, (void*)&out_profile->rpf_intf));
    }
    else if (ret == CTC_E_NO_RESOURCE)
    {
        ret = CTC_E_PROFILE_NO_RESOURCE;
    }
    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"), "Profile", p_rpf_info->rpf_id);

    return ret;
}

int32
sys_usw_rpf_update_profile(uint8 lchip, sys_rpf_info_t* p_rpf_info)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;

    sys_rpf_profile_t  old_profile;
    sys_rpf_profile_t  new_profile;
    sys_rpf_profile_t* out_profile = NULL;
    uint8 old_use_profile = 0;

    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_rpf_info);
    sal_memset(&old_profile, 0, sizeof(sys_rpf_profile_t));
    sal_memset(&new_profile, 0, sizeof(sys_rpf_profile_t));

    CTC_ERROR_RETURN(_sys_usw_rpf_sort_profile_with_check(lchip, p_rpf_info));

    if((p_rpf_info->mode == SYS_RPF_CHK_MODE_PROFILE) && (p_rpf_info->rpf_id != 0))
    {
        cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rpf_info->rpf_id, cmd, &old_profile.rpf_intf));
        old_profile.profile_id = p_rpf_info->rpf_id;
        old_profile.is_ipmc =   (p_rpf_info->rpf_id < p_usw_rpf_master[lchip]->ipmc_rpf_profile_num);
        old_use_profile = 1;
    }

    if (p_rpf_info->rpf_intf_cnt <= 1)
    {
        if (old_use_profile)
        {
            /*remove old*/
            ctc_spool_remove(p_usw_rpf_master[lchip]->rpf_spool, &old_profile, &out_profile);
        }
        p_rpf_info->mode = SYS_RPF_CHK_MODE_IFID ;
        p_rpf_info->rpf_id = p_rpf_info->rpf_intf_cnt ? p_rpf_info->rpf_intf[0] :0xFFFF;
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"), "One Interface", p_rpf_info->rpf_id);

        return CTC_E_NONE;
    }
    else
    {
        p_rpf_info->mode = SYS_RPF_CHK_MODE_PROFILE;
    }

    _sys_usw_rpf_intf_map_profile(lchip, p_rpf_info, &new_profile);
    new_profile.is_ipmc = p_rpf_info->is_ipmc;
    ret = ctc_spool_add(p_usw_rpf_master[lchip]->rpf_spool, &new_profile, &old_profile, &out_profile);
    if(ret == CTC_E_NONE)
    {
        p_rpf_info->rpf_id = out_profile->profile_id;
        cmd = DRV_IOW(DsRpf_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, out_profile->profile_id, cmd, (void*)&out_profile->rpf_intf));
    }
    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s: RPF check mode:%s, rpf_id:%d\n", (p_rpf_info->is_ipmc ? "IPMC" : "IPUC"), "Profile",p_rpf_info->rpf_id);

    return ret;
}
int32
sys_usw_rpf_restore_profile(uint8 lchip, uint32 profile_id)
{
    uint32 cmd;
    sys_rpf_profile_t rpf_profile;

    sal_memset(&rpf_profile, 0, sizeof(sys_rpf_profile_t));

    rpf_profile.profile_id = profile_id;
    rpf_profile.is_ipmc = (profile_id < p_usw_rpf_master[lchip]->ipmc_rpf_profile_num);

    cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_id, cmd, &rpf_profile.rpf_intf));

    CTC_ERROR_RETURN(ctc_spool_add(p_usw_rpf_master[lchip]->rpf_spool, &rpf_profile, NULL, NULL));

    return CTC_E_NONE;
}



int32
sys_usw_rpf_profile_dump_node_status(ctc_spool_node_t* node, void* user_data)
{
    sys_rpf_traverse_t *p_rpf_traverse = (sys_rpf_traverse_t*)user_data;
    sys_rpf_profile_t *p_rpf_profile;

    if(node && node->data && (((sys_rpf_profile_t *)(node->data))->is_ipmc ==p_rpf_traverse->is_ipmc) )
    {
        p_rpf_profile = node->data;
        p_rpf_traverse->profile_valid[p_rpf_profile->profile_id] = 1;
        p_rpf_traverse->ref_cnt = node->ref_cnt;
    }

    return CTC_E_NONE;
}


int32
sys_usw_rpf_profile_dump_status(uint8 lchip, uint8 is_ipmc)
{
    sys_rpf_traverse_t rpf_traverse;
    uint16 loop,loop2;
    DsRpf_m  rpf_intf;
    uint16 rpf_id = 0;
    uint32 cmd = 0;
    uint8 is_not_first_loop = 0;
    uint16 profile_cnt = 0;

    if(NULL == p_usw_rpf_master[lchip])
    {
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    sal_memset(&rpf_traverse,0,sizeof(sys_rpf_traverse_t));
    rpf_traverse.is_ipmc = is_ipmc;

    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10s %-10s %s\n","Profile ID","RefCnt","RPF ID");
    SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"---------------------------------------------------\n");
    ctc_spool_traverse(p_usw_rpf_master[lchip]->rpf_spool, (spool_traversal_fn)sys_usw_rpf_profile_dump_node_status, (void*)&rpf_traverse);

    cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
    for (loop = 0; loop < SYS_USW_MAX_RPF_PROFILE_NUM; loop++)
    {
        if(!rpf_traverse.profile_valid[loop])
        {
            continue;
        }
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-10d  %-10d  ",loop,rpf_traverse.ref_cnt);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &rpf_intf));
        is_not_first_loop = 0;
        for (loop2 = 0; loop2 < SYS_USW_MAX_RPF_IF_NUM; loop2++)
        {
            if(!GetDsRpf(V, array_0_rpfIfIdValid_f + (loop2*2),&rpf_intf))
            {
                continue;
            }

            rpf_id = GetDsRpf(V, array_0_rpfIfId_f + (loop2*2),&rpf_intf);
            if(is_not_first_loop)
            {
                SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,",%d",rpf_id);
            }
            else
            {
                SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-d",rpf_id);
            }
            is_not_first_loop = 1;
        }
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n")
        profile_cnt++;
   }
   SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Total used profile: %d\n",profile_cnt);

   return CTC_E_NONE;

}

/* init the rpf module */
int32
sys_usw_rpf_init(uint8 lchip)
{
    uint32 entry_num = 0;

    sys_usw_opf_t opf;
    ctc_spool_t spool;

    sal_memset(&spool, 0, sizeof(ctc_spool_t));

    LCHIP_CHECK(lchip);
    if (NULL != p_usw_rpf_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_rpf_master[lchip] = mem_malloc(MEM_RPF_MODULE, sizeof(sys_rpf_master_t));
    if (NULL == p_usw_rpf_master[lchip])
    {
        SYS_RPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_rpf_master[lchip], 0, sizeof(sys_rpf_master_t));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsRpf_t, &entry_num));

    /* profile = 0 : reserved for sdk and used as invalid id */

    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_rpf_master[lchip]->opf_type_rpf, 2, "opf-rpf-prifile"));

    p_usw_rpf_master[lchip]->ipmc_rpf_profile_num = entry_num/2;
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = p_usw_rpf_master[lchip]->opf_type_rpf;
    opf.pool_index = 0;   /*ipmc*/
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, (p_usw_rpf_master[lchip]->ipmc_rpf_profile_num-1)));

    opf.pool_index = 1;   /*ipuc*/
    entry_num -= p_usw_rpf_master[lchip]->ipmc_rpf_profile_num;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, p_usw_rpf_master[lchip]->ipmc_rpf_profile_num, entry_num));

    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = entry_num;
    spool.max_count = entry_num;
    spool.user_data_size = sizeof(sys_rpf_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_rpf_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_rpf_compare;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_rpf_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_rpf_free_index;
    p_usw_rpf_master[lchip]->rpf_spool = ctc_spool_create(&spool);

    return CTC_E_NONE;
}

/* deinit the rpf module */
int32
sys_usw_rpf_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_rpf_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_opf_deinit(lchip, p_usw_rpf_master[lchip]->opf_type_rpf);

    ctc_spool_free(p_usw_rpf_master[lchip]->rpf_spool);

    mem_free(p_usw_rpf_master[lchip]);

    return CTC_E_NONE;
}

#endif

