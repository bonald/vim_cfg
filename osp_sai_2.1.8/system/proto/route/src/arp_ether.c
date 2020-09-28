#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "gen/tbl_arp_define.h"
#include "gen/tbl_arp.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_iproute_node_define.h"
#include "gen/tbl_iproute_node.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "route.h"
#include "prefix.h"
#include "arp_define.h"
#include "arp_debug.h"
#include "arp_api.h"
#include "arp_ether.h"
#include "arp_sm.h"
#include "arp_log.h"
#include "arpinsp_api.h"
#include "sal_socket.h"
#include "gen/tbl_vrrp_global_define.h"
#include "gen/tbl_vrrp_global.h"
#include "gen/tbl_vrrp_session_define.h"
#include "gen/tbl_vrrp_session.h"
#include "vrrp.h" 
#include "vrrp_api.h"
#include "arpratelimit.h"
#include "arpratelimit_api.h"
#include "nat_api.h"

#define ARP_ETH_BUILD_HDR(_arp_p_, _op_) \
do { \
    (_arp_p_)->ar_hrd = sal_hton16(ARP_HRD_ETHER); \
    (_arp_p_)->ar_pro = sal_hton16(ETH_P_IP); \
    (_arp_p_)->ar_hln = MAC_ADDR_LEN; \
    (_arp_p_)->ar_pln = IPV4_MAX_BYTELEN; \
    (_arp_p_)->ar_op  = sal_hton16(_op_); \
} while (0)

#define ARP_ETH_BUILD_L2_HDR(_ptmp_, _macda_, _macsa_, _type_) \
do { \
    sal_memcpy(_ptmp_, _macda_, MAC_ADDR_LEN); \
    _ptmp_ += MAC_ADDR_LEN; \
    sal_memcpy(_ptmp_, _macsa_, MAC_ADDR_LEN); \
    _ptmp_ += MAC_ADDR_LEN; \
    sal_memcpy(_ptmp_, &_type_, sizeof(uint16)); \
    _ptmp_ += sizeof(uint16); \
} while (0)

#define ARP_ETH_BUILD_VLAN_TAG(_ptmp_, _vlan_cos_, _type_) \
do { \
    sal_memcpy(_ptmp_, &_vlan_cos_, 2); \
    _ptmp_ += 2; \
    sal_memcpy(_ptmp_, &_type_, sizeof(uint16)); \
    _ptmp_ += sizeof(uint16); \
} while (0)

