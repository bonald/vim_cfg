tbl_route_if_t*
tbl_route_if_get_route_if_by_name(char* ifname)
{
    tbl_route_if_key_t rt_if_key;

    sal_memset(&rt_if_key, 0, sizeof(rt_if_key));
    sal_memcpy(rt_if_key.name, ifname, IFNAME_SIZE);
    
    return tbl_route_if_get_route_if(&rt_if_key);
}