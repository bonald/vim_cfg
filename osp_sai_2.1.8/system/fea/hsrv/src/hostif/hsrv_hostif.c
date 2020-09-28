/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_if.h"
#include "sai.h"
#include "ctc_sai_common.h"
#include "hsrv_hostif.h"
#include "hsrv_router.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_ssh_cfg_define.h"
#include "gen/tbl_ssh_cfg.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "glb_openflow_define.h"
#include "lib_syslimit.h"
#include "glb_openflow_packet.h"
#include "hsrv_ptp.h"
#include "hsrv_openflow.h"
#include "ctc_task.h"
#include "gen/tbl_fea_nexthop_define.h"
#include "gen/tbl_fea_nexthop.h"
#include "gen/tbl_fea_copp_trap_group_define.h"
#include "gen/tbl_fea_copp_trap_group.h"

extern uint32 ftm_info_profile_type_gb;
extern uint32 ftm_info_profile_type_d2;
extern uint32 ftm_info_profile_type_tm;

#define CTC_QOS_POLICER_RATE        1000

ctc_task_t           *raps_rec_while = NULL;  
uint32               raps_count = 0;
#define              MAX_RAPS_PKT_REC_PER_SEC    50

mac_addr_t hsrv_l2pro_lldp_dmac      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};
mac_addr_t hsrv_l2pro_bpdu_dmac      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
mac_addr_t hsrv_l2pro_slow_proto_dmac = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};
mac_addr_t hsrv_l2pro_eapal_dmac     = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};

extern mac_addr_t l2protocol_group_mac[GLB_L2_PROTOCOL_NUM];
extern mac_addr_t l2protocol_protocol_mac[GLB_L2_PROTOCOL_NUM];
extern char       l2protocol_protocol_name[GLB_L2_PROTOCOL_NUM][GLB_L2_PROTOCOL_NAME_SIZE];

/******************************************************************************
* Global and Declaration
******************************************************************************/
hsrv_hostintf_master_t *hostif_master = NULL;

void hsrv_raps_rec_timer_fn()
{
    if (raps_rec_while)
    {
        CTC_TASK_STOP_TIMER(raps_rec_while);
        raps_count = 0;
        raps_rec_while = CTC_TASK_ADD_TIME(hsrv_raps_rec_timer_fn, NULL, 1);
    }
    return;
}


struct mpls_label_s
{
    uint32 label:20;
    uint32 exp:3;
    uint32 s:1;
    uint32 ttl:8;    
};
typedef struct mpls_label_s mpls_label_t;

struct hsrv_sockaddr_tpoam_s
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
typedef struct hsrv_sockaddr_tpoam_s hsrv_sockaddr_tpoam_t;




struct nexthop_data//add by   yangl 
{
   uint32 IP_type;
   addr_ipv4_t  nexthop;       /* IP address of next hop router */
   uint32 src_mask;   /* Source address prefix mask bits */
   uint32 dst_mask;   /* Destination address prefix mask bits */
};
typedef  struct sflow_flow_extended_router_data   sflow_flow_extended_router_data_t;
struct vlan_hdr_s
{
    uint16 tag;
    uint16 proto;
};
typedef struct vlan_hdr_s vlan_hdr_t;


struct hsrv_ptp_sockaddr_s
{
    mac_addr_t dest_mac;
    uint16 ether_type;
    uint32 ifindex;
    uint16 vlan_id;
    uint8 vlan_priority;
    uint8 ts_offset;
    uint8 replace;
    uint8 capture; //CTC_PTP_CORRECTION+CTC_PKT_FLAG_PTP_TS_PRECISION
    uint8 correction;
    uint8 is_unicast;
    uint8 seq_id;
    uint32  seconds;            /**< [GB] seconds */
    uint32  nanoseconds;        /**< [GB] nano seconds */
    uint8 ptp_offset;
    uint8 ptp_msg_type;/*add for debug*/
};
typedef struct hsrv_ptp_sockaddr_s hsrv_ptp_sockaddr_t;

/******************************************************************************
* Defines and Macros
******************************************************************************/
extern int32
hsrv_mirror_to_cpu_rx_packet(uint8 * h_data, uint32 h_data_len, tbl_interface_t *p_db_if);

extern int32
hsrv_openflow_tunnel_get_info_by_metadata(uint32 meta_data, uint16* p_ofport, uint32* p_tun_id);

extern int32
hsrv_debug_packets_tx(char *buffer, int32 buffer_size, uint32 pdu_type);

extern int32
hsrv_debug_packets_rx(char *buffer, int32 buffer_size, int32 reason, char* ifname_ext, int32 src_svid);


static void
_hsrv_packet_get_index_from_standard_type(uint32 type, int32 *index, uint32 *is_edit, void *buffer)
{
    uint32 i = 0;
    mac_addr_t l2pro_dmac;
    sal_memset(&l2pro_dmac, 0, MAC_ADDR_LEN);
    sal_memcpy(l2pro_dmac, (uint8*)buffer, MAC_ADDR_LEN);
    
    switch(type)
    {
    case GLB_PKT_EAPOL:
        *index = GLB_L2_PROTO_INDEX_0;
        break;
    case GLB_PKT_L2_LLDP:
        *index = GLB_L2_PROTO_INDEX_1;
        break;
    case GLB_PKT_LACP:
    case GLB_PKT_SSM:
    case GLB_PKT_EFM:
        *index = GLB_L2_PROTO_INDEX_2;
        break;
    case GLB_PKT_BPDU:
        *index = GLB_L2_PROTO_INDEX_3;
        break;
    default:
        *index = -1;
        break;
    }
    for (i = GLB_L2_PROTO_INDEX_0; i <= GLB_L2_PROTO_INDEX_3; i++)
    {
        if (0 == sal_memcmp(l2protocol_group_mac[i], l2pro_dmac, MAC_ADDR_LEN))
        {
            *is_edit = TRUE;
            return;
        }
    }
    *is_edit = FALSE;
}

static void
_hsrv_packet_get_index_from_protocol_mac_type(int32 *index, uint32 *is_edit, void *buffer)
{    
    mac_addr_t l2pro_dmac;
    sal_memset(&l2pro_dmac, 0, MAC_ADDR_LEN);
    sal_memcpy(l2pro_dmac, (uint8*)buffer, MAC_ADDR_LEN);
    uint32 i = 0;
    for (i = GLB_L2_PROTO_INDEX_4; i <= GLB_L2_PROTO_INDEX_7; i++)
    {
        if (0 == sal_memcmp(l2protocol_protocol_mac[i], l2pro_dmac, MAC_ADDR_LEN))
        {
            *index = i;
            *is_edit = FALSE;
            return;
        }    
    }
}

static void
_hsrv_packet_get_index_from_group_mac_type(int32 *index, uint32 *is_edit, void *buffer)
{
    mac_addr_t l2pro_dmac;
    sal_memset(&l2pro_dmac, 0, MAC_ADDR_LEN);
    sal_memcpy(l2pro_dmac, (uint8*)buffer, MAC_ADDR_LEN);
    uint32 i = 0;
    for (i = GLB_L2_PROTO_INDEX_0; i <= GLB_L2_PROTO_INDEX_7; i++)
    {
        if (0 == sal_memcmp(l2protocol_group_mac[i], l2pro_dmac, MAC_ADDR_LEN))
        {
            *index = i;
            *is_edit = TRUE;
            return;
        }
    }
}

static void
_hsrv_packet_get_proc_slow_proto(char *buffer, uint32 *pm_id, uint32 *type)
{
    struct ethhdr *eth = (struct ethhdr *)buffer;
    uint8 *slow_proto = (uint8*)buffer;
    uint8 *subtype = NULL;
    uint16 ether_type = 0;
    
    /*Modify by yejl for bug 48137, 2018-07-21*/
    ether_type = sal_ntoh16(eth->h_proto);
    if (GLB_ETH_P_8021Q == ether_type)
    {
        subtype = (uint8*)(slow_proto + 18);
    }
    else if (ETH_P_SLOW_PROTOCOL == ether_type)
    {
        subtype = (uint8*)(eth + 1);
    }
    else
    {
        return;
    }
    switch (*subtype)
    {
    case SLOW_PROTO_CONTROL_LACP:
    case SLOW_PROTO_MARKER_LACP:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_LACP;
        break;
        
    case SLOW_PROTO_OAM:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_EFM;
        break;
        
    case SLOW_PROTO_SSM:
        *pm_id = PM_ID_SSM;
        *type = GLB_PKT_SSM;
        break;

    default:
        *pm_id = PM_ID_INVALID;
        *type = GLB_PKT_UNKNOWN;
    }
}

/***************************************************************************************************
 * Name         : _hsrv_packet_event_get_pm_id 
 * Purpose      : get process module id from packet event     
 * Input        : trap_id, packet event
 * Output       : pm_id, process module id                  
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
static void
_hsrv_packet_event_get_pm_id(uint32 trap_id, uint32 *pm_id, uint32 *pm_id1, uint32 *pm_id2, uint32 *pm_id3, uint32 *pm_id4, uint32 *type)
{
    switch (trap_id)
    {
    case SAI_HOSTIF_TRAP_ID_STP:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_BPDU;
        break;
#if 0
    case SAI_HOSTIF_TRAP_ID_LACP:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_LACP;
        break;
#endif
    case SAI_HOSTIF_TRAP_ID_EAPOL:
        *pm_id = PM_ID_AUTHD;
        *type = GLB_PKT_EAPOL;
        break;

    case SAI_HOSTIF_TRAP_ID_LLDP:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_L2_LLDP;
        break;

    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_MAC_SEC;
        break;
        
    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_MISMATCH:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_MAC_SEC;
        break;
        
    case SAI_HOSTIF_TRAP_ID_CPU_MIRROR:
        *pm_id = PM_ID_INVALID;
        *type = GLB_PKT_CPU_MIRROR;
        break;

    case SAI_HOSTIF_TRAP_ID_ARP_REQUEST:
    case SAI_HOSTIF_TRAP_ID_ARP_RESPONSE:
        *pm_id = PM_ID_ROUTED;
        *type = GLB_PKT_ARP;
        break;

    case SAI_HOSTIF_TRAP_ID_DHCP:
        *pm_id = PM_ID_DHCSNOOPING;
        *pm_id1 = PM_ID_DHCRELAY;
        *pm_id2 = PM_ID_DHCLIENT;
        *pm_id3 = PM_ID_DHCLIENT6;
        *pm_id4 = PM_ID_DHCPD6;
        *type = GLB_PKT_DHCP;
        break;
        
    case SAI_HOSTIF_TRAP_ID_ERPS:
        *pm_id = PM_ID_OPM;
        *type = GLB_PKT_ERPS;
        break;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT:
    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_IGMP;
        break;
    /* support pim by yangl for bug 47256, 2018-11-30 */
    case SAI_HOSTIF_TRAP_ID_PIM:
         *pm_id = PM_ID_SWITCH;
         *type = GLB_PKT_PIM;
    break;
     /*yangl end*/

    case SAI_HOSTIF_TRAP_ID_SCL_MATCH:
        *pm_id = PM_ID_AUTHD;
        *type = GLB_PKT_MAC_BY_PASS;
        break;

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER:
        *pm_id = PM_ID_OPENFLOW;
        *type = GLB_PKT_OPENFLOW;
         break;

    /* support sflow by liwh for bug 43981, 2017-06-05 */
    case SAI_HOSTIF_TRAP_ID_SAMPLEPACKET:
        *pm_id = PM_ID_SWITCH;
        *type = GLB_PKT_SFLOW;
        break;
    /* liwh end */

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    case SAI_HOSTIF_TRAP_ID_VRRP:
        *pm_id = PM_ID_ROUTED;
        *type = GLB_PKT_VRRP;
        break;
    /* liwh end */

    /* support ptp modified by chenc */
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP:
        *pm_id = PM_ID_PTP;
        *type = GLB_PKT_PTP;
        break;
    /* chenc end */

    /* support tpoam modified by chenc */
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_TPOAM:
        *pm_id = PM_ID_OPM;
        *type = GLB_PKT_TPOAM;
        break;
    /* chenc end */

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_PROTOCOL_MAC:
        *type = GLB_PKT_L2_PROTOCOL_MAC;
        break;
        
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_GROUP_MAC:
        *type = GLB_PKT_L2_GROUP_MAC;
        break;

    case SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY:
        *pm_id = PM_ID_ROUTED;
        *type = GLB_PKT_ND;
        break;
    case SAI_HOSTIF_TRAP_ID_L2_COPY_TO_CPU:
        *pm_id = PM_ID_OPM;
        *type = GLB_PKT_RAPS;
        break;
         
    default:
        *pm_id = PM_ID_SWITCH;
         *type = GLB_PKT_UNKNOWN;
        break;
    }
}

#define MSTP_TYPE_MSTP  2

/* Added by kcao for bug 40778, add STP/ERPS status check */
int32 hsrv_hostif_port_state_check(uint32 tx, uint32 pkt_type, uint32 vid, tbl_interface_t *p_db_if)
{
    tbl_mstp_global_t *p_mstp_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_key_t msti_key;
    tbl_vlan_key_t vlan_key;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_mstp_port_t *p_db_mstp_port = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 stp_ok = FALSE;
    uint32 check_msti_state = FALSE;
//    uint32 erps_ok = FALSE;   TODO

    if (NULL == p_db_if)
    {
        return HSRV_E_INVALID_PTR;
    }

    if ((GLB_PKT_BPDU == pkt_type)
     || (GLB_PKT_LACP == pkt_type)
     || (GLB_PKT_CFM == pkt_type)
     || (GLB_PKT_EFM == pkt_type)
     || (GLB_PKT_L2_UDLD == pkt_type)
     || (GLB_PKT_L2_LLDP == pkt_type)
     || (GLB_PKT_L2_LBDT == pkt_type)
     /* modified by wangjj for fix erps bug 42245, 2016-12-21 */
     || (GLB_PKT_ERPS == pkt_type)
     /* modified by wangl for bug 50846, 2019-1-16 */
     || (GLB_PKT_OPENFLOW == pkt_type)
     || GLB_PKT_SSM == pkt_type)/*modified by yejl for bug 46449, 2019-02-26*/
    {
        return HSRV_E_NONE;
    }

    /* for bug 47230, check MSTI fea_state or CIST fea_state */
    if (MSTP_TYPE_MSTP == p_mstp_glb->type)
    {
        if (vid)
        {
            vlan_key.vid = vid;
        }
        else
        {
            if (p_db_if->brgif)
            {
                vlan_key.vid = p_db_if->brgif->pvid;
            }
            else
            {
                /* VLAN interface or routed port need not do STP check */
                stp_ok = TRUE;
                goto CHECK;
            }
        }
        p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
        if (NULL == p_db_vlan)
        {
            stp_ok = TRUE;
            goto CHECK;
        }
        else
        {
            if (0 == p_db_vlan->instance)
            {
                check_msti_state = FALSE;
            }
            else
            {
                check_msti_state = TRUE;
            }
        }
    }
    else
    {
        check_msti_state = FALSE;
    }

    if (check_msti_state)
    {
        sal_memset(&msti_key, 0, sizeof(msti_key));
        sal_strncpy(msti_key.name, p_db_if->key.name, IFNAME_SIZE);
        msti_key.instance = p_db_vlan->instance;
        p_db_msti_port = tbl_msti_port_get_msti_port(&msti_key);
        if (NULL == p_db_msti_port)
        {
            stp_ok = TRUE;
        }
        else
        {
            if ((GLB_BR_PORT_STATE_FORWARDING == p_db_msti_port->fea_state)
             || (GLB_BR_PORT_STATE_DISABLED == p_db_msti_port->fea_state))
            {
                stp_ok = TRUE;
            }
            else
            {    
                stp_ok = FALSE;
            }
        }
    }
    else
    {
        p_db_mstp_port = tbl_mstp_port_get_mstp_port((tbl_mstp_port_key_t*)(&p_db_if->key));
        if (NULL == p_db_mstp_port)
        {
            stp_ok = TRUE;
        }
        else
        {
            if ((GLB_BR_PORT_STATE_FORWARDING == p_db_mstp_port->fea_state)
             || (GLB_BR_PORT_STATE_DISABLED == p_db_mstp_port->fea_state))
            {
                stp_ok = TRUE;
            }
            else
            {    
                stp_ok = FALSE;
            }
        }
    }

CHECK:    
    if (!stp_ok)
    {
        if (tx)
        {
            if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_TX))
            {
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                log_sys(M_MOD_HSRV, E_DEBUG, "Interface %s drop dir %s pkt_type %u packet for STP state check fail", 
                     ifname_ext, "TX", pkt_type);
            }
        }
        else
        {
            if (HSRV_DBG_IS_ON(cpu, CPU_TRAFFIC_RX))
            {
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
                log_sys(M_MOD_HSRV, E_DEBUG, "Interface %s drop dir %s pkt_type %u packet for STP state check fail", 
                     ifname_ext, "RX", pkt_type);
            }
        }
        return HSRV_E_FAIL;
    }
    
    return HSRV_E_NONE;
}

