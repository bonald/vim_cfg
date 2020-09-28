/**
 @date 2009-12-22

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_scl_cli.h"
#include "ctc_usw_scl_cli.h"
#include "ctc_mirror.h"

typedef enum        /*Keep this enum same with sys_scl_key_type_t*/
{
    CTC_CLI_SCL_KEY_TCAM_VLAN,
    CTC_CLI_SCL_KEY_TCAM_MAC,
    CTC_CLI_SCL_KEY_TCAM_IPV4,
    CTC_CLI_SCL_KEY_TCAM_IPV4_SINGLE,
    CTC_CLI_SCL_KEY_TCAM_IPV6,
    CTC_CLI_SCL_KEY_TCAM_IPV6_SINGLE,
    CTC_CLI_SCL_KEY_TCAM_UDF,       /**< [TM] udf key 160 */
    CTC_CLI_SCL_KEY_TCAM_UDF_EXT,       /**< [TM] udf key 320 */
    CTC_CLI_SCL_KEY_HASH_PORT,
    CTC_CLI_SCL_KEY_HASH_PORT_CVLAN,
    CTC_CLI_SCL_KEY_HASH_PORT_SVLAN,
    CTC_CLI_SCL_KEY_HASH_PORT_2VLAN,
    CTC_CLI_SCL_KEY_HASH_PORT_CVLAN_COS,
    CTC_CLI_SCL_KEY_HASH_PORT_SVLAN_COS,
    CTC_CLI_SCL_KEY_HASH_MAC,
    CTC_CLI_SCL_KEY_HASH_PORT_MAC,
    CTC_CLI_SCL_KEY_HASH_IPV4,
    CTC_CLI_SCL_KEY_HASH_PORT_IPV4,
    CTC_CLI_SCL_KEY_HASH_IPV6,
    CTC_CLI_SCL_KEY_HASH_L2,
    CTC_CLI_SCL_KEY_HASH_PORT_IPV6,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_GRE,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_RPF,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_DA,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV6_DA,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV6,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV6_UDP,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY,
    CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V4_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V4_MODE1,  /* ipdaindex + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V4_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_V4_MODE1,     /* used by both uc and mc, ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V6_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V6_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V4_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V4_MODE1,  /* ipdaindex + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V4_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_V4_MODE1,     /* used by both uc and mc, ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V6_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V6_MODE0,  /* ipda + vn_id*/
    CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V6_MODE1,  /* ipda + ipsa + vn_id*/
    CTC_CLI_SCL_KEY_HASH_WLAN_IPV4,
    CTC_CLI_SCL_KEY_HASH_WLAN_IPV6,
    CTC_CLI_SCL_KEY_HASH_WLAN_RMAC,
    CTC_CLI_SCL_KEY_HASH_WLAN_RMAC_RID,
    CTC_CLI_SCL_KEY_HASH_WLAN_STA_STATUS,
    CTC_CLI_SCL_KEY_HASH_WLAN_MC_STA_STATUS,
    CTC_CLI_SCL_KEY_HASH_WLAN_VLAN_FORWARD,
    CTC_CLI_SCL_KEY_HASH_WLAN_MACDA_FORWARD,
    CTC_CLI_SCL_KEY_HASH_TUNNEL_MPLS,
    CTC_CLI_SCL_KEY_HASH_ECID,
    CTC_CLI_SCL_KEY_HASH_ING_ECID,
    CTC_CLI_SCL_KEY_HASH_PORT_CROSS,
    CTC_CLI_SCL_KEY_HASH_TRILL_UC,          /* ingressNickname + egressNickname */
    CTC_CLI_SCL_KEY_HASH_TRILL_UC_RPF,      /* ingressNickname */
    CTC_CLI_SCL_KEY_HASH_TRILL_MC,          /* ingressNickname + egressNickname */
    CTC_CLI_SCL_KEY_HASH_TRILL_MC_RPF,      /* ingressNickname + egressNickname */
    CTC_CLI_SCL_KEY_HASH_TRILL_MC_ADJ,      /* globalSrcPort + egressNickname */
    CTC_CLI_SCL_KEY_PORT_DEFAULT_INGRESS,
    CTC_CLI_SCL_KEY_PORT_DEFAULT_EGRESS,
    CTC_CLI_SCL_KEY_NUM
}ctc_cli_scl_key_type_t;


/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
extern int32
sys_usw_scl_show_status(uint8 lchip);
extern int32
sys_usw_scl_show_entry(uint8 lchip, uint8 type, uint32 param, uint32 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt);
extern int32
sys_usw_scl_show_tunnel_entry(uint8 lchip, uint8 key_type, uint8 detail);
extern int32
sys_usw_scl_show_tcam_alloc_status(uint8 lchip, uint8 block_id);
extern int32
sys_usw_scl_show_group_status(uint8 lchip);

extern int32
sys_usw_scl_show_spool(uint8 lchip, uint8 type);



