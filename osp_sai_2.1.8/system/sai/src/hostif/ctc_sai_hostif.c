
#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include "ctc_hash.h"
#include <sys/socket.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_vlan.h>
#include <ctc_sai_port.h>
#include <ctc_sai_neighbor.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_mirror.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_fdb.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_copp.h>
#include <ctc_sai_router.h>
#include <ctc_sai_vrrp.h>
#include <ctc_sai_route.h>
#include <ctc_sai_debug.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/icmp.h>

#ifndef NIPQUAD
#define NIPQUAD(addr)              \
    ((unsigned char *)&addr)[0],   \
    ((unsigned char *)&addr)[1],   \
    ((unsigned char *)&addr)[2],   \
    ((unsigned char *)&addr)[3]
#endif

/*added by kexl for bug 45790, 2017-12-05*/
#define UMPETH_ICMP_TYPE_TO_STR(Type, Code) \
   ((Type == 0) ? "echo-reply" : \
    (Type == 3) && (Code == 0) ? "net-unreachable" : \
    (Type == 3) && (Code == 1) ? "host-unreachable" : \
    (Type == 3) && (Code == 2) ? "protocol-unreachable" : \
    (Type == 3) && (Code == 3) ? "port-unreachable" : \
    (Type == 3) && (Code == 4) ? "fragmentation needed and DF set " : \
    (Type == 3) && (Code == 5) ? "source route failed" : \
    (Type == 4) ? "source quench" : \
    (Type == 5) && (Code == 0) ? "network-redirect" : \
    (Type == 5) && (Code == 1) ? "host-redirect" : \
    (Type == 5) && (Code == 2) ? "service-network-redirect" : \
    (Type == 5) && (Code == 3) ? "service-host-redirect" : \
    (Type == 8) ? "echo" : \
    (Type == 11) && (Code == 0) ? "TTL-exceeded" : \
    (Type == 11) && (Code == 1) ? "fragment reassembly time exceeded" : \
    (Type == 12) ? "parameter problem" : \
    (Type == 13) ? "timestamp request" : \
    (Type == 14) ? "timestamp-reply" : \
    (Type == 15) ? "information-request" : \
    (Type == 16) ? "information-reply" : \
    "unknown type")

uint32 g_ip_icmp_debug = 0;

#ifdef GREATBELT
uint32 g_ip_redirects_global_en = 0;
#else
uint32 g_ip_redirects_global_en = 1;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 32)
#define NLA_ALIGNTO           4
#define NLA_ALIGN(len)        (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN            ((int) NLA_ALIGN(sizeof(struct nlattr)))

struct nlattr {
    __u16           nla_len;
    __u16           nla_type;
};
#endif /* LINUX_VERSION_CODE */
extern int32  memory_check_pktdiscard;

#define PROTO_ICMPV6 0x3a /*or 58*/
#define ICMPV6_MLRM_V2 0x8f /*or 143*/

#ifdef HAVE_IPNET
#define sal_in6_addr        Ip_in_addr
#else
#define sal_in6_addr        in6_addr
#endif

/*copy from nd.h*/
/*struct ipv6hdr {
#if _GLB_HOST_IS_LE
    uint8     priority:4;
    uint8     version:4;
#else
    uint8     version:4;
    uint8     priority:4;
#endif
    uint8     flow_lbl[3];
    uint16   payload_len;
    uint8     nexthdr;
    uint8     hop_limit;
    struct sal_in6_addr    saddr;
    struct sal_in6_addr    daddr;
};*/

/* support nd modified by liwh for bug 47547, 2018-07-24 */
struct icmp6hdr {
       uint8     icmp6_type;
       uint8     icmp6_code;
       uint16   icmp6_cksum;

       union {
           uint32   un_data32[1];
           uint16   un_data16[2];
           uint8     un_data8[4];
       } ;
};
//#define IPV6_HDR_LEN    40
#define ETH_HDR_LEN     14

#define SAL_ND_ROUTER_SOLICIT           133
#define SAL_ND_ROUTER_ADVERT            134
#define SAL_ND_NEIGHBOR_SOLICIT        135
#define SAL_ND_NEIGHBOR_ADVERT         136
#define SAL_ND_REDIRECT                        137

#define ICMPV6_ECHO_REQUEST               128
#define ICMPV6_ECHO_REPLY                    129
/* liwh end */

extern struct ctc_sai_mirror_info_s g_ctc_sai_mirror_info;

ctc_sai_hostif_info_t g_hostif_info;
extern uint64
ctc_sai_policer_get_pir_by_policer_id(uint32 policer_id);
#define ________SAI_SAI_INNER_API_FUNC
uint32_t sai_trap_action_to_ctc_action(sai_hostif_trap_id_t trapid, sai_packet_action_t action)
{
    if (trapid  < SAI_HOSTIF_TRAP_ID_SWITCH_CUSTOM_RANGE_BASE)
    {
        switch (action)
        {
        case SAI_PACKET_ACTION_DROP:
            return CTC_PDU_L2PDU_ACTION_TYPE_DISCARD;

        case SAI_PACKET_ACTION_FORWARD:
            return CTC_PDU_L2PDU_ACTION_TYPE_FWD;

        case SAI_PACKET_ACTION_TRAP:
            return CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;

        case SAI_PACKET_ACTION_LOG:
            return CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU;

        default:
            return CTC_E_NOT_SUPPORT;
        }
    }
    else
    {
        switch (action)
        {
        case SAI_PACKET_ACTION_DROP:
            return CTC_PDU_L3PDU_ACTION_TYPE_FWD;

        case SAI_PACKET_ACTION_FORWARD:
            return CTC_PDU_L3PDU_ACTION_TYPE_FWD;

        case SAI_PACKET_ACTION_TRAP:
            return CTC_PDU_L3PDU_ACTION_TYPE_FWD;

        case SAI_PACKET_ACTION_LOG:
            return CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU;

        default:
            return CTC_E_NOT_SUPPORT;
        }
    }

    return SAI_STATUS_FAILURE;
}

uint32_t sai_trap_id_to_ctc_reason_id(sai_hostif_trap_id_t trapid)
{
    switch (trapid)
    {

    /*
    * switch trap 
    */

    case SAI_HOSTIF_TRAP_ID_STP:
        return CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU;

    case SAI_HOSTIF_TRAP_ID_LACP:
        return CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_SLOW_PROTO;

    case SAI_HOSTIF_TRAP_ID_EAPOL:
        return CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL;

    case SAI_HOSTIF_TRAP_ID_LLDP:
        return CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_LLDP;

    case SAI_HOSTIF_TRAP_ID_ERPS:
         return CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_ERPS;

    case SAI_HOSTIF_TRAP_ID_PVRST:
        return CTC_PKT_CPU_REASON_CUSTOM_BASE;

    case SAI_HOSTIF_TRAP_ID_IGMP:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;

    case SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT:
        return CTC_PKT_CPU_REASON_IGMP_SNOOPING;
        
    case SAI_HOSTIF_TRAP_ID_L2_COPY_TO_CPU:
        return CTC_PKT_CPU_REASON_L2_COPY_CPU;

    /*
    * router trap 
    */

    case SAI_HOSTIF_TRAP_ID_ARP_REQUEST:
#ifdef GREATBELT
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2;

    case SAI_HOSTIF_TRAP_ID_ARP_RESPONSE:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2;

    case SAI_HOSTIF_TRAP_ID_DHCP:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP_V2;
#else
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP;

    case SAI_HOSTIF_TRAP_ID_ARP_RESPONSE:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP;

    case SAI_HOSTIF_TRAP_ID_DHCP:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP;
#endif

    case SAI_HOSTIF_TRAP_ID_ARP:
#ifdef GREATBELT
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2;
#else
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP;
#endif

    case SAI_HOSTIF_TRAP_ID_OSPF:  
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_OSPF;

    case SAI_HOSTIF_TRAP_ID_PIM:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM;

    case SAI_HOSTIF_TRAP_ID_VRRP:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_VRRP;

    case SAI_HOSTIF_TRAP_ID_BGP:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_BGP;

    case SAI_HOSTIF_TRAP_ID_DHCPV6:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP;

    case SAI_HOSTIF_TRAP_ID_OSPFV6:  
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_OSPF;

    case SAI_HOSTIF_TRAP_ID_VRRPV6: 
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_VRRP;

    case SAI_HOSTIF_TRAP_ID_BGPV6:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_BGP;

    case SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6;

    case SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_V2:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM;

    case SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_REPORT:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM;

    case SAI_HOSTIF_TRAP_ID_IPV6_MLD_V1_DONE:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM;

    case SAI_HOSTIF_TRAP_ID_MLD_V2_REPORT:
        return CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM;

    /* 
    * pipeline exceptions   
    */

    case SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR:  
        return CTC_PKT_CPU_REASON_L3_MTU_FAIL;

    case SAI_HOSTIF_TRAP_ID_TTL_ERROR:
        return CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL;

    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT:
        return CTC_PKT_CPU_REASON_L2_PORT_LEARN_LIMIT;

    case SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_MISMATCH:
        return CTC_PKT_CPU_REASON_L2_MOVE;

    case SAI_HOSTIF_TRAP_ID_CPU_MIRROR:
        return CTC_PKT_CPU_REASON_MIRRORED_TO_CPU;

    case SAI_HOSTIF_TRAP_ID_SCL_MATCH:
        return CTC_PKT_CPU_REASON_SCL_MATCH;

    /*
     * Centec extension, added by lixd, 2017-02-08 
     */
    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L3_COPY_CPU:
    case SAI_HOSTIF_TRAP_ID_FWD_TO_CPU:
        return CTC_PKT_CPU_REASON_L3_COPY_CPU;

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER:
        return CTC_PKT_CPU_REASON_CUSTOM_BASE + 1; /*GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP:
        return CTC_PKT_CPU_REASON_PTP;

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_PROTOCOL_MAC:
        return CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC;

    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_GROUP_MAC:
        return CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC;
        
//    case SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_CPU_MIRROR:
 //       return CTC_PKT_CPU_REASON_MIRRORED_TO_CPU;

    default:
        return CTC_PKT_CPU_REASON_CUSTOM_BASE;
    }

    return CTC_PKT_CPU_REASON_CUSTOM_BASE;
}

void
ctc_sai_call_cmdel_dma()
{
    /*SYSTEM MODIFIED by xgu for cmodel performance optimize, 2018-10-11*/
    /*Do not use seperate thread call in cm_sim_dma_pkt_engine(), just call cmodel function to send packet*/
#if defined(_GLB_UML_SYSTEM_) && defined(TSINGMA)
    extern  int32 _cm_sim_dma_engine_chip_process(uint8 chip_id);
    _cm_sim_dma_engine_chip_process(0);
#endif
}

void
ctc_sai_set_reason_channel(const uint32_t reason_id, const sai_hostif_trap_channel_t channel)
{
    stbl_copp_reason_key_t key;
    stbl_copp_reason_t *p_sdb_reason = NULL;

    sal_memset(&key, 0, sizeof(key));
    key.reason_id = reason_id;
    p_sdb_reason = stbl_copp_reason_get_copp_reason(&key);
    if (p_sdb_reason)
    {
        p_sdb_reason->channel = channel;
    }
}

void
ctc_sai_set_reason_trap(const uint32_t reason_id, const sai_int32_t hostif_trap_id)
{
    stbl_copp_reason_key_t key;
    stbl_copp_reason_t *p_sdb_reason = NULL;

    sal_memset(&key, 0, sizeof(key));
    key.reason_id = reason_id;
    p_sdb_reason = stbl_copp_reason_get_copp_reason(&key);
    if (p_sdb_reason)
    {
        p_sdb_reason->trap_id = hostif_trap_id;
    }
}

sai_hostif_trap_channel_t
ctc_sai_get_reason_channel(const uint32_t reason_id)
{
    stbl_copp_reason_key_t key;
    stbl_copp_reason_t *p_sdb_reason = NULL;

    sal_memset(&key, 0, sizeof(key));
    key.reason_id = reason_id;
    p_sdb_reason = stbl_copp_reason_get_copp_reason(&key);
    if (p_sdb_reason)
    {
        return p_sdb_reason->channel;
    }
    else
    {
        return SAI_HOSTIF_TRAP_CHANNEL_NOPROCESS;
    }
}

sai_int32_t
ctc_sai_get_reason_trap(const uint32_t reason_id)
{
    stbl_copp_reason_key_t key;
    stbl_copp_reason_t *p_sdb_reason = NULL;

    sal_memset(&key, 0, sizeof(key));
    key.reason_id = reason_id;
    p_sdb_reason = stbl_copp_reason_get_copp_reason(&key);
    if (p_sdb_reason)
    {
        return p_sdb_reason->trap_id;
    }
    else
    {
        return -1;
    }
}

static uint32_t
_hostif_fd_hash_make(
    _In_  void* data)
{
    stbl_hostif_t *p_sdb_hif = (stbl_hostif_t*)data;
    return ctc_hash_caculate(sizeof(uint32), &p_sdb_hif->fd);
}

static bool
_hostif_fd_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    stbl_hostif_t* p_sdb_hif = data;
    stbl_hostif_t* p_sdb_hif1 = data1;

    if (p_sdb_hif->fd == p_sdb_hif1->fd)
    {
        return TRUE;
    }

    return FALSE;
}

static uint32_t
_hostif_port_hash_make(
    _In_  void* data)
{
    stbl_hostif_t *p_sdb_hif = (stbl_hostif_t*)data;

    return ctc_hash_caculate(sizeof(sai_object_id_t), &p_sdb_hif->port_oid);
}

static bool
_hostif_port_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    stbl_hostif_t* p_sdb_hif = data;
    stbl_hostif_t* p_sdb_hif1 = data1;

    if (p_sdb_hif->port_oid == p_sdb_hif1->port_oid)
    {
        return TRUE;
    }

    return FALSE;
}

static uint32_t
_hostif_vlan_hash_make(
    _In_  void* data)
{
    stbl_hostif_t *p_sdb_hif = (stbl_hostif_t*)data;
    uint32 u32_vid = 0;
    u32_vid = p_sdb_hif->vid;
    return ctc_hash_caculate(sizeof(uint32), &u32_vid);
}

static bool
_hostif_vlan_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    stbl_hostif_t* p_sdb_hif = data;
    stbl_hostif_t* p_sdb_hif1 = data1;

    if (p_sdb_hif->vid == p_sdb_hif1->vid)
    {
        return TRUE;
    }

    return FALSE;
}

stbl_copp_group_t*
ctc_sai_copp_get_group(const sai_object_id_t trap_group_id)
{
    stbl_copp_group_key_t key;

    sal_memset(&key, 0, sizeof(key));
    key.group_id = CTC_SAI_OBJECT_INDEX_GET(trap_group_id);
    return stbl_copp_group_get_copp_group(&key);
}

stbl_copp_trap_t*
ctc_sai_copp_get_trap(const uint32_t trap_id)
{
    stbl_copp_trap_key_t key;
    
    sal_memset(&key, 0, sizeof(key));
    key.trap_id = trap_id;
    return stbl_copp_trap_get_copp_trap(&key);
}

stbl_hostif_t*
ctc_sai_hostif_get_by_hostif_id(sai_object_id_t hif_oid)
{
    stbl_hostif_key_t key;    
    key.hif_oid = hif_oid;
    return stbl_hostif_get_hostif(&key);
}

stbl_hostif_t*
ctc_sai_hostif_get_by_fd(const int32 fd)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_hostif_t hostif;

    hostif.fd = fd;
    return ctclib_hash_lookup(p_sai_glb->hostif_fd_hash, &hostif);
}

stbl_hostif_t*
ctc_sai_hostif_get_by_port(sai_object_id_t port_oid)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_hostif_t hostif;

    hostif.port_oid = port_oid;
    return ctclib_hash_lookup(p_sai_glb->hostif_portif_hash, &hostif);
}

stbl_hostif_t*
ctc_sai_hostif_get_by_vlan(uint16 vid)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_hostif_t hostif;

    hostif.vid = vid;
    return ctclib_hash_lookup(p_sai_glb->hostif_vlanif_hash, &hostif);
}

stbl_hostif_t*
ctc_sai_hostif_db_alloc(char *ifname, int32 fd, sai_object_id_t port_oid, uint32 vlanid)
{
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    stbl_hostif_t hif;
    stbl_hostif_t *p_sdb_hif = NULL;
    ctc_nh_info_t nh_info;
    uint32 nhid = 0;
    uint32 gport = 0;
    uint32 index = 0;
    int ret = 0;

    ret = ctclib_opb_alloc_offset(&p_port_glb->hostif_id_opb, &index);
    if (ret)
    {
        return NULL;
    }

    sal_memset(&hif, 0, sizeof(hif));
    sal_memset(&nh_info, 0, sizeof(nh_info));

    hif.key.hif_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_HOST_INTERFACE, index);
    hif.fd = fd;
    hif.port_oid = port_oid;
    hif.vid = vlanid;
    sal_strncpy(hif.ifname, ifname, GLB_IFNAME_SZ);
    
    if (vlanid)
    {
        /* for vlan interface, all ports basic dsnh_offset is same, use gport 0's dsnh_offset */
        gport = 0;
    }
    else
    {
        ctc_sai_port_objectid_to_gport(port_oid, &gport);
    }
    ctc_nh_get_l2uc(gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nhid);
    ctc_nh_get_nh_info(nhid, &nh_info);
    hif.nexthop_ptr = nh_info.dsnh_offset[0];

    stbl_hostif_add_hostif(&hif);
    p_sdb_hif = stbl_hostif_get_hostif(&hif.key);
    if (NULL == p_sdb_hif)
    {
        return NULL;
    }

    ctclib_hash_get(p_sai_glb->hostif_fd_hash, (void*)p_sdb_hif, ctclib_hash_alloc_intern);
    ctclib_hash_get(p_sai_glb->hostif_portif_hash, (void*)p_sdb_hif, ctclib_hash_alloc_intern);
    if (0 != vlanid)
    {
        ctclib_hash_get(p_sai_glb->hostif_vlanif_hash, (void*)p_sdb_hif, ctclib_hash_alloc_intern);
    }
    return p_sdb_hif;
}

