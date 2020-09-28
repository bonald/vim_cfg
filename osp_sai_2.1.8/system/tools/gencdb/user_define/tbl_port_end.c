tbl_port_t*
tbl_port_get_port_by_slot_port(uint8 slot_no, uint8 port_no, uint8 sub_port_no)
{
    tbl_port_master_t *p_master = _g_p_tbl_port_master;
    tbl_port_t *p_db_port = NULL;
    uint32 port_idx = 0;

    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (p_db_port->slot_no == slot_no
         && p_db_port->panel_port_no == port_no
         && p_db_port->panel_sub_port_no == sub_port_no)
        {
            return p_db_port;
        }
    }

    return NULL;
}
