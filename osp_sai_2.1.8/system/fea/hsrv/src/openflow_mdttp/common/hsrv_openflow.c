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
    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_ttp_openflow(FILE *fp, char **argv, int32 argc)
{
    int32 value = 0;
    uint64 metadata_id = 0;
    int32 ret;

    if (!sal_strcmp(argv[0], "flow"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_ttp_show_flow_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "table"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_flow_table(fp, value);
        }
        else if (!sal_strcmp(argv[1], "flow-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_flow(fp, value);
        }
    }
    else if (!sal_strcmp(argv[0], "metadata"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_ttp_show_metadata_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "metadata-id"))
        {
            metadata_id = cdb_uint64_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_metadata(fp, metadata_id);
        }
    }
    else if(!sal_strcmp(argv[0], "tunnel"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_ttp_show_tunnel_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "tunnel-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_tunnel(fp, value);
        }
    }
    else if (!sal_strcmp(argv[0], "stats"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_ttp_show_stats_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "stats-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_stats(fp, value);
        }
    }
    else if (!sal_strcmp(argv[0], "group"))
    {
        if (!sal_strcmp(argv[1], "brief"))
        {
            hsrv_openflow_ttp_show_group_brief(fp);
        }
        else if (!sal_strcmp(argv[1], "group-id"))
        {
            value = cdb_int_str2val(argv[2], &ret);
            hsrv_openflow_ttp_show_group(fp, value);
        }
        else if (!sal_strcmp(argv[1], "group-type"))
        {
            hsrv_openflow_ttp_show_group_type(fp, argv[2]);
        }
    }
    
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
    HSRV_IF_ERROR_RETURN(hsrv_openflow_tunnel_start());
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