void
ctc_sai_hostif_db_release(stbl_hostif_t *p_sdb_hif)
{
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    if (NULL == p_sdb_hif)
    {
        return;
    }

    ctclib_opb_free_offset(&p_port_glb->hostif_id_opb, CTC_SAI_OBJECT_INDEX_GET(p_sdb_hif->key.hif_oid));
    if (0 != p_sdb_hif->vid)
    {
        ctclib_hash_release(p_sai_glb->hostif_vlanif_hash, (void*)p_sdb_hif);
    }
    ctclib_hash_release(p_sai_glb->hostif_portif_hash, (void*)p_sdb_hif);
    ctclib_hash_release(p_sai_glb->hostif_fd_hash, (void*)p_sdb_hif);

    stbl_hostif_del_hostif(&p_sdb_hif->key);
}

int32
ctc_sai_reason_pdu_enable(uint32_t reason_id, uint32_t gport, uint32_t action)
{
    switch (reason_id)
    {
    case (CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP):
        ctc_port_set_property(gport, CTC_PORT_PROP_L3PDU_ARP_ACTION, action);
        break;

    case (CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU):
        ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_BPDU, action);
        break;

    case (CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_LLDP):
        ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_LLDP, action);
        break;

    case (CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_SLOW_PROTO):
        ctc_l2pdu_set_port_action (gport, CTC_L2PDU_ACTION_INDEX_SLOW_PROTO, action);
        break;
    }

    return SAI_STATUS_SUCCESS;
}

/*added by yejl for bug 47118, 2018-12-20*/
int32
ctc_sai_pause_packet_init(void)
{
    ctc_pdu_global_l2pdu_action_t l2pdu_action;
    ctc_pdu_l2pdu_key_t l2pdu_key;
    int32 ret = 0;

    sal_memset(&l2pdu_key, 0, sizeof(l2pdu_key));
    l2pdu_key.l2hdr_proto = 0;
    CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x01, 0x80, 0xc2, 0x00, 0x00, 0x01);
    CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    ret = ctc_l2pdu_classify_l2pdu(CTC_PDU_L2PDU_TYPE_MACDA, SAI_L2PDU_KEY_INDEX_L2PROTO_PROTOCOL_MAC, &l2pdu_key);
    if (ret)
    {
        return ret;
    }

    sal_memset(&l2pdu_action, 0, sizeof(l2pdu_action));
    l2pdu_action.copy_to_cpu  = 1;
    l2pdu_action.entry_valid  = 1;
    l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_PAUSE_DMAC;    
    ret = ctc_l2pdu_set_global_action(CTC_PDU_L2PDU_TYPE_MACDA, SAI_L2PDU_KEY_INDEX_L2PROTO_PROTOCOL_MAC, &l2pdu_action);
    return ret;
}
/*end*/

int ctc_sai_erps_init(void)
{
    ctc_pdu_global_l2pdu_action_t l2pdu_action;
    ctc_pdu_l2pdu_key_t l2pdu_key;
    int32 ret = 0;
    FILE *startup_config_fd = NULL;
    char buf[256];
    bool erps_mode_huawei = FALSE;

    startup_config_fd = sal_fopen("/mnt/flash/startup-config.conf", "r");
    if (startup_config_fd)
    {
        while (sal_fgets(buf, 128, startup_config_fd))
        {   
            if (!sal_strncmp(buf, "erps mode rrpp", sal_strlen("erps mode rrpp")))
            {
                erps_mode_huawei = TRUE;
                break;
            }
        }
        sal_fclose(startup_config_fd);
    }

    if (!erps_mode_huawei)
    {
        /* ERPS hello/link down/flush address */  
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x01, 0x80, 0x63, 0x07, 0x00, 0x00);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                CTC_PDU_L2PDU_TYPE_MACDA,
                SAI_L2PDU_KEY_INDEX_ERPS_COMMON,
                &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu common return %d\n", ret);
        }
        
//        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;    
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_COMMON,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action common return %d\n", ret);
        }
    
        /* ERPS edge hello/major fault address */  
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x01, 0x7A, 0x4F, 0x48, 0x26);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_FLUSH,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }
    
//        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_FLUSH,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }
    }
    else
    {
        /* modified by wangjj for fix erps bug 42041, 2016-12-14 */
        /* ERPS hello/link down/flush address */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x0f, 0xe2, 0x07, 0x82, 0x00);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00);
        ret = ctc_l2pdu_classify_l2pdu(
                CTC_PDU_L2PDU_TYPE_MACDA,
                SAI_L2PDU_KEY_INDEX_ERPS_COMMON,
                &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu common return %d\n", ret);
        }

//        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_COMMON,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action common return %d\n", ret);
        }

        /* ERPS edge hello/major fault address */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x01, 0x80, 0x63, 0x07, 0x00, 0x00);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_FLUSH,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }

 //       l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_FLUSH,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }

        #if 0
        /* erps addr1 for hw */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x0f, 0xE2, 0x07, 0x82, 0x17);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR1,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }
    
        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR1,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }
    
        /* erps addr2 for hw */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x0f, 0xE2, 0x07, 0x82, 0x57);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR2,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }
    
        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR2,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }

        /* erps addr3 for hw */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x0f, 0xE2, 0x07, 0x82, 0x97);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR3,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }
    
        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR3,
                 &l2pdu_action);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }
    
        /* erps addr4 for hw */
        l2pdu_key.l2hdr_proto = 0;
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac, 0x00, 0x0f, 0xE2, 0x07, 0x82, 0xd6);
        CTC_SAI_CPU_TRAFFIC_SET_MAC(l2pdu_key.l2pdu_by_mac.mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
        ret = ctc_l2pdu_classify_l2pdu(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR4,
                 &l2pdu_key);
        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_classify_l2pdu flush return %d\n", ret);
        }
    
        l2pdu_action.bypass_all   = 0;
        l2pdu_action.copy_to_cpu  = 1;
        l2pdu_action.entry_valid  = 1;
        l2pdu_action.action_index = CTC_L2PDU_ACTION_INDEX_ERPS;
        ret = ctc_l2pdu_set_global_action(
                 CTC_PDU_L2PDU_TYPE_MACDA,
                 SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR4,
                 &l2pdu_action);

        if (ret)
        {
            sal_printf("ctc_sai_erps_init ctc_l2pdu_set_global_action flush return %d\n", ret);
        }
        #endif
    }

    return ret;
}

extern int32  cut_through_en;
int32 ctc_sai_cpu_traffic_init(void)
{
    ctc_qos_queue_cfg_t    qos_queue_cfg;
    uint32 cpu_reason_id = 0;
    int32 ret = 0;
    
    /*init all cpu reason to default queue*/
    for(cpu_reason_id = CTC_PKT_CPU_REASON_DROP; cpu_reason_id <= CTC_PKT_CPU_REASON_ARP_MISS; cpu_reason_id++)
    {
        /*set l3_copy_to_cpu (management traffic) trap to queue 5*/
        if(CTC_PKT_CPU_REASON_L3_COPY_CPU == cpu_reason_id)
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.reason_map.queue_id = 5;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }     
     /*
            //set copp protocol cpu mirror queue 0, these reasons is ilooped to cpu
            else if(CTC_SAI_COPP_EXT_CPU_MIRROR == cpu_reason_id)
            {
                sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
                qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;

                qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
                qos_queue_cfg.value.reason_map.queue_id = 0;
                qos_queue_cfg.value.reason_map.reason_group = 0;
                ret += ctc_qos_set_queue(&qos_queue_cfg);

                sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
                qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
                qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
                qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
                qos_queue_cfg.value.stats.stats_en = 1;
                ret += ctc_qos_set_queue(&qos_queue_cfg);
            }
        */
        /*set copp protocol traffic mapping queue 4, these reasons is ilooped to cpu*/
        else if(CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU == cpu_reason_id 
           || CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_SLOW_PROTO== cpu_reason_id 
           || CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_ERPS == cpu_reason_id 
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_OSPF == cpu_reason_id 
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_BGP == cpu_reason_id 
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_VRRP == cpu_reason_id 
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM == cpu_reason_id)

        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.reason_map.queue_id = 4;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }

        /*set copp protocol traffic mapping queue 3, these reasons is ilooped to cpu*/
        else if(CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_LLDP== cpu_reason_id 
           || CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL== cpu_reason_id 
           || CTC_PKT_CPU_REASON_IGMP_SNOOPING == cpu_reason_id 
           || CTC_PKT_CPU_REASON_PTP == cpu_reason_id 
#ifdef GREATBELT
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP_V2 == cpu_reason_id)
#else
           || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP == cpu_reason_id)
#endif
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.reason_map.queue_id = 3;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }

        /*set arp trap to queue 1*/
#ifdef GREATBELT
        else if(CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2 == cpu_reason_id
             || CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6 == cpu_reason_id
             || CTC_PKT_CPU_REASON_LINK_ID_FAIL == cpu_reason_id)
#elif DUET2
        else if(CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP == cpu_reason_id
             ||CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6 == cpu_reason_id
             || CTC_PKT_CPU_REASON_LINK_ID_FAIL == cpu_reason_id)
#elif TSINGMA
        else if(CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP == cpu_reason_id
             ||CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6 == cpu_reason_id
             || CTC_PKT_CPU_REASON_LINK_ID_FAIL == cpu_reason_id)
#else
        else if(CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP == cpu_reason_id)
#endif
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.reason_map.queue_id = 1;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }
        /* modified by huwx and wangl for bug 45725, 2017-11-27 */
        /*osap ttl reason map to queue 0 same with ospf */
        else if (CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL == cpu_reason_id ||
                 CTC_PKT_CPU_REASON_L3_MTU_FAIL == cpu_reason_id)
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.reason_map.queue_id = 0;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }
        /* huwx end */
        /*modified by chenc for bug 43956, 2019-04-15*/
#ifdef GOLDENGATE
        else if (CTC_PKT_CPU_REASON_FWD_CPU == cpu_reason_id && 1 == cut_through_en)
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id;
            qos_queue_cfg.value.reason_map.queue_id = 48;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }
#endif
        /*chenc end*/

		/*add by zhw to support ipmc*/
		else if (CTC_PKT_CPU_REASON_L3_IP_OPTION == cpu_reason_id)
		{
			sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id;
            qos_queue_cfg.value.reason_map.queue_id = 1;
            qos_queue_cfg.value.reason_map.reason_group = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
			
			/*shape cpu-reason 81 pir 200*/
			ctc_qos_shape_t qos_shape;
			ctc_qos_shape_queue_t shape;
			sal_memset(&qos_shape, 0, sizeof(qos_shape));
			sal_memset(&shape, 0, sizeof(shape));

			shape.queue.cpu_reason = cpu_reason_id;
			shape.queue.queue_id = 0;
    		shape.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    		qos_shape.type = CTC_QOS_SHAPE_QUEUE;
			shape.pir = 200;
        	shape.pbs = CTC_QOS_SHP_TOKE_THRD_DEFAULT;
        	shape.enable = 1;
			sal_memcpy(&qos_shape.shape, &shape, sizeof(shape));

			ctc_qos_set_shape(&qos_shape);
		}
		/*zhw end add*/
        else
        {
            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
            qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id;
            qos_queue_cfg.value.reason_map.queue_id = 0;
            qos_queue_cfg.value.reason_map.reason_group = 0;
            ret += ctc_qos_set_queue(&qos_queue_cfg);

            sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
            qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
            qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id;
            qos_queue_cfg.value.stats.stats_en = 1;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }
		
        /*should discard the following reason packets, because we re-mapping 
        different reason to different queue, so the sdk inited reason is modified, must reinit here*/
        if (CTC_PKT_CPU_REASON_L3_MARTIAN_ADDR == cpu_reason_id
            || CTC_PKT_CPU_REASON_L3_URPF_FAIL == cpu_reason_id
            || CTC_PKT_CPU_REASON_IPMC_TTL_CHECK_FAIL == cpu_reason_id
            || CTC_PKT_CPU_REASON_MPLS_TTL_CHECK_FAIL == cpu_reason_id
            || CTC_PKT_CPU_REASON_GRE_UNKNOWN == cpu_reason_id
            || CTC_PKT_CPU_REASON_LABEL_MISS == cpu_reason_id
            || CTC_PKT_CPU_REASON_OAM_HASH_CONFLICT == cpu_reason_id
            /* modified by liwh for bug 47344, 2018-05-22 */
            || CTC_PKT_CPU_REASON_DROP == cpu_reason_id)  
            /* liwh end */
        {
            /*drop reason*/
            qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
            qos_queue_cfg.value.reason_dest.cpu_reason = cpu_reason_id;
            qos_queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_DROP;
            ret += ctc_qos_set_queue(&qos_queue_cfg);
        }
        
    }

    /*openflow tp controller to queue 2*/
    /*CTC_PKT_CPU_REASON_CUSTOM_BASE + GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/
    cpu_reason_id = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1;
    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
    qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.reason_map.queue_id = 2;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
    qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
    qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.stats.stats_en = 1;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    //l2 protocol protocol mac, queue 4
    cpu_reason_id = CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC;
    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
    qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.reason_map.queue_id = 4;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
    qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
    qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.stats.stats_en = 1;
    ret += ctc_qos_set_queue(&qos_queue_cfg);
    // end l2protocol protocol-mac

    //l2 protocol group mac, queue 4
    cpu_reason_id = CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC;
    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
    qos_queue_cfg.value.reason_map.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.reason_map.queue_id = 4;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    sal_memset(&qos_queue_cfg, 0, sizeof(qos_queue_cfg));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN;
    qos_queue_cfg.value.stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;  
    qos_queue_cfg.value.stats.queue.cpu_reason = cpu_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.stats.stats_en = 1;
    ret += ctc_qos_set_queue(&qos_queue_cfg);
    // end l2protocol group-mac
            
#ifdef GREATBELT
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MODE;
    qos_queue_cfg.value.reason_mode.group_type = CTC_PKT_CPU_REASON_GROUP_FORWARD; 
    qos_queue_cfg.value.reason_mode.group_id = 0;
    qos_queue_cfg.value.reason_mode.mode = CTC_PKT_MODE_DMA;
    ret += ctc_qos_set_queue(&qos_queue_cfg);
#endif
#if 0
    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
    qos_queue_cfg.value.reason_map.cpu_reason = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1;/*for hybrid packet in GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/
    qos_queue_cfg.value.reason_map.queue_id = 2;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    qos_queue_cfg.value.reason_dest.cpu_reason = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1;
    qos_queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    ret += ctc_qos_set_queue(&qos_queue_cfg);
#endif   

/*added by wangl for hybrid and tap, bug45698*/
#ifndef OFPRODUCT
#ifndef TAPPRODUCT
    ctc_sai_erps_init();
#endif
#endif
    ctc_sai_pause_packet_init();
    return ret;
}

int32
ctc_sock_set_nonblocking (sal_sock_handle_t sock, int32 state)
{
    int val = 0;

    val = fcntl (sock, F_GETFL, 0);
    if (SAL_SOCK_ERROR != val)
    {
        fcntl (sock, F_SETFL, (state ? val | O_NONBLOCK : val & (~O_NONBLOCK)));
        return 0;
    }
    else
    {
        return errno;
    }
}

int32
ctc_netlink_talk(struct nlmsghdr *n)
{
    struct sockaddr_nl snl;
    struct iovec iov = { (void *) n, n->nlmsg_len };
    struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
    int32 status = 0;
    int32 save_errno = 0;

    sal_memset (&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;

    n->nlmsg_seq = ++g_hostif_info.seq;
    status = sendmsg(g_hostif_info.sock, &msg, 0);
    save_errno = errno;

    if (status < 0)
    {
        printf("_lib_netlink_talk sendmsg() error: %s", sal_strerror(save_errno));
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

int32
ctc_get_ifindex(struct nlmsghdr *nhmsg, void *data)
{
    struct ifinfomsg *ifinfo = NULL;
    int *ifindex = data;

    ifinfo = (void *)NLMSG_DATA(nhmsg);

    *ifindex = ifinfo->ifi_index;
    return SAI_STATUS_SUCCESS;
}

static int32
ctc_netlink_parse(int32 (*dispatch)(struct nlmsghdr *, void *), void *data)
{
    int32 status = 0;
    int32 rc = SAI_STATUS_SUCCESS;

    while (1)
    {
        char buf[4096];
        struct iovec iov = {buf, sizeof buf};
        struct sockaddr_nl snl;
        struct msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};
        struct nlmsghdr *h;

        status = recvmsg(g_hostif_info.sock, &msg, 0);
        if (status < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            
            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                break;
            }
            
            if (ESPIPE == errno)
            {
                printf("recvmsg overrun: %s", sal_strerror(errno));
                break;
            }
        }

        if (snl.nl_pid != 0)
        {
            continue;
        }

        if (status == 0)
        {
            return SAI_STATUS_FAILURE;
        }

        if (msg.msg_namelen != sizeof snl)
        {
            return SAI_STATUS_FAILURE;
        }

        for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, status); h = NLMSG_NEXT (h, status))
        {
            /* Finish of reading. */
            if (h->nlmsg_type == NLMSG_DONE)
            {
                return rc;
            }

            /* Error handling. */
            if (h->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);

                /* Sometimes the nlmsg_type is NLMSG_ERROR but the err->error
                 *                  is 0. This is a success. */
                if (err->error == 0)
                {
                    /* return if not a multipart message, otherwise continue */
                    if (!(h->nlmsg_flags & NLM_F_MULTI))
                    {
                        return SAI_STATUS_SUCCESS;
                    }
                    continue;
                }

                if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
                {
                    return SAI_STATUS_FAILURE;
                }

                return SAI_STATUS_FAILURE;
            }
     
            rc = (*dispatch) (h, data);
            if (rc < 0)
            {
                return rc;
            }
        }

        /* After error care. */
        if (msg.msg_flags & MSG_TRUNC)
        {
            continue;
        }
        
        if (status)
        {
            return SAI_STATUS_FAILURE;
        }
    }

    return rc;
}