CTC_CLI(ctc_cli_usw_scl_show_status,
        ctc_cli_usw_scl_show_status_cmd,
        "show scl status (lchip LCHIP|)",
        "Show",
        CTC_CLI_SCL_STR,
        "Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_status(lchip);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_usw_scl_show_spool,
        ctc_cli_usw_scl_show_spool_cmd,
        "show scl spool (vlan-edit|acl) (lchip LCHIP|)",
        "Show",
        CTC_CLI_SCL_STR,
        "Spool",
        "Vlan edit profile",
        "Acl profile",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 type = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");
    if (0xFF != index)
    {
        type = 0;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_spool(lchip, type);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_usw_scl_show_tunnel_entry,
        ctc_cli_usw_scl_show_tunnel_entry_cmd,
        "show tunnel entry (lchip LCHIP|)",
        "Show",
        "Tunnel",
        "Entry",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_tunnel_entry(lchip, 0, 0);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_usw_scl_show_entry_info,
        ctc_cli_usw_scl_show_entry_info_cmd,
        "show scl entry-info \
        (all   | entry ENTRY_ID |group GROUP_ID |priority PRIORITY) \
        (type (tcam-mac|tcam-ipv4 | tcam-ipv4-single | tcam-ipv6 | tcam-ipv6-single | tcam-udf | tcam-udf-ext | hash-port|  hash-port-cvlan | \
        hash-port-svlan | hash-port-2vlan | hash-port-cvlan-cos| \
        hash-port-svlan-cos | hash-mac | hash-port-mac | hash-ipv4 | hash-port-ipv4 | hash-ipv6| hash-port-ipv6 |hash-l2 | \
        hash-tunnel-ipv4 | hash-tunnel-ipv4-gre | hash-tunnel-ipv4-rpf | hash-tunnel-ipv4-da | hash-nvgre-v4-uc-mode0 | \
        hash-nvgre-v4-uc-mode1 | hash-nvgre-v4-mc-mode0 | hash-nvgre-v4-mode1 | hash-nvgre-v6-uc-mode0 | \
        hash-nvgre-v6-uc-mode1 | hash-nvgre-v6-mc-mode0 | hash-nvgre-v6-mc-mode1 | hash-vxlan-v4-uc-mode0 | \
        hash-vxlan-v4-uc-mode1 | hash-vxlan-v4-mc-mode0 | hash-vxlan-v4-mode1 | hash-vxlan-v6-uc-mode0 | \
        hash-vxlan-v6-uc-mode1 | hash-vxlan-v6-mc-mode0 | hash-vxlan-v6-mc-mode1 | hash-trill-uc | hash-trill-mc) |) \
        (grep {"CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_STR"} |) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SCL_STR,
        "Entry info",
        "All entries",
        "By entry",
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "By group",
        CTC_CLI_SCL_MAX_GROUP_ID_VALUE,
        "By priority",
        CTC_CLI_SCL_GROUP_PRIO_VALUE,
        "By type",
        "SCL Mac-entry",
        "SCL Ipv4-entry",
        "SCL Ipv4-entry(Single width)",
        "SCL Ipv6-entry",
        "SCL Ipv6-entry(Single width)",
        "SCL UDF-entry",
        "SCL UDF-ext-entry",
        "SCL Hash-port-entry",
        "SCL Hash-port-cvlan-entry",
        "SCL Hash-port-svlan-entry",
        "SCL Hash-port-2vlan-entry",
        "SCL Hash-port-cvlan-cos-entry",
        "SCL Hash-port-svlan-cos-entry",
        "SCL Hash-mac-entry",
        "SCL Hash-port-mac-entry",
        "SCL Hash-ipv4-entry",
        "SCL Hash-port-ipv4-entry",
        "SCL Hash-ipv6-entry",
        "SCL Hash-port-ipv6-entry",
        "SCL Hash-l2-entry",
        "SCL Hash-tunnel-ipv4-entry",
        "SCL Hash-tunnel-ipv4-gre-entry",
        "SCL Hash-tunnel-ipv4-rpf-entry",
        "SCL Hash-tunnel-ipv4-da-entry",
        "SCL Hash-nvgre-v4-uc-mode0",
        "SCL Hash-nvgre-v4-uc-mode1",
        "SCL Hash-nvgre-v4-mc-mode0",
        "SCL Hash-nvgre-v4-mode1",
        "SCL Hash-nvgre-v6-uc-mode0",
        "SCL Hash-nvgre-v6-uc-mode1",
        "SCL Hash-nvgre-v6-mc-mode0",
        "SCL Hash-nvgre-v6-mc-mode1",
        "SCL Hash-vxlan-v4-uc-mode0",
        "SCL Hash-vxlan-v4-uc-mode1",
        "SCL Hash-vxlan-v4-mc-mode0",
        "SCL Hash-vxlan-v4-mode1",
        "SCL Hash-vxlan-v6-uc-mode0",
        "SCL Hash-vxlan-v6-uc-mode1",
        "SCL Hash-vxlan-v6-mc-mode0",
        "SCL Hash-vxlan-v6-mc-mode1",
        "SCL hash-trill-uc",
        "SCL hash-trill-mc",
        "Grep",
        CTC_CLI_ACL_KEY_FIELD_TYPE_GREP_DESC,
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{

    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    uint8 type = 0;
    uint8 detail = 0;
    uint32 param = 0;
    uint32 key_type = CTC_SCL_KEY_NUM;
    uint8 grep_cnt = 0;
    uint8 grep_i = 0;
    uint8 is_add = 1;
    ctc_field_key_t grep_key[CTC_CLI_ACL_KEY_FIELED_GREP_NUM];
    mac_addr_t  mac_da_addr = {0};
    mac_addr_t  mac_da_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    mac_addr_t  mac_sa_addr = {0};
    mac_addr_t  mac_sa_addr_mask = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ipv6_addr_t ipv6_da_addr = {0};
    ipv6_addr_t ipv6_da_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    ipv6_addr_t ipv6_sa_addr = {0};
    ipv6_addr_t ipv6_sa_addr_mask = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    uint8* p_null = NULL;

    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));

    sal_memset(grep_key, 0, sizeof(grep_key));
    for (grep_i = 0; grep_i < CTC_CLI_ACL_KEY_FIELED_GREP_NUM; grep_i++)
    {
        grep_key[grep_i].mask = 0xFFFFFFFF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (INDEX_VALID(index))
    {
        type = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (INDEX_VALID(index))
    {
        type = 1;
        CTC_CLI_GET_UINT32("entry id", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("group");
    if (INDEX_VALID(index))
    {
        type = 2;
        CTC_CLI_GET_UINT32("group id", param, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT32("priority", param, argv[index + 1]);
        type = 3;
    }

    index = CTC_CLI_GET_ARGC_INDEX("type");
    if (INDEX_VALID(index))
    {
        if CTC_CLI_STR_EQUAL_ENHANCE("tcam-mac", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_MAC;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-ipv4", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_IPV4;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-ipv4-single", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_IPV4_SINGLE;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-ipv6", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_IPV6;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-ipv6-single", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_IPV6_SINGLE;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-vlan", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_VLAN;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-udf", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_UDF;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("tcam-udf-ext", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_TCAM_UDF_EXT;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-cvlan", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_CVLAN;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-svlan", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_SVLAN;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-2vlan", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_2VLAN;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-cvlan-cos", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_CVLAN_COS;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-svlan-cos", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_SVLAN_COS;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-mac", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_MAC;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-mac", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_MAC;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-ipv4", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_IPV4;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-ipv4", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_IPV4;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-ipv6", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_IPV6;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port-ipv6", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT_IPV6;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-l2", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_L2;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-port", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_PORT;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-tunnel-ipv4", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-tunnel-ipv4-gre", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_GRE;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-tunnel-ipv4-rpf", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_RPF;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-tunnel-ipv4-da", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_TUNNEL_IPV4_DA;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v4-uc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V4_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v4-uc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V4_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v4-mc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V4_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v4-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_V4_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v6-uc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V6_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v6-uc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_UC_V6_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v6-mc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V6_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-nvgre-v6-mc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_NVGRE_MC_V6_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v4-uc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V4_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v4-uc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V4_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v4-mc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V4_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v4-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_V4_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v6-uc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V6_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v6-uc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_UC_V6_MODE1;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v6-mc-mode0", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V6_MODE0;
        }
        else if CTC_CLI_STR_EQUAL_ENHANCE("hash-vxlan-v6-mc-mode1", index + 1)
        {
            key_type = CTC_CLI_SCL_KEY_HASH_VXLAN_MC_V6_MODE1;
        }
    }
    else /*all type*/
    {
        key_type = 0xFFFFFFFF; /* SYS_SCL_KEY_NUM; */
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (INDEX_VALID(index))
    {
        detail = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_ACL_KEY_FIELD_GREP_SET(grep_key[grep_cnt],CTC_CLI_ACL_KEY_ARG_CHECK(grep_cnt,CTC_CLI_ACL_KEY_FIELED_GREP_NUM, p_null));

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_entry(lchip, type, param, key_type, detail, grep_key, grep_cnt);
    if (ret < 0)
    {
        ctc_cli_out("ret = %d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_USW_SCL_ADD_HASH_ENTRY_STR  "scl add group GROUP_ID (entry ENTRY_ID)"
#define CTC_CLI_USW_SCL_HASH_KEY_TYPE_STR "\
| port-hash-entry \
| port-cvlan-hash-entry \
| port-svlan-hash-entry \
| port-double-vlan-hash-entry \
| port-cvlan-cos-hash-entry \
| port-svlan-cos-hash-entry \
| mac-hash-entry \
| port-mac-hash-entry \
| ipv4-hash-entry \
| svlan-hash-entry \
| svlan-mac-hash-entry\
| port-ipv4-hash-entry \
| ipv6-hash-entry \
| port-ipv6-hash-entry \
| l2-hash-entry \
| port-cross-hash-entry \
| port-vlan-cross-hash-entry \
| port-svlan-dscp-hash-entry \
"

#define CTC_CLI_USW_SCL_ADD_HASH_ENTRY_DESC \
    CTC_CLI_SCL_STR, \
    "Add one entry to software table", \
    CTC_CLI_SCL_GROUP_ID_STR, \
    CTC_CLI_SCL_HASH_GROUP_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_ID_STR,          \
    CTC_CLI_SCL_ENTRY_ID_VALUE

#define CTC_CLI_USW_SCL_HASH_KEY_TYPE_DESC \
"Port hash entry", \
"Port cvlan hash entry", \
"Port svlan hash entry", \
"Port double vlan hash entry", \
"Port cvlan cos hash entry", \
"Port svlan cos hash entry", \
"Mac hash entry", \
"Port mac hash entry", \
"Ipv4 hash entry", \
"Svlan hash entry",\
"Svlan mac hash entry",\
"Port ipv4 hash entry", \
"Ipv6 hash entry", \
"Port ipv6 hash entry", \
"L2 hash entry", \
"Port cross hash entry", \
"Port vlan cross hash entry", \
"Port svlan dscp hash entry"

#define CTC_CLI_USW_SCL_ADD_HASH_ENTRY_SET()  \
    CTC_CLI_GET_UINT32("group id", gid, argv[0]);                       \
    index = CTC_CLI_GET_ARGC_INDEX("entry");                                       \
    if (INDEX_VALID(index))                                                       \
    {                                                                              \
       CTC_CLI_GET_UINT32("entry id", scl_entry->entry_id, argv[index + 1]);     \
    }

#define CTC_CLI_USW_SCL_HASH_KEY_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("port-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_CVLAN;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_CVLAN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-svlan-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_SVLAN;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_SVLAN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-double-vlan-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_2VLAN;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_2VLAN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-cos-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_CVLAN_COS;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_CVLAN_COS;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_SVLAN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-mac-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_SVLAN_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-svlan-cos-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_SVLAN_COS;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_SVLAN_COS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-hash-entry");\
    if (0xFF != index)\
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_MAC;\
        gid = CTC_SCL_GROUP_ID_HASH_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-mac-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_MAC;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_IPV4;\
        gid = CTC_SCL_GROUP_ID_HASH_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-ipv4-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_IPV4;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_IPV6;\
        gid = CTC_SCL_GROUP_ID_HASH_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-ipv6-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_IPV6;\
        gid = CTC_SCL_GROUP_ID_HASH_PORT_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l2-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_L2;\
        gid = CTC_SCL_GROUP_ID_HASH_L2;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-cross-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_CROSS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-svlan-dscp-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_SVLAN_DSCP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-vlan-cross-hash-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_HASH_PORT_VLAN_CROSS;\
        break;\
    }\
}while(0);

#define CTC_CLI_USW_SCL_KEY_HASH_COMMON_STR   \
    "(resolve-conflict |) "

#define CTC_CLI_USW_SCL_KEY_HASH_COMMON_DESC  \
    "Resolve conflict, install to tcam"

#define CTC_CLI_USW_SCL_KEY_HASH_COMMON_SET()   \
    index = CTC_CLI_GET_ARGC_INDEX("resolve-conflict");                              \
    if (INDEX_VALID(index))                                                           \
    {                                           \
        scl_entry->resolve_conflict =1;                                  \
    }

CTC_CLI(ctc_cli_usw_scl_add_hash_entry_by_field,
        ctc_cli_usw_scl_add_hash_entry_by_field_cmd,
        CTC_CLI_USW_SCL_ADD_HASH_ENTRY_STR      \
        " ( "                                    \
        CTC_CLI_USW_SCL_HASH_KEY_TYPE_STR       \
        " ) "                                    \
        "field-mode (field-sel-id SEL_ID|)"                      \
        CTC_CLI_USW_SCL_KEY_HASH_COMMON_STR      \
        " ( ingress-action | egress-action | flow-action )",
        CTC_CLI_USW_SCL_ADD_HASH_ENTRY_DESC,
        CTC_CLI_USW_SCL_HASH_KEY_TYPE_DESC,
        "Use field mode to install entry ",
        CTC_CLI_FIELD_SEL_ID_DESC,
        "Field Select Id Value",
        CTC_CLI_USW_SCL_KEY_HASH_COMMON_DESC,
        "Ingress action",
        "Egress action",
        "Flow action")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_entry_t temp_scl_entry;
    ctc_scl_entry_t* scl_entry = &temp_scl_entry;
    uint32 gid = 0;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    scl_entry->mode = 1;

    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("field-sel-id", scl_entry->hash_field_sel_id, argv[index + 1]);
    }

    CTC_CLI_USW_SCL_HASH_KEY_TYPE_SET
    CTC_CLI_USW_SCL_ADD_HASH_ENTRY_SET()
    CTC_CLI_USW_SCL_KEY_HASH_COMMON_SET()

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_EGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_FLOW;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
#define CTC_CLI_USW_SCL_ADD_ENTRY_STR  "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY|)"
#define CTC_CLI_USW_SCL_ADD_ENTRY_DESC \
    CTC_CLI_SCL_STR, \
    "Add one entry to software table", \
    CTC_CLI_SCL_GROUP_ID_STR, \
    CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_ID_STR, \
    CTC_CLI_SCL_ENTRY_ID_VALUE, \
    CTC_CLI_SCL_ENTRY_PRIO_STR, \
    CTC_CLI_SCL_ENTRY_PRIO_VALUE

#define CTC_CLI_USW_SCL_ADD_ENTRY_SET()  \
    CTC_CLI_GET_UINT32("group id", gid, argv[0]);                   \
    CTC_CLI_GET_UINT32("entry id", scl_entry->entry_id, argv[1]);    \
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");                     \
    if (INDEX_VALID(index))                                         \
    {                                                               \
       scl_entry->priority_valid = 1;                                \
       CTC_CLI_GET_UINT32("entry priority", scl_entry->priority, argv[index + 1]); \
    }                                                                \

/**< [TM] Append udf entry */
#define CTC_CLI_USW_SCL_TCAM_KEY_TYPE_STR "\
| mac-entry \
| ipv4-entry \
| ipv4-single-entry \
| ipv6-entry \
| ipv6-single-entry \
| udf-entry \
| udf-ext-entry \
"

/**< [TM] Append udf entry */
#define CTC_CLI_USW_SCL_TCAM_KEY_TYPE_DESC \
"Mac tcam entry", \
"Ipv4 tcam entry", \
"Ipv4 single tcam entry", \
"Ipv6 tcam entry", \
"Ipv6 single tcam entry", \
"Udf tcam entry", \
"Udf extend tcam entry"

#define CTC_CLI_USW_SCL_TCAM_KEY_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("mac-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-single-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_IPV4_SINGLE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-single-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_IPV6_SINGLE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("udf-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_UDF;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("udf-ext-entry");\
    if (0xFF != index) \
    {\
        scl_entry->key_type = CTC_SCL_KEY_TCAM_UDF_EXT;\
        break;\
    }\
}while(0);

#define ____scl_add_tcam_entry_by_field____
CTC_CLI(ctc_cli_usw_scl_add_tcam_entry_by_field,
        ctc_cli_usw_scl_add_tcam_entry_by_field_cmd,
        CTC_CLI_USW_SCL_ADD_ENTRY_STR      \
        " ( "                                    \
        CTC_CLI_USW_SCL_TCAM_KEY_TYPE_STR       \
        " ) "                                    \
        "field-mode"                      \
        " ( ingress-action | egress-action | flow-action )",
        CTC_CLI_USW_SCL_ADD_ENTRY_DESC,
        CTC_CLI_USW_SCL_TCAM_KEY_TYPE_DESC,
        "Use field mode to install entry ",
        "Ingress action",
        "Egress action",
        "Flow action")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 gid = 0;
    ctc_scl_entry_t temp_scl_entry;
    ctc_scl_entry_t* scl_entry = &temp_scl_entry;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));

    scl_entry->mode = 1;

    CTC_CLI_USW_SCL_ADD_ENTRY_SET()

    CTC_CLI_USW_SCL_TCAM_KEY_TYPE_SET

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_EGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-action");
    if (INDEX_VALID(index))
    {
        scl_entry->action_type = CTC_SCL_ACTION_FLOW;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_entry(g_api_lchip, gid, scl_entry);
    }
    else
    {
        ret = ctc_scl_add_entry(gid, scl_entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

#define CTC_CLI_SCL_ACTION_VLAN_EDIT_STR   \
    " vlan-edit (stag-op TAG_OP (svid-sl TAG_SL (new-svid VLAN_ID|)|) (scos-sl TAG_SL (new-scos COS|)|) (scfi-sl TAG_SL (new-scfi CFI|)|)|) \
                (ctag-op TAG_OP (cvid-sl TAG_SL (new-cvid VLAN_ID|)|) (ccos-sl TAG_SL (new-ccos COS|)|) (ccfi-sl TAG_SL (new-ccfi CFI|)|)|) \
                (vlan-domain VLAN_DOMAIN |) (tpid-index TPID_INDEX |)\
     "
#define CTC_CLI_SCL_ACTION_VLAN_EDIT_DESC  \
    "Vlan Edit",                            \
    "Stag operation type",                  \
    CTC_CLI_TAG_OP_DESC,           \
    "Svid select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Scos select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Scfi select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New stag cfi",                         \
    "<0-1>",                                  \
    "Ctag operation type",                  \
    CTC_CLI_TAG_OP_DESC,           \
    "Cvid select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag vlan id",                     \
    CTC_CLI_VLAN_RANGE_DESC,                \
    "Ccos select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag cos",                         \
    CTC_CLI_COS_RANGE_DESC,                 \
    "Ccfi select",                          \
    CTC_CLI_TAG_SL_DESC,           \
    "New ctag cfi",                         \
    "<0-1>",                       \
    "Mapped vlan domain",                   \
    CTC_CLI_VLAN_DOMAIN_DESC,               \
    "Set svlan tpid index",                 \
    "Index, the corresponding svlan tpid is in EpeL2TpidCtl"

#define CTC_CLI_SCL_ADD_UDF_STR " \
| ip-protocol VALUE MASK \
| udf udf-id UDF_ID {udf-data VALUE MASK| } \
"

#define CTC_CLI_SCL_REMOVE_UDF_STR " \
| ip-protocol \
| udf \
"
#define CTC_CLI_SCL_ADD_UDF_DESC \
"ip protocol", "VALUE", "MASK", \
"udf","udf id","udf id value","udf data","value","mask"

#define CTC_CLI_SCL_REMOVE_UDF_DESC \
"ip protocol", \
"udf module"

#define CTC_CLI_SCL_ADD_UDF_SET \
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");\
    if (0xFF != index) \
    {\
        field_key.type = CTC_FIELD_KEY_IP_PROTOCOL;\
        CTC_CLI_GET_UINT8("ip-protocol", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT8("ip-protocol", field_key.mask, argv[index + 2]);\
        break; \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("udf");\
    if (0xFF != index) \
    {\
        field_key.type = CTC_FIELD_KEY_UDF;\
        index = CTC_CLI_GET_ARGC_INDEX("udf-id");\
        if (INDEX_VALID(index))\
        {\
            CTC_CLI_GET_UINT16("udf-id", udf_data.udf_id, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("udf-data");\
        if (INDEX_VALID(index))\
        {\
            uint8 sub_idx = 0;\
            for (sub_idx = 0; sub_idx < 16; sub_idx++)\
            {\
                sal_sscanf(argv[index+1] + 2*sub_idx, "%2hhx", &udf_data.udf[sub_idx]);\
                sal_sscanf(argv[index+2] + 2*sub_idx, "%2hhx", &udf_mask.udf[sub_idx]);\
            }\
        }\
        field_key.ext_data = &udf_data;\
        field_key.ext_mask = &udf_mask;\
        break;\
    }


#define CTC_CLI_SCL_REMOVE_UDF_SET \
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");\
    if (0xFF != index) \
    {\
        field_key.type = CTC_FIELD_KEY_IP_PROTOCOL;\
        break; \
    }\
    index = CTC_CLI_GET_ARGC_INDEX("udf"); \
    if (0xFF != index) \
    { \
        field_key.type =    CTC_FIELD_KEY_UDF; \
        break; \
    }

/**< [TM] Append key: 1.Packet stag valid 2.Packet ctag valid 3.Udf */
#define CTC_CLI_SCL_KEY_FIELD_TYPE_STR "\
| ether-type VALUE MASK \
| l2-type VALUE MASK \
| vlan-num VALUE MASK \
| mac-sa VALUE MASK \
| mac-da VALUE MASK \
| packet-stag-valid VALUE MASK \
| packet-ctag-valid VALUE MASK \
| stag-valid VALUE MASK \
| svlan-id VALUE MASK \
| stag-cos VALUE MASK \
| stag-cfi VALUE MASK \
| ctag-valid VALUE MASK \
| cvlan-id VALUE MASK \
| ctag-cos VALUE MASK \
| ctag-cfi VALUE MASK \
| ip-sa VALUE MASK \
| ip-da VALUE MASK \
| ipv6-sa VALUE MASK \
| ipv6-da VALUE MASK \
| l3-type VALUE MASK \
| l4-type VALUE MASK \
| l4-user-type VALUE MASK \
| ip-ecn VALUE MASK \
| ip-dscp VALUE MASK \
| ip-frag VALUE MASK \
| ip-hdr-error VALUE MASK \
| ip-options VALUE MASK \
| ip-pkt-len-range-id VALUE MASK \
| flow-label VALUE MASK \
| l4-dst-port VALUE MASK \
| l4-src-port VALUE MASK \
| tcp-ecn VALUE MASK \
| tcp-flags VALUE MASK \
| ttl VALUE MASK \
| gre-key VALUE MASK \
| nvgre-key VALUE MASK \
| vn-id VALUE MASK \
| icmp-type VALUE MASK \
| icmp-code VALUE MASK \
| igmp-type VALUE MASK \
| global-port VALUE MASK \
| logic-port VALUE MASK \
| port-class VALUE MASK \
| customer-id VALUE MASK \
| label-num VALUE MASK \
| mpls-label0 VALUE MASK \
| mpls-exp0 VALUE MASK \
| mpls-sbit0 VALUE MASK \
| mpls-ttl0 VALUE MASK \
| mpls-label1 VALUE MASK \
| mpls-exp1 VALUE MASK \
| mpls-sbit1 VALUE MASK \
| mpls-ttl1 VALUE MASK \
| mpls-label2 VALUE MASK \
| mpls-exp2 VALUE MASK \
| mpls-sbit2 VALUE MASK \
| mpls-ttl2 VALUE MASK \
| arp-op-code VALUE MASK \
| arp-protocol-type VALUE MASK \
| arp-target-ip VALUE MASK \
| arp-sender-ip VALUE MASK \
| rarp VALUE MASK \
| is-y1731-oam VALUE MASK \
| fcoe-dst-fcid VALUE MASK \
| fcoe-src-fcid VALUE MASK \
| egress-nickname VALUE MASK \
| ingress-nickname VALUE MASK \
| trill-multihop VALUE MASK \
| trill-length VALUE MASK \
| trill-version VALUE MASK \
| is-trill-channel VALUE MASK \
| is-esadi VALUE MASK \
| trill-inner-vlanid VALUE MASK \
| trill-inner-vlanidvalid VALUE MASK \
| trill-multicast VALUE MASK \
| trill-ttl VALUE MASK \
| trill-key-type VALUE MASK \
| ether-oam-level VALUE MASK \
| ether-oam-version VALUE MASK \
| ether-oam-op-code VALUE MASK \
| slow-protocol-sub-type VALUE MASK \
| slow-protocol-flags VALUE MASK \
| slow-protocol-code VALUE MASK \
| ptp-version VALUE MASK \
| ptp-message-type VALUE MASK \
"CTC_CLI_SCL_ADD_UDF_STR" \
"

#define CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_STR "\
| ether-type VALUE \
| mac-sa VALUE \
| mac-da VALUE \
| stag-valid VALUE \
| svlan-id VALUE \
| stag-cos VALUE \
| stag-cfi VALUE \
| ctag-valid VALUE \
| cvlan-id VALUE \
| ctag-cos VALUE \
| ip-dscp VALUE \
| ip-sa VALUE \
| ip-da VALUE \
| ipv6-sa VALUE \
| ipv6-da VALUE \
| global-port VALUE (egress-key |) \
| logic-port VALUE (egress-key |) \
| port-class VALUE (egress-key |) \
| customer-id VALUE\
| valid \
"

/**< [TM] Append key: 1.Packet stag valid 2.Packet ctag valid */
#define CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_STR "\
| ether-type \
| l2-type \
| vlan-num \
| mac-sa \
| mac-da \
| packet-stag-valid \
| packet-ctag-valid \
| stag-valid \
| svlan-id \
| stag-cos \
| stag-cfi \
| ctag-valid \
| cvlan-id \
| ctag-cos \
| ctag-cfi \
| ip-sa \
| ip-da \
| ipv6-sa \
| ipv6-da \
| l3-type \
| l4-type \
| l4-user-type \
| ip-ecn \
| ip-dscp \
| ip-frag \
| ip-hdr-error \
| ip-options \
| ip-pkt-len-range-id \
| flow-label \
| l4-dst-port \
| l4-src-port \
| tcp-ecn \
| tcp-flags \
| ttl \
| gre-key \
| nvgre-key \
| vn-id \
| icmp-type \
| icmp-code \
| igmp-type \
| global-port \
| logic-port \
| port-class \
| customer-id \
| label-num \
| mpls-label0 \
| mpls-exp0 \
| mpls-sbit0 \
| mpls-ttl0 \
| mpls-label1 \
| mpls-exp1 \
| mpls-sbit1  \
| mpls-ttl1 \
| mpls-label2 \
| mpls-exp2 \
| mpls-sbit2 \
| mpls-ttl2 \
| arp-op-code \
| arp-protocol-type \
| arp-target-ip \
| arp-sender-ip \
| rarp \
| is-y1731-oam \
| fcoe-dst-fcid \
| fcoe-src-fcid \
| egress-nickname \
| ingress-nickname \
| trill-multihop \
| trill-length \
| trill-version \
| is-trill-channel \
| is-esadi \
| trill-inner-vlanid \
| trill-inner-vlanidvalid \
| trill-multicast \
| trill-ttl \
| trill-key-type \
| ether-oam-level \
| ether-oam-version \
| ether-oam-op-code \
| slow-protocol-sub-type \
| slow-protocol-flags \
| slow-protocol-code \
| ptp-version \
| ptp-message-type \
| valid \
"CTC_CLI_SCL_REMOVE_UDF_STR

#define CTC_CLI_SCL_FIELD_ACTION_TYPE1_STR "\
| pending PENDING_VALUE \
| discard-unknown-ucast \
| discard-unknown-mcast \
| discard-known-ucast \
| discard-known-mcast \
| discard-bcast \
| discard \
| stats STATS_ID \
| cancel-all  \
| copy-to-cpu \
| redirect NHID \
| fid FID \
| vrf VRFID \
| "CTC_CLI_SCL_ACTION_VLAN_EDIT_STR "\
| policer-id POLICERID (service-policer-en|) \
| src-cid CID  \
| dst-cid CID  \
| span-flow  \
| redirect-port GPORT \
| oam L2VPN_OAM_ID (vpls | vpws) \
| qos-map {priority PRIORITY | color COLOR | dscp DSCP | trust-dscp | (trust-scos | trust-ccos) | cos-domain DOMAIN | dscp-domain DOMAIN} \
"

/**< [TM] Append action: 1.mac limit discard or to cpu 2.router mac match */
#define CTC_CLI_SCL_FIELD_ACTION_TYPE2_STR "\
| logic-port GPHYPORT_ID (logic-port-type |) \
| user-vlanptr VLAN_PTR \
| aps-select GROUP_ID (protected-vlan VLAN_ID|) (working-path|protection-path) \
| etree-leaf \
| stp-id STPID \
| ptp-clock-id CLOCK_ID \
| service-id SERVICE_ID \
| service-acl-en \
| logic-security-en \
| binding-en (gport GPORT | vlan-id VLAN | ipsa A.B.C.D | ipsa-vlan A.B.C.D VLAN | macsa MAC) \
| metadata METADATA \
| deny-bridge \
| deny-learning \
| deny-route \
| force-bridge \
| force-learning \
| force-route \
| acl-tcam-en tcam-lkup-type (l2 | l3 |l2-l3 | vlan | l3-ext | l2-l3-ext | cid | interface | forward | forward-ext | udf) \
    {acl-priority PRIORITY | }{class-id CLASS_ID |(use-port-bitmap | use-metadata | use-logic-port)| use-mapped-vlan |} \
| acl-hash-en {hash-type HASH_TYPE | sel-id FIELD_SEL_ID|} \
| acl-use-outer-info \
| force-decap offset-base-type (l2 | l3 | l4) ext-offset OFFSET payload-type TYPE (use-outer-ttl |) \
| snooping-parser offset-base-type (l2 | l3 | l4) ext-offset OFFSET payload-type TYPE (use-inner-hash-en |)\
| mac-limit (discard | to-cpu) \
| router-mac-match \
| lb-hash-select-id ID \
"

/**< [TM] Append action: 1.mac limit to cpu or discard 2.router mac match */
#define CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_STR "\
| discard \
| stats \
| cancel-all  \
| copy-to-cpu \
| redirect \
| fid \
| vrf \
| vlan-edit \
| logic-port \
| policer-id  \
| src-cid  \
| dst-cid  \
| span-flow  \
| redirect-port \
| oam (vpls | vpws) \
| qos-map \
| user-vlanptr \
| aps-select \
| etree-leaf \
| stp-id \
| service-id \
| service-acl-en \
| logic-security-en \
| binding-en \
| metadata \
| deny-bridge \
| deny-learning \
| deny-route \
| force-bridge \
| force-learning \
| force-route \
| acl-tcam-en \
| acl-hash-en \
| acl-use-outer-info \
| force-decap \
| snooping-parser \
| ptp-clock-id \
| mac-limit \
| router-mac-match \
| lb-hash-select-id \
"

/**< [TM] Append packet svlan, cvlan valid and udf */
#define CTC_CLI_SCL_KEY_FIELD_TYPE_DESC \
"ether type ","value" , "mask", \
"layer 2 type ","value" , "mask", \
"vlan number", "value", "mask", \
"source mac address ","value" , "mask", \
"destination mac address ","value" , "mask", \
"valid packet s-vlan", "value", "mask", \
"valid packet c-vlan", "value", "mask", \
"valid s-vlan ","value" , "mask", \
"s-vlan id ","value" , "mask", \
"stag cos ","value" , "mask", \
"stag cfi ","value" , "mask", \
"valid c-vlan ","value" , "mask", \
"c-vlan id ","value" , "mask", \
"ctag cos ","value" , "mask", \
"ctag cfi ","value" , "mask", \
"source ipv4 address ","value" , "mask", \
"destination ipv4 address ","value" , "mask", \
"source ipv6 address ","value" , "mask", \
"destination ipv6 address ","value" , "mask", \
"layer 3 type ","value" , "mask", \
"layer 4 type ","value" , "mask", \
"layer 4 user type ","value" , "mask", \
"ecn ","value" , "mask", \
"dscp", "value", "mask", \
"ip fragment information ","value" , "mask", \
"ip header error ","value" , "mask", \
"ip options ","value" , "mask", \
"ip packet length range id ","value" , "mask", \
"ipv6 flow label ","value" , "mask", \
"layer 4 dest port ","value" , "mask", \
"layer 4 src port ","value" , "mask", \
"tcp ecn ","value" , "mask", \
"tcp flags ","value" , "mask", \
"ttl ","value" , "mask", \
"gre key ","value" , "mask", \
"nvgre key ","value" , "mask", \
"Vxlan Network Id", "value", "mask", \
"icmp type", "value", "mask", \
"icmp code", "value", "mask", \
"igmp type", "value", "mask", \
"global port ","value" , "mask", \
"logic port ","value" , "mask", \
"port class ","value" , "mask", \
"customer id","value" , "mask", \
"label num","value" , "mask", \
"mpls label0","value" , "mask", \
"mpls exp0", "value", "mask",\
"mpls sbit0", "value", "mask",\
"mpls ttl0", "value", "mask",\
"mpls label1","value" , "mask", \
"mpls exp1", "value", "mask",\
"mpls sbit1", "value", "mask",\
"mpls ttl1", "value", "mask",\
"mpls label2","value" , "mask", \
"mpls exp2", "value", "mask",\
"mpls sbit2", "value", "mask",\
"mpls ttl2", "value", "mask",\
"arp op code","value" , "mask", \
"arp protocol type","value" , "mask", \
"arp target ip","value" , "mask", \
"arp sender ip","value" , "mask", \
"rarp", "value", "mask", \
"is y1731 oam","value" , "mask", \
"fcoe dst fcid","value" , "mask", \
"fcoe src fcid","value" , "mask", \
"egress nickname","value" , "mask", \
"ingress nickname","value" , "mask", \
"trill multihop","value" , "mask", \
"trill length","value" , "mask", \
"trill version","value" , "mask", \
"is trill channel","value" , "mask", \
"is esadi","value" , "mask", \
"trill inner vlanid","value" , "mask", \
"trill inner vlanidvalid","value" , "mask", \
"trill multicast","value" , "mask", \
"trill ttl","value" , "mask", \
"trill key type","value" , "mask", \
"ether oam level","value" , "mask", \
"ether oam version","value" , "mask", \
"ether oam op code","value" , "mask", \
"slow protocol sub type","value" , "mask", \
"slow protocol flags","value" , "mask", \
"slow protocol code","value" , "mask", \
"ptp version","value" , "mask", \
"ptp message type","value" , "mask", \
CTC_CLI_SCL_ADD_UDF_DESC

#define CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_DESC \
"ether type ","value" , \
"source mac address ","value" , \
"destination mac address ","value" , \
"valid s-vlan ","value" , \
"s-vlan id ","value" , \
"stag cos ","value" , \
"stag cfi ","value" , \
"valid c-vlan ","value" , \
"c-vlan id ","value" , \
"ctag cos ","value" , \
"ip dscp", "value",\
"source ipv4 address ","value" , \
"destination ipv4 address ","value" , \
"source ipv6 address ","value" , \
"destination ipv6 address ","value" , \
"global port ","value" , "egress key",\
"logic port ","value" , "egress key", \
"port class ","value" , "egress key", \
"valid key "

/**< [TM] Append packet cvlan, svlan id valid and udf */
#define CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_DESC \
"ether type ", \
"layer 2 type ", \
"vlan num", \
"source mac address ", \
"destination mac address ", \
"valid packet s-vlan", \
"valid packet c-vlan", \
"valid s-vlan ", \
"s-vlan id ", \
"stag cos ", \
"stag cfi ", \
"valid c-vlan ", \
"c-vlan id ", \
"ctag cos ", \
"ctag cfi ", \
"source ipv4 address ", \
"destination ipv4 address ", \
"source ipv6 address ", \
"destination ipv6 address ", \
"layer 3 type ", \
"layer 4 type ", \
"layer 4 user type ", \
"ecn ", \
"dscp ", \
"ip fragment information ", \
"ip header error ", \
"ip options ", \
"ip packet length range id ", \
"ipv6 flow label ", \
"layer 4 dest port ", \
"layer 4 src port ", \
"tcp ecn ", \
"tcp flags ", \
"ttl ", \
"gre key ", \
"nvgre key ", \
"Vxlan Network Id", \
"icmp type", \
"icmp code", \
"igmp type", \
"global port ", \
"logic port ", \
"port class ", \
"customer id", \
"label num", \
"mpls label0", \
"mpls exp0", \
"mpls sbit0", \
"mpls ttl0", \
"mpls label1", \
"mpls exp1", \
"mpls sbit1", \
"mpls ttl1", \
"mpls label2", \
"mpls exp2", \
"mpls sbit2", \
"mpls ttl2", \
"arp op code", \
"arp protocol type", \
"arp target ip", \
"arp sender ip", \
"rarp", \
"is y1731 oam", \
"fcoe dst fcid", \
"fcoe src fcid", \
"egress nickname", \
"ingress nickname", \
"trill multihop", \
"trill length", \
"trill version", \
"is trill channel", \
"is esadi", \
"trill inner vlanid", \
"trill inner vlanidvalid", \
"trill multicast", \
"trill ttl", \
"trill key type", \
"ether oam level", \
"ether oam version", \
"ether oam op code", \
"slow protocol sub type", \
"slow protocol flags", \
"slow protocol code", \
"ptp version", \
"ptp message type", \
"valid key ", \
CTC_CLI_SCL_REMOVE_UDF_DESC

#define CTC_CLI_SCL_FIELD_ACTION_TYPE1_DESC \
"Pending status will not install to hw immediately", "0: cancel pending and install to hw, 1: enable pending",\
"Drop unknown ucast packet", \
"Drop unknown mcast packet", \
"Drop known ucast packet", \
"Drop known mcast packet", \
"Drop broadcast packet", \
"Discard the packet", \
"Statistic","0~0xFFFFFFFF", \
"Reset to default action", \
"Copy to cpu", \
"Redirect to", CTC_CLI_NH_ID_STR, \
CTC_CLI_FID_DESC,CTC_CLI_FID_ID_DESC, \
CTC_CLI_VRFID_DESC,CTC_CLI_VRFID_ID_DESC, \
CTC_CLI_SCL_ACTION_VLAN_EDIT_DESC, \
"Policer id", CTC_CLI_QOS_FLOW_PLC_VAL, "Enable service policer", \
"Source Category id","CID VALUE", \
"Destination Category id","CID VALUE", \
"The packet is span Flow", \
"Redirect port", CTC_CLI_GPHYPORT_ID_DESC, \
"OAM enable", "L2vpn oam id, equal to fid when VPLS", "Vpls", "Vpws", \
"QoS map", "Priority","<0-0xF>", "Color",CTC_CLI_COLOR_VALUE, "Dscp",CTC_CLI_DSCP_VALUE, "trust-dscp", \
"Trust scos", "Trust ccos", "Cos domain", "VALUE", "Dscp domain", "VALUE"

/**< [TM] Append action: 1.mac limit discard or to cpu 2.router mac match */
#define CTC_CLI_SCL_FIELD_ACTION_TYPE2_DESC \
"Logic port",CTC_CLI_GPHYPORT_ID_DESC,"Logic-port-type", \
"User vlanptr",CTC_CLI_USER_VLAN_PTR, \
"Aps selector",CTC_CLI_APS_GROUP_ID_DESC, \
"Protected vlan, maybe working vlan",CTC_CLI_VLAN_RANGE_DESC, \
"This is working path","This is protection path", \
"It is leaf node in E-Tree networks", \
"Stp Id","<0-127>", \
"PTP clock id","CLOCK ID", \
"Service ID","SERVICE ID", \
"Service acl enable",  \
"Logic port security enable", \
"Binding enable", "Port bind", CTC_CLI_GPHYPORT_ID_DESC, "Vlan bind", "VLAN ID", "Ipv4-SA bind", CTC_CLI_IPV4_FORMAT, \
                         "Ipv4-SA and Vlan bind", CTC_CLI_IPV4_FORMAT, "VLAN ID", "MAC-SA bind", CTC_CLI_MAC_FORMAT,  \
"Metadata","metadata: <0-0x3FFF>", \
"Deny bridge", \
"Deny learning", \
"Deny route", \
"Force bridge", \
"Force learning", \
"Force route", \
"Acl tcam en","Tcam lookup type","L2 key","L3 key","L2 L3 key","Vlan key",\
    "L3 extend key","L2 L3 extend key","CID  key","Interface key","Forward key","Forward extend key","Udf key",\
    "Acl priority","Priority value","Port acl use class id","CLASS_ID","Port acl use port-bitmap","Port acl use metadata","Port acl use logic port","acl use mapped vlan to lookup", \
"Acl hash en","Hash type","Hash Lookup type value",CTC_CLI_FIELD_SEL_ID_DESC,CTC_CLI_FIELD_SEL_ID_VALUE, \
"Acl user outer info", \
"Force decap", \
"Offset base type", \
"Layer2 offset, refer to ctc_scl_offset_base_type_t", \
"Layer3 offset, refer to ctc_scl_offset_base_type_t", \
"Layer4 offset, refer to ctc_scl_offset_base_type_t", \
"Extend offset", \
"Extend offset value", \
"Payload type", \
"0: Ethernet; 1: IPv4; 3: IPv6, refer to ctc_parser_pkt_type_t", \
"Use outer ttl", \
"Snooping parser", \
"Offset base type", \
"Layer2 offset, refer to ctc_scl_offset_base_type_t", \
"Layer3 offset, refer to ctc_scl_offset_base_type_t", \
"Layer4 offset, refer to ctc_scl_offset_base_type_t", \
"Extend offset", \
"Extend offset value", \
"Payload type", \
"0: Ethernet; 1: IPv4; 3: IPv6, refer to ctc_parser_pkt_type_t", \
"Enable use inner hash", \
"MAC limit","discard","copy to cpu", \
"Router MAC match", \
"Load balance hash select id"

/**< [TM] Append action: 1.mac limit to cpu or discard 2.router mac match */
#define CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_DESC \
"Discard the packet", \
"Statistic", \
"Reset to default action", \
"Copy to cpu", \
"Redirect to", \
CTC_CLI_FID_DESC, \
CTC_CLI_VRFID_DESC, \
"Vlan Edit", \
"Logic port", \
"Policer id",  \
"Source Category id",  \
"Destination Category id",  \
"The packet is span Flow", \
"Redirect port", \
"OAM", "Vpls", "Vpws", \
"QoS map", \
"User vlanptr", \
"Aps selector", \
"It is leaf node in E-Tree networks", \
"Stp Id", \
"Service ID", \
"Service acl enable",  \
"Logic port security enable", \
"Binding enable", \
"Metadata", \
"Deny bridge", \
"Deny learning", \
"Deny route", \
"Force bridge", \
"Force learning", \
"Force route", \
"Acl tcam en", \
"Acl hash en", \
"Acl user outer info", \
"Force decap",\
"Snooping parser", \
"PTP clock id", \
"MAC limit", \
"Router MAC match", \
"Load balance hash select id"

/**< [TM] Append key: 1.Packet stag valid 2.Packet ctag valid 3.Udf */
#define CTC_CLI_SCL_KEY_FIELD_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ETHER_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l2-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L2_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-num");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_VLAN_NUM;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_SA;\
        CTC_CLI_GET_MAC_ADDRESS("value", mac, argv[index + 1]);\
        CTC_CLI_GET_MAC_ADDRESS("mask", mac_mask, argv[index + 2]);\
        field_key.ext_data = (void*)mac;\
        field_key.ext_mask = (void*)mac_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_DA;\
        CTC_CLI_GET_MAC_ADDRESS("value", mac, argv[index + 1]);\
        CTC_CLI_GET_MAC_ADDRESS("mask", mac_mask, argv[index + 2]);\
        field_key.ext_data = (void*)mac;\
        field_key.ext_mask = (void*)mac_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("packet-stag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PKT_STAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("packet-ctag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PKT_CTAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_SVLAN_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_COS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_CFI;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CVLAN_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_COS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_CFI;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_SA;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_IPV4_ADDRESS("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DA;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_IPV4_ADDRESS("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_SA;\
        CTC_CLI_GET_IPV6_ADDRESS("value", ipv6_address, argv[index + 1]);\
        ipv6_address[0] = sal_htonl(ipv6_address[0]);\
        ipv6_address[1] = sal_htonl(ipv6_address[1]);\
        ipv6_address[2] = sal_htonl(ipv6_address[2]);\
        ipv6_address[3] = sal_htonl(ipv6_address[3]);\
        field_key.ext_data = ipv6_address;\
        CTC_CLI_GET_IPV6_ADDRESS("mask", ipv6_address_mask, argv[index + 2]);\
        ipv6_address_mask[0] = sal_htonl(ipv6_address_mask[0]);\
        ipv6_address_mask[1] = sal_htonl(ipv6_address_mask[1]);\
        ipv6_address_mask[2] = sal_htonl(ipv6_address_mask[2]);\
        ipv6_address_mask[3] = sal_htonl(ipv6_address_mask[3]);\
        field_key.ext_mask = ipv6_address_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_DA;\
        CTC_CLI_GET_IPV6_ADDRESS("value", ipv6_address, argv[index + 1]);\
        ipv6_address[0] = sal_htonl(ipv6_address[0]);\
        ipv6_address[1] = sal_htonl(ipv6_address[1]);\
        ipv6_address[2] = sal_htonl(ipv6_address[2]);\
        ipv6_address[3] = sal_htonl(ipv6_address[3]);\
        field_key.ext_data = ipv6_address;\
        CTC_CLI_GET_IPV6_ADDRESS("mask", ipv6_address_mask, argv[index + 2]);\
        ipv6_address_mask[0] = sal_htonl(ipv6_address_mask[0]);\
        ipv6_address_mask[1] = sal_htonl(ipv6_address_mask[1]);\
        ipv6_address_mask[2] = sal_htonl(ipv6_address_mask[2]);\
        ipv6_address_mask[3] = sal_htonl(ipv6_address_mask[3]);\
        field_key.ext_mask = ipv6_address_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l3-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L3_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_USER_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-ecn");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_ECN;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-dscp");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DSCP;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_FRAG;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-hdr-error");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_HDR_ERROR;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-options");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_OPTIONS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len-range-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_PKT_LEN_RANGE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("flow-label");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_FLOW_LABEL;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_DST_PORT;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_SRC_PORT;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_TCP_ECN;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_TCP_FLAGS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ttl");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_TTL;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_GRE_KEY;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nvgre-key");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_NVGRE_KEY;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vn-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_VN_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ICMP_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ICMP_CODE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IGMP_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("global-port");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_GPORT;\
        CTC_CLI_GET_UINT32("value", port_info.gport, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", port_info_mask.gport, argv[index + 2]);\
        field_key.type = CTC_FIELD_KEY_PORT;\
        field_key.ext_data = &port_info;\
        field_key.ext_mask = &port_info_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;\
        CTC_CLI_GET_UINT16("value", port_info.logic_port, argv[index + 1]);\
        CTC_CLI_GET_UINT16("mask", port_info_mask.logic_port, argv[index + 2]);\
        field_key.type = CTC_FIELD_KEY_PORT;\
        field_key.ext_data = &port_info;\
        field_key.ext_mask = &port_info_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-class");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;\
        CTC_CLI_GET_UINT16("value", port_info.port_class_id, argv[index + 1]);\
        CTC_CLI_GET_UINT16("mask", port_info_mask.port_class_id, argv[index + 2]);\
        field_key.type = CTC_FIELD_KEY_PORT;\
        field_key.ext_data = &port_info;\
        field_key.ext_mask = &port_info_mask;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("customer-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CUSTOMER_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("label-num");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_LABEL_NUM;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL0;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP0;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT0;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL0;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL1;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP1;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT1;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL1;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL2;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP2;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT2;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL2;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_OP_CODE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-protocol-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-target-ip");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_TARGET_IP;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_IPV4_ADDRESS("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-sender-ip");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ARP_SENDER_IP;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_IPV4_ADDRESS("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("rarp");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_RARP;\
        CTC_CLI_GET_UINT8("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT8("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("is-y1731-oam");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_Y1731_OAM;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("fcoe-dst-fcid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_FCOE_DST_FCID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("fcoe-src-fcid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_FCOE_SRC_FCID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_EGRESS_NICKNAME;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_INGRESS_NICKNAME;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multihop");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_MULTIHOP;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-length");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_LENGTH;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_VERSION;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-trill-channel");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_TRILL_CHANNEL;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-esadi");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_ESADI;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanid");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_INNER_VLANID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanidvalid");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multicast");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_MULTICAST;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-ttl");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_TTL;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-key-type");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_KEY_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-level");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_LEVEL;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_VERSION;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-op-code");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_OP_CODE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-sub-type");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-flags");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-code");\
    if (0xFF != index) \
    {\
        field_key.type =      CTC_FIELD_KEY_SLOW_PROTOCOL_CODE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_PTP_VERSION;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-message-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PTP_MESSAGE_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        CTC_CLI_GET_UINT32("mask", field_key.mask, argv[index + 2]);\
        break;\
    }\
    CTC_CLI_SCL_ADD_UDF_SET \
}while(0);

#define CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ETHER_TYPE;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_SA;\
        CTC_CLI_GET_MAC_ADDRESS("value", mac, argv[index + 1]);\
        field_key.ext_data = (void*)mac;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_DA;\
        CTC_CLI_GET_MAC_ADDRESS("value", mac, argv[index + 1]);\
        field_key.ext_data = (void*)mac;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_SVLAN_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_COS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_CFI;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_VALID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CVLAN_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_COS;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-dscp");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DSCP;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_SA;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DA;\
        CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_SA;\
        CTC_CLI_GET_IPV6_ADDRESS("value", ipv6_address, argv[index + 1]);\
        ipv6_address[0] = sal_htonl(ipv6_address[0]);\
        ipv6_address[1] = sal_htonl(ipv6_address[1]);\
        ipv6_address[2] = sal_htonl(ipv6_address[2]);\
        ipv6_address[3] = sal_htonl(ipv6_address[3]);\
        field_key.ext_data = ipv6_address;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_DA;\
        CTC_CLI_GET_IPV6_ADDRESS("value", ipv6_address, argv[index + 1]);\
        ipv6_address[0] = sal_htonl(ipv6_address[0]);\
        ipv6_address[1] = sal_htonl(ipv6_address[1]);\
        ipv6_address[2] = sal_htonl(ipv6_address[2]);\
        ipv6_address[3] = sal_htonl(ipv6_address[3]);\
        field_key.ext_data = ipv6_address;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("global-port");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_GPORT;\
        CTC_CLI_GET_UINT32("value", port_info.gport, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("egress-key");\
        if(0xFF != index)\
        {\
            field_key.type = CTC_FIELD_KEY_DST_GPORT;\
        }\
        else\
        {\
            field_key.type = CTC_FIELD_KEY_PORT;\
        }\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;\
        CTC_CLI_GET_UINT16("value", port_info.logic_port, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("egress-key");\
        if(0xFF != index)\
        {\
            field_key.type = CTC_FIELD_KEY_DST_GPORT;\
        }\
        else\
        {\
            field_key.type = CTC_FIELD_KEY_PORT;\
        }\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-class");\
    if (0xFF != index) \
    {\
        port_info.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;\
        CTC_CLI_GET_UINT16("value", port_info.port_class_id, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("egress-key");\
        if(0xFF != index)\
        {\
            field_key.type = CTC_FIELD_KEY_DST_GPORT;\
        }\
        else\
        {\
            field_key.type = CTC_FIELD_KEY_PORT;\
        }\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("customer-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CUSTOMER_ID;\
        CTC_CLI_GET_UINT32("value", field_key.data, argv[index + 1]);\
        break;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_HASH_VALID;\
        break;\
    }\
}while(0);

/**< [TM] Append key: 1.Packet stag valid 2.Packet ctag valid 3.Udf */
#define CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("ether-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ETHER_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l2-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L2_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-num");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_VLAN_NUM;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_SA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MAC_DA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("packet-stag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PKT_STAG_VALID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("packet-ctag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PKT_CTAG_VALID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_VALID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("svlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_SVLAN_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_COS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stag-cfi");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_STAG_CFI;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_VALID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cvlan-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CVLAN_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_COS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ctag-cfi");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CTAG_CFI;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_SA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-sa");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_SA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-da");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_DA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l3-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L3_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-user-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_USER_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-ecn");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_ECN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-dscp");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_DSCP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_FRAG;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-hdr-error");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_HDR_ERROR;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-options");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_OPTIONS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len-range-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_PKT_LEN_RANGE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("flow-label");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IPV6_FLOW_LABEL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_DST_PORT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_L4_SRC_PORT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-ecn");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_TCP_ECN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_TCP_FLAGS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ttl");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IP_TTL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_GRE_KEY;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("nvgre-key");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_NVGRE_KEY;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vn-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_VN_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ICMP_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ICMP_CODE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_IGMP_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("global-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PORT;\
        port_info.type = CTC_FIELD_PORT_TYPE_GPORT;\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PORT;\
        port_info.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("port-class");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PORT;\
        port_info.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;\
        field_key.ext_data = &port_info;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("customer-id");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_CUSTOMER_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("label-num");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_LABEL_NUM;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL0;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP0;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT0;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl0");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL0;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL1;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP1;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT1;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl1");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL1;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-label2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_LABEL2;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-exp2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_EXP2;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-sbit2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_SBIT2;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-ttl2");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_MPLS_TTL2;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-op-code");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_OP_CODE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-protocol-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_PROTOCOL_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-target-ip");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_ARP_TARGET_IP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("arp-sender-ip");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ARP_SENDER_IP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("rarp");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_RARP;\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("is-y1731-oam");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_Y1731_OAM;\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("fcoe-dst-fcid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_FCOE_DST_FCID;\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("fcoe-src-fcid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_FCOE_SRC_FCID;\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("egress-nickname");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_EGRESS_NICKNAME;\
        break;\
    }\
     index = CTC_CLI_GET_ARGC_INDEX("ingress-nickname");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_INGRESS_NICKNAME;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multihop");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_MULTIHOP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-length");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_LENGTH;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_VERSION;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-trill-channel");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_TRILL_CHANNEL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("is-esadi");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_IS_ESADI;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanid");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_INNER_VLANID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-inner-vlanidvalid");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_INNER_VLANID_VALID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-multicast");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_MULTICAST;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-ttl");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_TTL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("trill-key-type");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_TRILL_KEY_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-level");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_LEVEL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_VERSION;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ether-oam-op-code");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_ETHER_OAM_OP_CODE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-sub-type");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_SLOW_PROTOCOL_SUB_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-flags");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_SLOW_PROTOCOL_FLAGS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("slow-protocol-code");\
    if (0xFF != index) \
    {\
        field_key.type =      CTC_FIELD_KEY_SLOW_PROTOCOL_CODE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-version");\
    if (0xFF != index) \
    {\
        field_key.type =     CTC_FIELD_KEY_PTP_VERSION;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-message-type");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_PTP_MESSAGE_TYPE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("valid");\
    if (0xFF != index) \
    {\
        field_key.type =    CTC_FIELD_KEY_HASH_VALID;\
        break;\
    }\
    CTC_CLI_SCL_REMOVE_UDF_SET \
}while(0);

