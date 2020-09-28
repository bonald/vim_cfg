/**
 @file sys_usw_nexthop_l2.c

 @date 2009-09-19

 @version v2.0

 The file contains all nexthop layer2 related callback function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_linklist.h"
#include "sys_usw_chip.h"
#include "sys_usw_register.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_aps.h"
#include "sys_usw_qos_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

STATIC  int32
_sys_usw_nh_brguc_write_dsnh_with_vlan_edit(uint8 lchip,  uint8 gchip,
                                                  uint16 dest_vlan_ptr,  uint8 dsnh_type,
                                                  ctc_vlan_egress_edit_info_t* p_vlan_edit_info,
                                                  sys_nh_info_brguc_t* p_nhdb_ucast_brg,
                                                  bool is_protection_path,
                                                  ctc_vlan_edit_nh_param_t* p_vlan_param)
{
    sys_nh_param_dsnh_t dsnh_param;
    int32 ret = 0;
    uint8 xgpon_en = 0;

    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));

    if (p_vlan_edit_info && CTC_FLAG_ISSET(p_vlan_edit_info->flag, CTC_VLAN_NH_HORIZON_SPLIT_EN)
        && CTC_FLAG_ISSET(p_vlan_edit_info->flag, CTC_VLAN_NH_PASS_THROUGH))
    {
        return CTC_E_INVALID_CONFIG;
    }

    if (p_vlan_param && p_vlan_param->logic_port && !is_protection_path)
    {
        CTC_MAX_VALUE_CHECK(p_vlan_param->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));
    }
    if (p_vlan_edit_info)
    {
        if (p_vlan_edit_info->user_vlanptr == 0xffff)
        {
            dsnh_param.dest_vlan_ptr = (DRV_IS_DUET2(lchip))?0x1800:0x1000;
        }
        else if (p_vlan_edit_info->user_vlanptr == 0)
        {
            dsnh_param.dest_vlan_ptr = 0;
        }
        else
        {
            dsnh_param.dest_vlan_ptr = p_vlan_edit_info->user_vlanptr;
        }
    }
    dsnh_param.dsnh_type = dsnh_type;
    dsnh_param.p_vlan_info = p_vlan_edit_info;
    dsnh_param.cid = p_vlan_param?p_vlan_param->cid:0;

    if (p_nhdb_ucast_brg->loop_nhid)
    {
        CTC_ERROR_RETURN(sys_usw_nh_add_loopback_l2edit(lchip, p_nhdb_ucast_brg->loop_nhid, 1, &dsnh_param.l2edit_ptr));
    }

    if (CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE))
    {
        if (SYS_GCHIP_IS_REMOTE(lchip, gchip)
            && (SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT != p_nhdb_ucast_brg->nh_sub_type))
        {
            return CTC_E_NONE;
        }
    }

    if (is_protection_path)
    {
        dsnh_param.dsnh_offset = p_nhdb_ucast_brg->hdr.dsfwd_info.dsnh_offset + 1;
        CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param), ret, error);
    }
    else
    {
        dsnh_param.dsnh_offset = p_nhdb_ucast_brg->hdr.dsfwd_info.dsnh_offset;
        dsnh_param.logic_port_check = p_vlan_param?p_vlan_param->logic_port_check:0;

        if (dsnh_param.logic_port_check)
        {
            CTC_SET_FLAG(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_PORT_CHECK);
        }

        sys_usw_global_get_xgpon_en(lchip, &xgpon_en);
        if (p_vlan_param && p_vlan_param->logic_port)
        {
            if (!xgpon_en || !DRV_IS_DUET2(lchip))
            {
                dsnh_param.logic_port               = p_vlan_param->logic_port;
                p_nhdb_ucast_brg->dest_logic_port   = p_vlan_param->logic_port;

                CTC_SET_FLAG(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOGIC_PORT);
            }
            else
            {
                dsnh_param.l3edit_ptr = p_vlan_param->logic_port;
            }

            if (p_vlan_param->service_id)
            {
                /*Call QoS for binding service with logic Port*/
                 CTC_ERROR_GOTO(sys_usw_qos_bind_service_logic_dstport(lchip, p_vlan_param->service_id,  p_vlan_param->logic_port), ret, error);
				 p_nhdb_ucast_brg->service_id = p_vlan_param->service_id;
            }
        }

        if (CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
        {
            CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param), ret, error);
        }
        else
        {
            CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param), ret, error);
        }
    }

    return CTC_E_NONE;

