int32
tbl_openflow_tunnel_local_ip_cnt_get_openflow_tunnel_local_ip_cnt_count()
{
    tbl_openflow_tunnel_local_ip_cnt_master_t *p_master = _g_p_tbl_openflow_tunnel_local_ip_cnt_master;

    return p_master->openflow_tunnel_local_ip_hash->count;
}

