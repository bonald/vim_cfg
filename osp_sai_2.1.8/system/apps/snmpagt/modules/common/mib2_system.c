
#include "snmpagt_define.h"
#include "snmpagt.h"

static oid SYS_OID[]        = { PUBLICMIB_OID, 1 };
static oid BASE_SYSOBJID[]  = { CTCENTERPRISE_OID, 99 };

#define  SYSDESCR_INDEX     1
#define  SYSOID_INDEX       2
#define  SYSUPTIME_INDEX    3
#define  SYSCONTACT_INDEX   4
#define  SYSNAME_INDEX      5
#define  SYSLOCATION_INDEX  6

typedef struct
{
    snmpagt_nodes_t node;
    char sysdescr[CMD_BUF_256];
    char sysname[CMD_BUF_256];
    char syscontact[SNMP_INFO_SIZE];
    char syslocation[SNMP_INFO_SIZE];
    oid  sysoid[MAX_OID_LEN];
    long sysoid_len;
    long uptime;
} mib2_sys_t;

mib2_sys_t g_mib2_sys;

oid
mib2_get_product_oid()
{
    oid product_oid = 0;
    char buf[CMD_BUF_256];
    char tmp[CMD_BUF_256];
    FILE *fp = NULL;
    uint32 board_type = 0;
    uint32 product_series = 0;
    
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    while(sal_fgets(buf, CMD_BUF_256, fp) != NULL)
    {
        if (!sal_strncmp(buf, GLB_BOARD_TYPE_STRING, sal_strlen(GLB_BOARD_TYPE_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &board_type);
        }
        if (!sal_strncmp(buf, GLB_PRODUCT_SERIES_STRING, sal_strlen(GLB_PRODUCT_SERIES_STRING)))
        {
            sal_sscanf(buf, "%s %x", tmp, &product_series);
        }
    }

    if (GLB_SERIES_E550 == product_series)
    {
        switch (board_type)
        {
        case GLB_BOARD_E550_24T16X:
            product_oid = 5501;
            break;
        case GLB_BOARD_E550_24T16Y:
            product_oid = 5502;
            break;
        default:
            break;
        }
    }
    if (GLB_SERIES_E530 == product_series)
    {
        /* TODO */
    }
    if (GLB_SERIES_E580 == product_series)
    {
        switch (board_type)
        {
        case GLB_BOARD_E580_24Q:
            product_oid = 5801;
            break;
        case GLB_BOARD_E580_48X2Q4Z:
            product_oid = 5802;
            break;
        case GLB_BOARD_E580_48X6Q:
            product_oid = 5803;
            break;
        case GLB_BOARD_E580_32X2Q:
            product_oid = 5804;
            break;

        default:
            break;
        }
    }
    else if (GLB_SERIES_E350 == product_series)
    {
        switch (board_type)
        {
        case GLB_BOARD_E350_48T4XG:
            product_oid = 3520;
            break;
        case GLB_BOARD_E350_8TS12X:
            product_oid = 3200;
            break;
        }
    
    }

    return product_oid;
}

int32
mib2_system_init_sysoid()
{
    uint32 i = 0;

    g_mib2_sys.sysoid_len = 9;
    
    for (i = 0; i < 8; i++)
    {
        g_mib2_sys.sysoid[i] = BASE_SYSOBJID[i];
        if (CTCENTERPRISE_ID == g_mib2_sys.sysoid[i])
        {
            g_mib2_sys.sysoid[i] = snmpagt_get_en_oid();
        }
    }
    g_mib2_sys.sysoid[8] = mib2_get_product_oid();

    return 0;
}

int32
mib2_read_sysuptime()
{
    char buf[CMD_BUF_32];
    char *p = NULL;
    FILE *fp = NULL;
    int32 val = 0;

    if ((fp = sal_fopen("/proc/uptime", SAL_OPEN_RO)) == NULL)
    {
        return -1;
    }

    while (sal_fgets(buf, CMD_BUF_32, fp) != NULL)
    {
        p = sal_strchr(buf, '.');
        if (NULL == p)
        {
            return -1;
        }

        p[0] = CMD_ZERO_CHAR;
        sal_sscanf(buf, "%u", &val);
        g_mib2_sys.uptime = 100 * val;
        break;
    }
    sal_fclose(fp);

    return 0;
}

#ifdef _GLB_UML_SYSTEM_
#define M_SNMPD_CFGFILE     "/centec_switch/etc/snmpd.conf"
#else
#define M_SNMPD_CFGFILE     CTC_ETC_BASE_DIR"/etc/snmpd.conf"
#endif

int
mib2_system_update_sysdescr()
{
    FILE *fp = NULL;
    char line[CMD_BUF_1024];
    char *p = NULL;

    if (sal_strlen(g_mib2_sys.sysdescr))
    {
        return 0;
    }
    
    fp = sal_fopen(M_SNMPD_CFGFILE, "r");
    if (NULL == fp)
    {
        return 0;
    }


    while(sal_fgets(line, CMD_BUF_1024, fp) != NULL)
    {
        p = sal_strstr(line, "sysdescr ");
        if (p)
        {
            p += sal_strlen("sysdescr ");
            sal_strncpy(g_mib2_sys.sysdescr, p, CMD_BUF_256);
            break;
        }
    }

    sal_fclose(fp);
    return 0;
}
        
u_char*
mib2_system_handle(struct variable *vp,
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
    case SYSDESCR_INDEX:
        mib2_system_update_sysdescr();
        SNMPAGT_RETURN_STRING(g_mib2_sys.sysdescr, sal_strlen(g_mib2_sys.sysdescr));
        break;
    case SYSOID_INDEX:
        SNMPAGT_RETURN_OID(g_mib2_sys.sysoid, g_mib2_sys.sysoid_len);
        break;
    case SYSUPTIME_INDEX:
        mib2_read_sysuptime();
        SNMPAGT_RETURN_TIMETICKS(g_mib2_sys.uptime);
        break;
    case SYSCONTACT_INDEX:
        SNMPAGT_RETURN_STRING(g_mib2_sys.syscontact, sal_strlen(g_mib2_sys.syscontact));
        break;
    case SYSNAME_INDEX:
        SNMPAGT_RETURN_STRING(g_mib2_sys.sysname, sal_strlen(g_mib2_sys.sysname));
        break;
    case SYSLOCATION_INDEX:
        SNMPAGT_RETURN_STRING(g_mib2_sys.syslocation, sal_strlen(g_mib2_sys.syslocation));
        break;
    default:
        break;
    }

    return NULL;
}

