#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ssm_global_define.h"
#include "gen/tbl_ssm_global.h"
#include "gen/tbl_ssm_port_define.h"
#include "gen/tbl_ssm_port.h"
#include "ssm.h"
#include "ssm_api.h"
#include "ssm_opts.h"
#include "ctc_task.h"
#include "ssm_pdu.h"
#include "ssm_fsm.h"
#include "ssm_desc.h"

static mac_addr_t ssm_dst_address      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};

static bool
_ssm_pdu_check_dnu_group(uint32 group_id)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    
    if (0 == p_ssm_glb->select_ifindex)
    {
        return FALSE;
    }
    
    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && (group_id == p_db_ssm_if->dnu_group_id) && 
            (p_ssm_glb->select_ifindex == p_db_ssm_if->ifindex))
        {
            return TRUE;
        }  
    }
    return FALSE;
}

tbl_ssm_port_t*
ssm_pdu_get_ssm_if(uint32 ifindex)
{
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && (ifindex == p_db_ssm_if->ifindex))
        {
            return p_db_ssm_if;
        }  
    }
    return NULL;
}

int32
ssm_pdu_encode(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if, tbl_ssm_port_t * p_db_ssm_if, bool event_pdu)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t * p_ssm_if = NULL;
    char ifname_ext[IFNAME_SIZE];
    char clock_id_str[SSM_CLOCKID_STR_LEN];
    uint32 device_quality = 0;
    uint32 extend_hop = 0;
    ssm_clock_id_t  clock_id;
    int32  len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;

    size = &len_left;
    ppnt = &p_buf;

    /*ESMC dest_mac and source_mac*/
    SSM_ENCODE_PUT(ssm_dst_address, MAC_ADDR_LEN);
    SSM_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);

    /*Slow Protocol Ethertype*/
    SSM_ENCODE_PUTW(SFLOW_PRO_ETHERTYPE);

    /*Slow Protocol Subtype*/
    SSM_ENCODE_PUTC(SFLOW_PRO_SUBTYPE);

    /*ITU OUT*/
    SSM_ENCODE_PUTC(SSM_OUI_VALUE1);
    SSM_ENCODE_PUTC(SSM_OUI_VALUE2);
    SSM_ENCODE_PUTC(SSM_OUI_VALUE3);

    /*ITU Subtype*/
    SSM_ENCODE_PUTW(SSM_ITU_SUBTYPE);

    /*version and Event flag*/
    if (event_pdu)
    {
        SSM_ENCODE_PUTC(SSM_EVENT_PDU);
    }
    else
    {
        SSM_ENCODE_PUTC(SSM_INFORMATION_PDU);
    }
    
    /*reverted*/
    SSM_ENCODE_PUT_EMPTY(3);

    /*SSM TLV*/
    SSM_ENCODE_PUTC(SSM_TLV_TYPE);
    SSM_ENCODE_PUTW(SSM_TLV_LEN);
    
    /*device quality*/
    device_quality = p_ssm_glb->current_quality;
    if (SSM_QL_INV3 != p_db_ssm_if->force_tx_quality)
    {
        device_quality = p_db_ssm_if->force_tx_quality;
    }
    if (p_ssm_glb->select_ifindex == p_db_ssm_if->ifindex)
    {
        device_quality = SSM_QL_DNU;
    }
    if ((p_db_ssm_if->dnu_group_id != SSM_DEFAULT_DNU_GROUP) && _ssm_pdu_check_dnu_group(p_db_ssm_if->dnu_group_id))
    {
        device_quality = SSM_QL_DNU;
    }
    SSM_ENCODE_PUTC(device_quality);
    p_db_ssm_if->send_quality = device_quality;
    /*end device quality*/

    sal_memset(clock_id, 0, sizeof(clock_id)); /*Modified by yejl for 46455, 2018-02-27*/
    if (p_ssm_glb->ssm_extend_enable)
    {
        SSM_ENCODE_PUTC(SSM_TLV_EX_TYPE);
        SSM_ENCODE_PUTW(SSM_TLV_EX_LEN);
        if (p_ssm_glb->select_ifindex)
        {
            p_ssm_if = ssm_get_tbl_ssm_by_ifindex(p_ssm_glb->select_ifindex);
            if (p_ssm_if && p_ssm_if->replace_clockid_en)
            {
                SSM_ENCODE_PUT(p_ssm_glb->device_clock_id, SSM_CLOCK_ID_LEN);
                SSM_ENCODE_PUTC(p_ssm_if->hop + 1);
                sal_memcpy(clock_id, p_ssm_glb->device_clock_id, SSM_CLOCK_ID_LEN);
                extend_hop = p_ssm_if->hop + 1;
            }
            else if (p_ssm_if && !p_ssm_if->replace_clockid_en)
            {
                SSM_ENCODE_PUT(p_ssm_if->receive_clock_id, SSM_CLOCK_ID_LEN);
                sal_memcpy(clock_id, p_ssm_if->receive_clock_id, SSM_CLOCK_ID_LEN);
                SSM_ENCODE_PUTC(p_ssm_if->hop + 1);
                extend_hop = p_ssm_if->hop + 1;
            }
        }
        else
        {
            SSM_ENCODE_PUT(p_ssm_glb->device_clock_id, SSM_CLOCK_ID_LEN);
            SSM_ENCODE_PUTC(0);
            sal_memcpy(clock_id, p_ssm_glb->device_clock_id, SSM_CLOCK_ID_LEN);
            extend_hop = 0;
        }
        SSM_ENCODE_PUT_EMPTY(20);
    }
    else
    {
        SSM_ENCODE_PUT_EMPTY(32);
    }

    /*dump the debug information*/
    SSM_TXPKT_DEBUG("TX Dump: port: %s, pdu: %s, quality: %s, ssm extend: %s, clockid: %s, hop: %d", 
                    IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext),
                    event_pdu ? "Event PDU" : "Information PDU", 
                    ssm_desc_quality(device_quality, 1), 
                    ssm_desc_enable(p_ssm_glb->ssm_extend_enable), 
                    ssm_desc_clockid(clock_id_str, clock_id), 
                    extend_hop);
    
    return (len - len_left);
}