int32
ctc_netlink_get_link(int *ifindex, char *ifname)
{
    struct nlattr *pt_nlattr = NULL;
    uint8 *pt_buf            = NULL;
    char *pt_ifname          = NULL;

    struct netlink_link_msg {
        struct nlmsghdr header;
        struct ifinfomsg ifinfo;
        uint8 buffer[1024];
    }link_msg;

    sal_memset(&link_msg, 0, sizeof(struct netlink_link_msg));
    link_msg.header.nlmsg_len = 0;
    link_msg.header.nlmsg_type = RTM_GETLINK;
    link_msg.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    link_msg.header.nlmsg_seq = 0;
    link_msg.header.nlmsg_pid = 0;

    link_msg.ifinfo.ifi_family = 0;
    link_msg.ifinfo.ifi_type   = 0;
    link_msg.ifinfo.ifi_index  = 0;
    link_msg.ifinfo.ifi_flags  = 0;
    link_msg.ifinfo.ifi_change = 0;

    link_msg.header.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct ifinfomsg);

    pt_buf = link_msg.buffer;
    pt_nlattr = (void *)pt_buf;
    pt_nlattr->nla_type = IFLA_IFNAME;
    pt_nlattr->nla_len  = NLA_ALIGN(16 + sizeof(struct nlattr));
    pt_ifname = (void*)(pt_buf + NLA_ALIGN(sizeof(struct nlattr)));
    sal_strcpy(pt_ifname, ifname);
    pt_buf = pt_buf + NLA_ALIGN(16 + sizeof(struct nlattr));
    link_msg.header.nlmsg_len += NLA_ALIGN(16  + sizeof(struct nlattr));

    ctc_netlink_talk((void *)&link_msg);

    ctc_netlink_parse(ctc_get_ifindex, ifindex);    
    return SAI_STATUS_SUCCESS;
}

