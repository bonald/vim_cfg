/****************************************************************************************
 * hsrv_openflow.c   :  Hal server openflow related function
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Yan Xing'an
 * Date         :       2014-12-15
 * Reason       :       First Create.
 ****************************************************************************************/
#include "hsrv_inc.h"

#include "hsrv_openflow.h"


/****************************************************************************************
 *  
 * Globals
 *
 ****************************************************************************************/
 hsrv_openflow_master_t *g_pst_openflow_master = NULL;

/****************************************************************************************
 *  
 * Function
 *
 ****************************************************************************************/

int32
hsrv_openflow_show_openflow(FILE *fp, char **argv, int32 argc)
{
    #if 0
    int32 value = 0;
    int32 ret;

    if (!sal_strcmp(argv[0], "port"))
    {
        hsrv_openflow_show_port(fp);
    }
    else if (!sal_strcmp(argv[0], "flow"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_show_flow_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "flow-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_show_flow(fp, value);
        }
        else if (!sal_strcmp(argv[1], "actlist"))
        {
            if (!sal_strcmp(argv[2], "brief"))
            {
                hsrv_openflow_show_flow_actlist_brief(fp);
            }
            else if (!sal_strcmp(argv[2], "ptr"))
            {
                value = cdb_int_str2val(argv[3], &ret);
                hsrv_openflow_show_flow_actlist_ptr(fp, value);
            }
        }
    }
    else if (!sal_strcmp(argv[0], "group"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_show_group_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "group-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_show_group(fp, value);
        }
    }
    else if (!sal_strcmp(argv[0], "tunnel"))
    {
        /* Modifed by weizj for tunnel bug 33523 */
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_show_tunnel_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "metadata"))
        {
            hsrv_openflow_show_tunnel_metadata_all(fp);
        }
        else if (!sal_strcmp(argv[1], "fid"))
        {
            hsrv_openflow_show_tunnel_fid_all(fp);
        }
        else
        {
            hsrv_openflow_show_tunnel_port_all(fp);
        }
    }
    else if (!sal_strcmp(argv[0], "bond"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_show_bond_brief(fp);
        }
    }
  #if 0
    else if (!sal_strcmp(argv[0], "inband"))
    {
        hsrv_openflow_show_inband(fp);
    }
  #endif
    else if (!sal_strcmp(argv[0], "nexthop"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_show_nexthop_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "opf"))
        {
            hsrv_openflow_show_nexthop_opf(fp, argv[2]);
        }
        else if (!sal_strcmp(argv[1], "hash"))
        {
            hsrv_openflow_show_nexthop_hash_type(fp, argv[2]);
        }
        else if (!sal_strcmp(argv[1], "ptr"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_show_nexthop_hash_ptr(fp, value);
        }
    }
    else if (!sal_strcmp(argv[0], "stats"))
    {
        hsrv_openflow_show_stats(fp);
    }
    #endif
    return HSRV_E_NONE;
}

int32
ctc_drv_ttp_init(void)
{
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_mirror_init());
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_scl_init());
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_acl_init());
    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_nexthop_init());

    return HSRV_E_NONE;
}


int32
hsrv_openflow_module_start(void)
{
    hsrv_module_info_t* p_openflow_module = NULL;  

    /* 1. get openflow master */
    p_openflow_module = HSRV_OPENFLOW_MALLOC(sizeof(hsrv_module_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_openflow_module);
    
    p_openflow_module->p_data_info = HSRV_OPENFLOW_CALLOC(sizeof(hsrv_openflow_master_t));
    if (NULL == p_openflow_module->p_data_info)
    {
        HSRV_OPENFLOW_FREE(p_openflow_module);
        return HSRV_E_NO_MEMORY;
    }
    g_pst_openflow_master = p_openflow_module->p_data_info;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_stats_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_flow_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_group_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_meter_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_register_cb());
    
    return HSRV_E_NONE;
}

/*****************************************************************************************************
 * Name    : hsrv_openflow_module_init
 * Purpose : init openflow database
 * Input   : p_system_info : system specific infomation
 * Output  : N/A
 * Return  : HSRV_E_***  - hal server return type
 * Note    : N/A
******************************************************************************************************/
int32
hsrv_openflow_module_init(hsrv_system_spec_info_t* p_system_info)
{
    HSRV_IF_ERROR_RETURN(ctc_drv_ttp_init());
    return HSRV_E_NONE;
}

