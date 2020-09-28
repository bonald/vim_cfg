#include "proto.h"

#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"

#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"

#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_lsp_aps_group_define.h"
#include "gen/tbl_lsp_aps_group.h"
#include "gen/tbl_pw_aps_group_define.h"
#include "gen/tbl_pw_aps_group.h"
#include "gen/tbl_aps_group_info_define.h"
#include "gen/tbl_aps_group_info.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"

#include "tpoam_sock.h"
#include "tpoam_pdu.h"
#include "tpoam_api.h"
#include "tpoam_debug.h"
#include "tpoam_error.h"

#define MPLS_OAM_ITUT_ALERT_LABEL13 13

#define MPLS_OAM_TYPE_LSP_P             0x1
#define MPLS_OAM_TYPE_LSP_PE            0x2
#define MPLS_OAM_TYPE_SECTION           0x3
#define MPLS_OAM_TYPE_PW                0x4
#define MPLS_OAM_TYPE_SPME              0x5

struct sockaddr_tpoam_s
{
  /* tx key type of OAM */
  unsigned int key_type;

  /* service key id, label/port/peer ip */
  unsigned int key;

  /* Outgoing/Incoming interface index */
  unsigned int port;

  /* Vlan id */
  unsigned short vlanid;

  /* SVlan id */
  unsigned short svlanid;

  /* pkt type */
  unsigned short type;

  /* reserve */
  unsigned short reserve;

  /* exp */
  unsigned char exp;

  /* ttl */
  unsigned char ttl;

  /* bypass */
  unsigned char bypass;

  /* reserve */
  unsigned char reserve2;  

  /* peer ip */
  unsigned int peer_ip;

  /* time stamp offset */
  unsigned int ts_offset;

  /* Destination Mac address */
  unsigned char dest_mac[ETH_ALEN];

  /* Source Mac address */
  unsigned char src_mac[ETH_ALEN];
  uint32  RxFcl;//for rcv
  uint32  priority;//for rcv
  uint32  oam_session_id;
  uint32 had_paser_label;//for rcv
  uint32 paser_label_length;//for rcv
  uint32  service_type;//for rcv
  uint32  label_num;
  mpls_label_t label;
  mpls_label_t outer_label;
};
typedef struct sockaddr_tpoam_s sockaddr_tpoam_t;


struct tpoam_sockaddr_s
{
    uint8   dest_mac[ETH_ALEN];
    uint16  ether_type;
    uint32  port;
    uint16  vlan_id;
    uint32  RxFcl; /* modified by yaom for y1731 tp oam adaption for GreatBelt 20130204*/
    uint32  priority;
    uint8   exception;
    uint8   rev;
};
typedef struct tpoam_sockaddr_s tpoam_sockaddr_t;

#define tpoam_eth_pkt_move_pld(buf, packet_p)\
do {                                    \
    uint8 *p = (uint8*)buf;               \
    p += ETH_HEADER_LEN;       \
    packet_p = p;              \
} while(0)

#define TPOAM_PUSHLABEL(pkt, label, exp, bos, ttl) \
do{                                         \
    uint32 shim = 0;                        \
    shim |= (((label) & 0xFFFFF) << 12);    \
    shim |= ((exp) & 0x7) << 9;             \
    shim |= ((bos) & 0x1) << 8;             \
    shim |= ((ttl) & 0xFF);                 \
    shim = sal_hton32(shim);                \
    sal_memcpy(pkt, &shim, LABEL_LEN);              \
}while(0);


#define TPOAM_PUSHACH(pkt, channel_type) \
do{                                         \
    uint32 ach = 0;                         \
    ach |= 0x1 << 28;                       \
    ach |= (channel_type) & 0xFFFF;         \
    ach = sal_hton32(ach);                  \
    sal_memcpy(pkt, &ach, ACH_LEN);               \
}while(0);



#define LABEL_LEN                  4
#define ACH_LEN                  4
#define ETHER_ADDR_LEN 6

uint8 router_mac_default[ETHER_ADDR_LEN]={0x00, 0x00, 0x0a, 0xff, 0xff, 0x00};


#define TPOAM_SKB_RESERVE 64
tpoam_sk_buff_t *
tpoam_alloc_skb(uint32 size)
{
    tpoam_sk_buff_t * skb;

    skb = XCALLOC(MEM_PM_TPOAM_PDU, sizeof(tpoam_sk_buff_t) + TPOAM_SKB_RESERVE + size);
    if (!skb)
    {
        return NULL;
    }
    
    skb->head = (uint8 *)(skb + 1);
    skb->end = (uint8 *)(skb + 1) + TPOAM_SKB_RESERVE + size;
    skb->data = skb->tail = (uint8 *)(skb + 1) + TPOAM_SKB_RESERVE;
    skb->len = 0;

    return skb;
}

