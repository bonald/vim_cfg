
#include "proto.h"
#include "switch.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_igsp_group_define.h"
#include "gen/tbl_igsp_group.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_vlan_mapping_table_define.h"
#include "gen/tbl_vlan_mapping_table.h"
#include "gen/tbl_vlan_mapping_entry_define.h"
#include "gen/tbl_vlan_mapping_entry.h"
#include "gen/tbl_egress_mapping_entry_define.h"
#include "gen/tbl_egress_mapping_entry.h"
#include "gen/tbl_vlan_group_define.h"
#include "gen/tbl_vlan_group.h"
#include "gen/tbl_evc_entry_define.h"
#include "gen/tbl_evc_entry.h"
#include "vlan_mapping.h"
#include "igsp_api.h"
#include "igmp_pdu.h"
#include "igsp_pdu.h"
#include "if_mgr.h"
#include "switch_api.h"


static int32
_igsp_pdu_tx_igmp(uint8 *p_pkt, uint32 pkt_len, tbl_interface_t *p_db_if, vid_t vid)
{
    INTF_CHECK_DOT1X_UNAUTH_TX_RETURN(p_db_if);
    if (!tbl_interface_is_running(p_db_if))
    {
        return PM_E_NONE;
    }
    if (switch_is_mlag_peer_link(p_db_if->ifindex))
    {
        return PM_E_NONE;
    }
    return ipc_pkt_tx_igmp(p_pkt, pkt_len, p_db_if->ifindex, vid);
}

int32
_igsp_pdu_forward_prepare_vlan_novlan(uint16 vid, uint8 *data_novlan, uint8 *data_vlan, uint32 *novlan_len, const ctc_msg_t *p_msg)
{
    int32 size_val = 0;
    uint8*  pnt = NULL;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint32 has_tag = FALSE;
    uint8*  p_data = NULL;
    uint16 ether_type = 0;
    uint32 MAC_HDR_LEN = MAC_ADDR_LEN*2;

    size_val = p_msg->data_len;
    pnt = (uint8*)p_msg->data;
    size = &size_val;
    ppnt = &pnt;

    /* L2 header */
    MSG_DECODE_GET_EMPTY(MAC_ADDR_LEN*2);
    MSG_DECODE_GETW(ether_type);
    if (GLB_ETH_P_8021Q == ether_type)
    {
        has_tag = TRUE;
        /* dont get vid from packet again to avoid replace the mapped vid */
        //MSG_DECODE_GETW(vid);
        *ppnt += 2;
        *size -= 2;
        p_data = *ppnt;
    }
    else
    {
        p_data = (*ppnt)-2;
        *size += 2;
    }
    
    if (has_tag)
    {
        sal_memcpy(data_vlan, p_msg->data, p_msg->data_len);
        data_vlan[MAC_HDR_LEN+2] = data_vlan[MAC_HDR_LEN+2] & 0xF0;
        data_vlan[MAC_HDR_LEN+2] = data_vlan[MAC_HDR_LEN+2] | ((vid) >> 8);
        data_vlan[MAC_HDR_LEN+3] = (vid) & 0xFF;
        sal_memcpy(data_novlan, p_msg->data, MAC_HDR_LEN);
        sal_memcpy(data_novlan+MAC_HDR_LEN, p_data, (*size));
        *novlan_len = p_msg->data_len - VLAN_TAG_LEN;
    }
    else
    {
        sal_memcpy(data_vlan, p_msg->data, MAC_HDR_LEN);
        data_vlan[MAC_HDR_LEN] = ((GLB_ETH_P_8021Q) >> 8) & 0xFF;
        data_vlan[MAC_HDR_LEN+1] = (GLB_ETH_P_8021Q) & 0xFF;
        data_vlan[MAC_HDR_LEN+2] = ((vid) >> 8) & 0xFF;
        data_vlan[MAC_HDR_LEN+3] = (vid) & 0xFF;
        sal_memcpy(data_vlan+MAC_HDR_LEN+VLAN_TAG_LEN, p_data, (*size));
        sal_memcpy(data_novlan, p_msg->data, p_msg->data_len);
        *novlan_len = p_msg->data_len;
    }

    return PM_E_NONE;
}

int32
igsp_pdu_forward_report(tbl_interface_t *p_src_db_if, tbl_igsp_intf_t *p_db_igsp_if, const ctc_msg_t *p_msg)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;
    uint32 novlan_len = 0;
    uint8 data_novlan[CMD_BUF_1024];
    uint8 data_vlan[CMD_BUF_1024];

    IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX forward report from mrouter port of vlan %u, source ifname %s", p_db_igsp_if->key.vid, IFNAME_ETH2FULL(p_src_db_if->key.name, ifname_ext));

    _igsp_pdu_forward_prepare_vlan_novlan(p_db_igsp_if->key.vid, data_novlan, data_vlan, &novlan_len, p_msg);

    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_mrouter_port->key.name);
            if (NULL == p_db_if)
            {
                continue;
            }
            /* For bug 47322 by kcao 2018-06-07, check mrouter interface same to source interface */
            if (p_src_db_if->ifindex == p_db_if->ifindex)
            {
                continue;
            }
            if (p_db_igsp_if->key.vid == p_db_if->brgif->pvid)
            {
                rc |= _igsp_pdu_tx_igmp(data_novlan, novlan_len, p_db_if, p_db_igsp_if->key.vid);
            }
            else
            {
                rc |= _igsp_pdu_tx_igmp(data_vlan, novlan_len+VLAN_TAG_LEN, p_db_if, p_db_igsp_if->key.vid); 
            }
        }
    }

    return rc;
}

int32
igsp_pdu_flood_query(tbl_interface_t *p_src_db_if, vid_t vid, const ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 ifindex = 0;
    tbl_vlan_key_t key;
    uint32 novlan_len = 0;
    uint8 data_novlan[CMD_BUF_1024];
    uint8 data_vlan[CMD_BUF_1024];

    key.vid = vid;
    
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        return PM_E_NONE;
    }
    
    _igsp_pdu_forward_prepare_vlan_novlan(vid, data_novlan, data_vlan, &novlan_len, p_msg);
    
    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        if (p_src_db_if->ifindex == ifindex)
        {
            continue;
        }
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_db_if)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        if (vid == p_db_if->brgif->pvid)
        {
            _igsp_pdu_tx_igmp(data_novlan, novlan_len, p_db_if, vid);
        }
        else
        {
            _igsp_pdu_tx_igmp(data_vlan, novlan_len+VLAN_TAG_LEN, p_db_if, vid); 
        }
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    return PM_E_NONE;
}

