/**
 @file ctc_overlay_tunnel_cli.c

 @date 2013-11-10

 @version v2.0

 The file apply clis of overlay tunnel module
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_overlay_tunnel.h"
#include "ctc_overlay_tunnel_cli.h"

#define CTC_CLI_OVERLAY_TUNNEL_DES      "overlay tunnel module, used to terminate an overlay tunnel"
#define CTC_CLI_OVERLAY_TUNNEL_VNID_DES "overlay tunnel virtual subnet ID"
#define CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE "virtual subnet ID"
#define CTC_CLI_OVERLAY_TUNNEL_ACL_STR \
    "acl-property {\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|\
    priority PRIORITY lookup-type LKP_TYPE {class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan|}|}"

#define CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC \
    "Acl priority",\
    "priority value",\
    "Acl lookup type",\
    "type value",\
    "acl use class id",\
    "CLASS_ID",\
    "acl use port-bitmap",\
    "acl use metadata",\
    "acl use logic-port",\
    "acl use mapped vlan"

#define CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_SET() \
index = CTC_CLI_GET_ARGC_INDEX("acl-property");\
if (index != 0xFF)\
{\
    index = CTC_CLI_GET_ARGC_INDEX("priority");\
    if (0xFF != index)\
    {\
        idx1 = index;\
        idx2 = 0;\
        while(last_time < 1)\
        {\
            idx2 = ctc_cli_get_prefix_item(&argv[idx1+1], argc-(idx1+1), "priority", 0);\
            if(0xFF == idx2)\
            {\
                last_time = 1;\
                idx2 = argc-1-idx1-1;\
            }\
            CTC_CLI_GET_UINT8("priority", acl_property[cnt].acl_priority, argv[idx1 + 1]);\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "lookup-type", 0);\
            if (0xFF != index)\
            {\
                CTC_CLI_GET_UINT8("lookup-type", acl_property[cnt].tcam_lkup_type, argv[idx1 + index + 2]);\
            }\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "class-id", 0);\
            if (0xFF != index)\
            {\
                CTC_CLI_GET_UINT8("class-id", acl_property[cnt].class_id, argv[idx1 + index + 2]);\
            }\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-logic-port", 0);\
            if(0xFF != index)\
            {\
                CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);\
            }\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-port-bitmap", 0);\
            if(0xFF != index)\
            {\
                CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);\
            }\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-metadata", 0);\
            if(0xFF != index)\
            {\
                CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_METADATA);\
            }\
            index = ctc_cli_get_prefix_item(&argv[idx1+1],idx2+1, "use-mapped-vlan", 0);\
            if(0xFF != index)\
            {\
                CTC_SET_FLAG(acl_property[cnt].flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);\
            }\
            cnt++;\
            idx1 = idx1 + idx2 + 1;\
        }\
    }\
}

CTC_CLI(ctc_cli_overlay_tunnel_set_fid,
        ctc_cli_overlay_tunnel_set_fid_cmd,
        "overlay-tunnel vn-id VN_ID fid FID",
        CTC_CLI_OVERLAY_TUNNEL_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC)
{
    uint32 vn_id = 0;
    uint16 fid = 0;
    int32 ret = CLI_ERROR;

    CTC_CLI_GET_UINT32("vn-id", vn_id, argv[0]);
    CTC_CLI_GET_UINT16("fid", fid, argv[1]);
    if(g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_set_fid(g_api_lchip, vn_id, fid);
    }
    else
    {
        ret = ctc_overlay_tunnel_set_fid(vn_id, fid);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_overlay_tunnel_get_fid,
        ctc_cli_overlay_tunnel_get_fid_cmd,
        "show overlay-tunnel fid vn-id VN_ID",
        "show",
        CTC_CLI_OVERLAY_TUNNEL_DES,
        CTC_CLI_FID_DESC,
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE)
{
    uint32 vn_id = 0;
    uint16 fid = 0;
    int32 ret = CLI_ERROR;

    CTC_CLI_GET_UINT32("vn-id", vn_id, argv[0]);
    if(g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_get_fid(g_api_lchip, vn_id, &fid);
    }
    else
    {
        ret = ctc_overlay_tunnel_get_fid(vn_id, &fid);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%% %s %d \n\r", "FID of this vn is",fid);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_overlay_tunnel_get_vnid,
        ctc_cli_overlay_tunnel_get_vnid_cmd,
        "show overlay-tunnel vn-id fid FID",
        "show",
        CTC_CLI_OVERLAY_TUNNEL_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC)
{
    uint32 vn_id = 0;
    uint16 fid = 0;
    int32 ret = CLI_ERROR;

    CTC_CLI_GET_UINT16("fid", fid, argv[0]);
    if(g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_get_vn_id(g_api_lchip, fid, &vn_id);
    }
    else
    {
        ret = ctc_overlay_tunnel_get_vn_id(fid, &vn_id);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("%% %s %d \n\r", "vnid of this fid is", vn_id);

    return CLI_SUCCESS;
}


#define CTC_CLI_OVERLAY_PARAM_STR \
    "({vrf VRFID | tunnel-src-port PORTID | metadata METADATA | ttl-check | use-outer-ttl | acl-use-outer | acl-follow-port | ipfix-use-outer | ipfix-follow-port | qos-use-outer-info | qos-follow-port | \
     stats STATS_ID | policer-id POLICER_ID | service-acl-en | inner-taged-chk-mode MODE | acl-tcam-en tcam-type TCAM_TYPE tcam-label TCAM_LABEL | \
     acl-hash-en hash-type HASH_TYPE sel-id FIELD_SEL_ID | overwrite-acl-tcam | overwrite-acl-hash | use-flex | scl-id SCL_ID | tunnel-port-type TYPE \
     | inner-router-mac MAC|deny-learning | arp-action (copy-to-cpu|forward|redirect-to-cpu|discard)| mac-limit-action (none|forward|discard|redirect-to-cpu) \
     | station-move-discard-en | aware-tunnel-info-en | dot1ae-en DOT1AE_CHANNEL_ID } |)("CTC_CLI_OVERLAY_TUNNEL_ACL_STR" | )"

#define CTC_CLI_OVERLAY_PARAM_DESC \
        "Add overlay tunnel terminate entry",\
        "Update overlay tunnel terminate entry",\
        "This is a NvGRE type tunnel",\
        "This is a VxLAN type tunnel",\
        "This is a VxLAN-GPE type tunnel",\
        "This is a GENEVE type tunnel",\
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,\
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE,\
        "destination ip address",\
        "destination ip address",\
        "source ip address",\
        "source ip address",\
        "Default entry",\
        "Set svlan tpid index",\
        "Index, the corresponding svlan tpid is in EpeL2TpidCtl",\
        "destination overlay tunnel virtual subnet ID",\
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE,\
        CTC_CLI_NH_ID_STR,\
        CTC_CLI_NH_ID_STR,\
        CTC_CLI_FID_DESC,\
        CTC_CLI_FID_ID_DESC,\
        "Inner route will lookup by this VRFID",\
        CTC_CLI_VRFID_ID_DESC,\
        "Overlay tunnel source port",\
        "Overlay tunnel source port , 0 means not used",\
        "Metadata",\
        "metadata: <0-0x3FFF>",\
        "Do ttl check for tunnel outer header",\
        "Use outer ttl for later process , or use inner ttl",\
        "Use outer header info do acl lookup, default use the inner",\
        "Use port config do acl lookup",\
        "Use outer header info do ipfix lookup, default use the inner",\
        "Use port config do ipfix lookup",\
        "Use outer dscp/cos do qos classification, default use inner",\
        "Use port config do qos classification",\
        "Statistic",\
        CTC_CLI_STATS_ID_VAL,\
        "Policer Id",\
        "<1-0xFFFF>",\
        "Enable service acl",\
        "Check mode for inner packet with vlan tag",\
        "Check mode, 0:No check; 1:discard; 2:forward to CPU; 3:copy to CPU",\
        "Set ACL Tcam lookup property",\
        "Tcam type",\
        "Tcam type value",\
        "Tcam label",\
        "<0-0xFF>",\
        "Set ACL hash lookup property",\
        "Hash type",\
        CTC_CLI_ACL_HASH_TYPE_VALUE,\
        "Field Select Id",\
        "<0-15>",\
        "Overwrite acl tcam",\
        "Overwrite acl hash",\
        "Use flex key",\
        "Scl ID",\
        CTC_CLI_SCL_ID_VALUE,\
        "Tunnel port type", \
        "1: do split horizon;0: not do split horizon",\
        "Router mac for Inner L2 MacDa",\
        CTC_CLI_MAC_FORMAT, \
        "Deny learning",\
        "ARP packet action", \
        "Forwarding and to cpu", \
        "Normal forwarding", \
        "Redirect to cpu", \
        "Discard", \
        "Mac limit action",\
        "None",\
        "Normal forwarding",\
        "Discard",\
        "Redirect to cpu",\
        "Station move discard",\
        "Aware tunnel info en",\
        "Enabel Dot1ae",\
        "Dot1ae channel id",\
        "Acl property", \
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC,\
        CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_DESC

CTC_CLI(ctc_cli_overlay_tunnel_add,
        ctc_cli_overlay_tunnel_add_cmd,
        "overlay-tunnel (add|update) (nvgre|vxlan|vxlan-gpe|geneve) (src-vnid VNID|) (ipda A.B.C.D) (ipsa A.B.C.D |) {default-entry||tpid-index TPID_INDEX|} (dst-vnid VNID | nhid NHID | fid FID) \
     "CTC_CLI_OVERLAY_PARAM_STR,
        CTC_CLI_OVERLAY_TUNNEL_DES,
          CTC_CLI_OVERLAY_PARAM_DESC)
{

    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM] = {{0}};

    ctc_overlay_tunnel_param_t tunnel_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;
    uint8 is_update = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0;
    uint8 idx2 = 0;
    uint8 last_time = 0;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        is_update = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_GENEVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("src-vnid", tunnel_param.src_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", tunnel_param.ipda.ipv4, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", tunnel_param.ipsa.ipv4, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dst-vnid", tunnel_param.action.dst_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nhid", tunnel_param.action.nh_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("fid", tunnel_param.fid, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_FID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vrfid", tunnel_param.vrf_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_VRF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-src-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("tunnel-src-port", tunnel_param.logic_src_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("metadata", tunnel_param.metadata, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl-check");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-outer-ttl");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_OUTER_TTL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT;
    }


    index = CTC_CLI_GET_ARGC_INDEX("ipfix-use-outer");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_BY_OUTER_HEAD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipfix-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_QOS_USE_OUTER_INFO;
    }
    index = CTC_CLI_GET_ARGC_INDEX("qos-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_QOS_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("stats-id", tunnel_param.stats_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("policer-id", tunnel_param.policer_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_SERVICE_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-acl-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("inner-taged-chk-mode");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("check-mode", tunnel_param.inner_taged_chk_mode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-tcam-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN;
        index = CTC_CLI_GET_ARGC_INDEX("tcam-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-type", tunnel_param.acl_tcam_lookup_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("tcam-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-label", tunnel_param.acl_tcam_label, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-tcam");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_TCAM_LKUP_PROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-hash-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN;
        index = CTC_CLI_GET_ARGC_INDEX("hash-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("hash-type", tunnel_param.acl_hash_lookup_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("sel-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("sel-id", tunnel_param.field_sel_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-hash");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_HASH_LKUP_PROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("default-entry");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tpid-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tpid-index", tunnel_param.tpid_index, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_param.scl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-port-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel-port-type", tunnel_param.logic_port_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("inner-router-mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", tunnel_param.route_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_DENY_LEARNING);
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_ARP_ACTIOIN);

        if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_NORMAL_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("redirect-to-cpu", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_DISCARD_AND_TO_CPU;
        }
        else if (CLI_CLI_STR_EQUAL("copy-to-cpu", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_FWD_AND_TO_CPU;
        }
        else
        {
            tunnel_param.arp_action = CTC_EXCP_DISCARD;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_MAC_LIMIT_ACTION);
        if (CLI_CLI_STR_EQUAL("none", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("discard", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_DISCARD;
        }
        else
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_TOCPU;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("station-move-discard-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_STATION_MOVE_DISCARD_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_AWARE_TUNNEL_INFO_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dot1ae-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dot1ae channel id", tunnel_param.dot1ae_chan_id, argv[index + 1]);
    }

    CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_SET()
    if ((tunnel_param.acl_lkup_num = cnt) > 0)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN);
        sal_memcpy(tunnel_param.acl_prop, acl_property, sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
    }


    if (is_update)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_overlay_tunnel_update_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_overlay_tunnel_update_tunnel(&tunnel_param);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_overlay_tunnel_add_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_overlay_tunnel_add_tunnel(&tunnel_param);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_overlay_tunnel_add_v6,
        ctc_cli_overlay_tunnel_add_v6_cmd,
        "overlay-tunnel ipv6 (add|update) (nvgre|vxlan|vxlan-gpe|geneve) (src-vnid VNID|) (ipda X:X::X:X) (ipsa X:X::X:X |) {default-entry|tpid-index TPID_INDEX|} (dst-vnid VNID | nhid NHID | fid FID) \
     "CTC_CLI_OVERLAY_PARAM_STR,
          CTC_CLI_OVERLAY_TUNNEL_DES,
          "IPv6",
          CTC_CLI_OVERLAY_PARAM_DESC)
{

    ctc_acl_property_t acl_property[CTC_MAX_ACL_LKUP_NUM] = {{0}};

    ctc_overlay_tunnel_param_t tunnel_param;
    uint32 index = 0xFF;
    ipv6_addr_t ipv6_address;
    int32 ret = CLI_ERROR;
    uint8 is_update = 0;
    uint8 cnt = 0;
    uint8 idx1 = 0;
    uint8 idx2 = 0;
    uint8 last_time = 0;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));

    index = CTC_CLI_GET_ARGC_INDEX("update");
    if (0xFF != index)
    {
        is_update = 1;
    }

    tunnel_param.flag = CTC_OVERLAY_TUNNEL_FLAG_IP_VER_6;
    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_GENEVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("src-vnid", tunnel_param.src_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index + 1]);
        tunnel_param.ipda.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_param.ipda.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_param.ipda.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_param.ipda.ipv6[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[index + 1]);
        /* adjust endian */
        tunnel_param.ipsa.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_param.ipsa.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_param.ipsa.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_param.ipsa.ipv6[3] = sal_htonl(ipv6_address[3]);
        tunnel_param.flag        |= CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dst-vnid", tunnel_param.action.dst_vn_id, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nhid", tunnel_param.action.nh_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("fid", tunnel_param.fid, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_FID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vrfid", tunnel_param.vrf_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_VRF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-src-port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("tunnel-src-port", tunnel_param.logic_src_port, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("metadata", tunnel_param.metadata, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_METADATA_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl-check");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-outer-ttl");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_OUTER_TTL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipfix-use-outer");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_BY_OUTER_HEAD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipfix-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_IPFIX_LKUP_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_QOS_USE_OUTER_INFO;
    }
    index = CTC_CLI_GET_ARGC_INDEX("qos-follow-port");
    if (0xFF != index)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_QOS_FOLLOW_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("stats-id", tunnel_param.stats_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("policer-id", tunnel_param.policer_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_SERVICE_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("service-acl-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_SERVICE_ACL_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("inner-taged-chk-mode");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("check-mode", tunnel_param.inner_taged_chk_mode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-tcam-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN;
        index = CTC_CLI_GET_ARGC_INDEX("tcam-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-type", tunnel_param.acl_tcam_lookup_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("tcam-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-label", tunnel_param.acl_tcam_label, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-tcam");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_TCAM_LKUP_PROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-hash-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_HASH_EN;
        index = CTC_CLI_GET_ARGC_INDEX("hash-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("hash-type", tunnel_param.acl_hash_lookup_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("sel-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("sel-id", tunnel_param.field_sel_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("overwrite-acl-hash");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OVERWRITE_PORT_HASH_LKUP_PROP;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("default-entry");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tpid-index");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tpid-index", tunnel_param.tpid_index, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_param.scl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-port-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("tunnel-port-type", tunnel_param.logic_port_type, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("inner-router-mac");
    if (0xFF != index)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac address", tunnel_param.route_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_DENY_LEARNING);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-limit-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_MAC_LIMIT_ACTION);
        if (CLI_CLI_STR_EQUAL("none", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_NONE;
        }
        else if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("discard", index+1))
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_DISCARD;
        }
        else
        {
            tunnel_param.limit_action = CTC_MACLIMIT_ACTION_TOCPU;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("station-move-discard-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_STATION_MOVE_DISCARD_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_AWARE_TUNNEL_INFO_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dot1ae-en");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dot1ae channel id", tunnel_param.dot1ae_chan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_ARP_ACTIOIN);

        if (CLI_CLI_STR_EQUAL("forward", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_NORMAL_FWD;
        }
        else if (CLI_CLI_STR_EQUAL("redirect-to-cpu", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_DISCARD_AND_TO_CPU;
        }
        else if (CLI_CLI_STR_EQUAL("copy-to-cpu", index+1))
        {
            tunnel_param.arp_action = CTC_EXCP_FWD_AND_TO_CPU;
        }
        else
        {
            tunnel_param.arp_action = CTC_EXCP_DISCARD;
        }
    }

    CTC_CLI_OVERLAY_TUNNEL_ACL_PARAM_SET()
    if ((tunnel_param.acl_lkup_num = cnt) > 0)
    {
        CTC_SET_FLAG(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN);
        sal_memcpy(tunnel_param.acl_prop, acl_property, sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
    }


    if (is_update)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_overlay_tunnel_update_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_overlay_tunnel_update_tunnel(&tunnel_param);
        }
    }
    else
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_overlay_tunnel_add_tunnel(g_api_lchip, &tunnel_param);
        }
        else
        {
            ret = ctc_overlay_tunnel_add_tunnel(&tunnel_param);
        }
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_overlay_tunnel_remove,
        ctc_cli_overlay_tunnel_remove_cmd,
        "overlay-tunnel del (nvgre|vxlan|vxlan-gpe|geneve) (src-vnid VNID|) (ipda A.B.C.D) (ipsa A.B.C.D |) (default-entry|) ({use-flex | scl-id SCL_ID} | )",
        CTC_CLI_OVERLAY_TUNNEL_DES,
        "Delete overlay tunnel terminate entry",
        "This is a NvGRE type tunnel",
        "This is a VxLAN type tunnel",
        "This is a VxLAN-GPE type tunnel",
        "This is a GENEVE type tunnel",
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE,
        "destination ip address",
        CTC_CLI_IPV4_FORMAT,
        "source ip address",
        CTC_CLI_IPV4_FORMAT,
        "Default entry",
        "Use flex key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE)
{
    ctc_overlay_tunnel_param_t tunnel_param;
    uint32 index = 0xFF;
    int32 ret = CLI_ERROR;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));
    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_GENEVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("src-vnid", tunnel_param.src_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", tunnel_param.ipda.ipv4, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", tunnel_param.ipsa.ipv4, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("default-entry");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_param.scl_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_remove_tunnel(g_api_lchip, &tunnel_param);
    }
    else
    {
        ret = ctc_overlay_tunnel_remove_tunnel(&tunnel_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_overlay_tunnel_remove_v6,
        ctc_cli_overlay_tunnel_remove_v6_cmd,
        "overlay-tunnel ipv6 del (nvgre|vxlan|vxlan-gpe|geneve) (src-vnid VNID|) (ipda X:X::X:X) (ipsa X:X::X:X |) (default-entry|) ({use-flex | scl-id SCL_ID} | )",
        CTC_CLI_OVERLAY_TUNNEL_DES,
        CTC_CLI_IPV6_STR,
        "Delete overlay tunnel terminate entry",
        "This is a NvGRE type tunnel",
        "This is a VxLAN type tunnel",
        "This is a VxLAN-GPE type tunnel",
        "This is a GENEVE type tunnel",
        CTC_CLI_OVERLAY_TUNNEL_VNID_DES,
        CTC_CLI_OVERLAY_TUNNEL_VNID_RANGE,
        "destination ip address",
        CTC_CLI_IPV6_FORMAT,
        "source ip address",
        CTC_CLI_IPV6_FORMAT,
        "Default entry",
        "Use flex key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE)
{
    ctc_overlay_tunnel_param_t tunnel_param;
    uint32 index = 0xFF;
    ipv6_addr_t ipv6_address;
    int32 ret = CLI_ERROR;

    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));
    tunnel_param.flag = CTC_OVERLAY_TUNNEL_FLAG_IP_VER_6;
    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_GENEVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("src-vnid", tunnel_param.src_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipda");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index + 1]);
        /* adjust endian */
        tunnel_param.ipda.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_param.ipda.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_param.ipda.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_param.ipda.ipv6[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[index + 1]);
        /* adjust endian */
        tunnel_param.ipsa.ipv6[0] = sal_htonl(ipv6_address[0]);
        tunnel_param.ipsa.ipv6[1] = sal_htonl(ipv6_address[1]);
        tunnel_param.ipsa.ipv6[2] = sal_htonl(ipv6_address[2]);
        tunnel_param.ipsa.ipv6[3] = sal_htonl(ipv6_address[3]);
        tunnel_param.flag         |= CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("default-entry");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_DEFAULT_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("use-flex");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("scl-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("scl id", tunnel_param.scl_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_overlay_tunnel_remove_tunnel(g_api_lchip, &tunnel_param);
    }
    else
    {
        ret = ctc_overlay_tunnel_remove_tunnel(&tunnel_param);
    }

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_overlay_tunnel_debug_on,
        ctc_cli_overlay_tunnel_debug_on_cmd,
        "debug overlay (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_OVERLAY_M_STR,
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
    else
    {
        level = CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = OVERLAY_TUNNEL_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = OVERLAY_TUNNEL_SYS;
    }

    ctc_debug_set_flag("overlay_tunnel", "overlay_tunnel", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_overlay_tunnel_debug_off,
        ctc_cli_overlay_tunnel_debug_off_cmd,
        "no debug overlay (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_OVERLAY_M_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = OVERLAY_TUNNEL_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = OVERLAY_TUNNEL_SYS;
    }

    ctc_debug_set_flag("overlay_tunnel", "overlay_tunnel", typeenum, level, FALSE);

    return CLI_SUCCESS;
}


int32
ctc_overlay_tunnel_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_set_fid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_get_fid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_get_vnid_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_add_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_add_v6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_remove_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_remove_v6_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_overlay_tunnel_debug_off_cmd);

    return CLI_SUCCESS;
}