void
tpoam_free_skb(tpoam_sk_buff_t** skb)
{
    if (*skb)
    {
        XFREE(MEM_PM_TPOAM_PDU, (*skb));
        *skb = NULL;
    }
}

int32
tpoam_get_route_mac(uint8 *route_mac)
{
    tbl_sys_global_t    *p_sys_glb = tbl_sys_global_get_sys_global();
    
    if (NULL == route_mac)
    {
        return TPOAM_E_FAIL;
    }
    
    sal_memcpy(route_mac, p_sys_glb->route_mac, sizeof(p_sys_glb->route_mac));
        
    return TPOAM_E_NONE;
}

uint8 *
tpoam_skb_push(tpoam_sk_buff_t* skb, uint32 len)
{
    skb->data -= len;
    skb->len  += len;
    return skb->data;
}

static void
_tpoam_encap_label(tpoam_sk_buff_t* skb, uint32 label, uint8 exp, uint8 bos, uint8 ttl)
{
    uint8 *data = NULL;
    data = tpoam_skb_push(skb, LABEL_LEN);
    
    TPOAM_PUSHLABEL(data, label, exp, bos, ttl);
    return;
}

static void
_tpoam_encap_ach(tpoam_sk_buff_t* skb, uint32 channel_type)
{
    uint8 *data = NULL;
    data = tpoam_skb_push(skb, ACH_LEN);
    
    TPOAM_PUSHACH(data, channel_type);
    return;
}

void
tpoam_desc_buffer(char *buffer, int32 buffer_size, uint32 is_tx)
{
    int i;
    int count = 0;
    char buf[543];

    memset(buf, 0, sizeof(buf));
    count += snprintf(buf + count, sizeof(buf) - count, "Raw:");
    for (i = 0 ; i < buffer_size ; ++i) 
    {
        if (count > sizeof(buf)) 
        {
            break;
        }
        
        if (!(i % 8))
        {
            /* The syslog-ng we used not permit multi-line log msg, every
             * newline character present a split new log msg */
            count += snprintf(buf + count, sizeof(buf) - count, " %02hhx", buffer[i]);
        } 
        else 
        {
            count += snprintf(buf + count, sizeof(buf) - count, "%02hhx", buffer[i]);
        }
    }
    if (is_tx)
    {
        TPOAM_DBG_PACKET_TX("Print packet tx buffer %d:%s\n", buffer_size, buf);
    }
    else
    {
        TPOAM_DBG_PACKET_RX("Print packet rx buffer %d:%s\n", buffer_size, buf);
    }
    return;
}

#define TPOAM_DESC_BUFFER_(A, B, is_tx) \
tpoam_desc_buffer(A, B, is_tx); 

int32
_tpoam_send_pkt(uint8 *p_pkt, uint32 pkt_len, sockaddr_tpoam_t *p_skaddr, uint32 aux_len, uint32 ifindex)
{
    uint32 flags = 0;
    
    if (ipc_pkt_tx_tpoam(p_pkt, pkt_len, (uint8 *)p_skaddr, aux_len, ifindex, flags))
    {
        TPOAM_DBG_PACKET_TX("Send packet through port %d failed", ifindex);
        
        return PM_E_SOCK;
    } 

    return PM_E_NONE;
}

#define ETH_P_MPLS_UC          0x8847 /* MPLS Unicast traffic   */

static void
_tpoam_encap_eth_hdr(tpoam_sk_buff_t* skb, char *mac_da, char *mac_sa)
{
    eth_hdr_t *p_eth_hdr = (eth_hdr_t *) tpoam_skb_push(skb, ETH_HEADER_LEN);
    
    sal_memset(p_eth_hdr, 0x00, ETH_HEADER_LEN);

    sal_memcpy(p_eth_hdr->dest_mac, mac_da, MAC_ADDR_LEN);
    sal_memcpy(p_eth_hdr->src_mac, mac_sa, MAC_ADDR_LEN);

    p_eth_hdr->ether_type = sal_hton16(ETH_P_MPLS_UC);
    
    return;
}

