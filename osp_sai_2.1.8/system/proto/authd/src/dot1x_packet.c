/****************************************************************************
*  dot1x process
*
* Modify History:
* Revision      : R0.01
* Author        : Liwh
* Date          : 2016-03-14 11:07
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netinet/ip6.h>
#include <assert.h>

#include "proto.h"
#include "param_check.h"
#include "gen/tbl_dot1x_global_define.h"
#include "gen/tbl_dot1x_global.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_dot1x_radius_define.h"
#include "gen/tbl_dot1x_radius.h"
#include "gen/tbl_dot1x_mac_define.h"
#include "gen/tbl_dot1x_mac.h"
#include "gen/tbl_auth_session_define.h"
#include "gen/tbl_auth_session.h"
#include "gen/tbl_auth_server_define.h"
#include "gen/tbl_auth_server.h"
#include "dot1x.h"
#include "dot1x_api.h"
#include "dot1x_packet.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
/* EAPOL type messages.  */
char auth_packet_type_msg[][20] =
{
  {"EAP"},
  {"Start"},
  {"Logoff"},
  {"Key"},
  {"ASF-Alert"}
};

char auth_eap_code_msg[][20] =
{
  {"Unknown"},
  {"Request"},
  {"Response"},
  {"Success"},
  {"Failure"}
};

char auth_eap_type_msg[][30] =
{
  {"Unknown:0"},
  {"Identity"},
  {"Notification"},
  {"Nak"},
  {"MD5-Challenge"},
  {"One-Time Password (OTP)"},
  {"Generic Token Card"},
  {"Unknown:7"},
  {"Unknown:8"},
  {"Unknown:9"},
  {"Unknown:10"},
  {"Unknown:11"},
  {"Unknown:12"},
  {"TLS"},
  {"Unknown:14"},
  {"Unknown:15"},
  {"Unknown:16"},
  {"Unknown:17"},
  {"Unknown:18"},
  {"Unknown:19"},
  {"Unknown:20"},
  {"Unknown:21"},
  {"Unknown:22"},
  {"Unknown:23"},
  {"Unknown:24"},
  {"PEAP"}
};

char auth_radius_code_msg[][30] =
{
  {"Unknown:0"},
  {"Access-Request"},
  {"Access-Accept"},
  {"Access-Reject"},
  {"Accounting-Request"},
  {"Accounting-Response"},
  {"Unknown:6"},
  {"Unknown:7"},
  {"Unknown:8"},
  {"Unknown:9"},
  {"Unknown:10"},
  {"Access-Challenge"}
};

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/
extern int
start_auth_process(tbl_auth_session_t *pSess);

extern int stop_auth_process(tbl_auth_session_t *pSess);

extern void
auth_hmac_md5 (unsigned char *text, int text_len,
    unsigned char *key, int key_len, unsigned char *digest);

/****************************************************************************
 *
* Function
* 
****************************************************************************/
bool
dot1x_eapol_check_header(tbl_dot1x_port_t  *p_db_dot1x_port, dot1x_sockaddr_vlan_t *addr,
     eapol_hdr_t *hdr, uint16 eap_length)
{
    tbl_dot1x_global_t     *p_db_glb = NULL;
    tbl_interface_t        *p_rcv_if = NULL;

    p_db_glb = tbl_dot1x_global_get_dot1x_global();
    if (!p_db_glb)
    {
        DOT1X_LOG_ERR("dot1x_eapol_check_header tbl_dot1x_global not found");
        return FALSE;
    }

    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_rcv_if)
    {
        DOT1X_LOG_ERR("dot1x_eapol_check_header p_rcv_if not found for ifindex %d",
            p_db_dot1x_port->key.ifindex);
        return FALSE;
    }
    
    /* 7.5.7.a) The Destination MAC address field contains the PAE group
       address, as specified in 7.8 (in nonshared media LANs), or the
       specific MAC address of the PAE (in shared media LANs).  */
    if (!sal_memcmp(addr->dest_mac, p_db_glb->eapol_group_address, MAC_ADDR_LEN)
        && !sal_memcmp(addr->dest_mac, p_rcv_if->mac_addr, MAC_ADDR_LEN))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-%s]: Wrong destination MAC address"
             "(%02x%02x.%02x%02x.%02x%02x)",
             auth_packet_type_msg[hdr->packet_type],
             addr->dest_mac[0], addr->dest_mac[1], addr->dest_mac[2],
             addr->dest_mac[3], addr->dest_mac[4], addr->dest_mac[5]);
  
        return FALSE;
    }
  
    /* 7.5.7.c) The Packet Type field contains one of the values EAP-Packet,
       EAPOL-Start, EAPOL-Logoff, or EAPOL-Key, as specified in 7.5.4.  */
    if (hdr->packet_type >= AUTH_PACKET_TYPE_MAX)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-Unknown]: Wrong packet type(%u)",
             hdr->packet_type);
  
        /* Section 9.4.2.1.3.j.  */
        p_db_dot1x_port->eapol_invalid_frames_rx++;
        return FALSE;
    }   
  
    /*Section 9.4.2.1.3.k and section 7.5.5*/
    if (!eap_length && 
      !(AUTH_PACKET_EAPOL_START == hdr->packet_type || AUTH_PACKET_EAPOL_LOGOFF == hdr->packet_type))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-%s]: Wrong EAP length(%u)",
             auth_packet_type_msg[hdr->packet_type], eap_length);
  
        /* Section 9.4.2.1.3.k.  */
        p_db_dot1x_port->eap_len_error_frames_rx++;  
        return FALSE;
    }
  
    /* Section 9.4.2.1.3.b) EAPOL frames received.  */
    p_db_dot1x_port->eapol_frames_rx++;
  
    /* Section 9.4.2.1.3.l) Last EAPOL frame version.  */
    p_db_dot1x_port->eapol_last_frame_version = hdr->protocol_version;
  
    /* Section 9.4.2.1.3.m) Last EAPOL frame source.  */
    sal_memcpy(&p_db_dot1x_port->eapol_last_frame_source, addr->src_mac, MAC_ADDR_LEN);
  
    return TRUE;
}

bool
dot1x_eap_check_header(struct eap_packet *eap)
{
    uint16 length = sal_ntoh16 (eap->length);

    switch (eap->code)
    {
    case AUTH_EAP_CODE_REQUEST:
    case AUTH_EAP_CODE_RESPONSE:
        if (length < AUTH_EAP_HDR_LEN)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAP-%s]: EAP length is short(%d)",
                auth_eap_code_msg[eap->code], length);
            return FALSE;
        }
        break;
        
    case AUTH_EAP_CODE_SUCCESS:
    case AUTH_EAP_CODE_FAILURE:
        /* RFC 2284 2.2.2.  Success and Failure length should be four.  */
        if (length != AUTH_EAP_HDR_LEN)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAP-%s]: EAP length is wrong(%d)",
                auth_eap_code_msg[eap->code], length);
            return FALSE;
        }
        break;
        
    default:
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAP-Unknown]: Unknown EAP code(%d)", eap->code);
        return FALSE;
    }

    return TRUE;
}

