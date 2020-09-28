/**
  @file sys_usw_nexthop_api.c

  @date 2009-11-11

  @version v2.0

  The file contains all nexthop API function for upper layer module
 */
/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_linklist.h"
#include "ctc_nexthop.h"
#include "ctc_packet.h"
#include "sys_usw_chip.h"
#include "sys_usw_aps.h"
#include "sys_usw_l3if.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_opf.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_common.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define SYS_NEXTHOP_PORT_HASH_MAX_BLK_NUM       64
#define SYS_NEXTHOP_PORT_HASH_BLK_SIZE       32

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
sys_usw_nh_api_master_t* p_usw_nh_api_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/

STATIC int32
_sys_usw_nh_brguc_port_hash_make(void* p_data)
{
    return ctc_hash_caculate(sizeof(uint32), p_data);
}

STATIC bool
_sys_usw_nh_brguc_port_hash_cmp(sys_hbnh_brguc_node_info_t* p_data0, sys_hbnh_brguc_node_info_t* p_data1)
{
    if (!p_data0 || !p_data1)
    {
        return TRUE;
    }

    if (p_data0->gport == p_data1->gport)
    {
        return TRUE;
    }

    return FALSE;
}
/**
 @brief This function is to initialize nexthop API data,
 */
int32
sys_usw_nh_api_init(uint8 lchip, ctc_nh_global_cfg_t* nh_cfg)
{

    int32    ret = 0;
    uint8 work_status = 0;
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
	}
    if (p_usw_nh_api_master[lchip])
    {
        return CTC_E_NONE;
    }

    p_usw_nh_api_master[lchip] = (sys_usw_nh_api_master_t*)mem_malloc(MEM_NEXTHOP_MODULE, (sizeof(sys_usw_nh_api_master_t)));
    sal_memset(p_usw_nh_api_master[lchip], 0, (sizeof(sys_usw_nh_api_master_t)));
    /*1. Bridge Ucast Data init*/
    if (NULL != p_usw_nh_api_master[lchip])
    {
        SYS_NH_CREAT_LOCK(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex);
        SYS_NH_CREAT_LOCK(p_usw_nh_api_master[lchip]->p_mutex);
    }
    else
    {
        mem_free(p_usw_nh_api_master[lchip]);
        p_usw_nh_api_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    p_usw_nh_api_master[lchip]->brguc_info.brguc_hash = ctc_hash_create(SYS_NEXTHOP_PORT_HASH_MAX_BLK_NUM,
                                                    SYS_NEXTHOP_PORT_HASH_BLK_SIZE,
                                                    (hash_key_fn)_sys_usw_nh_brguc_port_hash_make,
                                                    (hash_cmp_fn)_sys_usw_nh_brguc_port_hash_cmp);
    if (NULL == p_usw_nh_api_master[lchip]->brguc_info.brguc_hash)
    {
        mem_free(p_usw_nh_api_master[lchip]);
        p_usw_nh_api_master[lchip] = NULL;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    CTC_ERROR_RETURN(sys_usw_nh_init(lchip, nh_cfg));

    ret = sys_usw_nh_get_max_external_nhid(lchip, &p_usw_nh_api_master[lchip]->max_external_nhid);

    if (ret)
    {
        mem_free(p_usw_nh_api_master[lchip]);
        p_usw_nh_api_master[lchip] = NULL;
        return ret;
    }



    /*2. Create default nexthop*/
    if (!(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER))
    {
        /*wb reloading have recovered*/
        CTC_ERROR_RETURN(_sys_usw_misc_nh_create(lchip, SYS_NH_RESOLVED_NHID_FOR_DROP, SYS_NH_TYPE_DROP));
        CTC_ERROR_RETURN(_sys_usw_misc_nh_create(lchip, SYS_NH_RESOLVED_NHID_FOR_TOCPU, SYS_NH_TYPE_TOCPU));
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
	}
    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_api_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
sys_usw_nh_api_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_nh_api_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_nh_deinit(lchip);

    /*free bridge uc vector*/
    ctc_hash_traverse(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, (hash_traversal_fn)_sys_usw_nh_api_free_node_data, NULL);
    ctc_hash_free(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash);

    sal_mutex_destroy(p_usw_nh_api_master[lchip]->brguc_info.brguc_mutex);
    sal_mutex_destroy(p_usw_nh_api_master[lchip]->p_mutex);
    mem_free(p_usw_nh_api_master[lchip]);

    return CTC_E_NONE;
}

/**
 @brief This function is to create normal bridge nexthop

 @param[in] gport, global port id

 @param[in] nh_type, nexthop type

 @return CTC_E_XXX
 */
int32
sys_usw_brguc_nh_create(uint8 lchip, uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type)
{
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = 0x%x , nh_type = %d\n", gport, nh_type);

    SYS_NH_INIT_CHECK;
    SYS_GLOBAL_PORT_CHECK(gport);

    if (nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED
        || nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU
        || nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_SERVICE_QUEUE)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
		return CTC_E_NOT_SUPPORT;
    }

    SYS_NH_LOCK;

    if (((nh_type & CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC) != 0) || (nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL))
    {
        ret = (_sys_usw_brguc_nh_create_by_type(lchip, gport, SYS_NH_PARAM_BRGUC_SUB_TYPE_BASIC));
        if ((CTC_E_EXIST != ret) && (CTC_E_NONE != ret))
        {
            SYS_NH_UNLOCK;
            return ret;
        }
    }

    if (((nh_type & CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS) != 0) || (nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL))
    {
        ret = (_sys_usw_brguc_nh_create_by_type(lchip, gport, SYS_NH_PARAM_BRGUC_SUB_TYPE_BYPASS));
        if ((CTC_E_EXIST != ret) && (CTC_E_NONE != ret))
        {
            SYS_NH_UNLOCK;
            return ret;
        }
    }

    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to delete normal bridge nexthop

 @param[in] gport, global port id

 @param[in] nh_type, nexthop type

 @return CTC_E_XXX
 */
int32
sys_usw_brguc_nh_delete(uint8 lchip, uint32 gport)
{

    sys_hbnh_brguc_node_info_t* p_brguc_node;
    sys_hbnh_brguc_node_info_t brguc_node_tmp;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = %d\n", gport);

    SYS_NH_INIT_CHECK;
    SYS_GLOBAL_PORT_CHECK(gport);

    SYS_NH_BRGUC_LOCK;

    brguc_node_tmp.gport = gport;
    p_brguc_node = ctc_hash_lookup(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, &brguc_node_tmp);
    if (NULL == p_brguc_node)
    {
        SYS_NH_BRGUC_UNLOCK;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop not exist \n");
		return CTC_E_NOT_EXIST;
    }

    SYS_NH_LOCK;
    if (p_brguc_node->nhid_brguc != SYS_NH_INVALID_NHID)
    {
        ret = (sys_usw_nh_api_delete(lchip, p_brguc_node->nhid_brguc, SYS_NH_TYPE_BRGUC));
        p_brguc_node->nhid_brguc = SYS_NH_INVALID_NHID;

    }

    if (p_brguc_node->nhid_bypass != SYS_NH_INVALID_NHID)
    {
        ret = ret ? ret : (sys_usw_nh_api_delete(lchip, p_brguc_node->nhid_bypass, SYS_NH_TYPE_BRGUC));
        p_brguc_node->nhid_bypass = SYS_NH_INVALID_NHID;

    }
    SYS_NH_UNLOCK;

    if ((p_brguc_node->nhid_bypass == SYS_NH_INVALID_NHID)
        && (p_brguc_node->nhid_brguc == SYS_NH_INVALID_NHID))
    {
        ctc_hash_remove(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, p_brguc_node);
        mem_free(p_brguc_node);
    }

    SYS_NH_BRGUC_UNLOCK;
    return ret;
}

#define __IN_CALLBACK__
/*
  These function may used in nexthop cb function, and be called again, so do not using lock by in_cb is set to TRUE
*/
int32
sys_usw_nh_get_dsfwd_offset(uint8 lchip, uint32 nhid, uint32 *p_dsfwd_offset, uint8 in_cb)
{
    sys_nh_info_com_t* p_nhinfo;
    int32 ret = 0;

    /* get ds_fwd */
    SYS_NH_INIT_CHECK;

    if (!in_cb)
    {
        SYS_NH_LOCK;
    }

    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nhinfo), ret, error);

    if ((p_nhinfo->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_MCAST_PROFILE))
    {
        /*mcast profile do not support dsfwd*/
        ret = CTC_E_INVALID_PARAM;
        goto error;
    }

    if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
		&& (p_nhinfo->hdr.nh_entry_type != SYS_NH_TYPE_ECMP)
		&& !(p_nhinfo->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD))
    {
		sys_nh_param_com_t nh_param;
        uint8  lchip_start = lchip;
        uint8  lchip_end = lchip+1;
        uint8  lchip_tmp = 0;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

        sal_memset(&nh_param, 0, sizeof(sys_nh_param_com_t));
        nh_param.hdr.nhid = nhid;
        nh_param.hdr.nh_param_type = p_nhinfo->hdr.nh_entry_type;
        nh_param.hdr.change_type  = SYS_NH_CHANGE_TYPE_ADD_DYNTBL;

        CTC_ERROR_GOTO(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), ret, error);
        if (sys_usw_chip_get_rchain_en())
        {
            lchip_start = 0;
            lchip_end = sys_usw_get_local_chip_num();
            for(lchip_tmp = lchip_start; lchip_tmp < lchip_end; lchip_tmp++)
            {
                ret = sys_usw_nh_api_update(lchip_tmp, nhid, (sys_nh_param_com_t*)(&nh_param));
                if (ret && ret != CTC_E_NOT_EXIST)
                {
                    goto error;
                }
            }
        }
        CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nhinfo), ret, error);

        /*if nh have bind already, update old AD to use dsfwd, disable merge dsfwd mode*/
        CTC_ERROR_GOTO(sys_usw_nh_merge_dsfwd_disable(lchip, nhid), ret, error);
    }

    if (!(p_nhinfo->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_HAVE_DSFWD)
		&& !(p_nhinfo->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD) )
    {
        ret = CTC_E_INVALID_PARAM;
        goto error;
    }

    *p_dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