int32
ctc_create_net_device(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    char *clonedev = "/dev/net/tun";

    /* Arguments taken by the function:
    *
    * char *dev: the name of an interface (or '\0'). MUST have enough
    * space to hold the interface name if '\0' is passed
    * int flags: interface flags (eg, IFF_TUN etc.)
    */
    /* open the clone device */

    if( (fd = sal_open(clonedev, O_RDWR)) < 0 ) 
    {
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    sal_memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI; /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    if (*dev) 
    {
        /* if a device name was specified, put it in the structure; otherwise,
        * the kernel will try to allocate the "next" device of the
        * specified type */
        sal_strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    /* try to create the device */
    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) 
    {
        close(fd);
        return err;
    }

    return fd;
}

int32
ctc_remove_net_device(int32 fd)
{
    sal_close(fd);

    return SAI_STATUS_SUCCESS;
}

int32
ctc_lib_netlink_socket()
{
    struct sockaddr_nl snl;
    socklen_t namelen;
    int32 sock = -1;
    int32 rc = SAI_STATUS_SUCCESS;

    sock = sal_socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0)
    {
        return SAI_STATUS_FAILURE;
    }

    ctc_sock_set_nonblocking(sock, TRUE);

    sal_memset(&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = 0x1;
    /* Bind the socket to the netlink structure for anything. */
    rc = sal_bind(sock, (struct sal_sockaddr*)&snl, sizeof snl);
    if (rc < 0)
    {
        return SAI_STATUS_FAILURE;
    }

    namelen = sizeof snl;
    rc = getsockname(sock, (struct sal_sockaddr*)&snl, &namelen);
    if (rc < 0 || namelen != sizeof snl)
    {
        return SAI_STATUS_FAILURE;
    }

    g_hostif_info.snl = snl;
    g_hostif_info.sock = sock;
    return rc;
}

int32
ctc_packet_epool_init()
{
    g_hostif_info.epoll_sock = epoll_create(2048);
    if (g_hostif_info.epoll_sock < 0)
    {
        return SAI_STATUS_FAILURE;
    }

    ctc_sock_set_nonblocking(g_hostif_info.epoll_sock, TRUE);

    g_hostif_info.evl.data.fd = -1;
    g_hostif_info.evl.events = EPOLLIN;
    return SAI_STATUS_SUCCESS;
}

int32
ctc_packet_remove_vlan_for_nd(ctc_pkt_rx_t* p_pkt_rx)
{
    uint8 *pst_vlan = NULL;

    pst_vlan = p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN + 12;

    if (pst_vlan[0] != 0x81 || pst_vlan[1] != 0x00)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memmove(pst_vlan, pst_vlan + 4, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN - 12 - 4);
    p_pkt_rx->pkt_buf->len = p_pkt_rx->pkt_buf->len - 4;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_packet_remove_vlan(ctc_pkt_rx_t* p_pkt_rx)
{
    uint8 *pst_vlan = NULL;

    pst_vlan = p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN + 12;

    if (pst_vlan[0] != 0x81 || pst_vlan[1] != 0x00)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memmove(pst_vlan, pst_vlan + 4, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN - 12 - 4);
    return SAI_STATUS_SUCCESS;
}


#define MAX_PKT_BUFFER  20000

stbl_hostif_t*
ctc_sai_hostif_get_vlanif_for_mtu_excep(ctc_pkt_rx_t* p_pkt_rx)
{
#ifdef WB_TODO
    struct ctc_listnode *node = NULL;
    struct ctc_listnode *next = NULL;
    stbl_hostif_t *p_sdb_hif = NULL;
    
    ctc_sai_hostif_t    *p_db_hostif = NULL;
    ctc_port_bitmap_t   port_bitmap = {0};
    int32               is_fdb_exist = 0;

    CTC_SAI_SLIST_LOOP_DEL(g_hostif_info.hostif_port_list, p_db_hostif, node, next)
    {
        if (NULL != p_db_hostif && (SAI_VLAN_DEFAULT_VID < p_db_hostif->vlanid && SAI_VLAN_MAX > p_db_hostif->vlanid))
        {
            ctc_vlan_get_ports(p_db_hostif->vlanid, 0, port_bitmap);
            if (CTC_BMP_ISSET(port_bitmap,  p_pkt_rx->rx_info.src_port))
            {
                is_fdb_exist = ctc_fdb_get_by_port_and_vlanid(p_db_hostif->vlanid, p_pkt_rx->rx_info.src_port);
                if (!is_fdb_exist)
                {
                    return p_db_hostif;
                }
            }
        }
    }
#endif
    return NULL;
}

int32
ctc_packet_update_l2_header(ctc_pkt_rx_t* p_pkt_rx, uint8 *pkt_buffer)
{
    uint8       *pst_l2hder = NULL;
    uint8       mac_sa[6] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x02};
    mac_addr_t  mac_da;
    uint8       eth_type[2] = {0x08, 0x00};
    uint8       pkt_ttl = 0;
    uint16      hdr_check = 0;

    sal_memset(mac_da, 0, sizeof(mac_da));
    sal_memcpy(pkt_buffer, p_pkt_rx->pkt_buf->data, p_pkt_rx->pkt_buf->len);
    pst_l2hder = pkt_buffer + CTC_PKT_HEADER_LEN;
    if (pst_l2hder[0] != 0x45)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memcpy(pkt_buffer, p_pkt_rx->pkt_buf->data, p_pkt_rx->pkt_buf->len);
    sal_memmove(pst_l2hder + 14, pst_l2hder, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);

    ctc_l3if_get_router_mac(mac_da);
    sal_memset(pst_l2hder, 0x0, 14);
    sal_memcpy(pst_l2hder, mac_da, sizeof(mac_addr_t));
    sal_memcpy(pst_l2hder + 6, mac_sa, sizeof(mac_sa));
    sal_memcpy(pst_l2hder + 12, eth_type, sizeof(eth_type));
    /* modified by wangjj for fix bug 40953, the TTL was decreased in EPE process, so add it */
    pkt_ttl = *(uint8*)(pst_l2hder + 22);
    pkt_ttl += 1;
    sal_memcpy(pst_l2hder + 22, &pkt_ttl, 1);

    sal_memcpy(pst_l2hder + 24, &hdr_check, 2);
    hdr_check = ip_fast_csum((unsigned char *)(pst_l2hder + 14), 5);
    sal_memcpy(pst_l2hder + 24, &hdr_check, 2);

    p_pkt_rx->pkt_buf->len += 14;
    //sal_memmove(p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, pst_l2hder, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN + 14);
    return SAI_STATUS_SUCCESS;
}

int32
ctc_packet_add_l2_header(ctc_pkt_rx_t* p_pkt_rx)
{
    uint8       *pst_l2hder = NULL;
    uint8       mac_sa[6] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x02};
    mac_addr_t  mac_da;
    uint8       eth_type[2] = {0x08, 0x00};
    uint8       pkt_ttl = 0;
    uint16      hdr_check = 0;

    sal_memset(mac_da, 0, sizeof(mac_da));
    pst_l2hder = p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN;
    if (pst_l2hder[0] != 0x45)
    {
        return SAI_STATUS_SUCCESS;
    }

    sal_memmove(pst_l2hder + 14, pst_l2hder, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);

    ctc_l3if_get_router_mac(mac_da);
    sal_memset(pst_l2hder, 0x0, 14);
    sal_memcpy(pst_l2hder, mac_da, sizeof(mac_addr_t));
    sal_memcpy(pst_l2hder + 6, mac_sa, sizeof(mac_sa));
    sal_memcpy(pst_l2hder + 12, eth_type, sizeof(eth_type));
    /* modified by wangjj for fix bug 40953, the TTL was decreased in EPE process, so add it */
    pkt_ttl = *(uint8*)(pst_l2hder + 22);
    pkt_ttl += 1;
    sal_memcpy(pst_l2hder + 22, &pkt_ttl, 1);

    sal_memcpy(pst_l2hder + 24, &hdr_check, 2);
    hdr_check = ip_fast_csum((unsigned char *)(pst_l2hder + 14), 5);
    sal_memcpy(pst_l2hder + 24, &hdr_check, 2);

    p_pkt_rx->pkt_buf->len += 14;
    sal_memmove(p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, pst_l2hder, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN + 14);
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_ARP_FDB_LEARNING

int32
ctc_fdb_get_portid(uint8 *mac_address, uint32 vlanid, sai_object_id_t *portid)
{
    uint32 gport = 0;
    int ret = 0;

    if (NULL == portid)
    {
        return SAI_STATUS_FAILURE;
    }

    ret = ctc_fdb_get_gport(mac_address, vlanid, &gport);
    if (SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }

    ctc_sai_port_gport_to_objectid(gport, portid);
    return SAI_STATUS_SUCCESS;
}

int32
ctc_fdb_get_gport(uint8 *mac_address, uint32 vlanid, uint32 *gport)
{
    ctc_l2_addr_t           query_buffer[1];
    ctc_l2_fdb_query_t      fdb_query;
    ctc_l2_fdb_query_rst_t  query_rst;
    int                     ret = 0;

    if (NULL == gport)
    {
        return SAI_STATUS_FAILURE;
    }

    sal_memset(&fdb_query, 0, sizeof(fdb_query));
    sal_memcpy(fdb_query.mac, mac_address, sizeof(mac_addr_t));
    fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN;
    //fdb_query.query_flag = CTC_L2_FDB_ENTRY_STATIC;
    fdb_query.query_flag = CTC_L2_FDB_ENTRY_ALL;
    fdb_query.fid        = vlanid;
    fdb_query.query_hw   = FALSE;

    query_rst.buffer_len = sizeof(ctc_l2_addr_t);
    query_rst.buffer = query_buffer;
    sal_memset(query_rst.buffer, 0, query_rst.buffer_len);

    ret = ctc_l2_get_fdb_entry(&fdb_query, &query_rst);
    if ((0 == ret) && (fdb_query.count > 0))
    {
        *gport = query_rst.buffer->gport;
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_FAILURE;
}

#define ________SAI_SAI_PACKET_

int32
ctc_sai_hostif_tx_ip_icmp_debug(stbl_hostif_t *p_sdb_hif, uint8 *p_pkt, uint32 len)
{
    struct ethhdr *eth = NULL;
    struct iphdr *iph = NULL, *orig_iph = NULL;
    struct icmphdr *icmp_hdr = NULL;
    char ifname_ext[GLB_IFNAME_SZ];
    
    if (!g_ip_icmp_debug)
    {
        return SAI_STATUS_SUCCESS;
    }
    
    eth = (struct ethhdr *)(p_pkt);
    if ((eth != NULL) && (ETH_P_IPV4 == ntohs(eth->h_proto)))
    {
        iph = (struct iphdr *)(p_pkt + sizeof(struct ethhdr));
        if ((iph != NULL) && (IPPROTO_ICMP == iph->protocol))
        {
            icmp_hdr = (struct icmphdr *)((uint8_t *)iph + sizeof(struct iphdr));
            if (icmp_hdr != NULL)
            {
                IFNAME_ETH2SHORT(p_sdb_hif->ifname, ifname_ext);
                
                if ((ICMP_DEST_UNREACH == icmp_hdr->type) || (ICMP_SOURCE_QUENCH == icmp_hdr->type) ||
                    (ICMP_REDIRECT == icmp_hdr->type) || (ICMP_TIME_EXCEEDED == icmp_hdr->type) ||
                    (ICMP_PARAMETERPROB == icmp_hdr->type))
                {
                    /*these types of ICMP packet carry IP head of original packet, should be included in debug logging*/
                    orig_iph = (struct iphdr *)((uint8_t *)icmp_hdr + sizeof(struct icmphdr));
                    if (orig_iph)
                    {
                        log_sys(M_MOD_SAI, E_DEBUG, "ICMP send to ifname %s: %s(Type=%d, Code=%d), "
                            "SrcMac = %02x%02x.%02x%02x.%02x%02x, DestMac = %02x%02x.%02x%02x.%02x%02x, "
                            "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d; Original IP header: Pro = %d, "
                            "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d, First 8 bytes = %08x %08x",
                            ifname_ext, 
                            UMPETH_ICMP_TYPE_TO_STR(icmp_hdr->type, icmp_hdr->code), icmp_hdr->type, icmp_hdr->code,
                            eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
                            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
                            NIPQUAD(iph->saddr), NIPQUAD(iph->daddr), orig_iph->protocol,
                            NIPQUAD(orig_iph->saddr), NIPQUAD(orig_iph->daddr),
                            ntohl(*(uint32*)((uint8_t*)orig_iph + sizeof(struct iphdr))),
                            ntohl(*((uint32*)((uint8_t*)orig_iph + sizeof(struct iphdr))+1)));
                    }
                }
                else
                {
                    log_sys(M_MOD_SAI, E_DEBUG, "ICMP send to ifname %s: %s(Type=%d, Code=%d), "
                        "SrcMac = %02x%02x.%02x%02x.%02x%02x, DestMac = %02x%02x.%02x%02x.%02x%02x, "
                        "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d, ICMP Id = 0x%x, ICMP Seq = %d",
                        ifname_ext, 
                        UMPETH_ICMP_TYPE_TO_STR(icmp_hdr->type, icmp_hdr->code), icmp_hdr->type, icmp_hdr->code,
                        eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
                        eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
                        NIPQUAD(iph->saddr), NIPQUAD(iph->daddr),
                        ntohs(icmp_hdr->un.echo.id), ntohs(icmp_hdr->un.echo.sequence));
                }
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_hostif_rx_ip_icmp_debug(stbl_hostif_t *p_sdb_hif, uint8 *p_pkt, uint32 len)
{
    struct ethhdr *eth = NULL;
    struct iphdr *iph = NULL, *orig_iph = NULL;
    struct icmphdr *icmp_hdr = NULL;
    char ifname_ext[GLB_IFNAME_SZ];
    
    if (!g_ip_icmp_debug)
    {
        return SAI_STATUS_SUCCESS;
    }
    
    eth = (struct ethhdr *)(p_pkt);
    if ((eth != NULL) && (ETH_P_IPV4 == ntohs(eth->h_proto)))
    {
        iph = (struct iphdr *)(p_pkt + sizeof(struct ethhdr));
        if ((iph != NULL) && (IPPROTO_ICMP == iph->protocol))
        {
            icmp_hdr = (struct icmphdr *)(p_pkt + sizeof(struct ethhdr) + sizeof(struct iphdr));
            if (icmp_hdr != NULL)
            {
                IFNAME_ETH2SHORT(p_sdb_hif->ifname, ifname_ext);
                
                if ((ICMP_DEST_UNREACH == icmp_hdr->type) || (ICMP_SOURCE_QUENCH == icmp_hdr->type) || 
                    (ICMP_REDIRECT == icmp_hdr->type) || (ICMP_TIME_EXCEEDED == icmp_hdr->type) || 
                    (ICMP_PARAMETERPROB == icmp_hdr->type))
                {
                    /*these types of ICMP packet carry IP head of original packet, should be included in debug logging*/
                    orig_iph = (struct iphdr *)((uint8_t *)icmp_hdr + sizeof(struct icmphdr));
                    if (orig_iph)
                    {
                        log_sys(M_MOD_SAI, E_DEBUG, "ICMP receive from ifname %s: %s(Type=%d, Code=%d), "
                            "SrcMac = %02x%02x.%02x%02x.%02x%02x, DestMac = %02x%02x.%02x%02x.%02x%02x, "
                            "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d; Original IP header: Pro = %d, "
                            "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d, First 8 bytes = %08x %08x",
                            ifname_ext, 
                            UMPETH_ICMP_TYPE_TO_STR(icmp_hdr->type, icmp_hdr->code), icmp_hdr->type, icmp_hdr->code, 
                            eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
                            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
                            NIPQUAD(iph->saddr), NIPQUAD(iph->daddr), orig_iph->protocol, 
                            NIPQUAD(orig_iph->saddr), NIPQUAD(orig_iph->daddr), 
                            ntohl(*(uint32 *)((uint8_t *)orig_iph + sizeof(struct iphdr))), 
                            ntohl(*((uint32 *)((uint8_t *)orig_iph + sizeof(struct iphdr))+1)));
                    }
                }
                else
                {
                    log_sys(M_MOD_SAI, E_DEBUG, "ICMP receive from ifname %s: %s(Type=%d, Code=%d), "
                        "SrcMac = %02x%02x.%02x%02x.%02x%02x, DestMac = %02x%02x.%02x%02x.%02x%02x, "
                        "SrcIP = %d.%d.%d.%d, DstIP = %d.%d.%d.%d, ICMP Id = 0x%x, ICMP Seq = %d",
                        ifname_ext, 
                        UMPETH_ICMP_TYPE_TO_STR(icmp_hdr->type, icmp_hdr->code), icmp_hdr->type, icmp_hdr->code, 
                        eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
                        eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
                        NIPQUAD(iph->saddr), NIPQUAD(iph->daddr), 
                        ntohs(icmp_hdr->un.echo.id), ntohs(icmp_hdr->un.echo.sequence));
                }
            }
        }
    }
    
    return SAI_STATUS_SUCCESS;
}

/*added by yejl for bug 51424, 51756, 2019-03-27, 2019-04-23*/
int32 
ctc_hostif_is_local_mirror_dest_port(sai_object_id_t port_oid)
{
    uint32 i = 0;
    uint32 j = 0;
    for (i = 0; i < 4; i++)
    {
        if (g_ctc_sai_mirror_info.session[i].monitor_type == SAI_MIRROR_TYPE_LOCAL)
        {
            for (j = 0; j < CTC_SAI_MIRROR_MULTI_DEST_PORT_NUM; j++)
            {
                if (g_ctc_sai_mirror_info.session[i].monitor_port[j] == port_oid)
                {
                    return TRUE; 
                }
            }
        }
    }
    return FALSE;
}


int32
ctc_packet_send_to_sdk(stbl_hostif_t *p_sdb_hif, char *buffer, int buffer_size)
{
    sai_attribute_t attr[1];
    ctc_pkt_tx_t pkt_tx;
    ctc_pkt_skb_t *p_skb = NULL;
    uint32 gport = 0;
    uint32 is_mirror_dest_port = FALSE;
    int ret = 0;

    sal_memset(&pkt_tx, 0, sizeof(ctc_pkt_tx_t));
    p_skb = &(pkt_tx.skb);

    ctc_packet_skb_init(p_skb);

    /* vlan interface */
    if (0 != p_sdb_hif->vid)
    {
        ctc_packet_skb_put(p_skb, buffer_size);
        sal_memcpy(p_skb->data, buffer, buffer_size);
        pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_SVID_VALID;

        /* first get port from static fdb */
        ret = ctc_fdb_get_gport(p_skb->data, p_sdb_hif->vid, &gport);

        /* flooding in vlan */
        if (SAI_STATUS_SUCCESS != ret)
        {
            pkt_tx.tx_info.flags |= CTC_PKT_FLAG_MCAST;
            pkt_tx.tx_info.src_svid = p_sdb_hif->vid;
            pkt_tx.tx_info.dest_group_id = p_sdb_hif->vid;
        }
        /* unicat to port */
        else
        {
            pkt_tx.tx_info.flags |= CTC_PKT_FLAG_NH_OFFSET_VALID;
            pkt_tx.tx_info.src_svid = p_sdb_hif->vid;
            pkt_tx.tx_info.nh_offset = p_sdb_hif->nexthop_ptr;
            pkt_tx.tx_info.dest_gport = gport;
        }
    }
    /* port interface or linkagg interface */
    else
    {
        ctc_packet_skb_put(p_skb, buffer_size);
        sal_memcpy(p_skb->data, buffer, buffer_size);
        ctc_sai_port_objectid_to_gport(p_sdb_hif->port_oid, &pkt_tx.tx_info.dest_gport);
        pkt_tx.tx_info.flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;

        /*added by yejl for bug 51424, 2019-03-27*/
        is_mirror_dest_port = ctc_hostif_is_local_mirror_dest_port(p_sdb_hif->port_oid);
        if (is_mirror_dest_port)
        {
            return SAI_STATUS_SUCCESS;
        }
    }

    pkt_tx.mode = CTC_PKT_MODE_DMA;
    pkt_tx.lchip = 0;
    pkt_tx.tx_info.ttl = 1;
    pkt_tx.tx_info.oper_type = CTC_PKT_OPER_NORMAL;
    pkt_tx.tx_info.is_critical = TRUE;

    attr[0].value.u32 = 0; /* GLB_PKT_UNKNOWN */
    ctc_sai_get_sys_info()->sai_switch_notification_table.on_cpu_packet_debug(p_skb->data, p_skb->tail - p_skb->data + 1, 1, attr);

    ctc_sai_hostif_tx_ip_icmp_debug(p_sdb_hif, p_skb->data, buffer_size);

    ret = ctc_packet_tx(&pkt_tx);
    
    ctc_sai_call_cmdel_dma();

    return ret;
}

uint16
_ctc_get_copp_vid(ctc_pkt_rx_t* p_pkt_rx)
{
    uint16 vid = 0;
    uint8 tid = 0;
    #ifdef USW
    uint16 cnt = 0;
    #else
    uint8 cnt = 0;
    #endif
    uint32 port;
    
    vid = (0 == p_pkt_rx->rx_info.fid) ? p_pkt_rx->rx_info.src_svid : p_pkt_rx->rx_info.fid;
    if ((vid == 0) && (CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_LLDP == p_pkt_rx->rx_info.reason 
        ||CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_BPDU == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_SLOW_PROTO == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_EAPOL == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_ERPS == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_BGP == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_VRRP == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_IGMP_SNOOPING == p_pkt_rx->rx_info.reason
#ifdef GREATBELT 
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP_V2== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP_V2== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ICMPV6 == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_LINK_ID_FAIL == p_pkt_rx->rx_info.reason
#elif DUET2
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ICMPV6 == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_LINK_ID_FAIL == p_pkt_rx->rx_info.reason
#elif TSINGMA
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP== p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ICMPV6 == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_LINK_ID_FAIL == p_pkt_rx->rx_info.reason
#else
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP == p_pkt_rx->rx_info.reason
#endif
        ||CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_PIM == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_PTP == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_COPY_CPU == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL == p_pkt_rx->rx_info.reason
        ||CTC_PKT_CPU_REASON_L3_MTU_FAIL == p_pkt_rx->rx_info.reason))
    {
        /* Fix by kcao for bug 47877 2018-07-06, LACP from member port should not get tid, for ctc_linkagg_get_1st_local_port will fail */
        port = p_pkt_rx->rx_info.src_port;
        if (CTC_IS_LINKAGG_PORT(port))
        {
            tid = CTC_GPORT_LINKAGG_ID(port);
            CTC_ERROR_RETURN(ctc_linkagg_get_1st_local_port(0, tid, &port, &cnt));
            CTC_ERROR_RETURN(ctc_port_get_default_vlan(port, &vid));
        }
        else
        {
            CTC_ERROR_RETURN(ctc_port_get_default_vlan(p_pkt_rx->rx_info.src_port, &vid));
        }
    }
    else 
    {
        vid = (0 == p_pkt_rx->rx_info.fid) ? p_pkt_rx->rx_info.src_svid : p_pkt_rx->rx_info.fid;
    }

    return vid;
}

int32 g_crc_length = 4;

int32
ctc_packet_receive_from_sdk_process_dhcp(ctc_pkt_rx_t* p_pkt_rx)
{
    sai_attribute_t attr_list[4];
    sai_object_id_t portid = 0;
    uint32 tid             = 0;
    uint32 count           = 0;

    sal_memset(attr_list, 0, sizeof(attr_list));    
    attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
    if ((p_pkt_rx->rx_info.reason > CTC_PKT_CPU_REASON_CUSTOM_BASE*2
        && p_pkt_rx->rx_info.reason <= CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2)
        || p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE+1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2)
    {
        p_pkt_rx->rx_info.reason -= CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    }
    attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

    attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
    attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
    count = 2;

    attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
    attr_list[2].value.u16 = _ctc_get_copp_vid(p_pkt_rx);
    count = 3;

    tid = ctc_sai_port_get_lag_id(p_pkt_rx->rx_info.src_port);
    if(0 != tid)
    {
        attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
        portid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);            
        attr_list[3].value.oid = portid;
        count = 4;
    }

    /* remove vlan in the packet */
    ctc_packet_remove_vlan(p_pkt_rx);

/* modified by liwh for bug 39968, 2016-08-11 
   GB packet length include the inserted vlan tag */
#ifdef _GLB_UML_SYSTEM_
#ifdef GREATBELT
    p_pkt_rx->pkt_buf->len = p_pkt_rx->pkt_buf->len - 4;
#endif
#endif
/* liwh end */

#ifdef _GLB_UML_SYSTEM_
    ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
        p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
        p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
        count,
        attr_list);   
#else
    ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
        p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
        p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN - g_crc_length,
        count,
        attr_list); 
#endif

    return SAI_STATUS_SUCCESS;
}

int32
ctc_packet_receive_from_sdk_process_arp(ctc_pkt_rx_t* p_pkt_rx)
{
    sai_attribute_t attr_list[4];
    sai_object_id_t portid = 0;
    uint32 tid             = 0;
    uint32 count           = 0;

    sal_memset(attr_list, 0, sizeof(attr_list));    
    attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
    if ((p_pkt_rx->rx_info.reason > CTC_PKT_CPU_REASON_CUSTOM_BASE*2
        && p_pkt_rx->rx_info.reason <= CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2)
        || p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE+1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2)
    {
        p_pkt_rx->rx_info.reason -= CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    }
    attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

    attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
    attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
    count = 2;

    attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
    attr_list[2].value.u16 = _ctc_get_copp_vid(p_pkt_rx);
    count = 3;

    tid = ctc_sai_port_get_lag_id(p_pkt_rx->rx_info.src_port);
    if(0 != tid)
    {
        attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
        attr_list[3].value.oid = portid;
        count = 4;
    }

    /* remove vlan in the packet */
    ctc_packet_remove_vlan(p_pkt_rx);

    ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
        p_pkt_rx->pkt_buf->data + 40,
        p_pkt_rx->pkt_buf->len  - 40,
        count,
        attr_list);    

    return SAI_STATUS_SUCCESS;
}

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   find vmac according to dest ip address, then replace dest mac to if route mac */
void 
vrrp_replace_mac_da_for_icmp_request(uint8 *pkt, uint16 pkt_len)
{
    ctc_icmp_message_header_t    *p_icmp_header = NULL;  
    ctc_icmp_ip_header_t         *p_icmp_ip_header = NULL;
    stbl_vrrp_vip_t              *p_sdb_vip = NULL;
    sai_vrrp_vip_entry_t         vrrp_vip_entry;
    uint16                       eth_type = 0; 

    sal_memset(&vrrp_vip_entry, 0, sizeof(vrrp_vip_entry));

    p_icmp_header = (ctc_icmp_message_header_t*)pkt;

    eth_type = sal_ntohs(p_icmp_header->eth_type);

    if (GLB_DEFAULT_TPID == eth_type)
    {
        p_icmp_ip_header = (ctc_icmp_ip_header_t*)(pkt + ETH_HEADER_LEN + VLAN_TAG_LEN);
    }
    else if (ETH_P_IPV4 == eth_type)
    {
        p_icmp_ip_header = (ctc_icmp_ip_header_t*)(pkt + ETH_HEADER_LEN);
    }
    else
    {
        return;    
    }
    
    if (PROTO_ICMP != p_icmp_ip_header->protocol)
    {
        return;    
    }

    vrrp_vip_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    sal_memcpy(&vrrp_vip_entry.ip_address.addr, &p_icmp_ip_header->dest_ip_addr, sizeof(uint32));

    p_sdb_vip = ctc_vrrp_vip_get_by_key(&vrrp_vip_entry);
    if (!p_sdb_vip)
    {
        return;
    }

    if (0 == sal_memcmp(pkt, p_sdb_vip->vmac, MAC_ADDR_LEN))
    {
        sal_memcpy(pkt, p_sdb_vip->if_mac, MAC_ADDR_LEN);
    }
    
    return;
}
/* liwh end */

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   find vmac according to src ip address, then replace src mac by vmac */
void 
vrrp_replace_mac_da_for_icmp_reply(char *pkt, int pkt_len)
{
    ctc_icmp_message_header_t    *p_icmp_header = NULL;  
    ctc_icmp_ip_header_t         *p_icmp_ip_header = NULL;
    stbl_vrrp_vip_t              *p_sdb_vip = NULL;
    sai_vrrp_vip_entry_t         vrrp_vip_entry;
    uint16                       eth_type = 0; 

    sal_memset(&vrrp_vip_entry, 0, sizeof(vrrp_vip_entry));

    p_icmp_header = (ctc_icmp_message_header_t*)pkt;

    eth_type = sal_ntohs(p_icmp_header->eth_type);

    if (GLB_DEFAULT_TPID == eth_type)
    {
        p_icmp_ip_header = (ctc_icmp_ip_header_t*)(pkt + ETH_HEADER_LEN + VLAN_TAG_LEN);
     }
    else if (ETH_P_IPV4 == eth_type)
    {
        p_icmp_ip_header = (ctc_icmp_ip_header_t*)(pkt + ETH_HEADER_LEN);
    }
    else
    {
        return;
    }

    if (PROTO_ICMP != p_icmp_ip_header->protocol)
    {
        return;    
    }

    vrrp_vip_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    sal_memcpy(&vrrp_vip_entry.ip_address.addr, &p_icmp_ip_header->src_ip_addr, sizeof(uint32));

    p_sdb_vip = ctc_vrrp_vip_get_by_key(&vrrp_vip_entry);
    if (!p_sdb_vip)
    {
        return;
    }

    if (0 == sal_memcmp((pkt + MAC_ADDR_LEN), p_sdb_vip->if_mac, MAC_ADDR_LEN))
    {
        sal_memcpy((pkt + MAC_ADDR_LEN), p_sdb_vip->vmac, MAC_ADDR_LEN);
    }
    
    return;
}
/* liwh end */

/*icmpv6 143*/
/*3333 0000 0016 0051 5a4f 9600 86dd 6000
    0000 0060 0001 fe80 0000 0000 0000 0251
    5aff fe4f 9600 ff02 0000 0000 0000 0000
    0000 0000 0016 3a00 0502 0000 0100 8f00
    e1c9 0000 0004 0400 0000 ff02 0000 0000
    0000 0000 0001 ff00 0001 0400 0000 ff02*/
    
/*not send some icmpv6 packet for bug 48458*/
int32
check_icmpv6_pkt(char *pkt, int pkt_len)
{
    ctc_icmp_message_header_t    *p_icmp_header = NULL;  
    struct ipv6hdr         *p_icmp_ipv6_header = NULL;
    struct icmp6hdr             *p_icmp = NULL;
    uint16                       eth_type = 0; 
    uint8 next_head = 0;

    p_icmp_header = (ctc_icmp_message_header_t*)pkt;
    eth_type = sal_ntohs(p_icmp_header->eth_type);

    if (GLB_DEFAULT_TPID == eth_type)
    {
        p_icmp_ipv6_header = (struct ipv6hdr*)(pkt + ETH_HEADER_LEN + VLAN_TAG_LEN);
     }
    else if (ETH_P_IPV6 == eth_type)
    {
        p_icmp_ipv6_header = (struct ipv6hdr*)(pkt + ETH_HEADER_LEN);
    }
    else
    {
        return FALSE;
    }

    /*hop-by-hop option 0x00*/
    if (0 != p_icmp_ipv6_header->nexthdr)
    {
        return FALSE;
    }

    next_head = *(uint8 *)(pkt + IPV6_HDR_LEN + ETH_HDR_LEN);
    if (PROTO_ICMPV6 != next_head)
    {
        return FALSE;
    }

    p_icmp = (struct icmp6hdr*)(pkt + IPV6_HDR_LEN + ETH_HDR_LEN + 8);
    if (ICMPV6_MLRM_V2 == p_icmp->icmp6_type)
    {
        return TRUE;
    }
    
    return FALSE;
}

sai_int32_t ctc_sai_get_icmpv6_reason_trap(ctc_pkt_rx_t* p_pkt_rx)
{
    sai_int32_t reason_trap = 0;
    struct icmp6hdr *p_icmp = NULL;
    uint8 *pst_vlan = NULL;
    p_icmp = (struct icmp6hdr*)(p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN + IPV6_HDR_LEN + ETH_HDR_LEN);

    pst_vlan = p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN + 12;
    if (pst_vlan[0] == 0x81 && pst_vlan[1] == 0x00)
    {
        p_icmp = (struct icmp6hdr*)(p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN + IPV6_HDR_LEN + ETH_HDR_LEN + 4);
    }

    if ((SAL_ND_ROUTER_SOLICIT == p_icmp->icmp6_type) 
        || (SAL_ND_ROUTER_ADVERT == p_icmp->icmp6_type) 
        ||  (SAL_ND_NEIGHBOR_SOLICIT == p_icmp->icmp6_type) 
        ||  (SAL_ND_NEIGHBOR_ADVERT == p_icmp->icmp6_type)       
        ||  (SAL_ND_REDIRECT == p_icmp->icmp6_type))
    {
        reason_trap = ctc_sai_get_reason_trap((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6) );
    }
            
    return reason_trap;
}

extern int
ctc_sai_is_iloop_port(int port_id);

int32
ctc_packet_receive_from_sdk(ctc_pkt_rx_t* p_pkt_rx)
{
    stbl_hostif_t *p_sdb_hif = NULL;
    sai_attribute_t attr_list[6];
    sai_hostif_trap_channel_t channel;
    sai_object_id_t portid = 0;
    int32 tid              = 0;
    uint16 vid             = 0;
    uint32 count           = 0;
    uint8           vrrp_mac[6] = {0, 0, 0x5e, 0, 1, 0};
    uint8           mac_addr[6] = {0x01, 0x19, 0xA7, 0x00, 0x00, 0x01};
    /*added by shengx to cpu mirror two packet when config mirror cpu , bug49795, 2018-11-09*/
    if (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_MIRRORED_TO_CPU)
    {
        if (ctc_sai_is_iloop_port(p_pkt_rx->rx_info.src_port))
        {
            return SAI_STATUS_SUCCESS;
        }
    }

    if ((p_pkt_rx->rx_info.reason > CTC_PKT_CPU_REASON_CUSTOM_BASE*2
        && p_pkt_rx->rx_info.reason <= CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2) ||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_PKT_CPU_REASON_CUSTOM_BASE + 1)||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL)||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_PKT_CPU_REASON_L3_MTU_FAIL)||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC)||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC)||
        (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_CUSTOM_BASE*2 + CTC_SAI_COPP_EXT_CPU_MIRROR))
    {
        
        p_pkt_rx->rx_info.reason -= CTC_PKT_CPU_REASON_CUSTOM_BASE*2;

        /* copp all use logic src port with src port information */
#ifdef GREATBELT
        p_pkt_rx->rx_info.src_port = p_pkt_rx->rx_info.logic_src_port;
#else
        /* Added by kcao for bug 47646 2018-06-14, gport on slice1 is 0x80xx > 0x3FFF, cannot set into DsPhyPortExt.defaultLogicSrcPort */
        p_pkt_rx->rx_info.src_port = CTC_MAP_LPORT_TO_GPORT(0, p_pkt_rx->rx_info.logic_src_port);
#endif
    }

    if (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_L3_ICMP_REDIRECT && g_ip_redirects_global_en == 0)
    {
        return SAI_STATUS_SUCCESS;
    }

/*added by yejl for bug 51016, 2019-03-20*/
#ifdef OFPRODUCT
    if (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_EAPOL)
    {
        return SAI_STATUS_SUCCESS;
    }
#endif

    channel = ctc_sai_get_reason_channel(p_pkt_rx->rx_info.reason);
    sal_memset(attr_list, 0, sizeof(attr_list));