bool
dot1x_eapol_eap_packet(tbl_dot1x_port_t  *p_db_dot1x_port, uint16 eap_length, 
    eap_packet_t *eap_hdr, dot1x_sockaddr_vlan_t *addr)
{
    uint32                 len = 0;
    tbl_interface_t        *p_rcv_if = NULL;
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac_entry;

    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

    if (eap_length < AUTH_EAP_HDR_LEN)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP]: EAP length is short(%d)", eap_length);

        p_db_dot1x_port->eapNoReq = TRUE;
        p_db_dot1x_port->eapResp = FALSE;

        return FALSE;
    }

    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_rcv_if)
    {
        DOT1X_LOG_ERR("dot1x_eapol_eap_packet p_rcv_if not found for ifindex %d",
            p_db_dot1x_port->key.ifindex);

        return FALSE;
    }

    /* Check EAP packet length.  */
    if (!dot1x_eap_check_header(eap_hdr))
    {
        return FALSE;
    }

    /* Update the EAP length.  */
    eap_length = sal_ntoh16(eap_hdr->length);

    if (AUTH_EAP_CODE_RESPONSE != eap_hdr->code)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-%s]: Drop non EAP-Response",
            auth_eap_code_msg[eap_hdr->code]);

        p_db_dot1x_port->eapNoReq = TRUE;
        p_db_dot1x_port->eapResp = FALSE;

        return FALSE;
    }

    if (AUTH_PORT_MODE_PORT == p_db_dot1x_port->auth_mode)
    {
        /* RFC 2284 2.2.1 The peer MUST send a Response packet in reply to
         a Request packet.  The Idenfifier field of the Response MUST
         match that of the Request.  */
        if (eap_hdr->identifier != p_db_dot1x_port->currentId)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-%s]: Received different ID(%u) "
                 "with %u", auth_eap_code_msg[eap_hdr->code],
                 eap_hdr->identifier, p_db_dot1x_port->currentId);

            p_db_dot1x_port->eapNoReq = TRUE;
            p_db_dot1x_port->eapResp = FALSE; 
            
            return FALSE;
        }
    }
    else
    {   
        sal_memcpy(&dot1x_mac_entry.key.mac, p_db_dot1x_port->dest_mac, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = p_db_dot1x_port->key.ifindex;
    
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-%s]: Received none exist ID(%u) ",
                        auth_eap_code_msg[eap_hdr->code], eap_hdr->identifier);

            p_db_dot1x_port->eapNoReq = TRUE;
            p_db_dot1x_port->eapResp = FALSE;

            return FALSE;
        }
    }

    if (eap_hdr->type == AUTH_EAP_TYPE_IDENTITY)
    {
        /* Section 8.5.4.1.1.e.  */
        p_db_dot1x_port->eapResp = TRUE;
        p_db_dot1x_port->eapReq = TRUE;

        /* 9.4.2.1.3.f.  */
        p_db_dot1x_port->eap_respid_frames_rx++;

        len = eap_length - (AUTH_EAP_HDR_LEN + AUTH_EAP_DATA_TYPE_LEN);
        if (len < 0 || len > AUTH_USER_NAMLEN)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-Response-Identity]: "
                "Invalid length(%d)", len);

            return 0;
        }

        /* Get user name.  */
        sal_memcpy(p_db_dot1x_port->user_name, eap_hdr->type_data, len);
        p_db_dot1x_port->user_name[len] = '\0';
      
        if(p_db_dot1x_mac)
        {
            sal_memcpy (p_db_dot1x_mac->user_name, eap_hdr->type_data, len);
            p_db_dot1x_mac->user_name[len] = '\0';

            /* modified by liwh for bug 38704, 2016-05-25 */
            p_db_dot1x_mac->current_radius = NULL;
            /* liwh end */
        }

        /* modified by liwh for bug 38704, 2016-05-25 */
        p_db_dot1x_port->current_radius = NULL;
        p_db_dot1x_port->retry = 0;
        /* liwh end */

        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-Response-Identity]: "
            "User identify(%s) for %s supplicant",
            p_db_dot1x_port->user_name, get_intf_print_name(p_rcv_if->key.name));
    }
    else
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [EAPOL-EAP-Response-%s] ID(%u), length(%hu)",
            auth_eap_type_msg[eap_hdr->type], eap_hdr->identifier, eap_length);

        /* Section 8.5.8.1.1.b.  */
        p_db_dot1x_port->eapResp = TRUE;

        /* 9.4.2.1.3.g.  */
        p_db_dot1x_port->eap_resp_frames_rx++;
    }
    
    /* Section 7.8.  */
    sal_memcpy(p_db_dot1x_port->dest_mac, addr->src_mac, MAC_ADDR_LEN);

    return TRUE;
}

bool
dot1x_eapol_start(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    /* Section 8.5.4.1.1.b.  */
    p_db_dot1x_port->auth_pae_eapStart = TRUE;

    /* 9.4.2.1.3.d.  */
    p_db_dot1x_port->eapol_start_frames_rx++;

    return TRUE;
}

bool
dot1x_eapol_logoff(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    /* Section 8.5.4.1.1.a.  */
    p_db_dot1x_port->auth_pae_eapLogoff = TRUE;

    /* 9.4.2.1.3.e. */
    p_db_dot1x_port->eapol_logoff_frames_rx++;

    return TRUE;
}

int
dot1x_eapol_key(tbl_dot1x_port_t  *p_db_dot1x_port)
{   
    if (p_db_dot1x_port->auth_key_xmit_keyTxEnabled)
    {
        p_db_dot1x_port->auth_key_recv_rxKey = TRUE;
    }

    if (!p_db_dot1x_port->auth_key_xmit_keyTxEnabled)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[EAPOL-Key]: Key transmit disabled, discard the packet!");
    }

    return TRUE;
}

int
dot1x_eapol_asf_alert(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    /* 802.1X-2001 7.5.4.e) EAPOL-Encapsulated-ASF-Alert. */
    dot1x_send_radius_eap(p_db_dot1x_port);

    return TRUE;
}

bool
dot1x_proc_eapol(tbl_dot1x_port_t  *p_db_dot1x_port, dot1x_sockaddr_vlan_t *addr)
{
    bool                   rc = TRUE;
    uint8                  *p_rcv_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;
    uint16                 eap_length = 0;  
    tbl_dot1x_global_t     *p_db_glb = NULL;
    
    p_rcv_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));
    eap_length = sal_ntoh16 (hdr->packet_length);

    /*IEEE 802.1X 2001, section 8.5.4.9, In HELD state, the state machine ignores and discards all EAPOL packets, 
        so as to discourage brute force attacks.*/
    if (AUTH_PAE_STATE_HELD == p_db_dot1x_port->auth_pae_state)
    {
        /*only ignore for port mode*/
        if(AUTH_PORT_MODE_MAC != p_db_dot1x_port->auth_mode)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[EAPOL]: discard EAPOL packet when in HELD state.");
            return FALSE;
        }
    }

    /* Section 7.5.7 a-b */
    if (!dot1x_eapol_check_header(p_db_dot1x_port, addr, hdr, eap_length))
    {
        return FALSE;
    }

    /*over write the client mac*/
    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_memcpy(p_db_dot1x_port->dest_mac, addr->src_mac, MAC_ADDR_LEN);
    }

    switch (hdr->packet_type)
    {
    case AUTH_PACKET_EAP_PACKET:
        rc = dot1x_eapol_eap_packet(p_db_dot1x_port, eap_length, eap_hdr, addr);
        break;
        
    case AUTH_PACKET_EAPOL_START:    
        p_db_glb = tbl_dot1x_global_get_dot1x_global();
        if (p_db_glb)
        {
            sal_memcpy(p_db_dot1x_port->dest_mac, p_db_glb->eapol_group_address, sizeof(mac_addr_t));
        }

        rc = dot1x_eapol_start(p_db_dot1x_port);
        break;
        
    case AUTH_PACKET_EAPOL_LOGOFF:        
        if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
        {
            tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
            tbl_dot1x_mac_t dot1x_mac_entry;
     
            sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));
            
            sal_memcpy(&dot1x_mac_entry.key.mac, &addr->src_mac, sizeof(mac_addr_t));
            dot1x_mac_entry.key.ifindex = addr->ifindex;
    
            p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
            if (p_db_dot1x_mac)
            {
                dot1x_api_user_off_line(p_db_dot1x_port, p_db_dot1x_mac);
            }
    
            if(p_db_dot1x_port->current_user_count_db > 0)
            {
                /*do not change state machine when mac user log off*/
                return 0;
            }
         }
        
         rc = dot1x_eapol_logoff(p_db_dot1x_port);
         break;
         
    case AUTH_PACKET_EAPOL_KEY:
         rc = dot1x_eapol_key(p_db_dot1x_port);
         break;
         
    case AUTH_PACKET_EAPOL_ASF_ALERT:
         rc = dot1x_eapol_asf_alert(p_db_dot1x_port);
         break;
         
    default:
         break;
    }
    
    return rc;
}