int32
tpoam_sock_pkt_send_new(uint32 type, uint32 key_type, uint32 key, uint8 exp, uint8 ttl, uint32 oif_ix, uint32 peer_ip,
                                  uint32 vid, uint32 channel_type, uint32 flag, uint32 pkt_size, tpoam_sk_buff_t* tx_skb, uint32 ts_offset,
                                  uint32 is_bypass, mpls_label_t labels[], uint32 labels_count, mpls_label_t pw_label, 
                                  uint8 dest_mac[])
{
    sockaddr_tpoam_t skaddr;
    //uint32 i = 0;
    int32 ret = 0;
    unsigned char route_mac[ETHER_ADDR_LEN] = {0};
    
    /*mpls L3 edit */
    switch(type)
    {   
        case MPLS_OAM_TYPE_LSP_P:
        {
            /*encap the ach*/
            _tpoam_encap_ach(tx_skb, channel_type);

            /*encap label 13*/
            _tpoam_encap_label(tx_skb, MPLS_OAM_ITUT_ALERT_LABEL13, exp, 1, 254); /*modified by yejl for bug 53706, 2019-11-06*/
            
            /*3. push outlabel */
            if (is_bypass)
            {
                /*encap out label*/
                _tpoam_encap_label(tx_skb, labels[0].label, labels[0].exp, 0, labels[0].ttl);
            }
            break;
        }
        case MPLS_OAM_TYPE_LSP_PE:
        {
            /*encap the ach*/
            _tpoam_encap_ach(tx_skb, channel_type);
            
            /*2. push label 13 */
            /*encap label 13*/
            _tpoam_encap_label(tx_skb, MPLS_OAM_ITUT_ALERT_LABEL13, exp, 1, 254); /*modified by yejl for bug 53706, 2019-11-06*/
            
            /*3. push outlabel */
            if (is_bypass)
            {
                /*encap out label*/
                _tpoam_encap_label(tx_skb, labels[0].label, labels[0].exp, 0, labels[0].ttl);
            }
            break;
        }
        case MPLS_OAM_TYPE_SECTION:
        {
            /*encap the ach*/
            _tpoam_encap_ach(tx_skb, channel_type);

            /*encap label 13*/
            _tpoam_encap_label(tx_skb, MPLS_OAM_ITUT_ALERT_LABEL13, exp, 1, ttl);
            break;
        }
        case MPLS_OAM_TYPE_PW:
        {
            /*encap the ach*/
            _tpoam_encap_ach(tx_skb, channel_type);

            /*fix bug 47607*/
            tbl_openflow_t *p_master = tbl_openflow_get_openflow();
            if(p_master->pw_not_standard_mode)
            {
                /*encap label 13*/
                _tpoam_encap_label(tx_skb, MPLS_OAM_ITUT_ALERT_LABEL13, 6, 1, 254);
                ts_offset += 4;
            }
            
            if (is_bypass)
            {
                /*encap PW label*/
                _tpoam_encap_label(tx_skb, pw_label.label, pw_label.exp, 1, pw_label.ttl);
            
                /*encap tunnel label*/
                _tpoam_encap_label(tx_skb, labels[0].label, labels[0].exp, 0, labels[0].ttl);
            }
            break;
        }
        default : 
            break;
    }

    /* L2 edit */
    sal_memset(&skaddr, 0, sizeof(sockaddr_tpoam_t));
    skaddr.key_type = key_type;
    skaddr.key = key;
    skaddr.port = oif_ix;
    //skaddr.peer_ip = peer_ip;
    skaddr.vlanid = vid;
    skaddr.reserve = flag;
    skaddr.exp = exp;
    skaddr.ttl = ttl;
    skaddr.ts_offset = ts_offset;

    if (is_bypass)
    {
        ret = tpoam_get_route_mac(route_mac);
        if(TPOAM_E_NONE != ret)
        { 
            TPOAM_DBG_PACKET_TX("Send packet failed when get route mac");
            return TPOAM_E_FAIL;
        }

        sal_memcpy (skaddr.dest_mac, dest_mac, ETHER_ADDR_LEN);
        sal_memcpy (skaddr.src_mac, route_mac, ETHER_ADDR_LEN);
        skaddr.bypass = TRUE;

        /*encap the eth header*/
        _tpoam_encap_eth_hdr(tx_skb, (char *)skaddr.dest_mac, (char *)skaddr.src_mac);
    }
    
    //TPOAM_DESC_BUFFER_((char *)tx_skb->data, tx_skb->len, TRUE);
    
    ret = _tpoam_send_pkt(tx_skb->data, tx_skb->len, &skaddr, sizeof(sockaddr_tpoam_t), oif_ix);

    return ret;
}