static mac_addr_t arp_bcast_mac_address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static int32
_arp_ether_common_validate(tbl_interface_t *p_db_if, arp_eth_t *arp_p, ds_connected_t **pp_conn)
{
    addr_ipv4_t addr;
    addr_ipv4_t tar_addr;    
    addr_ipv4_t mask_addr;
    addr_ipv4_t host_addr;
    uint32 mac1 = 0;
    uint16 mac2 = 0;
    int32 is_routemac = FALSE;
    ds_connected_t *p_db_conn = NULL;

    sal_memset(&tar_addr, 0, sizeof(tar_addr));
    /* check if sender is in the subnet of incoming interface [R-ARP-2-0010] */
    sal_memcpy((char*)&addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    sal_memcpy((char*)&tar_addr, arp_p->ar_tip, IPV4_MAX_BYTELEN);
    if (arp_api_get_rt_ifc(p_db_if, &addr, &p_db_conn) || !p_db_conn)
    {
        return ARP_ETH_IF_NOT_MATCH;
    }

    /* check sender ip validation */
    if (IN_BADCLASS(sal_ntoh32(addr.s_addr))  ||
        IN_MULTICAST(sal_ntoh32(addr.s_addr)) ||
        IN_LOOPBACK(sal_ntoh32(addr.s_addr))  ||
        IPV4_NET0(sal_ntoh32(addr.s_addr)))
    {
        return ARP_ETH_IP_INVALID;
    }
    
    /* Filter subnet broadcast and network address */
    addr_mask_len2ip_v4(p_db_conn->key.address.prefixlen, &mask_addr);    
    host_addr.s_addr = addr.s_addr & ~(mask_addr.s_addr);
    if (host_addr.s_addr == ~(mask_addr.s_addr) || host_addr.s_addr == 0)
    {
        return ARP_ETH_IP_INVALID;
    }

    /* check sender mac validation */
    mac1 = *(uint32*)&arp_p->ar_sha[0];
    mac2 = *(uint16*)&arp_p->ar_sha[4];
    if ((mac1 == 0 && mac2 == 0) ||
        (mac1 == 0xffffffff && mac2 == 0xffff) ||
        (arp_p->ar_sha[0] & 0x01))
    {
        return ARP_ETH_MAC_INVALID;
    }

    /*check whether mac is routermac*/
    arp_api_check_is_routemac(p_db_if, arp_p->ar_sha, &is_routemac);
    if (is_routemac)
    {
        return ARP_ETH_ERR_IS_ROUTER;
    }

    *pp_conn = p_db_conn;

    return ARP_ETH_SUCCESS;
}

int32
arp_ether_gra_validate(tbl_interface_t *p_db_if, arp_eth_t *arp_p)
{
    ds_connected_t  *p_db_conn = NULL;
    addr_ipv4_t     addr;
    char            ifname_ext[IFNAME_SIZE];
    int32           rc = ARP_ETH_SUCCESS;

    rc = _arp_ether_common_validate(p_db_if, arp_p, &p_db_conn);
    switch (rc)
    {
      case ARP_ETH_IF_NOT_MATCH:
          ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because the sender ip is not in the subnet of the ip of interface",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_IP_INVALID:
          ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because the sender ip is invalid ",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_MAC_INVALID:
         ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because the sender mac is invalid ",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
         return rc;
      case ARP_ETH_ERR_IS_ROUTER:
         ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because the sender mac is router mac",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
         return rc;
      default:
          break;
    }
    if (NULL == p_db_conn)
    {
        return ARP_ETH_IF_NOT_MATCH;
    }

    sal_memcpy(&addr.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);

    /* check whether sender ip and sender mac is myself */
    if (addr.s_addr == p_db_conn->key.address.u.prefix4.s_addr
        && !sal_memcmp(arp_p->ar_sha, p_db_if->mac_addr, MAC_ADDR_LEN))
    {
        //loop packet recved;
        ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because receive the loop packet", 
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_IS_ROUTER;
    }

    return ARP_ETH_SUCCESS;
}

int32
arp_ether_req_validate(tbl_interface_t *p_db_if, arp_eth_t *arp_p)
{
    tbl_route_if_key_t  rtif_key;
    tbl_route_if_t      *p_db_rtif = NULL;
    ds_connected_t      *p_db_conn = NULL;
    addr_ipv4_t         addr;
    addr_ipv4_t         mask_addr;
    char                ifname_ext[IFNAME_SIZE];
    int32               rc = ARP_ETH_SUCCESS;

    sal_memset(&addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&mask_addr, 0, sizeof(addr_ipv4_t));

    rc = _arp_ether_common_validate(p_db_if, arp_p, &p_db_conn);
    switch (rc)   /*Modified by yejl for bug 47073, 2018-06-05*/
    {
      case ARP_ETH_IF_NOT_MATCH:
          ARP_PKT_DEBUG("Request ARP packet discarded on %s, because the sender ip is not in the subnet of the ip of interface",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_IP_INVALID:
          ARP_PKT_DEBUG("Request ARP packet discarded on %s, because the sender ip is invalid",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_MAC_INVALID:
          ARP_PKT_DEBUG("Request ARP packet discarded on %s, because the sender mac is invalid",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_ERR_IS_ROUTER:
          ARP_PKT_DEBUG("Request ARP packet discarded on %s, because the sender mac is router mac",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      default:
          break;
    }
    if (NULL == p_db_conn)
    {
        return ARP_ETH_IF_NOT_MATCH;
    }

    sal_memcpy(&addr.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    addr_mask_len2ip_v4(p_db_conn->key.address.prefixlen, &mask_addr);

    /* check whether sender ip is myself */
    if (addr.s_addr == p_db_conn->key.address.u.prefix4.s_addr)
    {
        //ip conflict is detected;
        ARP_PKT_DEBUG("Request ARP packet discarded on %s, because receive the loop packet",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_IS_ROUTER;
    }

    /* check if target and sender is not same subnet */
    arp_api_get_rt_ifc_for_shortest_prefix(p_db_if, &addr, &p_db_conn);
    sal_memcpy(&addr.s_addr, arp_p->ar_tip, IPV4_MAX_BYTELEN);

    sal_memset(&rtif_key, 0, sizeof(rtif_key));
    sal_strcpy(rtif_key.name, p_db_if->key.name);
    p_db_rtif = tbl_route_if_get_route_if(&rtif_key);
    if (NULL == p_db_rtif)
    {
        ARP_PKT_DEBUG("Request ARP packet discarded on %s, because can not get the related route interface",
                       IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    if (FALSE == p_db_rtif->arp_proxy_en
     && FALSE == p_db_rtif->local_arp_proxy_en
     &&(FALSE == nat_api_if_need_proxy_arp(p_db_rtif, &addr)))/* modified by taocy for static NAT proxy inside global IP, 2019-06-03*/
    {
        if ((addr.s_addr & mask_addr.s_addr) != (p_db_conn->key.address.u.prefix4.s_addr & mask_addr.s_addr))
        {
            ARP_PKT_DEBUG("Request ARP packet discarded on %s, because ARP packet target address is not same with router intf address when arp_proxy_en is disable and local_arp_proxy_en is disable",
                          IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return ARP_ETH_PKT_INVALID;
        }
    }

    /* check target ip validation */
    if (IN_BADCLASS(sal_ntoh32(addr.s_addr))  ||
        IN_MULTICAST(sal_ntoh32(addr.s_addr)) ||
        IN_LOOPBACK(sal_ntoh32(addr.s_addr))  ||
        IPV4_NET0(sal_ntoh32(addr.s_addr)))
    {
        ARP_PKT_DEBUG("Request ARP packet discarded on %s, because the target ip is invalid",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    if (0 == sal_memcmp(arp_p->ar_sha, p_db_if->mac_addr, MAC_ADDR_LEN))
    {
        ARP_PKT_DEBUG("Request ARP packet discarded on %s, because sender mac is same with interface mac address",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    return ARP_ETH_SUCCESS;
}

int32
arp_ether_rep_validate(tbl_interface_t *p_db_if, arp_eth_t *arp_p)
{
    ds_connected_t      *p_db_conn = NULL;
    addr_ipv4_t         addr;
    addr_ipv4_t         mask_addr;
    uint32              mac1 = 0;
    uint16              mac2 = 0;
    char                ifname_ext[IFNAME_SIZE];
    int32               rc = ARP_ETH_SUCCESS;

    sal_memset(&addr, 0, sizeof(addr_ipv4_t));
    sal_memset(&mask_addr, 0, sizeof(addr_ipv4_t));

    rc = _arp_ether_common_validate(p_db_if, arp_p, &p_db_conn);
    switch (rc)
    {
      case ARP_ETH_IF_NOT_MATCH:
          ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the sender ip is not in the subnet of the ip of interface",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_IP_INVALID:
          ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the sender ip is invalid",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_MAC_INVALID:
          ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the sender mac is invalid",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
      case ARP_ETH_ERR_IS_ROUTER:
          ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the sender mac is router mac",
                        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
          return rc;
         
      default:
          break;
    }
    if (NULL == p_db_conn)
    {
        return ARP_ETH_IF_NOT_MATCH;
    }
    
    sal_memcpy(&addr.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    addr_mask_len2ip_v4(p_db_conn->key.address.prefixlen, &mask_addr);

    /* check whether sender ip is myself */
    if (addr.s_addr == p_db_conn->key.address.u.prefix4.s_addr)
    {
        //ip conflict is detected;
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the conflict of arp src-ip is detected",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_IS_ROUTER;
    }

    /* check if target and sender is not same subnet */
    rc = arp_api_get_rt_ifc_for_shortest_prefix(p_db_if, &addr, &p_db_conn);
    if (rc)
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because fail to check if target and sender is not same subnet",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_PTR;
    }
    
    sal_memcpy(&addr.s_addr, arp_p->ar_tip, IPV4_MAX_BYTELEN);
    if ((addr.s_addr & mask_addr.s_addr) != (p_db_conn->key.address.u.prefix4.s_addr & mask_addr.s_addr))
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the target ip and sender ip is not in the same subnet",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_PKT_INVALID;
    }

    /* check if reply is send for this router */
    rc = arp_api_get_rt_ifc(p_db_if, &addr, &p_db_conn);
    if (rc)
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because fail to check if reply is send for this router",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_PTR;
    }

    if (addr.s_addr != p_db_conn->key.address.u.prefix4.s_addr)
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because target ip is not same with route_if ip",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_PKT_INVALID;
    }

    if (0 == sal_memcmp(arp_p->ar_sha, p_db_if->mac_addr, MAC_ADDR_LEN))
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because sender mac is same with interface mac address",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    /* check target mac validation */
    mac1 = *(uint32*)&arp_p->ar_tha[0];
    mac2 = *(uint16*)&arp_p->ar_tha[4];
    if ((mac1 == 0 && mac2 == 0)
            || (mac1 == 0xffffffff && mac2 == 0xffff)
            || (arp_p->ar_tha[0] & 0x01))
    {
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s, because the target mac is invalid",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    return ARP_ETH_SUCCESS;
}

static int32
_arp_ether_gra_rx(tbl_interface_t *p_db_if, uint8 *buf, arp_eth_t *arp_p)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    addr_ipv4_t     ip;
    tbl_arp_t       *ae_p = NULL;
    uint32          is_conflict = FALSE;
    char            ifname_ext[IFNAME_EXT_SIZE]; 
    char            buf1[CMD_BUF_128] = {'\0'};
    char            buf2[CMD_BUF_128] = {'\0'};
    char            buf3[CMD_BUF_128] = {'\0'};
    int32           rc = ARP_ETH_SUCCESS;

    /* 1. validate gratuitous arp */
    sal_memset(&ip, 0, sizeof(addr_ipv4_t));
    PM_E_RETURN(arp_ether_gra_validate(p_db_if, arp_p));

    is_conflict = (NULL != rt_if_lookup_ifc_ipv4(p_db_if, (addr_ipv4_t*)arp_p->ar_sip)) ? TRUE : FALSE;

    if (!p_rt_glb->gratuitous_arp_learn_en && !is_conflict)
    {
        ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s, because gratuitous_arp_learn_en is disable and arp src-ip is not a subnet of router intf address",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    /* 2. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_REV);

    tbl_interface_t *pst_if_cp = p_db_if;
    if (NULL == pst_if_cp)
    {
        return ARP_ETH_ERR_FAIL;
    }
    
    /* the sender is trusted and ip/mac will be learnt */
    /* 3. lookup ARP */
    sal_memcpy(&ip.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    ae_p = arp_db_entry_lookup(&ip);
    if (NULL == ae_p)
    {
        rc = arp_db_entry_create(pst_if_cp, (addr_ipv4_t*)arp_p->ar_sip, NULL,
                                  ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
        if (rc == PM_E_EXCEED_ARP_LIMIT_MAX_NUM)
        {
            if (ARP_PORT_NUMBERLIMIT_PROTECT == pst_if_cp->arp_numberlimit_violate)
            {
                return ARP_ETH_SUCCESS;
            }
            else if (ARP_PORT_NUMBERLIMIT_RESTRICT == pst_if_cp->arp_numberlimit_violate || 
                     ARP_PORT_NUMBERLIMIT_ERRDISABLE == pst_if_cp->arp_numberlimit_violate)
            {
                /*modified by yejl to fix bug44547, 2017-07-05*/
                /*log the arp discard reseaon*/
                IFNAME_ETH2FULL(pst_if_cp->key.name, ifname_ext);
                sal_sprintf(buf1, "Gratuitous arp is discard on %s because of arp number-limit!", ifname_ext);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf1);

                /*log the arp type: gratuitous ARP*/
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, "ARP opcode = Gratuitous ARP");

                /*log the sender MAC and sender IP*/
                sal_sprintf(buf2, " sender address = %02x:%02x:%02x:%02x:%02x:%02x, sender IP address = %d.%d.%d.%d", 
                                                    arp_p->ar_sha[0], arp_p->ar_sha[1], 
                                                    arp_p->ar_sha[2], arp_p->ar_sha[3],
                                                    arp_p->ar_sha[4], arp_p->ar_sha[5],
                                                    arp_p->ar_sip[0], arp_p->ar_sip[1],
                                                    arp_p->ar_sip[2], arp_p->ar_sip[3]);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf2);

                /*log the target MAC and target IP*/
                sal_sprintf(buf3, " target address = %02x:%02x:%02x:%02x:%02x:%02x, target IP address = %d.%d.%d.%d",
                                                    arp_p->ar_tha[0], arp_p->ar_tha[1], 
                                                    arp_p->ar_tha[2], arp_p->ar_tha[3],
                                                    arp_p->ar_tha[4], arp_p->ar_tha[5],
                                                    arp_p->ar_tip[0], arp_p->ar_tip[1],
                                                    arp_p->ar_tip[2], arp_p->ar_tip[3]);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf3);

                /*If violation mode is errdisable, shutdown the port*/
                if (ARP_PORT_NUMBERLIMIT_ERRDISABLE == pst_if_cp->arp_numberlimit_violate)
                {
                    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR));
                }
                return ARP_ETH_SUCCESS;
            }
        }
        else if (rc < 0)
        {
            ARP_PKT_DEBUG("Gratuitous ARP packet discarded on %s for arp_db_entry_create fail",
                          IFNAME_ETH2FULL(pst_if_cp->key.name, ifname_ext));
            return ARP_ETH_ERR_FAIL;
        }
    }

    ae_p->is_gratuitous = TRUE;
    ae_p->rx_buf = buf;

    /* 4. run SM */
    arp_sm(ae_p, AE_RX_ARP_REQ);
    return ARP_ETH_SUCCESS;
}

/*if arp pkt dst ip is me*/
int32 _arp_is_dst_to_me(char *ifname, arp_eth_t *arp_p)
{
    tbl_route_if_t *p_db_rt_if = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ds_connected_t      *p_tmp_conn = NULL;
    char ip_target[INET_ADDRSTRLEN + 1] = {0};
    char ip_tmp[INET_ADDRSTRLEN + 1] = {0};

    sal_inet_ntop(AF_INET, arp_p->ar_tip, ip_target, INET_ADDRSTRLEN);
    p_db_rt_if = tbl_route_if_get_route_if_by_name(ifname);
    
    CTCLIB_SLIST_LOOP(p_db_rt_if->ifc_ipv4.obj_list, p_tmp_conn, listnode)
    {
        cdb_addr_ipv4_val2str(ip_tmp, CMD_BUF_16, &p_tmp_conn->key.address.u.prefix4);
        if (0 == sal_strcmp(ip_tmp, ip_target))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static int32
_arp_ether_req_rx(tbl_interface_t *p_db_if, uint8 *buf, arp_eth_t *arp_p)
{
    tbl_interface_t *pst_if_cp = p_db_if;
    tbl_arp_t       *ae_p = NULL;
    addr_ipv4_t     ip;
    tbl_arp_t       tmp_arp;
    char            ifname_ext[IFNAME_EXT_SIZE]; 
    int32           rc = ARP_ETH_SUCCESS;
    char            buf1[CMD_BUF_64] = {'\0'};
    char            buf2[CMD_BUF_128] = {'\0'};
    char            buf3[CMD_BUF_128] = {'\0'};
    tbl_route_if_t *p_db_rt_if = NULL;

    arp_eth_pkt_move_pld(buf, arp_p);

    /* 1. validate arp request */
    PM_E_RETURN(arp_ether_req_validate(p_db_if, arp_p));

    /* 2. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_REV);
    
    /* 3. lookup ARP */
    sal_memcpy(&ip.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);

    /*fixbug 44425 , add by chenc*/
    if (TRUE != _arp_is_dst_to_me(p_db_if->key.name, arp_p))
    {
        addr_ipv4_t     target_ip = {0};

        p_db_rt_if = tbl_route_if_get_route_if_by_name(p_db_if->key.name);
        if (NULL == p_db_rt_if)
        {
            ARP_PKT_DEBUG("Request ARP packet discard on %s, because can not get the layer3 interface",
                          IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return ARP_ETH_ERR_FAIL;
        }

        sal_memcpy(&target_ip.s_addr, arp_p->ar_tip, IPV4_MAX_BYTELEN);

        /*fixbug 45226, add by chenc*/
        if (FALSE == p_db_rt_if->arp_proxy_en
         && FALSE == p_db_rt_if->local_arp_proxy_en
         &&(FALSE == nat_api_if_need_proxy_arp(p_db_rt_if, &target_ip)))/* modified by taocy for static NAT proxy inside global IP, 2019-06-03*/
        {
            return ARP_ETH_SUCCESS;
        }
    }
    
    ae_p = arp_db_entry_lookup(&ip);
    if (NULL == ae_p)
    {
        rc = arp_db_entry_create(pst_if_cp, &ip, NULL, 
                                  ARP_TYPE_DYNAMIC, FALSE, FALSE, &ae_p);
        if (rc == PM_E_EXCEED_MAX_NUM)
        {
            sal_memset(&tmp_arp, 0, sizeof(tbl_arp_t));
            tmp_arp.ifindex = pst_if_cp->ifindex;
            tmp_arp.key.ip.s_addr = ip.s_addr;
            tmp_arp.status = AS_INIT;
            tmp_arp.type = ARP_TYPE_DYNAMIC;
            tmp_arp.vrf_id = GLB_DEF_VRFID;
            tmp_arp.flags |= ARP_DONOT_INSTALL;
            ae_p = &tmp_arp;
        }
        else if (rc == PM_E_EXCEED_ARP_LIMIT_MAX_NUM)
        {
            if (ARP_PORT_NUMBERLIMIT_PROTECT == pst_if_cp->arp_numberlimit_violate)
            {
                return ARP_ETH_SUCCESS;
            }
            else if (ARP_PORT_NUMBERLIMIT_RESTRICT == pst_if_cp->arp_numberlimit_violate || 
                     ARP_PORT_NUMBERLIMIT_ERRDISABLE == pst_if_cp->arp_numberlimit_violate)
            {
                /*modified by yejl to fix bug44547, 2017-07-05*/
                /*log the arp discard reseaon*/
                IFNAME_ETH2FULL(pst_if_cp->key.name, ifname_ext);
                sal_sprintf(buf1, "The arp is discard on %s because of arp number-limit!", ifname_ext);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf1);

                /*log the arp type: request or reply*/
                if (ARP_OP_REQUEST == sal_hton16(arp_p->ar_op))
                {
                    logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, "ARP opcode = Request");
                }
                else if (ARP_OP_REPLY== sal_hton16(arp_p->ar_op))
                {
                    logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, "ARP opcode = reply");
                }
                else
                {
                    logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, "ARP opcode = other ARP type");
                }

                /*log the sender MAC and sender IP*/
                sal_sprintf(buf2, " sender address = %02x:%02x:%02x:%02x:%02x:%02x, sender IP address = %d.%d.%d.%d", 
                                                    arp_p->ar_sha[0], arp_p->ar_sha[1], 
                                                    arp_p->ar_sha[2], arp_p->ar_sha[3],
                                                    arp_p->ar_sha[4], arp_p->ar_sha[5],
                                                    arp_p->ar_sip[0], arp_p->ar_sip[1],
                                                    arp_p->ar_sip[2], arp_p->ar_sip[3]);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf2);

                /*log the target MAC and target IP*/
                sal_sprintf(buf3, " target address = %02x:%02x:%02x:%02x:%02x:%02x, target IP address = %d.%d.%d.%d",
                                                    arp_p->ar_tha[0], arp_p->ar_tha[1], 
                                                    arp_p->ar_tha[2], arp_p->ar_tha[3],
                                                    arp_p->ar_tha[4], arp_p->ar_tha[5],
                                                    arp_p->ar_tip[0], arp_p->ar_tip[1],
                                                    arp_p->ar_tip[2], arp_p->ar_tip[3]);
                logid_sys(LOG_ARPNUMBERLIMIT_4_EXCEED, buf3);

                /*If violation mode is errdisable, shutdown the port*/
                if (ARP_PORT_NUMBERLIMIT_ERRDISABLE == pst_if_cp->arp_numberlimit_violate)
                {
                    PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_ARP_NUMBERLIMIT_OCCUR));
                }
                return ARP_ETH_SUCCESS;
            }
        }
        else if (rc < 0)
        {
            ARP_PKT_DEBUG("Request ARP packet discarded on %s for arp_db_entry_create fail",
                           IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return ARP_ETH_ERR_FAIL;
        }
        else if (NULL == ae_p)
        {
            ARP_PKT_DEBUG("Request ARP packet discarded on %s for arp_db_entry_create fail",
                          IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return ARP_ETH_ERR_FAIL;
        }
    }

    ae_p->is_gratuitous = FALSE;
    ae_p->rx_buf = buf;

    /* 4. run SM */
    arp_sm(ae_p, AE_RX_ARP_REQ);
    return ARP_ETH_SUCCESS;
}

static int32
_arp_ether_rep_rx(tbl_interface_t *p_db_if, uint8 *buf, arp_eth_t *arp_p)
{
    addr_ipv4_t ip;
    tbl_arp_t *ae_p = NULL;
    char       ifname_ext[IFNAME_EXT_SIZE]; 
    int32 rc = ARP_ETH_SUCCESS;

    arp_eth_pkt_move_pld(buf, arp_p);

    /* 1. validate arp reply */
    PM_E_RETURN(arp_ether_rep_validate(p_db_if, arp_p));

    /* 2. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_REV);

    /* 3. lookup ARP */
    sal_memcpy(&ip.s_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    ae_p = arp_db_entry_lookup(&ip);
    if (NULL == ae_p)
    {
        /* this arp reply is not response for me */
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s for the arp reply is not response for me",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }
    else if (ARP_TYPE_STATIC == ae_p->type)
    {
        /* not response to static arp */
        ARP_PKT_DEBUG("Reply ARP packet discarded on %s for the arp reply is not response to related static arp",
                      IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return ARP_ETH_ERR_FAIL;
    }

    ae_p->is_gratuitous = FALSE;
    ae_p->rx_buf = buf;

    /* 4. run SM */
    arp_sm(ae_p, AE_RX_ARP_RSP);
    return ARP_ETH_SUCCESS;
}

int32
_arp_ether_get_sha(tbl_interface_t *p_db_if, tbl_arp_t *ae_r, arp_eth_t *arp_p, uint8 *ip_addr, 
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    bool is_req)
    /* liwh end */
{
    int32 rc = ARP_ETH_SUCCESS;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    uint8 vmac[MAC_ADDR_LEN];
    /* liwh end */

    sal_memset(vmac, 0, MAC_ADDR_LEN);

    if ((!is_req) && (p_db_if))
    {
        /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
        rc = vrrp_get_master_router_mac(p_db_if, (uint32*)arp_p->ar_sip, vmac);
        switch(rc)
        {
        case GLB_VRRP_MAC_VALID:
            sal_memcpy(arp_p->ar_sha, vmac, MAC_ADDR_LEN);
            break;
            
        case GLB_VRRP_MAC_INVALID:
            /* not send gratuious/request/response for non-master vrrp */
            return ARP_ETH_VRRP_MAC_INVALID;
            
        case GLB_VRRP_MAC_INTERFACE:
        /* liwh end */
        default:
            sal_memcpy(arp_p->ar_sha, ae_r->mac_addr, MAC_ADDR_LEN);
            break;    
        }
    }
    else
    {
        sal_memcpy(arp_p->ar_sha, ae_r->mac_addr, MAC_ADDR_LEN);
    }

    return ARP_ETH_SUCCESS;
}

static int32
_arp_ether_gra_tx(tbl_interface_t *p_db_if, tbl_arp_t *ae_p, tbl_arp_t *ae_r, arp_pkt_type_t type, arp_pkt_op_t op)
{
    uint16 usArpProtoType = sal_hton16(ETH_P_ARP);
    vid_t vid = GLB_ALL_VLAN_ID;
    uint8 buf[256];
    uint8 send[ARP_TOTAL_PDU_LEN];
    uint8 *ptmp = send;
    uint8 *macda = NULL;
    uint8 *macsa = NULL;
    arp_eth_t *arp_p = (arp_eth_t*)buf;
    arp_eth_t *arp_src = NULL;
    int32 rc = ARP_ETH_SUCCESS;
    
    if (ARP_OP_REPLY == op)
    {
        SAL_ASSERT(ae_p->rx_buf);
        arp_eth_pkt_move_pld(ae_p->rx_buf, arp_src); 
    }

    /* 1. fill the ARP header */
    ARP_ETH_BUILD_HDR(arp_p, op);
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       this is arp req, needn't get vrrp vmac*/
    rc = _arp_ether_get_sha(p_db_if, ae_r, arp_p, NULL, TRUE);
    if (rc < 0)
    {
        return (ARP_ETH_VRRP_MAC_INVALID == rc) ? ARP_ETH_SUCCESS : rc;
    }

    arp_p->ar_op = sal_hton16(op);
    
    sal_memcpy(arp_p->ar_sip, &(ae_r->key.ip), IPV4_MAX_BYTELEN);

    /* 2. get macda/macsa */
    if (ARP_UNICAST == type && arp_src != NULL)/* unicast gra */        
    {
        macda = arp_src->ar_sha;
    }
    else
    {
        macda = arp_bcast_mac_address;
    }
    macsa = arp_p->ar_sha;
    sal_memcpy(arp_p->ar_tha, macda, MAC_ADDR_LEN);
    sal_memcpy(arp_p->ar_tip, arp_p->ar_sip, IPV4_MAX_BYTELEN);

    /* 3. build l2 header */
    ARP_ETH_BUILD_L2_HDR(ptmp, macda, macsa, usArpProtoType);
    sal_memcpy(ptmp, arp_p, (sizeof(arp_eth_t)));

    /* 4. do send */
    vid = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
    rc = ipc_pkt_tx_arp(send, ARP_TOTAL_PDU_LEN, p_db_if->ifindex, vid);
    
    /* 5. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_SEND);
    
    return rc;
}
    
static int32
_arp_ether_req_tx(tbl_interface_t *p_db_if, tbl_arp_t *ae_p, tbl_arp_t *ae_r, arp_pkt_type_t type)
{
    uint16 usArpProtoType = sal_hton16(ETH_P_ARP);
    vid_t vid = GLB_ALL_VLAN_ID;
    uint8 buf[256];
    uint8 send[ARP_TOTAL_PDU_LEN];
    uint8 *ptmp = send;
    uint8 *macda = NULL;
    uint8 *macsa = NULL;
    arp_eth_t *arp_p = (arp_eth_t*)buf;
    int32 rc = ARP_ETH_SUCCESS;

    /* 1. fill the ARP header */
    ARP_ETH_BUILD_HDR(arp_p, ARP_OP_REQUEST);
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       this is arp req, needn't get vrrp vmac*/
    rc = _arp_ether_get_sha(p_db_if, ae_r, arp_p, NULL, TRUE);
    if (rc < 0)
    {
        return (ARP_ETH_VRRP_MAC_INVALID == rc) ? ARP_ETH_SUCCESS : rc;
    }
    
    sal_memcpy(arp_p->ar_sip, &(ae_r->key.ip), IPV4_MAX_BYTELEN);
    sal_memcpy(arp_p->ar_tip, &(ae_p->key.ip), IPV4_MAX_BYTELEN);
    sal_memset(arp_p->ar_tha, 0, MAC_ADDR_LEN);

    /* 2. get macda/macsa */
    if (ARP_UNICAST == type)
    {
        macda = ae_p->mac_addr;
    }
    else
    {
        macda = arp_bcast_mac_address;
    }
    macsa = arp_p->ar_sha;

    /* 3. build L2 and ARP header */
    ARP_ETH_BUILD_L2_HDR(ptmp, macda, macsa, usArpProtoType);
    sal_memcpy(ptmp, arp_p, (sizeof(arp_eth_t)));

    /* 4. do send */
    vid = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
    
    rc = ipc_pkt_tx_arp(send, ARP_TOTAL_PDU_LEN, p_db_if->ifindex, vid);
    
    /* 5. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_SEND);

    return rc;
}

static int32
_arp_ether_rep_tx(tbl_interface_t *p_db_if, tbl_arp_t *ae_p, tbl_arp_t *ae_r)
{
    uint16 usArpProtoType = sal_hton16(ETH_P_ARP);
    vid_t vid = GLB_ALL_VLAN_ID;
    uint8 buf[256];
    uint8 send[ARP_TOTAL_PDU_LEN];
    uint8 *ptmp = send;
    uint8 *macda = NULL;
    uint8 *macsa = NULL;
    arp_eth_t *arp_p = (arp_eth_t*)buf;
    int32 rc = ARP_ETH_SUCCESS;
    uint8 ip_addr[IPV4_MAX_BYTELEN];
    
    SAL_ASSERT(ae_p->rx_buf);
    arp_eth_pkt_move_pld(ae_p->rx_buf, arp_p);

    /* 1. exchange the sender and target */
    sal_memcpy(arp_p->ar_tha, arp_p->ar_sha, MAC_ADDR_LEN);
    sal_memcpy(ip_addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    sal_memcpy(arp_p->ar_sip, arp_p->ar_tip, IPV4_MAX_BYTELEN);
    sal_memcpy(arp_p->ar_tip, ip_addr, IPV4_MAX_BYTELEN);

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 
       this is arp response, should get vrrp vmac*/
    rc = _arp_ether_get_sha(p_db_if, ae_r, arp_p, NULL, FALSE);
    if (rc < 0)
    {
        return (ARP_ETH_VRRP_MAC_INVALID == rc) ? ARP_ETH_SUCCESS : rc;
    }
    arp_p->ar_op = sal_hton16(ARP_OP_REPLY);
    
    /* 2. get macda/macsa */
    macda = arp_p->ar_tha;
    macsa = arp_p->ar_sha;

    /* 3. build L2 and ARP header */
    ARP_ETH_BUILD_L2_HDR(ptmp, macda, macsa, usArpProtoType);
    sal_memcpy(ptmp, arp_p, (sizeof(arp_eth_t)));

    /* 4. do send */
    vid = tbl_interface_get_vid_by_ifindex(p_db_if->ifindex);
    rc = ipc_pkt_tx_arp(send, ARP_TOTAL_PDU_LEN, p_db_if->ifindex, vid);

    /* 5. debug print */
    arp_debug_pkt_trace(p_db_if->key.name, arp_p, ARP_ACT_SEND);

    return rc;
}

int32
arp_ether_packet_tx(tbl_arp_t *ae_p, arp_pkt_op_t op, arp_pkt_type_t type, uint8 is_gra)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    tbl_interface_t     *p_db_if    = NULL;
    tbl_interface_t     *pst_if     = NULL;    
    ds_connected_t      *p_conn     = NULL;
    tbl_route_if_t      *p_rtif     = NULL;
    tbl_arp_t           *ae_router  = NULL;
    tbl_arp_t           arp_router;
    int32               rc = ARP_ETH_SUCCESS;
    tbl_interface_t *pst_if_cp = NULL;

    p_db_if = tbl_interface_get_interface_by_ifindex(ae_p->ifindex);
    if (NULL == p_db_if)
    {
        return ARP_ETH_ERR_FAIL;
    }

    pst_if_cp = p_db_if;
    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        p_rtif = arp_api_check_addr_with_rt_if_exist_advance(&ae_p->key.ip);
        if (NULL == p_rtif)
        {
            return ARP_ETH_ERR_PTR;
        }
        
        pst_if = tbl_interface_get_interface_by_name(p_rtif->key.name);
        if (NULL != pst_if && GLB_IF_TYPE_VLAN_IF == pst_if->hw_type)
        {
            pst_if_cp = pst_if;
        }
    }

    if (NULL == pst_if_cp)
    {
        return ARP_ETH_ERR_FAIL;
    }
    
    /* because ifc has not installed before it is up */
    if (TRUE == ae_p->is_router)
    {
        ae_router = ae_p;
    }
    else
    {
        /* match the interface address */
        rc = arp_api_get_rt_ifc(pst_if_cp, &ae_p->key.ip, &p_conn);
        if (rc || NULL == p_conn)
        {
            //log error
            return ARP_ETH_ERR_PTR;
        }
    
        /* lookup the router arp */
        ae_router = arp_db_entry_lookup(&p_conn->key.address.u.prefix4);
    }

    if(NULL == ae_router || FALSE == ae_router->is_router)
    {
        rc = arp_api_get_rt_ifc(pst_if_cp, &ae_p->key.ip, &p_conn);
        if (rc || NULL == p_conn)
        {
            //log error
            return ARP_ETH_ERR_PTR;
        }
        
        sal_memcpy(arp_router.mac_addr, pst_if_cp->mac_addr, MAC_ADDR_LEN);
        sal_memcpy(&arp_router.key.ip, &p_conn->key.address.u.prefix4, sizeof(addr_ipv4_t));
        ae_router = &arp_router;
    }
    
    if (is_gra)
    {   
        rc = _arp_ether_gra_tx(pst_if_cp, ae_p, ae_router, type, op);        
    }
    else
    {
        switch(op)
        {
        case ARP_OP_REQUEST:
            /* send arp request */
            rc = _arp_ether_req_tx(pst_if_cp, ae_p, ae_router, type);
            break;
            
        case ARP_OP_REPLY:
            /* send arp reply */
            rc = _arp_ether_rep_tx(pst_if_cp, ae_p, ae_router);
            break;

        default:
            return ARP_ETH_ERR_NOT_SUP;
        }
    }

    if (0 == rc)
    {
        p_rt_glb->arp_pkt_tx_count++;
        //tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_TX_COUNT);
    }
    
    return rc;
}

int32
arpsnooping_send_pkt(tbl_interface_t *p_db_if, arp_eth_t *arp_p, eth_hdr_t *eth_p, uint32 vlanid)
{
    tbl_vlan_t          *p_db_vlan  = NULL;
    tbl_interface_t     *p_mem_if   = NULL;
    tbl_interface_t     *pst_if_lag = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_key_t      vlan_key;
    uint32              ifindex     = 0;
    uint16              usArpProtoType = sal_hton16(ETH_P_ARP);
    uint16              usVlanTagType = sal_hton16(GLB_DEFAULT_TPID);
    uint8               send[ARP_TOTAL_PDU_LEN];
    uint8               *ptmp = send;
    int32               rc = 0;
    uint32             pkt_len = 0;
    
    sal_memset(&vlan_key, 0, sizeof(vlan_key));
    vlan_key.vid = vlanid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return ARP_ETH_PKT_INVALID;
    }
    sal_memset(&if_key, 0, sizeof(if_key));
    GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
    {
        /*added by yejl to fix bug 44132, 2017-05-24*/
        ptmp = send;
        sal_memset(ptmp, 0, sizeof(send));
        /*ended by yejl*/
        if (ifindex == p_db_if->ifindex)
        {
            continue;
        }
        p_mem_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_mem_if)
        {
            continue;   
        }
        if (!tbl_interface_is_running(p_mem_if))
        {
            continue;
        }

        if (GLB_IF_MODE_L2 == p_mem_if->mode && INTERFACE_IS_LAG_MEMBER(p_mem_if))
        {
            sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_mem_if->lag->lag_id);
            pst_if_lag = tbl_interface_get_interface(&if_key);
            if (NULL == pst_if_lag)
            {
                return 0;
            }

            if (pst_if_lag->ifindex == p_db_if->ifindex)
            {
                continue;
            }

            if (!tbl_interface_is_running(pst_if_lag))
            {
                continue;
            }
            ifindex =  pst_if_lag->ifindex;
        }

        if (vlanid == 0)
        {
            pkt_len = ARP_TOTAL_PDU_LEN;
            ARP_ETH_BUILD_L2_HDR(ptmp, eth_p->dest_mac, eth_p->src_mac, usArpProtoType);
        }
        else
        {
            int32 add_vlan_tag = FALSE;

            /*access port*/
            if (GLB_VLAN_PORT_TYPE_ACCESS == p_mem_if->vlan_type)
            {
                add_vlan_tag = FALSE;
            }
            else if (GLB_VLAN_PORT_TYPE_TRUNK == p_mem_if->vlan_type)
            {
                /*trunk port*/
                if (p_mem_if->brgif->untagged_vlan.vlan_bmp == 0)
                {
                    if (p_mem_if->brgif->pvid == vlanid)
                    {
                        add_vlan_tag = FALSE;
                    }
                    else
                    {
                        if (GLB_BMP_ISSET(p_mem_if->brgif->allowed_vlan.vlan_bmp, vlanid))
                        {
                            add_vlan_tag = TRUE;
                        }
                    }
                }
                /*hybrid port*/
                else
                {
                    if (GLB_BMP_ISSET(p_mem_if->brgif->untagged_vlan.vlan_bmp, vlanid))
                    {
                        add_vlan_tag = FALSE;
                    }
                    else
                    {
                        add_vlan_tag = TRUE;
                    }
                }
            }
            
            if (add_vlan_tag)
            {
                uint16 vlan_cos = 0;
                uint16 vlan = 0;
                uint8 cos = 0;

                vlan = vlanid & 0xfff;
                vlan_cos = cos << 13 | vlan;
                vlan_cos = sal_hton16(vlan_cos);

                pkt_len = ARP_TOTAL_PDU_LEN + 4;
        
                ARP_ETH_BUILD_L2_HDR(ptmp, eth_p->dest_mac, eth_p->src_mac, usVlanTagType);
                ARP_ETH_BUILD_VLAN_TAG(ptmp, vlan_cos, usArpProtoType);
            }
            else
            {
                pkt_len = ARP_TOTAL_PDU_LEN;
                ARP_ETH_BUILD_L2_HDR(ptmp, eth_p->dest_mac, eth_p->src_mac, usArpProtoType);
            }
        }

        sal_memcpy(ptmp, arp_p, (sizeof(arp_eth_t)));
        rc = ipc_pkt_tx_arp(send, pkt_len, ifindex, vlanid); 
    }
    GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);

    return rc;
}

static int32
_arp_ether_packet_rx(int32 sock, ctc_msg_t *p_msg)
{
    tbl_interface_t *p_db_if    = NULL;
    tbl_interface_t *pst_if     = NULL;
    arp_eth_t       *arp_p      = NULL;
    eth_hdr_t       *eth_p      = NULL;
    tbl_route_if_t  *p_rtif     = NULL;
    uint8           *buf        = NULL;
    addr_ipv4_t     addr;
    uint32          macsa_is_router = FALSE;
    uint32          macda_is_router = FALSE;
    uint32          is_gra_arp      = FALSE;
    uint32          vlanid_pkt      = 0;
    uint32          vlanid_rtif     = 0;
    uint32          ifindex         = 0;
    tbl_macfilter_key_t macflt_key;
    tbl_macfilter_t *p_macflt       = NULL;
    int32           rc              = ARP_ETH_SUCCESS;
    int32           arpinsp_ret     = 0;
    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    uint8           vmac[MAC_ADDR_LEN];
    uint8           vmac_tmp[MAC_ADDR_LEN];
    /* liwh end */

    /* support vrrp modified by liwh for bug 45215, 2017-09-17 */
    sal_memset(vmac, 0, MAC_ADDR_LEN);
    sal_memset(vmac_tmp, 0, MAC_ADDR_LEN);
    /* liwh end */
    
    ifindex = p_msg->u_hdr.pkt.ifindex;
    vlanid_pkt = (uint32)p_msg->u_hdr.pkt.vid;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);    
    if (NULL == p_db_if)
    {
        ARP_PKT_DEBUG("ARP packet discarded for no related interface with port received");
        return ARP_ETH_ERR_FAIL;
    }

    buf = p_msg->data;
    eth_p = (eth_hdr_t*)buf;
    arp_eth_pkt_move_pld(buf, arp_p);

    /*arp rate limit entry*/
    rc = arpratelimit_packet_process(vlanid_pkt, p_db_if);
    if (PM_E_NONE != rc)
    {
        return PM_E_ARP_RATE_LIMIT_ERR;
    }
    
    /* ARP packet basic validate */
    if (arp_p->ar_hln != MAC_ADDR_LEN ||
        arp_p->ar_pln != IPV4_MAX_BYTELEN ||
        arp_p->ar_hrd != sal_hton16(ARP_HRD_ETHER) ||
        arp_p->ar_pro != sal_hton16(ETH_P_IP))
    {
        ARP_PKT_DEBUG("ARP packet discarded for fail of basic validate");
        return ARP_ETH_ERR_FAIL;
    }

    /*start ARP inspection check */
    arpinsp_ret = arpinsp_valid_check(p_db_if, arp_p, eth_p, vlanid_pkt);
    switch (arpinsp_ret)
    {
    case ARPINSP_SUCCESS:
        /*if check success, then send to other interface*/
        rc = arpsnooping_send_pkt(p_db_if, arp_p, eth_p, vlanid_pkt);
        if (rc < 0)
        {
            ARPINSP_EVENT_DEBUG("send ARP snooping packet failed");
        }
        break;
        
    case ARPINSP_NO_CHECK:
        break;
        
     default: /* Check fail */
        return ARP_ETH_ERR_FAIL;
    }
    /* end ARP inspection check */
    
    /* Get l3 interface */
    tbl_interface_t *pst_if_cp = p_db_if;
    sal_memset(&addr, 0, sizeof(addr));
    sal_memcpy((char*)&addr, arp_p->ar_sip, IPV4_MAX_BYTELEN);
    if (GLB_IF_MODE_L2 == p_db_if->mode && p_db_if->brgif)
    {
        p_rtif = arp_api_check_addr_with_rt_if_exist_advance(&addr);
        if (NULL == p_rtif)
        {
            ARP_PKT_DEBUG("ARP packet discarded in case of no related l3 interface");
            return ARP_ETH_ERR_PTR;
        }
        
        pst_if = tbl_interface_get_interface_by_name(p_rtif->key.name);
        if (NULL != pst_if && GLB_IF_TYPE_VLAN_IF == pst_if->hw_type)
        {
            sal_sscanf(pst_if->key.name, "vlan%u", &vlanid_rtif);
            if (vlanid_pkt && vlanid_rtif != vlanid_pkt)
            {
                ARP_PKT_DEBUG("ARP packet discarded in case of no related l3 interface");
                return ARP_ETH_ERR_PTR;
            }
            
            pst_if_cp = pst_if;
        }
    }

    if (GLB_IF_MODE_L3 == p_db_if->mode && (p_db_if->lacp || p_db_if->lag))
    {
        tbl_interface_t     *pst_if_lag    = NULL;
        tbl_interface_key_t if_key;
        sal_memset(&if_key, 0x00, sizeof(if_key));
        
        sal_snprintf(if_key.name, IFNAME_SIZE, "agg%u", p_db_if->lag->lag_id);
        pst_if_lag = tbl_interface_get_interface(&if_key);
        if (NULL == pst_if_lag)
        {
            ARP_PKT_DEBUG("ARP packet discarded in case of no related l3 interface");
            return ARP_ETH_ERR_PTR;
        }

        pst_if_cp = pst_if_lag;
    }

    if (NULL == pst_if_cp)
    {
        ARP_PKT_DEBUG("ARP packet discarded in case of no related l3 interface");
        return ARP_ETH_ERR_FAIL;
    }

    if (p_db_if)
    {
        /* support vrrp modified by liwh for bug 45215, 2017-09-17 
           check arp dest mac or src mac is vmac */
        rc = vrrp_get_master_router_mac(p_db_if, (uint32*)arp_p->ar_tip, vmac);
        if (rc == GLB_VRRP_MAC_VALID)
        {
            /* check src mac is vmac */
            sal_memcpy(vmac_tmp, eth_p->dest_mac, MAC_ADDR_LEN);
            macda_is_router = (sal_memcmp(vmac_tmp, vmac, MAC_ADDR_LEN) == 0);

            /* check dest mac is vmac */
            sal_memcpy(vmac_tmp, eth_p->src_mac, MAC_ADDR_LEN);
            macsa_is_router = (sal_memcmp(vmac_tmp, vmac, MAC_ADDR_LEN) == 0);
            sal_memcpy(macflt_key.mac, vmac_tmp, MAC_ADDR_LEN);
        }
        else
        {
            macda_is_router = (sal_memcmp(eth_p->dest_mac, pst_if_cp->mac_addr, MAC_ADDR_LEN) == 0);
            macsa_is_router = (sal_memcmp(eth_p->src_mac, pst_if_cp->mac_addr, MAC_ADDR_LEN) == 0);
            sal_memcpy(macflt_key.mac, eth_p->src_mac, MAC_ADDR_LEN);
        }
        /* liwh end */
    }

    /* added by chengw for bug id 15351 2011-07-06 */
    /* Not process in case of no l3 interface */
    if (NULL == p_db_if)
    {
        ARP_PKT_DEBUG("ARP packet discarded in case of no related l3 interface");
        return ARP_ETH_ERR_FAIL;
    }
    
    if (!tbl_interface_is_running(pst_if_cp))
    {
        ARP_PKT_DEBUG("ARP packet discarded for the interface strate is not up and running");
        return ARP_ETH_ERR_FAIL;
    }

    /* Accept all broadcast */
    if (FALSE == macda_is_router && !(GLB_IS_BCAST_MAC(eth_p->dest_mac)))
    {
        ARP_PKT_DEBUG("ARP packet discarded for the dest MAC differing from router MAC");
        return ARP_ETH_ERR_FAIL;
    }

    /* mac sa is ourselves, discasd it for loop */
    if (TRUE == macsa_is_router)
    {
        ARP_PKT_DEBUG("ARP packet discarded for the source MAC same with router MAC");
        return ARP_ETH_ERR_FAIL;
    }

    p_macflt = tbl_macfilter_get_macfilter(&macflt_key);
    if (p_macflt)
    {
        ARP_PKT_DEBUG("ARP packet discarded for the source MAC exist in MAC filter");
        return ARP_ETH_MACFILTER;
    }

    /* judge for gratuitous arp */
    if (0 == sal_memcmp(arp_p->ar_sip, arp_p->ar_tip, IPV4_MAX_BYTELEN))
    {
        is_gra_arp = TRUE;
    }
    
    if (is_gra_arp)
    {
        /* receive gratuitous arp */
        rc = _arp_ether_gra_rx(pst_if_cp, buf, arp_p);
    }
    else
    {
        switch (sal_hton16(arp_p->ar_op))
        {
        case ARP_OP_REQUEST:
            /* receive arp request */
            rc = _arp_ether_req_rx(pst_if_cp, buf, arp_p);
            break;
        
        case ARP_OP_REPLY:
            /* receive arp reply */
            rc = _arp_ether_rep_rx(pst_if_cp, buf, arp_p);
            break;
        
            /* ignore other arp pkt */
        default:
            break;    
        }
    }

    return rc;
}

int32
arp_ether_packet_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_route_global_t *p_rt_glb = tbl_route_global_get_route_global();
    int32 rc = ARP_ETH_SUCCESS;

    rc = _arp_ether_packet_rx(sock, p_msg);
    if (0 == rc)
    {
        p_rt_glb->arp_pkt_rx_count++;
        //tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_RX_COUNT);
    }
    else
    {
        p_rt_glb->arp_pkt_discard_count++;
        //ARP_LOG_USER(E_DEBUG, ARP_4_DISCARD_PACKET, LOG_RATE_LIMIT_TYPE_ARP_DISCARD_PKT, p_rt_glb->arp_pkt_discard_count);
        ARP_PKT_DEBUG("ARP packet has been discard, there are %d packets discarded in total", p_rt_glb->arp_pkt_discard_count);
        //tbl_route_global_set_route_global_field(p_rt_glb, TBL_ROUTE_GLOBAL_FLD_ARP_PKT_DISCARD_COUNT);
    }
    
    return PM_E_NONE;
}

int32
arp_ether_lladdr_update(tbl_arp_t *ae_p, uint8 *mac)
{
    /* 1. check duplicate */
    if (ae_p->is_router || 0 == sal_memcmp(ae_p->mac_addr, mac, MAC_ADDR_LEN))
    {
        return PM_E_NONE;
    }

    /* 2. remove old MAC from HW */
    arp_db_synch_hw_os(ae_p, ARP_SYN_DEL);

    /* 3. update DB */
    sal_memcpy(ae_p->mac_addr, mac, MAC_ADDR_LEN);
    
    /* 4. add updated MAC to HW */
    arp_db_synch_hw_os(ae_p, ARP_SYN_ADD);

    /* 5. update flags */
    GLB_UNSET_FLAG(ae_p->flags, ARP_SYN_FDB_AGE);

    /* log debug */
    arp_debug_entry_trace(ae_p, ARP_ENTRY_UP);

    return ARP_ETH_SUCCESS;
}

