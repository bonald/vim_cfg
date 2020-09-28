/**
 @file sys_usw_nexthop_trill.c

 @date 2015-10-25

 @version v3.0

 The file contains all nexthop trill related callback function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_opf.h"
#include "sys_usw_l3if.h"
#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

STATIC int32
_sys_usw_nh_trill_build_dsl3edit(uint8 lchip, ctc_nh_trill_param_t* p_nh_param,
                                              sys_nh_info_trill_t* p_edit_db,
                                              sys_nh_param_dsnh_t* dsnh)
{
    sys_dsl3edit_trill_t  dsl3edit4w;
    int32 ret = CTC_E_NONE;

    sal_memset(&dsl3edit4w, 0, sizeof(dsl3edit4w));
    dsl3edit4w.l3_rewrite_type = SYS_NH_L3EDIT_TYPE_TRILL;
    dsl3edit4w.ds_type   = SYS_NH_DS_TYPE_L3EDIT;

    dsl3edit4w.ingress_nickname = p_nh_param->ingress_nickname;
    dsl3edit4w.egress_nickname = p_nh_param->egress_nickname;
    dsl3edit4w.ttl = p_nh_param->ttl;

    if (CTC_FLAG_ISSET(p_nh_param->flag, CTC_NH_TRILL_MTU_CHECK))
    {
        dsl3edit4w.mtu_check_en = 1;
        dsl3edit4w.mtu_size  = p_nh_param->mtu_size;
    }

    /*stats*/
    if (p_nh_param->stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_nh_param->stats_id,
                                                           &(dsl3edit4w.stats_ptr)));
    }


    /*1. Allocate new dsl3edit trill entry*/
    ret = sys_usw_nh_offset_alloc(lchip,
                                        SYS_NH_ENTRY_TYPE_L3EDIT_TRILL, 1,
                                        &dsnh->l3edit_ptr);

    /*2. Write HW table*/
    ret = sys_usw_nh_write_asic_table(lchip,
                                            SYS_NH_ENTRY_TYPE_L3EDIT_TRILL,
                                            dsnh->l3edit_ptr, &dsl3edit4w);

    if (ret)
    {
        sys_usw_nh_offset_free(lchip,
                                     SYS_NH_ENTRY_TYPE_L3EDIT_TRILL, 1,
                                     dsnh->l3edit_ptr);
        return ret;
    }

    p_edit_db->dsl3edit_offset = dsnh->l3edit_ptr;
    p_edit_db->egress_nickname = p_nh_param->egress_nickname;
    p_edit_db->ingress_nickname = p_nh_param->ingress_nickname;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_trill_build_dsl2edit(uint8 lchip, ctc_nh_trill_param_t* p_nh_param,
                                           sys_nh_info_trill_t* p_edit_db,
                                           sys_nh_param_dsnh_t* dsnh)
{
    sys_nh_db_dsl2editeth4w_t   dsl2edit;
    sys_nh_db_dsl2editeth4w_t* p_dsl2edit;
    mac_addr_t mac;

    sal_memset(&dsl2edit, 0 , sizeof(sys_nh_db_dsl2editeth4w_t));
    sal_memset(&mac, 0 , sizeof(mac_addr_t));

    /*outer l2 edit*/
    sal_memcpy(dsl2edit.mac_da, p_nh_param->mac, sizeof(mac_addr_t));
    dsl2edit.output_vid =  p_nh_param->oif.vid;
    p_dsl2edit = &dsl2edit;
    CTC_ERROR_RETURN(sys_usw_nh_add_route_l2edit_outer(lchip, (sys_nh_db_dsl2editeth4w_t**)&p_dsl2edit, &dsnh->l2edit_ptr));
    p_edit_db->p_dsl2edit_4w = p_dsl2edit;
    p_edit_db->l2_edit_ptr = dsnh->l2edit_ptr;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_free_trill_resource(uint8 lchip, sys_nh_info_trill_t* p_nhinfo)
{
    uint8 gchip = 0;
    uint8 lchip2 = 0;
    uint8 l3edit_type = 0;
    sys_nh_db_dsl2editeth4w_t  dsl2edit;

    sal_memset(&dsl2edit, 0, sizeof(sys_nh_db_dsl2editeth4w_t));

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nhinfo->gport);
    CTC_ERROR_RETURN(sys_usw_get_local_chip_id(gchip, &lchip2));

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        l3edit_type = SYS_NH_ENTRY_TYPE_L3EDIT_TRILL;
        sys_usw_nh_offset_free(lchip, l3edit_type, 1, p_nhinfo->dsl3edit_offset);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
    {
        sys_usw_nh_remove_route_l2edit_outer(lchip, p_nhinfo->p_dsl2edit_4w);
    }

    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    CTC_UNSET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_trill_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
    sys_nh_info_trill_t* p_nh_info = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    int32 ret = CTC_E_NONE;

    p_nh_info     = (sys_nh_info_trill_t*)(p_com_db);

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nh_info->gport);
    dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nh_info->gport);

    /*Build DsFwd Table*/
    if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                             &(p_nh_info->hdr.dsfwd_info.dsfwd_offset));
    }
    dsfwd_param.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.dsnh_offset = p_nh_info->hdr.dsfwd_info.dsnh_offset;
    dsfwd_param.is_egress_edit =  CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);

     /*Write table*/
    ret = ret ? ret : sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param);
    CTC_SET_FLAG(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);

    if (ret != CTC_E_NONE)
    {
      sys_usw_nh_offset_free(lchip,  SYS_NH_ENTRY_TYPE_FWD, 1, p_nh_info->hdr.dsfwd_info.dsfwd_offset);

    }
    else
    {
        CTC_SET_FLAG(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsFwd(Lchip : %d) :: DestChipId = %d, DestId = %d"
                                         "DsNexthop Offset = %d, DsNexthopExt = %d\n", 0,
                                          dsfwd_param.dest_chipid, dsfwd_param.dest_id,
                                          dsfwd_param.dsnh_offset, dsfwd_param.nexthop_ext);

    return ret;
}