static int32
_tpoam_sock_get_vid_and_mac(uint32 oif_ix, uint32 mem_oif_ix, uint32* p_oifindex, uint32* p_vid, uint8* p_mac)
{
    tbl_interface_t *p_db_if = tbl_interface_get_interface_by_ifindex(oif_ix);
    if (NULL == p_db_if)
    {
        return TPOAM_E_INVALID_IFINDEX;
    }
    if (!tbl_interface_is_running(p_db_if))
    {
        return TPOAM_E_INTF_DOWN;
    }

    #ifdef support_tx_mac
    if (CHECK_FLAG(ifp->bind, NSM_IF_MPLS_TP_P2P))
    {
       sal_memcpy(p_mac, ifp->mpls_tp_p2p_tx_mac, ETHER_ADDR_LEN);
        *(uint16*)&p_mac[0] = sal_hton16(*(uint16*)&p_mac[0]);
        *(uint16*)&p_mac[2] = sal_hton16(*(uint16*)&p_mac[2]);
        *(uint16*)&p_mac[4] = sal_hton16(*(uint16*)&p_mac[4]);  
    }
    #endif

    #ifdef support_vlanif
    if (if_is_vlan(ifp))
    {
        pst_vlanif = (struct nsm_if *)ifp->info;     
        *p_vid = pst_vlanif->vid;

        if (mem_oif_ix)
        {
            mem_ifp = if_lookup_by_index (&nm->vr->ifm, mem_oif_ix);
            if (!mem_ifp)
            {
                return TPOAM_E_INVALID_IFINDEX;
            }
            *p_oifindex = mem_oif_ix;
        }
        else
        {
            *p_oifindex = 0;
        }
    }
    else
    #endif
    {
        *p_oifindex = oif_ix;
        *p_vid = 0;
    }
    
    return TPOAM_E_NONE;
}


static inline int32
_tpoam_sock_section_send_new (tbl_section_oam_t* pst_section, uint32 channel_type,
                int32 pkt_size, tpoam_sk_buff_t* tx_skb, uint32 exp, uint32 ttl, uint32 flag, uint32 ts_offset, uint32 is_bypass, uint32 pkt_type)
{
    int32 ret = TPOAM_E_NONE;
    uint32 oifindex = 0;
    uint32 vid = 0;
    mpls_label_t pwlabel;
    uint8 mac[ETHER_ADDR_LEN];
    mpls_label_t labels[1];
    uint32 labels_count = 1;
    
    if (!pst_section || !tx_skb || (0 == pkt_size))
    {
        return TPOAM_E_SOCK_TX_SECTION_CHECK_FAIL;
    }

    TPOAM_DBG_PACKET_TX("Oam session %u Section TX %s %s, size %d, exp %d, ttl %d", pst_section->oam_session_id,
        tpoam_api_pkt_type_desc(pkt_type), tpoam_api_get_ifname_ext_by_ifindex(pst_section->key.ifindex), pkt_size, exp, ttl);

    TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(pst_section->key.ifindex, 0, &oifindex, &vid, mac));

    //ret =  tpoam_sock_pkt_send(MPLS_OAM_TYPE_SECTION, GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP,
    //                               pst_section->oifindex, exp, ttl, oifindex, 0,
    //                               vid, channel_type, flag, pkt_size, p_pkt, ts_offset,
    //                               is_bypass, labels, labels_count, pwlabel, mac);
        
    ret =  tpoam_sock_pkt_send_new(MPLS_OAM_TYPE_SECTION, GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP,
                                   pst_section->key.ifindex, exp, ttl, oifindex, 0,
                                   vid, channel_type, flag, pkt_size, tx_skb, ts_offset,
                                   is_bypass, labels, labels_count, pwlabel, mac);

    if (TPOAM_E_INTF_DOWN == ret)
    {
        return ret;
    }
    
    if (ret < 0)
    {
        TPOAM_DBG_PACKET_TX("Section %s socket TX failed, ret %u", tpoam_api_get_ifname_ext_by_ifindex(pst_section->key.ifindex), ret);
        return TPOAM_E_SOCK_TX_SECTION_FAIL;
    }
    
    return TPOAM_E_NONE;
}

