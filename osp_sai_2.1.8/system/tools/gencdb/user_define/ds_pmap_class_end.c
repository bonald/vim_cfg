ds_pmap_class_t*
ds_pmap_class_get_pmap_class_by_name(tbl_pmap_t *p_db_pmap, char *name)
{
    ds_pmap_class_t pmap_class;

    sal_memset(&pmap_class, 0, sizeof(pmap_class));
    sal_strcpy(pmap_class.key.name, name);
   return ds_pmap_class_get_pmap_class(p_db_pmap, &pmap_class);
}

int32
ds_pmap_class_add_pmap_class_by_name(tbl_pmap_t *p_pmap, char* name)
{
    ds_pmap_class_t pmap_class;
    
    sal_memset(&pmap_class, 0, sizeof(pmap_class));
    sal_strcpy(pmap_class.key.name, name);
    return ds_pmap_class_add_pmap_class(p_pmap, &pmap_class);
}

int32
ds_pmap_class_del_pmap_class_by_name(tbl_pmap_t *p_pmap, char* name)
{
    ds_pmap_class_t pmap_class;
    
    sal_memset(&pmap_class, 0, sizeof(pmap_class));
    sal_strcpy(pmap_class.key.name, name);
    return ds_pmap_class_del_pmap_class(p_pmap, &pmap_class);
}
