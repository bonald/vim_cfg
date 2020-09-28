/**
 @file sys_usw_nexthop_misc.c

 @date 2009-11-25

 @version v2.0

 The file contains all non-layer2 and non-layer3 nexthop related callback function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_spool.h"
#include "ctc_linklist.h"
#include "ctc_packet.h"

#include "sys_usw_chip.h"
#include "sys_usw_vlan.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_l3if.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_register.h"
#include "sys_usw_stats_api.h"
#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define SYS_NH_MISC_FLAG_CONFLICT_CHECK(flag) \
    { \
        if (flag & CTC_MISC_NH_L2_EDIT_SWAP_MAC) \
        { \
            if ((flag & ~CTC_MISC_NH_L2_EDIT_SWAP_MAC) > 0) \
            { \
                return CTC_E_INVALID_PARAM; \
            } \
        } \
    }

extern int32
sys_usw_nh_write_asic_table(uint8 lchip,
                                  uint8 table_type, uint32 offset, void* value);   /*TBD*/
extern int32
sys_usw_lkup_ttl_index(uint8 lchip, uint8 ttl, uint32* ttl_index);
/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
 *
* Function
*
*****************************************************************************/

STATIC int32
_sys_usw_nh_misc_map_vlan_tag_op_translate(uint8 op_in, uint8* op_out, uint8* mode_out)
{
    if (CTC_VLAN_TAG_OP_REP_OR_ADD == op_in)
    {
        *mode_out = 1;
    }
    else
    {
        *mode_out = 0;
    }

    switch (op_in)
    {
    case CTC_VLAN_TAG_OP_NONE:
    case CTC_VLAN_TAG_OP_VALID:
        *op_out = 0;
        break;
    case CTC_VLAN_TAG_OP_REP:
    case CTC_VLAN_TAG_OP_REP_OR_ADD:
        *op_out = 1;
        break;
    case CTC_VLAN_TAG_OP_ADD:
        *op_out = 2;
        break;
    case CTC_VLAN_TAG_OP_DEL:
        *op_out = 3;
        break;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_misc_encoding_vlan_profile(uint8 lchip, ctc_misc_nh_flex_edit_param_t* p_para, DsOfEditVlanActionProfile_m* p_ds)
{

    uint8 tag_action = 0;
    uint8 tag_mode = 0;

    SetDsOfEditVlanActionProfile(V, cCfiAction_f, p_ds, 0);
    SetDsOfEditVlanActionProfile(V, cCosAction_f, p_ds,  p_para->ccos_sl);
    _sys_usw_nh_misc_map_vlan_tag_op_translate(p_para->ctag_op, &tag_action, &tag_mode);
    SetDsOfEditVlanActionProfile(V, cTagAction_f, p_ds,  tag_action);
    SetDsOfEditVlanActionProfile(V, cTagModifyMode_f , p_ds, tag_mode);
    SetDsOfEditVlanActionProfile(V, cVlanIdAction_f , p_ds,  p_para->cvid_sl);

    SetDsOfEditVlanActionProfile(V, sCfiAction_f, p_ds,  0);
    SetDsOfEditVlanActionProfile(V, sCosAction_f, p_ds, p_para->scos_sl);
    _sys_usw_nh_misc_map_vlan_tag_op_translate(p_para->stag_op, &tag_action, &tag_mode);
    SetDsOfEditVlanActionProfile(V, sTagAction_f, p_ds,  tag_action);
    SetDsOfEditVlanActionProfile(V, sTagModifyMode_f , p_ds, tag_mode);
    SetDsOfEditVlanActionProfile(V, sVlanIdAction_f , p_ds, p_para->svid_sl);

    if ((CTC_VLAN_TAG_OP_ADD == p_para->ctag_op) && (CTC_VLAN_TAG_OP_ADD == p_para->stag_op))
    {
        SetDsOfEditVlanActionProfile(V, cTagAddMode_f, p_ds,  1);  /*add cvlan before raw packet vlan*/
    }
    else
    {
        SetDsOfEditVlanActionProfile(V, cTagAddMode_f, p_ds,  0);
    }

    SetDsOfEditVlanActionProfile(V, sVlanTpidIndexEn_f, p_ds, p_para->new_stpid_en);
    SetDsOfEditVlanActionProfile(V, sVlanTpidIndex_f, p_ds, p_para->new_stpid_idx);

    return CTC_E_NONE;

}

/**
 @brief Callback function of create unicast bridge nexthop

 @param[in,out] p_com_nh_para, parameters used to create bridge nexthop,
                writeback dsfwd offset array to this param

 @param[out] p_com_db, pointer used to store nexthop data

 @return CTC_E_XXX
 */
int32
sys_usw_nh_create_special_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_special_t* p_nh_para;
    sys_nh_info_special_t* p_nhdb;


    sys_nh_param_dsfwd_t dsfwd_param;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);
    p_nh_para = (sys_nh_param_special_t*)p_com_nh_para;
    p_nhdb = (sys_nh_info_special_t*)p_com_db;
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    switch (p_nh_para->hdr.nh_param_type)
    {
    case SYS_NH_TYPE_UNROV:
    case SYS_NH_TYPE_DROP:
        dsfwd_param.drop_pkt = TRUE;
        break;

    case SYS_NH_TYPE_TOCPU:
        dsfwd_param.is_lcpu = TRUE;
        break;

    default:
        CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
    }

    p_nhdb->hdr.nh_entry_type =  \
        (p_nhdb->hdr.nh_entry_type == SYS_NH_TYPE_NULL) ? (p_nh_para->hdr.nh_param_type) : p_nhdb->hdr.nh_entry_type;



   /*Build DsFwd Table*/
    if(!CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                                       &p_nhdb->hdr.dsfwd_info.dsfwd_offset));

    }
    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);

    sys_usw_get_gchip_id(lchip, &dsfwd_param.dest_chipid);
    /*Writeback dsfwd offset*/
    dsfwd_param.dsfwd_offset =  p_nhdb->hdr.dsfwd_info.dsfwd_offset;

    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.is_egress_edit =  CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_EGRESS_EDIT_MODE);


    if(SYS_NH_TYPE_TOCPU == p_nh_para->hdr.nh_param_type)
    {
        dsfwd_param.dsnh_offset = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_FWD_CPU, 0);
        p_nhdb->hdr.dsfwd_info.dsnh_offset =  dsfwd_param.dsnh_offset;
    }

    /*Write table*/
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));


    return CTC_E_NONE;
}

int32
sys_usw_nh_delete_special_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{
    sys_nh_info_special_t* nh_info;
    sys_nh_param_dsfwd_t dsfwd_param;


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    nh_info = (sys_nh_info_special_t*)(p_data);


    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    /*Free dsfwd offset*/

    CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
    SYS_NH_ENTRY_TYPE_FWD, 1, nh_info->hdr.dsfwd_info.dsfwd_offset));

    dsfwd_param.dsfwd_offset = nh_info->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.drop_pkt = 1;
    /*Write table*/
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));


    return CTC_E_NONE;
}

