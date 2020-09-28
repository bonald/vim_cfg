
tbl_class_map_action_config_t*
tbl_class_map_action_config_get_class_map_action_config_by_name(const char* policy_map_name, const char* class_map_name)
{
    tbl_class_map_action_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.policy_name, policy_map_name, PMAP_NAME_SIZE);
    sal_strncpy(key.class_name, class_map_name, CMAP_NAME_SIZE);
    return tbl_class_map_action_config_get_class_map_action_config(&key);
}