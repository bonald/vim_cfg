
tbl_ipsg_s_mac_t*
tbl_ipsg_s_mac_get_ipsg_s_mac_by_macaddr(mac_addr_t mac)
{
    tbl_ipsg_s_mac_key_t ipsg_s_mac_key;

    sal_memset(&ipsg_s_mac_key, 0, sizeof(ipsg_s_mac_key));
    sal_memcpy(ipsg_s_mac_key.mac, mac, sizeof(mac_addr_t));
    return tbl_ipsg_s_mac_get_ipsg_s_mac(&ipsg_s_mac_key);
}

