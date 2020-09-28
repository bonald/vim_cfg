int32
tbl_snmp_notify_num_get()
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_notify_list->count;
    return count;
}

tbl_snmp_notify_t *
tbl_snmp_notify_get_by_tagname(char *tag_name)
{
    tbl_snmp_notify_master_t *p_master = _g_p_tbl_snmp_notify_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_notify_t *p_db_notify = NULL;

    if (NULL == p_master)
    {
        return NULL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_notify_list, p_db_notify, listnode, next)
    {
        if (0 == sal_strcmp(p_db_notify->tag_name, tag_name))
        {
            return p_db_notify;
        }
    }
    return NULL;
}


