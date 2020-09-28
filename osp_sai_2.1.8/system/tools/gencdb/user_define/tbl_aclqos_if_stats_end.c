tbl_aclqos_if_stats_t*
tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(const char* ifname)
{
    tbl_aclqos_if_stats_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_aclqos_if_stats_get_aclqos_if_stats(&key);
}