int32
sys_usw_nh_create_iloop_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_iloop_t* p_nh_para;
    sys_nh_info_special_t* p_nhdb;
    uint32 dsfwd_offset = 0;
    uint8  gchip = 0;
    sys_nh_param_dsfwd_t dsfwd_param;
    uint16 lport = 0;
    uint8 xgpon_en = 0;


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);
    p_nh_para = (sys_nh_param_iloop_t*)p_com_nh_para;
    CTC_PTR_VALID_CHECK(p_nh_para->p_iloop_param);
    if (p_nh_para->p_iloop_param->words_removed_from_hdr > MCHIP_CAP(SYS_CAP_NH_ILOOP_MAX_REMOVE_WORDS))
    {
        return CTC_E_INVALID_PARAM;
    }

    sys_usw_global_get_xgpon_en(lchip, &xgpon_en);

    lport = CTC_MAP_GPORT_TO_LPORT(p_nh_para->p_iloop_param->lpbk_lport);

    p_nhdb = (sys_nh_info_special_t*)p_com_db;
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    p_nhdb->hdr.nh_entry_type = (p_nh_para->hdr.nh_param_type);
    dsfwd_param.dest_id = ((lport&0x100)|SYS_RSV_PORT_ILOOP_ID);
    dsfwd_param.dsnh_offset = SYS_NH_ENCODE_ILOOP_DSNH(
            (lport&0xFF),
            (p_nh_para->p_iloop_param->logic_port ? \
             1 : 0), p_nh_para->p_iloop_param->customerid_valid,
            p_nh_para->p_iloop_param->words_removed_from_hdr,
            xgpon_en?0:1);

    sys_usw_get_gchip_id(lchip, &gchip);
    p_nhdb->dest_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, dsfwd_param.dest_id);

    if (p_nh_para->p_iloop_param->customerid_valid)
    {
        dsfwd_param.nexthop_ext = 1;
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W);
    }

    if(p_nh_para->p_iloop_param->inner_packet_type_valid)
    {
        CTC_BIT_SET(dsfwd_param.dsnh_offset, 11);
        dsfwd_param.dsnh_offset &= ~(0x7000);
        dsfwd_param.dsnh_offset |= ((p_nh_para->p_iloop_param->inner_packet_type& 0x7) << 12);
    }

    /*Build DsFwd Table*/
    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
                                                    &dsfwd_offset));
    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    p_nhdb->hdr.dsfwd_info.dsfwd_offset = dsfwd_offset;
    p_nh_para->hdr.dsfwd_offset = dsfwd_offset;
    p_nhdb->hdr.dsfwd_info.dsnh_offset = dsfwd_param.dsnh_offset;


    dsfwd_param.dest_chipid = gchip;
    /*Writeback dsfwd offset*/
    dsfwd_param.dsfwd_offset = p_nhdb->hdr.dsfwd_info.dsfwd_offset;
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.is_egress_edit = 1;
    /*Write table*/
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));


    return CTC_E_NONE;
}

int32
sys_usw_nh_delete_iloop_cb(uint8 lchip, sys_nh_info_com_t* p_data, uint32* p_nhid)
{
    sys_nh_info_special_t* nh_info;
    sys_nh_param_dsfwd_t dsfwd_param;


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    nh_info = (sys_nh_info_special_t*)(p_data);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_ILOOP, p_data->hdr.nh_entry_type);


    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    /*Free dsfwd offset*/

    CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
    SYS_NH_ENTRY_TYPE_FWD, 1, nh_info->hdr.dsfwd_info.dsfwd_offset));

    dsfwd_param.dsfwd_offset = nh_info->hdr.dsfwd_info.dsfwd_offset;

    dsfwd_param.drop_pkt = 1;
    /*Write table*/
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));


    return CTC_E_NONE;
}

int32
sys_usw_nh_update_iloop_cb(uint8 lchip, sys_nh_info_com_t* p_iloop_info, sys_nh_param_com_t* p_iloop_param)
{

    return CTC_E_NONE;
}

int32
sys_usw_nh_create_rspan_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                 sys_nh_info_com_t* p_com_db)
{

    sys_nh_param_rspan_t* p_nh_para;
    sys_nh_param_dsnh_t dsnh_param;
    ctc_vlan_egress_edit_info_t  vlan_egress_edit_info;
    sys_nh_info_rspan_t* p_nhdb = NULL;
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master))

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);
    p_nh_para = (sys_nh_param_rspan_t*)p_com_nh_para;
    p_nhdb = (sys_nh_info_rspan_t*)p_com_db;

    CTC_PTR_VALID_CHECK(p_nh_para->p_rspan_param);
    CTC_VLAN_RANGE_CHECK(p_nh_para->p_rspan_param->rspan_vid);

    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));
    sal_memset(&vlan_egress_edit_info, 0, sizeof(ctc_vlan_egress_edit_info_t));
    vlan_egress_edit_info.output_svid = p_nh_para->p_rspan_param->rspan_vid;

    dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_RSPAN;
    dsnh_param.p_vlan_info = &vlan_egress_edit_info;
    dsnh_param.dsnh_offset = p_nh_para->dsnh_offset;
    dsnh_param.dest_vlan_ptr = p_nh_para->p_rspan_param->rspan_vid;

    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_RSPAN;

    /*Only used for compatible GG Ingress Edit Mode */
    dsnh_param.dsnh_offset = SYS_DSNH_INDEX_FOR_REMOTE_MIRROR;
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));

    dsnh_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
    CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));

    return CTC_E_NONE;
}

int32
sys_usw_nh_delete_rspan_cb(uint8 lchip, sys_nh_info_com_t* p_com_db, uint32* p_nhid)
{
    return CTC_E_NONE;
}


 /* if vlan edit in bucket equals */
STATIC bool
_sys_usw_nh_misc_l2edit_compare_vlan_edit(sys_dsl2edit_vlan_edit_t* pv0,
                                          sys_dsl2edit_vlan_edit_t* pv1)
{
    if (!pv0 || !pv1)
    {
        return FALSE;
    }

    if (!sal_memcmp(pv0, pv1, sizeof(DsOfEditVlanActionProfile_m)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_sys_usw_nh_misc_vlan_edit_spool_alloc_index(sys_dsl2edit_vlan_edit_t* p_edit, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    uint32 profile_id = 0;
    int32 ret = 0;

    if(p_edit->profile_id)
    {
        ret = sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE, 1, p_edit->profile_id);
        profile_id = p_edit->profile_id;
    }
    else
    {
        ret = sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE, 1, &profile_id);

    }

    if(ret)
    {
        return ret;
    }

    p_edit->profile_id = profile_id;
    return ret;
}

int32
_sys_usw_nh_misc_vlan_edit_spool_free_index(sys_dsl2edit_vlan_edit_t* p_edit, void* user_data)
{
    uint8 lchip = *(uint8*)user_data;
    int32 ret = 0;

    CTC_PTR_VALID_CHECK(p_edit);

    ret = sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE, 1, p_edit->profile_id);

    return ret;
}
STATIC uint32
_sys_usw_nh_misc_l2edit_make_vlan_edit(sys_dsl2edit_vlan_edit_t* pv)
{

    uint8  * k = NULL;
    k    = (uint8 *) pv;

    return ctc_hash_caculate(sizeof(DsOfEditVlanActionProfile_m), k);
}

