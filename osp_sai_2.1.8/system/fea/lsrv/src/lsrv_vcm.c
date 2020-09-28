/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "lsrv_inc.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/


/***************************************************************************************************
 * Name         : lsrv_vcm_evnent_cb
 * Purpose      : creat or delete vcm from lai notification event    
 * Input        : event, add or delete; vcm, vcm information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_vcm_event_cb(lai_vcm_event_t  event,lai_vcm_info_t* p_vcm)
{
    tbl_vcm_t vcm;
    tbl_vcm_t* p_db_vcm = NULL;
    int32 i = 0;

    LSRV_VCM_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_vcm)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_vcm_evnent_cb, VCM is null!\n");
        return;
    }
    sal_memset(&vcm, 0, sizeof(vcm));

    if(event == LAI_VCM_EVENT_ADD)
    {
        LSRV_VCM_DEBUG("VCM insert in, slot %d, id %d\n", p_vcm->slot_id, (int32)p_vcm->oid);
        vcm.key.id = p_vcm->oid;
        p_db_vcm = tbl_vcm_get_vcm(&vcm.key);
        if(p_db_vcm)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_vcm_evnent_cb, VCM add but vcm is already exist!\n");
            return;
        }
        vcm.slot = p_vcm->slot_id;

        vcm.vcm_type = p_vcm->vcm_type;
        vcm.vcm_channel_nums = p_vcm->vcm_channel_nums;
        sal_memcpy(vcm.channel_name0, p_vcm->channel_name[0], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name1, p_vcm->channel_name[1], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name2, p_vcm->channel_name[2], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name3, p_vcm->channel_name[3], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name4, p_vcm->channel_name[4], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name5, p_vcm->channel_name[5], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name6, p_vcm->channel_name[6], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name7, p_vcm->channel_name[7], LAI_NAME_INFO_STR_MAX);
        sal_memcpy(vcm.channel_name8, p_vcm->channel_name[8], LAI_NAME_INFO_STR_MAX);
        for (i = 0; i < LAI_VCM_CHANNEL_NUM; i++)
        {
            vcm.flag[i] = p_vcm->flag[i];
            vcm.curr_depend_pin[i] = p_vcm->curr_depend_pin[i];
            vcm.volt_split_factor[i] = p_vcm->volt_split_factor[i];
            vcm.low[i] = p_vcm->low[i];
            vcm.high[i] = p_vcm->high[i];
            vcm.volt[i] = p_vcm->volt[i];
            vcm.curr[i] = p_vcm->curr[i];
            vcm.tmpr[i] = p_vcm->tmpr[i];
        }

        tbl_vcm_add_vcm(&vcm);
    }
    else
    {
        LSRV_VCM_DEBUG("VCM removed, slot %d, id %d\n", p_vcm->slot_id,(int32)p_vcm->oid);
        vcm.key.id = p_vcm->oid;
        tbl_vcm_del_vcm(&vcm.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_vcm_update_vcm
 * Purpose      : update vcm info in vcm
 * Input        : p_db_vcm, vcm info in db, p_lai_vcm, real vcm info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_vcm_update_vcm(tbl_vcm_t * p_db_vcm, lai_vcm_info_t* p_lai_vcm)
{
    int32 i = 0;
    int32 rc = 0;

    LSRV_VCM_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_vcm || NULL == p_lai_vcm)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_vcm_update_vcm, VCM is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_vcm->slot_id != p_db_vcm->slot)
    {
        return PM_E_NONE;
    }
    LSRV_VCM_DEBUG("VCM update, slot %d, id %d\n", p_lai_vcm->slot_id, (int32)p_lai_vcm->oid);

    for (i = 0; i < LAI_VCM_CHANNEL_NUM; i++)
    {
        p_db_vcm->volt[i] = p_lai_vcm->volt[i];
    }
    PM_E_RETURN(tbl_vcm_set_vcm_field(p_db_vcm, TBL_VCM_FLD_VOLT));

    for (i = 0; i < LAI_VCM_CHANNEL_NUM; i++)
    {
        p_db_vcm->curr[i] = p_lai_vcm->curr[i];
    }
    PM_E_RETURN(tbl_vcm_set_vcm_field(p_db_vcm, TBL_VCM_FLD_CURR));

    for (i = 0; i < LAI_VCM_CHANNEL_NUM; i++)
    {
        p_db_vcm->tmpr[i] = p_lai_vcm->tmpr[i];
    }
    PM_E_RETURN(tbl_vcm_set_vcm_field(p_db_vcm, TBL_VCM_FLD_TMPR));   

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_vcm_set_iter_status
 * Purpose      : set vcm info from lai layer to db
 * Input        : p_db_vcm, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_vcm_get_iter_status(tbl_vcm_t* p_db_vcm, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_vcm_api_t*  vcm_api = NULL;
    lai_vcm_info_t lai_vcm;
    lai_object_id_t oid;

    LSRV_VCM_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_vcm)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_VCM,(void**)&vcm_api));
    oid = p_db_vcm->key.id;
    

    PM_E_RETURN(vcm_api->get_vcm_info(oid, &lai_vcm));
    PM_E_RETURN(lsrv_vcm_update_vcm(p_db_vcm, &lai_vcm));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_vcm_routine_handle
 * Purpose      : poll all vcm status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_vcm_routine_handle()
{
    tbl_vcm_iterate((TBL_ITER_CB_FUNC) lsrv_vcm_get_iter_status, NULL);

    return PM_E_NONE;
}