struct variable MIB2_SYSTEM_VAR[] = {
    {1, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {1}},
    {2, ASN_OBJECT_ID, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {2}},
    {3, ASN_TIMETICKS, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {3}},
    {4, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {4}},
    {5, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {5}},
    {6, ASN_OCTET_STR, NETSNMP_OLDAPI_RONLY, mib2_system_handle, 1, {6}},
};

int32
mib2_system_init()
{
    snmpagt_nodes_t *p_node = &g_mib2_sys.node;
    int32 rc = 0;
    
    sal_memset(&g_mib2_sys, 0, sizeof(g_mib2_sys));

    snmpagt_oid2str(SYS_OID, sizeof(SYS_OID)/sizeof(oid), p_node->oid_str, CMD_BUF_256);
    p_node->vars = MIB2_SYSTEM_VAR;
    p_node->var_num = sizeof(MIB2_SYSTEM_VAR)/sizeof(MIB2_SYSTEM_VAR[0]);
    mib2_system_init_sysoid();
    rc = snmpagt_nodes_create(p_node);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "snmpagt_nodes_create %s failed", p_node->oid_str);
    }

    return rc;
}

int32
mib2_system_update_syscontact(char *str)
{
    sal_strncpy(g_mib2_sys.syscontact, str, SNMP_INFO_SIZE);
    return 0;
}

int32
mib2_system_update_syslocation(char *str)
{
    sal_strncpy(g_mib2_sys.syslocation, str, SNMP_INFO_SIZE);
    return 0;
}

int32
mib2_system_update_sysname(char *str)
{
    sal_strncpy(g_mib2_sys.sysname, str, CMD_BUF_256);
    return 0;
}

