
tbl_kernel_if_t *
tbl_kernel_if_get_by_name(char *name)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;

    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if(0 == sal_strcmp(p_db_kernel_if->name, name))
        {
             return p_db_kernel_if;
        }
    }
    
    return NULL;
}

tbl_kernel_if_t *
tbl_kernel_if_check_by_list_loop(addr_ipv4_t *prefix)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    addr_ipv4_t ip_addr_check;    

    sal_memset(&ip_addr_check, 0, sizeof(ip_addr_check));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            if (ip_addr_check.s_addr && (0 == sal_memcmp(&ip_addr_check, prefix, sizeof(addr_ipv4_t))))
            {
                return p_db_kernel_if;
            }                
        }
    }
    
    return NULL;
}

tbl_kernel_if_t *
tbl_kernel_if_check_by_ipmask_list_loop(addr_ipv4_t *ip_mask)
{
    ctclib_slistnode_t *listnode    = NULL;    
    tbl_kernel_if_t *p_db_kernel_if = NULL;
    tbl_kernel_if_master_t *p_master = _g_p_tbl_kernel_if_master;
    addr_ipv4_t ip_addr_check;
    uint32      mask = 0;
    
    sal_memset(&ip_addr_check, 0, sizeof(ip_addr_check));
    CTCLIB_SLIST_LOOP(p_master->kernel_if_list, p_db_kernel_if, listnode)
    {
        if (NULL != p_db_kernel_if)
        {
            ip_addr_check.s_addr = sal_htonl(p_db_kernel_if->ip_addr.s_addr);
            IPV4_LEN_TO_MASK(mask, p_db_kernel_if->masklen);
            ip_addr_check.s_addr = ip_addr_check.s_addr & mask;
            if (ip_addr_check.s_addr && (0 == sal_memcmp(&ip_addr_check, ip_mask, sizeof(addr_ipv4_t))))
            {
                return p_db_kernel_if;
            }                
        }
    }
    
    return NULL;
}