/* pv_out can be NULL.*/
STATIC int32
_sys_usw_nh_misc_l2edit_add_vlan_edit_spool(uint8 lchip, sys_dsl2edit_vlan_edit_t* pv,
                                             sys_dsl2edit_vlan_edit_t** pv_out)
{
    sys_dsl2edit_vlan_edit_t* pv_get = NULL; /* get from share pool*/
    int32                 ret      = 0;
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if(!p_nh_master->p_l2edit_vprof)
    {
       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }
    /* -- set vlan edit ptr -- */
    ret = ctc_spool_add(p_nh_master->p_l2edit_vprof, pv, NULL, &pv_get);
    if (ret < 0)
    {
        ret = CTC_E_NO_MEMORY;
        return ret;
    }

    if (pv_out)
    {
        *pv_out = pv_get;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_misc_l2edit_remove_vlan_edit_spool(uint8 lchip,  sys_dsl2edit_vlan_edit_t* pv)

{
    int32                 ret = 0;
    sys_dsl2edit_vlan_edit_t* pv_lkup;
    sys_usw_nh_master_t* p_nh_master = NULL;

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));
    if(NULL == p_nh_master->p_l2edit_vprof)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    ret = ctc_spool_remove(p_nh_master->p_l2edit_vprof, pv, &pv_lkup);
    if (ret < 0)
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_misc_build_l2_edit(uint8 lchip, ctc_misc_nh_param_t* p_edit_param,
                                    sys_nh_info_misc_t* p_nhdb)
{
    uint8 misc_type = 0;
    ctc_misc_nh_flex_edit_param_t  *p_flex_edit         = NULL;
    ctc_misc_nh_over_l2_edit_param_t  *p_over_l2edit    = NULL;

    uint8 entry_type = 0;
    sys_dsl2edit_of6w_t dsl2edit;
    sys_dsl2edit_t ds_eth_l2_edit;
    void *p_dsl2edit = NULL;

    uint32 entry_num = 0;
    uint32 l2_offset = 0;


    misc_type = p_edit_param->type;

    if((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == misc_type) || (CTC_MISC_NH_TYPE_OVER_L2 == misc_type))
    {
        p_over_l2edit = &p_edit_param->misc_param.over_l2edit;
    }
    else
    {
        p_flex_edit = &p_edit_param->misc_param.flex_edit;
    }

    if(p_flex_edit)
    {
        if (CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_SWAP_MACDA))
        {
            if (CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG) && (0 == p_flex_edit->user_vlanptr))
            {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			    return CTC_E_NOT_SUPPORT;
            }

            sal_memset(&dsl2edit, 0, sizeof(dsl2edit));
            dsl2edit.p_flex_edit = p_flex_edit;
            entry_type = SYS_NH_ENTRY_TYPE_L2EDIT_SWAP;
        }
        else
        {
            sys_dsl2edit_vlan_edit_t vlan_edit;
            sys_dsl2edit_vlan_edit_t *p_edit_out;

            sal_memset(&vlan_edit, 0, sizeof(sys_dsl2edit_vlan_edit_t));
            dsl2edit.vlan_profile_id = 0;
            dsl2edit.p_flex_edit = p_flex_edit;
            if (CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG) && (0 == p_flex_edit->user_vlanptr))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_misc_encoding_vlan_profile(lchip, p_flex_edit, &(vlan_edit.data)));
                CTC_ERROR_RETURN(_sys_usw_nh_misc_l2edit_add_vlan_edit_spool(lchip, &vlan_edit, &p_edit_out));
                CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,  SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE,
                    p_edit_out->profile_id, &(vlan_edit.data)));
                dsl2edit.vlan_profile_id = p_edit_out->profile_id;
                p_nhdb->vlan_profile_id = p_edit_out->profile_id;
            }
            entry_type = SYS_NH_ENTRY_TYPE_L2EDIT_OF_6W;
        }

        p_dsl2edit = &dsl2edit;
        entry_num = 1;

    }

    if(p_over_l2edit)
    {
        sal_memset(&ds_eth_l2_edit, 0, sizeof(sys_dsl2edit_t));
        entry_type = SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_8W;


        ds_eth_l2_edit.ds_type = SYS_NH_DS_TYPE_L2EDIT;
        ds_eth_l2_edit.l2_rewrite_type = SYS_NH_L2EDIT_TYPE_ETH_8W;

        ds_eth_l2_edit.is_vlan_valid = (p_over_l2edit->vlan_id != 0);
        ds_eth_l2_edit.output_vlan_id= p_over_l2edit->vlan_id;
        ds_eth_l2_edit.is_6w = 1;

        sal_memcpy(ds_eth_l2_edit.mac_da, p_over_l2edit->mac_da, sizeof(mac_addr_t));
        sal_memcpy(ds_eth_l2_edit.mac_sa, p_over_l2edit->mac_sa, sizeof(mac_addr_t));
        ds_eth_l2_edit.update_mac_sa = 1;
        ds_eth_l2_edit.ether_type    = p_over_l2edit->ether_type;

        if (CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == misc_type)
        {
            ds_eth_l2_edit.is_span_ext_hdr = 1;
        }

        entry_num = 1;
        p_dsl2edit = &ds_eth_l2_edit;

    }

    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip,  entry_type, entry_num,  &p_nhdb->dsl2edit_offset));

    l2_offset = (p_nhdb->dsl2edit_offset);
    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
    p_nhdb->l2edit_entry_type = entry_type;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,  entry_type, l2_offset, p_dsl2edit));
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsL2Edit, offset = %d\n",  l2_offset);

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_nh_misc_build_l3_edit(uint8 lchip, ctc_misc_nh_param_t* p_edit_param,
                                    sys_nh_info_misc_t* p_nhdb)
{
    ctc_misc_nh_flex_edit_param_t  *p_flex_edit = &p_edit_param->misc_param.flex_edit;
    uint8 entry_type = SYS_NH_ENTRY_TYPE_L3EDIT_OF_6W;
    if (!CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_IPV4)
          || (p_flex_edit->packet_type == CTC_MISC_NH_PACKET_TYPE_ARP
               && CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR)))
    {
        entry_type = SYS_NH_ENTRY_TYPE_L3EDIT_OF_12W;
    }
    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip,  entry_type, 1,  &p_nhdb->dsl3edit_offset));
    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    p_nhdb->l3edit_entry_type = entry_type;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip,  entry_type, p_nhdb->dsl3edit_offset, p_flex_edit));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_misc_encode_mpls_edit(uint8 lchip, ctc_mpls_nh_label_param_t* p_label, sys_dsmpls_t*  p_dsmpls)
{
    uint32 ttl_index = 0;

    p_dsmpls->l3_rewrite_type = SYS_NH_L3EDIT_TYPE_MPLS_4W;
    p_dsmpls->ds_type = SYS_NH_DS_TYPE_L3EDIT;
    p_dsmpls->oam_en = 0;

    if (CTC_FLAG_ISSET(p_label->lable_flag, CTC_MPLS_NH_LABEL_MAP_TTL))
    {
        p_dsmpls->map_ttl = 1;
        p_dsmpls->map_ttl_mode = 1;     /* only for lsp label */
    }

    if (CTC_FLAG_ISSET(p_label->lable_flag, CTC_MPLS_NH_LABEL_IS_MCAST))
    {
        p_dsmpls->mcast_label = 1;
    }

    CTC_ERROR_RETURN(sys_usw_lkup_ttl_index(lchip, p_label->ttl, &ttl_index));
    p_dsmpls->ttl_index = ttl_index;
    p_dsmpls->label  = (p_label->label & 0xFFFFF);   /*label*/

    switch (p_label->exp_type)
    {
        case CTC_NH_EXP_SELECT_ASSIGN:   /* user cfg exp  */
            p_dsmpls->derive_exp  = 0;
            p_dsmpls->exp = p_label->exp & 0x7;
            break;

        case CTC_NH_EXP_SELECT_MAP:  /*mapped exp */
            p_dsmpls->derive_exp = 1;
            p_dsmpls->exp = SYS_NH_MPLS_LABEL_EXP_SELECT_MAP;
            p_dsmpls->mpls_domain = p_label->exp_domain;
            break;

        case CTC_NH_EXP_SELECT_PACKET:   /*input packet exp  */

            p_dsmpls->derive_exp = 1;
            p_dsmpls->exp = SYS_NH_MPLS_LABEL_EXP_SELECT_PACKET;
            break;

        default:
            break;
    }

    p_dsmpls->derive_label = 1;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_misc_build_mpls_edit(uint8 lchip, ctc_misc_nh_param_t* p_edit_param,
                                    sys_nh_info_misc_t* p_nhdb, uint8 first_label)
{
    uint8 entry_type = SYS_NH_ENTRY_TYPE_L3EDIT_MPLS;
    sys_dsmpls_t dsmpls;
    uint32 offset = 0;
    int32 ret = 0;

    sal_memset(&dsmpls, 0, sizeof(sys_dsmpls_t));

    CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT);
    p_nhdb->l3edit_entry_type = entry_type;

    if (first_label)
    {
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip,  entry_type, 1,  &p_nhdb->dsl3edit_offset), ret, error0);

        if (p_edit_param->misc_param.flex_edit.label_num > 1)
        {
            CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip,  entry_type, 1,  &p_nhdb->next_l3edit_offset), ret, error1);
        }

        /*encoding mpls edit*/
        _sys_usw_nh_misc_encode_mpls_edit(lchip, &p_edit_param->misc_param.flex_edit.label[0], &dsmpls);

        offset = p_nhdb->dsl3edit_offset;
    }
    else
    {
        _sys_usw_nh_misc_encode_mpls_edit(lchip, &p_edit_param->misc_param.flex_edit.label[1], &dsmpls);
        offset = p_nhdb->next_l3edit_offset;
    }

    CTC_ERROR_GOTO(sys_usw_nh_write_asic_table(lchip,  entry_type, offset, &dsmpls), ret, error1);
    return CTC_E_NONE;

