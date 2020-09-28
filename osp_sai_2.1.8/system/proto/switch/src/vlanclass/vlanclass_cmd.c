#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlanclass_rule_define.h"
#include "gen/tbl_vlanclass_rule.h"
#include "gen/tbl_vlanclass_group_define.h"
#include "gen/tbl_vlanclass_group.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "gen/tbl_sys_spec_define.h"
#include "gen/tbl_sys_spec.h"
#include "switch.h"
#include "glb_macro.h"
#include "vlanclass.h"
#include "lib_spec.h"
#include "cdb_ds.h"

int32
vlanclass_cmd_rule_compare(tbl_vlanclass_rule_t *p_vclass_rule)
{
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_rule_key_t vclass_rkey;
    uint32 i = 0;
    for (i = 0; i < GLB_VLANCLASS_RULE_ID_MAX; i++)
    {
        sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_rkey.rule_id = i;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
        if (NULL == p_db_vclass_rule)
        {
            continue;
        }
        if (p_vclass_rule->rule_type != p_db_vclass_rule->rule_type)
        {
            continue;
        }
        if (p_vclass_rule->vlan_id != p_db_vclass_rule->vlan_id)
        {
            continue;
        }
        switch (p_vclass_rule->rule_type)
        {
        case GLB_VLAN_CLASSIFIER_MAC:
            if (0 == sal_memcmp (p_vclass_rule->mac, p_db_vclass_rule->mac, MAC_ADDR_LEN))
            {
                return -1;
            }
            break;
        case GLB_VLAN_CLASSIFIER_IPV4:
        case GLB_VLAN_CLASSIFIER_IPV6:
            if (0 == sal_memcmp(&p_vclass_rule->ip_address, &p_db_vclass_rule->ip_address, sizeof(addr_t)))
            {
                return -1;
            }
            break;
        case GLB_VLAN_CLASSIFIER_PROTOCOL:
            if (p_vclass_rule->protocol_type == p_db_vclass_rule->protocol_type &&
                p_vclass_rule->protocol_action == p_db_vclass_rule->protocol_action)
            {
                return -1;
            }
            break;
        }
    }
    return PM_E_NONE;
}

int32
vlanclass_cmd_add_rule(tbl_vlanclass_rule_t *p_vclass_rule, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_key;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    uint32 rule_id = 0;
    int32 rc = PM_E_NONE;

    sal_memset(&vclass_key, 0, sizeof(tbl_vlanclass_rule_key_t));
    
    /* 1. config check */
    rule_id = p_vclass_rule->key.rule_id;
    vclass_key.rule_id = rule_id;
    p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_key);
    if (p_db_vclass_rule)
    {
        CFG_CONFLICT_RET("Rule %d is used for another classifier", rule_id);
    }
    
    /* 2. check duplicate config */
    rc = vlanclass_cmd_rule_compare(p_vclass_rule);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Identical rule exists");
    }

    /* 3. call API */
    rc = vlanclass_add_rule(p_vclass_rule);
        
    return rc;
}

int32
vlanclass_cmd_del_rule(uint32 rule_id, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_key;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    sal_memset(&vclass_key, 0, sizeof(tbl_vlanclass_rule_key_t));
    vclass_key.rule_id = rule_id;
    p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_key);
    if (NULL == p_db_vclass_rule)
    {
        CFG_CONFLICT_RET("Rule %d does not exist!", rule_id);
    }
    
    /* 2. check duplicate config */

    /* 3. call API */
    rc = vlanclass_del_rule(rule_id);
        
    return rc;
}

int32
vlanclass_cmd_del_rule_all()
{
    int32	idx = 0;
    int32	idx_max_rule = 4096;
    tbl_vlanclass_rule_key_t vclass_key;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    int32 rc = PM_E_NONE;
    
    /* 1. config check */
    for(idx = 0; idx < idx_max_rule; idx++)
	{
        sal_memset(&vclass_key, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_key.rule_id = idx;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_key);
        if (NULL == p_db_vclass_rule)
        {
            continue;
        }

        rc = vlanclass_del_rule(idx);
	}
        
    return rc;
}


