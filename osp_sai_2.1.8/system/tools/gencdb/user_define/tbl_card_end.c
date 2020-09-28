tbl_card_t*
tbl_card_get_default_card()
{
    tbl_card_master_t *p_master = _g_p_tbl_card_master;

    return (p_master->card_array[0]);
}
