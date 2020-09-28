
#include "snmpagt_define.h"
#include "snmpagt.h"


static oid PRIV_SYSMEM_OID[]  = { CTCENTERPRISE_OID, 1, 1 };
static oid PRIV_SYSCPU_OID[]  = { CTCENTERPRISE_OID, 1, 2 };

#define  SYSMEMORYINDEX_INDEX       1
#define  SYSMEMORYTOTAL_INDEX       5
#define  SYSMEMORYFREE_INDEX        11
#define  SYSMEMORYUSED_INDEX        12
#define  SYSMEMORYUSAGE_INDEX       13

#define  CPUUSGINDEX_INDEX          1
#define  CPUUSGTABLE_INDEX          12

typedef struct
{
    snmpagt_nodes_t mem_node;
    long            mem_index;
    long            mem_total;
    long            mem_free;
    long            mem_used;
    long            mem_usage;
    
    snmpagt_nodes_t cpu_node;
    long            cpu_index;
    long            cpu_usage;
} priv_system_t;

priv_system_t g_priv_sys;


int32
priv_system_get_cpu_usage(long *usage)
{
    double cpu_usage;
    FILE *pFile = NULL;
    char strLine[CMD_BUF_256];
    uint32 cpu_total_line = FALSE;
    uint32 user_time = 0;
    uint32 nice_time = 0;
    uint32 sys_time = 0;
    uint32 idle_time = 0;
    uint32 iow_time = 0;
    uint32 irq_time = 0;
    uint32 sirq_time = 0;
    uint32 total_time = 0;

    *usage = 0;
    pFile = sal_fopen(CPU_INFO_PATH, "r");
    if((NULL == pFile) || (sal_feof(pFile)))
    {
        return -1;
    }

    sal_memset(strLine, 0, sizeof(strLine));
    while(sal_fgets(strLine, CMD_BUF_256, pFile) != NULL)
    {
        if (0 == sal_strncmp(strLine, "User  Nice", 10))
        {
            cpu_total_line = TRUE;
            continue;
        }
        if (cpu_total_line)
        {
            sal_sscanf(strLine, "%d%d%d%d%d%d%d%d", &user_time, &nice_time, &sys_time, &idle_time, &iow_time, &irq_time, &sirq_time, &total_time);
            cpu_total_line = FALSE;
            if (total_time)
            {
                cpu_usage = (100.0 * (total_time - idle_time)) / total_time;
                *usage = (long)cpu_usage;
                break;
            }
        }
    }
    sal_fclose(pFile);
 
    return 0;
}

u_char*
priv_syscpu_handle(struct variable *vp,
                           oid * name,
                           size_t * length,
                           int exact,
                           size_t * var_len, WriteMethod ** write_method)
{
    long usage = 0;
    int32 rc = 0;
    
    if (MATCH_FAILED == header_generic(vp, name, length, exact, var_len, write_method))
    {
        return NULL;
    }
    
    switch (vp->magic)
    {
    case CPUUSGINDEX_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.cpu_index);
        break;
    case CPUUSGTABLE_INDEX:
        rc = priv_system_get_cpu_usage(&usage);
        if (0 == rc)
        {
            g_priv_sys.cpu_usage = usage;
        }
        SNMPAGT_RETURN_INTEGER(g_priv_sys.cpu_usage);
        break;
    default:
        break;
    }

    return NULL;
}

u_char*
priv_sysmem_handle(struct variable *vp,
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
    case SYSMEMORYINDEX_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.mem_index);
        break;
    case SYSMEMORYTOTAL_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.mem_total);
        break;
    case SYSMEMORYFREE_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.mem_free);
        break;
    case SYSMEMORYUSED_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.mem_used);
        break;
    case SYSMEMORYUSAGE_INDEX:
        SNMPAGT_RETURN_INTEGER(g_priv_sys.mem_usage);
        break;
    default:
        break;
    }

    return NULL;
}

struct variable PRIV_SYSMEM_VAR[] = {
    {1,  ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_sysmem_handle, 1, {1}},
    {5,  ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_sysmem_handle, 1, {5}},
    {11, ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_sysmem_handle, 1, {11}},
    {12, ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_sysmem_handle, 1, {12}},
    {13, ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_sysmem_handle, 1, {13}},
};

struct variable PRIV_SYSCPU_VAR[] = {
    {1,  ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_syscpu_handle, 1, {1}},
    {12, ASN_INTEGER, NETSNMP_OLDAPI_RONLY, priv_syscpu_handle, 1, {12}},
};

int32
priv_system_init()
{
    snmpagt_nodes_t *p_node = &g_priv_sys.mem_node;
    int32 rc = 0;
    
    sal_memset(&g_priv_sys, 0, sizeof(g_priv_sys));

    snmpagt_oid2str(PRIV_SYSMEM_OID, sizeof(PRIV_SYSMEM_OID)/sizeof(oid), p_node->oid_str, CMD_BUF_256);
    p_node->vars = PRIV_SYSMEM_VAR;
    p_node->var_num = sizeof(PRIV_SYSMEM_VAR)/sizeof(PRIV_SYSMEM_VAR[0]);
    rc = snmpagt_nodes_create(p_node);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "snmpagt_nodes_create %s failed", p_node->oid_str);
    }

    g_priv_sys.cpu_index = 1;
    p_node = &g_priv_sys.cpu_node;
    snmpagt_oid2str(PRIV_SYSCPU_OID, sizeof(PRIV_SYSCPU_OID)/sizeof(oid), p_node->oid_str, CMD_BUF_256);
    p_node->vars = PRIV_SYSCPU_VAR;
    p_node->var_num = sizeof(PRIV_SYSCPU_VAR)/sizeof(PRIV_SYSCPU_VAR[0]);
    rc = snmpagt_nodes_create(p_node);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "snmpagt_nodes_create %s failed", p_node->oid_str);
    }
    
    return rc;
}

int32
priv_system_update_mem(uint32 total, uint32 used, uint32 free)
{
    double usage = 0;

    if (total)
    {
        usage = ((double)used/(double)total) * 100;
    }
    g_priv_sys.mem_total = (long)total;
    g_priv_sys.mem_used = (long)used;
    g_priv_sys.mem_free = (long)free;
    g_priv_sys.mem_usage = (long)usage;
    
    return 0;
}

