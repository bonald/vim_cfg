/**
 @file ctc_usw_l3if_cli.c

 @date 2010-03-16

 @version v1.0

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
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_l3if_cli.h"

extern int32
sys_usw_l3if_state_show(uint8 lchip);

extern int32
sys_usw_l3if_show_ecmp_if_info(uint8 lchip, uint16 ecmp_if_id);

extern int32
sys_usw_l3if_get_vrf_statsid(uint8 lchip, uint16 vrfid, uint32* p_statsid);


CTC_CLI(ctc_cli_usw_l3if_show_ecmp_if,
        ctc_cli_usw_l3if_show_ecmp_if_cmd,
        "show l3if ecmp-if IFID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_ECMP_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint16 l3if_id = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
    	CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT16_RANGE("l3if_id", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l3if_show_ecmp_if_info(lchip, l3if_id);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l3if_show_vrf_statsid,
        ctc_cli_usw_l3if_show_vrf_statsid_cmd,
        "show l3if stats vrfid VRFID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_STATS_STR,
        CTC_CLI_VRFID_DESC,
        CTC_CLI_VRFID_ID_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint16 vrfid = 0;
    uint32 statsid = 0;
    int32  ret = CLI_SUCCESS;
    uint8  lchip = 0;
    uint8  index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT16_RANGE("vrfid", vrfid, argv[0], 0, CTC_MAX_UINT16_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_l3if_get_vrf_statsid(lchip, vrfid, &statsid);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");
    ctc_cli_out(" %-24s%s\n", "Property", "Value");
    ctc_cli_out(" ------------------------------\n");
    ctc_cli_out(" %-24s%u\n", "StatsId",  statsid);
    ctc_cli_out("\n");

    return ret;
}

CTC_CLI(ctc_cli_usw_l3if_set_acl_property,
        ctc_cli_usw_l3if_set_acl_property_cmd,
        "l3if ifid IFID acl-property priority PRIORITY_ID direction (ingress | egress) acl-en (disable |(enable "\
        "tcam-lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext |"\
        "cid | interface | forward | forward-ext | udf) class-id CLASS_ID {use-metadata | use-mapped-vlan | use-port-bitmap | use-logic-port |}))",
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Acl property on l3if",
        "Acl priority",
        "PRIORITY_ID",
        "direction",
        "ingress",
        "egress",
        "l3if acl en",
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
        "l3if acl class id",
        "CLASS_ID",
        "l3if acl use metadata",
        "l3if acl use mapped vlan",
        "l3if acl use port bitmap",
        "l3if acl use logic port")
{
    int32 ret = CLI_SUCCESS;
    uint16 l3if = 0;
    uint8 index = 0;
    ctc_acl_property_t prop;

    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT16("l3if", l3if, argv[0] );
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
        ret = ctcs_l3if_set_acl_property(g_api_lchip, l3if, &prop);
    }
    else
    {
        ret = ctc_l3if_set_acl_property(l3if, &prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l3if_show_acl_property,
        ctc_cli_usw_l3if_show_acl_property_cmd,
        "show l3if ifid IFID acl-property direction (ingress | egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "L3if acl property",
        "Direction",
        "Ingress",
        "Egress")
{
    int32 ret = CLI_SUCCESS;
    uint16 l3if_id = 0;
    uint8  lkup_num = 0;
    uint8  idx = 0;
    ctc_acl_property_t prop;
    char* tcam_key_str[CTC_ACL_TCAM_LKUP_TYPE_MAX] = {
        "l2","l2-l3","l3","vlan","l3-ext","l2-l3-ext",
        "sgt","interface","forward", "forward-ext"
    };

    sal_memset(&prop,0,sizeof(ctc_acl_property_t));

    CTC_CLI_GET_UINT16("l3if", l3if_id, argv[0]);

    if(CLI_CLI_STR_EQUAL("ingress", 1 ))
    {
        prop.direction = CTC_INGRESS;
        lkup_num = 8;
    }
    else if(CLI_CLI_STR_EQUAL("egress", 1 ))
    {
        prop.direction = CTC_EGRESS;
        lkup_num = 3;
    }

    ctc_cli_out("=======================================================================================\n");
    ctc_cli_out("Prio dir acl-en tcam-lkup-type class-id port-bitmap logic-port mapped-vlan metadata\n");
    for(idx =0; idx < lkup_num; idx++)
    {
        prop.acl_priority = idx;
        if(g_ctcs_api_en)
        {
            ret = ctcs_l3if_get_acl_property(g_api_lchip, l3if_id, &prop);
        }
        else
        {
            ret = ctc_l3if_get_acl_property(l3if_id, &prop);
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


CTC_CLI(ctc_cli_usw_l3if_set_property_ext,
        ctc_cli_usw_l3if_set_property_ext_cmd,
        "l3if ifid IFID ip-property (cid | public-lkup-en | mcast-vlan-en | label-en | phb-tunnel-en | trust-dscp |"
        "dscp-mode | ingress-dscp-domain | egress-dscp-domain | default-dscp) value VALUE",
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "IP property",
        "Category Id",
        "Public lookup enable",
        "Enable vlan of ipmc",
        "Enable interface lable",
        "Phb use tunnel hdr",
        "Trust dscp",
        "Dscp select mode, refer to ctc_dscp_select_mode_t",
        "Ingress qos dscp domain",
        "Egress qos dscp domain",
        "Default dscp",
        "Value",
        "Value of the property")
{
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop;
    uint32 value = 0;
    int32  ret = 0;

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "cid", 3))
    {
        l3if_prop = CTC_L3IF_PROP_CID;
    }
    else if(0 == sal_memcmp(argv[1], "pub", 3))
    {
        l3if_prop = CTC_L3IF_PROP_PUB_ROUTE_LKUP_EN;
    }
    else if(0 == sal_memcmp(argv[1], "mcast", 5))
    {
        l3if_prop = CTC_L3IF_PROP_IPMC_USE_VLAN_EN;
    }
    else if(0 == sal_memcmp(argv[1], "label", 5))
    {
        l3if_prop = CTC_L3IF_PROP_INTERFACE_LABEL_EN;
    }
    else if(0 == sal_memcmp(argv[1], "phb", 3))
    {
        l3if_prop = CTC_L3IF_PROP_PHB_USE_TUNNEL_HDR;
    }
    else if(0 == sal_memcmp(argv[1], "trust", 5))
    {
        l3if_prop = CTC_L3IF_PROP_TRUST_DSCP;
    }
    else if(0 == sal_memcmp(argv[1], "dscp", 4))
    {
        l3if_prop = CTC_L3IF_PROP_DSCP_SELECT_MODE;
    }
    else if(0 == sal_memcmp(argv[1], "ingress", 7))
    {
        l3if_prop = CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN;
    }
    else if(0 == sal_memcmp(argv[1], "egress", 6))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN;
    }
    else if(0 == sal_memcmp(argv[1], "default", 7))
    {
        l3if_prop = CTC_L3IF_PROP_DEFAULT_DSCP;
    }
    else
    {
        ctc_cli_out("%% %s \n", "Unrecognized command");
        return CLI_ERROR;
    }

    CTC_CLI_GET_UINT32_RANGE("ip-property", value, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3if_set_property(g_api_lchip, l3if_id, l3if_prop, value);
    }
    else
    {
        ret = ctc_l3if_set_property(l3if_id, l3if_prop, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l3if_show_property_ext,
        ctc_cli_usw_l3if_show_property_ext_cmd,
        "show l3if ifid IFID ip-property (cid | public-lkup-en | mcast-vlan-en | label-en | phb-tunnel-en | trust-dscp |"
        "dscp-mode | ingress-dscp-domain | egress-dscp-domain | default-dscp)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        CTC_CLI_L3IF_ID_DESC,
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "IP property",
        "Category Id",
        "Public lookup enable",
        "Enable vlan of ipmc",
        "Enable interface lable",
        "Phb use tunnel hdr",
        "Trust dscp",
        "Dscp select mode, refer to ctc_dscp_select_mode_t",
        "Ingress qos dscp domain",
        "Egress qos dscp domain",
        "Default dscp")
{
    uint16 l3if_id = 0;
    ctc_l3if_property_t l3if_prop;
    uint32 value = 0;
    int32  ret = 0;
    bool print_head = FALSE;
    char s[50], f[50];

    CTC_CLI_GET_UINT16_RANGE("ifid", l3if_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if (0 == sal_memcmp(argv[1], "cid", 3))
    {
        l3if_prop = CTC_L3IF_PROP_CID;
    }
    else if(0 == sal_memcmp(argv[1], "pub", 3))
    {
        l3if_prop = CTC_L3IF_PROP_PUB_ROUTE_LKUP_EN;
    }
    else if(0 == sal_memcmp(argv[1], "mcast", 5))
    {
        l3if_prop = CTC_L3IF_PROP_IPMC_USE_VLAN_EN;
    }
    else if(0 == sal_memcmp(argv[1], "label", 5))
    {
        l3if_prop = CTC_L3IF_PROP_INTERFACE_LABEL_EN;
    }
    else if(0 == sal_memcmp(argv[1], "phb", 3))
    {
        l3if_prop = CTC_L3IF_PROP_PHB_USE_TUNNEL_HDR;
    }
    else if(0 == sal_memcmp(argv[1], "trust", 5))
    {
        l3if_prop = CTC_L3IF_PROP_TRUST_DSCP;
    }
    else if(0 == sal_memcmp(argv[1], "dscp", 4))
    {
        l3if_prop = CTC_L3IF_PROP_DSCP_SELECT_MODE;
    }
    else if(0 == sal_memcmp(argv[1], "ingress", 7))
    {
        l3if_prop = CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN;
    }
    else if(0 == sal_memcmp(argv[1], "egress", 6))
    {
        l3if_prop = CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN;
    }
    else if(0 == sal_memcmp(argv[1], "default", 7))
    {
        l3if_prop = CTC_L3IF_PROP_DEFAULT_DSCP;
    }
    else
    {
        ctc_cli_out("%% %s \n", "Unrecognized command");
        return CLI_ERROR;
    }

    if (MAX_CTC_L3IF_PROP_NUM != l3if_prop)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_l3if_get_property(g_api_lchip, l3if_id, l3if_prop, &value);
        }
        else
        {
            ret = ctc_l3if_get_property(l3if_id, l3if_prop, &value);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        if (!print_head)
        {
            ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, "Property", "Value");
            ctc_cli_out("-----------------------------------------\n");
            print_head = TRUE;
        }
        ctc_cli_out(L3IF_CLI_SHOW_PROPERTY_FORMATE, prop_str[l3if_prop], CTC_DEBUG_HEX_FORMAT(s, f, value, 0, L));
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_l3if_show_status,
        ctc_cli_usw_l3if_show_status_cmd,
        "show l3if status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L3IF_STR,
        "Show L3if status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 lchip = 0;
    int32  ret = 0;
    uint8 index = 0;
    
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_l3if_state_show(lchip);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

int32
ctc_usw_l3if_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_ecmp_if_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_set_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_set_property_ext_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_vrf_statsid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_property_ext_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_l3if_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_ecmp_if_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_set_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_acl_property_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_set_property_ext_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_vrf_statsid_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_l3if_show_property_ext_cmd);

    return CLI_SUCCESS;
}

