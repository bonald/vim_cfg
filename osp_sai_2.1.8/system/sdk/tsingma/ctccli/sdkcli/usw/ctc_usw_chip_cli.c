/**
 @file ctc_usw_chip_cli.c

 @date 2012-3-20

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_chip.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_usw_chip_cli.h"
#include "dal.h"

extern int32
sys_usw_datapath_show_status(uint8 lchip);
extern int32
sys_usw_datapath_show_info(uint8 lchip, uint8 type, uint32 start, uint32 end, uint8 is_all);
extern int32
sys_usw_chip_show_ecc_recover_status(uint8 lchip);


#include "usw/include/drv_enum.h"

extern int32
drv_ser_db_set_stop_store(uint8 lchip, uint8 enable);

extern int32
drv_ser_set_ecc_simulation_idx(uint8 lchip, uint8 mode, uint32 tbl_idx);

extern int32
sys_usw_register_show_status(uint8 lchip);
#ifdef TSINGMA
extern int32
sys_tsingma_lb_hash_show_hash_cfg(uint8 lchip,ctc_lb_hash_config_t hash_cfg);

extern int32
sys_tsingma_lb_hash_show_template(uint8 lchip,uint8 template_id);
#endif

extern int32
sys_usw_wb_show_module_version(uint8 lchip);

extern int32
sys_usw_mac_show_mcu_version(uint8 lchip);
extern int32
sys_usw_mac_show_mcu_debug_info(uint8 lchip);
extern int32 sys_usw_mac_set_mcu_enable(uint8 lchip, uint32 enable);
extern int32 sys_usw_mac_get_mcu_enable(uint8 lchip, uint32* enable);
extern int32 sys_usw_mac_set_mcu_port_enable(uint8 lchip, uint32 gport, uint32 enable);
extern int32 sys_usw_mac_get_mcu_port_enable(uint8 lchip, uint32 gport, uint32* enable);
extern int32 sys_usw_inband_show_status(uint8 lchip);
extern int32
sys_usw_parser_set_hash_mask(uint8 lchip, uint8 hash_type, uint8 pkt_field_type, uint16 sel_bitmap);

CTC_CLI(ctc_cli_parser_set_hash_mask,
    ctc_cli_parser_set_hash_mask_cmd,
    "parser hash (linkagg | ecmp | flow) (macsa|macda|ipsa|ipda) mask VALUE",
    CTC_CLI_PARSER_STR,
    "Load balance hash mask",
    "Linkagg hash mask field",
    "Ecmp hash mask field",
    "Flow hash mask field",
    "Add macsa mask, each bit control 8 bits of data",
    "Add macda mask, each bit control 8 bits of data",
    "Add ipsa mask, each bit control 8 bits of data",
    "Add ipda mask, each bit control 8 bits of data",
    "Mask",
    "Value"
    )
{
    uint8 index = 0;
    uint16 sel_bitmap = 0;
    int32 ret = 0;
    uint8 hash_usage = 0;
    ctc_field_key_type_t type = 0;
    index = CTC_CLI_GET_ARGC_INDEX("linkagg");
    if (index != 0xFF)
    {
        hash_usage = 1;                             /*SYS_PARSER_HASH_USAGE_LINKAGG */
    }
    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (index != 0xFF)
    {
        hash_usage = 0;                              /*SYS_PARSER_HASH_USAGE_ECMP*/
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow");
    if (index != 0xFF)
    {
        hash_usage = 2;                              /*SYS_PARSER_HASH_USAGE_FLOW*/
    }
    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (index != 0xFF)
    {
        type = 0;
        CTC_CLI_GET_UINT16("sel_bitmap", sel_bitmap, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("macda");
    if (index != 0xFF)
    {
        type = 1;
        CTC_CLI_GET_UINT16("sel_bitmap", sel_bitmap, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (index != 0xFF)
    {
        type = 2;
        CTC_CLI_GET_UINT16("sel_bitmap", sel_bitmap, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (index != 0xFF)
    {
        type = 3;
        CTC_CLI_GET_UINT16("sel_bitmap", sel_bitmap, argv[index + 1]);
    }

    ret = sys_usw_parser_set_hash_mask(g_api_lchip, hash_usage, type, sel_bitmap);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_datapath_show_status,
        ctc_cli_usw_datapath_show_status_cmd,
        "show datapath status ",
        CTC_CLI_SHOW_STR,
        "Datapath module",
        "Status")
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_datapath_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_datapath_show_info,
        ctc_cli_usw_datapath_show_info_cmd,
        "show datapath info (hss|serdes|clktree|calendar) (((start-idx SIDX) (end-idx EIDX))|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Datapath module",
        "Information",
        "Hss",
        "SerDes",
        "Clock Tree",
        "Calendar",
        "Start index",
        "Index",
        "End index",
        "Index")
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;
    uint32 start_idx = 0;
    uint32 end_idx = 0;
    uint8 is_all = 1;
    uint8 type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("hss");
    if(index != 0xFF)
    {
         type = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("serdes");
    if(index != 0xFF)
    {
         type = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("clktree");
    if(index != 0xFF)
    {
         type = 2;
    }
    index = CTC_CLI_GET_ARGC_INDEX("calendar");
    if(index != 0xFF)
    {
         type = 3;
    }
    index = CTC_CLI_GET_ARGC_INDEX("start-idx");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT32("start-idx", start_idx, argv[index+1]);
       is_all = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("end-idx");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT32("end-idx", end_idx, argv[index+1]);
       is_all = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_datapath_show_info(lchip, type, start_idx, end_idx, is_all);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


extern dal_op_t g_dal_op;

uint8 g_usw_debug = 0;

CTC_CLI(ctc_cli_usw_debug,
        ctc_cli_usw_debug_cmd,
        "debug driver (on|off)",
        "debug",
        "driver",
        "on",
        "off")
{
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("on");
    if(0xFF != index)
    {
        g_usw_debug = 1;
    }
    else
    {
        g_usw_debug = 0;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_chip_set_ecc_simulation_store,
        ctc_cli_usw_chip_set_ecc_simulation_store_cmd,
        "chip ecc simulation stop store (enable | disable)  (lchip LCHIP|)",
        "Chip module",
        "Drv ecc recover",
        "Simulation",
        "Stop"
        "Store",
        "Enable",
        "Disable",
        "lchip",
        "Local chip id")
{
    int32   ret = CLI_SUCCESS;
    uint8   lchip = 0;
    uint8   enable = 0;
    uint8   index = 0;


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if(0xFF != index)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = drv_ser_db_set_stop_store(lchip, enable);
    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_chip_show_ecc_status,
        ctc_cli_usw_chip_show_ecc_status_cmd,
        "show chip ecc status  (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Ecc error recover",
        "Status",
        "Local chip id",
        "Chip id")
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    sys_usw_chip_show_ecc_recover_status(lchip);

    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_chip_set_ecc_simulation_idx,
        ctc_cli_usw_chip_set_ecc_simulation_idx_cmd,
        "chip ecc simulation (disable | (tbl-index INDEX)) (lchip LCHIP|)",
        "Chip module",
        "Ecc error recover",
        "Simulation",
        "Disable",
        "Table index",
        "Index num",
        "Local chip id",
        "Chip id")
{
    int32  ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;
    uint8  enable = 0;
    uint32 idx_value = 0;



    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (index != 0xFF)
    {
        enable = 0;
    }
    else
    {
        enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tbl-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("tbl-index", idx_value, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = drv_ser_set_ecc_simulation_idx(lchip, enable, idx_value);

    if (ret)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* source master and queue*/
CTC_CLI(ctc_cli_usw_master_queue,
    ctc_cli_usw_master_queue_cmd,
    "master queue config",
    "master",
    "queue",
    "config")
{
    int32 ret = CLI_SUCCESS;
    char *line = "source master.txt";

    ret = ctc_vti_command(g_ctc_vti, line);
    if(ret)
    {
        ctc_cli_out("%s\n", line);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_chip_set_glb_acl_lkup_en,
        ctc_cli_usw_chip_set_glb_acl_lkup_en_cmd,
        "chip global-cfg acl-lkup-property priority PRIORITY_ID direction (ingress | egress ) acl-en (disable |(enable "\
        "tcam-lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext |"\
        "cid | interface | forward | forward-ext | copp | udf) {class-id CLASS_ID |(use-port-bitmap | use-metadata|use-logic-port )| use-mapped-vlan |use-wlan|}))",
        "chip module",
        "Global config",
        "Acl lookup property",
        "Acl lookup priority",
        "PRIORITY_ID",
        "direction",
        "ingress",
        "egress",
        "Global acl en",
        "Disable function",
        "Enable function",
        "Tcam lookup type",
        "L2 key",
        "L3 key",
        "L2 L3 key",
        "Vlan key",
        "L3 extend key",
        "L2 L3 extend key",
        "CID  key",
        "Interface key",
        "Forward key",
        "Forward extend key",
        "Udf key",
        "Copp key",
        "Port acl use class id",
        "CLASS_ID",
        "Global acl use port-bitmap",
        "Global acl use metadata",
        "Global acl use logic-port",
        "Use mapped vlan",
        "Use wlan")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;

    ctc_acl_property_t prop;

    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT8("priority",prop.acl_priority,argv[0] );

    if(CLI_CLI_STR_EQUAL("disable",2))
    {
        prop.acl_en = 0;
    }
    else
    {
        prop.acl_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if(0xFF != index)
    {
        prop.direction= CTC_INGRESS;

    }
    else
    {
        prop.direction= CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcam-lkup-type");
    if(0xFF != index)
    {
        if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3;
        }
        else if(CLI_CLI_STR_EQUAL("vlan",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_VLAN;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l3-ext",index + 1))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3_EXT;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3-ext",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;
        }
        else if(CLI_CLI_STR_EQUAL("cid",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;
        }
        else if(CLI_CLI_STR_EQUAL("interface",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_INTERFACE;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("forward",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("forward-ext",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("copp",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_COPP;
        }
        else if(CTC_CLI_STR_EQUAL_ENHANCE("udf",index + 1 ))
        {
            prop.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_UDF;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("class-id");
    if(0xFF != index)
    {
        CTC_CLI_GET_UINT8("class-id",prop.class_id, argv[index + 1] );
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-port-bitmap");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-metadata");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_METADATA);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-wlan");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_WLAN);
    }

	index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_ACL_LKUP_PROPERTY, &prop);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_ACL_LKUP_PROPERTY, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



CTC_CLI(ctc_cli_usw_chip_get_glb_acl_lkup_en,
        ctc_cli_usw_chip_get_glb_acl_lkup_en_cmd,
        "show chip global-cfg acl-lkup-property priority PRIORITY_ID direction (ingress | egress ) ",
        CTC_CLI_SHOW_STR,
        "chip module",
        "Global config",
        "Acl lookup property",
        "Acl lookup priority",
        "PRIORITY_ID",
        "direction",
        "ingress",
        "egress" )
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;

    ctc_acl_property_t acl_lkup_prop;

    sal_memset(&acl_lkup_prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT8("priority",acl_lkup_prop.acl_priority,argv[0] );


    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if(0xFF != index)
    {
        acl_lkup_prop.direction= CTC_INGRESS;

    }
    else
    {
        acl_lkup_prop.direction= CTC_EGRESS;
   	}

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_ACL_LKUP_PROPERTY, &acl_lkup_prop);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_ACL_LKUP_PROPERTY, &acl_lkup_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Direction: %s\n", acl_lkup_prop.direction ?  "EGRESS" :"INGRESS" );
    ctc_cli_out("Acl_priority : %d\n", acl_lkup_prop.acl_priority);
    ctc_cli_out("Tcam lkup type : %d\n", acl_lkup_prop.tcam_lkup_type);
    ctc_cli_out("class id : %d\n", acl_lkup_prop.class_id);
    if(CTC_FLAG_ISSET(acl_lkup_prop.flag,CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
       ctc_cli_out("Used port bitmap as key\n");
    }
    if(CTC_FLAG_ISSET(acl_lkup_prop.flag,CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
       ctc_cli_out("Used logic port as key\n");
    }
    if(CTC_FLAG_ISSET(acl_lkup_prop.flag,CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        ctc_cli_out("Used metadata as key\n");
    }
    if(CTC_FLAG_ISSET(acl_lkup_prop.flag,CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN))
    {
        ctc_cli_out("Used mapped vlan as key\n");
    }
    if(CTC_FLAG_ISSET(acl_lkup_prop.flag,CTC_ACL_PROP_FLAG_USE_WLAN))
    {
        ctc_cli_out("Used capwap info vlan as key\n");
    }
    return ret;
}

CTC_CLI(ctc_cli_chip_set_ipmc_config,
        ctc_cli_chip_set_ipmc_config_cmd,
        "chip global-cfg ipmc-property l2mc MODE",
        "Chip module",
        "Global config",
        "Ipmc property",
        "IP based l2mc",
        "If set,ip base L2MC lookup failed,will use mac based L2MC lookup result")
{
    int32 ret = CLI_SUCCESS;
    ctc_global_ipmc_property_t property;

    sal_memset(&property, 0, sizeof(ctc_global_ipmc_property_t));
    CTC_CLI_GET_UINT8("value", property.ip_l2mc_mode, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_IPMC_PROPERTY, &property);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_IPMC_PROPERTY, &property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_inband_cpu_traffic_timer,
        ctc_cli_chip_set_inband_cpu_traffic_timer_cmd,
        "chip global-cfg inband-cpu-traffic (timer TIMER)",
        "Chip module",
        "Global config",
        "Inband CPU traffic",
        "Timer",
        "Value")
{
    int32  ret = CLI_SUCCESS;
    uint32 timer = 0;
    uint8   index  = 0;

    index = CTC_CLI_GET_ARGC_INDEX("timer");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("timer", timer, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_INBAND_CPU_TRAFFIC_TIMER, &timer);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_INBAND_CPU_TRAFFIC_TIMER, &timer);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_show_chip_inband_status,
        ctc_cli_show_chip_inband_status_cmd,
        "show chip inband-cpu-traffic status  (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Inband CPU traffic",
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 index = 0;
    uint8 lchip = 0;
    int32 ret = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ret = sys_usw_inband_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_chip_show_ipmc_config,
        ctc_cli_chip_show_ipmc_config_cmd,
        "show chip global-cfg ipmc-property",
        "Show",
        "Chip module",
        "Global config",
        "Ipmc property")
{
    int32 ret = CLI_SUCCESS;
    ctc_global_ipmc_property_t property;

    sal_memset(&property, 0, sizeof(ctc_global_ipmc_property_t));
    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_IPMC_PROPERTY, &property);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_IPMC_PROPERTY, &property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show chip global-cfg ipmc info\n");
    ctc_cli_out("==============================\n");
    ctc_cli_out("l2mc mode : %d\n", property.ip_l2mc_mode);
    ctc_cli_out("==============================\n");

    return ret;
}

CTC_CLI(ctc_cli_usw_chip_dump_db,
        ctc_cli_usw_chip_dump_db_cmd,
        "chip global-cfg dump-db ({port|vlan|linkagg|chip|ftm|nexthop|l2|l3if|ipuc|ipmc|ip-tunnel|scl|acl|qos|security|stats|mpls|oam|aps|ptp|\
        dma|interrupt|packet|pdu|mirror|bpe|stacking|overlay|ipfix|efd|monitor|fcoe|trill|wlan|npm|vlan-mapping|dot1ae|sync-ether|app-vlanport|\
        datapath|register|diag} | all) {(detail)|( file FILE_NAME)|}",
        "Chip module",
        "Global config",
        "Dump db",
        "Port module",
        "Vlan module",
        "Linkagg module",
        "Chip module",
        "Ftm module",
        "Nexthop module",
        "L2 module",
        "L3if module",
        "Ipuc module",
        "Ipmc module",
        "Ip-tunnel module",
        "Scl module",
        "Acl module",
        "Qos module",
        "Security module",
        "Stats module",
        "Mpls module",
        "Oam module",
        "Aps module",
        "Ptp module",
        "Dma module",
        "Interrupt module",
        "Packet module",
        "Pdu module",
        "Mirror module",
        "Bpe module",
        "Stacking module",
        "Overlay module",
        "Ipfix module",
        "Efd module",
        "Monitor module",
        "Fcoe module",
        "Trill module",
        "Wlan module",
        "Npm module",
        "Vlan-mapping module",
        "Dot1ae module",
        "Sync-ether module",
        "App-vlanport module",
        "Datapath module",
        "Register module",
        "Diag module",
        "All module",
        "Detail data",
        "file",
        "File name and route")
{
    int32  ret    = CLI_SUCCESS;
    uint8  lchip = 0;
    uint8  index = 0;
    ctc_global_dump_db_t dump_param;
    sal_memset(&dump_param, 0, sizeof(ctc_global_dump_db_t));
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("file");
    if (0xFF != index)
    {
        sal_memcpy(dump_param.file, argv[argc - 1], sal_strlen(argv[argc - 1]));
    }
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        dump_param.detail = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_PORT);
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_VLAN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("linkagg");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_LINKAGG);
    }
    index = CTC_CLI_GET_ARGC_INDEX("chip");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_CHIP);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ftm");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_FTM);
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_NEXTHOP);
    }
    index = CTC_CLI_GET_ARGC_INDEX("l2");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_L2);
    }
    index = CTC_CLI_GET_ARGC_INDEX("l3if");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_L3IF);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipuc");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_IPUC);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipmc");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_IPMC);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-tunnel");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_IP_TUNNEL);
    }
    index = CTC_CLI_GET_ARGC_INDEX("scl");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_SCL);
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_ACL);
    }
    index = CTC_CLI_GET_ARGC_INDEX("qos");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_QOS);
    }
    index = CTC_CLI_GET_ARGC_INDEX("security");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_SECURITY);
    }
    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_STATS);
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_MPLS);
    }
    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_OAM);
    }
    index = CTC_CLI_GET_ARGC_INDEX("aps");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_APS);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ptp");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_PTP);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dma");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_DMA);
    }
    index = CTC_CLI_GET_ARGC_INDEX("interrupt");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_INTERRUPT);
    }
    index = CTC_CLI_GET_ARGC_INDEX("packet");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_PACKET);
    }
    index = CTC_CLI_GET_ARGC_INDEX("pdu");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_PDU);
    }
    index = CTC_CLI_GET_ARGC_INDEX("mirror");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_MIRROR);
    }
    index = CTC_CLI_GET_ARGC_INDEX("bpe");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_BPE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("stacking");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_STACKING);
    }
    index = CTC_CLI_GET_ARGC_INDEX("overlay");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_OVERLAY);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipfix");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_IPFIX);
    }
    index = CTC_CLI_GET_ARGC_INDEX("efd");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_EFD);
    }
    index = CTC_CLI_GET_ARGC_INDEX("monitor");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_MONITOR);
    }
    index = CTC_CLI_GET_ARGC_INDEX("fcoe");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_FCOE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("trill");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_TRILL);
    }
    index = CTC_CLI_GET_ARGC_INDEX("wlan");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_WLAN);
    }
    index = CTC_CLI_GET_ARGC_INDEX("npm");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_NPM);
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlan-mapping");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_VLAN_MAPPING);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dot1ae");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_DOT1AE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sync-ether");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_SYNC_ETHER);
    }
    index = CTC_CLI_GET_ARGC_INDEX("app-vlanport");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_APP_VLAN_PORT);
    }
    index = CTC_CLI_GET_ARGC_INDEX("datapath");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_DATAPATH);
    }
    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (0xFF != index)
    {
        dump_param.bit_map[0] = 0xffffffff;
        dump_param.bit_map[1] = 0xffffffff;
    }
    index = CTC_CLI_GET_ARGC_INDEX("register");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_REGISTER);
    }
    index = CTC_CLI_GET_ARGC_INDEX("diag");
    if (0xFF != index)
    {
        CTC_BMP_SET(dump_param.bit_map, CTC_FEATURE_DIAG);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_DUMP_DB, (void*)&dump_param);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_DUMP_DB, (void*)&dump_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