int32
dot1x_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    uint8                  *p_rcv_buf = NULL;
    dot1x_sockaddr_vlan_t  addr;
    tbl_interface_t        *p_rcv_if = NULL;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t       dot1x_port;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;
    bool                   run_sm = FALSE;
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac_entry;
    char                   ifname_ext[IFNAME_SIZE];

    sal_memset(&addr, 0, sizeof(struct dot1x_sockaddr_vlan));
    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

    p_rcv_buf = (uint8*)p_msg->data;

    sal_memcpy(&addr, p_rcv_buf, (2 * MAC_ADDR_LEN));
    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_msg->u_hdr.pkt.ifindex);
    if (!p_rcv_if)
    {
        DOT1X_LOG_ERR("dot1x_pdu_rx interface %d does not exist at all", p_msg->u_hdr.pkt.ifindex);
        return PM_E_NONE;
    }

    /*added by yejl to fix bug43874, 2017-06-07*/
    if (GLB_FLAG_ISSET(p_rcv_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "Dot1x receive fail because interface %s is mirror dest port\n", 
                   IFNAME_ETH2FULL(p_rcv_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    /*end by yejl*/
    
    addr.ifindex = p_rcv_if->ifindex;

    dot1x_port.key.ifindex = p_rcv_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
        return PM_E_NONE;
    }

    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    if (AUTH_PACKET_EAPOL_START == hdr->packet_type || AUTH_PACKET_EAPOL_LOGOFF == hdr->packet_type
        || AUTH_PACKET_EAPOL_KEY == hdr->packet_type)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[EAPOL-%s]: src(%02x%02x.%02x%02x.%02x%02x) -> "
            "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
            auth_packet_type_msg[hdr->packet_type],
            addr.src_mac[0], addr.src_mac[1], addr.src_mac[2],
            addr.src_mac[3], addr.src_mac[4], addr.src_mac[5],
            addr.dest_mac[0], addr.dest_mac[1], addr.dest_mac[2],
            addr.dest_mac[3], addr.dest_mac[4], addr.dest_mac[5],
            get_intf_print_name(p_rcv_if->key.name));
    }
    else
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[EAPOL-%s-%s-%s]: src(%02x%02x.%02x%02x.%02x%02x) -> "
            "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
            auth_packet_type_msg[hdr->packet_type],
            auth_eap_code_msg[eap_hdr->code],
            auth_eap_type_msg[eap_hdr->type],
            addr.src_mac[0], addr.src_mac[1], addr.src_mac[2],
            addr.src_mac[3], addr.src_mac[4], addr.src_mac[5],
            addr.dest_mac[0], addr.dest_mac[1], addr.dest_mac[2],
            addr.dest_mac[3], addr.dest_mac[4], addr.dest_mac[5],
            get_intf_print_name(p_rcv_if->key.name));
    }

    /* modified by liwh for bug 39835, 2016-07-28 */
    if (p_msg->hdr.length <= AUTH_EAPOL_MAX_LEN)
    {
        sal_memcpy(p_db_dot1x_port->p_buf, p_rcv_buf, p_msg->hdr.length);
    }
    else
    {
        sal_memcpy(p_db_dot1x_port->p_buf, p_rcv_buf, AUTH_EAPOL_MAX_LEN);
    }
    /* liwh end */

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "dot1x_pdu_rx src(%02x%02x.%02x%02x.%02x%02x) \n", 
        p_db_dot1x_port->src_mac[0], p_db_dot1x_port->src_mac[1], p_db_dot1x_port->src_mac[2],
        p_db_dot1x_port->src_mac[3], p_db_dot1x_port->src_mac[4], p_db_dot1x_port->src_mac[5]);
    
    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_memcpy(p_db_dot1x_port->dest_mac, addr.src_mac, MAC_ADDR_LEN);

        sal_memcpy(&dot1x_mac_entry.key.mac, addr.src_mac, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = addr.ifindex;

        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            if (dot1x_db_check_count(p_db_dot1x_port, TRUE))
            {
                return PM_E_NONE;
            }

            p_db_dot1x_mac = dot1x_db_add_mac(p_db_dot1x_port, &dot1x_mac_entry);
            if (!p_db_dot1x_mac)
            {
                return PM_E_NONE;
            }
        }
    }

    if (tbl_interface_is_up(p_rcv_if) 
        && (INVALID_RADIUS_MSG_ID == p_db_dot1x_port->radius_msg_id))
    {
        run_sm = dot1x_proc_eapol(p_db_dot1x_port, &addr);
        if (run_sm)
        {
            dot1x_run_sm(p_db_dot1x_port);
        }
    }

    return PM_E_NONE;
}

int32
dot1x_mac_by_pass_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    uint8                  *p_rcv_buf = NULL;
    dot1x_sockaddr_vlan_t  addr;
    tbl_interface_t        *p_rcv_if = NULL;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_dot1x_port_t       dot1x_port;
    
    sal_memset(&addr, 0, sizeof(struct dot1x_sockaddr_vlan));
    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    p_rcv_buf = (uint8*)p_msg->data;

    sal_memcpy(&addr, p_rcv_buf, (2 * MAC_ADDR_LEN));
    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_msg->u_hdr.pkt.ifindex);
    if (!p_rcv_if)
    {
        DOT1X_LOG_ERR("dot1x_pdu_rx interface %d does not exist at all", p_msg->u_hdr.pkt.ifindex);
        return PM_E_NONE;
    }

    addr.ifindex = p_rcv_if->ifindex;

    dot1x_port.key.ifindex = p_rcv_if->ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
        return PM_E_NONE;
    }

    if (addr.dest_mac[0] == 0x01 && addr.dest_mac[1] == 0x80 && addr.dest_mac[2] == 0xC2)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[PACKET]: discard eapol src(%02x%02x.%02x%02x.%02x%02x) -> "
            "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
            addr.src_mac[0], addr.src_mac[1], addr.src_mac[2],
            addr.src_mac[3], addr.src_mac[4], addr.src_mac[5],
            addr.dest_mac[0], addr.dest_mac[1], addr.dest_mac[2],
            addr.dest_mac[3], addr.dest_mac[4], addr.dest_mac[5],
            get_intf_print_name(p_rcv_if->key.name));
        return PM_E_NONE;
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[PACKET]: src(%02x%02x.%02x%02x.%02x%02x) -> "
        "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
        addr.src_mac[0], addr.src_mac[1], addr.src_mac[2],
        addr.src_mac[3], addr.src_mac[4], addr.src_mac[5],
        addr.dest_mac[0], addr.dest_mac[1], addr.dest_mac[2],
        addr.dest_mac[3], addr.dest_mac[4], addr.dest_mac[5],
        get_intf_print_name(p_rcv_if->key.name));   

    /* modified by liwh for bug 39835, 2016-07-28 */
    if (p_msg->hdr.length <= AUTH_EAPOL_MAX_LEN)
    {
        sal_memcpy(p_db_dot1x_port->p_buf, p_rcv_buf, p_msg->hdr.length);
    }
    else
    {
        sal_memcpy(p_db_dot1x_port->p_buf, p_rcv_buf, AUTH_EAPOL_MAX_LEN);
    }
    /* liwh end */
    
    sal_memcpy(p_db_dot1x_port->dest_mac, addr.src_mac, MAC_ADDR_LEN);

    if ((AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
        && p_db_dot1x_port->mac_auth_bypass)
    {
        tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
        tbl_dot1x_mac_t dot1x_mac_entry;
 
        sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

        sal_memcpy(&dot1x_mac_entry.key.mac, addr.src_mac, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = addr.ifindex;

        sal_sprintf((char*)dot1x_mac_entry.user_name, "%02x%02x%02x%02x%02x%02x",
            addr.src_mac[0], addr.src_mac[1], addr.src_mac[2],
            addr.src_mac[3], addr.src_mac[4], addr.src_mac[5]);

        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            if (dot1x_db_check_count(p_db_dot1x_port, TRUE))
            {
                return PM_E_NONE;
            }

            p_db_dot1x_mac = dot1x_db_add_mac(p_db_dot1x_port, &dot1x_mac_entry);
            if (!p_db_dot1x_mac)
            {
                return PM_E_NONE;
            }
        }
        else
        {
            return PM_E_NONE;    
        }

        p_db_dot1x_mac->is_auth_bypass = TRUE;

        sal_memcpy(p_db_dot1x_mac->user_name, dot1x_mac_entry.user_name, AUTH_USER_NAMLEN);

        dot1x_send_radius_user_authen(p_db_dot1x_port, p_db_dot1x_mac);
    }
    
    return PM_E_NONE;
}  


/* 802.1X-2001 Section 7.2 EAPOL frame format for 802.3/Ethernet.  */
void
dot1x_eapol_make_header(uint8 *p_rcv_buf, uint8 type, uint8 version)
{
    eapol_hdr_t            *hdr = NULL;

    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);

    /* Set the EAPOL protocol version.  */
    hdr->protocol_version = version;

    /* Set the type.  */
    hdr->packet_type = type;

    /* Length will be set later.  */
    hdr->packet_length = 0;

    return;
}

