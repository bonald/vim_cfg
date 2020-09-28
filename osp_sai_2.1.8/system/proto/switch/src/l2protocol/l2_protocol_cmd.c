#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h" 
#include "ctclib_opb.h"
#include "switch.h"
#include "cdb_ds.h"
#include "errdisable.h"
#include "l2_protocol.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"

extern ctclib_opb_t l2protocol_index_opb;
mac_addr_t l2pro_lldp_dmac      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};
mac_addr_t l2pro_bpdu_dmac      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
mac_addr_t l2pro_lacp_dmac      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};
mac_addr_t l2pro_eapal_dmac     = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};

int32
l2_protocol_cmd_set_group_mac(tbl_l2_protocol_t* p_l2protocol, tbl_l2_protocol_t* p_db_l2protocol, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char mac_str[MAC_ADDR_STR_LEN];
    
    tbl_l2_protocol_master_t *p_master = tbl_l2_protocol_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol1 = NULL;
    
    if (p_l2protocol->group_mac[0] != 0x01)
    {
        CFG_INVALID_PARAM_RET("Invalid mac address %s, MAC address should begin with 0x01", 
                               cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->group_mac));
    }
    if (!((p_l2protocol->group_mac[0] != 0x01) || (p_l2protocol->group_mac[1] != 0x80)|| 
        (p_l2protocol->group_mac[2] != 0xC2) || (p_l2protocol->group_mac[3] != 0x00)|| 
        (p_l2protocol->group_mac[4] != 0x00) || (p_l2protocol->group_mac[5] > 0x2f)))
    {
        CFG_INVALID_PARAM_RET("Invalid mac address %s, MAC address should exclude 0180.C200.0000 ~ 0180.C200.002F", 
                               cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->group_mac));
    }

    CTCLIB_SLIST_LOOP(p_master->l2_protocol_list, p_db_l2_protocol1, listnode)
    {
        if (!sal_memcmp(p_l2protocol->group_mac, p_db_l2_protocol1->group_mac, MAC_ADDR_LEN) 
            &&((0 != sal_strcmp(p_l2protocol->key.name, p_db_l2_protocol1->key.name))))
        {
            CFG_INVALID_PARAM_RET("Cannot configure MAC address %s, the mac address has been configured by other protocol", 
                                   cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->group_mac));
        }
    }
    
    if (!sal_memcmp(p_l2protocol->group_mac, p_db_l2protocol->group_mac, MAC_ADDR_LEN))
    {
        return PM_E_NONE;
    }

    PM_E_RETURN(l2_protocol_set_l2_protocol_group_mac(p_l2protocol));
    return rc ;
}

int32
l2_protocol_cmd_add_user_defined_protocol(tbl_l2_protocol_t* p_l2protocol, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char mac_str[MAC_ADDR_STR_LEN];
    uint32 non_standard_count = 0;
    tbl_l2_protocol_master_t *p_master = tbl_l2_protocol_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol = NULL;

    if ((p_l2protocol->protocol_mac[0] != 0x01) || (p_l2protocol->protocol_mac[1] != 0x80)|| 
        (p_l2protocol->protocol_mac[2] != 0xC2) || (p_l2protocol->protocol_mac[3] != 0x00)|| 
        (p_l2protocol->protocol_mac[4] != 0x00) || (p_l2protocol->protocol_mac[5] > 0x2f))
    {
        CFG_INVALID_PARAM_RET("Invalid mac address %s, MAC address should include 0180.C200.0000 ~ 0180.C200.002F", 
                               cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->protocol_mac));
    }
    
    if (!sal_memcmp(p_l2protocol->protocol_mac, l2pro_eapal_dmac, MAC_ADDR_LEN)||
        !sal_memcmp(p_l2protocol->protocol_mac, l2pro_lacp_dmac, MAC_ADDR_LEN)||
        !sal_memcmp(p_l2protocol->protocol_mac, l2pro_bpdu_dmac, MAC_ADDR_LEN)||
        !sal_memcmp(p_l2protocol->protocol_mac, l2pro_lldp_dmac, MAC_ADDR_LEN))
    {
        CFG_INVALID_PARAM_RET("Cannot configure protocol-mac %s, it is same with one of dest mac of dot1x or lldp or slow-proto or stp", 
                               cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->protocol_mac));
    }

    CTCLIB_SLIST_LOOP(p_master->l2_protocol_list, p_db_l2_protocol, listnode)
    {
        if (!sal_memcmp(p_l2protocol->protocol_mac, p_db_l2_protocol->protocol_mac, MAC_ADDR_LEN) 
            &&((0 != sal_strcmp(p_l2protocol->key.name, p_db_l2_protocol->key.name))))
        {
            CFG_INVALID_PARAM_RET("Cannot configure protocol mac address %s, the mac address has been configured by other protocol", 
                                   cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_l2protocol->protocol_mac));
        }
        if (!p_db_l2_protocol->is_standard_pro)  
        {
            non_standard_count++;
        }
    }
    
    if (non_standard_count >= 4)
    {
        CFG_INVALID_PARAM_RET("Cannot configure %s, only 4 user defined protocols can be configured", p_l2protocol->key.name);
    }
    
    PM_E_RETURN(l2_protocol_add_l2_protocol(p_l2protocol));
    return PM_E_NONE;
}