int32
_tpoam_sock_lsp_pe_send_new (tbl_lsp_pe_t* pst_lsp_pe, uint32 channel_type, int32 direction, 
        int32 pkt_size, tpoam_sk_buff_t* tx_skb, uint32 exp, uint32 ttl, uint32 flag, uint32 ts_offset, uint32 is_bypass, uint32 pkt_type)
{
    //tbl_lsp_pe_t* pst_tunnel_lsp = NULL;
    uint32 in_label = -1;
    uint32 oif_ix = 0;
    //uint32 mem_oif_ix = 0;
    
    int32 ret = TPOAM_E_NONE;
    uint32 oifindex = 0;
    uint32 vid = 0;

    mpls_label_t pwlabel;
    uint8 mac[ETHER_ADDR_LEN];
    mpls_label_t labels[2];
    uint32 labels_count = 2;
    
    if (!pst_lsp_pe || !tx_skb || (0 == pkt_size))
    {
        return TPOAM_E_SOCK_TX_LSP_CHECK_FAIL;
    }

    TPOAM_DBG_PACKET_TX("Oam session %u LSP-PE TX %s %s, size %d, exp %d, ttl %d", pst_lsp_pe->oam_session_id,
        tpoam_api_pkt_type_desc(pkt_type), pst_lsp_pe->key.name, pkt_size, exp, ttl);

    in_label = pst_lsp_pe->inlabel;

    labels[0].label = pst_lsp_pe->outlabel;
    labels[0].exp = exp;
    labels[0].ttl = ttl;
    labels_count = 1;

    oif_ix = pst_lsp_pe->ifindex;

    #ifdef support_vlanif
    mem_oif_ix = pst_lsp_pe->u.lsp_pe.mem_ifindex;
    #endif
    //TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, mem_oif_ix, &oifindex, &vid, mac));
    TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, 0, &oifindex, &vid, mac));
    sal_memcpy(mac, pst_lsp_pe->nexthop_mac, sizeof(mac));
    ret =  tpoam_sock_pkt_send_new(MPLS_OAM_TYPE_LSP_PE, GLB_TPOAM_TX_KEY_TYPE_LSP,
                                   in_label, exp, ttl, oifindex, 0,
                                   vid, channel_type, flag, pkt_size, tx_skb, ts_offset,
                                   is_bypass, labels, labels_count, pwlabel, mac);            

    if (TPOAM_E_INTF_DOWN == ret)
    {
        return ret;
    }

    if (ret < 0)
    {
        TPOAM_DBG_PACKET_TX("LSP-PE %s socket TX failed, ret %u", pst_lsp_pe->key.name, ret);
        return TPOAM_E_SOCK_TX_LSP_FAIL;
    }
    return TPOAM_E_NONE;
}

int32
_tpoam_sock_lsp_p_send_new(tbl_lsp_p_t* pst_lsp_p, uint32 channel_type, int32 direction, 
        int32 pkt_size, tpoam_sk_buff_t* tx_skb, uint32 exp, uint32 ttl, uint32 flag, uint32 ts_offset, uint32 is_bypass, uint32 pkt_type)
{
    //tbl_lsp_p_t* pst_tunnel_lsp = NULL;
    uint32 in_label = -1;
    uint32 oif_ix = 0;
    //uint32 mem_oif_ix = 0;
    
    int32 ret = TPOAM_E_NONE;
    uint32 oifindex = 0;
    uint32 vid = 0;

    mpls_label_t pwlabel;
    uint8 mac[ETHER_ADDR_LEN];
    mpls_label_t labels[2];
    uint32 labels_count = 2;
    
    if (!pst_lsp_p || !tx_skb || (0 == pkt_size))
    {
        return TPOAM_E_SOCK_TX_LSP_CHECK_FAIL;
    }

    TPOAM_DBG_PACKET_TX("Oam session %u LSP-P TX %s %s, size %d, exp %d, ttl %d, direction %s", pst_lsp_p->oam_session_id,
        tpoam_api_pkt_type_desc(pkt_type), pst_lsp_p->key.name, pkt_size, exp, ttl, tpoam_api_direction_desc(direction));
    
    if (TPOAM_DIR_EAST == direction)
    {
        in_label = pst_lsp_p->east_inlabel;

        {
            labels[0].label = pst_lsp_p->east_outlabel;
            labels[0].exp = exp;
            labels[0].ttl = ttl;
            labels_count = 1;
            
            oif_ix = pst_lsp_p->east_ifindex;
            #ifdef support_vlanif
            mem_oif_ix = pst_lsp_p->u.lsp_p.mem_ifindex_east;
            #endif
            //TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, mem_oif_ix, &oifindex, &vid, mac));
            TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, 0, &oifindex, &vid, mac));
            sal_memcpy(mac, pst_lsp_p->east_nexthop_mac, sizeof(mac));
            ret =  tpoam_sock_pkt_send_new(MPLS_OAM_TYPE_LSP_P, GLB_TPOAM_TX_KEY_TYPE_LSP,
                                           in_label, exp, ttl, oifindex, 0,
                                           vid, channel_type, flag, pkt_size, tx_skb, ts_offset,
                                           is_bypass, labels, labels_count, pwlabel, mac);            
        }
    }
    else
    {
        in_label = pst_lsp_p->west_inlabel;

        {
            labels[0].label = pst_lsp_p->west_outlabel;
            labels[0].exp = exp;
            labels[0].ttl = ttl;
            labels_count = 1;
            
            oif_ix = pst_lsp_p->west_ifindex;
            #ifdef support_vlanif
            mem_oif_ix = pst_lsp_p->u.lsp_p.mem_ifindex_west;
            #endif
            //TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, mem_oif_ix, &oifindex, &vid, mac));
            TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, 0, &oifindex, &vid, mac));
            sal_memcpy(mac, pst_lsp_p->west_nexthop_mac, sizeof(mac));
            ret =  tpoam_sock_pkt_send_new(MPLS_OAM_TYPE_LSP_P, GLB_TPOAM_TX_KEY_TYPE_LSP,
                                           in_label, exp, ttl, oifindex, 0,
                                           vid, channel_type, flag, pkt_size, tx_skb, ts_offset,
                                           is_bypass, labels, labels_count, pwlabel, mac);            
        }
    }

    if (TPOAM_E_INTF_DOWN == ret)
    {
        return ret;
    }

    if (ret < 0)
    {
        TPOAM_DBG_PACKET_TX("LSP-P %s socket TX failed, ret %u", pst_lsp_p->key.name, ret);
        return TPOAM_E_SOCK_TX_LSP_FAIL;
    }
    return TPOAM_E_NONE;
}