#if defined(OFDPAPRODUCT)
    /* Forward to OF-DPA */
extern int ctc_ofdpa_packet_receive_from_sdk(ctc_pkt_rx_t* p_pkt_rx);

    ctc_ofdpa_packet_receive_from_sdk(p_pkt_rx);
#endif
    
    sal_memset(attr_list, 0, sizeof attr_list);

    /* 1. packet debug */
    attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
    attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

    attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
    attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);

    attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;

    vid = _ctc_get_copp_vid(p_pkt_rx);
    attr_list[2].value.u16 = vid;
    count = 3;

    /* 2. get LAG tid */
    /*Added by yejl for bug 46384, 2018-02-07*/
    tid = ctc_sai_port_get_lag_id(p_pkt_rx->rx_info.src_port);
    if (tid < 0)
    {
        if (CTC_PKT_CPU_REASON_MIRRORED_TO_CPU == p_pkt_rx->rx_info.reason)
        {
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP, 0);
        }
    }
    else if (0 != tid)
    {
        attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
        attr_list[3].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
        count = 4;
    }
    /*End by yejl*/

    /* 3. OAM process */
    if (CTC_PKT_CPU_REASON_OAM <= p_pkt_rx->rx_info.reason && 
        CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE >= p_pkt_rx->rx_info.reason)
    {
        /* system modified by liwh for bug 53010, 2019-08-05 */
        if (CTC_OAM_TYPE_ETH == p_pkt_rx->rx_info.oam.type)
        {
            return SAI_STATUS_SUCCESS;
        }
        /* liwh end */
        
        attr_list[0].value.s32 = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_TPOAM;
    }

    if (((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6) == p_pkt_rx->rx_info.reason)
        || (CTC_PKT_CPU_REASON_LINK_ID_FAIL == p_pkt_rx->rx_info.reason))
    {
        attr_list[0].value.s32 =  ctc_sai_get_icmpv6_reason_trap(p_pkt_rx);
    }
    
    /* 4. Call Debug function */
    ctc_sai_get_sys_info()->sai_switch_notification_table.on_cpu_packet_debug(
        p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
        p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
        count,
        attr_list);

    /* 5. Channel process */
    if (SAI_HOSTIF_TRAP_CHANNEL_CB == channel)
    {
        attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
        attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);
    
        attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
        attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);

        attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
        attr_list[2].value.u16 = vid;
        count = 3;

        if (tid < 0)
        {
            if (CTC_PKT_CPU_REASON_MIRRORED_TO_CPU == p_pkt_rx->rx_info.reason)
            {
                attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP, 0);
            }
        }
        else if (0 != tid)
        {
            attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
            attr_list[3].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
            count = 4;
        }
        
        if(attr_list[0].value.s32 == SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER)
        {
            attr_list[4].id = SAI_HOSTIF_PACKET_TUNNEL_PAYLOAD_OFFSET;
            attr_list[4].value.u8 = p_pkt_rx->rx_info.payload_offset;
            count ++;

            attr_list[5].id = SAI_HOSTIF_PACKET_METADATA;
            attr_list[5].value.u16 = p_pkt_rx->rx_info.meta_data;
        }
        if (p_pkt_rx->rx_info.reason == CTC_PKT_CPU_REASON_PTP)
        {
            /*for ptp*/
            if(attr_list[0].value.s32 == SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP)
            {
                attr_list[4].id = SAI_HOSTIF_PACKET_PTP_PACKET_RCV;
                sal_memcpy(&(attr_list[4].value.ptp), &p_pkt_rx->rx_info.ptp, sizeof(ctc_pkt_ptp_info_t));
                count ++;
            }
        }
        
        /* support tpoam modified by chenc , 
            copy tpoam packet to CPU */
        if (CTC_PKT_CPU_REASON_OAM <= p_pkt_rx->rx_info.reason && 
            CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE >= p_pkt_rx->rx_info.reason)
        {
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            //attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);
            attr_list[0].value.s32 = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_TPOAM;
    
            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;
            
            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            attr_list[count].value.u16 = (p_pkt_rx->rx_info.src_cos << 13) | attr_list[2].value.u16;
            count = 3;

            if (tid < 0)
            {
                if (CTC_PKT_CPU_REASON_MIRRORED_TO_CPU == p_pkt_rx->rx_info.reason)
                {
                    attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP, 0);
                }
            }
            else if (0 != tid)
            {
                portid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
            }
            /* get phy port id */
            else
            {
                portid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            }
            
            if(0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                //attr_list[3].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
                count++;
            }

            if(attr_list[0].value.s32 == SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_TPOAM)
            {
                #ifdef _GLB_UML_SYSTEM_
                ctc_pkt_oam_info_t tpoam_info;
                sal_memset(&tpoam_info, 0x00, sizeof(tpoam_info));
                
                attr_list[count].id = SAI_HOSTIF_PACKET_TPOAM_PACKET_RCV;
                sal_memset(&(attr_list[count].value.tpoam), 0x00, sizeof (sai_pkt_oam_info_t));
                attr_list[count].value.tpoam.dm_ts.seconds = tpoam_info.dm_ts.seconds;
                attr_list[count].value.tpoam.dm_ts.nanoseconds = tpoam_info.dm_ts.nanoseconds;
                attr_list[count].value.tpoam.dm_ts_offset = tpoam_info.dm_ts_offset;
                attr_list[count].value.tpoam.flags = tpoam_info.flags;
                attr_list[count].value.tpoam.lm_fcl = tpoam_info.lm_fcl;
                attr_list[count].value.tpoam.mep_index = tpoam_info.mep_index;
                attr_list[count].value.tpoam.type = tpoam_info.type;
                attr_list[count].value.tpoam.vid = tpoam_info.vid;
                #else
                attr_list[count].id = SAI_HOSTIF_PACKET_TPOAM_PACKET_RCV;
                sal_memset(&(attr_list[count].value.tpoam), 0x00, sizeof (sai_pkt_oam_info_t));
                attr_list[count].value.tpoam.dm_ts.seconds = p_pkt_rx->rx_info.oam.dm_ts.seconds;
                attr_list[count].value.tpoam.dm_ts.nanoseconds = p_pkt_rx->rx_info.oam.dm_ts.nanoseconds;
                attr_list[count].value.tpoam.dm_ts_offset = p_pkt_rx->rx_info.oam.dm_ts_offset;
                attr_list[count].value.tpoam.flags = p_pkt_rx->rx_info.oam.flags;
                attr_list[count].value.tpoam.lm_fcl = p_pkt_rx->rx_info.oam.lm_fcl;
                attr_list[count].value.tpoam.mep_index = p_pkt_rx->rx_info.oam.mep_index;
                attr_list[count].value.tpoam.type = p_pkt_rx->rx_info.oam.type;
                attr_list[count].value.tpoam.vid = p_pkt_rx->rx_info.oam.vid;//vid is 0
                #endif
                
                if (CTC_FLAG_ISSET(p_pkt_rx->rx_info.oam.flags, CTC_PKT_OAM_FLAG_IS_LM))
                {
                    
                }
                attr_list[count].value.tpoam.priority = p_pkt_rx->rx_info.src_cos;
                count++;
            }
        }

        ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
            p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
            p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
            count,
            attr_list);
    }
    else if (SAI_HOSTIF_TRAP_CHANNEL_NETDEV == channel)
    {
#ifdef GREATBELT
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP_V2) == p_pkt_rx->rx_info.reason)   
#else
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP) == p_pkt_rx->rx_info.reason)   
#endif
        {
            ctc_packet_receive_from_sdk_process_dhcp(p_pkt_rx);

#ifndef _GLB_UML_SYSTEM_
            return SAI_STATUS_SUCCESS;
#endif
        }
        
        /* get link agg id */
        if (0 != tid)
        {
            portid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
        }
        /* get phy port id */
        else
        {
            portid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
        }

        /* get vlan interface */
        p_sdb_hif = ctc_sai_hostif_get_by_vlan(vid);
        if (NULL == p_sdb_hif)
        {
            p_sdb_hif = ctc_sai_hostif_get_by_port(portid);
        }

        if (NULL == p_sdb_hif)
        {
            if (CTC_PKT_CPU_REASON_L3_MTU_FAIL == p_pkt_rx->rx_info.reason)
            {
                p_sdb_hif = ctc_sai_hostif_get_vlanif_for_mtu_excep(p_pkt_rx);
            }
        }

        if ((CTC_PKT_CPU_REASON_SFLOW_SOURCE == p_pkt_rx->rx_info.reason)
            || (CTC_PKT_CPU_REASON_SFLOW_DEST == p_pkt_rx->rx_info.reason))
        {
            sal_memset(attr_list, 0, sizeof(attr_list));
            count = 0;

            attr_list[count].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[count].value.s32 = SAI_HOSTIF_TRAP_ID_SAMPLEPACKET;
            count++;

            
            attr_list[count].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[count].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count++;

            attr_list[count].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[count].value.u16 = (p_pkt_rx->rx_info.src_cos << 13) | vid;
            count++;
            
            if (0 != tid)
            {
                attr_list[count].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[count].value.oid = portid;
                count++;
            }
            
            attr_list[count].id = SAI_HOSTIF_PACKET_SUB_TYPE;

            if (CTC_PKT_CPU_REASON_SFLOW_SOURCE == p_pkt_rx->rx_info.reason)
            {
                attr_list[count].value.u16 = GLB_SFLOW_INGRESS_PACKET;
            }
            else
            {
                attr_list[count].value.u16 = GLB_SFLOW_EGRESS_PACKET;    
            }
            count++;

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);
            return SAI_STATUS_SUCCESS;    
        }

        if (NULL == p_sdb_hif)
        {
            return SAI_STATUS_FAILURE;
        }

#ifdef _GLB_UML_SYSTEM_
#ifdef GREATBELT
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP_V2) == p_pkt_rx->rx_info.reason)   
#else
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_DHCP) == p_pkt_rx->rx_info.reason)   
#endif
        {
             if (p_sdb_hif)
             {
                 /* send the packet to linux kernel */
                 ctc_packet_send_to_kernel(p_sdb_hif->fd, p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);
             }
             
             return SAI_STATUS_SUCCESS;    
         }
#endif

#ifdef GREATBELT
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP_V2) == p_pkt_rx->rx_info.reason)
#else
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP) == p_pkt_rx->rx_info.reason)
#endif
        {
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;

            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            count = 3;

            if(0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                count = 4;
            }

            /* remove vlan in the packet */
            ctc_packet_remove_vlan(p_pkt_rx);

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);    
            return SAI_STATUS_SUCCESS;
            //return ctc_packet_receive_from_sdk_process_arp(p_pkt_rx);
        }

        /* support nd modified by liwh for bug 47547, 2018-06-28 */
        if (((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6) == p_pkt_rx->rx_info.reason)
            || (CTC_PKT_CPU_REASON_LINK_ID_FAIL == p_pkt_rx->rx_info.reason))
        {
            if (SAI_HOSTIF_TRAP_ID_IPV6_NEIGHBOR_DISCOVERY == ctc_sai_get_icmpv6_reason_trap(p_pkt_rx))
            {
                sal_memset(attr_list, 0, sizeof(attr_list));    
                attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
                //attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);
                attr_list[0].value.s32 = ctc_sai_get_reason_trap((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6) );
    
                attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
                attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
                count = 2;
    
                attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
                attr_list[2].value.u16 = vid;
                count = 3;
    
                if(0 != tid)
                {
                    attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                    attr_list[3].value.oid = portid;
                    count = 4;
                }
    
                /* remove vlan in the packet */
                ctc_packet_remove_vlan_for_nd(p_pkt_rx);
    
                ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                    p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                    p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                    count,
                    attr_list);    
                return SAI_STATUS_SUCCESS;
            }
        }
        /* liwh end */

        if (CTC_PKT_CPU_REASON_FWD_CPU == p_pkt_rx->rx_info.reason)
        {
            if (0 == sal_memcmp((p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN), mac_addr, 6))
            {
                sal_memset(attr_list, 0, sizeof(attr_list));    
                attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
                attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

                attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
                attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
                count = 2;

                attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
                attr_list[2].value.u16 = vid;
                count = 3;

                if(0 != tid)
                {
                    attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                    attr_list[3].value.oid = portid;
                    count = 4;
                }

                ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                    p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                    p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                    count,
                    attr_list);

             }
        }

        if (CTC_PKT_CPU_REASON_L2_COPY_CPU == p_pkt_rx->rx_info.reason)
        {
            if (0 == sal_memcmp((p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN), mac_addr, 6))
            {
                sal_memset(attr_list, 0, sizeof(attr_list));    
                attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
                attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

                attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
                attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
                count = 2;

                attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
                attr_list[2].value.u16 = vid;
                count = 3;

                if(0 != tid)
                {
                    attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                    attr_list[3].value.oid = portid;
                    count = 4;
                }

               
                ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                    p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                    p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                    count,
                    attr_list);
             }
        }
        
        if (CTC_PKT_CPU_REASON_IGMP_SNOOPING == p_pkt_rx->rx_info.reason)
        {
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);

            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;

            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            count = 3;

            if(0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                count = 4;
            }

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);
        }
        /*modified by yangl for bug 47256,2018-11-30*/
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_PIM) == p_pkt_rx->rx_info.reason
#ifdef USW
        /* BUG53979 pim packet using PIM_SNOOPING for USW system */
          || CTC_PKT_CPU_REASON_PIM_SNOOPING == p_pkt_rx->rx_info.reason
#endif
        )
        {                                
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[0].value.s32 = SAI_HOSTIF_TRAP_ID_PIM;
            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;

            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            count = 3;

            if(0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                count = 4;
            }

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);
        }

        /*yangl end*/
        /* support vrrp modified by liwh for bug 45215, 2017-09-17 
           copy vrrp packet to CPU */
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_VRRP) == p_pkt_rx->rx_info.reason)
        {
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);
    
            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;
    
            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            count = 3;
    
            if (0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                count = 4;
            }

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);    
            return SAI_STATUS_SUCCESS;
        }
        /* liwh end */

        /* support ptp modified by chenc , 
            copy ptp packet to CPU */
        if (CTC_PKT_CPU_REASON_PTP == p_pkt_rx->rx_info.reason)
        {
            sal_memset(attr_list, 0, sizeof(attr_list));    
            attr_list[0].id = SAI_HOSTIF_PACKET_TRAP_ID;
            attr_list[0].value.s32 = ctc_sai_get_reason_trap(p_pkt_rx->rx_info.reason);
    
            attr_list[1].id = SAI_HOSTIF_PACKET_INGRESS_PORT;
            attr_list[1].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_pkt_rx->rx_info.src_port);
            count = 2;

            
            attr_list[2].id = SAI_HOSTIF_PACKET_VLAN_ID;
            attr_list[2].value.u16 = vid;
            attr_list[count].value.u16 = (p_pkt_rx->rx_info.src_cos << 13) | attr_list[2].value.u16;
            count = 3;
    
            if(0 != tid)
            {
                attr_list[3].id = SAI_HOSTIF_PACKET_INGRESS_LAG;
                attr_list[3].value.oid = portid;
                count++;
            }

            if(attr_list[0].value.s32 == SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP)
            {
                #ifdef _GLB_UML_SYSTEM_
                ctc_pkt_ptp_info_t ptp;
                sal_memset(&ptp, 0x00, sizeof(ptp));
                
                attr_list[count].id = SAI_HOSTIF_PACKET_PTP_PACKET_RCV;
                sal_memset(&(attr_list[count].value.ptp), 0x00, sizeof (sai_pkt_ptp_info_t));
                attr_list[count].value.ptp.ts.seconds = ptp.ts.seconds;
                attr_list[count].value.ptp.ts.nanoseconds = ptp.ts.nanoseconds;
                attr_list[count].value.ptp.oper = ptp.oper;
                attr_list[count].value.ptp.seq_id = ptp.seq_id;
                attr_list[count].value.ptp.ts_offset= ptp.ts_offset;
                #else
                attr_list[count].id = SAI_HOSTIF_PACKET_PTP_PACKET_RCV;
                sal_memset(&(attr_list[count].value.ptp), 0x00, sizeof (sai_pkt_ptp_info_t));
                attr_list[count].value.ptp.ts.seconds = p_pkt_rx->rx_info.ptp.ts.seconds;
                attr_list[count].value.ptp.ts.nanoseconds = p_pkt_rx->rx_info.ptp.ts.nanoseconds;
                attr_list[count].value.ptp.oper = p_pkt_rx->rx_info.ptp.oper;
                attr_list[count].value.ptp.seq_id = p_pkt_rx->rx_info.ptp.seq_id;
                attr_list[count].value.ptp.ts_offset = p_pkt_rx->rx_info.ptp.ts_offset;
                #endif

                //FILE * fp_console1 = NULL;
                //fp_console1 = fopen("/mnt/flash/ctc_sai_hostif_rx_info_ptp.log", "a+"); 
                //fprintf(fp_console1, "2 ptp.ts.seconds %u %u seq_id %hhu ts_offset %hhu oper %hhu, count %d\n", 
                //    attr_list[count].value.ptp.ts.seconds, attr_list[count].value.ptp.ts.nanoseconds, attr_list[count].value.ptp.seq_id, attr_list[count].value.ptp.ts_offset, attr_list[count].value.ptp.oper, count); 
                //fclose(fp_console1); 
                count++;
            }

            ctc_sai_get_sys_info()->sai_switch_notification_table.on_packet_event(
                p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN,
                p_pkt_rx->pkt_buf->len  - CTC_PKT_HEADER_LEN,
                count,
                attr_list);    
            return SAI_STATUS_SUCCESS;
        }

#ifndef USW/*fix ipuc_func_129-virtual_interface-mtu.tcl failed for not send icmp need to frag by chenc */
        if (CTC_PKT_CPU_REASON_L3_MTU_FAIL == p_pkt_rx->rx_info.reason)
        {
            /* add the l2 header excluding the vlan tag */
            //ctc_packet_add_l2_header(p_pkt_rx);
            uint8       pkt_buffer[MAX_PKT_BUFFER];

            sal_memset(pkt_buffer, 0, sizeof(pkt_buffer));
            ctc_packet_update_l2_header(p_pkt_rx, pkt_buffer);

#ifdef _GLB_UML_SYSTEM_
#ifdef GREATBELT
            p_pkt_rx->pkt_buf->len = p_pkt_rx->pkt_buf->len - 4;
#endif
#endif
            /* send the packet to linux kernel */
            //ctc_packet_send_to_kernel(pst_hostif->fd, p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);
            ctc_packet_send_to_kernel(p_sdb_hif->fd, pkt_buffer + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);

            return SAI_STATUS_SUCCESS;
        }
