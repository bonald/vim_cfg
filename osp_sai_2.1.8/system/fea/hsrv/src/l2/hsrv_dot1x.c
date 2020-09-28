/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
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
#include "hsrv_stp.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "hsrv_dot1x.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
static hsrv_dot1x_mac_db_t g_hsrv_dot1x_mac_db;

/******************************************************************************
* Defines and Macros
******************************************************************************/
int32
hsrv_dot1x_port_mac_en(tbl_dot1x_port_t  *p_dot1x_port)
{
    bool                       add = FALSE;
    bool                       remove = FALSE;
    bool                       update = FALSE;
    ctc_scl_default_action_t   dft_action;
    ctc_port_scl_property_t    scl_prop;
    uint32                     group_id = 0;
    tbl_interface_t            *p_db_if = NULL;
    ctc_scl_group_info_t       group_info;
    tbl_dot1x_port_t           *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t           dot1x_port;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_dot1x_port->key.ifindex);
    if (!p_db_if)
    {
        HSRV_LOG_ERR("hsrv_dot1x_port_mac_en fail because Invalid interface or interface does not exist");
        return HSRV_E_INVALID_PARAM;
    }

    dot1x_port.key.ifindex = p_dot1x_port->key.ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        HSRV_LOG_ERR("802.1x not configured on this interface!");
        return HSRV_E_NONE;
    }

    if (AUTH_PORT_MODE_MAC != p_dot1x_port->auth_mode)
    {
        if (0 != p_db_dot1x_port->class_id)
        {
            remove = TRUE;
        }
        else
        {
            return HSRV_E_NONE;
        }
    }
    else 
    {
        if (p_dot1x_port->auth_mode != p_db_dot1x_port->auth_mode)
        {
            /* enable mac mode */
            add = TRUE;
        }
        else
        {
            /* update mac mode */  
            update = TRUE;
        }
    }

    if ((remove || update) && (0 != p_db_dot1x_port->class_id))
    {
        /* disable mac mode*/ 
        sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.class_id_en = 0;
        scl_prop.class_id = 0;
        HSRV_IF_ERROR_RETURN(ctc_port_set_scl_property(p_db_if->portid, &scl_prop));
        
        /*remove old default entry*/        
        sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
        dft_action.gport = p_db_if->portid;
        dft_action.action.type = CTC_SCL_ACTION_INGRESS;
        dft_action.action.u.igs_action.flag = 0;
        HSRV_IF_ERROR_RETURN(ctc_scl_set_default_action(&dft_action));

        /*remove group*/
        group_id = HSRV_DOT1X_GROUP_BASE | p_db_if->portid;
        HSRV_IF_ERROR_RETURN(ctc_scl_uninstall_group (group_id));
        HSRV_IF_ERROR_RETURN(ctc_scl_destroy_group (group_id));

        if (remove)
        {
            ctclib_opf_free_offset(HSRV_DOT1X_CLASS_OPF, 0, 1, p_db_dot1x_port->class_id);
            p_db_dot1x_port->class_id = 0;
        }
    }
    
    if (add || update)
    {
        if (add)
        {
            p_db_dot1x_port->class_id = ctclib_opf_alloc_offset(HSRV_DOT1X_CLASS_OPF, 0, 1);
        }
        
        /*add default entry*/
        sal_memset(&dft_action, 0, sizeof(ctc_scl_default_action_t));
        dft_action.gport = p_db_if->portid;
        dft_action.action.type = CTC_SCL_ACTION_INGRESS;
        
        if (p_dot1x_port->mac_auth_bypass)/*mac auth by pass: redirect to cpu*/
        {
            GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_COPY_TO_CPU);
        }
        
        if (p_dot1x_port->guest_vlan)/*enable guest vlan: change the vlan*/
        {
            GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT);
            dft_action.action.u.igs_action.vlan_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
            dft_action.action.u.igs_action.vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;         
            dft_action.action.u.igs_action.vlan_edit.svid_new = p_dot1x_port->guest_vlan;
        }
        else
        {
            GLB_SET_FLAG(dft_action.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
        }        

        /*SDK code can not update default entries with this function*/
        HSRV_IF_ERROR_RETURN(ctc_scl_set_default_action(&dft_action));
        
        /*add group*/
        group_id = HSRV_DOT1X_GROUP_BASE | p_db_if->portid;
        sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
        group_info.type = CTC_SCL_GROUP_TYPE_PORT_CLASS;
        group_info.un.port_class_id = p_db_dot1x_port->class_id;
        HSRV_IF_ERROR_RETURN(ctc_scl_create_group(group_id, &group_info));
        HSRV_IF_ERROR_RETURN(ctc_scl_install_group(group_id, &group_info));

        /*enable*/
        sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
        scl_prop.scl_id = 0;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.class_id_en = 0;
        scl_prop.class_id = p_db_dot1x_port->class_id;
        HSRV_IF_ERROR_RETURN(ctc_port_set_scl_property(p_db_if->portid, &scl_prop));
    }     
    
    return HSRV_E_NONE;
}