static int32
l2_protocol_cmd_get_l2_protocol(char *l2pro_name, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_l2_protocol_key_t l2protocol_key;
    tbl_l2_protocol_t*  p_db_l2protocol = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    if (NULL == l2pro_name)
    {
        p_node->tbl_info->ops.iter_fn((TBL_ITER_CB_FUNC)tbl_l2_protocol_dump_one, &args);
    }
    else
    {
        sal_memset(&l2protocol_key, 0, sizeof(l2protocol_key));
        sal_strncpy(l2protocol_key.name, l2pro_name, GLB_L2_PROTOCOL_NAME_SIZE);
        p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(&l2protocol_key);
        if (!p_db_l2protocol)
        {
            CFG_DEL_SHOW_PATH(para->p_show->path, fp);
            CFG_CONFLICT_RET("L2 protocol %s does not exist", l2pro_name);
        }        
        tbl_l2_protocol_dump_one(p_db_l2protocol, &args);
    }
    sal_fclose(fp);
    fp = NULL;

    return PM_E_NONE;
}

int32
l2_protocol_cmd_process_l2_protocol(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_l2_protocol_t l2protocol;
    tbl_l2_protocol_key_t l2protocol_key;
    tbl_l2_protocol_t* p_db_l2protocol = NULL;
    char *l2pro_name = NULL;
    uint32 proto_index = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_L2_PROTOCOL);

    sal_memset(&l2protocol, 0, sizeof(l2protocol));
    sal_memset(&l2protocol_key, 0, sizeof(l2protocol_key));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
        if (0 == sal_strcmp(argv[0], "user-defined")) /*add user-defined l2protocol and update l2protocol protocol mac*/
        {
            if (check_name_character_and_len(argv[1], GLB_L2_PROTOCOL_NAME_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("The first character of protocol name should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20.");
            }
            /*Modified by yejl for bug 48263, 2018-08-06*/
            if ((0 == sal_strcasecmp(argv[1], "dot1x")) || (0 == sal_strcasecmp(argv[1], "slow-proto"))|| 
                (0 == sal_strcasecmp(argv[1], "lldp")) || (0 == sal_strcasecmp(argv[1], "stp")) || 
                (0 == sal_strcasecmp(argv[1], "uplink")))
            {
                CFG_INVALID_PARAM_RET("The user defined protocol name can not be dot1x or slow-proto or lldp or stp or uplink (include uppercases and lowercases)");
            }
            
            sal_strncpy(l2protocol.key.name, argv[1], GLB_L2_PROTOCOL_NAME_SIZE+1);
            p_db_l2protocol = tbl_l2_protocol_get_l2_protocol((&l2protocol.key));
            if (p_db_l2protocol) /*update user-defined l2protocol protocol mac: not allowed*/
            {
                rc = cdb_mac_str2val(l2protocol.protocol_mac, argv[2]);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid protocol mac address %s", argv[2]);
                }
                if (sal_memcmp(l2protocol.protocol_mac, p_db_l2protocol->protocol_mac, MAC_ADDR_LEN))
                {
                    CFG_INVALID_PARAM_RET("User defined protocol %s can not update protocol mac", argv[1]);
                }
                else
                {
                    return PM_E_NONE;    
                }
            }
            else /*add user-defined l2protocol*/
            {
                rc = cdb_mac_str2val(l2protocol.protocol_mac, argv[2]);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid protocol mac address %s", argv[2]);
                }
                rc = ctclib_opb_alloc_offset(&l2protocol_index_opb, &proto_index);
                if (PM_E_NONE != rc)
                {
                    return rc;
                }
                l2protocol.protocol_index = proto_index;
                rc = l2_protocol_cmd_add_user_defined_protocol(&l2protocol, para);
                if (rc)
                {
                    rc = ctclib_opb_free_offset(&l2protocol_index_opb, proto_index);
                    if (PM_E_NONE != rc)
                    {
                        return rc;
                    }
                }
            } 
        }
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        if (0 == sal_strcmp(argv[0], "user-defined")) /*del user-defined l2protocol*/
        {
            if (check_name_character_and_len(argv[1], GLB_L2_PROTOCOL_NAME_SIZE) != 0)
            {
                CFG_INVALID_PARAM_RET("The first character of protocol name should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 20.");
            }
            /*Modified by yejl for bug 48263, 2018-08-06*/
            if ((0 == sal_strcasecmp(argv[1], "dot1x")) || (0 == sal_strcasecmp(argv[1], "slow-proto"))|| 
                (0 == sal_strcasecmp(argv[1], "lldp")) || (0 == sal_strcasecmp(argv[1], "stp")) ||
                (0 == sal_strcasecmp(argv[1], "uplink")))
            {
                CFG_INVALID_PARAM_RET("The user defined protocol name can not be dot1x or slow-proto or lldp or stp or uplink (include uppercases and lowercases)");
            }
            sal_strncpy(l2protocol_key.name, argv[1], GLB_L2_PROTOCOL_NAME_SIZE+1);
            p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(&l2protocol_key);
            if (p_db_l2protocol)
            {
                /*process user defined protocol group mac*/
                proto_index = p_db_l2protocol->protocol_index;
                PM_E_RETURN(l2_protocol_del_user_define_protocol_on_interface(p_db_l2protocol));
                PM_E_RETURN(tbl_l2_protocol_del_l2_protocol(&l2protocol_key));

                rc = ctclib_opb_free_offset(&l2protocol_index_opb, proto_index);
                if (rc)
                {
                    return rc;
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Protocol name %s does not exist", argv[1]);    
            }
        }
        
        break;

    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
        if (argc == 2) /*update group-mac: no l2protocol XXXX group-mac*/
        {
            sal_strncpy(l2protocol.key.name, argv[1], GLB_L2_PROTOCOL_NAME_SIZE+1);
            p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(&(l2protocol.key));
            if (p_db_l2protocol)
            {
                sal_memset(l2protocol.group_mac, 0, MAC_ADDR_LEN); 
                PM_E_RETURN(l2_protocol_set_l2_protocol_group_mac(&l2protocol));
            }
            else
            {
                CFG_INVALID_PARAM_RET("Protocol name %s does not exist", argv[1]); 
            }
        }
        else if (argc == 3) /*update group-mac: l2protocol XXXX group-mac X.X.X*/
        {
            sal_strncpy(l2protocol.key.name, argv[1], GLB_L2_PROTOCOL_NAME_SIZE+1);
            p_db_l2protocol = tbl_l2_protocol_get_l2_protocol(&(l2protocol.key));
            if (p_db_l2protocol)
            {
                rc = cdb_mac_str2val(l2protocol.group_mac, argv[2]);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid group mac address %s", argv[2]);
                }
                rc = l2_protocol_cmd_set_group_mac(&l2protocol, p_db_l2protocol, para);
            }
            else
            {
                CFG_INVALID_PARAM_RET("Protocol name %s does not exist", argv[1]); 
            }
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            l2pro_name = NULL;
        }
        else if (1 == argc)
        {
            l2pro_name = argv[0];
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = l2_protocol_cmd_get_l2_protocol(l2pro_name, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
l2_protocol_show_l2_protocol_global(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_brg_global_t* p_brg_global = tbl_brg_global_get_brg_global();
    tbl_l2_protocol_master_t *p_master = tbl_l2_protocol_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_l2_protocol_t* p_db_l2_protocol = NULL;
    char mac_str1[MAC_ADDR_STR_LEN];
    char mac_str2[MAC_ADDR_STR_LEN];

    sal_fprintf(fp, "L2protocol cos: %d\n", p_brg_global->l2protocol_cos);
    sal_fprintf(fp, "Protocol Name         Type           Protocol mac    Group mac\n");
    sal_fprintf(fp, "---------------------+--------------+---------------+---------------\n");
    
    CTCLIB_SLIST_LOOP(p_master->l2_protocol_list, p_db_l2_protocol, listnode)
    {
        if (p_db_l2_protocol->is_standard_pro)
        {
            sal_fprintf(fp, "%-21s %-14s %-15s %-15s\n", 
                p_db_l2_protocol->key.name, 
                "standard", 
                GLB_IS_ZERO_MAC(p_db_l2_protocol->protocol_mac) ? "N/A" : cdb_mac_val2str(mac_str1, MAC_ADDR_STR_LEN, p_db_l2_protocol->protocol_mac),
                GLB_IS_ZERO_MAC(p_db_l2_protocol->group_mac) ? "N/A": cdb_mac_val2str(mac_str2, MAC_ADDR_STR_LEN, p_db_l2_protocol->group_mac));
        }
        else
        {
            sal_fprintf(fp, "%-21s %-14s %-15s %-15s\n", 
                p_db_l2_protocol->key.name, 
                "user-defined", 
                GLB_IS_ZERO_MAC(p_db_l2_protocol->protocol_mac) ? "N/A" : cdb_mac_val2str(mac_str1, MAC_ADDR_STR_LEN, p_db_l2_protocol->protocol_mac),
                GLB_IS_ZERO_MAC(p_db_l2_protocol->group_mac) ? "N/A": cdb_mac_val2str(mac_str2, MAC_ADDR_STR_LEN, p_db_l2_protocol->group_mac));
        }
    }

    sal_fclose(fp);
    fp = NULL;
    return PM_E_NONE;
}

static int32
l2_protocol_show_l2_protocol_interface_one_banner( FILE *fp)
{
    sal_fprintf(fp, "Protocol name         Type          protocol-vlan          Protocol-action\n");
    sal_fprintf(fp, "---------------------+-------------+----------------------+---------------\n");
    return PM_E_NONE;
}

int32
l2_protocol_show_l2_protocol_interface_one(tbl_interface_t* p_db_if, FILE* fp)
{
    tbl_evc_entry_t* p_db_evc_entry = NULL;
    tbl_evc_entry_key_t evc_entry_key;
    ds_l2protocol_t* p_ds_l2protocol = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 banner_is_configure = FALSE;
    uint32 port_is_uplink = FALSE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char tmp_str[CMD_BUF_16];
    char evc_name_str[CMD_BUF_32];

    sal_memset(tmp_str, 0, sizeof(tmp_str));
    sal_memset(evc_name_str, 0, sizeof(evc_name_str));

    if (p_db_if->l2pro_uplink)
    {
        sal_fprintf(fp, "Interface: %s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext)); 
        sal_fprintf(fp, "l2protocol uplink: enable\n");
        port_is_uplink = TRUE;
    }
    CTCLIB_SLIST_LOOP(p_db_if->l2protocol.obj_list, p_ds_l2protocol, listnode)
    {
        if (!banner_is_configure)
        {
            if (!port_is_uplink)
            {
                sal_fprintf(fp, "Interface: %s\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext)); 
                sal_fprintf(fp, "l2protocol uplink: %s\n", p_db_if->l2pro_uplink ? "enable" : "disable");
            }
            l2_protocol_show_l2_protocol_interface_one_banner(fp);
            banner_is_configure = TRUE;
        }            
        if (sal_strlen(p_ds_l2protocol->l2pro_evc_name))
        {
            sal_memset(&evc_entry_key, 0, sizeof(evc_entry_key));
            sal_strncpy(evc_name_str, p_ds_l2protocol->l2pro_evc_name, GLB_VLAN_EVC_NAME_LEN);
            sal_strncpy(evc_entry_key.name, p_ds_l2protocol->l2pro_evc_name, GLB_VLAN_EVC_NAME_LEN);
            p_db_evc_entry = tbl_evc_entry_get_evc_entry(&evc_entry_key);
            if (p_db_evc_entry)
            {
                sal_snprintf(tmp_str, CMD_BUF_16, "(%u)", p_db_evc_entry->mapped_svid); 
                sal_strcat(evc_name_str, tmp_str);
            }
            sal_fprintf(fp, "%-21s %-13s %-22s %-15s\n", 
                p_ds_l2protocol->key.l2pro_name, 
                p_ds_l2protocol->is_standard_pro ? "standard" : "user-defined",
                evc_name_str,
                L2_PROTOCOL_ACTION_STR(p_ds_l2protocol->l2pro_action));
        }
        else if (p_ds_l2protocol->l2pro_vlan)
        {
            sal_fprintf(fp, "%-21s %-13s %-22d %-15s\n", 
                p_ds_l2protocol->key.l2pro_name, 
                p_ds_l2protocol->is_standard_pro ? "standard" : "user-defined",
                p_ds_l2protocol->l2pro_vlan,
                L2_PROTOCOL_ACTION_STR(p_ds_l2protocol->l2pro_action));
        }
        else
        {
            sal_fprintf(fp, "%-21s %-13s %-22d %-15s\n", 
                p_ds_l2protocol->key.l2pro_name, 
                p_ds_l2protocol->is_standard_pro ? "standard" : "user-defined",
                p_db_if->brgif->pvid,
                L2_PROTOCOL_ACTION_STR(p_ds_l2protocol->l2pro_action));
        }
    }
    if (banner_is_configure || port_is_uplink)
    {
        sal_fprintf(fp, "\n");
    }

    return PM_E_NONE;
}

int32
l2_protocol_show_l2_protocol_interface(tbl_interface_t* p_db_if, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    if (p_db_if)
    {
        l2_protocol_show_l2_protocol_interface_one(p_db_if, fp);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            l2_protocol_show_l2_protocol_interface_one(p_db_if, fp);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
l2_protocol_cmd_show_l2_protocol_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    int32  rc = PM_E_NONE;
    
    CFG_DBG_FUNC();
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == sal_strcmp(argv[0], "global"))
        {
            rc = l2_protocol_show_l2_protocol_global(para);
        }
        else if (0 == sal_strcmp(argv[0], "interface"))
        {
            if (argc == 1)
            {
                rc = l2_protocol_show_l2_protocol_interface(NULL, para);
            }
            else
            {
                sal_memset(&if_key, 0, sizeof(if_key));
                sal_strncpy(if_key.name, argv[1], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&if_key);
                if (NULL == p_db_if)
                {
                    IFNAME_ETH2FULL(argv[1], ifname_ext);
                    CFG_INVALID_PARAM_RET("Interface %s does not exist", ifname_ext);
                }
                rc = l2_protocol_show_l2_protocol_interface(p_db_if, para);
            }
        }
        break;
    default:
        break;
    }
    return rc ; 
}