int32 
hsrv_packet_event_notification_insert_ptp(const sai_attribute_t * attr, uint32 vlan_id, ctc_msg_t *req_msg, hsrv_ptp_sockaddr_t *p_ptp, uint32 ifindex)
{

    ptp_common_message_header_t *hdr = NULL;
    uint32 with_vlan_tag = 0;
    uint16* p16 = 0;
    uint16 vlan_cos = 0;
    uint16 vlan = 0;
    uint8 cos = 0;
    uint16 ether_type = 0;
    uint32 eth_ptp = FALSE;
    uint32 udp_ptp = FALSE;
    
    struct ethhdr *eth = (struct ethhdr *)(req_msg->data);
    ether_type = sal_ntoh16(eth->h_proto);
    if (NULL == attr)
    {
        return HSRV_E_FAIL;
    }
    
    eth_ptp = (eth->h_proto == sal_htons(GLB_PTP_1588_ETYPE));
    udp_ptp = (eth->h_proto == sal_htons(GLB_PTP_IPV4_ETYPE));
    if (eth_ptp)
    {
        hdr = (ptp_common_message_header_t *)(req_msg->data + ETH_HEADER_LEN);
    }
    else if (udp_ptp)
    {
        hdr = (ptp_common_message_header_t *)(req_msg->data + ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN);
    }
    else
    {
        /*vlan process*/
        if (sal_htons(0x8100) == eth->h_proto || sal_htons(0x9100) == eth->h_proto || sal_htons(0x88a8) == eth->h_proto)
        {
            with_vlan_tag = TRUE;

            /*get vlan*/
            p16 = (uint16 *) (req_msg->data + ETH_HEADER_LEN);
            vlan_cos = sal_ntoh16(*p16);
            vlan = vlan_cos & 0x0fff;
            cos = vlan_cos >> 13;

            /*get ether type*/
            p16 = (uint16 *) (req_msg->data + ETH_HEADER_LEN + 2);
            ether_type = sal_ntoh16(*p16);
        
            hdr = (ptp_common_message_header_t *)(req_msg->data + ETH_HEADER_LEN + 4);
        }
    }

    p_ptp->seconds = attr->value.ptp.ts.seconds;
    p_ptp->nanoseconds = attr->value.ptp.ts.nanoseconds;
    p_ptp->vlan_id = vlan_id & 0xFFF;/* filter COS field */
    p_ptp->vlan_priority = vlan_id >> 13;

    // TODO:ggsys中从umpeth_rx.c的priv_proc_ptp中的eth头中获取
    //p_ptp->ether_type = ;
    p_ptp->ifindex = ifindex;
    #if 0
    vlan = (peth_vlanhdr_t *)(eth + 1);
        tag = sal_ntohs(vlan->tag);
        p_decap_info->peth_hdr.outer_vid = tag & 0xFFF;
        p_decap_info->outer_cos = tag >> 13;
    #endif

    /*skaddr*/

    if (NULL != hdr)
    {
        if (TRUE == with_vlan_tag)
        {
            HSRV_PTP_RX_DEBUG("With vlan tag (vlan%hu cos%hhu), msg_type(%s) ether_type (%hu), seconds(%d), nanoseconds(%d), vlan_id(%hu), vlan_priority(%hhu), ifindex(%d)", 
                vlan, cos, ptp_desc_msg_type(hdr->message_type), ether_type, p_ptp->seconds, p_ptp->nanoseconds, p_ptp->vlan_id, p_ptp->vlan_priority, p_ptp->ifindex);
        }
        else
        {
            HSRV_PTP_RX_DEBUG("msg_type(%s), ether_type (%hu), seconds(%d), nanoseconds(%d), vlan_id(%hu), vlan_priority(%hhu), ifindex(%d)", 
                ptp_desc_msg_type(hdr->message_type), ether_type, p_ptp->seconds, p_ptp->nanoseconds, p_ptp->vlan_id, p_ptp->vlan_priority, p_ptp->ifindex);
        }
    }
        
    return HSRV_E_NONE;
}

char*
tpoam_api_service_type_desc(uint32 type, tpoam_pw_type_t pw_type)
{
    switch (type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        return "Section";
    case GLB_OAM_TYPE_MPLS_LSP:
        return "LSP";
    case GLB_OAM_TYPE_MPLS_PW:
        switch (pw_type)
        {
        case TPOAM_PW_TYPE_SPE:
            return "PW SPE";
        case TPOAM_PW_TYPE_TPE:
            return "PW TPE";            
        default:
            return "PW";
        }
    default:
        return "Error";
    }
}

/* MACRO for LABEL parser */
#define TPOAM_MPLS_LABEL_BYTE_WIDTH         4
#define GLB_MPLS_TP_ALERT_LABEL          13
#define GLB_MPLS_OAM_PW_MAC_WD_TYPE       0x0028

#define TPOAM_GET_LABEL_ID(_label_) \
(((_label_) >> 12) & 0xFFFFF)

#define TPOAM_GET_LABEL_EXP(_label_) \
(((_label_) >> 9) & 0x7)

#define TPOAM_GET_LABEL_S(_label_) \
(((_label_) >> 8) & 0x1)

#define TPOAM_GET_LABEL_TTL(_label_) \
((_label_) & 0xFF)

#ifndef TTPPRODUCT
static int32 
_hsrv_tpoam_pdu_decode_pdu_label
(uint8 *p_data, mpls_label_t* p_label, mpls_label_t *p_outer_label, uint32 *p_service_type, uint32 *paser_label_length, uint32 *p_label_num)
{
    uint32 service_type = 0;
    uint32 current_label = 0;
    uint32 next_label = 0;
    uint32 service_label = 0;
    uint32 label_num = 0;
    bool end_parser = FALSE;
    bool sbit = FALSE;
    mpls_label_t label_13_not_standard = {0};
    
    current_label = sal_ntohl(*(uint32 *)p_data);
    sbit = GLB_IS_BIT_SET(current_label, 8);
    p_data += TPOAM_MPLS_LABEL_BYTE_WIDTH;
    *paser_label_length += TPOAM_MPLS_LABEL_BYTE_WIDTH;
    next_label = sal_ntohl(*(uint32 *)p_data);
    label_num = 1;
    
    while (!end_parser)
    {
        if (GLB_MPLS_TP_ALERT_LABEL == TPOAM_GET_LABEL_ID(current_label))
        {
            HSRV_TPOAM_PKT_RX_DEBUG("section label %u 0x%x\n", TPOAM_GET_LABEL_ID(current_label), current_label);
            /* section */
            end_parser = TRUE;
            service_label = current_label;
            service_type = GLB_OAM_TYPE_MPLS_SECTION;
        }
        else if ((!sbit) && (GLB_MPLS_TP_ALERT_LABEL == TPOAM_GET_LABEL_ID(next_label)) && (1 == label_num))
        {
            /* lsp */
            HSRV_TPOAM_PKT_RX_DEBUG("lsp label %u 0x%x, next label %u 0x%x\n", 
                TPOAM_GET_LABEL_ID(current_label), current_label, TPOAM_GET_LABEL_ID(next_label), next_label);
            end_parser = TRUE;
            service_label = current_label;
            service_type = GLB_OAM_TYPE_MPLS_LSP;
            p_data += TPOAM_MPLS_LABEL_BYTE_WIDTH;
            *paser_label_length += TPOAM_MPLS_LABEL_BYTE_WIDTH;
        }
        /*end label && next label is GACH 1000 8902*/
        else if (sbit && ((next_label >> 28) == 0x0001))
        {
            /* pw */
            end_parser = TRUE;
            service_label = current_label;
            /* modified by huwx for bug 20931, 2012.10.17 */
            if ((next_label & 0xFFFF) == GLB_MPLS_OAM_PW_MAC_WD_TYPE)
            {
                HSRV_TPOAM_PKT_RX_DEBUG("Not support, label %u 0x%x\n", 
                    TPOAM_GET_LABEL_ID(current_label), current_label);
                // not support
                //service_type = GLB_OAM_TYPE_PW_MAC_WD;
            }
            else
            {
                HSRV_TPOAM_PKT_RX_DEBUG("PW label %u 0x%x\n", TPOAM_GET_LABEL_ID(current_label), current_label);
                service_type = GLB_OAM_TYPE_MPLS_PW;
            }
        }
        /*not end label && next label is label 13 && next label is end label*/
        else if (!sbit && (GLB_MPLS_TP_ALERT_LABEL == TPOAM_GET_LABEL_ID(next_label)) && (1 == TPOAM_GET_LABEL_S(next_label)) && (2 == label_num))
        {
            HSRV_TPOAM_PKT_RX_DEBUG("PW label %u 0x%x, is not standard mode %u, label13 %u 0x%x\n", 
                TPOAM_GET_LABEL_ID(current_label), current_label, !hsrv_openflow_tpoam_api_get_pw_mode(), TPOAM_GET_LABEL_ID(next_label), next_label);
            label_num++;
            label_13_not_standard.exp = TPOAM_GET_LABEL_EXP(next_label);
            label_13_not_standard.label = TPOAM_GET_LABEL_ID(next_label);
            label_13_not_standard.s = TPOAM_GET_LABEL_S(next_label);
            label_13_not_standard.ttl  = TPOAM_GET_LABEL_TTL(next_label);
            end_parser = TRUE;
            service_label = current_label;
            service_type = GLB_OAM_TYPE_MPLS_PW;
            *paser_label_length += TPOAM_MPLS_LABEL_BYTE_WIDTH;
        }
        else
        {
            HSRV_TPOAM_PKT_RX_DEBUG("Outer label %u 0x%x\n", TPOAM_GET_LABEL_ID(current_label), current_label);
            label_num++;
            p_outer_label->label = TPOAM_GET_LABEL_ID(current_label);
            p_outer_label->exp = TPOAM_GET_LABEL_EXP(current_label);
            p_outer_label->s = TPOAM_GET_LABEL_S(current_label);
            p_outer_label->ttl = TPOAM_GET_LABEL_TTL(current_label);
            current_label = next_label;
            end_parser = sbit || (label_num > 4);
            p_data += TPOAM_MPLS_LABEL_BYTE_WIDTH;
            *paser_label_length += TPOAM_MPLS_LABEL_BYTE_WIDTH;
            next_label = sal_ntohl(*(uint32 *)p_data);
            sbit = GLB_IS_BIT_SET(current_label, 8);
        }
    }

    *p_service_type = service_type;
    p_label->label = TPOAM_GET_LABEL_ID(service_label);
    p_label->exp = TPOAM_GET_LABEL_EXP(service_label);
    p_label->s = TPOAM_GET_LABEL_S(service_label);
    p_label->ttl = TPOAM_GET_LABEL_TTL(service_label);
    label_num = label_num;
    *p_label_num = label_num;

    if (label_num == 2)
    {
        HSRV_TPOAM_PKT_RX_DEBUG("%s RX %u label 0x%08X (label %d, exp %d, s %d, ttl %d), outlabel (label %d, exp %d, s %d, ttl %d), paser label length %u", 
            tpoam_api_service_type_desc(service_type, TPOAM_PW_TYPE_MAX), 
            label_num,
            service_label, 
            p_label->label, p_label->exp, p_label->s, p_label->ttl, 
            p_outer_label->label, p_outer_label->exp, p_outer_label->s, p_outer_label->ttl,
            *paser_label_length);
    }
    else if (label_num == 3)
    {
        HSRV_TPOAM_PKT_RX_DEBUG("%s RX %u label 0x%08X (label %d, exp %d, s %d, ttl %d), outlabel (label %d, exp %d, s %d, ttl %d), label13 (label %d, exp %d, s %d, ttl %d),paser label length %u", 
            tpoam_api_service_type_desc(service_type, TPOAM_PW_TYPE_MAX), 
            label_num,
            service_label, 
            p_label->label, p_label->exp, p_label->s, p_label->ttl, 
            p_outer_label->label, p_outer_label->exp, p_outer_label->s, p_outer_label->ttl,
            label_13_not_standard.label, label_13_not_standard.exp, label_13_not_standard.s, label_13_not_standard.ttl,
            *paser_label_length);
    }
    else
    {
        HSRV_TPOAM_PKT_RX_DEBUG("%s RX %u label 0x%08X (label %d, exp %d, s %d, ttl %d), paser label length %u", 
            tpoam_api_service_type_desc(service_type, TPOAM_PW_TYPE_MAX), 
            label_num,
            service_label,
            p_label->label, p_label->exp, p_label->s, p_label->ttl,
            *paser_label_length);
    }

    return HSRV_E_NONE;
}
#endif

#ifndef TTPPRODUCT
int32 
hsrv_packet_event_notification_insert_tpoam(const sai_attribute_t * attr, uint32 vlan_id, ctc_msg_t *req_msg, hsrv_sockaddr_tpoam_t *p_tpoam, uint32 ifindex)
{
    //uint32 with_vlan_tag = 0;
    uint16* p16 = 0;
    //uint16 vlan_cos = 0;
    //uint16 vlan = 0;
    //uint8 cos = 0;
    //uint32 eth_tpoam = FALSE;
    mpls_label_t label;
    mpls_label_t outer_label;
    uint32 service_type;
    uint32 paser_label_length = 0;
    uint32 oam_session_id = 0;

    uint16 vlan_tmp = 0;
    uint8 cos_tmp = 0;
    
    if (NULL == attr)
    {
        return HSRV_E_FAIL;
    }
    sal_memset(&outer_label, 0, sizeof(mpls_label_t));
    p_tpoam->priority = attr->value.tpoam.priority;
    p_tpoam->vlanid = attr->value.tpoam.vid;
    vlan_tmp = vlan_id & 0xFFF;/* filter COS field */
    cos_tmp = vlan_id >> 13;
    p_tpoam->RxFcl = attr->value.tpoam.lm_fcl;

    if (vlan_tmp != p_tpoam->vlanid || cos_tmp != p_tpoam->priority)
    {
        /*vlan_tmp is 1, vlanid is 0*/
        //HSRV_LOG_ERR("vlan_tmp %hu, vlanid %hu, cos_tmp %hu, priority %hu\n", vlan_tmp, p_tpoam->vlanid, cos_tmp, p_tpoam->priority);
    }
    //HSRV_LOG_ERR("vlan_tmp %hu, vlanid %hu, cos_tmp %hu, priority %hu\n", vlan_tmp, p_tpoam->vlanid, cos_tmp, p_tpoam->priority);
    p_tpoam->port = ifindex;
    #if 0
    vlan = (peth_vlanhdr_t *)(eth + 1);
        tag = sal_ntohs(vlan->tag);
        p_decap_info->peth_hdr.outer_vid = tag & 0xFFF;
        p_decap_info->outer_cos = tag >> 13;
    #endif
    uint8 *skb_data = NULL;
    uint32 key = 0;
    uint32 decode_label_offset = ETH_HEADER_LEN;
    uint32 label_num = 0;
    uint32 mv_vlan_length = 0;
    uint32 has_vlan_tag = FALSE;
    uint16 ether_type = 0;
    eth_hdr_t *eth = (eth_hdr_t *)(req_msg->data);
    ether_type = sal_ntoh16(eth->ether_type);

    if (0x8100 == ether_type || 0x9100 == ether_type || 0x88a8 == ether_type)
    {
        skb_data = req_msg->data;
        skb_data += ETH_HEADER_LEN;
        skb_data += 2;
        
        mv_vlan_length += 4;
        has_vlan_tag = TRUE;
        
        p16 = (uint16 *) (skb_data);
        ether_type = sal_ntoh16(*p16);
        if (0x8100 == ether_type || 0x9100 == ether_type || 0x88a8 == ether_type)
        {
            mv_vlan_length += 4;
            HSRV_TPOAM_PKT_RX_DEBUG("with two vlan tag");
        }
        else if (0x8847 == ether_type)
        {
            HSRV_TPOAM_PKT_RX_DEBUG("with one vlan tag, next ether type is 0x8847");
        }
        else
        {
            HSRV_TPOAM_PKT_RX_DEBUG("ERR with one vlan tag, next ether type is 0x%x", ether_type);
        }
    }
    
    _hsrv_tpoam_pdu_decode_pdu_label(req_msg->data + decode_label_offset + mv_vlan_length, 
        &label, &outer_label, &service_type, &paser_label_length, &label_num);

    if (has_vlan_tag)
    {
        paser_label_length += mv_vlan_length;
    }
    
    if (GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP == service_type)
    {
        key = ifindex;
    }
    else
    {
        key = label.label;
    }
    
    hsrv_openflow_tpoam_api_get_oam_session_id(service_type, key, &oam_session_id);
    p_tpoam->had_paser_label = TRUE;
    p_tpoam->oam_session_id = oam_session_id;
    p_tpoam->service_type = service_type;
    p_tpoam->paser_label_length = paser_label_length;
    p_tpoam->label_num = label_num;
    sal_memcpy(&p_tpoam->label, &label, sizeof(label));
    sal_memcpy(&p_tpoam->outer_label, &outer_label, sizeof(outer_label));
    
    return HSRV_E_NONE;
}
#endif


