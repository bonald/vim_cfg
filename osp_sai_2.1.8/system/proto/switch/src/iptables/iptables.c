
#include "proto.h"

#include "glb_iptables_define.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "iptables.h"
#include "swth_sys.h"
#include "lib_ioctl.h"


static int32
_ipt_config_exec(char *pszcmd)
{
    char sz_file[] = "/tmp/ipt_set_file";
    char sz_fullcmd[GLB_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == pszcmd)
    {
        return PM_E_INVALID_PARAM;
    }
    
    snprintf(sz_fullcmd, GLB_IPTABLES_MAX_CMD_BUFSZ, "%s >%s 2>&1",
                 pszcmd, sz_file);
    if (sal_system(sz_fullcmd) == -1)
    {
        return PM_E_HAL_FAIL;
    }

    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_EXIST;
    }
    while (sal_fgets(sz_linebuf, GLB_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Try", 3))
        {
            /* Try...set rule error */
            rc = PM_E_HAL_FAIL;
            break;
        }
    }
    
    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

#define _____IPTABLES_SERVICE_ACL_____

#define MTYPE_IPT_LINK_NODE MEM_PM_TEMP
#define MTYPE_IPT_LINK_LIST MEM_PM_TEMP

struct ipt_acl_rule_s
{
    ctclib_list_node_t node;
    uint32  priority;
    uint32  rule_id;
};
typedef struct ipt_acl_rule_s ipt_acl_rule_t;

ctclib_list_t *ipt_telnet_rules_list = NULL;
ctclib_list_t *ipt_ssh_rules_list    = NULL;


static char *
ipt_seracl_get_chain_name(int service_type)
{
    char *str = NULL;

    switch (service_type)
    {
    case GLB_SERVICE_TYPE_TELNET:
        str = IPTABLES_CHAIN_NAME_TELNET;
        break;
    case GLB_SERVICE_TYPE_SSH:
        str = IPTABLES_CHAIN_NAME_SSH;
        break;
    default:
        break;
    }

    return str;
}


int32
ipt_update_service_acl_port(uint32 service_type, uint32 port)
{
    int32  rc = PM_E_NONE;
    char   sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    uint32 len = 0;
    uint32 chain_idx    = 1;
    uint32 chain_idx_in = chain_idx + 4;

    /* inband small packet filter add two highest priority rules attached to input chain */
    tbl_iptables_prevent_t *p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }
    if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT))
    {
        chain_idx_in += 2;
    }

    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_NETNS_CMD_PREFIX" -R FORWARD %d -p tcp --dport %d -j %s\n", 
        GLB_SERVICE_TYPE_TELNET == service_type ? chain_idx : chain_idx+1,
        port, ipt_seracl_get_chain_name(service_type));
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -R INPUT %d -p tcp --dport %d -j %s", 
        GLB_SERVICE_TYPE_TELNET == service_type ? chain_idx_in : chain_idx_in+1,
        port, ipt_seracl_get_chain_name(service_type));
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    return rc;
}


static int32
ipt_free_rule_id(uint32 priority, uint32 service_type, uint32 *idx)
{
    int32  rc = PM_E_NONE;
    int32  deleted = FALSE;
    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ipt_acl_rule_t *p_rule = NULL;
    ctclib_list_t  *ipt_service_rules_list = NULL;

    ipt_service_rules_list = GLB_SERVICE_TYPE_TELNET == service_type ? ipt_telnet_rules_list : ipt_ssh_rules_list;

    ctclib_list_for_each_safe(p_node, p_next, ipt_service_rules_list)
    {
        p_rule = ctclib_container_of(p_node, ipt_acl_rule_t, node);
        if (deleted)
        {
            p_rule->rule_id--;
            continue;
        }

        if (p_rule->priority == priority)
        {
            *idx = p_rule->rule_id;
            ctclib_list_delete(ipt_service_rules_list, p_node);
            XFREE(MTYPE_IPT_LINK_NODE, p_rule);
            deleted = TRUE;
        }
    }

    return rc;
}

static int32
ipt_alloc_rule_id(uint32 priority, uint32 service_type, uint32 *idx)
{
    int32  rc = PM_E_NONE;
    ipt_acl_rule_t *p_rule = NULL;
    ipt_acl_rule_t *p_rule_new = NULL;
    ctclib_list_node_t *p_node = NULL;
    ctclib_list_node_t *p_next = NULL;
    ctclib_list_t  *ipt_service_rules_list = NULL;

    p_rule_new = XCALLOC(MTYPE_IPT_LINK_NODE, sizeof(ipt_acl_rule_t));
    sal_memset(p_rule_new, 0, sizeof(ipt_acl_rule_t));
    p_rule_new->priority = priority;

    ipt_service_rules_list = GLB_SERVICE_TYPE_TELNET == service_type ? ipt_telnet_rules_list : ipt_ssh_rules_list;

    ctclib_list_for_each_safe(p_node, p_next, ipt_service_rules_list)
    {
        p_rule = ctclib_container_of(p_node, ipt_acl_rule_t, node);

        /* p_rule_new-->p_rule */
        if (priority < p_rule->priority)
        {
            p_rule_new->rule_id = p_rule->rule_id;
            ctclib_list_insert_before(ipt_service_rules_list, p_node, &p_rule_new->node);
            break;
        }
    }

    if (NULL == p_node)
    {
        /* insert to list tail */
        p_node = ctclib_list_tail(ipt_service_rules_list);
        if (NULL != p_node)
        {
            p_rule = ctclib_container_of(p_node, ipt_acl_rule_t, node);
            p_rule_new->rule_id = p_rule->rule_id + 1;
        }
        else
        {
            p_rule_new->rule_id = 1;
        }
        ctclib_list_insert_tail(ipt_service_rules_list, &p_rule_new->node);
    }
    else
    {
        /* increase rule id after new rule */
        while (p_node)
        {
            p_rule = ctclib_container_of(p_node, ipt_acl_rule_t, node);
            p_rule->rule_id++;
            p_node = ctclib_list_next(p_node);
        }
    }

    *idx = p_rule_new->rule_id;

    return rc;
}


