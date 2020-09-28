
/**
 @file ctc_greatbel_vlan_cli.c

 @date 2011-11-25

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_vlan_cli.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

extern int32
sys_usw_vlan_get_internal_property(uint8 lchip, uint16 vlan_id, uint32 vlan_prop, uint32* value);

extern int32
sys_usw_scl_show_entry(uint8 lchip, uint8 type, uint32 param, uint32 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt);

extern int32
sys_usw_vlan_show_default_vlan_class(uint8 lchip);

extern int32
sys_usw_scl_show_vlan_mapping_entry(uint8 lchip);

extern int32
sys_usw_scl_show_vlan_class_entry(uint8 lchip, uint8 key_type, uint8 detail);

extern int32
sys_usw_vlan_get_internal_property(uint8 lchip, uint16 vlan_id, uint32 vlan_prop, uint32* value);



CTC_CLI(ctc_cli_usw_vlan_show_vlan_info,
        ctc_cli_usw_vlan_show_vlan_info_cmd,
        "show vlan VLAN_ID pim-snooping",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Pim snooping")
{
    int32 ret = 0;
    uint16 vlan_id;
    uint32 val = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);


    if (g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_PIM_SNOOP_EN, &val);
    }
    else
    {
        ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_PIM_SNOOP_EN, &val);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-24s%s\n", "Property", "Value");
    ctc_cli_out("---------------------------------------\n");
    ctc_cli_out("%-24s%d\n", "pim snooping enable", val);
    return CTC_E_NONE;

}


CTC_CLI(ctc_cli_usw_vlan_show_vlan_l3if_id,
        ctc_cli_usw_vlan_show_vlan_l3if_id_cmd,
        "show vlan VLAN_ID l3if-id (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "L3if id",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint16 vlan_id;
    uint32 l3if_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    /* SYS_VLAN_PROP_INTERFACE_ID */
    ret  = sys_usw_vlan_get_internal_property(lchip, vlan_id, 1, &l3if_id);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-24s%s\n", "Property", "Value");
    ctc_cli_out("---------------------------------------\n");
    if (0 == l3if_id)
    {
        ctc_cli_out("%-24s%s\n", "l3if-id", "none");
    }
    else
    {
        ctc_cli_out("%-24s%u\n", "l3if-id", l3if_id);
    }

    return CTC_E_NONE;
}

CTC_CLI(ctc_cli_usw_vlan_show_vlan_mapping_default_entry,
        ctc_cli_usw_vlan_show_vlan_mapping_default_entry_cmd,
        "show vlan mapping default info port GPORT_ID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Default",
        "Vlan mapping default information",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret;
    uint16 gport;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_entry( lchip,4, gport, 0, 1, NULL, 0);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_vlan_show_vlan_class_default_entry,
        ctc_cli_usw_vlan_show_vlan_class_default_entry_cmd,
        "show vlan classifier default info (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Default",
        "Vlan classification default information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_vlan_show_default_vlan_class(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
    }

    return ret;
}
extern int32
sys_usw_vlan_dump_status(uint8 lchip);

