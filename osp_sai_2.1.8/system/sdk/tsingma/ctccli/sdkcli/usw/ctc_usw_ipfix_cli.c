#if (FEATURE_MODE == 0)
/**
 @file ctc_ipfix_cli.c

 @date 2010-06-09

 @version v2.0

 This file defines functions for ptp cli module

*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
 /*#include "ctc_ipfix_cli.h"*/
#include "ctc_ipfix.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"


#include "ctc_ipfix_cli.h"

#if 0
#include "ctc_vti_vec.h"
int32
ctc_vti_vec_replace_value(vector v, void* old_val, void* new_val)
{
    uint32 i;

    for (i = 0; i < v->max; i++)
    {
        if (v->index[i] != NULL && v->index[i] == old_val)
        {
            v->index[i] = new_val;
            return 0;
        }
    }

    return -1;
}

/* Install a command into a node. */
void
install_element2(ctc_node_type_t ntype, ctc_cmd_element_t* cmd,ctc_cmd_element_t* old_cmd)
{
    extern vector cmdvec;
    extern vector ctc_cmd_make_descvec(char* string, char** descstr);
    extern  int32 ctc_cmd_cmdsize(vector strvec);
    ctc_cmd_node_t* cnode;

    cnode = vector_slot(cmdvec, ntype);

    if (cnode == NULL)
    {
        ctc_cli_out("Command node %d doesn't exist, please check it\n", ntype);
        return;
    }

    ctc_vti_vec_replace_value(cnode->cmd_vector, old_cmd, cmd);

    cmd->strvec = ctc_cmd_make_descvec(cmd->string, cmd->doc);
    cmd->cmdsize = ctc_cmd_cmdsize(cmd->strvec);


}
#endif
/**
 @brief  Initialize sdk IPFIX module CLIs

 @param[in/out]   cli_tree     CLI tree

 @return CTC_E_XXX

*/

extern int32
sys_usw_ipfix_get_entry_by_key(uint8 lchip, ctc_ipfix_data_t* p_key, uint32* p_rst_hit, uint32* p_rst_key_index);
extern int32
sys_usw_ipfix_delete_ad_by_index(uint8 lchip, uint32 index, uint8 dir);
extern int32
sys_usw_ipfix_delete_entry_by_index(uint8 lchip, uint8 type, uint32 index, uint8 dir);
extern int32
sys_usw_ipfix_get_ad_by_index(uint8 lchip, uint32 index, ctc_ipfix_data_t* p_out);
extern int32
sys_usw_ipfix_get_entry_by_index(uint8 lchip, uint32 index, uint8 dir, uint8 key_type, ctc_ipfix_data_t* p_out);
extern int32
sys_usw_ipfix_show_status(uint8 lchip, void *user_data);
extern int32
sys_usw_ipfix_show_stats(uint8 lchip, void *user_data);
extern int32
sys_usw_ipfix_clear_stats(uint8 lchip, void * user_data);
extern int32
sys_usw_ipfix_show_entry_info(ctc_ipfix_data_t* p_info, void* user_data);

extern int32
_ctc_cli_ipfix_parser_mpls_key(unsigned char argc,char** argv, ctc_ipfix_data_t *data);