int32 
hsrv_packet_event_notification_insert_sflow(ctc_msg_t *req_msg, struct nexthop_data *p_nexthop) //add by yangl 20180718
{
    //struct nexthop_data *p_nexthop = NULL;//nethop struct  
    //struct nexthop_data  nexthop;
    addr_ipv4_t  dst_addr;
    ip_v4_hdr  *  ip_hdr = NULL;
    addr_ipv4_t  pst_nexthop;
    uint32 mask_len = 0;  
    uint16 ether_type = 0;
    eth_hdr_t * eth = NULL;
    vlan_hdr_t  *vlan_hdr = NULL;
    uint32 temp_t_1=0;
    uint32 temp_t_2 = 0;

    if (!req_msg || !req_msg->data)
    {
        return HSRV_E_NONE;    
    }

    eth = (eth_hdr_t *)(req_msg->data);
    ether_type = sal_ntoh16(eth->ether_type);
    if (0x8100 == ether_type )
    {
        vlan_hdr = (vlan_hdr_t*)(eth + 1);
        ip_hdr   = (ip_v4_hdr *)(vlan_hdr+1); 
        dst_addr = (ip_hdr->dst_ip_addr); 
    }
    else if(0x0800 == ether_type )
    {
        ip_hdr   = (ip_v4_hdr  *)(eth+1);
        temp_t_1 = *(uint32*)(&(ip_hdr->dst_ip_addr));
        temp_t_2 = sal_ntoh32(temp_t_1);
        dst_addr = *(addr_ipv4_t *)(&temp_t_2);
    }
    else
    {
        return HSRV_E_NONE;
    }

    /*get nexthop*/
    pst_nexthop.s_addr = 0; 
    hsrv_route_api_get_nexthop_by_addr4(&dst_addr, &mask_len, &pst_nexthop);
    p_nexthop->nexthop  = (pst_nexthop);
    p_nexthop->src_mask = mask_len;
    p_nexthop->dst_mask = mask_len; 
    /*skaddr*/
    return HSRV_E_NONE;
}

int32
l2protocol_pdu_forward_prepare_edit(uint32 type, uint32 is_edit, uint8 *data_l2protocol, uint8 *data_l2protocol_tag, 
        uint8* group_mac, vid_t vlan_id, uint32 index, uint8 *buffer, uint32 buffer_size)
{
    tbl_brg_global_t* p_brg_global = tbl_brg_global_get_brg_global();
    int32 size_val = 0;
    uint8*  pnt = NULL;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint16 ether_type = 0;
    uint32 MAC_HDR_LEN = MAC_ADDR_LEN*2;
    uint8  l2proto_cos = p_brg_global->l2protocol_cos;

    size_val = buffer_size;
    pnt = buffer;
    size = &size_val;
    ppnt = &pnt;
    
    /* L2 header */
    MSG_DECODE_GET_EMPTY(MAC_ADDR_LEN*2);
    MSG_DECODE_GETW(ether_type);
    if (GLB_PKT_L2_GROUP_MAC == type)
    {
        if (GLB_ETH_P_8021Q == ether_type)
        {
            sal_memcpy(data_l2protocol, l2protocol_protocol_mac[index], MAC_ADDR_LEN);
            sal_memcpy(data_l2protocol+MAC_ADDR_LEN, buffer+MAC_ADDR_LEN, MAC_ADDR_LEN);
            sal_memcpy(data_l2protocol+MAC_HDR_LEN, buffer+MAC_HDR_LEN+VLAN_TAG_LEN, buffer_size-MAC_HDR_LEN-VLAN_TAG_LEN);
            sal_memcpy(data_l2protocol+MAC_HDR_LEN, buffer+MAC_HDR_LEN+VLAN_TAG_LEN, buffer_size-MAC_HDR_LEN-VLAN_TAG_LEN);
        }
        else
        {
            return -1;
        }
    }
    else if (GLB_PKT_L2_PROTOCOL_MAC == type)
    {
        if (GLB_ETH_P_8021Q != ether_type)
        {
            sal_memcpy(data_l2protocol_tag, group_mac, MAC_ADDR_LEN);
            sal_memcpy(data_l2protocol_tag+MAC_ADDR_LEN, buffer+MAC_ADDR_LEN, MAC_ADDR_LEN);
            data_l2protocol_tag[MAC_HDR_LEN] = ((GLB_ETH_P_8021Q) >> 8) & 0xFF;
            data_l2protocol_tag[MAC_HDR_LEN+1] = (GLB_ETH_P_8021Q) & 0xFF;
            data_l2protocol_tag[MAC_HDR_LEN+2] = (((vlan_id) >> 8) & 0xFF) | (((l2proto_cos << 5) & (0xFF)));
            data_l2protocol_tag[MAC_HDR_LEN+3] = (vlan_id) & 0xFF;
            sal_memcpy(data_l2protocol_tag+MAC_HDR_LEN+VLAN_TAG_LEN, buffer+MAC_HDR_LEN, buffer_size-MAC_HDR_LEN);
        }
        else
        {
            return -2;
        }
    }
    else
    {
        if (is_edit)
        {
            if (GLB_ETH_P_8021Q == ether_type)
            {
                sal_memcpy(data_l2protocol, l2protocol_protocol_mac[index], MAC_ADDR_LEN);
                sal_memcpy(data_l2protocol+MAC_ADDR_LEN, buffer+MAC_ADDR_LEN, MAC_ADDR_LEN);
                sal_memcpy(data_l2protocol+MAC_HDR_LEN, buffer+MAC_HDR_LEN+VLAN_TAG_LEN, buffer_size-MAC_HDR_LEN-VLAN_TAG_LEN);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            if (GLB_ETH_P_8021Q != ether_type)
            {
                sal_memcpy(data_l2protocol_tag, group_mac, MAC_ADDR_LEN);
                sal_memcpy(data_l2protocol_tag+MAC_ADDR_LEN, buffer+MAC_ADDR_LEN, MAC_ADDR_LEN);
                data_l2protocol_tag[MAC_HDR_LEN] = ((GLB_ETH_P_8021Q) >> 8) & 0xFF;
                data_l2protocol_tag[MAC_HDR_LEN+1] = (GLB_ETH_P_8021Q) & 0xFF;
                data_l2protocol_tag[MAC_HDR_LEN+2] = (((vlan_id) >> 8) & 0xFF) | (((l2proto_cos << 5) & (0xFF)));
                data_l2protocol_tag[MAC_HDR_LEN+3] = (vlan_id) & 0xFF;
                sal_memcpy(data_l2protocol_tag+MAC_HDR_LEN+VLAN_TAG_LEN, buffer+MAC_HDR_LEN, buffer_size-MAC_HDR_LEN);
            }
            else
            {
                return -2;
            }
        }
    }
    return HSRV_E_NONE;
}

int32
hsrv_packet_l2proto_forward_common(uint32 type, int32 index, uint32 is_edit, tbl_interface_t *p_db_if, void *buffer, uint32 buffer_size)
{
    int32 rc = HSRV_E_NONE;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if1 = NULL;
    ds_brgif_t * p_brgif = NULL;
    ds_brgif_t * p_brgif1 = NULL;
    mac_addr_t group_mac;
    ctc_msg_t req_msg;
    vid_t vlan_id = 0;
    uint16* p16 = 0;
    uint16 vlan_cos = 0;
    uint16 vlan = 0;
    
    char ifname_ext[IFNAME_SIZE];
    char l2protocol_name[GLB_L2_PROTOCOL_NAME_SIZE];

    uint8 data_l2protocol[CMD_BUF_2048];
    uint8 data_l2protocol_tag[CMD_BUF_2048];
    uint8 data_l2protocol_buffer[CMD_BUF_2048];

    uint32 l2protocol_buffer_len = 0;

    sal_memset(&group_mac, 0, MAC_ADDR_LEN);
    sal_memset(data_l2protocol, 0, sizeof(data_l2protocol));
    sal_memset(data_l2protocol_tag, 0, sizeof(data_l2protocol_tag));
    sal_memset(data_l2protocol_buffer, 0, sizeof(data_l2protocol_buffer));
    sal_memset(l2protocol_name, 0, sizeof(l2protocol_name));

    sal_strncpy(l2protocol_name, l2protocol_protocol_name[index], GLB_L2_PROTOCOL_NAME_SIZE);
    if (0 == sal_strlen(l2protocol_name))
    {
        return 0;
    }
    
    if (type == GLB_PKT_L2_PROTOCOL_MAC)
    {
        if (GLB_L2_PROTO_PEER == p_db_if->l2pro_if_action_vlan.l2action[index] ||
            GLB_L2_PROTO_DISCARD == p_db_if->l2pro_if_action_vlan.l2action[index])
        {
            HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s protocol-action is peer or discard on %s", 
                l2protocol_name, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return -1;
        }
    }
    else if (type == GLB_PKT_L2_GROUP_MAC)
    {
        if (!p_db_if->l2pro_uplink) 
        {
            HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s is not uplink port", 
                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return -1;
        }
        if (GLB_L2_PROTO_DISCARD == p_db_if->l2pro_if_action_vlan.l2action[index])
        {
            HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s protocol-action is discard on %s", 
                l2protocol_name, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return -1;
        }
        
        if (p_db_if->vlan_type != GLB_VLAN_PORT_TYPE_TRUNK)
        {
            HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: Interface %s is not trunk port", 
                            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return -1;
        }
        /*get vlan*/
        p16 = (uint16 *)(buffer + ETH_HEADER_LEN);
        vlan_cos = sal_ntoh16(*p16);
        vlan = vlan_cos & 0x0fff;
        
        p_brgif = p_db_if->brgif;
        if (p_brgif)
        {
            if (!GLB_BMP_ISSET(p_brgif->allowed_vlan.vlan_bmp, vlan))
            {
                HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: Interface %s don't allow vlan %d", 
                                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vlan);
                return -1;
            }
        }
    }
    else
    {
        if (is_edit)
        {
            if (!p_db_if->l2pro_uplink) 
            {
                HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s is not uplink port", 
                    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                return -1;
            }
            if (GLB_L2_PROTO_DISCARD == p_db_if->l2pro_if_action_vlan.l2action[index])
            {
                HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s protocol-action is discard on %s", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                return -1;
            }
            if (p_db_if->vlan_type != GLB_VLAN_PORT_TYPE_TRUNK)
            {
                HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: Interface %s is not trunk port", 
                                IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                return -1;
            }
            /*get vlan*/
            p16 = (uint16 *)(buffer + ETH_HEADER_LEN);
            vlan_cos = sal_ntoh16(*p16);
            vlan = vlan_cos & 0x0fff;
            
            p_brgif = p_db_if->brgif;
            if (p_brgif)
            {
                if (!GLB_BMP_ISSET(p_brgif->allowed_vlan.vlan_bmp, vlan))
                {
                    HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: Interface %s don't allow vlan %d", 
                                    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), vlan);
                    return -1;
                }
            }
        }
        else
        {
            if (GLB_L2_PROTO_DISCARD == p_db_if->l2pro_if_action_vlan.l2action[index])
            {
                HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: %s protocol-action is discard on %s", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
                return -1;
            }
            else if (GLB_L2_PROTO_PEER == p_db_if->l2pro_if_action_vlan.l2action[index])
            {
                return 0;
            }
        }
    }

    vlan_id = p_db_if->l2pro_if_action_vlan.l2vlan[index];
    if (!vlan_id)
    {
        vlan_id = p_db_if->brgif->pvid;
    }
    sal_memcpy(group_mac, l2protocol_group_mac[index], MAC_ADDR_LEN);
    if (GLB_IS_ZERO_MAC(group_mac))
    {
        HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: protocol %s group mac is zero mac", l2protocol_name);
        return -1;
    }
    sal_memset(data_l2protocol_buffer, 0, sizeof(data_l2protocol_buffer));
    #ifdef _GLB_UML_SYSTEM_
    sal_memcpy(data_l2protocol_buffer, buffer, buffer_size);
    l2protocol_buffer_len = buffer_size;
    #else
    sal_memcpy(data_l2protocol_buffer, buffer, buffer_size-4);  
    l2protocol_buffer_len = buffer_size - 4;
    #endif
    
    rc = l2protocol_pdu_forward_prepare_edit(type, is_edit, data_l2protocol, data_l2protocol_tag, group_mac, vlan_id, index, data_l2protocol_buffer, l2protocol_buffer_len);
    if (rc == -1)
    {
        HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: protocol %s has redirected to cpu by group-mac but has no tag", l2protocol_name);
        return rc;
    }
    else if (rc == -2)
    {
        HSRV_L2PROTOCOL_PKT_RX_DEBUG("L2protocol rx fail: original protocol %s has redirected to cpu but has tag", l2protocol_name);
        return rc;
    }
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if1, listnode)
    {
        if (p_db_if1->ifindex == p_db_if->ifindex)
        {
            continue;
        }
        if (!tbl_interface_is_running(p_db_if1))
        {
            continue;
        }
        if (GLB_IF_MODE_L2 != p_db_if1->mode)
        {
            continue;
        }
        if (GLB_FLAG_ISSET(p_db_if1->mirror_enable, MIRROR_DEST_FLAG))
        {
            continue;
        }
        if ((GLB_IF_TYPE_PORT_IF != p_db_if1->hw_type) && 
            (GLB_IF_TYPE_LINKAGG_IF != p_db_if1->hw_type))
        {
            continue;
        }
        sal_memset(&req_msg, 0, sizeof(req_msg));
        req_msg.u_hdr.pkt.ifindex = p_db_if1->ifindex;
        req_msg.u_hdr.pkt.sub_type = 0;
        req_msg.u_hdr.pkt.flags   = 0;
        
        if (p_db_if1->l2pro_uplink)
        {
            /*added by yejl for bug 48104, 2018-07-26*/
            /*If the uplink port's action is discard, then don't send to the port*/
            if (GLB_L2_PROTO_DISCARD == p_db_if1->l2pro_if_action_vlan.l2action[index])
            {
                continue;
            }
            /*If the uplink port is not trunk port, then don't send to the port*/
            if (p_db_if1->vlan_type != GLB_VLAN_PORT_TYPE_TRUNK)
            {
                HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: Interface %s is not trunk port", 
                                IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                continue;
            }
            p_brgif1 = p_db_if1->brgif;         
            
            if (type == GLB_PKT_L2_GROUP_MAC)
            {
                /*If the uplink port is not allowed the vlan, then don't send to the port*/
                if (!GLB_BMP_ISSET(p_brgif1->allowed_vlan.vlan_bmp, vlan))
                {
                    HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: Interface %s don't allow vlan %d", 
                                    IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext), vlan);
                    continue;
                }
                req_msg.u_hdr.pkt.type    = type;
                req_msg.u_hdr.pkt.vid     = 0;
                req_msg.data_len          = l2protocol_buffer_len;
                req_msg.data              = (void*)data_l2protocol_buffer;
            }
            else if (type == GLB_PKT_L2_PROTOCOL_MAC)
            {
                /*If the uplink port is not allowed the vlan, then don't send to the port*/
                if (!GLB_BMP_ISSET(p_brgif1->allowed_vlan.vlan_bmp, vlan_id))
                {
                    HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: Interface %s don't allow vlan %d", 
                                    IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext), vlan_id);
                    continue;
                }
                req_msg.u_hdr.pkt.type    = type;
                req_msg.u_hdr.pkt.vid     = vlan_id & 0xFFF;    /* filter COS field */
                req_msg.data_len          = l2protocol_buffer_len+VLAN_TAG_LEN;
                req_msg.data              = (void*)data_l2protocol_tag;
            }
            else
            {
                if (is_edit)
                {
                    /*If the uplink port is not allowed the vlan, then don't send to the port*/
                    if (!GLB_BMP_ISSET(p_brgif1->allowed_vlan.vlan_bmp, vlan))
                    {
                        HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: Interface %s don't allow vlan %d", 
                                        IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext), vlan);
                        continue;
                    }
                    /*If the packet is lldp, dot1x, slow-protocol, and the uplink port is agg port, don't send to the port*/
                    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if1->hw_type)
                    {
                        HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: L2protocol %s can't sent to linkagg port %s", 
                                            l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                        continue;
                    }
                    req_msg.u_hdr.pkt.type    = type;
                    req_msg.u_hdr.pkt.vid     = 0;    /* filter COS field */
                    req_msg.data_len          = l2protocol_buffer_len;
                    req_msg.data              = (void*)data_l2protocol_buffer;
                }
                else
                {
                    /*If the uplink port is not allowed the vlan, then don't send to the port*/
                    if (!GLB_BMP_ISSET(p_brgif1->allowed_vlan.vlan_bmp, vlan_id))
                    {
                        HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: Interface %s don't allow vlan %d", 
                                        IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext), vlan_id);
                        continue;
                    }
                    /*If the packet is lldp, dot1x, slow-protocol, and the uplink port is agg port,  don't send to the port*/
                    if (type != GLB_PKT_BPDU)
                    {
                        if (GLB_IF_TYPE_LINKAGG_IF == p_db_if1->hw_type)
                        {
                            HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: L2protocol %s can't sent to linkagg port %s", 
                                                l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                            continue;
                        }
                    }
                    req_msg.u_hdr.pkt.type    = type;
                    req_msg.u_hdr.pkt.vid     = vlan_id & 0xFFF;    /* filter COS field */
                    req_msg.data_len          = l2protocol_buffer_len+VLAN_TAG_LEN;
                    req_msg.data              = (void*)data_l2protocol_tag;
                }
            }
            rc = hsrv_packet_event_send(1, &req_msg);
            if (rc < 0)
            {
                HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: protocol %s send to uplink port %s fail!", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                return rc;
            }
            HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx success: protocol %s send to uplink port %s success!", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
        }
        else if (GLB_L2_PROTO_TUNNEL == p_db_if1->l2pro_if_action_vlan.l2action[index])
        {
            if (type == GLB_PKT_L2_GROUP_MAC)
            {
                req_msg.u_hdr.pkt.type    = type;
                req_msg.u_hdr.pkt.vid     = 0;    /* filter COS field */
                req_msg.data_len          = l2protocol_buffer_len-VLAN_TAG_LEN;
                req_msg.data              = (void*)data_l2protocol;
            }
            else if (type == GLB_PKT_L2_PROTOCOL_MAC)
            {
                req_msg.u_hdr.pkt.type    = type;
                req_msg.u_hdr.pkt.vid     = 0;
                req_msg.data_len          = l2protocol_buffer_len;
                req_msg.data              = (void*)data_l2protocol_buffer;
            }
            else
            {
                if (is_edit)
                {
                    /*If the packet is lldp, dot1x, slow-protocol, and the tunnel port is agg port, don't send to the port*/
                    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if1->hw_type)
                    {
                        HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: L2protocol %s can't sent to linkagg port %s", 
                                            l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                        continue;
                    }
                    req_msg.u_hdr.pkt.type    = type;
                    req_msg.u_hdr.pkt.vid     = 0;    /* filter COS field */
                    req_msg.data_len          = l2protocol_buffer_len-VLAN_TAG_LEN;
                    req_msg.data              = (void*)data_l2protocol;
                }
                else
                {
                    /*If the packet is lldp, dot1x, slow-protocol, and the tunnel port is agg port, don't send to the port*/
                    if (type != GLB_PKT_BPDU)
                    {
                        if (GLB_IF_TYPE_LINKAGG_IF == p_db_if1->hw_type)
                        {
                            HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: L2protocol %s can't sent to linkagg port %s", 
                                                l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                            continue;
                        }
                    }
                    req_msg.u_hdr.pkt.type    = type;
                    req_msg.u_hdr.pkt.vid     = 0;    /* filter COS field */
                    req_msg.data_len          = l2protocol_buffer_len;
                    req_msg.data              = (void*)data_l2protocol_buffer;
                }
            }
            rc = hsrv_packet_event_send(1, &req_msg);
            if (rc < 0)
            {
                HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx fail: protocol %s send to tunnel port %s fail!", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
                return rc;
            }
            HSRV_L2PROTOCOL_PKT_TX_DEBUG("L2protocol tx success: protocol %s send to tunnel port %s success!", 
                    l2protocol_name, IFNAME_ETH2FULL(p_db_if1->key.name, ifname_ext));
        }
    }
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_packet_event_notification 
 * Purpose      : receive trap packet and process     
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/

