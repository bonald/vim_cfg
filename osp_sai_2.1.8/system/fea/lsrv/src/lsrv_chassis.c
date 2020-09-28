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

int32
lsrv_chassis_reset_reboot_info_db()
{
    tbl_reboot_info_master_t *p_master = tbl_reboot_info_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_reboot_info_t *p_db_reboot_info = NULL;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->reboot_list, p_db_reboot_info, listnode, next)
    {
        tbl_reboot_info_del_reboot_info(&p_db_reboot_info->key);
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : lsrv_chassis_sync
 * Purpose      : load chassis cdb to lai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_chassis_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_chassis_t *p_chassis = NULL;

    int32 rc = 0;
    lai_card_api_t*  card_api = NULL;
    lai_led_api_t*  led_api = NULL;
    lai_object_id_t oid;
    lai_led_info_t led;
    
    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_chassis = (tbl_chassis_t*)p_tbl;
        oid = 0; /* currently we use card to save chassis infomation here. */
        switch (field_id)
        {
            case TBL_CHASSIS_FLD_NEXT_STM_MODE:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_stmmode(oid, p_chassis->next_stm_mode));
                break;  

            case TBL_CHASSIS_FLD_LOCATION_LED:
                sal_memset(&led, 0, sizeof(led));
                PM_E_RETURN(lai_api_query(LAI_API_LED,(void**)&led_api));
                led.oid = LAI_LED_LOCATION_CHASSIS;
                led.mode = p_chassis->location_led;
                PM_E_RETURN(led_api->set_led_mode(LAI_LED_LOCATION_CHASSIS, &led));
                break;  

            case TBL_CHASSIS_FLD_RESET_REBOOT_INFO:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(lsrv_chassis_reset_reboot_info_db());
                PM_E_RETURN(card_api->reset_reboot_info(oid));
                break;  
                
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : lsrv_ifname_info_sync
 * Purpose      : load chassis cdb to lai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_ifname_info_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    lai_card_api_t*  card_api = NULL;
    lai_object_id_t oid;
    lai_portname_info_t ifname_info;
    tbl_ifname_info_t *p_ifname_info = NULL;
    int32 i = 0;
    int32 rc = 0;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);

    sal_memset(&ifname_info, 0, sizeof(ifname_info));
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_ifname_info = (tbl_ifname_info_t*)p_tbl;
        switch (field_id)
        {
        /* PM should set prefix_num at last */
        case TBL_IFNAME_INFO_FLD_PREFIX_NUM:
            oid = 0; /* currently we use card to save chassis infomation here. */
            /*set ifname info once time*/
            PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
            ifname_info.connection = p_ifname_info->connection;
            ifname_info.sub_connection = p_ifname_info->sub_connection;
            ifname_info.prefix_num = p_ifname_info->prefix_num;
            for (i = 0; i < GLB_PHT_PORT_TYPE_MAX; i++)
            {
                sal_strcpy(ifname_info.prefix[i], p_ifname_info->prefix[i]);
                sal_strcpy(ifname_info.full_prefix[i], p_ifname_info->full_prefix[i]);
            }        
            PM_E_RETURN(card_api->set_ifname_info(oid, &ifname_info));
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

