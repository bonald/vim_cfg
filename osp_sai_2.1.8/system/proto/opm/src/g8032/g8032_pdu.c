#include "proto.h"
#include "g8032_define.h"
#include "g8032_fsm.h"
#include "g8032.h"
#include "g8032_pdu.h"
#include "gen/tbl_g8032_ring_define.h"
#include "gen/tbl_g8032_ring.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"

extern struct
{
   int32 (* func)(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t *p_raps_pdu_all, int32 request_state, int32 is_east);
   int32 next_state;
}G8032FSM[G8032_FSM_STATE_MAX][G8032_FSM_EVENT_MAX] ;

int32 
g8032_ring_pkt_get_mac(ctc_msg_t *p_msg, uint8_t *dst_mac, uint8_t *src_mac)
{
    raps_eth_hdr_t                *eth = NULL;
   
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }

    eth = (raps_eth_hdr_t*)p_msg->data;
    
    sal_memcpy(dst_mac, eth->dest_mac, sizeof(eth->dest_mac));
    sal_memcpy(src_mac, eth->src_mac, sizeof(eth->src_mac));
    return PM_E_NONE;
}

int32 
g8032_ring_pkt_get_vid(ctc_msg_t *p_msg, uint32_t *vid)
{
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }
    *vid = p_msg->u_hdr.pkt.vid;
    return PM_E_NONE;
}

int32 
g8032_ring_pkt_get_ifindex(ctc_msg_t *p_msg, uint32_t *ifindex)
{
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }
    *ifindex = p_msg->u_hdr.pkt.ifindex;
    return PM_E_NONE;
}

int32 
g8032_ring_pkt_get_raps_hdr(ctc_msg_t *p_msg, raps_pdu_hdr_t *p_raps_pdu_hdr)
{
    raps_eth_hdr_t                           *eth = NULL;
    raps_vlan_hdr_t                    *vlan = NULL;
    raps_pdu_hdr_t            *raps_pdu_hdr = NULL;
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }
    eth = (raps_eth_hdr_t*)p_msg->data;
    vlan = (raps_vlan_hdr_t*)(eth + 1);
    raps_pdu_hdr=(raps_pdu_hdr_t  *)(vlan+1);
    sal_memcpy(p_raps_pdu_hdr, raps_pdu_hdr,sizeof(raps_pdu_hdr_t));

    return PM_E_NONE;
}

int32 
g8032_ring_pkt_get_raps_pdu_all(ctc_msg_t *p_msg, raps_pdu_all_t *p_raps_pdu_all)
{
    raps_eth_hdr_t                           *eth = NULL;
    raps_vlan_hdr_t                    *vlan = NULL;
    raps_pdu_all_t            *raps_pdu_all = NULL;
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }
        if (!p_raps_pdu_all)
    {
        return FALSE;    
    }
    eth = (raps_eth_hdr_t *)p_msg->data;
    vlan = (raps_vlan_hdr_t *)(eth + 1);
    raps_pdu_all = (raps_pdu_all_t  *)(vlan+1);
    sal_memcpy(p_raps_pdu_all, raps_pdu_all, sizeof(raps_pdu_all_t));
    if (0x28 != p_raps_pdu_all->opcode)
    {
        return FALSE;
    }

    if (0x01 != (p_raps_pdu_all->mel_version & 0x07))
    {
        return FALSE;
    }
    
    return PM_E_NONE;
}

int32 
g8032_ring_pkt_get_raps_info(ctc_msg_t *p_msg, raps_info_t *p_raps_info)
{
    raps_eth_hdr_t                           *eth = NULL;
    raps_vlan_hdr_t                    *vlan = NULL;
    raps_pdu_type_t                     *pdu_type = NULL;
    raps_pdu_hdr_t           *raps_pdu_hdr = NULL; 
    raps_info_t                *raps_info = NULL;
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }
    eth = (raps_eth_hdr_t*)p_msg->data;
    vlan = (raps_vlan_hdr_t*)(eth + 1);
    pdu_type = (raps_pdu_type_t  *)(vlan+1);
    raps_pdu_hdr=(raps_pdu_hdr_t  *)(pdu_type+1);
    p_raps_info=(raps_info_t  *)(raps_pdu_hdr+1);
    sal_memcpy(p_raps_info, raps_info,sizeof(raps_info_t));
    return PM_E_NONE;
}

