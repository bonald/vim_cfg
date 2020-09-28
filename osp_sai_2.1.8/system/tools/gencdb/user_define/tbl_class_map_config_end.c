int32
tbl_class_map_config_get_class_map_config_count()
{
    tbl_class_map_config_master_t *p_master = _g_p_tbl_class_map_config_master;

    return p_master->class_map_config_hash->count;
}