#define CTC_CLI_SCL_FIELD_ACTION_TYPE1_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("pending");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_PENDING ;\
        CTC_CLI_GET_UINT32("pending value", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DISCARD ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stats");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_STATS ;\
        CTC_CLI_GET_UINT32("stats id", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("cancel-all");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_CANCEL_ALL;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT ;\
        CTC_CLI_GET_UINT32("nexthop id", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("fid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FID ;\
        CTC_CLI_GET_UINT32("fid id", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vrf");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_VRFID ;\
        CTC_CLI_GET_UINT32("vrf id", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");\
    if (0xFF != index)\
    {\
        sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;\
        index = CTC_CLI_GET_ARGC_INDEX("stag-op");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("stag-op", vlan_edit.stag_op, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("svid-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("svid-sl", vlan_edit.svid_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-svid");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT16("new-svid", vlan_edit.svid_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("scos-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("scos-sl", vlan_edit.scos_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-scos");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("new-scos", vlan_edit.scos_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("scfi-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("scfi-sl", vlan_edit.scfi_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-scfi");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("new-scfi", vlan_edit.scfi_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ctag-op");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("ctag-op", vlan_edit.ctag_op, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("cvid-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("cvid-sl", vlan_edit.cvid_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-cvid");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT16("new-cvid", vlan_edit.cvid_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ccos-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("ccos-sl", vlan_edit.ccos_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-ccos");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("new-ccos", vlan_edit.ccos_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ccfi-sl");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("ccfi-sl", vlan_edit.ccfi_sl, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("new-ccfi");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("new-ccfi", vlan_edit.ccfi_new, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("vlan-domain");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("vlan-domain", vlan_edit.vlan_domain, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("tpid-index");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("tpid-index", vlan_edit.tpid_index, argv[index + 1]);\
        }\
        field_action.ext_data = (void*)&vlan_edit;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("policer-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;\
        CTC_CLI_GET_UINT32("policer_id", field_action.data0, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("service-policer-en");\
        if (0xFF != index) \
        {\
            field_action.data1 = 1;\
        }\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SRC_CID;\
        CTC_CLI_GET_UINT32("src-cid", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DST_CID;\
        CTC_CLI_GET_UINT32("dst-cid", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("span-flow");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-port");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT;\
        CTC_CLI_GET_UINT32("redirect-port", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("oam");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;\
        CTC_CLI_GET_UINT32("l2vpn-oam-id", field_action.data1, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("vpls");\
        if (0xFF != index) \
        {\
            field_action.data0 = 0;\
        }\
        else\
        {\
            field_action.data0 = 1;\
        }\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("qos-map");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;\
        index = CTC_CLI_GET_ARGC_INDEX("priority");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_PRIORITY_VALID); \
            CTC_CLI_GET_UINT8("priority", qos_map.priority, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("color");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("color", qos_map.color, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_DSCP_VALID); \
            CTC_CLI_GET_UINT8("dscp", qos_map.dscp, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-dscp");\
        if (0xFF != index)\
        {\
            qos_map.trust_dscp = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-scos");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID); \
            qos_map.trust_cos = 0;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("trust-ccos");\
        if (0xFF != index)\
        {\
            CTC_SET_FLAG(qos_map.flag, CTC_SCL_QOS_MAP_FLAG_TRUST_COS_VALID); \
            qos_map.trust_cos = 1;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("cos-domain");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("cos-domain", qos_map.cos_domain, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("dscp-domain");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("dscp-domain", qos_map.dscp_domain, argv[index + 1]);\
        }\
        field_action.ext_data = (void*)&qos_map;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard-unknown-ucast");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_UCAST ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard-unknown-mcast");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DROP_UNKNOWN_MCAST ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard-known-ucast");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_UCAST ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard-known-mcast");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DROP_KNOWN_MCAST ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("discard-bcast");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DROP_BCAST ;\
        break;\
    }\
}while(0);
/**< [TM] Append action: 1.mac limit discard or copy to cpu 2.router mac match */
#define CTC_CLI_SCL_FIELD_ACTION_TYPE2_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT ;\
        CTC_CLI_GET_UINT16("logic-port", logic_port.logic_port, argv[index + 1]);\
        index = CTC_CLI_GET_ARGC_INDEX("logic-port-type");\
        if (0xFF != index)\
        {\
            logic_port.logic_port_type = 1;\
        }\
        field_action.ext_data = (void*)&logic_port;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;\
        CTC_CLI_GET_UINT16("user-vlanptr", field_action.data0, argv[index + 1]);\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("aps-select");\
    if (0xFF != index)\
    {\
        CTC_CLI_GET_UINT16("aps select group", aps.aps_select_group_id, argv[index + 1]);\
        aps.protected_vlan_valid = 0;\
        index = CTC_CLI_GET_ARGC_INDEX("protected-vlan");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT16("aps protected vlan", aps.protected_vlan, argv[index + 1]);\
            aps.protected_vlan_valid = 1;\
        }\
        else\
        {\
            aps.protected_vlan = 0;\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("working-path");\
        if (0xFF != index)\
        {\
            aps.is_working_path = 1;\
        }\
        else\
        {\
            aps.is_working_path = 0;\
        }\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_APS;\
        field_action.ext_data = (void*)&aps;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("etree-leaf");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stp-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_STP_ID;\
        CTC_CLI_GET_UINT16("stp-id", field_action.data0, argv[index + 1]);            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-clock-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID;\
        CTC_CLI_GET_UINT8("ptp-clock-id", field_action.data0, argv[index + 1]);            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("service-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;\
        CTC_CLI_GET_UINT16("service-id", field_action.data0, argv[index + 1]);            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;\
        field_action.data0 = 1;            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-security-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN;\
        field_action.data0 = 1;            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("binding-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;\
        index = CTC_CLI_GET_ARGC_INDEX("gport");\
        if (0xFF != index) \
        {\
            bind.type = CTC_SCL_BIND_TYPE_PORT; \
            CTC_CLI_GET_UINT16("gport", bind.gport, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("vlan-id");\
        if (0xFF != index) \
        {\
            bind.type = CTC_SCL_BIND_TYPE_VLAN; \
            CTC_CLI_GET_UINT16("vlan-id", bind.vlan_id, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ipsa");\
        if (0xFF != index) \
        {\
            bind.type = CTC_SCL_BIND_TYPE_IPV4SA; \
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", bind.ipv4_sa, argv[index+1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ipsa-vlan");\
        if (0xFF != index) \
        {\
            bind.type = CTC_SCL_BIND_TYPE_IPV4SA_VLAN; \
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", bind.ipv4_sa, argv[index+1]); \
            CTC_CLI_GET_UINT16("vlan-id", bind.vlan_id, argv[index + 2]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("macsa");\
        if (0xFF != index) \
        {\
            bind.type = CTC_SCL_BIND_TYPE_MACSA; \
            CTC_CLI_GET_MAC_ADDRESS("macsa", bind.mac_sa, argv[index + 1]); \
        }\
        field_action.ext_data = &bind;            \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");\
    if (0xFF != index)\
    {\
        CTC_CLI_GET_UINT32("metadata", field_action.data0, argv[index + 1]);\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_METADATA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-bridge");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-route");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-bridge");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-learning");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-route");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-en");\
    if (0xFF != index)\
    {\
        index = CTC_CLI_GET_ARGC_INDEX("tcam-lkup-type");\
        if(0xFF != index)\
        {\
            field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN;\
            if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3",index + 1))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3;\
            }\
            else if(CLI_CLI_STR_EQUAL("vlan",index + 1))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_VLAN;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l3-ext",index + 1))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L3_EXT;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l2-l3-ext",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;\
            }\
            else if(CLI_CLI_STR_EQUAL("cid",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_CID;\
            }\
            else if(CLI_CLI_STR_EQUAL("interface",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_INTERFACE;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("forward",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("forward-ext",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("udf",index + 1 ))\
            {\
                acl.tcam_lkup_type = CTC_ACL_TCAM_LKUP_TYPE_UDF;\
            }\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("acl-priority");\
        if(0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("acl-priority",acl.acl_priority, argv[index + 1] );\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("class-id");\
        if(0xFF != index)\
        {\
            CTC_CLI_GET_UINT8("class-id",acl.class_id, argv[index + 1] );\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-port-bitmap");\
        if(0xFF != index)\
        {\
            CTC_SET_FLAG(acl.flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-metadata");\
        if(0xFF != index)\
        {\
            CTC_SET_FLAG(acl.flag, CTC_ACL_PROP_FLAG_USE_METADATA);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-logic-port");\
        if(0xFF != index)\
        {\
            CTC_SET_FLAG(acl.flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-mapped-vlan");\
        if(0xFF != index)\
        {\
            CTC_SET_FLAG(acl.flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);\
        }\
        field_action.ext_data = (void*)&acl;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-hash-en");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN;\
        CTC_SET_FLAG(acl.flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP);\
        index = CTC_CLI_GET_ARGC_INDEX("hash-type");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT32("hash-type", acl.hash_lkup_type, argv[index + 1]);\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("sel-id");\
        if (0xFF != index)\
        {\
            CTC_CLI_GET_UINT32("field-sel-id", acl.hash_field_sel_id, argv[index + 1]);\
        }\
        field_action.ext_data = (void*)&acl;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD;\
        field_action.data0 = 1;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-decap");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP;\
        decap.force_decap_en = 1;\
        index = CTC_CLI_GET_ARGC_INDEX("offset-base-type");\
        if (0xFF != index) \
        {\
            if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))\
            {\
                decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L2;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))\
            {\
                decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L3;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l4",index + 1))\
            {\
                decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L4;\
            }\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ext-offset");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("ext-offset", decap.ext_offset, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("payload-type");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("payload-type", decap.payload_type, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-outer-ttl");\
        if (0xFF != index) \
        {\
            decap.use_outer_ttl = 1; \
        }\
        field_action.ext_data = (void*)&decap;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("snooping-parser");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER;\
        snooping_parser.enable = 1;\
        index = CTC_CLI_GET_ARGC_INDEX("offset-base-type");\
        if (0xFF != index) \
        {\
            if(CTC_CLI_STR_EQUAL_ENHANCE("l2",index + 1))\
            {\
                snooping_parser.start_offset_type = CTC_SCL_OFFSET_BASE_TYPE_L2;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l3",index + 1))\
            {\
                snooping_parser.start_offset_type = CTC_SCL_OFFSET_BASE_TYPE_L3;\
            }\
            else if(CTC_CLI_STR_EQUAL_ENHANCE("l4",index + 1))\
            {\
                snooping_parser.start_offset_type = CTC_SCL_OFFSET_BASE_TYPE_L4;\
            }\
        }\
        index = CTC_CLI_GET_ARGC_INDEX("ext-offset");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("ext-offset", snooping_parser.ext_offset, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("payload-type");\
        if (0xFF != index) \
        {\
            CTC_CLI_GET_UINT8("payload-type", snooping_parser.payload_type, argv[index + 1]); \
        }\
        index = CTC_CLI_GET_ARGC_INDEX("use-inner-hash-en");\
        if (0xFF != index) \
        {\
            snooping_parser.use_inner_hash_en = 1; \
        }\
        field_action.ext_data = (void*)&snooping_parser;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-limit");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT; \
        if (0xFF != CTC_CLI_GET_ARGC_INDEX("discard")) \
        { \
            field_action.data0 = CTC_MACLIMIT_ACTION_DISCARD; \
        } \
        if (0xFF != CTC_CLI_GET_ARGC_INDEX("to-cpu")) \
        { \
            field_action.data0 = CTC_MACLIMIT_ACTION_TOCPU; \
        } \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("router-mac-match");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH; \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-select-id");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID; \
        CTC_CLI_GET_UINT8("select-id", field_action.data0, argv[index + 1]); \
        break;\
    }\
}while(0);

/**< [TM] Append action: 1.logic port mac limit discard or to cpu 2.router mac match */
#define CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_SET \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("discard");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DISCARD ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stats");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_STATS ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_COPY_TO_CPU;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("fid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FID ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vrf");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_VRFID ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("vlan-edit");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_VLAN_EDIT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("policer-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-port");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT ;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SRC_CID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DST_CID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("span-flow");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SPAN_FLOW;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("redirect-port");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_REDIRECT_PORT;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("oam");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_OAM;\
        index = CTC_CLI_GET_ARGC_INDEX("vpls");\
        if (0xFF != index) \
        {\
            field_action.data0 = 0;\
        }\
        else\
        {\
            field_action.data0 = 1;\
        }\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("qos-map");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("user-vlanptr");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_USER_VLANPTR;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("aps-select");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_APS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("etree-leaf");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ETREE_LEAF;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("stp-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_STP_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ptp-clock-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_PTP_CLOCK_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("service-id");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ID;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("service-acl-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("logic-security-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_SECURITY_EN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("binding-en");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("metadata");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_METADATA;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-bridge");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_BRIDGE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-learning");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("deny-route");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_DENY_ROUTE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-bridge");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_BRIDGE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-learning");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_LEARNING;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-route");\
    if (0xFF != index) \
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_ROUTE;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-tcam-en");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ACL_TCAM_EN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-hash-en");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_ACL_HASH_EN;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FLOW_LKUP_BY_OUTER_HEAD;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("force-decap");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("snooping-parser");\
    if (0xFF != index)\
    {\
        field_action.type =    CTC_SCL_FIELD_ACTION_TYPE_SNOOPING_PARSER;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mac-limit");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_MAC_LIMIT; \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("router-mac-match");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_ROUTER_MAC_MATCH; \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("lb-hash-select-id");\
    if (0xFF != index) \
    {\
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LB_HASH_SELECT_ID; \
        break;\
    }\
}while(0);

CTC_CLI(ctc_cli_usw_scl_add_field_key,
        ctc_cli_usw_scl_add_field_key_cmd,
        "scl entry ENTRY_ID add tcam-key-field ("\
        CTC_CLI_SCL_KEY_FIELD_TYPE_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "Add one key field to an entry",
        "tcam key field",
        CTC_CLI_SCL_KEY_FIELD_TYPE_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_field_key_t field_key;
    uint32 entry_id = 0;
    ipv6_addr_t ipv6_address;
    ipv6_addr_t ipv6_address_mask;
    mac_addr_t  mac;
    mac_addr_t mac_mask;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_mask;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_mask, 0, sizeof(ctc_acl_udf_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_KEY_FIELD_TYPE_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_key_field(g_api_lchip, entry_id, &field_key);
    }
    else
    {
        ret = ctc_scl_add_key_field(entry_id, &field_key);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_add_hash_field_key,
        ctc_cli_usw_scl_add_hash_field_key_cmd,
        "scl entry ENTRY_ID add key-field  ("\
        CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "Add one key field to an entry",
        "hash key field",
        CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_field_key_t field_key;
    uint32 entry_id = 0;
    ipv6_addr_t ipv6_address;
    mac_addr_t  mac;
    ctc_field_port_t port_info;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_mask;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_mask, 0, sizeof(ctc_acl_udf_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_KEY_FIELD_TYPE_HASH_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_key_field(g_api_lchip, entry_id, &field_key);
    }
    else
    {
        ret = ctc_scl_add_key_field(entry_id, &field_key);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_remove_field_key,
        ctc_cli_usw_scl_remove_field_key_cmd,
        "scl entry ENTRY_ID remove key-field ("\
        CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "remove one key field to an entry",
        "key field",
        CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_field_key_t field_key;
    uint32 entry_id = 0;
    ctc_field_port_t port_info;
    ctc_field_port_t port_info_mask;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_mask;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&port_info, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_info_mask, 0, sizeof(ctc_field_port_t));
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_mask, 0, sizeof(ctc_acl_udf_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_REMOVE_KEY_FIELD_TYPE_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_remove_key_field(g_api_lchip, entry_id, &field_key);
    }
    else
    {
        ret = ctc_scl_remove_key_field(entry_id, &field_key);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_add_field_action_1,
        ctc_cli_usw_scl_add_field_action_1_cmd,
        "scl entry ENTRY_ID add action-field ("\
        CTC_CLI_SCL_FIELD_ACTION_TYPE1_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "Add one action field to an entry",
        "action field",
        CTC_CLI_SCL_FIELD_ACTION_TYPE1_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_field_action_t field_action;
    uint32 entry_id = 0;
    ctc_scl_vlan_edit_t vlan_edit;
    ctc_scl_qos_map_t qos_map;

    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));
    sal_memset(&qos_map, 0, sizeof(qos_map));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_FIELD_ACTION_TYPE1_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_action_field(g_api_lchip, entry_id, &field_action);
    }
    else
    {
        ret = ctc_scl_add_action_field(entry_id, &field_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_add_field_action_2,
        ctc_cli_usw_scl_add_field_action_2_cmd,
        "scl entry ENTRY_ID add action-field ("\
        CTC_CLI_SCL_FIELD_ACTION_TYPE2_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "Add one action field to an entry",
        "action field",
        CTC_CLI_SCL_FIELD_ACTION_TYPE2_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_field_action_t field_action;
    uint32 entry_id = 0;
    ctc_scl_logic_port_t logic_port;
    ctc_scl_bind_t bind;
    ctc_scl_aps_t aps;
    ctc_acl_property_t acl;
    ctc_scl_force_decap_t decap;
    ctc_scl_snooping_parser_t snooping_parser;

    sal_memset(&acl,  0, sizeof(ctc_acl_property_t));
    sal_memset(&bind, 0, sizeof(ctc_scl_bind_t));
    sal_memset(&aps, 0, sizeof(ctc_scl_aps_t));
    sal_memset(&decap, 0, sizeof(ctc_scl_force_decap_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&logic_port, 0, sizeof(ctc_scl_logic_port_t));
    sal_memset(&snooping_parser, 0, sizeof(ctc_scl_snooping_parser_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_FIELD_ACTION_TYPE2_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_add_action_field(g_api_lchip, entry_id, &field_action);
    }
    else
    {
        ret = ctc_scl_add_action_field(entry_id, &field_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_remove_field_action,
        ctc_cli_usw_scl_remove_field_action_cmd,
        "scl entry ENTRY_ID remove action-field ("\
        CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_STR     \
        ")",
        CTC_CLI_SCL_STR,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        "remove one action field to an entry",
        "action field",
        CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_field_action_t field_action;
    uint32 entry_id = 0;

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    CTC_CLI_GET_UINT32("entry id", entry_id, argv[0]);

    CTC_CLI_SCL_REMOVE_FIELD_ACTION_TYPE_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_remove_action_field(g_api_lchip, entry_id, &field_action);
    }
    else
    {
        ret = ctc_scl_remove_action_field(entry_id, &field_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_set_default_action_1,
        ctc_cli_usw_scl_set_default_action_1_cmd,
        "scl set default-action gport GPORT_ID (ingress-action | egress-action) (scl-id SCL_ID |) ("\
        CTC_CLI_SCL_FIELD_ACTION_TYPE1_STR     \
        ")",
        CTC_CLI_SCL_STR,
        "Set scl default action",
        "Default action",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Ingress action",
        "Egress action",
        "Scl id",
        "SCL ID",
        CTC_CLI_SCL_FIELD_ACTION_TYPE1_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t vlan_edit;
    ctc_scl_qos_map_t qos_map;
    ctc_scl_default_action_t def_action;

    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));
    sal_memset(&qos_map, 0, sizeof(qos_map));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&def_action, 0, sizeof(ctc_scl_default_action_t));

    CTC_CLI_GET_UINT32("gport", def_action.gport, argv[0]);
    def_action.mode = 1;
    def_action.field_action = &field_action;

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (0xFF != index)
    {
        def_action.dir = CTC_SCL_ACTION_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (0xFF != index)
    {
        def_action.dir = CTC_SCL_ACTION_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("scl-id", def_action.scl_id, argv[index + 1]);
    }

    CTC_CLI_SCL_FIELD_ACTION_TYPE1_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_set_default_action(g_api_lchip, &def_action);
    }
    else
    {
        ret = ctc_scl_set_default_action(&def_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_set_default_action_2,
        ctc_cli_usw_scl_set_default_action_2_cmd,
        "scl set default-action gport GPORT_ID (ingress-action | egress-action) (scl-id SCL_ID |) ("\
        CTC_CLI_SCL_FIELD_ACTION_TYPE2_STR \
        ")",
        CTC_CLI_SCL_STR,
        "Set scl default action",
        "Default action",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Ingress action",
        "Egress action",
        "Scl id",
        "SCL ID",
        CTC_CLI_SCL_FIELD_ACTION_TYPE2_DESC)
{

    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ctc_scl_field_action_t field_action;
    ctc_scl_logic_port_t logic_port;
    ctc_scl_bind_t bind;
    ctc_scl_aps_t aps;
    ctc_acl_property_t acl;
    ctc_scl_force_decap_t decap;
    ctc_scl_default_action_t def_action;
    ctc_scl_snooping_parser_t snooping_parser;

    sal_memset(&acl,  0, sizeof(ctc_acl_property_t));
    sal_memset(&bind, 0, sizeof(ctc_scl_bind_t));
    sal_memset(&aps, 0, sizeof(ctc_scl_aps_t));
    sal_memset(&decap, 0, sizeof(ctc_scl_force_decap_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&def_action, 0, sizeof(ctc_scl_default_action_t));
    sal_memset(&logic_port, 0, sizeof(ctc_scl_logic_port_t));
    sal_memset(&snooping_parser, 0, sizeof(ctc_scl_snooping_parser_t));

    CTC_CLI_GET_UINT32("gport", def_action.gport, argv[0]);
    def_action.mode = 1;
    def_action.field_action = &field_action;

    index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
    if (0xFF != index)
    {
        def_action.dir = CTC_SCL_ACTION_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress-action");
    if (0xFF != index)
    {
        def_action.dir = CTC_SCL_ACTION_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("scl-id", def_action.scl_id, argv[index + 1]);
    }

    CTC_CLI_SCL_FIELD_ACTION_TYPE2_SET
    if(g_ctcs_api_en)
    {
        ret = ctcs_scl_set_default_action(g_api_lchip, &def_action);
    }
    else
    {
        ret = ctc_scl_set_default_action(&def_action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_usw_scl_show_block_status,
        ctc_cli_usw_scl_show_block_status_cmd,
        "show scl (block-id BLOCK_ID) status (lchip LCHIP|)",
        "Show",
        CTC_CLI_SCL_STR,
        "block id",
        "BLOCK ID",
        "status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 block_id = 0;

    index = CTC_CLI_GET_ARGC_INDEX("block-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("block-id", block_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ret = sys_usw_scl_show_tcam_alloc_status(lchip, block_id);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_usw_scl_show_default_entry,
        ctc_cli_usw_scl_show_default_entry_cmd,
        "show scl default-action port GPORT_ID (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SCL_STR,
        "Default action",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret;
    uint32 gport;
    uint8 lchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT32_VALUE);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_entry( lchip,4, gport, 0, 1, NULL, 0);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_usw_scl_show_all_group_info,
        ctc_cli_usw_scl_show_all_group_info_cmd,
        "show scl group-info all (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_SCL_STR,
        "Group info",
        "All group",
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


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_scl_show_group_status( lchip);

    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
int32
ctc_usw_scl_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_entry_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_spool_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_tunnel_entry_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_key_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_hash_field_key_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_remove_field_key_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_action_1_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_action_2_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_remove_field_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_hash_entry_by_field_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_tcam_entry_by_field_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_set_default_action_1_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_set_default_action_2_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_block_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_default_entry_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_all_group_info_cmd);

    return 0;
}

int32
ctc_usw_scl_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_entry_info_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_spool_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_tunnel_entry_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_key_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_hash_field_key_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_remove_field_key_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_action_1_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_field_action_2_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_remove_field_action_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_hash_entry_by_field_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_add_tcam_entry_by_field_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_set_default_action_1_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_set_default_action_2_cmd);

    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_block_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_default_entry_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_scl_show_all_group_info_cmd);
    return 0;
}