void
dot1x_send_eapol(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint8                  *p_send_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;
    uint16                 length = 0;
    tbl_interface_t        *p_rcv_if = NULL;
    uint16                 ethertype = 0;
    char                   ifname_ext[IFNAME_SIZE];
    
    p_rcv_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (!p_rcv_if)
    {
        DOT1X_LOG_ERR("dot1x_send_eapol p_rcv_if not found for ifindex %d",
            p_db_dot1x_port->key.ifindex);
        return;
    }

    /*added by yejl to fix bug43874, 2017-06-07*/
    if (GLB_FLAG_ISSET(p_rcv_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "Dot1x send fail because interface %s is mirror dest port\n", 
                   IFNAME_ETH2FULL(p_rcv_if->key.name, ifname_ext));
        return;
    }
    /*end by yejl*/
   
    p_send_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_send_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_send_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    sal_memcpy(p_send_buf, p_db_dot1x_port->dest_mac, MAC_ADDR_LEN);
    sal_memcpy((p_send_buf + MAC_ADDR_LEN), p_db_dot1x_port->src_mac, MAC_ADDR_LEN);

    ethertype = sal_ntoh16(DOT1X_EAPOL_ETHERTYPE);
    sal_memcpy((p_send_buf + 2 * MAC_ADDR_LEN), &ethertype, sizeof(uint16));
    
    length = 2 * MAC_ADDR_LEN + 2 + sizeof(eapol_hdr_t) + sal_ntoh16(hdr->packet_length);

    if (length < AUTH_EAPOL_MIN_LEN)
    {
        sal_memset((p_send_buf + length), 0, (AUTH_EAPOL_MIN_LEN - length));
        length = AUTH_EAPOL_MIN_LEN;
    }
    
    if (AUTH_PACKET_EAPOL_START == hdr->packet_type || AUTH_PACKET_EAPOL_LOGOFF == hdr->packet_type
        || AUTH_PACKET_EAPOL_KEY == hdr->packet_type)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "SEND[EAPOL-%s-%s-%s]: src(%02x%02x.%02x%02x.%02x%02x) -> "
            "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
            auth_packet_type_msg[hdr->packet_type],
            auth_eap_code_msg[eap_hdr->code], 
            auth_eap_type_msg[eap_hdr->type], 
            p_db_dot1x_port->src_mac[0], p_db_dot1x_port->src_mac[1], p_db_dot1x_port->src_mac[2],
            p_db_dot1x_port->src_mac[3], p_db_dot1x_port->src_mac[4], p_db_dot1x_port->src_mac[5],
            p_db_dot1x_port->dest_mac[0], p_db_dot1x_port->dest_mac[1], p_db_dot1x_port->dest_mac[2],
            p_db_dot1x_port->dest_mac[3], p_db_dot1x_port->dest_mac[4], p_db_dot1x_port->dest_mac[5],
            get_intf_print_name(p_rcv_if->key.name));
    }
    else
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "SEND[EAPOL-%s-%s]: src(%02x%02x.%02x%02x.%02x%02x) -> "
            "dst(%02x%02x.%02x%02x.%02x%02x) on %s",
            auth_packet_type_msg[hdr->packet_type],
            auth_eap_code_msg[eap_hdr->code],
            p_db_dot1x_port->src_mac[0], p_db_dot1x_port->src_mac[1], p_db_dot1x_port->src_mac[2],
            p_db_dot1x_port->src_mac[3], p_db_dot1x_port->src_mac[4], p_db_dot1x_port->src_mac[5],
            p_db_dot1x_port->dest_mac[0], p_db_dot1x_port->dest_mac[1], p_db_dot1x_port->dest_mac[2],
            p_db_dot1x_port->dest_mac[3], p_db_dot1x_port->dest_mac[4], p_db_dot1x_port->dest_mac[5],
            get_intf_print_name(p_rcv_if->key.name));
    }
    
    /* send pkt through eapol socket to specific interface */
    if (ipc_pkt_tx_eapol((uint8*)p_db_dot1x_port->p_buf, length, p_db_dot1x_port->key.ifindex))
    {
        DOT1X_LOG_ERR("send eapol through port %d fail", 
            p_db_dot1x_port->key.ifindex);
        return;
    }   

    if (p_rcv_if && tbl_interface_is_up(p_rcv_if))
    {
        p_db_dot1x_port->eapol_frames_tx++; 
    }

    return;
}

/* 802.1X-2001 Section 8.5.4.1.3.c txReqId.  */
void
dot1x_send_eapol_req_id(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint8                  *p_rcv_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;
    uint8                  eapol_group_address[MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
    tbl_interface_t        *p_db_if = NULL;

    p_rcv_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));
    
    /* Make the EAPOL header.  */
    dot1x_eapol_make_header(p_rcv_buf, AUTH_PACKET_EAP_PACKET, p_db_dot1x_port->protocol_version);

    /* Set the EAP code.  */
    eap_hdr->code = AUTH_EAP_CODE_REQUEST;

    /* Send the EAP identifier.  */
    eap_hdr->identifier = p_db_dot1x_port->currentId;

    /* Set the EAP length.  */
    eap_hdr->length = sal_ntoh16(AUTH_EAP_HDR_LEN + AUTH_EAP_DATA_TYPE_LEN);

    /* Set the EAP type.  */
    eap_hdr->type = AUTH_EAP_TYPE_IDENTITY;
    
    /* Fill the EAPOL length.  */
    hdr->packet_length = eap_hdr->length; 

    /* modified by liwh for bug 38581, 2016-05-25 */
    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_memcpy(p_db_dot1x_port->dest_mac, eapol_group_address, MAC_ADDR_LEN);        
    }
    /* liwh end */

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (p_db_if && tbl_interface_is_up(p_db_if))
    {
        /* Send EAPOL frame.  */
        dot1x_send_eapol(p_db_dot1x_port);
    
        /* Section 9.4.2.1.3.h.  */
        p_db_dot1x_port->eap_reqid_frames_tx++;
    }

    return;
}

/* 802.1X-2001 Section 8.5.8.1.3.a txReq.  */
void
dot1x_send_eapol_req(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint8                  *p_rcv_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;
    uint16                 eap_length = 0;
    tbl_interface_t        *p_db_if = NULL;

    p_rcv_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    eap_length = sal_ntoh16(eap_hdr->length);
    
    /* Make the EAPOL header.  */
    dot1x_eapol_make_header(p_rcv_buf, AUTH_PACKET_EAP_PACKET, p_db_dot1x_port->protocol_version);

    if (eap_length < AUTH_EAP_HDR_LEN)
    {
        /* Set the EAP code.  */
        eap_hdr->code = AUTH_EAP_CODE_REQUEST;
    
        /* Send the EAP identifier.  */
        eap_hdr->identifier = p_db_dot1x_port->currentId;
    
        /* Set the EAP length.  */
        eap_hdr->length = sal_ntoh16(AUTH_EAP_HDR_LEN);        
    }
    
    /* Fill the EAPOL length.  */
    hdr->packet_length = eap_hdr->length;    

    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_dot1x_port->key.ifindex);
    if (p_db_if && tbl_interface_is_up(p_db_if))
    {
        /* Send EAPOL frame.  */
        dot1x_send_eapol(p_db_dot1x_port);
    
        /* Section 9.4.2.1.3.h.  */
        p_db_dot1x_port->eap_req_frames_tx++;
    }

    return;
}

/* 802.1X-2001 Section 8.5.8.1.3.c txCannedSuccess.  */
void
dot1x_send_eapol_success(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint8                  *p_rcv_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;

    p_rcv_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    /* Make the EAPOL header.  */
    dot1x_eapol_make_header(p_rcv_buf, AUTH_PACKET_EAP_PACKET, p_db_dot1x_port->protocol_version);

    /* Set the EAP code.  */
    eap_hdr->code = AUTH_EAP_CODE_SUCCESS;

    /* Send the EAP identifier.  */
    eap_hdr->identifier = p_db_dot1x_port->currentId;

    /* Set the EAP length.  */
    eap_hdr->length = sal_ntoh16(AUTH_EAP_HDR_LEN);

    /* Fill the EAPOL length.  */
    hdr->packet_length = eap_hdr->length; 
   
    /* Send EAPOL frame.  */
    dot1x_send_eapol(p_db_dot1x_port);

    return;
}

