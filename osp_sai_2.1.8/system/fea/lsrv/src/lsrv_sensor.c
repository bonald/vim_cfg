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
 * Name         : lsrv_sensor_evnent_cb
 * Purpose      : creat or delete sensor from lai notification event    
 * Input        : event, add or delete; sensor, sensor information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_sensor_event_cb(lai_thermal_event_t  event,lai_thermal_info_t* p_sensor)
{
    tbl_sensor_t sensor;
    tbl_sensor_t* p_db_sensor = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};
    lai_card_info_t card = {0};
    lai_card_api_t*  card_api = NULL;

    LSRV_SENSOR_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_sensor)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_sensor_evnent_cb, SENSOR is null!\n");
        return ;
    }
    sal_memset(&sensor, 0, sizeof(sensor));

    if (event == LAI_THERMAL_EVENT_ADD)
    {
        LSRV_SENSOR_DEBUG("SENSOR insert in, slot %d, id %d\n", p_sensor->slot_id, (int32)p_sensor->oid);
        sensor.key.id = p_sensor->oid;
        p_db_sensor = tbl_sensor_get_sensor(&sensor.key);
        if(p_db_sensor)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_sensor_evnent_cb, SENSOR add but sensor is already exist!\n");
            return;
        }
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "sensor");
        sal_sprintf(str, "%d", sensor.key.id);
        sal_strcat(buff, str);
        sal_strcpy(sensor.name, buff);
        sensor.slot = p_sensor->slot_id;

        if(GLB_FLAG_ISSET(p_sensor->status, LAI_THERMAL_STATUS_FAILED))
        {
            sal_strcpy(sensor.status, "FAIL");
        }
        else
        {
            sal_strcpy(sensor.status, "OK");
        }
        sensor.temperature = p_sensor->tempr;
        sal_strncpy(sensor.position, p_sensor->posision, LAI_NAME_INFO_STR_MAX);
        lai_api_query(LAI_API_CARD,(void**)&card_api);
        card_api->get_card_info(p_sensor->slot_id, &card);
        sensor.is_chip = p_sensor->is_chip;
        
        if (p_sensor->is_chip)
        {
            sensor.tmpr_low = card.tmpr_chip_low_alarm;
            sensor.tmpr_high = card.tmpr_chip_alarm;
            sensor.tmpr_critical = card.tmpr_chip_crit;
        }
        else
        {
            sensor.tmpr_low = card.tmpr_low;
            sensor.tmpr_high = card.tmpr_high;
            sensor.tmpr_critical = card.tmpr_crit;
        }
        
        tbl_sensor_add_sensor(&sensor);
    }
    else
    {
        LSRV_SENSOR_DEBUG("SENSOR removed, slot %d, id %d\n", p_sensor->slot_id,(int32)p_sensor->oid);
        sensor.key.id = p_sensor->oid;
        tbl_sensor_del_sensor(&sensor.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_sensor_update_sensor
 * Purpose      : update sensor info in cdb
 * Input        : p_db_sensor, sensor info in db, p_lai_sensor, real sensor info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_sensor_update_sensor(tbl_sensor_t * p_db_sensor, lai_thermal_info_t* p_lai_sensor)
{
    int32 rc = 0;
    char tmp[GLB_NAME_INFO_STR_MAX] = {0};

    LSRV_SENSOR_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_sensor || NULL == p_lai_sensor)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_sensor_update_sensor, SENSOR is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_sensor->slot_id != p_db_sensor->slot)
    {
        return PM_E_NONE;
    }
    LSRV_SENSOR_DEBUG("SENSOR update, slot %d, id %d\n", p_lai_sensor->slot_id, (int32)p_lai_sensor->oid);



    if(GLB_FLAG_ISSET(p_lai_sensor->status, LAI_THERMAL_STATUS_FAILED))
    {
        sal_strcpy(tmp, "FAIL");
    }
    else
    {
        sal_strcpy(tmp, "OK");
    } 
    if(sal_strncmp(tmp, p_db_sensor->status, GLB_NAME_INFO_STR_MAX))
    {
        sal_strncpy(p_db_sensor->status, tmp,GLB_NAME_INFO_STR_MAX);
        PM_E_RETURN(tbl_sensor_set_sensor_field(p_db_sensor, TBL_SENSOR_FLD_STATUS));
    }

    p_db_sensor->temperature = p_lai_sensor->tempr;
    PM_E_RETURN(tbl_sensor_set_sensor_field(p_db_sensor, TBL_SENSOR_FLD_TEMPERATURE));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_sensor_set_iter_status
 * Purpose      : set sensor info from lai layer to db
 * Input        : p_db_sensor, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_sensor_get_iter_status(tbl_sensor_t* p_db_sensor, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_thermal_api_t*  sensor_api = NULL;
    lai_thermal_info_t lai_sensor;
    lai_object_id_t oid;

    LSRV_SENSOR_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_sensor)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_THERMAL,(void**)&sensor_api));
    oid = p_db_sensor->key.id;
    

    PM_E_RETURN(sensor_api->get_thermal_info(oid, &lai_sensor));
    PM_E_RETURN(lsrv_sensor_update_sensor(p_db_sensor, &lai_sensor));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_sensor_routine_handle
 * Purpose      : poll all sensor status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_sensor_routine_handle()
{
    tbl_sensor_iterate((TBL_ITER_CB_FUNC) lsrv_sensor_get_iter_status, NULL);

    return PM_E_NONE;
}