#endif
        /*added by yejl to discard the ospf packet when system memory is not enough, bug43529, 2017-03-31*/
        if ((CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_OSPF) == p_pkt_rx->rx_info.reason)
        {
            if (memory_check_pktdiscard)
            {
                return SAI_STATUS_SUCCESS;
            }
        }
        /* remove vlan in the packet */
        ctc_packet_remove_vlan(p_pkt_rx);

        /* support vrrp modified by liwh for bug 45215, 2017-09-17 
           check whether replace vmac by route mac */
        if ((CTC_PKT_CPU_REASON_L3_COPY_CPU == p_pkt_rx->rx_info.reason)
            && (0 == sal_memcmp((p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN), vrrp_mac, 5)))
        {
            vrrp_replace_mac_da_for_icmp_request(p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);
        }
        /* liwh end */

        ctc_sai_hostif_rx_ip_icmp_debug(p_sdb_hif, p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);

        /* send the packet to linux kernel */
        ctc_packet_send_to_kernel(p_sdb_hif->fd, p_pkt_rx->pkt_buf->data + CTC_PKT_HEADER_LEN, p_pkt_rx->pkt_buf->len - CTC_PKT_HEADER_LEN);
    }

    return SAI_STATUS_SUCCESS;
}

#define HOSTIF_SEND_TO_KERNEL_MAX_TIMES     1000
uint32 g_hostif_send_to_kernel_error_count = 0;