error:
    if(!in_cb)
    {
        SYS_NH_UNLOCK;
    }
    return ret;

}

int32
sys_usw_nh_get_aps_working_path(uint8 lchip, uint32 nhid, uint32* gport, uint32* nexthop_ptr, bool* p_protect_en)
{
    bool protect_en = 0;
    uint16 next_group_id = 0;
    sys_nh_info_dsnh_t sys_nh_info;

    SYS_NH_INIT_CHECK;

    sal_memset(&sys_nh_info, 0, sizeof(sys_nh_info));
    CTC_ERROR_RETURN(_sys_usw_nh_get_nhinfo(lchip, nhid, &sys_nh_info));
    if (NULL == p_protect_en)
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_protection(lchip, sys_nh_info.gport, &protect_en));
    }
    else
    {
        protect_en = *p_protect_en;
    }

    if (sys_nh_info.individual_nexthop && protect_en)/*two dsnexthop*/
    {
        *nexthop_ptr = sys_nh_info.dsnh_offset + 1*(1 + sys_nh_info.nexthop_ext);
    }
    CTC_ERROR_RETURN(sys_usw_aps_get_next_group(lchip, sys_nh_info.gport, &next_group_id, (!protect_en)));
    if (CTC_MAX_UINT16_VALUE == next_group_id)/*no nexthop aps group*/
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, sys_nh_info.gport, gport, (!protect_en)));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_aps_get_protection(lchip, next_group_id, &protect_en));
        CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, next_group_id, gport, (!protect_en)));
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_get_nhinfo(uint8 lchip, uint32 nhid, sys_nh_info_dsnh_t* p_nhinfo, uint8 in_cb)
{
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


    SYS_NH_INIT_CHECK;
    if(!in_cb)
    {
        SYS_NH_LOCK;
    }
    ret = _sys_usw_nh_get_nhinfo(lchip, nhid, p_nhinfo);
    if(!in_cb)
    {
        SYS_NH_UNLOCK;
    }
    return ret;
}

int32
sys_usw_l2_get_ucast_nh(uint8 lchip, uint32 gport, ctc_nh_param_brguc_sub_type_t nh_type, uint32* nhid)
{
    int32 ret = CTC_E_NONE;
    sys_hbnh_brguc_node_info_t* p_brguc_nhid_info = NULL;
    sys_hbnh_brguc_node_info_t brguc_node_tmp;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = %d\n", gport);

    SYS_NH_INIT_CHECK;
    SYS_GLOBAL_PORT_CHECK(gport);
    if (nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED
        || nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU
        || nh_type == CTC_NH_PARAM_BRGUC_SUB_TYPE_SERVICE_QUEUE)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
		return CTC_E_NOT_SUPPORT;
    }


    SYS_NH_BRGUC_LOCK;
    brguc_node_tmp.gport = gport;
    p_brguc_nhid_info = ctc_hash_lookup(p_usw_nh_api_master[lchip]->brguc_info.brguc_hash, &brguc_node_tmp);
    if (NULL == p_brguc_nhid_info)
    {
        SYS_NH_BRGUC_UNLOCK;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop not exist \n");
		return CTC_E_NOT_EXIST;
    }

    switch (nh_type)
    {
    case CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC:
        if (p_brguc_nhid_info->nhid_brguc != SYS_NH_INVALID_NHID)
        {
            *nhid = p_brguc_nhid_info->nhid_brguc;
        }
        else
        {
            ret  = CTC_E_NOT_EXIST;
        }

        break;

    case CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS:
        if (p_brguc_nhid_info->nhid_bypass != SYS_NH_INVALID_NHID)
        {
            *nhid = p_brguc_nhid_info->nhid_bypass;
        }
        else
        {
            ret  = CTC_E_NOT_EXIST;
        }

        break;

    default:
        SYS_NH_BRGUC_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    SYS_NH_BRGUC_UNLOCK;
    return ret;
}


#if 0
int32
sys_usw_nh_get_dsnh_offset_by_nhid(uint8 lchip, uint32 nhid, uint32 *p_dsnh_offset, uint8* p_use_dsnh8w)
{
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_dsnh_offset_by_nhid(lchip, nhid, p_dsnh_offset, p_use_dsnh8w), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}
#endif
int32
sys_usw_nh_get_mcast_member(uint8 lchip, uint32 nhid, ctc_nh_info_t* p_nh_info)
{
    sys_nh_info_com_t* p_com_db = NULL;
    sys_nh_info_mcast_t* p_mcast_db = NULL;
    sys_usw_nh_master_t* p_nh_master = NULL;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "\n%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    CTC_ERROR_GOTO(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), ret, error);
    CTC_ERROR_GOTO(
        _sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_com_db), ret, error);

    p_mcast_db = (sys_nh_info_mcast_t*)p_com_db;

    CTC_ERROR_GOTO(
        sys_usw_nh_get_mcast_member_info(lchip, p_mcast_db, p_nh_info), ret, error);

error:
    SYS_NH_UNLOCK;

    return ret;
}