static int32
ipt_format_service_acl_rule(char* rule_str_mgmt, char* rule_str_deft, tbl_ace_config_t *p_ace_config)
{
    int32  rc = PM_E_NONE;
    addr_ipv4_t ip_mask;
    char   src_ip[256] = {0};
    char   dst_ip[256] = {0};
    uint32 src_mask_len = 0;
    uint32 dst_mask_len = 0;

    if (p_ace_config->src_ip_mask.u.prefix4.s_addr == 0xffffffff)
    {
        sal_strcpy(src_ip, "0.0.0.0");
    }
    else
    {
        cdb_addr_ipv4_val2str(src_ip, CMD_BUF_256, &p_ace_config->src_ip.u.prefix4);
        ip_mask = p_ace_config->src_ip_mask.u.prefix4;
        ip_mask.s_addr = ~ip_mask.s_addr;
        src_mask_len = ip_convert_masklen(ip_mask);
    }

    if (GLB_FLAG_ISSET(p_ace_config->key_flags, GLB_IP_ACE_IPDA))
    {
        if (p_ace_config->dst_ip_mask.u.prefix4.s_addr == 0xffffffff)
        {
            sal_strcpy(dst_ip, "0.0.0.0");
        }
        else
        {
            cdb_addr_ipv4_val2str(dst_ip, CMD_BUF_256, &p_ace_config->dst_ip.u.prefix4);
            ip_mask = p_ace_config->dst_ip_mask.u.prefix4;
            ip_mask.s_addr = ~ip_mask.s_addr;
            dst_mask_len = ip_convert_masklen(ip_mask);
        }
    }

    if (p_ace_config->deny)
    {
        sal_snprintf(rule_str_mgmt, GLB_IPTABLES_MAX_CMD_BUFSZ, 
            "-s %s/%d -j DROP", src_ip, src_mask_len);
        sal_snprintf(rule_str_deft, GLB_IPTABLES_MAX_CMD_BUFSZ, 
            "-s %s/%d -d %s/%d -j DROP", src_ip, src_mask_len, dst_ip, dst_mask_len);
    }
    else
    {
        sal_snprintf(rule_str_mgmt, GLB_IPTABLES_MAX_CMD_BUFSZ, 
            "-s %s/%d -j ACCEPT", src_ip, src_mask_len);
        sal_snprintf(rule_str_deft, GLB_IPTABLES_MAX_CMD_BUFSZ, 
            "-s %s/%d -d %s/%d -j ACCEPT", src_ip, src_mask_len, dst_ip, dst_mask_len);
    }

    return rc;
}

int32
ipt_del_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type)
{
    int32  rc = PM_E_NONE;
    char   sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    uint32 idx = 0;
    uint32 len = 0;

    ipt_free_rule_id(p_ace_config->key.seq_no, service_type, &idx);

    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_NETNS_CMD_PREFIX" -D %s %d\n", 
        ipt_seracl_get_chain_name(service_type), idx);
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -D %s %d", 
        ipt_seracl_get_chain_name(service_type), idx+1);

    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    return rc;
}

int32
ipt_add_service_acl_rule(tbl_ace_config_t *p_ace_config, uint32 service_type)
{
    int32  rc = PM_E_NONE;
    char   sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    char   sz_rule_mgmt[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    char   sz_rule_deft[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    uint32 idx = 0;
    uint32 len = 0;

    ipt_format_service_acl_rule(sz_rule_mgmt, sz_rule_deft, p_ace_config);
    ipt_alloc_rule_id(p_ace_config->key.seq_no, service_type, &idx);

    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_NETNS_CMD_PREFIX" -I %s %d %s\n", 
        ipt_seracl_get_chain_name(service_type), idx, sz_rule_mgmt);
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -I %s %d %s", 
        ipt_seracl_get_chain_name(service_type), idx+1, sz_rule_deft);

    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    return rc;
}


int32
ipt_del_service_acl_chain(uint32 service_type)
{
    int32  rc = PM_E_NONE;
    char   sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    uint32 len = 0;

    ctclib_list_node_t *p_node = NULL, *p_next = NULL;
    ipt_acl_rule_t *p_rule = NULL;
    ctclib_list_t  *ipt_service_rules_list = NULL;

    /* flush outband iptables rules */
    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_NETNS_CMD_PREFIX" -F %s\n", ipt_seracl_get_chain_name(service_type));
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    /* flush inband iptables rules and add default vemgmt ip ACCEPT rule */
    sal_memset(sz_cmd, 0, sizeof(sz_cmd));
    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_CMD_PREFIX" -F %s\n", ipt_seracl_get_chain_name(service_type));
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -I %s -s %s -j ACCEPT\n",
        ipt_seracl_get_chain_name(service_type), GLB_NS_PORT_FORWARDING_VETH_MGMT_IP_ADDR);
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    /* remove all iptables rule node in list */
    ipt_service_rules_list = GLB_SERVICE_TYPE_TELNET == service_type ? ipt_telnet_rules_list : ipt_ssh_rules_list;

    ctclib_list_for_each_safe(p_node, p_next, ipt_service_rules_list)
    {
        p_rule = ctclib_container_of(p_node, ipt_acl_rule_t, node);
        ctclib_list_delete(ipt_service_rules_list, p_node);
        XFREE(MTYPE_IPT_LINK_NODE, p_rule);
    }

    return rc;
}