int32
igsp_pdu_rx_query_host_join(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, uint32 has_tag, const ctc_msg_t *p_msg)
{    
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    ctclib_slistnode_t *listnode = NULL;
    ds_join_group_t *p_db_group = NULL;
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    igmp_encode_info_t info;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    tbl_interface_t *p_db_mroute_if = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    uint32 is_running = 0;

    if (!p_db_glb->host_join_enable)
    {
        return PM_E_NONE;
    }
    
    if (IGMP_VERSION_1 == p_db_igsp_if->version)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP(p_db_igsp_if->join_list.obj_list, p_db_group, listnode)
    {
        /* check group address match */
        if (igmp_header->group_address)
        {
            if (igmp_header->group_address != p_db_group->key.group.s_addr)
            {
                continue;
            }
        }

        /* BUG52515 forward report from all mroute port */
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode1)
        {
            p_db_mroute_if = tbl_interface_get_interface_by_name(p_db_mrouter_port->key.name);
            if (NULL == p_db_mroute_if)
            {
                continue;
            }
            is_running = tbl_interface_is_running(p_db_mroute_if);
            if (!is_running)
            {
                continue;
            }

            sal_memset(&info, 0, sizeof(info));
            info.mac_addr = p_db_mroute_if->mac_addr;
            info.vid = 0;
            if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_mroute_if->vlan_type)
            {
                if (p_db_mroute_if->brgif->pvid != p_db_igsp_if->key.vid)
                {
                    info.vid = p_db_igsp_if->key.vid;
                }
            }
            info.src_ip_addr.s_addr = p_db_igsp_if->querier_oper_address.s_addr;
            info.group_addr.s_addr = p_db_group->key.group.s_addr;

            if (IGMP_VERSION_2 == p_db_igsp_if->version)
            {
                info.dest_ip_addr.s_addr = p_db_group->key.group.s_addr;
                pkt_len = igmp_pdu_encode_v1_v2(pkt, MSG_SIZE, IGMP_MSG_V2_MEMBERSHIP_REPORT, IGMP_VERSION_2, &info);
                if (pkt_len < 0)
                {
                    continue;
                }
            }
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX %s on interface %s vlan %u, version %u, group_address %s", "join",
                IFNAME_ETH2FULL(p_db_mroute_if->key.name, ifname_ext), p_db_igsp_if->key.vid, p_db_igsp_if->version, cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_group->key.group));

            rc |= _igsp_pdu_tx_igmp(pkt, pkt_len, p_db_mroute_if, p_db_igsp_if->key.vid);
        }
    }

    return PM_E_NONE;
}

int32
igsp_pdu_rx_query(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, uint32 has_tag, const ctc_msg_t *p_msg)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    ds_mrouter_port_t mrouter_port;
    ds_query_t query;
    ds_query_t *p_query = &query;
    uint32 max_resp_time = 0;
//    uint8 *p_tag = NULL;
//    uint32 data_len = 0;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_igsp_group_master_t *p_master = tbl_igsp_group_get_master();
    ctclib_hash_backet_t *hb;
    ctclib_hash_backet_t *next = NULL;
    tbl_igsp_group_t *p_db_group = NULL;
    uint32 i = 0;

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX query on interface %s vlan %u, type %u(%s), max_resp_time %u, group_address %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid,
        igmp_header->type, igmp_pdu_type_str(igmp_header->type), igmp_header->max_resp_time, 
        sal_inet_ntop(AF_INET, (void*)(&igmp_header->group_address), ip_str, CMD_BUF_16));

    /* 1. querier operate process */
    if (INADDR_ANY != ip_header->src_ip_addr)
    {
        /* add this for RFC 2236, page 5, during specific query period, querier-to-non-queiror transition shouldn't happen */
        if (INADDR_ANY == igmp_header->group_address)
        {
            if (IPV4_ADDR_CMP(&ip_header->src_ip_addr, &p_db_igsp_if->querier_oper_address) < 0)
            {
                igsp_api_intf_set_querier_operate(p_db_igsp_if, FALSE, ip_header->src_ip_addr);
            }
        }
    }
    else
    {
        IGSP_DEBUG(IGSP_PACKET_RX, "Ignore IGMP RX query from src_ip_addr 0.0.0.0");
        return rc;
    }

    /* add this for RFC 2236, page 5, during specific query period, querier-to-non-queiror transition shouldn't happen */
    if (INADDR_ANY == igmp_header->group_address)
    {
        if (IPV4_ADDR_CMP(&ip_header->src_ip_addr, &p_db_igsp_if->other_querier_address) == 0)
        {
            CTC_TASK_STOP_TIMER(p_db_igsp_if->t_other_querier);
            p_db_igsp_if->t_other_querier = ctc_task_add_timer(TASK_PRI_NORMAL, igsp_api_timer_other_querier_fn, 
                    p_db_igsp_if, p_db_igsp_if->other_query_interval);
        }
    }

    /* 2. add mrouter port */
    /* in igmp general query msg, if
       1) psrc should not be 0.0.0.0
       2) pdst is 224.0.0.1
       3) pgrp is 0.0.0.0, learn mrouter port
       ==> modified for BUG 47103/47113 also learn mrouter when receive group-specific query message*/
#if 0       
    if ((sal_hton32(INADDR_ALLHOSTS_GROUP) == ip_header->dest_ip_addr)
     && (INADDR_ANY != ip_header->src_ip_addr)
     && (INADDR_ANY == igmp_header->group_address))
