/**
 @file ctc_app_packet_cli.c

 @author  Copyright (C) 2012 Centec Networks Inc.  All rights reserved.

 @date 2012-11-25

 @version v2.0

 This file define packet sample CLI functions

*/

#include "sal.h"
#include "ctc_api.h"
#include "ctc_cli.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_opf.h"
#include "ctc_cli_common.h"
#include "ctc_app_index.h"
#include "ctc_app_vlan_port_api.h"
#include "ctc_app_cli.h"
#include "ctcs_gint_api.h"

CTC_CLI(ctc_cli_app_index_alloc,
        ctc_cli_app_index_alloc_cmd,
        "app index alloc (tunnel-id|nh-id|l3if-id|mcast-group-id|\
        acl-entry-id|policer-id|stats-id|logic-port|aps-group-id|\
        scl-entry-id|arp-id|acl-group-id|scl-group-id) (gchip GCHIP|)",
        CTC_CLI_APP_M_STR,
        "Index",
        "Alloc",
        "Tunnel-id" ,
        "Nh-id",
        "L3if-id",
        "Mcast-group-id",
        "Acl-entry-id",
        "Policer-id ",
        "Stats-id ",
        "Logic-port",
        "Aps-group-id",
        "Scl-entry-id",
        "Arp-id",
        "Acl-group-id",
        "Scl-group-id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_index_t app_index = {0};


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_MPLS_TUNEL_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nh-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_L3IF_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_MCAST_GROUP_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-entry-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ACL_ENTRY_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_POLICER_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_STATSID_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_LOGIC_PORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("aps-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_APS_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-entry-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_SCL_ENTRY_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("arp-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ARP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ACL_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_SCL_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", app_index.gchip, argv[index + 1]);
    }

    ret = ctc_app_index_alloc(&app_index);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%%Alloc offset by APP-Index, index: %u entry_num: %d\n", app_index.index, app_index.entry_num);

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_app_index_free,
        ctc_cli_app_index_free_cmd,
        "app index free (tunnel-id|nh-id|l3if-id|global-dsnh-offset (entry-num ENTRY-NUM|)|mcast-group-id|\
        acl-entry-id|policer-id|stats-id|logic-port|\
        aps-group-id|scl-entry-id|arp-id|acl-group-id|scl-group-id) (index INDEX) (gchip GCHIP|)",
        CTC_CLI_APP_M_STR,
        "Index",
        "Free",
        "Tunnel-id" ,
        "Nh-id",
        "L3if-id",
        "Global-dsnh-offset",
        "Entry num",
        "The value of entry num",
        "Mcast-group-id",
        "Acl-entry-id" ,
        "Policer-id ",
        "Stats-id ",
        "Logic-port",
        "Aps-group-id",
        "Scl-entry-id",
        "Arp-id",
        "Acl-group-id",
        "Scl-group-id",
        "Index",
        "Start index to free",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_index_t app_index = {0};
    app_index.entry_num = 1;
	app_index.index_type = CTC_APP_INDEX_TYPE_MAX;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_MPLS_TUNEL_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nh-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_NHID;
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_L3IF_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("global-dsnh-offset");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_MCAST_GROUP_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-entry-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ACL_ENTRY_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_POLICER_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_STATSID_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_LOGIC_PORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("aps-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_APS_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-entry-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_SCL_ENTRY_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("arp-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ARP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_ACL_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-group-id");
    if (index != 0xFF)
    {
        app_index.index_type = CTC_APP_INDEX_TYPE_SCL_GROUP_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("index", app_index.index, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("entry-num", app_index.entry_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("gchip", app_index.gchip, argv[index + 1]);
    }

    ret = ctc_app_index_free(&app_index);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_app_set_rchip_dsnh_offset_range,
        ctc_cli_app_set_rchip_dsnh_offset_range_cmd,
        "app index dsnh-offset-range (gchip GCHIP min-index MIN-INDEX max-index MAX-INDEX)",
        CTC_CLI_APP_M_STR,
        "Index",
        "Configure remote chip's dsnh-offset-range",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Min dsnh offset",
        "Default to 1",
        "Max dsnh offset",
        "Refer to [GLB_NH_TBL_SIZE] from mem_profile")
{
    int32 ret = 0;
	uint8 gchip = 0;
	uint8 index = 0;
	uint32 min_index = 0;
	uint32 max_index = 0;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

	   CTC_CLI_GET_UINT8("gchip", gchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("min-index");
    if (index != 0xFF)
    {

	   CTC_CLI_GET_UINT32("min-index", min_index, argv[index + 1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("max-index");
    if (index != 0xFF)
    {

	   CTC_CLI_GET_UINT32("max-index", max_index, argv[index + 1]);
    }
    ret = ctc_app_index_set_rchip_dsnh_offset_range( gchip,min_index, max_index);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_show_index_used_status,
        ctc_cli_app_show_index_used_status_cmd,
        "show app index used-status (tunnel-id|nh-id|l3if-id|global-dsnh-offset (gchip GCHIP)|mcast-group-id|\
        acl-entry-id|policer-id|stats-id) (gchip GCHIP|)",
        CTC_CLI_SHOW_MEMORY_STR,
        CTC_CLI_APP_M_STR,
        "Index",
        "Used status",
        "Tunnel-id" ,
        "Nh-id",
        "L3if-id",
        "Global-dsnh-offset",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Mcast-group-id",
        "Acl-entry-id" ,
        "Policer-id ",
        "Stats-id ",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = 0;
	uint8 index = 0;
    ctc_opf_t opf;
    opf.multiple = 0;
    opf.pool_index = 0;
    opf.reverse = 0;


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-id");
    if (index != 0xFF)
    {
        opf.pool_type = CTC_OPF_MPLS_TUNNEL_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nh-id");
    if (index != 0xFF)
    {
     opf.pool_type = CTC_OPF_NHID;
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3if-id");
    if (index != 0xFF)
    {
        opf.pool_type = CTC_OPF_L3IF_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("global-dsnh-offset");
    if (index != 0xFF)
    {
       opf.pool_type = CTC_OPF_GLOBAL_DSNH_OFFSET;
	   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
       if (index != 0xFF)
       {
           CTC_CLI_GET_UINT8("gchip", opf.pool_index, argv[index + 1]);
       }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast-group-id");
    if (index != 0xFF)
    {
        opf.pool_type = CTC_OPF_MCAST_GROUP_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-entry-id");
    if (index != 0xFF)
    {
       opf.pool_type = CTC_OPF_ACL_ENTRY_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        opf.pool_type = CTC_OPF_POLICER_ID;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-id");
    if (index != 0xFF)
    {
       opf.pool_type = CTC_OPF_STATSID_ID;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", opf.pool_index, argv[index + 1]);
    }

    ret = ctc_opf_print_alloc_info(&opf);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}


#define _____VLAN_PORT_____ ""

CTC_CLI(ctc_cli_app_vlan_port_init,
        ctc_cli_app_vlan_port_init_cmd,
        "app vlan-port module init (vdev-num VALUE vdev-base-vlan VLAN|) {mcast-tunnel-vlan VALUE | bcast-tunnel-vlan VALUE | multi-nni-en | stats-en|}",
         CTC_CLI_APP_M_STR,
        "Vlan port",
        "Module",
        "Init",
        "Vdev num",
        "Value",
        "Vdev base vlan",
        "Vlan id",
        "Mcast tunnel vlan",
        "Vlan id",
        "Bcast tunnel vlan",
        "Vlan id",
        "Multi nni ports",
        "Stats enable")
{
    uint8 index = 0;
    uint8 lchip = 0;
    int32 ret = 0;
    ctc_app_vlan_port_init_cfg_t app_vlan_port_init;

    sal_memset(&app_vlan_port_init, 0, sizeof(ctc_app_vlan_port_init_cfg_t));
    app_vlan_port_init.vdev_num = 1;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev num", app_vlan_port_init.vdev_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-base-vlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev base vlan", app_vlan_port_init.vdev_base_vlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast-tunnel-vlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("mcast tunnel vlan", app_vlan_port_init.mcast_tunnel_vlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bcast-tunnel-vlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("bcast tunnel vlan", app_vlan_port_init.bcast_tunnel_vlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("multi-nni-en");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(app_vlan_port_init.flag, CTC_APP_VLAN_PORT_FLAG_MULTI_NNI_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats-en");
    if (index != 0xFF)
    {
        CTC_SET_FLAG(app_vlan_port_init.flag, CTC_APP_VLAN_PORT_FLAG_STATS_EN);
    }

    ret = ctc_app_vlan_port_init(lchip, (void*)&app_vlan_port_init);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_app_vlan_port_create_port,
        ctc_cli_app_vlan_port_create_port_cmd,
        "app vlan-port (create|destroy|query) (uni-port | nni-port (rx-en EN|)) (port GPORT) (vdev-id VALUE|)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Create",
        "Destroy",
        "Query",
        "UNI port",
        "NNI port",
        "Receive enable",
        "Enable VALUE",
        "Port",
        "Value",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_uni_t uni_port;
    ctc_app_nni_t nni_port;
    uint32 port = 0;
    uint8 is_create = 0;
    uint8 is_query = 0;
    uint8 lchip = 0;
    uint16 vdev_id = 0;

    sal_memset(&uni_port, 0, sizeof(ctc_app_uni_t));
    sal_memset(&nni_port, 0, sizeof(ctc_app_nni_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("create");
    if (index != 0xFF)
    {
       is_create = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("query");
    if (index != 0xFF)
    {
       is_query = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port", port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("uni-port");
    if (index != 0xFF)
    {
        uni_port.port = port;
        uni_port.vdev_id = vdev_id;

        if (is_create)
        {
            ret = ctc_app_vlan_port_create_uni(lchip, &uni_port);
        }
        else if(is_query)
        {
            ret = ctc_app_vlan_port_get_uni(lchip, &uni_port);
        }
        else
        {
            ret = ctc_app_vlan_port_destory_uni(lchip, &uni_port);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (is_create || is_query)
        {
            ctc_cli_out("uni mcast nhid     : %d\n", uni_port.mc_nhid);
            ctc_cli_out("associate port     : 0x%x\n", uni_port.associate_port);
        }


    }
    else
    {
        nni_port.rx_en = 1;
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rx-en");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT8("receive enable", nni_port.rx_en, argv[index + 1]);
        }
        nni_port.port = port;
        nni_port.vdev_id = vdev_id;

        if (is_create)
        {

            ret = ctc_app_vlan_port_create_nni(lchip, &nni_port);
        }
        else if(is_query)
        {
            ret = ctc_app_vlan_port_get_nni(lchip, &nni_port);
        }
        else
        {
            ret = ctc_app_vlan_port_destory_nni(lchip, &nni_port);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        if (is_create || is_query)
        {
            ctc_cli_out("logic port     : %d\n", nni_port.logic_port);
        }

    }


    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_app_vlan_port_update_nni,
        ctc_cli_app_vlan_port_update_nni_cmd,
        "app vlan-port update nni-port (port GPORT) (rx-en EN)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Update",
        "NNI port",
        "Port",
        "Value",
        "Receive enable",
        "Enable VALUE")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_nni_t nni_port;
    uint8 lchip = 0;

    sal_memset(&nni_port, 0, sizeof(ctc_app_nni_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port", nni_port.port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rx-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("receive enable", nni_port.rx_en, argv[index + 1]);
    }

    ret = ctc_app_vlan_port_update_nni(lchip, &nni_port);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}



CTC_CLI(ctc_cli_app_vlan_port_create_gem_port,
        ctc_cli_app_vlan_port_create_gem_port_cmd,
        "app vlan-port (create|destroy|query)  gem-port (port GPORT tunnel-value TUNNEL) (pass-through-en VALUE|)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Create",
        "Destroy",
        "Query",
        "GEM port",
        "Port",
        "Value",
        "Tunnel value (tunnel vlan)",
        "Value",
        "Pass through",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_gem_port_t gem_port;
    uint8 is_create = 0;
    uint8 is_query = 0;
    uint8 lchip = 0;

   sal_memset(&gem_port, 0, sizeof(ctc_app_gem_port_t));

   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("create");
   if (index != 0xFF)
   {
       is_create = 1;
   }

   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("query");
   if (index != 0xFF)
   {
       is_query = 1;
   }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port", gem_port.port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("tunnel value", gem_port.tunnel_value, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("pass-through-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("pass through", gem_port.pass_trough_en, argv[index + 1]);
    }

    if (is_create)
    {
        ret = ctc_app_vlan_port_create_gem_port(lchip, &gem_port);
    }
    else if(is_query)
    {
        ret = ctc_app_vlan_port_get_gem_port(lchip, &gem_port);
    }
    else
    {
        ret = ctc_app_vlan_port_destory_gem_port(lchip, &gem_port);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (is_create|| is_query)
    {
        ctc_cli_out("gem logic port         : %d\n",   gem_port.logic_port);
        ctc_cli_out("gem associate port     : 0x%x\n", gem_port.ga_gport);
    }

    return CLI_SUCCESS;

}



CTC_CLI(ctc_cli_app_vlan_port_update_gem_port,
        ctc_cli_app_vlan_port_update_gem_port_cmd,
        "app vlan-port update  gem-port (port GPORT tunnel-value TUNNEL) (isolate-en (enable|disable)| bind vlan-port-id ID \
         | unbind vlan-port-id ID | igs-policer-id ID | egs-policer-id ID (cos-index INDEX|) | pass-through-en VALUE | igs-stats-id ID | egs-stats-id ID | station-move-action ACTION)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Update",
        "GEM port",
        "Port",
        "Value",
        "Tunnel value (tunnel vlan)",
        "Value",
        "Isoate between gem port",
        "Enable",
        "Disable",
        "Bind",
        "Vlan port id",
        "ID",
        "Unbind",
        "Vlan port id",
        "ID",
        "Ingress policer id",
        "Policer id value",
        "Egress policer id",
        "Policer id value",
        "Hbwp cos index",
        "Cos index value",
        "Pass through",
        "Value",
        "Ingress stats id",
        "Stats id value",
        "Egress stats id",
        "Stats id value",
        "Station move action",
        "Action value, 0:fwd, 1:discard, 2:discard and to cpu")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_app_gem_port_t gem_port;
    uint8 lchip = 0;

    sal_memset(&gem_port, 0, sizeof(ctc_app_gem_port_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("port", gem_port.port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("tunnel value", gem_port.tunnel_value, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("isolate-en");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
        if (index != 0xFF)
        {
            gem_port.isolation_en = 1;
        }

        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_ISOLATE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bind");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("bind", gem_port.vlan_port_id, argv[index + 2]);
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_BIND_VLAN_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("unbind");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("unbind", gem_port.vlan_port_id, argv[index + 2]);
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_UNBIND_VLAN_PORT;
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-policer-id");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_IGS_POLICER;
        CTC_CLI_GET_UINT32("igs-policer-id", gem_port.ingress_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-policer-id");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_EGS_POLICER;
        CTC_CLI_GET_UINT32("egs-policer-id", gem_port.egress_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cos-index");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_EGS_POLICER;
        CTC_CLI_GET_UINT8("egs-cos-idx", gem_port.egress_cos_idx, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("pass-through-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("pass through", gem_port.pass_trough_en, argv[index + 1]);
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_PASS_THROUGH_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-stats-id");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_IGS_STATS;
        CTC_CLI_GET_UINT32("igs-stats-id", gem_port.ingress_stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-stats-id");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_EGS_STATS;
        CTC_CLI_GET_UINT32("egs-stats-id", gem_port.egress_stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("station-move-action");
    if (index != 0xFF)
    {
        gem_port.update_type = CTC_APP_GEM_PORT_UPDATE_STATION_MOVE_ACTION;
        CTC_CLI_GET_UINT8("station move action", gem_port.station_move_action, argv[index + 1]);
    }

    ret = ctc_app_vlan_port_update_gem_port(lchip, &gem_port);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}




#define CTC_CLI_APP_VLAN_ACTION_OP "(none|add|del|replace)"

#define CTC_CLI_APP_VLAN_ACTION " (svlan " CTC_CLI_APP_VLAN_ACTION_OP " (new-svid SVID|) |) " \
                                " (scos  " CTC_CLI_APP_VLAN_ACTION_OP " (new-scos SCOS|) |) " \
                                " (cvlan " CTC_CLI_APP_VLAN_ACTION_OP " (new-cvid CVID|) |) " \
                                " (ccos  " CTC_CLI_APP_VLAN_ACTION_OP " (new-ccos CCOS|) |) "


#define CTC_CLI_APP_VLAN_ACTION_OP_DSCP \
    "None",       \
    "Add",        \
    "Del",        \
    "Replace"     \


#define CTC_CLI_APP_VLAN_ACTION_DSCP \
    "Svlan",                              \
    CTC_CLI_APP_VLAN_ACTION_OP_DSCP,      \
    "New svid",                            \
    "Svlan value",                        \
    "Scos",                               \
    CTC_CLI_APP_VLAN_ACTION_OP_DSCP,      \
    "New scos",                            \
    "Scos value",                         \
    "Cvlan",                              \
    CTC_CLI_APP_VLAN_ACTION_OP_DSCP,      \
    "New cvid",                            \
    "Cvlan value",                        \
    "Ccos",                               \
    CTC_CLI_APP_VLAN_ACTION_OP_DSCP,      \
    "New cos",                            \
    "Cos value"                           \



 STATIC int32
_ctc_app_cli_parser_vlan_action(unsigned char argc,
                                    char** argv,
                                    uint8 start,
                                    char *field,
                                    uint32* p_vlan_action,
                                    uint16* p_new_value)
{
    uint8 index = 0;
    uint8 start_index = 0;
    uint16 value = 0;

    start_index = CTC_CLI_GET_SPECIFIC_INDEX(field, start);
    if (start_index == 0xFF)
    {
        return 0;
    }

    index = start + start_index + 1;

    if (CLI_CLI_STR_EQUAL("none", index))
    {
        *p_vlan_action = CTC_APP_VLAN_ACTION_NONE;
    }
    else if(CLI_CLI_STR_EQUAL("add", index))
    {
        *p_vlan_action = CTC_APP_VLAN_ACTION_ADD;
    }
    else if(CLI_CLI_STR_EQUAL("del", index))
    {
        *p_vlan_action = CTC_APP_VLAN_ACTION_DEL;
    }
    else if(CLI_CLI_STR_EQUAL("replace", index))
    {
        *p_vlan_action = CTC_APP_VLAN_ACTION_REPLACE;
    }

    index = start + start_index + 2;

    if (argv[(index)] && index < argc)
    {
        if (CLI_CLI_STR_EQUAL("new-svid", index))
        {
            CTC_CLI_GET_UINT16("new svid", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-scos", index))
        {
            CTC_CLI_GET_UINT16("new scos", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-cvid", index))
        {
            CTC_CLI_GET_UINT16("new cvid", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-ccos", index))
        {
            CTC_CLI_GET_UINT16("new ccos", value, argv[index + 1]);
        }
        *p_new_value = value;
    }


    return 0;

}



CTC_CLI(ctc_cli_app_vlan_port_create,
        ctc_cli_app_vlan_port_create_cmd,
        "app vlan-port (create|destroy|query|update) vlan-port ((port GPORT (match-tunnel-value TUNNEL|) (match-svlan SVLAN {match-cvlan CVLAN| match-svlan-end SVLAN|} |\
        flex (use-mac-key | use-ipv6-key |) {mac-sa MAC MASK | mac-da MAC MASK | ip-sa A.B.C.D A.B.C.D | ip-da A.B.C.D A.B.C.D | cvlan-id CVLAN_ID MASK | ctag-cos CTAG_COS MASK |\
        svlan-id SVLAN_ID MASK | stag-cos STAG_COS MASK | ether-type ETHER_TYPE MASK | l3-type L3_TYPE MASK | ctag-cfi CTAG_CFI | stag-cfi STAG_CFI | stag-valid STAG_VALID |\
        ctag-valid CTAG_VALID | vlan-num VLAN_MUM MASK | l4-src-port L4_SRC_PORT MASK | l4-dst-port L4_DST_PORT MASK | ipv6-sa X:X::X:X MASK | ipv6-da X:X::X:X MASK |}| \
        mcast group GROUP_ID mac MAC fid FID )| vlan-port-id ID) \
        {igs-vlan-action" CTC_CLI_APP_VLAN_ACTION "|egs-vlan-action" CTC_CLI_APP_VLAN_ACTION "|} {igs-policer-id ID | egs-policer-id ID|igs-stats-id ID | egs-stats-id ID|}",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Create",
        "Destroy",
        "Query",
        "Update",
        "Vlan port",
        "Port",
        "Value",
        "Tunnel value (tunnel vlan)",
        "Value",
        "Svlan",
        "Value",
        "Cvlan",
        "Value",
        "Svlan end",
        "Value",
        "Flexible vlan edit",
        "Use Mac key", "Use Ipv6 key",
        "Mac sa",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_FORMAT,
        "Mac da",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_MAC_FORMAT,
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_FORMAT,
        "IPv4 destination address",
        CTC_CLI_IPV4_FORMAT,
        CTC_CLI_IPV4_MASK_FORMAT,
        "Cvlan id",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_VLAN_RANGE_MASK,
        "Cvlan cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_COS_RANGE_MASK,
        "Svlan id",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_VLAN_RANGE_MASK,
        "Stag cos",
        CTC_CLI_COS_RANGE_DESC,
        CTC_CLI_COS_RANGE_MASK,
        "Ether type",
        CTC_CLI_ETHTYPE_DESC,
        "Mask",
        "Layer 3 type",
        "Value",
        "Mask",
        "Ctag cfi",
        CTC_CLI_CFI_RANGE_DESC,
        "Stag cfi",
        CTC_CLI_CFI_RANGE_DESC,
        "Stag valid",
        "Tag valid value",
        "Ctag valid",
        "Tag valid value",
        "Vlan num",
        "Vlan num value",
        "Mask",
        "L4 src port",
        "L4 src port value",
        "Mask",
        "L4 dst port",
        "L4 dst port value",
        "Mask",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_FORMAT,
        "IPv6 destination address",
        CTC_CLI_IPV6_FORMAT,
        CTC_CLI_IPV6_MASK_FORMAT,
        "Multicast",
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Vlan port id",
        "Vlan port ID",
        "Ingress vlan action",
        CTC_CLI_APP_VLAN_ACTION_DSCP,
        "Egress vlan action",
        CTC_CLI_APP_VLAN_ACTION_DSCP,
        "Ingress policer id",
        "Policer id value",
        "Egresss policer id",
        "Policer id value",
        "Ingress stats id",
        "Stats id value",
        "Egress stats id",
        "Stats id value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 start_index = 0;
    ctc_app_vlan_port_t vlan_port;
    uint8 is_create = 0;
    uint8 is_query = 0;
    uint8 is_update = 0;
    uint8 lchip = 0;
    uint8 port_base = 0;
    uint8 key_sel = 0;      /*0-ipv4, 1-mac, 2-ipv6, others-invalid*/
    ipv6_addr_t ipv6_address;
    ipv6_addr_t ipv6_address_mask;

   sal_memset(&vlan_port, 0, sizeof(ctc_app_vlan_port_t));


   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("create");
   if (index != 0xFF)
   {
       is_create = 1;
   }

   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("query");
   if (index != 0xFF)
   {
       is_query = 1;
   }


   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("update");
   if (index != 0xFF)
   {
       is_update = 1;
   }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-port-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("vlan-port-id", vlan_port.vlan_port_id, argv[index + 1]);
    }
    else
    {
        /*match key*/
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT32("port", vlan_port.port, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-tunnel-value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("tunnel value", vlan_port.match_tunnel_value, argv[index + 1]);
    }
    else
    {
       port_base = 1;
    }

    if (0xFF != CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-mac-key"))
    {
        key_sel = 1;
    }
    else if (0xFF != CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-ipv6-key"))
    {
        key_sel = 2;
    }
    else
    {
        key_sel = 0;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("flex");
    if ((0xFF != index) && (2 == key_sel))
    {
        ctc_acl_ipv6_key_t *p_key = NULL;

        if (port_base)
        {
           vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_FLEX;
        }
        else
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX;
        }

        vlan_port.flex_key.type = CTC_ACL_KEY_IPV6;

        p_key = &vlan_port.flex_key.u.ipv6_key;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_SA);

        }


        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_MAC_DA);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6-sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-sa", ipv6_address, argv[index + 1]);
            p_key->ip_sa[0] = sal_htonl(ipv6_address[0]);
            p_key->ip_sa[1] = sal_htonl(ipv6_address[1]);
            p_key->ip_sa[2] = sal_htonl(ipv6_address[2]);
            p_key->ip_sa[3] = sal_htonl(ipv6_address[3]);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-sa-mask", ipv6_address_mask, argv[index + 2]);
            p_key->ip_sa_mask[0] = sal_htonl(ipv6_address_mask[0]);
            p_key->ip_sa_mask[1] = sal_htonl(ipv6_address_mask[1]);
            p_key->ip_sa_mask[2] = sal_htonl(ipv6_address_mask[2]);
            p_key->ip_sa_mask[3] = sal_htonl(ipv6_address_mask[3]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_IP_SA);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6-da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-da", ipv6_address, argv[index + 1]);
            p_key->ip_da[0] = sal_htonl(ipv6_address[0]);
            p_key->ip_da[1] = sal_htonl(ipv6_address[1]);
            p_key->ip_da[2] = sal_htonl(ipv6_address[2]);
            p_key->ip_da[3] = sal_htonl(ipv6_address[3]);
            CTC_CLI_GET_IPV6_ADDRESS("ipv6-da-mask", ipv6_address_mask, argv[index + 2]);
            p_key->ip_da_mask[0] = sal_htonl(ipv6_address_mask[0]);
            p_key->ip_da_mask[1] = sal_htonl(ipv6_address_mask[1]);
            p_key->ip_da_mask[2] = sal_htonl(ipv6_address_mask[2]);
            p_key->ip_da_mask[3] = sal_htonl(ipv6_address_mask[3]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_IP_DA);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("cvlan-id", p_key->cvlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("cvlan-id-mask", p_key->cvlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_CVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("svlan-id", p_key->svlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("svlan-id-mask", p_key->svlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_SVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cos", p_key->ctag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("ctag-cos-mask", p_key->ctag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cos", p_key->stag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("stag-cos-mask", p_key->stag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ether-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ether-type", p_key->eth_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("ether-type-mask", p_key->eth_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_ETH_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l3-type", p_key->l3_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("l3-type-mask", p_key->l3_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_L3_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cfi", p_key->stag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-valid", p_key->stag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_STAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-valid", p_key->ctag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_CTAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-num");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("vlan-num", p_key->vlan_num, argv[index + 1]);
            CTC_CLI_GET_UINT16("vlan-num-mask", p_key->vlan_num_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV6_KEY_FLAG_VLAN_NUM);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l4-src-port", p_key->l4_src_port_0, argv[index + 1]);
            CTC_CLI_GET_UINT16("l4-src-port-mask", p_key->l4_src_port_1, argv[index + 2]);
            CTC_SET_FLAG(p_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_SRC_PORT);
            p_key->l4_src_port_use_mask = 1;
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-dst-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l4-dst-port", p_key->l4_dst_port_0, argv[index + 1]);
            CTC_CLI_GET_UINT16("l4-dst-port-mask", p_key->l4_dst_port_1, argv[index + 2]);
            CTC_SET_FLAG(p_key->sub_flag, CTC_ACL_IPV6_KEY_SUB_FLAG_L4_DST_PORT);
            p_key->l4_dst_port_use_mask = 1;
        }

    }
    else if ((0xFF != index) && (1 == key_sel))
    {
        ctc_acl_mac_key_t *p_key = NULL;

        if (port_base)
        {
           vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_FLEX;
        }
        else
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX;
        }

        vlan_port.flex_key.type = CTC_ACL_KEY_MAC;

        p_key = &vlan_port.flex_key.u.mac_key;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_MAC_SA);

        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_MAC_DA);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("cvlan-id", p_key->cvlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("cvlan-id-mask", p_key->cvlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_CVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("svlan-id", p_key->svlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("svlan-id-mask", p_key->svlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_SVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cos", p_key->ctag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("ctag-cos-mask", p_key->ctag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_CTAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cos", p_key->stag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("stag-cos-mask", p_key->stag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_STAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ether-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ether-type", p_key->eth_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("ether-type-mask", p_key->eth_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_ETH_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l3-type", p_key->l3_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("l3-type-mask", p_key->l3_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_L3_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_CTAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cfi", p_key->stag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_STAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-valid", p_key->stag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_STAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-valid", p_key->ctag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_CTAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-num");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("vlan-num", p_key->vlan_num, argv[index + 1]);
            CTC_CLI_GET_UINT16("vlan-num-mask", p_key->vlan_num_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_MAC_KEY_FLAG_VLAN_NUM);
        }

    }
    else if ((index != 0xFF) && (0 == key_sel))
    {
        ctc_acl_ipv4_key_t *p_key = NULL;

        if (port_base)
        {
           vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_FLEX;
        }
        else
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX;
        }

        vlan_port.flex_key.type = CTC_ACL_KEY_IPV4;

        p_key = &vlan_port.flex_key.u.ipv4_key;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-sa", p_key->mac_sa, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-sa-mask", p_key->mac_sa_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_SA);

        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac-da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_MAC_ADDRESS("mac-da", p_key->mac_da, argv[index + 1]);
            CTC_CLI_GET_MAC_ADDRESS("mac-da-mask", p_key->mac_da_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_MAC_DA);
        }


        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-sa");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa, argv[index + 1]);
            CTC_CLI_GET_IPV4_ADDRESS("ip-sa", p_key->ip_sa_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_SA);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-da");
        if (index != 0xFF)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da, argv[index + 1]);
            CTC_CLI_GET_IPV4_ADDRESS("ip-da", p_key->ip_da_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_IP_DA);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("cvlan-id", p_key->cvlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("cvlan-id-mask", p_key->cvlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_CVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan-id");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("svlan-id", p_key->svlan, argv[index + 1]);
            CTC_CLI_GET_UINT16("svlan-id-mask", p_key->svlan_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_SVLAN);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cos", p_key->ctag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("ctag-cos-mask", p_key->ctag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cos");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cos", p_key->stag_cos, argv[index + 1]);
            CTC_CLI_GET_UINT16("stag-cos-mask", p_key->stag_cos_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_COS);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ether-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ether-type", p_key->eth_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("ether-type-mask", p_key->eth_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l3-type");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l3-type", p_key->l3_type, argv[index + 1]);
            CTC_CLI_GET_UINT16("l3-type-mask", p_key->l3_type_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-cfi", p_key->ctag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-cfi");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-cfi", p_key->stag_cfi, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_CFI);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("stag-valid", p_key->stag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_STAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ctag-valid");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("ctag-valid", p_key->ctag_valid, argv[index + 1]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_CTAG_VALID);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-num");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("vlan-num", p_key->vlan_num, argv[index + 1]);
            CTC_CLI_GET_UINT16("vlan-num-mask", p_key->vlan_num_mask, argv[index + 2]);
            CTC_SET_FLAG(p_key->flag, CTC_ACL_IPV4_KEY_FLAG_VLAN_NUM);
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l4-src-port", p_key->l4_src_port_0, argv[index + 1]);
            CTC_CLI_GET_UINT16("l4-src-port-mask", p_key->l4_src_port_1, argv[index + 2]);
            CTC_SET_FLAG(p_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_SRC_PORT);
            p_key->l4_src_port_use_mask = 1;  /*use src port value,not use src port range*/
        }
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-dst-port");
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16("l4-dst-port", p_key->l4_dst_port_0, argv[index + 1]);
            CTC_CLI_GET_UINT16("l4-dst-port-mask", p_key->l4_dst_port_1, argv[index + 2]);
            CTC_SET_FLAG(p_key->sub_flag, CTC_ACL_IPV4_KEY_SUB_FLAG_L4_DST_PORT);
            p_key->l4_dst_port_use_mask = 1; /*use dst port value,not use dst port range*/
        }


    }
    else
    {

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mcast");
        if (index != 0xFF)
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_MCAST;
            index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("group");
            if (index != 0xFF)
            {
                CTC_CLI_GET_UINT16("group id", vlan_port.l2mc_addr.l2mc_grp_id, argv[index + 1]);
            }

            index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mac");
            if (index != 0xFF)
            {
                CTC_CLI_GET_MAC_ADDRESS("mac address", vlan_port.l2mc_addr.mac, argv[index + 1]);
            }

            index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("fid");
            if (index != 0xFF)
            {
                CTC_CLI_GET_UINT16("forwarding id", vlan_port.l2mc_addr.fid, argv[index + 1]);
            }
        }
        else if (port_base)
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_SVLAN_CVLAN;
        }
        else
        {
            vlan_port.criteria = CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_SVLAN_CVLAN;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("macth svlan value", vlan_port.match_svlan, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("macth cvlan value", vlan_port.match_cvlan, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-svlan-end");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("macth svlan end value", vlan_port.match_svlan_end, argv[index + 1]);
    }

    /*igress action*/
    start_index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-vlan-action");

    if (start_index != 0xFF)
    {
        ret = _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "svlan",
                                        &vlan_port.ingress_vlan_action_set.svid,
                                        &vlan_port.ingress_vlan_action_set.new_svid);



        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "scos",
                                        &vlan_port.ingress_vlan_action_set.scos,
                                        &vlan_port.ingress_vlan_action_set.new_scos);



        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "cvlan",
                                        &vlan_port.ingress_vlan_action_set.cvid,
                                        &vlan_port.ingress_vlan_action_set.new_cvid);


        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "ccos",
                                        &vlan_port.ingress_vlan_action_set.ccos,
                                        &vlan_port.ingress_vlan_action_set.new_ccos);


    }


    /*egress action*/
    start_index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-vlan-action");

    if (start_index != 0xFF)
    {
        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "svlan",
                                        &vlan_port.egress_vlan_action_set.svid,
                                        &vlan_port.egress_vlan_action_set.new_svid);



        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "scos",
                                        &vlan_port.egress_vlan_action_set.scos,
                                        &vlan_port.egress_vlan_action_set.new_scos);



        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "cvlan",
                                        &vlan_port.egress_vlan_action_set.cvid,
                                        &vlan_port.egress_vlan_action_set.new_cvid);


        ret = ret ? ret : _ctc_app_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "ccos",
                                        &vlan_port.egress_vlan_action_set.ccos,
                                        &vlan_port.egress_vlan_action_set.new_ccos);

    }



    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-policer-id");
    if (index != 0xFF)
    {
        vlan_port.update_type = CTC_APP_VLAN_PORT_UPDATE_IGS_POLICER;
        CTC_CLI_GET_UINT32("igs-policer-id", vlan_port.ingress_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-policer-id");
    if (index != 0xFF)
    {
        vlan_port.update_type = CTC_APP_VLAN_PORT_UPDATE_EGS_POLICER;
        CTC_CLI_GET_UINT32("egs-policer-id", vlan_port.egress_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-stats-id");
    if (index != 0xFF)
    {
        vlan_port.update_type = CTC_APP_VLAN_PORT_UPDATE_IGS_STATS;
        CTC_CLI_GET_UINT32("igs-stats-id", vlan_port.ingress_stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-stats-id");
    if (index != 0xFF)
    {
        vlan_port.update_type = CTC_APP_VLAN_PORT_UPDATE_EGS_STATS;
        CTC_CLI_GET_UINT32("egs-stats-id", vlan_port.egress_stats_id, argv[index + 1]);
    }

    if (is_create)
    {
        ret = ret ? ret : ctc_app_vlan_port_create(lchip, &vlan_port);
    }
    else if(is_query)
    {
        ret = ret ? ret : ctc_app_vlan_port_get(lchip, &vlan_port);
    }
    else if(is_update)
    {
        ret = ret ? ret : ctc_app_vlan_port_update(lchip, &vlan_port);
    }
    else
    {
        ret = ret ? ret : ctc_app_vlan_port_destory(lchip, &vlan_port);
    }

    if (ret == CLI_ERROR)
    {
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (is_create || is_query)
    {
        ctc_cli_out("vlan port id          : %d\n",   vlan_port.vlan_port_id);
        ctc_cli_out("logic port            : %d\n",   vlan_port.logic_port);


        if ((vlan_port.criteria == CTC_APP_VLAN_PORT_MATCH_PORT_TUNNEL_FLEX)
            || (vlan_port.criteria == CTC_APP_VLAN_PORT_MATCH_PORT_FLEX))
        {
            ctc_cli_out("flexible nhid         : %d\n",   vlan_port.flex_nhid);
            ctc_cli_out("pon flexible nhid     : %d\n",   vlan_port.pon_flex_nhid);
        }
        if (vlan_port.criteria == CTC_APP_VLAN_PORT_MATCH_MCAST)
        {
            ctc_cli_out("mcast xlate nhid         : %d\n",   vlan_port.flex_nhid);
        }
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_app_vlan_port_global_cfg,
        ctc_cli_app_vlan_port_global_cfg_cmd,
        "app vlan-port global-cfg (uni-outer-isolate-en|uni-inner-isolate-en|unknown-mcast-drop-en | vlan-glb-vdev-en VLAN_ID|vlan-isolate-en vlan-id VLAN_ID) (enable|disable) (vdev-id VALUE|)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Global configure",
        "Uni outer isolate en",
        "Uni inner isolate en",
        "Uni unknown mcast drop en",
        "Vlan support slice",
        "Vlan Id",
        "VLAN isolate en",
        "Vlan id", "VLAN ID",
        "Enable",
        "Disable",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;

    ctc_app_global_cfg_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_app_global_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("uni-outer-isolate-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("uni-inner-isolate-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("unknown-mcast-drop-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-glb-vdev-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN;
        CTC_CLI_GET_UINT16("vlan value", cfg.vlan_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-isolate-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_VLAN_ISOLATE;
        CTC_CLI_GET_UINT16("vlan id", cfg.vlan_id, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (index != 0xFF)
    {
        cfg.value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", cfg.vdev_id, argv[index + 1]);
    }

    ret = ctc_app_vlan_port_set_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_app_vlan_port_show_global_cfg,
        ctc_cli_app_vlan_port_show_global_cfg_cmd,
        "show app vlan-port global-cfg {vlan-isolate-en vlan-id VLAN_ID|} (vdev-id VALUE|)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Global configure",
        "VLAN isolate enable",
        "Vlan id", "VLAN ID",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_app_global_cfg_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_app_global_cfg_t));

    index = CTC_CLI_GET_ARGC_INDEX("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", cfg.vdev_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-isolate-en");
    if (index != 0xFF)
    {
        cfg.cfg_type = CTC_APP_VLAN_CFG_VLAN_ISOLATE;
        CTC_CLI_GET_UINT16("vlan id", cfg.vlan_id, argv[index + 2]);
        ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);
        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("%-30s: %u\n", "vlan-isolate-en", cfg.value);
        return CLI_SUCCESS; /*only show vlan isolate en*/
    }

    cfg.cfg_type = CTC_APP_VLAN_CFG_UNI_OUTER_ISOLATE;
    ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-30s: %u\n", "uni-outer-isolate-en", cfg.value);

    cfg.cfg_type = CTC_APP_VLAN_CFG_UNI_INNER_ISOLATE;
    ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-30s: %u\n", "uni-inner-isolate-en", cfg.value);

    cfg.cfg_type = CTC_APP_VLAN_CFG_UNKNOWN_MCAST_DROP;
    ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-30s: %u\n", "unknown-mcast-drop-en", cfg.value);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_vlan_port_show_vlan_support_slice,
        ctc_cli_app_vlan_port_show_vlan_support_slice_cmd,
        "show app vlan-port global-cfg vlan-glb-vdev-en VLAN_ID",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Global configure",
        "Vlan support slice",
        "Vlan Id")
{
    int32 ret = 0;
    uint8 lchip = 0;
    ctc_app_global_cfg_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_app_global_cfg_t));

    cfg.cfg_type = CTC_APP_VLAN_CFG_VLAN_GLOBAL_VDEV_EN;
    CTC_CLI_GET_UINT16("vlan value", cfg.vlan_id, argv[0]);

    ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("vlan-id %d global vdev enable   : %d\n", cfg.vlan_id, cfg.value);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_vlan_port_security_set_learn_limit,
        ctc_cli_app_vlan_port_security_set_learn_limit_cmd,
        "app vlan-port global-cfg learn-limit type (gem-port GPHYPORT_ID TUNNEL_VALUE | fid FID | vdev-id ID |system) (maximum NUM) action (fwd|discard|redirect-to-cpu)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Global configure",
        "Learn limit",
        "Learn limit type",
        "GemPort based learn limit",
        CTC_CLI_GPORT_DESC,
        "Gem vlan value",
        "FID based learn limit",
        "FID value",
        "Vdev based learn limit",
        "Vdev id",
        "System based learn limit",
        "Maximum number",
        "0xFFFFFFFF means disable",
        "The action of mac limit",
        "Forwarding packet",
        "Discard packet",
        "Redirect to cpu")
{
    uint8 index = 0;
    int32 ret = 0;
    ctc_app_global_cfg_t cfg;
    uint8 lchip = 0;

    sal_memset(&cfg, 0, sizeof(ctc_app_global_cfg_t));

    cfg.cfg_type = CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT;
    if (CLI_CLI_STR_EQUAL("gem-port", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT;
        index ++;
        CTC_CLI_GET_UINT32_RANGE("gport", cfg.mac_learn.gport, argv[index], 0, CTC_MAX_UINT32_VALUE);
        index ++;
        CTC_CLI_GET_UINT16_RANGE("tunnel value", cfg.mac_learn.tunnel_value, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("fid", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vlan id", cfg.mac_learn.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);

    }
    else if (CLI_CLI_STR_EQUAL("vdev-id", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vdev id", cfg.vdev_id, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("system", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_SYSTEM;
    }

    index ++;

    if (CLI_CLI_STR_EQUAL("maximum", index))
    {
        index ++;
        CTC_CLI_GET_UINT32_RANGE("max num", cfg.mac_learn.limit_num, argv[index], 0, CTC_MAX_UINT32_VALUE);
    }

    index ++;

    if (CLI_CLI_STR_EQUAL("fwd", index))
    {
        cfg.mac_learn.limit_action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (CLI_CLI_STR_EQUAL("discard", index))
    {

        cfg.mac_learn.limit_action  = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else if (CLI_CLI_STR_EQUAL("redirect-to-cpu", index))
    {

        cfg.mac_learn.limit_action  = CTC_MACLIMIT_ACTION_TOCPU;
    }


    ret = ctc_app_vlan_port_set_global_cfg(lchip, &cfg);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_app_vlan_port_show_learn_limit,
        ctc_cli_app_vlan_port_show_learn_limit_cmd,
        "show app vlan-port global-cfg learn-limit type (gem-port GPHYPORT_ID TUNNEL_VLAN |fid VLAN_ID | vdev-id ID|system)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Global configure",
        "Learn limit",
        "Learn limit type",
        "GemPort based learn limit",
        CTC_CLI_GPORT_DESC,
        "Gem vlan value",
        "FID based learn limit",
        "FID value",
        "Vdev based learn limit",
        "Vdev id",
        "System based learn limit")
{
    uint8 index = 0;
    int32 ret = 0;
    ctc_app_global_cfg_t cfg;
    uint8 lchip = 0;
    uint32 limit_num = 0;
    uint32 action = 0;
    char action_str[CTC_MACLIMIT_ACTION_TOCPU + 1][32] = {{"None"}, {"forwarding packet"}, {"discard packet"}, {"redirect to cpu"}};

    sal_memset(&cfg, 0, sizeof(ctc_app_global_cfg_t));

    cfg.cfg_type = CTC_APP_VLAN_CFG_MAC_LEARN_LIMIT;
    if (CLI_CLI_STR_EQUAL("gem-port", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT;
        index ++;
        CTC_CLI_GET_UINT32_RANGE("gport", cfg.mac_learn.gport, argv[index], 0, CTC_MAX_UINT32_VALUE);
        index ++;
        CTC_CLI_GET_UINT16_RANGE("tunnel value", cfg.mac_learn.tunnel_value, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("fid", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vlan id", cfg.mac_learn.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);

    }
    else if (CLI_CLI_STR_EQUAL("vdev-id", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV;
        index ++;
        CTC_CLI_GET_UINT16_RANGE("vdev id", cfg.vdev_id, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }
    else if (CLI_CLI_STR_EQUAL("system", index))
    {
        cfg.mac_learn.limit_type = CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_SYSTEM;
    }

    ret = ctc_app_vlan_port_get_global_cfg(lchip, &cfg);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    action = cfg.mac_learn.limit_action;
    limit_num = cfg.mac_learn.limit_num;

    if (CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_GEM_PORT == cfg.mac_learn.limit_type)
    {
        ctc_cli_out("Port 0x%x :\n", cfg.mac_learn.gport);
        ctc_cli_out("Tunnel Vlan 0x%x :\n", cfg.mac_learn.tunnel_value);
    }
    else if (CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_FID == cfg.mac_learn.limit_type)
    {
        ctc_cli_out("Fid 0x%x :\n", cfg.mac_learn.fid);
    }
    else if (CTC_APP_VLAN_PORT_MAC_LEARN_LIMIT_TYPE_VDEV == cfg.mac_learn.limit_type)
    {
        ctc_cli_out("Vdev-id 0x%x :\n", cfg.vdev_id);
    }
    else
    {
        ctc_cli_out("System :\n");
    }

    ctc_cli_out("Mac limit action %s\n", action_str[action]);
    if (0xFFFFFFFF != limit_num)
    {
        ctc_cli_out("Max number is  0x%x\n", limit_num);
    }
    else
    {
        ctc_cli_out("Max number is  %s\n", "disabled");
    }

    ctc_cli_out("Running number is  0x%x\n", cfg.mac_learn.limit_count);

    return ret;
}

CTC_CLI(ctc_cli_app_api_debug_on,
        ctc_cli_app_api_debug_on_cmd,
        "debug app api (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "App",
        "Core app module",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    typeenum = APP_API;

    ctc_debug_set_flag("app", "api", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_api_debug_off,
        ctc_cli_app_api_debug_off_cmd,
        "no debug app api",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "App",
        "Core app module")
{
    uint32 typeenum = APP_API;
    uint8 level = 0;

    ctc_debug_set_flag("app", "api", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

#if defined(GREATBELT) || defined(DUET2)

/*
Type:
0 --> status
1 --> Pon port
2 --> Gem port
3 --> Vlan port
*/
extern int32 ctc_gbx_app_vlan_port_show(uint8 lchip, uint8 type, uint8 detail, uint32 param0, uint32 param1);

CTC_CLI(ctc_cli_gbx_app_vlan_port_show,
        ctc_cli_gbx_app_vlan_port_show_cmd,
        "show app vlan-port (status | nni-port (port GPORT) | uni-port (port GPORT) | gem-port (port GPORT tunnel-value TUNNEL|) | vlan-port (vlan-port-id ID|) )",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Status",
        "Nni port",
        "Port",
        "Value",
        "Uni port",
        "Port",
        "Value",
        "Gem port",
        "Port",
        "Value",
        "Tunnel value (tunnel vlan)",
        "Value",
        "Vlan port",
        "Vlan port id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 type = 0;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint32 param0 = 0;
    uint32 param1 = 0;

    /*Status*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("status");
    if (index != 0xFF)
    {
        type = 0;
    }

    /*UNI port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("uni-port");
    if (index != 0xFF)
    {
        type = 1;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }
    }

    /*GEM port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gem-port");
    if (index != 0xFF)
    {
        type = 2;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-value");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("tunnel value", param1, argv[index + 1]);
        }
    }

    /*VLAN port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-port");
    if (index != 0xFF)
    {
        type = 3;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-port-id");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("vlan port id", param0, argv[index + 1]);
        }
    }

    /*NNI port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nni-port");
    if (index != 0xFF)
    {
        type = 4;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }
    }

    ret = ctc_gbx_app_vlan_port_show(lchip, type, detail, param0, param1);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

extern int32 ctc_gbx_app_vlan_port_show_logic_port(uint8 lchip, uint32 logic_port);

CTC_CLI(ctc_cli_gbx_app_vlan_port_show_logic_port,
        ctc_cli_gbx_app_vlan_port_show_logic_port_cmd,
        "show app vlan-port (logic-port PORT)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Logic port",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint32 logic_port = 0;


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("logic port", logic_port, argv[index + 1]);
    }

    ret = ctc_gbx_app_vlan_port_show_logic_port(lchip, logic_port);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

extern int32 ctc_gbx_app_vlan_port_show_sync_db(uint8 lchip, uint8 detail, uint32 logic_port, uint16 match_svlan, uint16 match_cvlan);
CTC_CLI(ctc_cli_app_vlan_port_show_sync_db,
        ctc_cli_app_vlan_port_show_sync_db_cmd,
        "show app vlan-port sync db (logic-port ID match-svlan SVLAN match-cvlan CVLAN|)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Sync",
        "Database",
        "Logic port",
        "Logic port Value",
        "Match svlan",
        "Svlan ID",
        "Match cvlan",
        "Cvlan id")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint32 logic_port = 0;
    uint16 svlan = 0;
    uint16 cvlan = 0;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        detail = 1;
        CTC_CLI_GET_UINT32("logic port", logic_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("match svlan", svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("match-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("match cvlan", cvlan, argv[index + 1]);
    }

    ret = ctc_gbx_app_vlan_port_show_sync_db(lchip, detail, logic_port, svlan, cvlan);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

extern int32 ctc_gbx_app_vlan_port_show_fid(uint8 lchip, uint16 vdev_id);
CTC_CLI(ctc_cli_gbx_app_vlan_port_show_fid_db,
        ctc_cli_gbx_app_vlan_port_show_fid_db_cmd,
        "show app vlan-port fid db (packet-svlan SVLAN packet-cvlan CVLAN|) (vdev-id VALUE|)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Fid",
        "Database",
        "Packet svlan",
        "Svlan ID",
        "Packet cvlan",
        "Cvlan id",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint16 svlan = 0;
    uint16 cvlan = 0;
    uint16 vdev_id = 0;
    ctc_app_vlan_port_fid_t port_fid;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-svlan");
    if (index != 0xFF)
    {
        detail = 1;
        CTC_CLI_GET_UINT16("packet svlan", svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet cvlan", cvlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    if (detail)
    {
        sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_t));
        port_fid.vdev_id = vdev_id;
        port_fid.pkt_svlan = svlan;
        port_fid.pkt_cvlan = cvlan;
        ret = ctc_app_vlan_port_get_fid_mapping_info(lchip, &port_fid);
        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        else
        {
            ctc_cli_out("%-10s %15s %15s %10s %10s\n", "vDev id", "Packet svlan", "Packet cvlan", "flex", "fid");
            ctc_cli_out("---------------------------------------------------------------\n");
            ctc_cli_out("%-10d" , port_fid.vdev_id);
            ctc_cli_out("%15d" , port_fid.pkt_svlan);
            ctc_cli_out("%15d" , port_fid.pkt_cvlan);
            ctc_cli_out("%15d" , port_fid.is_flex);
            ctc_cli_out("%10d" , port_fid.fid);
            ctc_cli_out("\n");
        }
    }
    else
    {
        ret = ctc_gbx_app_vlan_port_show_fid(lchip, vdev_id);
        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    return CLI_SUCCESS;
}

#endif
#if defined(TSINGMA)

/*
Type:
0 --> status
1 --> Pon port
2 --> Gem port
3 --> Vlan port
*/
extern int32 ctc_usw_app_vlan_port_show(uint8 lchip, uint8 type, uint8 detail, uint32 param0, uint32 param1);

CTC_CLI(ctc_cli_usw_app_vlan_port_show,
        ctc_cli_usw_app_vlan_port_show_cmd,
        "show app vlan-port (status | nni-port (port GPORT) | uni-port (port GPORT) | gem-port (port GPORT tunnel-value TUNNEL|) | vlan-port (vlan-port-id ID|) )",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Status",
        "Nni port",
        "Port",
        "Value",
        "Uni port",
        "Port",
        "Value",
        "Gem port",
        "Port",
        "Value",
        "Tunnel value (tunnel vlan)",
        "Value",
        "Vlan port",
        "Vlan port id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 type = 0;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint32 param0 = 0;
    uint32 param1 = 0;

    /*Status*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("status");
    if (index != 0xFF)
    {
        type = 0;
    }

    /*UNI port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("uni-port");
    if (index != 0xFF)
    {
        type = 1;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }
    }

    /*GEM port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gem-port");
    if (index != 0xFF)
    {
        type = 2;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-value");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("tunnel value", param1, argv[index + 1]);
        }
    }

    /*VLAN port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-port");
    if (index != 0xFF)
    {
        type = 3;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-port-id");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("vlan port id", param0, argv[index + 1]);
        }
    }

    /*NNI port*/
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nni-port");
    if (index != 0xFF)
    {
        type = 4;

        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("port");
        if (index != 0xFF)
        {
            detail = 1;
            CTC_CLI_GET_UINT32("port", param0, argv[index + 1]);
        }
    }

    ret = ctc_usw_app_vlan_port_show(lchip, type, detail, param0, param1);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

extern int32 ctc_usw_app_vlan_port_show_logic_port(uint8 lchip, uint32 logic_port);

CTC_CLI(ctc_cli_usw_app_vlan_port_show_logic_port,
        ctc_cli_usw_app_vlan_port_show_logic_port_cmd,
        "show app vlan-port (logic-port PORT)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Logic port",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint32 logic_port = 0;


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("logic port", logic_port, argv[index + 1]);
    }

    ret = ctc_usw_app_vlan_port_show_logic_port(lchip, logic_port);

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

extern int32 ctc_usw_app_vlan_port_show_fid(uint8 lchip, uint16 vdev_id);
CTC_CLI(ctc_cli_usw_app_vlan_port_show_fid_db,
        ctc_cli_usw_app_vlan_port_show_fid_db_cmd,
        "show app vlan-port fid db (packet-svlan SVLAN packet-cvlan CVLAN|) (vdev-id VALUE|)",
        "Show",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Fid",
        "Database",
        "Packet svlan",
        "Svlan ID",
        "Packet cvlan",
        "Cvlan id",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint8 detail = 0;
    uint16 svlan = 0;
    uint16 cvlan = 0;
    uint16 vdev_id = 0;
    ctc_app_vlan_port_fid_t port_fid;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-svlan");
    if (index != 0xFF)
    {
        detail = 1;
        CTC_CLI_GET_UINT16("packet svlan", svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet cvlan", cvlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    if (detail)
    {
        sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_t));
        port_fid.vdev_id = vdev_id;
        port_fid.pkt_svlan = svlan;
        port_fid.pkt_cvlan = cvlan;
        ret = ctc_app_vlan_port_get_fid_mapping_info(lchip, &port_fid);
        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        else
        {
            ctc_cli_out("%-10s %15s %15s %10s %10s\n", "vDev id", "Packet svlan", "Packet cvlan", "flex", "fid");
            ctc_cli_out("---------------------------------------------------------------\n");
            ctc_cli_out("%-10d" , port_fid.vdev_id);
            ctc_cli_out("%15d" , port_fid.pkt_svlan);
            ctc_cli_out("%15d" , port_fid.pkt_cvlan);
            ctc_cli_out("%15d" , port_fid.is_flex);
            ctc_cli_out("%10d" , port_fid.fid);
            ctc_cli_out("\n");
        }
    }
    else
    {
        ret = ctc_usw_app_vlan_port_show_fid(lchip, vdev_id);
        if (ret < 0)
        {
            ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    return CLI_SUCCESS;
}
#endif

CTC_CLI(ctc_cli_app_show_fdb,
        ctc_cli_app_show_fdb_cmd,
        "show app vlan-port fdb entry by (vdev-id VALUE|)(mac MAC | mac-fid MAC FID | port GPORT_ID | logic-port LOGIC_PORT_ID | fid FID | port-fid GPORT_ID FID | all ) ((static|dynamic|local-dynamic|mcast|conflict|all)|) (hw|) (entry-file FILE_NAME|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_APP_M_STR,
        "Vlan port",
        CTC_CLI_FDB_DESC,
        "FDB entries",
        "Query condition",
        "VDevice id",
        "Value",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "MAC+Fid",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Logic port",
        "Reference to global config",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+Fid",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "All condition",
        "Static fdb table",
        "All dynamic fdb table",
        "Local chip dynamic fdb table",
        "Mcast",
        "Conflict fdb table",
        "Static and dynamic",
        "Dump by Hardware",
        "File store fdb entry",
        "File name")
{

    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2_fdb_query_rst_t query_rst;
    ctc_l2_write_info_para_t* write_entry_para = NULL;
    uint32 total_count = 0;
    ctc_l2_fdb_query_t Query;
    uint16 entry_cnt = 0;
    uint16 vdev_id = 0;
    uint8 lchip = 0;
    uint32 count = 0;

    sal_memset(&query_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));
    sal_memset(&Query, 0, sizeof(ctc_l2_fdb_query_t));

    Query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Query.query_flag = CTC_L2_FDB_ENTRY_ALL;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    /*1) parse MAC address */
    index = ctc_cli_get_prefix_item(&argv[0], 1, "mac", 3);
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Query.mac, argv[index + 1]);

    }

    /* parse MAC+FID address */
    index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Query.mac, argv[index + 1]);
        CTC_CLI_GET_UINT16_RANGE("forwarding id", Query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*2) parse PORT address */
    index = ctc_cli_get_prefix_item(&argv[0], 1, "port", 4);
    if (index != 0xFF)
    {

        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        CTC_CLI_GET_UINT16_RANGE("gport", Query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*2-1 parse logic port */
    index = ctc_cli_get_prefix_item(&argv[0], 1, "logic-port", 10);
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        Query.use_logic_port = 1;
        CTC_CLI_GET_UINT16_RANGE("logic-port", Query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse fid address*/
    index = ctc_cli_get_prefix_item(&argv[0], 1, "fid", 3);
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        CTC_CLI_GET_UINT16_RANGE("forwarding id", Query.fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", Query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", Query.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        Query.query_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        Query.query_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        Query.query_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mcast");
    if (index != 0xFF)
    {
        Query.query_flag = CTC_L2_FDB_ENTRY_MCAST;
    }

    index = CTC_CLI_GET_ARGC_INDEX("conflict");
    if (index != 0xFF)
    {
        Query.query_flag = CTC_L2_FDB_ENTRY_CONFLICT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-file");
    if (index != 0xFF)
    {
        MALLOC_ZERO(MEM_CLI_MODULE, write_entry_para, sizeof(ctc_l2_write_info_para_t));
        sal_strcpy((char*)&(write_entry_para->file), argv[index + 1]);
    }

    /* 6. parse hw mode */
    index = CTC_CLI_GET_ARGC_INDEX("hw");
    if (index != 0xFF)
    {
        Query.query_hw = 1;
    }
    entry_cnt = 100;

    {
        query_rst.buffer_len = sizeof(ctc_l2_addr_t) * entry_cnt;
        query_rst.buffer = (ctc_l2_addr_t*)mem_malloc(MEM_CLI_MODULE, query_rst.buffer_len);
        if (NULL == query_rst.buffer)
        {
            ctc_cli_out("%% Alloc  memory  failed \n");
            if (write_entry_para)
            {
                mem_free(write_entry_para);
            }
            return CLI_ERROR;
        }

        sal_memset(query_rst.buffer, 0, query_rst.buffer_len);

        ctc_cli_out("%-10s %8s  %14s  %4s %8s\n", "vDev id", "MAC", "FID", "GPORT", "Static");
        ctc_cli_out("-------------------------------------------\n");

        do
        {
            ctc_app_vlan_port_fid_t port_fid;

            query_rst.start_index = query_rst.next_query_index;
            if (g_ctcs_api_en)
            {
                ret = ctcs_l2_get_fdb_entry(g_api_lchip, &Query, &query_rst);
            }
            else
            {
                ret = ctc_l2_get_fdb_entry(&Query, &query_rst);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                mem_free(query_rst.buffer);
                query_rst.buffer = NULL;
                if (write_entry_para)
                {
                    mem_free(write_entry_para);
                }
                return CLI_ERROR;
            }

            for (index = 0; index < Query.count; index++)
            {
                sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_t));
                port_fid.fid = query_rst.buffer[index].fid;
                ret = ctc_app_vlan_port_get_fid_mapping_info(lchip, &port_fid);
                if (!ret && (port_fid.vdev_id == vdev_id))
                {
                    query_rst.buffer[index].fid = port_fid.pkt_svlan;
                }
                else
                {
                    continue;
                }
                ctc_cli_out("%-10d", vdev_id);
                ctc_cli_out("%.4x.%.4x.%.4x%4s  ", sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                            " ");

                ctc_cli_out("%.4d  ", query_rst.buffer[index].fid);
                ctc_cli_out("0x%.4x  ", query_rst.buffer[index].gport);
                ctc_cli_out("%4s\n", (query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No");
                sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
                count++;
            }

            total_count += count;
            sal_task_sleep(100);

        }
        while (query_rst.is_end == 0);

        ctc_cli_out("-------------------------------------------\n");
        ctc_cli_out("Total Entry Num: %d\n", total_count);

        mem_free(query_rst.buffer);
        query_rst.buffer = NULL;

        if (write_entry_para)
        {
            mem_free(write_entry_para);
        }

    }
    return ret;

}

CTC_CLI(ctc_cli_app_vlan_port_alloc_fid,
        ctc_cli_app_vlan_port_alloc_fid_cmd,
        "app vlan-port alloc fid (packet-svlan SVLAN packet-cvlan CVLAN vdev-id VALUE)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Alloc",
        "Fid",
        "Packet svlan",
        "Svlan ID",
        "Packet cvlan",
        "Cvlan id",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 svlan = 0;
    uint16 cvlan = 0;
    uint16 vdev_id = 0;
    ctc_app_vlan_port_fid_t port_fid;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet svlan", svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet cvlan", cvlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_t));
    port_fid.vdev_id = vdev_id;
    port_fid.pkt_svlan = svlan;
    port_fid.pkt_cvlan = cvlan;
    ret = ctc_app_vlan_port_alloc_fid(lchip, &port_fid);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("%-10s %15s %15s %10s\n", "vDev id", "Packet svlan", "Packet cvlan", "fid");
        ctc_cli_out("---------------------------------------------------------------\n");
        ctc_cli_out("%-10d" , port_fid.vdev_id);
        ctc_cli_out("%15d" , port_fid.pkt_svlan);
        ctc_cli_out("%15d" , port_fid.pkt_cvlan);
        ctc_cli_out("%10d" , port_fid.fid);
        ctc_cli_out("\n");
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_app_vlan_port_free_fid,
        ctc_cli_app_vlan_port_free_fid_cmd,
        "app vlan-port free fid (packet-svlan SVLAN packet-cvlan CVLAN vdev-id VALUE)",
        CTC_CLI_APP_M_STR,
        "Vlan port",
        "Free",
        "Fid",
        "Packet svlan",
        "Svlan ID",
        "Packet cvlan",
        "Cvlan id",
        "Vdev id",
        "Value")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 lchip = 0;
    uint16 svlan = 0;
    uint16 cvlan = 0;
    uint16 vdev_id = 0;
    ctc_app_vlan_port_fid_t port_fid;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet svlan", svlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("packet-cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("packet cvlan", cvlan, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vdev-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("vdev id", vdev_id, argv[index + 1]);
    }

    sal_memset(&port_fid, 0, sizeof(ctc_app_vlan_port_fid_t));
    port_fid.vdev_id = vdev_id;
    port_fid.pkt_svlan = svlan;
    port_fid.pkt_cvlan = cvlan;
    ret = ctc_app_vlan_port_free_fid(lchip, &port_fid);
    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

#define _____GINT_____ ""
#if defined(TSINGMA)


 STATIC int32
_ctc_app_gint_cli_parser_vlan_action(unsigned char argc,
                                    char** argv,
                                    uint8 start,
                                    char *field,
                                    uint8* p_vlan_action,
                                    uint16* p_new_value)
{
    uint8 index = 0;
    uint8 start_index = 0;
    uint16 value = 0;

    start_index = CTC_CLI_GET_SPECIFIC_INDEX(field, start);
    if (start_index == 0xFF)
    {
        return 0;
    }

    index = start + start_index + 1;

    if (CLI_CLI_STR_EQUAL("none", index))
    {
        *p_vlan_action = CTC_GINT_VLAN_ACTION_NONE;
    }
    else if(CLI_CLI_STR_EQUAL("add", index))
    {
        *p_vlan_action = CTC_GINT_VLAN_ACTION_ADD;
    }
    else if(CLI_CLI_STR_EQUAL("del", index))
    {
        *p_vlan_action = CTC_GINT_VLAN_ACTION_DEL;
    }
    else if(CLI_CLI_STR_EQUAL("replace", index))
    {
        *p_vlan_action = CTC_GINT_VLAN_ACTION_REPLACE;
    }

    index = start + start_index + 2;

    if (argv[(index)] && index < argc)
    {
        if (CLI_CLI_STR_EQUAL("new-svid", index))
        {
            CTC_CLI_GET_UINT16("new svid", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-scos", index))
        {
            CTC_CLI_GET_UINT16("new scos", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-cvid", index))
        {
            CTC_CLI_GET_UINT16("new cvid", value, argv[index + 1]);
        }
        else if(CLI_CLI_STR_EQUAL("new-ccos", index))
        {
            CTC_CLI_GET_UINT16("new ccos", value, argv[index + 1]);
        }
        *p_new_value = value;
    }


    return 0;

}


CTC_CLI(ctc_cli_gint_init,
        ctc_cli_gint_init_cmd,
        "app gint init",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Init")
{
    int32 ret = 0;

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_init(g_api_lchip);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_deinit,
        ctc_cli_gint_deinit_cmd,
        "app gint deinit",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Deinit")
{
    int32 ret = 0;

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_deinit(g_api_lchip);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_gint_create_uni,
        ctc_cli_gint_create_uni_cmd,
        "app gint create uni (dsl-id DSLID sid SID gport GPORT)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Create",
        "Uni",
        "Dsl id",
        "Dsl port",
        "Sid",
        "Stream ID",
        "Gport",
        "Gport")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_uni_t uni;

    sal_memset(&uni, 0, sizeof(ctc_gint_uni_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", uni.dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("sid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("sid", uni.sid, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", uni.gport, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_create_uni(g_api_lchip, &uni);
         if (0 == ret)
         {
            ctc_cli_out("alloc logic port:%d\n", uni.logic_port);
         }
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_destory_uni,
        ctc_cli_gint_destory_uni_cmd,
        "app gint destroy uni (dsl-id DSLID)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Destory",
        "Uni",
        "Dsl id",
        "Dsl port")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_uni_t uni;

    sal_memset(&uni, 0, sizeof(ctc_gint_uni_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", uni.dsl_id, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_destroy_uni(g_api_lchip, &uni);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_create_nni,
        ctc_cli_gint_create_nni_cmd,
        "app gint create nni (gport GPORT)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Create",
        "Nni",
        "Gport",
        "Gport")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_nni_t nni;

    sal_memset(&nni, 0, sizeof(ctc_gint_nni_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", nni.gport, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_create_nni(g_api_lchip, &nni);
         if (0 == ret)
         {
            ctc_cli_out("alloc logic port:%d\n", nni.logic_port);
         }
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_destory_nni,
        ctc_cli_gint_destory_nni_cmd,
        "app gint destroy nni (gport GPORT)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Destory",
        "Nni",
        "Gport",
        "Gport")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_nni_t nni;

    sal_memset(&nni, 0, sizeof(ctc_gint_nni_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("gport", nni.gport, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_destroy_nni(g_api_lchip, &nni);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_gint_service,
        ctc_cli_gint_service_cmd,
        "app gint (create |destroy) service ((dsl-id DSL_ID) (svlan SVLAN {cvlan CVLAN}) \
        {igs-vlan-action" CTC_CLI_APP_VLAN_ACTION "|egs-vlan-action" CTC_CLI_APP_VLAN_ACTION "|} ",
        CTC_CLI_APP_M_STR,
        "Gint",
        "Create",
        "Destroy",
        "Service",
        "Dsl id",
        "Value",
        "Svlan",
        "Value",
        "Cvlan",
        "Value",
        "Ingress vlan action",
        CTC_CLI_APP_VLAN_ACTION_DSCP,
        "Egress vlan action",
        CTC_CLI_APP_VLAN_ACTION_DSCP)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 start_index = 0;
    ctc_gint_service_t service;
    uint8 is_add = 0;
    uint8 lchip = 0;

   sal_memset(&service, 0, sizeof(ctc_gint_service_t));


   index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("create");
   if (index != 0xFF)
   {
       is_add = 1;
   }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl-id", service.dsl_id, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("svlan value", service.svlan, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("cvlan value", service.cvlan, argv[index + 1]);
    }

    /*igress action*/
    start_index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("igs-vlan-action");

    if (start_index != 0xFF)
    {
        ret = _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "svlan",
                                        &service.igs_vlan_action.svid_action,
                                        &service.igs_vlan_action.new_svid);



        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "scos",
                                        &service.igs_vlan_action.scos_action,
                                        &service.igs_vlan_action.new_scos);



        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "cvlan",
                                        &service.igs_vlan_action.cvid_action,
                                        &service.igs_vlan_action.new_cvid);


        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "ccos",
                                        &service.igs_vlan_action.ccos_action,
                                        &service.igs_vlan_action.new_ccos);


    }


    /*egress action*/
    start_index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egs-vlan-action");

    if (start_index != 0xFF)
    {
        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "svlan",
                                        &service.egs_vlan_action.svid_action,
                                        &service.egs_vlan_action.new_svid);



        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "scos",
                                        &service.egs_vlan_action.scos_action,
                                        &service.egs_vlan_action.new_scos);



        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "cvlan",
                                        &service.egs_vlan_action.cvid_action,
                                        &service.egs_vlan_action.new_cvid);


        ret = ret ? ret : _ctc_app_gint_cli_parser_vlan_action(argc,
                                        &argv[0],
                                        start_index,
                                        "ccos",
                                        &service.egs_vlan_action.ccos_action,
                                        &service.egs_vlan_action.new_ccos);

    }



    if (is_add)
    {
        ret =  ctcs_gint_add_service(lchip,    &service);
    }
    else
    {
        ret =  ctcs_gint_remove_service(lchip, &service);
    }

    if (ret == CLI_ERROR)
    {
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if (is_add)
    {
        ctc_cli_out("alloc logic port:%d\n", service.logic_port);
    }
    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_gint_update,
        ctc_cli_gint_update_cmd,
        "app gint update (dsl-id DSL_ID) {(ingress-policer IGS_POLICER)| (egress-policer EGS_POLICER)| (ingress-stats IGS_STATS)| (egress-stats EGS_STATS)}",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Update",
        "Dsl id",
        "Value",
        "Ingress policer",
        "Policer id",
        "Egress policer",
        "Policer id",
        "Ingress stats",
        "Stats id",
        "Egress stats",
        "Stats id")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_upd_t gint_upd;

    sal_memset(&gint_upd, 0, sizeof(ctc_gint_upd_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl-id", gint_upd.dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ingress-policer");
    if (index != 0xFF)
    {
        gint_upd.flag |= CTC_GINT_UPD_ACTION_IGS_POLICER;
        CTC_CLI_GET_UINT16("ingress-policer", gint_upd.igs_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egress-policer");
    if (index != 0xFF)
    {
        gint_upd.flag |= CTC_GINT_UPD_ACTION_EGS_POLICER;
        CTC_CLI_GET_UINT16("egress-policer", gint_upd.egs_policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ingress-stats");
    if (index != 0xFF)
    {
        gint_upd.flag |= CTC_GINT_UPD_ACTION_IGS_STATS;
        CTC_CLI_GET_UINT32("ingress-stats", gint_upd.igs_stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("egress-stats");
    if (index != 0xFF)
    {
        gint_upd.flag |= CTC_GINT_UPD_ACTION_EGS_STATS;
        CTC_CLI_GET_UINT32("egress-stats", gint_upd.egs_stats_id, argv[index + 1]);
    }
    

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_update(g_api_lchip, &gint_upd);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_gint_port_isolate,
        ctc_cli_gint_port_isolate_cmd,
        "app gint port-isolation  (dsl-id DSLID) (enable|disable)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Port isolation",
        "Dsl id",
        "Dsl port",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint8 index = 0;
    uint32 dsl_id = 0;
    uint8  enable = 0;
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (index != 0xFF)
    {
        enable = 1;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_set_port_isolation(g_api_lchip, dsl_id, enable);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_port_isolate_show,
        ctc_cli_gint_port_isolate_show_cmd,
        "show app gint port-isolation  (dsl-id DSLID)",
        "Show",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Port isolation",
        "Dsl id",
        "Dsl port")
{
    int32 ret = 0;
    uint8 index = 0;
    uint32 dsl_id = 0;
    uint8  enable = 0;
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", dsl_id, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_get_port_isolation(g_api_lchip, dsl_id, &enable);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("dsl-id%d: %s\n", dsl_id, enable ? "enable" : "disable");
    }
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_gint_storm_restrict,
        ctc_cli_gint_storm_restrict_cmd,
        "app gint storm-restrict  (dsl-id DSLID) (boardcast|muticast|unknown-packet) {(cir CIR)| (cbs CBS)|} (enable|disable)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "Storm restric",
        "Dsl id",
        "Value",
        "Boardcast",
        "Muticast",
        "Unknown packet",
        "Cir",
        "Value",
        "Crs",
        "Value",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_storm_restrict_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_gint_storm_restrict_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", cfg.dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("boardcast");
    if (index != 0xFF)
    {
        cfg.type = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("muticast");
    if (index != 0xFF)
    {
        cfg.type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("unknown-packet");
    if (index != 0xFF)
    {
        cfg.type = 2;
    }
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("cir", cfg.cir, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cbs");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("cbs", cfg.cbs, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (index != 0xFF)
    {
        cfg.enable = 1;
    }
    else
    {
        cfg.enable = 0;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_set_storm_restrict(g_api_lchip, &cfg);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_storm_restrict_show,
        ctc_cli_gint_storm_restrict_show_cmd,
        "show app gint storm-restrict  (dsl-id DSLID) (boardcast|muticast|unknown-packet)",
        "Show"
        CTC_CLI_APP_M_STR,
        "GINT",
        "Storm restric",
        "Dsl id",
        "Value",
        "Boardcast",
        "Muticast",
        "Unknown packet")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_storm_restrict_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_gint_storm_restrict_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", cfg.dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("boardcast");
    if (index != 0xFF)
    {
        cfg.type = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("muticast");
    if (index != 0xFF)
    {
        cfg.type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("unknown-packet");
    if (index != 0xFF)
    {
        cfg.type = 2;
    }
    
    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_get_storm_restrict(g_api_lchip, &cfg);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("Dsl id          :%d\n",   cfg.dsl_id);
        ctc_cli_out("--type          :%s\n",   cfg.type ? (cfg.type-1?"unknown-packet":"muticast"):"boardcast");
        ctc_cli_out("--Storm restrict:%s\n",  cfg.enable?"enable":"disable");
        ctc_cli_out("--cir           :%d\n",  cfg.cir);
        ctc_cli_out("--cbs           :%d\n",  cfg.cbs);
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_gint_mac_limit,
        ctc_cli_gint_mac_limit_cmd,
        "app gint mac-limit  (dsl-id DSL_ID) {(limit-num LIMIT_NUM)|} (enable|disable)",
        CTC_CLI_APP_M_STR,
        "GINT",
        "MAC limit",
        "Dsl id",
        "Value",
        "Limit number",
        "Threshold value",
        "Enable",
        "Disable")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_mac_limit_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_gint_mac_limit_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", cfg.dsl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("limit-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("limit num", cfg.limit_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("enable");
    if (index != 0xFF)
    {
        cfg.enable = 1;
    }
    else
    {
        cfg.enable = 0;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_set_mac_limit(g_api_lchip, &cfg);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_gint_mac_limit_show,
        ctc_cli_gint_mac_limit_show_cmd,
        "show app gint mac-limit  (dsl-id DSL_ID) ",
        "Show",    
        CTC_CLI_APP_M_STR,
        "GINT",
        "MAC limit",
        "Dsl id",
        "Value",
        "Limit number",
        "Threshold value")
{
    int32 ret = 0;
    uint8 index = 0;
    ctc_gint_mac_limit_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_gint_mac_limit_t));
    
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("dsl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("dsl id", cfg.dsl_id, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
         ret = ctcs_gint_get_mac_limit(g_api_lchip, &cfg);
    }
    else
    {
        ctc_cli_out("GINT not support in this mode.\n");
        return CLI_ERROR;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s\n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    else
    {
        ctc_cli_out("Dsl id          :%d\n",  cfg.dsl_id);
        ctc_cli_out("--MAC limit     :%s\n",  cfg.enable?"enable":"disable");
        ctc_cli_out("--limit num     :%d\n",  cfg.limit_num);
        ctc_cli_out("--limit cnt     :%d\n",  cfg.limit_cnt);
    }

    return CLI_SUCCESS;
}

#endif

int32
ctc_app_api_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_app_index_alloc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_index_free_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_set_rchip_dsnh_offset_range_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_show_index_used_status_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_init_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_create_gem_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_update_gem_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_create_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_create_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_update_nni_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_global_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_show_global_cfg_cmd);
#if defined(GREATBELT) || defined(DUET2) 
    install_element(CTC_SDK_MODE, &ctc_cli_gbx_app_vlan_port_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gbx_app_vlan_port_show_logic_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gbx_app_vlan_port_show_fid_db_cmd);
#endif
#if defined(TSINGMA)
    install_element(CTC_SDK_MODE, &ctc_cli_usw_app_vlan_port_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_app_vlan_port_show_logic_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_app_vlan_port_show_fid_db_cmd);
#endif
#if defined(GREATBELT) || defined(DUET2)
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_show_sync_db_cmd);
#endif
    install_element(CTC_SDK_MODE, &ctc_cli_app_api_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_api_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_show_fdb_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_security_set_learn_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_show_learn_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_show_vlan_support_slice_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_alloc_fid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_app_vlan_port_free_fid_cmd);

#if defined(TSINGMA)
    install_element(CTC_SDK_MODE, &ctc_cli_gint_init_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_deinit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_create_uni_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_destory_uni_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_create_nni_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_destory_nni_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_service_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_update_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_port_isolate_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_port_isolate_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_storm_restrict_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_storm_restrict_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_mac_limit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_gint_mac_limit_show_cmd);
#endif

    return CLI_SUCCESS;
}