int32
ipt_add_service_acl_chain(uint32 service_type)
{
    int32  rc = PM_E_NONE;
    char   sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ] = {0};
    uint32 len = 0;
    uint32 dport = 0;

    /* get service listening port */
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t    *p_ssh_cfg = tbl_ssh_cfg_get_ssh_cfg();

    if (NULL == p_sys_glb || NULL == p_ssh_cfg)
    {
        return PM_E_NONE;
    }

    dport = GLB_SERVICE_TYPE_TELNET == service_type ? p_sys_glb->telnet_port : p_ssh_cfg->listen_port;

    /* add outband iptables chain */
    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_NETNS_CMD_PREFIX" -N %s\n", ipt_seracl_get_chain_name(service_type));
    len = sal_strlen(sz_cmd);
    /* outband chain should attached to FORWARD after DNAT process */
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_NETNS_CMD_PREFIX" -A FORWARD -p tcp --dport %d -j %s",
        dport, ipt_seracl_get_chain_name(service_type));
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    /* add inband iptables chain */
    sal_memset(sz_cmd, 0, sizeof(sz_cmd));
    len = 0;
    sal_snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, 
        IPTABLES_CMD_PREFIX" -N %s\n", ipt_seracl_get_chain_name(service_type));
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -A INPUT -p tcp --dport %d -j %s\n",
        dport, ipt_seracl_get_chain_name(service_type));
    /* add default ACCEPT for vemgmt ip */
    len = sal_strlen(sz_cmd);
    sal_snprintf(sz_cmd+len, GLB_IPTABLES_MAX_CMD_BUFSZ-len, 
        IPTABLES_CMD_PREFIX" -I %s -s %s -j ACCEPT\n",
        ipt_seracl_get_chain_name(service_type), GLB_NS_PORT_FORWARDING_VETH_MGMT_IP_ADDR);
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_FAIL;
    }

    return rc;
}


int32
ipt_del_service_acl(uint32 service_type)
{
    int32  rc = PM_E_NONE;
    tbl_acl_config_t    *p_acl_config = NULL;
    tbl_acl_config_key_t acl_config_key;

    /* delete iptables chain */
    PM_E_RETURN(ipt_del_service_acl_chain(service_type));

    /* unset attached in acl config */
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
    tbl_ssh_cfg_t    *p_ssh_cfg = tbl_ssh_cfg_get_ssh_cfg();
    if (NULL == p_sys_glb || NULL == p_ssh_cfg)
    {
        return PM_E_NONE;
    }

    sal_memset(&acl_config_key, 0, sizeof(tbl_acl_config_key_t));
    sal_strcpy(acl_config_key.name, GLB_SERVICE_TYPE_TELNET == service_type ? p_sys_glb->telnet_acl : p_sys_glb->ssh_acl);
    p_acl_config = tbl_acl_config_get_acl_config(&acl_config_key);
    if (NULL == p_acl_config)
    {
        return rc;
    }

    GLB_UNSET_FLAG(p_acl_config->service_ipt_attached, 
        GLB_SERVICE_TYPE_TELNET == service_type ? GLB_SERVICE_TYPE_TELNET : GLB_SERVICE_TYPE_SSH);
    PM_E_RETURN(tbl_acl_config_set_acl_config_field(p_acl_config, TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED));

    return rc;
}

int32
ipt_add_service_acl(tbl_acl_config_t *p_acl_config, uint32 service_type)
{
    int32  rc = PM_E_NONE;

    tbl_ace_config_master_t *p_master = tbl_ace_config_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_ace_config_t   *p_ace_config = NULL;

    /* added iptables chain when module start */
    //PM_E_RETURN(ipt_add_service_acl_chain(service_type));
    
    /* add iptables rules */
    CTCLIB_SLIST_LOOP_DEL(p_master->ace_config_list, p_ace_config, listnode, next)
    {
        if (NULL == p_ace_config)
        {
            continue;
        }
        if (0 != sal_strcmp(p_ace_config->key.acl_name, p_acl_config->key.name))
        {
            continue;
        }
        PM_E_RETURN(ipt_add_service_acl_rule(p_ace_config, service_type));
    }

    /* set attached in acl config */
    GLB_SET_FLAG(p_acl_config->service_ipt_attached, 
        GLB_SERVICE_TYPE_TELNET == service_type ? GLB_SERVICE_TYPE_TELNET : GLB_SERVICE_TYPE_SSH);
    PM_E_RETURN(tbl_acl_config_set_acl_config_field(p_acl_config, TBL_ACL_CONFIG_FLD_SERVICE_IPT_ATTACHED));

    return rc;
}


int32
ipt_service_acl_start(switch_master_t *master)
{
    ipt_telnet_rules_list = XCALLOC(MTYPE_IPT_LINK_LIST, sizeof(ctclib_list_t));
    ipt_ssh_rules_list = XCALLOC(MTYPE_IPT_LINK_LIST, sizeof(ctclib_list_t));
    ctclib_list_init(ipt_telnet_rules_list);
    ctclib_list_init(ipt_ssh_rules_list);

    ipt_add_service_acl_chain(GLB_SERVICE_TYPE_TELNET);
    ipt_add_service_acl_chain(GLB_SERVICE_TYPE_SSH);

    return PM_E_NONE;
}