int32
ctc_packet_send_to_kernel(int fd, uint8 *buf, uint32 length)
{
    uint8 *head = buf;
    int writenLen = 0;
    int count = 0;
    int resend = 0;

    while (resend <= HOSTIF_SEND_TO_KERNEL_MAX_TIMES)
    {
        writenLen = write(fd, head + count, length);
        resend++;
        if (writenLen == -1)
        {
            if (errno == EAGAIN)
            {
                break;
            }
            else if(errno == ECONNRESET)
            {
                break;
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            {

            }
        }

        if (writenLen == 0)
        {
            break;
        }

        count += writenLen;
        if (count == length)
        {
            break;
        }
    }

    if (resend >= HOSTIF_SEND_TO_KERNEL_MAX_TIMES)
    {
        g_hostif_send_to_kernel_error_count++;
    }
    
    return SAI_STATUS_SUCCESS;
}

/* support vrrp modified by liwh for bug 45215, 2017-09-17 
   update stale nd fib when send icmpv6 echo request */
void 
nd_update_nd_fib_for_icmpv6_echo(char *pkt, int pkt_len, char *ifname)
{
    ctc_icmp_message_header_t    *p_icmp_header = NULL;  
    uint16                                    eth_type = 0;
    struct ipv6hdr                       *p_ipv6_hdr = NULL;
    struct icmp6hdr                     *p_icmp = NULL;

    p_icmp_header = (ctc_icmp_message_header_t*)pkt;

    eth_type = sal_ntohs(p_icmp_header->eth_type);
    if (ETH_P_IPV6 != eth_type)
    {
        return;
    }

    p_ipv6_hdr = (struct ipv6hdr*)(pkt + ETH_HDR_LEN);
    if (IPPROTO_ICMPV6 != p_ipv6_hdr->nexthdr)
    {
        return;    
    }

    p_icmp = (struct icmp6hdr*)(pkt + IPV6_HDR_LEN + ETH_HDR_LEN);
    if ((ICMPV6_ECHO_REQUEST != p_icmp->icmp6_type) 
        && (ICMPV6_ECHO_REPLY != p_icmp->icmp6_type))
    {
        return;
    }

    ctc_nd_update_neighbor(p_ipv6_hdr->daddr, ifname);

    return;
}
/* liwh end */
void
ctc_packet_receive_from_kernel(void *data)
{
    stbl_hostif_t *p_sdb_hif = NULL;
    struct epoll_event events[2048];
    int sockfd = 0;
    int nfds   = 0;
    int idx    = 0;

    while (TRUE)
    {
        nfds = epoll_wait(g_hostif_info.epoll_sock, events, 2048, 1);
        for (idx = 0; idx < nfds; ++idx)
        {
            if (events[idx].events & EPOLLIN)
            {
                if ( (sockfd = events[idx].data.fd) < 0)
                {
                    continue;
                }

                char *head = g_hostif_info.pkt_buff;
                bool bReadOk = FALSE;
                int recvNum = 0;
                int count = 0;

                while (TRUE)
                {
                    recvNum = read(sockfd, head + count, 9600);
                    if(recvNum < 0)
                    {
                        if (errno == EAGAIN)
                        {
                            bReadOk = true;
                            break;
                        }
                        else if (errno == ECONNRESET)
                        {
                            break;
                         }
                        else if (errno == EINTR)
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                   }
                   else if (recvNum == 0)
                   {
                        break;
                   }

                   count += recvNum;
                   if (recvNum >= 9600)
                   {
                       break;
                   }
                   else
                   {
                       bReadOk = true;
                       break;
                   }
                }

                if (bReadOk == true)
                {
                    g_hostif_info.pkt_buff[count] = '\0';
                    p_sdb_hif = ctc_sai_hostif_get_by_fd(sockfd);
                    if (NULL == p_sdb_hif)
                    {
                        continue;
                    }
                    else if (0 == sal_strncmp(p_sdb_hif->ifname, "loopback", 8))
                    {
                        continue;
                    }

                    /*for icmpv6 143 not send out*/
                    if (check_icmpv6_pkt(g_hostif_info.pkt_buff, count))
                    {
                        continue;
                    }

                    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
                       check whether replace src mac by vmac for icmp reply */
                    vrrp_replace_mac_da_for_icmp_reply(g_hostif_info.pkt_buff, count);
                    /* liwh end */

                    /* modified by liwh for bug 48486, 2018-08-14 
                         update stale nd fib when send icmpv6 echo request */
                    nd_update_nd_fib_for_icmpv6_echo(g_hostif_info.pkt_buff, count, p_sdb_hif->ifname);
                    /* liwh end */

                    /* modified by liwh for bug 51981, 2019-06-26 */
                    if (count < 60)
                    {
                        count = 60;    
                    }
                    /* liwh end */
                    
                    ctc_packet_send_to_sdk(p_sdb_hif, g_hostif_info.pkt_buff, count);
                }
            }
        }
    }
}

/*these 2 trap are inited by SDK to drop */
/*this function reset these 2 trap to up to cpu*/
int32
ctc_sai_init_other_cpu_traffic(void)
{
    int32 ret = 0;
    ctc_qos_queue_cfg_t    qos_queue_cfg;

    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    qos_queue_cfg.value.reason_map.cpu_reason = CTC_PKT_CPU_REASON_L3_IP_OPTION;
    qos_queue_cfg.value.reason_map.queue_id = 0;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    qos_queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    qos_queue_cfg.value.reason_dest.dest_port = CTC_LPORT_CPU;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    qos_queue_cfg.value.reason_map.cpu_reason = CTC_PKT_CPU_REASON_L3_MTU_FAIL;
    qos_queue_cfg.value.reason_map.queue_id = 0;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    qos_queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    qos_queue_cfg.value.reason_dest.dest_port = CTC_LPORT_CPU;
    ret += ctc_qos_set_queue(&qos_queue_cfg);

    /* support nd modified by liwh for bug 47547, 2018-08-03 */
    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    qos_queue_cfg.value.reason_map.cpu_reason = CTC_PKT_CPU_REASON_LINK_ID_FAIL;
    qos_queue_cfg.value.reason_map.queue_id = 0;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    qos_queue_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    qos_queue_cfg.value.reason_dest.dest_port = CTC_LPORT_CPU;
    ret += ctc_qos_set_queue(&qos_queue_cfg);
    /* liwh end */
        
    return ret;
}


#define ________SAI_SAI_DEBUG_FUNC
sai_status_t sai_create_hostif_debug_param(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_HOSTIF_ATTR_TYPE:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_ATTR_TYPE");
            break;
        case SAI_HOSTIF_ATTR_RIF_OR_PORT_ID:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_ATTR_RIF_OR_PORT_ID 0x%llx", attr->value.oid);
            break;
        case SAI_HOSTIF_ATTR_NAME:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE %s", attr->value.chardata);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t sai_create_hostif_trap_group_debug_param(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    const sai_attribute_t *attr = NULL;
    uint32_t          attr_idx = 0; 

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE %u", attr->value.u32);
            break;
        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            CTC_SAI_DEBUG("out:SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER 0x%llx", attr->value.oid);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_set_trap_group_queue(sai_object_id_t trap_group_id, int32 queueid)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    ctc_qos_queue_cfg_t        qos_queue_cfg;
    CTC_SAI_DEBUG_FUNC();

    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    p_sdb_group->queue_id = queueid;

    sal_memset(&qos_queue_cfg, 0, sizeof(ctc_qos_queue_cfg_t));
    qos_queue_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP;
    qos_queue_cfg.value.reason_map.cpu_reason = p_sdb_group->ctc_reason_id + CTC_PKT_CPU_REASON_CUSTOM_BASE*2;
    qos_queue_cfg.value.reason_map.queue_id = queueid;
    qos_queue_cfg.value.reason_map.reason_group = 0;
    ctc_qos_set_queue(&qos_queue_cfg);

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_set_trap_group_policer(sai_object_id_t trap_group_id, sai_object_id_t policer_oid)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    sai_object_id_t acl_oid[8];
    uint32 i;

    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(acl_oid, 0, sizeof(acl_oid));
    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    
    CTC_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    CTC_ERROR_RETURN(ctc_sai_copp_get_acl_oid_from_reason(p_sdb_group->ctc_reason_id, acl_oid));

    /*1, apply all acl entry policer for this trap group*/
    for (i = 0; i < 8; i++)
    {
        if(acl_oid[i] != 0)
        {
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.oid = policer_oid;
            CTC_ERROR_RETURN(acl_api->set_acl_entry_attribute(acl_oid[i], &attr));
        }
            
    }

    /*2, save acl policeroid*/
    p_sdb_group->police_oid = policer_oid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_set_trap_group_ssh_policer(sai_object_id_t trap_group_id, sai_object_id_t policer_oid)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    sai_object_id_t acl_oid[8];
    uint32 i;

    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(acl_oid, 0, sizeof(acl_oid));
    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    
    CTC_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    CTC_ERROR_RETURN(ctc_sai_copp_get_ssh_acl_oid_from_reason(p_sdb_group->ctc_reason_id, acl_oid));

    /*1, apply all acl entry policer for this trap group*/
    for (i = 0; i < 8; i++)
    {
        if(acl_oid[i] != 0)
        {
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.oid = policer_oid;
            CTC_ERROR_RETURN(acl_api->set_acl_entry_attribute(acl_oid[i], &attr));
        }
            
    }

    /*2, save acl policeroid*/
    p_sdb_group->police_ssh_oid = policer_oid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_set_trap_group_telnet_policer(sai_object_id_t trap_group_id, sai_object_id_t policer_oid)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    sai_object_id_t acl_oid[8];
    uint32 i;

    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(acl_oid, 0, sizeof(acl_oid));
    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    
    CTC_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    CTC_ERROR_RETURN(ctc_sai_copp_get_telnet_acl_oid_from_reason(p_sdb_group->ctc_reason_id, acl_oid));

    /*1, apply all acl entry policer for this trap group*/
    for (i = 0; i < 8; i++)
    {
        if(acl_oid[i] != 0)
        {
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.oid = policer_oid;
            CTC_ERROR_RETURN(acl_api->set_acl_entry_attribute(acl_oid[i], &attr));
        }
            
    }

    /*2, save acl policeroid*/
    p_sdb_group->police_telnet_oid = policer_oid;

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_set_trap_group_mlag_policer(sai_object_id_t trap_group_id, sai_object_id_t policer_oid)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    sai_object_id_t acl_oid[8];
    uint32 i;

    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr;

    CTC_SAI_DEBUG_FUNC();

    sal_memset(acl_oid, 0, sizeof(acl_oid));
    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    
    CTC_ERROR_RETURN(sai_api_query(SAI_API_ACL,(void**)&acl_api));

    CTC_ERROR_RETURN(ctc_sai_copp_get_mlag_acl_oid_from_reason(p_sdb_group->ctc_reason_id, acl_oid));

    /*1, apply all acl entry policer for this trap group*/
    for (i = 0; i < 8; i++)
    {
        if(acl_oid[i] != 0)
        {
            sal_memset(&attr, 0, sizeof(sai_attribute_t));
            attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER;
            attr.value.aclaction.enable = TRUE;
            attr.value.aclaction.parameter.oid = policer_oid;
            CTC_ERROR_RETURN(acl_api->set_acl_entry_attribute(acl_oid[i], &attr));
        }
            
    }

    /*2, save acl policeroid*/
    p_sdb_group->police_mlag_oid = policer_oid;

    return SAI_STATUS_SUCCESS;
}

    
sai_status_t ctc_sai_get_trap_group_queue(sai_object_id_t trap_group_id, sai_attribute_t* attr)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    CTC_SAI_DEBUG_FUNC();

    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    attr->value.u32 = p_sdb_group->queue_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_get_trap_group_policer(sai_object_id_t trap_group_id, sai_attribute_t* attr)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    CTC_SAI_DEBUG_FUNC();

    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);
    attr->value.oid= p_sdb_group->police_oid;

    return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t ctc_sai_get_trap_group_counter(sai_object_id_t trap_group_id, sai_attribute_t* attr)
{
    stbl_copp_group_t* p_sdb_group = NULL;
    sai_object_id_t counter_oid;
    CTC_SAI_DEBUG_FUNC();

    p_sdb_group = ctc_sai_copp_get_group(trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);

    CTC_ERROR_RETURN(ctc_sai_copp_get_counter_oid_from_reason(p_sdb_group->ctc_reason_id, &counter_oid));
    attr->value.oid = counter_oid;

    return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t ctc_sai_create_hostif_trap(
    _In_ sai_hostif_trap_id_t hostif_trapid,
    _In_ sai_hostif_trap_channel_t trap_channel)
{
    stbl_copp_trap_t sdb_trap;
    stbl_copp_trap_t *p_sdb_trap = NULL;
    int32 ret = 0;

    sal_memset(&sdb_trap, 0, sizeof(sdb_trap));
    sdb_trap.key.trap_id = hostif_trapid;
    
    stbl_copp_trap_add_copp_trap(&sdb_trap);
    p_sdb_trap = stbl_copp_trap_get_copp_trap(&sdb_trap.key);
    if (NULL == p_sdb_trap)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /* will overwrite SAI_HOSTIF_TRAP_ID_OSPF etc. , bypass those trapid */
    if ((SAI_HOSTIF_TRAP_ID_OSPFV6 != hostif_trapid)
     && (SAI_HOSTIF_TRAP_ID_VRRPV6 != hostif_trapid)
     && (SAI_HOSTIF_TRAP_ID_BGPV6 != hostif_trapid))
    {
        p_sdb_trap->ctc_reason_id = sai_trap_id_to_ctc_reason_id(hostif_trapid);
        ctc_sai_set_reason_trap(p_sdb_trap->ctc_reason_id, hostif_trapid);
    }
    p_sdb_trap->channel = trap_channel;
    ctc_sai_set_reason_channel(p_sdb_trap->ctc_reason_id, p_sdb_trap->channel);
    
    return ret;
}


sai_status_t ctc_sai_set_trap_attr_group(sai_object_id_t trap_id, sai_object_id_t trap_group_id)
{
    stbl_copp_trap_t *p_sdb_trap = NULL;
    stbl_copp_group_t *p_sdb_group = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:hostif_trapid %u", trap_id);

    /*1, check trap and trap group exist*/

    p_sdb_trap = ctc_sai_copp_get_trap(trap_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_trap);
    p_sdb_group = ctc_sai_copp_get_group(trap_group_id); 
    CTC_SAI_PTR_VALID_CHECK(p_sdb_group);

    /*2, set trap_group and set ctc_reason_id to trap_group*/
    /*Note, currently CTC API must need ctc_reason_id to set trap_group queue,
    so, we require user to bind trap to one trap_group first, and then set 
    trap_group attr for queueid*/
    p_sdb_trap->group_oid = trap_group_id;
    p_sdb_group->ctc_reason_id = p_sdb_trap->ctc_reason_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_get_trap_attr_group(sai_object_id_t trap_id, sai_attribute_t* attr)
{
    stbl_copp_trap_t *p_sdb_trap = NULL;
    CTC_SAI_DEBUG_FUNC();

    p_sdb_trap = ctc_sai_copp_get_trap(trap_id);
    CTC_SAI_PTR_VALID_CHECK(p_sdb_trap);
    attr->value.oid = p_sdb_trap->group_oid;

    return SAI_STATUS_SUCCESS;
}



#define ________SAI_SAI_API_FUNC

/*
* Routine Description:
*    Create host interface trap group
*
* Arguments:
*  [out] hostif_trap_group_id  - host interface trap group id
*  [in] attr_count - number of attributes
*  [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_hostif_trap_group(
    _Out_ sai_object_id_t *hostif_trap_group_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_port_glb_t      *p_port_glb = stbl_port_glb_get_port_glb();
    stbl_copp_group_t     sdb_group;
    uint32_t              index       = 0;
    uint32_t              attr_idx    = 0;
    sai_attribute_t      *attr        = NULL;
    stbl_copp_group_t    *p_sdb_group = NULL;
    sai_status_t          ret         = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    sai_create_hostif_trap_group_debug_param(attr_count, attr_list);
    CTC_SAI_PTR_VALID_CHECK(hostif_trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&sdb_group, 0, sizeof(sdb_group));
    ret = ctclib_opb_alloc_offset(&p_port_glb->hostif_group_id_opb, &index);
    if (ret)
    {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    sdb_group.key.group_id = index;
    
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
            sdb_group.queue_id = attr->value.u32;
            break;

        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            sdb_group.police_oid = attr->value.oid;
            break;

        default:
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

    stbl_copp_group_add_copp_group(&sdb_group);
    p_sdb_group = stbl_copp_group_get_copp_group(&sdb_group.key);
    if (NULL == p_sdb_group)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    *hostif_trap_group_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_TRAP_GROUP, p_sdb_group->key.group_id);

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove host interface trap group
*
* Arguments:
*  [in] hostif_trap_group_id - host interface trap group id
*
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_hostif_trap_group(
    _In_ sai_object_id_t hostif_trap_group_id
    )
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Set host interface trap group attribute value.
*
* Arguments:
*    [in] hostif_trap_group_id - host interface trap group id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_trap_group_attribute(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ const sai_attribute_t *attr
    )
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:hostif_trap_group_id 0x%llx", hostif_trap_group_id);
    CTC_SAI_PTR_VALID_CHECK(attr);

    switch(attr->id)
    {        
    case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
        ret = ctc_sai_set_trap_group_queue(hostif_trap_group_id, attr->value.u32);
        break;
        
    case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
        ret = ctc_sai_set_trap_group_policer(hostif_trap_group_id, attr->value.oid);
        break;
    case SAI_HOSTIF_TRAP_GROUP_ATTR_SSH_POLICER:
        ret = ctc_sai_set_trap_group_ssh_policer(hostif_trap_group_id, attr->value.oid);
        break;
        
    case SAI_HOSTIF_TRAP_GROUP_ATTR_TELNET_POLICER:
        ret = ctc_sai_set_trap_group_telnet_policer(hostif_trap_group_id, attr->value.oid);
        break;
    
    case SAI_HOSTIF_TRAP_GROUP_ATTR_MLAG_POLICER:
        ret = ctc_sai_set_trap_group_mlag_policer(hostif_trap_group_id, attr->value.oid);
        break;

    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (0 != ret)
    {
        log_sys(M_MOD_SAI, E_ERROR, "TODO Trap group ID %"PRIu64", attr ID %u ret %d", hostif_trap_group_id, attr->id, ret);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_trap_group_attribute(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{

    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE:
            ret = ctc_sai_get_trap_group_queue(hostif_trap_group_id, attr);
            break;
            
        case SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER:
            ret = ctc_sai_get_trap_group_policer(hostif_trap_group_id, attr);
            break;

        case SAI_HOSTIF_TRAP_GROUP_ATTR_COUNTER:
            ret = ctc_sai_get_trap_group_counter(hostif_trap_group_id, attr);
            break;

        default:
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    return ret;
}


/*
* Routine Description:
*   Set trap attribute value.
*
* Arguments:
*    [in] hostif_trap_id - host interface trap id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_trap_attribute(
    _In_ sai_hostif_trap_id_t hostif_trapid,
    _In_ const sai_attribute_t *attr
    )
{
    stbl_copp_trap_t sdb_trap;
    stbl_copp_trap_t *p_sdb_trap = NULL;
    sai_status_t ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:hostif_trapid %u", hostif_trapid);
    CTC_SAI_PTR_VALID_CHECK(attr);
    
    p_sdb_trap = ctc_sai_copp_get_trap(hostif_trapid);
    if (NULL == p_sdb_trap)
    {
        sal_memset(&sdb_trap, 0, sizeof(sdb_trap));
        sdb_trap.key.trap_id = hostif_trapid;
        stbl_copp_trap_add_copp_trap(&sdb_trap);
        p_sdb_trap = stbl_copp_trap_get_copp_trap(&sdb_trap.key);
        if (NULL == p_sdb_trap)
        {
            return SAI_STATUS_NO_MEMORY;
        }
    }
    p_sdb_trap->ctc_reason_id = sai_trap_id_to_ctc_reason_id(hostif_trapid);

    switch(attr->id)
    {        
    case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
        ret = ctc_sai_set_trap_attr_group(hostif_trapid, attr->value.oid);
        break;
        
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }
    
    return ret;

}

/*
* Routine Description:
*   Get trap attribute value.
*
* Arguments:
*    [in] hostif_trap_id - host interface trap id
*    [in] attr_count - number of attributes
*    [in,out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_trap_attribute(
    _In_ sai_hostif_trap_id_t hostif_trapid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{

    sai_status_t                ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t*            attr        = NULL;
    uint32_t                    attr_idx    = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP:
            ret = ctc_sai_get_trap_attr_group(hostif_trapid, attr);
            break;
        default: 
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    return ret;
}


/*
* Routine Description:
*   Set user defined trap attribute value.
*
* Arguments:
*    [in] hostif_user_defined_trap_id - host interface user defined trap id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_user_defined_trap_attribute(
    _In_ sai_hostif_user_defined_trap_id_t hostif_user_defined_trapid,
    _In_ const sai_attribute_t *attr)
{

    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Get user defined trap attribute value.
*
* Arguments:
*    [in] hostif_user_defined_trap_id - host interface user defined trap id
*    [in] attr_count - number of attributes
*    [in,out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_user_defined_trap_attribute(
    _In_ sai_hostif_user_defined_trap_id_t hostif_user_defined_trapid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Create host interface
*
* Arguments:
*    [out] hif_id - host interface id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_hostif(
    _Out_ sai_object_id_t *hif_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    stbl_hostif_t       *p_sdb_hif = NULL;
    sai_attribute_t     *attr       = NULL;
    sai_object_id_t     portid       = 0;
    uint32_t            attr_idx = 0;
    char    ifname[32];
    uint32  vlanid = 0;
    int     fd = -1;

    CTC_SAI_DEBUG_FUNC();
    sai_create_hostif_debug_param(attr_count, attr_list);
    sal_memset(ifname, 0, sizeof(ifname));

    for(attr_idx = 0; attr_idx < attr_count; attr_idx ++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;

        switch (attr->id)
        {
        case SAI_HOSTIF_ATTR_TYPE:
            break;

        case SAI_HOSTIF_ATTR_RIF_OR_PORT_ID:
            portid = attr->value.oid;
            break;

        case SAI_HOSTIF_ATTR_NAME:
            sal_strcpy(ifname, attr->value.chardata);
            break;
        }
    }

    /* use ioctl to create tap interface in linux kernel */
    fd = ctc_create_net_device(ifname);    
    if (fd < 0)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /* get vlan id by vlan interface name */
    if (0 == sal_memcmp(ifname, "vlan", 4))
    {
        sal_sscanf(ifname, "vlan""%u", &vlanid);
        ctc_vlan_set_arp_excp_type(vlanid, CTC_EXCP_FWD_AND_TO_CPU);    
    }

    p_sdb_hif = ctc_sai_hostif_db_alloc(ifname, fd, portid, vlanid);
    if (NULL == p_sdb_hif)
    {
        ctc_remove_net_device(fd);
        return SAI_STATUS_NO_MEMORY;
    }

    *hif_id = p_sdb_hif->key.hif_oid;

    /* add fd to epoll socket list */
    g_hostif_info.evl.data.fd = fd;
    epoll_ctl(g_hostif_info.epoll_sock, EPOLL_CTL_ADD, fd, &g_hostif_info.evl);
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Remove host interface
*
* Arguments:
*    [in] hif_id - host interface id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
int32_t sai_remove_hostif(
    _In_ sai_object_id_t hif_id
    )
{
    stbl_hostif_t *p_sdb_hif = NULL;
    int32 fd = -1;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:hif_id 0x%llx", hif_id);
    p_sdb_hif = ctc_sai_hostif_get_by_hostif_id(hif_id);
    if (NULL == p_sdb_hif)
    {
        return SAI_STATUS_FAILURE;
    }

    /* vlan interface process */
    if (0 != p_sdb_hif->vid)
    {
        ctc_vlan_set_arp_excp_type(p_sdb_hif->vid, CTC_EXCP_NORMAL_FWD);
    }

    /* remove fd from epoll socket list */
    epoll_ctl(g_hostif_info.epoll_sock, EPOLL_CTL_DEL, p_sdb_hif->fd, &g_hostif_info.evl);

    /* remove host interface from hash and free the memory */
    fd = p_sdb_hif->fd;
    ctc_sai_hostif_db_release(p_sdb_hif);

    /* remove net device from linux kernel */
    ctc_remove_net_device(fd);

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Set host interface attribute
*
* Arguments:
*    [in] hif_id - host interface id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_hostif_attribute(
    _In_ sai_object_id_t hif_id,
    _In_ const sai_attribute_t *attr)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*    Get host interface attribute
*
* Arguments:
*    [in] hif_id - host interface id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_hostif_attribute(
    _In_ sai_object_id_t  hif_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   hostif receive function
*
* Arguments:
*    [in]  hif_id  - host interface id
*    [out] buffer - packet buffer
*    [in,out] buffer_size - [in] allocated buffer size. [out] actual packet size in bytes
*    [in,out] attr_count - [in] allocated list size. [out] number of attributes
*    [out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    SAI_STATUS_BUFFER_OVERFLOW if buffer_size is insufficient,
*    and buffer_size will be filled with required size. Or
*    if attr_count is insufficient, and attr_count
*    will be filled with required count.
*    Failure status code on error
*/
sai_status_t sai_recv_hostif_packet(
    _In_ sai_object_id_t  hif_id,
    _Out_ void *buffer,
    _Inout_ sai_size_t *buffer_size,
    _Inout_ uint32_t *attr_count,
    _Out_ sai_attribute_t *attr_list)
{
    CTC_SAI_DEBUG_FUNC();
    return SAI_STATUS_SUCCESS;
}

typedef struct
{
#if _GLB_HOST_IS_LE
    uint8   ihl:4;                  /* header length */
    uint8   version:4;              /* version */
#else
    uint8   version:4;              /* version */
    uint8   ihl:4;                  /* header length */
#endif
    uint8   tos;                    /* type of service */
    uint16  total_length;           /* total length */
    
    uint16  identification;         /* identification */
    uint16  offset;                 /* fragment offset field */
    
    uint8   ttl;                    /* time to live */
    uint8   protocol;               /* protocol */
    uint16  checksum;               /* checksum */
    
    uint32  src_ip_addr;            /* src ip */    
    uint32  dest_ip_addr;           /* dest ip */
} ptp_ip_header_t;

int32
_hsrv_ipuc_get_nb_by_ipda_fn(void *obj, stbl_iter_args_t *pargs)
{
    stbl_neighbor_t *p_sdb_nb = (stbl_neighbor_t*)obj;
    stbl_neighbor_key_t *p_key = (stbl_neighbor_key_t*)pargs->argv[0];

    if (p_sdb_nb->key.ip_address.addr_family == p_key->ip_address.addr_family &&
       p_sdb_nb->key.ip_address.addr.ip4== p_key->ip_address.addr.ip4)
    {
        p_key->rif_id = p_sdb_nb->key.rif_id;
        return -1;
    }

    return 0;
}

int32
_hsrv_ipuc_get_nb_by_ipda(stbl_neighbor_key_t *p_key)
{
    stbl_iter_args_t args;

    args.argv[0] = p_key;

    stbl_neighbor_iterate(_hsrv_ipuc_get_nb_by_ipda_fn, &args);
    return 0;
}

int32
_hsrv_ipuc_get_nhid_by_ipda(ip_addr_t ip, uint64* p_nhid, uint64 vr_id)
{
    uint32 len_mask = 0;
    ip_addr_t ip_da;
    stbl_route_t                *p_sdb_route = NULL;
    stbl_neighbor_t             *p_sdb_nb    = NULL;
    stbl_nexthop_t              *p_sdb_nh    = NULL;
    stbl_neighbor_key_t         nb_key;
    stbl_route_key_t            rt_key;
    stbl_nexthop_key_t          nh_key;
    
    sal_memset(&nb_key, 0, sizeof(nb_key));
    
    nb_key.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    nb_key.ip_address.addr.ip4 = ip;
    nb_key.rif_id = 0;

    /*lookup neighbor*/
    _hsrv_ipuc_get_nb_by_ipda(&nb_key);
    
    if (0 != nb_key.rif_id)
    {
        p_sdb_nb = stbl_neighbor_get_neighbor(&nb_key);
        if (NULL != p_sdb_nb)
        {
            /*get nexthop*/
            sal_memset(&nh_key, 0, sizeof(nh_key));
            nh_key.rif_id = p_sdb_nb->key.rif_id;
            sal_memcpy(&nh_key.ip_address, &p_sdb_nb->key.ip_address, sizeof(sai_ip_address_t));
            p_sdb_nh = stbl_nexthop_get_nexthop(&nh_key);
            if (NULL == p_sdb_nh)
            {
                CTC_SAI_DEBUG("Get nexthop error by ip_address %x rif_id %"PRIu64"\n", p_sdb_nb->key.ip_address.addr.ip4, p_sdb_nb->key.rif_id);
                CTC_SAI_DEBUG("Next step lookup route \n");
            }
            else
            {
                /*got it*/
                *p_nhid = p_sdb_nh->nh_oid;
                return 0;
            }
        }
    }

    /*lookup route*/
    sal_memset(&rt_key, 0, sizeof(rt_key));
    
    for(len_mask = 32; len_mask > 0; len_mask--)
    {
        if(32 == len_mask)
        {
            ip_da = ip;
        }
        else
        {
           ip_da = (ip & (~(0xffffffff >> len_mask)));
        }
        
        rt_key.vr_id = vr_id;

        //if (AF_INET == p_rt_node->key.p.family)
        {
            rt_key.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
            rt_key.destination.addr.ip4 = sal_htonl(ip_da);
            IPV4_LEN_TO_MASK(rt_key.destination.mask.ip4, len_mask);
        }
        
        CTC_SAI_DEBUG("Get ip %x family %d in vr id %"PRIu64"\n",
               rt_key.destination.addr.ip4, rt_key.destination.addr_family, rt_key.vr_id);
               
        p_sdb_route = stbl_route_get_route(&rt_key);
        if (NULL != p_sdb_route)
        {
            /*got it*/
            break;
        }
    }
    
    if (NULL == p_sdb_route)
    {
        CTC_SAI_DEBUG("Get nexthop id fail\n");
        return -1; /*ipuc is not exist*/
    }

    *p_nhid = p_sdb_route->nh_oid;

    return 0;
}

int32 sai_send_hostif_packet_insert_ptp(ctc_pkt_info_t  *tx_info, sai_pkt_ptp_info_t *pkt_ptp_info, char *buffer)
{
    int32 ret = 0;
    if (pkt_ptp_info->oper == SAI_PTP_CAPTURE_ONLY)
    {
        #ifdef USW
        pkt_ptp_info->oper = SAI_PTP_CORRECTION;
        GLB_SET_FLAG(tx_info->flags, CTC_PKT_FLAG_PTP_TS_PRECISION);
        #endif
    }

    ctc_nh_info_t nh_info;
    
    tx_info->ptp.oper = pkt_ptp_info->oper;
    tx_info->ptp.seq_id = pkt_ptp_info->seq_id;
    tx_info->ptp.ts.seconds = pkt_ptp_info->ts.seconds;
    tx_info->ptp.ts.nanoseconds = pkt_ptp_info->ts.nanoseconds;
    tx_info->ptp.ts_offset = pkt_ptp_info->ts_offset;

    /*from _umpeth_tx_priv_proc_ptp*/
    if (pkt_ptp_info->is_unicast)
    {
        //PTP_DESC_BUFFER_(buffer, ETH_HEADER_LEN + 20 + 8, TRUE);
        
        //ptp_ip_header_t *hdr = NULL;
        //hdr = (ptp_ip_header_t *)(buffer + ETH_HEADER_LEN);
        uint32 destIp = 0;
        uint64 nhid = 0;
        
        destIp = (buffer[30] << 24) | (buffer[31] << 16) | (buffer[32] << 8) | (buffer[33]);
        
        ret = _hsrv_ipuc_get_nhid_by_ipda(destIp, &nhid, pkt_ptp_info->vr_id);
        if(ret || (2 == nhid))
        {
            CTC_SAI_DEBUG("get nexthop id fail (%d), or nhid is 2\n", ret);
            return -1;
        }
        else
        {
            ret = _route_check_nexthop_exist(nhid);
            if (ret != 0)
            {
                CTC_SAI_DEBUG("nexthop id (%"PRIu64") is not exist\n", nhid);
                return ret;
            }

            ctc_nh_get_nh_info(nhid, &nh_info);
            tx_info->nh_offset = nh_info.dsnh_offset[0];
            //hsrv_nh_get_offset_by_nh_id(nhid, &(p_encap_info->tx_info.nh_offset));
              CTC_SET_FLAG(tx_info->flags, CTC_PKT_FLAG_NH_OFFSET_VALID);
            //p_encap_info->is_bypass = 0;
        }
    }
            
    /*from _umpeth_tx_priv_proc_ptp*/
    CTC_SET_FLAG(tx_info->flags, CTC_PKT_FLAG_PRIORITY);
    /*fix bug 49187*/
    //tx_info->priority = pkt_ptp_info->vlan_priority * 8;
    #ifdef USW
    tx_info->priority = 15;
    #else
    tx_info->priority = 63;
    #endif
    /*cos not have correspongdign flag */
    tx_info->src_cos = pkt_ptp_info->vlan_priority;
    //CTC_SET_FLAG(tx_info->flags, CTC_PKT_FLAG_COLOR);  
    //tx_info->color = GLB_QOS_COLOR_NONE;
    if (pkt_ptp_info->vlan_id != 0)
    {
        CTC_SET_FLAG(tx_info->flags, CTC_PKT_FLAG_SRC_SVID_VALID);
        tx_info->src_svid = pkt_ptp_info->vlan_id;
        tx_info->fid= pkt_ptp_info->vlan_id;
    }

    return 0;
}

int32 sai_send_hostif_packet_insert_tpoam(ctc_pkt_info_t  *tx_info, sai_pkt_oam_info_t *pkt_tpoam_info)
{
    ctc_nh_info_t nh_info = {0};
    
    tx_info->oam.type = pkt_tpoam_info->type;
    tx_info->oam.flags = pkt_tpoam_info->flags;
    tx_info->oam.vid = pkt_tpoam_info->vid;
    tx_info->oam.mep_index = pkt_tpoam_info->mep_index;
    tx_info->oam.lm_fcl = pkt_tpoam_info->lm_fcl;
    tx_info->oam.dm_ts.seconds = pkt_tpoam_info->dm_ts.seconds;
    tx_info->oam.dm_ts.nanoseconds = pkt_tpoam_info->dm_ts.nanoseconds;
    tx_info->oam.dm_ts_offset = pkt_tpoam_info->dm_ts_offset;

    tx_info->flags = pkt_tpoam_info->bit_flags;
    
    //tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_IS_8W;
    tx_info->priority = pkt_tpoam_info->priority;
    tx_info->src_cos = pkt_tpoam_info->src_cos;
    tx_info->ttl = pkt_tpoam_info->ttl;
    tx_info->dest_gport = pkt_tpoam_info->dest_gport;

    ctc_nh_get_nh_info(pkt_tpoam_info->nh_id, &nh_info);
    tx_info->nh_offset = nh_info.dsnh_offset[0];
    tx_info->nhid = pkt_tpoam_info->nh_id;
    
    tx_info->color = pkt_tpoam_info->color;
    //tx_info-> = pkt_tpoam_info->is_bypass;
    tx_info->src_port = pkt_tpoam_info->src_gport;
    
    tx_info->oper_type = CTC_PKT_OPER_OAM;
    tx_info->is_critical = TRUE;
    tx_info->oam.type = CTC_OAM_TYPE_ACH;
        
    return 0;
}

/*
* Routine Description:
*   hostif send function
*
* Arguments:
*    [in] hif_id  - host interface id. only valid for send through FD channel. Use SAI_NULL_OBJECT_ID for send through CB channel.
*    [In] buffer - packet buffer
*    [in] buffer size - packet size in bytes
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_send_hostif_packet(
    _In_ sai_object_id_t hif_id,
    _In_ void *buffer,
    _In_ sai_size_t buffer_size,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    ctc_pkt_tx_t    pkt_tx;
    ctc_pkt_skb_t   *p_skb = NULL;
    uint32_t        attr_idx = 0;
    uint32_t        port_type = 0;
    uint32_t        tid = 0;
    sai_object_id_t portid = 0;
    sai_object_id_t ingress_portid = 0;
    sai_attribute_t *attr = NULL;
    stbl_hostif_t   *p_sdb_hif = NULL;
    int32_t         sdk_rc = 0;
    int32_t         ret = 0;
    sai_uint16_t    vlan_id = 0;
    uint32          gport = 0;
    uint32          is_mcast = FALSE;
    uint8           oper_type = CTC_PKT_OPER_NORMAL;
    sai_attribute_t debug_attr[1];
    
    sai_pkt_ptp_info_t pkt_ptp_info;
    sal_memset(&pkt_ptp_info, 0, sizeof(sai_pkt_ptp_info_t));

    sai_pkt_oam_info_t pkt_tpoam_info;
    sal_memset(&pkt_tpoam_info, 0, sizeof(sai_pkt_oam_info_t));

    debug_attr[0].value.u32 = 0;
    
    CTC_SAI_DEBUG_FUNC();
    sal_memset(&pkt_tx, 0, sizeof(ctc_pkt_tx_t));
    p_skb = &(pkt_tx.skb);
    ctc_packet_skb_init(p_skb);

    ctc_packet_skb_put(p_skb, buffer_size);
    sal_memcpy(p_skb->data, buffer, buffer_size);

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;

        switch (attr->id)
        {
        case SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG:
            portid = attr->value.oid;
            break;
            
        case SAI_HOSTIF_PACKET_EGRESS_PORT_VLAN:
            vlan_id = attr->value.u16;
            break;    
            
        case SAI_HOSTIF_PACKET_INGRESS_PORT:
            is_mcast = TRUE;
            ingress_portid = attr->value.oid;
            break;

        case SAI_HOSTIF_PACKET_PTP_METADATA:
            oper_type = CTC_PKT_OPER_PTP;
            break;

        case SAI_HOSTIF_PACKET_PTP_PACKET_RCV:
            sal_memcpy(&pkt_ptp_info, &(attr->value.ptp), sizeof(sai_pkt_ptp_info_t));
            oper_type = CTC_PKT_OPER_PTP;
            is_mcast = FALSE;
            break;

        case SAI_HOSTIF_PACKET_TPOAM_PACKET_RCV:
            sal_memcpy(&pkt_tpoam_info, &(attr->value.tpoam), sizeof(sai_pkt_oam_info_t));
            oper_type = CTC_PKT_OPER_OAM;
            is_mcast = FALSE;
            break;
    
        case SAI_HOSTIF_PACKET_PDU_TYPE:
            debug_attr[0].value.u32 = attr->value.u32;
            break;
    
        default:
            break;
        }
    }

    if (is_mcast)
    {
        pkt_tx.mode = CTC_PKT_MODE_DMA;
        pkt_tx.lchip = 0;
        pkt_tx.tx_info.ttl = 1;
        pkt_tx.tx_info.oper_type = oper_type;
        pkt_tx.tx_info.is_critical = TRUE;
        pkt_tx.tx_info.src_svid = vlan_id;
        pkt_tx.tx_info.dest_group_id = vlan_id;
        pkt_tx.tx_info.flags |= CTC_PKT_FLAG_HASH_VALID;
        pkt_tx.tx_info.flags |= CTC_PKT_FLAG_MCAST;
        pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_SVID_VALID;

        if (ingress_portid)
        {
            port_type = CTC_SAI_OBJECT_TYPE_GET(ingress_portid);
            if (SAI_OBJECT_TYPE_LAG == port_type)
            {
                tid = CTC_SAI_OBJECT_INDEX_GET(ingress_portid);
                pkt_tx.tx_info.src_port = CTC_MAP_TID_TO_GPORT(tid);
            }
            else if (SAI_OBJECT_TYPE_PORT == port_type)
            {
                pkt_tx.tx_info.src_port = CTC_SAI_OBJECT_INDEX_GET(ingress_portid);
            }
            else 
            {
                return SAI_STATUS_INVALID_PARAMETER;
            }
            pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_PORT_VALID;
            //ret = ctc_fdb_get_gport(p_skb->data, vlan_id, &gport);
        }
        else
        {
            pkt_tx.tx_info.src_port = 0;
            //pkt_tx.tx_info.src_port = CPU_PORT; ?
        }

        sdk_rc = ctc_packet_tx(&pkt_tx);
        if (!sdk_rc) {
            ctc_sai_get_sys_info()->sai_switch_notification_table.on_cpu_packet_debug(p_skb->data, p_skb->tail - p_skb->data + 1, 1, debug_attr);
        }

        ctc_sai_call_cmdel_dma();
        
        return ctc_sai_get_error_from_sdk_error(sdk_rc);
    }
    else
    {
        if (vlan_id)
        {
            /* vlan interface */
            /* modified by liwh for bug 48710, 2018-09-06 */
            //ctc_packet_skb_put(p_skb, buffer_size);
            //sal_memcpy(p_skb->data, buffer, buffer_size);
            /* liwh end */
            
            pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_SVID_VALID;

            /* first get port from static fdb */
            ret = ctc_fdb_get_gport(p_skb->data, vlan_id, &gport);

            p_sdb_hif = ctc_sai_hostif_get_by_vlan(vlan_id);
            if (NULL == p_sdb_hif)
            {
                return SAI_STATUS_FAILURE;
            }

            /* flooding in vlan */
            if (SAI_STATUS_SUCCESS != ret)
            {
                pkt_tx.tx_info.flags |= CTC_PKT_FLAG_MCAST;
                pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_SVID_VALID;
                pkt_tx.tx_info.src_svid = p_sdb_hif->vid;
                pkt_tx.tx_info.dest_group_id = p_sdb_hif->vid;
            }
            /* unicat to port */
            else
            {
                pkt_tx.tx_info.flags |= CTC_PKT_FLAG_NH_OFFSET_VALID;
                pkt_tx.tx_info.flags |= CTC_PKT_FLAG_SRC_SVID_VALID;
                pkt_tx.tx_info.src_svid = p_sdb_hif->vid;
                pkt_tx.tx_info.nh_offset = p_sdb_hif->nexthop_ptr;
                pkt_tx.tx_info.dest_gport = gport;
            }

            if (oper_type == CTC_PKT_OPER_PTP)
            {
                ret= sai_send_hostif_packet_insert_ptp(&(pkt_tx.tx_info), &pkt_ptp_info, (char *)buffer);
                if (0 != ret)
                {
                    return SAI_STATUS_FAILURE;
                }
            }

            if (oper_type == CTC_PKT_OPER_OAM)
            {
                ret= sai_send_hostif_packet_insert_tpoam(&(pkt_tx.tx_info), &pkt_tpoam_info);
                if (0 != ret)
                {
                    return SAI_STATUS_FAILURE;
                }
            }
        
            pkt_tx.mode = CTC_PKT_MODE_DMA;
            pkt_tx.lchip = 0;
            pkt_tx.tx_info.ttl = 1;
            pkt_tx.tx_info.oper_type = oper_type;
            pkt_tx.tx_info.is_critical = TRUE;

            sdk_rc = ctc_packet_tx(&pkt_tx);
            if (!sdk_rc) {
                ctc_sai_get_sys_info()->sai_switch_notification_table.on_cpu_packet_debug(p_skb->data, p_skb->tail - p_skb->data + 1, 1, debug_attr);
            }

            ctc_sai_call_cmdel_dma();
            
            return ctc_sai_get_error_from_sdk_error(sdk_rc);
        }

        if (portid)
        {
            port_type = CTC_SAI_OBJECT_TYPE_GET(portid);
            if (SAI_OBJECT_TYPE_LAG == port_type)
            {
                tid = CTC_SAI_OBJECT_INDEX_GET(portid);
                pkt_tx.tx_info.dest_gport = CTC_MAP_TID_TO_GPORT(tid);
            }
            else if (SAI_OBJECT_TYPE_PORT == port_type)
            {
                pkt_tx.tx_info.dest_gport = CTC_SAI_OBJECT_INDEX_GET(portid);
            }
            else 
            {
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            /* Delete by kcao for bug 49009, dest gport is invalid */
            //pkt_tx.tx_info.dest_gport = CTC_SAI_OBJECT_INDEX_GET(hif_id);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (oper_type == CTC_PKT_OPER_PTP)
        {
            ret = sai_send_hostif_packet_insert_ptp(&(pkt_tx.tx_info), &pkt_ptp_info, buffer);
            if (0 != ret)
            {
                CTC_SAI_DEBUG("insert ptp failed \n");
                return SAI_STATUS_FAILURE;
            }
        }

        if (oper_type == CTC_PKT_OPER_OAM)
        {
            ret = sai_send_hostif_packet_insert_tpoam(&(pkt_tx.tx_info), &pkt_tpoam_info);
            if (0 != ret)
            {
                CTC_SAI_DEBUG("insert tpoam failed \n");
                return SAI_STATUS_FAILURE;
            }
        }

        pkt_tx.mode = CTC_PKT_MODE_DMA;
        pkt_tx.lchip = 0;
        pkt_tx.tx_info.ttl = 1;
        pkt_tx.tx_info.oper_type = oper_type;
        pkt_tx.tx_info.is_critical = TRUE;
        /* Added by kcao for bug 43952, only ucast packet need set bypass */
        if (!CTC_FLAG_ISSET(pkt_tx.tx_info.flags, CTC_PKT_FLAG_MCAST))
        {
            //if (oper_type != CTC_PKT_OPER_OAM)
            {
                pkt_tx.tx_info.flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;
            }
        }
        
        if (oper_type == CTC_PKT_OPER_OAM)
        {
            //pkt_tx.tx_info.flags = 344;
            if (!pkt_tpoam_info.is_bypass)
            {
                CTC_UNSET_FLAG(pkt_tx.tx_info.flags, CTC_PKT_FLAG_NH_OFFSET_BYPASS);
            }
            else
            {
                /*lbm only bypass flag*/
                pkt_tx.tx_info.flags = 32;
            }
            pkt_tx.tx_info.ttl = 255;
        }

        sdk_rc = ctc_packet_tx(&pkt_tx);
        if (!sdk_rc) {
            ctc_sai_get_sys_info()->sai_switch_notification_table.on_cpu_packet_debug(p_skb->data, p_skb->tail - p_skb->data + 1, 1, debug_attr);
        }

        ctc_sai_call_cmdel_dma();

        return ctc_sai_get_error_from_sdk_error(sdk_rc);
    }
}

sai_status_t
ctc_sai_hostif_db_dhcp_sys_syn(uint32 enable)
{
    uint32_t trap_id = SAI_HOSTIF_TRAP_ID_DHCP;

    if (enable)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);
    }
    else
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NOPROCESS);
    }
    
    return SAI_STATUS_SUCCESS;
}