STATIC int32
_sys_usw_nh_trill_build_dsnh(uint8 lchip, ctc_nh_trill_param_t* p_trill_nh_param,
                                       sys_nh_info_trill_t* p_edit_db,
                                       sys_nh_param_dsfwd_t * p_fwd_info)
{
    uint8 gchip, lchip2;
    sys_nh_param_dsnh_t dsnh_param;
    sys_l3if_prop_t l3if_prop;
    uint32 gport = 0;
    int32 ret = CTC_E_NONE;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_trill_nh_param);
    CTC_PTR_VALID_CHECK(p_edit_db);

    lchip2 = 0;
    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));
    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_trill_nh_param->oif.gport);
    CTC_ERROR_RETURN(sys_usw_get_local_chip_id(gchip, &lchip2));
    gport = p_trill_nh_param->oif.gport;
    if (!(CTC_IS_CPU_PORT(gport) || p_trill_nh_param->oif.is_l2_port))
    {
        CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, gport, p_trill_nh_param->oif.vid, &l3if_prop))
        p_edit_db->l3ifid  = l3if_prop.l3if_id;
    }
    else
    {
        l3if_prop.vlan_ptr = 0xFFFF;
    }
    p_edit_db->gport = gport;

    dsnh_param.dest_vlan_ptr = l3if_prop.vlan_ptr;
    p_edit_db->dest_vlan_ptr = dsnh_param.dest_vlan_ptr;
    CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HORIZON_SPLIT_EN);

    /*1. Build dsL2Edit*/
    ret = _sys_usw_nh_trill_build_dsl2edit(lchip, p_trill_nh_param, p_edit_db, &dsnh_param);
    if (ret)
    {
       goto error_proc;
    }

    if (dsnh_param.l2edit_ptr != 0)
    {
        CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
    }

    /*2. Build dsL3Edit*/
     ret = _sys_usw_nh_trill_build_dsl3edit(lchip, p_trill_nh_param, p_edit_db, &dsnh_param);
    if (ret)
    {
       goto error_proc;
    }

    if (dsnh_param.l3edit_ptr != 0)
    {
        CTC_SET_FLAG(p_edit_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    }
    dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_TRILL;

    /*3. Write nexthop*/
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Lchip = %d, Write DsNexthop4w Table\n", lchip);
    dsnh_param.dsnh_offset = p_edit_db->hdr.dsfwd_info.dsnh_offset;
    ret = sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param);

    if (ret)
    {
       goto error_proc;
    }

   return CTC_E_NONE;
error_proc:
    _sys_usw_nh_free_trill_resource(lchip, p_edit_db);
     return ret;

}


