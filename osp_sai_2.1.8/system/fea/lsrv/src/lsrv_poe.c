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
 * Name         : lsrv_poe_evnent_cb
 * Purpose      : creat or delete poe from lai notification event    
 * Input        : event, add or delete; poe, poe information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_poe_event_cb(lai_poe_event_t    event,lai_poe_info_t* p_poe)
{
    tbl_poe_t poe;
    tbl_poe_t* p_db_poe = NULL;
    int32 i = 0;

    LSRV_POE_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_poe)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_poe_evnent_cb, POE is null!\n");
        return;
    }
    sal_memset(&poe, 0, sizeof(poe));

    if(event == LAI_POE_EVENT_ADD)
    {
        LSRV_POE_DEBUG("POE insert in, slot %d, id %d\n", p_poe->slot_id, (int32)p_poe->oid);
        poe.key.id = p_poe->oid;
        p_db_poe = tbl_poe_get_poe(&poe.key);
        if(p_db_poe)
        {
            //log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_poe_evnent_cb, POE add but poe is already exist!\n");
            return;
        }
        poe.slot = p_poe->slot_id;

        poe.glb_enable = p_poe->glb_enable;
        poe.glb_power_limit = p_poe->glb_power_limit;
        poe.glb_status = p_poe->glb_status;
        poe.glb_cur_consump = p_poe->glb_cur_consump;
        poe.glb_aver_consump = p_poe->glb_aver_consump;
        poe.glb_peak_consump = p_poe->glb_peak_consump;
        poe.pse_channel_num = p_poe->pse_channel_num;
        poe.const_power_limit = p_poe->const_power_limit;
        
        for (i = 0; i < LAI_POE_PORT_NUM; i++)
        {
            poe.pse_channel_id[i] = p_poe->pse_channel_id[i];
            poe.panel_slot_no[i] = p_poe->panel_slot_no[i];
            poe.panel_port_no[i] = p_poe->panel_port_no[i];
            poe.panel_sub_port_no[i] = p_poe->panel_sub_port_no[i];
            poe.enable[i] = p_poe->enable[i];
            poe.force_power[i] = p_poe->force_power[i];
            poe.mode[i] = p_poe->mode[i];
            poe.priority[i] = p_poe->priority[i];
            poe.power_limit[i] = p_poe->power_limit[i];
            poe.class_level[i] = p_poe->class_level[i];
            poe.status[i] = p_poe->status[i];
            poe.cur_consump[i] = p_poe->cur_consump[i];
            poe.aver_consump[i] = p_poe->aver_consump[i];
            poe.peak_consump[i] = p_poe->peak_consump[i];
            poe.current[i] = p_poe->current[i];
            poe.voltage[i] = p_poe->voltage[i];
            poe.temper[i] = p_poe->temper[i];
        }

        tbl_poe_add_poe(&poe);
    }
    else
    {
        LSRV_POE_DEBUG("POE removed, slot %d, id %d\n", p_poe->slot_id,(int32)p_poe->oid);
        poe.key.id = p_poe->oid;
        tbl_poe_del_poe(&poe.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_poe_update_poe
 * Purpose      : update poe info in poe
 * Input        : p_db_poe, poe info in db, p_lai_poe, real poe info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
lsrv_poe_update_poe(tbl_poe_t * p_db_poe, lai_poe_info_t* p_lai_poe)
{
    int32 i = 0;
    int32 rc = 0;

    LSRV_POE_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_poe || NULL == p_lai_poe)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_poe_update_poe, POE is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_poe->slot_id != p_db_poe->slot)
    {
        return PM_E_NONE;
    }
    LSRV_POE_DEBUG("POE update, slot %d, id %d\n", p_lai_poe->slot_id, (int32)p_lai_poe->oid);

    p_db_poe->glb_enable = p_lai_poe->glb_enable;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_ENABLE));

    p_db_poe->glb_power_limit = p_lai_poe->glb_power_limit;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_POWER_LIMIT));

    p_db_poe->glb_status = p_lai_poe->glb_status;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_STATUS));

    p_db_poe->glb_cur_consump = p_lai_poe->glb_cur_consump;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_CUR_CONSUMP));

    p_db_poe->glb_aver_consump = p_lai_poe->glb_aver_consump;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_AVER_CONSUMP));

    p_db_poe->glb_peak_consump = p_lai_poe->glb_peak_consump;
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_GLB_PEAK_CONSUMP));

    
    for (i = 0; i < LAI_POE_PORT_NUM; i++)
    {
        p_db_poe->enable[i] = p_lai_poe->enable[i];
        p_db_poe->force_power[i] = p_lai_poe->force_power[i];
        p_db_poe->mode[i] = p_lai_poe->mode[i];
        p_db_poe->priority[i] = p_lai_poe->priority[i];
        p_db_poe->power_limit[i] = p_lai_poe->power_limit[i];
        p_db_poe->class_level[i] = p_lai_poe->class_level[i];
        p_db_poe->status[i] = p_lai_poe->status[i];
        p_db_poe->cur_consump[i] = p_lai_poe->cur_consump[i];
        p_db_poe->aver_consump[i] = p_lai_poe->aver_consump[i];
        p_db_poe->peak_consump[i] = p_lai_poe->peak_consump[i];
        p_db_poe->current[i] = p_lai_poe->current[i];
        p_db_poe->voltage[i] = p_lai_poe->voltage[i];
        p_db_poe->temper[i] = p_lai_poe->temper[i];

    }
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_ENABLE));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_FORCE_POWER));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_MODE));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_PRIORITY));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_POWER_LIMIT));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_CLASS_LEVEL));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_STATUS));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_CUR_CONSUMP));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_AVER_CONSUMP));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_PEAK_CONSUMP));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_CURRENT));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_VOLTAGE));
    PM_E_RETURN(tbl_poe_set_poe_field(p_db_poe, TBL_POE_FLD_TEMPER));


    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_poe_set_iter_status
 * Purpose      : set poe info from lai layer to db
 * Input        : p_db_poe, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
lsrv_poe_get_iter_status(tbl_poe_t* p_db_poe, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_poe_api_t*  poe_api = NULL;
    lai_poe_info_t lai_poe;
    lai_object_id_t oid;

    LSRV_POE_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_poe)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_POE,(void**)&poe_api));
    oid = p_db_poe->key.id;
    

    PM_E_RETURN(poe_api->get_poe_info(oid, &lai_poe));
    PM_E_RETURN(lsrv_poe_update_poe(p_db_poe, &lai_poe));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_poe_routine_handle
 * Purpose      : poll all poe status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_poe_routine_handle()
{
    tbl_poe_iterate((TBL_ITER_CB_FUNC) lsrv_poe_get_iter_status, NULL);

    return PM_E_NONE;
}