/**
 @brief This function is to create egress vlan edit nexthop

 @param[in] nhid, nexthop id of this nexthop

 @param[in] gport, global port id

 @param[in] p_vlan_info, vlan edit information

 @param[in] dsnh_offset, nexthop offset of this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_egress_vlan_edit_nh_create(uint8 lchip, uint32 nhid, uint32 gport,
                                         ctc_vlan_egress_edit_info_t* p_vlan_info, uint32 dsnh_offset,
                                         ctc_vlan_edit_nh_param_t* p_nh_param)
{
    sys_nh_param_brguc_t nh_brg;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, gport = %d, dsnh_offset = %d",
                   nhid, gport, dsnh_offset);

    SYS_NH_INIT_CHECK;
    SYS_GLOBAL_PORT_CHECK(gport);
    CTC_PTR_VALID_CHECK(p_vlan_info);
    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info->output_cvid);
    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info->output_svid);
    }
    if(CTC_FLAG_ISSET(p_vlan_info->flag, CTC_VLAN_NH_MTU_CHECK_EN))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_info->mtu_size, SYS_MTU_MAX_SIZE);
    }

    SYS_USW_CID_CHECK(lchip, p_nh_param->cid);

    CTC_MAX_VALUE_CHECK(p_vlan_info->user_vlanptr,0x1fff);

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, dsnh_offset));
    sal_memset(&nh_brg, 0, sizeof(sys_nh_param_brguc_t));

    SYS_NH_LOCK;
    nh_brg.hdr.nh_param_type = SYS_NH_TYPE_BRGUC;
    nh_brg.hdr.is_internal_nh = FALSE;
    nh_brg.hdr.nhid = nhid;
    nh_brg.nh_sub_type = SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT;
    nh_brg.gport = gport;
    nh_brg.p_vlan_edit_info = p_vlan_info;
    nh_brg.dsnh_offset = dsnh_offset;
    nh_brg.loop_nhid = p_vlan_info->loop_nhid;
    nh_brg.p_vlan_edit_nh_param = p_nh_param;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_brg)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to delete egress vlan edit nexthop

 @param[in] nhid, nexthop id of this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_egress_vlan_edit_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_BRGUC), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to create egress vlan edit nexthop

 @param[in] nhid, nexthop id of this nexthop

 @param[in] gport, global port id

 @param[in] p_vlan_info, vlan edit information

 @param[in] dsnh_offset, nexthop offset of this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_egress_vlan_edit_nh_update(uint8 lchip, uint32 nhid,
                                         ctc_vlan_egress_edit_info_t* p_vlan_info,
                                         ctc_vlan_edit_nh_param_t* p_nh_param)
{
    sys_nh_param_brguc_t nh_brg;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_vlan_info);
    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info->output_cvid);
    }

    if (CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info->output_svid);
    }
    if(CTC_FLAG_ISSET(p_vlan_info->flag, CTC_VLAN_NH_MTU_CHECK_EN))
    {
        CTC_MAX_VALUE_CHECK(p_vlan_info->mtu_size, SYS_MTU_MAX_SIZE);
    }

    SYS_USW_CID_CHECK(lchip, p_nh_param->cid);

    CTC_MAX_VALUE_CHECK(p_vlan_info->user_vlanptr,0x1fff);

    sal_memset(&nh_brg, 0, sizeof(sys_nh_param_brguc_t));

    SYS_NH_LOCK;
    nh_brg.hdr.nh_param_type = SYS_NH_TYPE_BRGUC;
    nh_brg.hdr.is_internal_nh = FALSE;
    nh_brg.nh_sub_type = SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT;
    nh_brg.p_vlan_edit_info = p_vlan_info;
    nh_brg.loop_nhid = p_vlan_info->loop_nhid;
    nh_brg.p_vlan_edit_nh_param = p_nh_param;
    nh_brg.hdr.change_type = SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_brg)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to create aps egress vlan edit nexthop

 @param[in] nhid, nexthop id of this nexthop

 @param[in] gport, global port id

 @param[in] p_vlan_info, vlan edit information

 @param[in] dsnh_offset, nexthop offset of this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_aps_egress_vlan_edit_nh_create(uint8 lchip, uint32 nhid,
                                             uint32 dsnh_offset, uint16 aps_bridge_id,
                                             ctc_vlan_egress_edit_info_t* p_vlan_info_working_path,
                                             ctc_vlan_egress_edit_info_t* p_vlan_info_protection_path)
{
    sys_nh_param_brguc_t nh_brg;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, apsBridgeId = %d, dsnh_offset = %d",
                   nhid, aps_bridge_id, dsnh_offset);

    /*SYS_GLOBAL_PORT_CHECK(gport); Check APS bridgeId ?*/
    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_vlan_info_working_path);
    CTC_PTR_VALID_CHECK(p_vlan_info_protection_path);
    if (CTC_FLAG_ISSET(p_vlan_info_working_path->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info_working_path->output_cvid);
    }

    if (CTC_FLAG_ISSET(p_vlan_info_working_path->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info_working_path->output_svid);
    }

    if (CTC_FLAG_ISSET(p_vlan_info_protection_path->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info_protection_path->output_cvid);
    }

    if (CTC_FLAG_ISSET(p_vlan_info_protection_path->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
    {
        CTC_VLAN_RANGE_CHECK(p_vlan_info_protection_path->output_svid);
    }

    sal_memset(&nh_brg, 0, sizeof(sys_nh_param_brguc_t));
    SYS_NH_LOCK;
    nh_brg.hdr.nh_param_type = SYS_NH_TYPE_BRGUC;
    nh_brg.hdr.is_internal_nh = FALSE;
    nh_brg.hdr.nhid = nhid;
    nh_brg.nh_sub_type = SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT;
    nh_brg.gport = aps_bridge_id;
    nh_brg.p_vlan_edit_info = p_vlan_info_working_path;
    nh_brg.p_vlan_edit_info_prot_path = p_vlan_info_protection_path;
    nh_brg.dsnh_offset = dsnh_offset;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_brg)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

#if 0
/**
 @brief This function is to delete egress vlan edit nexthop

 @param[in] nhid, nexthop id of this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_aps_egress_vlan_edit_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_BRGUC), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}
#endif
/**
 @brief This function is to create ipuc nexthop

 @param[in] nhid, nexthop ID

 @param[in] p_member_list, member list to be added to this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_ipuc_nh_create(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_nh_param)
{
    sys_nh_param_ipuc_t nh_param;
    ctc_ip_nh_param_t ipuc_param;
    uint16 arp_id = 0;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_STRIP_MAC))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
		return CTC_E_NOT_SUPPORT;
    }

    if(!CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_UNROV))
    {
        /*unrov nexthop donot care oif parameter*/
        SYS_GLOBAL_PORT_CHECK(p_nh_param->oif.gport);
        CTC_MAX_VALUE_CHECK(p_nh_param->oif.vid, CTC_MAX_VLAN_ID);
        SYS_USW_CID_CHECK(lchip, p_nh_param->cid);
    }

    if (p_nh_param->aps_en)
    {
        if(!CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_UNROV))
        {
            /*unrov nexthop donot care oif parameter*/
            SYS_GLOBAL_PORT_CHECK(p_nh_param->p_oif.gport);
            CTC_MAX_VALUE_CHECK(p_nh_param->p_oif.vid, CTC_MAX_VLAN_ID);
        }
    }

    SYS_NH_LOCK;

    sal_memset(&nh_param, 0, sizeof(sys_nh_param_ipuc_t));
    sal_memset(&ipuc_param, 0, sizeof(ctc_ip_nh_param_t));
    nh_param.p_ipuc_param = &ipuc_param;

    arp_id = p_nh_param->arp_id;
    CTC_ERROR_GOTO(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_param->oif,
        (uint8*)p_nh_param->mac, &nh_param.hdr.is_drop), ret, error);

    if(CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_UNROV) && arp_id)
    {
       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop with arp id cannot support unrov \n");
       SYS_NH_UNLOCK;
       return CTC_E_INVALID_CONFIG;
    }

    if(p_nh_param->aps_en)
    {
        bool protect_en = FALSE;
        arp_id = p_nh_param->p_arp_id;
        CTC_ERROR_GOTO(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_param->p_oif, (uint8*)p_nh_param->p_mac,
            &nh_param.hdr.is_drop), ret, error);

        CTC_ERROR_GOTO(sys_usw_aps_get_protection(lchip, p_nh_param->aps_bridge_group_id, &protect_en), ret, error);
        if(0 != sal_memcmp(nh_param.p_ipuc_param->mac_sa, p_nh_param->mac_sa, sizeof(mac_addr_t)))
        {
            SYS_NH_UNLOCK;
            return  CTC_E_FEATURE_NOT_SUPPORT;
        }
    }

    CTC_ERROR_GOTO(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset), ret, error);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d, gport = %d, vid = %d, mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
                   nhid, p_nh_param->dsnh_offset, p_nh_param->oif.gport, p_nh_param->oif.vid, p_nh_param->mac[0], p_nh_param->mac[1],
                   p_nh_param->mac[2], p_nh_param->mac[3], p_nh_param->mac[4], p_nh_param->mac[5]);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "protection_gport = %d, protection_vid = %d, protection_mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
                   p_nh_param->p_oif.gport, p_nh_param->p_oif.vid, p_nh_param->p_mac[0], p_nh_param->p_mac[1],
                   p_nh_param->p_mac[2], p_nh_param->p_mac[3], p_nh_param->p_mac[4], p_nh_param->p_mac[5]);

    nh_param.hdr.nh_param_type = SYS_NH_TYPE_IPUC;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.hdr.nhid = nhid;
    nh_param.p_ipuc_param = p_nh_param;
    nh_param.is_unrov_nh = CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_UNROV);  /* CTC_IP_NH_FLAG_UNROV*/

    CTC_ERROR_GOTO(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), ret, error);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
error:
    SYS_NH_UNLOCK;
    return ret;
}

/**
 @brief This function is to remove member from multicast bridge nexthop

 @param[in] nhid, nexthop id

 @return CTC_E_XXX
 */
int32
sys_usw_ipuc_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, SYS_NH_CHANGE_TYPE_NH_DELETE, 0));
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_IPUC), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

/**
 @brief This function is to update ipuc nexthop

 @param[in] nhid, nexthop id
 @param[in] vid, vlan  id
 @param[in] gport, global destination port
 @param[in] mac, nexthop mac address
 @param[in] update_type, nexthop update type

 @return CTC_E_XXX
 */
int32
sys_usw_ipuc_nh_update(uint8 lchip, uint32 nhid, ctc_ip_nh_param_t* p_nh_param,
                             sys_nh_entry_change_type_t update_type)
{
    sys_nh_param_ipuc_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_ipuc_t));

    if (SYS_NH_CHANGE_TYPE_FWD_TO_UNROV != update_type)
    {
        SYS_USW_CID_CHECK(lchip, p_nh_param->cid);
    }

    SYS_NH_LOCK;
    nh_param.hdr.nhid = nhid;
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_IPUC;
    nh_param.hdr.change_type = update_type;
    nh_param.p_ipuc_param = p_nh_param;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, update_type, 0));

    return CTC_E_NONE;
}

