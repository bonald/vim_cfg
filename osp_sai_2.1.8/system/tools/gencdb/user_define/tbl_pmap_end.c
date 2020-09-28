tbl_pmap_t*
tbl_pmap_get_pmap_by_name(char *name)
{
    tbl_pmap_master_t *p_master = _g_p_tbl_pmap_master;
    tbl_pmap_t lkp;
    
    sal_memset(&lkp, 0, sizeof(lkp));
    sal_strcpy(lkp.key.name, name);
    return ctclib_hash_lookup(p_master->pmap_hash, &lkp);
}

int32
tbl_pmap_add_pmap_by_name(char *name)
{
    tbl_pmap_t pmap;
    
    sal_memset(&pmap, 0, sizeof(pmap));
    sal_strcpy(pmap.key.name, name);
    return tbl_pmap_add_pmap(&pmap);
}

int32
tbl_pmap_del_pmap_by_name(char *name)
{
    tbl_pmap_t pmap;
    
    sal_memset(&pmap, 0, sizeof(pmap));
    sal_strcpy(pmap.key.name, name);
    return tbl_pmap_del_pmap(&pmap.key);
}
