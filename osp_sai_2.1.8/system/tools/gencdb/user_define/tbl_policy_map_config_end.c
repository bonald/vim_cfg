int32
tbl_policy_map_config_get_policy_map_config_count()
{
    tbl_policy_map_config_master_t *p_master = _g_p_tbl_policy_map_config_master;

    return p_master->policy_map_config_hash->count;
}

tbl_policy_map_config_t*
tbl_policy_map_config_get_policy_map_config_by_name(const char* policy_map_name)
{
    tbl_policy_map_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, policy_map_name, PMAP_NAME_SIZE);
    return tbl_policy_map_config_get_policy_map_config(&key);
}


