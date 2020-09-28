
int32
tbl_tap_group_get_truncation_rcs(tbl_tap_group_t *p_tap_grp, tbl_iter_args_t *pargs)
{
    uint32 *num = pargs->argv[0];

    if (p_tap_grp->truncation_use)
    {
        *num += 1;
    }
    return PM_E_NONE;
}

int32
tbl_tap_group_get_truncation_count()
{
    tbl_iter_args_t args;
    int32 num = 0;
    args.argv[0] = &num;
    tbl_tap_group_iterate((TBL_ITER_CB_FUNC)tbl_tap_group_get_truncation_rcs, &args);
    return num;
}