int32
_ctc_sai_copp_add_default_reason()
{
    stbl_copp_reason_t reason;
    uint32_t reason_id = 0;

    sal_memset(&reason, 0, sizeof(reason));

    /* only add reason_id used in sai_trap_id_to_ctc_reason_id() */
    for (reason_id = 0; reason_id < CTC_SAI_COPP_EXT_MAX; reason_id++)
    {
        reason.key.reason_id = reason_id;
        reason.channel = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;
        if ((reason_id >= CTC_PKT_CPU_REASON_OAM) && (reason_id <= CTC_PKT_CPU_REASON_OAM_DEFECT_MESSAGE))
        {
            reason.channel = SAI_HOSTIF_TRAP_CHANNEL_CB;
        }
        if (CTC_SAI_COPP_EXT_CPU_MIRROR == reason_id)
        {
            reason.channel = SAI_HOSTIF_TRAP_CHANNEL_CB;    
        }
        if ((reason_id >= CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC) && (reason_id <= CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC))
        {
            reason.channel = SAI_HOSTIF_TRAP_CHANNEL_CB;
        }

        stbl_copp_reason_add_copp_reason(&reason);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_hostif_db_init()
{
    stbl_port_glb_t *p_port_glb = stbl_port_glb_get_port_glb();
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    uint32_t trap_id = 0;

    sal_memset(&g_hostif_info, 0, sizeof(g_hostif_info));
    p_sai_glb->hostif_group_max_count = 256;
    p_sai_glb->hostif_max_count = 2048;
    
    if (0 != ctclib_opb_create(&p_port_glb->hostif_group_id_opb, 0, p_sai_glb->hostif_group_max_count, "hostif_group_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    if (0 != ctclib_opb_create(&p_port_glb->hostif_id_opb, 0, p_sai_glb->hostif_max_count, "hostif_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    p_sai_glb->hostif_fd_hash = ctclib_hash_create(_hostif_fd_hash_make, _hostif_fd_hash_cmp);
    p_sai_glb->hostif_portif_hash = ctclib_hash_create(_hostif_port_hash_make, _hostif_port_hash_cmp);
    p_sai_glb->hostif_vlanif_hash = ctclib_hash_create(_hostif_vlan_hash_make, _hostif_vlan_hash_cmp);

    _ctc_sai_copp_add_default_reason();
    
    /* init all trap id */
    for (trap_id = SAI_HOSTIF_TRAP_ID_STP; trap_id <= SAI_HOSTIF_TRAP_ID_SCL_MATCH; trap_id++)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_CB);
    }

    for (trap_id = SAI_HOSTIF_TRAP_ID_ARP_REQUEST; trap_id <= SAI_HOSTIF_TRAP_ID_MLD_V2_REPORT; trap_id++)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);
    }
    
    for (trap_id = SAI_HOSTIF_TRAP_ID_L2_PORT_MAC_LIMIT; trap_id <= SAI_HOSTIF_TRAP_ID_CPU_MIRROR; trap_id++)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_CB);
    }

    for (trap_id = SAI_HOSTIF_TRAP_ID_L3_MTU_ERROR; trap_id <= SAI_HOSTIF_TRAP_ID_TTL_ERROR; trap_id++)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);
    }
    for (trap_id = SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY; trap_id <= SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT; trap_id++)
    {
        ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);
    }

    /*added openflow trap*/
    ctc_sai_create_hostif_trap(SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_OPENFLOW_TO_CONTROLLER, SAI_HOSTIF_TRAP_CHANNEL_CB);

    /*modified by yejl to change SAI_HOSTIF_TRAP_CHANNEL_CB to SAI_HOSTIF_TRAP_CHANNEL_NETDEV*/
    trap_id = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L3_COPY_CPU;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);

    trap_id = SAI_HOSTIF_TRAP_ID_DHCP;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);

    trap_id = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_PTP_TIMESTAMP;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);

    trap_id = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_PROTOCOL_MAC;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_CB);

    trap_id = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_L2PRO_GROUP_MAC;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_CB);

    trap_id = SAI_HOSTIF_TRAP_ID_L2_COPY_TO_CPU;
    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);
//    trap_id = SAI_HOSTIF_TRAP_ID_CUSTOM_EXCEPTION_CPU_MIRROR;
//    ctc_sai_create_hostif_trap(trap_id, SAI_HOSTIF_TRAP_CHANNEL_CB);
    return SAI_STATUS_SUCCESS;
}

static sai_status_t __init_mode(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    ctc_sai_hostif_db_init();

    /* create socket to kernal */
    ctc_lib_netlink_socket();

    ctc_packet_epool_init();

    ctc_sai_cpu_traffic_init();
    ctc_sai_init_other_cpu_traffic();
    
    system("mkdir /dev/net > /dev/null 2>&1");
    system("mknod /dev/net/tun c 10 200 > /dev/null 2>&1");
    system("chmod 777 /dev/net/tun > /dev/null 2>&1");
    preg->init_status = INITIALIZED;

    sal_task_create(&g_hostif_info.recv_task, "saiRecvThread", SAL_DEF_TASK_STACK_SIZE, 0,
        ctc_packet_receive_from_kernel, NULL);
    return ret;
}

static sai_status_t __exit_mode(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/*
* hostif methods table retrieved with sai_api_query()
*/
 static sai_hostif_api_t g_sai_api_func = {
    .create_hostif                       = sai_create_hostif,
    .remove_hostif                       = sai_remove_hostif,
    .set_hostif_attribute                = sai_set_hostif_attribute,
    .get_hostif_attribute                = sai_get_hostif_attribute,
    .create_hostif_trap_group            = sai_create_hostif_trap_group,
    .remove_hostif_trap_group            = sai_remove_hostif_trap_group,
    .set_trap_group_attribute            = sai_set_trap_group_attribute,
    .get_trap_group_attribute            = sai_get_trap_group_attribute,
    .set_trap_attribute                  = sai_set_trap_attribute,
    .get_trap_attribute                  = sai_get_trap_attribute,
    .set_user_defined_trap_attribute     = sai_set_user_defined_trap_attribute,
    .get_user_defined_trap_attribute     = sai_get_user_defined_trap_attribute,
    .recv_packet                         = sai_recv_hostif_packet,
    .send_packet                         = sai_send_hostif_packet,
};


static ctc_sai_api_reg_info_t g_api_reg_info = {
    .id               = SAI_API_HOST_INTERFACE,
    .init_func        = __init_mode,
    .exit_func        = __exit_mode,
    .api_method_table = &g_sai_api_func,
    .private_data     = NULL,
};

#define ________SAI_OUTER_FUNC
sai_status_t ctc_sai_hostinterface_init()
{
    api_reg_register_fn(&g_api_reg_info);
    return SAI_STATUS_SUCCESS;
}

