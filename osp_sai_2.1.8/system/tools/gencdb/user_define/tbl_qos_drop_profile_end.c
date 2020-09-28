tbl_qos_drop_profile_t*
tbl_qos_drop_profile_get_profile_by_name(char *name)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    tbl_qos_drop_profile_t *p_db_qos_drop = NULL;
    uint32 id = 0;

    for (id = 0; id < GLB_QOS_DROP_PROFILE_NUM; id++)
    {
        p_db_qos_drop = p_master->qos_drop_profile_array[id];
        if (NULL == p_db_qos_drop)
        {
            continue;
        }
    
        if (0 == sal_strcmp(p_db_qos_drop->name, name))
        {
            return p_db_qos_drop;
        }
    }

    return NULL;
}

int32
tbl_qos_drop_profile_alloc_profile_id(uint32 *p_id)
{
    tbl_qos_drop_profile_master_t *p_master = _g_p_tbl_qos_drop_profile_master;
    uint32 i = 0;

    for (i = 0; i < GLB_QOS_DROP_PROFILE_NUM; i++)
    {
        if (!p_master->qos_drop_profile_array[i])
        {
            *p_id = i;
            return PM_E_NONE;
        }
    }

    return PM_E_RESOURCE_FULL;
}
