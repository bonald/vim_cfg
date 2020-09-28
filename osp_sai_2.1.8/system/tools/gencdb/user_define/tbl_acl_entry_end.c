int32
tbl_acl_entry_get_acl_entry_count()
{
    tbl_acl_entry_master_t *p_master = _g_p_tbl_acl_entry_master;

    return p_master->acl_entry_hash->count;
}


int32
tbl_acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    uint32 *num_ingress = pargs->argv[0];
    uint32 *num_egress = pargs->argv[1];
    uint32 *num_global_ingress = pargs->argv[2];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_ingress += 2;
        }
        else
        {
            *num_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_egress += 2;
        }
        else
        {
            *num_egress += 1;
        }
    }
    if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            *num_global_ingress += 2;
        }
        else
        {
            *num_global_ingress += 1;
        }
    }
    return PM_E_NONE;
}

int32
tbl_acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_worm_filter)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = cnt_ingress;
    args.argv[1] = cnt_egress;
    args.argv[2] = cnt_worm_filter;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)tbl_acl_entry_get_count, &args);
    return PM_E_NONE;
}

int32
tbl_acl_entry_get_count_1(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs)
{
    acl_entry_count_t *entry_count = pargs->argv[0];

    if (p_acl_entry->in_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_ingress6 += 1;
        }
        else
        {
            entry_count->cur_ingress += 1;
        }     
    }
    if (p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_egress6 += 1;
        }
        else
        {
            entry_count->cur_egress += 1;
        }
    }

	if (0 == p_acl_entry->in_port && 0 == p_acl_entry->out_port)
    {
        if (p_acl_entry->is_ipv6 || p_acl_entry->inner_is_ipv6)
        {
            entry_count->cur_worm_filter6 += 1;
        }
        else
        {
            entry_count->cur_worm_filter += 1;
        }
    }

    return PM_E_NONE;
}


int32
tbl_acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count)
{
    tbl_iter_args_t args;
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = entry_count;
    tbl_acl_entry_iterate((TBL_ITER_CB_FUNC)tbl_acl_entry_get_count_1, &args);
    return PM_E_NONE;
}