void 
hsrv_packet_event_notification(
    void *buffer, 
    sai_size_t buffer_size, 
    uint32_t attr_count,
    const sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr_vlan = NULL;
    const sai_attribute_t *attr_trap = NULL;
    const sai_attribute_t *attr_port = NULL;
    const sai_attribute_t *attr_lag = NULL;
    const sai_attribute_t *attr_subtype = NULL;
    ctc_msg_t req_msg;
    uint32 pm_id = PM_ID_INVALID;
    uint32 pm_id1 = PM_ID_INVALID;
    uint32 pm_id2 = PM_ID_INVALID;
    uint32 pm_id3 = PM_ID_INVALID;
    uint32 pm_id4 = PM_ID_INVALID;
    uint32 type  = 0;
    uint32 sub_type  = 0;
    uint16 vlan_id = 0;
    int32 index = 0;
    int32 rc = 0;
    

    tbl_fea_port_if_t* p_port_if = NULL;
    tbl_fea_port_if_key_t  fea_port_if_key;
    tbl_interface_t* p_db_if = NULL;
    hsrv_sockaddr_tpoam_t *p_tpoam = NULL;
    hsrv_sockaddr_tpoam_t tpoam;
    struct nexthop_data *p_nexthop = NULL;//nethop struct  
    struct nexthop_data  nexthop;
    hsrv_ptp_sockaddr_t *p_ptp = NULL;
    hsrv_ptp_sockaddr_t  ptp;
    glb_openflow_msg_tail_t* p_tail = NULL;
    glb_openflow_msg_tail_t  tail;
    /* Added by weizj for tunnel */
    uint16 ofport = 0;
    uint32 tun_id = 0;
    uint8 need_send_openflow =FALSE;

    attr_trap = attr_list;
    attr_port = attr_trap + 1;
    attr_vlan = attr_trap + 2;
    attr_lag = attr_trap + 3;

    /*for slow protocol SyncE SSM, OAM and LACP*/
    if (SAI_HOSTIF_TRAP_ID_LACP == attr_trap->value.s32)
    {
        _hsrv_packet_get_proc_slow_proto(buffer, &pm_id, &type);
        if (type == GLB_PKT_UNKNOWN) /*added by yejl for bug 53013, 2019-07-31*/
        {
            return ;
        }
    }
    else
    {
        _hsrv_packet_event_get_pm_id(attr_trap->value.s32, &pm_id, &pm_id1, &pm_id2, &pm_id3, &pm_id4, &type);
        if (type == GLB_PKT_UNKNOWN) /*added by yejl for bug 53013, 2019-07-31*/
        {
            return ;
        }
    }

    if (type == GLB_PKT_RAPS)
    {
        if (raps_count > MAX_RAPS_PKT_REC_PER_SEC)
        {
            return;
        }
        raps_count++;
    }
    
    /*added by yejl*/
    if (syslimit_memory_threshold_check(SYSLIMIT_MEMCHECK_STATE_WARNING) != 0)
    {
        if (type == GLB_PKT_LACP || type == GLB_PKT_IGMP || type == GLB_PKT_ARP || 
            type == GLB_PKT_DHCP || type == GLB_PKT_L2_LLDP || type == GLB_PKT_ERPS)
        {
            return;    
        }
    }

    /* modified by liwh for bug 38926, 2016-06-28 */
    if (SAI_HOSTIF_TRAP_ID_DHCPV6 == attr_trap->value.s32)
    {
        return;        
    }
    /* liwh end */

    if (type == GLB_PKT_LACP || type == GLB_PKT_L2_LLDP || type == GLB_PKT_SSM)
    {
        if(attr_port->id == SAI_HOSTIF_PACKET_INGRESS_PORT)
        {
            fea_port_if_key.portid = attr_port->value.oid;
        }
        if(hsrv_openflow_phyport_is_openflow_port_by_portid(attr_port->value.oid))
        {
            /*modified by wangl for hybrid, now LACP is sended to local*/
            if(type == GLB_PKT_L2_LLDP)
            {
                need_send_openflow = TRUE;
                pm_id = PM_ID_OPENFLOW;
            }
            
        }
    }
    else
    {
        if((attr_count >= 4) && attr_lag->id == SAI_HOSTIF_PACKET_INGRESS_LAG)
        {
            fea_port_if_key.portid = attr_lag->value.oid;
        }
        else if(attr_port->id == SAI_HOSTIF_PACKET_INGRESS_PORT)
        {
            fea_port_if_key.portid = attr_port->value.oid;
        }

        if (attr_vlan->id == SAI_HOSTIF_PACKET_VLAN_ID)
        {
            vlan_id = attr_vlan->value.u16;
        }
    }

    /* support sflow by liwh for bug 43981, 2017-06-05 */
    if (type == GLB_PKT_SFLOW)
    {
        if (attr_lag->id == SAI_HOSTIF_PACKET_INGRESS_LAG)
        {
            attr_subtype = attr_trap + 4;
        }
        else
        {
            attr_subtype = attr_trap + 3;
        }        
    }
    /* liwh end */

    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(fea_port_if_key.portid))
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(GLB_CPU_IFINDEX_MIN);
        if (NULL == p_db_if)
        {
            return;
        }
    }
    else
    {
        p_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key); 
        if(NULL == p_port_if)
        {
            return;
        }

        p_db_if = tbl_interface_get_interface_by_ifindex(p_port_if->ifindex);
        if (NULL == p_db_if)
        {
            return;
        }

        /*Added by chenc for bug 51924, 2019-06-05*/
        if (GLB_PKT_ARP == type && 0 != vlan_id && p_db_if->brgif)
        {
            if (!GLB_BMP_ISSET(p_db_if->brgif->allowed_vlan.vlan_bmp, vlan_id))
            {
                return;
            }
        }
        /*chenc end*/
    }
    
    /* Added by shengx for cpu mirror, 2018-08-17*/
    if (GLB_PKT_CPU_MIRROR == type)
    {
        hsrv_mirror_to_cpu_rx_packet(buffer, buffer_size, p_db_if);
        return;
    }

    /* Added by kcao for bug 40778, add STP/ERPS status check */
    rc = hsrv_hostif_port_state_check(FALSE, type, vlan_id, p_db_if);
    if (rc < 0)
    {
        return;
    }
    
    //l2protocol start
    if (type == GLB_PKT_LACP || type == GLB_PKT_L2_LLDP || type == GLB_PKT_SSM || 
        type == GLB_PKT_EAPOL || type == GLB_PKT_EFM || type == GLB_PKT_L2_LLDP ||
        type == GLB_PKT_BPDU || type == GLB_PKT_L2_PROTOCOL_MAC || type == GLB_PKT_L2_GROUP_MAC)
    {
        uint32 is_edit = FALSE;
        uint32 is_peer = FALSE;
        if (type == GLB_PKT_L2_PROTOCOL_MAC)
        {
            _hsrv_packet_get_index_from_protocol_mac_type(&index, &is_edit, buffer);
            hsrv_packet_l2proto_forward_common(type, index, is_edit, p_db_if, buffer, buffer_size);
            return;
        }
        else if (type == GLB_PKT_L2_GROUP_MAC)
        {
            _hsrv_packet_get_index_from_group_mac_type(&index, &is_edit, buffer);
            hsrv_packet_l2proto_forward_common(type, index, is_edit, p_db_if, buffer, buffer_size);
            return;
        }
        else
        {
            _hsrv_packet_get_index_from_standard_type(type, &index, &is_edit, buffer);
            if (GLB_L2_PROTO_PEER == p_db_if->l2pro_if_action_vlan.l2action[index])
            {
                is_peer = TRUE;
            }
            hsrv_packet_l2proto_forward_common(type, index, is_edit, p_db_if, buffer, buffer_size);
            if (!is_peer || is_edit)
            {
                return;
            }
        }
    }
    /*l2protocol end*/

    sal_memset(&req_msg, 0, sizeof(ctc_msg_t));
    if(pm_id == PM_ID_OPENFLOW)
    {
        MSG_BUILD_HDR(&req_msg, MSG_OPER_PKT, buffer_size + sizeof(glb_openflow_msg_tail_t));
    }
    else if (pm_id == PM_ID_PTP)
    {
        MSG_BUILD_HDR(&req_msg, MSG_OPER_PKT, buffer_size + sizeof(hsrv_ptp_sockaddr_t));
    }
    else if (pm_id == PM_ID_OPM && type == GLB_PKT_TPOAM)
    {
        MSG_BUILD_HDR(&req_msg, MSG_OPER_PKT, buffer_size + sizeof(hsrv_sockaddr_tpoam_t));
    }
    else if (pm_id == PM_ID_SWITCH  && type == GLB_PKT_SFLOW)
    {
        MSG_BUILD_HDR(&req_msg, MSG_OPER_PKT, buffer_size + sizeof(struct nexthop_data));
    }
    else
    {
        MSG_BUILD_HDR(&req_msg, MSG_OPER_PKT, buffer_size);
    }
    
    if (SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT == attr_trap->value.s32)
    {
        sub_type = GLB_PORT_SECURITY_VIO_TYPE_PORT_MACLIMIT_DISCARD;
    }
    else if (SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_MISMATCH == attr_trap->value.s32)
    {
        sub_type = GLB_PORT_SECURITY_VIO_TYPE_MACSA_MISMATCH;
    }

    /* support sflow by liwh for bug 43981, 2017-06-05 */
    if (type == GLB_PKT_SFLOW)
    {
        sub_type = attr_subtype->value.u16;
    }
    /* liwh end */
    
    req_msg.u_hdr.pkt.ifindex = p_port_if->ifindex;
    req_msg.u_hdr.pkt.vid     = vlan_id & 0xFFF;    /* filter COS field */
    req_msg.u_hdr.pkt.type    = type;
    req_msg.u_hdr.pkt.sub_type = sub_type;
    req_msg.u_hdr.pkt.flags   = 0;
    req_msg.data_len          = buffer_size;
    req_msg.data              = buffer;

    /* support sflow by liwh for bug 43981, 2017-06-05 */
    if (type == GLB_PKT_SFLOW)
    {
        sal_memset(&nexthop, 0, sizeof(nexthop));
        p_nexthop = &nexthop;
        req_msg.u_hdr.pkt.vid = vlan_id;
        rc = hsrv_packet_event_notification_insert_sflow(&req_msg, p_nexthop);
        if (HSRV_E_NONE != rc)
        {
            return ;
        }
        req_msg.data1_len = sizeof(struct nexthop_data);
        req_msg.data1 = p_nexthop;
    }
    /* liwh end */

    /*for ptp*/
    if (type == GLB_PKT_PTP)
    {
        sal_memset(&ptp, 0, sizeof(ptp));
        p_ptp = &ptp;
        rc = hsrv_packet_event_notification_insert_ptp(&(attr_list[attr_count - 1]), vlan_id, &req_msg, p_ptp,p_port_if->ifindex);
        if (HSRV_E_NONE != rc)
        {
            return ;
        }

        req_msg.data1_len = sizeof(hsrv_ptp_sockaddr_t);
        req_msg.data1 = p_ptp;
        HSRV_PTP_RX_DEBUG("seconds(%d), nanoseconds(%d), vlan_id(%hu), vlan_priority(%hhu), ifindex(%d)", 
            p_ptp->seconds, p_ptp->nanoseconds, p_ptp->vlan_id, p_ptp->vlan_priority, p_ptp->ifindex);
    }

#ifndef TTPPRODUCT
    /*for tpoam*/
    if (type == GLB_PKT_TPOAM)
    {
        sal_memset(&tpoam, 0, sizeof(tpoam));
        p_tpoam = &tpoam;
        rc = hsrv_packet_event_notification_insert_tpoam(&(attr_list[attr_count - 1]), vlan_id, &req_msg, p_tpoam, p_port_if->ifindex);
        if (HSRV_E_NONE != rc)
        {
            return ;
        }
        req_msg.data1_len = sizeof(hsrv_sockaddr_tpoam_t);
        req_msg.data1 = p_tpoam;
        HSRV_TPOAM_PKT_RX_DEBUG("RxFcl(%u), vlan_id(%hu), vlan_priority(%hhu), ifindex(%d)", 
            p_tpoam->RxFcl, p_tpoam->vlanid, p_tpoam->priority, p_tpoam->port);
    }
