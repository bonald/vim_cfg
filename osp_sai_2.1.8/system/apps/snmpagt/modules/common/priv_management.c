
#include "snmpagt_define.h"
#include "snmpagt.h"

static oid PRIV_MGMT_OID[]  = { CTCENTERPRISE_OID, 43 };

#define  MGMTIPADDR_INDEX       100
#define  MGMTIPGATEWAY_INDEX    101

typedef struct
{
    snmpagt_nodes_t node;
    char            mgmt_addr[CMD_BUF_32];
    addr_ipv4_t     mgmt_gw;
} priv_mgmt_t;

priv_mgmt_t g_priv_mgmt;

u_char*
priv_mgmt_handle(struct variable *vp,
                           oid * name,
                           size_t * length,
                           int exact,
                           size_t * var_len, WriteMethod ** write_method)
{
    if (MATCH_FAILED == header_generic(vp, name, length, exact, var_len, write_method))
    {
        return NULL;
    }
    
    switch (vp->magic)
    {
    case MGMTIPADDR_INDEX:
        SNMPAGT_RETURN_STRING(g_priv_mgmt.mgmt_addr, sal_strlen(g_priv_mgmt.mgmt_addr));
        break;
    case MGMTIPGATEWAY_INDEX:
        SNMPAGT_RETURN_IPADDRESS(g_priv_mgmt.mgmt_gw);
        break;
    default:
        break;
    }

    return NULL;
}

struct variable PRIV_MGMT_VAR[] = {
    {100, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY, priv_mgmt_handle, 1, {100}},
    {101, ASN_IPADDRESS, NETSNMP_OLDAPI_RONLY, priv_mgmt_handle, 1, {101}},
};

int32
priv_mgmt_init()
{
    snmpagt_nodes_t *p_node = &g_priv_mgmt.node;
    int32 rc = 0;
    
    sal_memset(&g_priv_mgmt, 0, sizeof(g_priv_mgmt));

    snmpagt_oid2str(PRIV_MGMT_OID, sizeof(PRIV_MGMT_OID)/sizeof(oid), p_node->oid_str, CMD_BUF_256);
    p_node->vars = PRIV_MGMT_VAR;
    p_node->var_num = sizeof(PRIV_MGMT_VAR)/sizeof(PRIV_MGMT_VAR[0]);
    rc = snmpagt_nodes_create(p_node);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "snmpagt_nodes_create %s failed", p_node->oid_str);
    }

    return rc;
}

int32
priv_mgmt_update_mgmtip(char *str)
{
    char *p = NULL;
    sal_strncpy(g_priv_mgmt.mgmt_addr, str, CMD_BUF_32);
    p = sal_strchr(g_priv_mgmt.mgmt_addr, CMD_KEY_DIV_CHAR);
    if (p)
    {
        p[0] = CMD_DIVISION_CHAR;
    }
    return 0;
}

int32
priv_mgmt_update_mgmtgateway(addr_ipv4_t gw)
{
    g_priv_mgmt.mgmt_gw = gw;
    return 0;
}