static inline int32
_tpoam_sock_pw_tpe_send_new(tbl_pw_tpe_t* pst_tpe, uint32 channel_type, 
        int32 pkt_size, tpoam_sk_buff_t* tx_skb, uint32 exp, uint32 ttl, uint32 flag, uint32 ts_offset, uint32 is_bypass, uint32 pkt_type)
{
    tbl_aps_group_info_t* p_db_ag_info = NULL;
    tbl_aps_group_info_key_t ag_info_key;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    uint32 oif_ix = 0;
    //uint32 mem_oif_ix = 0;
    int32  ret = TPOAM_E_NONE;
    uint32 oifindex = 0;
    uint32 vid = 0;
    mpls_label_t pwlabel;
    uint8 mac[ETHER_ADDR_LEN];
    mpls_label_t labels[1];
    uint32 labels_count = 1;
    
    if (!pst_tpe /*|| !pst_tpe->tunnel_name*/ || !tx_skb || (0 == pkt_size))
    {
        return TPOAM_E_SOCK_TX_TPE_CHECK_FAIL;
    }

    TPOAM_DBG_PACKET_TX("Oam session %u PW-TPE TX %s %s, size %d, exp %d, ttl %d", pst_tpe->oam_session_id,
        tpoam_api_pkt_type_desc(pkt_type), pst_tpe->key.name, pkt_size, exp, ttl);

    /*If lsp-name or lsp-group-id is exit, can get the lsp-pe from pw-tpe*/
    pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_tpe->lsp_name);
    if (NULL == pst_lsp_pe)
    {
        sal_memset(&ag_info_key, 0, sizeof(ag_info_key));
        ag_info_key.aps_group_id = pst_tpe->lsp_aps_group_id;
        p_db_ag_info = tbl_aps_group_info_get_aps_group_info(&ag_info_key);
        if (p_db_ag_info)
        {
            pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(p_db_ag_info->protection_name);
            if (NULL == pst_lsp_pe)
            {
                return TPOAM_E_SOCK_TX_TPE_CHECK_FAIL;
            }
        }
        else
        {
            return TPOAM_E_SOCK_TX_TPE_CHECK_FAIL;
        }
    }

    labels[0].label = pst_lsp_pe->outlabel;
    labels[0].exp = exp;
    labels[0].ttl = 255;
    
    pwlabel.label = pst_tpe->outlabel;
    pwlabel.exp = exp;
    pwlabel.ttl = ttl;

    oif_ix = pst_lsp_pe->ifindex;
    #ifdef support_vlanif
    mem_oif_ix = pst_lsp->u.lsp_pe.mem_ifindex;
    #endif
    //TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, mem_oif_ix, &oifindex, &vid, mac));
    TPOAM_OP_ERR_RET_NO_LOG(_tpoam_sock_get_vid_and_mac(oif_ix, 0, &oifindex, &vid, mac));

    ret =  tpoam_sock_pkt_send_new(MPLS_OAM_TYPE_PW, GLB_TPOAM_TX_KEY_TYPE_PW,
                                   pst_tpe->inlabel, exp, ttl, oifindex, 0,
                                   vid, channel_type, flag, pkt_size, tx_skb, ts_offset,
                                   is_bypass, labels, labels_count, pwlabel, mac);            
        
    if (TPOAM_E_INTF_DOWN == ret)
    {
        return ret;
    }

    if (ret < 0)
    {
        TPOAM_DBG_PACKET_TX("PW-TPE %s socket TX failed, ret %u", pst_tpe->key.name, ret);
        return TPOAM_E_SOCK_TX_PW_FAIL;
    }

    return TPOAM_E_NONE;
}

