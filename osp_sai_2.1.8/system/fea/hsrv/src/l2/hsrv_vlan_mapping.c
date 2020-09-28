

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
#include "hsrv_brgif.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"
#include "saivlanmap.h"


int32
hsrv_basic_qinq_add_delete_default_entry(tbl_interface_t *p_if, vid_t pvid, 
        uint32 add, uint32 replace)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));
    
    sai_vm_entry.def_vid = pvid;
    sai_vm_entry.port_id = p_if->portid;
    sai_vm_entry.replace = replace;
    sai_vm_entry.default_entry = TRUE;

    if (add)
    {
        HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_set_default_vid(tbl_interface_t *p_if, vid_t pvid)
{
    sai_port_api_t*  port_api = NULL;
    sai_attribute_t  attr[1];
    sai_object_id_t  port = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    port = p_if->portid;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_PORT_ATTR_PORT_VLAN_ID;
    attr[0].value.u16 = pvid;
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port, attr));

    return HSRV_E_NONE;
}


int32
hsrv_vlanmap_apply_vm_entry_for_defvid(tbl_interface_t *p_if, vid_t pvid, uint32 add)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt;
    tbl_vlan_mapping_entry_t    *p_db_vme;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    uint32 find_entry = FALSE;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    if (0 == sal_strlen(p_if->brgif->vlan_mapping_table))
    {
        return HSRV_E_NONE;
    }
    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    sal_strncpy(vmt_key.name, p_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vme, listnode, next)
    {
        if (!sal_strncmp(p_db_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            if (p_db_vme->mapped_svid == pvid || p_db_vme->key.raw_vid == pvid)
            {
                find_entry = TRUE;
                break;
            }
        }
    }

    if (!find_entry)
    {
        if (add)
        {
            HSRV_IF_ERROR_RETURN(hsrv_vlanmap_set_default_vid(p_if, pvid));
        }
        return HSRV_E_NONE;
    }

    if (add)
    {
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_set_default_vid(p_if, p_db_vme->key.raw_vid));
    }

    if (add)
    {
        sai_vm_entry.untag_pvid = !p_if->brgif->dot1q_tag_native_en;
    }
    if (p_db_vme->key.raw_vid == pvid)
    {
        pvid = p_db_vme->mapped_svid;
    }
    sai_vm_entry.def_vid = pvid;
    sai_vm_entry.port_id = p_if->portid;
    sai_vm_entry.replace = TRUE;
    sai_vm_entry.raw_vid = p_db_vme->key.raw_vid;
    sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
    sai_vm_entry.mapped_cvlan = p_db_vme->mapped_cvid;
    sai_vm_entry.change_defvid = TRUE;
    
    if (add)
    {
        HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_if_set_pvid(tbl_interface_t *p_if, vid_t pvid)
{
    if (GLB_VLAN_PORT_TYPE_QINQ == p_if->vlan_type
        && GLB_VLAN_QINQ_TYPE_SELECTIVE != p_if->brgif->qinq_type
        && GLB_MIN_VLAN_ID != pvid)
    {
        HSRV_IF_ERROR_RETURN(hsrv_basic_qinq_add_delete_default_entry(p_if, pvid, TRUE, TRUE));
    }

    if (GLB_VLAN_PORT_TYPE_TRUNK == p_if->vlan_type && p_if->brgif->vlan_translation_en)
    {
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_apply_vm_entry_for_defvid(p_if, p_if->brgif->pvid, FALSE));    
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_apply_vm_entry_for_defvid(p_if, pvid, TRUE));
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_if_set_untag_pvid(tbl_interface_t *p_if, bool untag_pvid)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt;
    tbl_vlan_mapping_entry_t    *p_db_vme;
    tbl_vlan_mapping_entry_master_t *p_master = tbl_vlan_mapping_entry_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    if ((0 == sal_strlen(p_if->brgif->vlan_mapping_table))
      || ((GLB_IF_TYPE_PORT_IF != p_if->hw_type) && (GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type))
      || (GLB_VLAN_PORT_TYPE_TRUNK != p_if->vlan_type)
      || (!p_if->brgif->vlan_translation_en))
    {
        return HSRV_E_NONE;
    }

    sal_strncpy(vmt_key.name, p_if->brgif->vlan_mapping_table, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    CTCLIB_SLIST_LOOP_DEL(p_master->vlan_mapping_entry_list, p_db_vme, listnode, next)
    {
        if (!sal_strncmp(p_db_vme->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX))
        {
            if (p_if->brgif->pvid != p_db_vme->mapped_svid)
            {
                continue;
            }

            sai_vm_entry.def_vid  = p_if->brgif->pvid;
            sai_vm_entry.port_id  = p_if->portid;
            sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
            sai_vm_entry.raw_vid  = p_db_vme->key.raw_vid;
            sai_vm_entry.replace  = TRUE;
            sai_vm_entry.change_defvid = TRUE;
            sai_vm_entry.untag_pvid    = untag_pvid;

            HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
            return HSRV_E_NONE;
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_create_vlanmap_entry(tbl_vlan_mapping_entry_t *p_db_vme)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    uint32 applied_ports_bmp_zero    = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;
    ds_lag_t           *p_lag = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    tbl_mapped_vlan_info_t *p_mapped_info = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next = NULL;
    uint32  add_egress_scl = TRUE;
    uint32  check_egress_scl = FALSE;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_memset(&if_key, 0, sizeof(if_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    sal_strncpy(vmt_key.name, p_db_vme->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    GLB_BMP_CHECK_ALL_ZERO(p_db_vmt->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vmt->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vmt->applied_ports, ifindex))
            {
                HSRV_IF_ERROR_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key);
                HSRV_PTR_CHECK(p_db_if);

                sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                sai_vm_entry.def_vid  = p_db_if->brgif->pvid;
                sai_vm_entry.port_id  = p_db_if->portid;
                sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
                sai_vm_entry.mapped_cvlan = p_db_vme->mapped_cvid;
                sai_vm_entry.raw_vid  = p_db_vme->key.raw_vid;

                if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
                {
                    sai_vm_entry.replace  = FALSE;
                }
                else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
                {
                    sai_vm_entry.replace  = TRUE;
                }
                else
                {
                    HSRV_IF_DEBUG("Interface %s not configured trunk port or selective QinQ", p_db_if->key.name);
                }

                HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));

                if (GLB_MAX_VLAN_ID == sai_vm_entry.raw_vid)
                {
                    if (INTERFACE_IS_LAG_GROUP(p_db_if))
                    {
                        p_lag = p_db_if->lag;
                        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
                        {
                            sai_vm_entry.port_id = p_if_mem->portid;
                            HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                        }
                    }
                }

                if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
                {
                    /* update for and vlan translation default entry */
                    if (p_db_vme->mapped_svid == p_db_if->brgif->pvid)
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_if_set_pvid(p_db_if, p_db_if->brgif->pvid));
                    }
                    else if (p_db_vme->key.raw_vid == p_db_if->brgif->pvid)
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_if_set_pvid(p_db_if, p_db_vme->mapped_svid));
                    }
                }

                if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_db_if->brgif->qinq_type && GLB_VLAN_NONE == p_db_vme->mapped_cvid)
                {
                    if (!check_egress_scl)
                    {
                        CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_mapped_info, listnode1, next)
                        {
                            if (!sal_strncmp(p_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
                             && !sal_strncmp(p_mapped_info->key.evc_name, p_db_vme->evc_entry, GLB_VLAN_EVC_NAME_LEN))
                            {
                                add_egress_scl = FALSE;
                                break;
                            }

                            if (!sal_strncmp(p_mapped_info->key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
                             && !sal_strncmp(p_mapped_info->key.evc_name, p_db_vmt->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN))
                            {
                                add_egress_scl = FALSE;
                                break;
                            }
                        }

                        check_egress_scl = TRUE;
                    }

                    if (add_egress_scl)
                    {
                        sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                        sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
                        sai_vm_entry.port_id = p_db_if->portid;
                        sai_vm_entry.egress  = TRUE;
                        sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
                        HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                    }
                }
            }
        }
        GLB_BMP_ITER_END(p_db_vmt->applied_ports, ifindex);
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_delete_vlanmap_entry(tbl_vlan_mapping_entry_t *p_db_vm_entry)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt = NULL;
    tbl_vlan_mapping_entry_t    *p_db_vme = NULL;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    uint32 applied_ports_bmp_zero    = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;
    ds_lag_t           *p_lag = NULL;
    tbl_interface_t    *p_if_mem = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mapped_vlan_info_t *p_mapped_info = NULL;
    tbl_mapped_vlan_info_key_t   mapped_info_key;
    uint32  del_egress_scl = TRUE;
    uint32  check_egress_scl = FALSE;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_memset(&if_key, 0, sizeof(if_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    sal_strncpy(vmt_key.name, p_db_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    p_db_vme = tbl_vlan_mapping_entry_get_vlan_mapping_entry(&p_db_vm_entry->key);
    HSRV_PTR_CHECK(p_db_vme);

    GLB_BMP_CHECK_ALL_ZERO(p_db_vmt->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vmt->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vmt->applied_ports, ifindex))
            {
                HSRV_IF_ERROR_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key); 
                HSRV_PTR_CHECK(p_db_if);
                sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                sai_vm_entry.def_vid  = p_db_if->brgif->pvid;
                sai_vm_entry.port_id  = p_db_if->portid;
                sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
                sai_vm_entry.mapped_cvlan = p_db_vme->mapped_cvid;
                sai_vm_entry.raw_vid  = p_db_vme->key.raw_vid;
                sai_vm_entry.default_fwd = p_db_if->brgif->qinq_default_fwd || p_db_if->brgif->vlan_trans_default_fwd;

                if (GLB_MAX_VLAN_ID == p_db_vme->key.raw_vid)
                {
                    sai_vm_entry.mapped_svlan = p_db_if->brgif->pvid;
                }

                if (GLB_VLAN_PORT_TYPE_QINQ == p_db_if->vlan_type)
                {
                    sai_vm_entry.replace  = FALSE;
                }
                else if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
                {
                    sai_vm_entry.replace  = TRUE;
                }
                else
                {
                    HSRV_IF_DEBUG("Interface %s not configured trunk port or selective QinQ", p_db_if->key.name);
                }
                
                HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));

                if (GLB_MAX_VLAN_ID == sai_vm_entry.raw_vid)
                {
                    if (INTERFACE_IS_LAG_GROUP(p_db_if))
                    {
                        p_lag = p_db_if->lag;
                        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
                        {
                            sai_vm_entry.port_id = p_if_mem->portid;
                            HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                        }
                    }
                }

                if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type && 
                    (p_db_vme->mapped_svid == p_db_if->brgif->pvid || p_db_vme->key.raw_vid == p_db_if->brgif->pvid))
                {
                    /* update for and vlan translation default entry */
                    hsrv_vlanmap_apply_vm_entry_for_defvid(p_db_if, p_db_if->brgif->pvid, FALSE);
                }

                if (GLB_VLAN_QINQ_TYPE_SELECTIVE == p_db_if->brgif->qinq_type && GLB_VLAN_NONE == p_db_vme->mapped_cvid)
                {
                    if (!check_egress_scl)
                    {
                        sal_memset(&mapped_info_key, 0, sizeof(mapped_info_key));
                        sal_strncpy(mapped_info_key.table_name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
                        sal_strncpy(mapped_info_key.evc_name, p_db_vme->evc_entry, GLB_VLAN_EVC_NAME_LEN);
                        p_mapped_info = tbl_mapped_vlan_info_get_mapped_vlan_info(&mapped_info_key);

                        if (p_mapped_info)
                        {
                            del_egress_scl = FALSE;
                        }

                        if (p_db_vmt->group_id && !sal_strncmp(p_db_vme->evc_entry, p_db_vmt->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN))
                        {
                            del_egress_scl = FALSE;
                        }

                        check_egress_scl = TRUE;
                    }

                    if (del_egress_scl)
                    {
                        sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                        sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
                        sai_vm_entry.port_id = p_db_if->portid;
                        sai_vm_entry.egress  = TRUE;
                        sai_vm_entry.mapped_svlan = p_db_vme->mapped_svid;
                        sai_vm_entry.mapped_cvlan = GLB_INVALID_VLANID;
                        HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                    }
                }
            }
        }
        GLB_BMP_ITER_END(p_db_vmt->applied_ports, ifindex);
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlanmap_entry_sync
 * Purpose      : load vlan mapping entry cdb to sai          
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
hsrv_vlanmap_entry_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_mapping_entry_t    *p_db_vme;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    p_db_vme = (tbl_vlan_mapping_entry_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_delete_vlanmap_entry(p_db_vme));
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}


