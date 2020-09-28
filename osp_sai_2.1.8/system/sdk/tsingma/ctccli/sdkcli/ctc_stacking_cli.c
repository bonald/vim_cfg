/**
 @file ctc_stacking_cli.c

 @date 2010-7-9

 @version v2.0

---file comments----
*/

#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"

#include "ctc_stacking_cli.h"

CTC_CLI(ctc_cli_stacking_create_trunk,
        ctc_cli_stacking_create_trunk_cmd,
        "stacking create trunk TRUNK_ID (max-mem-cnt COUNT|hash-offset OFFSET|)(encap {layer2 mac-da MACDA mac-sa MACSA (vlan-id VLAN|) | (ipv4 ip-da A.B.C.D| ipv6 ip-da X:X::X:X) } |) ((dlb|failover)|)(mem-ascend-order|) (gchip GCHIP|)",
        "Stacking",
        "Create",
        "Trunk",
        "Trunk id",
        "Max member count",
        "Count value",
        "Hash offset",
        "Value",
        "Encap header",
        "Layer2 header",
        "MAC destination address",
        "MAC address in HHHH.HHHH.HHHH format",
        "MAC source address",
        "MAC address in HHHH.HHHH.HHHH format",
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "Ipv4 header",
        "IPv4 destination address",
        "IPv4 destination address in format of A.B.C.D",
        "Ipv6 header",
        "IPv6 destination address",
        "IPv6 destination address in format of X:X::X:X",
        "Dynamic load balance mode",
        "Failover mode",
        "Member port is in order",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_trunk_t stacking_trunk;
    uint8 idx = 0;
    ipv6_addr_t ipv6_address;
    uint8 load_mode = 0;

    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));

    CTC_CLI_GET_UINT8_RANGE("trunk id", stacking_trunk.trunk_id, argv[0], 0, CTC_MAX_UINT8_VALUE);


    /* mac destination address*/
    idx = CTC_CLI_GET_ARGC_INDEX("max-mem-cnt");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("max mem cnt", stacking_trunk.max_mem_cnt, argv[idx + 1]);
    }


	idx = CTC_CLI_GET_ARGC_INDEX("hash-offset");
    if(idx != 0xFF)
    {
        CTC_CLI_GET_UINT16("hash-offset", stacking_trunk.lb_hash_offset, argv[idx+1]);
		stacking_trunk.flag |= CTC_STACKING_TRUNK_LB_HASH_OFFSET_VALID;
    }


    stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_NONE;

    /* mac destination address*/
    idx = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != idx)
    {
        stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_L2;
        CTC_CLI_GET_MAC_ADDRESS("destination mac address", stacking_trunk.encap_hdr.mac_da, argv[idx + 1]);
    }

    /* mac source address*/
    idx = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (0xFF != idx)
    {
        CTC_CLI_GET_MAC_ADDRESS("source mac address", stacking_trunk.encap_hdr.mac_sa, argv[idx + 1]);
    }

    /* vlan-id*/
    idx = CTC_CLI_GET_ARGC_INDEX("vlan-id");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan-id", stacking_trunk.encap_hdr.vlan_id, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);

        stacking_trunk.encap_hdr.vlan_valid = 1;
    }

    /* ipv4 da*/
    idx = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (0xFF != idx)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", stacking_trunk.encap_hdr.ipda.ipv4, argv[idx + 2]);
        if (CTC_STK_HDR_WITH_L2 == stacking_trunk.encap_hdr.hdr_flag)
        {
            stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_L2_AND_IPV4;
        }
        else
        {
            stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_IPV4;
        }
    }

    /* ipv6 da*/
    idx = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != idx)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6 destination address", ipv6_address, argv[idx + 2]);
        /* adjust endian */
        stacking_trunk.encap_hdr.ipda.ipv6[0] = sal_htonl(ipv6_address[0]);
        stacking_trunk.encap_hdr.ipda.ipv6[1] = sal_htonl(ipv6_address[1]);
        stacking_trunk.encap_hdr.ipda.ipv6[2] = sal_htonl(ipv6_address[2]);
        stacking_trunk.encap_hdr.ipda.ipv6[3] = sal_htonl(ipv6_address[3]);
        if (CTC_STK_HDR_WITH_L2 == stacking_trunk.encap_hdr.hdr_flag)
        {
            stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_L2_AND_IPV6;
        }
        else
        {
            stacking_trunk.encap_hdr.hdr_flag = CTC_STK_HDR_WITH_IPV6;
        }
    }

    /* trunk mode */
    idx = CTC_CLI_GET_ARGC_INDEX("dlb");
    if (0xFF != idx)
    {
        load_mode = CTC_STK_LOAD_DYNAMIC;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("failover");
    if (0xFF != idx)
    {
        load_mode = CTC_STK_LOAD_STATIC_FAILOVER;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("mem-ascend-order");
    if (0xFF != idx)
    {
        CTC_SET_FLAG(stacking_trunk.flag, CTC_STACKING_TRUNK_MEM_ASCEND_ORDER);
    }

    stacking_trunk.load_mode = load_mode;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_create_trunk(g_api_lchip, &stacking_trunk);
    }
    else
    {
        ret = ctc_stacking_create_trunk(&stacking_trunk);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_remove_trunk,
        ctc_cli_stacking_remove_trunk_cmd,
        "stacking destroy trunk TRUNK_ID (gchip GCHIP|)",
        "Stacking",
        "Destroy",
        "Trunk",
        "Trunk id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    ctc_stacking_trunk_t stacking_trunk;


    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));

    CTC_CLI_GET_UINT8_RANGE("trunk id", stacking_trunk.trunk_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_destroy_trunk(g_api_lchip, &stacking_trunk);
    }
    else
    {
        ret = ctc_stacking_destroy_trunk(&stacking_trunk);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_add_remove_trunk_port,
        ctc_cli_stacking_add_remove_trunk_port_cmd,
        "stacking trunk TRUNK_ID (add|remove) port GPORT (gchip GCHIP|)",
        "Stacking",
        "Trunk",
        "Trunk id",
        "Add",
        "Remove",
        "Trunk port",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    bool is_add = FALSE;
    uint8 idx = 0;
    ctc_stacking_trunk_t stacking_trunk;

    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));

    CTC_CLI_GET_UINT8_RANGE("trunk id", stacking_trunk.trunk_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if (CLI_CLI_STR_EQUAL("add", 1))
    {
        is_add = TRUE;
    }
    else
    {
        is_add = FALSE;
    }

    CTC_CLI_GET_UINT16_RANGE("gport", stacking_trunk.gport, argv[2], 0, CTC_MAX_UINT16_VALUE);

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[idx+1]);
    }

    if (is_add)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_stacking_add_trunk_port(g_api_lchip, &stacking_trunk);
        }
        else
        {
            ret = ctc_stacking_add_trunk_port(&stacking_trunk);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_stacking_remove_trunk_port(g_api_lchip, &stacking_trunk);
        }
        else
        {
            ret = ctc_stacking_remove_trunk_port(&stacking_trunk);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
#define CTC_CLI_STACKING_REPLACE_MEMBER_MAX 255
CTC_CLI(ctc_cli_stacking_trunk_replace_ports,
        ctc_cli_stacking_trunk_replace_ports_cmd,
        "stacking trunk TRUNK_ID replace {member-port GPORT | end} (gchip GCHIP|)",
        "Stacking",
        "Trunk",
        "Trunk ID",
        "Replace member ports",
        "Member-port, global port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Input over, and replace ports",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    static uint8 tid = 0xFF;
    static uint16 mem_num = 0;
    static uint32 mem_port[CTC_CLI_STACKING_REPLACE_MEMBER_MAX] = {0};
    uint8  cur_tid = 0xFF;
    uint8 index = 0;
    uint8 is_end = 0;
    ctc_stacking_trunk_t stacking_trunk;

    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));
    CTC_CLI_GET_UINT8("trunk id", cur_tid, argv[0]);

    if ((cur_tid != tid) && (tid != 0xFF))
    {
        sal_memset(mem_port, 0, sizeof(mem_port));
        mem_num = 0;
        tid = 0xFF;
        ret = CTC_E_INVALID_PARAM;
        ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    tid = cur_tid;

    index = CTC_CLI_GET_ARGC_INDEX("member-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("gport", mem_port[mem_num], argv[index+1]);
        mem_num++;
    }

    index = CTC_CLI_GET_ARGC_INDEX("end");
    if (0xFF != index)
    {
        is_end = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != index)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[index+1]);
    }

    if (is_end)
    {
        ctc_stacking_trunk_t stk_trunk;
        sal_memset(&stk_trunk, 0, sizeof(stk_trunk));
        stk_trunk.trunk_id = tid;
        if (g_ctcs_api_en)
        {
            ret = ctcs_stacking_replace_trunk_ports(g_api_lchip, &stk_trunk, mem_port, mem_num);
        }
        else
        {
            ret = ctc_stacking_replace_trunk_ports(&stk_trunk, mem_port, mem_num);
        }

        sal_memset(mem_port, 0, sizeof(mem_port));
        mem_num = 0;
        tid = 0xFF;
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    else
    {
        if (mem_num > CTC_CLI_STACKING_REPLACE_MEMBER_MAX)
        {
            sal_memset(mem_port, 0, sizeof(mem_port));
            mem_num = 0;
            tid = CTC_MAX_LINKAGG_GROUP_NUM;
            ret = CTC_E_INVALID_PARAM;
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_stacking_add_remove_trunk_rchip,
        ctc_cli_stacking_add_remove_trunk_rchip_cmd,
        "stacking trunk TRUNK_ID (add|remove) (src-port PORT|) (remote-chip RCHIP) (gchip GCHIP|)",
        "Stacking",
        "Trunk",
        "Trunk id",
        "Add",
        "Remove",
        "Source port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Remote chip",
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    bool is_add = FALSE;
    ctc_stacking_trunk_t stacking_trunk;
    uint8 idx = 0;

    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));

    CTC_CLI_GET_UINT8_RANGE("trunk id", stacking_trunk.trunk_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if (CLI_CLI_STR_EQUAL("add", 1))
    {
        is_add = TRUE;
    }
    else
    {
        is_add = FALSE;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32_RANGE("Src-port", stacking_trunk.src_gport, argv[idx+1], 0, CTC_MAX_UINT32_VALUE);
		stacking_trunk.select_mode = 1;
    }


    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("Remote chip", stacking_trunk.remote_gchip, argv[idx+1], 0, CTC_MAX_UINT8_VALUE);
    }


    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[idx+1]);
    }

    if (is_add)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_stacking_add_trunk_rchip(g_api_lchip, &stacking_trunk);
        }
        else
        {
            ret = ctc_stacking_add_trunk_rchip(&stacking_trunk);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_stacking_remove_trunk_rchip(g_api_lchip, &stacking_trunk);
        }
        else
        {
            ret = ctc_stacking_remove_trunk_rchip(&stacking_trunk);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_stacking_show_trunk_rchip,
        ctc_cli_stacking_show_trunk_rchip_cmd,
        "show stacking trunk-id  (src-port PORT|) (remote-chip RCHIP)  (gchip GCHIP|)",
        "Show",
        "Stacking",
        "Trunk id",
        "Source port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Remote chip",
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    ctc_stacking_trunk_t stacking_trunk;

    sal_memset(&stacking_trunk, 0, sizeof(ctc_stacking_trunk_t));


    idx = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32_RANGE("Src-port", stacking_trunk.src_gport, argv[idx+1], 0, CTC_MAX_UINT32_VALUE);
		stacking_trunk.select_mode = 1;
    }


    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("Remote chip", stacking_trunk.remote_gchip, argv[idx+1], 0, CTC_MAX_UINT8_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_trunk.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_trunk.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_stacking_get_trunk_rchip(g_api_lchip, &stacking_trunk);
    }
    else
    {
        ret = ctc_stacking_get_trunk_rchip(&stacking_trunk);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Remote chip:%2d  Trunk id:%2d\n", stacking_trunk.remote_gchip, stacking_trunk.trunk_id);

    return ret;

}