int32
tpoam_sock_tx_new(tbl_oam_session_t* pst_oam, uint32 direction, uint32 pkt_size, tpoam_sk_buff_t* tx_skb, 
    uint32 exp, uint32 ttl, uint32 flag, uint32 is_dmr, uint32 is_bypass, uint32 pkt_type)
{
    tbl_section_oam_t* pst_section = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_lsp_p_t *pst_lsp_p = NULL;
    tbl_pw_tpe_t* pst_tpe = NULL;
    uint32 ts_offset = 0;

    tbl_tpoam_global_t *p_db_tpoam_glb = tbl_tpoam_global_get_tpoam_global();

    if (TPOAM_SOCK_DM_FLAG_SET == flag)
    {
        switch (pst_oam->oam_type)
        {
            case GLB_OAM_TYPE_MPLS_SECTION:
                if (is_dmr)
                {
                    ts_offset = GLB_MPLSTP_OAM_SECTION_DMR_OFFSET;
                }
                else
                {
                    ts_offset = GLB_MPLSTP_OAM_SECTION_1DM_DMM_OFFSET;
                }
                break;
            case GLB_OAM_TYPE_MPLS_LSP:
                if (is_dmr)
                {
                    ts_offset = GLB_MPLSTP_OAM_LSP_DMR_OFFSET;
                }
                else
                {
                    ts_offset = GLB_MPLSTP_OAM_LSP_1DM_DMM_OFFSET;
                }
                break;
            case GLB_OAM_TYPE_MPLS_PW:
                if (is_dmr)
                {
                    ts_offset = GLB_MPLSTP_OAM_PW_DMR_OFFSET;
                }
                else
                {
                    ts_offset = GLB_MPLSTP_OAM_PW_1DM_DMM_OFFSET;
                }
                break;
            default:
                TPOAM_DBG_PACKET_TX("Send packet failed: invalid oam type %u", pst_oam->oam_type);
                return TPOAM_E_INVALID_TYPE;
        }
    }
    
    switch (pst_oam->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        pst_section = tbl_section_oam_get_section_oam_by_ifindex(pst_oam->service_ifindex);
        if (NULL != pst_section)
        {
            return _tpoam_sock_section_send_new(pst_section, p_db_tpoam_glb->channel_type, pkt_size, tx_skb, exp, ttl, flag, ts_offset, is_bypass, pkt_type);
        }
        break;
    case GLB_OAM_TYPE_MPLS_LSP:
        if (pst_oam->mp_type == TPOAM_MP_TYPE_MIP)
        {
            pst_lsp_p = tbl_lsp_p_get_lsp_p_by_name(pst_oam->service_name);
            if (NULL != pst_lsp_p)
            {
                return _tpoam_sock_lsp_p_send_new(pst_lsp_p, p_db_tpoam_glb->channel_type, direction, pkt_size, tx_skb, exp, ttl, flag, ts_offset, is_bypass, pkt_type);
            }
        }
        else
        {
            pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(pst_oam->service_name);
            if (NULL != pst_lsp_pe)
            {
                return _tpoam_sock_lsp_pe_send_new(pst_lsp_pe, p_db_tpoam_glb->channel_type, direction, pkt_size, tx_skb, exp, ttl, flag, ts_offset, is_bypass, pkt_type);
            }
        }
        break;
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pst_oam->pw_type)
        {
        case TPOAM_PW_TYPE_TPE:
            pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(pst_oam->service_name);
            if (NULL != pst_tpe)
            {
                return _tpoam_sock_pw_tpe_send_new(pst_tpe, p_db_tpoam_glb->channel_type, pkt_size, tx_skb, exp, ttl, flag, ts_offset, is_bypass, pkt_type);
            }
        default:
            TPOAM_DBG_PACKET_TX("Send packet failed: invalid pw type %u", pst_oam->pw_type);
            return TPOAM_E_INVALID_TYPE;
        }
    default:
        TPOAM_DBG_PACKET_TX("Send packet failed: invalid oam type %u", pst_oam->oam_type);
        return TPOAM_E_INVALID_TYPE;
    }
    return TPOAM_E_NONE;
}

