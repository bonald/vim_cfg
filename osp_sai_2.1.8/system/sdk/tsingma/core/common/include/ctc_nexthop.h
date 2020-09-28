/**
 @file ctc_nexthop.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-11

 @version v2.0

 This file contains all Nexthop related data structure, enum, macro and proto.
*/

#ifndef _CTC_NEXTHOP_H
#define _CTC_NEXTHOP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_port.h"
#include "common/include/ctc_vlan.h"
#include "common/include/ctc_stats.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_MPLS_NH_MAX_PUSH_LABEL_NUM     4 /**< [GB.GG.D2.TM] The maximum of MPLS lable can be pushed */
#define CTC_MPLS_NH_MAX_TUNNEL_LABEL_NUM   12 /**< [GB.GG.D2.TM] The maximum of MPLS lable can be pushed */
#define CTC_MPLS_NH_MAX_FLEX_LABEL_NUM   2 /**< [D2.TM] The maximum of MPLS lable can be pushed for flex edit*/
#define CTC_NH_DEFAULT_MAX_EXTERNAL_NEXTHOP_NUM  16384
#define CTC_NH_DEFAULT_MAX_MPLS_TUNNEL_NUM  1024

#define CTC_NH_UDF_MAX_SIZE  4
#define CTC_NH_UDF_MAX_EDIT_NUM  4

/****************************************************************
*
* Data Structures
*
****************************************************************/

/**
 @defgroup nexthop NEXTHOP
 @{
*/

/**
 @brief  Define Global reserved nexthop ID
*/
enum ctc_nh_reserved_nhid_e
{
    CTC_NH_RESERVED_NHID_FOR_NONE   = 0,    /**< [GB.GG.D2.TM] Global reserved nexthop Id for none */
    CTC_NH_RESERVED_NHID_FOR_DROP   = 1,    /**< [GB.GG.D2.TM] Global reserved nexthop Id for drop */
    CTC_NH_RESERVED_NHID_FOR_TOCPU  = 2,    /**< [GB.GG.D2.TM] Global reserved nexthop Id for cpu */
    CTC_NH_RESERVED_NHID_MAX        = 3
};
typedef enum ctc_nh_reserved_nhid_e ctc_nh_reserved_nhid_t;

enum ctc_nh_reserved_dsnh_offset_e
{
    CTC_NH_RESERVED_DSNH_OFFSET_FOR_RSPAN   = 0,    /**< [GB.GG.D2.TM] Global reserved nexthop offset for rspan */

    CTC_NH_RESERVED_DSNH_OFFSET_MAX       = 1
};
typedef enum ctc_nh_reserved_dsnh_offset_e ctc_nh_reserved_dsnh_offset_t;

enum ctc_nh_reserved_dsnh_offset_type_e
{
    CTC_NH_RES_DSNH_OFFSET_TYPE_BRIDGE_NH = 0,   /**< [GB.GG.D2.TM] Reserved L2Uc Nexthop */
    CTC_NH_RES_DSNH_OFFSET_TYPE_BYPASS_NH,       /**< [GB.GG.D2.TM] Reserved bypass Nexthop */
    CTC_NH_RES_DSNH_OFFSET_TYPE_MIRROR_NH,       /**< [GB.GG.D2.TM] Reserved mirror Nexthop*/
    CTC_NH_RES_DSNH_OFFSET_TYPE_MAX
};
typedef enum ctc_nh_reserved_dsnh_offset_type_e ctc_nh_reserved_dsnh_offset_type_t;

/**
 @brief  Define Nexthop type
*/
enum ctc_nh_type_e
{
    CTC_NH_TYPE_NULL,   /**< [GB.GG.D2.TM] Invaid Nexthop Type*/
    CTC_NH_TYPE_MCAST,  /**< [GB.GG.D2.TM] Mcast Nexthop Type*/
    CTC_NH_TYPE_L2UC,   /**< [GB.GG.D2.TM] L2 Ucast Nexthop Type*/
    CTC_NH_TYPE_XLATE,  /**< [GB.GG.D2.TM] Xlate Nexthop Type*/
    CTC_NH_TYPE_IPUC,   /**< [GB.GG.D2.TM] IPUC Nexthop Type*/
    CTC_NH_TYPE_MPLS,   /**< [GB.GG.D2.TM] MPLS Nexthop Type*/
    CTC_NH_TYPE_ECMP,   /**< [GB.GG.D2.TM] ECMP Nexthop Type*/
    CTC_NH_TYPE_ILOOP,  /**< [GB.GG.D2.TM] ILOOP Nexthop Type*/
    CTC_NH_TYPE_RSPAN,  /**< [GB.GG.D2.TM] RSPAN Nexthop Type*/
    CTC_NH_TYPE_IP_TUNNEL, /**< [GB.GG.D2.TM] IP Tunnel Nexthop Type*/
    CTC_NH_TYPE_TRILL,  /**< [GB.GG.D2.TM] Trill Nexthop Type*/
    CTC_NH_TYPE_MISC,   /**< [GB.GG.D2.TM] Misc Nexthop Type*/
    CTC_NH_TYPE_WLAN,   /**< [D2.TM] Misc Nexthop Type*/
    CTC_NH_TYPE_MAX
};
typedef enum ctc_nh_type_e ctc_nh_type_t;

/**
 @brief  Define Nexthop outgoing interface type ,pls refer to ctc_l3if_type_t
*/
enum ctc_nh_oif_type_e
{

    CTC_NH_OIF_TYPE_ROUTED_PORT,    /**< [GB.D2.TM] Outgoing interface is routed port*/
    CTC_NH_OIF_TYPE_SUB_IF,         /**< [GB.D2.TM] Outgoing interface is sub-interface*/
    CTC_NH_OIF_TYPE_VLAN_PORT,      /**< [GB.D2.TM] Outgoing interface is vlan port */
    CTC_NH_OIF_TYPE_MAX
};
typedef enum ctc_nh_oif_type_e ctc_nh_oif_type_t;
/**
 @brief  Define Nexthop outgoing interface information structure
*/
struct ctc_nh_oif_info_s
{
    uint32 gport;             /**< [GB.GG.D2.TM] Outgoing global port */
    uint16   vid;             /**< [GB.GG.D2.TM] Outgoing vlan id*/
    uint16   ecmp_if_id;      /**< [GG.D2.TM] if not equal 0, means outgoing interface is ecmp interface */
	uint8    is_l2_port;      /**< [GB.GG.D2.TM] if set,indicate oif is L2 port,and only do bridging*/
	uint8    rsv[3];
};
typedef struct ctc_nh_oif_info_s ctc_nh_oif_info_t;

/**
 @brief  Define ECMP update type
*/
enum ctc_nh_ecmp_update_type_e
{
    CTC_NH_ECMP_ADD_MEMBER,        /**< [GB.GG.D2.TM] Add a member into ECMP group */
    CTC_NH_ECMP_REMOVE_MEMBER,     /**< [GB.GG.D2.TM] Remove a member from ECMP group */

    CTC_NH_ECMP_UPDATE_OP_MAX
};
typedef enum ctc_nh_ecmp_update_type_e ctc_nh_ecmp_update_type_t;

/**
 @brief  Define nexthop edit mode
*/
enum ctc_nh_edit_mode_e
{
    CTC_NH_EDIT_MODE_SINGLE_CHIP = 0,  /**< [GB.GG.D2.TM] All Nexthop's dsnh_offset will be allocated in SDK */
    CTC_NH_EDIT_MODE_MULTI_CHIP  = 1,  /**< [GB.GG.D2.TM]  Nexthop's dsnh_offset will be allocated in high level
                                                      application except IPUC/Mcast/To CPU/Drop Nexthop */

    CTC_NH_CHIP_EDIT_MODEMAX
};
typedef enum ctc_nh_edit_mode_e ctc_nh_edit_mode_t;

/**
 @brief  Define Nexthop outgoing interface type flag
*/
struct ctc_nh_global_cfg_s
{
    uint32 max_external_nhid;               /**< [GB.GG.D2.TM] max nhid used for external nexthop  */

    uint32 acl_redirect_fwd_ptr_num;        /**< [GB.GG.D2.TM] The number of ds_fwd_ptr reserved for acl redirect function,only humber have the limition */
    uint32 nh_edit_mode;                    /**< [GB.GG.D2.TM] if it is CTC_NH_EDIT_MODE_SINGLE_CHIP, ,indicate dsnh_offset will be allocated in SDK.
                                                              else dsnh_offset will be allocated in high level application.Default value 0*/
    uint16 max_ecmp;                        /**< [GB.GG.D2.TM] the maximum ECMP paths allowed for a route */
    uint16 max_tunnel_id;                   /**< [GB.GG.D2.TM] max tunnel id used for mpls tunnel label  */
};
typedef struct ctc_nh_global_cfg_s ctc_nh_global_cfg_t;

/**
 @brief  Define Nexthop update type
*/
enum ctc_nh_upd_type_s
{
    CTC_NH_UPD_UNRSV_TO_FWD,    /**< [GB.GG.D2.TM] update to forwarding nexthop, have no meaning when use arp */
    CTC_NH_UPD_FWD_TO_UNRSV,    /**< [GB.GG.D2.TM] update to unresoved nexthop, have no meaning when use arp */
    CTC_NH_UPD_FWD_ATTR,        /**< [GB.GG.D2.TM] updated forwarding nexthop attribute */
    MAX_CTC_NH_UPDTYPE
};
typedef enum ctc_nh_upd_type_s ctc_nh_upd_type_t;

/**
 @brief  Define ECMP Nexthop type
*/
enum ctc_nh_ecmp_type_s
{
    CTC_NH_ECMP_TYPE_STATIC,        /**< [GG.D2.TM] static ECMP */
    CTC_NH_ECMP_TYPE_DLB,           /**< [GG.D2.TM] ECMP dynamic load balancing modes, refer ctc_nh_ecmp_dlb_mode_t */
    CTC_NH_ECMP_TYPE_RR,            /**< [GG.D2.TM] ECMP round robin load balancing mode */
    CTC_NH_ECMP_TYPE_RANDOM_RR,     /**< [GG.D2.TM] ECMP random round robin load balancing mode */
    CTC_NH_ECMP_TYPE_XERSPAN,       /**< [GG.D2.TM] ECMP erspan used for symmetric hash */

    MAX_CTC_NH_ECMP_TYPE
};
typedef enum ctc_nh_ecmp_type_s ctc_nh_ecmp_type_t;

/**
@brief  Define ECMP create data structure
*/
struct ctc_nh_ecmp_nh_param_s
{
    uint32 nhid[CTC_MAX_ECPN]; /**< [GB.GG.D2.TM] ECMP member nh ID */
    uint8  nh_num;             /**< [GB.GG.D2.TM] Number of nh ID  */
    uint8  type;               /**< [GG.D2.TM] ECMP type, refer to ctc_nh_ecmp_type_t */
    uint16 member_num;         /**< [GB.GG.D2.TM] ECMP member number in this group, valid when ctc_nh_global_cfg_t.max_ecmp is 0 and member_num = 9~16 is regarded as 16 in GB */
    uint8  upd_type;           /**< [GB.GG.D2.TM] CTC_NH_ECMP_ADD_MEMBER or CTC_NH_ECMP_REMOVE_MEMBER */
    uint8  failover_en;        /**< [GG.D2.TM] Indicate linkdown use hw based linkup member select, only used for static or rr ecmp */
    uint32 stats_id;           /**< [GB.GG.D2.TM] Stats id*/



};
typedef struct ctc_nh_ecmp_nh_param_s ctc_nh_ecmp_nh_param_t;

/**
 @brief  Define brg unicast nexthop sub type enum
*/
enum ctc_nh_param_brguc_sub_type_e
{
    CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC               = (1 << 0), /**< [GB.GG.D2.TM] basic type nexthop*/
    CTC_NH_PARAM_BRGUC_SUB_TYPE_UNTAGGED            = (1 << 1), /**< [GB.GG.D2.TM] apply for port untagged mode */
    CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS              = (1 << 2), /**< [GB.GG.D2.TM] bypass type nexthop */
    CTC_NH_PARAM_BRGUC_SUB_TYPE_RAW_PACKET_ELOG_CPU = (1 << 3), /**< [GB.GG.D2.TM] raw pacekt type nexthop */
    CTC_NH_PARAM_BRGUC_SUB_TYPE_SERVICE_QUEUE       = (1 << 4), /**< [GB.GG.D2.TM] service queue type nexthop*/
    CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL                 =  0        /**< [GB.GG.D2.TM] all subtype  */

};
typedef enum ctc_nh_param_brguc_sub_type_e ctc_nh_param_brguc_sub_type_t;

/**
 @brief  Define nexthop arp mac sub type enum
*/
enum ctc_nh_nexthop_mac_flag_e
{
    CTC_NH_NEXTHOP_MAC_VLAN_VALID               = (1 << 0),  /**< [GB.GG.D2.TM] if set,indicate outgoing port is Phy_if,and packet need to do vlan edit.(To achieve sub interface) */
    CTC_NH_NEXTHOP_MAC_ECMP_IF_ARP              = (1 << 1),  /**< [GG.D2.TM] if set ,indicate configure ecmp interface member MACDA*/
    CTC_NH_NEXTHOP_MAC_DISCARD                  = (1 << 2),  /**< [GB.GG.D2.TM] if set, config arp miss action discard */
    CTC_NH_NEXTHOP_MAC_REDIRECT_TO_CPU          = (1 << 3),  /**< [GB.GG.D2.TM] if set, config arp miss action to CPU */
};
typedef enum ctc_nh_nexthop_mac_flag_e ctc_nh_nexthop_mac_flag_t;

