
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
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"

sal_mutex_t *g_hsrv_brgif_mutex = NULL;

int32
hsrv_brgif_set_mlag_id(uint32 ifindex, uint32 mlag_id)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->mlag_id = mlag_id;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_mlag_peer_conf(uint32 ifindex, uint32 enable)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->mlag_peer_conf = enable;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_peer_conf_cb(ifindex, enable);
    
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_mlag_peer_if_up(uint32 ifindex, uint32 enable)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->mlag_peer_if_up = enable;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_mlag_is_group(uint32 ifindex, uint32 is_group)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->mlag_is_group = is_group;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_mac_learning_en(uint32 ifindex, uint32 enable)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->mac_learning_en = enable;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_port_security_en(uint32 ifindex, uint32 enable)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->port_security_en = enable;
    }
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_flags(uint32 ifindex, uint32 flags)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->flags = flags;
    }
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_max_mac(uint32 ifindex, uint32 count)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->max_mac = count;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_lag_id(uint32 ifindex, uint32 lag_id)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
        p_db_if->lag_id = lag_id;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    return HSRV_E_NONE;
}

tbl_fea_brg_if_t*
hsrv_brgif_get(uint32 ifindex)
{
    tbl_fea_brg_if_key_t key;

    key.ifindex = ifindex;
    return tbl_fea_brg_if_get_fea_brg_if(&key);
}

int32
hsrv_brgif_add(tbl_interface_t *p_if, ds_brgif_t *p_brgif)
{
    tbl_fea_brg_if_t brgif;
    tbl_fea_brg_if_t *p_db_if = NULL;
    sai_object_id_t port_oid = 0;
    int32 rc = HSRV_E_NONE;

    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    /* 1. check exist */
    sal_memset(&brgif, 0, sizeof(brgif));
    
    /* 2. alloc a new db entry */
    brgif.key.ifindex = p_if->ifindex;
    brgif.flags = p_if->flags;
    brgif.hw_type = p_if->hw_type;
    brgif.port_security_en = p_brgif->port_security_en;
    brgif.mac_learning_en = p_brgif->mac_learning_en;
    brgif.max_mac = p_brgif->max_mac;
    brgif.curr_mac = 0;
    brgif.curr_mac_static = 0;
    hsrv_interface_get_objectid(p_if, &port_oid);
    brgif.portid = port_oid;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    rc = tbl_fea_brg_if_add_fea_brg_if(&brgif);
    if (rc < 0)
    {
        goto EXIT;
    }
    
    p_db_if = tbl_fea_brg_if_get_fea_brg_if(&brgif.key);
    if (NULL == p_db_if)
    {
        rc = HSRV_E_NO_MEMORY;
        goto EXIT;
    }

#ifdef FDB_SLIST
    p_db_if->fdb_list = ctclib_slist_create(NULL, NULL);
#endif

EXIT:
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return HSRV_E_NONE;    
}

int32
hsrv_brgif_del(tbl_interface_t *p_if)
{
    tbl_fea_brg_if_key_t brgif_key;
    tbl_fea_brg_if_t *p_db_if = NULL;

    if (GLB_IF_TYPE_PORT_IF != p_if->hw_type && GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    sal_mutex_lock(g_hsrv_brgif_mutex);
    /* 1. check exist */
    brgif_key.ifindex = p_if->ifindex;
    p_db_if = tbl_fea_brg_if_get_fea_brg_if(&brgif_key);
    if (NULL == p_db_if)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /* 2. remove fdb list */
#ifdef FDB_SLIST
    ctclib_slist_delete(p_db_if->fdb_list);
#endif

    /* 3. del from db */
    tbl_fea_brg_if_del_fea_brg_if(&brgif_key);
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;    
}

int32
hsrv_brgif_set_mstp_port_state(uint32 ifindex, uint32 fea_state)
{
    tbl_fea_brg_if_key_t brgif_key;
    tbl_fea_brg_if_t *p_db_if = NULL;
    uint32 instance_index = GLB_DEFAULT_INSTANCE;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    /* 1. check exist */
    brgif_key.ifindex = ifindex;
    p_db_if = tbl_fea_brg_if_get_fea_brg_if(&brgif_key);
    if (NULL == p_db_if)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        log_sys(M_MOD_HSRV, E_ERROR, "Cannot found fea_brg_if for ifindex %u", ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (GLB_BR_PORT_STATE_FORWARDING == fea_state || GLB_BR_PORT_STATE_LISTENING == fea_state || GLB_BR_PORT_STATE_LEARNING == fea_state)
    {
        p_db_if->disable_learn[instance_index] = FALSE;
    }
    else
    {
        p_db_if->disable_learn[instance_index] = TRUE;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_msti_port_state(uint32 ifindex, uint32 instance, uint32 fea_state)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    tbl_fea_brg_if_key_t brgif_key;
    tbl_fea_brg_if_t *p_db_if = NULL;
    uint32 instance_index = 0;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    /* 1. check exist */
    brgif_key.ifindex = ifindex;
    p_db_if = tbl_fea_brg_if_get_fea_brg_if(&brgif_key);
    if (NULL == p_db_if)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    inst_key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    instance_index = CTC_SAI_OBJECT_INDEX_GET(p_db_inst->instance_oid);
    if (instance_index >= GLB_STP_INSTANCE_MAX)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (GLB_BR_PORT_STATE_FORWARDING == fea_state || GLB_BR_PORT_STATE_LISTENING == fea_state || GLB_BR_PORT_STATE_LEARNING == fea_state || GLB_BR_PORT_STATE_DISABLED == fea_state)
    {
        p_db_if->disable_learn[instance_index] = FALSE;
    }
    else
    {
        p_db_if->disable_learn[instance_index] = TRUE;
    }
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

int32
hsrv_brgif_set_mirror_dest_port_enable(uint32 ifindex, uint32 is_mirror_dest)
{
    tbl_fea_brg_if_key_t brgif_key;
    tbl_fea_brg_if_t *p_db_brg_if = NULL;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    /* 1. check exist */
    sal_memset(&brgif_key, 0, sizeof(brgif_key));
    brgif_key.ifindex = ifindex;
    p_db_brg_if = tbl_fea_brg_if_get_fea_brg_if(&brgif_key);
    if (NULL == p_db_brg_if)
    {
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_NONE;
    }
    p_db_brg_if->is_mirror_dest = is_mirror_dest;
    
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

static int32
_hsrv_brgif_cmd_get_brgif(uint32 ifindex, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (ifindex)
    {
        p_db_if = hsrv_brgif_get(ifindex);
        if (p_db_if)
        {
            tbl_fea_brg_if_dump_one(p_db_if, &args);
        }
    }
    else
    {
        tbl_fea_brg_if_iterate((TBL_ITER_CB_FUNC)tbl_fea_brg_if_dump_one, &args);
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_brgif_cmd_process_brgif(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 ifindex = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_FEA_BRG_IF);
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (argc == 1)
        {
            PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        
        rc = _hsrv_brgif_cmd_get_brgif(ifindex, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_brgif_start()
{
    cdb_register_cfg_tbl_cb(TBL_FEA_BRG_IF, hsrv_brgif_cmd_process_brgif);
    sal_mutex_create(&g_hsrv_brgif_mutex);

    return HSRV_E_NONE;
}