#define _____IPTABLES_PREVENT_____

static int32
_ipt_prevent_get_smalpkt_static_base(uint32 *p_value)
{
    char sz_file[] = "/tmp/ipt_get_file";
    char sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    glb_iptables_chain_type_t chain_flag = GLB_IPTABLES_CHAIN_INPUT;

    snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, "%s %s >%s 2>&1",
            IPTABLES_CMD_PREFIX, "-L -x -v", sz_file);

    if (sal_system(sz_cmd) == -1 || p_value == NULL)
    {
        return PM_E_INVALID_PARAM;
    }
    
    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_INITED;
    }
    while (sal_fgets(sz_linebuf, GLB_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Chain INPUT", sal_strlen("Chain INPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_INPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain FORWARD", sal_strlen("Chain FORWARD")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_FOWRWARD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain OUTPUT", sal_strlen("Chain OUTPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_OUTPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain icmp_flood", sal_strlen("Chain icmp_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_ICMP_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain smurf", sal_strlen("Chain smurf")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SMURF;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain syn_flood", sal_strlen("Chain syn_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SYN_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain udp_chain", sal_strlen("Chain udp_chain")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_UDP_FRAGGLE;
            continue;
        }
        
        if (GLB_IPTABLES_CHAIN_INPUT == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "length"))
            {
                sal_sscanf(sz_linebuf, "%d", p_value);
                break;
            }
        }
    }

    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

static int32
_ipt_prevent_get_vrf_smalpkt_static_base(uint32 *p_value)
{
    char sz_file[] = "/tmp/ipt_get_vrf_file";
    char sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    glb_iptables_chain_type_t chain_flag = GLB_IPTABLES_CHAIN_INPUT;

    snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, "%s %s >%s 2>&1",
                 IPTABLES_NETNS_CMD_PREFIX, "-L -x -v", sz_file);

    if (sal_system(sz_cmd) == -1 || p_value == NULL)
    {
        return PM_E_INVALID_PARAM;
    }
    
    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_INITED;
    }
    while (sal_fgets(sz_linebuf, GLB_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Chain INPUT", sal_strlen("Chain INPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_INPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain FORWARD", sal_strlen("Chain FORWARD")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_FOWRWARD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain OUTPUT", sal_strlen("Chain OUTPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_OUTPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain icmp_flood", sal_strlen("Chain icmp_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_ICMP_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain smurf", sal_strlen("Chain smurf")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SMURF;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain syn_flood", sal_strlen("Chain syn_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SYN_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain udp_chain", sal_strlen("Chain udp_chain")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_UDP_FRAGGLE;
            continue;
        }
        
        if (GLB_IPTABLES_CHAIN_INPUT == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "length"))
            {
                sal_sscanf(sz_linebuf, "%d", p_value);
                break;
            }
        }
    }

    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

int32
_ipt_prevent_get_statistic(tbl_iptables_prevent_t* p_ipt_prevent)
{
    char sz_file[] = "/tmp/ipt_set_file";
    char sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    glb_iptables_chain_type_t chain_flag = GLB_IPTABLES_CHAIN_INPUT;
    int32 num1,num2,num3;
    num1 = num2 = num3 = 0;

    /* Modified by kcao for bug 51612 2019-04-15, for show ip-intercept statistics slow problem */
    snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, "%s %s >%s 2>&1",
        IPTABLES_CMD_PREFIX, "-L -x -v -n", sz_file);

    if (sal_system(sz_cmd) == -1 || NULL == p_ipt_prevent)
    {
        return PM_E_INVALID_PARAM;
    }
    
    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_INITED;
    }
    while (sal_fgets(sz_linebuf, GLB_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Chain INPUT", sal_strlen("Chain INPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_INPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain FORWARD", sal_strlen("Chain FORWARD")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_FOWRWARD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain OUTPUT", sal_strlen("Chain OUTPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_OUTPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain icmp_flood", sal_strlen("Chain icmp_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_ICMP_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain smurf", sal_strlen("Chain smurf")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SMURF;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain syn_flood", sal_strlen("Chain syn_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SYN_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain udp_chain", sal_strlen("Chain udp_chain")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_UDP_FRAGGLE;
            continue;
        }
        
        if (GLB_IPTABLES_CHAIN_INPUT == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "length"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->small_pkt_count = num1 + p_ipt_prevent->small_pkt_static_base;
                num1 = 0;
                continue;
            }
            else
            {
                continue;
            } 
        }

        if (GLB_IPTABLES_CHAIN_ICMP_FLOOD == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "DROP"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->icmp_count = num1;
                num1 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_SMURF == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match src-type BROADCAST"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match dst-type BROADCAST"))
            {
                sal_sscanf(sz_linebuf, "%d", &num2);
                p_ipt_prevent->smurf_count = num1 + num2;
                num1 = 0;
                num2 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_SYN_FLOOD == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "DROP"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->tcp_count = num1;
                num1 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_UDP_FRAGGLE == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "udp dpt:echo")
                || NULL != sal_strstr(sz_linebuf, "udp dpt:7"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "udp dpt:chargen")
                     || NULL != sal_strstr(sz_linebuf, "udp dpt:19"))
            {
                sal_sscanf(sz_linebuf, "%d", &num2);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match"))
            {
                sal_sscanf(sz_linebuf, "%d", &num3);
                p_ipt_prevent->fraggle_count = num1 + num2 + num3;
                num1 = 0;
                num2 = 0;
                num3 = 0;
                continue;
            }

            else if (NULL != sal_strstr(sz_linebuf, "DROP") 
                     && NULL == sal_strstr(sz_linebuf, "udp dpt")
                     && NULL == sal_strstr(sz_linebuf, "ADDRTYPE match"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->udp_count = num1;
                num1 = 0;
                continue;
            }
        }        
    }
    
    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

