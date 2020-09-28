/****************************************************************************
* $Id$
*  Centec interface information related MACRO, ENUM, Date Structure defines file
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
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"
#include "hsrv_stp.h"
#include "hsrv_vlan_mapping.h"
#include "ctc_sai_port.h"
/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

/***************************************************************************************************
 * Name         : hsrv_lag_get_aggregator 
 * Purpose      : get linkagg interface, if it is not exist, creat it         
 * Input        : p_mif: member interface; p_lagif: LinkAGG interface
 * Output       : tbl_fea_lag_t* p_fea_lag                 
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_get_aggregator(tbl_interface_t* p_mif, ds_lag_t* p_lagif, tbl_fea_lag_t** pp_fea_lag)
{
    tbl_fea_lag_t* p_fea_lag = NULL;
    tbl_fea_lag_key_t fea_lag_key;
    tbl_fea_lag_t fea_lag;
    sai_lag_api_t*  lag_api = NULL;
    sai_object_id_t sai_lag_id = 0;
    tbl_fea_port_if_t port_if;
    tbl_interface_key_t if_key;
    tbl_interface_t* p_agg_if = NULL;
    sai_attribute_t   attr;
    sai_uint8_t load_balance_mode;
    uint64 hostif_id = 0;
    int32 rc = 0;
        
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_mif);
    HSRV_PTR_CHECK(p_lagif);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_LAG,(void**)&lag_api));
    
    load_balance_mode = p_lagif->load_balance_mode;
    if (GLB_LAG_LOAD_BALANCE_DLB == load_balance_mode)
    {
        attr.value.u32 = SAI_LAG_LOAD_BALANCE_DLB;      
    }
    else if (GLB_LAG_LOAD_BALANCE_RR == load_balance_mode)
    {
        attr.value.u32 = SAI_LAG_LOAD_BALANCE_RR;
    }
    else if (GLB_LAG_LOAD_BALANCE_MODE_STATIC == load_balance_mode)
    {
        attr.value.u32 = SAI_LAG_LOAD_BALANCE_STATIC_FAILOVER;
    }
    else if (GLB_LAG_LOAD_BALANCE_RESILIENT == load_balance_mode)
    {
        attr.value.u32 = SAI_LAG_LOAD_BALANCE_RESILIENT;
    }
    attr.id = SAI_LAG_LOAD_BALANCE_MODE;
    
    fea_lag_key.id = p_lagif->lag_id;
    p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
    sal_memset(&if_key, 0, sizeof(if_key));

    if (NULL == p_fea_lag)
    {        
        HSRV_IF_ERROR_RETURN(lag_api->create_lag(&sai_lag_id, 1, &attr));

        fea_lag.key.id = p_lagif->lag_id;
        fea_lag.lag_id = sai_lag_id;
        HSRV_IF_ERROR_RETURN(tbl_fea_lag_add_fea_lag(&fea_lag));
        p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
        if(p_fea_lag)
        {
            *pp_fea_lag = p_fea_lag;
        }
        else
        {
            return HSRV_E_INVALID_PARAM;
        }
 
        port_if.key.portid = sai_lag_id;
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
        p_agg_if = tbl_interface_get_interface(&if_key);
        HSRV_PTR_CHECK(p_agg_if);

        port_if.ifindex = p_agg_if->ifindex;
        sal_memcpy(port_if.name, p_agg_if->key.name, sizeof(p_agg_if->key.name));
        sal_mutex_lock(g_hsrv_brgif_mutex);
        rc = tbl_fea_port_if_add_fea_port_if(&port_if);
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        if (rc < 0)
        {
            return rc;
        }
        
        if (GLB_IF_TYPE_LINKAGG_IF == p_mif->hw_type)
        {
            p_mif->portid = sai_lag_id;
            tbl_interface_set_interface_field(p_mif, TBL_INTERFACE_FLD_PORTID);
            hsrv_hostif_create(p_mif->key.name, p_mif->portid, &hostif_id);
            p_mif->hostifid = hostif_id;
            tbl_interface_set_interface_field(p_mif, TBL_INTERFACE_FLD_HOSTIFID);
        }
    }
    else
    {
        *pp_fea_lag = p_fea_lag;
    }
 
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_lag_remove_aggregator 
 * Purpose      : remove linkagg interface         
 * Input        : p_if: linkagg interface; p_lagif: LinkAgg lag ds
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_remove_aggregator(tbl_interface_t* p_if, ds_lag_t* p_lagif)
{
    tbl_fea_port_if_key_t fea_port_if_key;
    tbl_fea_lag_key_t fea_lag_key;
    tbl_fea_lag_t* p_fea_lag = NULL;
    sai_lag_api_t*  lag_api = NULL;
    uint32            rc = 0;
    
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_if);
    HSRV_PTR_CHECK(p_lagif);

    if (GLB_IF_TYPE_LINKAGG_IF != p_if->hw_type)
    {
        return HSRV_E_NONE;
    }

    /* 1. lookup FEA LAG */
    sal_memset(&fea_lag_key, 0, sizeof(fea_lag_key));
    fea_lag_key.id = p_lagif->lag_id;
    p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
    if (NULL == p_fea_lag)
    {
        return HSRV_E_INVALID_PTR;
    }

    /* 2. remove SAI LAG */
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_LAG,(void**)&lag_api));
    HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_if->ifindex));
    rc = lag_api->remove_lag(p_fea_lag->lag_id);

    hsrv_hostif_remove(p_if->hostifid);
            
    /* 3. remove FEA LAG, FEA port if */
    fea_port_if_key.portid = p_fea_lag->lag_id;
    HSRV_IF_ERROR_RETURN(tbl_fea_lag_del_fea_lag(&fea_lag_key));
    sal_mutex_lock(g_hsrv_brgif_mutex);
    rc = tbl_fea_port_if_del_fea_port_if(&fea_port_if_key);
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    if (rc < 0)
    {
        return rc;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_lag_erps_sync_member_port_state(tbl_interface_t* p_mif, ds_lag_t* p_lagif)
{
    tbl_erps_ring_master_t *p_master = tbl_erps_ring_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_erps_ring_t *p_db_erps_ring = NULL;
    tbl_interface_key_t if_key;
    tbl_interface_t* p_db_lag = NULL;
    uint32 index = 0;
    
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_lag = tbl_interface_get_interface(&if_key);
    if (!p_db_lag->erps_enable)
    {
        return HSRV_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP(p_master->erps_ring_list, p_db_erps_ring, listnode)
    {
        if (p_db_erps_ring->primary_port_ifindex == p_db_lag->ifindex
         || p_db_erps_ring->secondary_port_ifindex == p_db_lag->ifindex)
        {
            if (p_db_erps_ring->blocked)
            {
                if (p_db_erps_ring->blocked_port_ifindex == p_db_lag->ifindex)
                {
                    for (index = 0; index < p_db_erps_ring->instance_id_num; index++)
                    {
                        hsrv_stp_set_port_state(p_mif->ifindex, p_db_erps_ring->instance_id[index], GLB_BR_PORT_STATE_BLOCKING);
                    }
                }
                else
                {
                    for (index = 0; index < p_db_erps_ring->instance_id_num; index++)
                    {
                        hsrv_stp_set_port_state(p_mif->ifindex, p_db_erps_ring->instance_id[index], GLB_BR_PORT_STATE_FORWARDING);
                    }
                }
            }
            else
            {
                for (index = 0; index < p_db_erps_ring->instance_id_num; index++)
                {
                    hsrv_stp_set_port_state(p_mif->ifindex, p_db_erps_ring->instance_id[index], GLB_BR_PORT_STATE_FORWARDING);
                }
            }
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_lag_sync_member_port_state(tbl_interface_t* p_mif, ds_lag_t* p_lagif)
{
    tbl_mstp_port_key_t port_key;
    tbl_mstp_port_t* p_db_lag_mstp_port = NULL;
    tbl_msti_port_t* p_db_lag_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 instance = MSTP_DEFAULT_INSTANCE;
    
    sal_memset(&port_key, 0, sizeof(port_key));
    sal_snprintf(port_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);

    /* 1. sync stp state */
    p_db_lag_mstp_port = tbl_mstp_port_get_mstp_port(&port_key);
    if (p_db_lag_mstp_port)
    {
        HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_mif->ifindex, instance, p_db_lag_mstp_port->fea_state));

        CTCLIB_SLIST_LOOP(p_db_lag_mstp_port->msti_list.obj_list, p_db_lag_msti_port, listnode)
        {
            instance = p_db_lag_msti_port->key.instance;
            HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_mif->ifindex, instance, p_db_lag_mstp_port->fea_state));
        }
    }

    hsrv_lag_erps_sync_member_port_state(p_mif, p_lagif);

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_lag_add_member 
 * Purpose      : add member interface to linkagg interface         
 * Input        : p_mif: member interface; p_lagif: LinkAGG interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_add_member(tbl_interface_t* p_mif, ds_lag_t* p_lagif)
{
    tbl_fea_lag_t* p_fea_lag = NULL;
    sai_lag_api_t*  lag_api = NULL;
    sai_object_id_t lag_memberid;
    sai_attribute_t attr[2];
    tbl_fea_port_if_t fea_port_if;
    tbl_fea_port_if_t *p_fea_port_if = NULL;
    
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_mif);
    HSRV_PTR_CHECK(p_lagif);
    if(p_mif->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_LAG,(void**)&lag_api));

    /*1. get agg, if agg does not exist, create a new agg */
    HSRV_IF_ERROR_RETURN(hsrv_lag_get_aggregator(p_mif, p_lagif, &p_fea_lag));
    if (p_fea_lag == NULL)
    {
        return HSRV_E_INVALID_PTR;
    }

    /*2. create agg member */
    attr[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr[0].value.oid = p_fea_lag->lag_id;
    attr[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr[1].value.oid = p_mif->portid;
    HSRV_IF_ERROR_RETURN(lag_api->create_lag_member(&lag_memberid, 2, attr));
    
    fea_port_if.key.portid = p_mif->portid;
    p_fea_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if.key);
    if (NULL == p_fea_port_if)
    {
        return HSRV_E_INVALID_PTR;
    }
    p_fea_port_if->lag_member = lag_memberid;
    tbl_fea_port_if_set_fea_port_if_field(p_fea_port_if, TBL_FEA_PORT_IF_FLD_LAG_MEMBER);

    /* added for basic qinq delete default entry for member port while join agg */
    if (GLB_VLAN_PORT_TYPE_QINQ == p_mif->vlan_type)
    {
        if (GLB_VLAN_QINQ_TYPE_BASIC == p_mif->brgif->qinq_type)
        {
            hsrv_basic_qinq_add_delete_default_entry(p_mif, p_mif->brgif->pvid, FALSE, FALSE);
        }
    }

    /*3. set agg member prop */
    HSRV_AGG_DEBUG("add port %d to lag, id %d", p_mif->portid, p_fea_lag->lag_id);
    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_mif->portid, SAI_PORT_FLAG_AGGREGATED, TRUE));
    if(p_lagif->oper_state)
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_mif->portid, SAI_PORT_FLAG_AGG_OPRED, TRUE));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_mif->portid, SAI_PORT_FLAG_AGG_OPRED, FALSE));
    }
