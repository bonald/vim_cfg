
int32
route_if_addr_exists(addr_t *p_addr)
{
    tbl_route_if_master_t *p_master = tbl_route_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    tbl_route_if_t *p_db_rtif = NULL;
    ds_connected_t *p_db_conn = NULL;

    if (AF_INET == p_addr->family)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_rtif, listnode)
        {
            CTCLIB_SLIST_LOOP(p_db_rtif->ifc_ipv4.obj_list, p_db_conn, listnode1)
            {
                if ((p_db_conn->key.address.family == p_addr->family)
                 && (p_db_conn->key.address.u.prefix4.s_addr == p_addr->u.prefix4.s_addr))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}