/**
 @brief  Define Next hop router Mac parametes structure
*/
struct ctc_nh_nexthop_mac_param_s
{
    uint16      flag;           /**< [GB.GG.D2.TM] refer to ctc_nh_nexthop_mac_flag_t */
    mac_addr_t  mac;            /**< [GB.GG.D2.TM] Outgoing mac */
    uint16      vlan_id;        /**< [GB.GG.D2.TM] Vlan Id*/
    uint32      gport;          /**< [GG.D2.TM] GG: used to config ecmp if arp mac;
                                             D2: Outgoing port */
};
typedef struct ctc_nh_nexthop_mac_param_s ctc_nh_nexthop_mac_param_t;

/**
 @brief  Define ip nexthop flag enum
*/
enum ctc_ip_nh_flag_e
{
    CTC_IP_NH_FLAG_UNROV          = 0x00000001,        /**< [GB.GG.D2.TM] unresolved IP next hop, have no meaning when use arp */
    CTC_IP_NH_FLAG_TTL_NO_DEC     = 0x00000002,        /**< [GB.GG.D2.TM] ttl not decrease */
    CTC_IP_NH_FLAG_STRIP_MAC      = 0x00000004,        /**< [GB] strip mac and vlan */
    CTC_IP_NH_FLAG_FPMA           = 0x00000008,        /**< [GG.D2.TM] FPMA mode for FCoE MACDA */
    CTC_IP_NH_FLAG_MERGE_DSFWD    = 0x00000010,        /**< [GB.GG.D2.TM] using merge dsfwd mode */
    /* SYSTEM MODIFIED by yaom for OPENFLOW 20160227 */
    CTC_IP_NH_FLAG_USE_L2EDIT             = 0x00000020,        /**< [GG] force use l2edit to edit vlan */ 
    CTC_IP_NH_FLAG_REWRITE_ETHER_TYPE     = 0x00000040,        /**< [GG] rewrite ether type */ 
    /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-21 */
    CTC_IP_NH_FLAG_TPOAM_SECTION = 0x00000080,/*SYSTEM MODEFIED by yangl for tpoam 2019-09-21*/
    /* liwh end */
    CTC_IP_NH_FLAG_MAX
};
typedef enum ctc_ip_nh_flag_e ctc_ip_nh_flag_t;


/**
 @brief  Define ip nexthop parametes structure
*/
struct ctc_ip_nh_param_s
{
    uint32                  flag;       /**< [GB.GG.D2.TM] IP nexthop flag --> ctc_ip_nh_flag_t,CTC_IP_NH_FLAG_xxx flags*/
    ctc_nh_upd_type_t       upd_type;   /**< [GB.GG.D2.TM] nexthop update type ,used for update Operation*/

    ctc_nh_oif_info_t       oif;        /**< [GB.GG.D2.TM] Outgoing interface*/
    mac_addr_t              mac;        /**< [GB.GG.D2.TM] Outgoing mac */
    mac_addr_t              mac_sa;     /**< [GG.D2.TM] Outgoing MAC address. Value 0 means invalid */

    ctc_nh_oif_info_t    p_oif;         /**< [GB.GG.D2.TM] if aps_en set,protection outgoing interface*/
    mac_addr_t           p_mac;         /**< [GB.GG.D2.TM] if aps_en set,protection outgoing mac */
    uint32               dsnh_offset;   /**< [GB.GG.D2.TM] if Chipset use Egress chip Edit mode ,the
                                              dsnh_offset is allocated in platform adaption Layer;
                                              else the dsnh_offset is internally allocated in SDK */

    uint8   aps_en;                     /**< [GB.GG.D2.TM] if set, APS enable*/
    uint8   mtu_no_chk;                 /**< [GB.GG.D2.TM] If set , means mtu check disable */
    uint16  aps_bridge_group_id;        /**< [GB.GG.D2.TM] the APS group ID of the nexthop. */

    uint32 loop_nhid;                   /**< [GB] loopback to destination nexthop id, 0 means invalid */
    uint16 arp_id;                      /**< [GB.GG.D2.TM] working path arp id, 0 means invalid */
    uint16 p_arp_id;                    /**< [GB.GG.D2.TM] protection path arp id, 0 means invalid */
    uint16 cid;                         /**< [D2.TM] category id, 0 means disable */
    uint8  adjust_length;               /**< [GG.D2.TM] The value of adjust length */
};
typedef struct ctc_ip_nh_param_s ctc_ip_nh_param_t;

/**
 @brief  Define ip nexthop tunnel flag
*/
enum ctc_ip_nh_tunnel_flag_e
{
    CTC_IP_NH_TUNNEL_FLAG_MAP_TTL        =    (1 << 0),  /**< [GB.GG.D2.TM] If set , means new TTL mapped from (pkt_ttl - ttl)*/
    CTC_IP_NH_TUNNEL_FLAG_COPY_DONT_FRAG =    (1 << 1),  /**< [GB.GG.D2.TM] [Only for In4] If set , means new dont frag will copy payload ip dont frag);else will use user defined dont frag*/
    CTC_IP_NH_TUNNEL_FLAG_DONT_FRAG      =    (1 << 2),  /**< [GB.GG.D2.TM] [Only for In4] User defined dont frag,if set ,dont frag will set to 1;else to 0 */
    CTC_IP_NH_TUNNEL_FLAG_MTU_CHECK      =    (1 << 3),  /**< [GB.GG.D2.TM] If set , means mtu check enable(only 6to4 auto )*/
    CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY   =    (1 << 4),   /**< [GB.GG.D2.TM] If set ,means GRE header with KEY*/
    CTC_IP_NH_TUNNEL_FLAG_REROUTE_WITH_TUNNEL_HDR =  (1 << 5), /**< [GB.GG.D2.TM] If set ,means encap packet will re-route with new tunnel header; for ivi, will re-route with new ip header */
    CTC_IP_NH_TUNNEL_FLAG_6TO4_USE_MANUAL_IPSA    = (1 << 6),  /**< [GB.GG.D2.TM] If set ,means encap packet's IPSA is manual */
    CTC_IP_NH_TUNNEL_FLAG_MIRROR         = (1 << 7),      /**< [GB.GG.D2.TM] If set ,means encap packet will be applied to mirror session */
    CTC_IP_NH_TUNNEL_FLAG_IP_BFD               = (1 << 8),      /**< [GB.GG.D2.TM] If set , packet will be applied to IP BFD.
                                                           only for CTC_TUNNEL_TYPE_IPV4_IN4 & CTC_TUNNEL_TYPE_IPV4_IN6 */
    CTC_IP_NH_TUNNEL_FLAG_NAT_REPLACE_IP       = (1 << 9),      /**< [GB] [Only for Nat] If set , only ip will be replace */
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_HOVERLAY      = (1 << 10),    /**< [GG.D2.TM] If set, means the tunnel is a hierarchy overlay tunnel, it will disable split horizon and set logic port*/
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI    = (1 << 11),   /**< [GG.D2.TM] If set, means overlay tunnel is encap new VN ID */
    CTC_IP_NH_TUNNEL_FLAG_STRIP_VLAN    = (1 << 12),   /**< [GG] If set, means pakcet will strip original vlan  */
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GPE    = (1 << 13),   /**< [GG.D2.TM] If set, means overlay tunnel is VxLAN-GPE */
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_UPDATE_MACSA = (1 << 14),   /**< [GG.D2.TM] If set, means update inner MACSA for overlay tunnel*/
    CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR  = (1 << 15),   /**< [GG.D2.TM] If set ,means erspan will encapsulate extend header */
    CTC_IP_NH_TUNNEL_FLAG_XERSPN_WITH_EXT_HDR_HASH = (1 << 16),/**< [GG.D2.TM] If set ,means erspan extend header will  include symmetric hash value*/
    CTC_IP_NH_TUNNEL_FLAG_XERSPN_KEEP_IGS_TS   = (1 << 17),     /**< [GG.D2.TM] If set ,means erspan extend header timestamp is from ingress, else from egress*/
    CTC_IP_NH_TUNNEL_FLAG_VCCV_PW_BFD = (1 << 18),     /**< [GB] If set , packet will be applied to VCCV PW BFD to add IP and UDP header when do eloop.
                                                           only for CTC_TUNNEL_TYPE_IPV4_IN4 & CTC_TUNNEL_TYPE_IPV4_IN6 */
    CTC_IP_NH_TUNNEL_FLAG_LOGIC_PORT_CHECK       = (1 << 19),      /**< [D2.TM] If set , logic port check enable */
    CTC_IP_NH_TUNNEL_FLAG_INNER_TTL_NO_DEC       = (1 << 20),      /**< [GB.GG.D2.TM] If set , inner ttl not decrease */
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT    = (1 << 21),  /**< [GG.D2.TM] If set, means overlay tunnel vxlan is encap with the new l4 source port */
    CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_GBP_EN    = (1 << 22),     /**< [TM] If set, means overlay tunnel vxlan is encap with Group Based Police*/

    CTC_IP_NH_TUNNEL_FLAG_OPENFLOW_IP_PROTOCOL    = (1 << 23)    /**< SYSTEM MODIFY for hybrid v4/v6 exchange If set, means ip header ip-protocol field  */
};
typedef enum ctc_ip_nh_tunnel_flag_e ctc_ip_nh_tunnel_flag_t;


/**
 @brief  Define ip nexthop Tunnel types
*/
enum ctc_nh_ip_tunnel_type_e
{
    CTC_TUNNEL_TYPE_NONE,               /**< [GB.GG.D2.TM] NO IP tunneling. */
    CTC_TUNNEL_TYPE_IPV4_IN4,           /**< [GB.GG.D2.TM] RFC 2003/2893: IPv4-in-IPv4 tunnel. */
    CTC_TUNNEL_TYPE_IPV6_IN4,           /**< [GB.GG.D2.TM] RFC 2003/2893: IPv6-in-IPv4 tunnel. */
    CTC_TUNNEL_TYPE_IPV4_IN6,           /**< [GB.GG.D2.TM] RFC 2003/2893: IPv4-in-IPv6 tunnel. */
    CTC_TUNNEL_TYPE_IPV6_IN6,           /**< [GB.GG.D2.TM] RFC 2003/2893: IPv6-in-IPv6 tunnel. */
    CTC_TUNNEL_TYPE_GRE_IN4,            /**< [GB.GG.D2.TM] RFC 1701/2784/2890: GRE IPv4-in-IPv4  tunnel. */
    CTC_TUNNEL_TYPE_GRE_IN6,            /**< [GB.GG.D2.TM] RFC 1701/2784/2890: GRE IPv4-in-IPv6  tunnel. */
    CTC_TUNNEL_TYPE_ISATAP,             /**< [GB.GG.D2.TM] RFC 5214/5579 ,IPv6-in-IPv4 tunnel,ipv4 da copy ipv6 da(last 32bit) */
    CTC_TUNNEL_TYPE_6TO4,               /**< [GB.GG.D2.TM] RFC 3056,IPv6-in-IPv4 tunnel,ipv4 da and sa copy ipv6 da and (middle 32bit) */
    CTC_TUNNEL_TYPE_6RD,                /**< [GB.GG.D2.TM] RFC 3056,IPv6-in-IPv4 tunnel,ipv4 da and sa copy ipv6 da and (middle 32bit) */
    CTC_TUNNEL_TYPE_IPV4_NAT,           /**< [GB.GG.D2.TM] NAT nexthop. for NAT:replace ipda; for NAPT: replace ipda and l4dstport */
    CTC_TUNNEL_TYPE_IVI_6TO4,           /**< [GB.GG.D2.TM] IVI nexthop. do ipv6 address translate to ipv4 address */
    CTC_TUNNEL_TYPE_IVI_4TO6,           /**< [GB.GG.D2.TM] IVI nexthop. do ipv4 address translate to ipv6 address */
    CTC_TUNNEL_TYPE_NVGRE_IN4,          /**< [GG.D2.TM] NVGRE nexthop. do l2 overlay network encapsulation by NvGRE in ipv4*/
    CTC_TUNNEL_TYPE_VXLAN_IN4,          /**< [GG.D2.TM] VXLAN nexthop. do l2 overlay network encapsulation by Vxlan/eVxLAN in ipv4*/
    CTC_TUNNEL_TYPE_GENEVE_IN4,         /**< [GG.D2.TM] GENEVE nexthop. do l2 overlay network encapsulation by GENEVE in ipv4*/
    CTC_TUNNEL_TYPE_NVGRE_IN6,          /**< [GG.D2.TM] NVGRE nexthop. do l2 overlay network encapsulation by NvGRE in ipv6*/
    CTC_TUNNEL_TYPE_VXLAN_IN6,          /**< [GG.D2.TM] VXLAN nexthop. do l2 overlay network encapsulation by Vxlan/eVxLAN in ipv6*/
    CTC_TUNNEL_TYPE_GENEVE_IN6,         /**< [GG.D2.TM] GENEVE nexthop. do l2 overlay network encapsulation by GENEVE in ipv6*/
    MAX_CTC_TUNNEL_TYPE                 /* Must be last */
};
typedef enum ctc_nh_ip_tunnel_type_e ctc_nh_ip_tunnel_type_t;