int32
sys_usw_nh_bind_dsfwd_cb(uint8 lchip, uint32 nhid, sys_nh_update_dsnh_param_t* p_update_dsnh)
{

    sys_nh_info_com_t* p_nh_info = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_info);

    if(!p_nh_info )
    {
        if(!p_update_dsnh->updateAd)
        {
            SYS_NH_UNLOCK;
            return CTC_E_NONE;
        }
        else
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop not exist \n");
            SYS_NH_UNLOCK;
			return CTC_E_NOT_EXIST;
        }
    }
    /*ecmp if MUST use merge Dsfwd mode, so can not bind nexthop*/
    if(p_nh_info->hdr.nh_entry_flags & SYS_NH_INFO_FLAG_USE_ECMP_IF)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ecmp interface can not bind nexthop\n");
        SYS_NH_UNLOCK;
        return CTC_E_NONE;
    }

    switch(p_nh_info->hdr.nh_entry_type )
    {
        case SYS_NH_TYPE_IPUC:
        {
            sys_nh_info_ipuc_t* p_ipuc_info = (sys_nh_info_ipuc_t*)p_nh_info;

            if (p_update_dsnh->updateAd && p_ipuc_info->updateAd)
            {
                SYS_NH_UNLOCK;
    			return CTC_E_IN_USE;
            }
            p_ipuc_info->updateAd  = p_update_dsnh->updateAd;
            p_ipuc_info->data      = p_update_dsnh->data;

        }
            break;
        case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_ip_tunnel_info = (sys_nh_info_ip_tunnel_t*)p_nh_info;

            if (p_update_dsnh->updateAd && p_ip_tunnel_info->updateAd)
            {
                SYS_NH_UNLOCK;
    			return CTC_E_IN_USE;
            }
            p_ip_tunnel_info->updateAd  = p_update_dsnh->updateAd;
            p_ip_tunnel_info->data      = p_update_dsnh->data;

        }
            break;
        case SYS_NH_TYPE_MPLS:
        {
            sys_nh_info_mpls_t* p_mpls_info = (sys_nh_info_mpls_t*)p_nh_info;

            if (p_update_dsnh->updateAd && p_mpls_info->updateAd)
            {
                SYS_NH_UNLOCK;
    			return CTC_E_IN_USE;
            }

            p_mpls_info->updateAd  = p_update_dsnh->updateAd;
            p_mpls_info->data      = p_update_dsnh->data;

        }
            break;
        case SYS_NH_TYPE_TRILL:
        {
            sys_nh_info_trill_t* p_trill_info = (sys_nh_info_trill_t*)p_nh_info;

            if (p_update_dsnh->updateAd && p_trill_info->updateAd)
            {
                SYS_NH_UNLOCK;
    			return CTC_E_IN_USE;
            }

            p_trill_info->updateAd  = p_update_dsnh->updateAd;
            p_trill_info->data      = p_update_dsnh->data;

        }
            break;

        case SYS_NH_TYPE_MISC:
        {
            sys_nh_info_misc_t* p_misc_info = (sys_nh_info_misc_t*)p_nh_info;

            if (p_update_dsnh->updateAd && p_misc_info->updateAd)
            {
                SYS_NH_UNLOCK;
    			return CTC_E_IN_USE;
            }

            p_misc_info->updateAd  = p_update_dsnh->updateAd;
            p_misc_info->data      = p_update_dsnh->data;

        }
            break;
        default:
            break;
    }

    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

#if 0
int32
sys_usw_nh_bind_ipuc_cb(uint8 lchip, uint32 nhid, sys_nh_update_dsnh_param_t* p_update_dsnh)
{

    sys_nh_info_ipuc_t* p_nh_ipuc_info = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_ipuc_info), p_usw_nh_api_master[lchip]->p_mutex);
    if (p_update_dsnh->updateAd && p_nh_ipuc_info->updateAd)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Host nexthop already in use. \n");
        SYS_NH_UNLOCK;
		return CTC_E_IN_USE;
    }

    if (p_nh_ipuc_info->hdr.nh_entry_type != SYS_NH_TYPE_IPUC)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
        SYS_NH_UNLOCK;
		return CTC_E_INVALID_CONFIG;
    }

    p_nh_ipuc_info->updateAd  = p_update_dsnh->updateAd;
    p_nh_ipuc_info->data         = p_update_dsnh->data;

    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}
#endif
/**
 @brief This function is to create mcast nexthop

 @param[in] groupid, basic met offset of this multicast nexthop

 @param[in] p_nh_mcast_group,  group and member information this multicast nexthop

 @return CTC_E_XXX
 */

int32
sys_usw_mcast_nh_create(uint8 lchip, uint32 groupid, sys_nh_param_mcast_group_t* p_nh_mcast_group)
{
    sys_nh_param_mcast_t nh_mcast;
	sys_nh_info_mcast_t* p_mcast_info = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "groupid = %d\n", groupid);

    /*Sanity Check*/
    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_mcast_group);

    if (!p_nh_mcast_group->is_mcast_profile)
    {
        CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_met_offset(lchip, groupid));
    }

    SYS_LOCAL_CHIPID_CHECK(lchip);

    /*mcast profile donot suport stats&mirro*/
    if (((p_nh_mcast_group->is_mirror) || (p_nh_mcast_group->stats_valid)) && p_nh_mcast_group->is_mcast_profile)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " mcast profile donot support mirror or stats\n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_NH_LOCK;
    sal_memset(&nh_mcast, 0, sizeof(sys_nh_param_mcast_t));
    nh_mcast.is_mcast_profile = p_nh_mcast_group->is_mcast_profile;
    nh_mcast.hdr.nh_param_type = SYS_NH_TYPE_MCAST;
    if (p_nh_mcast_group->is_nhid_valid)
    {
        nh_mcast.hdr.nhid = p_nh_mcast_group->nhid;
        nh_mcast.hdr.is_internal_nh = FALSE;
    }
    else
    {   nh_mcast.hdr.nhid = SYS_NH_INVALID_NHID;
        nh_mcast.hdr.is_internal_nh = TRUE;
    }
    if ((p_nh_mcast_group->is_mirror) || (p_nh_mcast_group->stats_valid))
    {
        nh_mcast.hdr.have_dsfwd = TRUE;
    }

    nh_mcast.hdr.stats_valid = p_nh_mcast_group->stats_valid;
    nh_mcast.hdr.stats_id = p_nh_mcast_group->stats_id;

    nh_mcast.groupid = groupid;
    nh_mcast.p_member = &(p_nh_mcast_group->mem_info);
    nh_mcast.opcode = p_nh_mcast_group->opcode;
    nh_mcast.is_mirror = p_nh_mcast_group->is_mirror;


	CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_mcast)), p_usw_nh_api_master[lchip]->p_mutex);
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_get_nhinfo_by_nhid(lchip, nh_mcast.hdr.nhid,
                                           (sys_nh_info_com_t**)&p_mcast_info), p_usw_nh_api_master[lchip]->p_mutex);
    p_nh_mcast_group->fwd_offset = p_mcast_info->hdr.dsfwd_info.dsfwd_offset;
    p_nh_mcast_group->nhid = nh_mcast.hdr.nhid;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to update mcast nexthop

 @param[in] nhid,  nexthopid

 @return CTC_E_XXX
 */

int32
sys_usw_mcast_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_MCAST), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

/**
 @brief This function is to update mcast nexthop

 @param[in] p_nh_mcast_group,  group and member information this multicast nexthop

 @return CTC_E_XXX
 */

int32
sys_usw_mcast_nh_update(uint8 lchip, sys_nh_param_mcast_group_t* p_nh_mcast_group)
{
    sys_nh_param_mcast_t nh_mcast;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_mcast_group);
    SYS_LOCAL_CHIPID_CHECK(lchip);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %u\n", p_nh_mcast_group->nhid);
    SYS_NH_LOCK;

    sal_memset(&nh_mcast, 0, sizeof(sys_nh_param_mcast_t));
    nh_mcast.hdr.nh_param_type = SYS_NH_TYPE_MCAST;
    nh_mcast.p_member = &(p_nh_mcast_group->mem_info);
    nh_mcast.hdr.nhid = p_nh_mcast_group->nhid;
    nh_mcast.opcode = p_nh_mcast_group->opcode;
    nh_mcast.is_mcast_profile = p_nh_mcast_group->is_mcast_profile;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, p_nh_mcast_group->nhid,
                                                 (sys_nh_param_com_t*)(&nh_mcast)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to traverse mcast nexthop

 @param[in] fn callback function to deal with all the ipuc entry

 @param[in] p_data data used by the callback function

 @return CTC_E_XXX
 */
int32
sys_usw_nh_traverse_mcast(uint8 lchip, ctc_nh_mcast_traverse_fn fn, ctc_nh_mcast_traverse_t* p_data)
{
    int32 ret = CTC_E_NONE;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(fn);
    CTC_PTR_VALID_CHECK(p_data);
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_traverse_mcast(lchip, fn, p_data), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return ret;
}

