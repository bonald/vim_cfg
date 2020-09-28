
#include "sdb.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_nexthop_define.h"

const char *sai_packet_action_strs[SAI_PACKET_ACTION_MAX] = 
{
    "drop",
    "forward",
    "copy",
    "copy_cancel",
    "trap",
    "log",
    "deny",
    "transit"
};

char*
sdb_bitmap_val2str(char *str, uint32 str_len, const uint32 *bmp, const uint32 bmp_len)
{
    uint32 offset = 0;
    uint32 start_value = 0;
    uint32 stop_value = 0;
    char tmp[256];
    uint32 used = 0;
    uint32 tmp_len = 0;

    str[0] = 0;
    for (offset = 0; offset < GLB_UINT32_BITS * bmp_len; offset++)
    {
        if (GLB_BMP_ISSET(bmp, offset))
        {
            start_value = offset;
            stop_value = start_value;
            do 
            {
                stop_value++;
            } while (GLB_BMP_ISSET(bmp, stop_value));
            stop_value--;
            offset = stop_value;
            if (start_value == stop_value)
            {
                sal_sprintf(tmp, "%d,", start_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 5);
            }
            else
            {
                if (stop_value == start_value + 1)
                    sal_sprintf(tmp, "%d,%d,", start_value, stop_value);
                else
                    sal_sprintf(tmp, "%d-%d,", start_value, stop_value);
                tmp_len = sal_strlen(tmp);
                if (used + tmp_len >= str_len)
                {
                    return str;
                }                
                used += sal_strlen(tmp);
                sal_strncat(str, tmp, 10);   
            }            
        }
    }

    /* strip ',' */
    if ((used > 0) && (str[used-1] == ','))
    {
        str[used-1] = 0;
    }
    return str;
}

char*
sdb_mac_addr_val2str(char *str, uint32 str_len, const uint8 *mac)
{
    sal_sprintf(str, "%.04hx.%.04hx.%.04hx",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]));
    return str;
}

/*
char*
prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix)
{
    uint32 len = 0;

    if (1)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET, (void*)&(prefix->u.prefix4), str, str_len);
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len, "/%u", prefix->prefixlen);
    return str;
}
*/

/* refer prefix_val2str */
char*
sdb_ipuc_val2str(char *str, uint32 str_len, const sai_unicast_route_entry_t *ipuc)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    if (SAI_IP_ADDR_FAMILY_IPV4 == ipuc->destination.addr_family)
    {
        sal_inet_ntop(AF_INET, (void*)&(ipuc->destination.addr.ip4), str, str_len);
        sal_strcat(str, CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        sal_inet_ntop(AF_INET, (void*)&(ipuc->destination.mask.ip4), (str+len), str_len);
    }
    
    len = sal_strlen(str);
    sal_sprintf(str + len, CMD_KEY_DIV_STR"0x%"PRIx64, ipuc->vr_id);
        
    return str;
}

char*
sdb_l2mc_val2str(char *str, uint32 str_len, const sai_l2mc_entry_t *l2mc)
{
    const uint8 *mac = l2mc->mac_address;

    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%u",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        l2mc->vlan_id);
    return str;
}

char*
sdb_ipmc_val2str(char *str, uint32 str_len, const sai_ipmc_entry_t *ipmc)
{
    uint32 len = 0;

    str[0] = CMD_ZERO_CHAR;
    if (SAI_IP_ADDR_FAMILY_IPV4 == ipmc->destination.addr_family)
    {
        sal_inet_ntop(AF_INET, (void*)&(ipmc->destination.addr.ip4), str, str_len);
        sal_strcat(str, CMD_KEY_DIV_STR);
        len = sal_strlen(str);
        sal_inet_ntop(AF_INET, (void*)&(ipmc->source.addr.ip4), (str+len), str_len);
    }
    
    len = sal_strlen(str);
    sal_sprintf(str + len, CMD_KEY_DIV_STR"0x%"PRIx64, ipmc->vrf_id);
	
    return str;
}

/*
char*
cdb_addr_val2str(char *str, uint32 str_len, const addr_t *addr)
{
    str[0] = CMD_ZERO_CHAR;
    if (addr->family == AF_INET) {
        sal_inet_ntop(AF_INET, (void*)&(addr->u.prefix4), str, str_len);
    } else if(addr->family == AF_INET6) {
        sal_inet_ntop(AF_INET6, (void*)&(addr->u.prefix6), str, str_len);
    }
    return str;
}
*/