int32
ssm_pdu_decode(tbl_ssm_port_t *p_db_ssm_if, uint8 *p_buf, uint32 len)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    char clock_id_str[SSM_CLOCKID_STR_LEN];
    char ifname_ext[IFNAME_SIZE];
    mac_addr_t ssm_dst_mac;
    ssm_clock_id_t device_clock_id;
    uint32 version_flag = 0;
    uint32 receive_quality = 0;
    uint32 ssm_extend_type = 0;
    uint32 hop = 0;
    int32 rc = PM_E_NONE;
        
    size = &len_left;
    ppnt = &p_buf;

    SSM_DECODE_GET(ssm_dst_mac, MAC_ADDR_LEN); /*dest mac*/
    if (sal_memcmp(ssm_dst_mac, ssm_dst_address, MAC_ADDR_LEN) != 0)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail on port %s bacause dest mac %x%x%x%x%x%x is wrong!\n",
        IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext),
        ssm_dst_mac[0], ssm_dst_mac[1], ssm_dst_mac[2], ssm_dst_mac[3], ssm_dst_mac[4], ssm_dst_mac[5]);
        return PM_E_FAIL;
    }
    SSM_DECODE_GET_EMPTY(MAC_ADDR_LEN); /*source mac*/
    SSM_DECODE_GET_EMPTY(2);            /*Slow Protocol Ethertype:*/
    SSM_DECODE_GET_EMPTY(1);            /*Slow Protocol Subtype*/
    SSM_DECODE_GET_EMPTY(3);            /*ITU OUT*/
    SSM_DECODE_GET_EMPTY(2);            /*ITU Subtype*/
    
    /*version and Event flag*/
    SSM_DECODE_GETC(version_flag);
    if (SSM_EVENT_PDU != version_flag && SSM_INFORMATION_PDU != version_flag)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail on port %s bacause version and flag %d is wrong\n", 
        IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext), version_flag);
        return PM_E_FAIL;
    }

    SSM_DECODE_GET_EMPTY(3); /*reverted*/
    SSM_DECODE_GET_EMPTY(1); /*QL TLV type*/
    SSM_DECODE_GET_EMPTY(2); /*QL TLV length*/

    /*device quality*/
    SSM_DECODE_GETC(receive_quality);
    if ((SSM_OPTION_MODE_1 == p_ssm_glb->option_mode &&
        (SSM_QL_PRC == receive_quality || SSM_QL_TNC == receive_quality||
        SSM_QL_SSUB == receive_quality || SSM_QL_EEC1 == receive_quality))
        || (SSM_OPTION_MODE_2 == p_ssm_glb->option_mode && 
        (SSM_QL_PRS == receive_quality || SSM_QL_STU == receive_quality ||
         SSM_QL_TNC == receive_quality || SSM_QL_ST2 == receive_quality ||
         SSM_QL_EEC2 == receive_quality || SSM_QL_SMC == receive_quality ||
         SSM_QL_ST3E == receive_quality || SSM_QL_PROV == receive_quality)))
    {
        p_db_ssm_if->receive_pdu_enable = TRUE;
    }
    else
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail on port %s because quality %s is wrong\n", 
        IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext),
        ssm_desc_quality(receive_quality, 1));
        
        if (SSM_QL_DNU == receive_quality)
        {
            p_db_ssm_if->receive_pdu_enable = TRUE;
            p_db_ssm_if->receive_quality = SSM_QL_DNU;
        }
        return PM_E_NONE;
    }
    p_db_ssm_if->receive_quality = receive_quality;
    
    /*extend ssm*/
    SSM_DECODE_GETC(ssm_extend_type); /*ssm extend type*/
    SSM_DECODE_GET_EMPTY(2);          /*ssm extend length*/
    SSM_DECODE_GET(device_clock_id, SSM_CLOCK_ID_LEN); /*device clock id*/
    SSM_DECODE_GETC(hop);            /*hop*/

    /*dump the debug information*/
    SSM_RXPKT_DEBUG("RX Dump: port: %s, pdu: %s, quality: %s, ssm extend: %s, clockid: %s, hop: %d",
                    IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext),
                    (version_flag == SSM_EVENT_PDU ? "Event PDU" : "Information PDU"), 
                    ssm_desc_quality(receive_quality, 1), 
                    (SSM_TLV_EX_TYPE == ssm_extend_type ? "enable" : "disable"),
                    ssm_desc_clockid(clock_id_str, device_clock_id), 
                    hop);
    
    if (SSM_TLV_EX_TYPE == ssm_extend_type)
    {
        sal_memcpy(p_db_ssm_if->receive_clock_id, device_clock_id, SSM_CLOCK_ID_LEN);
        p_db_ssm_if->hop = hop;
    }
    else
    {
        sal_memset(p_db_ssm_if->receive_clock_id, 0x00, SSM_CLOCK_ID_LEN);
        p_db_ssm_if->hop = 0;
    }

    if (SSM_EVENT_PDU == version_flag)
    {
        PM_E_RETURN(ssm_fsm_select_clock());
        if (SSM_PDU_ALL == p_db_ssm_if->ssm_pdu_direct)
        {
            /*reset information timer*/
            PM_E_RETURN(ssm_opts_refresh_send_timer(p_db_ssm_if));
        }
    }
    else
    {
        /*reset information timer*/
        PM_E_RETURN(ssm_opts_refresh_timeout_timer(p_db_ssm_if));
        PM_E_RETURN(ssm_fsm_select_clock());
    }
    
    return PM_E_NONE;
}