error1:
    if (first_label)
    {
        sys_usw_nh_offset_free(lchip, entry_type, 1, p_nhdb->dsl3edit_offset);
        if (p_nhdb->next_l3edit_offset)
        {
            sys_usw_nh_offset_free(lchip, entry_type, 1, p_nhdb->next_l3edit_offset);
        }
    }

error0:

    return ret;
}

STATIC int32
_sys_usw_nh_misc_map_dsnh_vlan_info(uint8 lchip, ctc_misc_nh_flex_edit_param_t* p_flex_edit,
                                             ctc_vlan_egress_edit_info_t* p_vlan_info)
{

    CTC_PTR_VALID_CHECK(p_flex_edit);
    CTC_PTR_VALID_CHECK(p_vlan_info);

    /* 1. svlan info map */
    switch (p_flex_edit->stag_op)
    {
    case CTC_VLAN_TAG_OP_NONE:
    case CTC_VLAN_TAG_OP_VALID:
        p_vlan_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
        break;

    case CTC_VLAN_TAG_OP_REP_OR_ADD:
        p_vlan_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
        break;

    case CTC_VLAN_TAG_OP_REP:
        p_vlan_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE);
        break;

    case CTC_VLAN_TAG_OP_ADD:
        p_vlan_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
        break;

    case CTC_VLAN_TAG_OP_DEL:
        p_vlan_info->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if (p_flex_edit->svid_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
        p_vlan_info->output_svid = p_flex_edit->new_svid;
    }
    else if ((p_flex_edit->svid_sl == CTC_VLAN_TAG_SL_ALTERNATIVE) && (p_flex_edit->cvid_sl == CTC_VLAN_TAG_SL_ALTERNATIVE))
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN);

        if ((p_flex_edit->scos_sl == CTC_VLAN_TAG_SL_ALTERNATIVE) && (p_flex_edit->ccos_sl == CTC_VLAN_TAG_SL_ALTERNATIVE))
        {
            CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN);
        }
    }
    else if (p_flex_edit->svid_sl != CTC_VLAN_TAG_SL_AS_PARSE)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_flex_edit->scos_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
        p_vlan_info->stag_cos = p_flex_edit->new_scos;
    }
    else if (p_flex_edit->scos_sl == CTC_VLAN_TAG_SL_DEFAULT)
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_MAP_SVLAN_COS);
    }
    else if ((p_flex_edit->scos_sl == CTC_VLAN_TAG_SL_ALTERNATIVE) && (!CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_flex_edit->new_stpid_en)
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID);
        p_vlan_info->svlan_tpid_index = p_flex_edit->new_stpid_idx;
    }


    /* 2. cvlan info map */
    switch (p_flex_edit->ctag_op)
    {
    case CTC_VLAN_TAG_OP_NONE:
    case CTC_VLAN_TAG_OP_VALID:
        p_vlan_info->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
        break;

    case CTC_VLAN_TAG_OP_REP_OR_ADD:
        p_vlan_info->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
        break;

    case CTC_VLAN_TAG_OP_ADD:
        p_vlan_info->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
        break;

    case CTC_VLAN_TAG_OP_DEL:
        p_vlan_info->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
        break;

    default: /* CTC_VLAN_TAG_OP_REP */
        return CTC_E_INVALID_PARAM;
    }

    if (p_flex_edit->cvid_sl == CTC_VLAN_TAG_SL_NEW)
    {
        CTC_SET_FLAG(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
        p_vlan_info->output_cvid = p_flex_edit->new_cvid;
    }
    else if ((p_flex_edit->cvid_sl != CTC_VLAN_TAG_SL_AS_PARSE) && (!CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN)))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_flex_edit->ccos_sl != CTC_VLAN_TAG_SL_AS_PARSE) && (!CTC_FLAG_ISSET(p_vlan_info->edit_flag, CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN)))
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_misc_write_dsnh(uint8 lchip, sys_nh_param_misc_t* p_nh_misc ,
                                    sys_nh_info_misc_t* p_nhdb)
{
    uint8 gchip = 0;
    sys_nh_param_dsnh_t dsnh_param;
    uint8 misc_type = 0;
    uint16 dest_vlan_ptr = SYS_DSNH_DESTVLANPTR_SPECIAL;
    sys_l3if_prop_t l3if_prop;
    ctc_vlan_egress_edit_info_t vlan_info;
    uint8 dsnh_type = SYS_NH_PARAM_DSNH_TYPE_OF;
    uint8 index = 0;

    ctc_misc_nh_flex_edit_param_t  *p_flex_edit = NULL;
    ctc_misc_nh_over_l2_edit_param_t  *p_over_l2_edit = NULL;
    ctc_nh_oif_info_t *p_oif = NULL;

    sal_memset(&l3if_prop, 0, sizeof(l3if_prop));
    sal_memset(&vlan_info, 0, sizeof(vlan_info));

    misc_type =  p_nh_misc->p_misc_param->type;
    if ((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == misc_type) || (CTC_MISC_NH_TYPE_OVER_L2 == misc_type))
    {
        p_over_l2_edit = &p_nh_misc->p_misc_param->misc_param.over_l2edit;
    }
    else
    {
        p_flex_edit = &p_nh_misc->p_misc_param->misc_param.flex_edit;
    }

    sys_usw_get_gchip_id(lchip, &gchip);

    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));

    dsnh_param.stats_ptr = p_nh_misc->hdr.stats_ptr;
    dsnh_param.cid = p_nh_misc->p_misc_param->cid;
    if(p_flex_edit)
    {
        if (p_flex_edit->is_reflective)
        {
           p_nh_misc->p_misc_param->gport = CTC_MAP_LPORT_TO_GPORT(gchip, 0);
           p_nhdb->is_reflective = p_flex_edit->is_reflective;
           p_nh_misc->hdr.have_dsfwd = 1;
        }

        dest_vlan_ptr = p_flex_edit->user_vlanptr ? p_flex_edit->user_vlanptr : dest_vlan_ptr;
    }

    p_oif = &(p_nh_misc->p_misc_param->oif);
    p_nhdb->gport = p_nh_misc->p_misc_param->is_oif ? p_oif->gport : p_nh_misc->p_misc_param->gport;

    if (DRV_IS_DUET2(lchip))
    {
        if (((dest_vlan_ptr>>10)&0x7) == 4)
        {
            p_nhdb->l3ifid = (dest_vlan_ptr&0x3ff);
        }
    }
    else
    {
        if ((dest_vlan_ptr&0x1000) && (dest_vlan_ptr&0xfff))
        {
            p_nhdb->l3ifid = (dest_vlan_ptr&0xfff);
        }
    }

    if (p_nh_misc->p_misc_param->is_oif && (!(CTC_IS_CPU_PORT(p_oif->gport) || p_oif->is_l2_port)))
    {
        CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info_with_port_and_vlan(lchip, p_oif->gport, p_oif->vid, &l3if_prop));
        dest_vlan_ptr = l3if_prop.vlan_ptr;
        p_nhdb->l3ifid = l3if_prop.l3if_id;
    }


    if (p_flex_edit)
    {
        if ((p_flex_edit->packet_type == CTC_MISC_NH_PACKET_TYPE_MPLS) && p_flex_edit->flag)
        {
            return CTC_E_NOT_SUPPORT;
        }

        if (p_flex_edit->packet_type == CTC_MISC_NH_PACKET_TYPE_MPLS)
        {
            if (p_flex_edit->label_num > CTC_MPLS_NH_MAX_FLEX_LABEL_NUM)
            {
                return CTC_E_INVALID_PARAM;
            }

            for (index = 0; index < p_flex_edit->label_num; index++)
            {
                SYS_NH_CHECK_MPLS_LABVEL_VALUE(p_flex_edit->label[index].label);
                SYS_NH_CHECK_MPLS_EXP_VALUE(p_flex_edit->label[index].exp);
                SYS_NH_CHECK_MPLS_EXP_TYPE_VALUE(p_flex_edit->label[index].exp_type);
                CTC_MAX_VALUE_CHECK(p_flex_edit->label[index].exp_domain, 15);
            }

            CTC_ERROR_RETURN(_sys_usw_nh_misc_build_mpls_edit(lchip, p_nh_misc->p_misc_param, p_nhdb, 1));

            if (p_flex_edit->label_num > 1)
            {
                CTC_ERROR_RETURN(_sys_usw_nh_misc_build_mpls_edit(lchip, p_nh_misc->p_misc_param, p_nhdb, 0));
            }

            dsnh_type = SYS_NH_PARAM_DSNH_TYPE_MPLS_PUSH_OP_NONE;
            if (p_flex_edit->label_num > 1)
            {
                dsnh_param.lspedit_ptr = p_nhdb->next_l3edit_offset;
            }
        }
        else
        {
            if(CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR))
            {
                CTC_ERROR_RETURN(_sys_usw_nh_misc_build_l2_edit(lchip, p_nh_misc->p_misc_param,p_nhdb));

                if (CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_OP_BRIDGE))
                {
                    CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_MISC_FLEX_PLD_BRIDGE);
                }

                if (CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG) && (0 != p_flex_edit->user_vlanptr))
                {
                    CTC_ERROR_RETURN(_sys_usw_nh_misc_map_dsnh_vlan_info(lchip, p_flex_edit, &vlan_info));
                    dsnh_param.p_vlan_info = &vlan_info;
                }
            }

            if(((p_flex_edit->packet_type != CTC_MISC_NH_PACKET_TYPE_ARP)
                && CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR))
                || CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR))
            {
              CTC_ERROR_RETURN(_sys_usw_nh_misc_build_l3_edit(lchip, p_nh_misc->p_misc_param,p_nhdb));
            }
        }
        dsnh_param.dsnh_type = dsnh_type;
        dsnh_param.dest_vlan_ptr = 	dest_vlan_ptr;
        dsnh_param.l2edit_ptr = p_nhdb->dsl2edit_offset;
        dsnh_param.l3edit_ptr = p_nhdb->dsl3edit_offset;
        dsnh_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
        dsnh_param.mtu_no_chk = CTC_FLAG_ISSET(p_flex_edit->flag, CTC_MISC_NH_FLEX_EDIT_MTU_CHECK) ? 0 : 1;

         if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
        {
            CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh4w(lchip, &dsnh_param));
        }

    }

    if (p_over_l2_edit)
    {
        CTC_ERROR_RETURN(_sys_usw_nh_misc_build_l2_edit(lchip, p_nh_misc->p_misc_param,p_nhdb));

        dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_IPTUNNEL;
        dsnh_param.dest_vlan_ptr = dest_vlan_ptr;
        dsnh_param.l2edit_ptr  = p_nhdb->dsl2edit_offset;
        dsnh_param.l3edit_ptr  = 0;
        dsnh_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
        dsnh_param.span_id     = p_over_l2_edit->flow_id;
        if (p_over_l2_edit->flag & CTC_MISC_NH_OVER_L2_EDIT_VLAN_EDIT)
        {
            dsnh_param.p_vlan_info = &p_over_l2_edit->vlan_info;
        }
        CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_ERSPAN_EN);
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param));
    }


    return CTC_E_NONE;
}