/* refer cdb_addr_val2str */
char*
sdb_neighbor_val2str(char *str, uint32 str_len, const sai_neighbor_entry_t *neighbor)
{
    uint32 ipaddr[4] = {0, 0, 0, 0};
    char tmp[CMD_BUF_16];
    
    tmp[0] = CMD_ZERO_CHAR;
    str[0] = CMD_ZERO_CHAR;
    if (SAI_IP_ADDR_FAMILY_IPV4 == neighbor->ip_address.addr_family)
    {
        ipaddr[0] = sal_ntohl(neighbor->ip_address.addr.ip4);
        sal_inet_ntop(AF_INET, (void*)ipaddr, str, str_len);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == neighbor->ip_address.addr_family)
    {
        ipaddr[0] = sal_ntohl(*(uint32*)(&neighbor->ip_address.addr.ip6[12]));
        ipaddr[1] = sal_ntohl(*(uint32*)(&neighbor->ip_address.addr.ip6[8]));
        ipaddr[2] = sal_ntohl(*(uint32*)(&neighbor->ip_address.addr.ip6[4]));
        ipaddr[3] = sal_ntohl(*(uint32*)(&neighbor->ip_address.addr.ip6[0]));
        sal_inet_ntop(AF_INET6, (void*)ipaddr, str, str_len);
    }
    sal_snprintf(tmp, CMD_BUF_16, CMD_KEY_DIV_STR"0x%"PRIx64, neighbor->rif_id);
    sal_strncat(str, tmp, CMD_BUF_16);
    
    return str;
}

char*
sdb_ipaddr_val2str(char *str, uint32 str_len, const sai_ip_address_t *ip)
{
    uint32 ipaddr[4] = {0, 0, 0, 0};
    
    str[0] = CMD_ZERO_CHAR;
    if (SAI_IP_ADDR_FAMILY_IPV4 == ip->addr_family)
    {
        ipaddr[0] = sal_ntohl(ip->addr.ip4);
        sal_inet_ntop(AF_INET, (void*)ipaddr, str, str_len);
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == ip->addr_family)
    {
        ipaddr[0] = sal_ntohl(*(uint32*)(&ip->addr.ip6[12]));
        ipaddr[1] = sal_ntohl(*(uint32*)(&ip->addr.ip6[8]));
        ipaddr[2] = sal_ntohl(*(uint32*)(&ip->addr.ip6[4]));
        ipaddr[3] = sal_ntohl(*(uint32*)(&ip->addr.ip6[0]));
        sal_inet_ntop(AF_INET6, (void*)ipaddr, str, str_len);
    }
    
    return str;
}

char*
sdb_opb_val2str(char *str, uint32 str_len, const ctclib_opb_t *opb)
{
    uint32 used_len = 0;
    
    sal_snprintf(str, str_len, "desc:%s"CMD_KEY_DIV_STR"words:%u"CMD_KEY_DIV_STR"start:%u"CMD_KEY_DIV_STR"max_size:%u",
        opb->desc, opb->words, opb->start_offset, opb->max_size);
    used_len = sal_strlen(str);
    sdb_bitmap_val2str((str+used_len), (str_len-used_len), opb->data, opb->words);
    
    return str;
}

char*
sdb_object_id_val2str(char *str, uint32 str_len, const sai_object_id_t *oid)
{
    sal_snprintf(str, str_len, "0x%"PRIx64, *oid);
    
    return str;
}

SDB_MAC_RX_VAL2STR_CB_FUNC g_mac_rx_val2str_fn;
SDB_MAC_TX_VAL2STR_CB_FUNC g_mac_tx_val2str_fn;

int32
sdb_mac_rx_stats_ptr_val2str_register_fn(SDB_MAC_RX_VAL2STR_CB_FUNC fn)
{
    g_mac_rx_val2str_fn = fn;
    return 0;
}

int32
sdb_mac_tx_stats_ptr_val2str_register_fn(SDB_MAC_TX_VAL2STR_CB_FUNC fn)
{
    g_mac_tx_val2str_fn = fn;
    return 0;
}

char*
sdb_mac_rx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_rx_stats_ptr_t *rx_ptr)
{
    if (g_mac_rx_val2str_fn)
    {
        return g_mac_rx_val2str_fn(str, str_len, rx_ptr);
    }
    
    return str;
}

