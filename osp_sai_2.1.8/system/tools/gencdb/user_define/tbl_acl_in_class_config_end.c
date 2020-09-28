
tbl_acl_in_class_config_t*
tbl_acl_in_class_config_get_acl_in_class_config_by_name(const char* class_map_name, const char* acl_name)
{
    tbl_acl_in_class_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.class_name, class_map_name, CMAP_NAME_SIZE);
    sal_strncpy(key.acl_name, acl_name, ACL_NAME_SIZE);
    return tbl_acl_in_class_config_get_acl_in_class_config(&key);
}

tbl_acl_in_class_config_t*
tbl_acl_in_class_config_get_acl_in_class_config_by_acl_priority(const uint64 acl_priority)
{
    tbl_acl_in_class_config_master_t *p_master = _g_p_tbl_acl_in_class_config_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_acl_in_class_config_t *p_db_acl_in_class_config = NULL;

    CTCLIB_SLIST_LOOP_DEL(p_master->acl_in_class_config_list, p_db_acl_in_class_config, listnode, next)
    {
        if(acl_priority == p_db_acl_in_class_config->acl_priority)
        {
            return p_db_acl_in_class_config;
        }
    }

    return NULL;
}

