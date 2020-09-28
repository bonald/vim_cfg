#include "proto.h"
#include "glb_ns_port_forwarding.h"
#include "gen/tbl_ns_port_forwarding_define.h"
#include "gen/tbl_ns_port_forwarding.h"
#include "switch.h"
#include "ns_port_forwarding.h"

#define NS_PORT_FORWARDING_PROTO_MAP(proto) (((proto) == NS_PORT_FORWARDING_TCP) ? "tcp" : "udp")

#define NS_PORT_FORWARDING_OP_PREROUTING_RULE(op, in_ifname, proto, port, out_ip) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
         GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX \
         " -t nat %s PREROUTING  -i %s -p %s --dport %d -j DNAT --to-destination %s", \
         (op), \
         (in_ifname), \
         (proto), \
         (port), \
         (out_ip)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_HAL_FAIL; \
    } \
} \
while (0)   

#define NS_PORT_FORWARDING_OP_POSTROUTING_RULE(op, out_ifname, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX \
             " -t nat %s POSTROUTING  -o %s -p %s --dport %d -j MASQUERADE", \
             (op), \
             (out_ifname), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_IPTABLES_EXEC_FAIL; \
    } \
} \
while (0)

#define NS_PORT_FORWARDING_OP_POSTROUTING_RULE_WITH_SOURCE(op, source_ip, out_ifname, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX \
             " -t nat %s POSTROUTING  -o %s --source %s -p %s --dport %d -j MASQUERADE", \
             (op), \
             (source_ip), \
             (out_ifname), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_IPTABLES_EXEC_FAIL; \
    } \
} \
while (0)

/* Use src ip to match local connection. */
#define NS_PORT_FORWARDING_CONNTRACK_CONN_DEL_REMOTE(source_ip, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_CONNTRACK_MGMT_PREFIX \
             " -D -s %s -p %s --dport %d", \
             (source_ip), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_conntrack_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_CONNTRACK_EXEC_FAIL; \
    } \
} \
while (0)

/* Use reply src ip to match local connection.
   currently the reply src ip address should always be 169.254.1.1. */
#define NS_PORT_FORWARDING_CONNTRACK_CONN_DEL_LOCAL(source_ip, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_CONNTRACK_MGMT_PREFIX \
             " -D -r %s -p %s --dport %d", \
             (source_ip), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_conntrack_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_CONNTRACK_EXEC_FAIL; \
    } \
} \
while (0)

#define NS_PORT_FORWARDING_OP_PREROUTING_RULE_V6(op, in_ifname, proto, port, out_ip) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
         GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6 \
         " -t nat %s PREROUTING  -i %s -p %s --dport %d -j DNAT --to-destination %s", \
         (op), \
         (in_ifname), \
         (proto), \
         (port), \
         (out_ip)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_HAL_FAIL; \
    } \
} \
while (0)   

#define NS_PORT_FORWARDING_OP_POSTROUTING_RULE_V6(op, out_ifname, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6 \
             " -t nat %s POSTROUTING  -o %s -p %s --dport %d -j MASQUERADE", \
             (op), \
             (out_ifname), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_IPTABLES_EXEC_FAIL; \
    } \
} \
while (0)

#define NS_PORT_FORWARDING_OP_POSTROUTING_RULE_WITH_SOURCE_V6(op, source_ip, out_ifname, proto, port) \
do \
{ \
    snprintf(cmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, \
             GLB_NS_PORT_FORWARDING_IPTABLES_MGMT_PREFIX_V6 \
             " -t nat %s POSTROUTING  -o %s --source %s -p %s --dport %d -j MASQUERADE", \
             (op), \
             (source_ip), \
             (out_ifname), \
             (proto), \
             (port)); \
    rc = _ns_port_forwarding_iptables_config_exec(cmd); \
    if (rc) \
    { \
        return PM_E_IPTABLES_EXEC_FAIL; \
    } \
} \
while (0)

