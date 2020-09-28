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
 * Name         : lsrv_fan_event_cb
 * Purpose      : creat or delete fan from lai notification event    
 * Input        : event, add or delete; fan, fan information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_fan_event_cb(lai_fan_event_t  event,lai_fan_info_t* p_fan)
{
    tbl_fan_t fan;
    tbl_fan_t* p_db_fan = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};

    LSRV_FAN_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_fan)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_fan_evnent_cb, FAN is null!\n");
        return ;
    }
    sal_memset(&fan, 0, sizeof(fan));

    if(event == LAI_FAN_EVENT_ADD)
    {
        LSRV_FAN_DEBUG("FAN insert in, slot %d, tray %d, id %d\n", p_fan->slot_id, p_fan->tray, (int32)p_fan->oid);
        fan.key.id = p_fan->oid;
        p_db_fan = tbl_fan_get_fan(&fan.key);
        if(p_db_fan)
        {
            //LSRV_LOG_ERR("lsrv_fan_evnent_cb, FAN add but fan is already exist!\n");
            return;
        }
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "fan");
        sal_sprintf(str, "%d", fan.key.id);
        sal_strcat(buff, str);
        sal_strcpy(fan.name, buff);
        fan.slot = p_fan->slot_id;
        fan.tray = p_fan->tray;
        fan.percent = p_fan->percentage;
        fan.absent = (GLB_FLAG_ISSET(p_fan->status, LAI_FAN_STATUS_ABSENT))?1:0;
        if(GLB_FLAG_ISSET(p_fan->status, LAI_FAN_STATUS_FAILED))
        {
            sal_strcpy(fan.status, "FAIL");
        }
        else
        {
            sal_strcpy(fan.status, "OK");
        }        

        if(GLB_FLAG_ISSET(p_fan->status, LAI_FAN_STATUS_B2F))
        {
            sal_strcpy(fan.direction, "B2F");
        }
        else
        {
            sal_strcpy(fan.direction, "F2B");
        } 
        
        if(GLB_FLAG_ISSET(p_fan->status, LAI_FAN_STATUS_AUTO))
        {
            sal_strcpy(fan.mode, "AUTO");
        }
        else
        {
            sal_strcpy(fan.mode, "FORCE");
        }  
        
        tbl_fan_add_fan(&fan);
    }
    else
    {
        LSRV_FAN_DEBUG("FAN removed, slot %d, tray %d, id %d\n", p_fan->slot_id, p_fan->tray, (int32)p_fan->oid);
        fan.key.id = p_fan->oid;
        tbl_fan_del_fan(&fan.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_fan_update_fan
 * Purpose      : update fan info in cdb
 * Input        : p_db_fan, fan info in db, p_lai_fan, real fan info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fan_update_fan(tbl_fan_t * p_db_fan, lai_fan_info_t* p_lai_fan)
{
    int32 rc= 0;
    uint32 absent = 0;
    char status[GLB_NAME_INFO_STR_MAX];

    LSRV_FAN_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_fan || NULL == p_lai_fan)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_fan_update_fan, FAN is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_fan->slot_id != p_db_fan->slot)
    {
        return PM_E_NONE;
    }
    LSRV_FAN_DEBUG("FAN update, slot %d, tray %d, id %d\n", p_lai_fan->slot_id, p_lai_fan->tray, (int32)p_lai_fan->oid);

    if(p_db_fan->percent != p_lai_fan->percentage)
    {
        p_db_fan->percent = p_lai_fan->percentage;
        PM_E_RETURN(tbl_fan_set_fan_field(p_db_fan, TBL_FAN_FLD_PERCENT));
    }
    
    absent = (GLB_FLAG_ISSET(p_lai_fan->status, LAI_FAN_STATUS_ABSENT))?1:0;
    if(absent != p_db_fan->absent)
    {
        p_db_fan->absent = absent;
        PM_E_RETURN(tbl_fan_set_fan_field(p_db_fan, TBL_FAN_FLD_ABSENT));
    }
    sal_memset(status, 0, sizeof(status));
    if(GLB_FLAG_ISSET(p_lai_fan->status, LAI_FAN_STATUS_FAILED))
    {
        sal_strcpy(status, "FAIL");
    }
    else
    {
        sal_strcpy(status, "OK");
    } 
    if(sal_strncmp(p_db_fan->status, status, GLB_NAME_INFO_STR_MAX))
    {
        sal_strncpy(p_db_fan->status, status, GLB_NAME_INFO_STR_MAX);
        PM_E_RETURN(tbl_fan_set_fan_field(p_db_fan, TBL_FAN_FLD_STATUS));
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_fan_set_iter_status
 * Purpose      : set fan info from lai layer to db
 * Input        : p_db_fan, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fan_get_iter_status(tbl_fan_t* p_db_fan, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_fan_api_t*  fan_api = NULL;
    lai_fan_info_t lai_fan;
    lai_object_id_t oid;

    LSRV_FAN_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_fan)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_FAN,(void**)&fan_api));
    oid = p_db_fan->key.id;
    

    PM_E_RETURN(fan_api->get_fan_info(oid, &lai_fan));
    PM_E_RETURN(lsrv_fan_update_fan(p_db_fan, &lai_fan));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_fan_routine_handle
 * Purpose      : poll all fan status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fan_routine_handle()
{
    tbl_fan_iterate((TBL_ITER_CB_FUNC) lsrv_fan_get_iter_status, NULL);
    
    return PM_E_NONE;
}