uint32 stacking_end_chip_bitmap = 0;

CTC_CLI(ctc_cli_stacking_set_end_rchip_bitmap,
        ctc_cli_stacking_set_end_rchip_bitmap_cmd,
        "stacking (add|remove) (end-chip-point GCHIP | break-point TRUNK_ID) (gchip GCHIP|)",
        "Stacking",
        "Add",
        "Remove",
        "End the remote chip id",
        CTC_CLI_GCHIP_ID_DESC,
        "Break point for ring",
        "Trunk id <1-63>",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 gchip_id;
    bool is_add = FALSE;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_stop_rchip_t stop_rchip;
    ctc_stacking_mcast_break_point_t break_point;
    uint8 idx = 0;


    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&stop_rchip, 0, sizeof(ctc_stacking_stop_rchip_t));

    if (CLI_CLI_STR_EQUAL("add", 0))
    {
        is_add = TRUE;
    }
    else
    {
        is_add = FALSE;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("end-chip-point");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("end-bitmap", gchip_id, argv[idx+1], 0, CTC_MAX_UINT8_VALUE);
        if (gchip_id > 29)
        {
            ctc_cli_out("%% Invalid global chip id, chip id must be <0-29>\n");
            return CLI_ERROR;
        }

        if (is_add)
        {
            CTC_BIT_SET(stacking_end_chip_bitmap, gchip_id);
        }
        else
        {
            CTC_BIT_UNSET(stacking_end_chip_bitmap, gchip_id);
        }

        stop_rchip.rchip_bitmap = stacking_end_chip_bitmap;

        stacking_prop.prop_type = CTC_STK_PROP_MCAST_STOP_RCHIP;
        stacking_prop.p_value = &stop_rchip;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("break-point");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("Break Point", break_point.trunk_id, argv[idx+1], 0, CTC_MAX_UINT8_VALUE);

        break_point.enable = (TRUE == is_add) ? 1:0;
        stacking_prop.prop_type = CTC_STK_PROP_MCAST_BREAK_POINT;
        stacking_prop.p_value = &break_point;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}




