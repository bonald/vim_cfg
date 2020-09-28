tbl_cmap_t*
tbl_cmap_get_cmap_by_name(char *name)
{
    tbl_cmap_key_t cmap_key;
    
    sal_memset(&cmap_key, 0, sizeof(cmap_key));
    sal_strcpy(cmap_key.name, name);
    return tbl_cmap_get_cmap(&cmap_key);
}