STATIC int32
_sys_usw_nh_update_trill_fwd_to_spec(uint8 lchip, sys_nh_param_trill_t* p_nhpara,
                                               sys_nh_info_trill_t* p_nhinfo)
{
    sys_nh_param_special_t nh_para_spec;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&nh_para_spec, 0, sizeof(sys_nh_param_special_t));

    nh_para_spec.hdr.have_dsfwd = TRUE;
    nh_para_spec.hdr.nh_param_type = SYS_NH_TYPE_UNROV;
    nh_para_spec.hdr.is_internal_nh = TRUE;

   /*1.2 update dsfwd to unrov nh's dsfwd, write dsfwd*/
    CTC_ERROR_RETURN(sys_usw_nh_create_special_cb(lchip, (
                                                            sys_nh_param_com_t*)(&nh_para_spec), (sys_nh_info_com_t*)(p_nhinfo)));
    CTC_SET_FLAG(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);

    _sys_usw_nh_free_trill_resource(lchip, p_nhinfo);
    _sys_usw_nh_free_offset_by_nhinfo(lchip, SYS_NH_TYPE_TRILL, (sys_nh_info_com_t*)p_nhinfo);

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_nh_update_trill_fwd_attr(uint8 lchip, sys_nh_param_trill_t* p_nhpara,
                                            sys_nh_info_trill_t* p_nhinfo)
{
    int32 ret = 0;
    uint8 is_update = (p_nhpara->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);
    sys_nh_info_trill_t old_nh_info;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);


     if(is_update)
     {
         /*store nh old data, for recover*/
         sal_memcpy(&old_nh_info, p_nhinfo, sizeof(sys_nh_info_trill_t));
         sal_memset(p_nhinfo, 0, sizeof(sys_nh_info_trill_t));
         sys_usw_nh_copy_nh_entry_flags(lchip, &old_nh_info.hdr, &p_nhinfo->hdr);
         p_nhinfo->updateAd = old_nh_info.updateAd;
         p_nhinfo->data = old_nh_info.data;

     }

    CTC_ERROR_GOTO(sys_usw_nh_create_trill_cb(lchip, (sys_nh_param_com_t*)p_nhpara,
        (sys_nh_info_com_t*)p_nhinfo), ret, error);

    /*for update free resource here*/
    if(is_update)
    {
       _sys_usw_nh_free_trill_resource(lchip, &old_nh_info);
    }
    return CTC_E_NONE;
error:
   if(is_update)
   {
       sal_memcpy( p_nhinfo, &old_nh_info,sizeof(sys_nh_info_trill_t));
   }
    return ret;
}

int32
sys_usw_nh_create_trill_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_trill_t* p_nh_param = NULL;
    ctc_nh_trill_param_t* p_nh_trill_param = NULL;
    sys_nh_info_trill_t* p_nhdb = NULL;
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_nh_param_special_t nh_para_spec;
    int ret = 0;

    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db)
    CTC_EQUAL_CHECK(SYS_NH_TYPE_TRILL, p_com_nh_para->hdr.nh_param_type);
    p_nh_param = (sys_nh_param_trill_t*)(p_com_nh_para);
    p_nh_trill_param = p_nh_param->p_trill_nh_param;
    p_nhdb     = (sys_nh_info_trill_t*)(p_com_db);
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_TRILL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "flag = 0x%x\n", p_nh_trill_param->flag);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "upd_type = 0x%x\n", p_nh_trill_param->upd_type);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport = 0x%x\n", p_nh_trill_param->oif.gport);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vid = %d\n", p_nh_trill_param->oif.vid);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "is_l2_port = %d\n", p_nh_trill_param->oif.is_l2_port);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ecmp_if_id = %d\n", p_nh_trill_param->oif.ecmp_if_id);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dsnh_offset = %d\n", p_nh_trill_param->dsnh_offset);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ingress_nickname = %d\n", p_nh_trill_param->ingress_nickname);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "egress_nickname = %d\n", p_nh_trill_param->egress_nickname);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ttl = %d\n", p_nh_trill_param->ttl);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mtu_size = %d\n", p_nh_trill_param->mtu_size);
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "stats_id = %d\n", p_nh_trill_param->stats_id);

    if (p_nh_trill_param->ttl > 63) /*trill hop count*/
    {
        return CTC_E_INVALID_PARAM;
    }

    /*1.Create unresolved trilll nh*/
    if (CTC_FLAG_ISSET(p_nh_trill_param->flag, CTC_NH_TRILL_IS_UNRSV))
    {
        sal_memset(&nh_para_spec, 0, sizeof(nh_para_spec));
        nh_para_spec.hdr.nh_param_type = SYS_NH_TYPE_UNROV;
        nh_para_spec.hdr.is_internal_nh = TRUE;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Create unresolved trill tunnel nexthop\n");

        CTC_ERROR_RETURN(sys_usw_nh_create_special_cb(lchip, (
                                                                sys_nh_param_com_t*)(&nh_para_spec), (sys_nh_info_com_t*)(p_nhdb)));

        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV);
        return CTC_E_NONE;
    }

    /*2.Create normal trill-tunnel nh*/
    /*2.1 op dsnh,l2edit,l3edit*/
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.nexthop_ext = 0;
    CTC_ERROR_GOTO(_sys_usw_nh_trill_build_dsnh(lchip, p_nh_trill_param, p_nhdb,&dsfwd_param),ret,error_proc);

     /*3.Create Dwfwd*/
    if(p_nh_param->hdr.have_dsfwd)
    {
      CTC_ERROR_GOTO(_sys_usw_nh_trill_add_dsfwd(lchip,  p_com_db),ret,error_proc);
    }
    return CTC_E_NONE;