/**
 @brief  Define IPv6 flow label mode
*/
enum ctc_nh_dscp_select_mode_e
{
    CTC_NH_DSCP_SELECT_ASSIGN,          /**< [GB.GG.D2.TM] Use user-define DSCP to outer header.*/
    CTC_NH_DSCP_SELECT_MAP,             /**< [GB.GG.D2.TM] Use DSCP value from DSCP map */
    CTC_NH_DSCP_SELECT_PACKET,          /**< [GB.GG.D2.TM] Copy packet DSCP to outer header */
    CTC_NH_DSCP_SELECT_NONE,            /**< [GG.D2.TM] Do not change DSCP */
    MAX_CTC_NH_DSCP_SELECT_MODE         /* Must be last */
};
typedef enum ctc_nh_dscp_select_mode_e ctc_nh_dscp_select_mode_t;

enum ctc_nh_ecn_select_mode_e
{
    CTC_NH_ECN_SELECT_NONE,            /**< [D2.TM] Ecn value is 0*/
    CTC_NH_ECN_SELECT_ASSIGN,          /**< [D2.TM] Use user-define ECN, only support for flex hdr edit*/
    CTC_NH_ECN_SELECT_MAP,             /**< [D2.TM] Use ECN value from ECN map */
    CTC_NH_ECN_SELECT_PACKET,          /**< [D2.TM] Copy packet ECN */
    MAX_CTC_NH_ECN_SELECT_MODE         /* Must be last */
};
typedef enum ctc_nh_ecn_select_mode_e ctc_nh_ecn_select_mode_t;


/**
 @brief  Define flow label mode
*/
enum ctc_nh_flow_label_mode_e
{
    CTC_NH_FLOW_LABEL_NONE,            /**< [GB.GG.D2.TM] Do not set flow label value */
    CTC_NH_FLOW_LABEL_WITH_HASH,       /**< [GB.GG.D2.TM] Use (user-define flow label + header hash) to outer header.*/
    CTC_NH_FLOW_LABEL_ASSIGN,          /**< [GB.GG.D2.TM] Use user-define flow label to outer header.*/
    MAX_CTC_NH_FLOW_LABEL              /* Must be last */
};
typedef enum ctc_nh_flow_label_mode_e ctc_nh_flow_label_mode_t;

/**
 @brief  Define flow label mode
*/
enum ctc_nh_sp_prefix_length_e
{
    CTC_NH_SP_PREFIX_LENGTH_16  = 0,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 16. */
    CTC_NH_SP_PREFIX_LENGTH_24  = 1,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 24. */
    CTC_NH_SP_PREFIX_LENGTH_32  = 2,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 32. */
    CTC_NH_SP_PREFIX_LENGTH_40  = 3,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 40. */
    CTC_NH_SP_PREFIX_LENGTH_48  = 4,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 48. */
    CTC_NH_SP_PREFIX_LENGTH_56  = 5,  /**< [GB.GG.D2.TM] 6rd type tunnel SP prefix length is 56. */
    MAX_CTC_NH_SP_PREFIX_LENGTH       /* Must be last */
};
typedef enum ctc_nh_sp_prefix_length_e ctc_nh_sp_prefix_length_t;

/**
 @brief  Define ivi prefix length
*/
enum ctc_nh_ivi_prefix_len_e
{
    CTC_NH_IVI_PREFIX_LEN_32         = 0, /**< [GB.GG.D2.TM] ivi prefix length is 32 */
    CTC_NH_IVI_PREFIX_LEN_40         = 1, /**< [GB.GG.D2.TM] ivi prefix length is 40 */
    CTC_NH_IVI_PREFIX_LEN_48         = 2, /**< [GB.GG.D2.TM] ivi prefix length is 48 */
    CTC_NH_IVI_PREFIX_LEN_56         = 3, /**< [GB.GG.D2.TM] ivi prefix length is 56 */
    CTC_NH_IVI_PREFIX_LEN_64         = 4, /**< [GB.GG.D2.TM] ivi prefix length is 64 */
    CTC_NH_IVI_PREFIX_LEN_72         = 5, /**< [GB.GG.D2.TM] ivi prefix length is 72 */
    CTC_NH_IVI_PREFIX_LEN_80         = 6, /**< [GB.GG.D2.TM] ivi prefix length is 80 */
    CTC_NH_IVI_PREFIX_LEN_96         = 7, /**< [GB.GG.D2.TM] ivi prefix length is 96 */

    MAX_CTC_NH_IVI_PREFIX_LEN
};
typedef enum ctc_nh_ivi_prefix_len_e ctc_nh_ivi_prefix_len_t;

/**
 @brief  Define ip nexthop tunnel info structure
*/
struct ctc_ip_nh_tunne_info_s
{
    uint8  tunnel_type;    /**< [GB.GG.D2.TM] Ip tunnel type, CTC_TUNNEL_TYPE_XXX,refer to ctc_nh_ip_tunnel_type_t*/
    uint8  ttl;            /**< [GB.GG.D2.TM] TTL value*/
    uint8  dscp_domain;    /**< [D2.TM] Dscp domain, used when dscp_select is CTC_NH_DSCP_SELECT_MAP*/
    uint8  ecn_select;     /**< [D2.TM] Ecn select mode, refer to ctc_nh_ecn_select_mode_t, for tunnel edit only support map or copy*/

    uint32 flag;           /**< [GB.GG.D2.TM] CTC_IP_NH_TUNNEL_FLAG_XXX,ctc_ip_nh_tunnel_flag_t*/

    uint8  dscp_select;    /**< [GB.GG.D2.TM] Dscp select mode,refer to ctc_nh_dscp_select_mode_t*/
    uint8  dscp_or_tos;    /**< [GB.GG.D2.TM] Dscp value for Ip version 4 , tos for Ip version 6(Dscp value for Ip version 6 for GG) */
    uint16 mtu_size;       /**< [GB.GG.D2.TM] MTU size*/

    uint32 flow_label;     /**< [GB.GG.D2.TM] Ip version 6 flow label */

    uint8  flow_label_mode; /**< [GB.GG.D2.TM] Ip version 6 flow label mode,refer to ctc_nh_flow_label_mode_t */
    uint8  inner_packet_type;    /**< [GG.D2.TM] indicate payload packet type when encap overlay tunnel, refer to packet_type_t*/
    uint16 l4_dst_port;     /**< [GB.GG.D2.TM] layer4 destination port used by NAPT */
    uint16 l4_src_port;     /**< [GG.D2.TM] layer4 source port used by VXLAN */

    uint32 vn_id;          /**< [GG.D2.TM] vn_id value used for overlay(NvGRE/VxLAN/eVxLAN/GENEVE) encap , if 0 means use the vn_id from IPE */
    mac_addr_t inner_macda;/**< [GG.D2.TM] inner_macda value only valid when CTC_IP_NH_TUNNEL_FLAG_OVERLAY_CROSS_VNI is set */
    mac_addr_t inner_macsa;/**< [GG.D2.TM] inner_macsa value only valid when CTC_IP_NH_TUNNEL_FLAG_OVERLAY_ROUTE_MACSA is set, it will
                                    use route MAC when inner_macsa = 0*/

    uint16 logic_port;     /**< [GG.D2.TM] Assign a logic port to the tunnel */
    uint16 span_id;        /**< [GG.D2.TM] Assign span_id for erspan session, <0-1023> */

    uint32 stats_id;       /**< [GB.GG.D2.TM] Stats id, 0 is disable */

    uint32 dot1ae_chan_id;    /**< [TM] bind Dot1AE SC ID to tunnel,value = 0 is unbind*/

    union
    {
        ip_addr_t   ipv4; /**< [GB.GG.D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6; /**< [GB.GG.D2.TM] Ip version 6 address */
    } ip_sa;               /**< [GB.GG.D2.TM] New ip sa address in ip header*/

    union
    {
        ip_addr_t   ipv4; /**< [GB.GG.D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6; /**< [GB.GG.D2.TM] Ip version 6 address */
    } ip_da;               /**< [GB.GG.D2.TM] New ip da address in ip header*/

    struct
    {
        uint8  gre_ver;         /**< [GB.GG.D2.TM] GRE verion,delete */
        uint8  rsv;             /**< [GB.GG.D2.TM] reserved */
        uint16 protocol_type;   /**< [GB.GG.D2.TM] GRE header's Protocol Type field  */
        uint32 gre_key;         /**< [GB.GG.D2.TM] GRE Key. When enable mtu check, GRE key only have 16 bit */
    } gre_info;

    struct
    {
        uint8 sp_prefix_len;    /**< [GB.GG.D2.TM] SP prefix length for 6rd type tunnel, refer to ctc_nh_sp_prefix_length_t */
        uint8 v4_sa_masklen;    /**< [GB.GG.D2.TM] The number of high-order bits for ipv4 sa that can be config by user. */
        uint8 v4_da_masklen;    /**< [GB.GG.D2.TM] The number of high-order bits for ipv4 da that can be config by user. */
        uint8 rsv0;
    } sixrd_info;

    struct
    {
        ctc_nh_ivi_prefix_len_t  prefix_len;    /**< [GB.GG.D2.TM] ivi prefix length */
        uint8 is_rfc6052;                       /**< [GB.GG.D2.TM] the IPv4-embedded IPv6 address's 64 to 71 bits are set to zero*/
        ipv6_addr_t ipv6;                       /**< [GB.GG.D2.TM] ipv6 prefix address used for IVI4TO6 */
    } ivi_info;

    uint8  adjust_length;                        /**< [GG.D2.TM] The value of adjust length */
    /*UDF Header*/
    uint8 udf_profile_id;                        /**< [TM] UDF edit profile ID, refer to function ctc_nh_add_udf_profile() */
    uint8 rsv1;
    uint16 udf_replace_data[CTC_NH_UDF_MAX_EDIT_NUM];     /**< [TM] UDF replace data, if type in struct ctc_nh_udf_edit_t is 1 */

    uint8 openflow_ip_protocol;   /**< SYSTEM MODIFY for hybrid v4/v6 exchange */
};
typedef struct ctc_ip_nh_tunne_info_s ctc_ip_nh_tunne_info_t;

/**
 @brief  Define ip tunnel nexthop parameter structure
*/
struct ctc_ip_tunnel_nh_param_s
{
    uint32                  flag;       /**< [GB.GG.D2.TM] IP nexthop flag --> ctc_ip_nh_flag_t,CTC_IP_NH_FLAG_xxx flags*/
    ctc_nh_upd_type_t       upd_type;   /**< [GB.GG.D2.TM] nexthop update type ,used for update Operation*/
    ctc_nh_oif_info_t       oif;        /**< [GB.GG.D2.TM] if encap packet directly send out,it is outgoing interface information,else it is invalid*/
    mac_addr_t              mac;        /**< [GB.GG.D2.TM] if encap packet directly send out,it is nexthop router mac,else it is invalid */
    mac_addr_t              mac_sa;     /**< [GG.D2.TM] if encap packet directly send out by vlan port,it is nexthop mac sa,else it is invalid */
    ctc_ip_nh_tunne_info_t  tunnel_info; /**< [GB.GG.D2.TM]  tunneling information */
    uint32                  dsnh_offset; /**< [GB.GG.D2.TM] if Chipset use Egress chip Edit mode ,
                                                  the dsnh_offset is allocated in platform adaption Layer;
                                                  else the dsnh_offset is internally allocated in SDK */
    uint16 arp_id;                       /**< [GB.GG.D2.TM] arp id */
    uint32 loop_nhid;                    /**< [GB.GG.D2.TM] loopback to destination nexthop id, 0 means invalid */
};
typedef struct ctc_ip_tunnel_nh_param_s ctc_ip_tunnel_nh_param_t;


