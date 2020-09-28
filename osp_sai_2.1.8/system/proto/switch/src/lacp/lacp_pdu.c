
#include "proto.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "switch.h"
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp_sm.h"
#include "lacp.h"

mac_addr_t lacp_mac_address = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};

int32
lacp_pdu_encode_lacp(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if)
{
    tbl_lag_global_t* p_lag_global = tbl_lag_global_get_lag_global();
    ds_lacp_t *p_lacp = p_db_if->lacp;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    
    size = &len_left;
    ppnt = &p_buf;

    /* L2 header */
    MSG_ENCODE_PUT (lacp_mac_address, MAC_ADDR_LEN);
    MSG_ENCODE_PUT (p_db_if->hw_mac_addr, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(ETH_P_SLOW_PROTOCOL);
    
    MSG_ENCODE_PUTC(LACP_SUBTYPE);
    MSG_ENCODE_PUTC(LACP_VERSION);
    MSG_ENCODE_PUTC(LACP_ACTOR_INFORMATION_TLV);
    MSG_ENCODE_PUTC(LACP_ACTOR_INFORMATION_LENGTH);

#ifdef CONFIG_MLAG
    /* MLAG modification */
    if (p_lacp->mlag_id)
    {
        MSG_ENCODE_PUTW(p_lag_global->mlag_system_priority);
        MSG_ENCODE_PUT(p_lag_global->mlag_system_id, MAC_ADDR_LEN);
    }
    else
#endif /* !CONFIG_MLAG */
    {
        MSG_ENCODE_PUTW(p_lag_global->system_priority);
        MSG_ENCODE_PUT(p_lag_global->system_id, MAC_ADDR_LEN);
    }
    MSG_ENCODE_PUTW(p_lacp->actor_oper_port_key);

    MSG_ENCODE_PUTW(p_lacp->actor_port_priority);
    MSG_ENCODE_PUTW(p_lacp->actor_port_number);
    MSG_ENCODE_PUTC(p_lacp->actor_oper_port_state);
    MSG_ENCODE_PUT_EMPTY(3);

    MSG_ENCODE_PUTC(LACP_PARTNER_INFORMATION_TLV);
    MSG_ENCODE_PUTC(LACP_PARTNER_INFORMATION_LENGTH);
    MSG_ENCODE_PUTW(p_lacp->partner_oper_system_priority);
    MSG_ENCODE_PUT (p_lacp->partner_oper_system, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(p_lacp->partner_oper_key);
    MSG_ENCODE_PUTW(p_lacp->partner_oper_port_priority);
    MSG_ENCODE_PUTW(p_lacp->partner_oper_port_number);
    MSG_ENCODE_PUTC(p_lacp->partner_oper_port_state);
    MSG_ENCODE_PUT_EMPTY(3);

    MSG_ENCODE_PUTC(LACP_COLLECTOR_INFORMATION_TLV);
    MSG_ENCODE_PUTC(LACP_COLLECTOR_INFORMATION_LENGTH);
    MSG_ENCODE_PUTW(LACP_COLLECTOR_MAX_DELAY);
    MSG_ENCODE_PUT_EMPTY(12);
    
    MSG_ENCODE_PUTC(LACP_TERMINATOR_TLV);
    MSG_ENCODE_PUTC(LACP_TERMINATOR_LENGTH);
    MSG_ENCODE_PUT_EMPTY(50);

    return (len - len_left);
}

int32
lacp_pdu_decode_lacp(uint8 *p_buf, uint32 len, lacp_pdu_t *p_pdu)
{
    lacp_info_t *p_actor = &p_pdu->actor;
    lacp_info_t *p_partner = &p_pdu->partner;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    
    size = &len_left;
    ppnt = &p_buf;

    /* bypass L2 header */
    MSG_DECODE_GET_EMPTY(ETH_HEADER_LEN); 

    if (LACP_SUBTYPE != p_buf[0])
    {
        return PM_E_FAIL;
    }
    
    MSG_DECODE_GETC(p_pdu->subtype);
    MSG_DECODE_GETC(p_pdu->version_number);
    MSG_DECODE_GET_EMPTY(2);

    /* Actor information */
    MSG_DECODE_GETW(p_actor->system_priority);
    MSG_DECODE_GET(p_actor->system, MAC_ADDR_LEN);
    MSG_DECODE_GETW(p_actor->key);
    MSG_DECODE_GETW(p_actor->port_priority);
    MSG_DECODE_GETW(p_actor->port);
    MSG_DECODE_GETC(p_actor->state);
    MSG_DECODE_GET_EMPTY(5);
    
    /* Partner information */
    MSG_DECODE_GETW(p_partner->system_priority);
    MSG_DECODE_GET(p_partner->system, MAC_ADDR_LEN);
    MSG_DECODE_GETW(p_partner->key);
    MSG_DECODE_GETW(p_partner->port_priority);
    MSG_DECODE_GETW(p_partner->port);
    MSG_DECODE_GETC(p_partner->state);
    MSG_DECODE_GET_EMPTY(5);

    /* Collector information */
    MSG_DECODE_GETW(p_pdu->collector_max_delay);

    return PM_E_NONE;
}

int32
lacp_pdu_tx_lacp(tbl_interface_t *p_db_if)
{
    ds_lacp_t *p_db_lacp = p_db_if->lacp;
    uint8 pkt[LACP_PDU_PKT_LEN];
    int32 pkt_len = 0;
    int32 rc = PM_E_NONE;
    
    pkt_len = lacp_pdu_encode_lacp(pkt, LACP_PDU_PKT_LEN, p_db_if);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_lacp(pkt, pkt_len, p_db_if->ifindex));
    
    LACP_DEBUG(LACP_PACKET, "Interface %s TX LACPDU actor_state: %02X, partner_state: %02X", 
        p_db_if->key.name, p_db_lacp->actor_oper_port_state, p_db_lacp->partner_oper_port_state);

    return PM_E_NONE;
}

int32
lacp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    ds_lacp_t *p_ds_lacp = NULL;
    lacp_pdu_t *p_pdu = NULL;
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;
    
    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if || !p_db_if->lacp)
    {
        return PM_E_FAIL;
    }

    p_ds_lacp = p_db_if->lacp;
    if (NULL == p_ds_lacp->pdu)
    {
        p_pdu = XCALLOC(MEM_LACP_PDU, sizeof(lacp_pdu_t));
        if (NULL == p_pdu)
        {
            return PM_E_NO_MEMORY;
        }
        p_ds_lacp->pdu = p_pdu;
    }
    else
    {
        p_pdu = p_ds_lacp->pdu;
    }

    PM_E_RETURN(lacp_pdu_decode_lacp(p_msg->data, p_msg->data_len, p_pdu));

    if (LACP_SUBTYPE == p_pdu->subtype)
    {
        LACP_DEBUG(LACP_PACKET, "Interface %s RX LACPDU, actor_state: %02X, partner_state: %02X", 
            p_db_if->key.name, p_pdu->actor.state, p_pdu->partner.state);

        p_ds_lacp->rx_lacp_count++;
        PM_E_RETURN(lacp_sm_receive_process_event(p_db_if, RX_EVENT_PDU_RX));
    }
    else
    {
        p_ds_lacp->rx_error_count++;
    }

    return PM_E_NONE;
}