int32
hsrv_vlanmap_entry_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_mapping_entry_t    *p_db_vme;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_db_vme = (tbl_vlan_mapping_entry_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_create_vlanmap_entry(p_db_vme));
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}


int32
hsrv_vlanmap_create_egress_vlanmap_entry(tbl_egress_mapping_entry_t *p_db_vme)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    uint32 applied_ports_bmp_zero    = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_memset(&if_key, 0, sizeof(if_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    sal_strncpy(vmt_key.name, p_db_vme->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    GLB_BMP_CHECK_ALL_ZERO(p_db_vmt->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vmt->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vmt->applied_ports, ifindex))
            {
                HSRV_IF_ERROR_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key);
                HSRV_PTR_CHECK(p_db_if);

                if (GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type || GLB_VLAN_QINQ_TYPE_SELECTIVE != p_db_if->brgif->qinq_type)
                {
                    continue;
                }

                sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                sai_vm_entry.egress   = TRUE;
                sai_vm_entry.port_id  = p_db_if->portid;
                sai_vm_entry.mapped_cvlan = p_db_vme->key.raw_cvid;
                sai_vm_entry.mapped_svlan = p_db_vme->key.raw_svid;
                sai_vm_entry.raw_vid = p_db_vme->mapped_vid;

                HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
            }
        }
        GLB_BMP_ITER_END(p_db_vmt->applied_ports, ifindex);
    }

    return HSRV_E_NONE;
}