int32
_ipt_prevent_get_vrf_statistic(tbl_iptables_prevent_t* p_ipt_prevent)
{
    char sz_file[] = "/tmp/ipt_set_vrf_file";
    char sz_cmd[GLB_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    glb_iptables_chain_type_t chain_flag = GLB_IPTABLES_CHAIN_INPUT;
    int32 num1,num2,num3;
    num1 = num2 = num3 = 0;

    snprintf(sz_cmd, GLB_IPTABLES_MAX_CMD_BUFSZ, "%s %s >%s 2>&1",
        IPTABLES_NETNS_CMD_PREFIX, "-L -x -v", sz_file);

    if (sal_system(sz_cmd) == -1 || NULL == p_ipt_prevent)
    {
        return PM_E_INVALID_PARAM;
    }
    
    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_INITED;
    }
    while (sal_fgets(sz_linebuf, GLB_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Chain INPUT", sal_strlen("Chain INPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_INPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain FORWARD", sal_strlen("Chain FORWARD")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_FOWRWARD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain OUTPUT", sal_strlen("Chain OUTPUT")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_OUTPUT;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain icmp_flood", sal_strlen("Chain icmp_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_ICMP_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain smurf", sal_strlen("Chain smurf")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SMURF;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain syn_flood", sal_strlen("Chain syn_flood")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_SYN_FLOOD;
            continue;
        }
        
        if (!sal_strncmp(sz_linebuf, "Chain udp_chain", sal_strlen("Chain udp_chain")))
        {
            chain_flag = GLB_IPTABLES_CHAIN_UDP_FRAGGLE;
            continue;
        }
        
        if (GLB_IPTABLES_CHAIN_INPUT == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "length"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->small_pkt_mgmt_count = num1 + p_ipt_prevent->small_pkt_mgmt_static_base;
                num1 = 0;
                continue;
            }
            else
            {
                continue;
            } 
        }

        if (GLB_IPTABLES_CHAIN_ICMP_FLOOD == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "DROP"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->icmp_mgmt_count= num1;
                num1 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_SMURF == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match src-type BROADCAST"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match dst-type BROADCAST"))
            {
                sal_sscanf(sz_linebuf, "%d", &num2);
                p_ipt_prevent->smurf_mgmt_count = num1 + num2;
                num1 = 0;
                num2 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_SYN_FLOOD == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "DROP"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->tcp_mgmt_count = num1;
                num1 = 0;
                continue;
            }
        }

        if (GLB_IPTABLES_CHAIN_UDP_FRAGGLE == chain_flag)
        {
            if (NULL != sal_strstr(sz_linebuf, "udp dpt:echo")
                || NULL != sal_strstr(sz_linebuf, "udp dpt:7"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "udp dpt:chargen")
                     || NULL != sal_strstr(sz_linebuf, "udp dpt:19"))
            {
                sal_sscanf(sz_linebuf, "%d", &num2);
                continue;
            }
            else if (NULL != sal_strstr(sz_linebuf, "ADDRTYPE match"))
            {
                sal_sscanf(sz_linebuf, "%d", &num3);
                p_ipt_prevent->fraggle_mgmt_count = num1 + num2 + num3;
                num1 = 0;
                num2 = 0;
                num3 = 0;
                continue;
            }

            else if (NULL != sal_strstr(sz_linebuf, "DROP") 
                     && NULL == sal_strstr(sz_linebuf, "udp dpt")
                     && NULL == sal_strstr(sz_linebuf, "ADDRTYPE match"))
            {
                sal_sscanf(sz_linebuf, "%d", &num1);
                p_ipt_prevent->udp_mgmt_count = num1;
                num1 = 0;
                continue;
            }
        }        
    }
    
    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}


int32
ipt_prevent_set_small_pkt(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT));
    }
    else
    {
        snprintf(sz_cmd, 512,
        IPTABLES_CMD_PREFIX" -D INPUT -m length --length :%d -j DROP\n%s -D INPUT -m length --length :%d -j DROP\n%s%s%s%s",
        p_db_ipt->small_pkt_length-1 , IPTABLES_NETNS_CMD_PREFIX, p_db_ipt->small_pkt_length-1,
        IPTABLES_CMD_PREFIX, " -D INPUT -i lo -j ACCEPT\n", IPTABLES_NETNS_CMD_PREFIX, " -D INPUT -i lo -j ACCEPT");
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        p_db_ipt->small_pkt_static_base = 0;
        p_db_ipt->small_pkt_mgmt_static_base = 0;
        p_db_ipt->small_pkt_count = 0;
        p_db_ipt->small_pkt_mgmt_count = 0;
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE));
    }

    return rc;
}

