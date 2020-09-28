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
 * Name         : lsrv_psu_evnent_cb
 * Purpose      : creat or delete psu from lai notification event    
 * Input        : event, add or delete; psu, psu information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_psu_event_cb(lai_psu_event_t  event,lai_psu_info_t* p_psu)
{
    tbl_psu_t psu;
    tbl_psu_t* p_db_psu = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};

    LSRV_PSU_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_psu)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_psu_evnent_cb, PSU is null!\n");
        return ;
    }
    sal_memset(&psu, 0, sizeof(psu));

    if(event == LAI_PSU_EVENT_ADD)
    {
        LSRV_PSU_DEBUG("PSU insert in, slot %d, id %d\n", p_psu->slot_id, (int32)p_psu->oid);
        psu.key.id = p_psu->oid;
        p_db_psu = tbl_psu_get_psu(&psu.key);
        if(p_db_psu)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_psu_evnent_cb, PSU add but psu is already exist!\n");
            return;
        }
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "psu");
        sal_sprintf(str, "%d", psu.key.id);
        sal_strcat(buff, str);
        sal_strcpy(psu.name, buff);
        psu.slot = p_psu->slot_id;

        if(GLB_FLAG_ISSET(p_psu->status, LAI_PSU_STATUS_FAILED))
        {
            sal_strcpy(psu.run_status, "FAIL");
        }
        else
        {
            sal_strcpy(psu.run_status, "OK");
        }        

        if(GLB_FLAG_ISSET(p_psu->status, LAI_PSU_STATUS_ABESENT))
        {
            psu.absent = 1;
        }

        
        if(GLB_FLAG_ISSET(p_psu->status, LAI_PSU_STATUS_ALERT))
        {
            sal_strcpy(psu.alert_status, "ALERT");
        }
        else
        {
            sal_strcpy(psu.alert_status, "NO");
        }
        
        if(GLB_FLAG_ISSET(p_psu->mode, LAI_PSU_MODE_AC))
        {
            sal_strcpy(psu.mode, "AC");
        }
        else if(GLB_FLAG_ISSET(p_psu->mode, LAI_PSU_MODE_DC12)
            || GLB_FLAG_ISSET(p_psu->mode, LAI_PSU_MODE_DC48))
        {
            sal_strcpy(psu.mode, "DC");
        }
        /* modified by liuht for bug41517, 2016-11-04 */
        /* if can't get psu mode, just display '-'. */
        else
        {
            sal_strcpy(psu.mode, "-");
        }

        psu.fixed_status = p_psu->fixed_status;

        psu.psu_health_status_change = p_psu->psu_health_status_change;

        if (psu.psu_health_status_change)
        {
            psu.support_pmbus = p_psu->support_pmbus;

            psu.pmbus_status_byte = p_psu->status_byte;
            psu.pmbus_status_word = p_psu->status_word;
            psu.pmbus_status_vout = p_psu->status_vout;
            psu.pmbus_status_iout = p_psu->status_iout;
            psu.pmbus_status_input = p_psu->status_input;
            psu.pmbus_status_temperature = p_psu->status_temperature;
            psu.pmbus_status_cml = p_psu->status_cml;
            psu.pmbus_status_other = p_psu->status_other;
            psu.pmbus_status_mfr_specific = p_psu->status_mfr_specific;
            psu.pmbus_status_fans_1_2 = p_psu->status_fans_1_2;
            psu.pmbus_status_fans_3_4 = p_psu->status_fans_3_4;
        }
        
        tbl_psu_add_psu(&psu);
    }
    else
    {
        LSRV_PSU_DEBUG("PSU removed, slot %d, id %d\n", p_psu->slot_id,(int32)p_psu->oid);
        psu.key.id = p_psu->oid;
        tbl_psu_del_psu(&psu.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_psu_update_psu
 * Purpose      : update psu info in cdb
 * Input        : p_db_psu, psu info in db, p_lai_psu, real psu info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_psu_update_psu(tbl_psu_t * p_db_psu, lai_psu_info_t* p_lai_psu)
{
    int32 rc = 0;
    char tmp[GLB_NAME_INFO_STR_MAX] = {0};
    uint32 absent = 0;

    LSRV_PSU_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_psu || NULL == p_lai_psu)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_psu_update_psu, PSU is null!\n");
        return PM_E_NONE;        
    }
    if(p_lai_psu->slot_id != p_db_psu->slot)
    {
        return PM_E_NONE;
    }
    LSRV_PSU_DEBUG("PSU update, slot %d, id %d\n", p_lai_psu->slot_id, (int32)p_lai_psu->oid);

    if (p_lai_psu->psu_health_status_change)
    {
        p_db_psu->psu_health_status_change = p_lai_psu->psu_health_status_change;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PSU_HEALTH_STATUS_CHANGE));
        p_db_psu->support_pmbus= p_lai_psu->support_pmbus;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_SUPPORT_PMBUS));
        p_db_psu->pmbus_status_byte = p_lai_psu->status_byte;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_BYTE));
        p_db_psu->pmbus_status_word = p_lai_psu->status_word;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_WORD));
        p_db_psu->pmbus_status_vout = p_lai_psu->status_vout;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_VOUT));
        p_db_psu->pmbus_status_iout = p_lai_psu->status_iout;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_IOUT));
        p_db_psu->pmbus_status_input = p_lai_psu->status_input;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_INPUT));
        p_db_psu->pmbus_status_temperature = p_lai_psu->status_temperature;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_TEMPERATURE));
        p_db_psu->pmbus_status_cml= p_lai_psu->status_cml;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_CML));
        p_db_psu->pmbus_status_other= p_lai_psu->status_other;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_OTHER));
        p_db_psu->pmbus_status_mfr_specific = p_lai_psu->status_mfr_specific;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_MFR_SPECIFIC));
        p_db_psu->pmbus_status_fans_1_2 = p_lai_psu->status_fans_1_2;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_FANS_1_2));
        p_db_psu->pmbus_status_fans_3_4 = p_lai_psu->status_fans_3_4;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_PMBUS_STATUS_FANS_3_4));
    }

    
    if(GLB_FLAG_ISSET(p_lai_psu->status, LAI_PSU_STATUS_FAILED))
    {
        sal_strcpy(tmp, "FAIL");
    }
    else
    {
        sal_strcpy(tmp, "OK");
    } 
    if(sal_strncmp(tmp, p_db_psu->run_status, GLB_NAME_INFO_STR_MAX))
    {
        sal_strncpy(p_db_psu->run_status, tmp, GLB_NAME_INFO_STR_MAX);
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_RUN_STATUS));
    }

    if(GLB_FLAG_ISSET(p_lai_psu->status, LAI_PSU_STATUS_ABESENT))
    {
        absent = 1;
    }
    if(absent != p_db_psu->absent)
    {
        p_db_psu->absent = absent;
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_ABSENT));
    }

    
    if(GLB_FLAG_ISSET(p_lai_psu->status, LAI_PSU_STATUS_ALERT))
    {
        sal_strcpy(tmp, "ALERT");
    }
    else
    {
        sal_strcpy(tmp, "NO");
    }
    if(sal_strncmp(tmp, p_db_psu->alert_status, GLB_NAME_INFO_STR_MAX))
    {
        sal_strncpy(p_db_psu->alert_status, tmp, GLB_NAME_INFO_STR_MAX);
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_ALERT_STATUS));
    }

    /* modified by liuht for bug41517, 2016-11-04 */
    /* update psu mode */
    if(GLB_FLAG_ISSET(p_lai_psu->mode, LAI_PSU_MODE_AC))
    {
        sal_strcpy(tmp, "AC");
    }
    else if(GLB_FLAG_ISSET(p_lai_psu->mode, LAI_PSU_MODE_DC12)
        || GLB_FLAG_ISSET(p_lai_psu->mode, LAI_PSU_MODE_DC48))
    {
        sal_strcpy(tmp, "DC");
    }
    else
    {
        sal_strcpy(tmp, "-");
    }
    if(sal_strncmp(tmp, p_db_psu->mode, GLB_NAME_INFO_STR_MAX))
    {
        sal_strncpy(p_db_psu->mode, tmp, GLB_NAME_INFO_STR_MAX);
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_MODE));
    }
        PM_E_RETURN(tbl_psu_set_psu_field(p_db_psu, TBL_PSU_FLD_FIXED_STATUS));
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_psu_set_iter_status
 * Purpose      : set psu info from lai layer to db
 * Input        : p_db_psu, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_psu_get_iter_status(tbl_psu_t* p_db_psu, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_psu_api_t*  psu_api = NULL;
    lai_psu_info_t lai_psu;
    lai_object_id_t oid;

    LSRV_PSU_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_psu)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_PSU,(void**)&psu_api));
    oid = p_db_psu->key.id;
    

    PM_E_RETURN(psu_api->get_psu_info(oid, &lai_psu));
    PM_E_RETURN(lsrv_psu_update_psu(p_db_psu, &lai_psu));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_psu_routine_handle
 * Purpose      : poll all psu status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_psu_routine_handle()
{
    tbl_psu_iterate((TBL_ITER_CB_FUNC) lsrv_psu_get_iter_status, NULL);

    return PM_E_NONE;
}