int32
_sys_usw_nh_misc_add_dsfwd(uint8 lchip, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_nh_info_misc_t* p_nhdb = (sys_nh_info_misc_t*)(p_com_db);
    int ret = 0;
    uint8 truncate_proflile = 0;

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    dsfwd_param.dest_chipid = SYS_MAP_CTC_GPORT_TO_GCHIP(p_nhdb->gport);
    dsfwd_param.dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_nhdb->gport);
    dsfwd_param.is_cpu  = CTC_IS_CPU_PORT(p_nhdb->gport);
    dsfwd_param.is_mcast = FALSE;
    dsfwd_param.is_reflective = p_nhdb->is_reflective;


    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        dsfwd_param.nexthop_ext = 1;
    }

    if (p_nhdb->truncated_len)
    {
        ret = sys_usw_register_add_truncation_profile(lchip, p_nhdb->truncated_len, 0, &truncate_proflile);
        dsfwd_param.truncate_profile_id = truncate_proflile;
    }

    /*Build DsFwd Table*/
    if (!CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        ret = ret ? ret : sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, (p_nhdb->is_reflective?2:1),
                                             &p_nhdb->hdr.dsfwd_info.dsfwd_offset);
        if (p_nhdb->is_reflective)
        {
            CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_DSFWD_IS_6W);
        }
    }
    dsfwd_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
    dsfwd_param.dsfwd_offset = p_nhdb->hdr.dsfwd_info.dsfwd_offset;

    /*Write table*/
    ret = ret ? ret : sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param);

    if (ret != CTC_E_NONE)
    {
        sys_usw_nh_offset_free(lchip,  SYS_NH_ENTRY_TYPE_FWD, (p_nhdb->is_reflective?2:1), p_nhdb->hdr.dsfwd_info.dsfwd_offset);
        if (p_nhdb->truncated_len)
        {
            sys_usw_register_remove_truncation_profile(lchip, 0, p_nhdb->truncated_len);
        }
    }
    else
    {
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD);
    }
    return ret;

    return CTC_E_NONE;
}
int32
_sys_usw_nh_free_misc_nh_resource(uint8 lchip, sys_nh_info_misc_t* p_nhinfo)
{
    uint32 entry_num = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_nhinfo);

    if (p_nhinfo->vlan_profile_id)
    {
        sys_dsl2edit_vlan_edit_t vlan_edit;
        sal_memset(&vlan_edit, 0, sizeof(sys_dsl2edit_vlan_edit_t));

        sys_usw_nh_get_asic_table(lchip,  SYS_NH_ENTRY_TYPE_L2EDIT_VLAN_PROFILE,
                    p_nhinfo->vlan_profile_id, &(vlan_edit.data));
        _sys_usw_nh_misc_l2edit_remove_vlan_edit_spool(lchip, &vlan_edit);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
    {
        entry_num = 1;
        sys_usw_nh_offset_free(lchip,  p_nhinfo->l2edit_entry_type, entry_num, p_nhinfo->dsl2edit_offset);
    }

    if (CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L3EDIT))
    {
        sys_usw_nh_offset_free(lchip,  p_nhinfo->l3edit_entry_type, 1, p_nhinfo->dsl3edit_offset);

        if (p_nhinfo->next_l3edit_offset)
        {
            sys_usw_nh_offset_free(lchip,  p_nhinfo->l3edit_entry_type, 1, p_nhinfo->next_l3edit_offset);
        }
    }

    if (p_nhinfo->truncated_len && p_nhinfo->misc_nh_type != CTC_MISC_NH_TYPE_TO_CPU)
    {
        sys_usw_register_remove_truncation_profile(lchip, 0, p_nhinfo->truncated_len);
    }
    return CTC_E_NONE;
}