int32_t
g8032_ring_pkt_get_raps_status(raps_info_t * p_raps_info)
{
    uint8_t raps_status = 0; 
    int32_t status_action = 0;
    if (!p_raps_info)
    {
        return G8032_FAILURE;    
    }
    raps_status = p_raps_info->status;
    if (G8032_FLAG_ISSET(raps_status, 0x80))
    {
        status_action |= G8032_STATUS_RB_SET;
    }

    if (G8032_FLAG_ISSET(raps_status, 0x40))
    {
        status_action |= G8032_STATUS_DNF_SET;
    }

    if (G8032_FLAG_ISSET(raps_status, 0x20))
    {
        status_action |= G8032_STATUS_BPR_SET;
    }
    
    return status_action;
}


int32
g8032_ring_pkt_get_raps_state(raps_pdu_all_t *p_raps_pdu_all)
{
    uint8_t raps_state = 0; 
    uint8_t raps_status = 0; 
    
    if (!p_raps_pdu_all)
    {
        return G8032_FAILURE;    
    }
    raps_state = p_raps_pdu_all->state_sub;
    raps_status = p_raps_pdu_all->status;

    if (0xB0 == (raps_state & 0xF0))
    {
        return G8032_PRI_REQ_R_APS_SF;
    }
    else if (0x00 == (raps_state & 0xF0))
    {
        if(G8032_FLAG_ISSET(raps_status, 0x80))
        {
            return G8032_PRI_REQ_R_APS_NR_RB;
        }
        else
        {
            return G8032_PRI_REQ_R_APS_NR;
        }
    }
    else if (0xD0 == (raps_state & 0xF0))
    {
        return G8032_PRI_REQ_R_APS_FS;
    }
    else if (0x70 == (raps_state & 0xF0))
    {
        return G8032_PRI_REQ_R_APS_MS;
    }
    else if (0xE0 == (raps_state))
    {
        return G8032_PRI_REQ_R_APS_EVENT;
    }
    else
    {
        return G8032_FAILURE;
    }
    return G8032_FAILURE;
}

int32
g8032_ring_pkt_get_raps_subcode_v2(raps_info_t * p_raps_info)
{
    uint8_t raps_state = 0; 
    if (!p_raps_info)
    {
        return G8032_FAILURE;    
    }
    raps_state = p_raps_info->state_sub;
    if (0xE0 == raps_state)
    {
        return G8032_RAPS_SUB_CODE_FLUSH_FDB;
    }
    return PM_E_NONE;
    }

int32
g8032_ring_pkt_get_raps_node_id(raps_info_t * p_raps_info, uint8_t *node_id)
{
    if (!p_raps_info)
    {
        return G8032_FAILURE;
    }
    sal_memcpy(node_id, p_raps_info->node_id, 6);
    return PM_E_NONE;  
}

int32
g8032_ring_pkt_get_ring_id_by_vid(uint32_t vid, uint32_t *ring_id)
{
    tbl_g8032_ring_t *p_db_g8032_ring = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_g8032_ring_master_t *p_g8032_ring_master = NULL;

    p_g8032_ring_master= tbl_g8032_ring_get_master();
    CTCLIB_SLIST_LOOP(p_g8032_ring_master->g8032_ring_list, p_db_g8032_ring, listnode)
    {
        if (p_db_g8032_ring->control_vlan == vid)
        {
            *ring_id = p_db_g8032_ring->key.ring_id;
        }
    }
    return PM_E_NONE;
}

int32
g8032_ring_pkt_tx_raps_event(tbl_g8032_ring_t *p_g8032_ring)
{
    uint8_t **pnt =  NULL;
    uint8_t *bufptr =  NULL;
    uint32_t *size =  NULL;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH]; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0};
    uint8_t srcmac[6] = {0};
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t vlan_tag_h [2]= G8032_RING_VLAN_TAG;
    uint8_t pkt_type[2] = G8032_RING_PACKET_TYPE;
    uint8_t vlan_tag_l[2]= {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }

    if (p_g8032_ring->is_sub_ring)
    {
        return PM_E_NONE;
    }

    if (!p_g8032_ring->sub_ring_id[0])
    {
        return PM_E_NONE;
    }
    G8032_TX_DEBUG("G8032 Ring %d tx raps-event packets",p_g8032_ring->key.ring_id);
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_interface)
    {
        bufptr = buf;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8;
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_EVENT);
        G8032_ENCODE_PUTC (0x00);

        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
         return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);
        
        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    
   }

    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
   if (p_interface)
   {
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8;
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_EVENT);
        G8032_ENCODE_PUTC (0x00);
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
         return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);
        
        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    
   }
    
 return PM_E_NONE;
}

