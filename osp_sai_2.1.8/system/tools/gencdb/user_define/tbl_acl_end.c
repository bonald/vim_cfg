tbl_acl_t*
tbl_acl_get_acl_by_name(char *name)
{
    tbl_acl_key_t acl_key;

    sal_memset(&acl_key, 0, sizeof(acl_key));
    sal_strcpy(acl_key.name, name);
    return tbl_acl_get_acl(&acl_key);
}
