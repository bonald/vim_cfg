int32
tbl_acl_config_get_acl_config_count()
{
    tbl_acl_config_master_t *p_master = _g_p_tbl_acl_config_master;

    return p_master->acl_config_hash->count;
}

int32
tbl_acl_config_get_l4port_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_config->l4_port_ref * (p_acl_config->intf_ref_in + p_acl_config->intf_ref_out);
    return PM_E_NONE;
}

int32
tbl_acl_config_get_l4port_config_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_acl_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_config_get_l4port_one, &args);
    return num;
}

int32
tbl_acl_config_get_tcp_flags_one(tbl_acl_config_t *p_acl_config, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_config->tcp_flags_ref * (p_acl_config->intf_ref_in + p_acl_config->intf_ref_out);
    return PM_E_NONE;
}

int32
tbl_acl_config_get_tcp_flags_config_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_acl_config_iterate((TBL_ITER_CB_FUNC)tbl_acl_config_get_tcp_flags_one, &args);
    return num;
}