int32
ipt_prevent_set_small_pkt_len(tbl_iptables_prevent_t* p_ipt_prevent, uint16 len_val)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    uint32 static_base = 0;
    int32 rc = PM_E_NONE;
    
    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT))
    {
        if (p_db_ipt->small_pkt_length == len_val)
        {
            return PM_E_NONE;
        }
        if (!_ipt_prevent_get_smalpkt_static_base(&static_base))
        {
            p_db_ipt->small_pkt_static_base +=  static_base;
            PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_db_ipt, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE));
        }
        if (!_ipt_prevent_get_vrf_smalpkt_static_base(&static_base))
        {
            p_db_ipt->small_pkt_mgmt_static_base +=  static_base;
            PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_db_ipt, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE));
        }
        
        snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D INPUT -m length --length :%d -j DROP\n%s -D INPUT -m length --length :%d -j DROP\n%s%s%d %s\n%s%s%d %s",
            p_db_ipt->small_pkt_length-1, IPTABLES_NETNS_CMD_PREFIX, p_db_ipt->small_pkt_length-1, 
            IPTABLES_CMD_PREFIX, " -I INPUT 2 -m length --length :", len_val-1, "-j DROP",
            IPTABLES_NETNS_CMD_PREFIX, " -I INPUT 2 -m length --length :", len_val-1, "-j DROP");

    }
    else
    {
        snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I INPUT 1 -m length --length :%d -j DROP\n%s -I INPUT 1 -m length --length :%d -j DROP\n%s %s\n%s %s",
            len_val-1, IPTABLES_NETNS_CMD_PREFIX, len_val-1, 
            IPTABLES_CMD_PREFIX, "-I INPUT 1 -i lo -j ACCEPT",
            IPTABLES_NETNS_CMD_PREFIX, "-I INPUT 1 -i lo -j ACCEPT");

    }
    
    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_HAL_FAIL;
    }
    
    p_ipt_prevent->small_pkt_length = len_val;
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH));
    GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMALL_PKT);
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT));

    return rc;
}

int32
ipt_prevent_set_pkt_statistic_update(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;
    
    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        /* modified by huwx for bug 45088, 2017-10-18 */
        rc = _ipt_prevent_get_statistic(p_db_ipt);
        if (rc)
        {
            return PM_E_FAIL;
        }
        rc = _ipt_prevent_get_vrf_statistic(p_db_ipt);
        if (rc)
        {
            return PM_E_FAIL;
        }

    }
    else
    {
        snprintf(sz_cmd, 512, IPTABLES_CMD_PREFIX" -Z");
        rc = _ipt_config_exec(sz_cmd);
        snprintf(sz_cmd, 512, IPTABLES_NETNS_CMD_PREFIX" -Z");
        rc = _ipt_config_exec(sz_cmd);
        p_ipt_prevent->small_pkt_static_base = 0;
        p_ipt_prevent->small_pkt_mgmt_static_base = 0;
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_STATIC_BASE));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_STATIC_BASE));
    }

    return rc;
}

int32
ipt_prevent_set_maceq(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        //rc = fei_ipt_set_mac_equal_intercept(TRUE);
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ));
     }
    else
    {
        //rc = fei_ipt_set_mac_equal_intercept(FALSE);
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_MACEQ);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ));
    }

    return rc;
}

int32
ipt_prevent_set_ipeq(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        //rc = fei_ipt_set_ip_equal_intercept(TRUE);
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ));
     }
    else
    {
        //rc = fei_ipt_set_ip_equal_intercept(FALSE);
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_IPEQ);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ));
    }

    return rc;
}



int32
ipt_prevent_set_smurf(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        snprintf(sz_cmd, 512,
                IPTABLES_CMD_PREFIX" -A smurf -m addrtype --src-type broadcast -j DROP\n%s%s%s%s%s%s", 
                IPTABLES_CMD_PREFIX, " -A smurf -m addrtype --dst-type broadcast -j DROP\n",
                IPTABLES_NETNS_CMD_PREFIX, " -A smurf -m addrtype --src-type broadcast -j DROP\n",
                IPTABLES_NETNS_CMD_PREFIX," -A smurf -m addrtype --dst-type broadcast -j DROP");
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }

        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMURF);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF));
    }
    else
    {
        snprintf(sz_cmd, 512,
                IPTABLES_CMD_PREFIX" -F smurf\n%s -F smurf", IPTABLES_NETNS_CMD_PREFIX);
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        p_db_ipt->smurf_count = 0;
        p_db_ipt->smurf_mgmt_count = 0;
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_SMURF);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT));
    }

    return rc;
}

int32
ipt_prevent_set_fraggle(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_UDP)
            && !GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I udp_chain 1 -m addrtype --dst-type broadcast -j DROP\n%s%s%s%s%s%s%s%s%s%s", 
            IPTABLES_CMD_PREFIX, " -I udp_chain 1 -p UDP --dport 7 -j DROP\n",
            IPTABLES_CMD_PREFIX, " -I udp_chain 1 -p UDP --dport 19 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -I udp_chain 1 -m addrtype --dst-type broadcast -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -I udp_chain 1 -p UDP --dport 7 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -I udp_chain 1 -p UDP --dport 19 -j DROP");

        }
        else if (!GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -A udp_chain -p UDP --dport 7 -j DROP\n%s%s%s%s%s%s%s%s%s%s", 
            IPTABLES_CMD_PREFIX, " -A udp_chain -p UDP --dport 19 -j DROP\n",
            IPTABLES_CMD_PREFIX, " -A udp_chain -m addrtype --dst-type broadcast -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A udp_chain -p UDP --dport 7 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A udp_chain -p UDP --dport 19 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A udp_chain -m addrtype --dst-type broadcast -j DROP");
        }

        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }

        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE));
    }
    else
    {
        if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_UDP))
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D udp_chain -p UDP --dport 7 -j DROP\n%s%s%s%s%s%s%s%s%s%s", 
            IPTABLES_CMD_PREFIX, " -D udp_chain -p UDP --dport 19 -j DROP\n",
            IPTABLES_CMD_PREFIX, " -D udp_chain -m addrtype --dst-type broadcast -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -D udp_chain -p UDP --dport 7 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -D udp_chain -p UDP --dport 19 -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -D udp_chain -m addrtype --dst-type broadcast -j DROP");            
        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -F udp_chain\n%s -F udp_chain", IPTABLES_NETNS_CMD_PREFIX);
        }
        
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        p_db_ipt->fraggle_count = 0;
        p_db_ipt->fraggle_mgmt_count = 0;
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT));
    }

    return rc;
}