static int32
vlanclass_cmd_get_vlanclass_rule(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    uint32 i = 0;
    int32 rc = PM_E_NONE;


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    for (i = 0; i < GLB_VLANCLASS_RULE_ID_MAX; i++)
    {
        sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
        vclass_rkey.rule_id = i;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
        if (p_db_vclass_rule)
        {
            tbl_vlanclass_rule_dump_one(p_db_vclass_rule, &args);
        }
    }
    sal_fclose(fp);
    
    return rc;
}

int32
vlanclass_cmd_process_vlanclass_rule(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_vlanclass_rule_t vclass_rule;
    cdb_node_t* p_node = NULL;
    uint32 parse_field = FALSE;
    uint32 rule_id = 0;
    uint32 rule_type = 0;
    vid_t vlan_id = 0;
    uint32 pro_type = 0;
    addr_ipv4_t addr_ipv4;
    int32 rc = PM_E_NONE;
   
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_VLANCLASS_RULE);

    sal_memset(&vclass_rule, 0, sizeof(tbl_vlanclass_rule_t));
     
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 4, para->p_rs);
        PM_CFG_GET_UINT(rule_id, argv[0], para->p_rs);
        vclass_rule.key.rule_id = rule_id;    
        if (0 == sal_strcmp(argv[1], "mac"))
        {
            rule_type = GLB_VLAN_CLASSIFIER_MAC;
            rc = cdb_mac_str2val(vclass_rule.mac, argv[2]);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[2]);
            }
            /*added by yejl to fix bug41772, broadcast mac and multicast mac or all zero mac is forbidden to configure*/
            if (GLB_IS_MCAST_MAC(vclass_rule.mac) || GLB_IS_BCAST_MAC(vclass_rule.mac) || GLB_IS_ZERO_MAC(vclass_rule.mac))
            {
                CFG_INVALID_PARAM_RET("Invalid mac address %s", argv[2]); 
            }
        }
        else if (0 == sal_strcmp(argv[1], "ip"))
        {
            rule_type = GLB_VLAN_CLASSIFIER_IPV4;
            CFG_PARSER_IP_ADDR(argv[2], &addr_ipv4, para->p_rs);
            if (IN_BADCLASS(sal_ntoh32(addr_ipv4.s_addr))
                || IN_MULTICAST(sal_ntoh32(addr_ipv4.s_addr))
                || IN_LOOPBACK(sal_ntoh32(addr_ipv4.s_addr)))
            {
                CFG_CONFLICT_RET("Invalid IP address %s", argv[2]);
            }
            vclass_rule.ip_address.family = AF_INET;
            vclass_rule.ip_address.u.prefix4 = addr_ipv4;
        }
        else if (0 == sal_strcmp(argv[1], "protocol"))
        {
            rule_type = GLB_VLAN_CLASSIFIER_PROTOCOL;
            pro_type = cdb_enum_str2val(vlanclass_rule_protype_strs, VLANCLASS_PROTYPE_MAX, argv[2]);
            vclass_rule.protocol_type = pro_type; 
            vclass_rule.protocol_action = VLANCLASS_PROTYPE_REPLACETAG; 
        }
        else if (0 == sal_strcmp(argv[1], "ipv6"))
        {
            rule_type = GLB_VLAN_CLASSIFIER_IPV6;
            rc = cdb_addr_str2val(&vclass_rule.ip_address, argv[2], IPADDR_BUF_SIZE);
            if (rc < 0) 
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[2]);
            }
            if (IN6_IS_ADDR_MULTICAST (&vclass_rule.ip_address.u.prefix6)
                || IN6_IS_ADDR_LOOPBACK (&vclass_rule.ip_address.u.prefix6)
                || IN6_IS_ADDR_UNSPECIFIED (&vclass_rule.ip_address.u.prefix6))
            {
                CFG_INVALID_PARAM_RET("Invalid IP address %s", argv[2]);
            }
        }
        else
        {
            return PM_E_NOT_SUPPORT;
        }
        vclass_rule.rule_type = rule_type;
        PM_CFG_GET_UINT(vlan_id, argv[3], para->p_rs);
        vclass_rule.vlan_id = vlan_id;
        rc = vlanclass_cmd_add_rule(&vclass_rule, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        if(0 == sal_strcmp(argv[0], "all"))
        {
            rc = vlanclass_cmd_del_rule_all();
        }
		else
		{
            PM_CFG_GET_UINT(rule_id, argv[0], para->p_rs);
            rc = vlanclass_cmd_del_rule(rule_id, para);
        }
        break;
        
    case CDB_OPER_SET:
        break;     

    case CDB_OPER_GET:
        parse_field = FALSE;
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        rc = vlanclass_cmd_get_vlanclass_rule(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}


static int32
vlanclass_cmd_check_dynamic_size(tbl_vlanclass_rule_t* p_db_vclass_rule)
{
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    int32 rc = PM_E_NONE;
    if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_MAC)
    {
        if (p_brg_global->mac_based_vlan_class >= lib_spec_get_field(TBL_SYS_SPEC_FLD_MAC_BASED_VLAN_CLASS))
        {
            return -1;
        }
    }
    else if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_IPV4)
    {
        if (p_brg_global->ipv4_based_vlan_class >= lib_spec_get_field(TBL_SYS_SPEC_FLD_IPV4_BASED_VLAN_CLASS))
        {
            return -1;
        }
    }
    else if (p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_IPV6)
    {
        if (p_brg_global->ipv6_based_vlan_class >= lib_spec_get_field(TBL_SYS_SPEC_FLD_IPV6_BASED_VLAN_CLASS))
        {
            return -1;
        }
    }
    return rc;
}


