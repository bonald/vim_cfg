
#include "snmpagt_define.h"
#include "snmpagt.h"

#define SNMP_CHECK_CONNECTION_INTERVAL   10

extern netsnmp_session *main_session;
extern void
agentx_reopen_session(unsigned int clientreg, void *clientarg);
extern int
subagent_open_master_session(void);

extern void
ctc_init_sigpipe(void);

extern int 
oem_init();

extern int
split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
             char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr);
extern int
free_tokens(char ***pppTokArr);

static char* SA_AGENT_NAME      = "snmpagent";
static char* SA_MASTER_SOCKET   = "/var/run/agentx-master";
static oid enterpriseID = CTCENTERPRISE_ID;

static port_bmp_t ifg_stats_en;

int32
snmpagt_get_ifg_stats_en(uint32 ifindex)
{
    return GLB_BMP_ISSET(ifg_stats_en, ifindex);
}

int32
snmpagt_set_ifg_stats_en(tbl_interface_t *p_if)
{
    int32 rc = 0;
    if (NULL == p_if)
    {
        return rc;
    }
    if (p_if->ifg_stats_en)
    {
        GLB_BMP_SET(ifg_stats_en, p_if->ifindex);
    }
    else
    {
        GLB_BMP_UNSET(ifg_stats_en, p_if->ifindex);
    }

    return rc;
}

oid
snmpagt_get_en_oid()
{
    return enterpriseID;
}

int32
snmpagt_oid2str(oid *oids, size_t oid_len, char *oid_str, size_t str_len)
{
    char tmp[16];
    uint32 i = 0;
    int oid = 0;
    
    sal_memset(oid_str, 0, str_len);

    for (i = 0; i < oid_len; i++)
    {
        oid = (int)oids[i];
        if (CTCENTERPRISE_ID == oid)
        {
            oid = (int)snmpagt_get_en_oid();
        }
        sal_snprintf(tmp, 16, "%d.", oid);
        sal_strcat(oid_str, tmp);
    }

    i = sal_strlen(oid_str);
    if (i > 0)
    {
        oid_str[i-1] = CMD_ZERO_CHAR;
    }

    return 0;
}

int32
snmpagt_str2oid(char *oid_str, oid *oids, size_t *oid_len)
{
    char **ppTypeArr = NULL;
    unsigned long nCnt = 0;
    unsigned long nMaxToks = 0;
    int int_value;
    size_t i = 0;

    nMaxToks = (unsigned long)(*oid_len);
    
    if (split_tokens(oid_str, sal_strlen(oid_str), nMaxToks, ".", &nCnt, &ppTypeArr) != 0)
    {
        return -1;
    }
    
    for (i = 0; i < nCnt; i++)
    {
        int_value = 0;
        sal_sscanf(ppTypeArr[i], "%d", &int_value);
        oids[i] = (oid)int_value;
    }
    *oid_len = i;
    
    free_tokens(&ppTypeArr);
    return 0;
}

#define __SNMPAGT_API_TABLE__

int32
snmpagt_nodes_create(snmpagt_nodes_t *p_node)
{
    oid oid[MAX_OID_LEN];
    size_t oid_len = 0;
    int32 rc = 0;

    sal_memset(oid, 0, sizeof(oid));
    oid_len = MAX_OID_LEN;
    snmpagt_str2oid(p_node->oid_str, oid, &oid_len);
    rc = register_mib(p_node->oid_str,
                 p_node->vars,
                 sizeof(struct variable), 
                 p_node->var_num,
                 oid, oid_len);
        
    return rc;
}

int32
snmpagt_table_set_allow_creation(snmpagt_table_t *table, uint32 onoff)
{
    table->p_dataset->allow_creation = onoff;
    return 0;
}

int32
snmpagt_table_set_rowstatus_column(snmpagt_table_t *table, uint32 column)
{
    table->p_dataset->rowstatus_column = column;
    return 0;
}

netsnmp_table_row*
snmpagt_table_get_row_by_int32_index(snmpagt_table_t *p_table, int32 index)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 index_i32 = index;

    if (NULL == p_table)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &index_i32, sizeof(index_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    
    row = netsnmp_table_data_get(p_table->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}

netsnmp_table_row*
snmpagt_table_get_row_by_int32_double_index(snmpagt_table_t *p_table, int32 index1, int32 index2)
{
    netsnmp_variable_list *indexes = NULL;
    netsnmp_table_row *row = NULL;
    int32 index1_i32 = index1;
    int32 index2_i32 = index2;

    if (NULL == p_table)
    {
        return NULL;
    }
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &index1_i32, sizeof(index1_i32));
    snmp_varlist_add_variable(&indexes, NULL, 0, ASN_INTEGER, &index2_i32, sizeof(index2_i32));
    if (NULL == indexes)
    {
        return NULL;
    }
    
    row = netsnmp_table_data_get(p_table->p_dataset->table, indexes);
    snmp_free_varbind(indexes);
    return row;
}