#ifndef TAPPRODUCT    
    hsrv_lag_sync_member_port_state(p_mif, p_lagif);
#endif
    /*4. set member lag_id */
    hsrv_brgif_set_lag_id(p_mif->ifindex, p_lagif->lag_id);
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_lag_remove_member 
 * Purpose      : remove member interface from linkagg interface         
 * Input        : p_mif: member interface; p_lagif: LinkAgg interface
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_remove_member(tbl_interface_t* p_mif, ds_lag_t* p_lagif)
{
    tbl_fea_lag_t* p_fea_lag = NULL;
    sai_lag_api_t*  lag_api = NULL;
    tbl_fea_port_if_t*     p_fea_port_if = NULL;
    tbl_fea_port_if_key_t  p_fea_port_if_key;
    tbl_fea_lag_key_t fea_lag_key;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if_lag = NULL;
    uint32            rc = 0;
    
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_mif);
    HSRV_PTR_CHECK(p_lagif);

    if(p_mif->hw_type != GLB_IF_TYPE_PORT_IF)
    {
        return HSRV_E_NONE;
    }

    /*added by yejl for bug41599, 2016-12-16*/
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_lagif->lag_id);
    p_db_if_lag = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if_lag)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    if (p_db_if_lag->vlanclass_active_type)
    {   
        p_mif->vlanclass_active_type = 0;
        p_mif->vlanclass_group_id = 0;
        HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_type_and_group_id(p_mif));

        if (p_db_if_lag->vlanclass_default_action)
        {
            p_mif->vlanclass_default_action = 0;
            HSRV_IF_ERROR_RETURN(hsrv_interface_set_vlanclass_default_action(p_mif));
        }
    }
    /*ended by yejl*/
    
    sal_memset(&fea_lag_key, 0, sizeof(fea_lag_key));
    fea_lag_key.id = p_lagif->lag_id;
    p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag_key);
    if(NULL == p_fea_lag)
    {
        return HSRV_E_INVALID_PTR;
    }

    /*1. reset agg member prop */
    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_mif->portid,SAI_PORT_FLAG_AGGREGATED, FALSE));
    HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_mif->portid,SAI_PORT_FLAG_AGG_OPRED, FALSE));

    /* added for basic qinq add default entry for member port while leave agg */
    if (GLB_VLAN_PORT_TYPE_QINQ == p_mif->vlan_type)
    {
        if (GLB_VLAN_QINQ_TYPE_BASIC == p_mif->brgif->qinq_type)
        {
            hsrv_basic_qinq_add_delete_default_entry(p_mif, p_mif->brgif->pvid, TRUE, FALSE);
        }
    }

    /*2. remove agg member */
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_LAG,(void**)&lag_api));

    sal_memset(&p_fea_port_if_key, 0, sizeof(p_fea_port_if_key));
    p_fea_port_if_key.portid= p_mif->portid;
    p_fea_port_if = tbl_fea_port_if_get_fea_port_if(&p_fea_port_if_key);
    if (NULL == p_fea_port_if)
    {
        return HSRV_E_INVALID_PTR;
    }
    rc = lag_api->remove_lag_member(p_fea_port_if->lag_member);
    p_fea_port_if->lag_member = 0;
    tbl_fea_port_if_set_fea_port_if_field(p_fea_port_if, TBL_FEA_PORT_IF_FLD_LAG_MEMBER);

    /*3. try to remove lag */
    
    /*4. set member lag_id */
    hsrv_brgif_set_lag_id(p_mif->ifindex, 0);
    (void)rc;
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : _hsrv_lag_dump_fea_lag_all 
 * Purpose      : dump the lag         
 * Input        : p_fea_lag: table fea_lag; para,
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
_hsrv_lag_dump_fea_lag_all(tbl_fea_lag_t* p_fea_lag, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;
    field_parse_t field;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    if (p_fea_lag)
    {
       tbl_fea_lag_dump_one(p_fea_lag, &args);
    }
    else
    {
        tbl_fea_lag_iterate((TBL_ITER_CB_FUNC)tbl_fea_lag_dump_one, &args);
    }
    
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}
/***************************************************************************************************
 * Name         : hsrv_lag_cmd_process_lag 
 * Purpose      : process LinkAGG command for FEA         
 * Input        : argv: CLI parameter; argc: parameter counter; para: related parameter;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_cmd_process_lag(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_fea_lag_t fea_lag;
    tbl_fea_lag_t* p_fea_lag = NULL;
    uint32         fea_lag_id = 0;
    int32          rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(&fea_lag, 0, sizeof(fea_lag));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        CFG_INVALID_PARAM_RET("Not support to set FDB");
        break;

    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            fea_lag_id = cdb_int_str2val(argv[0], &rc);
            fea_lag.key.id = fea_lag_id;
            p_fea_lag = tbl_fea_lag_get_fea_lag(&fea_lag.key);
            if (NULL == p_fea_lag)
            {
                CFG_FUNC_ERR_RET("Linkagg %d is not exist\n", fea_lag_id);
            }
        }
        _hsrv_lag_dump_fea_lag_all(p_fea_lag, para);
        
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_lag_memberport_down_fast_protect(tbl_interface_t *p_db_if)
{
    if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        if (p_db_if->lag)
        {
            if (p_db_if->lag->oper_state)
            {
                ctc_sai_port_set_port_flag(p_db_if->portid, SAI_PORT_FLAG_AGG_OPRED, FALSE);
                /*ADD by yangl for bug 53272*/
                ctc_sai_port_set_port_flag(p_db_if->portid, SAI_PORT_FLAG_MIRROR_DEST, FALSE);
            }
        }
    }

    return HSRV_E_NONE;
}

