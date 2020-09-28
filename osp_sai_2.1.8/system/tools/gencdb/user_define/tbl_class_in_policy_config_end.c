

tbl_class_in_policy_config_t*
tbl_class_in_policy_config_get_class_in_policy_config_by_name(const char* policy_map_name, const char* class_map_name)
{
    tbl_class_in_policy_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.policy_name, policy_map_name, PMAP_NAME_SIZE);
    sal_strncpy(key.class_name, class_map_name, CMAP_NAME_SIZE);
    return tbl_class_in_policy_config_get_class_in_policy_config(&key);
}

tbl_class_in_policy_config_t*
tbl_class_in_policy_config_get_class_in_policy_config_by_class_priority(const uint64 class_priority)
{
    tbl_class_in_policy_config_master_t *p_master = _g_p_tbl_class_in_policy_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_class_in_policy_config_t *p_db_class_in_policy_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->class_in_policy_config_list, p_db_class_in_policy_config, listnode, next)
    {
        if(class_priority == p_db_class_in_policy_config->class_priority)
        {
            return p_db_class_in_policy_config;
        }
    }

    return NULL;
}