#endif
    sal_memset(&mrouter_port, 0, sizeof(mrouter_port));
    sal_strcpy(mrouter_port.key.name, p_db_if->key.name);
    mrouter_port.type = IGMP_MROUTE_DYNAMIC;
    mrouter_port.uptime = ctc_time_boottime_sec(NULL);
    igsp_api_intf_add_dynamic_mrouter_port(p_db_igsp_if, &mrouter_port);

    /* 3. response process */
    if (p_db_glb->host_join_enable)
    {
        igsp_pdu_rx_query_host_join(p_db_if, p_db_igsp_if, ip_header, igmp_header, has_tag, p_msg);
    }
    else
    {
        max_resp_time = igmp_header->max_resp_time;
        if (0 == max_resp_time)
        {
            max_resp_time = IGMP_QUERY_RESPONSE_INTERVAL_DEF;
        }

        if (igmp_header->group_address)
        {
            sal_memset(&query, 0, sizeof(query));
            igsp_api_intf_add_query(p_query, p_db_igsp_if, igmp_header->group_address, max_resp_time);
        }
        else
        {
            ctclib_hash_t *hash = p_master->group_hash;
            for (i = 0; i < hash->size; i++)
            {
                for (hb = hash->index[i]; hb; hb = next)
                {
                    next = hb->next;
                    p_db_group = hb->data;
                    if (p_db_group->key.vid != p_db_igsp_if->key.vid)
                    {
                        continue;
                    }
                    sal_memset(&query, 0, sizeof(query));
                    igsp_api_intf_add_query(p_query, p_db_igsp_if, (uint32) p_db_group->key.ip.s_addr, max_resp_time);
                }
            }
        }

        rc = igsp_pdu_flood_query(p_db_if, p_db_igsp_if->key.vid, p_msg);
    }
    
    return rc;
}

int32
igsp_pdu_rx_v1_report(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, const ctc_msg_t *p_msg)
{
    int32 rc = PM_E_NONE;
    char  ip_str[CMD_BUF_16];
    char  ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX v1 report on interface %s vlan %u, type %u(%s), max_resp_time %u, group_address %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid,
        igmp_header->type, igmp_pdu_type_str(igmp_header->type), igmp_header->max_resp_time, 
        sal_inet_ntop(AF_INET, (void*)(&igmp_header->group_address), ip_str, CMD_BUF_16));

    PM_E_RETURN(igsp_api_intf_acl_prefix_check(p_db_if, p_db_igsp_if, igmp_header->group_address, ip_header->src_ip_addr));
    
    rc = igsp_api_group_add_dynamic(p_db_if, p_db_igsp_if, igmp_header->group_address, ip_header->src_ip_addr, TRUE, FALSE, p_msg);

    return rc;
}

int32
igsp_pdu_rx_v2_report(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, const ctc_msg_t *p_msg)
{
    int32 rc = PM_E_NONE;
    char  ip_str[CMD_BUF_16];
    char  ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX v2 report on interface %s vlan %u, type %u(%s), max_resp_time %u, group_address %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid,
        igmp_header->type, igmp_pdu_type_str(igmp_header->type), igmp_header->max_resp_time, 
        sal_inet_ntop(AF_INET, (void*)(&igmp_header->group_address), ip_str, CMD_BUF_16));

    if (ip_header->dest_ip_addr != igmp_header->group_address)
    {
        return PM_E_FAIL;
    }

    PM_E_RETURN(igsp_api_intf_acl_prefix_check(p_db_if, p_db_igsp_if, igmp_header->group_address, ip_header->src_ip_addr));

    rc = igsp_api_group_add_dynamic(p_db_if, p_db_igsp_if, igmp_header->group_address, ip_header->src_ip_addr, FALSE, FALSE, p_msg);

    return rc;
}

int32
igsp_pdu_rx_v3_report(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, uint8 *p_buf, int32 left_len, const ctc_msg_t *p_msg)
{
    igmp_v3_pdu_info_t pdu_info;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint32 i = 0;
    uint32 j = 0;
    int32   rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];

    size = &left_len;
    ppnt = &p_buf;

    if (IGMP_VERSION_1 == p_db_igsp_if->version || IGMP_VERSION_2 == p_db_igsp_if->version)
    {
        return PM_E_FAIL;
    }

    sal_memcpy(&pdu_info.header, igmp_header, sizeof(igmp_v3_header_t));
    pdu_info.header.number_of_group = sal_ntoh16(pdu_info.header.number_of_group);

    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX v3 report on interface %s vlan %u, type %u(%s), number_of_group %u",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid,
        igmp_header->type, igmp_pdu_type_str(igmp_header->type), pdu_info.header.number_of_group);

    if (left_len < IGMP_MSG_V3_REPORT_GRP_REC_MIN_SIZE * pdu_info.header.number_of_group)
    {
        return PM_E_FAIL;
    }
    
    pdu_info.group_record = XCALLOC(MEM_IGMP_RECORD, sizeof(igmp_group_record_t) * pdu_info.header.number_of_group);
    for (i = 0; i < pdu_info.header.number_of_group; i++)
    {
        MSG_DECODE_GETC(pdu_info.group_record[i].record_type);
        MSG_DECODE_GETC(pdu_info.group_record[i].aux_data_len);
        MSG_DECODE_GETW(pdu_info.group_record[i].number_of_sources);
        MSG_DECODE_GETL(pdu_info.group_record[i].multicast_address);
        pdu_info.group_record[i].multicast_address = sal_hton32(pdu_info.group_record[i].multicast_address);

        rc = igsp_api_intf_acl_prefix_check(p_db_if, p_db_igsp_if, pdu_info.group_record[i].multicast_address, ip_header->src_ip_addr);
        if (rc < 0)
        {
            goto EXIT;
        }
    
        pdu_info.group_record[i].source_address = XCALLOC(MEM_IGMP_RECORD, sizeof(uint32) * pdu_info.group_record[i].number_of_sources);
        for (j = 0; j < pdu_info.group_record[i].number_of_sources; j++)
        {
            MSG_DECODE_GETL(pdu_info.group_record[i].source_address[j]);
        }
        if (pdu_info.group_record[i].aux_data_len)
        {
            MSG_DECODE_GET_EMPTY(pdu_info.group_record[i].aux_data_len * 4);
        }
    }

