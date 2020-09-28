int32
tbl_snmp_view_num_get_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    int32 count = 0;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return 0;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            count++;
        }
    }
    return count;
}


int32
tbl_snmp_view_refcnt_add_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            p_db_snmp_view->refcnt++;
        }
    }
    return PM_E_NONE;
}

int32
tbl_snmp_view_refcnt_del_by_view_name(char *view_name)
{
    tbl_snmp_view_master_t *p_master = _g_p_tbl_snmp_view_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_snmp_view_t *p_db_snmp_view = NULL;

    if (NULL == p_master)
    {
        return PM_E_FAIL;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->snmp_view_list, p_db_snmp_view, listnode, next)
    {
        if (0 == sal_strcmp(p_db_snmp_view->key.view.name, view_name))
        {
            if (p_db_snmp_view->refcnt > 0)
            {
                p_db_snmp_view->refcnt--;
            }
        }
    }
    return PM_E_NONE;
}

