int32
tbl_ace_config_get_ace_config_count()
{
    tbl_ace_config_master_t *p_master = _g_p_tbl_ace_config_master;

    return p_master->ace_config_hash->count;
}

tbl_ace_config_t*
tbl_ace_config_get_ace_config_by_name(const char* acl_name, const uint32 seq_no)
{
    tbl_ace_config_key_t key;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.acl_name, acl_name, ACL_NAME_SIZE);
    key.seq_no = seq_no;
    return tbl_ace_config_get_ace_config(&key);
}