#if 0
    log_sys(M_MOD_IGSP, E_ERROR, "IGMP v3 Hdr: type %u, number_of_group %u", pdu_info.header.type, pdu_info.header.number_of_group);
    for (i = 0; i < pdu_info.header.number_of_group; i++)
    {
        log_sys(M_MOD_IGSP, E_ERROR, "  group[%u] = record_type %u, aux_data_len %u, number_of_sources %u, multicast_address 0x%08x", 
            i,
            pdu_info.group_record[i].record_type,
            pdu_info.group_record[i].aux_data_len,
            pdu_info.group_record[i].number_of_sources,
            pdu_info.group_record[i].multicast_address);
        for (j = 0; j < pdu_info.group_record[i].number_of_sources; j++)
        {
            log_sys(M_MOD_IGSP, E_ERROR, "    source_address[%u] = 0x%08x", j, pdu_info.group_record[i].source_address[j]);
        }
    }
#endif

    for (i = 0; i < pdu_info.header.number_of_group; i++)
    {
        switch (pdu_info.group_record[i].record_type)
        {
        case IGMP_FMC_MODE_IS_INCLUDE:
        case IGMP_FMC_CHANGE_TO_INCLUDE_MODE:
            if (pdu_info.group_record[i].number_of_sources)
            {
                igsp_api_group_add_dynamic(p_db_if, p_db_igsp_if, pdu_info.group_record[i].multicast_address, ip_header->src_ip_addr, FALSE, TRUE, p_msg);
            }
            else
            {
                igsp_api_group_rx_leave(p_db_if, p_db_igsp_if, pdu_info.group_record[i].multicast_address, TRUE);
            }
            break;

        case IGMP_FMC_MODE_IS_EXCLUDE:
        case IGMP_FMC_CHANGE_TO_EXCLUDE_MODE:
        case IGMP_FMC_ALLOW_NEW_SOURCES:
            igsp_api_group_add_dynamic(p_db_if, p_db_igsp_if, pdu_info.group_record[i].multicast_address, ip_header->src_ip_addr, FALSE, TRUE, p_msg);
            break;
            
        case IGMP_FMC_BLOCK_OLD_SOURCES:
            igsp_api_group_rx_leave(p_db_if, p_db_igsp_if, pdu_info.group_record[i].multicast_address, TRUE);
            break;
        }
    }

    igsp_pdu_forward_report(p_db_if, p_db_igsp_if, p_msg);

EXIT:   
    /* free resource */
    for (i = 0; i < pdu_info.header.number_of_group; i++)
    {
        if (pdu_info.group_record[i].source_address)
        {
            XFREE(MEM_IGMP_RECORD, pdu_info.group_record[i].source_address);
            pdu_info.group_record[i].source_address = NULL;
        }
    }
    if (pdu_info.group_record)
    {
        XFREE(MEM_IGMP_RECORD, pdu_info.group_record);
        pdu_info.group_record = NULL;
    }    

    return rc;
}

int32
igsp_pdu_rx_v2_leave(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, 
    ip_header_t *ip_header, igmp_header_t *igmp_header, const ctc_msg_t *p_msg)
{
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX v2 leave on interface %s vlan %u, type %u(%s), max_resp_time %u, group_address %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid,
        igmp_header->type, igmp_pdu_type_str(igmp_header->type), igmp_header->max_resp_time, 
        sal_inet_ntop(AF_INET, (void*)(&igmp_header->group_address), ip_str, CMD_BUF_16));

    PM_E_RETURN(igsp_api_intf_acl_prefix_check(p_db_if, p_db_igsp_if, igmp_header->group_address, ip_header->src_ip_addr));
    rc = igsp_api_group_rx_leave(p_db_if, p_db_igsp_if, igmp_header->group_address, FALSE);

    return rc;
}

