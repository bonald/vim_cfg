int32
tbl_snmp_context_num_get()
{
    tbl_snmp_context_master_t *p_master = _g_p_tbl_snmp_context_master;
    int32 count = 0;

    if (NULL == p_master)
    {
        return 0;
    }

    count = p_master->snmp_context_list->count;
    return count;
}