/*modified by yangl for bug 47157&47168, 2018-09-14*/
int32 
hsrv_lag_set_mem_port_security(tbl_interface_t *p_if,ds_brgif_t *p_brgif)
{
    uint8              lag_id = 0;
    tbl_interface_t  *p_lag_if = NULL;
    char ifname[IFNAME_SIZE] = {0};
    tbl_fea_brg_if_t *p_fea_brg_if = NULL;
    if (NULL == p_if)
    {
        return HSRV_E_NONE;
    }
    if (INTERFACE_IS_LAG_MEMBER(p_if))
    {
         lag_id = p_if->lag->lag_id;
         sal_sprintf(ifname, "agg%u", lag_id);
         p_lag_if = tbl_interface_get_interface_by_name(ifname);
         if (NULL != p_lag_if)
         {
            p_fea_brg_if = hsrv_brgif_get(p_lag_if->ifindex);
         }
     }
    else
    {
        p_fea_brg_if = hsrv_brgif_get(p_if->ifindex);
    }
    if (NULL != p_fea_brg_if)
    {
        if ((p_fea_brg_if->curr_mac + p_fea_brg_if->curr_mac_static) >= p_fea_brg_if->max_mac)
        {
            hsrv_interface_set_port_security(p_if, p_brgif, TRUE);
        }
        else
        {
            hsrv_interface_set_port_security(p_if, p_brgif, FALSE);
        }
    }
    return HSRV_E_NONE;
                     
}
      
