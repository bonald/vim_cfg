int32
ds_aclqos_if_get_aclqos_if_field(tbl_interface_t *p_db_if, ds_aclqos_if_field_id_t field_id, void* field_value)
{
    ds_aclqos_if_t *p_db_aclqos_if = NULL;

    p_db_aclqos_if = ds_aclqos_if_get_aclqos_if(p_db_if);
    if (NULL == p_db_aclqos_if)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case DS_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_shape_profile, sizeof(p_db_aclqos_if->queue_shape_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_drop_profile, sizeof(p_db_aclqos_if->queue_drop_profile));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_CLASS:
        sal_memcpy(field_value, p_db_aclqos_if->queue_class, sizeof(p_db_aclqos_if->queue_class));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_WEIGHT:
        sal_memcpy(field_value, p_db_aclqos_if->queue_weight, sizeof(p_db_aclqos_if->queue_weight));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_drop_mode, sizeof(p_db_aclqos_if->queue_drop_mode));
        break;

    case DS_ACLQOS_IF_FLD_QUEUE_ECN_ENABLE:
        sal_memcpy(field_value, p_db_aclqos_if->queue_ecn_enable, sizeof(p_db_aclqos_if->queue_ecn_enable));
        break;

    case DS_ACLQOS_IF_FLD_MAX:
        sal_memcpy(field_value, p_db_aclqos_if, sizeof(ds_aclqos_if_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}