CTC_CLI(ctc_cli_stacking_set_break_en,
        ctc_cli_stacking_set_break_en_cmd,
        "stacking (break|full-mesh|port-en (port GPORT) (direction DIR)) (enable|disable) (hdr-type TYPE |) (gchip GCHIP|)",
        "Stacking",
        "Break if exist break point",
        "Full mesh topo",
        "Stacking port enable",
        "Gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Direction",
        "0:Ingress, 1:Egress, 2:Both",
        "Enable",
        "Disable",
        "Hdr type",
        "0:None, 1:L2",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    uint32 idx = 0;
    ctc_stacking_port_cfg_t stk_port;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));

    if (CLI_CLI_STR_EQUAL("break", 0))
    {
        stacking_prop.prop_type = CTC_STK_PROP_BREAK_EN;
    }
    else
    {
        stacking_prop.prop_type = CTC_STK_PROP_FULL_MESH_EN;
    }

    if (CLI_CLI_STR_EQUAL("port-en", 0))
    {
        sal_memset(&stk_port, 0, sizeof(ctc_stacking_port_cfg_t));
        stacking_prop.prop_type = CTC_STK_PROP_STK_PORT_EN;

        idx = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("gport", stk_port.gport, argv[idx + 1]);
        }

        idx = CTC_CLI_GET_ARGC_INDEX("enable");
        if (0xFF != idx)
        {
            stk_port.enable = 1;
        }

        idx = CTC_CLI_GET_ARGC_INDEX("disable");
        if (0xFF != idx)
        {
            stk_port.enable = 0;
        }

        idx = CTC_CLI_GET_ARGC_INDEX("direction");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("dir", stk_port.direction, argv[idx + 1]);
        }

        idx = CTC_CLI_GET_ARGC_INDEX("hdr-type");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("hdr type", stk_port.hdr_type, argv[idx + 1]);
        }
        stacking_prop.p_value = &stk_port;
    }
    if (CLI_CLI_STR_EQUAL("en", 1))
    {
        stacking_prop.value = 1;
    }
    else
    {
        stacking_prop.value = 0;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_show_property,
        ctc_cli_stacking_show_property_cmd,
        "show stacking (break-en | end-chip-bitmap| full-mesh-en|port-en (port GPORT) (direction DIR)|) (gchip GCHIP|)",
        "Show",
        "Stacking",
        "Break enable",
        "End chip bitmap",
        "Full mesh topo enable",
        "Stacking port enable",
        "Gport",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Direction",
        "0:Ingress, 1:Egress, 2:Both",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_stop_rchip_t stop_rchip;
    ctc_stacking_port_cfg_t stk_port;
    uint32 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));


    if (CLI_CLI_STR_EQUAL("break-en", 0))
    {
        stacking_prop.prop_type = CTC_STK_PROP_BREAK_EN;
    }
    else if (CLI_CLI_STR_EQUAL("full-mesh-en", 0))
    {
        stacking_prop.prop_type = CTC_STK_PROP_FULL_MESH_EN;
    }
    else if (CLI_CLI_STR_EQUAL("port-en", 0))
    {
        sal_memset(&stk_port, 0, sizeof(ctc_stacking_port_cfg_t));
        stacking_prop.prop_type = CTC_STK_PROP_STK_PORT_EN;

        idx = CTC_CLI_GET_ARGC_INDEX("port");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT32("gport", stk_port.gport, argv[idx + 1]);
        }

        idx = CTC_CLI_GET_ARGC_INDEX("direction");
        if (0xFF != idx)
        {
            CTC_CLI_GET_UINT8("dir", stk_port.direction, argv[idx + 1]);
        }
        stacking_prop.p_value = &stk_port;
    }
    else
    {
        stacking_prop.prop_type = CTC_STK_PROP_MCAST_STOP_RCHIP;
        sal_memset(&stop_rchip, 0, sizeof(ctc_stacking_stop_rchip_t));
        stacking_prop.p_value = &stop_rchip;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_get_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("stacking property info below\n");
    ctc_cli_out("------------------------------------\n");

    switch (stacking_prop.prop_type)
    {
    case  CTC_STK_PROP_BREAK_EN:
        ctc_cli_out("stacking break-en:%d\n", stacking_prop.value);
        break;

    case  CTC_STK_PROP_MCAST_STOP_RCHIP:

        ctc_cli_out("stacking end-chip-bitmap:0x%x\n", stop_rchip.rchip_bitmap);
        break;

    case  CTC_STK_PROP_FULL_MESH_EN:
        ctc_cli_out("stacking full-mesh-en:%d\n", stacking_prop.value);
        break;
    case  CTC_STK_PROP_STK_PORT_EN:
        ctc_cli_out("stacking port-en:%d, hdr-type:%d\n", stk_port.enable, stk_port.hdr_type);
        break;
    default:
        return CLI_SUCCESS;
    }

    return CLI_SUCCESS;

}