enum ctc_nh_wlan_tunnel_flag_e {
    CTC_NH_WLAN_TUNNEL_FLAG_IPV6                = (1<<0), /**< [D2.TM] If set, means the is ipv6 wlan tunnel */
    CTC_NH_WLAN_TUNNEL_FLAG_MAP_TTL             = (1<<1), /**< [D2.TM] If set , means new TTL mapped from (pkt_ttl - ttl), else out_pkt_ttl = ttl*/
    CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS             = (1<<2), /**< [D2.TM] If set, cos from mapping; otherwise, from inner packet.*/
    CTC_NH_WLAN_TUNNEL_FLAG_ENCRYPT_EN          = (1<<3), /**< [D2.TM] If set, means need encrypt the wlan packet*/
    CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN        = (1<<4), /**< [D2.TM] If set, means wlan need from 802.3 translation to 802.11*/
    CTC_NH_WLAN_TUNNEL_FLAG_FRAGMENT_EN         = (1<<5), /**< [D2.TM] If set, means support frag the wlan packet, when the packet size is big*/
    CTC_NH_WLAN_TUNNEL_FLAG_MC_EN               = (1<<6), /**< [D2.TM] If set, means the packet is multicast*/
    CTC_NH_WLAN_TUNNEL_FLAG_RADIO_MAC_EN        = (1<<7), /**< [D2.TM] If set, means the radio mac is valid*/
    CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN              = (1<<8), /**< [D2.TM] If set, the CTC_WLAN_NH_TUNNEL_FLAG_TRANS_EN and CTC_WLAN_NH_TUNNEL_FLAG_RADIO_MAC_EN must set. */
    CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET     = (1<<9), /**< [D2.TM] If set, means need rewrite macDa and macSa */
    CTC_NH_WLAN_TUNNEL_FLAG_POP_POA_ROAM        = (1<<10),/**< [D2.TM] If set, means work in POP/POA Mode */
    CTC_NH_WLAN_TUNNEL_FLAG_UNTAG_EN            = (1<<11),/**< [D2.TM] If set, means the packet will be untaged inner vlan, only for bridge packet.*/
    CTC_NH_WLAN_TUNNEL_FLAG_KEEPALIVE           = (1<<12),/**< [D2.TM] If set, the packet use nexthop is keepalive*/
    CTC_NH_WLAN_TUNNEL_FLAG_COPY_DONT_FRAG      = (1<<13),/**< [D2.TM] [Only for ipv4] If set , means new dont frag will copy payload ip dont frag);else will use user defined dont frag*/
    CTC_NH_WLAN_TUNNEL_FLAG_DONT_FRAG           = (1<<14),/**< [D2.TM] [Only for ipv4] User defined dont frag,if set ,dont frag will set to 1;else to 0 */
    CTC_NH_WLAN_TUNNEL_FLAG_LOGIC_PORT_CHECK    = (1<<15) /**< [D2.TM] Logic port check enable */
};
typedef enum ctc_nh_wlan_tunnel_flag_e ctc_nh_wlan_tunnel_flag_t;



/**
 @brief  Define wlan nexthop tunnel info structure
*/
struct ctc_nh_wlan_tunnel_param_s
{
    uint32 flag;            /**< [D2.TM] CTC_NH_WLAN_TUNNEL_FLAG_XXX,ctc_nh_wlan_tunnel_flag_t*/
    uint8  upd_type;        /**< [D2.TM] nexthop update type ,used for update Operation*/
    uint32 dsnh_offset;     /**< [D2.TM] if Chipset use Egress chip Edit mode ,
                                                  the dsnh_offset is allocated in platform adaption Layer;
                                                  else the dsnh_offset is internally allocated in SDK */
    /*encap capwap header info*/
    union
    {
        ip_addr_t   ipv4;   /**< [D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6;   /**< [D2.TM] Ip version 6 address */
    } ip_sa;                /**< [D2.TM] New ip sa address in ip header*/

    union
    {
        ip_addr_t   ipv4;   /**< [D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6;   /**< [D2.TM] Ip version 6 address */
    } ip_da;                /**< [D2.TM] New ip da address in ip header*/
    uint32 flow_label;      /**< [D2.TM] Ip version 6 flow label */
    uint8  flow_label_mode; /**< [D2.TM] Ip version 6 flow label mode,refer to ctc_nh_flow_label_mode_t */
    uint8  ecn_select;      /**< [D2.TM] Ecn select mode, refer to ctc_nh_ecn_select_mode_t, for wlan tunnel edit only support map or copy*/
    uint16 l4_src_port;     /**< [D2.TM] l4 source port*/
    uint16 l4_dst_port;     /**< [D2.TM] l4 destination port*/
    uint8  ttl;             /**< [D2.TM] TTL value*/
    uint8  dscp_select;     /**< [D2.TM] Dscp select mode,refer to ctc_nh_dscp_select_mode_t*/
    uint8  dscp_or_tos;     /**< [D2.TM] Dscp value for Ip version 4 , tos for Ip version 6(Dscp value for Ip version 6 for GG) */
    uint8  dscp_domain;     /**< [D2.TM] Dscp Qos Domain, used when dscp_select equal CTC_NH_DSCP_SELECT_MAP*/
    uint8  cos_domain;      /**< [D2.TM] Cos Qos Domain, used when CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS is set*/
    mac_addr_t radio_mac;   /**< [D2.TM] radio mac*/
    mac_addr_t mac_da;      /**< [D2.TM] mac da, if CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN, indicate bssid destination mac */
    uint8  radio_id;        /**< [D2.TM] radio id*/
    uint8  dot11_sub_type;  /**< [D2.TM] width 4 bits.802.11 sub type. Value <0-15>,default is 0 */
    uint32 bssid_bitmap;    /**< [D2.TM] bssid bitmap for ap do multicast, If equal 0, bssid_bitmap indicate invalid */
    uint16 vlan_id;         /**< [D2.TM] inner vlan id, 0 means have no vlan, others means vlan ID to edited, only used for route packet
                                ToAC: (1) bridge packet must be keep vlantag; CTC_NH_WLAN_TUNNEL_FLAG_UNTAG_EN set 0
                                      (2) routed packet must be append a vlanTag; vlan_id !=0
                                ToWTP:(1) bridge packet could be keep vlantag or untag, control by flag CTC_NH_WLAN_TUNNEL_FLAG_UNTAG_EN;
                                      (2) routed packet could be append a vlanTag or untag; control by vlan_id equal 0 or not */
    uint8  is_pop;          /**< [D2.TM] if set,means AC Type is POP in POP/POA roam mode.*/

    /*encrypt info*/
    uint8  encrypt_id;      /**< [D2.TM] encrypt id*/
    uint16 mtu_size;        /**< [D2.TM] MTU size for plain packet, if want to encrypt,mtu less then 1420 is recommended*/
    uint16 logic_port;      /**< [D2.TM] Assign a logic port to the tunnel */
    uint32 stats_id;        /**< [D2.TM] Stats id, 0 is disable */
};
typedef struct ctc_nh_wlan_tunnel_param_s ctc_nh_wlan_tunnel_param_t;


/**
 @brief Define the type of vlan edit type in EPE of Humber
*/
enum ctc_vlan_egress_edit_type_e
{
    /*output vlan is same as ingress mapped vlan(eg. vlan translation vid,epe do not edit the vlan*/
    CTC_VLAN_EGRESS_EDIT_NONE,                  /**< [GB.GG.D2.TM] Will keep ingress mapped vlan*/
    /*Keep output vlan same as input vlan(raw vlan) in the packet*/
    CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE,    /**< [GB.GG.D2.TM] Will keep original packet's vlan*/
    CTC_VLAN_EGRESS_EDIT_INSERT_VLAN,           /**< [GB.GG.D2.TM] Will insert a new vlan, according to ingress mapped vlan or egress specified vlan, egress vlan have high priority
                                                    [D2.TM]:Insert edit will depend on vlan-domain, for s-domain, do not insert, for c-domain insert svlan */
    CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN,          /**< [GB.GG.D2.TM] Will replace original vlan, according to ingress mapped vlan or egress specified vlan, egress vlan have high priority;
                                                    [GB.GG.D2.TM]:if original packet is untagged, will append a new tag
                                                    [D2.TM]:Support edit depend on vlan-domain, for s-domain, do replace, for c-domain do nothing */
    CTC_VLAN_EGRESS_EDIT_STRIP_VLAN,            /**< [GB.GG.D2.TM] Will strip original vlan*/
    CTC_VLAN_EGRESS_EDIT_MAX
};
typedef enum ctc_vlan_egress_edit_type_e ctc_vlan_egress_edit_type_t;

/**
 @brief  Define Egress Vlan Editing nexthop flag
*/
enum ctc_vlan_egress_edit_nh_flag_e
{
    CTC_VLAN_NH_SERVICE_QUEUE_FLAG = (1 << 0),     /**< [GB] create a egress valn nexthop with service queue select type*/
    CTC_VLAN_NH_LENGTH_ADJUST_EN   = (1 << 1),     /**< [GB] This flag indicate need adjust packet length when do shaping*/
    CTC_VLAN_NH_ETREE_LEAF         = (1 << 2),     /**< [GB.GG.D2.TM] This flag indicate the nexthop is leaf node in E-Tree networks */
    CTC_VLAN_NH_HORIZON_SPLIT_EN   = (1 << 3),     /**< [GB.GG.D2.TM] This flag indicate the nexthop enable horizon split in VPLS networks */
    CTC_VLAN_NH_STATS_EN           = (1 << 4),     /**< [GB.GG.D2.TM] This flag indicate the nexthop enable stats */
    CTC_VLAN_NH_MTU_CHECK_EN       = (1 << 5),     /**< [GB] Enable MTU check, will use nexthop8w */
    CTC_VLAN_NH_PASS_THROUGH       = (1 << 6),      /**< [GB.GG] If set, will ignore dot1q and untag default vlan configuration on port */
    /* SYSTEM MODIFIED by yaom for Openflow 20160322 */
    CTC_VLAN_NH_HAVE_DSFWD         = (1 << 7)      /**< [GG] Enable DsFwd */
};
typedef enum ctc_vlan_egress_edit_nh_flag_e ctc_vlan_egress_edit_nh_flag_t;

/**
 @brief  Define egress vlan edit information structure
*/
struct ctc_vlan_egress_edit_info_s
{
    uint32 flag;                                    /**< [GB.GG.D2.TM] nexthop flag, CTC_VLAN_NH_XXX_FLAG,pls refer to ctc_vlan_egress_edit_nh_flag_t*/
    /*If cvlan edit type is insert cvlan, then svlan edit should not be replace;
    And if svlan edit type is insert svlan, cvlan edit should not be replace*/
    uint32 stats_id;                                /**< [GB.GG.D2.TM] Stats id*/
    ctc_vlan_egress_edit_type_t cvlan_edit_type;    /**< [GB.GG.D2.TM] Edit type on cvlan*/
    ctc_vlan_egress_edit_type_t svlan_edit_type;    /**< [GB.GG.D2.TM] Edit type on svlan*/
    uint16 output_cvid;                             /**< [GB.GG.D2.TM] Specified output cvlan id*/
    uint16 output_svid;                             /**< [GB.GG.D2.TM] Specified output svlan id*/
    uint16 edit_flag;                                /**< [GB.GG.D2.TM] Edit flag*/
    uint8 svlan_tpid_index;                         /**< [GB.GG.D2.TM] svlan tpid index*/
#define CTC_VLAN_EGRESS_EDIT_VLAN_SWAP_EN           (1 << 0)    /**< [GB.GG.D2.TM] Edit flag indicate vlan swap*/
#define CTC_VLAN_EGRESS_EDIT_TPID_SWAP_EN           (1 << 1)    /**< [GB.GG.D2.TM] Edit flag indicate tpid swap*/
#define CTC_VLAN_EGRESS_EDIT_COS_SWAP_EN            (1 << 2)    /**< [GB.GG.D2.TM] Edit flag indicate cos swap*/
#define CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID       (1 << 3)    /**< [GB.GG.D2.TM] Edit flag indicate output cvlan id*/
#define CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID       (1 << 4)    /**< [GB.GG.D2.TM] Edit flag indicate output svlan id*/
#define CTC_VLAN_EGRESS_EDIT_OUTPUT_SVLAN_TPID_VALID   (1 << 5)    /**< [GB.GG.D2.TM] This flag indicate the nexthop can change svlan tpid */
#define CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS   (1 << 6)    /**< [GB.GG.D2.TM] This flag indicate the nexthop can change svlan cos */
#define CTC_VLAN_EGRESS_EDIT_MAP_SVLAN_COS       (1 << 7)    /**< [GB.GG.D2.TM] This flag indicate the new stag cos is mapped from priority */
#define CTC_VLAN_EGRESS_EDIT_SVLAN_AWARE         (1 << 8)    /**< [D2.TM] This flag indicate svlan edit will depend on packet svlan or ingress mapping svlan
                                                             insert svlan:  Svlan exist do nothing, Svlan not exist do insert,
                                                             replace svlan: Svlan exist do replace, Svlan not exist do nothing*/

    uint8 stag_cos;                                /**< [GB.GG.D2.TM] Specified output stag cos*/

    uint8 resv;

    uint16 user_vlanptr;                            /**< [GB.GG.D2.TM] Set the dest_vlanptr of the nexthop, 0 means use src_vlan_ptr */
    uint16 mtu_size;                                /**< [GB] MTU size */
    uint32 loop_nhid;                               /**< [GB.GG.D2.TM] loopback to destination nexthop id, 0 means invalid */
};
typedef struct ctc_vlan_egress_edit_info_s ctc_vlan_egress_edit_info_t;

/**
 @brief Define the data structure about vlan nexthop parameter structure
*/
struct ctc_vlan_edit_nh_param_s
{
    uint32 dsnh_offset;                           /**< [GB.GG.D2.TM] if Chipset use Egress chip Edit , the dsnh_offset is allocated in platform adaption Layer; else the dsnh_offset is internally allocated in SDK */

    uint32 gport_or_aps_bridge_id;                /**< [GB.GG.D2.TM]  Destination global port  or APS bridge group id */
    uint8  aps_en;                                 /**< [GB.GG.D2.TM] if set,indicate aps enable */
    uint8  logic_port_valid;                      /**< [GB.GG] if set, indicated logic_port is valid.In VPLS networks,it should be set to 1. */

    uint16 logic_port;                            /**< [GB.GG.D2.TM] destination logic port in VPLS networks, 0 means disable */
    uint8  logic_port_check;                      /**< [D2.TM] Logic port check enable  */
    uint8  rsv0;                                  /**< Reserved0 */

