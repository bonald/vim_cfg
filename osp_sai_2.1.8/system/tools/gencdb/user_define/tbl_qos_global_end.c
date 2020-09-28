bool
tbl_qos_global_get_phb_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->phb_enable;
}

bool
tbl_qos_global_get_qos_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->qos_enable;
}

bool
tbl_qos_global_get_port_policer_first_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->port_policer_first_enable ;
}

bool
tbl_qos_global_get_policer_stats_enable()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->policer_stats_enable;
}

int32
tbl_qos_global_get_def_cpu_rate()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->def_cpu_rate;
}

int32
tbl_qos_global_get_cur_cpu_rate()
{
    tbl_qos_global_t *p_db_qos_glb = _g_p_tbl_qos_global;
    return p_db_qos_glb->cur_cpu_rate;
}