int32
tpoam_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    //tpoam_sockaddr_t *p_skaddr;
    sockaddr_tpoam_t *p_skaddr;
    tpoam_rx_pdu_t rx_pdu;
    uint32 msg_len = 0;
    uint32 skaddr_len = 0;
    eth_hdr_t *eth_p = NULL;
    uint8 *buf = NULL;
    uint8 *skb_data;
    uint32 lsp_type = TPOAM_MP_TYPE_MEP;
    tbl_oam_session_t* pst_oam = NULL;
    
    //skaddr_len = sizeof(tpoam_sockaddr_t);
    skaddr_len = sizeof(sockaddr_tpoam_t);
    if (p_msg->data_len < skaddr_len)
    {
        TPOAM_DBG_PACKET_RX("Data length %u is less than skaddr length %u, error\n", p_msg->data_len, skaddr_len);
        return PM_E_FAIL;
    }

    /*get sockaddr*/
    msg_len = p_msg->data_len - skaddr_len;
    //p_skaddr = (tpoam_sockaddr_t*)((uint8*)p_msg->data + msg_len);
    p_skaddr = (sockaddr_tpoam_t*)((uint8*)p_msg->data + msg_len);

    /*msg with skaddr at tail*/
    //TPOAM_DESC_BUFFER_((char *)p_msg->data, p_msg->data_len, FALSE);
    
    buf = p_msg->data;
    eth_p = (eth_hdr_t*)buf;

    struct peth_vlanhdr_s
    {
        uint16_t tag;
        uint16_t proto;
    };
    typedef struct peth_vlanhdr_s peth_vlanhdr_t;

    peth_vlanhdr_t *vlan;
    uint16 tag;
    uint8 outer_cos;
    //uint16_t outer_vid;
    
    /*get priority*/
    vlan = (peth_vlanhdr_t *)(eth_p + 1);
    tag = sal_ntohs(vlan->tag);
    //outer_vid = tag & 0xFFF;
    outer_cos = tag >> 13;

    if (outer_cos != p_skaddr->priority)
    {
        // error
        //TPOAM_DBG_PACKET_RX("Outer cos %hhu not equal to priority %u, error\n", outer_cos, p_skaddr->priority);
    }

    /*msg strip skaddr*/
    //TPOAM_DESC_BUFFER_((char *)buf, msg_len, FALSE);
    
    sal_memset(&rx_pdu, 0, sizeof(rx_pdu));
    rx_pdu.ifindex = p_skaddr->port;
    //rx_pdu.vid = p_skaddr->vlan_id;
    rx_pdu.vid = p_skaddr->vlanid;
    rx_pdu.RxFcl = p_skaddr->RxFcl; /* modified by yaom for y1731 tp oam adaption for GreatBelt 20130204*/
    rx_pdu.priority = p_skaddr->priority;

    /*msg strip eth*/
    tpoam_eth_pkt_move_pld(buf, skb_data);
    rx_pdu.p_head = skb_data;
    rx_pdu.p_data = skb_data;
    rx_pdu.len = msg_len - ETH_HEADER_LEN;
    rx_pdu.left_len = msg_len - ETH_HEADER_LEN;
    
    if (p_skaddr->had_paser_label)
    {
        skb_data += p_skaddr->paser_label_length;
        rx_pdu.len = rx_pdu.len - p_skaddr->paser_label_length;
        rx_pdu.left_len = rx_pdu.left_len - p_skaddr->paser_label_length;
        rx_pdu.p_head = skb_data;
        rx_pdu.p_data = skb_data;
        
        rx_pdu.service_type = p_skaddr->service_type;
        rx_pdu.label_num = p_skaddr->label_num;
        sal_memcpy(&rx_pdu.label, &p_skaddr->label, sizeof(p_skaddr->label));
        sal_memcpy(&rx_pdu.outer_label, &p_skaddr->outer_label, sizeof(p_skaddr->outer_label));
    }
    //TPOAM_DESC_BUFFER_((char *)rx_pdu.p_data, rx_pdu.len, FALSE);

    pst_oam = tbl_oam_session_get_oam_session_by_session_id(p_skaddr->oam_session_id);
    if (pst_oam)
    {
        lsp_type = pst_oam->mp_type;
    }
    else
    {
        TPOAM_DBG_PACKET_RX("Cannot get tpoam session by session id %u\n", p_skaddr->oam_session_id);
    }
    
    /* Handle PDU. */
    tpoam_pdu_rx_pdu(&rx_pdu, p_skaddr->had_paser_label, p_skaddr->oam_session_id, TPOAM_PW_TYPE_TPE, lsp_type);

    return PM_E_NONE;
}

