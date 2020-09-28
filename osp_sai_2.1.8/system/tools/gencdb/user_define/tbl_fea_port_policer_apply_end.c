tbl_fea_port_policer_apply_t*
tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(const char* ifname)
{
    tbl_fea_port_policer_apply_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    return tbl_fea_port_policer_apply_get_fea_port_policer_apply(&key);
}