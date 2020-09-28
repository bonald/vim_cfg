tbl_aclqos_if_t*
tbl_aclqos_if_get_aclqos_if_by_name(const char* ifname)
{
    tbl_aclqos_if_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_aclqos_if_get_aclqos_if(&key);
}