
#include "snmpagt_define.h"
#include "snmpagt.h"

static oid PRIV_OPER_OID[]     = { CTCENTERPRISE_OID, 1, 3};

#define  CONFIGSAVE_INDEX       1
#define  RELOAD_INDEX           2
#define  RESETTOFACTORY_INDEX   20


typedef struct
{
    snmpagt_nodes_t node;
    long            configsave;
    long            reload;
    long            resettofactory;
} priv_oper_t;

priv_oper_t g_priv_oper;


#define SNMP_OPERATE_CONFIGSAVE_WRITE       1

#define SNMP_OPERATE_RELOAD_WITHOUT_SAVE    1
#define SNMP_OPERATE_RELOAD_WITH_SAVE       2

#define SNMP_OPERATE_RESETTOFACTORY_RESET   1

#define SNMP_WRITE_FILE "config-snmpset"

int32
priv_oper_do_write()
{
    char cmd[CMD_BUF_256];
    int32 rc = 0;
    
    sal_snprintf(cmd, CMD_BUF_256, "cdbctl buildcfg/startup/%s/cdb;cp /tmp/%s /mnt/flash/startup-config.conf;rm -rf /tmp/%s", 
        SNMP_WRITE_FILE, SNMP_WRITE_FILE, SNMP_WRITE_FILE);
    rc = sal_system(cmd);
    if (0 != rc)
    {
        return -1;
    }

    sal_system("cdbctl update/cdb/sys/send_trap/write_success");
    return 0;
}

/* The write method for dot1dBridge Scalar objects */
int
priv_oper_handle_write_configsave(int action, u_char * var_val,
            u_char var_val_type, size_t var_val_len,
            u_char * statP, oid * name, size_t length,
            struct variable *v, u_int32_t vr_id)
{
    long value = 0;
    int32 rc = 0;
    
    /* omit other actions */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 != action) && (SNMP_MSG_INTERNAL_SET_ACTION != action))
    {
        return 0;
    }

    /* check */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 == action) || (SNMP_MSG_INTERNAL_SET_ACTION == action))
    {
        if (var_val_type != ASN_INTEGER)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len != sizeof (long))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        sal_memcpy(&value, var_val, var_val_len);
        if (SNMP_OPERATE_CONFIGSAVE_WRITE != value)
        {
            return SNMP_ERR_WRONGVALUE;
        }
    }

    /* action */
    if (SNMP_MSG_INTERNAL_SET_ACTION == action)
    {
        rc = priv_oper_do_write();
        if (rc < 0)
        {
            return SNMP_ERR_COMMITFAILED;
        }
        return SNMP_ERR_NOERROR;
    }

    return 0;
}

int
priv_oper_handle_write_reload(int action, u_char * var_val,
            u_char var_val_type, size_t var_val_len,
            u_char * statP, oid * name, size_t length,
            struct variable *v, u_int32_t vr_id)
{
    long value = 0;
    
    /* omit other actions */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 != action) && (SNMP_MSG_INTERNAL_SET_ACTION != action))
    {
        return 0;
    }

    /* check */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 == action) || (SNMP_MSG_INTERNAL_SET_ACTION == action))
    {
        if (var_val_type != ASN_INTEGER)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len != sizeof (long))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        sal_memcpy(&value, var_val, var_val_len);
        if ((SNMP_OPERATE_RELOAD_WITHOUT_SAVE != value) && (SNMP_OPERATE_RELOAD_WITH_SAVE != value))
        {
            return SNMP_ERR_WRONGVALUE;
        }
    }

    /* action */
    if (SNMP_MSG_INTERNAL_SET_ACTION == action)
    {
        if (SNMP_OPERATE_RELOAD_WITH_SAVE == value)
        {
            priv_oper_do_write();
            sal_system("cdbctl update/cdb/sys/reboot/normal/save");
        }
        else
        {
            sal_system("cdbctl update/cdb/sys/reboot/normal");
        }
        sal_system("reboot_record 0x06 1>/dev/null 2>&1 && sync");
    }

    return 0;
}

int
priv_oper_handle_write_factory(int action, u_char * var_val,
            u_char var_val_type, size_t var_val_len,
            u_char * statP, oid * name, size_t length,
            struct variable *v, u_int32_t vr_id)
{
    char cmd[CMD_BUF_256];
    long value = 0;
    int32 rc = 0;
    
    /* omit other actions */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 != action) && (SNMP_MSG_INTERNAL_SET_ACTION != action))
    {
        return 0;
    }

    /* check */
    if ((SNMP_MSG_INTERNAL_SET_RESERVE1 == action) || (SNMP_MSG_INTERNAL_SET_ACTION == action))
    {
        if (var_val_type != ASN_INTEGER)
        {
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len != sizeof (long))
        {
            return SNMP_ERR_WRONGLENGTH;
        }
        sal_memcpy(&value, var_val, var_val_len);
        if (SNMP_OPERATE_RESETTOFACTORY_RESET != value)
        {
            return SNMP_ERR_WRONGVALUE;
        }
    }

    /* action */
    if (SNMP_MSG_INTERNAL_SET_ACTION == action)
    {
        sal_snprintf(cmd, CMD_BUF_256, "cp -rf %s %s", GLB_FACTORY_CONFIG_FILE_PATH, GLB_STARTUP_CONFIG_FILE_PATH);
        rc = sal_system(cmd);
        if (0 != rc)
        {
            return SNMP_ERR_COMMITFAILED;
        }
    }

    return 0;
}

u_char*
priv_oper_handle(struct variable *vp,
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
    case CONFIGSAVE_INDEX:
        *write_method = (WriteMethod*)priv_oper_handle_write_configsave;
        SNMPAGT_RETURN_INTEGER(g_priv_oper.configsave);
        break;
    case RELOAD_INDEX:
        *write_method = (WriteMethod*)priv_oper_handle_write_reload;
        SNMPAGT_RETURN_INTEGER(g_priv_oper.reload);
        break;
    case RESETTOFACTORY_INDEX:
        *write_method = (WriteMethod*)priv_oper_handle_write_factory;
        SNMPAGT_RETURN_INTEGER(g_priv_oper.resettofactory);
        break;
    default:
        break;
    }

    return NULL;
}

struct variable PRIV_OPER_VAR[] = {
    {1,  ASN_INTEGER, NETSNMP_OLDAPI_RWRITE, priv_oper_handle, 1, {1}},
    {2,  ASN_INTEGER, NETSNMP_OLDAPI_RWRITE, priv_oper_handle, 1, {2}},
    {20, ASN_INTEGER, NETSNMP_OLDAPI_RWRITE, priv_oper_handle, 1, {20}},
};

int32
priv_operate_init()
{
    snmpagt_nodes_t *p_node = &g_priv_oper.node;
    int32 rc = 0;
    
    sal_memset(&g_priv_oper, 0, sizeof(g_priv_oper));

    snmpagt_oid2str(PRIV_OPER_OID, sizeof(PRIV_OPER_OID)/sizeof(oid), p_node->oid_str, CMD_BUF_256);
    p_node->vars = PRIV_OPER_VAR;
    p_node->var_num = sizeof(PRIV_OPER_VAR)/sizeof(PRIV_OPER_VAR[0]);
    rc = snmpagt_nodes_create(p_node);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "snmpagt_nodes_create %s failed", p_node->oid_str);
    }

    return rc;
}