int32
igsp_pdu_decode(tbl_interface_t *p_db_if, vid_t vid, void *p_neigh,
    uint8 *p_buf, uint32 len, const ctc_msg_t *p_msg)
{
    mac_addr_t igs_addr = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    mac_addr_t macda;
    mac_addr_t macsa;
    uint32 macda_u32 = 0;
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint16  ether_type = 0;
    uint32 has_tag = FALSE;
    ip_header_t *p_ip_header_data = NULL;
    ip_header_t ip_header;
    igmp_header_t *igmp_header = NULL;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ip_str1[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    /* bypass mirror dest port */
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        return PM_E_NONE;
    }

    size = &len_left;
    ppnt = &p_buf;
    
    /* L2 header */
    MSG_DECODE_GET(macda, MAC_ADDR_LEN);     /* MACDA */
    MSG_DECODE_GET(macsa, MAC_ADDR_LEN);     /* MACSA */
    MSG_DECODE_GETW(ether_type);
    if (GLB_ETH_P_8021Q == ether_type)
    {
        MSG_DECODE_GETW(vid);
        MSG_DECODE_GETW(ether_type);
        /* ignore 802.1P and CFI */
        vid = vid & 0xFFF;
        has_tag = TRUE;
    }

    if (ETH_P_IPV4 != ether_type)
    {
        return PM_E_FAIL;
    }

    p_ip_header_data = (ip_header_t*)*ppnt;
    MSG_DECODE_GET_EMPTY(4 * p_ip_header_data->ihl);
    
    igmp_header = (igmp_header_t*)*ppnt;
    MSG_DECODE_GET_EMPTY(sizeof(igmp_header_t));

    sal_memcpy(&ip_header, p_ip_header_data, sizeof(ip_header_t));
    ip_header.total_length = sal_ntoh16(ip_header.total_length);

    /* processing vlan mapping */
    vlanmap_api_process_pdu(p_db_if, &vid);

    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }

    if (!p_db_igsp_if->enable)
    {
        return PM_E_NONE;
    }

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ip_str1, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX on interface %s vlan %u, IP Hdr: ihl %u, version %u, total_length %u, protocol %u, src_ip_addr %s, dest_ip_addr %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid,
        ip_header.ihl, ip_header.version, ip_header.total_length, ip_header.protocol, 
        sal_inet_ntop(AF_INET, (void*)(&(ip_header.src_ip_addr)), ip_str, CMD_BUF_16), 
        sal_inet_ntop(AF_INET, (void*)(&(ip_header.dest_ip_addr)), ip_str1, CMD_BUF_16));

    if (IPPROTO_IGMP != ip_header.protocol)
    {
        return PM_E_FAIL;
    }

    if (ETH_P_IPV4 == ether_type)
    {
        /* 1. check version */
        if (0 != sal_memcmp(macda, igs_addr, 3))
        {
            return PM_E_FAIL;
        }
        
        macda_u32 = ((macda[2]) << 24) | ((macda[3]) << 16) | ((macda[4]) << 8) | (macda[5]);
        /* 2. check address */
        if ((macda_u32 & 0x7FFFFF) != (sal_hton32(ip_header.dest_ip_addr) & 0x7FFFFF))
        {
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_RX, "Ignore IGMP RX on interface %s vlan %u for MAC address 0x%06x not match to IP address 0x%06x",
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid, (macda_u32 & 0x7FFFFF), (sal_hton32(ip_header.dest_ip_addr) & 0x7FFFFF));
            return PM_E_FAIL;
        }

        /* 3. check TTL */
        if (1 != ip_header.ttl)
        {
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_RX, "Ignore IGMP RX on interface %s vlan %u for TTL %u not equal to 1",
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid, ip_header.ttl);
            return PM_E_FAIL;
        }
    }
    else if (ETH_P_IPV6 == ether_type)
    {
        return PM_E_FAIL;
    }
    else
    {
        return PM_E_FAIL;
    }

    /* Do not process LAN groups */
    if ((IGMP_MSG_V3_MEMBERSHIP_REPORT != igmp_header->type) && (IGMP_MSG_MEMBERSHIP_QUERY != igmp_header->type))
    {
        if (sal_ntoh32(igmp_header->group_address) <= INADDR_MAX_LOCAL_GROUP
            || ! IN_MULTICAST (sal_ntoh32(igmp_header->group_address)))
        {
            sal_memset(ip_str, 0, CMD_BUF_16);
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_RX, "Ignore IGMP RX on interface %s vlan %u for group_address %s check",
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid, sal_inet_ntop(AF_INET, (void*)(&igmp_header->group_address), ip_str, CMD_BUF_16));
            return PM_E_FAIL;
        }
    }

    switch (igmp_header->type)
    {
    case IGMP_MSG_MEMBERSHIP_QUERY:
        rc = igsp_pdu_rx_query(p_db_if, p_db_igsp_if, &ip_header, igmp_header, has_tag, p_msg);            
        break;

/* Added by kcao 2019-04-08 hybrid not process report and leave */
#ifndef OFPRODUCT
    case IGMP_MSG_V1_MEMBERSHIP_REPORT:
        rc = igsp_pdu_rx_v1_report(p_db_if, p_db_igsp_if, &ip_header, igmp_header, p_msg);
        break;

    case IGMP_MSG_V2_MEMBERSHIP_REPORT:
        if (IGMP_VERSION_2 == p_db_igsp_if->version || IGMP_VERSION_3 == p_db_igsp_if->version)
        {
            rc = igsp_pdu_rx_v2_report(p_db_if, p_db_igsp_if, &ip_header, igmp_header, p_msg);
        }
        else
        {
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_RX, "V2 report on interface %s vlan %u configured for version 1, ignored",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid);
        }
        break;

    case IGMP_MSG_V2_LEAVE_GROUP:
        if (IGMP_VERSION_2 == p_db_igsp_if->version || IGMP_VERSION_3 == p_db_igsp_if->version)
        {
            rc = igsp_pdu_rx_v2_leave(p_db_if, p_db_igsp_if, &ip_header, igmp_header, p_msg);
        }
        else
        {
            IGSP_DEBUG(IGSP_PACKET_RX, "V2 leave on interface %s vlan %u configured for version 1, ignored",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid);
        }
        break;

    case IGMP_MSG_V3_MEMBERSHIP_REPORT:
        if (IGMP_VERSION_3 == p_db_igsp_if->version)
        {
            rc = igsp_pdu_rx_v3_report(p_db_if, p_db_igsp_if, &ip_header, igmp_header, p_buf, len_left, p_msg);
        }
        else
        {
            IGSP_DEBUG(IGSP_PACKET_RX, "V3 report on interface %s vlan %u configured for version %u, ignored",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid, p_db_igsp_if->version);
        }
        break;
#endif

    default:
        return PM_E_FAIL;
    }

    return rc;
}