error:
    if (p_nhdb_ucast_brg->loop_nhid)
    {
        sys_usw_nh_remove_loopback_l2edit(lchip, p_nhdb_ucast_brg->loop_nhid, 1);
    }
    return ret;
}

/**
 @brief the subfunction of create unicast APS bridge nexthop

 @return CTC_E_XXX
 */
int32
_sys_usw_nh_brguc_aps_process(uint8 lchip, sys_nh_param_brguc_t* p_nh_para_bridge,
                                    sys_nh_info_brguc_t* p_nhdb_ucast_brg)
{
    uint8  gchip = 0;
    uint32 wroking_gport = 0;
    uint32 protect_gport = 0;
    uint16 aps_bridge_group_id = 0;

    CTC_PTR_VALID_CHECK(p_nh_para_bridge->p_vlan_edit_info);
    CTC_PTR_VALID_CHECK(p_nh_para_bridge->p_vlan_edit_info_prot_path);

    /*1.get working port & protection port by aps group id*/
    aps_bridge_group_id = p_nh_para_bridge->gport;

    CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, aps_bridge_group_id, &wroking_gport, TRUE));
    CTC_ERROR_RETURN(sys_usw_aps_get_bridge_path(lchip, aps_bridge_group_id, &protect_gport, FALSE));

    if ((CTC_MAP_GPORT_TO_GCHIP(wroking_gport) != CTC_MAP_GPORT_TO_GCHIP(protect_gport ))
        || (CTC_MAP_GPORT_TO_GCHIP(wroking_gport) == 0x1F)
        || (CTC_MAP_GPORT_TO_GCHIP(protect_gport) == 0x1F))
    {
        gchip = 0x1F;
    }
    else
    {
        gchip = CTC_MAP_GPORT_TO_GCHIP(wroking_gport);
    }



    CTC_ERROR_RETURN(_sys_usw_nh_brguc_write_dsnh_with_vlan_edit(lchip,
                                                                       gchip,
                                                                       SYS_DSNH_DESTVLANPTR_SPECIAL,
                                                                       SYS_NH_PARAM_DSNH_TYPE_BRGUC,
                                                                       p_nh_para_bridge->p_vlan_edit_info,
                                                                       p_nhdb_ucast_brg, FALSE,
                                                                       p_nh_para_bridge->p_vlan_edit_nh_param));

    if (!CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_SHARE_DSNH))
    {

        CTC_ERROR_RETURN(_sys_usw_nh_brguc_write_dsnh_with_vlan_edit(lchip,
                                                                           gchip,
                                                                           SYS_DSNH_DESTVLANPTR_SPECIAL,
                                                                           SYS_NH_PARAM_DSNH_TYPE_BRGUC,
                                                                           p_nh_para_bridge->p_vlan_edit_info_prot_path,
                                                                           p_nhdb_ucast_brg, TRUE,
                                                                           p_nh_para_bridge->p_vlan_edit_nh_param));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_brguc_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_dsfwd_t dsfwd_param;
   sys_nh_info_brguc_t* p_nhinfo = (sys_nh_info_brguc_t*)(p_com_db);
   uint8 gchip = 0;
   int ret = 0;

   sal_memset(&dsfwd_param,0,sizeof(sys_nh_param_dsfwd_t));

   if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
   {
       ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,  &(p_nhinfo->hdr.dsfwd_info.dsfwd_offset));
   }
   if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN))
   {
       dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nhinfo->dest_gport);
       dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nhinfo->dest_gport);
   }
   else
   {
       sys_usw_get_gchip_id(lchip, &gchip);
       dsfwd_param.dest_chipid = gchip;
       dsfwd_param.dest_id = (p_nhinfo->dest_gport);
       dsfwd_param.aps_type = CTC_APS_BRIDGE;
   }
   dsfwd_param.is_mcast = 0;
   dsfwd_param.drop_pkt  = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);

   dsfwd_param.dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
   dsfwd_param.nexthop_ext = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
   dsfwd_param.is_egress_edit = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);
   dsfwd_param.dsfwd_offset  = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
   if (!DRV_IS_DUET2(lchip))
   {
       /*for vpwsfiden use dsfwd full, cannot use half*/
       dsfwd_param.is_6w = 1;
   }
   /*Write table*/
   ret = ret ? ret : sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param);


    if (ret != CTC_E_NONE)
    {
       sys_usw_nh_offset_free(lchip,  SYS_NH_ENTRY_TYPE_FWD, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset);
    }
    else
    {
        CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }
	return ret;
}
/**
 @brief Callback function of create unicast bridge nexthop

 @return CTC_E_XXX
 */