#if 1
extern int cli_com_source_file(ctc_cmd_element_t *, ctc_vti_t *, int, char **);
extern ctc_cmd_element_t cli_com_source_file_cmd;
extern ctc_vti_t *g_ctc_vti;

/**
 @brief The function is to initialize datapath
*/
CTC_CLI(ctc_cli_chip_show_status,
        ctc_cli_chip_show_status_cmd,
        "show chip status (lchip LCHIP|)",
        "Show",
        "Chip module",
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_register_show_status(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_usw_chip_show_wb_version,
        ctc_cli_usw_chip_show_wb_version_cmd,
        "show warmboot version",
        "Show",
        "Warmboot module",
        "Version")
{
    uint8 lchip = 0;

    sys_usw_wb_show_module_version(lchip);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_chip_show_mcu_version,
        ctc_cli_usw_chip_show_mcu_version_cmd,
        "show chip mcu version",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "MCU",
        "Version")
{
    uint8 lchip = 0;

    sys_usw_mac_show_mcu_version(lchip);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_chip_show_mcu_info,
        ctc_cli_usw_chip_show_mcu_info_cmd,
        "show chip mcu info",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "MCU",
        "Debug Information")
{
    uint8 lchip = 0;

    sys_usw_mac_show_mcu_debug_info(lchip);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_chip_set_mcu_on,
        ctc_cli_usw_chip_set_mcu_on_cmd,
        "chip set mcu (enable | disable) (gport GPHYPORT_ID |) (lchip LCHIP |)",
        "chip module",
        "set operate",
        "MCU",
        "Enable",
        "Disable",
        "gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 value = 0;
    uint32 gport = 0;
    uint8 port_cli_flag = 0;

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        port_cli_flag = 1;
    }
    else
    {
        port_cli_flag = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (port_cli_flag)
    {
        ret = sys_usw_mac_set_mcu_port_enable(lchip, gport, value);
    }
    else
    {
        ret = sys_usw_mac_set_mcu_enable(lchip, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_chip_show_mcu_status,
        ctc_cli_usw_chip_show_mcu_status_cmd,
        "show chip mcu status (gport (GPHYPORT_ID | all)|) (lchip LCHIP |)",
        CTC_CLI_SHOW_STR,
        "chip module",
        "MCU",
        "Status",
        "gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 value = 0;
    uint32 gport = 0;
    uint8  port_cli_flag = 0;
    uint8  port_all_flag = 0;

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        if (CLI_CLI_STR_EQUAL("all", 1))
        {
            port_all_flag = 1;
        }
        else
        {
            CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
        port_cli_flag = 1;
    }
    else
    {
        port_cli_flag = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (port_cli_flag)
    {
        if (port_all_flag)
        {
            ctc_cli_out("-----------------------------------\n");
            ctc_cli_out(" Port id\tPort enable?\n");
            ctc_cli_out("-----------------------------------\n");
            for (gport = 0; gport < 64; gport++)
            {
                ret = sys_usw_mac_get_mcu_port_enable(lchip, gport, &value);
                if (ret < 0)
                {
                    ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                    return CLI_ERROR;
                }
                ctc_cli_out("% 8d\t%s\n", gport, value?"enable\t":"\tdisable");
            }

            ctc_cli_out("-----------------------------------\n");
        }
        else
        {
            ret = sys_usw_mac_get_mcu_port_enable(lchip, gport, &value);
            if (ret < 0)
            {
                ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            ctc_cli_out("MCU port %d status:  %s\n", gport, value?"enable":"disable");
        }
    }
    else
    {
        ret = sys_usw_mac_get_mcu_enable(lchip, &value);
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("%-20s:  %s\n","MCU status", value?"enable":"disable");
    }

    return CLI_SUCCESS;
}


int32
tsingma_datapath_init(void)
{
    return 0;
}



extern int write_entry(uint8 lchip, tbls_id_t id, uint32 index, uint32 offset, uint32 value);
extern int32 write_tbl_reg(uint8 lchip, tbls_id_t id, uint32 index, fld_id_t field_id, uint32* value);

extern int32 drv_usw_chip_write(uint8 lchip, uint32 offset, uint32 value);


extern int32 drv_usw_get_field_id_by_string(uint8 lchip, tbls_id_t tbl_id, fld_id_t* field_id, char* name);
extern  int32 drv_usw_get_tbl_id_by_string(uint8 lchip, tbls_id_t* tbl_id, char* name);

CTC_CLI(cli_dbg_tool_write_tbl_fld_reg,
    cli_dbg_tool_write_tbl_reg_fld_cmd,
    "write chip CHIP_ID tbl-reg TBL_REG_ID INDEX (field FIELD_NAME| offset OFFSET) VAL",
    "write chip table or register info",
    "chip information",
    "chipid value",
    "write table or register",
    "table or register ID name",
    "table or register entry index <0>",
    "Table/register fileld id name",
    "Table/register field name(string)",
    "Table offset",
    "Table offset value",
    "Set value")
{

#define TIPS_NONE           0
#define TIPS_TBL_REG        1
#define TIPS_TBL_REG_FIELD  2


    uint32 chip_id = 0;
    char* id_name = NULL;
    char* field_name = NULL;

    tbls_id_t id = 0;
    fld_id_t field_id = 0;
    uint32 index = 0;
    uint32 idx = 0;
    uint32 value[64] = {0}    ;
    uint32 offset = 0xFFFFFFFF;
    int32 ret = 0;

    CTC_CLI_GET_INTEGER("chipid", chip_id, argv[0]);

    id_name = argv[1];
    if (0 != drv_usw_get_tbl_id_by_string(0, &id, id_name))
    {
        ctc_cli_out("can't find tbl %s \n", id_name);
        return CLI_ERROR;
    }
    CTC_CLI_GET_INTEGER("tbl/reg index", index, argv[2]);

    idx = ctc_cli_get_prefix_item(&argv[0], argc, "field", sal_strlen("field"));
    if (0xFF != idx)
    {
        field_name = argv[idx + 1];
        if (0 != drv_usw_get_field_id_by_string(0, id, &field_id, field_name))
        {
            ctc_cli_out("can't find table %s field %s \n", id_name, field_name);
            return CLI_ERROR;
        }

        CTC_CLI_GET_INTEGER_N("field-value", (uint32 *)value, argv[idx + 2]);
    }

    idx = ctc_cli_get_prefix_item(&argv[0], argc, "offset", sal_strlen("offset"));
    if (0xFF != idx)
    {
        CTC_CLI_GET_INTEGER("offset", offset, argv[idx + 1]);
        CTC_CLI_GET_INTEGER_N("field-value", (uint32 *)value, argv[idx + 2]);
    }

    if (0xFFFFFFFF != offset)
    {
        write_entry(0, id, index, offset, value[0]);
    }
    else
    {
        ret = write_tbl_reg(0, id, index, field_id, value);

        if (ret < 0)
        {
            return ret;
        }
    }

    return CLI_SUCCESS;
}

/* Cmd format: write address <addr_offset> <value> */
CTC_CLI(cli_dbg_tool_write_addr,
    cli_dbg_tool_write_addr_cmd,
    "write chip CHIP_ID address ADDR_OFFSET WRITE_VALUE",
    "write value to address cmd",
    "chip information",
    "chip id value",
    "write address 0xXXXXXXXX <value>",
    "address value",
    "value to write")
{
    uint32 address_offset = 0, chip_id = 0, value = 0;
    int32 ret = 0;

    CTC_CLI_GET_INTEGER("chip_id", chip_id, argv[0]);
    CTC_CLI_GET_INTEGER("address", address_offset, argv[1]);
    CTC_CLI_GET_INTEGER("value", value, argv[2]);


    ret = drv_usw_chip_write(0, address_offset, value);

    if (ret < 0)
    {
        ctc_cli_out("0x%08x address write ERROR! Value = 0x%08x\n", address_offset, value);
    }

    return CLI_SUCCESS;
}

#ifdef TSINGMA
CTC_CLI(ctc_cli_chip_show_lb_hash_status,
        ctc_cli_chip_show_lb_hash_status_cmd,
        "show lb-hash select-id SEL-ID (hash-select|hash-control) status",
        CTC_CLI_SHOW_STR,
        "Load banlance hash ",
        "Selector",
        "Value of selector",
        "Hash select",
        "Hash control",
        "status")
        {

            uint32 ret = 0;
            uint8 index = 0;
            ctc_lb_hash_config_t hash_cfg;
            sal_memset(&hash_cfg,0,sizeof(ctc_lb_hash_config_t));
            CTC_CLI_GET_INTEGER("select-id", hash_cfg.sel_id, argv[0]);
            index = CTC_CLI_GET_ARGC_INDEX("hash-select");
            if (0xFF != index)
            {
                hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_SELECT;

            }
            index = CTC_CLI_GET_ARGC_INDEX("hash-control");
            if (0xFF != index)
            {
                hash_cfg.cfg_type = CTC_LB_HASH_CFG_HASH_CONTROL;

            }
            ret = sys_tsingma_lb_hash_show_hash_cfg(0,hash_cfg);
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            return ret;

        }
CTC_CLI(ctc_cli_chip_show_lb_hash_template,
        ctc_cli_chip_show_lb_hash_template_cmd,
        "show lb-hash template VALUE",
        CTC_CLI_SHOW_STR,
        "Load banlance hash ",
        "hash-key template",
        "Value of template")
        {
            int32 ret = 0;
            uint8 temp1 = 0;
            CTC_CLI_GET_INTEGER("value", temp1, argv[0]);
            ret = sys_tsingma_lb_hash_show_template(0, temp1);
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }
            return ret;
        }
#endif


#endif

int32
ctc_usw_chip_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_datapath_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_datapath_show_info_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_ecc_simulation_store_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_chip_show_ecc_status_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_ecc_simulation_idx_cmd);

    install_element(CTC_CMODEL_MODE, &ctc_cli_usw_master_queue_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_debug_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_chip_set_glb_acl_lkup_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_chip_get_glb_acl_lkup_en_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_ipmc_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_ipmc_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_inband_cpu_traffic_timer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_show_chip_inband_status_cmd);
#ifdef TSINGMA
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_template_cmd);
#endif

#if 1
    install_element(CTC_SDK_MODE, &cli_dbg_tool_write_tbl_reg_fld_cmd);
    install_element(CTC_SDK_MODE, &cli_dbg_tool_write_addr_cmd);
#endif

    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_chip_show_wb_version_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_version_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_info_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_mcu_on_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_status_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_parser_set_hash_mask_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_chip_dump_db_cmd);

    return 0;
}

int32
ctc_usw_chip_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_datapath_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_datapath_show_info_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_ecc_simulation_store_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_chip_show_ecc_status_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_ecc_simulation_idx_cmd);

    uninstall_element(CTC_CMODEL_MODE, &ctc_cli_usw_master_queue_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_debug_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_chip_set_glb_acl_lkup_en_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_chip_get_glb_acl_lkup_en_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_chip_show_wb_version_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_chip_set_inband_cpu_traffic_timer_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_show_chip_inband_status_cmd);
#ifdef TSINGMA
    uninstall_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_template_cmd);
#endif

    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_version_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_info_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_set_mcu_on_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_usw_chip_show_mcu_status_cmd);
    uninstall_element(CTC_INTERNAL_MODE, &ctc_cli_parser_set_hash_mask_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_chip_dump_db_cmd);

    return 0;
}