int32
igsp_pdu_pim_decode(tbl_interface_t *p_db_if, vid_t vid, void *p_neigh,
    uint8 *p_buf, uint32 len, const ctc_msg_t *p_msg)
{
    mac_addr_t igs_addr = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    mac_addr_t macda;
    mac_addr_t macsa;
    ds_mrouter_port_t mrouter_port;
    uint32 macda_u32 = 0;
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint16  ether_type = 0;
    ip_header_t *p_ip_header_data = NULL;
    ip_header_t ip_header;
    pim_hello_header_t *pim_hello_header = NULL;
    pim_hello_option1_t *pim_hello_option1 = NULL;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ip_str1[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    /* bypass mirror dest port */
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        return PM_E_NONE;
    }

    size = &len_left;
    ppnt = &p_buf;
    
    /* L2 header */
    MSG_DECODE_GET(macda, MAC_ADDR_LEN);     /* MACDA */
    MSG_DECODE_GET(macsa, MAC_ADDR_LEN);     /* MACSA */
    MSG_DECODE_GETW(ether_type);
    if (GLB_ETH_P_8021Q == ether_type)
    {
        MSG_DECODE_GETW(vid);
        MSG_DECODE_GETW(ether_type);
        /* ignore 802.1P and CFI */
        vid = vid & 0xFFF;
    }

    if (ETH_P_IPV4 != ether_type)
    {
        return PM_E_FAIL;
    }

    p_ip_header_data = (ip_header_t*)*ppnt;
    MSG_DECODE_GET_EMPTY(4 * p_ip_header_data->ihl);
    
    pim_hello_header = (pim_hello_header_t*)*ppnt;
    MSG_DECODE_GET_EMPTY(sizeof(pim_hello_header_t));
    pim_hello_option1 = (pim_hello_option1_t*)*ppnt;
    MSG_DECODE_GET_EMPTY(sizeof(pim_hello_option1_t));
    sal_memcpy(&ip_header, p_ip_header_data, sizeof(ip_header_t));
    ip_header.total_length = sal_ntoh16(ip_header.total_length);

    /* processing vlan mapping */
    vlanmap_api_process_pdu(p_db_if, &vid);

    key.vid = vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }

    if (!p_db_igsp_if->enable)
    {
        return PM_E_NONE;
    }

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ip_str1, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_RX, "IGMP RX on interface %s vlan %u, IP Hdr: ihl %u, version %u, total_length %u, protocol %u, src_ip_addr %s, dest_ip_addr %s",
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid,
        ip_header.ihl, ip_header.version, ip_header.total_length, ip_header.protocol, 
        sal_inet_ntop(AF_INET, (void*)(&(ip_header.src_ip_addr)), ip_str, CMD_BUF_16), 
        sal_inet_ntop(AF_INET, (void*)(&(ip_header.dest_ip_addr)), ip_str1, CMD_BUF_16));

    if (103 != ip_header.protocol)
    {
        return PM_E_FAIL;
    }

    if (ETH_P_IPV4 == ether_type)
    {
        /* 1. check version */
        if (0 != sal_memcmp(macda, igs_addr, 3))
        {
            return PM_E_FAIL;
        }
        
        macda_u32 = ((macda[2]) << 24) | ((macda[3]) << 16) | ((macda[4]) << 8) | (macda[5]);
        /* 2. check address */
        if ((macda_u32 & 0x7FFFFF) != (sal_hton32(ip_header.dest_ip_addr) & 0x7FFFFF))
        {
            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_RX, "Ignore IGMP RX on interface %s vlan %u for MAC address 0x%06x not match to IP address 0x%06x",
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vid, (macda_u32 & 0x7FFFFF), (sal_hton32(ip_header.dest_ip_addr) & 0x7FFFFF));
            return PM_E_FAIL;
        }
    }
    else if (ETH_P_IPV6 == ether_type)
    {
        return PM_E_FAIL;
    }
    else
    {
        return PM_E_FAIL;
    }
    if ((pim_hello_header->version_type&0x0F) == IGMP_MSG_PIM_HELLO)
    {
        sal_memset(&mrouter_port, 0, sizeof(mrouter_port));
        sal_strcpy(mrouter_port.key.name, p_db_if->key.name);
        mrouter_port.type = IGMP_MROUTE_DYNAMIC;
        mrouter_port.uptime = ctc_time_boottime_sec(NULL);
        /* BUG53979 moruter received pim-hello aging time should be mrouter_aging_interval when configured */
        p_db_igsp_if->mrouter_aging_interval = (IGMP_MROUTER_PORT_TIMEOUT_DEF != p_db_igsp_if->mrouter_aging_interval) ? 
                                                p_db_igsp_if->mrouter_aging_interval : sal_hton16(pim_hello_option1->holdtime);
        igsp_api_intf_add_dynamic_mrouter_port(p_db_igsp_if, &mrouter_port);
    }

    return rc;
}


int32
igsp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;
    vid_t vid = 0;
    int32 rc = PM_E_NONE;

    ifindex = p_msg->u_hdr.pkt.ifindex;
    vid = p_msg->u_hdr.pkt.vid;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);

    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* Added by kcao for bug 38892, not process PDU on LAG member port */
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return PM_E_FAIL;
    }
    
    if (!tbl_interface_is_running(p_db_if))
    {
        return PM_E_NONE;
    }

    /*add by kcao for bug 39374, check the interface is dot1x unauthed in ingress*/
    INTF_CHECK_DOT1X_UNAUTH_RX_RETURN(p_db_if);

#ifndef _GLB_UML_SYSTEM_
    /* Added by kcao for bug38940, remove 4 bytes CRC */
    p_msg->data_len = p_msg->data_len - 4;
#endif

    rc = igsp_pdu_decode(p_db_if, vid, NULL, p_msg->data, p_msg->data_len, p_msg);
    if (PM_E_NONE == rc)
    {
//        p_db_lldp_if->rx_count++;
//        lldp_if_update_neigh(p_db_lldp_if, &neigh);
    }
    else
    {
//        p_db_lldp_if->rx_error_count++;
    }
    
    return PM_E_NONE;
}

int32
igsp_pdu_rx_pim(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;
    vid_t vid = 0;

    ifindex = p_msg->u_hdr.pkt.ifindex;
    vid = p_msg->u_hdr.pkt.vid;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);

    if (NULL == p_db_if)
    {
        return PM_E_FAIL;
    }

    /* Added by kcao for bug 38892, not process PDU on LAG member port */
    if (INTERFACE_IS_LAG_MEMBER(p_db_if))
    {
        return PM_E_FAIL;
    }
    
    if (!tbl_interface_is_running(p_db_if))
    {
        return PM_E_NONE;
    }

    /*add by kcao for bug 39374, check the interface is dot1x unauthed in ingress*/
    INTF_CHECK_DOT1X_UNAUTH_RX_RETURN(p_db_if);

#ifndef _GLB_UML_SYSTEM_
    /* Added by kcao for bug38940, remove 4 bytes CRC */
    p_msg->data_len = p_msg->data_len - 4;
#endif

    igsp_pdu_pim_decode(p_db_if, vid, NULL, p_msg->data, p_msg->data_len, p_msg);
    return PM_E_NONE;
}
/* Declaration of 32 bit Floating-point Bit mask */
#define IGMP_FLOAT32_EXPONENT_MASK              (0x7F800000UL)
#define IGMP_FLOAT32_SIGNIFICANT_MASK           (0x007FFFFFUL)
#define IGMP_FLOAT32_1SCOMPLEMENT_EXPONENT      (0x7F)
#define IGMP_FLOAT32_SIGNIFICANT_BIT_SIZE       (23)
#define IGMP_MSG_TIME_INTERVAL_MANTISSA_MASK    (0x0F)
#define IGMP_MSG_TIME_INTERVAL_MANTISSA_BITSIZE (4)
#define IGMP_MSG_TIME_INTERVAL_BASE_EXPONENT    (3)
#define IGMP_MSG_TIME_INTERVAL_EXPONENT         (0x80)
#define IGMP_MSG_TIME_INTERVAL_EXPONENT_MASK    (0x70)