int32
sys_usw_nh_create_brguc_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_brguc_t* p_nh_para_bridge;
    sys_nh_info_brguc_t* p_nhdb_ucast_brg;

    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_BRGUC, p_com_nh_para->hdr.nh_param_type);
    p_nh_para_bridge = (sys_nh_param_brguc_t*)p_com_nh_para;
    p_nhdb_ucast_brg = (sys_nh_info_brguc_t*)p_com_db;
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    sal_memset(&dsfwd_param.dsfwd_offset, 0xFF, sizeof(dsfwd_param.dsfwd_offset));


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "nhType = %d, NHID = %d, isInternaNH = %d, \n\
        GlobalPort = %d, DsNHOffset = %d\n",
                   p_nh_para_bridge->hdr.nh_param_type,
                   p_nh_para_bridge->hdr.nhid,
                   p_nh_para_bridge->hdr.is_internal_nh,
                   p_nh_para_bridge->gport,
                   p_nh_para_bridge->dsnh_offset);

    p_nhdb_ucast_brg->hdr.nh_entry_type = SYS_NH_TYPE_BRGUC;
    p_nhdb_ucast_brg->dest_gport = p_nh_para_bridge->gport;
    p_nhdb_ucast_brg->loop_nhid = p_nh_para_bridge->loop_nhid;

    if (!CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSNH))
    {
        if (p_nh_para_bridge->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT)
        {

            if (CTC_FLAG_ISSET(p_nh_para_bridge->p_vlan_edit_info->flag, CTC_VLAN_NH_STATS_EN) &&
                p_nh_para_bridge->p_vlan_edit_nh_param->cid && !CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
            {
                return CTC_E_INVALID_CONFIG;
            }

            CTC_ERROR_RETURN(_sys_usw_nh_brguc_write_dsnh_with_vlan_edit(lchip,
                                                                               CTC_MAP_GPORT_TO_GCHIP(p_nh_para_bridge->gport),
                                                                               SYS_DSNH_DESTVLANPTR_SPECIAL,
                                                                               SYS_NH_PARAM_DSNH_TYPE_BRGUC,
                                                                               p_nh_para_bridge->p_vlan_edit_info,
                                                                               p_nhdb_ucast_brg, FALSE,
                                                                               p_nh_para_bridge->p_vlan_edit_nh_param));

        }
        else if (p_nh_para_bridge->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT)
        {

            CTC_ERROR_RETURN(_sys_usw_nh_brguc_aps_process(lchip, p_nh_para_bridge, p_nhdb_ucast_brg));
        }
    }

    if (p_nh_para_bridge->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_APS_VLAN_EDIT)
    {
      CTC_SET_FLAG(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_APS_EN);
    }

    if (!p_nh_para_bridge->hdr.have_dsfwd)
    {
        return ret ;
    }
    _sys_usw_nh_brguc_add_dsfwd(lchip, (sys_nh_info_com_t*) p_nhdb_ucast_brg);

	 p_nh_para_bridge->hdr.dsfwd_offset = p_nhdb_ucast_brg->hdr.dsfwd_info.dsfwd_offset;


    return CTC_E_NONE;
}