int32
sys_usw_nh_create_misc_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_misc_t* p_nh_misc;
    int32 ret = 0;
    uint8 service_queue_egress_en = 0;

    sys_nh_info_misc_t* p_nhdb = NULL;
    ctc_misc_nh_param_t* p_tmp_misc_param = NULL;
    ctc_misc_nh_type_t misc_nh_type;
    uint32 dest_port = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);

    p_nh_misc = (sys_nh_param_misc_t*)p_com_nh_para;
    p_nhdb    = (sys_nh_info_misc_t*)p_com_db;
    p_tmp_misc_param = p_nh_misc->p_misc_param;
    misc_nh_type = p_tmp_misc_param->type;

    if(CTC_MISC_NH_TYPE_REPLACE_L2HDR == misc_nh_type
		|| CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR == misc_nh_type)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if ((CTC_MISC_NH_TYPE_TO_CPU == misc_nh_type || CTC_MISC_NH_TYPE_LEAF_SPINE == misc_nh_type)
        && (p_tmp_misc_param->stats_id || p_tmp_misc_param->cid))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if ((CTC_MISC_NH_TYPE_TO_CPU == misc_nh_type || CTC_IS_CPU_PORT(p_tmp_misc_param->gport))
        && p_tmp_misc_param->truncated_len)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
    if (service_queue_egress_en && p_tmp_misc_param->truncated_len)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (p_tmp_misc_param->stats_id && p_tmp_misc_param->cid && !CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Cnnot support stats and cid in 4w mode \n");
        return CTC_E_INVALID_CONFIG;
    }

    if (p_tmp_misc_param->stats_id)
    {
        CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip,
                                                           p_tmp_misc_param->stats_id,
                                                           &p_nh_misc->hdr.stats_ptr));
    }

    if(CTC_MISC_NH_TYPE_FLEX_EDIT_HDR == misc_nh_type)
    {
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.ecn, 3);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.dscp_or_tos, 63);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.new_stpid_idx, 3);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.packet_type, CTC_MISC_NH_PACKET_TYPE_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.stag_op, CTC_VLAN_TAG_OP_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.svid_sl, CTC_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.scos_sl, CTC_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.ctag_op, CTC_VLAN_TAG_OP_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.cvid_sl, CTC_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.ccos_sl, CTC_VLAN_TAG_SL_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.ecn_select, (MAX_CTC_NH_ECN_SELECT_MODE-1));
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.user_vlanptr, 0x1fff);
        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.flex_edit.dscp_select, (MAX_CTC_NH_DSCP_SELECT_MODE-1));

        if (CTC_VLAN_TAG_SL_NEW == p_tmp_misc_param->misc_param.flex_edit.svid_sl)
        {
            CTC_VLAN_RANGE_CHECK(p_tmp_misc_param->misc_param.flex_edit.new_svid);
        }

        if (CTC_VLAN_TAG_SL_NEW == p_tmp_misc_param->misc_param.flex_edit.cvid_sl)
        {
            CTC_VLAN_RANGE_CHECK(p_tmp_misc_param->misc_param.flex_edit.new_cvid);
        }

        if (CTC_VLAN_TAG_SL_NEW == p_tmp_misc_param->misc_param.flex_edit.scos_sl)
        {
            CTC_COS_RANGE_CHECK(p_tmp_misc_param->misc_param.flex_edit.new_scos);
        }

        if (CTC_VLAN_TAG_SL_NEW == p_tmp_misc_param->misc_param.flex_edit.ccos_sl)
        {
            CTC_COS_RANGE_CHECK(p_tmp_misc_param->misc_param.flex_edit.new_ccos);
        }
    }
    else if((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == misc_nh_type) || (CTC_MISC_NH_TYPE_OVER_L2 == misc_nh_type))
    {
        if (p_tmp_misc_param->misc_param.over_l2edit.vlan_id)
        {
            CTC_VLAN_RANGE_CHECK(p_tmp_misc_param->misc_param.over_l2edit.vlan_id);
        }

        CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.over_l2edit.flow_id, 0x3FF);

        if (p_tmp_misc_param->misc_param.over_l2edit.flag & CTC_MISC_NH_OVER_L2_EDIT_VLAN_EDIT)
        {
            if (CTC_FLAG_ISSET(p_tmp_misc_param->misc_param.over_l2edit.vlan_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID))
            {
                CTC_VLAN_RANGE_CHECK(p_tmp_misc_param->misc_param.over_l2edit.vlan_info.output_cvid);
            }

            if (CTC_FLAG_ISSET(p_tmp_misc_param->misc_param.over_l2edit.vlan_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID))
            {
                CTC_VLAN_RANGE_CHECK(p_tmp_misc_param->misc_param.over_l2edit.vlan_info.output_svid);
            }

            CTC_MAX_VALUE_CHECK(p_tmp_misc_param->misc_param.over_l2edit.vlan_info.user_vlanptr, 0x1fff);
        }
    }

    p_nhdb->hdr.nh_entry_type   = SYS_NH_TYPE_MISC;
    p_nhdb->misc_nh_type        = misc_nh_type;


    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nhType = %d, miscNhType = %d, NHID = %d, isInternaNH = %d, \n\
        gport = %d, dsnh_offset = %d\n",
                   p_nh_misc->hdr.nh_param_type,
                   misc_nh_type,
                   p_nh_misc->hdr.nhid,
                   p_nh_misc->hdr.is_internal_nh,
                   p_nh_misc->p_misc_param->gport,
                   p_nh_misc->p_misc_param->dsnh_offset);


    if (CTC_MISC_NH_TYPE_TO_CPU == misc_nh_type)
    {
        CTC_ERROR_RETURN(sys_usw_cpu_reason_alloc_dsfwd_offset(lchip, p_nh_misc->p_misc_param->misc_param.cpu_reason.cpu_reason_id,
                  &p_nhdb->hdr.dsfwd_info.dsfwd_offset,&p_nhdb->hdr.dsfwd_info.dsnh_offset, &dest_port));

        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD);
        p_nhdb->gport = dest_port;
        p_nhdb->truncated_len = p_nh_misc->p_misc_param->misc_param.cpu_reason.cpu_reason_id;
        p_nh_misc->hdr.have_dsfwd = 0;
    }
    else if (CTC_MISC_NH_TYPE_LEAF_SPINE == misc_nh_type)
    {
        uint8 gchip = 0;

        p_nh_misc->hdr.have_dsfwd = 1;
        gchip = CTC_MAP_GPORT_TO_GCHIP(p_nh_misc->p_misc_param->gport);
        SYS_GLOBAL_CHIPID_CHECK(gchip);

        p_nhdb->gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, SYS_RSV_PORT_SPINE_LEAF_PORT);
        p_nhdb->hdr.dsfwd_info.dsnh_offset = (1<<17); /*bypass ingress edit*/
    }
    else
    {
        /*write l2edit and nexthop*/
        CTC_ERROR_GOTO(_sys_usw_nh_misc_write_dsnh(lchip,  p_nh_misc, p_nhdb),ret, error_proc);
    }

    if (p_tmp_misc_param->truncated_len)
    {
        p_nhdb->truncated_len = p_tmp_misc_param->truncated_len;
        p_nh_misc->hdr.have_dsfwd = 1;
    }

	/*3.Create Dwfwd*/
    if(!p_nh_misc->hdr.have_dsfwd)
    {
      return CTC_E_NONE;
    }

	ret = _sys_usw_nh_misc_add_dsfwd(lchip, p_com_db);
