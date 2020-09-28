tbl_qos_port_shape_profile_t*
tbl_qos_port_shape_profile_get_profile_by_name(char *name)
{
    tbl_qos_port_shape_profile_master_t *p_master = _g_p_tbl_qos_port_shape_profile_master;
    tbl_qos_port_shape_profile_t *p_db_qos_port_shape_profile = NULL;
    uint32 id = 0;

    for (id = 0; id < GLB_QOS_PORT_SHAPE_PROFILE_NUM; id++)
    {
        p_db_qos_port_shape_profile = p_master->qos_port_shape_profile_array[id];
        if (NULL == p_db_qos_port_shape_profile)
        {
            continue;
        }

        if (0 == sal_strcmp(p_db_qos_port_shape_profile->name, name))
        {
            return p_db_qos_port_shape_profile;
        }
    }
    return NULL;
}

int32
tbl_qos_port_shape_profile_alloc_profile_id(uint32 *p_id)
{
    tbl_qos_port_shape_profile_master_t *p_master = _g_p_tbl_qos_port_shape_profile_master;
    uint32 i = 0;

    for (i = 0; i < GLB_QOS_PORT_SHAPE_PROFILE_NUM; i++)
    {
        if (!p_master->qos_port_shape_profile_array[i])
        {
            *p_id = i;
            return PM_E_NONE;
        }
    }

    return PM_E_RESOURCE_FULL;
}