tbl_vlanclass_rule_t*
vlanclass_cmd_rule_content_check(tbl_vlanclass_group_t* p_vclass_group, tbl_vlanclass_rule_t* p_vclass_rule)
{
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_rule_key_t p_vclass_rkey;
    uint32 i = 0;

    if (NULL == p_vclass_group)
    {
        return NULL;
    }
    for (i = 0; i< GLB_VLANCLASS_RULE_ID_MAX; i++)
    {
        if (!GLB_BMP_ISSET(p_vclass_group->rule_id, i))
        {
            continue;
        }
        sal_memset(&p_vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
        p_vclass_rkey.rule_id  = i;
        p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&p_vclass_rkey);
        if (NULL == p_db_vclass_rule)
        {
            continue;
        }
        if (p_vclass_rule->rule_type != p_db_vclass_rule->rule_type)
        {
            continue;
        }
        switch (p_vclass_rule->rule_type)
        {
        case GLB_VLAN_CLASSIFIER_MAC:
            if (0 == sal_memcmp (p_vclass_rule->mac, p_db_vclass_rule->mac, MAC_ADDR_LEN))
            {
                return  p_db_vclass_rule;
            }
            break;
        case GLB_VLAN_CLASSIFIER_IPV4:
        case GLB_VLAN_CLASSIFIER_IPV6:
            if (0 == sal_memcmp(&p_vclass_rule->ip_address, &p_db_vclass_rule->ip_address, sizeof(addr_t)))
            {
                return  p_db_vclass_rule;
            }
            break;
        case GLB_VLAN_CLASSIFIER_PROTOCOL:
            if (p_vclass_rule->protocol_type == p_db_vclass_rule->protocol_type &&
                p_vclass_rule->protocol_action == p_db_vclass_rule->protocol_action)
            {
                return  p_db_vclass_rule;
            }
            break;
        }
    }
    return NULL;
}

int32
vlanclass_cmd_add_rule_to_group(uint32 group_id, uint32 rule_id, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    tbl_vlanclass_rule_t *p_db_vclass_rule_cmp = NULL;
    int32 rc = PM_E_NONE;

    /* 1. vlanclas_rule check */
    vclass_rkey.rule_id = rule_id;
    p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
    if (NULL == p_db_vclass_rule)
    {
        CFG_CONFLICT_RET("Rule %d does not exist", rule_id);
    }
    if ((p_db_vclass_rule->rule_type == GLB_VLAN_CLASSIFIER_PROTOCOL) && (group_id != GLB_VLANCLASS_GROUP_ID_MAX))
    {
        CFG_CONFLICT_RET("Protocol rule only can be added to group 31");
    }

    /* 2. vlanclas_group check */
    vclass_gkey.group_id = group_id;
    p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
    if (p_db_vclass_group)
    {
        if (GLB_BMP_ISSET(p_db_vclass_group->rule_id, rule_id))
        {
            CFG_CONFLICT_RET("Rule %d has been already in group %d", rule_id, group_id);
        }
    }

    /* 3. vlanclass rule content check */
    p_db_vclass_rule_cmp = vlanclass_cmd_rule_content_check(p_db_vclass_group, p_db_vclass_rule);
    if (p_db_vclass_rule_cmp)
    {
        CFG_CONFLICT_RET("The content of rule %d is the same as rule %d in group %d", 
            rule_id, p_db_vclass_rule_cmp->key.rule_id, group_id);
    }

    /* 4. size check */
    rc = vlanclass_cmd_check_dynamic_size(p_db_vclass_rule);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Rule %d can't join group %d for no resource", rule_id, group_id);
    }
    
    /*5. call API*/
    rc = vlanclass_add_rule_to_group(group_id, p_db_vclass_rule);
    if (PM_E_EXIST == rc)
    {
        CFG_CONFLICT_RET("Only one VLAN classifier group can be supported");
    }
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Can not add rule %d to group %d", rule_id, group_id);
    }
    
    /*6. set dynamic count*/
    PM_E_RETURN(vlanclass_set_entry_dynamic_count(p_db_vclass_rule, TRUE));
   
    return PM_E_NONE;
}

