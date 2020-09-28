tbl_time_range_t*
tbl_time_range_get_time_range_by_name(char *name)
{
    tbl_time_range_key_t key;
    
    sal_memset(&key, 0, sizeof(key));
    sal_strcpy(key.name, name);
    return tbl_time_range_get_time_range(&key);
}


int32
tbl_time_range_get_time_range_count()
{
    tbl_time_range_master_t *p_master = _g_p_tbl_time_range_master;

    return p_master->time_range_list->count;
}