int32
hsrv_vlanmap_delete_egress_vlanmap_entry(tbl_egress_mapping_entry_t *p_db_vm_entry)
{
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vmt = NULL;
    tbl_egress_mapping_entry_t  *p_db_vme = NULL;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    uint32 applied_ports_bmp_zero = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;

    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_memset(&if_key, 0, sizeof(if_key));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    sal_strncpy(vmt_key.name, p_db_vm_entry->key.table_name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vmt = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vmt);

    p_db_vme = tbl_egress_mapping_entry_get_egress_mapping_entry(&p_db_vm_entry->key);
    HSRV_PTR_CHECK(p_db_vme);

    GLB_BMP_CHECK_ALL_ZERO(p_db_vmt->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vmt->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vmt->applied_ports, ifindex))
            {
                HSRV_IF_ERROR_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key);
                HSRV_PTR_CHECK(p_db_if);

                if (GLB_VLAN_PORT_TYPE_QINQ != p_db_if->vlan_type || GLB_VLAN_QINQ_TYPE_SELECTIVE != p_db_if->brgif->qinq_type)
                {
                    continue;
                }

                sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                sai_vm_entry.egress   = TRUE;
                sai_vm_entry.port_id  = p_db_if->portid;
                sai_vm_entry.mapped_cvlan = p_db_vme->key.raw_cvid;
                sai_vm_entry.mapped_svlan = p_db_vme->key.raw_svid;
                sai_vm_entry.raw_vid = p_db_vme->mapped_vid;

                HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
            }
        }
        GLB_BMP_ITER_END(p_db_vmt->applied_ports, ifindex);
    }

    return HSRV_E_NONE;
}