char*
sdb_mac_tx_stats_ptr_val2str(char *str, uint32 str_len, const sdb_mac_tx_stats_ptr_t *tx_ptr)
{
    if (g_mac_tx_val2str_fn)
    {
        return g_mac_tx_val2str_fn(str, str_len, tx_ptr);
    }
    
    return str;
}

char*
sdb_fdb_key_val2str(char *str, uint32 str_len, const fdb_key_t *fdb_key)
{
    const uint8 *mac = fdb_key->mac;

    sal_sprintf(str, "%.04hX.%.04hX.%.04hX"CMD_KEY_DIV_STR"%u",
        sal_ntoh16(((uint16*)mac)[0]),
        sal_ntoh16(((uint16*)mac)[1]), 
        sal_ntoh16(((uint16*)mac)[2]),
        fdb_key->fid);
    return str;
}

char*
sdb_addr_val2str(char *str, uint32 str_len, const addr_t *addr)
{
    str[0] = CMD_ZERO_CHAR;
    if (addr->family == AF_INET) {
        /* IPv4 */
        sal_inet_ntop(AF_INET, (void*)&(addr->u.prefix4), str, str_len);
    } else if(addr->family == AF_INET6) {
        /* IPv6 */
        sal_inet_ntop(AF_INET6, (void*)&(addr->u.prefix6), str, str_len);
    }
    return str;
}

char*
sdb_prefix_val2str(char *str, uint32 str_len, const prefix_t *prefix)
{
    uint32 len = 0;

    if (AF_INET == prefix->family)
    {
        /* IPv4 */
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET, (void*)&(prefix->u.prefix4), str, str_len);
        len = sal_strlen(str);
    }
    else if (AF_INET6 == prefix->family)
    {
        str[0] = CMD_ZERO_CHAR;
        sal_inet_ntop(AF_INET6, (void*)&(prefix->u.prefix6), str, str_len);
        len = sal_strlen(str);
    }
    
    sal_sprintf(str + len, CMD_KEY_DIV_STR"%u", prefix->prefixlen);
    return str;
}

char*
sdb_sai_acl_field_val2str(char *str, uint32 str_len, const sai_acl_field_data_t *acl_field)
{
    sal_sprintf(str, "todo");
    return str;
}

char*
sdb_sai_acl_tap_val2str(char *str, uint32 str_len, const ctc_sai_acl_tap_t *tap)
{
    sal_sprintf(str, "ctc_sai_acl_tap_t");
    return str;
}

char*
sdb_sai_acl_field_list_val2str(char *str, uint32 str_len, const ctc_sai_acl_field_list_t *list)
{
    sal_sprintf(str, "ctc_sai_acl_field_list_t");
    return str;
}

char*
sdb_sai_acl_action_list_val2str(char *str, uint32 str_len, const ctc_sai_acl_action_list_t *list)
{
    sal_sprintf(str, "ctc_sai_acl_action_list_t");
    return str;
}

char*
sdb_sai_acl_action_attrlist_val2str(char *str, uint32 str_len, const ctc_sai_acl_action_attrlist_t *list)
{
    sal_sprintf(str, "ctc_sai_acl_action_attrlist_t");
    return str;
}

char*
sdb_sai_acl_tunnel_key_val2str(char *str, uint32 str_len, const ctc_sai_acl_tunnel_key_t *key)
{
    sal_sprintf(str, "ctc_sai_acl_tunnel_key_t");
    return str;
}