CTC_CLI(ctc_cli_usw_vlan_status_show,
        ctc_cli_usw_vlan_status_show_cmd,
        "show vlan status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_vlan_dump_status(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_usw_vlan_show_vlan_class,
        ctc_cli_usw_vlan_show_vlan_class_cmd,
        "show vlan classifier (mac |ipv4 |ipv6) entry (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Mac entry",
        "Ipv4 entry",
        "Ipv6 entry",
        "Entry",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != index)
    {
        type = 1; /* SYS_SCL_KEY_TCAM_MAC */
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (0xFF != index)
    {
        type = 3; /* SYS_SCL_KEY_TCAM_IPV4_SINGLE */
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        type = 4; /* SYS_SCL_KEY_TCAM_IPV6 */
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

     /* key_type = 40 : SYS_SCL_KEY_NUM; */
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_vlan_class_entry(lchip, type, 0);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_vlan_show_vlan_mapping,
        ctc_cli_usw_vlan_show_vlan_mapping_cmd,
        "show vlan mapping entry (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Entry",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

     /* key_type = 40 : SYS_SCL_KEY_NUM; */
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_vlan_mapping_entry(lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_vlan_set_acl_property,
        ctc_cli_usw_vlan_set_acl_property_cmd,
        "vlan VLAN_ID acl-property priority PRIORITY_ID direction (ingress | egress) acl-en (disable |(enable "\
        "tcam-lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext |"\
        "cid | interface | forward | forward-ext | udf) class-id CLASS_ID {use-metadata | use-mapped-vlan | use-port-bitmap | use-logic-port |}))",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Acl property on vlan",
        "Acl priority",
        "PRIORITY_ID",
        "direction",
        "ingress",
        "egress",
        "Vlan acl en",
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
        "Vlan acl class id",
        "CLASS_ID",
        "Vlan acl use metadata",
        "Vlan acl use mapped vlan",
        "Vlan acl use port bitmap",
        "Vlan acl use logic port")
{
    int32 ret = CLI_SUCCESS;
    uint16 vlan_id = 0;
    uint8 index = 0;
    ctc_acl_property_t prop;

    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT16("vlan", vlan_id, argv[0] );
    CTC_CLI_GET_UINT8("priority",prop.acl_priority,argv[1] );

    if(CLI_CLI_STR_EQUAL("disable",3))
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
    index = CTC_CLI_GET_ARGC_INDEX("use-port-bitmap");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
    }
    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if(0xFF != index)
    {
        CTC_SET_FLAG(prop.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_acl_property(g_api_lchip, vlan_id, &prop);
    }
    else
    {
        ret = ctc_vlan_set_acl_property(vlan_id, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_vlan_show_acl_property,
        ctc_cli_usw_vlan_show_acl_property_cmd,
        "show vlan VLAN_ID acl-property direction (ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan acl property",
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret = CLI_SUCCESS;
    uint16 vlan_id = 0;
    uint8  lkup_num = 0;
    uint8  idx = 0;
    uint8 direction = 0;
    ctc_acl_property_t prop;
    char* tcam_key_str[CTC_ACL_TCAM_LKUP_TYPE_MAX] = {
        "l2","l2-l3","l3","vlan","l3-ext","l2-l3-ext",
        "sgt","interface","forward", "forward-ext"
    };

    CTC_CLI_GET_UINT16("vlan_id", vlan_id, argv[0]);

    if(CLI_CLI_STR_EQUAL("ingress", 1 ))
    {
        direction = CTC_INGRESS;
        lkup_num = 8;
    }
    else if(CLI_CLI_STR_EQUAL("egress", 1 ))
    {
        direction = CTC_EGRESS;
        lkup_num = 3;
    }

    ctc_cli_out("=======================================================================================\n");
    ctc_cli_out("Prio dir acl-en tcam-lkup-type class-id port-bitmap logic-port mapped-vlan metadata\n");
    for(idx =0; idx < lkup_num; idx++)
    {
        sal_memset(&prop,0,sizeof(ctc_acl_property_t));
        prop.acl_priority = idx;
        prop.direction = direction;
        if(g_ctcs_api_en)
        {
            ret = ctcs_vlan_get_acl_property(g_api_lchip, vlan_id, &prop);
        }
        else
        {
            ret = ctc_vlan_get_acl_property(vlan_id, &prop);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-5d%-4s%-7d",prop.acl_priority,prop.direction ? "egs":"igs", prop.acl_en);
        if(prop.acl_en)
        {
            ctc_cli_out("%-15s%-9d%-12d%-11d%-12d%-9d\n", tcam_key_str[prop.tcam_lkup_type], prop.class_id,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP) ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT)  ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN) ? 1 : 0,
                            CTC_FLAG_ISSET(prop.flag, CTC_ACL_PROP_FLAG_USE_METADATA)    ? 1 : 0);
        }
        else
        {
            ctc_cli_out("%-15s%-9s%-12s%-11s%-12s%-9s\n","-","-","-","-","-","-");
        }

    }
    ctc_cli_out("=======================================================================================\n");
    return ret;

}

CTC_CLI(ctc_cli_usw_vlan_set_cid,
        ctc_cli_usw_vlan_set_cid_cmd,
        "vlan VLAN_ID cid VALUE",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "category id",
        "Value")
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    uint32 value = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    CTC_CLI_GET_UINT32("value", value, argv[argc - 1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_CID, value);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_CID, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_vlan_show_cid_property,
        ctc_cli_usw_vlan_show_cid_property_cmd,
        "show vlan VLAN_ID property cid",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "vlan property",
        "vlan cid")
{
    int32 ret = 0;
    uint16 vlan_id = 0;
    uint32 value = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);
    ctc_cli_out("Show vlan                        :  0x%04x\n", vlan_id);
    ctc_cli_out("-------------------------------------------\n");


    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_get_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_CID,&value);
    }
    else
    {
        ret = ctc_vlan_get_property(vlan_id, CTC_VLAN_PROP_CID, &value);
    }
    if (ret >= 0)
    {
        ctc_cli_out("%-34s:  %-3d\n", "cid", value);
    }

    ctc_cli_out("------------------------------------------\n");
    if(ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}