    uint16  cid;                                  /**< [D2.TM] category id, 0 means disable*/
    uint16  service_id;                           /**< [TM] sevice id  which use for Egress service Qos, 0 means disable*/

    ctc_vlan_egress_edit_info_t vlan_edit_info;   /**< [GB.GG.D2.TM] Working path's Vlan Edit */
    ctc_vlan_egress_edit_info_t vlan_edit_info_p; /**< [GB.GG.D2.TM] Protection path's Vlan Edit */
};
typedef struct ctc_vlan_edit_nh_param_s ctc_vlan_edit_nh_param_t;

/**
 @brief  Define MPLS nexthop operation type
*/
enum ctc_mpls_nh_op_e
{
    /*Push operation*/
    CTC_MPLS_NH_PUSH_OP_NONE,             /**< [GB.GG.D2.TM] Will append new layer2 header, mpls payload is Ethernet, and will keep mpls payload unchanged.   */
    /*Swap  label on LSR /Pop label and do label  Swap on LER */
    CTC_MPLS_NH_PUSH_OP_ROUTE,            /**< [GB.GG.D2.TM] Will append new layer2 header, mpls payload is IP, and ip TTL will be updated. (L3VPN/FTN)*/
    CTC_MPLS_NH_PUSH_OP_L2VPN,            /**< [GB.GG.D2.TM] Will append new layer2 header, mpls payload is Ethernet, ethernet packet's tag could be edited (L2VPN)*/

    /* pop operation*/
    CTC_MPLS_NH_PHP,                     /**< [GB.GG.D2.TM] Will append new layer2 header, TTL processing according to ctc_mpls_nexthop_pop_param_t.ttl_mode*/

    CTC_MPLS_NH_OP_MAX
};
typedef enum ctc_mpls_nh_op_e ctc_mpls_nh_op_t;

/**
 @brief  Define mpls nexthop label type
*/
enum ctc_mpls_nh_label_type_flag_e
{
    CTC_MPLS_NH_LABEL_IS_VALID  = (1 << 0),   /**< [GB.GG.D2.TM] If set , means this MPLS label is valid*/
    CTC_MPLS_NH_LABEL_IS_MCAST  = (1 << 1),   /**< [GB.GG.D2.TM] If set , means this MPLS label is mcast label*/
    CTC_MPLS_NH_LABEL_MAP_TTL   = (1 << 2)    /**< [GB.GG.D2.TM] If set , means new TTL mapped from (pkt_ttl - ttl)*/
};
typedef enum ctc_mpls_nh_label_type_flag_e ctc_mpls_nh_label_type_flag_t;

/**
 @brief  Define mpls exp select mode
*/
enum ctc_nh_exp_select_mode_e
{
    CTC_NH_EXP_SELECT_ASSIGN,          /**< [GB.GG.D2.TM] Use user-define EXP to outer header.*/
    CTC_NH_EXP_SELECT_MAP,             /**< [GB.GG.D2.TM] Use EXP value from EXP map */
    CTC_NH_EXP_SELECT_PACKET,          /**< [GB.GG.D2.TM] Copy packet EXP to outer header */
    MAX_CTC_NH_EXP_SELECT_MODE        /* Must be last */
};
typedef enum ctc_nh_exp_select_mode_e ctc_nh_exp_select_mode_t;

/**
 @brief  Define MPLS label parameter structure
*/
struct ctc_mpls_nh_label_param_s
{
    uint8  lable_flag; /**< [GB.GG.D2.TM] MPLS label flag,ctc_mpls_nh_label_type_flag_t */
    uint8  ttl;        /**< [GB.GG.D2.TM] TTL value.
                                      if CTC_MPLS_NH_LABEL_MAP_TTL set to 1:
                                        FTN Pipe mode ttl set to 0,else 1.
                                      else ttl will fare up to (0£¬1£¬2£¬8£¬15£¬16£¬31£¬32£¬60£¬63£¬64£¬65£¬127£¬128£¬254£¬255)
                                      according to user defined value*/
    uint8  exp_type;   /**< [GB.GG.D2.TM] New MPLS exp type in this label,pls refer to ctc_nh_exp_select_mode_t*/
    uint8  exp;        /**< [GB.GG.D2.TM] EXP value*/
    uint32 label;      /**< [GB.GG.D2.TM] MPLS label value*/
    uint8  exp_domain; /**< [D2.TM] Mpls Exp Domain, used when exp_type equal CTC_NH_EXP_SELECT_MAP*/
    uint8  rsv[3];
};
typedef struct ctc_mpls_nh_label_param_s ctc_mpls_nh_label_param_t;

/**
 @brief  Define MPLS martini label sequence type
*/
enum ctc_mpls_nh_martini_seq_type_e
{
    CTC_MPLS_NH_MARTINI_SEQ_NONE = 0,   /**< [GB.GG.D2.TM] Martini label sequence number type none, means doesn't support martini label sequence*/

    CTC_MPLS_NH_MARTINI_SEQ_MAX
};
typedef enum ctc_mpls_nh_martini_seq_type_e ctc_mpls_nh_martini_seq_type_t;

/**
 @brief  Define MPLS nexthop flag type
*/
enum ctc_mpls_nh_flag_type_e
{
    CTC_MPLS_NH_FLAG_SERVICE_QUEUE_EN  =  (1 << 0),     /**< [GB] Enable service queue, ,only need set the bit in humber */
    /* SYSTEM MODIFIED by yaom for Openflow 20160228 */
    CTC_MPLS_NH_FLAG_NO_SRC_DSCP_COS   =  (1 << 1),     /** [GG]do not use l2 header cos or l3 header dscp */
    CTC_MPLS_NH_FLAG_L2EDIT_OUTPUT_VID =  (1 << 2),     /** [GG]use l2edit to output vlan id */
};
typedef enum ctc_mpls_nh_flag_type_e ctc_mpls_nh_flag_type_t;

/**
 @brief  Define MPLS nexthop aps flag type
*/
enum ctc_mpls_nh_tunnel_aps_flag_type_e
{
    CTC_NH_MPLS_APS_EN          = (1 << 0),               /**< [GB.GG.D2.TM] if set,indicate tunnel enable aps  */
    CTC_NH_MPLS_APS_2_LAYER_EN                =(1 << 1),  /**< [GB.GG.D2.TM] if set, if label_num is equal to 2 ,aps_en is 1 and it
                                                              is set,indicate two layer tunnel protection are enable  */
    CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION   = (1 << 2), /**< [GB.GG.D2.TM] if set ,indicate tunnel parameter is protection  path parameter
                                                              ;else indicate tunnel parameter is working path parameter */
};
typedef enum ctc_mpls_nh_tunnel_aps_flag_type_e ctc_mpls_nh_tunnel_aps_flag_type_t;

/**
 @brief  Define MPLS nexthop common parameter structure
*/
struct ctc_mpls_nexthop_com_param_s
{
    uint32  mpls_nh_flag;                  /**< [GB.GG.D2.TM] MPLS nexthop flag, CTC_MPLS_NH_FLAG_XXXX*/

    ctc_mpls_nh_op_t   opcode;             /**< [GB.GG.D2.TM] MPLS nexthop operation code*/
    ctc_nh_oif_info_t  oif;                /**< [GB.GG.D2.TM] Outgoing interface for mpls POP nexthop.*/
    mac_addr_t mac;                        /**< [GB.GG.D2.TM] MPLS nexthop outgoing MAC address*/
    mac_addr_t mac_sa;                     /**< [GG.D2.TM] MPLS nexthop outgoing MAC address for L3VPN to L2VPN*/
    ctc_vlan_egress_edit_info_t vlan_info; /**< [GB.GG.D2.TM] VLAN edit information, used for layer2 VPN*/
};
typedef struct ctc_mpls_nexthop_com_param_s ctc_mpls_nexthop_com_param_t;

/**
 @brief  Define MPLS tunnel label nexthop parameter structure
*/
struct ctc_mpls_nexthop_tunnel_info_s
{
    ctc_mpls_nh_label_param_t  tunnel_label[CTC_MPLS_NH_MAX_TUNNEL_LABEL_NUM]; /**< [GB.GG.D2.TM] MPLS lable  information.Order:VC label-->Tunnel labell*/
    mac_addr_t mac;        /**< [GB.GG.D2.TM] MPLS nexthop outgoing MAC address*/
    mac_addr_t mac_sa;     /**< [GG.D2.TM] MPLS nexthop outgoing MAC address. Value 0 means invalid*/
    ctc_nh_oif_info_t  oif;/**< [GB.GG.D2.TM] Outgoing interface.*/
    uint32 stats_id;       /**< [GB.GG.D2.TM] stats id*/
    uint8  stats_valid;    /**< [GB.GG.D2.TM] stats valid*/
    uint8  label_num;      /**< [GB.GG.D2.TM] the number of tunnel label */
    uint8  is_spme;        /**< [GG.D2.TM] If label number is 1, use spme edit to add label*/
    uint8  is_sr;          /**< [D2.TM] If set means mpls segment route, and the segment labels are all pushed by the mpls tunnel */
    uint16 arp_id;         /**< [GG.D2.TM] arp id */
    uint32 loop_nhid;      /**< [TM] loopback to destination nexthop id, 0 means invalid */
};
typedef struct ctc_mpls_nexthop_tunnel_info_s ctc_mpls_nexthop_tunnel_info_t;

/**
 @brief  Define MPLS tunnel label  parameter structure
 */

/*

 if tunnel label have two layer label,taking LSP label & SPME label as a case;
         and if tunnel label have only one layer label,taking LSP label as a case
    if CTC_NH_MPLS_APS_EN & CTC_NH_MPLS_APS_2_LAYER_EN are set, aps_bridge_group_id is 2-level(Lsp) protection group id
                                    SPME label(w)  (nh_param)
              LSP label(w)------>                                (CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION isn't set)
                                    SPME label(p)   (nh_p_param)


                                    SPME label(w)   (nh_param)
              LSP label(P)------>                                (CTC_NH_MPLS_APS_2_LAYER_EN_PROTECTION is set)
                                    SPME label(p)   (nh_p_param)

    if CTC_NH_MPLS_APS_EN only is set, there are two case,aps_bridge_group_id is 2-level(Lsp) protection group id.

             (1) LSP label(w)------> SPME label  (-->nh_param)
                 LSP label(P)------> SPME label  (-->nh_p_param)
             (2)
                LSP label(w)  (-->nh_param)
                LSP label(P)  (-->nh_p_param)
*/
struct ctc_mpls_nexthop_tunnel_param_s
{
    ctc_mpls_nexthop_tunnel_info_t    nh_param; /**< [GB.GG.D2.TM] MPLS lable  information,Order :LSP label (w)-->SPME label (w)*/
    ctc_mpls_nexthop_tunnel_info_t    nh_p_param; /**< [GB.GG.D2.TM] MPLS lable  information,Order :LSP label(P)-->SPME label (w)*/
    uint32 aps_flag;                              /**< [GB.GG.D2.TM]ctc_mpls_nh_tunnel_aps_flag_type_t */
    uint16 aps_bridge_group_id;                   /**< [GB.GG.D2.TM] aps group id*/
};
typedef struct ctc_mpls_nexthop_tunnel_param_s ctc_mpls_nexthop_tunnel_param_t;
/**
 @brief  Define MPLS push nexthop parameter structure*/
struct ctc_mpls_nexthop_push_param_s
{
    ctc_mpls_nexthop_com_param_t nh_com;    /**< [GB.GG.D2.TM] MPLS nexthop common information*/
    ctc_mpls_nh_label_param_t    push_label[CTC_MPLS_NH_MAX_PUSH_LABEL_NUM]; /**< [GB.GG.D2.TM] MPLS lable  information,Order :PW label(inner label)->LSP Label ->SPME Label (outer label)*/
    uint8  martini_encap_valid;               /**< [GB.GG.D2.TM] Martini label is valid*/
    uint8  martini_encap_type;                /**< [GB.GG.D2.TM] Martini label encap type, 0: Fix mode; 1:specify Control Word*/
    uint32 stats_id;                          /**< [GB.GG.D2.TM] stats id*/
    uint8  stats_valid;                       /**< [GB.GG.D2.TM]pw stats is valid */
    uint8  mtu_check_en;                      /**< [GB] MTU check is enabled*/

    uint32 seq_num_index;                   /**< [GB.GG.D2.TM] if martini_encap_type set,indicate it is 32-bit CW*/
    uint8 label_num;                        /**< [GB.GG.D2.TM] the label_num only include vc label,it may be 0,1;if nexthop is tunnel nexthop,it should be set to 0*/
    uint8  rsv0[1];                         /**< [GB.GG.D2.TM] researve for future */

    uint16 tunnel_id;                       /**< [GB.GG.D2.TM]the tunnel id, will use it to find the tunnel info.*/

    uint16 mtu_size;                        /**< [GB] MTU size for this nexthop */
    uint32 loop_nhid;                       /**< [D2.TM] loopback to destination nexthop id, 0 means invalid */
};
typedef struct ctc_mpls_nexthop_push_param_s ctc_mpls_nexthop_push_param_t;