char*
sdb_uint8_array_val2str(char *str, uint32 str_len, const uint8 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%u,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

char*
sdb_uint32_array_val2str(char *str, uint32 str_len, const uint32 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%u,", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

char*
sdb_uint64_array_val2str(char *str, uint32 str_len, const uint64 *array, uint32 len)
{
    char buf[CMD_BUF_256];
    uint32 length = 0;
    int32 i = 0;
    
    str[0] = CMD_ZERO_CHAR;
    for (i = 0; i < len; i++)
    {
        sal_sprintf(buf, "%"PRIu64",", array[i]);
        sal_strncat(str, buf, str_len);
    }

    length = sal_strlen(str);
    /* strip ',' */
    if ((length > 0) && (str[length-1] == ','))
    {
        str[length-1] = CMD_ZERO_CHAR;
    }    
    return str;
}

char*
sdb_reference_list_val2str(char *str, uint32 str_len, sdb_reference_list_t *ref_list)
{
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slist_t *p_list = NULL;
    stbl_rif_t *p_sdb_rif = NULL;
    stbl_nexthop_t *p_sdb_nh = NULL;
    char buf[CMD_BUF_256];
    char tmp_buf[CMD_BUF_64];
    void *p_object = NULL;
    uint32 loop = 0;
    
    p_list = ref_list->obj_list;

    if (CTCLIB_SLIST_ISEMPTY(p_list))
    {
        return "";
    }

    str[0] = CMD_ZERO_CHAR;
    
    if (SDB_NODE_TBL == ref_list->type)
    {
        CTCLIB_SLIST_LOOP(p_list, p_object, listnode)
        {
            switch (ref_list->id)
            {
            case STBL_RIF:
                p_sdb_rif = p_object;
                sal_snprintf(buf, CMD_BUF_1024, "%u,", p_sdb_rif->key.rif_id);
                sal_strcat(str, buf);
                break;
            case STBL_NEXTHOP:
                p_sdb_nh = p_object;
                sal_snprintf(buf, CMD_BUF_1024, "%s,", sdb_object_id_val2str(tmp_buf, SDB_MAX_CMD_STR_LEN, &p_sdb_nh->nh_oid));
                sal_strcat(str, buf);
                break;
            default:
                SAL_ASSERT(0);
            }
            loop++;
        }
    }
    
    return str;
}

int32
sdb_reference_list_add_object(sdb_reference_list_t *ref_list, void *p_object)
{
    ctclib_slistnode_t *listnode = NULL;

    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return SDB_E_NOT_EXIST;
    }
    
    listnode = ctclib_slistnode_insert_sort(ref_list->obj_list, p_object);
    if (NULL == listnode)
    {
        return SDB_E_FAIL;
    }
    return SDB_E_NONE;
}

int32
sdb_reference_list_del_object(sdb_reference_list_t *ref_list, void *p_object)
{
    int32 rc = SDB_E_NONE;

    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return SDB_E_NOT_EXIST;
    }
    
    rc = ctclib_slistnode_delete(ref_list->obj_list, p_object);
    if (rc < 0)
    {
        return SDB_E_FAIL;
    }
    return SDB_E_NONE;
}

void*
sdb_reference_list_lookup_object(sdb_reference_list_t *ref_list, void *p_object)
{
    if (NULL == ref_list->obj_list)
    {
        SAL_ASSERT(0);
        return NULL;
    }
    
    return ctclib_slistdata_lookup(ref_list->obj_list, p_object);
}

int32
sdb_reference_list_init(sdb_reference_list_t *ref_list, uint16 type, uint16 id, CTCLIB_SLIST_CMP_FN cmp)
{
    ref_list->type = type;
    ref_list->id = id;
    ref_list->obj_list = ctclib_slist_create(cmp, NULL);
    if (NULL == ref_list->obj_list)
    {
        return SDB_E_NO_MEMORY;
    }
    return SDB_E_NONE;
}

int32
sdb_reference_list_deinit(sdb_reference_list_t *ref_list)
{
    ref_list->type = 0;
    ref_list->id = 0;
    if (!CTCLIB_SLIST_ISEMPTY(ref_list->obj_list))
    {
        SAL_ASSERT(0);
    }
    ctclib_slist_delete(ref_list->obj_list);
    ref_list->obj_list = NULL;
    return SDB_E_NONE;
}



char*
sdb_nat_forward_val2str(char *str, uint32 str_len, const sai_nat_forward_entry_t *p_data)
{
    char ip1[64];
    char ip2[64];

    ip1[0] = 0;
    ip2[0] = 0;
    str[0] = 0;
    sal_sprintf(str, "(flags=0x%x"CMD_KEY_DIV_STR"local_ip=%s"CMD_KEY_DIV_STR"local_port=%u"CMD_KEY_DIV_STR"glb_ip=%s"CMD_KEY_DIV_STR"glb_port=%u)", 
                p_data->flags, 
                sdb_ipaddr_val2str(ip1, 64, &p_data->inside_local_ip),
                p_data->inside_local_port, 
                sdb_ipaddr_val2str(ip2, 64, &p_data->inside_global_ip),
                p_data->inside_global_port);
    return str;
}