CTC_CLI(ctc_cli_usw_vlan_show_vlan_statsid,
        ctc_cli_usw_vlan_show_vlan_statsid_cmd,
        "show vlan VLAN_ID property (ingress-statsid|egress-statsid)(lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Vlan property",
        "Ingress StatsId",
        "Egress StatsId",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint16 vlan_id = 0;
    uint32 value = 0;
    uint8 index = 0;
    uint8  dir = 0, lchip = 0;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    ctc_cli_out("==============================\n");

    if (CLI_CLI_STR_EQUAL("in", 1))
    {
        dir = CTC_INGRESS;
        ctc_cli_out("Ingress:\n");
    }
    else if (CLI_CLI_STR_EQUAL("eg", 1))
    {
        dir = CTC_EGRESS;
        ctc_cli_out("Egress:\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if(0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1] );
    }

    ctc_cli_out("%-24s%s\n", "Property", "Value");
    ctc_cli_out("------------------------------\n");

    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = sys_usw_vlan_get_internal_property(lchip, vlan_id, (CTC_INGRESS == dir)
                                                    ? 32 /*SYS_VLAN_PROP_INGRESS_STATSPTR*/ : 43 /*SYS_VLAN_PROP_EGRESS_STATSPTR*/,
                                                    &value);

    if (ret >= 0)
    {
        ctc_cli_out("%-24s%d\n", "statid:", value);
    }

    ctc_cli_out("==============================\n");
    if(ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_vlan_set_pim_snoop,
        ctc_cli_vlan_set_pim_snoop_cmd,
        "vlan VLAN_ID pim-snooping (enable|disable)",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "Pim snooping on vlan",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE)
{
    int32 ret;
    uint16 vlan_id;
    bool is_enable = TRUE;

    CTC_CLI_GET_UINT16("vlan id", vlan_id, argv[0]);

    if (0 == sal_memcmp("dis", argv[1], 3))
    {
        is_enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_vlan_set_property(g_api_lchip, vlan_id, CTC_VLAN_PROP_PIM_SNOOP_EN, is_enable);
    }
    else
    {
        ret = ctc_vlan_set_property(vlan_id, CTC_VLAN_PROP_PIM_SNOOP_EN, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_vlan_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_l3if_id_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_mapping_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_class_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_status_show_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_mapping_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_class_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_set_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_set_cid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_cid_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_statsid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_vlan_set_pim_snoop_cmd);
    return CLI_SUCCESS;
}

int32
ctc_usw_vlan_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_l3if_id_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_mapping_default_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_class_default_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_status_show_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_mapping_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_class_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_set_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_set_cid_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_cid_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_vlan_show_vlan_statsid_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_vlan_set_pim_snoop_cmd);
    return CLI_SUCCESS;
}