/* Macro to Extract IGMPv3 defined Mantissa from 32-bit integer */
#define IGMP_GET_INT32_MANT(F_INT32)                                  \
  (((u_int8_t)(((*((u_int32_t *)(&(F_INT32)))) &                      \
                IGMP_FLOAT32_SIGNIFICANT_MASK)                        \
               >> (IGMP_FLOAT32_SIGNIFICANT_BIT_SIZE                  \
                   - IGMP_MSG_TIME_INTERVAL_MANTISSA_BITSIZE)))       \
   & IGMP_MSG_TIME_INTERVAL_MANTISSA_MASK)

/* Macro to Extract IGMPv3 defined Exponent from 32-bit integer */
#define IGMP_GET_INT32_EXPONENT(F_INT32)                              \
  (((u_int8_t)((*((u_int32_t *)(&(F_INT32))) &                        \
                IGMP_FLOAT32_EXPONENT_MASK)                           \
                >> IGMP_FLOAT32_SIGNIFICANT_BIT_SIZE)                 \
    - IGMP_FLOAT32_1SCOMPLEMENT_EXPONENT)                             \
   - IGMP_MSG_TIME_INTERVAL_MANTISSA_BITSIZE                          \
   - IGMP_MSG_TIME_INTERVAL_BASE_EXPONENT)

uint8
igsp_pdu_get_qqic(uint32 query_interval)
{
    float32 tmp_f32;
    uint32 qqic_u32 = 0;
    uint8 qqic_u8 = 0;

    qqic_u32 = query_interval;

    if (qqic_u32 < IGMP_MSG_TIME_INTERVAL_EXPONENT)
    {
        qqic_u8 = qqic_u32;
    }
    else
    {
        tmp_f32 = (float32)qqic_u32;
        qqic_u8 = IGMP_GET_INT32_MANT(tmp_f32);
        qqic_u8 |= (IGMP_GET_INT32_EXPONENT(tmp_f32)
               << IGMP_MSG_TIME_INTERVAL_MANTISSA_BITSIZE);
        qqic_u8 |= (uint8)IGMP_MSG_TIME_INTERVAL_EXPONENT;
    }
    
    return qqic_u8;
}

int32
igsp_pdu_tx_general_query(tbl_igsp_intf_t *p_db_igsp_if, uint32 max_response_time, uint32 is_tcn_query)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;
    char  ifname_ext[IFNAME_EXT_SIZE];
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    igmp_encode_info_t info;
    int32 rc = PM_E_NONE;

    vlan_key.vid = p_db_igsp_if->key.vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return PM_E_FAIL;
    }

    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_db_if)
        {
            continue;
        }
        if (SWITCH_CHECK_PKT_TX_CONDTION(p_db_if, GLB_PKT_IGMP) < 0)
        {
            continue;
        }

        info.mac_addr = p_db_if->mac_addr;
        info.vid = 0;
        if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            if (p_db_if->brgif->pvid != p_db_igsp_if->key.vid)
            {
                info.vid = p_db_igsp_if->key.vid;
            }
        }
        info.src_ip_addr.s_addr =p_db_igsp_if->querier_operate ? p_db_igsp_if->querier_oper_address.s_addr:p_db_igsp_if->other_querier_address.s_addr;
        info.dest_ip_addr.s_addr = p_db_glb->all_hosts.s_addr;
        info.group_addr.s_addr = 0;
        info.querier_max_response_time = max_response_time;
        if (IGMP_VERSION_3 == p_db_igsp_if->version)
        {
            info.v3_qrv = p_db_igsp_if->robustness_var;
            info.v3_qqic = is_tcn_query ? igsp_pdu_get_qqic(p_db_glb->tcn_query_interval) : igsp_pdu_get_qqic(p_db_igsp_if->query_interval);
            info.v3_group_number = 0;
            pkt_len = igmp_pdu_encode_v3(pkt, MSG_SIZE, IGMP_MSG_MEMBERSHIP_QUERY, &info);
        }
        else
        {
            pkt_len = igmp_pdu_encode_v1_v2(pkt, MSG_SIZE, IGMP_MSG_MEMBERSHIP_QUERY, p_db_igsp_if->version, &info);
        }
        if (pkt_len < 0)
        {
            continue;
        }
        rc |= _igsp_pdu_tx_igmp(pkt, pkt_len, p_db_if, p_db_igsp_if->key.vid);

        sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
        IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX general query on vlan %u, interface %s", p_db_igsp_if->key.vid, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);


    return rc;
}

int32
igsp_pdu_tx_group_query(tbl_interface_t *p_db_if, tbl_igsp_intf_t *p_db_igsp_if, uint32 group_address)
{
    uint32 is_running = 0;
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    igmp_encode_info_t info;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];
    
    is_running = tbl_interface_is_running(p_db_if);
    if (!is_running)
    {
        return rc;
    }

    info.mac_addr = p_db_if->mac_addr;
    info.vid = 0;
    if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
    {
        if (p_db_if->brgif->pvid != p_db_igsp_if->key.vid)
        {
            info.vid = p_db_igsp_if->key.vid;
        }
    }
    info.src_ip_addr.s_addr = p_db_igsp_if->querier_oper_address.s_addr;
    info.dest_ip_addr.s_addr = group_address;
    info.group_addr.s_addr = group_address;
    info.querier_max_response_time = p_db_igsp_if->lmqi / IGMP_ONE_SEC_MILLISECOND;
    if (IGMP_VERSION_3 == p_db_igsp_if->version)
    {
        info.v3_qrv = p_db_igsp_if->robustness_var;
        info.v3_qqic = igsp_pdu_get_qqic(p_db_igsp_if->query_interval);
        info.v3_group_number = 0;
        pkt_len = igmp_pdu_encode_v3(pkt, MSG_SIZE, IGMP_MSG_MEMBERSHIP_QUERY, &info);
    }
    else
    {
        pkt_len = igmp_pdu_encode_v1_v2(pkt, MSG_SIZE, IGMP_MSG_MEMBERSHIP_QUERY, p_db_igsp_if->version, &info);
    }
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    rc |= _igsp_pdu_tx_igmp(pkt, pkt_len, p_db_if, p_db_igsp_if->key.vid);

    sal_memset(ip_str, 0, CMD_BUF_16);
    sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
    IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX group query on interface %s vlan %u, group_address %s, max_response_time %u", 
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid, sal_inet_ntop(AF_INET, (void*)(&group_address), ip_str, CMD_BUF_16), info.querier_max_response_time);

    return rc;
}