/* 802.1X-2001 Section 8.5.8.1.3.c txCannedSuccess.  */
void
dot1x_send_eapol_fail(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint8                  *p_rcv_buf = NULL;
    eapol_hdr_t            *hdr = NULL;
    eap_packet_t           *eap_hdr = NULL;

    p_rcv_buf = (uint8*)p_db_dot1x_port->p_buf;
    hdr = (eapol_hdr_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2);
    eap_hdr = (eap_packet_t*)(p_rcv_buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    /* Make the EAPOL header.  */
    dot1x_eapol_make_header(p_rcv_buf, AUTH_PACKET_EAP_PACKET, p_db_dot1x_port->protocol_version);

    /* Set the EAP code.  */
    eap_hdr->code = AUTH_EAP_CODE_FAILURE;

    /* Send the EAP identifier.  */
    eap_hdr->identifier = p_db_dot1x_port->currentId;

    /* Set the EAP length.  */
    eap_hdr->length = sal_ntoh16(AUTH_EAP_CODE_FAILURE);

    /* Fill the EAPOL length.  */
    hdr->packet_length = eap_hdr->length; 
   
    /* Send EAPOL frame.  */
    dot1x_send_eapol(p_db_dot1x_port);

    return;
}

void
dot1x_send_radius_eap(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    uint32                       session_id = 0;
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    int32                        rc = PM_E_NONE;
    char                         szSession[128];
    tbl_auth_session_t           auth_session;
    tbl_auth_session_t           *p_auth_session = NULL;
    eap_packet_t                 *eap_hdr = NULL;
    tbl_dot1x_radius_t           *p_db_dot1x_radius = NULL;

    sal_memset(&session_key, 0, sizeof(auth_session_key_t));
    sal_memset(&auth_session, 0, sizeof(tbl_auth_session_t));
    sal_memset(szSession, 0, 128);

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) 
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "Exceeding maximum authentication session numbers limit: %d", 
            GLB_AUTH_SESSIONS_NUM);
        return;
    }

    /* allocate session_id */
    if (!p_db_dot1x_port->retransmit)
    {
        session_id = dot1x_allocate_session_id();
        if (session_id >= RADIUS_SESSION_BUFFER)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "There is no free session");  
            return;
        }
    
        p_db_dot1x_port->radius_msg_id = session_id;
    }
 
    session_key.index = p_db_dot1x_port->key.ifindex;
    session_key.type = AUTH_SERVER_TYPE_RADIUS;
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session) 
    {
        sal_memcpy(&auth_session.key, &session_key, sizeof(auth_session_key_t));
        rc = tbl_auth_session_add_auth_session(&auth_session); 
        if (rc)
        {
            dot1x_free_session(session_id);
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            DOT1X_LOG_ERR("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
    
        p_auth_session = tbl_auth_session_get_auth_session(&session_key);
        if (!p_auth_session)
        {
            dot1x_free_session(session_id);
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            DOT1X_LOG_ERR("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
         
        pmaster->session_cnt++;
        
        sal_strcpy(p_auth_session->user.name, (char*)p_db_dot1x_port->user_name);

        p_auth_session->uCurSessId = p_db_dot1x_port->radius_msg_id;
        p_auth_session->is_dot1x_session = TRUE;

        eap_hdr = (eap_packet_t*)((char*)p_db_dot1x_port->buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));
        p_auth_session->eap_length = sal_ntoh16(eap_hdr->length);
        if (0 != p_auth_session->eap_length)
        {
            sal_memcpy(&p_auth_session->eap_buf, (char*)eap_hdr, p_auth_session->eap_length);
        }

        p_db_dot1x_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;

        if (p_db_dot1x_radius)
        {
            p_auth_session->radius_state_len = p_db_dot1x_radius->radius_state_len;
            if (0 != p_auth_session->radius_state_len)
            {
                sal_memcpy(&p_auth_session->radius_state_buf, (char*)p_db_dot1x_radius->radius_state, 
                    p_db_dot1x_radius->radius_state_len);
            }
        }
        else
        {
            p_auth_session->radius_state_len = 0;
        }

        if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
        {
            dot1x_mac_connect_session(p_db_dot1x_port, p_auth_session);
        }
           
        if ('\0' != p_auth_session->user.name[0]) 
        { 
            start_auth_process(p_auth_session);
        }
    }
    else
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "Session for ifindex %d is exist", p_db_dot1x_port->key.ifindex);      
    }
    
    return;
}

void
dot1x_send_radius_user_authen(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_db_dot1x_mac)
{
    uint32                       session_id = 0;
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    int32                        rc = PM_E_NONE;
    char                         szSession[128];
    tbl_auth_session_t           auth_session;
    tbl_auth_session_t           *p_auth_session = NULL;

    sal_memset(&session_key, 0, sizeof(auth_session_key_t));
    sal_memset(&auth_session, 0, sizeof(tbl_auth_session_t));
    sal_memset(szSession, 0, 128);

    pmaster = tbl_auth_session_get_master();
    if (pmaster->session_cnt >= GLB_AUTH_SESSIONS_NUM) 
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "Exceeding maximum authentication session numbers limit: %d", 
            GLB_AUTH_SESSIONS_NUM);
        return;
    }

    session_id = dot1x_allocate_session_id();
    if (session_id >= RADIUS_SESSION_BUFFER)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "There is no free session");  
        return;
    }

    p_db_dot1x_mac->session_id = session_id;
 
    session_key.index = AUTH_MAC_SESSION_HEADER +  p_db_dot1x_mac->session_id;
    session_key.type = AUTH_SERVER_TYPE_RADIUS;
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session) 
    {
        sal_memcpy(&auth_session.key, &session_key, sizeof(auth_session_key_t));
        rc = tbl_auth_session_add_auth_session(&auth_session); 
        if (rc)
        {
            dot1x_free_session(session_id);
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            DOT1X_LOG_ERR("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
    
        p_auth_session = tbl_auth_session_get_auth_session(&session_key);
        if (!p_auth_session)
        {
            dot1x_free_session(session_id);
            cdb_auth_session_key_val2str(szSession, 128, &session_key);
            DOT1X_LOG_ERR("add session %s, count %u failure", szSession, pmaster->session_cnt);
            return;
        }
        
        pmaster->session_cnt++;
        
        sal_strcpy(p_auth_session->user.name, (char*)p_db_dot1x_mac->user_name);

        p_auth_session->uCurSessId = p_db_dot1x_mac->session_id;
        p_auth_session->is_dot1x_session = TRUE;
        p_auth_session->is_auth_bypass = TRUE;
        
        sal_memcpy(p_auth_session->dot1x_mac_addr, p_db_dot1x_mac->key.mac, MAC_ADDR_LEN);
        p_auth_session->dot1x_mac_ifindex = p_db_dot1x_mac->key.ifindex;
    }
    
    if ('\0' != p_auth_session->user.name[0]) 
    { 
        start_auth_process(p_auth_session);
    }
    
    return;
}

int
dot1x_radius_check_header(tbl_auth_server_t *pSrv, struct radius_hdr *hdr)
{
    uint16 length = sal_ntoh16 (hdr->length);

    if (hdr->code >= RADIUS_CODE_MAX)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-Unknown]: Invalid code(%u) supplicant", hdr->code);
        return 0;
    }

    if (length != pSrv->blen)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Header length(%u) is different "
               "from the received length(%d)",
               auth_radius_code_msg[hdr->code],
               length, pSrv->blen);
        return 0;
    }

    if (length < RADIUS_MIN_PKT_LEN || length > RADIUS_MAX_PKT_LEN)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Invalid length(%u) for supplicant",
               auth_radius_code_msg[hdr->code],
               length);
        return 0;
    }

    return 1;
}

/* RFC 2869 EAP-Message Attribute.  */
int
dot1x_radius_proc_attr_eap(tbl_dot1x_port_t *p_db_dot1x_port, char *buf, uint16 len, uint16  buf_start)
{
    char     *p_buf_start = NULL;
    uint32   move_bytes = 0;
    
    /* reset the EAP frame buffer and proceed the EAPOL header length only for first time */
    if (AUTH_EAPOL_HDR_LEN == buf_start)
    {
        move_bytes = 2 * MAC_ADDR_LEN + 2 + buf_start;

        /* Reset the EAP frame buffer.  */
        sal_memset((p_db_dot1x_port->buf + move_bytes), 0,
            (AUTH_EAPOL_MAX_LEN - move_bytes));
    }

    /* Proceed the EAPOL header length.  */
    p_buf_start = (char*)p_db_dot1x_port->buf + 2 * MAC_ADDR_LEN + 2 + buf_start;

    /* Strip off the radius attribute header.  */
    buf += RADIUS_ATTR_HDR_LEN;
    len -= RADIUS_ATTR_HDR_LEN;

    sal_memcpy(p_buf_start, buf, len);

    return 1;
}

/* RFC 2869 5.14 Message-Authenticator Attribute.  */
int
dot1x_radius_proc_attr_authenticator(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, 
    char *buf, uint16 len)
{
    radius_hdr_t      *hdr = NULL;
    uint16            length = 0;
    u_char            msg_auth[RADIUS_AUTHENTICATOR_LEN];
    u_char            calc_auth[RADIUS_AUTHENTICATOR_LEN];
    
    sal_memset (msg_auth, 0, RADIUS_AUTHENTICATOR_LEN);
    sal_memset (calc_auth, 0, RADIUS_AUTHENTICATOR_LEN);
    
    hdr = (radius_hdr_t*)pSrv->buf;
    length = sal_ntoh16 (hdr->length);

    /* Strip off the radius attribute header.  */
    buf += RADIUS_ATTR_HDR_LEN;
    len -= RADIUS_ATTR_HDR_LEN;

    if (len != RADIUS_AUTHENTICATOR_LEN)
    {
        return 0;
    }

    sal_memcpy(msg_auth, buf, RADIUS_AUTHENTICATOR_LEN);
    sal_memset (buf, 0, RADIUS_AUTHENTICATOR_LEN);

    /* The message authenticator is calculated using the request
     authenticator from the access-request this packet is in
     reply to so restore the original req authenticator.  */
    sal_memcpy (hdr->authenticator, pSess->oauthenticator, RADIUS_AUTHENTICATOR_LEN);

    auth_hmac_md5((unsigned char *)pSrv->buf, length, 
                  (unsigned char *)pSrv->secret, sal_strlen ((char *)pSrv->secret), (unsigned char *)calc_auth);

    /* Compare the calculated msg authenticator to that received.  */
    if (sal_memcmp(calc_auth, msg_auth, RADIUS_AUTHENTICATOR_LEN))
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Invalid Message-Authenticator!",
            auth_radius_code_msg[hdr->code]);
        return 0;
    }

    /* Restroe the message-authenticator and the request authenticator 
        to their original values and return.  */
    sal_memcpy(buf, msg_auth, RADIUS_AUTHENTICATOR_LEN);
    sal_memcpy(hdr->authenticator, pSess->oauthenticator, RADIUS_AUTHENTICATOR_LEN);

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Correct Message-Authenticator",
        auth_radius_code_msg[hdr->code]);
  
    return 1;
}