CTC_CLI(ctc_cli_stacking_bind_trunk,
        ctc_cli_stacking_bind_trunk_cmd,
        "stacking bind-trunk port GPORT trunk TRUNKID (gchip GCHIP|)",
        "Stacking",
        "Bind trunk with trunk port",
        "Trunk port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Trunk",
        "Trunk id",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_bind_trunk_t bind;
    uint32 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&bind, 0, sizeof(ctc_stacking_bind_trunk_t));

    stacking_prop.prop_type = CTC_STK_PROP_PORT_BIND_TRUNK;

    CTC_CLI_GET_UINT32("port", bind.gport, argv[0]);
    CTC_CLI_GET_UINT8("trunk-id", bind.trunk_id, argv[1]);

    stacking_prop.p_value = &bind;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_show_bind_trunk,
        ctc_cli_stacking_show_bind_trunk_cmd,
        "show stacking bind-trunk port GPORT (gchip GCHIP|)",
        CTC_CLI_SHOW_STR,
        "Stacking",
        "Bind trunk with trunk port",
        "Trunk port",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_bind_trunk_t bind;
    uint32 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&bind, 0, sizeof(ctc_stacking_bind_trunk_t));

    stacking_prop.prop_type = CTC_STK_PROP_PORT_BIND_TRUNK;

    CTC_CLI_GET_UINT32("port", bind.gport, argv[0]);

    stacking_prop.p_value = &bind;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_get_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("port 0x%04x bind-trunk: %d\n", bind.gport, bind.trunk_id);

    return ret;

}