int32
vlanclass_cmd_del_rule_from_group(uint32 group_id, uint32 rule_id, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t *p_db_vclass_rule = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    int32 rc = PM_E_NONE;

    /* 1. vlanclas_group check */
    vclass_gkey.group_id = group_id;
    p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
    if (NULL == p_db_vclass_group)
    {
        CFG_CONFLICT_RET("Group %d does not exist", group_id);
    }

    /* 2. vlanclas_rule check */
    vclass_rkey.rule_id = rule_id;
    p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
    if (NULL == p_db_vclass_rule)
    {
        CFG_CONFLICT_RET("Rule %d does not exist", rule_id);
    }
    
    if (!GLB_BMP_ISSET(p_db_vclass_group->rule_id, rule_id))
    {
        CFG_CONFLICT_RET("Rule %d does not in group %d", rule_id, group_id);
    }
    
    /*3. call API*/
    rc = vlanclass_del_rule_from_group(p_db_vclass_group, rule_id);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Can not delete rule %d from group %d", rule_id, group_id);
    }
   
    return PM_E_NONE;
}

int32
vlanclass_cmd_del_group(uint32 group_id, cfg_cmd_para_t *para)
{
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    int32 rc = PM_E_NONE;

    /*1. vlanclas_group check */
    vclass_gkey.group_id = group_id;
    p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
    if (NULL == p_db_vclass_group)
    {
        /*Modified by yejl for bug 47451, 2018-06-04*/
        CFG_CONFLICT_RET("Group %d does not exist", group_id);
    }

    /*2. duplicate check */
    
    /*3. call API*/
    rc = vlanclass_del_group(p_db_vclass_group);
    if (rc < 0)
    {
        CFG_CONFLICT_RET("Can not delete group %d", group_id);
    }
    return PM_E_NONE;
}

static int32
vlanclass_cmd_get_vlanclass_group(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_vlanclass_group_t *p_db_vclass_group = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    uint32 i = 0;
    int32 rc = PM_E_NONE;


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    for (i = 0; i <= GLB_VLANCLASS_GROUP_ID_MAX; i++)
    {
        sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
        vclass_gkey.group_id = i;
        p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
        if (p_db_vclass_group)
        {
            tbl_vlanclass_group_dump_one(p_db_vclass_group, &args);
        }
    }
    sal_fclose(fp);
    
    return rc;
}