#endif

    if((pm_id == PM_ID_OPENFLOW) || need_send_openflow)
    {
        sal_memset(&tail, 0, sizeof(tail));
        p_tail = &tail;
        
        /* Modifed by weizj for tunnel */
        p_tail->fid = 0; //TODO
        p_tail->logical_src_port = 0; //TODO
        p_tail->payload_offset = attr_list[4].value.u8;
        p_tail->metadata = attr_list[5].value.u16;
        if (p_tail->metadata)
        {
            hsrv_openflow_tunnel_get_info_by_metadata(p_tail->metadata, &ofport, &tun_id);
            if (ofport)
            {
                p_tail->tnl_ofport = ofport;
            }
            if (tun_id)
            {
                p_tail->fid = tun_id;
            }
        }
        /* End by weizj for tunnel */
        req_msg.u_hdr.pkt.sub_type  =  1;/*GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/ 
        req_msg.data1_len = sizeof(glb_openflow_msg_tail_t);
        req_msg.data1 = p_tail;
    }

    /* support nd modified by liwh for bug 47547, 2018-07-23 */
    //if (GLB_PKT_ARP == type)
    if ((GLB_PKT_ARP == type) || (GLB_PKT_ND == type))
    /* liwh end */
    {
        if (GLB_VLAN_NONE != vlan_id)
        {
            hsrv_fdb_learn_arp_fdb(vlan_id, p_port_if->ifindex, (req_msg.data+MAC_ADDR_LEN));
        }
    }
    
    ctc_sock_send(hostif_master->pm_pkt_sock, pm_id, &req_msg);

    /* Added by kcao 2016-11-21 to support multi-dest PMs */
    if (PM_ID_INVALID != pm_id1)
    {
        ctc_sock_send(hostif_master->pm_pkt_sock, pm_id1, &req_msg);
    }
    if (PM_ID_INVALID != pm_id2)
    {
        ctc_sock_send(hostif_master->pm_pkt_sock, pm_id2, &req_msg);
    }

    if (PM_ID_INVALID != pm_id3)
    {
        ctc_sock_send(hostif_master->pm_pkt_sock, pm_id3, &req_msg);
    }

    if (PM_ID_INVALID != pm_id4)
    {
        ctc_sock_send(hostif_master->pm_pkt_sock, pm_id4, &req_msg);
    }
     
}

/***************************************************************************************************
 * Name         : hsrv_cpu_packet_debug
 * Purpose      : receive trap packet and process     
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
void 
hsrv_cpu_packet_debug(
    void *buffer, 
    sai_size_t buffer_size, 
    uint32_t attr_count,
    const sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr_vlan = NULL;
    const sai_attribute_t *attr_trap = NULL;
    const sai_attribute_t *attr_port = NULL;
    const sai_attribute_t *attr_lag = NULL;
    uint32 pm_id = PM_ID_INVALID;
    uint32 pm_id1 = PM_ID_INVALID;
    uint32 pm_id2 = PM_ID_INVALID;
    uint32 pm_id3 = PM_ID_INVALID;
    uint32 pm_id4 = PM_ID_INVALID;
    uint32 type  = 0;
    uint16 vlan_id = 0;
    int32  reason = 0;
    int32 rc = 0;

    tbl_fea_port_if_t* p_port_if = NULL;
    tbl_fea_port_if_key_t  fea_port_if_key;
    tbl_interface_t* p_db_if = NULL;
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];

    if (attr_count == 1) {
        hsrv_debug_packets_tx(buffer, buffer_size, attr_list[0].value.u32);
        return;
    }

    attr_trap = attr_list;
    attr_port = attr_trap + 1;
    attr_vlan = attr_trap + 2;
    attr_lag = attr_trap + 3;

    reason = attr_trap->value.s32;
    /*for slow protocol SyncE SSM, OAM and LACP*/
    if (SAI_HOSTIF_TRAP_ID_LACP == reason)
    {
        _hsrv_packet_get_proc_slow_proto(buffer, &pm_id, &type);
    }
    else
    {
        _hsrv_packet_event_get_pm_id(attr_trap->value.s32, &pm_id, &pm_id1, &pm_id2, &pm_id3, &pm_id4, &type);
    }

    if (type == GLB_PKT_LACP || type == GLB_PKT_L2_LLDP || type == GLB_PKT_SSM)
    {
        if(attr_port->id == SAI_HOSTIF_PACKET_INGRESS_PORT)
        {
            fea_port_if_key.portid = attr_port->value.oid;
        }
    }
    else
    {
        if(4 == attr_count && attr_lag->id == SAI_HOSTIF_PACKET_INGRESS_LAG)
        {
            fea_port_if_key.portid = attr_lag->value.oid;
        }
        else if(attr_port->id == SAI_HOSTIF_PACKET_INGRESS_PORT)
        {
            fea_port_if_key.portid = attr_port->value.oid;
        }

        if (attr_vlan->id == SAI_HOSTIF_PACKET_VLAN_ID)
        {
            vlan_id = attr_vlan->value.u16;
        }
    }

    if (SAI_OBJECT_TYPE_COPP == CTC_SAI_OBJECT_TYPE_GET(fea_port_if_key.portid))
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(GLB_CPU_IFINDEX_MIN);
        if (NULL == p_db_if)
        {
            return;
        }
    }
    else
    {
        p_port_if = tbl_fea_port_if_get_fea_port_if(&fea_port_if_key); 
        if(NULL == p_port_if)
        {
            return;
        }

        p_db_if = tbl_interface_get_interface_by_ifindex(p_port_if->ifindex);
        if (NULL == p_db_if)
        {
            return;
        }
    }
    
    /* Added by kcao for bug 40778, add STP/ERPS status check */
    rc = hsrv_hostif_port_state_check(FALSE, type, vlan_id, p_db_if);
    if (rc < 0)
    {
        return;
    }
    
    /* Modified by lixd for cpu packets debugging, use the 5th attribute as a
     * flag of cpu packet debugging  */
    /* print debugging info */
    sal_memcpy(ifname, p_db_if->key.name, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);
    hsrv_debug_packets_rx(buffer, buffer_size, reason, ifname_ext, vlan_id);
}

void
hsrv_hostif_ptp_tx_discard_cb(hsrv_ptp_sockaddr_t *p_skddr_ptp)
{
    if (p_skddr_ptp != NULL && (p_skddr_ptp->capture || p_skddr_ptp->replace))
    {
        hsrv_ptp_tx_discard_cb();
    }
    return ;
}

int32
hsrv_packet_event_send_insert_ptp(hsrv_ptp_sockaddr_t *p_skddr_ptp, sai_attribute_t *attr_intf)
{   
    tbl_fea_global_t    *p_fea_global       = NULL;
    if (p_skddr_ptp == NULL)
    {
        return HSRV_E_FAIL;
    }
    
    attr_intf->id = SAI_HOSTIF_PACKET_PTP_PACKET_RCV;
    
    sai_pkt_ptp_info_t pkt_ptp_info;
    sal_memset(&pkt_ptp_info, 0, sizeof (sai_pkt_ptp_info_t));
    
    pkt_ptp_info.vlan_priority = p_skddr_ptp->vlan_priority;
    
    if (p_skddr_ptp->capture)
    {
        pkt_ptp_info.oper = CTC_PTP_CAPTURE_ONLY;
    }
    else if (p_skddr_ptp->replace)
    {
        pkt_ptp_info.oper = CTC_PTP_REPLACE_ONLY;
    }
    else if (p_skddr_ptp->correction)
    {
        pkt_ptp_info.oper = CTC_PTP_CORRECTION;
    }
    else
    {
        pkt_ptp_info.oper = CTC_PTP_NULL_OPERATION;
    }

    pkt_ptp_info.seq_id = p_skddr_ptp->seq_id%4;
    pkt_ptp_info.ts.seconds = p_skddr_ptp->seconds;
    pkt_ptp_info.ts.nanoseconds = p_skddr_ptp->nanoseconds;
    pkt_ptp_info.ts_offset = p_skddr_ptp->ptp_offset;
    pkt_ptp_info.is_unicast = p_skddr_ptp->is_unicast;

    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);
    pkt_ptp_info.vr_id = p_fea_global->router_id;

    pkt_ptp_info.vlan_id = p_skddr_ptp->vlan_id;
    
    HSRV_PTP_TX_DEBUG("\nptp_msg_type(%s), vlan_id(%hu), vlan_priority(%hhu), oper(%s), seq_id(%hhu)(%hhu), seconds(%hhu), nanoseconds(%hhu), ts_offset(%hhu), is_unicast(%hhu) vr_id(%"PRIu64")",
        ptp_desc_msg_type(p_skddr_ptp->ptp_msg_type), pkt_ptp_info.vlan_id, pkt_ptp_info.vlan_priority, ptp_desc_oper(pkt_ptp_info.oper), pkt_ptp_info.seq_id, p_skddr_ptp->seq_id, 
        pkt_ptp_info.ts.seconds, pkt_ptp_info.ts.nanoseconds, pkt_ptp_info.ts_offset, pkt_ptp_info.is_unicast, pkt_ptp_info.vr_id);
        
    sal_memcpy(&(attr_intf->value.ptp), &pkt_ptp_info, sizeof(sai_pkt_ptp_info_t));

    return HSRV_E_NONE;
}

#ifdef CONFIG_TPOAM
int32
hsrv_packet_event_send_insert_tpoam(hsrv_sockaddr_tpoam_t*p_skddr_tpoam, sai_attribute_t *attr_intf)
{   
    sai_pkt_oam_info_t pkt_tpoam_info;
    sal_memset(&pkt_tpoam_info, 0, sizeof (sai_pkt_oam_info_t));
    uint32 nh_id = 0;
    uint32 nh_offset = 0;
    uint32 rc = 0;
    uint16 dest_gport;
    uint16 src_gport;
    uint32 lsp_aps = TRUE;
    uint32 working = TRUE;
    
    if (p_skddr_tpoam == NULL)
    {
        return HSRV_E_FAIL;
    }
    
    attr_intf->id = SAI_HOSTIF_PACKET_TPOAM_PACKET_RCV;
    
    pkt_tpoam_info.ttl = p_skddr_tpoam->ttl;
    pkt_tpoam_info.src_cos = p_skddr_tpoam->exp;
    pkt_tpoam_info.vid = (p_skddr_tpoam->vlanid & 0xFFF);//maybe need htons
    #ifdef USW
    pkt_tpoam_info.priority = p_skddr_tpoam->exp * 2;
    #else
    pkt_tpoam_info.priority = p_skddr_tpoam->exp * 8;
    #endif
    GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_PRIORITY);
    GLB_UNSET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_SRC_SVID_VALID);

    rc = hsrv_openflow_tpoam_api_get_dest_gport(p_skddr_tpoam->port, &dest_gport, &src_gport);
    if (HSRV_E_NONE == rc)
    {
        pkt_tpoam_info.dest_gport = dest_gport;
        pkt_tpoam_info.src_gport = src_gport;
    }

    /*vlan not support too*/
    
    /*not support mcast*/
    GLB_UNSET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_MCAST);
    GLB_UNSET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_SRC_PORT_VALID);         

    pkt_tpoam_info.is_bypass = p_skddr_tpoam->bypass;
    if (p_skddr_tpoam->bypass)
    {
        GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS);
    }
    else
    {
        GLB_UNSET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS);  
    }

    GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_COLOR);  

    pkt_tpoam_info.color = GLB_QOS_COLOR_NONE;
    
    rc = hsrv_openflow_tpoam_api_get_nhid_offset(p_skddr_tpoam->key_type, p_skddr_tpoam->key, &nh_id, &nh_offset, &lsp_aps, &working);
    if (HSRV_E_NONE == rc)
    {
        GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_NH_OFFSET_VALID);
        GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_NHID_VALID);
        pkt_tpoam_info.nh_id = nh_id;

        if (lsp_aps)
        {
            if (working)
            {
                GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_APS_W_PATH);    
            }
            else
            {
                GLB_SET_FLAG(pkt_tpoam_info.bit_flags, CTC_PKT_FLAG_APS_P_PATH); 
            }
        }
    }
    else
    {
        /*get nh_offset error*/
        return HSRV_E_FAIL;
    }

    /*packet type*/
    if (GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP == p_skddr_tpoam->key_type)
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_LINK_OAM);
    }
    if (GLB_TPOAM_TX_KEY_TYPE_PW != p_skddr_tpoam->key_type)
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_HAS_MPLS_GAL);
    }
    else if((GLB_TPOAM_TX_KEY_TYPE_PW == p_skddr_tpoam->key_type) && (!hsrv_openflow_tpoam_api_get_pw_mode()))
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_HAS_MPLS_GAL);
    }
    else
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_HAS_MPLS_CW);
    }
    
    if (1 == p_skddr_tpoam->reserve)
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_IS_DM);
        pkt_tpoam_info.dm_ts_offset = p_skddr_tpoam->ts_offset;
    }

    if (2 == p_skddr_tpoam->reserve)
    {
        GLB_SET_FLAG(pkt_tpoam_info.flags, CTC_PKT_OAM_FLAG_IS_LM);
    }

    /*do at ctc_sai_hostif*/
    /*
    p_encap_info->tx_info.oper_type = CTC_PKT_OPER_OAM;
        p_encap_info->tx_info.is_critical = TRUE;
        p_encap_info->tx_info.oam.type = PETH_GB_OAM_TYPE_ACH;
    */
    
    sal_memcpy(&(attr_intf->value.tpoam), &pkt_tpoam_info, sizeof(sai_pkt_oam_info_t));

    return HSRV_E_NONE;
}
 #endif/* !CONFIG_TPOAM */