CTC_CLI(ctc_cli_stacking_set_header,
        ctc_cli_stacking_set_header_cmd,
        "stacking header {layer2 vlan-tpid TPID cos COS ether-type ETHTYPE (mac-da-check-en|) (ethtype-check-en|) | \
                      layer3 (ipv4 ip-sa A.B.C.D| ipv6 ip-sa X:X::X:X) ttl TTL dscp DSCP ip-protocol PROTOCOL| \
                      layer4 (udp src-port SRCPORT dest-port DESTPORT )} (gchip GCHIP|)",
        "Stacking",
        "Header",
        "Layer2 header",
        "Vlan tpid such as <0x8100>",
        "Value <0-0xFFFF>",
        "Cos",
        "Value <0-7>",
        "Ether type",
        "Value <0-0xFFFF>",
        "Mac da check enable when receive stacking header",
        "Ether type check enable when receive stacking header",
        "Layer3 header",
        "Ipv4 header",
        "IPv4 source address",
        "IPv4 source address in format of A.B.C.D",
        "Ipv6 header",
        "IPv6 source address",
        "IPv6 source address in format of X:X::X:X",
        "Ttl",
        "Value <0-255>",
        "Dscp",
        "Value <0-63>",
        "Ip protocol",
        "Value <0-255>",
        "Layer4 header",
        "Udp",
        "Udp source port",
        "Value <0-0xFFFF>",
        "Udp dest port",
        "Value <0-0xFFFF>",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;

    ctc_stacking_property_t stacking_prop;
    ctc_stacking_glb_cfg_t glb_cfg;

    ipv6_addr_t ipv6_address;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&glb_cfg, 0, sizeof(ctc_stacking_glb_cfg_t));

    stacking_prop.p_value = &glb_cfg;

    idx = CTC_CLI_GET_ARGC_INDEX("vlan-tpid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("vlan-tpid", glb_cfg.hdr_glb.vlan_tpid, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("cos");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("cos", glb_cfg.hdr_glb.cos, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("ether-type");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("ether-type", glb_cfg.hdr_glb.ether_type, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("mac-da-check-en");
    if (0xFF != idx)
    {
        glb_cfg.hdr_glb.mac_da_chk_en = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("ethtype-check-en");
    if (0xFF != idx)
    {
        glb_cfg.hdr_glb.ether_type_chk_en = 1;
    }

    /* ipv4 da*/
    idx = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (0xFF != idx)
    {
        glb_cfg.hdr_glb.is_ipv4 = 1;
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", glb_cfg.hdr_glb.ipsa.ipv4, argv[idx + 2]);
    }

    /* ipv6 da*/
    idx = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != idx)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6 destination address", ipv6_address, argv[idx + 2]);
        /* adjust endian */
        glb_cfg.hdr_glb.ipsa.ipv6[0] = sal_htonl(ipv6_address[0]);
        glb_cfg.hdr_glb.ipsa.ipv6[1] = sal_htonl(ipv6_address[1]);
        glb_cfg.hdr_glb.ipsa.ipv6[2] = sal_htonl(ipv6_address[2]);
        glb_cfg.hdr_glb.ipsa.ipv6[3] = sal_htonl(ipv6_address[3]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", glb_cfg.hdr_glb.ip_ttl, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", glb_cfg.hdr_glb.ip_dscp, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8_RANGE("ip-protocol", glb_cfg.hdr_glb.ip_protocol, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("udp");
    if (0xFF != idx)
    {
        glb_cfg.hdr_glb.udp_en = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("L4port", glb_cfg.hdr_glb.udp_src_port, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("dest-port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("L4port", glb_cfg.hdr_glb.udp_dest_port, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    stacking_prop.prop_type = CTC_STK_PROP_GLOBAL_CONFIG;
    stacking_prop.p_value = &glb_cfg;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_show_header,
        ctc_cli_stacking_show_header_cmd,
        "show stacking header info (gchip GCHIP|)",
        "Show",
        "Stacking",
        "Header",
        "Infomation",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint32 tempip = 0;

    ctc_stacking_property_t stacking_prop;
    ctc_stacking_glb_cfg_t glb_cfg;
    uint32 ipv6_addr[4] = {0};
    char buf[CTC_IPV6_ADDR_STR_LEN] = {0};
    uint32 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&glb_cfg, 0, sizeof(ctc_stacking_glb_cfg_t));

    stacking_prop.prop_type = CTC_STK_PROP_GLOBAL_CONFIG;
    stacking_prop.p_value = &glb_cfg;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_get_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Stacking header global configure:\n");

    ctc_cli_out("\n----------------------------------------\n");
    ctc_cli_out("Layer2 info      :\n");
    ctc_cli_out("----------------------------------------\n");
    ctc_cli_out("vlan-tpid        :0x%x\n", glb_cfg.hdr_glb.vlan_tpid);
    ctc_cli_out("cos              :%d\n", glb_cfg.hdr_glb.cos);
    ctc_cli_out("ether-type       :0x%x\n", glb_cfg.hdr_glb.ether_type);
    ctc_cli_out("mac-da-check-en  :%d\n", glb_cfg.hdr_glb.mac_da_chk_en);
    ctc_cli_out("ethtype-check-en :%d\n", glb_cfg.hdr_glb.ether_type_chk_en);

    ctc_cli_out("\n----------------------------------------\n");
    ctc_cli_out("Layer3 info      :\n");
    ctc_cli_out("----------------------------------------\n");
    if (glb_cfg.hdr_glb.is_ipv4)
    {
        tempip = sal_ntohl(glb_cfg.hdr_glb.ipsa.ipv4);
        sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
        ctc_cli_out("ipv4-sa          :%-20s\n", buf);
    }
    else
    {
                ipv6_addr[0] = sal_htonl(glb_cfg.hdr_glb.ipsa.ipv6[0]);
                ipv6_addr[1] = sal_htonl(glb_cfg.hdr_glb.ipsa.ipv6[1]);
                ipv6_addr[2] = sal_htonl(glb_cfg.hdr_glb.ipsa.ipv6[2]);
                ipv6_addr[3] = sal_htonl(glb_cfg.hdr_glb.ipsa.ipv6[3]);
                sal_inet_ntop(AF_INET6, ipv6_addr, buf, CTC_IPV6_ADDR_STR_LEN);
                ctc_cli_out("ipv6-sa          :%-20s \n", buf);
    }

    ctc_cli_out("ip-ttl           :%d\n", glb_cfg.hdr_glb.ip_ttl);
    ctc_cli_out("ip-dscp          :%d\n", glb_cfg.hdr_glb.ip_dscp);
    ctc_cli_out("ip-protocol      :%d\n", glb_cfg.hdr_glb.ip_protocol);

    ctc_cli_out("\n----------------------------------------\n");
    ctc_cli_out("Layer4 info      :\n");
    ctc_cli_out("----------------------------------------\n");
    ctc_cli_out("udp-en           :%d\n", glb_cfg.hdr_glb.udp_en);
    ctc_cli_out("udp-src-port     :0x%x\n", glb_cfg.hdr_glb.udp_src_port);
    ctc_cli_out("udp-dest-port    :0x%x\n", glb_cfg.hdr_glb.udp_dest_port);

    return ret;

}

CTC_CLI(ctc_cli_stacking_create_keeplive,
        ctc_cli_stacking_create_keeplive_cmd,
        "stacking keeplive-group (create|destroy) group GROUP_ID",
        "Stacking",
        "KeepLive group for cpu-to-cpu",
        "Create group",
        "Destroy group",
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    bool is_create = FALSE;
    uint16 group_id = 0;

    if (CLI_CLI_STR_EQUAL("create", 0))
    {
        is_create = TRUE;
    }
    else
    {
        is_create = FALSE;
    }

    CTC_CLI_GET_UINT16_RANGE("group id", group_id, argv[1], 0, CTC_MAX_UINT16_VALUE);


    if (is_create)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_stacking_create_keeplive_group(g_api_lchip, group_id);

        }
        else
        {
            ret = ctc_stacking_create_keeplive_group(group_id);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_stacking_destroy_keeplive_group(g_api_lchip, group_id);

        }
        else
        {
            ret = ctc_stacking_destroy_keeplive_group(group_id);

        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_stacking_set_keeplive,
        ctc_cli_stacking_set_keeplive_cmd,
        "stacking keeplive-group (group GROUP_ID|) (add|remove) (trunk TRUNK_ID | {tbmp0 BMP0 | tbmp1 BMP1} | cpu-port GPORT) (gchip GCHIP|)",
        "Stacking",
        "KeepLive group for cpu-to-cpu",
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Add group member",
        "Remove group member",
        "Trunk id",
        "Trunk id",
        "Trunk bitmap",
        "Trunk bitmap value",
        "Trunk bitmap",
        "Trunk bitmap value",
        "Member port(Cpu port)",
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    bool is_add = FALSE;
    ctc_stacking_keeplive_t keeplive;

    sal_memset(&keeplive, 0, sizeof(ctc_stacking_keeplive_t));

    idx = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != idx)
    {
        is_add = TRUE;
    }
    else
    {
        is_add = FALSE;
    }

    /* trunk id*/
    idx = CTC_CLI_GET_ARGC_INDEX("trunk");
    if (0xFF != idx)
    {
        keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_TRUNK;
        CTC_CLI_GET_UINT8_RANGE("trunk id", keeplive.trunk_id, argv[idx + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    /* trunk bitmap*/
    idx = CTC_CLI_GET_ARGC_INDEX("tbmp0");
    if (0xFF != idx)
    {
        keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_TRUNK;
        keeplive.trunk_bmp_en = 1;
        CTC_CLI_GET_UINT32_RANGE("trunk bitmap 0", keeplive.trunk_bitmap[0], argv[idx + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("tbmp1");
    if (0xFF != idx)
    {
        keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_TRUNK;
        keeplive.trunk_bmp_en = 1;
        CTC_CLI_GET_UINT32_RANGE("trunk bitmap 1", keeplive.trunk_bitmap[1], argv[idx + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    /* gport*/
    idx = CTC_CLI_GET_ARGC_INDEX("cpu-port");
    if (0xFF != idx)
    {
        keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_PORT;
        CTC_CLI_GET_UINT32_RANGE("cpu port", keeplive.cpu_gport, argv[idx + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    /* group id*/
    idx = CTC_CLI_GET_ARGC_INDEX("group");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16_RANGE("group id", keeplive.group_id, argv[idx + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        keeplive.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", keeplive.extend.gchip, argv[idx + 1]);
    }

    if (is_add )
    {

        if (g_ctcs_api_en)
        {
            ret = ctcs_stacking_keeplive_add_member(g_api_lchip, &keeplive);

        }
        else
        {
            ret = ctc_stacking_keeplive_add_member(&keeplive);

        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_stacking_keeplive_remove_member(g_api_lchip, &keeplive);

        }
        else
        {
            ret = ctc_stacking_keeplive_remove_member(&keeplive);

        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_show_keeplive_members,
        ctc_cli_stacking_show_keeplive_members_cmd,
        "show stacking keeplive-group group GROUP_ID (trunk | cpu-port) (gchip GCHIP|)",
        "Show",
        "Stacking",
        "KeepLive group for cpu-to-cpu",
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Trunk",
        "Member port(Cpu port)",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    uint8 mem_cnt = 0;
    ctc_stacking_keeplive_t p_keeplive;

    sal_memset(&p_keeplive, 0, sizeof(ctc_stacking_keeplive_t));

    p_keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_TRUNK;

    CTC_CLI_GET_UINT16_RANGE("group id", p_keeplive.group_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    idx = CTC_CLI_GET_ARGC_INDEX("cpu-port");
    if (0xFF != idx)
    {
        p_keeplive.mem_type = CTC_STK_KEEPLIVE_MEMBER_PORT;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        p_keeplive.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", p_keeplive.extend.gchip, argv[idx + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_stacking_keeplive_get_members(g_api_lchip, &p_keeplive);

    }
    else
    {
        ret = ctc_stacking_keeplive_get_members(&p_keeplive);

    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Keeplive Group %d Members:\n", p_keeplive.group_id);
    ctc_cli_out("----------------------------------\n");

    if (p_keeplive.mem_type == CTC_STK_KEEPLIVE_MEMBER_PORT)
    {
        if (CTC_IS_CPU_PORT(p_keeplive.cpu_gport))
        {
            ctc_cli_out("CPU port: 0x%8x\n", p_keeplive.cpu_gport);
        }
        ctc_cli_out("\n");
        return CLI_SUCCESS;
    }

    ctc_cli_out("No.    Trunk-Id\n");
    ctc_cli_out("-------------------\n");

    for (idx = 0; idx < CTC_STK_TRUNK_BMP_NUM * CTC_UINT32_BITS; idx++)
    {
        if (CTC_IS_BIT_SET(p_keeplive.trunk_bitmap[idx / CTC_UINT32_BITS], idx % CTC_UINT32_BITS))
        {
            ctc_cli_out("%2d     %2d\n", mem_cnt, idx);
            mem_cnt++;
        }
    }
    ctc_cli_out("\n");
    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_stacking_create_mcast_profile,
        ctc_cli_stacking_create_mcast_profile_cmd,
        "stacking trunk-mcast-profile (create|destroy) (mcast-profile-id ID) (append-en|) (gchip GCHIP|)",
        "Stacking",
        "Trunk mcast group",
        "Create group",
        "Destroy group",
        "Mcast profile id",
        "Value",
        "Append to default stacking profile",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 idx = 0;
	uint16 profile_id = 0;
    ctc_stacking_trunk_mcast_profile_t mcast_profile;

    sal_memset(&mcast_profile, 0, sizeof(mcast_profile));

    idx = CTC_CLI_GET_ARGC_INDEX("create");

    if (0xFF != idx)
    {
        mcast_profile.type = CTC_STK_MCAST_PROFILE_CREATE;
    }
    else
    {
        mcast_profile.type = CTC_STK_MCAST_PROFILE_DESTROY;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("mcast-profile-id");

    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("mcast profile id", mcast_profile.mcast_profile_id, argv[idx + 1]);
		profile_id = mcast_profile.mcast_profile_id;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("append-en");

    if (0xFF != idx)
    {
        mcast_profile.append_en = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        mcast_profile.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", mcast_profile.extend.gchip, argv[idx + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_trunk_mcast_profile(g_api_lchip, &mcast_profile);

    }
    else
    {
        ret = ctc_stacking_set_trunk_mcast_profile(&mcast_profile);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (0 == profile_id &&  mcast_profile.type == CTC_STK_MCAST_PROFILE_CREATE)
    {
        ctc_cli_out("SDK alloc trunk mcast profile id:%d \n", mcast_profile.mcast_profile_id);
    }

    return ret;

}


CTC_CLI(ctc_cli_stacking_add_mcast_profile_member,
        ctc_cli_stacking_add_mcast_profile_member_cmd,
        "stacking trunk-mcast-profile (mcast-profile-id ID) (add|remove) {trunk-bitmap0 BITMAP |trunk-bitmap1 BITMAP} (gchip GCHIP|)",
        "Stacking",
        "Trunk mcast profile",
        "Profile id",
        "ID value",
        "Add",
        "Remove",
        "Trunk id bitmap0",
        "BITMAP",
         "Trunk id bitmap1",
        "BITMAP",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 idx = 0;
    ctc_stacking_trunk_mcast_profile_t mcast_profile;

    sal_memset(&mcast_profile, 0, sizeof(mcast_profile));


    idx = CTC_CLI_GET_ARGC_INDEX("mcast-profile-id");

    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("mcast profile id", mcast_profile.mcast_profile_id, argv[idx + 1]);
    }


    idx = CTC_CLI_GET_ARGC_INDEX("add");

    if (0xFF != idx)
    {
        mcast_profile.type = CTC_STK_MCAST_PROFILE_ADD;
    }
    else
    {
        mcast_profile.type = CTC_STK_MCAST_PROFILE_REMOVE;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("trunk-bitmap0");

    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("trunk bitmap0", mcast_profile.trunk_bitmap[0], argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("trunk-bitmap1");

    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("trunk bitmap1", mcast_profile.trunk_bitmap[1], argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        mcast_profile.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", mcast_profile.extend.gchip, argv[idx + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_trunk_mcast_profile(g_api_lchip, &mcast_profile);

    }
    else
    {
        ret = ctc_stacking_set_trunk_mcast_profile(&mcast_profile);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_stacking_show_mcast_profile,
        ctc_cli_stacking_show_mcast_profile_cmd,
        "show stacking trunk-mcast-profile (mcast-profile-id ID) (gchip GCHIP|)",
        CTC_CLI_SHOW_STR,
        "Stacking",
        "Trunk mcast profile",
        "Profile id",
        "ID value",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    uint16 idx = 0;
    ctc_stacking_trunk_mcast_profile_t mcast_profile;

    sal_memset(&mcast_profile, 0, sizeof(mcast_profile));


    idx = CTC_CLI_GET_ARGC_INDEX("mcast-profile-id");

    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("mcast profile id", mcast_profile.mcast_profile_id, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        mcast_profile.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", mcast_profile.extend.gchip, argv[idx + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_trunk_mcast_profile(g_api_lchip, &mcast_profile);

    }
    else
    {
        ret = ctc_stacking_get_trunk_mcast_profile(&mcast_profile);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    ctc_cli_out("stcking trunk mcast profile id:%d \n", mcast_profile.mcast_profile_id);
    ctc_cli_out("================================= \n");

    ctc_cli_out("bitmap0 :0x%x \n", mcast_profile.trunk_bitmap[0]);
    ctc_cli_out("bitmap1 :0x%x \n", mcast_profile.trunk_bitmap[1]);

    return ret;

}



CTC_CLI(ctc_cli_stacking_show_trunk_members,
        ctc_cli_stacking_show_trunk_members_cmd,
        "show stacking trunk trunk-id TRUNID  member (lchip LCHIP|)",
        "Show",
        "Stacking",
        "Trunk",
        "Trunk id",
        "Value <1-63>",
        "Member ports",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint8 idx = 0;
    uint8 trunk_id = 0;
    uint8 lchip = 0;
    uint32 p_gports[32];
    uint8 cnt = 0;

    /* trunk id*/
    CTC_CLI_GET_UINT8_RANGE("Trunkid", trunk_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    idx = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[idx + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_member_ports(g_api_lchip, trunk_id, p_gports, &cnt);
    }
    else
    {
        ret = ctc_stacking_get_member_ports(lchip, trunk_id, p_gports, &cnt);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Trunk %d members ports:\n", trunk_id);
    ctc_cli_out("=================================\n");

    for (idx = 0; idx < cnt; idx++)
    {
        ctc_cli_out("%2d 0x%04x\n", idx, p_gports[idx]);
    }

    return CLI_SUCCESS;
}



CTC_CLI(ctc_cli_stacking_set_rchip_port_en,
        ctc_cli_stacking_set_rchip_port_en_cmd,
        "stacking (remote-chip CHIP) enable {pbmp0 PORT_BITMAP_0 | pbmp1 PORT_BITMAP_1 | pbmp8 PORT_BITMAP_8 | pbmp9 PORT_BITMAP_9 } (gchip GCHIP|)",
        "Stacking",
        "Remote chip",
        CTC_CLI_GCHIP_ID_DESC,
        "Enable",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_rchip_port_t rchip_port;
    uint8 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&rchip_port, 0, sizeof(rchip_port));


    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Remote chip", rchip_port.rchip, argv[idx + 1]);
    }


    idx = CTC_CLI_GET_ARGC_INDEX("pbmp0");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 0", rchip_port.pbm[0], argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("pbmp1");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 1", rchip_port.pbm[1], argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pbmp8");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 8", rchip_port.pbm[8], argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("pbmp9");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 9", rchip_port.pbm[9], argv[idx + 1]);
    }

    stacking_prop.prop_type = CTC_STK_PROP_RCHIP_PORT_EN;
    stacking_prop.p_value = &rchip_port;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;


}

CTC_CLI(ctc_cli_stacking_show_rchip_port_en,
        ctc_cli_stacking_show_rchip_port_en_cmd,
        "show stacking (remote-chip CHIP) ports (gchip GCHIP|)",
        "Show",
        "Stacking",
        "Remote chip",
        CTC_CLI_GCHIP_ID_DESC,
        "Ports",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{

    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_rchip_port_t rchip_port;
    uint8 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&rchip_port, 0, sizeof(rchip_port));


    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Remote chip", rchip_port.rchip, argv[idx + 1]);
    }

    stacking_prop.prop_type = CTC_STK_PROP_RCHIP_PORT_EN;
    stacking_prop.p_value = &rchip_port;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_get_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Remote chip %d enable ports:\n", rchip_port.rchip);
    ctc_cli_out("=================================\n");

    for (idx = 0; idx < sizeof(rchip_port.pbm)/4; idx++)
    {
        ctc_cli_out("0x%08x\n", rchip_port.pbm[idx]);
    }


    return CLI_SUCCESS;
}





CTC_CLI(ctc_cli_stacking_discard_port_chip,
        ctc_cli_stacking_discard_port_chip_cmd,
        "stacking discard (port PORT|) (remote-chip GCHIP) (enable|disable) (gchip GCHIP|)",
        "Stacking",
        "Discard",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Remote chip id",
        CTC_CLI_GCHIP_ID_DESC,
        "Enable",
        "Disable",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_stop_rchip_t stop_rchip;
    uint8 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&stop_rchip, 0, sizeof(ctc_stacking_stop_rchip_t));


    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Remote chip", stop_rchip.rchip, argv[idx + 1]);
        stop_rchip.mode = CTC_STK_STOP_MODE_DISCARD_RCHIP;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Port", stop_rchip.src_gport, argv[idx + 1]);
        stop_rchip.mode = CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT;
    }


    idx = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != idx)
    {
        stop_rchip.discard = 1;
    }

    stacking_prop.prop_type = CTC_STK_PROP_MCAST_STOP_RCHIP;
    stacking_prop.p_value = &stop_rchip;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_stacking_block_port_chip,
        ctc_cli_stacking_block_port_chip_cmd,
        "stacking  block (port PORT) (remote-chip GCHIP) dest {pbmp0 PORT_BITMAP_0 | pbmp1 PORT_BITMAP_1} (gchip GCHIP|)",
        "Stacking",
        "Block",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Remote chip id",
        CTC_CLI_GCHIP_ID_DESC,
        "Destination",
        "Bitmap of ports",
        "Bitmap value",
        "Bitmap of ports",
        "Bitmap value",
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_stop_rchip_t stop_rchip;
    uint8 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&stop_rchip, 0, sizeof(ctc_stacking_stop_rchip_t));

    stop_rchip.mode = CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT;

    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Port", stop_rchip.src_gport, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Remote chip", stop_rchip.rchip, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pbmp0");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 0", stop_rchip.pbm[0], argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("pbmp1");
    if (idx != 0xFF)
    {
        CTC_CLI_GET_UINT32("port bitmap 1", stop_rchip.pbm[1], argv[idx + 1]);
    }

    stacking_prop.prop_type = CTC_STK_PROP_MCAST_STOP_RCHIP;
    stacking_prop.p_value = &stop_rchip;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_set_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_set_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}



CTC_CLI(ctc_cli_stacking_show_port_rchip_discard,
        ctc_cli_stacking_show_port_rchip_discard_cmd,
        "show stacking (discard|block) (port PORT|) (remote-chip GCHIP) (gchip GCHIP|)",
        "Show",
        "Stacking",
        "Discard",
        "Block",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Remote chip id",
        CTC_CLI_GCHIP_ID_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC)
{
    int32 ret = CLI_SUCCESS;
    ctc_stacking_property_t stacking_prop;
    ctc_stacking_stop_rchip_t stop_rchip;
    uint8 idx = 0;

    sal_memset(&stacking_prop, 0, sizeof(ctc_stacking_property_t));
    sal_memset(&stop_rchip, 0, sizeof(ctc_stacking_stop_rchip_t));


    idx = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != idx)
    {
        stop_rchip.mode = CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("block");
    if (0xFF != idx)
    {
        stop_rchip.mode = CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT;
    }


    idx = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Port", stop_rchip.src_gport, argv[idx + 1]);
    }
    else
    {
        stop_rchip.mode = CTC_STK_STOP_MODE_DISCARD_RCHIP;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("Remote chip", stop_rchip.rchip, argv[idx + 1]);
    }

    stacking_prop.prop_type = CTC_STK_PROP_MCAST_STOP_RCHIP;
    stacking_prop.p_value = &stop_rchip;

    idx = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != idx)
    {
        stacking_prop.extend.enable = 1;
        CTC_CLI_GET_UINT8("gchip", stacking_prop.extend.gchip, argv[idx+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_stacking_get_property(g_api_lchip, &stacking_prop);
    }
    else
    {
        ret = ctc_stacking_get_property(&stacking_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    if (CTC_STK_STOP_MODE_DISCARD_RCHIP_PORT == stop_rchip.mode ||
        CTC_STK_STOP_MODE_DISCARD_RCHIP == stop_rchip.mode )
    {
        ctc_cli_out("Discard: %d\n", stop_rchip.discard);
    }

    if (CTC_STK_STOP_MODE_BLOCK_RCHIP_PORT == stop_rchip.mode)
    {
        ctc_cli_out("pbm0: 0x%x\n", stop_rchip.pbm[0]);
        ctc_cli_out("pbm1: 0x%x\n", stop_rchip.pbm[1]);
    }


    return ret;

}




CTC_CLI(ctc_cli_stacking_debug_on,
        ctc_cli_stacking_debug_on_cmd,
        "debug stacking (ctc|sys)  (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "stacking",
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_NONE;
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
    else
    {
        level = CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = STK_CTC;
    }
    else
    {
        typeenum = STK_SYS;
    }

    ctc_debug_set_flag("stacking", "stacking", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stacking_debug_off,
        ctc_cli_stacking_debug_off_cmd,
        "no debug stacking (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "stacking",
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = STK_CTC;
    }
    else
    {
        typeenum = STK_SYS;
    }

    ctc_debug_set_flag("stacking", "stacking", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

int32
ctc_stacking_cli_init(void)
{
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_create_trunk_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_remove_trunk_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_add_remove_trunk_port_cmd);
	install_element(CTC_SDK_MODE,  &ctc_cli_stacking_add_remove_trunk_rchip_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_set_end_rchip_bitmap_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_trunk_rchip_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_set_break_en_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_bind_trunk_cmd);
	install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_bind_trunk_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_set_header_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_header_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_property_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_create_keeplive_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_set_keeplive_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_keeplive_members_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_create_mcast_profile_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_add_mcast_profile_member_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_mcast_profile_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_trunk_members_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_set_rchip_port_en_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_rchip_port_en_cmd);

    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_discard_port_chip_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_block_port_chip_cmd);
	install_element(CTC_SDK_MODE,  &ctc_cli_stacking_show_port_rchip_discard_cmd);


    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_debug_on_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_debug_off_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_stacking_trunk_replace_ports_cmd);

    return CLI_SUCCESS;

}