int32
ipt_prevent_set_icmp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP));
    }
    else
    {
        snprintf(sz_cmd, 512,
        IPTABLES_CMD_PREFIX" -F icmp_flood\n%s -F icmp_flood", IPTABLES_NETNS_CMD_PREFIX);
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        p_db_ipt->icmp_count = 0;
        p_db_ipt->icmp_mgmt_count = 0;
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT));
    }

    return rc;
}

int32
ipt_prevent_set_icmp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;
    
    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_ICMP))
    {
        if (p_db_ipt->icmp_rate_limit == rate_val)
        {
            return PM_E_NONE;
        }
        if (p_db_ipt->icmp_rate_limit && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -R icmp_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -R icmp_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX,rate_val, rate_val);
        }
        else if ((p_db_ipt->icmp_rate_limit == 0) && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I icmp_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -I icmp_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX,rate_val, rate_val);
        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D icmp_flood 1\n%s -D icmp_flood 1", IPTABLES_NETNS_CMD_PREFIX);
        } 
    }
    else
    {
        if (rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -A icmp_flood -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -A icmp_flood -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s%s%s%s",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val, IPTABLES_CMD_PREFIX, " -A icmp_flood -p icmp -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A icmp_flood -p icmp -j DROP");
        }
        else
        {
            snprintf(sz_cmd, 512, 
            IPTABLES_CMD_PREFIX" -A icmp_flood -p icmp -j DROP\n%s  -A icmp_flood -p icmp -j DROP",
            IPTABLES_NETNS_CMD_PREFIX);
        }
    }

    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_HAL_FAIL;
    }
    
    p_ipt_prevent->icmp_rate_limit = rate_val;
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT));
    GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_ICMP);
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP));

    return rc;
}

int32
ipt_prevent_set_tcp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP));
    }
    else
    {
        snprintf(sz_cmd, 512,
        IPTABLES_CMD_PREFIX" -F syn_flood\n%s -F syn_flood", IPTABLES_NETNS_CMD_PREFIX);
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP));
    }

    return rc;
}

int32
ipt_prevent_set_tcp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;
    
    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_TCP))
    {
        if (p_db_ipt->tcp_rate_limit == rate_val)
        {
            return PM_E_NONE;
        }
        if (p_db_ipt->tcp_rate_limit && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -R syn_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -R syn_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX,rate_val, rate_val);
        }
        else if ((p_db_ipt->tcp_rate_limit == 0) && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I syn_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -I syn_flood 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX,rate_val, rate_val);
        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D syn_flood 1\n%s -D syn_flood 1", IPTABLES_NETNS_CMD_PREFIX);
        } 
    }
    else
    {
        if (rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -A syn_flood -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -A syn_flood -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s%s%s%s",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val, IPTABLES_CMD_PREFIX, " -A syn_flood -p tcp -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A syn_flood -p tcp -j DROP");
        }
        else
        {
            snprintf(sz_cmd, 512, 
            IPTABLES_CMD_PREFIX" -A syn_flood -p tcp -j DROP\n%s  -A syn_flood -p tcp -j DROP",
            IPTABLES_NETNS_CMD_PREFIX);
        }
    }

    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_HAL_FAIL;
    }
    
    p_ipt_prevent->tcp_rate_limit = rate_val;
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT));
    GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_TCP);
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP));

    return rc;
}

int32
ipt_prevent_set_udp(tbl_iptables_prevent_t* p_ipt_prevent, bool enable)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;

    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (enable)
    {
        GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP));
    }
    else
    {
        if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
        {
            if (p_db_ipt->udp_rate_limit)
            {
                snprintf(sz_cmd, 512,
                IPTABLES_CMD_PREFIX" -D udp_chain -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -D udp_chain -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s%s%s%s", 
                p_db_ipt->udp_rate_limit, p_db_ipt->udp_rate_limit, IPTABLES_NETNS_CMD_PREFIX, 
                p_db_ipt->udp_rate_limit, p_db_ipt->udp_rate_limit, 
                IPTABLES_CMD_PREFIX, " -D udp_chain -p UDP -j DROP\n",
                IPTABLES_NETNS_CMD_PREFIX, " -D udp_chain -p UDP -j DROP");
            }
            else
            {
                snprintf(sz_cmd, 512,
                IPTABLES_CMD_PREFIX" -D udp_chain -p UDP -j DROP\n%s -D udp_chain -p UDP -j DROP", IPTABLES_NETNS_CMD_PREFIX);
            }

        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -F udp_chain\n%s -F udp_chain", IPTABLES_NETNS_CMD_PREFIX);

        }
        
        rc = _ipt_config_exec(sz_cmd);
        if (rc)
        {
            return PM_E_HAL_FAIL;
        }
        p_db_ipt->udp_count = 0;
        p_db_ipt->udp_mgmt_count = 0;
        GLB_UNSET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP);
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_UDP_COUNT));
        PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT));
        
    }

    return rc;
}