/***************************************************************************************************
 * Name         : hsrv_packet_event_send 
 * Purpose      : 
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_packet_event_send(int32 sock, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_rif_t       *p_rif = NULL;
    tbl_rif_t       rif;
    sai_attribute_t attr_intf[4];
    uint32_t        attr_count = 0;
    int32           rc = HSRV_E_NONE;
    hsrv_ptp_sockaddr_t *p_skddr_ptp = NULL;
    uint32 is_ptp_msg = FALSE;
#ifdef CONFIG_TPOAM
    hsrv_sockaddr_tpoam_t *p_skddr_tpoam = NULL;
    uint32 is_tpoam_msg = FALSE;
#endif

    /*for ptp*/
    if (p_msg->u_hdr.pkt.type == GLB_PKT_PTP)
    {
        //PTP_DESC_BUFFER_((char *)p_msg->data, p_msg->data_len, TRUE);
        
        p_msg->data1_len = sizeof(hsrv_ptp_sockaddr_t);
        p_msg->data1 = p_msg->data + p_msg->data_len - p_msg->data1_len;
        p_skddr_ptp = (hsrv_ptp_sockaddr_t *)p_msg->data1;
        //printf("hsrv_packet_event_send\n");
        //ptp_desc_ptp_sockaddr((ptp_sockaddr_t *)p_skddr_ptp);
        
        //PTP_DESC_BUFFER_((char *)p_msg->data1, p_msg->data1_len, TRUE);
        is_ptp_msg = TRUE;
    }

    /*for tpoam*/
    if (p_msg->u_hdr.pkt.type == GLB_PKT_TPOAM)
    {  
        p_msg->data1_len = sizeof(hsrv_sockaddr_tpoam_t);
        p_msg->data1 = p_msg->data + p_msg->data_len - p_msg->data1_len;
#ifdef CONFIG_TPOAM
        p_skddr_tpoam = (hsrv_sockaddr_tpoam_t *)p_msg->data1;

		
        is_tpoam_msg = TRUE;
#endif
    }

    if (GLB_FLAG_ISSET(p_msg->u_hdr.pkt.flags, GLB_PKT_FLAGS_TX_MCAST))
    {
        /* added by kcao 2016-04-06 for support
         * 1. flood in VLAN 
         * 2. flood in VLAN, but not send to ingress port
         */
        attr_intf[0].id = SAI_HOSTIF_PACKET_EGRESS_PORT_VLAN;
        attr_intf[0].value.u16 = p_msg->u_hdr.pkt.vid;

        attr_intf[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
        if (p_msg->u_hdr.pkt.ifindex)
        {
            p_db_if = tbl_interface_get_interface_by_ifindex(p_msg->u_hdr.pkt.ifindex);
            if (NULL == p_db_if)
            {
                hsrv_hostif_ptp_tx_discard_cb(p_skddr_ptp);
                return 0;
            }
            rc = hsrv_interface_get_objectid(p_db_if, &attr_intf[1].value.oid);
        }
        else
        {
            attr_intf[1].value.oid = 0;
        }
        attr_count = 2;
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_ifindex(p_msg->u_hdr.pkt.ifindex);
        if (NULL == p_db_if)
        {
            HSRV_LOG_ERR("Interface does not exist, so sending Final RA failed");
            hsrv_hostif_ptp_tx_discard_cb(p_skddr_ptp);
            return 0;
        }
        /* Added by kcao for bug 40778, add STP/ERPS status check */
        rc = hsrv_hostif_port_state_check(TRUE, p_msg->u_hdr.pkt.type, 0, p_db_if);
        if (rc < 0)
        {
            hsrv_hostif_ptp_tx_discard_cb(p_skddr_ptp);
            return 0;
        }

        sal_memcpy(rif.key.name, p_db_if->key.name, sizeof(p_db_if->key.name));
        p_rif = tbl_rif_get_rif(&rif.key);

        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {
            attr_intf[0].id = SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG;
            rc = hsrv_interface_get_objectid(p_db_if, &attr_intf[0].value.oid);
        }
        else if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
        {
            attr_intf[0].id = SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG;        
            rc = hsrv_interface_get_objectid(p_db_if, &attr_intf[0].value.oid);
        }
        else if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
        {
            attr_intf[0].id = SAI_HOSTIF_PACKET_EGRESS_PORT_VLAN;
            attr_intf[0].value.u16 = p_msg->u_hdr.pkt.vid;
            attr_count = 1;
        }
        else
        {
            hsrv_hostif_ptp_tx_discard_cb(p_skddr_ptp);
            return HSRV_E_INVALID_PARAM;
        }
        attr_count = 1;
    }

    attr_intf[attr_count].id = SAI_HOSTIF_PACKET_PDU_TYPE;
    attr_intf[attr_count].value.u32 = p_msg->u_hdr.pkt.type;
    attr_count++;
    
    if (rc < 0)
    {
        hsrv_hostif_ptp_tx_discard_cb(p_skddr_ptp);
        return rc;
    }

    /*for ptp*/
    if (TRUE == is_ptp_msg)
    {
        rc = hsrv_packet_event_send_insert_ptp(p_skddr_ptp, &attr_intf[attr_count]);
        if (rc == HSRV_E_NONE)
        {
            HSRV_PTP_TX_DEBUG("ptp_msg_type(%s), vlan_id(%hu), vlan_priority(%hhu), oper(%s), seq_id(%hhu), seconds(%hhu), nanoseconds(%hhu), ts_offset(%hhu), is_unicast(%hhu)",
                ptp_desc_msg_type(p_skddr_ptp->ptp_msg_type), 
                attr_intf[attr_count].value.ptp.vlan_id, attr_intf[attr_count].value.ptp.vlan_priority, ptp_desc_oper(attr_intf[attr_count].value.ptp.oper), attr_intf[attr_count].value.ptp.seq_id, 
                attr_intf[attr_count].value.ptp.ts.seconds, attr_intf[attr_count].value.ptp.ts.nanoseconds, attr_intf[attr_count].value.ptp.ts_offset, attr_intf[attr_count].value.ptp.is_unicast);
            attr_count++;
            p_msg->data_len = p_msg->data_len - sizeof(hsrv_ptp_sockaddr_t);
            //PTP_DESC_BUFFER_((char *)p_msg->data, p_msg->data_len, TRUE);
        }
        else
        {
            HSRV_PTP_TX_DEBUG("Insert ptp para fail %d", rc);
        }
    }

#ifdef CONFIG_TPOAM
    /*for tpoam*/
    if(TRUE == is_tpoam_msg)
    {
        rc = hsrv_packet_event_send_insert_tpoam(p_skddr_tpoam, &attr_intf[attr_count]);
        if (rc == HSRV_E_NONE)
        {
            attr_count++;
            p_msg->data_len = p_msg->data_len - sizeof(hsrv_sockaddr_tpoam_t);
        }
        else
        {
            HSRV_TPOAM_PKT_TX_DEBUG("Insert tpam para fail %d", rc);
        }
    }
#endif/* !CONFIG_TPOAM */

    if (p_rif)
    {
        return hostif_master->hostintf_api->send_packet(p_rif->rif_id, p_msg->data, 
        p_msg->data_len, attr_count, attr_intf);
    }
    else
    {
        return hostif_master->hostintf_api->send_packet(0, p_msg->data, 
        p_msg->data_len, attr_count, attr_intf);
    }
}

/***************************************************************************************************
 * Name         : hsrv_hostif_create 
 * Purpose      : 
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_hostif_create(char *ifname, uint64 portid, uint64 *hif_id)
{
    sai_attribute_t attr[3];

    if (NULL == hif_id)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_ATTR_TYPE;
    attr[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;

    attr[1].id = SAI_HOSTIF_ATTR_RIF_OR_PORT_ID;
    attr[1].value.oid = portid;

    attr[2].id = SAI_HOSTIF_ATTR_NAME;
    sal_strcpy(attr[2].value.chardata, ifname);

    hostif_master->hostintf_api->create_hostif(hif_id, 3, attr);
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_hostif_remove 
 * Purpose      : 
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_hostif_remove(uint64 hif_id)
{
    hostif_master->hostintf_api->remove_hostif(hif_id);
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : _hsrv_packet_event_sock 
 * Purpose      : 
 * Input        : 
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
static int32
_hsrv_packet_event_sock()
{
    lib_master_t *lib_master = lib_get_master();
    int32 sock = INVALID_SOCK_ID;
    int32 rc = HSRV_E_NONE;

    /* PM packet init */
    sock = ctc_socket(CTC_SOCK_IPC_PKT, 0, "PmPkt");
    if (sock < 0)
    {
        return HSRV_E_SOCK_CREATE_ERR;
    }

    ctc_sock_set_pmid(sock, lib_master->pm_id);
    rc = ctc_sock_bind(sock, SOCK_ADDR_PM_PKT);
    if (rc < 0)
    {
        ctc_sock_close(sock);
        return HSRV_E_SOCK_BIND_ERR;
    }

    hostif_master->pm_pkt_sock = sock;
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_hostif_trap_group_get 
 * Purpose      : get trap group oid by global reasonid
 * Input        : reasonid defined by glb_packet_tocpu_reason_t
 * Output       : sai_object_id_t trap_group_oid 
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
sai_object_id_t
hsrv_hostif_trap_group_get(glb_packet_tocpu_reason_t reason_id)
{
    return hostif_master->reason_info[reason_id].trap_group_oid;  
}


extern sai_hostif_trap_id_t
hsrv_cpu_traffic_system_to_sai_trap_id(glb_packet_tocpu_reason_t cpu_reason);
/***************************************************************************************************
 * Name         : hsrv_packet_trap_set 
 * Purpose      : create trap group for the trap and set policer and queue to it
 * Input        : NONE
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/

int32
hsrv_cpu_traffic_create_default_policer(sai_object_id_t *policer_oid)
{
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[8];
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    /*create policer_id */
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_POLICER_ATTR_METER_TYPE;
    attr[0].value.s32 = SAI_METER_TYPE_BYTES;
    attr[1].id = SAI_POLICER_ATTR_MODE;
    attr[1].value.s32 = SAI_POLICER_MODE_Sr_TCM;
    attr[2].id = SAI_POLICER_ATTR_COLOR_SOURCE;
    attr[2].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
    attr[3].id = SAI_POLICER_ATTR_CIR;
    attr[3].value.u64 = CTC_QOS_POLICER_RATE*1000/8;
    attr[4].id = SAI_POLICER_ATTR_CBS;
    attr[4].value.u64 = GLB_QOS_POLICER_DEFAULT_BURST;
    attr[5].id = SAI_POLICER_ATTR_PBS;
    attr[5].value.u64 = GLB_QOS_POLICER_DEFAULT_BURST;
    attr[6].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
    attr[6].value.s32 = 1;
    HSRV_IF_ERROR_RETURN(policer_api->create_policer(policer_oid, 7, attr));

    return HSRV_E_NONE; 
}

int32 
_hsrv_cpu_traffic_add_ssh_entry(uint16 dst_port)
{
    sai_copp_api_t *copp_api = NULL;
    sai_attribute_t attr[10];
    sai_object_id_t entry_oid[8] = {0};

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0]));
    }
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1]));
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_COPP_ENTRY_ATTR_FIELD_TYPE;
    attr[0].value.u8 = CTC_SAI_SSH;
    attr[1].id = SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr[1].value.u8 = 6; /*tcp*/
    attr[2].id = SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr[2].value.u16 = dst_port; /*tcp*/
    HSRV_IF_ERROR_RETURN(copp_api->create_copp_entry(&entry_oid[0], 3, attr));
    hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0] = entry_oid[0];
    hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1] = entry_oid[1];
    return HSRV_E_NONE;
}

int32 
_hsrv_cpu_traffic_add_telnet_entry(uint16 dst_port)
{
    sai_copp_api_t *copp_api = NULL;
    sai_attribute_t attr[10];
    sai_object_id_t entry_oid[8] = {0};

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0]));
    }
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1]));
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_COPP_ENTRY_ATTR_FIELD_TYPE;
    attr[0].value.u8 = CTC_SAI_TELNET;
    attr[1].id = SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr[1].value.u8 = 6; /*tcp*/
    attr[2].id = SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr[2].value.u16 = dst_port; /*tcp*/
    HSRV_IF_ERROR_RETURN(copp_api->create_copp_entry(&entry_oid[0], 3, attr));
    hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0] = entry_oid[0];
    hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1] = entry_oid[1];
    return HSRV_E_NONE;
}

int32 
_hsrv_cpu_traffic_add_mlag_entry(uint16 dst_port)
{
    sai_copp_api_t *copp_api = NULL;
    sai_attribute_t attr[10];
    sai_object_id_t entry_oid[8] = {0};

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_COPP_ENTRY_ATTR_FIELD_TYPE;
    attr[0].value.u8 = CTC_SAI_MLAG;
    attr[1].id = SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr[1].value.u8 = 6; /*tcp*/
    attr[2].id = SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr[2].value.u16 = dst_port; /*tcp*/
    HSRV_IF_ERROR_RETURN(copp_api->create_copp_entry(&entry_oid[0], 3, attr));
    hostif_master->reason_info[GLB_CPU_TRAFFIC_MLAG].copp_entry_oid[0] = entry_oid[0];
    hostif_master->reason_info[GLB_CPU_TRAFFIC_MLAG].copp_entry_oid[1] = entry_oid[1];
    return HSRV_E_NONE;
}

int32
hsrv_packet_trap_set(uint32 reasonid, uint32 queueid)
{
    sai_object_id_t trap_group_oid = 0;
    sai_object_id_t policer_oid = 0;
    sai_object_id_t trap_oid = 0;
    sai_attribute_t attr[8];
    tbl_fea_copp_trap_group_t copp_trap_group;

    HSRV_IF_ERROR_RETURN(hsrv_cpu_traffic_create_default_policer(&policer_oid));
    
    /*1, create trap group first*/
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr[0].value.u32 = queueid;
    attr[1].id = SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER;
    attr[1].value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->create_hostif_trap_group(&trap_group_oid, 2, attr));
    hostif_master->reason_info[reasonid].trap_group_oid = trap_group_oid;
    hostif_master->reason_info[reasonid].trap_group_policer_oid = policer_oid;
    
    /*2, set policer to trapgroup for the trap*/
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP;
    attr[0].value.oid = trap_group_oid;
    trap_oid = hsrv_cpu_traffic_system_to_sai_trap_id(reasonid);
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_attribute(trap_oid, attr));

    /*3, set policer and queue to this trap*/
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE;
    attr[0].value.u32 = queueid;
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER;
    attr[0].value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    /*cnos not support pim & bgp now, open later*/
    if (GLB_CPU_TRAFFIC_PIM == reasonid || GLB_CPU_TRAFFIC_BGP == reasonid)
    {
        return HSRV_E_NONE;
    }
    
    /*4. add the fea copp trap group table*/
    sal_memset(&copp_trap_group, 0, sizeof(copp_trap_group));
    copp_trap_group.key.copp_trap_reason_id = reasonid;
    copp_trap_group.policer_oid = policer_oid;
    copp_trap_group.trap_oid = trap_oid;
    copp_trap_group.queue_id = queueid;
    return tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(&copp_trap_group, FALSE);
}

int32
hsrv_packet_sepcial_trap_set(uint32 reasonid, uint32 queueid)
{
    sai_object_id_t trap_group_oid = 0;
    sai_object_id_t policer_oid = 0;
    sai_object_id_t trap_oid = 0;
    sai_attribute_t attr[8];
    tbl_fea_copp_trap_group_t copp_trap_group;

    HSRV_IF_ERROR_RETURN(hsrv_cpu_traffic_create_default_policer(&policer_oid));

    /*1. get the trap_group_id and policy_id*/
    trap_group_oid = hostif_master->reason_info[GLB_CPU_TRAFFIC_COPY_TO_CPU].trap_group_oid;
    hostif_master->reason_info[reasonid].trap_group_oid = trap_group_oid;
    hostif_master->reason_info[reasonid].trap_group_policer_oid = policer_oid;

    /*2. set the policy to the trap*/
    trap_oid = hsrv_cpu_traffic_system_to_sai_trap_id(reasonid);
    if (GLB_CPU_TRAFFIC_SSH == reasonid)
    {
        attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_SSH_POLICER;
        attr[0].value.oid = policer_oid;
        HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    }
    else if (GLB_CPU_TRAFFIC_TELNET == reasonid)   
    {
        attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_TELNET_POLICER;
        attr[0].value.oid = policer_oid;
        HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    }
    else if (GLB_CPU_TRAFFIC_MLAG == reasonid)   
    {
        attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_MLAG_POLICER;
        attr[0].value.oid = policer_oid;
        HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    }

    /*3. add the fea copp trap group table*/
    sal_memset(&copp_trap_group, 0, sizeof(copp_trap_group));
    copp_trap_group.key.copp_trap_reason_id = reasonid;
    copp_trap_group.policer_oid = policer_oid;
    copp_trap_group.trap_oid = trap_oid;
    copp_trap_group.queue_id = queueid;
    return tbl_fea_copp_trap_group_add_fea_copp_trap_group_sync(&copp_trap_group, FALSE);
}

int32 
hsrv_cpu_traffic_update_ssh_entry(uint16 dst_port)
{
    sai_copp_api_t *copp_api = NULL;
    sai_attribute_t attr[10];
    sai_object_id_t entry_oid[8] = {0};
    sai_object_id_t policer_oid = 0;
    sai_object_id_t trap_group_oid = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    /*1. delete the ssh copp entry if the copp_entry_oid has existed*/
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0]));
    }
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1]));
    }

    /*2. creat ssh copp entry*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_COPP_ENTRY_ATTR_FIELD_TYPE;
    attr[0].value.u8 = CTC_SAI_SSH;
    attr[1].id = SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr[1].value.u8 = 6; /*tcp*/
    attr[2].id = SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr[2].value.u16 = dst_port; /*tcp*/
    HSRV_IF_ERROR_RETURN(copp_api->create_copp_entry(&entry_oid[0], 3, attr));

    /*3. set the copp_entry_id, policy_oid and trap_group_id*/
    hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[0] = entry_oid[0];
    hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].copp_entry_oid[1] = entry_oid[1];
    policer_oid = hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].trap_group_policer_oid;
    trap_group_oid = hostif_master->reason_info[GLB_CPU_TRAFFIC_SSH].trap_group_oid;

    /*4. set the trap group attribute*/
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_SSH_POLICER;
    attr[0].value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    return HSRV_E_NONE;
}

int32 
hsrv_cpu_traffic_update_telnet_entry(uint16 dst_port)
{
    sai_copp_api_t *copp_api = NULL;
    sai_attribute_t attr[10];
    sai_object_id_t entry_oid[8] = {0};
    sai_object_id_t policer_oid = 0;
    sai_object_id_t trap_group_oid = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    /*1. delete the telnet copp entry if the copp_entry_oid has existed*/
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0]));
    }
    if (0 != hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1])
    {
        sal_memset(attr, 0x0, sizeof(attr));
        HSRV_IF_ERROR_RETURN(copp_api->delete_copp_entry(hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1]));
    }

    /*2. creat telnet copp entry*/
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_COPP_ENTRY_ATTR_FIELD_TYPE;
    attr[0].value.u8 = CTC_SAI_TELNET;
    attr[1].id = SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr[1].value.u8 = 6; /*tcp*/
    attr[2].id = SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr[2].value.u16 = dst_port; /*tcp*/
    HSRV_IF_ERROR_RETURN(copp_api->create_copp_entry(&entry_oid[0], 3, attr));

    /*3. set the copp_entry_id, policy_oid and trap_group_id*/
    hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[0] = entry_oid[0];
    hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].copp_entry_oid[1] = entry_oid[1];
    policer_oid = hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].trap_group_policer_oid;
    trap_group_oid = hostif_master->reason_info[GLB_CPU_TRAFFIC_TELNET].trap_group_oid;

    /*4. set the trap group attribute*/
    attr[0].id = SAI_HOSTIF_TRAP_GROUP_ATTR_TELNET_POLICER;
    attr[0].value.oid = policer_oid;
    HSRV_IF_ERROR_RETURN(hostif_master->hostintf_api->set_trap_group_attribute(trap_group_oid, attr));
    return HSRV_E_NONE;
}