int32
ssm_pdu_tx(tbl_ssm_port_t * p_db_ssm_if, bool event_pdu)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_SIZE];
    uint8 pkt[SSMDU_FRAME_SIZE];
    int32 pkt_len = 0;
    
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strncpy(if_key.name, p_db_ssm_if->key.name, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        SSM_TXPKT_DEBUG("TX fail because interface %s is not exist\n", 
                        IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    if (!ssm_port_is_up(p_db_ssm_if))
    {
        /*SSM debug*/
        SSM_TXPKT_DEBUG("TX fail because interface %s is not up\n", 
                        IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    pkt_len = ssm_pdu_encode(pkt, MSG_SIZE, p_db_if, p_db_ssm_if, event_pdu);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_ssm(pkt, pkt_len, p_db_ssm_if->ifindex));
    return PM_E_NONE;
}

int32
ssm_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_ssm_global_t *p_ssm_glb = tbl_ssm_global_get_ssm_global();
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    char ifname_ext[IFNAME_SIZE];
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_ssm_if = ssm_pdu_get_ssm_if(ifindex);

    if (NULL == p_db_ssm_if)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because interface is not exist\n");
        return PM_E_NONE;
    }
    if (!p_db_ssm_if->synce_enable)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because synce is disable on interface %s\n", 
                   IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    if (SSM_PDU_TX == p_db_ssm_if->ssm_pdu_direct || SSM_PDU_NONE == p_db_ssm_if->ssm_pdu_direct)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because SSM pdu is not rx or both on interface %s\n", 
                   IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    if (!p_ssm_glb->synce_enable)
    {   
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because device synce is disable\n");
        return PM_E_NONE;
    }   
    if (!p_ssm_glb->ssm_enable)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because device ssm is disable\n");
        return PM_E_NONE;
    }
    if (SSM_DEVICE_MAST == p_ssm_glb->device_role)
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because device role is MASTER\n");  
        return PM_E_NONE;
    }
    if (!ssm_port_is_up(p_db_ssm_if))
    {
        /*SSM debug*/
        SSM_RXPKT_DEBUG("RX fail because interface %s is not up\n", 
                   IFNAME_ETH2FULL(p_db_ssm_if->key.name, ifname_ext));
        return PM_E_NONE;
    }

    rc = ssm_pdu_decode(p_db_ssm_if, p_msg->data, p_msg->data_len);
    if (rc < 0)
    {
        return PM_E_FAIL;
    }
    return PM_E_NONE;
}