snmpagt_table_t*
snmpagt_table_create(snmpagt_table_param_t *param)
{
    snmpagt_table_t *p_table = NULL;
    netsnmp_handler_registration *reginfo = NULL;
    uint32 i = 0;
    uint32 colno = 1;
    int32 count32 = 0;
    int64 count64 = 0;
    uint32 u32 = 0;
    int32  i32 = 0;
    void  *default_value = NULL;
    size_t default_value_len = 0;
    oid oid[MAX_OID_LEN];
    size_t oid_len = 0;
    int rc = 0;

    p_table = XCALLOC(MEM_TEMP, sizeof(snmpagt_table_t));
    p_table->p_dataset = netsnmp_create_table_data_set(param->oid_str);
    p_table->param.oid_str = XSTRDUP(MEM_TEMP, param->oid_str);
    p_table->param.no_idx_col = param->no_idx_col;

    p_table->p_dataset->allow_creation = 0;
    // self._col_writable = {}

    for (i = 0; i < param->index_count; i++)
    {
        netsnmp_table_dataset_add_index(p_table->p_dataset, param->index_type[i]);
        if (0 == param->no_idx_col)
        {
            p_table->_col_writable[colno] = 0;
        }
        colno = colno + 1;
    }

    for (i = 0; i < param->row_count; i++)
    {
        switch (param->row[i].type)
        {
        case ASN_OCTET_STR:
            default_value = NULL;
            default_value_len = 0;
            break;
        case ASN_COUNTER:
            default_value = &count32;
            default_value_len = sizeof(count32);
            break;
        case ASN_COUNTER64:
            default_value = &count64;
            default_value_len = sizeof(count64);
            break;
        case ASN_UNSIGNED:
            default_value = &u32;
            default_value_len = sizeof(u32);
            break;
        case ASN_INTEGER:
            default_value = &i32;
            default_value_len = sizeof(i32);
            break;
        case ASN_TIMETICKS:
            default_value = &u32;
            default_value_len = sizeof(u32);
            break;
        case ASN_OBJECT_ID:
            default_value = NULL;
            default_value_len = 0;
            break;
        default:
            SAL_ASSERT(0);
            break;            
        }
        rc = netsnmp_table_set_add_default_row(p_table->p_dataset,
                                  param->row[i].colno,
                                  param->row[i].type, 
                                  param->row[i].writable, 
                                  default_value,
                                  default_value_len);
        if (rc < 0)
        {
            log_sys(M_MOD_SNMPAGT, E_ERROR, "Init netsnmp_table_set_add_default_row() oid %s fail", param->oid_str);
            goto ERROR;
        }
    }

    sal_memset(oid, 0, sizeof(oid));
    oid_len = MAX_OID_LEN;
    snmpagt_str2oid(param->oid_str, oid, &oid_len);
    reginfo = netsnmp_create_handler_registration
                                    (param->oid_str,
                                     param->nodehandler,
                                     oid,
                                     oid_len,
                                     HANDLER_CAN_RWRITE);
    if (NULL == reginfo)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Call netsnmp_create_handler_registration() oid %s fail", param->oid_str);
        goto ERROR;
    }
    
    rc = netsnmp_register_table_data_set(reginfo, p_table->p_dataset, NULL);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Call netsnmp_register_table_data_set() oid %s fail", param->oid_str);
        goto ERROR;
    }
        
    return p_table;

ERROR:
    if (p_table->p_dataset)
    {
        netsnmp_delete_table_data_set(p_table->p_dataset);
    }
    if (p_table)
    {
        XFREE(MEM_TEMP, p_table);
    }

    return NULL;
}

#define __SNMPAGT_CONNECT_AGENTX__

int
_snmpagt_log_handler(int majorid, int minorid, void *serverarg,
                       void *clientarg)
{
    struct snmp_log_message *log = (struct snmp_log_message*)serverarg;

    if (sal_strstr(log->msg, "Cannot find module"))
    {
        return PM_E_NONE;
    }
    else if (sal_strstr(log->msg, "AgentX master disconnected us"))
    {
        log_sys(M_MOD_SNMPAGT, log->priority, "AgentX subagent disconnected from snmpd");
        return PM_E_NONE;
    }
    else if (sal_strstr(log->msg, "AgentX subagent connected"))
    {
        log_sys(M_MOD_SNMPAGT, log->priority, "AgentX subagent connected to snmpd");
        return PM_E_NONE;
    }
    log_sys(M_MOD_SNMPAGT, log->priority, "%s", log->msg);

    return PM_E_NONE;
}

int
_snmpagt_index_stop_handler(int majorid, int minorid, void *serverarg,
                       void *clientarg)
{
    /* TODO */
    return PM_E_NONE;
}