int32
g8032_ring_pkt_tx_raps_sf(raps_pdu_tx_t  *p_raps_pdu_tx)
{

    uint8_t **pnt =  NULL;
    uint8_t *bufptr =  NULL;
    uint32_t *size =  NULL;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH]; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0};
    uint8_t srcmac[6] = {0};
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t vlan_tag_h [2]= G8032_RING_VLAN_TAG;
    uint8_t pkt_type[2] = G8032_RING_PACKET_TYPE;
    uint8_t vlan_tag_l[2]= {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    uint8 bpr_data = 0;
    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_raps_pdu_tx->ring_id));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_TX_DEBUG("G8032 Ring %d tx raps-sf packets",p_g8032_ring->key.ring_id);
    
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
   if (p_interface)
   {
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_SF);
        if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
         return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);
        
        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    
   }

    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
   if (p_interface)
   {
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_SF);
       if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
         return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);
        
        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    
   }
   else if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
   {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            return PM_E_NONE;  
        }
        
        if (!p_major_g8032_ring->east_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->east_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_SF);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
             return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);

            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
        }

        if (!p_major_g8032_ring->west_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->west_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_SF);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
             return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);

            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
        }
   }

   return PM_E_NONE;

}

int32
g8032_ring_pkt_tx_raps_fs(raps_pdu_tx_t  *p_raps_pdu_tx)
{
    uint8_t         **pnt = NULL;
    uint8_t         *bufptr = NULL;
    uint32_t        *size = NULL;
    uint32_t        length = 0;
    uint32_t        buf_len = 0;
    uint8_t          buf[G8032_RAPS_LENGTH] = {0}; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0};
    uint8_t          srcmac[6] = {0};
    uint8_t          destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t        vlan_tag_h[2] = G8032_RING_VLAN_TAG;
    uint8_t        pkt_type[2] = G8032_RING_PACKET_TYPE;
    uint8_t        vlan_tag_l[2]= {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    uint8 bpr_data = 0;

    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_raps_pdu_tx->ring_id));
    
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    if (NULL == p_raps_pdu_tx)
    {
        return PM_E_NONE;
    }
    G8032_TX_DEBUG("G8032 Ring %d tx raps-fs packets",p_g8032_ring->key.ring_id);
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_interface)
    {
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_FS);
         if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 | bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 | bpr_data);/*DNF*/
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
        return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    }
    
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
    if (p_interface)
    {
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] =(uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_FS);
        if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
        return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    }
    else if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            return PM_E_NONE;  
        }

        if (!p_major_g8032_ring->east_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->east_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] =(uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_FS);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
        }

         if (!p_major_g8032_ring->west_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->west_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] =(uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_FS);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
        }
        
    }
    return PM_E_NONE;  
}


int32
g8032_ring_pkt_tx_raps_ms(raps_pdu_tx_t  *p_raps_pdu_tx)
{
    uint8_t **pnt =  NULL;
    uint8_t *bufptr =  NULL;
    uint32_t *size =  NULL;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH]; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0};
    uint8_t srcmac[6] = {0};
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t vlan_tag_h[2] = G8032_RING_VLAN_TAG;
    uint8_t pkt_type [2]= G8032_RING_PACKET_TYPE;
    uint8_t vlan_tag_l [2] = {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    uint8 bpr_data = 0;

    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_raps_pdu_tx->ring_id));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_TX_DEBUG("G8032 Ring %d tx raps-ms packets",p_g8032_ring->key.ring_id);
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_interface)
    {
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_MS);
         if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
             return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    }
   
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
    if (p_interface)
    {
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_MS);
         if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
        return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    }
    else if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            return PM_E_NONE;  
        }
        if (!p_major_g8032_ring->east_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->east_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_MS);
             if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
        }

        if (!p_major_g8032_ring->west_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->west_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_MS);
             if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
        }
        
    }
    
     return PM_E_NONE;
    
}