/**
 @brief This function is to create mpls nexthop

 @param[in] nhid  nexthop ID

 @param[in] dsnh_offset  DsNexthop offset used by this nexthop

 @param[in] p_nh_param  Nexthop param used to create this nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_mpls_nh_create(uint8 lchip, uint32 nhid,   ctc_mpls_nexthop_param_t* p_nh_param)
{
    sys_nh_param_mpls_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_param);

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset));

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d\n", nhid, p_nh_param->dsnh_offset);
    SYS_NH_LOCK;

    if (p_nh_param->nh_prop == CTC_MPLS_NH_POP_TYPE)
    {
        if(CTC_FLAG_ISSET(p_nh_param->flag, CTC_MPLS_NH_IS_UNRSV) && (p_nh_param->nh_para.nh_param_pop.arp_id))
        {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop with arp id cannot support unrov \n");
           SYS_NH_UNLOCK;
           return CTC_E_INVALID_CONFIG;
        }
    }
    else if (p_nh_param->nh_prop == CTC_MPLS_NH_PUSH_TYPE && !p_nh_param->nh_para.nh_param_push.loop_nhid)
    {
        sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
        uint8  use_arp  = 0;

        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_lkup_mpls_tunnel(lchip, p_nh_param->nh_para.nh_param_push.tunnel_id,
            &p_mpls_tunnel), p_usw_nh_api_master[lchip]->p_mutex);
        if (!p_mpls_tunnel)
        {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Cannot Find mpls tunnel Nexthop ID\n");
           SYS_NH_UNLOCK;
           return CTC_E_NOT_EXIST;
        }

        use_arp = p_mpls_tunnel->arp_id[0][0] || p_mpls_tunnel->arp_id[0][1] ||
                  p_mpls_tunnel->arp_id[1][0] || p_mpls_tunnel->arp_id[1][1];

        if(CTC_FLAG_ISSET(p_nh_param->flag, CTC_MPLS_NH_IS_UNRSV) && use_arp)
        {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop with arp id cannot support unrov \n");
           SYS_NH_UNLOCK;
           return CTC_E_INVALID_CONFIG;
        }
    }
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_mpls_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_MPLS;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.hdr.nhid = nhid;
    nh_param.p_mpls_nh_param = p_nh_param;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to remove mpls nexthop by nexthop id

 @param[in] nhid nexthop id

 @return CTC_E_XXX
 */
int32
sys_usw_mpls_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, SYS_NH_CHANGE_TYPE_NH_DELETE, 0));
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_MPLS), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to update mpls nexthop

 @param[in] nhid, nexthop id

 @param[in] p_nh_param nexthop paramter

 @return CTC_E_XXX
 */
int32
sys_usw_mpls_nh_update(uint8 lchip, uint32 nhid, ctc_mpls_nexthop_param_t* p_nh_param,
                             sys_nh_entry_change_type_t change_type)
{
    sys_nh_param_mpls_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_mpls_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_MPLS;
    nh_param.hdr.nhid = nhid;
    nh_param.p_mpls_nh_param = p_nh_param;
    nh_param.hdr.change_type = change_type;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, change_type, 0));

    return CTC_E_NONE;
}

int32
sys_usw_ecmp_nh_create(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_parama)
{
    sys_nh_param_ecmp_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_nh_parama);
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_ecmp_t));

    nh_param.hdr.nh_param_type = SYS_NH_TYPE_ECMP;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.p_ecmp_param = p_nh_parama;
    nh_param.hdr.nhid = nhid;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ecmp_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_ECMP), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ecmp_nh_update(uint8 lchip, uint32 nhid, ctc_nh_ecmp_nh_param_t* p_nh_param)
{
    sys_nh_param_ecmp_t nh_param;
    ctc_nh_ecmp_nh_param_t ecmp_param;
    uint8 loop = 0;

    CTC_PTR_VALID_CHECK(p_nh_param);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_ecmp_t));
    sal_memset(&ecmp_param, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    sal_memcpy(&ecmp_param, p_nh_param, sizeof(ctc_nh_ecmp_nh_param_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_ECMP;
    nh_param.hdr.nhid = nhid;
    nh_param.p_ecmp_param = &ecmp_param;

    for (loop = 0; loop < p_nh_param->nh_num; loop++)
    {
        nh_param.p_ecmp_param->nh_num = 1;
        nh_param.p_ecmp_param->nhid[0] = p_nh_param->nhid[loop];
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief This function is to create IPE Loopback nexthop
 */
int32
sys_usw_iloop_nh_create(uint8 lchip, uint32 nhid, ctc_loopback_nexthop_param_t* p_nh_param)
{
    sys_nh_param_iloop_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_param);
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_iloop_t));

    nh_param.hdr.nh_param_type = SYS_NH_TYPE_ILOOP;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.p_iloop_param = p_nh_param;
    nh_param.hdr.nhid = nhid;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_iloop_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_ILOOP), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

/**
 @brief This function is to create rspan  nexthop
 */
int32
sys_usw_rspan_nh_create(uint8 lchip, uint32* nhid, uint32 dsnh_offset, ctc_rspan_nexthop_param_t* p_nh_param)
{
    sys_nh_param_rspan_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_param);
    CTC_PTR_VALID_CHECK(nhid);
    SYS_NH_LOCK;

    sal_memset(&nh_param, 0, sizeof(sys_nh_param_rspan_t));

    nh_param.hdr.nh_param_type = SYS_NH_TYPE_RSPAN;
    nh_param.hdr.is_internal_nh = (*nhid == CTC_MAX_UINT32_VALUE) ? TRUE : FALSE;
    nh_param.p_rspan_param = p_nh_param;
    nh_param.hdr.nhid = *nhid;
    nh_param.dsnh_offset = dsnh_offset;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);

    if (nh_param.hdr.is_internal_nh)
    {
        *nhid = nh_param.hdr.nhid;
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_rspan_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_RSPAN), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_nh_create(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param)
{
    sys_nh_param_ip_tunnel_t nh_param;
    uint16 arp_id = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_nh_param);

    sal_memset(&nh_param, 0, sizeof(nh_param));

    arp_id = p_nh_param->arp_id;
    CTC_ERROR_RETURN(sys_usw_nh_get_arp_oif(lchip, arp_id, &p_nh_param->oif, (uint8*)p_nh_param->mac, &nh_param.hdr.is_drop));
    SYS_GLOBAL_PORT_CHECK(p_nh_param->oif.gport);
    CTC_MAX_VALUE_CHECK(p_nh_param->tunnel_info.dscp_domain, 15);

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d, gport = %d\n",
                   nhid, p_nh_param->dsnh_offset, p_nh_param->oif.gport);

    if(CTC_FLAG_ISSET(p_nh_param->flag, CTC_IP_NH_FLAG_UNROV) && arp_id)
    {
       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Nexthop with arp id cannot support unrov \n");
       return CTC_E_INVALID_CONFIG;
    }

    SYS_NH_LOCK;
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_IP_TUNNEL;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.hdr.nhid = nhid;
    nh_param.p_ip_nh_param = p_nh_param;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, SYS_NH_CHANGE_TYPE_NH_DELETE, 0));
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_IP_TUNNEL), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_nh_update(uint8 lchip, uint32 nhid, ctc_ip_tunnel_nh_param_t* p_nh_param)
{
    sys_nh_param_ip_tunnel_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_ip_tunnel_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_IP_TUNNEL;
    nh_param.hdr.nhid = nhid;
    nh_param.p_ip_nh_param = p_nh_param;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_get_arp_oif(lchip, p_nh_param->arp_id, &p_nh_param->oif,
        (uint8*)p_nh_param->mac, &nh_param.hdr.is_drop), p_usw_nh_api_master[lchip]->p_mutex);

    if (p_nh_param->upd_type == CTC_NH_UPD_FWD_ATTR)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR;
    }
    else if (p_nh_param->upd_type == CTC_NH_UPD_UNRSV_TO_FWD)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UNROV_TO_FWD;
    }
    else
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_FWD_TO_UNROV;
    }

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, p_nh_param->upd_type, 0));

    return CTC_E_NONE;
}

int32
sys_usw_wlan_tunnel_nh_create(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param)
{
    sys_nh_param_wlan_tunnel_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_nh_param);
    CTC_MAX_VALUE_CHECK(p_nh_param->dscp_domain, 15);
    CTC_MAX_VALUE_CHECK(p_nh_param->cos_domain, 7);
    CTC_MAX_VALUE_CHECK(p_nh_param->encrypt_id, 0x7f);
    CTC_MAX_VALUE_CHECK(p_nh_param->vlan_id, CTC_MAX_VLAN_ID);
    CTC_MAX_VALUE_CHECK(p_nh_param->flow_label_mode, CTC_NH_FLOW_LABEL_ASSIGN);
    CTC_MAX_VALUE_CHECK(p_nh_param->ecn_select, CTC_NH_ECN_SELECT_PACKET);
    CTC_MAX_VALUE_CHECK(p_nh_param->dscp_select, CTC_NH_DSCP_SELECT_NONE);

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d\n",
                   nhid, p_nh_param->dsnh_offset);
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_WLAN_TUNNEL;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.hdr.nhid = nhid;
    nh_param.p_wlan_nh_param = p_nh_param;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_wlan_tunnel_nh_delete(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_WLAN_TUNNEL), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_wlan_tunnel_nh_update(uint8 lchip, uint32 nhid, ctc_nh_wlan_tunnel_param_t* p_nh_param)
{
    sys_nh_param_wlan_tunnel_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_wlan_tunnel_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_WLAN_TUNNEL;
    nh_param.hdr.nhid = nhid;
    nh_param.p_wlan_nh_param = p_nh_param;

    if (p_nh_param->upd_type == CTC_NH_UPD_FWD_ATTR)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR;
    }
    else if (p_nh_param->upd_type == CTC_NH_UPD_UNRSV_TO_FWD)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UNROV_TO_FWD;
    }
    else
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_FWD_TO_UNROV;
    }

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief The function is to add misc nexthop

*/

