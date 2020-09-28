int32
tbl_snmp_target_params_num_get()
{
    tbl_snmp_target_params_master_t *p_master = _g_p_tbl_snmp_target_params_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_target_params_list->count;
    return count;
}