int32
hsrv_vlanmap_egress_entry_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_egress_mapping_entry_t    *p_db_vme;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    p_db_vme = (tbl_egress_mapping_entry_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_delete_egress_vlanmap_entry(p_db_vme));
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_egress_entry_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_egress_mapping_entry_t    *p_db_vme;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_db_vme = (tbl_egress_mapping_entry_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_create_egress_vlanmap_entry(p_db_vme));
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_table_set_group_id(tbl_vlan_mapping_table_t *p_db_vmt, tbl_vlan_group_t *p_db_vg, uint32 is_add)
{
    sai_port_api_t    *port_api    = NULL;
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_attribute_t    attr[1];
    sai_vlanmap_entry_t     sai_vm_entry;
    tbl_vlan_mapping_table_key_t vmt_key;
    tbl_vlan_mapping_table_t    *p_db_vm_table = NULL;
    tbl_vlan_group_key_t vg_key;
    
    uint32  i = 0;
    uint32 applied_ports_bmp_zero    = TRUE;
    uint32 ifindex = 0;
    tbl_interface_key_t if_key;
    tbl_interface_t    *p_db_if = NULL;

    tbl_mapped_vlan_info_master_t *p_mapped_master = tbl_mapped_vlan_info_get_master();
    tbl_mapped_vlan_info_t *p_mapped_info = NULL;
    tbl_mapped_vlan_info_key_t   mapped_info_key;
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next = NULL;
    uint32  add_egress_scl = TRUE;
    uint32  del_egress_scl = TRUE;
    uint32  check_egress_scl = FALSE;

    sal_memset(&vmt_key, 0, sizeof(vmt_key));
    sal_memset(&vg_key, 0, sizeof(vg_key));
    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT, (void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));

    sal_strncpy(vmt_key.name, p_db_vmt->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
    p_db_vm_table = tbl_vlan_mapping_table_get_vlan_mapping_table(&vmt_key);
    HSRV_PTR_CHECK(p_db_vm_table);
    
    if (!is_add)
    {
        if (!p_db_vm_table->group_id)
        {
            HSRV_IF_DEBUG("VLAN group profile %d not exist", p_db_vm_table->group_id);
        }

        vg_key.group_id = p_db_vm_table->group_id;
        p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key);
        HSRV_PTR_CHECK(p_db_vg);
    }

    GLB_BMP_CHECK_ALL_ZERO(p_db_vm_table->applied_ports, applied_ports_bmp_zero);
    if(!applied_ports_bmp_zero)
    {
        GLB_BMP_ITER_BEGIN(p_db_vm_table->applied_ports, ifindex)
        {
            if (GLB_BMP_ISSET(p_db_vm_table->applied_ports, ifindex))
            {
                HSRV_IF_ERROR_RETURN(tbl_interface_get_name_by_ifindex(ifindex, if_key.name, IFNAME_SIZE));
                p_db_if = tbl_interface_get_interface(&if_key);
                HSRV_PTR_CHECK(p_db_if);
                sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                sai_vm_entry.def_vid  = p_db_if->brgif->pvid;
                sai_vm_entry.port_id  = p_db_if->portid;
                sai_vm_entry.replace  = FALSE;
                sai_vm_entry.group_id = p_db_vg->key.group_id;
                sai_vm_entry.mapped_svlan = p_db_vm_table->group_mapped_svid;
                for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
                {
                    if (p_db_vg->valid[i])
                    {
                        sai_vm_entry.raw_vid = p_db_vg->min_vid[i];
                        sai_vm_entry.raw_vid_max = p_db_vg->max_vid[i];
                    }
                    else
                    {
                        continue;
                    }

                    if (is_add)
                    {
                        HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                    }
                    else
                    {
                        HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                    }
                }
                HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlan_range_en(p_db_if, p_db_vg->key.group_id, is_add));
            }

            if (!check_egress_scl)
            {
                if (is_add)
                {
                    CTCLIB_SLIST_LOOP_DEL(p_mapped_master->mapped_vlan_info_list, p_mapped_info, listnode1, next)
                    {
                        if (!sal_strncmp(p_mapped_info->key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX)
                         && !sal_strncmp(p_mapped_info->key.evc_name, p_db_vm_table->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN))
                        {
                            add_egress_scl = FALSE;
                            break;
                        }
                    }
                }
                else
                {
                    sal_memset(&mapped_info_key, 0, sizeof(mapped_info_key));
                    sal_strncpy(mapped_info_key.table_name, p_db_vm_table->key.name, GLB_VLAN_MAPPING_TAB_NAME_MAX);
                    sal_strncpy(mapped_info_key.evc_name, p_db_vm_table->group_evc_entry_name, GLB_VLAN_EVC_NAME_LEN);
                    p_mapped_info = tbl_mapped_vlan_info_get_mapped_vlan_info(&mapped_info_key);
                    if (p_mapped_info)
                    {
                        del_egress_scl = FALSE;
                    }
                }
                
                check_egress_scl = TRUE;
            }

            if (is_add)
            {
                if (add_egress_scl)
                {
                    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                    sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
                    sai_vm_entry.port_id = p_db_if->portid;
                    sai_vm_entry.egress  = TRUE;
                    sai_vm_entry.mapped_svlan = p_db_vm_table->group_mapped_svid;
                    HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlanmap_entry(&sai_vm_entry, attr));
                }
            }
            else
            {
                if (del_egress_scl)
                {
                    sal_memset(&sai_vm_entry, 0, sizeof(sai_vm_entry));
                    sai_vm_entry.raw_vid = GLB_INVALID_VLANID;
                    sai_vm_entry.port_id = p_db_if->portid;
                    sai_vm_entry.egress  = TRUE;
                    sai_vm_entry.mapped_svlan = p_db_vm_table->group_mapped_svid;
                    sai_vm_entry.mapped_cvlan = GLB_INVALID_VLANID;
                    HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlanmap_entry(&sai_vm_entry, attr));
                }
            }
        }
        GLB_BMP_ITER_END(p_db_vm_table->applied_ports, ifindex);
    }

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_vlanmap_table_sync
 * Purpose      : load vlan mapping table cdb to sai          
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
hsrv_vlanmap_table_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_mapping_table_t     *p_db_vmt = NULL;
    tbl_vlan_group_key_t          vg_key;
    tbl_vlan_group_t             *p_db_vg = NULL;
    uint32   is_add = FALSE;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        /* no need to subscribe del, we subscribed vm entry delete event */
        break;

    case CDB_OPER_SET:
        p_db_vmt = (tbl_vlan_mapping_table_t*)p_tbl;
        switch (field_id)
        {
        case TBL_VLAN_MAPPING_TABLE_FLD_GROUP_ID:
            if (p_db_vmt->group_id)
            {
                is_add = TRUE;
                sal_memset(&vg_key, 0, sizeof(vg_key));
                vg_key.group_id = p_db_vmt->group_id;
                p_db_vg = tbl_vlan_group_get_vlan_group(&vg_key); 
                HSRV_PTR_CHECK(p_db_vg);
            }
            
            HSRV_IF_ERROR_RETURN(hsrv_vlanmap_table_set_group_id(p_db_vmt, p_db_vg, is_add));
            break;

        /* no need to subscribe applied ports, subscribe brgif->vmt_name and the vm entry add/del */
        default:
            break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}