/* RFC 2865 5.24. State Attribute.  */
int
dot1x_radius_proc_attr_state (tbl_auth_server_t *pSrv, tbl_dot1x_port_t *p_db_dot1x_port,
    char *buf, uint16 len)
{
    radius_hdr_t           *hdr = NULL;
    tbl_dot1x_radius_t     *p_db_dot1x_radius = NULL;

    hdr = (radius_hdr_t*)pSrv->buf;

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Type(State), length(%u)",
        auth_radius_code_msg[hdr->code], len);
    
    /* Strip off the RADIUS attribute header.  */
    buf += RADIUS_ATTR_HDR_LEN;
    len -= RADIUS_ATTR_HDR_LEN;

    p_db_dot1x_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;

    /* Copy state message.  */
    if (p_db_dot1x_radius && (len <= RADIUS_MAX_STATE_LEN))
    {
        sal_memcpy (p_db_dot1x_radius->radius_state, buf, len);
        p_db_dot1x_radius->radius_state_len = len;
    }
 
    return 1;
}

int
dot1x_radius_proc_attr(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, tbl_dot1x_port_t *p_db_dot1x_port)
{
    radius_hdr_t           *hdr = NULL;
    char                   *limit = NULL;
    char                   *ptr = NULL;
    uint16                 buf_start = AUTH_EAPOL_HDR_LEN;
    uint8                  code = 0;
    uint16                 len = 0;
    bool                   auth = FALSE;
 
    hdr = (radius_hdr_t*)pSrv->buf;
    limit = pSrv->buf + pSrv->blen;
    ptr = pSrv->buf + sizeof(radius_hdr_t);

    /* Process RADIUS attributes.  */
    while (ptr + RADIUS_ATTR_HDR_LEN <= limit)
    {
        code = *ptr;
        len = *(ptr + 1);

        switch (code)
        {
            case RADIUS_EAP_MESSAGE:
                DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Type(EAP-Message), length(%u)",
                         auth_radius_code_msg[hdr->code], len);
                                    
                /* Copy the EAP message to the buffer.  */
                if (!dot1x_radius_proc_attr_eap(p_db_dot1x_port, ptr, len, buf_start))
                {
                    return 0;
                }
                
                buf_start = buf_start + len - RADIUS_ATTR_HDR_LEN;
                break;

            case RADIUS_MSG_AUTHENTICATOR:
                if (auth)
                {
                    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Drop duplicate "
                         "Message-Authenticator", auth_radius_code_msg[hdr->code]);
                    return 0;
                }

                /* Check the authenticator.  */
                if (!dot1x_radius_proc_attr_authenticator(pSrv, pSess, ptr, len))
                {   
                    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Process duplicate "
                         "Message-Authenticator", auth_radius_code_msg[hdr->code]);
                    return 0;
                }

                auth = TRUE;
                break;

            case RADIUS_STATE:
                if (!dot1x_radius_proc_attr_state(pSrv, p_db_dot1x_port, ptr, len))
                {
                    return 0;
                }

                break;
        }
        ptr += len;
    }

    if (!auth)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: Missing Message-Authenticator!",
             auth_radius_code_msg[hdr->code]);
        return 0;
    }

    return 1;
}

void
dot1x_radius_proc_eap_request(tbl_dot1x_port_t *p_db_dot1x_port, struct eap_packet *eap)
{
    /* Section 8.5.8.1.1.f.  */
    p_db_dot1x_port->auth_be_idFromServer = eap->identifier;
  
    /* Section 8.5.8.1.1.e.  */
    p_db_dot1x_port->eapReq = 1;
  
    return;
}

void
dot1x_radius_proc_eap_response(tbl_dot1x_port_t *p_db_dot1x_port)
{
    /* EAP-Response shouldn't be sent by the RADIUS server.  */
    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-EAP-Response]: Drop for %s supplicant",
        get_intf_print_name(p_db_dot1x_port->name));

    /* Reset the retransmit value */
    p_db_dot1x_port->retry = 0;
  
    return;
}

void
dot1x_radius_proc_eap_success(tbl_dot1x_port_t *p_db_dot1x_port, struct eap_packet *eap, 
    tbl_auth_session_t *pSess)
{  
    tbl_dot1x_mac_t *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t dot1x_mac_entry;
 
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_memcpy(&dot1x_mac_entry.key.mac, pSess->dot1x_mac_addr, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = pSess->dot1x_mac_ifindex;
    
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            return;
        }
    
        p_db_dot1x_mac->session_id = 0;

        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS);

        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
        {
            if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH))
            {
                GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH);
            }

            /*restart timer*/
            if (p_db_dot1x_mac->t_reauth)
            {
                ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
                p_db_dot1x_mac->t_reauth = NULL;
            }

            if (p_db_dot1x_port->reAuthEnabled)  
            {    
                p_db_dot1x_mac->t_reauth = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                                    p_db_dot1x_mac, p_db_dot1x_port->reAuthPeriod); 
            }
            
            /* Section 8.5.8.1.1.f.  */
            p_db_dot1x_port->auth_be_idFromServer = eap->identifier;

            return;
        }
        
        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
        {
            GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);
        }
        else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
        {
            GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);
            if (p_db_dot1x_mac->t_reauth)
            {
                ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
                p_db_dot1x_mac->t_reauth = NULL;
            }
        }
        else
        {
            if (dot1x_db_check_count(p_db_dot1x_port, FALSE))
            {
                return;
            }
            
            /*entry not exist, add new entry*/
            dot1x_db_mac_inc_count(p_db_dot1x_port);
        }
        
        /*update flag*/
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);

        dot1x_set_fea_add_mac(p_db_dot1x_mac);
        
        if (p_db_dot1x_port->reAuthEnabled)  
        {   
            if (!p_db_dot1x_mac->t_reauth)
            {
                p_db_dot1x_mac->t_reauth = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                                p_db_dot1x_mac, p_db_dot1x_port->reAuthPeriod); 
            }
        }
    }

    /* Section 8.5.8.1.1.f.  */
    p_db_dot1x_port->auth_be_idFromServer = eap->identifier;

    return;
}

void
dot1x_radius_proc_eap_failure(tbl_dot1x_port_t *p_db_dot1x_port, struct eap_packet *eap,
    tbl_auth_session_t *pSess)
{
    tbl_dot1x_mac_t    *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t    dot1x_mac_entry;
 
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

    if (AUTH_PORT_MODE_MAC == p_db_dot1x_port->auth_mode)
    {
        sal_memcpy(&dot1x_mac_entry.key.mac, pSess->dot1x_mac_addr, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = pSess->dot1x_mac_ifindex;
    
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            return;
        }
    
        p_db_dot1x_mac->session_id = 0;

        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS);
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_EAPOL_REAUTH);

        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
        {
            /*do nothing*/
            return;
        }
                
        if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
        {
            GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
        }
        else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
        {
            GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);               
        }
        else
        {
            if (dot1x_db_check_count(p_db_dot1x_port, FALSE))
            {
                return;
            }
            
            /*entry not exist, add new entry*/
            dot1x_db_mac_inc_count(p_db_dot1x_port);
        }
        
        /*update flag*/
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);

        dot1x_set_fea_add_mac(p_db_dot1x_mac);

        if (p_db_dot1x_mac->t_reauth)
        {
            ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
            p_db_dot1x_mac->t_reauth = NULL;
        }

        if (!p_db_dot1x_mac->t_reject)
        {
            p_db_dot1x_mac->t_reject = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reject_timer_fn, 
                p_db_dot1x_mac, p_db_dot1x_port->auth_pae_quietPeriod); 
        }
    }

    /* Section 8.5.8.1.1.f.  */
    p_db_dot1x_port->auth_be_idFromServer = eap->identifier;
  
    return;
}

void
dot1x_radius_proc_eap(tbl_auth_server_t *pSrv, tbl_dot1x_port_t *p_db_dot1x_port, tbl_auth_session_t *pSess)
{
    radius_hdr_t           *hdr = NULL;
    eap_packet_t           *eap = NULL;

    hdr = (radius_hdr_t*)pSrv->buf;
    
    eap = (eap_packet_t*)(p_db_dot1x_port->buf + 2 * MAC_ADDR_LEN + 2 + sizeof(struct eapol_hdr));

    /* Parse out the little bits and update the port data structure */
    if (!dot1x_eap_check_header(eap))
    {
        return;
    }

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-%s]: EAP-%s for %s supplicant",
        auth_radius_code_msg[hdr->code],
        auth_eap_code_msg[eap->code], get_intf_print_name(p_db_dot1x_port->name));

    switch (eap->code)
    {
    case AUTH_EAP_CODE_REQUEST:
        dot1x_radius_proc_eap_request(p_db_dot1x_port, eap);
        break;
    case AUTH_EAP_CODE_RESPONSE:
        dot1x_radius_proc_eap_response(p_db_dot1x_port);
        break;
    case AUTH_EAP_CODE_SUCCESS:
        dot1x_radius_proc_eap_success(p_db_dot1x_port, eap, pSess);
        break;
    case AUTH_EAP_CODE_FAILURE:
        dot1x_radius_proc_eap_failure(p_db_dot1x_port, eap, pSess);
        break;
    default:
        break;
    }
    
    return;
}