/* Borrowed from iptables... */
static int32
_ns_port_forwarding_iptables_config_exec(char *pszcmd)
{
    char sz_file[] = "/tmp/ns_port_iptables_set_file";
    char sz_fullcmd[GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_NS_PORT_FORWARDING_IPTABLES_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == pszcmd)
    {
        return PM_E_INVALID_PARAM;
    }
    
    snprintf(sz_fullcmd, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ, "%s >%s 2>&1",
            pszcmd, sz_file);
    if (sal_system(sz_fullcmd) == -1)
    {
        return PM_E_IPTABLES_EXEC_FAIL;
    }

    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_EXIST;
    }
    while (sal_fgets(sz_linebuf, GLB_NS_PORT_FORWARDING_IPTABLES_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Try", 3))
        {
            /* Try...set rule error */
            rc = PM_E_IPTABLES_EXEC_FAIL;
            break;
        }
    }
    
    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

static int32
_ns_port_forwarding_conntrack_config_exec(char *pszcmd)
{
    char sz_file[] = "/tmp/ns_port_conntrack_set_file";
    char sz_fullcmd[GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_CMD_BUFSZ];
    char sz_linebuf[GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_LINE_BUFSZ];
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    
    if (NULL == pszcmd)
    {
        return PM_E_INVALID_PARAM;
    }
    
    snprintf(sz_fullcmd, GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_CMD_BUFSZ, "%s >%s 2>&1",
            pszcmd, sz_file);
    if (sal_system(sz_fullcmd) == -1)
    {
        return PM_E_IPTABLES_EXEC_FAIL;
    }

    fp = sal_fopen(sz_file, "r");
    if (fp == NULL)
    {
        return PM_E_NOT_EXIST;
    }
    while (sal_fgets(sz_linebuf, GLB_NS_PORT_FORWARDING_CONNTRACK_MAX_LINE_BUFSZ, fp) != NULL)
    {
        if (!sal_strncmp(sz_linebuf, "Try", 3))
        {
            /* Try...conntrack exec error */
            rc = PM_E_CONNTRACK_EXEC_FAIL;
            break;
        }
    }
    
    sal_fclose(fp);
    sal_unlink(sz_file);
    return rc;
}

static int32
_ns_port_forwarding_op_remote(cdb_oper_t oper, tbl_ns_port_forwarding_t *entry)
{
    int32 rc = PM_E_NONE;
    char cmd[GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ];
    char ip[16]; /* xxx.xxx.xxx.xxx */
    tbl_ns_port_forwarding_t *entry_in_db;
    tbl_ns_port_forwarding_key_t *key = &entry->key;
    char mapped_addr[16];
    char source_addr[16];
    char mgmt_if[16]; /* vemgmt/vemgmt2/vemgmt3. */
    
    sal_inet_ntop(AF_INET, (void *)&key->mapped_addr, mapped_addr, sizeof(mapped_addr));

    if (!strcmp(mapped_addr, GLB_NS_PORT_FORWARDING_VETH_MGMT2_IP_ADDR))
    {
        strcpy(mgmt_if, GLB_NS_PORT_FORWARDING_VETH_MGMT2_IFNAME);
        strcpy(source_addr, GLB_NS_PORT_FORWARDING_VETH_DEFAULT2_IP_ADDR);
    }
    else if (!strcmp(mapped_addr, GLB_NS_PORT_FORWARDING_VETH_MGMT3_IP_ADDR))
    {
        strcpy(mgmt_if, GLB_NS_PORT_FORWARDING_VETH_MGMT3_IFNAME);
        strcpy(source_addr, GLB_NS_PORT_FORWARDING_VETH_DEFAULT3_IP_ADDR);
    }
    else
    {
        strcpy(mgmt_if, GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME);
        strcpy(source_addr, GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IP_ADDR);
    }

    if (CDB_OPER_ADD == oper)
    {
        sal_inet_ntop(AF_INET,(void*)&(entry->ip), ip, 16);

        /* Check to prevent add duplicant iptables rule. */
        entry_in_db = tbl_ns_port_forwarding_get_ns_port_forwarding(key);
        if (entry_in_db)
        {
            return PM_E_EXIST;
        }

        NS_PORT_FORWARDING_OP_PREROUTING_RULE("-A", mgmt_if,
                                  NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port, ip);

        NS_PORT_FORWARDING_OP_POSTROUTING_RULE_WITH_SOURCE("-A", GLB_NS_PORT_FORWARDING_MGMT_IFNAME,
                               source_addr, NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        PM_E_RETURN(tbl_ns_port_forwarding_add_ns_port_forwarding(entry));
    }
    else
    {
        entry_in_db = tbl_ns_port_forwarding_get_ns_port_forwarding(key);
        if (!entry_in_db)
        {
            return PM_E_NOT_EXIST;
        }

        /* When delete ,use the ip saved in entry. */
        sal_inet_ntop(AF_INET,(void*)&(entry_in_db->ip), ip, 16);

        NS_PORT_FORWARDING_OP_PREROUTING_RULE("-D", mgmt_if,
                              NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port, ip);

        NS_PORT_FORWARDING_OP_POSTROUTING_RULE_WITH_SOURCE("-D", GLB_NS_PORT_FORWARDING_MGMT_IFNAME,
                               source_addr, NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        /* Delete the conntrack entry to make connection down. */
        NS_PORT_FORWARDING_CONNTRACK_CONN_DEL_REMOTE(source_addr, 
                                NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);
            
        PM_E_RETURN(tbl_ns_port_forwarding_del_ns_port_forwarding(key));    
    }
    
    return PM_E_NONE;
}

static int32
_ns_port_forwarding_op_local(cdb_oper_t oper, tbl_ns_port_forwarding_t *entry)
{
    int32 rc = PM_E_NONE;
    char cmd[GLB_NS_PORT_FORWARDING_IPTABLES_MAX_CMD_BUFSZ];
    tbl_ns_port_forwarding_t *entry_in_db;
    tbl_ns_port_forwarding_key_t *key = &entry->key;
    
    /* Not used now. sal_inet_ntop(AF_INET,(void*)&(entry->ip), ip, 16); */
    
    if (CDB_OPER_ADD == oper)
    {
        /* Check to prevent add duplicant iptables rule. */
        entry_in_db = tbl_ns_port_forwarding_get_ns_port_forwarding(key);
        if (entry_in_db)
        {
            return PM_E_EXIST;
        }

        NS_PORT_FORWARDING_OP_PREROUTING_RULE("-A", GLB_NS_PORT_FORWARDING_MGMT_IFNAME, 
                                              NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port,
                                              GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IP_ADDR);
        NS_PORT_FORWARDING_OP_POSTROUTING_RULE("-A", GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME, 
                                               NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        /* IPv6 */
        NS_PORT_FORWARDING_OP_PREROUTING_RULE_V6("-A", GLB_NS_PORT_FORWARDING_MGMT_IFNAME, 
                                              NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port,
                                              GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IPV6_ADDR);
        NS_PORT_FORWARDING_OP_POSTROUTING_RULE_V6("-A", GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME, 
                                               NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        PM_E_RETURN(tbl_ns_port_forwarding_add_ns_port_forwarding(entry));
    }
    else
    {
        entry_in_db = tbl_ns_port_forwarding_get_ns_port_forwarding(key);
        if (!entry_in_db)
        {
            return PM_E_NOT_EXIST;
        }

        NS_PORT_FORWARDING_OP_PREROUTING_RULE("-D", GLB_NS_PORT_FORWARDING_MGMT_IFNAME, 
                                              NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port,
                                              GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IP_ADDR);
        NS_PORT_FORWARDING_OP_POSTROUTING_RULE("-D", GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME, 
                                               NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        /* Delete the conntrack entry to make connection down. */
        NS_PORT_FORWARDING_CONNTRACK_CONN_DEL_LOCAL(GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IP_ADDR,
                        NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        /* IPv6 */
        NS_PORT_FORWARDING_OP_PREROUTING_RULE_V6("-D", GLB_NS_PORT_FORWARDING_MGMT_IFNAME, 
                                              NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port,
                                              GLB_NS_PORT_FORWARDING_VETH_DEFAULT_IPV6_ADDR);
        NS_PORT_FORWARDING_OP_POSTROUTING_RULE_V6("-D", GLB_NS_PORT_FORWARDING_VETH_MGMT_IFNAME, 
                                               NS_PORT_FORWARDING_PROTO_MAP(key->proto), key->port);

        /* Delete conntrack 
         ip netns exec mgmt conntrack -D -f ipv6 -r fec0:fffe:fffe::1:1 -p tcp --dport 23 */
        
        PM_E_RETURN(tbl_ns_port_forwarding_del_ns_port_forwarding(key));    
    }
    
    return PM_E_NONE;
}

int32
ns_port_forwarding_op_entry(cdb_oper_t oper, tbl_ns_port_forwarding_t *entry)
{
    int32 rc = PM_E_NONE;

    if (NS_PORT_FORWARDING_REMOTE == entry->key.type)
    {
        PM_E_RETURN(_ns_port_forwarding_op_remote(oper, entry));
    }
    else
    {
        PM_E_RETURN(_ns_port_forwarding_op_local(oper, entry));
    }

    return PM_E_NONE;
}

int32
ns_port_forwarding_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_NS_PORT_FORWARDING, ns_port_forwarding_cmd_process);

    return PM_E_NONE;
}

int32
ns_port_forwarding_stop(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_NS_PORT_FORWARDING, NULL);
    return PM_E_NONE;
}
