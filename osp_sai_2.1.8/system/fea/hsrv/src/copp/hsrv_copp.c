/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2017-05-18
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/


/******************************************************************************
* Defines and Macros
******************************************************************************/


/***************************************************************************************************
 * Name         : hsrv_copp_cfg_set_total_rate
 * Purpose      : set copp total rate        
 * Input        : total_rate
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_copp_cfg_set_total_rate(uint32 total_rate)
{
    sai_copp_api_t*  copp_api;
    sai_attribute_t attr;
    sai_object_id_t obj_id = 0;

    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    attr.id = SAI_COPP_QUEUE_ATTR_TOTAL_RATE;
    attr.value.u64 = total_rate;
    HSRV_IF_ERROR_RETURN(copp_api->set_copp_queue_attribute(obj_id, &attr));
 
    return HSRV_E_NONE;
}



/***************************************************************************************************
 * Name         : hsrv_copp_cfg_sync
 * Purpose      : load copp cfg cdb to sai          
 * Input        : p_tbl_node: table node pointer
                     p_ds_node: ds node pointer
                     oper: operation code
                     field_id: field id with operation
                     p_tbl: cdb table pointer
                     p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_copp_cfg_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_copp_cfg_t* p_copp_cfg = NULL;
    
    HSRV_MIRROR_DEBUG("Enter into %s.", __FUNCTION__);  
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_copp_cfg = (tbl_copp_cfg_t*)p_tbl;
        switch (field_id)
        {
        case TBL_COPP_CFG_FLD_COPP_PNAME:
            break;

        case TBL_COPP_CFG_FLD_COPP_TOTAL_RATE:
            HSRV_IF_ERROR_RETURN(hsrv_copp_cfg_set_total_rate(p_copp_cfg->copp_total_rate));
            break;

        default:
            break;
        }
        break;
        
    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}


int32
hsrv_copp_start()
{
    cdb_pm_subscribe_tbl(TBL_COPP_CFG, hsrv_copp_cfg_sync, NULL);
    return HSRV_E_NONE;
}