int32
hsrv_vlanmap_create_vlan_group(tbl_vlan_group_t *p_db_vg)
{
    sai_vlanmap_api_t *vlanmap_api = NULL;
    sai_vlan_group_t   vlan_group;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));
    HSRV_PTR_CHECK(p_db_vg);
    uint32 i = 0;

    sal_memset(&vlan_group, 0, sizeof(vlan_group));

    vlan_group.group_id = p_db_vg->key.group_id;
    for (i = 0; i < GLB_VLAN_RANGE_ENTRY_NUM; i++)
    {
        if (p_db_vg->valid[i])
        {
            vlan_group.vlanmin[i] = p_db_vg->min_vid[i];
            vlan_group.vlanmax[i] = p_db_vg->max_vid[i];
        }
    }

    HSRV_IF_ERROR_RETURN(vlanmap_api->create_vlan_group(&vlan_group));
    return HSRV_E_NONE;
}

int32
hsrv_vlanmap_delete_vlan_group(tbl_vlan_group_t *p_db_vg)
{
    sai_vlanmap_api_t *vlanmap_api = NULL;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLANMAP, (void**)&vlanmap_api));
    HSRV_PTR_CHECK(p_db_vg);

    HSRV_IF_ERROR_RETURN(vlanmap_api->delete_vlan_group(p_db_vg->key.group_id));

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_group_sync
 * Purpose      : load vlan group cdb to sai          
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
hsrv_vlan_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_vlan_group_t    *p_db_vg = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_db_vg = (tbl_vlan_group_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_vlanmap_create_vlan_group(p_db_vg));
        break;

    case CDB_OPER_DEL:
        p_db_vg = (tbl_vlan_group_t*)p_tbl;
         HSRV_IF_ERROR_RETURN(hsrv_vlanmap_delete_vlan_group(p_db_vg));
        break;

    case CDB_OPER_SET:
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

