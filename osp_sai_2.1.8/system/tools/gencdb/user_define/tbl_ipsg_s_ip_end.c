
tbl_ipsg_s_ip_t*
tbl_ipsg_s_ip_get_ipsg_s_ip_by_ipaddr(prefix_t *preifx)
{
    tbl_ipsg_s_ip_key_t ipsg_s_ip_key;

    sal_memset(&ipsg_s_ip_key, 0, sizeof(tbl_ipsg_s_ip_key_t));
    sal_memcpy(&ipsg_s_ip_key.ip, preifx, sizeof(prefix_t));
    return tbl_ipsg_s_ip_get_ipsg_s_ip(&ipsg_s_ip_key);
}