int32
hsrv_dot1x_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_dot1x_port_t        *p_dot1x_port = NULL;
    int32                   rc = HSRV_E_NONE;
            
    switch (oper)
    {
    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_DOT1X_PORT_FLD_AUTH_MODE:  
            p_dot1x_port = (tbl_dot1x_port_t*)p_tbl;
            rc = hsrv_dot1x_port_mac_en(p_dot1x_port);
            if (rc)
            {
                HSRV_LOG_ERR("hsrv_dot1x_port_mac_en return rc %d", rc);
                return rc;  
            }
            break;

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
hsrv_dot1x_mac_set_entry(tbl_dot1x_mac_t  *p_dot1x_mac, tbl_dot1x_mac_t  *p_db_dot1x_mac)
{
    ctc_scl_entry_t         scl_entry;
    int                     ret = 0;
    int                     group_id = 0;
    tbl_interface_t         *p_db_if = NULL;
    bool                    update = TRUE;
    tbl_dot1x_port_t        dot1x_port;
    tbl_dot1x_port_t        *p_db_dot1x_port = NULL;
   
    /*2. fill param*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    p_db_if = tbl_interface_get_interface_by_ifindex(p_dot1x_mac->key.ifindex);
    if (!p_db_if)
    {
        HSRV_LOG_ERR("hsrv_dot1x_port_mac_en fail because Invalid interface or interface does not exist");
        return HSRV_E_INVALID_PARAM;
    }

    dot1x_port.key.ifindex = p_dot1x_mac->key.ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        HSRV_LOG_ERR("802.1x not configured on this interface!");
        return HSRV_E_NONE;
    }

    if (0 == p_db_dot1x_mac->entry_id)
    {
        update = FALSE;    
        p_db_dot1x_mac->entry_id = ctclib_opf_alloc_offset(HSRV_DOT1X_LABEL_OPF, 0, 1);
    }
    
    scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_MAC;
    sal_memcpy(scl_entry.key.u.hash_port_mac_key.mac, p_dot1x_mac->key.mac, sizeof(mac_addr_t));
    scl_entry.key.u.hash_port_mac_key.mac_is_da = FALSE;
    scl_entry.key.u.hash_port_mac_key.gport = p_db_if->portid;
   
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = p_db_dot1x_mac->entry_id;
   
    HSRV_DOT1X_DEBUG("  update=%d, flag=%x, entry_id=%d, guest vlan=%d, mac=%02x%02x.%02x%02x.%02x%02x",
        update, p_dot1x_mac->auth_mac_flag, p_db_dot1x_mac->entry_id, p_db_dot1x_port->guest_vlan,
        scl_entry.key.u.hash_port_mac_key.mac[0], scl_entry.key.u.hash_port_mac_key.mac[1],
        scl_entry.key.u.hash_port_mac_key.mac[2], scl_entry.key.u.hash_port_mac_key.mac[3],
        scl_entry.key.u.hash_port_mac_key.mac[4], scl_entry.key.u.hash_port_mac_key.mac[5]);
    
    /*3. mapping actions*/
    if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
    {        
        if (p_db_dot1x_port->guest_vlan)
        {
            /*guest vlan enabled, then classify the vlan*/
            GLB_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT);
            scl_entry.action.u.igs_action.vlan_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
            scl_entry.action.u.igs_action.vlan_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;         
            scl_entry.action.u.igs_action.vlan_edit.svid_new = p_db_dot1x_port->guest_vlan;
        }
        else
        {
            /*discard the entry*/
            GLB_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_DISCARD);
        }
    }
    else if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
    {
        scl_entry.action.u.igs_action.flag = 0; /*forward*/
    }
    else if (GLB_FLAG_ISSET(p_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        GLB_SET_FLAG(scl_entry.action.u.igs_action.flag, CTC_SCL_IGS_ACTION_FLAG_COPY_TO_CPU);
    }
    else
    {
        /*invalid mac state*/
        return HSRV_E_NONE;
    }
    
    /*4. SDK can handle update with function*/
    if (update)
    {
        HSRV_IF_ERROR_RETURN(ctc_scl_update_action(p_db_dot1x_mac->entry_id, &scl_entry.action));
    }
    else
    {
        ret = ctc_scl_add_entry(CTC_SCL_GROUP_ID_HASH_PORT_MAC, &scl_entry);
        if (CTC_E_SCL_HASH_CONFLICT == ret)
        {
            group_id = HSRV_DOT1X_GROUP_BASE | p_db_if->portid;
            sal_memset(&scl_entry.key, 0, sizeof(ctc_scl_key_t));
            scl_entry.key.type = CTC_SCL_KEY_TCAM_MAC;
            scl_entry.key.u.tcam_mac_key.flag |= CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_SA;
            
            sal_memcpy(scl_entry.key.u.tcam_mac_key.mac_sa, p_dot1x_mac->key.mac, sizeof(mac_addr_t));
            sal_memset(scl_entry.key.u.tcam_mac_key.mac_sa_mask, 1, sizeof(mac_addr_t));
            
            HSRV_IF_ERROR_RETURN(ctc_scl_add_entry (group_id, &scl_entry));
        }
        else 
        {
            HSRV_IF_ERROR_RETURN(ret);
        }
        HSRV_IF_ERROR_RETURN(ctc_scl_install_entry(p_db_dot1x_mac->entry_id));
    }

    return HSRV_E_NONE;
}