int32
sys_usw_nh_add_misc(uint8 lchip, uint32* nhid, ctc_misc_nh_param_t* p_nh_param, bool is_internal_nh)
{

    sys_nh_param_misc_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_nh_param);

     if (p_nh_param->is_oif)
    {
        SYS_GLOBAL_PORT_CHECK(p_nh_param->oif.gport);
        CTC_MAX_VALUE_CHECK(p_nh_param->oif.vid, CTC_MAX_VLAN_ID);
    }
    else
    {
        SYS_GLOBAL_PORT_CHECK(p_nh_param->gport);
    }

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d, gport = %d\n",
                   *nhid, p_nh_param->dsnh_offset, p_nh_param->gport);

    SYS_USW_CID_CHECK(lchip, p_nh_param->cid);

    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_MISC;
    nh_param.hdr.is_internal_nh = is_internal_nh;

    if (FALSE == is_internal_nh)
    {
        nh_param.hdr.nhid = *nhid;
    }

    nh_param.p_misc_param = p_nh_param;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);

    if (TRUE == is_internal_nh)
    {
        *nhid = nh_param.hdr.nhid;
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief The function is to remove misc nexthop

*/
int32
sys_usw_nh_remove_misc(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, SYS_NH_CHANGE_TYPE_NH_DELETE, 0));
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_MISC), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}


/**
 @brief This function is to update misc nexthop
 @return CTC_E_XXX
 */
int32
sys_usw_misc_nh_update(uint8 lchip, uint32 nhid, ctc_misc_nh_param_t* p_nh_param,
                             sys_nh_entry_change_type_t update_type)
{
    sys_nh_param_misc_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_nh_param);
    SYS_USW_CID_CHECK(lchip, p_nh_param->cid);

     if (p_nh_param->is_oif)
    {
        SYS_GLOBAL_PORT_CHECK(p_nh_param->oif.gport);
        CTC_MAX_VALUE_CHECK(p_nh_param->oif.vid, CTC_MAX_VLAN_ID);
    }
    else
    {
        SYS_GLOBAL_PORT_CHECK(p_nh_param->gport);
    }

    sal_memset(&nh_param, 0, sizeof(sys_nh_param_misc_t));

    SYS_NH_LOCK;
    nh_param.hdr.nhid = nhid;
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_MISC;
    nh_param.hdr.change_type = update_type;
    nh_param.p_misc_param = p_nh_param;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, update_type, 0));

    return CTC_E_NONE;
}


int32
sys_usw_nh_trill_create(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param)
{
    sys_nh_param_trill_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_nh_param);
    SYS_GLOBAL_PORT_CHECK(p_nh_param->oif.gport);

    CTC_ERROR_RETURN(sys_usw_nh_check_max_glb_nh_offset(lchip, p_nh_param->dsnh_offset));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d, dsnhoffset = %d, gport = %d\n",
                   nhid, p_nh_param->dsnh_offset, p_nh_param->oif.gport);
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_TRILL;
    nh_param.hdr.is_internal_nh = FALSE;
    nh_param.hdr.nhid = nhid;
    nh_param.p_trill_nh_param = p_nh_param;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_create(lchip, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_trill_remove(uint8 lchip, uint32 nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, SYS_NH_CHANGE_TYPE_NH_DELETE, 0));
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_delete(lchip, nhid, SYS_NH_TYPE_TRILL), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_nh_trill_update(uint8 lchip, uint32 nhid, ctc_nh_trill_param_t* p_nh_param)
{
    sys_nh_param_trill_t nh_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhid = %d\n", nhid);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_trill_t));
    nh_param.hdr.nh_param_type = SYS_NH_TYPE_TRILL;
    nh_param.hdr.nhid = nhid;
    nh_param.p_trill_nh_param = p_nh_param;

    if (p_nh_param->upd_type == CTC_NH_UPD_FWD_ATTR)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR;
    }
    else if (p_nh_param->upd_type == CTC_NH_UPD_UNRSV_TO_FWD)
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_UNROV_TO_FWD;
    }
    else
    {
        nh_param.hdr.change_type = SYS_NH_CHANGE_TYPE_FWD_TO_UNROV;
    }

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_api_update(lchip, nhid, (sys_nh_param_com_t*)(&nh_param)), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, nhid, p_nh_param->upd_type, 0));

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_udf_profile(uint8 lchip, ctc_nh_udf_profile_param_t* p_edit)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_edit);
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_add_udf_profile(lchip, p_edit), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_remove_udf_profile(uint8 lchip, uint8 profile_id)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_remove_udf_profile(lchip, profile_id), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_oam_en(uint8 lchip, uint32 nhid, sys_nh_update_oam_info_t *p_oam_info)
{
    sys_nh_info_com_t* p_nh_com_info;
    sys_nh_info_mpls_t* p_nh_mpls_info = NULL;
    DsL3EditMpls3W_m  ds_edit_mpls;
    uint32 cmd = 0;
    uint8 lchip2 = 0,  gchip = 0;
    uint32 offset = 0;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;
    uint8 bind_dsma = 0;
    int32 ret = 0;
    uint8 aps_en = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_LOCK;
    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info), ret, error);

    if (SYS_NH_TYPE_MPLS == p_nh_com_info->hdr.nh_entry_type)
    {
        p_nh_mpls_info = (sys_nh_info_mpls_t*)p_nh_com_info;
        if (!p_nh_mpls_info)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			ret = CTC_E_NOT_EXIST;
            goto error;
        }
        gchip = CTC_MAP_GPORT_TO_GCHIP(p_nh_mpls_info->gport);
        CTC_ERROR_GOTO(sys_usw_get_local_chip_id(gchip, &lchip2), ret, error);

        aps_en = CTC_FLAG_ISSET(p_nh_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
        if (CTC_FLAG_ISSET(p_nh_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
        {
             /*PW nexthop*/
             SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update pw lm ,  innerEditPtr = %d\n",
                            p_nh_mpls_info->working_path.dsl3edit_offset);

            if (!p_nh_mpls_info->protection_path && CTC_FLAG_ISSET(p_nh_mpls_info->working_path.p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS))
            {
                /*pw no aps+lsp aps*/
                bind_dsma = 1;
            }

            if (!p_oam_info->is_protection_path)
            {
              offset = p_nh_mpls_info->working_path.dsl3edit_offset;
            }
            if (p_oam_info->is_protection_path
                && !CTC_FLAG_ISSET(p_nh_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH)
                && CTC_FLAG_ISSET(p_nh_mpls_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
            {
                offset = p_nh_mpls_info->protection_path->dsl3edit_offset;
            }

        }
        else
        {   /*tunnel with aps,only need consider LSP, Because SPME Nexthop will be regarded as LSP Nexthp.*/
            /*oam lock on tunnel, here nhid means tunnel id*/

            p_mpls_tunnel =  p_nh_mpls_info->working_path.p_mpls_tunnel;
            if (!p_mpls_tunnel)
            {
                ret = CTC_E_NOT_EXIST;
                goto error;
            }

            if (!CTC_FLAG_ISSET(p_mpls_tunnel->flag, SYS_NH_MPLS_TUNNEL_FLAG_APS)
    		  	|| !p_oam_info->is_protection_path)
            {
               offset = p_mpls_tunnel->lsp_offset[0];
           	}
    	    else
    	   	{
    		   offset = p_mpls_tunnel->lsp_offset[1];
    		}
    	}

        if (p_oam_info->update_type)
        {
    	    if (p_oam_info->dsma_en && bind_dsma && DRV_IS_DUET2(lchip))
    	    {
    	        p_nh_mpls_info->ma_idx = p_oam_info->ma_idx;
    	        p_nh_mpls_info->dsma_valid = 1;
    	    }
    	    else
    	    {
    	        p_nh_mpls_info->ma_idx = 0;
    	        p_nh_mpls_info->dsma_valid = 0;
    	    }
        }
        else
        {
            if(offset == 0)
            {  /*invalid nexthop */
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
                ret = CTC_E_INVALID_PTR;
                goto error;
            }

            cmd = DRV_IOR(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, offset, cmd, &ds_edit_mpls), ret, error);
            if (p_oam_info->lm_en)
            {
                SetDsL3EditMpls3W(V, mplsOamIndex_f, &ds_edit_mpls, p_oam_info->oam_mep_index);
            }
            else
            {
                SetDsL3EditMpls3W(V, mplsOamIndex_f, &ds_edit_mpls, 0);
            }

            if (p_oam_info->lock_en)
            {
                SetDsL3EditMpls3W(V, discardType_f,&ds_edit_mpls, 1);
                SetDsL3EditMpls3W(V, discard_f,    &ds_edit_mpls, 1);
            }
            else
            {
                SetDsL3EditMpls3W(V, discardType_f,&ds_edit_mpls, 0);
                SetDsL3EditMpls3W(V, discard_f,    &ds_edit_mpls, 0);
            }
            cmd = DRV_IOW(DsL3EditMpls3W_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, offset, cmd, &ds_edit_mpls), ret, error);
        }
    }
    else if (SYS_NH_TYPE_IPUC == p_nh_com_info->hdr.nh_entry_type)
    {
        sys_nh_info_ipuc_t* p_nh_ipuc_info = NULL;
        p_nh_ipuc_info = (sys_nh_info_ipuc_t*)p_nh_com_info;
        if (!p_nh_ipuc_info)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			ret = CTC_E_NOT_EXIST;
            goto error;
        }
        aps_en = CTC_FLAG_ISSET(p_nh_ipuc_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
    }
    else
    {
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

	if ((p_oam_info->update_type == 1) && !aps_en)
	{
      	sys_nh_oam_info_t oam_info;
        sal_memset(&oam_info, 0, sizeof(sys_nh_oam_info_t));

        /*aps destmap used dsapsbridge */
        oam_info.mep_index = p_oam_info->mep_index;
        oam_info.mep_type = p_oam_info->mep_type;
    	CTC_ERROR_GOTO(_sys_usw_nh_update_oam_ref_info(lchip, p_nh_com_info, &oam_info, p_oam_info->dsma_en), ret, error);
  	}

error:
    SYS_NH_UNLOCK;
    return ret;
}

#define __ARP__
int32
sys_usw_nh_add_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_param)
{
    int32 ret = CTC_E_NONE;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_param);
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_add_nexthop_mac(lchip, arp_id, p_param), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return ret;
}