int32
_fea_copp_trap_group_cmd_read_session_all(cfg_cmd_para_t *para)
{
    field_parse_t field;
    field_parse_t *p_field = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (NULL == fp)
    {
        return PM_E_NO_MEMORY;
    }

    sal_memset(&args, 0 ,sizeof(args));
    p_field = &field;
    sal_memset(p_field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_fea_copp_trap_group_iterate((TBL_ITER_CB_FUNC)tbl_fea_copp_trap_group_dump_one, &args);

    sal_fclose(fp);
    return PM_E_NONE;
}

int32
tbl_fea_copp_trap_group_update_stats(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, cfg_cmd_para_t *para)
{
    tbl_chip_t *p_db_chip = tbl_chip_get_chip();
    sai_policer_api_t*  policer_api;
    sai_policer_stat_counter_t counter_ids[8];
    uint32_t number_of_counters = 0;
    uint64_t counters[8];
    sai_object_id_t policer_id;

    if(0 != p_fea_copp_trap_group->policer_oid)
    {
        /*modified by yejl for bug 50885, 54259, 2019-01-21*/
        if (GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY == p_fea_copp_trap_group->key.copp_trap_reason_id)
        {
            if (GLB_CHIP_GOLDENGATE == p_db_chip->type)
            {
                return HSRV_E_NONE;
            }
            else if (GLB_CHIP_GREATEBELT == p_db_chip->type)
            {
                if (CTC_FTM_PROFILE_5 != ftm_info_profile_type_gb)
                {
                    return HSRV_E_NONE;
                }
            }
            else if (GLB_CHIP_DUET2 == p_db_chip->type)
            {
                if (GLB_STM_MODE_IPUCV6 != ftm_info_profile_type_d2)
                {
                    return HSRV_E_NONE;
                }
            }
            else if (GLB_CHIP_TSINGMA == p_db_chip->type)
            {
                if (GLB_STM_MODE_IPUCV6 != ftm_info_profile_type_tm)
                {
                    return HSRV_E_NONE;
                }
            }
        }
     #ifdef OFPRODUCT  /*added by yejl for bug 51458, 51016, 2019-03-28*/
        if (GLB_CPU_TRAFFIC_EAPOL == p_fea_copp_trap_group->key.copp_trap_reason_id)
        {
            return HSRV_E_NONE;
        }
     #endif
        
        sal_memset(&counters, 0x0,(sizeof(uint64_t))*8);
        sal_memset(&counter_ids, 0x0,(sizeof(sai_policer_stat_counter_t))*8);

        policer_id = p_fea_copp_trap_group->policer_oid;

        counter_ids[number_of_counters] = SAI_POLICER_STAT_PACKETS;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_ATTR_BYTES;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_PACKETS;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_BYTES;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_PACKETS;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_BYTES;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_PACKETS;
        number_of_counters++;
        counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_BYTES;
        number_of_counters++;

        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

        HSRV_IF_ERROR_RETURN(policer_api->get_policer_statistics(policer_id, counter_ids, number_of_counters, counters));

        p_fea_copp_trap_group->green_packet = counters[2];
        p_fea_copp_trap_group->green_byte = counters[3];

        p_fea_copp_trap_group->yellow_packet = counters[4];
        p_fea_copp_trap_group->yellow_byte = counters[5];

        p_fea_copp_trap_group->red_packet = counters[6];
        p_fea_copp_trap_group->red_byte = counters[7];

        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE));
    }

    return HSRV_E_NONE;
}

int32
tbl_fea_copp_trap_group_delete_stats(tbl_fea_copp_trap_group_t *p_fea_copp_trap_group, cfg_cmd_para_t *para)
{
    tbl_chip_t *p_db_chip = tbl_chip_get_chip();
    sai_policer_api_t*  policer_api;
    sai_object_id_t policer_id;

    if(0 != p_fea_copp_trap_group->policer_oid)
    {
        /*modified by yejl for bug 50885, 54259, 2019-01-21*/
        if (GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY == p_fea_copp_trap_group->key.copp_trap_reason_id)
        {
            if (GLB_CHIP_GOLDENGATE == p_db_chip->type)
            {
                return HSRV_E_NONE;
            }
            else if (GLB_CHIP_GREATEBELT == p_db_chip->type)
            {
                if (CTC_FTM_PROFILE_5 != ftm_info_profile_type_gb)
                {
                    return HSRV_E_NONE;
                }
            }
            else if (GLB_CHIP_DUET2 == p_db_chip->type)
            {
                if (GLB_STM_MODE_IPUCV6 != ftm_info_profile_type_d2)
                {
                    return HSRV_E_NONE;
                }
            }
            else if (GLB_CHIP_TSINGMA == p_db_chip->type)
            {
                if (GLB_STM_MODE_IPUCV6 != ftm_info_profile_type_tm)
                {
                    return HSRV_E_NONE;
                }
            }
        }
    #ifdef OFPRODUCT /*added by yejl for bug 51458, 51016, 2019-03-28*/
        if (GLB_CPU_TRAFFIC_EAPOL == p_fea_copp_trap_group->key.copp_trap_reason_id)
        {
            return HSRV_E_NONE;
        }
    #endif
        policer_id = p_fea_copp_trap_group->policer_oid;
        HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
        HSRV_IF_ERROR_RETURN(policer_api->clear_policer_statistics(policer_id));

        p_fea_copp_trap_group->green_byte = 0;
        p_fea_copp_trap_group->green_packet = 0;
        p_fea_copp_trap_group->yellow_byte = 0;
        p_fea_copp_trap_group->yellow_packet = 0;
        p_fea_copp_trap_group->red_byte = 0;
        p_fea_copp_trap_group->red_packet = 0;
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_GREEN_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_YELLOW_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_RED_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_fea_copp_trap_group_set_fea_copp_trap_group_field(p_fea_copp_trap_group, TBL_FEA_COPP_TRAP_GROUP_FLD_RED_BYTE));
    }

    return HSRV_E_NONE;
}

int32
_fea_copp_trap_group_cmd_update_stats_all(cfg_cmd_para_t *para)
{
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    tbl_fea_copp_trap_group_iterate((TBL_ITER_CB_FUNC)tbl_fea_copp_trap_group_update_stats, &args);

    return PM_E_NONE;
}

