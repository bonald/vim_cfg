/**
 @file ctc_l2_cli.c

 @date 2009-10-30

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
#include "ctc_l2_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_l2.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#define CTC_L2_FDB_QUERY_ENTRY_NUM  100

sal_task_t* ctc_l2_write_fdb_entry_to_file = NULL;
sal_task_t* ctc_l2_flush_dump = NULL;
uint32      ctc_l2_flush_dump_flag = 1;

char*
ctc_get_state_desc(enum stp_state_e state)
{
    switch (state)
    {
    case CTC_STP_FORWARDING:
        return "Forwarding";

    case  CTC_STP_BLOCKING:
        return "Blocking";

    case  CTC_STP_LEARNING:
        return "Learning";

    default:
        return "wrong state!";
    }
}

char*
ctc_get_entry_flag_desc(enum ctc_l2_fdb_entry_flag_e entry_flag)
{
    switch (entry_flag)
    {
    case CTC_L2_FDB_ENTRY_STATIC:
        return "All static FDB entry";

    case CTC_L2_FDB_ENTRY_DYNAMIC:
        return "All dynamic FDB entry";

    case  CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC:
        return "All local chip's dynamic fdb entry";

    case  CTC_L2_FDB_ENTRY_ALL:
        return "All Fdb static and dynamic entry";

    default:
        return "Wrong FDB type!";
    }
}

char*
ctc_get_flush_type_desc(enum ctc_l2_fdb_entry_op_type_e flush_type)
{
    switch (flush_type)
    {
    case CTC_L2_FDB_ENTRY_OP_BY_VID:
        return "Fdb entry operated by vlan";

    case CTC_L2_FDB_ENTRY_OP_BY_PORT:
        return "Fdb entry operated by port";

    case  CTC_L2_FDB_ENTRY_OP_BY_MAC:
        return "Fdb entry operated by mac";

    case  CTC_L2_FDB_ENTRY_OP_ALL:
        return "Fdb entry operated by vlan, mac and port";

    default:
        return "wrong flush type";
    }
}

CTC_CLI(ctc_cli_l2_add_ucast_nexthop,
        ctc_cli_l2_add_ucast_nexthop_cmd,
        "l2 fdb add ucast-nexthop port GPORT_ID ( (type { basic-nh | untag-nh | bypass-nh | raw-nh | service-queue-nh } ) | )",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        "Ucast bridge nexthop entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Type of nexthop (if undefine any subtype, all will be add)",
        "Basic brgu nexthop",
        "Untagged brgu nexthop",
        "Bypass brgu nexthop",
        "Raw_pkt brgu nexthop",
        "Service queue brgu nexthop")
{
    uint16   gport = 0;
    int32    ret  = CLI_SUCCESS;
    uint8    index = 0xFF;
    ctc_nh_param_brguc_sub_type_t nh_type = CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL;

    /*1) parse port id */
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("basic-nh");
    if (0xFF != index)
    {
        nh_type |= CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("untag-nh");
    if (0xFF != index)
    {
        nh_type |= CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bypass-nh");
    if (0xFF != index)
    {
        nh_type |= CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("raw-nh");
    if (0xFF != index)
    {
        nh_type |= CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-queue-nh");
    if (0xFF != index)
    {
        nh_type |= CTC_NH_PARAM_BRGUC_SUB_TYPE_SERVICE_QUEUE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_add_l2uc(g_api_lchip, gport, nh_type);
    }
    else
    {
        ret = ctc_nh_add_l2uc(gport, nh_type);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_remove_ucast_nexthop,
        ctc_cli_l2_remove_ucast_nexthop_cmd,
        "l2 fdb remove ucast-nexthop port GPORT_ID",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_REMOVE,
        "Ucast bridge nexthop entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC)
{
    uint16   gport = 0;
    int32    ret  = CLI_SUCCESS;

    /*1) parse port id */
    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_remove_l2uc(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_nh_remove_l2uc(gport);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_add_addr,
        ctc_cli_l2_add_addr_cmd,
        "l2 fdb (add | replace match-mac MAC match-fid FID) mac MAC fid FID {port GPORT_ID | nexthop NHPID} \
       (((vpls src-port GPORT_ID)| virtual-port VIRTUAL_PORT)|)(assign-port GPORT_ID|)\
       ({static | remote-dynamic | discard | src-discard | src-discard-tocpu | copy-tocpu | bind-port | raw-pkt-elog-cpu | service-queue | untagged | protocol-entry | system-mac | ptp-entry | self-address | white-list-entry|aging-disable|learn-limit-exempt}|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        "Replace operation",
        "Match MAC address",
        CTC_CLI_MAC_FORMAT,
        "Match FID",
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Nexthop",
        CTC_CLI_NH_ID_STR,
        "Vpls",
        "Src port after vpls loopback",
        CTC_CLI_GPORT_DESC,
        "Virtual port, use for vpls/pbb/qinq service queue",
        "Virtual port value, range 0-0x1FFF, and 0x1FFF is invalid",
        "Assign output gport",
        CTC_CLI_GPORT_ID_DESC,
        "Static fdb table",
        "Remote chip learing dynamic fdb",
        "Destination discard fdb table",
        "Source discard fdb table",
        "Source discard and send to cpu fdb table",
        "Forward and copy to cpu fdb table",
        "Port bind fdb table",
        "Send raw packet to cpu,using for terminating protocol pakcet(LBM/DMM/LMM) to cpu",
        "Packet will enqueue by service id",
        "Output packet will has no vlan tag",
        "Protocol entry enable exception flag",
        "Indicate the entry is system mac, it can't be deleted by flush api, using for MAC DA",
        "PTP mac address for ptp process",
        "Indicate mac address is switch's",
        "Indicate the entry is white list entry, it won't do aging, and will do learning",
        "Indicate mac address should not be aged",
        "Learn limit exempt")
{
    int32 ret = CLI_SUCCESS;
    uint32 nexthpid = 0;
    ctc_l2_addr_t l2_addr;
    uint8 index = 0xFF;
    bool nexthop_config = FALSE;
    uint8 is_replace = 0;
    ctc_l2_replace_t l2_replace;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));
    sal_memset(&l2_replace, 0, sizeof(ctc_l2_replace_t));

    index = CTC_CLI_GET_ARGC_INDEX("replace");
    if (index != 0xFF)
    {
        is_replace = 1;
        CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[5]);
        CTC_CLI_GET_UINT16_RANGE("FID", l2_addr.fid, argv[6], 0, CTC_MAX_UINT16_VALUE);
    }
    else
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[1]);
        CTC_CLI_GET_UINT16_RANGE("FID", l2_addr.fid, argv[2], 0, CTC_MAX_UINT16_VALUE);
    }

     /*1) parse match MAC address */
    index = CTC_CLI_GET_ARGC_INDEX("match-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("match mac address", l2_replace.match_addr.mac, argv[index+1]);
    }

    /*2)parse match FID */
    index = CTC_CLI_GET_ARGC_INDEX("match-fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("match FID", l2_replace.match_addr.fid, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse port/nexthop */
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("src-port", l2_addr.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (0xFF != index)
    {
        nexthop_config = TRUE;
        CTC_CLI_GET_UINT32_RANGE("next-hop-id", nexthpid, argv[index+1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpls");
    if (0xFF != index)
    {
    }

    index = CTC_CLI_GET_ARGC_INDEX("virtual-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("logic-port", l2_addr.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", l2_addr.assign_port, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        l2_addr.flag |= CTC_L2_FLAG_ASSIGN_OUTPUT_PORT;
    }

    /*5) parse static|discard|src-discard|src-discard-tocpu
    |copy-tocpu|fwd-tocpu|storm-ctl|protocol|bind-port} */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_IS_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-dynamic");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_REMOTE_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard-tocpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("copy-tocpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_COPY_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bind-port");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_BIND_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("protocol-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("system-mac");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SYSTEM_RSV;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("self-address");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("white-list-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_WHITE_LIST_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aging-disable");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_AGING_DISABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("learn-limit-exempt");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_LEARN_LIMIT_EXEMPT;
    }



    if (is_replace)
    {
        sal_memcpy(&l2_replace.l2_addr, &l2_addr, sizeof(ctc_l2_addr_t));
        l2_replace.nh_id = nexthpid;
        if(g_ctcs_api_en)
        {
            ret = ctcs_l2_replace_fdb(g_api_lchip, &l2_replace);
        }
        else
        {
            ret = ctc_l2_replace_fdb(&l2_replace);
        }
    }
    else if (nexthop_config)
    {
        if (g_ctcs_api_en)
        {
             ret = ctcs_l2_add_fdb_with_nexthop(g_api_lchip, &l2_addr, nexthpid);
        }
        else
        {
            ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, nexthpid);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret =  ctcs_l2_add_fdb(g_api_lchip, &l2_addr);
        }
        else
        {
            ret =  ctc_l2_add_fdb(&l2_addr);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_remove_addr,
        ctc_cli_l2_remove_addr_cmd,
        "l2 fdb remove mac MAC fid FID",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_REMOVE,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC)
{

    int32    ret  = CLI_SUCCESS;
    ctc_l2_addr_t l2_addr;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));
    /*2) parse MAC address */

    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);

    /*3) parse FID */
    CTC_CLI_GET_UINT16_RANGE("FID", l2_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_remove_fdb(g_api_lchip, &l2_addr);
    }
    else
    {
        ret = ctc_l2_remove_fdb(&l2_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_flush_fdb,
        ctc_cli_l2_flush_fdb_cmd,
        "l2 fdb flush by (mac MAC | mac-fid MAC FID  | port GPORT_ID | fid FID | port-fid GPORT_ID FID | all) (is-logic-port|) (static | dynamic | local-dynamic | all) ",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Flush fdb",
        "Query condition",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Mac+FID",
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Port+FID",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FID_ID_DESC,
        "All condition",
        "Is logic port",
        "Static fdb table",
        "All dynamic fdb table",
        "Local chip dynamic fdb table",
        "Static and dynamic")
{
    uint8 index   = 0;
    int32    ret  = CLI_SUCCESS;

    ctc_l2_flush_fdb_t Flush;

    sal_memset(&Flush, 0, sizeof(ctc_l2_flush_fdb_t));

    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;
        index++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Flush.mac, argv[index]);

    }

    /*2) mac+fid */
    index = CTC_CLI_GET_ARGC_INDEX("mac-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
        index++;
        CTC_CLI_GET_MAC_ADDRESS("mac address", Flush.mac, argv[index]);
        index++;
        CTC_CLI_GET_UINT16_RANGE("vlan id",  Flush.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*3) parse PORT address */

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        index++;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*4)parse fid */

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        index++;
        CTC_CLI_GET_UINT16_RANGE("vlan id",  Flush.fid, argv[index], 0, CTC_MAX_UINT16_VALUE);
    }

    /*5)port+fid*/
    index = CTC_CLI_GET_ARGC_INDEX("port-fid");
    if (index != 0xFF)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        CTC_CLI_GET_UINT16_RANGE("gport", Flush.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("fid", Flush.fid, argv[index + 2], 0, CTC_MAX_UINT16_VALUE);
    }

    /* is logic port*/
    index = CTC_CLI_GET_ARGC_INDEX("is-logic-port");
    if (index != 0xFF)
    {
        Flush.use_logic_port = 1;
    }

    /*6) parse static|dynamic */
    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_STATIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("local-dynamic");
    if (index != 0xFF)
    {
        Flush.flush_flag = CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_flush_fdb(g_api_lchip, &Flush);
    }
    else
    {
        ret = ctc_l2_flush_fdb(&Flush);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_set_nhid_by_logic_port,
        ctc_cli_l2_set_nhid_by_logic_port_cmd,
        "l2 fdb logic-nhid logic-port LOGIC_PORT_ID (gport GPORT_ID |nexthop NH_ID )",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Set logic-port to nexthop mapping",
        "Logic port",
        "Reference to global config",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Next hop",
        CTC_CLI_NH_ID_STR)
{
    int32    ret  = CLI_SUCCESS;
    uint16 port = 0;
    uint32 nh_id = 0;
    uint32 logic_port = 0;
    uint8 index = 0xFF;

    CTC_CLI_GET_UINT32_RANGE("logic-port", logic_port, argv[0], 0, CTC_MAX_UINT32_VALUE);

    /* gport */
    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", port, argv[index+1], 0, CTC_MAX_UINT16_VALUE);

        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_get_l2uc(g_api_lchip, port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id);
        }
        else
        {
            ret = ctc_nh_get_l2uc(port, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id);
        }

        if (ret < 0)
        {
            ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

    }

    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("nexthop", nh_id, argv[index+1], 0, CTC_MAX_UINT32_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_set_nhid_by_logic_port(g_api_lchip, logic_port, nh_id);
    }
    else
    {
        ret = ctc_l2_set_nhid_by_logic_port(logic_port, nh_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l2_get_nhid_by_logic_port,
        ctc_cli_l2_get_nhid_by_logic_port_cmd,
        "show l2 fdb logic-nhid by logic-port LOGIC_PORT_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "logic-port to nexthop mapping",
        "Condition",
        "Logic port",
        "Reference to global config")
{
    int32    ret  = CLI_SUCCESS;
    uint32 nh_id = 0;
    uint32 logic_port = 0;

    CTC_CLI_GET_UINT32_RANGE("logic-port", logic_port, argv[0], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_get_nhid_by_logic_port(g_api_lchip, logic_port, &nh_id);
    }
    else
    {
        ret = ctc_l2_get_nhid_by_logic_port(logic_port, &nh_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%-12s%s\n", "logic-port", "NHID");
    ctc_cli_out("-----------------------------\n");
    ctc_cli_out("%-12u%u\n", logic_port, nh_id);
    return CLI_SUCCESS;
}

#define _1_fdb_default_
CTC_CLI(ctc_cli_l2_add_default_entry,
        ctc_cli_l2_add_default_entry_cmd,
        "l2 fdb add vlan-default-entry (fid FID) (port GPORT_ID|) (group GROUP_ID) (share-group|) (use-logic-port|)(unknown-ucast-drop|)(unknown-mcast-drop|)(proto-to-cpu|) ",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Share mcast group",
        "Use logic port check",
        "Unknown ucast mac to drop",
        "Unknown mcast mac to drop",
        "Protocol exception to cpu")
{
    uint16 fid = 0;
    int32    ret  = CLI_SUCCESS;
    uint32 l2mc_grp_id = 0;
    uint8 index = 0;

    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    /*2) parse*/

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("group id", l2mc_grp_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        l2dflt_addr.l2mc_grp_id = l2mc_grp_id;
    }

    index = CTC_CLI_GET_ARGC_INDEX("share-group");
    if (index != 0xFF)
    {
        l2dflt_addr.share_grp_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-ucast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-mcast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("proto-to-cpu");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_add_default_entry(g_api_lchip, &l2dflt_addr);
    }
    else
    {
        ret = ctc_l2_add_default_entry(&l2dflt_addr);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_remove_default_entry,
        ctc_cli_l2_remove_default_entry_cmd,
        "l2 fdb remove vlan-default-entry (fid FID) (port GPORT_ID |)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_REMOVE,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC)
{

    uint16 fid = 0;
    int32    ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_remove_default_entry(g_api_lchip, &l2dflt_addr);
    }
    else
    {
        ret = ctc_l2_remove_default_entry(&l2dflt_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_l2_get_default_entry_feature,
        ctc_cli_l2_get_default_entry_feature_cmd,
        "show l2 fdb vlan-default-entry (fid FID) features",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "All features")

{
    uint16 fid = 0;
    int32  ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));
    /*FID*/
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_get_default_entry_features(g_api_lchip, &l2dflt_addr);
    }
    else
    {
        ret = ctc_l2_get_default_entry_features(&l2dflt_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Get vlan default entry features fid %4d, group id: %4d\n", l2dflt_addr.fid, l2dflt_addr.l2mc_grp_id);
    ctc_cli_out("---------------------------------------------------------\n");
    ctc_cli_out("%20s : %d\n", "use-logic-port    ", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "unknown-ucast-drop", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "unknown-mcast-drop", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "proto-to-cpu      ", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "learn-dis         ", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_LEARNING_DISABLE) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "port-mac-auth     ", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTH) ? 1 : 0);
    ctc_cli_out("%20s : %d\n", "vlan-mac-auth     ", (l2dflt_addr.flag & CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH) ? 1 : 0);

    return ret;

}
CTC_CLI(ctc_cli_l2_set_default_entry_feature,
        ctc_cli_l2_set_default_entry_feature_cmd,
        "l2 fdb vlan-default-entry (fid FID) (port GPORT_ID |) ({use-logic-port | unknown-ucast-drop | unknown-mcast-drop | proto-to-cpu | learn-dis | port-mac-auth | vlan-mac-auth} | default)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Use logic port check",
        "Unknown ucast mac to drop",
        "Unknown mcast mac to drop",
        "Protocol exception to cpu",
        "Learning disable",
        "Port base MAC authentication",
        "Vlan base MAC authentication",
        "Recover the default unknown mac action")
{
    uint16 fid = 0;
    int32  ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    /*FID*/
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT;
    }

    /*FEATURE*/
    index = CTC_CLI_GET_ARGC_INDEX("unknown-ucast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-mcast-drop");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("proto-to-cpu");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("learn-dis");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_LEARNING_DISABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("port-mac-auth");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTH;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlan-mac-auth");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_set_default_entry_features(g_api_lchip, &l2dflt_addr);
    }
    else
    {
        ret = ctc_l2_set_default_entry_features(&l2dflt_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_add_or_remove_port_to_default_entry,
        ctc_cli_l2_add_or_remove_port_to_default_entry_cmd,
        "l2 fdb vlan-default-entry (fid FID) (port GPORT_ID |) (add |remove) (({port GPORT_ID |nexthop NHPID })| ("CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|))) (remote-chip|) (service-queue|) (assign-output|)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Vlan default entry",
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_ADD,
        CTC_CLI_REMOVE,
        "Global port /Remote chip id",
        "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)",
        "Nexthop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Indicate the member is remote chip",
        "Enable transport by service queue",
        "Indicate output port is user assigned")
{
    int32  ret  = CLI_SUCCESS;
    uint16 fid = 0;
    uint8 index = 0xFF;
    uint8 index_tmp = 0xFF;
    uint8 baddflag = 0;
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0, sizeof(ctc_l2dflt_addr_t));

    /*FID*/
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("forwarding id", fid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        l2dflt_addr.fid = fid;
    }

    /*ADD/REMOVE*/
    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (index != 0xFF)
    {
        baddflag = 1;
        index_tmp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (index != 0xFF)
    {
        baddflag = 0;
        index_tmp = index;
    }

    /*3) parse global port id */
    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index == 0xFF)
    {
        index = CTC_CLI_GET_SPECIFIC_INDEX("port", index_tmp);
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("gport", l2dflt_addr.member.mem_port, argv[index + index_tmp + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    else
    {
        index = CTC_CLI_GET_SPECIFIC_INDEX("port", index_tmp);
        if (index != 0xFF)
        {
            CTC_CLI_GET_UINT16_RANGE("remote", l2dflt_addr.member.mem_port, argv[index + index_tmp + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    /*3) parse nexhop id id */
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        l2dflt_addr.with_nh = 1;
        CTC_CLI_GET_UINT32_RANGE("nexhop id ", l2dflt_addr.member.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        index++;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index != 0xFF)
    {
        l2dflt_addr.remote_chip = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-output");
    if (index != 0xFF)
    {
        l2dflt_addr.flag |= CTC_L2_DFT_VLAN_FLAG_ASSIGN_OUTPUT_PORT;
    }
    l2dflt_addr.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(l2dflt_addr.member.port_bmp);
    if (l2dflt_addr.member.port_bmp[0] || l2dflt_addr.member.port_bmp[1] ||
        l2dflt_addr.member.port_bmp[8] || l2dflt_addr.member.port_bmp[9])
    {
        l2dflt_addr.port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", l2dflt_addr.gchip_id, argv[index + 1]);
    }

    if (baddflag)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_add_port_to_default_entry(g_api_lchip, &l2dflt_addr);
        }
        else
        {
            ret = ctc_l2_add_port_to_default_entry(&l2dflt_addr);
        }
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_remove_port_from_default_entry(g_api_lchip, &l2dflt_addr);
        }
        else
        {
            ret = ctc_l2_remove_port_from_default_entry(&l2dflt_addr);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2mcast_add_addr,
        ctc_cli_l2mcast_add_addr_cmd,
        "l2 mcast add group GROUP_ID mac MAC fid FID ({protocol-exp | ptp-entry | self-address}|cid CID | share-group |)",
        CTC_CLI_L2_M_STR,
        "Multicast",
        CTC_CLI_ADD,
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Protocol Entry flag",
        "PTP mac address for ptp process",
        "Indicate mac address is switch's",
        "Cid",
        "Cid value",
        "Share mcast group")
{
    int32 ret  = CLI_SUCCESS;
    ctc_l2_mcast_addr_t l2mc_addr;
    uint8 index = 0xFF;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    /*1) parse add/remove*/
    /*2) parse group id*/
    CTC_CLI_GET_UINT16_RANGE("group id", l2mc_addr.l2mc_grp_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    /*3) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[1]);

    /*4) parser fid*/
    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2mc_addr.fid, argv[2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("protocol-exp");
    if (0xFF != index)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp-entry");
    if (0xFF != index)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("self-address");
    if (0xFF != index)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cid", l2mc_addr.cid, argv[index+1], 0, CTC_MAX_UINT16_VALUE);;
    }

    index = CTC_CLI_GET_ARGC_INDEX("share-group");
    if (0xFF != index)
    {
        l2mc_addr.share_grp_en = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2mcast_add_addr(g_api_lchip, &l2mc_addr);
    }
    else
    {
        ret = ctc_l2mcast_add_addr(&l2mc_addr);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2mcast_remove_addr,
        ctc_cli_l2mcast_remove_addr_cmd,
        "l2 mcast remove group mac MAC fid FID {keep-empty-entry |}",
        CTC_CLI_L2_M_STR,
        "Multicast",
        CTC_CLI_REMOVE,
        "Multicast group",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Remove all member from group")
{
    int32 ret  = CLI_SUCCESS;
    ctc_l2_mcast_addr_t l2mc_addr;
    uint8 index = 0xFF;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    /*1) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[0]);

    /*2) parser fid*/
    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2mc_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("keep-empty-entry");
    if (index != 0xFF)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_KEEP_EMPTY_ENTRY;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2mcast_remove_addr(g_api_lchip, &l2mc_addr);
    }
    else
    {
        ret = ctc_l2mcast_remove_addr(&l2mc_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2mcast_add_member,
        ctc_cli_l2mcast_add_member_cmd,
        "l2 mcast add member mac MAC fid FID (({port GPORT_ID | nexthop NH_ID}) | (" CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|)))  (remote-chip|) (service-queue|) (assign-output|)",
        CTC_CLI_L2_M_STR,
        "L2 multicast",
        CTC_CLI_ADD,
        "The member of multicast group",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Global port /Remote chip id",
        "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)",
        "Next hop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Indicate the member is remote chip entry",
        "Enable transport by service queue",
        "Indicate output port is user assigned")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2_mcast_addr_t l2mc_addr;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    /*1) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[0]);

    /*2) parser fid*/
    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2mc_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    /*3) parse global port id */
    l2mc_addr.member_invalid = 1;
    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index == 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("gport", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("remote", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    /*4) parser nexthop id */
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        l2mc_addr.with_nh = 1;
        l2mc_addr.member_invalid  = 0;
        CTC_CLI_GET_UINT32_RANGE("nh id",  l2mc_addr.member.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index != 0xFF)
    {
        l2mc_addr.remote_chip   = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-output");
    if (index != 0xFF)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_ASSIGN_OUTPUT_PORT;
    }
    l2mc_addr.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(l2mc_addr.member.port_bmp);
    if (l2mc_addr.member.port_bmp[0] || l2mc_addr.member.port_bmp[1] ||
        l2mc_addr.member.port_bmp[8] || l2mc_addr.member.port_bmp[9])
    {
        l2mc_addr.port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", l2mc_addr.gchip_id, argv[index + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret  = ctcs_l2mcast_add_member(g_api_lchip, &l2mc_addr);
    }
    else
    {
        ret  = ctc_l2mcast_add_member(&l2mc_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2mcast_remove_member,
        ctc_cli_l2mcast_remove_member_cmd,
        "l2 mcast remove member mac MAC fid FID (({port GPORT_ID | nexthop NH_ID})| ("CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|))) (remote-chip|) (assign-output|) ",
        CTC_CLI_L2_M_STR,
        "L2 multicast",
        CTC_CLI_REMOVE,
        "The member of multicast group",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Global port /Remote chip id",
        "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)",
        CTC_CLI_NH_ID_STR,
        "Nexthop id",
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Indicate the member is remote chip entry",
        "Indicate output port is user assigned")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2_mcast_addr_t l2mc_addr;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    /*2) parse MAC address */
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2mc_addr.mac, argv[0]);

    /*3) parser fid*/
    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2mc_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);

    /*4) parse global port id */
    l2mc_addr.member_invalid = 1;

    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index == 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("gport", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("remote", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }

    /*6) parser nexthop id */
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        l2mc_addr.with_nh = 1;
        l2mc_addr.member_invalid  = 0;
        CTC_CLI_GET_UINT32_RANGE("nh id",  l2mc_addr.member.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index != 0xFF)
    {
        l2mc_addr.remote_chip   = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-output");
    if (index != 0xFF)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_ASSIGN_OUTPUT_PORT;
    }
    l2mc_addr.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(l2mc_addr.member.port_bmp);
    if (l2mc_addr.member.port_bmp[0] || l2mc_addr.member.port_bmp[1] ||
        l2mc_addr.member.port_bmp[8] || l2mc_addr.member.port_bmp[9])
    {
        l2mc_addr.port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (index != 0xFF)
    {

        CTC_CLI_GET_UINT8("gchip", l2mc_addr.gchip_id, argv[index + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret  = ctcs_l2mcast_remove_member(g_api_lchip, &l2mc_addr);
    }
    else
    {
        ret  = ctc_l2mcast_remove_member(&l2mc_addr);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n",  ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_show_fdb_count,
        ctc_cli_l2_show_fdb_count_cmd,
        "show l2 fdb count by (mac MAC | mac-fid MAC FID | port GPORT_ID |logic-port LOGIC_PORT_ID | fid FID | port-fid GPORT_ID FID | all) (static|) (dynamic|) (local-dynamic|) (all|) (hw|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Count FDB entries",
        "Query condition",
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
        "Static and dynamic",
        "Show by Hardware")
{

    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2_fdb_query_t Query;

    sal_memset(&Query, 0, sizeof(ctc_l2_fdb_query_t));
    Query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Query.query_flag = CTC_L2_FDB_ENTRY_ALL;

    /*1) parse MAC address */

    index = CTC_CLI_GET_ARGC_INDEX("mac");
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
    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        CTC_CLI_GET_UINT16_RANGE("gport", Query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    /*2-1 parse logic port*/

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        Query.use_logic_port = 1;
        CTC_CLI_GET_UINT16_RANGE("logic-port", Query.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }


    /*3) parse fid address*/

    index = CTC_CLI_GET_ARGC_INDEX("fid");
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

    /* 6. parse hw mode */
    index = CTC_CLI_GET_ARGC_INDEX("hw");
    if (index != 0xFF)
    {
        Query.query_hw = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_get_fdb_count(g_api_lchip, &Query);
    }
    else
    {
        ret = ctc_l2_get_fdb_count(&Query);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("  Query number : %u\n", Query.count);

    return ret;

}

void
ctc_write_fdb_entry_to_file(void* user_param)
{
    int32 ret = 0;
    uint8 index = 0;
    uint32 total_count = 0;
    sal_file_t fp = NULL;
    ctc_l2_fdb_query_rst_t query_rst;
    ctc_l2_write_info_para_t write_entry_para;
    char* char_buffer = NULL;
    ctc_l2_fdb_query_t* query = NULL;
    uint16 entry_num = 0;

    sal_memset(&query_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));
    sal_memset(&write_entry_para, 0, sizeof(ctc_l2_write_info_para_t));

    sal_memcpy(&write_entry_para, user_param, sizeof(ctc_l2_write_info_para_t));
    mem_free(user_param);

    query = (ctc_l2_fdb_query_t*)write_entry_para.pdata;

    entry_num = (query->query_hw)?CTC_L2_FDB_QUERY_ENTRY_NUM:10;

    query_rst.buffer_len = sizeof(ctc_l2_addr_t) * entry_num;
    query_rst.buffer = (ctc_l2_addr_t*)mem_malloc(MEM_CLI_MODULE, query_rst.buffer_len);
    if (NULL == query_rst.buffer)
    {
        mem_free(write_entry_para.pdata);
        write_entry_para.pdata = NULL;
        ctc_cli_out("%% Alloc  memory  failed \n");
        return;
    }

    fp = sal_fopen((char*)write_entry_para.file, "w+");
    if (NULL == fp)
    {
        mem_free(query_rst.buffer);
        query_rst.buffer = NULL;
        mem_free(write_entry_para.pdata);
        write_entry_para.pdata = NULL;
        return;
    }

    char_buffer = (char*)mem_malloc(MEM_CLI_MODULE, 128);
    if (NULL == char_buffer)
    {
        mem_free(query_rst.buffer);
        query_rst.buffer = NULL;
        mem_free(write_entry_para.pdata);
        sal_fclose(fp);
        fp = NULL;
        write_entry_para.pdata = NULL;
        return;
    }

    ctc_cli_out("Please read entry-file after seeing \"FDB entry have been writen!\"\n");
    sal_fprintf(fp, "%8s  %14s  %4s %6s\n", "MAC", "FID", "GPORT", "Static");
    sal_fprintf(fp, "-------------------------------------------\n");

    do
    {
        query_rst.start_index = query_rst.next_query_index;

        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fdb_entry(g_api_lchip, query, &query_rst);
        }
        else
        {
            ret = ctc_l2_get_fdb_entry(query, &query_rst);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            mem_free(query_rst.buffer);
            query_rst.buffer = NULL;
            mem_free(write_entry_para.pdata);
            write_entry_para.pdata = NULL;
            mem_free(char_buffer);
            char_buffer = NULL;
            sal_fclose(fp);
            fp = NULL;
            return;
        }

        for (index = 0; index < query->count; index++)
        {
            sal_sprintf(char_buffer, "%.4x.%.4x.%.4x%4s  ", sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                        sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                        sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                        " ");

            sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%.4d  ", query_rst.buffer[index].fid);
            sal_sprintf((char_buffer + sal_strlen(char_buffer)), "0x%.4x  ", query_rst.buffer[index].gport);
            sal_sprintf((char_buffer + sal_strlen(char_buffer)), "%s\n", (query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No");
            sal_fprintf(fp, "%s", char_buffer);

            sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
        }

        total_count += query->count;
        sal_task_sleep(100);

    }
    while (query_rst.is_end == 0);

    sal_fprintf(fp, "-------------------------------------------\n");
    sal_fprintf(fp, "Total Entry Num: %d\n", total_count);

    ctc_cli_out("FDB entry have been writen\n");

    mem_free(query_rst.buffer);
    query_rst.buffer = NULL;
    mem_free(write_entry_para.pdata);
    write_entry_para.pdata = NULL;
    mem_free(char_buffer);
    char_buffer = NULL;
    sal_fclose(fp);
    fp = NULL;
    return;
}

CTC_CLI(ctc_cli_l2_show_fdb,
        ctc_cli_l2_show_fdb_cmd,
        "show l2 fdb entry by (mac MAC | mac-fid MAC FID | port GPORT_ID | logic-port LOGIC_PORT_ID | fid FID | port-fid GPORT_ID FID | all ) ((static|dynamic|local-dynamic|mcast|conflict|all)|) (hw|) (entry-file FILE_NAME|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "FDB entries",
        "Query condition",
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
    uint8 is_hw = 0;
    uint16 entry_cnt = 0;

    sal_memset(&query_rst, 0, sizeof(ctc_l2_fdb_query_rst_t));
    sal_memset(&Query, 0, sizeof(ctc_l2_fdb_query_t));

    Query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Query.query_flag = CTC_L2_FDB_ENTRY_ALL;

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
        is_hw = 1;
        Query.query_hw = 1;
    }
    entry_cnt = is_hw?CTC_L2_FDB_QUERY_ENTRY_NUM:100;

    if (write_entry_para && *write_entry_para->file)
    {
        /* free memery will be done in ctc_write_fdb_entry_to_file */
        write_entry_para->pdata = (ctc_l2_fdb_query_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_l2_fdb_query_t));
        if (NULL == write_entry_para->pdata)
        {
            if (write_entry_para)
            {
                mem_free(write_entry_para);
            }
            return CLI_ERROR;
        }

        sal_memcpy(write_entry_para->pdata, &Query, sizeof(ctc_l2_fdb_query_t));

        if (0 != sal_task_create(&ctc_l2_write_fdb_entry_to_file,
                                 "ctcFdbWFl",
                                 SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, ctc_write_fdb_entry_to_file, write_entry_para))
        {
            sal_task_destroy(ctc_l2_write_fdb_entry_to_file);
            return CTC_E_NOT_INIT;
        }
    }
    else
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

        ctc_cli_out("%8s  %14s  %4s %8s\n", "MAC", "FID", "GPORT", "Static");
        ctc_cli_out("-------------------------------------------\n");

        do
        {
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
                ctc_cli_out("%.4x.%.4x.%.4x%4s  ", sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[0]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[2]),
                            sal_ntohs(*(unsigned short*)&query_rst.buffer[index].mac[4]),
                            " ");

                ctc_cli_out("%.4d  ", query_rst.buffer[index].fid);
                ctc_cli_out("0x%.4x  ", query_rst.buffer[index].gport);
                ctc_cli_out("%4s\n", (query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No");

                sal_memset(&query_rst.buffer[index], 0, sizeof(ctc_l2_addr_t));
            }

            total_count += Query.count;
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

CTC_CLI(ctc_cli_l2_stp_set_state,
        ctc_cli_l2_stp_set_state_cmd,
        "stp state port GPHYPORT_ID stpid STP_ID state (forwarding|blocking|learning) ",
        CTC_CLI_STP_M_STR,
        "Configure stp state",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "MST instance",
        "MST instance ID <0-127>",
        "Stp state",
        "Forwarding state",
        "Blocking state",
        "Learning state")
{
    int32 ret  = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 stpid = 0;
    uint8 state = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("stpid", stpid, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if (!sal_memcmp(argv[2], "fo", 2))
    {
        state = CTC_STP_FORWARDING;
    }
    else if (!sal_memcmp(argv[2], "bl", 2))
    {
        state = CTC_STP_BLOCKING;
    }
    else
    {
        state = CTC_STP_LEARNING;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_stp_set_state(g_api_lchip, gport,  stpid,  state);
    }
    else
    {
        ret = ctc_stp_set_state(gport,  stpid,  state);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_l2_show_stp_state_by_port_and_stpid,
        ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd,
        "show stp state port GPHYPORT_ID stpid STP_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STP_M_STR,
        "Show stp state",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "MST instance",
        "MST instance ID <0-127>")
{
    int32 ret  = CLI_SUCCESS;
    uint16 gport = 0;
    uint8 stpid = 0;
    uint8 state = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    CTC_CLI_GET_UINT8("stpid", stpid, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_stp_get_state(g_api_lchip, gport, stpid, &state);
    }
    else
    {
        ret = ctc_stp_get_state(gport, stpid, &state);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out(" %-9s%-8s%s\n", "Gport", "StpId", "State");
    ctc_cli_out(" ---------------------------------\n");
    ctc_cli_out(" 0x%.4x   %-8u%s\n", gport, stpid, ctc_get_state_desc(state));

    return ret;
}

CTC_CLI(ctc_cli_l2_show_stp_state_by_port,
        ctc_cli_l2_show_stp_state_by_port_cmd,
        "show stp state port GPHYPORT_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_STP_M_STR,
        "Stp state",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint16 gport = 0;
    uint32 stpid = 0;
    uint8  stp_state = 0;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);
    ctc_cli_out(" Gport:0x%04X\n", gport);

    ctc_cli_out(" ------------------\n");
    ctc_cli_out(" %-8s%s\n", "StpId", "State");
    ctc_cli_out(" ------------------\n");

    for (stpid = 0; stpid < 128; stpid++)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_stp_get_state(g_api_lchip, gport, stpid, &stp_state);
        }
        else
        {
            ret = ctc_stp_get_state(gport, stpid, &stp_state);
        }

        if (ret < 0)
        {
            break;
        }
        ctc_cli_out(" %-8u%s\n", stpid, ctc_get_state_desc(stp_state));
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_clear_stp_state,
        ctc_cli_l2_clear_stp_state_cmd,
        "clear stp state port GPHYPORT_ID",
        CTC_CLI_CLEAR_STR,
        CTC_CLI_STP_M_STR,
        "Stp state",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret  = CLI_SUCCESS;
    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_stp_clear_all_inst_state(g_api_lchip, gport);
    }
    else
    {
        ret = ctc_stp_clear_all_inst_state(gport);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_debug_on,
        ctc_cli_l2_debug_on_cmd,
        "debug l2 (fdb|stp) (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_STP_M_STR,
        "Ctc layer",
        "Sys layer",
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

    if (0 == sal_memcmp(argv[0], "fdb", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_FDB_CTC;

        }
        else
        {
            typeenum = L2_FDB_SYS;

        }
    }
    else if (0 == sal_memcmp(argv[0], "stp", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_STP_CTC;

        }
        else
        {
            typeenum = L2_STP_SYS;

        }
    }

    ctc_debug_set_flag("l2", argv[0], typeenum, level, TRUE);
    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_l2_debug_off,
        ctc_cli_l2_debug_off_cmd,
        "no debug l2 (fdb|stp) (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_STP_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "fdb", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_FDB_CTC;
        }
        else
        {
            typeenum = L2_FDB_SYS;
        }
    }
    else if (0 == sal_memcmp(argv[0], "stp", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_STP_CTC;
        }
        else
        {
            typeenum = L2_STP_SYS;
        }
    }

    ctc_debug_set_flag("l2", argv[0], typeenum, level, FALSE);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l2_set_fid_property,
        ctc_cli_l2_set_fid_property_cmd,
        "l2 fid FID (drop-unknown-ucast|drop-unknown-mcast|unknown-mcast-copy-tocpu |drop-bcast |igmp-snooping|mac-learning|igs-stats|egs-stats) value VALUE",
        CTC_CLI_L2_M_STR,
        "Fid",
        "Fid value",
        "Drop unknown ucast packet",
        "Drop unknown mcast packet",
        "Unknown mcast packet copy to cpu",
        "Drop bcast packet",
        "IGMP snooping",
        "Mac learning",
        "Igs stats",
        "Egs stats",
        "Value",
        "Value of the property")
{
    int32 ret;
    uint16 fid;
    uint32 value = 0;
    uint8 type = 0;

    CTC_CLI_GET_UINT16("fid id", fid, argv[0]);

    if(CLI_CLI_STR_EQUAL("drop-unknown-ucast", 1))
    {
        type = CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST;
    }
    else if (CLI_CLI_STR_EQUAL("drop-unknown-mcast", 1))
    {
        type = CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST;
    }
    else if (CLI_CLI_STR_EQUAL("unknown-mcast-copy-tocpu", 1))
    {
        type = CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU;
    }
    else if(CLI_CLI_STR_EQUAL("drop-bcast", 1))
    {
        type = CTC_L2_FID_PROP_DROP_BCAST;
    }
    else if (CLI_CLI_STR_EQUAL("igmp-snooping", 1))
    {
        type = CTC_L2_FID_PROP_IGMP_SNOOPING;
    }
    else if (CLI_CLI_STR_EQUAL("mac-learning", 1))
    {
        type = CTC_L2_FID_PROP_LEARNING;
    }
    else if (CLI_CLI_STR_EQUAL("igs-stats", 1))
    {
        type = CTC_L2_FID_PROP_IGS_STATS_EN;
    }
    else if (CLI_CLI_STR_EQUAL("egs-stats", 1))
    {
        type = CTC_L2_FID_PROP_EGS_STATS_EN;
    }

    CTC_CLI_GET_UINT32_RANGE("fid value", value, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_set_fid_property(g_api_lchip, fid, type, value);
    }
    else
    {
        ret = ctc_l2_set_fid_property(fid, type, value);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}



CTC_CLI(ctc_cli_l2_get_fid_property,
        ctc_cli_l2_get_fid_property_cmd,
        "show l2 fid FID {mac-learning | igmp-snooping | drop-unknown-ucast | drop-unknown-mcast | unknown-mcast-copy-tocpu |drop-bcast }",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        "Fid",
        "Fid value",
        "Mac learning ",
        "IGMP snooping ",
        "Drop unknown ucast packet",
        "Drop unknown mcast packet",
        "Unknown mcast packet copy to cpu",
        "Drop bcast packet")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint16 fid;
    uint32 val = 0;

    CTC_CLI_GET_UINT16("fid id", fid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("mac-learning");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_LEARNING, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_LEARNING, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("learning enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop-unknown-ucast");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST,&val);
        }
         else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST,&val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("drop unknown ucast enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop-unknown-mcast");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("drop unknown mcast enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("unknown-mcast-copy-tocpu");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("unknown mcast copy to cpu enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("drop-bcast");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_DROP_BCAST, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_DROP_BCAST, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("drop bcast enable             :%d\n", val);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-snooping");
    if (0xFF != index)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_get_fid_property(g_api_lchip, fid, CTC_L2_FID_PROP_IGMP_SNOOPING, &val);
        }
        else
        {
            ret = ctc_l2_get_fid_property(fid, CTC_L2_FID_PROP_IGMP_SNOOPING, &val);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("igmp snooping               :%d\n", val);
    }

    return ret;

}

CTC_CLI(ctc_cli_l2_update_fdb,
        ctc_cli_l2_update_fdb_cmd,
        "l2 fdb update mac MAC fid FID hit VALUE",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "Update FDB",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Hit",
        "Value")
{
    int32 ret;
    uint8 hit = 0;
    ctc_l2_addr_t l2_addr;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);
    CTC_CLI_GET_UINT16("fid id", l2_addr.fid, argv[1]);
    CTC_CLI_GET_UINT8("value", hit, argv[2]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_fdb_set_entry_hit(g_api_lchip, &l2_addr, hit);
    }
    else
    {
        ret = ctc_l2_fdb_set_entry_hit(&l2_addr, hit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l2_get_fdb_hit,
        ctc_cli_l2_get_fdb_hit_cmd,
        "show l2 fdb mac MAC fid FID hit",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Hit")
{
    int32 ret;
    uint8 hit = 0;
    ctc_l2_addr_t l2_addr;

    sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);
    CTC_CLI_GET_UINT16("fid id", l2_addr.fid, argv[1]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2_fdb_get_entry_hit(g_api_lchip, &l2_addr, &hit);
    }
    else
    {
        ret = ctc_l2_fdb_get_entry_hit(&l2_addr, &hit);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("hit              :%d\n", hit);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_l2_show_debug,
        ctc_cli_l2_show_debug_cmd,
        "show debug l2 (fdb|stp) (ctc|sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_STP_M_STR,
        "Ctc layer",
        "Sys layer")
{

    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "fdb", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_FDB_CTC;
        }
        else
        {
            typeenum = L2_FDB_SYS;
        }
    }
    else if (0 == sal_memcmp(argv[0], "stp", 3))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = L2_STP_CTC;
        }
        else
        {
            typeenum = L2_STP_SYS;
        }
    }

    en = ctc_debug_get_flag("l2", argv[0], typeenum, &level);
    ctc_cli_out(" l2 %s:%s debug %s level:%s\n", argv[0], argv[1],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));
    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_l2_show_fdb_entry_by_index,
        ctc_cli_l2_show_fdb_entry_by_index_cmd,
        "show l2 fdb entry by-index INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        "FDB entry",
        "by-index",
        "INDEX")
{
    int32 ret = 0;
    uint32 index = 0;
    ctc_l2_addr_t  l2_addr;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    CTC_CLI_GET_UINT32("by-index", index, argv[0]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_l2_get_fdb_by_index(g_api_lchip, index, &l2_addr);
    }
    else
    {
        ret = ctc_l2_get_fdb_by_index(index, &l2_addr);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%8s  %14s  %4s %6s\n", "MAC", "FID", "GPORT", "Static");
    ctc_cli_out("-------------------------------------------\n");
    ctc_cli_out("%.4x.%.4x.%.4x%4s  ", sal_ntohs(*(unsigned short*)&l2_addr.mac[0]), sal_ntohs(*(unsigned short*)&l2_addr.mac[2]), sal_ntohs(*(unsigned short*)&l2_addr.mac[4]), " ");

    ctc_cli_out("%.4d  ", l2_addr.fid);
    ctc_cli_out("0x%.4x  ", l2_addr.gport);
    ctc_cli_out("%s\n", (l2_addr.flag & CTC_L2_FLAG_IS_STATIC) ? "Yes" : "No");


    return ret;

}

CTC_CLI(ctc_cli_l2_remove_fdb_entry_by_index,
        ctc_cli_l2_remove_fdb_entry_by_index_cmd,
        "l2 fdb remove entry by-index INDEX",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_REMOVE,
        "FDB entry",
        "by-index",
        "INDEX")
{
    int32 ret = 0;
    uint32 index = 0;


    CTC_CLI_GET_UINT32("by-index", index, argv[0]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_l2_remove_fdb_by_index(g_api_lchip, index);
    }
    else
    {
        ret = ctc_l2_remove_fdb_by_index(index);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;


}


int32
ctc_l2_cli_init(void)
{

    install_element(CTC_SDK_MODE, &ctc_cli_l2_set_fid_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_get_fid_property_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l2_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_debug_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_ucast_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_remove_ucast_nexthop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_addr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_remove_addr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_flush_fdb_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_remove_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_set_default_entry_feature_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_get_default_entry_feature_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_add_or_remove_port_to_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2mcast_add_addr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2mcast_remove_addr_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2mcast_remove_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2mcast_add_member_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_set_nhid_by_logic_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_get_nhid_by_logic_port_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l2_stp_set_state_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_stp_state_by_port_and_stpid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_stp_state_by_port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_clear_stp_state_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_fdb_count_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_fdb_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_update_fdb_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_get_fdb_hit_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_show_fdb_entry_by_index_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_l2_remove_fdb_entry_by_index_cmd);




    return 0;
}