/**
 @brief  Define MPLS pop nexthop parameter structure
*/
struct ctc_mpls_nexthop_pop_param_s
{
    ctc_mpls_nexthop_com_param_t nh_com; /**< [GB.GG.D2.TM] MPLS nexthop common information*/
    uint8    ttl_mode;                   /**< [GB.GG.D2.TM] uniform/pipe/short pipe,refer cto ctc_mpls_tunnel_mode_e*/
    uint16   arp_id;                     /**< [GB.GG.D2.TM] arp id */
};
typedef struct ctc_mpls_nexthop_pop_param_s ctc_mpls_nexthop_pop_param_t;

/**
 @brief  Define MPLS nexthop property associated type emun
*/
enum ctc_mpls_nh_prop_e
{
    CTC_MPLS_NH_NONE,                /**< [GB.GG.D2.TM]  any mpls nexthop */
    CTC_MPLS_NH_PUSH_TYPE,           /**< [GB.GG.D2.TM]  mpls push nexthop */
    CTC_MPLS_NH_POP_TYPE,            /**< [GB.GG.D2.TM]  mpls POP(Only apply to PHP) nexthop */
    CTC_MPLS_NH_PROP_TYPE_MAX
};
typedef enum ctc_mpls_nh_prop_e ctc_mpls_nh_prop_t;

/**
 @brief  Define MPLS nexthop operation type emun
*/
enum ctc_mpls_nh_operation_type_flag_e
{
    CTC_MPLS_NH_IS_UNRSV             = 0x00000001,  /**< [GB.GG.D2.TM] if set, nexthop is unresolved nexthop  */
	CTC_MPLS_NH_IS_HVPLS             = 0x00000002,  /**< [GB.GG.D2.TM] if set,Enable hvpls and disable split horizon in VPLS network*/

};
typedef enum ctc_mpls_nh_operation_type_flag_e ctc_mpls_nh_operation_type_flag_t;

/**
 @brief  Define MPLS nexthop parameter structure
*/
struct ctc_mpls_nexthop_param_s
{
    ctc_mpls_nh_prop_t nh_prop;               /**< [GB.GG.D2.TM] MPLS nexthop property associated type --> ctc_mpls_nh_prop_t */
    ctc_nh_upd_type_t upd_type;               /**< [GB.GG.D2.TM] nexthop update type ,used for update Operation*/
    uint32  dsnh_offset;                      /**< [GB.GG.D2.TM] dsNexthop offset,if is_p_para_valid is enable ,when protected object is user service
                                                    (working and protection path's use different VC label ),it will occupy 2 entries dsNexthop
                                                     and the offset are dsnh_offset and dsnh_offset+1*/
    uint32   flag;                           /**< [GB.GG.D2.TM] CTC_MPLS_NH_XX_XX,pls refer to ctc_mpls_nh_operation_type_flag_t */
    uint8    aps_en;                         /**< [GB.GG.D2.TM] if set, indicate PW layer APS enable*/
    uint8    logic_port_valid;               /**< [GB.GG] if set, indicated logic_port is valid.In VPLS networks,it should be set to 1. */
    uint8    adjust_length;                  /**<[GB.GG.D2.TM] The value of adjust length . */

    uint16   aps_bridge_group_id;            /**< [GB.GG.D2.TM]  the APS group ID of the nexthop,*/
    uint16   logic_port;                     /**< [GB.GG.D2.TM] destination logic port in VPLS networks */

    uint16  service_id;                      /**< [TM] sevice id  which use for Egress service Qos, 0 means disable*/
	uint16  rsv;

    union
    {
        ctc_mpls_nexthop_pop_param_t   nh_param_pop;      /**< [GB.GG.D2.TM] mpls (asp) pop nexthop */
        ctc_mpls_nexthop_push_param_t  nh_param_push;     /**< [GB.GG.D2.TM] mpls push (asp) nexthop */
    }  nh_para;     /**< [GB.GG.D2] nexthop parameter used to create this nexthop */

    union
    {
        ctc_mpls_nexthop_pop_param_t   nh_p_param_pop;      /**< [GB.GG.D2.TM] mpls asp pop nexthop */
        ctc_mpls_nexthop_push_param_t  nh_p_param_push;     /**< [GB.GG.D2.TM] mpls push asp nexthop */
    }  nh_p_para;       /**< [GB.GG.D2.TM] protection path's Nexthop parameter used to create this nexthop ,only used in create a mpls aps nexthop */

};
typedef struct ctc_mpls_nexthop_param_s ctc_mpls_nexthop_param_t;

/**
 @brief  Define loopback nexthop parameter structure
*/
struct ctc_loopback_nexthop_param_s
{
    uint8 customerid_valid;                  /**< [GB.GG.D2.TM] If set, indicates the inner payload has an MPLS encapsulation or GRE key, which contains the customer ID as the label */
    uint8 inner_packet_type_valid;           /**< [GB.GG.D2.TM] This field indicates if the inner_packet_type is valid*/
    uint8 logic_port;                     /**< [GB.GG.D2.TM] If set, indicates that the packet is from a VPLS tunnel port*/
    uint8 service_queue_en;                  /**< [GB] If set, indicates that the packets will do service queue processing*/
    uint8 sequence_en;                       /**< [GB] If set, indicates that the packets will do sequence order */
    ctc_parser_pkt_type_t inner_packet_type; /**< [GB.GG.D2.TM] This field indicates the encapsulated packet type */
    uint16 lpbk_lport;                       /**< [GB.GG.D2.TM] Loopback source global port, should not be linkagg port*/
    uint8 words_removed_from_hdr;           /**< [GB.GG.D2.TM] 1 word stands for 4 bytes, not include customerId 4 bytes;*/
    uint8 sequence_counter_index;           /**< [GB] Sequence number counter index */
};
typedef struct ctc_loopback_nexthop_param_s ctc_loopback_nexthop_param_t;

/**
 @brief  Define remote mirror nexthop parameter structure
*/
struct ctc_rspan_nexthop_param_s
{
    uint16 rspan_vid;   /**< [GB.GG.D2.TM] RSPAN VLAN ID  */
    uint8  remote_chip;   /**<[GB.GG.D2.TM] if set,indicate it is crosschip RSPAN nexthop*/
    uint8  rsv;

    uint32 dsnh_offset; /**< [GB.GG.D2.TM] DsNextHop offset */
};
typedef struct ctc_rspan_nexthop_param_s ctc_rspan_nexthop_param_t;

/**
 @brief  Define Misc Nexthop l2/L3 edit flags
*/
enum ctc_misc_nh_edit_flag_e
{
    CTC_MISC_NH_L2_EDIT_REPLACE_MAC_DA      = 0x00000001,   /**< [GB.GG.D2.TM] Will replace layer2 header mac da */
    CTC_MISC_NH_L2_EDIT_REPLACE_MAC_SA      = 0x00000002,   /**< [GB.GG.D2.TM] Will replace layer2 header mac sa */
    CTC_MISC_NH_L2_EDIT_SWAP_MAC            = 0x00000004,   /**< [GB.GG.D2.TM] Will swap layer2 header mac da & sa, cannot mix with other types */
    CTC_MISC_NH_L2_EDIT_VLAN_EDIT           = 0x00000008,   /**< [GB] Will do vlan edit */
    CTC_MISC_NH_L3_EDIT_REPLACE_IPDA        = 0x00000010,   /**< [GB] Will replace ipda */
    CTC_MISC_NH_L3_EDIT_REPLACE_DST_PORT    = 0x00000020,   /**< [GB] Will replace dst port */
};

/**
 @brief  Define Misc nexthop L2 Edit parameter structure
*/
struct ctc_misc_nh_l2edit_param_s
{

    uint32       flag;                          /**< [GB.GG.D2.TM] CTC_MISC_NH_L2_EDIT_XXX */

    mac_addr_t   mac_da;                        /**< [GB.GG.D2.TM] Mac destination address */
    mac_addr_t   mac_sa;                        /**< [GB.GG.D2.TM] Mac source address */
    uint16       vlan_id;                       /**< [GB.GG.D2.TM] Vlan id */
    uint8        is_reflective;                /**< [GB.GG.D2.TM] if set,packet will be reflectived to ingress port */
    uint8        rsv[1];                           /**< [GB.GG.D2.TM] reserved */

};
typedef struct ctc_misc_nh_l2edit_param_s ctc_misc_nh_l2edit_param_t;

/**
 @brief  Define Misc nexthop L2/L3 Edit parameter structure
*/
struct ctc_misc_nh_l2_l3edit_param_s
{
    uint32       flag;                          /**< [GB] CTC_MISC_NH_L2_EDIT_XXX */
    mac_addr_t   mac_da;                        /**< [GB] Mac destination address */
    mac_addr_t   mac_sa;                        /**< [GB] Mac source address */

    uint32       dst_port;                      /**< [GB] dst-port */
    uint8        is_reflective;                 /**< [GB] if set,packet will be reflectived to ingress port */
    uint8        rsv;                        /**< [GB] reserved */
    uint32       ipda;                          /**< [GB] ipda */
    ctc_vlan_egress_edit_info_t vlan_edit_info;   /**< [GB] while egress valn nexthop : Egress vlan edit information ; while ASP Egress Vlan Editing nexthop : Egress vlan edit information of working path*/
};
typedef struct ctc_misc_nh_l2_l3edit_param_s ctc_misc_nh_l2_l3edit_param_t;
/**
 @brief  Define cpu reason nexthop parameter structure
*/
struct ctc_misc_nh_cpu_reason_param_s
{
    uint16  cpu_reason_id;      /**< [GB.GG.D2.TM] ctc_pkt_cpu_reason_t */
    uint8   lport_en;           /**< [GB.GG] cpu reason with source port. 0:use global source port 1:local phy source port.only user define reason valid*/
};
typedef struct ctc_misc_nh_cpu_reason_param_s ctc_misc_nh_cpu_reason_param_t;
/**
 @brief  Define Misc nexthop Flex Edit flags
*/
enum ctc_misc_nh_flex_edit_flag_e
{
   /*L2*/
    CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR    = 0x00000001,      /**< [GG.D2.TM] replace l2 header  */
    CTC_MISC_NH_FLEX_EDIT_SWAP_MACDA        = 0x00000002,      /**< [GG.D2.TM] swap macda/macsa  */
    CTC_MISC_NH_FLEX_EDIT_REPLACE_MACDA     = 0x00000004,      /**< [GG.D2.TM] mac destination address */
    CTC_MISC_NH_FLEX_EDIT_REPLACE_MACSA     = 0x00000008,      /**< [GG.D2.TM] mac source address*/
    CTC_MISC_NH_FLEX_EDIT_REPLACE_ETHERTYPE = 0x00000010,      /**< [GG.D2.TM] etherType*/
    CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG  = 0x00000020,      /**< [GG.D2.TM] Vlan tag*/
    CTC_MISC_NH_FLEX_EDIT_OP_BRIDGE         = 0x00000040,      /**< [GG.D2.TM] Flex edit do bridge opreation */

   /*IP*/
   CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR     = 0x00000100,      /**< [GG.D2.TM] replace ip header */
   CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA       = 0x00000200,      /**< [GG.D2.TM] ipda*/
   CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA       = 0x00000400,      /**< [GG.D2.TM] ipsa*/
   CTC_MISC_NH_FLEX_EDIT_SWAP_IP            = 0x00000800,      /**< [GG.D2.TM] swap packet ipsa and ipda*/
   CTC_MISC_NH_FLEX_EDIT_REPLACE_ECN        = 0x00002000,      /**< [GG.D2.TM] replace ecn*/
   CTC_MISC_NH_FLEX_EDIT_REPLACE_TTL        = 0x00004000,      /**< [GG.D2.TM] replace ttl is high priority*/
   CTC_MISC_NH_FLEX_EDIT_DECREASE_TTL       = 0x00008000,      /**< [GG.D2.TM] decrease ttl */
   CTC_MISC_NH_FLEX_EDIT_REPLACE_PROTOCOL   = 0x00010000,      /**< [GG.D2.TM] replace protocol */
   CTC_MISC_NH_FLEX_EDIT_REPLACE_FLOW_LABEL = 0x00020000,      /**< [GG.D2.TM] replace flow label*/
   CTC_MISC_NH_FLEX_EDIT_IPV4               = 0x00040000,      /**< [GG.D2.TM] ipv4/ipv6*/
   CTC_MISC_NH_FLEX_EDIT_MTU_CHECK          = 0x00080000,      /**< [GG.D2.TM] If set , means mtu check enable */

   CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_HDR     = 0x10000000,      /**< [GG.D2.TM] replace L4 header (include udp/tcp/gre/icmp)  */
  /*tcp/udp*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_SRC_PORT  = 0x00100000,     /**< [GG.D2.TM] replace udp/tcp src port */
  CTC_MISC_NH_FLEX_EDIT_REPLACE_L4_DST_PORT  = 0x00200000,     /**< [GG.D2.TM] replace udp/tcp dst port*/
  CTC_MISC_NH_FLEX_EDIT_SWAP_L4_PORT         = 0x00400000,     /**< [GG.D2.TM] swap packet src and dst port */
  CTC_MISC_NH_FLEX_EDIT_REPLACE_UDP_PORT     = 0x00800000,     /**< [GG.D2.TM] replace udp/tcp src port */
  CTC_MISC_NH_FLEX_EDIT_REPLACE_TCP_PORT     = 0x01000000,     /**< [GG.D2.TM] replace udp/tcp src port */