CTC_CLI(ctc_cli_usw_ipfix_delete_by_index,
        ctc_cli_usw_ipfix_delete_by_index_cmd,
        "ipfix delete (mac-key|ipv4-key|ipv6-key|mpls-key|l2l3-key)  by index IDX {dir DIR|lchip LCHIP|}",
        CTC_CLI_IPFIX_M_STR,
        "Delete",
        "L2 key",
        "Ipv4 key",
        "Ipv6 key",
        "Mpls key",
        "L2l4 key",
        "By",
        "Index",
        "Index value",
        "Direction",
        "<0:Ingress,1:Egress>",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 index = 0;
    uint8 type = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 dir = 0;

    if (0 == sal_memcmp("mac", argv[0], sal_strlen("mac")))
    {
        type = CTC_IPFIX_KEY_HASH_MAC;
    }
    else if (0 == sal_memcmp("ipv4", argv[0], sal_strlen("ipv4")))
    {
        type = CTC_IPFIX_KEY_HASH_IPV4;
    }
    else if (0 == sal_memcmp("ipv6", argv[0], sal_strlen("ipv6")))
    {
        type = CTC_IPFIX_KEY_HASH_IPV6;
    }
    else if (0 == sal_memcmp("mpls", argv[0], sal_strlen("mpls")))
    {
        type = CTC_IPFIX_KEY_HASH_MPLS;
    }
    else if (0 == sal_memcmp("l2l3", argv[0], sal_strlen("l2l3")))
    {
        type = CTC_IPFIX_KEY_HASH_L2_L3;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("dir", dir, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    CTC_CLI_GET_UINT32_RANGE("index", index, argv[1],0, CTC_MAX_UINT32_VALUE);


    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_delete_ad_by_index(lchip, index, dir);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ret = sys_usw_ipfix_delete_entry_by_index(lchip, type, index, dir);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_status,
        ctc_cli_usw_ipfix_show_status_cmd,
        "show ipfix status (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPFIX_M_STR,
        "Ipfix Status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_show_status(lchip, NULL);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_stats,
        ctc_cli_usw_ipfix_show_stats_cmd,
        "show ipfix stats (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPFIX_M_STR,
        "Ipfix Stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_show_stats(lchip, NULL);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_clear_stats,
        ctc_cli_usw_ipfix_clear_stats_cmd,
        "clear stats ipfix (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_IPFIX_M_STR,
        "Clear Ipfix Stats",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_clear_stats(lchip, NULL);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_usw_ipfix_show_l2_hashkey,
        ctc_cli_usw_ipfix_show_l2_hashkey_cmd,
        "show ipfix entry-info by entry-key mac-key (src-gport SRCPORT|hash-field-sel FIELD_ID){"CTC_CLI_IPFIX_KEY_PORT_STR "|profile-id PROFILE_ID |"CTC_CLI_IPFIX_KEY_CID_STR "|" CTC_CLI_IPFIX_KEY_L2_STR "} (dir DIR) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry key",
        "L2 key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_KEY_PORT_STR_DESC,
        "Config profile id",
        "Profile id",
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        CTC_CLI_IPFIX_KEY_L2_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint32 rst_hit;
    uint32 rst_key_index;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 detail = 0;
    uint32 entry_idx = 0;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MAC;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("lport", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan prio", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_key(lchip, &use_data, &rst_hit, &rst_key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

	ret = sys_usw_ipfix_get_ad_by_index(lchip, rst_key_index, &use_data);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (detail == 1)
    {
        entry_idx = rst_key_index + (detail<<16);
    }
    else
    {
        entry_idx = rst_key_index;
    }

    ret = sys_usw_ipfix_show_entry_info(&use_data, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_l2l3_hashkey,
        ctc_cli_usw_ipfix_show_l2l3_hashkey_cmd,
        "show ipfix entry-info by entry-key l2l3-key (src-gport SRCPORT| hash-field-sel FIELD_ID)" CTC_CLI_IPFIX_L2_L3_KEY_STR "|" CTC_CLI_IPFIX_KEY_CID_STR "} (dir DIR) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry key",
        "L2L3 key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        CTC_CLI_IPFIX_L2_L3_KEY_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    uint32 rst_hit = 0;
    uint32 rst_key_index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 detail = 0;
    uint32 entry_idx = 0;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_L2_L3;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("lport", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-da", use_data.dst_mac, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("mac-sa", use_data.src_mac, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eth-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ether type", use_data.ether_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan", use_data.svlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan piro", use_data.svlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("svlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan cfi", use_data.svlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan", use_data.cvlan, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-prio");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan prio", use_data.cvlan_prio, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvlan-cfi");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan cfi", use_data.cvlan_cfi, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_CVLAN_TAGGED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_key(lchip, &use_data, &rst_hit, &rst_key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (detail == 1)
    {
        entry_idx = rst_key_index + (detail<<16);
    }
    else
    {
        entry_idx = rst_key_index;
    }

    ret = sys_usw_ipfix_show_entry_info(&use_data, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_ipv4_hashkey,
        ctc_cli_usw_ipfix_show_ipv4_hashkey_cmd,
        "show ipfix entry-info by entry-key ipv4-key (src-gport SRCPORT|hash-field-sel FIELD_ID){profile-id PROFILE_ID |"CTC_CLI_IPFIX_KEY_PORT_STR "|" CTC_CLI_IPFIX_KEY_CID_STR "|vrfid VRFID_VALUE|"CTC_CLI_IPFIX_KEY_L3_STR"|"CTC_CLI_IPFIX_KEY_IPV4_L4_STR"} (dir DIR) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry key",
        "Ipv4 key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        "vrfid",
        "VRFID_VALUE",
        CTC_CLI_IPFIX_KEY_L3_STR_DESC,
        CTC_CLI_IPFIX_KEY_IPV4_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    uint32 temp = 0;
    uint32 rst_hit = 0;
    uint32 rst_key_index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 detail = 0;
    uint32 entry_idx = 0;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV4;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("lport", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipsa = temp;
        use_data.l3_info.ipv4.ipsa_masklen = mask_len;

    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipda", temp, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv4.ipda = temp;
        use_data.l3_info.ipv4.ipda_masklen = mask_len;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv4.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ttl", use_data.l3_info.ipv4.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrfid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vrfid", use_data.l3_info.vrfid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ecn", use_data.l3_info.ipv4.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-identification");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-identification", use_data.l3_info.ipv4.ip_identification, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip-pkt-len");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("ip-pkt-len", use_data.l3_info.ipv4.ip_pkt_len, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_key(lchip, &use_data, &rst_hit, &rst_key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    if (detail == 1)
    {
        entry_idx = rst_key_index + (detail<<16);
    }
    else
    {
        entry_idx = rst_key_index;
    }

    ret = sys_usw_ipfix_show_entry_info(&use_data, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_mpls_hashkey,
        ctc_cli_usw_ipfix_show_mpls_hashkey_cmd,
        "show ipfix entry-info by entry-key mpls-key (src-gport SRCPORT| hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR "|" CTC_CLI_IPFIX_KEY_MPLS_STR "}" " (dir DIR) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry key",
        "Mpls key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        CTC_CLI_IPFIX_KEY_MPLS_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint32 rst_hit = 0;
    uint32 rst_key_index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 detail = 0;
    uint32 entry_idx = 0;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_MPLS;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("lport", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    _ctc_cli_ipfix_parser_mpls_key(argc, &argv[0], &use_data);

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_key(lchip, &use_data, &rst_hit, &rst_key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    if (detail == 1)
    {
        entry_idx = rst_key_index + (detail<<16);
    }
    else
    {
        entry_idx = rst_key_index;
    }

    ret = sys_usw_ipfix_show_entry_info(&use_data, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_ipv6_hashkey,
        ctc_cli_usw_ipfix_show_ipv6_hashkey_cmd,
        "show ipfix entry-info by entry-key ipv6-key (src-gport SRCPORT |hash-field-sel FIELD_ID){profile-id PROFILE_ID|"CTC_CLI_IPFIX_KEY_PORT_STR "| ip-sa X:X::X:X mask-len LEN| ip-da X:X::X:X mask-len LEN|dscp DSCP|flow-label FLOW_LABEL|ttl TTL|ecn ECN|tcp-flags VALUE|ip-frag FRAG|ip-protocol PROTOCOL|wlan-radio-id ID|wlan-radio-mac MAC|wlan-is-ctl-pkt|"CTC_CLI_IPFIX_KEY_CID_STR "|"CTC_CLI_IPFIX_KEY_L4_STR"} (dir DIR) (detail|) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry key",
        "Ipv6 key",
        "Source phyPort",
        CTC_CLI_GPORT_ID_DESC,

        CTC_CLI_FIELD_SEL_ID_DESC,
        CTC_CLI_FIELD_SEL_ID_VALUE,
        "Config profile id",
        "Profile id",
        CTC_CLI_KEY_PORT_STR_DESC,
        "IP-sa",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "IP-da",
        CTC_CLI_IPV6_FORMAT,
        "Mask-len",
        "Mask len value<8,12,16,...,128>",
        "DSCP",
        "DSCP value",
        "Flow label",
        "Flow label value, it will cover l4_port",
        "TTL",
        "TTL Value",
        "Ecn",
        "Ecn value",
        "TCP-flags",
        "Flags value",
        "IP-frag",
        "IP fragement value",
        "IP-protocol",
        "IP protocol value",
        "Radio-id",
        "ID value",
        "Radio mac",
        CTC_CLI_MAC_FORMAT,
        "Is wlan control packet",
        CTC_CLI_IPFIX_KEY_CID_STR_DESC,
        CTC_CLI_IPFIX_KEY_L4_STR_DESC,
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_ipfix_data_t use_data;
    uint8 index = 0;
    uint8 mask_len = 0;
    ipv6_addr_t ipv6_address;
    uint32 rst_hit = 0;
    uint32 rst_key_index = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    uint32 detail = 0;
    uint32 entry_idx = 0;

    sal_memset(&use_data, 0, sizeof(ctc_ipfix_data_t));

    use_data.key_type = CTC_IPFIX_KEY_HASH_IPV6;

    index = CTC_CLI_GET_ARGC_INDEX("src-gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src-gport", use_data.gport, argv[index+1], 0, CTC_MAX_UINT16_VALUE);        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-field-sel");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("hash-field-sel", use_data.field_sel_id, argv[index+1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("profile-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("profile-id", use_data.profile_id, argv[index + 1],0,CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("gport", use_data.gport, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_GPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("lport", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_LOGIC_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("metadata");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("metadata", use_data.logic_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        use_data.port_type = CTC_IPFIX_PORT_TYPE_METADATA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-sa");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipsa_masklen = mask_len;
        use_data.l3_info.ipv6.ipsa[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipsa[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipsa[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipsa[3] = sal_htonl(ipv6_address[3]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-da");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index+1]);
        CTC_CLI_GET_UINT8_RANGE("mask", mask_len, argv[index+3],0, CTC_MAX_UINT8_VALUE);
        use_data.l3_info.ipv6.ipda_masklen = mask_len;
        use_data.l3_info.ipv6.ipda[0] = sal_htonl(ipv6_address[0]);
        use_data.l3_info.ipv6.ipda[1] = sal_htonl(ipv6_address[1]);
        use_data.l3_info.ipv6.ipda[2] = sal_htonl(ipv6_address[2]);
        use_data.l3_info.ipv6.ipda[3] = sal_htonl(ipv6_address[3]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("src-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("src cid", use_data.src_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_SRC_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("dst cid", use_data.dst_cid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        use_data.flags |= CTC_IPFIX_DATA_DST_CID_VALID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("l4-type", use_data.l4_info.type.l4_type, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-sub-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-sub-type", use_data.l4_info.l4_sub_type, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aware-tunnel-info-en");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("aware-tunnel-info-en", use_data.l4_info.aware_tunnel_info_en, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-src-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src", use_data.l4_info.l4_port.source_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l4-dst-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-dst", use_data.l4_info.l4_port.dest_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-type", use_data.l4_info.icmp.icmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("icmp-code");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("icmp-code", use_data.l4_info.icmp.icmpcode, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igmp-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("igmp-type", use_data.l4_info.igmp.igmp_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gre-key");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("gre-key", use_data.l4_info.gre_key, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dscp");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("dscp", use_data.l3_info.ipv6.dscp, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("flow-label");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("flow label", use_data.l3_info.ipv6.flow_label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ttl", use_data.l3_info.ipv6.ttl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecn");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ecn", use_data.l3_info.ipv6.ecn, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-protocol");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-protocol", use_data.l4_info.type.ip_protocol, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-frag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("ip-frag", use_data.l3_info.ip_frag, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcp-flags");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("tcp-flags", use_data.l4_info.tcp_flags, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vni");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("vni", use_data.l4_info.vni, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("radio-id", use_data.l4_info.wlan.radio_id, argv[index+1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-radio-mac");
    if (index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("radio-mac", use_data.l4_info.wlan.radio_mac, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wlan-is-ctl-pkt");
    if (index != 0xFF)
    {
        use_data.l4_info.wlan.is_ctl_pkt = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("dir", use_data.dir, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_key(lchip, &use_data, &rst_hit, &rst_key_index);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    if (detail == 1)
    {
        entry_idx = rst_key_index + (detail<<16);
    }
    else
    {
        entry_idx = rst_key_index;
    }

    ret = sys_usw_ipfix_show_entry_info(&use_data, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_show_by_index,
        ctc_cli_usw_ipfix_show_by_index_cmd,
        "show ipfix entry-info by (entry-idx IDX) (entry-type (mac-key|ipv4-key|ipv6-key|mpls-key|l2l3-key)) {dir DIR|detail|lchip LCHIP|}",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        "By",
        "Entry index",
        "Entry index vlave",
        "Entry type",
        "L2 key",
        "Ipv4 key",
        "Ipv6 key",
        "Mpls key",
        "L2l3 key",
        "Direction",
        "<0:Ingress,1:Egress>",
        "Detail",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint32 entry_idx = 0;
    int32 ret = CLI_SUCCESS;
    uint8 lchip = 0;
    ctc_ipfix_data_t  info;
    uint32 detail = 0;
    uint8 index = 0;
    uint8 key_type = 0;
    uint8 dir = CTC_INGRESS;

    sal_memset(&info, 0, sizeof(ctc_ipfix_data_t));

    index = CTC_CLI_GET_ARGC_INDEX("entry-type");
    if(index != 0xFF)
    {
        if (0 == sal_memcmp("mac", argv[index + 1], sal_strlen("mac")))
        {
            key_type = CTC_IPFIX_KEY_HASH_MAC;
        }
        else if (0 == sal_memcmp("ipv4", argv[index + 1], sal_strlen("ipv4")))
        {
            key_type = CTC_IPFIX_KEY_HASH_IPV4;
        }
        else if (0 == sal_memcmp("ipv6", argv[index + 1], sal_strlen("ipv6")))
        {
            key_type = CTC_IPFIX_KEY_HASH_IPV6;
        }
        else if (0 == sal_memcmp("mpls", argv[index + 1], sal_strlen("mpls")))
        {
            key_type = CTC_IPFIX_KEY_HASH_MPLS;
        }
        else if (0 == sal_memcmp("l2l3", argv[index + 1], sal_strlen("l2l3")))
        {
            key_type = CTC_IPFIX_KEY_HASH_L2_L3;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-idx");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("entry idx", entry_idx, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("dir");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("dir", dir, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (index != 0xFF)
    {
        detail = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    ret = sys_usw_ipfix_get_entry_by_index(lchip, entry_idx, dir, key_type, &info);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    info.dir = dir;
    ret = sys_usw_ipfix_get_ad_by_index(lchip, entry_idx, &info);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");
    if(detail == 1)
    {
        entry_idx = entry_idx + (detail<<16);
    }

    ret = sys_usw_ipfix_show_entry_info(&info, &entry_idx);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_usw_ipfix_dump,
        ctc_cli_usw_ipfix_dump_cmd,
        "show ipfix entry-info (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Ipfix module",
        "Entry info",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)

{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint16 traverse_action = 0;
    uint8 lchip = 0;
    ctc_ipfix_traverse_t ipfix_traverse;

    sal_memset(&ipfix_traverse, 0, sizeof(ctc_ipfix_traverse_t));
     /*traverse_action = SYS_IPFIX_DUMP_ENYRY_INFO;*/
    traverse_action = 0x0011;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ctc_cli_out("\n%-11s%-12s%-7s%-8s%-9s%-9s%-12s%-7s\n", "ENTRY_IDX", "ENTRY_TYPE", "PORT", "SEL_ID", "MAX_TTL", "MIN_TTL", "TIMESTAMP", "DIR");
    ctc_cli_out("--------------------------------------------------------------------------\n");

    ipfix_traverse.lchip_id = lchip;
    while (!ipfix_traverse.is_end)
    {
        ipfix_traverse.entry_num = 100;
        ipfix_traverse.start_index = ipfix_traverse.next_traverse_index;
        ipfix_traverse.user_data = & traverse_action;

        if (g_ctcs_api_en)
        {
            ret = ctcs_ipfix_traverse(g_api_lchip, sys_usw_ipfix_show_entry_info, &ipfix_traverse);
        }
        else
        {
            ret = ctc_ipfix_traverse(sys_usw_ipfix_show_entry_info, &ipfix_traverse);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

int32
sys_usw_ipfix_remove_cmp(ctc_ipfix_data_t* p_info, void* user_data)
{
    return 0;
}
CTC_CLI(ctc_cli_usw_ipfix_remove_all_,
         ctc_cli_usw_ipfix_remove_all_cmd,
         "ipfix remove-all entry (lchip LCHIP|)",
         CTC_CLI_IPFIX_M_STR,
         "remove all",
         "entry",
         CTC_CLI_LCHIP_ID_STR,
         CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint16 traverse_action = 0;
    uint8 lchip = 0;
    ctc_ipfix_traverse_t ipfix_traverse;

    sal_memset(&ipfix_traverse, 0, sizeof(ctc_ipfix_traverse_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    ipfix_traverse.lchip_id = lchip;

    while (!ipfix_traverse.is_end)
    {
        ipfix_traverse.entry_num = 100;
        ipfix_traverse.start_index = ipfix_traverse.next_traverse_index;
        ipfix_traverse.user_data = & traverse_action;

        if (g_ctcs_api_en)
        {
            ret = ctcs_ipfix_traverse_remove(g_api_lchip, sys_usw_ipfix_remove_cmp, &ipfix_traverse);
        }
        else
        {
            ret = ctc_ipfix_traverse_remove(sys_usw_ipfix_remove_cmp, &ipfix_traverse);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }
    return CLI_SUCCESS;
}
/**
 @brief  Initialize sdk oam module CLIs
*/
int32
ctc_usw_ipfix_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_clear_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_delete_by_index_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_dump_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_l2_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_l2l3_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_ipv4_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_mpls_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_ipv6_hashkey_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_by_index_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_remove_all_cmd);

    return CLI_SUCCESS;
}

int32
ctc_usw_ipfix_cli_deinit(void)
{
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_status_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_stats_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_clear_stats_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_delete_by_index_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_dump_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_l2_hashkey_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_l2l3_hashkey_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_ipv4_hashkey_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_mpls_hashkey_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_ipv6_hashkey_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_ipfix_show_by_index_cmd);

    return CLI_SUCCESS;
}

#endif