int32
snmpagt_init_agentx()
{
    int rc = 0;

    /* register callbacks */
    rc = snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_LOGGING, _snmpagt_log_handler, NULL);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Init snmp_register_callback() LOGGING fail");
    }

    snmp_enable_calllog();

    rc = snmp_register_callback(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_INDEX_STOP, _snmpagt_index_stop_handler, NULL);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Init snmp_register_callback() INDEX_STOP fail");
    }
    
    /*  TODO
        # Register it with net-snmp
        if libnsa.snmp_register_callback(
            SNMP_CALLBACK_LIBRARY,
            SNMP_CALLBACK_USER_FUNC_1,
            self._index_data_mutex_callback,
            None
        ) != SNMPERR_SUCCESS:
            raise netsnmpAgentException(
                "snmp_register_callback() failed for _netsnmp_index_callback!"
            )
    */

    rc = netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Init netsnmp_ds_set_boolean() AGENT_ROLE 1 fail");
    }

    rc = netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET, SA_MASTER_SOCKET);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Init netsnmp_ds_set_string() AGENT_X_SOCKET %s fail", SA_MASTER_SOCKET);
    }

    /* TODO
    netsnmp_set_mib_directory('+/centec_switch/mibs')
    */

    rc = init_agent(SA_AGENT_NAME);
    if (rc < 0)
    {
        log_sys(M_MOD_SNMPAGT, E_ERROR, "Init init_agent() fail");
    }

    netsnmp_init_mib();

    ctc_init_sigpipe();
    
    return PM_E_NONE;
}

int32
snmpagt_connect_agentx()
{
    init_snmp(SA_AGENT_NAME);
    oem_init();
    
    return PM_E_NONE;
}

void
snmpagt_agentx_read_cb(void *p_data)
{
    int             count, numfds, block;
    fd_set          readfds,writefds,exceptfds;
    struct timeval  timeout, *tvp;

    numfds = 0;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    block = 0;
    tvp = &timeout;
    timerclear(tvp);
    tvp->tv_sec = 0;
    tvp->tv_usec = 1000;
    snmp_select_info(&numfds, &readfds, tvp, &block);
    count = select(numfds, &readfds, &writefds, &exceptfds, tvp);
    if (count > 0)
    {
        snmp_read(&readfds);
    }

    return;
}

int32
snmpagt_add_read_agext()
{
    int numfds = 0;
    int block = 0;
    int i = 0;
    fd_set          readfds;
    struct timeval  timeout;

    sal_memset(&readfds, 0, sizeof(readfds));
    snmp_select_info(&numfds, &readfds, &timeout, &block);

    for (i = 0; i < numfds; i++)
    {
        if (FD_ISSET(i, &readfds))
        {
            ctc_task_add_read(TASK_PRI_NORMAL, snmpagt_agentx_read_cb, NULL, i);
        }
    }
    
    return 0;
}

void
snmpagt_check_connection_fn(void* p_data)
{
    int rc = 0;
    
    if (NULL == main_session)
    {
        rc = subagent_open_master_session();
        if (rc < 0)
        {
            log_sys(M_MOD_SNMPAGT, E_WARNING, "Reconnect SNMPD failed");
        }
        else
        {
            log_sys(M_MOD_SNMPAGT, E_WARNING, "Reconnect SNMPD success");
            snmpagt_sub_resync();
        }
    }

    ctc_task_add_timer(TASK_PRI_NORMAL, snmpagt_check_connection_fn, NULL, SNMP_CHECK_CONNECTION_INTERVAL);
    
    return;
}

int32
snmpagt_init_en_oid()
{
    char buf[CMD_BUF_256];
    FILE *fp = NULL;
    int32 value = CTCENTERPRISE_ID;
    
    fp = sal_fopen("/tmp/oem_info", "r");
    if (NULL == fp)
    {
        return PM_E_NONE;
    }

    while(sal_fgets(buf, CMD_BUF_256, fp) != NULL)
    {
        if (!sal_strncmp(buf, "snmp_enterprise_oid", sal_strlen("snmp_enterprise_oid")))
        {
            sal_sscanf(buf, "snmp_enterprise_oid %d", &value);
        }
    }
    enterpriseID = (oid)value;

    return 0;
}

extern int32 interface_update_rate();

void
snmpagt_routine_fn(void* p_data)
{
    interface_update_rate();
    ctc_task_add_timer(TASK_PRI_NORMAL, snmpagt_routine_fn, NULL, SNMP_CHECK_CONNECTION_INTERVAL);
    
    return;
}

int32
snmpagt_agent_init()
{
    GLB_BMP_INIT(ifg_stats_en);
    snmpagt_init_en_oid();
    snmpagt_init_agentx();
    if_table_init();
    ifx_table_init();
    interface_table_init();
    priv_system_init();
    priv_operate_init();
    priv_mgmt_init();
    priv_device_init();
    mib2_system_init();
    mib2_entPhysicalTable_init();

    rmon_table_init();

    snmpagt_connect_agentx();
    snmpagt_add_read_agext();

    /* for reconnect to snmpd cannot register TBLs, then disable reconnect timer */
    //ctc_task_add_timer(TASK_PRI_NORMAL, snmpagt_check_connection_fn, NULL, SNMP_CHECK_CONNECTION_INTERVAL);
    ctc_task_add_timer(TASK_PRI_NORMAL, snmpagt_routine_fn, NULL, SNMP_CHECK_CONNECTION_INTERVAL);

    return PM_E_NONE;
}

