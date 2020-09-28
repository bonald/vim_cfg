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
#include "ctc_mirror_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_mirror.h"
#include "ctc_error.h"
#include "ctc_debug.h"

/****************************************************************************
 *
* Function
*
*****************************************************************************/

CTC_CLI(ctc_cli_mirror_set_port_enable,
        ctc_cli_mirror_set_port_enable_cmd,
        "mirror source port (GPHYPORT_ID |cpu ) (gchip GCHIP|) direction (ingress | egress | both) (enable session SESSION_ID | disable)",
        CTC_CLI_MIRROR_M_STR,
        "Mirror source, port or vlan",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Network cpu port or dma cpu channel",
        "Gchip",
        "Gchip ID",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Mirror enable",
        "Configure a mirror session",
        "Session_id range <0-3>, cpu mirror don't care session id",
        "Mirror disable")
{
    uint8 session_id = 0;
    uint32 gport;
    uint8  gchip = 0;
    ctc_direction_t dir = CTC_INGRESS;
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("gchip", gchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cpu");
    if (0xFF != index)
    {
        gport = CTC_GPORT_RCPU(gchip);
    }
    else
    {
        CTC_CLI_GET_UINT32("gport", gport, argv[0]);
    }

    /*parse direction*/
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (0xFF != index)
    {
        dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (0xFF != index)
    {
        dir = CTC_EGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("both");
    if (0xFF != index)
    {
        dir = CTC_BOTH_DIRECTION;
    }
    /*parse session*/
    index = CTC_CLI_GET_ARGC_INDEX("session");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[index + 1],
                                0, CTC_MAX_UINT8_VALUE);
        index = 0xFF;
    }


    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_port_en(g_api_lchip, gport, dir, TRUE, session_id);
        }
        else
        {
            ret = ctc_mirror_set_port_en(gport, dir, TRUE, session_id);
        }
        index = 0xFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_port_en(g_api_lchip, gport, dir, FALSE, session_id);
        }
        else
        {
            ret = ctc_mirror_set_port_en(gport, dir, FALSE, session_id);
        }
        index = 0xFF;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_mirror_set_vlan_enable,
        ctc_cli_mirror_set_vlan_enable_cmd,
        "mirror source vlan VLAN_ID direction (ingress|egress|both) (enable session SESSION_ID | disable)",
        CTC_CLI_MIRROR_M_STR,
        "Mirror source, port or vlan",
        CTC_CLI_VLAN_M_STR,
        CTC_CLI_VLAN_RANGE_DESC,
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Mirror enable",
        "Configure a mirror session",
        "Session_id range <0-3>",
        "Mirror disable")
{
    uint8 session_id = 0;
    uint16 vlan_id;
    ctc_direction_t dir;
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0xFF;

    /*parse vlan*/
    CTC_CLI_GET_UINT16("vlan", vlan_id, argv[0]);

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        dir = CTC_EGRESS;
    }
    else
    {
        dir = CTC_BOTH_DIRECTION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("session");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("session id", session_id, argv[index + 1],
                                0, CTC_MAX_UINT8_VALUE);
        index = 0xFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_vlan_en(g_api_lchip, vlan_id, dir, TRUE, session_id);
        }
        else
        {
            ret = ctc_mirror_set_vlan_en(vlan_id, dir, TRUE, session_id);
        }
        index = 0xFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_vlan_en(g_api_lchip, vlan_id, dir, FALSE, session_id);
        }
        else
        {
            ret = ctc_mirror_set_vlan_en(vlan_id, dir, FALSE, session_id);
        }
        index = 0xFF;
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_mirror_add_rspan_session,
        ctc_cli_mirror_add_rspan_session_cmd,
        "mirror remote-destination add session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session | \
        acl-session (priority-id PRIORITY_ID|) |cpu-session |ipfix-log-session ) (port GPORT_ID) (vlan VLAN_ID | nexthop NH_ID) (truncated-len LEN|)",
        CTC_CLI_MIRROR_M_STR,
        "Remote mirror destination",
        "Add destination for mirror session",
        "Configure a mirror session",
        "Session_id range <0-3>",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Port session",
        "Vlan session",
        "Acl session",
        "Acl priority id",
        CTC_CLI_ACL_PRIORITY_ID_DESC,
        "Cpu session",
        "Ipfix log session",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Add vlan tagged for remote mirror",
        CTC_CLI_VLAN_RANGE_DESC,
        "Nexthop for remote mirror",
        CTC_CLI_NH_ID_STR,
        "Truncate length",
        "length")
{
    ctc_mirror_dest_t mirror;
    uint8 index = 0xFF;
    int32 ret  = CLI_SUCCESS;

    sal_memset(&mirror, 0, sizeof(ctc_mirror_dest_t));

    /*parse session*/
    CTC_CLI_GET_UINT8("session", mirror.session_id, argv[0]);

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        mirror.dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        mirror.dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "bo", 2))
    {
        mirror.dir = CTC_BOTH_DIRECTION;
    }

    /*parse session type*/
    if (0 == sal_memcmp(argv[2], "po", 2))
    {
        mirror.type = CTC_MIRROR_L2SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "vl", 2))
    {
        mirror.type = CTC_MIRROR_L3SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "ac", 2))
    {
        mirror.type = CTC_MIRROR_ACLLOG_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "cp", 2))
    {
        mirror.type = CTC_MIRROR_CPU_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "ip", 2))
    {
        mirror.type = CTC_MIRROR_IPFIX_LOG_SESSION;
    }

    /*parse dest_gport */
    index = CTC_CLI_GET_SPECIFIC_INDEX("port", 3);
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", mirror.dest_gport, argv[index + 4],
                                 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("priority-id", mirror.acl_priority, argv[index + 1],
                                0, CTC_MAX_UINT8_VALUE);
        index = 0xFF;
    }

    /*parse special vlan*/
    index = CTC_CLI_GET_SPECIFIC_INDEX("vlan", 4);
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("add vlan id", mirror.rspan.vlan_id, argv[index + 5],
                                 0, CTC_MAX_UINT16_VALUE);
        mirror.is_rspan = 1;
        mirror.vlan_valid = 1;
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("nexthop");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("Nexthop id", mirror.rspan.nh_id, argv[index + 1],
                                     0, CTC_MAX_UINT32_VALUE);
            mirror.is_rspan = 1;
            mirror.vlan_valid = 0;
        }
        else
        {
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("truncated-len", mirror.truncated_len, argv[index + 1]);
        index = 0xFF;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_add_session(g_api_lchip, &mirror);
    }
    else
    {
        ret = ctc_mirror_add_session(&mirror);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_mirror_remove_session,
        ctc_cli_mirror_remove_session_cmd,
        "mirror destination remove session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session | acl-session (priority-id PRIORITY_ID|) | cpu-session | ipfix-log-session)",
        CTC_CLI_MIRROR_M_STR,
        "Mirror destination",
        "Remove mirror destination",
        "Configure a mirror session",
        "Session_id range <0-3>",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Port session",
        "Vlan session",
        "Acl session",
        "Acl priority id",
        CTC_CLI_ACL_PRIORITY_ID_DESC,
        "Cpu session",
        "Ipfix log session")
{
    ctc_mirror_dest_t mirror;
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0xFF;

    sal_memset(&mirror, 0, sizeof(ctc_mirror_dest_t));

    /*parse session*/
    CTC_CLI_GET_UINT8("session", mirror.session_id, argv[0]);

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        mirror.dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        mirror.dir = CTC_EGRESS;
    }
    else
    {
        mirror.dir = CTC_BOTH_DIRECTION;
    }

    /*parse session type*/
    if (0 == sal_memcmp(argv[2], "po", 2))
    {
        mirror.type = CTC_MIRROR_L2SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "vl", 2))
    {
        mirror.type = CTC_MIRROR_L3SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "ac", 2))
    {
        mirror.type = CTC_MIRROR_ACLLOG_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "cp", 2))
    {
        mirror.type = CTC_MIRROR_CPU_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "ip", 2))
    {
        mirror.type = CTC_MIRROR_IPFIX_LOG_SESSION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("priority-id", mirror.acl_priority, argv[index + 1],
                                0, CTC_MAX_UINT8_VALUE);
        index = 0xFF;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_remove_session(g_api_lchip, &mirror);
    }
    else
    {
        ret = ctc_mirror_remove_session(&mirror);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_mirror_add_session,
        ctc_cli_mirror_add_session_cmd,
        "mirror destination add session SESSION_ID direction (ingress|egress|both) (port-session | vlan-session |   \
        acl-session (priority-id PRIORITY_ID|) | cpu-session |ipfix-log-session)  (port GPORT_ID) (truncated-len LEN|)",
        CTC_CLI_MIRROR_M_STR,
        "Mirror destination",
        "Add destination for mirror session",
        "Configure a mirror session",
        "Session_id range <0-3>",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Port session",
        "Vlan session",
        "Acl session",
        "Acl priority id",
        CTC_CLI_ACL_PRIORITY_ID_DESC,
        "Cpu session",
        "Ipfix log session",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "Truncate length",
        "length")
{
    ctc_mirror_dest_t mirror;
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0xFF;

    sal_memset(&mirror, 0, sizeof(ctc_mirror_dest_t));

    /*parse session*/
    CTC_CLI_GET_UINT8("session", mirror.session_id, argv[0]);

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        mirror.dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        mirror.dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "bo", 2))
    {
        mirror.dir = CTC_BOTH_DIRECTION;
    }

    /*parse session type*/
    if (0 == sal_memcmp(argv[2], "po", 2))
    {
        mirror.type = CTC_MIRROR_L2SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "vl", 2))
    {
        mirror.type = CTC_MIRROR_L3SPAN_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "ac", 2))
    {
        mirror.type = CTC_MIRROR_ACLLOG_SESSION;
    }
    else if (0 == sal_memcmp(argv[2], "cp", 2))
    {
        mirror.type = CTC_MIRROR_CPU_SESSION;
    }
    else if((0 == sal_memcmp(argv[2], "ip", 2)))
    {
        mirror.type = CTC_MIRROR_IPFIX_LOG_SESSION;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("priority-id", mirror.acl_priority, argv[index + 1],
                                0, CTC_MAX_UINT8_VALUE);
        index = 0xFF;
    }

    /*parse dest_gport*/
    index = CTC_CLI_GET_SPECIFIC_INDEX("port", 3);
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("gport", mirror.dest_gport, argv[index + 4],
                                 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("truncated-len");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("truncated-len", mirror.truncated_len, argv[index + 1]);
        index = 0xFF;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_add_session(g_api_lchip, &mirror);
    }
    else
    {
        ret = ctc_mirror_add_session(&mirror);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_mirror_set_mirror_discard,
        ctc_cli_mirror_set_mirror_discard_cmd,
        "mirror dropped-pkt (port-session|vlan-session|acl-session|ipfix-log-session |all) direction (ingress|egress|both) (enable | disable)",
        CTC_CLI_MIRROR_M_STR,
        "Packets dropped",
        "Port session",
        "Vlan session",
        "Acl session",
        "Ipfix log session",
        "All port and vlan and acl session",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic",
        "Mirror received and transmitted traffic",
        "Packet enable to log when dropped",
        "Packet disable to log when dropped")
{
    uint16 discard_flag = 0;
    ctc_direction_t dir = 0;
    int32 ret = CLI_SUCCESS;
    bool is_enable = FALSE;

    /*parse log kind*/
    if (0 == sal_memcmp(argv[0], "po", 2))
    {
        discard_flag = CTC_MIRROR_L2SPAN_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "vl", 2))
    {
        discard_flag = CTC_MIRROR_L3SPAN_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "ac", 2))
    {
        discard_flag = CTC_MIRROR_ACLLOG_PRI_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "ip", 2))
    {
        discard_flag = CTC_MIRROR_IPFIX_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "al", 2))
    {
        discard_flag = CTC_MIRROR_L2SPAN_DISCARD | CTC_MIRROR_L3SPAN_DISCARD | CTC_MIRROR_ACLLOG_PRI_DISCARD | CTC_MIRROR_IPFIX_DISCARD;
    }

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        dir = CTC_EGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "bo", 2))
    {
        dir = CTC_BOTH_DIRECTION;
    }

    /*parse enable*/
    if (0 == sal_memcmp(argv[2], "en", 2))
    {
        is_enable = TRUE;
    }
    else if (0 == sal_memcmp(argv[2], "di", 2))
    {
        is_enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_set_mirror_discard(g_api_lchip, dir, discard_flag, is_enable);
    }
    else
    {
        ret = ctc_mirror_set_mirror_discard(dir, discard_flag, is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_mirror_show_mirror_discard,
        ctc_cli_mirror_show_mirror_discard_cmd,
        "show mirror dropped-pkt (port-session|vlan-session|acl-session|ipfix-log-session) direction (ingress|egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MIRROR_M_STR,
        "Packets dropped",
        "Port session",
        "Vlan session",
        "Acl session",
        "Ipfix log session",
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic")
{
    ctc_mirror_discard_t discard_flag = 0;
    ctc_direction_t dir = CTC_BOTH_DIRECTION;
    int32 ret = CLI_SUCCESS;
    bool is_enable = FALSE;

    /*parse log kind*/
    if (0 == sal_memcmp(argv[0], "po", 2))
    {
        discard_flag = CTC_MIRROR_L2SPAN_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "vl", 2))
    {
        discard_flag = CTC_MIRROR_L3SPAN_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "ac", 2))
    {
        discard_flag = CTC_MIRROR_ACLLOG_PRI_DISCARD;
    }
    else if (0 == sal_memcmp(argv[0], "ip", 2))
    {
        discard_flag = CTC_MIRROR_IPFIX_DISCARD;
    }
    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        dir = CTC_INGRESS;
    }
    else if (0 == sal_memcmp(argv[1], "eg", 2))
    {
        dir = CTC_EGRESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_get_mirror_discard(g_api_lchip, dir, discard_flag, &is_enable);
    }
    else
    {
        ret = ctc_mirror_get_mirror_discard(dir, discard_flag, &is_enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("mirror dropped-pkt enable:%d\n", is_enable);

    return ret;
}

CTC_CLI(ctc_cli_mirror_show_port_mirror_state,
        ctc_cli_mirror_show_port_mirror_state_cmd,
        "show mirror port GPHYPORT_ID direction (ingress|egress) ",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MIRROR_M_STR,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic")
{
    uint16 gport;
    uint8 session_id;
    ctc_direction_t dir;
    bool enable;
    int32 ret  = CLI_SUCCESS;

    /*parse port*/
    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        dir = CTC_INGRESS;
    }
    else
    {
        dir = CTC_EGRESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_get_port_info(g_api_lchip, gport, dir, &enable, &session_id);
    }
    else
    {
        ret = ctc_mirror_get_port_info(gport, dir, &enable, &session_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("show gport=0x%.4x mirror state\n", gport);
    ctc_cli_out("===================================\n");
    ctc_cli_out("mirror enable                   :%d\n", enable);

    if (TRUE == enable)
    {
        ctc_cli_out("session_id                      :%d\n", session_id);
    }

    ctc_cli_out("===================================\n");

    return ret;
}

CTC_CLI(ctc_cli_mirror_show_vlan_mirror_state,
        ctc_cli_mirror_show_vlan_mirror_state_cmd,
        "show mirror vlan VLAN_ID direction (ingress|egress)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MIRROR_M_STR,
        CTC_CLI_VLAN_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "The direction of monitored traffic",
        "Mirror received traffic",
        "Mirror transmitted traffic")
{
    uint16 vlan_id;
    uint8 session_id;
    ctc_direction_t dir;
    bool enable;
    int32 ret  = CLI_SUCCESS;

    /*parse vlan*/
    CTC_CLI_GET_UINT16("vlan", vlan_id, argv[0]);

    /*parse direction*/
    if (0 == sal_memcmp(argv[1], "in", 2))
    {
        dir = CTC_INGRESS;
    }
    else
    {
        dir = CTC_EGRESS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_get_vlan_info(g_api_lchip, vlan_id, dir, &enable, &session_id);
    }
    else
    {
        ret = ctc_mirror_get_vlan_info(vlan_id, dir, &enable, &session_id);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("show vlan=%d mirror state\n", vlan_id);
    ctc_cli_out("===================================\n");
    ctc_cli_out("mirror enable                   :%d\n", enable);

    if (TRUE == enable)
    {
        ctc_cli_out("session_id                      :%d\n", session_id);
    }

    ctc_cli_out("===================================\n");

    return ret;
}

CTC_CLI(ctc_cli_mirror_debug_on,
        ctc_cli_mirror_debug_on_cmd,
        "debug mirror (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MIRROR_M_STR,
        "Ctc layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
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

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MIRROR_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MIRROR_SYS;
    }

    ctc_debug_set_flag("mirror", "mirror", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mirror_debug_off,
        ctc_cli_mirror_debug_off_cmd,
        "no debug mirror (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MIRROR_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MIRROR_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MIRROR_SYS;
    }

    ctc_debug_set_flag("mirror", "mirror", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mirror_rspan_escape_en,
        ctc_cli_mirror_rspan_escape_en_cmd,
        "mirror rspan escape (enable | disable)",
        CTC_CLI_MIRROR_M_STR,
        "Remote mirror",
        "Discard some special mirrored packet",
        "Enable",
        "Disable")
{
    int32 ret = CLI_SUCCESS;

    if (0 == sal_memcmp(argv[0], "enable", sal_strlen("enable")))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_escape_en(g_api_lchip, TRUE);
        }
        else
        {
            ret = ctc_mirror_set_escape_en(TRUE);
        }
    }
    else if (0 == sal_memcmp(argv[0], "disable", sal_strlen("disable")))
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_mirror_set_escape_en(g_api_lchip, FALSE);
        }
        else
        {
            ret = ctc_mirror_set_escape_en(FALSE);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_mirror_rspan_escape_mac,
        ctc_cli_mirror_rspan_escape_mac_cmd,
        "mirror rspan escape {mac1 MAC (mask1 MASK |) | mac2 MAC (mask2 MASK |)}",
        CTC_CLI_MIRROR_M_STR,
        "Remote mirror",
        "Discard some special mirrored packet",
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Mac1 mask",
        CTC_CLI_MAC_MASK_FORMAT,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Mac2 mask",
        CTC_CLI_MAC_MASK_FORMAT)
{
    int32 ret = CLI_SUCCESS;
    uint8 index_mac = 0;
    uint8 index_mac_mask = 0;
    mac_addr_t macsa;
    mac_addr_t macsa_mask;
    ctc_mirror_rspan_escape_t escape;

    sal_memset(macsa, 0, sizeof(mac_addr_t));
    sal_memset(macsa_mask, 0, sizeof(mac_addr_t));
    sal_memset(&escape, 0, sizeof(ctc_mirror_rspan_escape_t));
    sal_memset(escape.mac_mask0, 0xFF, sizeof(mac_addr_t));
    sal_memset(escape.mac_mask1, 0xFF, sizeof(mac_addr_t));

    index_mac = CTC_CLI_GET_ARGC_INDEX("mac1");
    if (index_mac != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index_mac + 1]);

        sal_memcpy(&(escape.mac0), &macsa, sizeof(mac_addr_t));
        sal_memset(escape.mac_mask0, 0xFF, sizeof(mac_addr_t));
    }

    index_mac_mask = CTC_CLI_GET_ARGC_INDEX("mask1");
    if (index_mac_mask != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa mask", macsa_mask, argv[index_mac_mask + 1]);

        sal_memcpy(&(escape.mac_mask0), &macsa_mask, sizeof(mac_addr_t));
    }

    sal_memset(macsa, 0, sizeof(mac_addr_t));
    index_mac = CTC_CLI_GET_ARGC_INDEX("mac2");
    if (index_mac != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", macsa, argv[index_mac + 1]);

        sal_memcpy(&(escape.mac1), &macsa, sizeof(mac_addr_t));
        sal_memset(escape.mac_mask1, 0xFF, sizeof(mac_addr_t));
    }

    sal_memset(macsa_mask, 0, sizeof(mac_addr_t));
    index_mac_mask = CTC_CLI_GET_ARGC_INDEX("mask2");
    if (index_mac_mask != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa mask", macsa_mask, argv[index_mac_mask + 1]);

        sal_memcpy(&(escape.mac_mask1), &macsa_mask, sizeof(mac_addr_t));
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_set_escape_mac(g_api_lchip, &escape);
    }
    else
    {
        ret = ctc_mirror_set_escape_mac(&escape);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_mirror_set_erspan_psc,
        ctc_cli_mirror_set_erspan_psc_cmd,
        "mirror erspan psc (ipv4|ipv6) (disable | {ip-addr | (l4-port (is-tcp |is-udp|is-dccp|is-sctp))})",
        CTC_CLI_MIRROR_M_STR,
        "ERSPAN",
        "Port Selection Criteria",
        "Ipv4 packet",
        "Ipv6 packet",
        "Disable",
        "IP address",
        "L4 port",
        "Is TCP packet",
        "Is UDP packet",
        "Is DCCP packet",
        "Is SCTP packet")
{
    int32 ret = 0;
    ctc_mirror_erspan_psc_t psc;
    sal_memset(&psc, 0, sizeof(psc));

    if (CLI_CLI_STR_EQUAL("ipv4", 0))
    {
        psc.type = CTC_MIRROR_ERSPAN_PSC_TYPE_IPV4;
        if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("disable")))
        {
            psc.ipv4_flag = 0;
        }
        else
        {
            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("ip-addr")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IP_ADDR;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("l4-port")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_L4_PORT;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-tcp")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_TCP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-udp")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_UDP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-dccp")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_DCCP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-sctp")))
            {
                psc.ipv4_flag |= CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_SCTP;
            }
        }

    }
    else
    {
        psc.type = CTC_MIRROR_ERSPAN_PSC_TYPE_IPV6;
        if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("disable")))
        {
            psc.ipv6_flag = 0;
        }
        else
        {
            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("ip-addr")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IP_ADDR;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("l4-port")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_L4_PORT;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-tcp")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_TCP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-udp")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_UDP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-dccp")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_DCCP;
            }

            if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("is-sctp")))
            {
                psc.ipv6_flag |= CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_SCTP;
            }
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_mirror_set_erspan_psc(g_api_lchip, &psc);
    }
    else
    {
        ret = ctc_mirror_set_erspan_psc(&psc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}


CTC_CLI(ctc_cli_mirror_show_debug,
        ctc_cli_mirror_show_debug_cmd,
        "show debug mirror (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MIRROR_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = MIRROR_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = MIRROR_SYS;
    }

    en = ctc_debug_get_flag("mirror", "mirror", typeenum, &level);
    ctc_cli_out("mirror:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

int32
ctc_mirror_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_set_port_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_set_vlan_enable_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_add_session_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_add_rspan_session_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_remove_session_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_set_mirror_discard_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_rspan_escape_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_rspan_escape_mac_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_set_erspan_psc_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_mirror_show_port_mirror_state_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_show_vlan_mirror_state_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_show_mirror_discard_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_mirror_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_mirror_show_debug_cmd);

    return 0;
}