int32
g8032_ring_pkt_tx_raps_nr(raps_pdu_tx_t  *p_raps_pdu_tx)
{
    uint8_t **pnt =  NULL;
    uint8_t *bufptr =  NULL;
    uint32_t *size =  NULL;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH]; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0};
    uint8_t srcmac[6] = {0};
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t vlan_tag_h [2]= G8032_RING_VLAN_TAG;
    uint8_t pkt_type[2] = G8032_RING_PACKET_TYPE;
    uint8_t vlan_tag_l [2]= {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    uint8 bpr_data = 0;

    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_raps_pdu_tx->ring_id));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    G8032_TX_DEBUG("G8032 Ring %d tx raps-nr packets",p_g8032_ring->key.ring_id);
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_interface)
    {
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_NR);
        if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
    
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
             return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    }
   
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
    if (p_interface)
    {
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_NR);
        if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
        }
        
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
            return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    }
    else if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            return PM_E_NONE;  
        }
        if (!p_major_g8032_ring->east_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->east_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }

            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_NR);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
                return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
        }

        if (!p_major_g8032_ring->west_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->west_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }
            
            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_NR);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0x40 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x00 |bpr_data);/*DNF*/
            }
            
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
                return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
        }


        
        
    }
    
     return PM_E_NONE;
    
}