error_proc:
	if(ret != CTC_E_NONE)
    {
        _sys_usw_nh_free_misc_nh_resource(lchip, p_nhdb);
    }
    return ret;

}

int32
sys_usw_nh_delete_misc_cb(uint8 lchip, sys_nh_info_com_t* p_flex_info, uint32* p_nhid)
{

    sys_nh_info_misc_t* p_nh_info;
    sys_nh_param_dsfwd_t dsfwd_param;
    sys_nh_ref_list_node_t* p_ref_node, * p_tmp_node;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_flex_info);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MISC, p_flex_info->hdr.nh_entry_type);

    p_nh_info = (sys_nh_info_misc_t*)(p_flex_info);

    /*2. Free DsFwd/DsNexthop/DsL2Edit offset*/
    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    if (p_nh_info->misc_nh_type == CTC_MISC_NH_TYPE_TO_CPU)
    {
        sys_usw_cpu_reason_alloc_dsfwd_offset(lchip, p_nh_info->truncated_len, NULL, NULL, NULL);
    }

    if (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
    {
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, \
                                              SYS_NH_ENTRY_TYPE_FWD,  \
                                              (CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_DSFWD_IS_6W)?2:1),  \
                                              p_nh_info->hdr.dsfwd_info.dsfwd_offset));

        dsfwd_param.drop_pkt  = TRUE;
        dsfwd_param.dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;

        /*Write table*/
        CTC_ERROR_RETURN(sys_usw_nh_write_entry_dsfwd(lchip, &dsfwd_param));
    }

    sys_usw_nh_update_ecmp_member(lchip, *p_nhid, SYS_NH_CHANGE_TYPE_NH_DELETE);

    /* delete all reference ecmp nh*/
    p_ref_node = p_nh_info->hdr.p_ref_nh_list;

    while (p_ref_node)
    {
        /*Remove from db*/
        p_tmp_node = p_ref_node;
        p_ref_node = p_ref_node->p_next;
        mem_free(p_tmp_node);
    }
    p_nh_info->hdr.p_ref_nh_list = NULL;

    _sys_usw_nh_free_misc_nh_resource(lchip, p_nh_info);
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_nh_update_misc_fwd_attr(uint8 lchip, sys_nh_param_misc_t* p_nhpara,
                                            sys_nh_info_misc_t* p_nhinfo)
{
    sys_nh_param_misc_t* p_para = (sys_nh_param_misc_t*)p_nhpara;
    sys_nh_info_misc_t old_nh_info;
    int32 ret = 0;
    uint8 is_update = (p_nhpara->hdr.change_type != SYS_NH_CHANGE_TYPE_NULL);
    uint32 dsnh_offset = 0;
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    DsNextHop8W_m data_w;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)
       && p_para->p_misc_param
    && ((CTC_MISC_NH_TYPE_OVER_L2_WITH_TS == p_para->p_misc_param->type) ||
         (CTC_MISC_NH_TYPE_OVER_L2 == p_para->p_misc_param->type) ||
        (p_para->p_misc_param->cid && !g_usw_nh_master[lchip]->cid_use_4w) ||
        (p_para->p_misc_param->stats_id && g_usw_nh_master[lchip]->cid_use_4w) ||
        p_para->p_misc_param->misc_param.flex_edit.user_vlanptr)
    && (!CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV)))
    {
       SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Should use DsNexthop8W table \n");
       return CTC_E_INVALID_CONFIG;
    }

    if(is_update)
    {
         /*store nh old data, for recover*/
         sal_memcpy(&old_nh_info, p_nhinfo, sizeof(sys_nh_info_misc_t));
         sal_memset(p_nhinfo, 0, sizeof(sys_nh_info_misc_t));
         sys_usw_nh_copy_nh_entry_flags(lchip, &old_nh_info.hdr, &p_nhinfo->hdr);
         p_nhinfo->updateAd = old_nh_info.updateAd;
         p_nhinfo->data = old_nh_info.data;

        tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        sal_memset(&data_w, 0, sizeof(DsNextHop8W_m));
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);
    }

    CTC_ERROR_GOTO(sys_usw_nh_create_misc_cb(lchip, (sys_nh_param_com_t*)p_nhpara,
        (sys_nh_info_com_t*)p_nhinfo), ret, error);

    /*for update free resource here*/
   if(is_update)
   {
        if (old_nh_info.misc_nh_type == CTC_MISC_NH_TYPE_TO_CPU)
        {
            sys_usw_cpu_reason_alloc_dsfwd_offset(lchip, old_nh_info.truncated_len, NULL, NULL, NULL);
        }

        _sys_usw_nh_free_misc_nh_resource(lchip, &old_nh_info);
   }
   return CTC_E_NONE;