  /*GRE*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_GRE_KEY  = 0x00100000,         /**< [GG.D2.TM] replace GRE Key  */

  /*ICMP*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_TYPE  = 0x00100000,       /**< [GG.D2.TM] replace ICMP Type*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ICMP_CODE  = 0x00200000,       /**< [GG.D2.TM] replace ICMP Code */

 /*ARP*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HDR  = 0x00000100,         /**< [GG.D2.TM] replace arp header */
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HT   = 0x00000200,         /**< [GG.D2.TM] format of hardware type*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_PT   = 0x00000400,         /**< [GG.D2.TM] format of protocol type*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_HLEN = 0x00001000,         /**< [GG.D2.TM] length of hardware address*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_PLEN = 0x00002000,         /**< [GG.D2.TM] length of protocol address*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_OP   = 0x00004000,         /**< [GG.D2.TM] ARP Opcode*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SHA  = 0x00008000,         /**< [GG.D2.TM] sender hardware address*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_SPA  = 0x00010000,         /**< [GG.D2.TM] sender protocol address*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_THA  = 0x00020000,         /**< [GG.D2.TM] target hardware address*/
  CTC_MISC_NH_FLEX_EDIT_REPLACE_ARP_TPA  = 0x00040000          /**< [GG.D2.TM] target protocol address */
};
typedef enum ctc_misc_nh_flex_edit_flag_e ctc_misc_nh_flex_edit_flag_t;


/**
 @brief  Define flex nexthop use packet type
*/
enum ctc_misc_nh_packet_type_e
{
    CTC_MISC_NH_PACKET_TYPE_NONE,      /**< [GG.D2.TM] other packet */
    CTC_MISC_NH_PACKET_TYPE_ARP,       /**< [GG.D2.TM] arp packet*/
    CTC_MISC_NH_PACKET_TYPE_ICMP,      /**< [GG.D2.TM] icmp packet */
    CTC_MISC_NH_PACKET_TYPE_GRE,       /**< [GG.D2.TM] gre packet*/
    CTC_MISC_NH_PACKET_TYPE_UDPORTCP,  /**< [GG.D2.TM] tcp or udp packet */
    CTC_MISC_NH_PACKET_TYPE_MPLS,      /**< [D2.TM] Mpls packet */

    CTC_MISC_NH_PACKET_TYPE_MAX
};
typedef enum ctc_misc_nh_packet_type_e ctc_misc_nh_packet_type_t;

/**
 @brief  Define flex nexthop parameter structure
*/
struct ctc_misc_nh_flex_edit_param_s
{

    uint32       flag;                          /**< [GG.D2.TM] CTC_MISC_NH_FLEX_EDIT_XXX, ctc_misc_nh_flex_edit_flag_t */
    uint8        is_reflective;                 /**< [GG.D2.TM] if set,packet will be reflectived to ingress port */

    uint8        packet_type;                   /**< [GG.D2.TM] CTC_MISC_NH_PACKET_TYPE_XXX, ctc_misc_nh_packet_type_t */

    /*L2 Header*/
    mac_addr_t   mac_da;                        /**< [GG.D2.TM] Mac destination address */
    mac_addr_t   mac_sa;                        /**< [GG.D2.TM] Mac source address */
    uint16       ether_type;                    /**< [GG.D2.TM] Ether TYPE*/

    /*vlan edit*/
    ctc_vlan_tag_op_t      stag_op;         /**< [GG.D2.TM] operation type of stag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t      svid_sl;         /**< [GG.D2.TM] operation type of svlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t      scos_sl;         /**< [GG.D2.TM] operation type of scos, see ctc_vlan_tag_sl_t struct*/

    ctc_vlan_tag_op_t      ctag_op;         /**< [GG.D2.TM] operation type of ctag, see ctc_vlan_tag_op_t struct*/
    ctc_vlan_tag_sl_t      cvid_sl;         /**< [GG.D2.TM] operation type of cvlan id, see ctc_vlan_tag_sl_t struct*/
    ctc_vlan_tag_sl_t      ccos_sl;         /**< [GG.D2.TM] operation type of ccos, see ctc_vlan_tag_sl_t struct*/

    uint8  new_stpid_en;                    /**< [GG.D2.TM] new svid tpid enable */
    uint8 new_stpid_idx;                   /**< [GG.D2.TM] new svid tpid index */
    uint16 new_svid;                        /**< [GG.D2.TM] new svid */
    uint16 new_cvid;                        /**< [GG.D2.TM] new cvid*/
    uint8  new_scos;                        /**< [GG.D2.TM] new stag cos*/
    uint8  new_ccos;                        /**< [GG.D2.TM] new ctag cos*/
    uint16 user_vlanptr;                    /**< [GG.D2.TM] Set the dest_vlanptr of the nexthop, 0 means use src_vlan_ptr */

    /*Mpls Header*/
    ctc_mpls_nh_label_param_t label[CTC_MPLS_NH_MAX_FLEX_LABEL_NUM];  /**< [D2.TM] mpls label information*/
    uint8  label_num;                       /**< [D2.TM] mpls label number*/

    /*IP Header*/
    union
    {
        ip_addr_t   ipv4; /**< [GG.D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6; /**< [GG.D2.TM] Ip version 6 address */
    } ip_sa;              /**< [GG.D2.TM] New ip sa address in ip header*/

    union
    {
        ip_addr_t   ipv4; /**< [GG.D2.TM] Ip version 4 address */
        ipv6_addr_t ipv6; /**< [GG.D2.TM] Ip version 6 address */
    } ip_da;
    uint8  protocol;       /**< [GG.D2.TM] protocol type */
    uint8  ttl;            /**< [GG.D2.TM] ttl */
    uint8  ecn;            /**< [GG.D2.TM] ecn */
    uint8  ecn_select;     /**< [GG.D2.TM] ecn select mode, refer to  ctc_nh_ecn_select_mode_t*/
    uint8  dscp_select;    /**< [GG.D2.TM] Dscp select mode,refer to ctc_nh_dscp_select_mode_t*/
    uint8  dscp_or_tos;    /**< [GG.D2.TM] Dscp value for Ip version 4 , tos for Ip version 6 */

    uint32 flow_label;     /**< [GG.D2.TM] flow label */

    /*UDP/TCP Port*/
    uint16 l4_src_port;  /**< [GG.D2.TM] L4 source Port */
    uint16 l4_dst_port;  /**< [GG.D2.TM] L4 dest Port */

    /*GRE KEY*/
    uint32 gre_key;/**< [GG.D2.TM] GRE Key */

    /*ICMP*/
    uint8  icmp_type;   /**< [GG.D2.TM] ICMP type */
    uint8  icmp_code;    /**< [GG.D2.TM] ICMP Code */

    /*ARP*/
    uint16  arp_ht;    /**< [GG.D2.TM] hardware type */
    uint16  arp_pt;    /**< [GG.D2.TM] protocol type */
    uint8   arp_halen; /**<[GG.D2.TM] length of hardware address */
    uint8   arp_palen; /**< [GG.D2.TM]length of protocol address */
    uint16  arp_op;    /**< [GG.D2.TM]ARP/RARP operation */
    mac_addr_t   arp_sha; /**<[GG.D2.TM] sender hardware address */
    mac_addr_t   arp_tha; /**<[GG.D2.TM] target hardware address */
    uint32  arp_spa;      /**< [GG.D2.TM]sender protocol address */
    uint32  arp_tpa;      /**< [GG.D2.TM]target protocol address */

};
typedef struct ctc_misc_nh_flex_edit_param_s ctc_misc_nh_flex_edit_param_t;

/**
 @brief  Define Misc nexthop over l2 Edit flags
*/
enum ctc_misc_nh_over_l2_edit_flag_e
{
    CTC_MISC_NH_OVER_L2_EDIT_VLAN_EDIT  = 0x00000001,      /**< [GG.D2.TM] Raw packet vlan tag edit*/
};
typedef enum ctc_misc_nh_over_l2_edit_flag_e ctc_misc_nh_over_l2_edit_flag_t;

/**
 @brief  Define over l2 nexthop parameter structure
*/
struct ctc_misc_nh_over_l2_edit_param_s
{
    mac_addr_t   mac_da;                        /**< [GG.D2.TM] Mac destination address */
    mac_addr_t   mac_sa;                        /**< [GG.D2.TM] Mac source address */
    uint16       ether_type;                    /**< [GG.D2.TM] Ether TYPE*/

    uint16       vlan_id;                       /**< [GG.D2.TM] vlan id */
    uint16       flow_id;                       /**< [GG.D2.TM] flow id*/
    uint32       flag;                          /**< [GG.D2.TM] ctc_misc_nh_over_l2_edit_flag_t */
    ctc_vlan_egress_edit_info_t vlan_info;      /**< [GG.D2.TM] raw packet vlan edit information */
};
typedef struct ctc_misc_nh_over_l2_edit_param_s ctc_misc_nh_over_l2_edit_param_t;

/**
 @brief  Define flex nexthop flag
*/
enum ctc_misc_nh_type_e
{
    CTC_MISC_NH_TYPE_REPLACE_L2HDR,     /**< [GB] Replace layer2 header */
    CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR,  /**< [GB] Replace layer2 header and ipda & destport*/
    CTC_MISC_NH_TYPE_TO_CPU,            /**< [GB.GG.D2.TM] Send to CPU with specified reason */
    CTC_MISC_NH_TYPE_FLEX_EDIT_HDR,     /**< [GG.D2.TM] Flex edit layer2/Layer3/layer4 header*/
    CTC_MISC_NH_TYPE_OVER_L2_WITH_TS,    /**< [GG.D2.TM] Add layer2 header with time stamp
                                                macda+macsa+vlan+ether_type+flowid+source port+ time stamp
                                            */
    CTC_MISC_NH_TYPE_OVER_L2,           /**< [GG.D2.TM] Add only layer2 header
                                                macda+macsa+vlan+ether_type
                                            */
    CTC_MISC_NH_TYPE_LEAF_SPINE,     /**< [D2.TM] Used for LEAF to SPINE system */

};
typedef enum ctc_misc_nh_type_e ctc_misc_nh_type_t;

/**
@brief  Define flex nexthop parameter structure
*/
struct ctc_misc_nh_param_s
{

    uint8    type;             /**< [GB.GG.D2.TM] CTC_MISC_NH_TYPE_XXX */
    uint8    is_oif;               /**< [GG.D2.TM] Outgoing is interface, used for oif instand of gport */
    uint32   gport;            /**< [GB.GG.D2.TM] Dest global port id ,if is_reflective is set, the gport is invalid*/
    uint32   dsnh_offset;      /**< [GB.GG.D2.TM]DsNextHop offset */
    uint16   truncated_len;    /**< [D2.TM] The length of packet after truncate, value 0 is disable */
    ctc_nh_oif_info_t oif;        /**< [GG.D2.TM] Outgoing interface */
    uint32   stats_id;            /**< [GB.GG.D2.TM] Stats Id, 0 is disable */
    uint16   cid;                 /**< [D2.TM] category id, 0 means disable */

    union
    {
        ctc_misc_nh_l2edit_param_t     l2edit;   /**<  [GB.GG.D2.TM] L2Edit parameter */
        ctc_misc_nh_l2_l3edit_param_t  l2_l3edit;/**<  [GB] L2Edit/L3Edit parameter */
        ctc_misc_nh_cpu_reason_param_t cpu_reason;/**< [GB.GG.D2.TM] To Cpu parameter */
        ctc_misc_nh_flex_edit_param_t  flex_edit;/**<  [GG.D2.TM] Flex packet edit parameter */
        ctc_misc_nh_over_l2_edit_param_t over_l2edit;/**< [GG.D2.TM] Over l2 eidt parameter */
    }
    misc_param;
}
;
typedef struct ctc_misc_nh_param_s ctc_misc_nh_param_t;

/**
 @brief  Define mcast nexthop operation code
*/
enum ctc_mcast_nh_param_opcode_e
{
    CTC_NH_PARAM_MCAST_NONE                  = 0, /**< [GB.GG.D2.TM] None */
    CTC_NH_PARAM_MCAST_ADD_MEMBER            = 1, /**< [GB.GG.D2.TM] Add mcast member */
    CTC_NH_PARAM_MCAST_DEL_MEMBER            = 2  /**< [GB.GG.D2.TM] Del mcast member */
};
typedef enum ctc_mcast_nh_param_opcode_e ctc_mcast_nh_param_opcode_t;

/**
 @brief  Define mcast nexthop member type
*/
enum ctc_mcast_nh_param_member_type_s
{
    CTC_NH_PARAM_MEM_BRGMC_LOCAL = 0,       /**< [GB.GG.D2.TM] Bridge mcast member  */
    CTC_NH_PARAM_MEM_IPMC_LOCAL,            /**< [GB.GG.D2.TM] IP mcast member  */
    CTC_NH_PARAM_MEM_LOCAL_WITH_NH,         /**< [GB.GG.D2.TM] Mcast member with nexthop */
    CTC_NH_PARAM_MEM_REMOTE,                /**< [GB.GG.D2.TM] Remote member  */
    CTC_NH_PARAM_MEM_INVALID                /**< [GB.GG.D2.TM] Invalid mcast member  */
};
typedef enum ctc_mcast_nh_param_member_type_s ctc_mcast_nh_param_member_type_t;