void
ssm_pdu_information_send_trigger(void *p_data)
{
    tbl_ssm_port_t * p_db_ssm_if = NULL;
    p_db_ssm_if = (tbl_ssm_port_t*)p_data;
    if (!p_db_ssm_if)
    {
        SSM_LOG_ERR("Information pdu send fail because interface is NULL");
        return;
    }
    p_db_ssm_if->ssm_send_timer = NULL;
    p_db_ssm_if->send_quality = SSM_QL_INV3;

    /*ssm information pdu send*/
    ssm_pdu_tx(p_db_ssm_if, FALSE);

    /* start timer for new interval */
    p_db_ssm_if->ssm_send_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, ssm_pdu_information_send_trigger, p_db_ssm_if, SSM_INTERN_INTERVAL_SEND);
    if (!p_db_ssm_if->ssm_send_timer)
    {
        SSM_LOG_ERR("Start ssm send timer fail");
        return;
    }
    return;
}


int32
ssm_pdu_event_send_trigger()
{
    tbl_ssm_port_master_t *p_master = tbl_ssm_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_ssm_port_t* p_db_ssm_if = NULL;
    int32 rc = PM_E_NONE;

    CTCLIB_SLIST_LOOP(p_master->ssm_port_list, p_db_ssm_if, listnode)
    {
        if (p_db_ssm_if && p_db_ssm_if->ssm_send_timer)
        {
            /*ssm event pdu send*/
            PM_E_RETURN(ssm_pdu_tx(p_db_ssm_if, TRUE));
        }  
    }
    return PM_E_NONE;
}

