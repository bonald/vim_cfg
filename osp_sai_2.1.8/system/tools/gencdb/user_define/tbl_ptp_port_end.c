tbl_ptp_port_t*
tbl_ptp_port_get_ptp_port_by_name(char* ifname)
{
    tbl_ptp_port_key_t ptp_port_key;

    sal_memset(&ptp_port_key, 0, sizeof(ptp_port_key));
    sal_memcpy(ptp_port_key.name, ifname, IFNAME_SIZE);
    
    return tbl_ptp_port_get_ptp_port(&ptp_port_key);
}