/**
 @brief Callback function of delete unicast bridge nexthop

 @param[in] p_com_db, pointer used to store nexthop data

 @return CTC_E_XXX
 */
int32
sys_usw_nh_delete_brguc_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{
    sys_nh_info_brguc_t* nh_brg_ucast;


    sys_nh_param_dsfwd_t dsfwd_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_BRGUC, p_data->hdr.nh_entry_type);
    nh_brg_ucast = (sys_nh_info_brguc_t*)(p_data);

    /*2. Free DsFwd offset*/
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));


    if (nh_brg_ucast->loop_nhid)
    {
        CTC_ERROR_RETURN(sys_usw_nh_remove_loopback_l2edit(lchip, nh_brg_ucast->loop_nhid, 1));
    }

    if (nh_brg_ucast->service_id)
    {
        /*Call QoS for unbinding service with logic Port*/
        CTC_ERROR_RETURN(sys_usw_qos_unbind_service_logic_dstport(lchip, nh_brg_ucast->service_id,  nh_brg_ucast->dest_logic_port));
    }

     if (CTC_FLAG_ISSET(nh_brg_ucast->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
     {
            CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
                                                          SYS_NH_ENTRY_TYPE_FWD, 1, nh_brg_ucast->hdr.dsfwd_info.dsfwd_offset));

            dsfwd_param.drop_pkt  = TRUE;
            dsfwd_param.dsfwd_offset = nh_brg_ucast->hdr.dsfwd_info.dsfwd_offset;
             /*Write table*/
            CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
     }

    return CTC_E_NONE;
}


int32
sys_usw_nh_update_brguc_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                  sys_nh_param_com_t* p_para)
{

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_BRGUC, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_BRGUC, p_nh_db->hdr.nh_entry_type);
    switch (p_para->hdr.change_type)
    {
    case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
        {

           if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
            {
                return CTC_E_NONE;
            }
            return _sys_usw_nh_brguc_add_dsfwd(lchip, p_nh_db);
        }
    	break;
    case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
         {
            sys_nh_param_brguc_t* p_nh_para_bridge = NULL;
            sys_nh_info_brguc_t* p_nhdb_ucast_brg = NULL;
            p_nh_para_bridge = (sys_nh_param_brguc_t*)p_para;
            p_nhdb_ucast_brg = (sys_nh_info_brguc_t*)p_nh_db;
            if (p_nh_para_bridge->nh_sub_type == SYS_NH_PARAM_BRGUC_SUB_TYPE_VLAN_EDIT)
            {

                if (CTC_FLAG_ISSET(p_nh_para_bridge->p_vlan_edit_info->flag, CTC_VLAN_NH_STATS_EN) &&
                    p_nh_para_bridge->p_vlan_edit_nh_param->cid && !CTC_FLAG_ISSET(p_nhdb_ucast_brg->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
                {
                    return CTC_E_INVALID_CONFIG;
                }

                CTC_ERROR_RETURN(_sys_usw_nh_brguc_write_dsnh_with_vlan_edit(lchip,
                                                                                   CTC_MAP_GPORT_TO_GCHIP(p_nh_para_bridge->gport),
                                                                                   SYS_DSNH_DESTVLANPTR_SPECIAL,
                                                                                   SYS_NH_PARAM_DSNH_TYPE_BRGUC,
                                                                                   p_nh_para_bridge->p_vlan_edit_info,
                                                                                   p_nhdb_ucast_brg, FALSE,
                                                                                   p_nh_para_bridge->p_vlan_edit_nh_param));

            }
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

  return CTC_E_NONE;
}

