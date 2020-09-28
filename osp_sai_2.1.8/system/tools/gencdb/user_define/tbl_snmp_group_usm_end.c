int32
tbl_snmp_group_usm_num_get()
{
    tbl_snmp_group_usm_master_t *p_master = _g_p_tbl_snmp_group_usm_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_group_usm_list->count;
    return count;
}

int32
tbl_snmp_group_usm_security_exist_check(char *sec_name)
{
    tbl_snmp_group_usm_master_t *p_master = _g_p_tbl_snmp_group_usm_master;
    tbl_snmp_group_usm_t *p_db_group = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 ret = 0;

    if (NULL == p_master)
    {
        return 0;
    }
    CTCLIB_SLIST_LOOP(p_master->snmp_group_usm_list, p_db_group, listnode)
    {
        if (0 == sal_strcmp(p_db_group->security_name, sec_name))
        {
            ret++;
        }
    }
    return ret;
}

int32
tbl_snmp_group_usm_group_exist_check(char *grp_name)
{
    tbl_snmp_group_usm_master_t *p_master = _g_p_tbl_snmp_group_usm_master;
    tbl_snmp_group_usm_t *p_db_group = NULL;
    ctclib_slistnode_t *listnode = NULL;
    int32 ret = 0;

    if (NULL == p_master)
    {
        return 0;
    }
    CTCLIB_SLIST_LOOP(p_master->snmp_group_usm_list, p_db_group, listnode)
    {
        if (0 == sal_strcmp(p_db_group->key.group_usm.grp_name, grp_name))
        {
            ret++;
        }
    }
    return ret;
}