int32
g8032_ring_pkt_tx_raps_nr_rb(raps_pdu_tx_t  *p_raps_pdu_tx)
{
    uint8_t **pnt =  NULL;
    uint8_t *bufptr =  NULL;
    uint32_t *size =  NULL;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH] = {0}; 
    uint8_t  buf_west[G8032_RAPS_LENGTH] = {0}; 
    uint8_t srcmac[6] = {0};
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t vlan_tag_h [2]= G8032_RING_VLAN_TAG;
    uint8_t pkt_type[2]= G8032_RING_PACKET_TYPE;
    uint8_t vlan_tag_l[2] = {0};
    tbl_interface_t *p_interface = NULL;
    tbl_sys_global_t *p_sys_global = NULL;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_g8032_ring_t *p_major_g8032_ring = NULL;
    uint8 bpr_data = 0;

    p_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t *) (&p_raps_pdu_tx->ring_id));
    if (NULL == p_g8032_ring)
    {
        return PM_E_NONE;
    }
    G8032_TX_DEBUG("G8032 Ring %d tx raps-nr_rb packets",p_g8032_ring->key.ring_id);
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->east_port_ifindex);
    if (p_interface)
    {
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_NR);
         if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0xC0 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x80 |bpr_data);/*DNF*/
        }
        
    
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
             return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf, length, p_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
    }
   
    p_interface = tbl_interface_get_interface_by_ifindex(p_g8032_ring->west_port_ifindex);
    if (p_interface)
    {
            
        bufptr = buf_west;
        pnt = &bufptr;
        buf_len = G8032_RAPS_LENGTH;
        size = &buf_len;
        sal_memcpy(srcmac, &p_interface->mac_addr, 6);
        vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
        vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
        G8032_ENCODE_PUT(destmac, 6);
        G8032_ENCODE_PUT(srcmac, 6);
        G8032_ENCODE_PUT(vlan_tag_h, 2);
        G8032_ENCODE_PUT(vlan_tag_l, 2);
        G8032_ENCODE_PUT(pkt_type, 2);
        G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
        G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
        G8032_ENCODE_PUTC (G8032_INVALID_VAL);
        G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
        G8032_ENCODE_PUTC (G8032_STATE_NR);
        if(p_g8032_ring->west_port_is_blocked)
        {
            bpr_data = 0x20;
        }
        if (p_raps_pdu_tx->not_flush_fdb)
        {
            G8032_ENCODE_PUTC (0xC0 |bpr_data);/*DNF*/
        }
        else
        {
            G8032_ENCODE_PUTC (0x80 |bpr_data);/*DNF*/
        }
        
        p_sys_global = tbl_sys_global_get_sys_global();
        if (NULL == p_sys_global)
        {
        return PM_E_NONE;
        }
        G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
        G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
        length = G8032_RAPS_LENGTH - (*size);

        ipc_pkt_tx_raps(buf_west, length, p_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
    }
    else if (p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
    {
        p_major_g8032_ring = tbl_g8032_ring_get_g8032_ring((tbl_g8032_ring_key_t * )(&p_g8032_ring->g8032_major_ring_id));
        if (!p_major_g8032_ring)
        {
            return PM_E_NONE;  
        }
        if (!p_major_g8032_ring->east_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->east_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }

            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_NR);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0xC0 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x80 |bpr_data);/*DNF*/
            }
            
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->east_port_ifindex, p_g8032_ring->control_vlan);
        }

        if (!p_major_g8032_ring->west_port_is_blocked)
        {
            p_interface = tbl_interface_get_interface_by_ifindex(p_major_g8032_ring->west_port_ifindex);
            if (!p_interface)
            {
                return PM_E_NONE;
            }

            sal_memset(buf_west, 0, G8032_RAPS_LENGTH);
            bufptr = buf_west;
            pnt = &bufptr;
            buf_len = G8032_RAPS_LENGTH;
            size = &buf_len;
            sal_memcpy(srcmac, &p_interface->mac_addr, 6);
            vlan_tag_l[0] = (uint8_t)(((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff00)>>8);
            vlan_tag_l[1] = (uint8_t)((0x7000 | (p_g8032_ring->control_vlan & 0xFFF))&0xff);
            G8032_ENCODE_PUT(destmac, 6);
            G8032_ENCODE_PUT(srcmac, 6);
            G8032_ENCODE_PUT(vlan_tag_h, 2);
            G8032_ENCODE_PUT(vlan_tag_l, 2);
            G8032_ENCODE_PUT(pkt_type, 2);
            G8032_ENCODE_PUTC((p_g8032_ring->ring_level_id<<5) | G8032_RAPS_PDU_VERSION);
            G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
            G8032_ENCODE_PUTC (G8032_INVALID_VAL);
            G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
            G8032_ENCODE_PUTC (G8032_STATE_NR);
            if(p_g8032_ring->west_port_is_blocked)
            {
                bpr_data = 0x20;
            }
            if (p_raps_pdu_tx->not_flush_fdb)
            {
                G8032_ENCODE_PUTC (0xC0 |bpr_data);/*DNF*/
            }
            else
            {
                G8032_ENCODE_PUTC (0x80 |bpr_data);/*DNF*/
            }
            
            p_sys_global = tbl_sys_global_get_sys_global();
            if (NULL == p_sys_global)
            {
            return PM_E_NONE;
            }
            G8032_ENCODE_PUT(p_sys_global->route_mac, 6);
            G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
            length = G8032_RAPS_LENGTH - (*size);
            
            ipc_pkt_tx_raps(buf_west, length, p_major_g8032_ring->west_port_ifindex, p_g8032_ring->control_vlan);
        }
        
    }
    
     return PM_E_NONE;
    
}