/**
 @brief Remove Next Hop Router Mac
*/
int32
sys_usw_nh_remove_nexthop_mac(uint8 lchip, uint16 arp_id)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_remove_nexthop_mac(lchip, arp_id), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief Update Next Hop Router Mac
*/
int32
sys_usw_nh_update_nexthop_mac(uint8 lchip, uint16 arp_id, ctc_nh_nexthop_mac_param_t* p_new_param)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_update_nexthop_mac(lchip, arp_id, p_new_param), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    CTC_ERROR_RETURN(sys_usw_nh_update_ad(lchip, arp_id, 0, 1));

    return CTC_E_NONE;
}

int32
sys_usw_nh_del_ipmc_dsnh_offset(uint8 lchip, uint16 l3ifid)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_INIT_CHECK;

    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    SYS_NH_LOCK;
    if (p_nh_master->ipmc_dsnh_offset[l3ifid] != 0)
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_4W, 1,
                                         p_nh_master->ipmc_dsnh_offset[l3ifid]);
        p_nh_master->ipmc_dsnh_offset[l3ifid] = 0;

    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_alloc_ecmp_offset(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32* p_offset)
{
    sys_usw_opf_t opf;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sys_usw_nh_get_nh_master(lchip, &p_nh_master);

    if (is_grp)
    {
        opf.pool_type = p_nh_master->ecmp_group_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_alloc_offset(lchip, &opf, entry_num, p_offset), p_usw_nh_api_master[lchip]->p_mutex);
    }
    else
    {
        opf.pool_type = p_nh_master->ecmp_member_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_alloc_offset(lchip, &opf, entry_num, p_offset), p_usw_nh_api_master[lchip]->p_mutex);
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_alloc_ecmp_offset_from_position(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32 offset)
{
    sys_usw_opf_t opf;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sys_usw_nh_get_nh_master(lchip, &p_nh_master);

    if (is_grp)
    {
        opf.pool_type = p_nh_master->ecmp_group_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_alloc_offset_from_position(lchip, &opf, entry_num, offset),
            p_usw_nh_api_master[lchip]->p_mutex);
    }
    else
    {
        opf.pool_type = p_nh_master->ecmp_member_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_alloc_offset_from_position(lchip, &opf, entry_num, offset),
            p_usw_nh_api_master[lchip]->p_mutex);
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_free_ecmp_offset(uint8 lchip, uint8 is_grp, uint8 entry_num, uint32 offset)
{
    sys_usw_opf_t opf;
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sys_usw_nh_get_nh_master(lchip, &p_nh_master);

    if (is_grp)
    {
        opf.pool_type = p_nh_master->ecmp_group_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_free_offset(lchip, &opf, entry_num, offset), p_usw_nh_api_master[lchip]->p_mutex);
    }
    else
    {
        opf.pool_type = p_nh_master->ecmp_member_opf_type;
        opf.pool_index = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_opf_free_offset(lchip, &opf, entry_num, offset), p_usw_nh_api_master[lchip]->p_mutex);
    }
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

#define __ECMP__
int32
sys_usw_nh_get_current_ecmp_group_num(uint8 lchip, uint16* cur_group_num)
{

    sys_usw_nh_master_t* p_nh_master = NULL;
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), p_usw_nh_api_master[lchip]->p_mutex);
    *cur_group_num = p_nh_master->cur_ecmp_cnt;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_max_ecmp_group_num(uint8 lchip, uint16* max_group_num)
{

    sys_usw_nh_master_t* p_nh_master = NULL;
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), p_usw_nh_api_master[lchip]->p_mutex);

    *max_group_num = p_nh_master->max_ecmp_group_num;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/*Notice: the function only can used by ctc layer or sys layer from other module, for nexthop module cannot use!!!!*/
int32
sys_usw_nh_get_max_ecmp(uint8 lchip, uint16* max_ecmp)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_max_ecmp(lchip, max_ecmp), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}


int32
sys_usw_nh_get_resolved_offset(uint8 lchip, sys_usw_nh_res_offset_type_t type, uint32* p_offset)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_offset);

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_resolved_offset(lchip, type, p_offset),
        p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

#define __INTERNAL_API__

int32
sys_usw_nh_alloc_from_position(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32 offset)
{

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_offset_alloc_from_position(lchip, entry_type, entry_num, offset),
        p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_alloc(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32* p_offset)
{

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_offset_alloc(lchip, entry_type, entry_num, p_offset), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_free(uint8 lchip, sys_nh_entry_table_type_t entry_type, uint32 entry_num, uint32 offset)
{
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_offset_free(lchip, entry_type, entry_num, offset), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_nh_add_dsfwd(uint8 lchip, sys_nh_param_dsfwd_t* p_dsfwd_param)
{
    CTC_PTR_VALID_CHECK(p_dsfwd_param);
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_write_entry_dsfwd(lchip, p_dsfwd_param), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_dsfwd(uint8 lchip, uint32 *offset, uint32 dest_map, uint32 dsnh_offset, uint8 dsnh_8w,
                               uint8 del, uint8 critical_packet)
{
    CTC_PTR_VALID_CHECK(offset);
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_update_entry_dsfwd(lchip, offset, dest_map, dsnh_offset, dsnh_8w, del, critical_packet),
        p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;

}

int32
sys_usw_nh_add_dsmet(uint8 lchip, sys_nh_param_dsmet_t* p_met_param)
{
    sys_met_t dsmet;

    CTC_PTR_VALID_CHECK(p_met_param);
    SYS_NH_INIT_CHECK;

    sal_memset(&dsmet, 0, sizeof(sys_met_t));

    SYS_NH_LOCK;
    dsmet.dest_id = p_met_param->dest_id;
    dsmet.remote_chip = p_met_param->remote_chip;
    dsmet.end_local_rep = p_met_param->end_local_rep;
    dsmet.next_met_entry_ptr = p_met_param->next_met_entry_ptr;
    dsmet.next_hop_ptr = p_met_param->next_hop_ptr;

    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_MET, p_met_param->met_offset, &dsmet),
        p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_vpws_vpnid(uint8 lchip, uint32 nhid, uint32 vpn_id)
{

    DsFwd_m dsfwd;
    int32  ret = 0;
    sys_nh_info_com_t* p_nh_com_info;
    uint32 dsfwd_offset = 0;

    sal_memset(&dsfwd, 0, sizeof(dsfwd));
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_com_info), ret, error);
    if (CTC_FLAG_ISSET(p_nh_com_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LEN_ADJUST_EN))
    {
        ret = CTC_E_INVALID_CONFIG;
        goto error;
    }

    dsfwd_offset = p_nh_com_info->hdr.dsfwd_info.dsfwd_offset;

    CTC_ERROR_GOTO(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, dsfwd_offset, &dsfwd), ret, error);
    if (GetDsFwd(V, isHalf_f, &dsfwd))
    {
        ret =  CTC_E_INVALID_CONFIG;
        goto error;
    }

    SetDsFwd(V, vpwsFidEn_f, &dsfwd, 1);
    SetDsFwd(V, truncateLenProfId_f, &dsfwd, (vpn_id&0xf));
    SetDsFwd(V, truncateLenProfIdType_f, &dsfwd, ((vpn_id>>4)&0x1));
    SetDsFwd(V, serviceId_f, &dsfwd, ((vpn_id>>5)&0x1ff));

    CTC_ERROR_GOTO(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_FWD, dsfwd_offset/2, &dsfwd), ret, error);

error:
    SYS_NH_UNLOCK;

    return ret;
}

uint8
sys_usw_nh_get_edit_mode(uint8 lchip)
{
    uint8 edit_mode = 0;

    if (!g_usw_nh_master[lchip])
    {
        return 0;
    }

    SYS_NH_LOCK;
    edit_mode =  g_usw_nh_master[lchip]->pkt_nh_edit_mode;
    SYS_NH_UNLOCK;

    return edit_mode;
}