int32
ipt_prevent_set_udp_rate_limit(tbl_iptables_prevent_t* p_ipt_prevent, uint16 rate_val)
{
    tbl_iptables_prevent_t *p_db_ipt = NULL;
    char sz_cmd[512];
    int32 rc = PM_E_NONE;
    
    p_db_ipt = tbl_iptables_prevent_get_iptables_prevent();
    if (NULL == p_db_ipt)
    {
        return PM_E_NOT_EXIST;
    }

    if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_UDP)
        && GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
    {
        if (p_db_ipt->udp_rate_limit == rate_val)
        {
            return PM_E_NONE;
        }
        if (p_db_ipt->udp_rate_limit && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -R udp_chain 4 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -R udp_chain 4 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val);
        }
        else if ((p_db_ipt->udp_rate_limit == 0) && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I udp_chain 4 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -I udp_chain 4 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val);
        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D udp_chain 4\n%s -D udp_chain 4", IPTABLES_NETNS_CMD_PREFIX);
        } 
    }
    else if (GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_UDP)
            && !GLB_FLAG_ISSET(p_db_ipt->flags, GLB_IPTABLES_CFG_FLAG_FRAGGLE))
    {
        if (p_db_ipt->udp_rate_limit == rate_val)
        {
            return PM_E_NONE;
        }
        if (p_db_ipt->udp_rate_limit && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -R udp_chain 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -R udp_chain 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val);
        }
        else if ((p_db_ipt->udp_rate_limit == 0) && rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -I udp_chain 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -I udp_chain 1 -m limit --limit %d/s --limit-burst %d -j ACCEPT",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val);
        }
        else
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -D udp_chain 1\n%s -D udp_chain 1", IPTABLES_NETNS_CMD_PREFIX);
        } 
    }
    else
    {
        if (rate_val)
        {
            snprintf(sz_cmd, 512,
            IPTABLES_CMD_PREFIX" -A udp_chain -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s -A udp_chain -m limit --limit %d/s --limit-burst %d -j ACCEPT\n%s%s%s%s",
            rate_val, rate_val, IPTABLES_NETNS_CMD_PREFIX, rate_val, rate_val, 
            IPTABLES_CMD_PREFIX, " -A udp_chain -p UDP -j DROP\n",
            IPTABLES_NETNS_CMD_PREFIX, " -A udp_chain -p UDP -j DROP");
        }
        else
        {
            snprintf(sz_cmd, 512, 
            IPTABLES_CMD_PREFIX" -A udp_chain -p UDP -j DROP\n%s  -A udp_chain -p UDP -j DROP",
            IPTABLES_NETNS_CMD_PREFIX);
        }
    }

    rc = _ipt_config_exec(sz_cmd);
    if (rc)
    {
        return PM_E_HAL_FAIL;
    }
    
    p_ipt_prevent->udp_rate_limit = rate_val;
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT));
    GLB_SET_FLAG(p_ipt_prevent->flags, GLB_IPTABLES_CFG_FLAG_UDP);
    PM_E_RETURN(tbl_iptables_prevent_set_iptables_prevent_field(p_ipt_prevent, TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP));

    return rc;
}


int32
ipt_prevent_start(switch_master_t *master)
{
    tbl_iptables_prevent_t ipt_prevent_glb;
    char szCmd[512];

    cdb_register_cfg_tbl_cb(TBL_IPTABLES_PREVENT, ipt_prevent_cmd_process);

    sal_memset(&ipt_prevent_glb, 0, sizeof(tbl_iptables_prevent_t));

    snprintf(szCmd, 512, IPTABLES_CMD_PREFIX" -N syn_flood\n%s -A INPUT -p tcp --syn -j syn_flood", IPTABLES_CMD_PREFIX);
    _ipt_config_exec(szCmd);
    
    snprintf(szCmd, 512, IPTABLES_CMD_PREFIX" -N smurf\n%s -A INPUT -p icmp --icmp-type echo-request -j smurf", IPTABLES_CMD_PREFIX);
    _ipt_config_exec(szCmd);
        
    snprintf(szCmd, 512, IPTABLES_CMD_PREFIX" -N icmp_flood\n%s -A INPUT -p icmp -j icmp_flood", IPTABLES_CMD_PREFIX);
    _ipt_config_exec(szCmd);

    snprintf(szCmd, 512, IPTABLES_CMD_PREFIX" -N udp_chain\n%s -A INPUT -p UDP -j udp_chain", IPTABLES_CMD_PREFIX);
    _ipt_config_exec(szCmd);

    snprintf(szCmd, 512, IPTABLES_NETNS_CMD_PREFIX" -N syn_flood\n%s -A INPUT -p tcp --syn -j syn_flood", IPTABLES_NETNS_CMD_PREFIX);
    _ipt_config_exec(szCmd);
    
    snprintf(szCmd, 512, IPTABLES_NETNS_CMD_PREFIX" -N smurf\n%s -A INPUT -p icmp --icmp-type echo-request -j smurf", IPTABLES_NETNS_CMD_PREFIX);
    _ipt_config_exec(szCmd);
        
    snprintf(szCmd, 512, IPTABLES_NETNS_CMD_PREFIX" -N icmp_flood\n%s -A INPUT -p icmp -j icmp_flood", IPTABLES_NETNS_CMD_PREFIX);
    _ipt_config_exec(szCmd);

    snprintf(szCmd, 512, IPTABLES_NETNS_CMD_PREFIX" -N udp_chain\n%s -A INPUT -p UDP -j udp_chain", IPTABLES_NETNS_CMD_PREFIX);
    _ipt_config_exec(szCmd);

    return PM_E_NONE;
}

int32
ipt_prevent_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_IPTABLES_PREVENT, NULL);
    return PM_E_NONE;
}



