/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "ctc_api.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"

/***************************************************************************************************
 * Name         : _hsrv_macfilter_check_fdb_exist_by_mac
 * Purpose      : check whether the mac is exist in fdb table
 * Input        : mac, mac_addr_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static int32
_hsrv_macfilter_check_fdb_exist_by_mac(mac_addr_t mac)
{
    ctclib_hash_backet_t *hb_next = NULL;
    ctclib_hash_backet_t *pst_hb = NULL;
    tbl_fea_fdb_master_t* p_master = tbl_fea_fdb_get_master();
    tbl_fea_fdb_t* p_db_fdb = NULL;
    int32 index = 0;
    bool entry_exist = FALSE;

    for (index = 0; index < p_master->fdb_hash->size; index++)
    {
        for (pst_hb = p_master->fdb_hash->index[index]; pst_hb; pst_hb = hb_next)
        {
            hb_next = pst_hb->next;
            if (!pst_hb->data)
            {
                continue;
            }
            p_db_fdb = (tbl_fea_fdb_t*)pst_hb->data;
            if (0 == sal_memcmp(p_db_fdb->key.mac, mac, MAC_ADDR_LEN))
            {
                entry_exist = TRUE;
            }
        }
    }

    if (entry_exist)
    {
        return HSRV_E_NONE;
    }
    else
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
}

/***************************************************************************************************
 * Name         : hsrv_macfilter_add_mac
 * Purpose      : creat static blackhole mac
 * Input        : p_macfilter,tbl_macfilter_t;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_macfilter_add_mac(tbl_macfilter_t* p_macfilter)
{
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    sai_attribute_t macfilter_attr[1];
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    
    macfilter_attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    macfilter_attr[0].value.s32 = SAI_FDB_ENTRY_MACFILTER;

    /*added by yejl to fix bug 38783, check whether mac is exist in fdb table*/
    rc = _hsrv_macfilter_check_fdb_exist_by_mac(p_macfilter->key.mac);
    if (HSRV_E_NONE == rc)
    {
        log_sys(M_MOD_FDB, E_WARNING, "WARNING: the mac %02X:%02X:%02X:%02X:%02X:%02X has existed in FDB entries when adding mac-filter entry\n", 
            p_macfilter->key.mac[0], p_macfilter->key.mac[1], p_macfilter->key.mac[2], 
            p_macfilter->key.mac[3], p_macfilter->key.mac[4], p_macfilter->key.mac[5]);
    }

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    sal_memcpy(l2_addr.mac_address, p_macfilter->key.mac, sizeof(mac_addr_t));
    HSRV_IF_ERROR_RETURN(fdb_api->create_macfilter_fdb(&l2_addr, sizeof(macfilter_attr)/sizeof(macfilter_attr[0]), macfilter_attr));

    /*added by yejl to fix bug38783, clush dynamic fdb by mac*/
    rc = hsrv_fdb_flush_fdb_mac(p_macfilter->key.mac);

    return rc;

}


/***************************************************************************************************
 * Name         : hsrv_macfilter_del_mac
 * Purpose      : del static blackhole mac
 * Input        : p_macfilter, tbl_macfilter_t
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_macfilter_del_mac(tbl_macfilter_t* p_macfilter)
{    
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    sal_memcpy(l2_addr.mac_address, p_macfilter->key.mac, sizeof(mac_addr_t));
    HSRV_IF_ERROR_RETURN(fdb_api->remove_macfilter_fdb(&l2_addr));
    
    return rc;
}

/***************************************************************************************************
 * Name         : hsrv_macfilter_sync 
 * Purpose      : load macfilter fdb cdb to sai          
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
hsrv_macfilter_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_macfilter_t *      p_macfilter = NULL;
    
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    p_macfilter = (tbl_macfilter_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_FDB_DEBUG("macfilter add entry mac = %02X%02X.%02X%02X.%02X%02X",
            p_macfilter->key.mac[0], p_macfilter->key.mac[1], p_macfilter->key.mac[2], p_macfilter->key.mac[3], 
            p_macfilter->key.mac[4], p_macfilter->key.mac[5]);
        HSRV_IF_ERROR_RETURN(hsrv_macfilter_add_mac(p_macfilter));
        break;

    case CDB_OPER_DEL:
       HSRV_FDB_DEBUG("macfilter del entry mac = %02X%02X.%02X%02X.%02X%02X",
            p_macfilter->key.mac[0], p_macfilter->key.mac[1], p_macfilter->key.mac[2], p_macfilter->key.mac[3], 
            p_macfilter->key.mac[4], p_macfilter->key.mac[5]);
       HSRV_IF_ERROR_RETURN(hsrv_macfilter_del_mac(p_macfilter));
       break;

    case CDB_OPER_SET:
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_macfilter_cmd_show_macfilter
 * Purpose      : show the information of blackhole mac
 * Input        : argv, argc,and para
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32 
hsrv_macfilter_cmd_show_macfilter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_sys_spec_t * p_sys_spec = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *nextnode = NULL;
    tbl_macfilter_t *p_db_macflt = NULL;
    tbl_macfilter_master_t* p_master = tbl_macfilter_get_master();
    char mac_str[CMD_BUF_32];
    int32 type = GLB_STM_MODE_DEFAULT;
    FILE *fp = NULL;

    sal_memset(mac_str, 0, sizeof(mac_str));
    p_sys_spec = tbl_sys_spec_get_sys_spec_by_type(type);
    if(NULL == p_sys_spec)
    {
        return HSRV_E_NO_MEMORY;
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf (fp, "     MAC Filter Address Table     \n");
    sal_fprintf (fp, "----------------------------------\n");
    sal_fprintf (fp, "%-20s: %u\n", "Current count  ", p_master->macflt_list->count);
    sal_fprintf (fp, "%-20s: %u\n", "Max count      ", p_sys_spec->mac_filter);
    sal_fprintf (fp, "%-20s: %u\n", "Left count     ", p_sys_spec->mac_filter - p_master->macflt_list->count);
    sal_fprintf (fp, "----------------------------------\n");

    sal_fprintf (fp, "%s:\n", "Mac Address");
    CTCLIB_SLIST_LOOP_DEL(p_master->macflt_list, p_db_macflt, listnode, nextnode)
    {
        sal_fprintf (fp, "%-16s\n", cdb_mac_addr_val2str(mac_str, CMD_BUF_32, p_db_macflt->key.mac));
    }
    sal_fclose(fp);
    
    return HSRV_E_NONE;
}

int32
hsrv_macfilter_start()
{
    cdb_register_cfg_act_cb(ACT_SHOW_MACFILTER, hsrv_macfilter_cmd_show_macfilter);
    return HSRV_E_NONE;
}

