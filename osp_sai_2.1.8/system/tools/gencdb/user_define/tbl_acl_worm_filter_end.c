tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_name(char *name)
{
    tbl_acl_worm_filter_key_t acl_worm_filter_key;

    sal_memset(&acl_worm_filter_key, 0, sizeof(acl_worm_filter_key));
    sal_strcpy(acl_worm_filter_key.worm_filter_name, name);
    return tbl_acl_worm_filter_get_acl_worm_filter(&acl_worm_filter_key);
}


int32
_acl_worm_filter_seq_check(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    tbl_acl_worm_filter_t *pst_new_worm_filter = pargs->argv[0];

    if (pst_new_worm_filter->seq_num == p_acl_worm_filter->seq_num) 
    {
        pargs->argv[1] = p_acl_worm_filter;
    }

    return PM_E_NONE;
}

tbl_acl_worm_filter_t*
tbl_acl_worm_filter_get_by_seq(uint32 seq_num)
{
    tbl_iter_args_t args;
    tbl_acl_worm_filter_t *pst_same_worm_filter = NULL;
    tbl_acl_worm_filter_t acl_worm_fitler;

    sal_memset(&acl_worm_fitler, 0, sizeof(acl_worm_fitler));
    sal_memset(&args, 0, sizeof(args));
    acl_worm_fitler.seq_num = seq_num;

    args.argv[0] = &acl_worm_fitler;
    args.argv[1] = pst_same_worm_filter;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)_acl_worm_filter_seq_check, &args);
    return args.argv[1];
}


int32
tbl_acl_worm_filter_get_range_one(tbl_acl_worm_filter_t *p_acl_worm_filter, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];
    
    *num += p_acl_worm_filter->isrange ? 1 : 0;
    return PM_E_NONE;
}

int32
tbl_acl_worm_filter_get_range_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    
    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = &num;
    tbl_acl_worm_filter_iterate((TBL_ITER_CB_FUNC)tbl_acl_worm_filter_get_range_one, &args);
    return num;
}