/***************************************************************************************************
 * Name         : hsrv_lag_sync 
 * Purpose      : load interface cdb to sai          
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
hsrv_lag_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_interface_t *p_if = NULL;
    tbl_interface_key_t *p_if_key = NULL;
    ds_lag_t* p_lagif = NULL;
    tbl_fea_lag_t *p_fea_lag = NULL;
    ds_brgif_t* p_brgif = NULL;

    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    HSRV_PTR_CHECK(p_ds);

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_if_key);
        p_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_if);
        if (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)
        {
            HSRV_IF_ERROR_RETURN(hsrv_lag_get_aggregator(p_if, p_ds, &p_fea_lag));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_lag_add_member(p_if, p_ds));
        }

        break;

    case CDB_OPER_DEL:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        HSRV_PTR_CHECK(p_if_key);
        p_if = tbl_interface_get_interface(p_if_key);
        HSRV_PTR_CHECK(p_if);
        if (GLB_IF_TYPE_LINKAGG_IF == p_if->hw_type)
        {
            HSRV_IF_ERROR_RETURN(hsrv_lag_remove_aggregator(p_if, p_ds));
        }
        else
        {
            HSRV_IF_ERROR_RETURN(hsrv_lag_remove_member(p_if, p_ds));
            HSRV_IF_ERROR_RETURN(hsrv_fdb_flush_fdb_port(p_if->ifindex));
        }        
        break;

    case CDB_OPER_SET:
        p_if_key = (tbl_interface_key_t*)p_tbl;
        p_if = tbl_interface_get_interface(p_if_key);
        p_lagif = (ds_lag_t*)p_ds;
        HSRV_PTR_CHECK(p_if);
        HSRV_PTR_CHECK(p_lagif);
        
        if(p_if->hw_type != GLB_IF_TYPE_PORT_IF)
        {
            return HSRV_E_NONE;
        }
        switch(field_id)
        {
        case DS_LAG_FLD_OPER_STATE:
                HSRV_IF_ERROR_RETURN(ctc_sai_port_set_port_flag(p_if->portid,
                    SAI_PORT_FLAG_AGG_OPRED, p_lagif->oper_state?TRUE:FALSE));
                if (GLB_IF_MODE_L3 == p_if->mode) 
                {
                    ctc_sai_port_set_phy_en (p_if->portid, TRUE);
                }
                /*added by yejl to fix bug40732, 2016-09-27*/
                if (GLB_IF_MODE_L2 == p_if->mode) 
                {
                    p_brgif = p_if->brgif;
                    if (NULL == p_brgif)
                    {
                        return HSRV_E_NONE;
                    }
                    /*added by yejl to fix bug41457, 2017-01-03*/
                    if (p_brgif->max_mac != 0)
                    {
                    #ifdef GREATBELT
                        hsrv_lag_set_mem_port_security(p_if,p_brgif);/*modified by yangl for bug 47157&47168, 2018-09-14*/
                    #else
                        HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security(p_if, p_brgif, FALSE));
                    #endif

#ifdef GREATBELT
                        /* added by kcao for bug 46781, 46795, security and MLAG peer-link uses same CTC_PORT_PROP_LEARNING_EN */
                        HSRV_IF_ERROR_RETURN(hsrv_interface_set_port_security_mac_learning(p_if, p_brgif->oper_mac_learning_en));
#endif
                    }
                }
                /* modified by liwh for bug 50952, 2019-01-31 */
                if (GLB_IF_MODE_L3 == p_if->mode) 
                {
                    tbl_route_if_key_t srif_key;
                    tbl_route_if_t *p_srif = NULL;

                    sal_memcpy(&srif_key.name, p_if->key.name, IFNAME_SIZE);
                    p_srif = tbl_route_if_get_route_if(&srif_key);
                    if (p_srif)
                    {
                        hsrv_dhcp_relay_update_service_when_add_delete_rtif(p_srif, TRUE);
                    }
                }
                /* liwh end */
            break;
        }
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_lag_start 
 * Purpose      : start LinkAGG module in FEA, register command here        
 * Input        : 
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_start()
{
    cdb_register_cfg_tbl_cb(TBL_FEA_LAG, hsrv_lag_cmd_process_lag);
    return HSRV_E_NONE;
}