error:
   if(is_update)
   {
       sal_memcpy( p_nhinfo, &old_nh_info,sizeof(sys_nh_info_misc_t));
        tbl_id = CTC_FLAG_ISSET(p_nhinfo->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_USE_DSNH8W)?DsNextHop8W_t:DsNextHop4W_t;
        dsnh_offset = p_nhinfo->hdr.dsfwd_info.dsnh_offset;
        cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, dsnh_offset, cmd, &data_w);
   }
   return ret;

}

int32
sys_usw_nh_update_misc_cb(uint8 lchip, sys_nh_info_com_t* p_nh_db,
                                sys_nh_param_com_t* p_para)
{
    sys_nh_info_misc_t* p_nh_info;
    sys_nh_param_misc_t* p_nh_para;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_para);
    CTC_PTR_VALID_CHECK(p_nh_db);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MISC, p_para->hdr.nh_param_type);
    CTC_EQUAL_CHECK(SYS_NH_TYPE_MISC, p_nh_db->hdr.nh_entry_type);

    p_nh_info = (sys_nh_info_misc_t*)(p_nh_db);
    p_nh_para = (sys_nh_param_misc_t*)(p_para);

    switch (p_para->hdr.change_type)
    {
    case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
        break;
    case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:
        break;
    case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
        p_nh_para->hdr.have_dsfwd  = CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD)
                               || (NULL != p_nh_info->hdr.p_ref_nh_list);

        CTC_ERROR_RETURN(_sys_usw_nh_update_misc_fwd_attr(lchip, p_nh_para, p_nh_info));
        break;

    case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
        {

           if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD) ||
               CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_RSV_DSFWD))
            {
                return CTC_E_NONE;
            }
            CTC_ERROR_RETURN(_sys_usw_nh_misc_add_dsfwd(lchip, p_nh_db));
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

    if (CTC_FLAG_ISSET(p_nh_db->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_LOOP_USED))
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_loopback_l2edit(lchip, p_nh_para->hdr.nhid));
    }

    return CTC_E_NONE;
}



int32
sys_usw_nh_misc_init(uint8 lchip)
{
    sys_usw_nh_master_t* p_nh_master = NULL;
    ctc_spool_t spool;

    sal_memset(&spool, 0, sizeof(ctc_spool_t));

    CTC_ERROR_RETURN(sys_usw_nh_get_nh_master(lchip, &p_nh_master));

    spool.lchip = lchip;
    spool.block_num = 1;
    spool.block_size = MCHIP_CAP(SYS_CAP_NH_L2EDIT_VLAN_PROFILE_NUM);
    spool.max_count = MCHIP_CAP(SYS_CAP_NH_L2EDIT_VLAN_PROFILE_NUM);
    spool.user_data_size = sizeof(sys_dsl2edit_vlan_edit_t);
    spool.spool_key = (hash_key_fn) _sys_usw_nh_misc_l2edit_make_vlan_edit;
    spool.spool_cmp = (hash_cmp_fn) _sys_usw_nh_misc_l2edit_compare_vlan_edit;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_nh_misc_vlan_edit_spool_alloc_index;
    spool.spool_free = (spool_free_fn)_sys_usw_nh_misc_vlan_edit_spool_free_index;
    p_nh_master->p_l2edit_vprof = ctc_spool_create(&spool);


  return CTC_E_NONE;
}