int32
g8032_ring_pkt_check_flush_fdb(tbl_g8032_ring_t *p_g8032_ring, raps_pdu_all_t * p_raps_pdu_all, uint32 if_index, int32 request_state)
{
    tbl_g8032_ring_t *p_g8032_majorring = NULL;
    tbl_g8032_ring_key_t g8032_ring_key;
    tbl_sys_global_t *p_sys_global = NULL;
    
    uint8 bpr = 0;
    uint32 i = 0;

    p_sys_global = tbl_sys_global_get_sys_global();
    if (NULL == p_sys_global)
    {
         return PM_E_NONE;
    }
    
    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    bpr = (p_raps_pdu_all->status & 0x20)>>5;
    if (!p_g8032_ring)
    {
        return PM_E_NONE;
    }
    if (!p_g8032_ring->is_sub_ring)
    {
        /*Major ring*/
        if (request_state != G8032_PRI_REQ_R_APS_NR)
        {
            if (if_index == p_g8032_ring->east_port_ifindex)
            {
                /*Compare receive node_id & bpr with this interface*/
                if (sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                    || p_g8032_ring->east_port_rec_bpr != bpr)
                {
                    sal_memcpy(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                    p_g8032_ring->east_port_rec_bpr = bpr;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                    /*Compare receive node_id & bpr with other interface*/
                    if ((sal_memcmp(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->west_port_rec_bpr != bpr) && !(p_raps_pdu_all->status & 0x40) 
                        && sal_memcmp(p_raps_pdu_all->node_id, &p_sys_global->route_mac, 6))
                    {
                        g8032_fsm_set_aps_protection_en(p_g8032_ring);
                        g8032_fsm_flush_port_fdb(p_g8032_ring,TRUE);
                    }

                }
            }

            if (if_index == p_g8032_ring->west_port_ifindex)
            {
                if (sal_memcmp(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                    || p_g8032_ring->west_port_rec_bpr != bpr)
                {
                    sal_memcpy(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                    p_g8032_ring->west_port_rec_bpr = bpr;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                    if ((sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->east_port_rec_bpr != bpr) && !(p_raps_pdu_all->status & 0x40)
                        && sal_memcmp(p_raps_pdu_all->node_id, &p_sys_global->route_mac, 6))
                    {
                        g8032_fsm_set_aps_protection_en(p_g8032_ring);
                        g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                    }
                }
            }
        }
        else
        {
            if (if_index == p_g8032_ring->east_port_ifindex)
            {
                sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                p_g8032_ring->east_port_rec_bpr = 0;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
            }

            if (if_index == p_g8032_ring->west_port_ifindex)
            {
                sal_memset(p_g8032_ring->west_port_rec_node_id, 0, 6);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
                p_g8032_ring->west_port_rec_bpr = 0;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);
            }
        }
    }
    else if (p_g8032_ring->is_sub_ring && p_g8032_ring->g8032_major_ring_id)
    {
        /*Interconnection node*/
        sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
        if (if_index != p_g8032_ring->east_port_ifindex)
        {
            return PM_E_NONE; 
        }

        g8032_ring_key.ring_id = p_g8032_ring->g8032_major_ring_id;
        p_g8032_majorring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);

        if (!p_g8032_majorring)
        {
            return PM_E_NONE; 
        }
        
        if (request_state != G8032_PRI_REQ_R_APS_NR)
        {
            if (sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                            || p_g8032_ring->east_port_rec_bpr != bpr)
            {
                sal_memcpy(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                p_g8032_ring->east_port_rec_bpr = bpr;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                if (!(p_raps_pdu_all->status & 0x40))
                {
                    g8032_fsm_set_aps_protection_en(p_g8032_ring);
                    g8032_fsm_flush_port_fdb(p_g8032_ring,TRUE);
                }

                /*If interconnection get the change of sub-ring, it will send 3 event packets to major ring*/
                for (i = 0; i<3; i++)
                {
                    g8032_ring_pkt_tx_raps_event(p_g8032_majorring);
                }
            }
        }
        else 
        {
            sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
            p_g8032_ring->east_port_rec_bpr = 0;
            tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);  
        }
        
    }
    else if (p_g8032_ring->is_sub_ring && !p_g8032_ring->g8032_major_ring_id && p_g8032_ring->virtual_channel)
    {
        /*Sub-ring without virtual-channel*/
        if (request_state != G8032_PRI_REQ_R_APS_NR)
        {
            if (if_index == p_g8032_ring->east_port_ifindex)
            {
                if (sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                    || p_g8032_ring->east_port_rec_bpr != bpr)
                {
                    sal_memcpy(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                    p_g8032_ring->east_port_rec_bpr = bpr;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                    
                    if ((sal_memcmp(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->west_port_rec_bpr != bpr) && !(p_raps_pdu_all->status & 0x40)
                        && sal_memcmp(p_raps_pdu_all->node_id, &p_sys_global->route_mac, 6))
                    {
                        g8032_fsm_set_aps_protection_en(p_g8032_ring);
                        g8032_fsm_flush_port_fdb(p_g8032_ring,TRUE);
                    }

                }
            }

            if (if_index == p_g8032_ring->west_port_ifindex)
            {
                if (sal_memcmp(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                    || p_g8032_ring->west_port_rec_bpr != bpr)
                {
                    sal_memcpy(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                    p_g8032_ring->west_port_rec_bpr = bpr;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                    if ((sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->east_port_rec_bpr != bpr) && !(p_raps_pdu_all->status & 0x40)
                        && sal_memcmp(p_raps_pdu_all->node_id, &p_sys_global->route_mac, 6))
                    {
                        g8032_fsm_set_aps_protection_en(p_g8032_ring);
                        g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                    }
                }
            }
        }
        else
        {
            if (if_index == p_g8032_ring->east_port_ifindex)
            {
                sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                p_g8032_ring->east_port_rec_bpr = 0;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
            }

            if (if_index == p_g8032_ring->west_port_ifindex)
            {
                sal_memset(p_g8032_ring->west_port_rec_node_id, 0, 6);
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
                p_g8032_ring->west_port_rec_bpr = 0;
                tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);
            }
        }
    }
    else if (p_g8032_ring->is_sub_ring && !p_g8032_ring->g8032_major_ring_id && !p_g8032_ring->virtual_channel)
    {
        /*Sub-ring without virtual-channel*/
        if (request_state != G8032_PRI_REQ_R_APS_NR)
        {
                if (if_index == p_g8032_ring->east_port_ifindex)
                {
                    if (sal_memcmp(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->east_port_rec_bpr != bpr)
                    {
                        sal_memcpy(p_g8032_ring->east_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                        p_g8032_ring->east_port_rec_bpr = bpr;
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                        if (!(p_raps_pdu_all->status & 0x40))
                        {
                            g8032_fsm_set_aps_protection_en(p_g8032_ring);
                            g8032_fsm_flush_port_fdb(p_g8032_ring,TRUE);
                        }
                        sal_memset(p_g8032_ring->west_port_rec_node_id, 0, 6);
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
                        p_g8032_ring->west_port_rec_bpr = 0;
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);

                    }
                }

                if (if_index == p_g8032_ring->west_port_ifindex)
                {
                    if (sal_memcmp(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6)
                        || p_g8032_ring->west_port_rec_bpr != bpr)
                    {
                        sal_memcpy(p_g8032_ring->west_port_rec_node_id, p_raps_pdu_all->node_id, 6);
                        p_g8032_ring->west_port_rec_bpr = bpr;
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                        if (!(p_raps_pdu_all->status & 0x40))
                        {
                            g8032_fsm_set_aps_protection_en(p_g8032_ring);
                            g8032_fsm_flush_port_fdb(p_g8032_ring, TRUE);
                        }
                        sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                        p_g8032_ring->east_port_rec_bpr = 0;
                        tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                    }
                }
            }
            else
            {
                if (if_index == p_g8032_ring->east_port_ifindex)
                {
                    sal_memset(p_g8032_ring->east_port_rec_node_id, 0, 6);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_NODE_ID);
                    p_g8032_ring->east_port_rec_bpr = 0;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_EAST_PORT_REC_BPR);
                }

                if (if_index == p_g8032_ring->west_port_ifindex)
                {
                    sal_memset(p_g8032_ring->west_port_rec_node_id, 0, 6);
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_NODE_ID);
                    p_g8032_ring->west_port_rec_bpr = 0;
                    tbl_g8032_ring_set_g8032_ring_field(p_g8032_ring, TBL_G8032_RING_FLD_WEST_PORT_REC_BPR);
                }
            }
        }
    return PM_E_NONE; 
}


int32
g8032_ring_pkt_tx(raps_pdu_all_t * p_raps_pdu_all, uint32 ifindex, uint32 vid)
{

    uint8_t **pnt;
    uint8_t *bufptr;
    uint32_t *size;
    uint32_t length;
    uint32_t buf_len;
    uint8_t  buf[G8032_RAPS_LENGTH]; 
    uint8_t destmac[6] = G8032_RAPS_DEST_MAC;
    uint8_t destmac1[6] = {0x00,0x00,0x02,0x00,0x00,0x02};
    uint8_t vlan_tag[6] = {0};
    if (NULL == p_raps_pdu_all)
    {
      return PM_E_NONE;
    }
    
    bufptr = buf;
    pnt = &bufptr;
    buf_len = G8032_RAPS_LENGTH;
    size = &buf_len;
    G8032_ENCODE_PUT(destmac, 6);
    G8032_ENCODE_PUT(destmac1, 6);
    G8032_ENCODE_PUT(vlan_tag, 6);
    
    G8032_ENCODE_PUTC (p_raps_pdu_all->mel_version);
    G8032_ENCODE_PUTC (G8032_RAPS_OPCODE); 
    G8032_ENCODE_PUTC (G8032_INVALID_VAL);
    G8032_ENCODE_PUTC (G8032_RAPS_FIRST_TLV_OFFSET);
    G8032_ENCODE_PUTC (p_raps_pdu_all->state_sub);
    G8032_ENCODE_PUTC (p_raps_pdu_all->status);
    G8032_ENCODE_PUT(destmac, 6);
    G8032_ENCODE_PUT_EMPTY (G8032_RESERVED_PDU);
    G8032_ENCODE_PUTC (G8032_INVALID_VAL); 
    length = G8032_RAPS_LENGTH - (*size);
    ipc_pkt_tx_raps(buf, length, ifindex, vid);
    
    return PM_E_NONE; 
}


int32
g8032_ring_pkt_fun(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    raps_info_t raps_info;
    uint32_t vid = 0;
    uint32_t ring_id = 0;
    int32 request_state = 0;
    raps_pdu_all_t raps_pdu_all;
    tbl_g8032_ring_key_t g8032_ring_key;
    tbl_g8032_ring_t *p_g8032_ring = NULL;
    tbl_interface_t *p_interface = NULL;
    uint32 if_index = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_sys_global_t *p_sys_global = NULL;
    p_sys_global = tbl_sys_global_get_sys_global();
    if (!p_msg || !p_msg->data)
    {
        return PM_E_NONE;    
    }

    sal_memset(&raps_info, 0, sizeof(raps_info));
    sal_memset(&raps_pdu_all, 0, sizeof(raps_pdu_all));
    sal_memset(&g8032_ring_key, 0, sizeof(g8032_ring_key));
    g8032_ring_pkt_get_vid(p_msg, &vid);
    g8032_ring_pkt_get_ring_id_by_vid(vid, &ring_id);
    if_index = p_msg->u_hdr.pkt.ifindex;
    p_interface = tbl_interface_get_interface_by_ifindex(if_index);
    if (!p_interface)
    {
       return PM_E_NONE; 
    }

    IFNAME_ETH2FULL(p_interface->key.name, ifname_ext);
    g8032_ring_key.ring_id = ring_id;
    p_g8032_ring = tbl_g8032_ring_get_g8032_ring(&g8032_ring_key);
    if (!p_g8032_ring)
    {
        G8032_RX_DEBUG("G8032 ring %d not found", ring_id);
        return PM_E_NONE;
    }

    if (!p_g8032_ring->protection_en)
    {
        return PM_E_NONE;
    }
    
    g8032_ring_pkt_get_raps_pdu_all(p_msg, &raps_pdu_all);

    if (!sal_memcmp(p_sys_global->route_mac, raps_pdu_all.node_id, 6))
    {
        return PM_E_NONE;
    }
    
    request_state = g8032_ring_pkt_get_raps_state(&raps_pdu_all);
    if (request_state != G8032_FAILURE)
    {
        G8032_RX_DEBUG("G8032 Ring %d recieve a %s packet from interface %s", p_g8032_ring->key.ring_id,  G8032_CURR_PKT_TYPE(request_state), ifname_ext) ; 
    }
    else
    {
        return PM_E_NONE;
    }
    if ((request_state == G8032_PRI_REQ_R_APS_EVENT))
    {
        if (p_g8032_ring->is_sub_ring)
        {
            return PM_E_NONE;
        }
        /*Valid check of raps-event*/
        if (raps_pdu_all.status)
        {
            return PM_E_NONE; 
        }

        g8032_fsm_set_aps_protection_en(p_g8032_ring);
        g8032_fsm_flush_port_fdb(p_g8032_ring,TRUE);
    }
    else if (!p_g8032_ring->p_guard_while)
    {
        g8032_ring_pkt_check_flush_fdb(p_g8032_ring, &raps_pdu_all, if_index, request_state);
        G8032FSM[p_g8032_ring->current_node_state][request_state].func(p_g8032_ring, &raps_pdu_all, request_state, FALSE);
    }
   return PM_E_NONE;
}