/**
 @brief  Define mcast nexthop flag
*/
enum ctc_mcast_nh_flag_e
{
    CTC_MCAST_NH_FLAG_ASSIGN_PORT = 0x00000001,    /**< [GB.GG.D2.TM]Use destid as the assign port for CTC_NH_PARAM_MEM_LOCAL_WITH_NH */
    CTC_MCAST_NH_FLAG_LEAF_CHECK_EN = 0x0000002,   /**< [GG.D2.TM] Enable leaf check for CTC_NH_PARAM_MEM_IPMC_LOCAL*/
    CTC_MCAST_NH_FLAG_MAX
};
typedef enum ctc_mcast_nh_flag_e ctc_mcast_nh_flag_t;

/**
 @brief  Define mcast nexthop member parameter structure
*/
struct ctc_mcast_nh_param_member_s
{
    ctc_mcast_nh_param_member_type_t member_type;   /**< [GB.GG.D2.TM] Mcast member type */
    uint32 flag;                    /**< [GB.GG.D2.TM] Mcast nexthop flag, refer to  ctc_mcast_nh_flag_t*/
    uint32  ref_nhid;               /**< [GB.GG.D2.TM] Reference other nhid, eg egress vlan translation nhid */
    uint32  destid;                 /**< [GB.GG.D2.TM] Local member:local portid(gchip(8bit) +local phy port(8bit)), LAGID(eg.0x1F01);
                                         remote chip member: gchip(8bit) + mcast_profile_id(8bit) */
    uint16  vid;                    /**< [GB.GG.D2.TM] For IPMC */
    uint8   l3if_type;              /**< [GB.GG.D2.TM] ctc_l3if_type_t */
    uint8   is_vlan_port;           /**< [GB.GG.D2.TM] If set and  l3if is vlan interface , output packet only do L2 bridging . */
    uint8   is_reflective;          /**< [GG.D2.TM] If set ,Mcast reflective  valid except member_type is CTC_NH_PARAM_MEM_IPMC_LOCAL */
    uint8   is_source_check_dis;    /**< [GB.GG.D2.TM] If set ,Mcast packet will not check source and dest port, else check*/
    uint8   gchip_id;               /**< [GB.GG.D2.TM] global chip id, used for add/remove port bitmap member,gchip_id=0x1f means linkagg */
    uint8   port_bmp_en;            /**< [GB.GG.D2.TM] if set, means add/remove members by bitmap */
    uint8   rsv;
    uint16  logic_dest_port;        /**< [GG.D2.TM] logic dest port, not zero will do logic port check*/
    ctc_port_bitmap_t  port_bmp;    /**<[GB.GG.D2.TM] port bitmap,used for add/remove members */

};
typedef struct ctc_mcast_nh_param_member_s ctc_mcast_nh_param_member_t;

/**
 @brief  Define mcast nexthop parametes structure
*/
struct ctc_mcast_nh_param_group_s
{
    uint16                        mc_grp_id;    /**< [GB.GG.D2.TM] Identify a group */
    uint8                         opcode;       /**< [GB.GG.D2.TM] ctc_mcast_nh_param_opcode_t */
    uint8                         is_mirror;    /**< [GB.GG.D2.TM] if set,indicate the a mcast  group is applied to mirror */
    uint8                         stats_en;     /**< [GB.GG.D2.TM] if set,indicate stats is enable */
    uint8                         is_profile; /**< [D2.TM] if set,indicate mcast group is profile, can be used as mcast group member */
    ctc_mcast_nh_param_member_t   mem_info;     /**< [GB.GG.D2.TM] Mcast member info */
    uint32                        stats_id;     /**< [GB.GG.D2.TM] Stats id */
};
typedef struct ctc_mcast_nh_param_group_s ctc_mcast_nh_param_group_t;

/**
 @brief  Define nexthop information flag
*/
enum ctc_nh_info_flag_e
{
    CTC_NH_INFO_FLAG_IS_UNROV   = 0x0001, /**< [GB.GG.D2.TM] Indicate unrovle nexthop,packet will be discarded */
    CTC_NH_INFO_FLAG_IS_DSNH8W = 0x0002,  /**< [GB.GG.D2.TM] Indicate DsNexthop is double wide table */
    CTC_NH_INFO_FLAG_IS_APS_EN  = 0x0004, /**< [GB.GG.D2.TM] Indicate Aps enable */
    CTC_NH_INFO_FLAG_IS_MCAST   = 0x0010, /**< [GB.GG.D2.TM] Indicate Mcast Nexthop */
    CTC_NH_INFO_FLAG_IS_ECMP    = 0x0020  /**< [GB.GG.D2.TM] Indicate ECMP Nexthop */
};
typedef enum ctc_nh_info_flag_e ctc_nh_info_flag_t;

/**
 @brief  Define nexthop information structure
*/
struct ctc_nh_info_s
{
   uint32   flag;              /**< [GB.GG.D2.TM] CTC_NH_INFO_FLAG_XX, pls refer to ctc_nh_info_flag_t*/
   uint32   dsnh_offset[CTC_MAX_LOCAL_CHIP_NUM];  /**< DsNextHop offset */
   uint32   gport;             /**< [GB.GG.D2.TM]if CTC_NH_INFO_FLAG_IS_APS_EN set,it is aps group id;
                                        else if CTC_NH_INFO_FLAG_IS_MCAST set,mcast group id;
                                        else gport   */

   uint8    ecmp_cnt;          /*[GB.GG.D2.TM] include resoved & unresoved nexthop member*/
   uint8    valid_ecmp_cnt;    /*[GB.GG.D2.TM] valid(resoved) member*/
   uint16   tunnel_id;         /*[D2.TM] mpls tunnel id*/
   uint32   ecmp_mem_nh[CTC_MAX_ECPN]; /*[GB.GG.D2.TM] valid nh array[valid_cnt] -->unresoved nh*/
   uint8    nh_type;           /*[GG.D2.TM] nexthop type, please refer to ctc_nh_type_t */

    /*mcast  information*/
    ctc_mcast_nh_param_member_t* buffer;      /**<[GB.GG.D2.TM] [in/out] A buffer store query results */
    uint32  start_index;        /**<[GB.GG.D2.TM] [in] If it is the first query, it is equal to 0, else it is equal to the last next_query_index */
    uint32  next_query_index;   /**<[GB.GG.D2.TM] [out] return index of the next query */
    uint8   is_end;             /**<[GB.GG.D2.TM] [out] if is_end == 1, indicate the end of query */
    uint8   rsv;                /**< reserved*/
    uint16  buffer_len;         /**<[GB.GG.D2.TM] [in]multiple of sizeof(ctc_mcast_nh_param_member_t) */
    uint32  valid_number;       /**<[GB.GG.D2.TM] [out] The valid numbers */

};
typedef struct ctc_nh_info_s ctc_nh_info_t;

/**
 @brief  Define nexthop stats type
*/
enum ctc_nh_stats_type_e
{
    CTC_NH_STATS_TYPE_NHID         = 0x0001,    /**< [GB.GG.D2.TM] indicate nhid */
    CTC_NH_STATS_TYPE_TUNNELID     = 0x0002     /**< [GB.GG.D2.TM] indicate mpls tunnel id */
};
typedef enum ctc_nh_stats_type_e ctc_nh_stats_type_t;

/**
 @brief  Define nexthop stats structure
*/
struct ctc_nh_stats_info_s
{
    uint8 stats_type;           /**< [GB.GG.D2.TM] stats type, refer to ctc_nh_stats_type_t */
    uint8 is_protection_path;   /**< [GB.GG.D2.TM] protection path */
    union
    {
        uint16 nhid;            /**< [GB.GG.D2.TM] nexthop id */
        uint16 tunnel_id;       /**< [GB.GG.D2.TM] mpls tunnel id */
    }id;

    ctc_stats_basic_t stats;    /**< [GB.GG.D2.TM] output stats data */
};
typedef struct ctc_nh_stats_info_s ctc_nh_stats_info_t;


/**
 @brief  Define set nexthop drop data structure
*/
struct ctc_nh_drop_info_s
{
    uint8 drop_en;               /**< [GB] drop en, If set data  drop, else not */
    uint8 is_protection_path;    /**< [GB] protection path */
};
typedef struct ctc_nh_drop_info_s  ctc_nh_drop_info_t;


/**
 @brief  Define trill nexthop operation type emun
*/
enum ctc_nh_trill_flag_e
{
    CTC_NH_TRILL_IS_UNRSV             = 0x00000001,  /**< [GG.D2.TM] If set, nexthop is unresolved nexthop  */
    CTC_NH_TRILL_MTU_CHECK            = 0x00000002,  /**< [GG.D2.TM] If set , means mtu check enable*/
};
typedef enum ctc_nh_trill_flag_e ctc_nh_trill_flag_t;

/**
 @brief  Define trill tunnel nexthop structure
*/
struct ctc_nh_trill_param_s
{
    uint32 flag;                 /**< [GG.D2.TM] TRILL nexthop flag, refer to ctc_nh_trill_flag_t*/
    ctc_nh_upd_type_t upd_type;  /**< [GG.D2.TM] nexthop update type ,used for update Operation*/
    ctc_nh_oif_info_t oif;       /**< [GG.D2.TM] Outgoing port or interface, and outer Vlan*/
    mac_addr_t mac;              /**< [GG.D2.TM] If encap packet directly send out,it is nexthop router mac,else it is invalid */
    uint32 dsnh_offset;          /**< [GG.D2.TM] If Chipset use Egress chip Edit mode ,
                                               the dsnh_offset is allocated in platform adaption Layer;
                                               else the dsnh_offset is internally allocated in SDK */
    uint16 ingress_nickname;     /**< [GG.D2.TM] Ingress nickname of TRILL header*/
    uint16 egress_nickname;      /**< [GG.D2.TM] Egress nickname of TRILL header*/

    uint8 ttl;                   /**< [GG.D2.TM] Hop count of TRILL header*/
    uint8 rsv;
    uint16 mtu_size;             /**< [GG.D2.TM] MTU size for this nexthop */

    uint32 stats_id;             /**< [GG.D2.TM] Stats id, 0 is disable */
};
typedef struct ctc_nh_trill_param_s ctc_nh_trill_param_t;

/**
 @brief  Define UDF edit replace parameter
*/
struct ctc_nh_udf_edit_s
{
    uint8 type;                     /**< [TM]0:replace none, 1:replace data from nexthop, 2: payload length, 3: replace data from metadata, */
    uint8 offset;                   /**< [TM]replace data offset in flex edit data, unit is bit */
    uint8 length;                   /**< [TM]replace data length, unit is bits */
    uint8 rsv;
};
typedef struct ctc_nh_udf_edit_s ctc_nh_udf_edit_t;

/**
 @brief  Define UDF edit tunnel type
*/
enum ctc_nh_udf_offset_type_e
{
    CTC_NH_UDF_OFFSET_TYPE_UDP,                    /**< [TM]udf edit tunnel type : IP + UDP + UDF  */
    CTC_NH_UDF_OFFSET_TYPE_GRE,                    /**< [TM]udf edit tunnel type : IP + GRE(without key) + UDF  */
    CTC_NH_UDF_OFFSET_TYPE_GRE_WITH_KEY,           /**< [TM]udf edit tunnel type : IP + GRE(with key) + UDF  */
    CTC_NH_UDF_OFFSET_TYPE_IP,                     /**< [TM] udf edit tunnel type : IP + UDF */
    CTC_NH_UDF_OFFSET_TYPE_RAW,                    /**< [TM] udf edit tunnel type : Only have UDF data */

    CTC_NH_UDF_OFFSET_MAX_TYPE
};
typedef enum ctc_nh_udf_offset_type_e ctc_nh_udf_offset_type_t;

/**
 @brief  Define UDF edit profile parameter
*/
struct ctc_nh_udf_profile_param_s
{
    uint8  profile_id;
    uint8  offset_type;                           /**< [TM]udf offset type */
    uint16 ether_type;                            /**< [TM]if udf offset type is CTC_NH_UDF_OFFSET_RAW_TYPE, may need rewrite ether type for no-ip packet*/
    uint16 edit_length;                           /**< [TM]UDF edit data length, uint is byte */
    uint32 edit_data[CTC_NH_UDF_MAX_SIZE];        /**< [TM]UDF edit data */
    uint16 hdr_option_length;                     /**< [TM]UDF option header length, used to calculate payload length*/
    ctc_nh_udf_edit_t edit[CTC_NH_UDF_MAX_EDIT_NUM];  /**< [TM]Replace special data of UDF edit data if necessary */
};
typedef struct ctc_nh_udf_profile_param_s ctc_nh_udf_profile_param_t;
typedef int32 (* ctc_nh_mcast_traverse_fn)(uint16 mc_grp_id, ctc_mcast_nh_param_member_t* p_data, void* user_data);

struct ctc_nh_mcast_traverse_s
{
    void*   user_data;           /**< [D2.TM] User data*/
    uint8   lchip_id;            /**< [D2.TM] Traverse for specific local chip id */
    uint8   rsv[3];
};
typedef struct ctc_nh_mcast_traverse_s ctc_nh_mcast_traverse_t;


/**@} end of @defgroup  nexthop */

#ifdef __cplusplus
}
#endif

#endif /*!_CTC_NEXTHOP_H*/