int32
vlanclass_cmd_process_vlanclass_group(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    uint32 parse_field = FALSE;
    uint32 group_id = 0;
    uint32 rule_id = 0;
    int32 rc = PM_E_NONE;
   
    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_VLANCLASS_GROUP);

    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT(group_id, argv[0], para->p_rs);
        PM_CFG_GET_UINT(rule_id, argv[1], para->p_rs);
        rc = vlanclass_cmd_add_rule_to_group(group_id, rule_id, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        PM_CFG_GET_UINT(group_id, argv[0], para->p_rs);
        if (1 == argc)
        {
            rc = vlanclass_cmd_del_group(group_id, para);
        }
        else if (2 == argc)
        {
            PM_CFG_GET_UINT(rule_id, argv[1], para->p_rs);
            rc = vlanclass_cmd_del_rule_from_group(group_id, rule_id, para);
        }
        break;
        
    case CDB_OPER_SET:
        break;     

    case CDB_OPER_GET:
        parse_field = FALSE;
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        rc = vlanclass_cmd_get_vlanclass_group(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}

static int32
_vlanclass_cmd_process_vlanclass_rule_id_show_one(tbl_vlanclass_rule_t* p_db_vclass_rule, FILE *fp)
{
    char ipv4_addr_buf[CMD_BUF_256];
    char ipv6_addr_buf[CMD_BUF_256];
    char mac_addr_buf[CMD_BUF_32];
    
    switch (p_db_vclass_rule->rule_type)
    {
    case GLB_VLAN_CLASSIFIER_MAC:
         sal_memset(mac_addr_buf, 0, sizeof(mac_addr_buf));
         sal_fprintf(fp, "vlan classifier rule %u mac %s vlan %u\n", 
                    p_db_vclass_rule->key.rule_id,
                    cdb_mac_addr_val2str(mac_addr_buf, CMD_BUF_32, p_db_vclass_rule->mac),
                    p_db_vclass_rule->vlan_id);
         break;
    case GLB_VLAN_CLASSIFIER_IPV4:
         sal_memset(ipv4_addr_buf, 0, sizeof(ipv4_addr_buf));
         sal_fprintf(fp, "vlan classifier rule %u ip %s vlan %u\n",
                    p_db_vclass_rule->key.rule_id, 
                    cdb_addr_ipv4_val2str(ipv4_addr_buf, CMD_BUF_256, &p_db_vclass_rule->ip_address.u.prefix4),
                    p_db_vclass_rule->vlan_id);
         break;
    case GLB_VLAN_CLASSIFIER_IPV6:
         sal_memset(ipv6_addr_buf, 0, sizeof(ipv6_addr_buf));
         sal_fprintf(fp, "vlan classifier rule %u ipv6 %s vlan %u\n",
                    p_db_vclass_rule->key.rule_id, 
                    cdb_addr_val2str(ipv6_addr_buf, CMD_BUF_256, &p_db_vclass_rule->ip_address), 
                    p_db_vclass_rule->vlan_id);
         break;
    case GLB_VLAN_CLASSIFIER_PROTOCOL:
         sal_fprintf(fp, "vlan classifier rule %u protocol %s vlan %u\n",
                    p_db_vclass_rule->key.rule_id, 
                    cdb_enum_val2str(vlanclass_rule_protype_strs, VLANCLASS_PROTYPE_MAX, p_db_vclass_rule->protocol_type),
                    p_db_vclass_rule->vlan_id);
         break;
   }
   return PM_E_NONE;
}   

int32
vlanclass_cmd_process_vlanclass_rule_id_show(tbl_vlanclass_rule_t* p_vclass_rule, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t* p_db_vclass_rule = NULL;
    uint32 i = 0;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_vclass_rule)
    {
        _vlanclass_cmd_process_vlanclass_rule_id_show_one(p_vclass_rule, fp);
    }
    else
    {
        for (i = 0; i< GLB_VLANCLASS_RULE_ID_MAX; i++)
        {
            sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
            vclass_rkey.rule_id = i;
            p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
            if (NULL == p_db_vclass_rule)
            {
                continue;
            }
            _vlanclass_cmd_process_vlanclass_rule_id_show_one(p_db_vclass_rule, fp);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32 
vlanclass_cmd_process_vlanclass_rule_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vlanclass_rule_key_t vclass_rkey;
    tbl_vlanclass_rule_t* p_db_vclass_rule = NULL;
    uint32 rule_id = 0;
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            vlanclass_cmd_process_vlanclass_rule_id_show(NULL, para);
        }
        else if (argc == 1)
        {
            PM_CFG_GET_UINT(rule_id, argv[0], para->p_rs);
            sal_memset(&vclass_rkey, 0, sizeof(tbl_vlanclass_rule_key_t));
            vclass_rkey.rule_id = rule_id;
            p_db_vclass_rule = tbl_vlanclass_rule_get_vlanclass_rule(&vclass_rkey);
            if (NULL == p_db_vclass_rule)
            {
                CFG_CONFLICT_RET("Rule %u does not exist", rule_id);
            }
            vlanclass_cmd_process_vlanclass_rule_id_show(p_db_vclass_rule ,para);
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
    return PM_E_NONE;
}

static int32
_vlanclass_cmd_process_vlanclass_group_id_show_one(tbl_vlanclass_group_t* p_db_vclass_group, FILE *fp)
{
    uint32 j = 0;
    
    for (j = 0; j< GLB_VLANCLASS_RULE_ID_MAX; j++)
    {
        if (GLB_BMP_ISSET(p_db_vclass_group->rule_id, j))
        {
            sal_fprintf(fp, "vlan classifier group %u add rule %u\n", p_db_vclass_group->key.group_id, j);
        }
    }
    return PM_E_NONE;
}  

int32
vlanclass_cmd_process_vlanclass_group_id_show(tbl_vlanclass_group_t* p_vclass_group, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t* p_db_vclass_group = NULL;
    uint32 i = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_vclass_group)
    {
        _vlanclass_cmd_process_vlanclass_group_id_show_one(p_vclass_group, fp);
    }
    else
    {
        for (i = 0; i<= GLB_VLANCLASS_GROUP_ID_MAX; i++)
        {
            sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
            vclass_gkey.group_id = i;
            p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
            if (NULL == p_db_vclass_group)
            {
                continue;
            }
            _vlanclass_cmd_process_vlanclass_group_id_show_one(p_db_vclass_group, fp);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}


int32
vlanclass_cmd_process_vlanclass_group_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t* p_db_vclass_group = NULL;
    uint32 group_id = 0;
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            vlanclass_cmd_process_vlanclass_group_id_show(NULL, para);
        }
        else if (argc == 1)
        {
            PM_CFG_GET_UINT(group_id, argv[0], para->p_rs);
            sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
            vclass_gkey.group_id= group_id;
            p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
            if (NULL == p_db_vclass_group)
            {
                CFG_CONFLICT_RET("Group %u does not exist", group_id);
            }
            vlanclass_cmd_process_vlanclass_group_id_show(p_db_vclass_group ,para);
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }
    return PM_E_NONE;
}

static int32
_vlanclass_cmd_process_vlanclass_if_group_show_one(tbl_vlanclass_group_t* p_db_vclass_group, FILE *fp)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
        
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (0 == p_db_if->vlanclass_active_type)
        {
            continue;
        }
        if (GLB_VLAN_CLASS_MAC_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, "vlan classifier group %u on interface %s, based mac\n", 
                p_db_vclass_group->key.group_id, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        else if (GLB_VLAN_CLASS_IP_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, "vlan classifier group %u on interface %s, based ip\n", 
                p_db_vclass_group->key.group_id, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
        else if (GLB_VLAN_CLASS_PROTOCOL_BASED == p_db_if->vlanclass_active_type)
        {
            sal_fprintf(fp, "vlan classifier group %u on interface %s, based protocol\n", 
                p_db_vclass_group->key.group_id, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        }
    }
    return PM_E_NONE;
}

int32
vlanclass_cmd_process_vlanclass_if_group_show(tbl_vlanclass_group_t* p_vclass_group, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t* p_db_vclass_group = NULL;
    uint32 i = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_vclass_group)
    {
        _vlanclass_cmd_process_vlanclass_if_group_show_one(p_vclass_group, fp);
    }
    else
    {
        for (i = 0; i<= GLB_VLANCLASS_GROUP_ID_MAX; i++)
        {
            sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
            vclass_gkey.group_id = i;
            p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
            if (NULL == p_db_vclass_group)
            {
                continue;
            }
            _vlanclass_cmd_process_vlanclass_if_group_show_one(p_db_vclass_group, fp);
        }
    }
    sal_fclose(fp);
    fp = NULL;
    
    return PM_E_NONE;
}

int32
vlanclass_cmd_process_vlanclass_if_show(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_vlanclass_group_key_t vclass_gkey;
    tbl_vlanclass_group_t* p_db_vclass_group = NULL;
    uint32 group_id = 0;
    int32 rc = PM_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            vlanclass_cmd_process_vlanclass_if_group_show(NULL, para);
        }
        else if (argc == 1)
        {
            PM_CFG_GET_UINT(group_id, argv[0], para->p_rs);
            sal_memset(&vclass_gkey, 0, sizeof(tbl_vlanclass_group_key_t));
            vclass_gkey.group_id= group_id;
            p_db_vclass_group = tbl_vlanclass_group_get_vlanclass_group(&vclass_gkey);
            if (NULL == p_db_vclass_group)
            {
                CFG_CONFLICT_RET("Group %u does not exist", group_id);
            }
            vlanclass_cmd_process_vlanclass_if_group_show(p_db_vclass_group ,para);
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    return PM_E_NONE;
}

