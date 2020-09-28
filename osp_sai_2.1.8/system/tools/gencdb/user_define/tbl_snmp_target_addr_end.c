int32
tbl_snmp_target_addr_num_get()
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_target_addr_list->count;
    return count;
}

tbl_snmp_target_addr_t *
tbl_snmp_target_addr_get_by_param_name(char *param_name)
{
    tbl_snmp_target_addr_master_t *p_master = _g_p_tbl_snmp_target_addr_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_target_addr_t *p_db_snmp_target_addr = NULL;

    if (NULL == p_master)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_target_addr_list, p_db_snmp_target_addr, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_target_addr->para_name, param_name))
        {
            return p_db_snmp_target_addr;
        }
    }
    return NULL;
}