void
dot1x_radius_access_request(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, tbl_dot1x_port_t *p_db_dot1x_port)
{
    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "    [RADIUS-Access-Request]: Drop supplicant");
    
    return;
}

void
dot1x_radius_access_accept(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, tbl_dot1x_port_t *p_db_dot1x_port)
{ 
    tbl_dot1x_radius_t     *p_db_dot1x_radius = NULL;
    
    /* Process RADIUS attributes.  */
    if (!dot1x_radius_proc_attr(pSrv, pSess, p_db_dot1x_port))
    {
        return;
    }

    /* Section 8.5.8.1.1.c - See Note in Section 8.4.8.  */
    p_db_dot1x_port->eapSuccess = TRUE;
    p_db_dot1x_port->eapFail = FALSE;

    /* Process EAP frame encoded in the EAP-Message Attribute.  */
    dot1x_radius_proc_eap(pSrv, p_db_dot1x_port, pSess);

    /* Clear our local variables now that the supplicant is authenticated.  */
    p_db_dot1x_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;
    if (p_db_dot1x_radius)
    {
        p_db_dot1x_radius->radius_state_len = 0;
    }

    p_db_dot1x_port->current_radius = NULL;

    dot1x_run_sm(p_db_dot1x_port);

    return;
}

void
dot1x_radius_access_reject(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, tbl_dot1x_port_t *p_db_dot1x_port)
{
    tbl_dot1x_radius_t     *p_db_dot1x_radius = NULL;
    
    /* Process RADIUS attributes.  */
    if (!dot1x_radius_proc_attr(pSrv, pSess, p_db_dot1x_port))
    {
        return;
    }

    /* Section 8.5.8.1.1.d - See Note in Section 8.4.8.  */
    p_db_dot1x_port->eapSuccess = FALSE;
    p_db_dot1x_port->eapFail = TRUE;

    /* Process EAP frame encoded in the EAP-Message Attribute.  */
    dot1x_radius_proc_eap(pSrv, p_db_dot1x_port, pSess);

    /* Clear our local variables now that the supplicant is authenticated.  */
    p_db_dot1x_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;
    if (p_db_dot1x_radius)
    {
        p_db_dot1x_radius->radius_state_len = 0;
    }
    p_db_dot1x_port->current_radius = NULL;
    
    dot1x_run_sm(p_db_dot1x_port);

    return;
}

void 
dot1x_radius_access_challenge(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess, tbl_dot1x_port_t *p_db_dot1x_port)
{
    /* Process RADIUS attributes.  */
    if (!dot1x_radius_proc_attr(pSrv, pSess, p_db_dot1x_port))
    {
        return;
    }
    
    dot1x_radius_proc_eap(pSrv, p_db_dot1x_port, pSess);
    
    dot1x_run_sm(p_db_dot1x_port);
    
    return;
}

void
dot1x_mac_access_accept(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_db_dot1x_mac)
{
    GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS);
    
    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
    {
        /*do nothing*/
        return;
    }
    else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        /* stop timer*/
        if (p_db_dot1x_mac->t_reauth)
        {
            ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
            p_db_dot1x_mac->t_reauth = NULL;
        }

        /*update entry*/
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
    }
    else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
    {
        /*update entry*/
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
    }
    else
    {
        if (dot1x_db_check_count(p_db_dot1x_port, FALSE))
        {
            return;
        }
        
        /*entry not exist, add new entry*/
        dot1x_db_mac_inc_count(p_db_dot1x_port);
        
        /*update flag*/
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
    }
  
    dot1x_set_fea_add_mac(p_db_dot1x_mac);

    if (p_db_dot1x_port->reAuthEnabled)  
    {    
        if (!p_db_dot1x_mac->t_reauth)
        {
            p_db_dot1x_mac->t_reauth = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reauth_timer_fn, 
                            p_db_dot1x_mac, p_db_dot1x_port->reAuthPeriod); 
        }
    }
    
    return;
}

void
dot1x_mac_access_reject(tbl_dot1x_port_t  *p_db_dot1x_port, tbl_dot1x_mac_t *p_db_dot1x_mac)
{
    GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_BYPASS);
    
    if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT))
    {
        /*do nothing*/
        return;
    }
    else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT))
    {
        /*update entry*/
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REAUTH_ACCEPT);
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
    }
    else if (GLB_FLAG_ISSET(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT))
    {
        /*update entry*/
        GLB_UNSET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_ACCEPT);
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);
    }
    else
    {
        if (dot1x_db_check_count(p_db_dot1x_port, FALSE))
        {
            return;
        }
        
        /*entry not exist, add new entry*/
        dot1x_db_mac_inc_count(p_db_dot1x_port);
        
        /*update flag*/
        GLB_SET_FLAG(p_db_dot1x_mac->auth_mac_flag, AUTH_MAC_REJECT);
    }
  
    dot1x_set_fea_add_mac(p_db_dot1x_mac);
    
    /* stop timer*/
    if (p_db_dot1x_mac->t_reauth)
    {
        ctc_task_delete_timer(p_db_dot1x_mac->t_reauth);
        p_db_dot1x_mac->t_reauth = NULL;
    }

    if (!p_db_dot1x_mac->t_reject)
    {
        p_db_dot1x_mac->t_reject = ctc_task_add_timer(TASK_PRI_NORMAL, dot1x_reject_timer_fn, 
            p_db_dot1x_mac, p_db_dot1x_port->auth_pae_quietPeriod); 
    }
    
    return;
}

void
dot1x_mac_proc_radius(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess)
{
    tbl_dot1x_port_t             dot1x_port;
    tbl_dot1x_port_t             *p_db_dot1x_port = NULL;
    tbl_dot1x_radius_t           dot1x_radius;
    tbl_dot1x_radius_t           *p_db_dot1x_radius = NULL;
    radius_hdr_t                 *hdr = NULL;
    tbl_dot1x_mac_t              *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t              dot1x_mac_entry;
 
    hdr = (radius_hdr_t*)pSrv->buf;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));    
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));

    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[RADIUS-%s]: src(%x) supplicant",
         auth_radius_code_msg[hdr->code], pSrv->key.addr.u.prefix4);
    
    dot1x_port.key.ifindex = pSess->dot1x_mac_ifindex;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
        return;
    }    

    sal_memcpy(&dot1x_radius.key.addr.u.prefix4, &pSrv->key.addr.u.prefix4,
        sizeof(addr_ipv4_t));
    dot1x_radius.key.addr.family = AF_INET;
    dot1x_radius.key.sin_port = pSrv->key.sin_port;
    p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
    if (!p_db_dot1x_radius)
    {
        char szHost[128];
        cdb_addr_val2str(szHost, 128, &dot1x_radius.key.addr);
        DOT1X_LOG_ERR("RADIUS host %s port %d does not exist", szHost, 
            dot1x_radius.key.sin_port);
        return;
    }

    sal_memcpy(&dot1x_mac_entry.key.mac, pSess->dot1x_mac_addr, sizeof(mac_addr_t));
    dot1x_mac_entry.key.ifindex = pSess->dot1x_mac_ifindex;

    p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
    if (!p_db_dot1x_mac)
    {
        return;
    }
    
    switch (hdr->code)
    {
        case RADIUS_ACCESS_ACCEPT:
            dot1x_mac_access_accept(p_db_dot1x_port, p_db_dot1x_mac);
            break;

        case RADIUS_ACCESS_REJECT:
            dot1x_mac_access_reject(p_db_dot1x_port, p_db_dot1x_mac);
            break;

        default:
            break;
    }

    /* delete session */
    dot1x_radius_end_mac_association(p_db_dot1x_mac);

    return;
}