int32
igsp_pdu_tx_group_record_leave(tbl_igsp_group_t *p_db_igsp_grp)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_igsp_intf_key_t igsp_if_key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 is_running = 0;
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    igmp_encode_info_t info;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    igsp_if_key.vid = p_db_igsp_grp->key.vid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&igsp_if_key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_FAIL;
    }

    if (IGMP_VERSION_1 == p_db_igsp_if->version)
    {
        return PM_E_NONE;
    }
    
    if (p_db_igsp_if->mrouter_list.obj_list)
    {
        CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_mrouter_port->key.name);
            if (NULL == p_db_if)
            {
                continue;
            }
            is_running = tbl_interface_is_running(p_db_if);
            if (!is_running)
            {
                continue;
            }

            info.mac_addr = p_db_if->mac_addr;
            info.vid = 0;
            if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
            {
                if (p_db_if->brgif->pvid != p_db_igsp_if->key.vid)
                {
                    info.vid = p_db_igsp_if->key.vid;
                }
            }
            info.src_ip_addr.s_addr = p_db_igsp_grp->last_reporter_address.s_addr;
            info.group_addr.s_addr = p_db_igsp_grp->key.ip.s_addr;

            if (IGMP_VERSION_2 == p_db_igsp_if->version)
            {
                info.dest_ip_addr.s_addr = p_db_glb->all_routers.s_addr;
                pkt_len = igmp_pdu_encode_v1_v2(pkt, MSG_SIZE, IGMP_MSG_V2_LEAVE_GROUP, IGMP_VERSION_2, &info);
                if (pkt_len < 0)
                {
                    continue;
                }
            }
            else if (IGMP_VERSION_3 == p_db_igsp_if->version)
            {
                info.dest_ip_addr.s_addr = p_db_glb->v3_routers.s_addr;
                info.v3_group_number = 1;
                info.v3_group_record[0].record_type = IGMP_FMC_CHANGE_TO_INCLUDE_MODE;
                info.v3_group_record[0].aux_data_len = 0;
                info.v3_group_record[0].number_of_sources = 0;
                info.v3_group_record[0].multicast_address = p_db_igsp_grp->key.ip.s_addr;
                info.v3_group_record[0].source_address = NULL;
                pkt_len = igmp_pdu_encode_v3(pkt, MSG_SIZE, IGMP_MSG_V3_MEMBERSHIP_REPORT, &info);
                if (pkt_len < 0)
                {
                    continue;
                }
            }

            sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
            IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX leave on interface %s vlan %u, version %u, group_address %s", 
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid, p_db_igsp_if->version, cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_igsp_grp->key.ip));

            rc |= _igsp_pdu_tx_igmp(pkt, pkt_len, p_db_if, p_db_igsp_if->key.vid);
        }
    }

    return PM_E_NONE;
}

int32
igsp_pdu_host_tx_report(tbl_igsp_intf_t *p_db_igsp_if, ds_join_group_t *p_db_group, uint32 is_join)
{
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    tbl_interface_t *p_db_if = NULL;
    ds_mrouter_port_t *p_db_mrouter_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 is_running = 0;
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    uint8 type = 0;
    igmp_encode_info_t info;
    int32 rc = PM_E_NONE;
    char ip_str[CMD_BUF_16];
    char ifname_ext[IFNAME_EXT_SIZE];

    if (!p_db_glb->host_join_enable)
    {
        return PM_E_NONE;
    }
    
    if (IGMP_VERSION_1 == p_db_igsp_if->version)
    {
        return PM_E_NONE;
    }
    
    if (NULL == p_db_igsp_if->mrouter_list.obj_list)
    {
        return PM_E_NONE;
    }
    
    CTCLIB_SLIST_LOOP(p_db_igsp_if->mrouter_list.obj_list, p_db_mrouter_port, listnode)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_mrouter_port->key.name);
        if (NULL == p_db_if)
        {
            continue;
        }
        is_running = tbl_interface_is_running(p_db_if);
        if (!is_running)
        {
            continue;
        }

        info.mac_addr = p_db_if->mac_addr;
        info.vid = 0;
        if (GLB_VLAN_PORT_TYPE_TRUNK == p_db_if->vlan_type)
        {
            if (p_db_if->brgif->pvid != p_db_igsp_if->key.vid)
            {
                info.vid = p_db_igsp_if->key.vid;
            }
        }
        info.src_ip_addr.s_addr = p_db_igsp_if->querier_oper_address.s_addr;
        info.group_addr.s_addr = p_db_group->key.group.s_addr;

        type = is_join ? IGMP_MSG_V2_MEMBERSHIP_REPORT : IGMP_MSG_V2_LEAVE_GROUP;
        if (IGMP_VERSION_2 == p_db_igsp_if->version)
        {
            info.dest_ip_addr.s_addr = is_join ? p_db_group->key.group.s_addr : p_db_glb->all_routers.s_addr;
            pkt_len = igmp_pdu_encode_v1_v2(pkt, MSG_SIZE, type, IGMP_VERSION_2, &info);
            if (pkt_len < 0)
            {
                continue;
            }
        }
        sal_memset(ifname_ext, 0, IFNAME_EXT_SIZE);
        IGSP_DEBUG(IGSP_PACKET_TX, "IGMP TX %s on interface %s vlan %u, version %u, group_address %s", is_join ? "join" : "leave",
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_igsp_if->key.vid, p_db_igsp_if->version, cdb_addr_ipv4_val2str(ip_str, CMD_BUF_16, &p_db_group->key.group));

        rc |= _igsp_pdu_tx_igmp(pkt, pkt_len, p_db_if, p_db_igsp_if->key.vid);
    }

    return PM_E_NONE;
}

