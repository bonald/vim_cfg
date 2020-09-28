tbl_qos_queue_shape_profile_t*
tbl_qos_queue_shape_profile_get_profile_by_name(char *name)
{
    tbl_qos_queue_shape_profile_master_t *p_master = _g_p_tbl_qos_queue_shape_profile_master;
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shape_profile = NULL;
    uint32 id = 0;

    for (id = 0; id < GLB_QOS_SHAPE_PROFILE_NUM; id++)
    {
        p_db_qos_queue_shape_profile = p_master->qos_queue_shape_profile_array[id];
        if (NULL == p_db_qos_queue_shape_profile)
        {
            continue;
        }

        if (0 == sal_strcmp(p_db_qos_queue_shape_profile->name, name))
        {
            return p_db_qos_queue_shape_profile;
        }
    }
    return NULL;
}

int32
tbl_qos_queue_shape_profile_get_field(char *name, tbl_qos_queue_shape_profile_field_id_t field_id, void* field_value)
{
    tbl_qos_queue_shape_profile_t *p_db_qos_queue_shape_profile = NULL;

    /* 1. lookup entry exist */
    p_db_qos_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(name);
    if (NULL == p_db_qos_queue_shape_profile)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_NAME:
        sal_memcpy(field_value, p_db_qos_queue_shape_profile->name, sizeof(p_db_qos_queue_shape_profile->name));
        break;

    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR:
        sal_memcpy(field_value, &p_db_qos_queue_shape_profile->pir, sizeof(p_db_qos_queue_shape_profile->pir));
        break;
    
    case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MAX:
        sal_memcpy(field_value, p_db_qos_queue_shape_profile, sizeof(tbl_qos_queue_shape_profile_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}

int32
tbl_qos_queue_shape_profile_alloc_profile_id(uint32 *p_id)
{
    tbl_qos_queue_shape_profile_master_t *p_master = _g_p_tbl_qos_queue_shape_profile_master;
    uint32 i = 0;

    for (i = 0; i < GLB_QOS_SHAPE_PROFILE_NUM; i++)
    {
        if (!p_master->qos_queue_shape_profile_array[i])
        {
            *p_id = i;
            return PM_E_NONE;
        }
    }

    return PM_E_RESOURCE_FULL;
}