void
dot1x_proc_radius(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess)
{
    radius_hdr_t           *hdr = NULL;
    tbl_dot1x_port_t       dot1x_port;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_dot1x_radius_t     dot1x_radius;
    tbl_dot1x_radius_t     *p_db_dot1x_radius = NULL;

    if (pSess->is_auth_bypass)
    {
        dot1x_mac_proc_radius(pSrv, pSess);
        return;
    }
 
    hdr = (radius_hdr_t*)pSrv->buf;

    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));
    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    
    DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "RECV[RADIUS-%s]: src(%x) supplicant",
         auth_radius_code_msg[hdr->code], pSrv->key.addr.u.prefix4);

    if (!dot1x_radius_check_header(pSrv, hdr))
    {
        return;
    }

    dot1x_port.key.ifindex = pSess->key.index;
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
    if (!p_db_dot1x_port)
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
        return;
    }    

    sal_memcpy(&dot1x_radius.key.addr.u.prefix4, &pSrv->key.addr.u.prefix4,
        sizeof(addr_ipv4_t));
    dot1x_radius.key.addr.family = AF_INET;
    dot1x_radius.key.sin_port = pSrv->key.sin_port;
    p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
    if (!p_db_dot1x_radius)
    {
        char szHost[128];
        cdb_addr_val2str(szHost, 128, &dot1x_radius.key.addr);
        DOT1X_LOG_ERR("RADIUS host %s port %d does not exist", szHost, 
            dot1x_radius.key.sin_port);
        return;
    }

    p_db_dot1x_port->current_radius = (void*)p_db_dot1x_radius;
    
    switch (hdr->code)
    {
        case RADIUS_ACCESS_REQUEST:
            dot1x_radius_access_request(pSrv, pSess, p_db_dot1x_port);
            break;
        case RADIUS_ACCESS_ACCEPT:
            dot1x_radius_access_accept(pSrv, pSess, p_db_dot1x_port);
            break;
        case RADIUS_ACCESS_REJECT:
            dot1x_radius_access_reject(pSrv, pSess, p_db_dot1x_port);
            break;
        case RADIUS_ACCESS_CHALLENGE:
            dot1x_radius_access_challenge(pSrv, pSess, p_db_dot1x_port);
            break;
        default:
            break;
    }

    /* delete session */
    dot1x_radius_end_association(p_db_dot1x_port); 

    dot1x_delete_session(p_db_dot1x_port->key.ifindex);
     
    return;
}

void
dot1x_delete_session(uint32  index)
{
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    tbl_auth_session_t           *p_auth_session = NULL;
    
    sal_memset(&session_key, 0, sizeof(auth_session_key_t));

    session_key.index = index;
    session_key.type = AUTH_SERVER_TYPE_RADIUS;
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session) 
    {
        DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "dot1x_delete_session session %d is not exist",
            index);
        return;
    }

    if (NULL != p_auth_session->pHandle)
    {
        stop_auth_process(p_auth_session);
    }
    
    pmaster = tbl_auth_session_get_master();
    if (!pmaster)
    {
        return;
    }
    pmaster->session_cnt--;
    dot1x_free_session(p_auth_session->uCurSessId);

    tbl_auth_session_del_auth_session(&session_key);

    return;
}

void
dot1x_radius_end_association(tbl_dot1x_port_t  *p_db_dot1x_port)
{
    p_db_dot1x_port->radius_msg_id = INVALID_RADIUS_MSG_ID;
        
    return;
}

void
dot1x_radius_end_mac_association(tbl_dot1x_mac_t *p_db_dot1x_mac)
{
    auth_session_key_t           session_key;
    tbl_auth_session_master_t    *pmaster;
    tbl_auth_session_t           *p_auth_session = NULL;
    
    sal_memset(&session_key, 0, sizeof(auth_session_key_t));
    
    session_key.index = AUTH_MAC_SESSION_HEADER + p_db_dot1x_mac->session_id;
    session_key.type = AUTH_SERVER_TYPE_RADIUS;
    p_auth_session = tbl_auth_session_get_auth_session(&session_key);
    if (!p_auth_session) 
    {
        dot1x_free_session(p_db_dot1x_mac->session_id);
        p_db_dot1x_mac->session_id= INVALID_RADIUS_MSG_ID;
        return;
    }

    if (NULL != p_auth_session->pHandle)
    {
        stop_auth_process(p_auth_session);
    }
        
    pmaster = tbl_auth_session_get_master();
    if (!pmaster)
    {
        return;
    }
    pmaster->session_cnt--;

    tbl_auth_session_del_auth_session(&session_key);

    dot1x_free_session(p_db_dot1x_mac->session_id);
    p_db_dot1x_mac->session_id = INVALID_RADIUS_MSG_ID;
    
    return;
}

void
dot1x_radius_set_identifier(tbl_auth_server_t *pSrv, tbl_auth_session_t *pSess)
{
    tbl_dot1x_radius_t     dot1x_radius;
    tbl_dot1x_radius_t     *p_db_dot1x_radius = NULL; 
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac_entry;
    tbl_dot1x_port_t       dot1x_port;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_dot1x_radius_t     *p_curr_radius = NULL; 

    sal_memset(&dot1x_radius, 0, sizeof(tbl_dot1x_radius_t));
    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));
    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    sal_memcpy(&dot1x_radius.key.addr.u.prefix4, &pSrv->key.addr.u.prefix4,
        sizeof(addr_ipv4_t));
    dot1x_radius.key.addr.family = AF_INET;
    dot1x_radius.key.sin_port = pSrv->key.sin_port;
    p_db_dot1x_radius = tbl_dot1x_radius_get_dot1x_radius(&dot1x_radius.key);
    if (!p_db_dot1x_radius)
    {
        char szHost[128];
        cdb_addr_val2str(szHost, 128, &dot1x_radius.key.addr);
        DOT1X_LOG_ERR("RADIUS host %s port %d does not exist", szHost, 
            dot1x_radius.key.sin_port);
        return;
    }

    p_db_dot1x_radius->identifier = pSess->uCurSessId;

    if (pSess->is_auth_bypass)
    {
        sal_memcpy(&dot1x_mac_entry.key.mac, pSess->dot1x_mac_addr, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = pSess->dot1x_mac_ifindex;
    
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            return;
        }

        if (!p_db_dot1x_mac->current_radius)
        {
            p_db_dot1x_mac->current_radius = (void*)p_db_dot1x_radius;
        } 
    }
    else
    {
        dot1x_port.key.ifindex = pSess->key.index;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
            return;
        }  
        else
        {
            if (!p_db_dot1x_port->current_radius)
            {
                p_db_dot1x_port->retry = 1;
                p_db_dot1x_port->current_radius = (void*)p_db_dot1x_radius;
            }
            else 
            {
                p_curr_radius = (tbl_dot1x_radius_t*)p_db_dot1x_port->current_radius;      
                if (p_curr_radius == p_db_dot1x_radius)
                {
                    p_db_dot1x_port->retry++; 
                }
                else
                {
                    p_db_dot1x_port->retry = 1;
                    p_db_dot1x_port->current_radius = (void*)p_db_dot1x_radius;                    
                }
            }
        } 
    }
     
    return;
}

void
dot1x_radius_release_session(tbl_auth_session_t *pSess)
{
    tbl_dot1x_mac_t        *p_db_dot1x_mac = NULL;
    tbl_dot1x_mac_t        dot1x_mac_entry;
    tbl_dot1x_port_t       dot1x_port;
    tbl_dot1x_port_t       *p_db_dot1x_port = NULL;
    tbl_auth_session_master_t    *pmaster;

    sal_memset(&dot1x_mac_entry, 0, sizeof(tbl_dot1x_mac_t));
    sal_memset(&dot1x_port, 0, sizeof(tbl_dot1x_port_t));

    if (pSess->is_auth_bypass)
    {
        sal_memcpy(&dot1x_mac_entry.key.mac, pSess->dot1x_mac_addr, sizeof(mac_addr_t));
        dot1x_mac_entry.key.ifindex = pSess->dot1x_mac_ifindex;
    
        p_db_dot1x_mac = tbl_dot1x_mac_get_dot1x_mac(&dot1x_mac_entry.key);
        if (!p_db_dot1x_mac)
        {
            /* modified by liwh for bug 47489, 2018-05-31 
                maybe dot1x mac is cleared, therefore auth session should be deleted */
            pmaster = tbl_auth_session_get_master();
            if (!pmaster)
            {
                return;
            }
            pmaster->session_cnt--;
        
            tbl_auth_session_del_auth_session(&pSess->key);
            /* liwh end */
            return;
        }

        p_db_dot1x_mac->current_radius = NULL;

        dot1x_radius_end_mac_association(p_db_dot1x_mac);

        /* set dot1x mac to reject */
        dot1x_port.key.ifindex = p_db_dot1x_mac->key.ifindex;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (p_db_dot1x_port)
        {
            dot1x_mac_access_reject(p_db_dot1x_port, p_db_dot1x_mac);
        }
        else
        {
            DOT1X_LOG_ERR("802.1x not configured on interface ifindex %d", dot1x_port.key.ifindex);
        }
    }
    else
    {
        dot1x_port.key.ifindex = pSess->key.index;
        p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&dot1x_port.key);
        if (!p_db_dot1x_port)
        {
            DOT1X_LOG_DEBUG(DOT1X_FLAG_PACKET, "802.1x not configured on this interface!");
            return;
        }    

        p_db_dot1x_port->current_radius = NULL;
        
        dot1x_radius_end_association(p_db_dot1x_port);

        dot1x_delete_session(p_db_dot1x_port->key.ifindex);
    }
 
    return;
}