int32
hsrv_dot1x_mac_unset_entry(tbl_dot1x_mac_t  *p_dot1x_mac, tbl_dot1x_mac_t  *p_db_dot1x_mac)
{
    HSRV_DOT1X_DEBUG(" hsrv_dot1x_mac_unset_entry delete entry %d",
        p_db_dot1x_mac->entry_id);

    /* modified by liwh for bug 42739, 2017-03-23 
      if dot1x mac is not added to hardware, shouldn't remove it */
    if (0 == p_db_dot1x_mac->entry_id)
    {
        return HSRV_E_NONE;
    }
    /* liwh end */
    
    ctclib_opf_free_offset(HSRV_DOT1X_LABEL_OPF, 0, 1, p_db_dot1x_mac->entry_id);

    HSRV_IF_ERROR_RETURN(ctc_scl_uninstall_entry(p_db_dot1x_mac->entry_id));
    HSRV_IF_ERROR_RETURN(ctc_scl_remove_entry(p_db_dot1x_mac->entry_id));
    return HSRV_E_NONE;
} 

int32
hsrv_dot1x_mac_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_dot1x_mac_t        *p_dot1x_mac = NULL;
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    int32                  rc = HSRV_E_NONE;

    switch (oper)
    { 
    case CDB_OPER_SET:
        p_dot1x_mac = (tbl_dot1x_mac_t*)p_tbl;
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key);
        if (!p_db_dot1x_mac)
        {
            HSRV_LOG_ERR("hsrv_dot1x_mac_sync set action return failure");
            return rc;
        }
    
        switch (field_id)
        {
        case TBL_DOT1X_MAC_FLD_AUTH_MAC_ACCEPT: 
        case TBL_DOT1X_MAC_FLD_AUTH_MAC_REJECT:
        case TBL_DOT1X_MAC_FLD_AUTH_MAC_REAUTH_ACCEPT:
            rc = hsrv_dot1x_mac_set_entry(p_dot1x_mac, p_db_dot1x_mac);
            if (rc)
            {
                HSRV_LOG_ERR("hsrv_dot1x_port_mac_en return rc %d", rc);
                return rc;  
            }
            break;

        default:
            break;
        }
        break;

    case CDB_OPER_DEL:
        p_dot1x_mac = (tbl_dot1x_mac_t*)p_tbl;
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&p_dot1x_mac->key);
        if (!p_db_dot1x_mac)
        {
            HSRV_LOG_ERR("hsrv_dot1x_mac_sync set action return failure");
            return rc;
        }

        rc = hsrv_dot1x_mac_unset_entry(p_dot1x_mac, p_db_dot1x_mac);
        if (rc)
        {
            HSRV_LOG_ERR("hsrv_dot1x_port_mac_en return rc %d", rc);
            return rc;  
        }
        break;
        
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32 
hsrv_dot1x_init()
{   
    HSRV_DOT1X_LABEL_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset(HSRV_DOT1X_LABEL_OPF, 0, HSRV_DOT1X_ENTRY_ID_MIN, HSRV_DOT1X_ENTRY_ID_MAX);

    HSRV_DOT1X_CLASS_OPF = ctclib_opf_init(1);
    ctclib_opf_init_offset(HSRV_DOT1X_CLASS_OPF, 0, HSRV_DOT1X_CLASS_ID_MIN, HSRV_DOT1X_CLASS_ID_MAX);
     
    return HSRV_E_NONE;
}