int32
sys_usw_nh_traverse_mcast_db(uint8 lchip, vector_traversal_fn2 fn, void* data)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    SYS_NH_INIT_CHECK;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    ctc_vector_traverse2(p_nh_master->mcast_group_vec, 0, fn, data);

    return CTC_E_NONE;
}

int32
sys_usw_nh_check_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num,
                                      bool should_not_inuse)
{
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_check_glb_met_offset(lchip, start_offset, entry_num, should_not_inuse), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}


int32
sys_usw_nh_set_glb_met_offset(uint8 lchip, uint32 start_offset, uint32 entry_num, bool is_set)
{
    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_set_glb_met_offset(lchip, start_offset, entry_num, is_set), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}


int32
sys_usw_nh_get_fatal_excp_dsnh_offset(uint8 lchip, uint32* p_offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_PTR_VALID_CHECK(p_offset);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_offset);

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), p_usw_nh_api_master[lchip]->p_mutex);
    *p_offset = p_nh_master->fatal_excp_base;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_get_max_external_nhid(uint8 lchip, uint32* nhid)
{
	sys_usw_nh_master_t* p_nh_master = NULL;

	SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
	CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    *nhid = p_nh_master->max_external_nhid;
    return CTC_E_NONE;
}

int32
sys_usw_nh_get_mirror_info_by_nhid(uint8 lchip, uint32 nhid, uint32* dsnh_offset, uint32* gport, bool enable)
{
    sys_nh_info_com_t* p_nhinfo;
    sys_usw_nh_master_t* p_nh_master = NULL;
    uint8  gchip = 0;

    CTC_PTR_VALID_CHECK(dsnh_offset);
    CTC_PTR_VALID_CHECK(gport);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;

    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), p_usw_nh_api_master[lchip]->p_mutex);
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_api_get_nhinfo_by_nhid(lchip, nhid, p_nh_master, &p_nhinfo), p_usw_nh_api_master[lchip]->p_mutex);

    switch (p_nhinfo->hdr.nh_entry_type)
    {
    case SYS_NH_TYPE_RSPAN:
        {
            *gport = 0xFFFF;
            *dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        }
        break;

    case SYS_NH_TYPE_MCAST:
        {
            *gport = 0;
            *dsnh_offset = SYS_DSNH_INDEX_FOR_NONE;
        }
        break;

    case SYS_NH_TYPE_IP_TUNNEL:
        {
            sys_nh_info_ip_tunnel_t* p_ip_tunnel = (sys_nh_info_ip_tunnel_t*)p_nhinfo;

            *gport = p_ip_tunnel->gport;
            *dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;

            if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
            {
                CTC_BIT_SET(*dsnh_offset, 31);
            }

        }
        break;

    case  SYS_NH_TYPE_TOCPU:
        sys_usw_get_gchip_id(lchip, &gchip);
        *gport = CTC_GPORT_RCPU(gchip);
        *dsnh_offset = SYS_DSNH_INDEX_FOR_NONE;
        break;

    case SYS_NH_TYPE_ILOOP:
        {
            sys_nh_info_special_t* p_nh_spec_info = (sys_nh_info_special_t*)p_nhinfo;

            *gport = p_nh_spec_info->dest_gport;
            *dsnh_offset = p_nh_spec_info->hdr.dsfwd_info.dsnh_offset;
        }
        break;

    case SYS_NH_TYPE_MISC:
        {
            sys_nh_info_misc_t* p_nh_misc_info = (sys_nh_info_misc_t*)p_nhinfo;
            *gport = p_nh_misc_info->gport;
            *dsnh_offset = p_nh_misc_info->hdr.dsfwd_info.dsnh_offset;

            if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
            {
                CTC_BIT_SET(*dsnh_offset, 31);
            }

        }
        break;

    case  SYS_NH_TYPE_ECMP:
        {
            sys_nh_info_ecmp_t* p_ecmp = (sys_nh_info_ecmp_t*)p_nhinfo;
            *gport = p_ecmp->gport;
            *dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
            CTC_BIT_SET(*dsnh_offset, 31);
        }
        break;
    default:
        SYS_NH_UNLOCK;
        return CTC_E_INVALID_PARAM;

    }

    SYS_NH_UNLOCK;
    return CTC_E_NONE;

}

int32
sys_usw_nh_get_reflective_dsfwd_offset(uint8 lchip, uint32* p_dsfwd_offset)
{
    sys_usw_nh_master_t* p_nh_master = NULL;

    SYS_NH_LOCK;

    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_nh_master(lchip, &p_nh_master), p_usw_nh_api_master[lchip]->p_mutex);

    *p_dsfwd_offset = p_nh_master->reflective_resolved_dsfwd_offset;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/*Notice: This function donot use Lock, nexthop&Aps will call each other, Lock have no meaning and will cause deadlock*/
int32
sys_usw_nh_map_destmap_to_port(uint8 lchip, uint32 destmap, uint32 *gport)
{
    uint32 cmd = 0;
    uint32 arp_destmap = 0;
    uint16 destmap_profile = 0;

    SYS_NH_INIT_CHECK;

    if ((destmap >> 17)&0x01)
    {
        destmap_profile = (destmap&0x1FF);
        cmd = DRV_IOR(DsDestMapProfileUc_t, DsDestMapProfileUc_destMap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, destmap_profile, cmd, &arp_destmap));
        if (arp_destmap == 0xffff)
        {
            uint8 gchip = 0;
            /*Arp to cpu*/
            sys_usw_get_gchip_id(lchip, &gchip);
            *gport = CTC_GPORT_RCPU(gchip);
        }
        else
        {
            *gport = SYS_USW_DESTMAP_TO_DRV_GPORT(arp_destmap);
            *gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(*gport);
        }

    }
    else
    {
        *gport = SYS_USW_DESTMAP_TO_DRV_GPORT(destmap);
        *gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(*gport);
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_set_bpe_en(uint8 lchip, bool enable)
{
	sys_usw_nh_master_t* p_nh_master = NULL;
    CTC_ERROR_RETURN(_sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    SYS_NH_LOCK;

    /*0. DsNexthop for bpe ecid transparent*/
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_dsnh_init_for_bpe_transparent(lchip,
             &(p_nh_master->sys_nh_resolved_offset[SYS_NH_RES_OFFSET_TYPE_BPE_TRANSPARENT_NH])), p_usw_nh_api_master[lchip]->p_mutex);

    p_nh_master->bpe_en = enable;
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

/*The function is called during fdb init, do not using lock*/
int32
sys_usw_nh_vxlan_vni_init(uint8 lchip)
{
    CTC_ERROR_RETURN(_sys_usw_nh_vxlan_vni_init(lchip));
    return CTC_E_NONE;
}

bool
sys_usw_nh_is_ipmc_logic_rep_enable(uint8 lchip)
{
    int32 ret = 0;

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    ret = _sys_usw_nh_is_ipmc_logic_rep_enable(lchip);
    SYS_NH_UNLOCK;

    return ret ? TRUE : FALSE;
}


int32
sys_usw_nh_set_ipmc_logic_replication(uint8 lchip, uint8 enable)
{
    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_set_ipmc_logic_replication(lchip, enable), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

uint8
sys_usw_nh_get_met_mode(uint8 lchip)
{
    uint8 mode = 0;

    if (!p_usw_nh_api_master[lchip])
    {
        return 0;
    }

    SYS_NH_LOCK;
    mode = g_usw_nh_master[lchip]->met_mode;
    SYS_NH_UNLOCK;

    return mode;
}

int32
sys_usw_nh_get_mcast_nh(uint8 lchip, uint32 group_id, uint32* nhid)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    CTC_PTR_VALID_CHECK(nhid);

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_nh_get_mcast_nh(lchip, group_id, nhid), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_dot1ae(uint8 lchip, void* param)
{
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_ip_tunnel_update_dot1ae(lchip, param), p_usw_nh_api_master[lchip]->p_mutex);
    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nh_traverse_mpls_nexthop(uint8 lchip, uint16 tunnel_id, nh_traversal_fn cb, void* user_data)
{
    sys_nh_info_mpls_t* p_nh_info = NULL;
    sys_nh_ref_list_node_t* p_curr = NULL;
    sys_nh_db_mpls_tunnel_t* p_mpls_tunnel = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_NH_INIT_CHECK;

    SYS_NH_LOCK;
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_nh_lkup_mpls_tunnel(lchip, tunnel_id, &p_mpls_tunnel), p_usw_nh_api_master[lchip]->p_mutex);
    if (!p_mpls_tunnel)
    {
        SYS_NH_UNLOCK;
        return CTC_E_NOT_EXIST;
    }
    p_curr = p_mpls_tunnel->p_ref_nh_list;
    while (p_curr)
    {
        p_nh_info = (sys_nh_info_mpls_t*)p_curr->p_ref_nhinfo;
        if (cb)
        {
            cb(lchip, p_nh_info, user_data);
        }
        p_curr = p_curr->p_next;
    }

    SYS_NH_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_nexthop_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;

    SYS_NH_INIT_CHECK;
    SYS_NH_LOCK;
    ret = _sys_usw_nexthop_dump_db(lchip, p_f,p_dump_param);
    SYS_NH_UNLOCK;

    return ret;
}

