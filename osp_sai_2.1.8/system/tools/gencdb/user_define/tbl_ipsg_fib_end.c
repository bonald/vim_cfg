
tbl_ipsg_fib_t*
tbl_ipsg_fib_get_ipsg_fib_by_ifname(char *ifname)
{
    tbl_ipsg_fib_master_t *p_master     = _g_p_tbl_ipsg_fib_master;
    ctclib_slistnode_t  *listnode       = NULL;    
    tbl_ipsg_fib_t      *p_db_ipsg_fib  = NULL;

    CTCLIB_SLIST_LOOP(p_master->ipsg_fib_list, p_db_ipsg_fib, listnode)
    {
        if(0 == sal_strncmp(p_db_ipsg_fib->ifname, ifname, sizeof(p_db_ipsg_fib->ifname)))
        {
             return p_db_ipsg_fib;
        }
    }
    
    return NULL;
}