int32
fea_copp_trap_group_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;

    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_FEA_COPP_TRAP_GROUP);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }

    switch (para->oper)
    {
    case CDB_OPER_GET:
        _fea_copp_trap_group_cmd_read_session_all(para);
        break;

    case CDB_OPER_SET:
        _fea_copp_trap_group_cmd_update_stats_all(para);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
_fea_map_copp_trap_group_reason_name_to_id(char *reason_name, uint32 *reason_id)
{
    if (0 == sal_strcmp(reason_name, "arp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_ARP;
    }
    else if (0 == sal_strcmp(reason_name, "bpdu"))
    {
        *reason_id = GLB_CPU_TRAFFIC_STP;
    }
    else if (0 == sal_strcmp(reason_name, "slow-protocol"))
    {
        *reason_id = GLB_CPU_TRAFFIC_LACP;
    }
    else if (0 == sal_strcmp(reason_name, "eapol"))
    {
        *reason_id = GLB_CPU_TRAFFIC_EAPOL;
    }
    else if (0 == sal_strcmp(reason_name, "lldp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_LLDP;
    }
    else if (0 == sal_strcmp(reason_name, "igmp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_IGMP;
    }
    else if (0 == sal_strcmp(reason_name, "erps"))
    {
        *reason_id = GLB_CPU_TRAFFIC_ERPS;
    }
    else if (0 == sal_strcmp(reason_name, "dhcp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_DHCP;
    }
    else if (0 == sal_strcmp(reason_name, "ospf"))
    {
        *reason_id = GLB_CPU_TRAFFIC_OSPF;
    }
    else if (0 == sal_strcmp(reason_name, "pim"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PIM;
    }
    else if (0 == sal_strcmp(reason_name, "vrrp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_VRRP;
    }
    else if (0 == sal_strcmp(reason_name, "bgp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_BGP;
    }
    else if (0 == sal_strcmp(reason_name, "management-traffic"))
    {
        *reason_id = GLB_CPU_TRAFFIC_COPY_TO_CPU;
    }
    else if (0 == sal_strcmp(reason_name, "packet-in"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PACKET_IN;
    }
    else if (0 == sal_strcmp(reason_name, "ptp"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PTP;
    }
    else if (0 == sal_strcmp(reason_name, "ttl-expired"))
    {
        *reason_id = GLB_CPU_TRAFFIC_TTL_ERROR;
    }
    else if (0 == sal_strcmp(reason_name, "mtu-fail"))
    {
        *reason_id = GLB_CPU_TRAFFIC_L3_MTU_ERROR;
    }
    else if (0 == sal_strcmp(reason_name, "ssh"))
    {
        *reason_id = GLB_CPU_TRAFFIC_SSH;
    }
    else if (0 == sal_strcmp(reason_name, "telnet"))
    {
        *reason_id = GLB_CPU_TRAFFIC_TELNET;
    }
    else if (0 == sal_strcmp(reason_name, "mlag"))
    {
        *reason_id = GLB_CPU_TRAFFIC_MLAG;
    }
    else if (0 == sal_strcmp(reason_name, "l2pro-protocol-mac"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC;
    }
    else if (0 == sal_strcmp(reason_name, "l2pro-group-mac"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC;
    }
    else if (0 == sal_strcmp(reason_name, "cpu-mirror"))
    {
        *reason_id = GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR;
    }
    else if (0 == sal_strcmp(reason_name, "icmpv6"))
    {
        *reason_id = GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY;
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

int32
_fea_map_copp_trap_group_id_to_reason_name(uint32 reason_id, char *reason_name)
{
    if (reason_id == GLB_CPU_TRAFFIC_ARP)
    {   
        sal_strcpy(reason_name, "arp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_STP)
    {
        sal_strcpy(reason_name, "bpdu");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_LACP)
    {
        sal_strcpy(reason_name, "slow-protocol");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_EAPOL)
    {
        sal_strcpy(reason_name, "eapol");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_LLDP)
    {
        sal_strcpy(reason_name, "lldp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_IGMP)
    {
        sal_strcpy(reason_name, "igmp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_ERPS)
    {
        sal_strcpy(reason_name, "erps");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_DHCP)
    {
        sal_strcpy(reason_name, "dhcp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_OSPF)
    {
        sal_strcpy(reason_name, "ospf");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PIM)
    {
        sal_strcpy(reason_name, "pim");
    }
    
    else if (reason_id == GLB_CPU_TRAFFIC_BGP)
    {
        sal_strcpy(reason_name, "bgp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_COPY_TO_CPU)
    {
        sal_strcpy(reason_name, "management-traffic");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PACKET_IN)
    {
        sal_strcpy(reason_name, "packet-in");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_TTL_ERROR)
    {
        sal_strcpy(reason_name, "ttl-expired");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_L3_MTU_ERROR)
    {
        sal_strcpy(reason_name, "mtu-fail");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_SSH)
    {
        sal_strcpy(reason_name, "ssh");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_TELNET)
    {
        sal_strcpy(reason_name, "telnet");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR)
    {
        sal_strcpy(reason_name, "cpu-mirror");
    }
    /*modified by yejl for bug 49548, 2018-12-12*/
    /*modified by yejl for bug 50657, 2019-01-14*/
    else if (reason_id == GLB_CPU_TRAFFIC_VRRP)
    {
        sal_strcpy(reason_name, "vrrp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_MLAG)
    {
        sal_strcpy(reason_name, "mlag");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC)
    {
        sal_strcpy(reason_name, "l2pro-protocol-mac");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC)
    {
        sal_strcpy(reason_name, "l2pro-group-mac");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_PTP)
    {
        sal_strcpy(reason_name, "ptp");
    }
    else if (reason_id == GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY)
    {
        sal_strcpy(reason_name, "icmpv6");
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}


int32
fea_show_copp_trap_group_counter(FILE* fp, char *reason_name, cfg_cmd_para_t *para)
{
    tbl_fea_copp_trap_group_t *p_fea_copp_trap_group = NULL;
    tbl_fea_copp_trap_group_key_t key_reason;
    ctclib_slistnode_t *listnode = NULL;
    char tmp_str[50];
    tbl_fea_copp_trap_group_master_t *p_master = tbl_fea_copp_trap_group_get_master();
    int32 rc = HSRV_E_NONE;
    uint8 queue_id;
    uint64 total_t_pkts = 0;
    uint64 total_t_bytes = 0;
    uint64 total_d_pkts = 0;
    uint64 total_d_bytes = 0;
    tbl_qos_global_t *p_qos_master = tbl_qos_global_get_qos_global();
    uint32 policer_ipg_len = 0;
    uint64 total_t_pkts2 = 0;
    uint64 total_t_bytes2 = 0;
    uint64 total_d_pkts2 = 0;
    uint64 total_d_bytes2 = 0;

    /*added by hansf for bug 48982 2018/10/11*/
#ifdef USW
    policer_ipg_len = 20;
#elif defined(GOLDENGATE)
    policer_ipg_len = 20;
#elif defined(GREATBELT)
    policer_ipg_len = 0;
#endif
    /* end by hansf */

    sal_memset(&key_reason, 0, sizeof(key_reason));
    if (NULL != reason_name)
    {
        rc = _fea_map_copp_trap_group_reason_name_to_id(reason_name, &key_reason.copp_trap_reason_id);
        if (rc != PM_E_NONE)
        {
            return PM_E_NOT_EXIST;
        }
        p_fea_copp_trap_group = tbl_fea_copp_trap_group_get_fea_copp_trap_group(&key_reason);
        if (p_fea_copp_trap_group)
        {
            total_t_pkts = p_fea_copp_trap_group->green_packet + p_fea_copp_trap_group->yellow_packet;
            total_t_bytes = p_fea_copp_trap_group->green_byte + p_fea_copp_trap_group->yellow_byte;
            total_d_pkts = p_fea_copp_trap_group->red_packet;
            total_d_bytes = p_fea_copp_trap_group->red_byte;
            if (p_qos_master->qos_ipg_enable)
            {
                total_t_bytes = total_t_bytes + total_t_pkts*policer_ipg_len;
                total_d_bytes = total_d_bytes +total_d_pkts*policer_ipg_len;
            }
            sal_fprintf (fp, "COPP reason %s stats\n", reason_name);
            sal_fprintf (fp, "  %-20s: %-15"PRIu64"\n", "Transmit-Pkts", total_t_pkts);
            sal_fprintf (fp, "  %-20s: %-15"PRIu64"\n", "Transmit-Bytes", total_t_bytes);
            sal_fprintf (fp, "  %-20s: %-15"PRIu64"\n", "Drop-Pkts ", total_d_pkts);
            sal_fprintf (fp, "  %-20s: %-15"PRIu64"\n", "Drop-Bytes", total_d_bytes);
        }
    }
    else
    {
        sal_fprintf(fp, "Reason              Transmit-Pkts  Transmit-Bytes Drop-Pkts      Drop-Bytes\n");
        sal_fprintf(fp, "-------------------+--------------+--------------+--------------+--------------\n");   

        for (queue_id = 0; queue_id < 7; queue_id++)
        {
            CTCLIB_SLIST_LOOP(p_master->fea_copp_trap_group_list, p_fea_copp_trap_group, listnode)
            {
                if (p_fea_copp_trap_group->queue_id != queue_id)
                {
                    continue;
                } 
                sal_memset(tmp_str, 0, sizeof(tmp_str));
                if(PM_E_NONE != _fea_map_copp_trap_group_id_to_reason_name(p_fea_copp_trap_group->key.copp_trap_reason_id,
                                                                            tmp_str))
                {
                    continue;
                }    
                total_t_pkts2 = p_fea_copp_trap_group->green_packet + p_fea_copp_trap_group->yellow_packet;
                total_t_bytes2 = p_fea_copp_trap_group->green_byte + p_fea_copp_trap_group->yellow_byte;
                total_d_pkts2 = p_fea_copp_trap_group->red_packet;
                total_d_bytes2 = p_fea_copp_trap_group->red_byte;
                if (p_qos_master->qos_ipg_enable)
                {
                    total_t_bytes2 = total_t_bytes2 + total_t_pkts2*policer_ipg_len;
                    total_d_bytes2 = total_d_bytes2 +total_d_pkts2*policer_ipg_len;
                }
                sal_fprintf (fp, "%-20s%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"\n", tmp_str,
                                total_t_pkts2, total_t_bytes2, total_d_pkts2, total_d_bytes2);
                total_t_pkts += p_fea_copp_trap_group->green_packet;
                total_t_pkts += p_fea_copp_trap_group->yellow_packet;
                total_t_bytes += p_fea_copp_trap_group->green_byte;
                total_t_bytes += p_fea_copp_trap_group->yellow_byte;
                total_d_pkts += p_fea_copp_trap_group->red_packet;
                total_d_bytes += p_fea_copp_trap_group->red_byte;
            }
        }

        if (p_qos_master->qos_ipg_enable)
        {
            total_t_bytes = total_t_bytes + total_t_pkts*policer_ipg_len;
            total_d_bytes = total_d_bytes +total_d_pkts*policer_ipg_len;
        }
        sal_fprintf(fp, "%-20s%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"\n", "total",
                    total_t_pkts, total_t_bytes, total_d_pkts, total_d_bytes);
    }
    return PM_E_NONE;
}

int32
hsrv_fea_copp_trap_group_cmd_show_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;

    p_node = cdb_get_act(ACT_SHOW_FEA_COPP_TRAP_GROUP_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }

        if (1 == argc)
        {
            fea_show_copp_trap_group_counter(fp, argv[0], para);
        }
        else if (0 == argc)
        {
            fea_show_copp_trap_group_counter(fp, NULL, para);
        }

        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
fea_clear_copp_trap_group_counter(char *reason_name, cfg_cmd_para_t *para)
{
    tbl_fea_copp_trap_group_t *p_fea_copp_trap_group = NULL;
    tbl_fea_copp_trap_group_key_t key_reason;
    int32 rc = PM_E_NONE;
    tbl_iter_args_t args;

    sal_memset(&args, 0, sizeof(args));
    sal_memset(&key_reason, 0, sizeof(key_reason));
    
    if (NULL == reason_name)
    {
        tbl_fea_copp_trap_group_iterate((TBL_ITER_CB_FUNC)tbl_fea_copp_trap_group_delete_stats, &args);
    }
    else
    {
        rc = _fea_map_copp_trap_group_reason_name_to_id(reason_name, &key_reason.copp_trap_reason_id);
        if (rc != PM_E_NONE)
        {
            return PM_E_NOT_EXIST;
        }
        p_fea_copp_trap_group = tbl_fea_copp_trap_group_get_fea_copp_trap_group(&key_reason);
        if (p_fea_copp_trap_group)
        {
            tbl_fea_copp_trap_group_delete_stats(p_fea_copp_trap_group, para);
        }
    }
    return PM_E_NONE;
}

int32
hsrv_fea_copp_trap_group_cmd_clear_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (1 == argc)
        {
            fea_clear_copp_trap_group_counter(argv[0], para);
        }
        else if (0 == argc)
        {
            fea_clear_copp_trap_group_counter(NULL, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Wrong arg input");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support clear action");
        break;
    }

    return rc;    
}

int32
_fea_copp_class_stats_get(FILE* fp, sai_object_id_t queue_oid, char *class_name,
                        uint64_t *p_t_pkt, uint64_t *p_t_byte, uint64_t *p_d_pkt, uint64_t *p_d_byte)
{
    sai_copp_api_t*  copp_api;
    sai_copp_queue_stat_counter_t counter_ids[4];
    uint32_t number_of_counters = 0;;
    uint64_t counters[4];
    uint32 queue_ipg_len = 0;
    tbl_qos_global_t *p_master = tbl_qos_global_get_qos_global();

    /*added by hansf for bug 48982 2018/10/11*/
#ifdef USW
    queue_ipg_len = 20;
#elif defined(GOLDENGATE)
    queue_ipg_len = 20;
#elif defined(GREATBELT)
    queue_ipg_len = 20;
#endif
    /* end by hansf */

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));

    counter_ids[number_of_counters] = SAI_COPP_QUEUE_STAT_PACKETS;
    number_of_counters++;
    counter_ids[number_of_counters] = SAI_COPP_QUEUE_STAT_BYTES;
    number_of_counters++;
    counter_ids[number_of_counters] = SAI_COPP_QUEUE_STAT_DROPPED_PACKETS;
    number_of_counters++;
    counter_ids[number_of_counters] = SAI_COPP_QUEUE_STAT_DROPPED_BYTES;
    number_of_counters++;

    HSRV_IF_ERROR_RETURN(copp_api->get_copp_queue_stats(queue_oid, counter_ids, number_of_counters, counters));
    *p_t_pkt += counters[0];
    *p_t_byte += counters[1];
    *p_d_pkt += counters[2];
    *p_d_byte += counters[3];
    if (p_master->qos_ipg_enable)
    {
        counters[1] = counters[1] + counters[0]*queue_ipg_len;
        counters[3] = counters[3] + counters[2]*queue_ipg_len;
    }
    sal_fprintf (fp, "%-20s%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"\n", class_name,
                counters[0], counters[1], counters[2], counters[3]);
    return PM_E_NONE;
}


int32
fea_show_copp_class_counter(FILE* fp, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint64_t total_t_pkts = 0;
    uint64_t total_t_bytes = 0;
    uint64_t total_d_pkts = 0;
    uint64_t total_d_bytes = 0;
    uint32 queue_ipg_len = 0;
    tbl_qos_global_t *p_master = tbl_qos_global_get_qos_global();

    /*added by hansf for bug 48982 2018/10/11*/
#ifdef USW
    queue_ipg_len = 20;
#elif defined(GOLDENGATE)
    queue_ipg_len = 20;
#elif defined(GREATBELT)
    queue_ipg_len = 20;
#endif
    /* end by hansf */

    sal_fprintf(fp, "Class               Transmit-Pkts  Transmit-Bytes Drop-Pkts      Drop-Bytes\n");
    sal_fprintf(fp, "-------------------+--------------+--------------+--------------+--------------\n");   

    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[0], "class 0", 
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);
    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[1], "class 1",
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);
    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[2], "class 2",
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);
    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[3], "class 3",
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);
    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[4], "class 4",
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);
    rc |= _fea_copp_class_stats_get(fp, hostif_master->group_queue[5], "class 5",
                                    &total_t_pkts, &total_t_bytes, &total_d_pkts, &total_d_bytes);

    if (p_master->qos_ipg_enable)
    {
        total_t_bytes = total_t_bytes + total_t_pkts*queue_ipg_len;
        total_d_bytes = total_d_bytes + total_d_pkts*queue_ipg_len;
    }
 
    sal_fprintf (fp, "%-20s%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"%-15"PRIu64"\n", "total",
                total_t_pkts, total_t_bytes, total_d_pkts, total_d_bytes);

    return rc;
}

int32
hsrv_fea_copp_class_cmd_show_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    FILE *fp = NULL;

    p_node = cdb_get_tbl(ACT_SHOW_COPP_CLASS_COUNTER);
    if (NULL == p_node)
    {
        return PM_E_NO_MEMORY;
    }
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
        fp = sal_fopen(para->p_show->path, "w+");
        if (NULL == fp)
        {
            return PM_E_NO_MEMORY;
        }
        fea_show_copp_class_counter(fp, para);
        sal_fclose(fp);
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support show action");
        break;
    }

    return rc;
}

int32
_fea_copp_class_clear_stats(sai_object_id_t queue_oid)
{
    sai_copp_api_t*  copp_api;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP,(void**)&copp_api));
    HSRV_IF_ERROR_RETURN(copp_api->clear_copp_queue_stats(queue_oid));
    return HSRV_E_NONE;
}

int32
fea_clear_copp_class_counter(char *class_id, cfg_cmd_para_t *para)
{
    int32 rc = HSRV_E_NONE;
    
    if (NULL == class_id)
    {
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[0]);
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[1]);
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[2]);
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[3]);
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[4]);
        rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[5]);
    }
    else
    {
        if (0 == sal_strcmp(class_id, "0"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[0]);
        }
        else if (0 == sal_strcmp(class_id, "1"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[1]);
        }
        else if (0 == sal_strcmp(class_id, "2"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[2]);
        }
        else if (0 == sal_strcmp(class_id, "3"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[3]);
        }
        else if (0 == sal_strcmp(class_id, "4"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[4]);
        }
        else if (0 == sal_strcmp(class_id, "5"))
        {
            rc |= _fea_copp_class_clear_stats(hostif_master->group_queue[5]);
        }
        else
        {
            rc = HSRV_E_INVALID_PARAM;
        } 
            
    }
    return rc;
}

int32
hsrv_fea_copp_class_cmd_clear_counter(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (1 == argc)
        {
            fea_clear_copp_class_counter(argv[0], para);
        }
        else if (0 == argc)
        {
            fea_clear_copp_class_counter(NULL, para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Wrong arg input");
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Only support clear action");
        break;
    }

    return rc;    
}

/***************************************************************************************************
 * Name         : hsrv_packet_event_init 
 * Purpose      : creat trap_group and set one trap_group to one trap
 * Input        : NONE
 * Output       :      
 * Return       : N/A
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_packet_event_init()
{
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[8];
    sai_object_id_t scheduler_id;
    sai_scheduler_api_t*  scheduler_api;
    uint8 i = 0;

    hostif_master = XMALLOC(MEM_HSRV_HOSTIF_INFO,sizeof(hsrv_hostintf_master_t));
    if (NULL == hostif_master)
    {
        return HSRV_E_NO_MEMORY;
    }
    
    sal_memset(hostif_master, 0, sizeof(*hostif_master));
    /* init socket to PMs*/
    _hsrv_packet_event_sock();

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HOST_INTERFACE, (void**)(&(hostif_master->hostintf_api))));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_COPP, (void**)(&(hostif_master->copp_api))));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    /*1, Create trap group for traps and set policer and queue*/
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_STP, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_LACP, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_EAPOL,  GLB_CPU_TRAFFIC_QUEUE_MAP_3));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_LLDP,  GLB_CPU_TRAFFIC_QUEUE_MAP_3));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_IGMP, GLB_CPU_TRAFFIC_QUEUE_MAP_3));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_ERPS, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_ARP, GLB_CPU_TRAFFIC_QUEUE_MAP_1));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_DHCP,  GLB_CPU_TRAFFIC_QUEUE_MAP_3));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_OSPF,  GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PIM,  GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_VRRP,  GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_BGP,  GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_COPY_TO_CPU, GLB_CPU_TRAFFIC_QUEUE_MAP_5));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PACKET_IN, GLB_CPU_TRAFFIC_QUEUE_MAP_2));
    /*for ptp add by chenc*/
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PTP, GLB_CPU_TRAFFIC_QUEUE_MAP_3));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_TTL_ERROR, GLB_CPU_TRAFFIC_QUEUE_MAP_0));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_L3_MTU_ERROR, GLB_CPU_TRAFFIC_QUEUE_MAP_0));
    /*for cpu_mirror by shengx*/
    //HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PACKET_CPU_MIRROR, GLB_CPU_TRAFFIC_QUEUE_MAP_0));

    /*for l2protocol by yejl*/
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PACKET_L2PRO_PROTOCOL_MAC, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_PACKET_L2PRO_GROUP_MAC, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
    HSRV_IF_ERROR_RETURN(hsrv_packet_trap_set(GLB_CPU_TRAFFIC_IPV6_NEIGHBOR_DISCOVERY, GLB_CPU_TRAFFIC_QUEUE_MAP_1));
    
    HSRV_IF_ERROR_RETURN(_hsrv_cpu_traffic_add_ssh_entry(22));
    HSRV_IF_ERROR_RETURN(hsrv_packet_sepcial_trap_set(GLB_CPU_TRAFFIC_SSH, GLB_CPU_TRAFFIC_QUEUE_MAP_5));
    HSRV_IF_ERROR_RETURN(_hsrv_cpu_traffic_add_telnet_entry(23));
    HSRV_IF_ERROR_RETURN(hsrv_packet_sepcial_trap_set(GLB_CPU_TRAFFIC_TELNET, GLB_CPU_TRAFFIC_QUEUE_MAP_5));
    HSRV_IF_ERROR_RETURN(_hsrv_cpu_traffic_add_mlag_entry(61000));
    HSRV_IF_ERROR_RETURN(hsrv_packet_sepcial_trap_set(GLB_CPU_TRAFFIC_MLAG, GLB_CPU_TRAFFIC_QUEUE_MAP_4));
        
    /*2,  create for class 0-3 queue scheduler */
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));
    
    sal_memset(attr, 0x0, sizeof(attr));
    attr[0].id = SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM;
    attr[0].value.s32 = SAI_SCHEDULING_STRICT;
    attr[1].id = SAI_SCHEDULER_ATTR_SHAPER_TYPE;
    attr[1].value.s32 = SAI_METER_TYPE_BYTES;
    attr[2].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
    attr[2].value.u64 = GLB_CPU_TRAFFIC_CLASS0_DEFAULT_RATE*2000/8;
    attr[3].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
    attr[3].value.u64 = GLB_QOS_POLICER_DEFAULT_BURST;
    HSRV_IF_ERROR_RETURN(scheduler_api->create_scheduler_profile(&scheduler_id, 4, attr));
    hostif_master->scheduler_id[GLB_CPU_TRAFFIC_GROUP_0] = scheduler_id;
    hostif_master->group_queue[GLB_CPU_TRAFFIC_QUEUE_MAP_0] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, GLB_CPU_TRAFFIC_QUEUE_MAP_0);

    for (i = GLB_CPU_TRAFFIC_GROUP_1; i< GLB_CPU_TRAFFIC_GROUP_MAX; i++) /*group 1-5,queue 1-5*/
    {
        sal_memset(attr, 0x0, sizeof(attr));
        attr[0].id = SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM;
        attr[0].value.s32 = SAI_SCHEDULING_STRICT;
        attr[1].id = SAI_SCHEDULER_ATTR_SHAPER_TYPE;
        attr[1].value.s32 = SAI_METER_TYPE_BYTES;
        attr[2].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
        attr[2].value.u64 = GLB_CPU_TRAFFIC_DEFAULT_RATE*4000/8;
        attr[3].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE;
        attr[3].value.u64 = GLB_QOS_POLICER_DEFAULT_BURST;
        HSRV_IF_ERROR_RETURN(scheduler_api->create_scheduler_profile(&scheduler_id, 4, attr));
        hostif_master->scheduler_id[i] = scheduler_id;
        hostif_master->group_queue[i] = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, i);
    }

    if (raps_rec_while == NULL)
    {
        raps_rec_while = CTC_TASK_ADD_TIME(hsrv_raps_rec_timer_fn, NULL, 1);
    }

    return HSRV_E_NONE;
}


int32
hsrv_fea_copp_entry_update_ssh(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ssh_cfg_t* p_ssh_global = NULL;

    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_ssh_global = (tbl_ssh_cfg_t*)p_tbl;
        if (TBL_SSH_CFG_FLD_LISTEN_PORT == field_id)
        {
           hsrv_cpu_traffic_update_ssh_entry(p_ssh_global->listen_port);
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}



int32
hsrv_hostif_start()
{
    cdb_register_cfg_tbl_cb(TBL_FEA_COPP_TRAP_GROUP, fea_copp_trap_group_cmd_process);
    cdb_register_cfg_act_cb(ACT_SHOW_FEA_COPP_TRAP_GROUP_COUNTER, hsrv_fea_copp_trap_group_cmd_show_counter);
    cdb_register_cfg_act_cb(ACT_CLEAR_FEA_COPP_TRAP_GROUP_COUNTER, hsrv_fea_copp_trap_group_cmd_clear_counter);
    cdb_register_cfg_act_cb(ACT_SHOW_COPP_CLASS_COUNTER, hsrv_fea_copp_class_cmd_show_counter);
    cdb_register_cfg_act_cb(ACT_CLEAR_COPP_CLASS_COUNTER, hsrv_fea_copp_class_cmd_clear_counter);

    cdb_pm_subscribe_tbl(TBL_SSH_CFG, NULL, hsrv_fea_copp_entry_update_ssh);

    return HSRV_E_NONE;
}