error_proc:
    _sys_usw_nh_free_trill_resource(lchip, p_nhdb);
    return ret;
}

int32
sys_usw_nh_delete_trill_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{
    uint32 entry_type = SYS_NH_TYPE_TRILL;
    sys_nh_param_dsfwd_t    dsfwd_param =    { 0};
    sys_nh_info_trill_t* p_nhinfo   = NULL;
    sys_nh_ref_list_node_t* p_ref_node, * p_tmp_node;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_EQUAL_CHECK(entry_type, p_data->hdr.nh_entry_type);
    p_nhinfo = (sys_nh_info_trill_t*)(p_data);

    sys_usw_nh_update_ecmp_member(lchip, *p_nhid, SYS_NH_CHANGE_TYPE_NH_DELETE);

    /*1. delete all reference ecmp nh*/
    p_ref_node = p_nhinfo->hdr.p_ref_nh_list;

    while (p_ref_node)
    {
        /*Remove from db*/
        p_tmp_node = p_ref_node;
        p_ref_node = p_ref_node->p_next;
        mem_free(p_tmp_node);
    }
    p_nhinfo->hdr.p_ref_nh_list = NULL;

    /*2. Delete dsfwd*/
    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        sal_memset(&dsfwd_param, 0, sizeof(dsfwd_param));
        dsfwd_param.dsfwd_offset = p_nhinfo->hdr.dsfwd_info.dsfwd_offset;
        dsfwd_param.drop_pkt = 1;
        /*Write table*/
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
        /*Free DsFwd offset*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
        SYS_NH_ENTRY_TYPE_FWD, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset));
    }

    _sys_usw_nh_free_trill_resource(lchip, p_nhinfo);
    return CTC_E_NONE;
}

int32
sys_usw_nh_update_trill_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para)
{
    sys_nh_info_trill_t* p_nh_info;
    sys_nh_param_trill_t* p_nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_TRILL, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_TRILL, p_nh_db->hdr.nh_entry_type);

    p_nh_info = (sys_nh_info_trill_t*)(p_nh_db);
    p_nh_para = (sys_nh_param_trill_t*)(p_para);

    switch (p_nh_para->hdr.change_type)
    {
    case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
        CTC_ERROR_RETURN(_sys_usw_nh_update_trill_fwd_to_spec(lchip, p_nh_para, p_nh_info));
        break;

    case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
       p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_trill_fwd_attr(lchip, p_nh_para, p_nh_info));
        break;

    case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:
        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Current nexthop isnt unresolved nexthop \n");
			return CTC_E_INVALID_CONFIG;

        }
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_trill_fwd_attr(lchip, p_nh_para, p_nh_info));
        break;
    case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
        {
            if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
            {
                return CTC_E_NONE;
            }
            CTC_ERROR_RETURN(_sys_usw_nh_trill_add_dsfwd(lchip,  p_nh_db));
        }
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    if (p_nh_info->hdr.p_ref_nh_list)
    {
        int32 ret = 0;
        ret = (sys_usw_nh_update_ecmp_member(lchip, p_nh_para->hdr.nhid, p_nh_para->hdr.change_type));
        if (ret)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop update for ecmp fail!, ret:%d \n", ret);
        }
    }

    return CTC_E_NONE;
}


