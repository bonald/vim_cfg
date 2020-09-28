/****************************************************************************
* $Id$
*  Centec STM(Switch Table Management) related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Xianghong Gu
* Date          : 2010-07-06 14:36
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_STM_DEFINE_H__
#define __GLB_STM_DEFINE_H__
#include "glb_hw_define.h"
#ifndef CNOS_OS
#ifdef UDET2
#include "glb_acl_service_define.h"
#endif
#endif
/***************************************************************************************
*system reserved number define, GLB_SYS_RESV_xxxx
****************************************************************************************/
/*reserved number define*/
#define GLB_SYS_RESV_UCAST_FDB               53
#define GLB_SYS_RESV_HOST_ROUTE              0 
#define GLB_SYS_RESV_REMOTE_ROUTE            40 /* 33 block + 3(martion addr)+1(default entry)+. */
#define GLB_SYS_RESV_MAC_BASED_VLAN_CLASS    1
#define GLB_SYS_RESV_IPV4_BASED_VLAN_CLASS   1
#define GLB_SYS_RESV_IPV6_BASED_VLAN_CLASS   1
#define GLB_SYS_RESV_IPV4_SOURCE_GUARD       1
#define GLB_SYS_RESV_TCAM_GLB_PBR_TAIL        1
/*modified by wangl for bug 20994 20121027*/
/*modified by wangl for bug 21388 20121123*/
#define GLB_STM_RESERVED_IPV6_QOS_RULE_ENTRY_EXTERNAL_TCAM_NUMBER  3
#define GLB_STM_RESERVED_IPV6_QOS_RULE_ENTRY_INTERNAL_TCAM_NUMBER  1
/*For humber system, 1 entry for global default entry, 64 for per port default entry
modified by chengw for bug 17094 , 2011-12-07, sdk reserved 2 + 64 for normal vlan userid*/
#define GLB_SYS_RESV_VLAN_MAPPING            66
/*modify by chenxw for bug 25358, 2013-10-17.*/
#define GLB_SYS_RESV_DEFAULT_QOS_RULE        1 /*MAC permit any; ip permit any; ip deny tiny fragment*/
#define GLB_SYS_RESV_L2MC                    0
/* in ASIC, we reserve 2 entries,
one is default entry; the other is (*,G) because even if there is any  (*,G) entry,
we still need to reserve at least one entry for the algrithm purpose. except them, there may be
a few other applications which will use IPMC entry. e.g. VRRP. Considering those entries,
so we have to reserve more entries for them. Now it's 4.*/
/*modified by dingyi for bug21941, vrrp do not need special group now,
modify the resv l3mc number to 2, one for default entry and other for algrithm purpose, 2013-01-31*/
  /* modified by chenxw for ipmc resource mgr, bug 24095, 2013-09-12. */      
#define GLB_SYS_RESV_L3MC                    1
#define GLB_SYS_RESV_HOST_ROUTE_V6           0 
#define GLB_SYS_RESV_REMOTE_ROUTE_V6         135 /* 129 block + 2(martion addr) + 1(default entry)+. */
/*=========================FOR SRAM reserved===========================*/
#define GLB_SYS_RESV_SRAM_DSFWD              1536 /*1322=64 (ACL redirect)+544+714(port nh reserve)*/
#define GLB_SYS_RESV_SRAM_NH                 96 /*87=16(sdk reserve)+6 (loopback nh)+1(mpls tp nh)+64 tunnel decap nh*/
#define GLB_SYS_RESV_SRAM_MET                64 /*48=1(VLAN 1 default entry)+47(local met for vlan 1 member port)*/
#define GLB_SYS_RESV_SRAM_L2EDIT             0
#define GLB_SYS_RESV_SRAM_L3EDIT             0
#define GLB_SYS_RESV_SRAM_MPLS               0
/* modified by liwh for bug 17404 to support cfm lck&csf,
  global 3 acl entries, 32 acl entries are used by voice vlan */
#define GLB_SYS_RESV_CFM_LCK                 3
/*icmpv4 to cpu, icmpv6 to cpu, natv4 default entry, natv6 default entry, for 
bug 21227 */
#define GLB_SYS_RESV_IVI_PEER                5
#define GLB_NH_ID_BLOCK_SIZE 1024
/*==================for software database========================================*/
#define GLB_ARP_CACHE_SIZE   24576 /*modified by ychen for bug 15264 in 2011-07-19*/
#define GLB_NEIGH_CACHE_SIZE 24576

#define GLB_ARP_CACHE_MAX   512 /*modified by wangjj for syn with kernel arp tbl in 2016-03-02*/

#define GLB_TAP_GROUP_SIZE   (16*1024)

/* when allocate 1024 eth oam key resource, only can support 1000 meps,
therefore change from 1024 to 1000 */
#define GLB_CFM_MAX_MEP_RMEP_SIZE  1000
/*========================for stm parse=========================================*/
#define GLB_STM_PROFILE_PATH    SWITCH_PREFIX"/etc/memory_profile/"
#define GLB_STM_PROFILE_MODE    "tcam_mode.txt"
#define GLB_STM_PROFILE_PATH_UML    "/mnt/flash/"
/***************************************************************************************
*system soft profile maxinum define, GLB_SPEC_PM_xxxx
****************************************************************************************/
#define GLB_STM_SRAM_MAX (32)   /* CTC_FTM_SRAM_MAX */
#define GLB_FLEX_FTM_TYPE 12 /*CTC_FTM_PROFILE_USER_DEFINE*/
#define GLB_STM_TUNNEL_IPSA_NUM  8 /*added by ychen in 2013-11-20 for bug 24232*/
#define GLB_OPENFLOW_SELECT_GROUP_MAX 63

enum glb_stm_mode_e
{
  GLB_STM_MODE_DEFAULT = 0,      /*0  default*/
  GLB_STM_MODE_ACCESS,           /*1  access*/
  GLB_STM_MODE_IPUCV4,           /*2  ipucv4*/
  GLB_STM_MODE_VLAN,             /*3  vlan*/  
  GLB_STM_MODE_IPUCV6,           /*4  ipucv6 routing */
  GLB_STM_MODE_NVGRE,            /*5 nvgre, NVGRE_DEV, L2VPN is not supported according 25517. */
  GLB_STM_MODE_L2VPN,            /*6  l2vpn */
  /* modify for vxlan by chenxw, 2014-05-29. */
  // TODO: later we may support overlay profile, and overlay can only work under overlay profile
  GLB_STM_MODE_OVERLAY,          /*7  overlay */
  /* Modified by Yan Xing'an for OpenFlow, 2014-12-08 */
  GLB_STM_MODE_OF_DEFAULT,       /*8  default, standalone */
  GLB_STM_MODE_OF_HYBRID,        /*9  hybrid */
  GLB_STM_MODE_OF_NVCLOUD,       /*10 nvcloud */
  GLB_STM_MODE_OF_TAP,           /*11 tap */
  GLB_STM_MODE_OF_IPV6,
  GLB_STM_MODE_TAP_IPV4_IPV6,		/* 13 tap-350 support flow-ipv6 */
  /* for bug 29455 modified by liuynag 29455*/
  GLB_STM_MODE_MAX = GLB_STM_MODE_TAP_IPV4_IPV6 + 1, /*temp modified by ychen in 2013-10-28 for bug 25517*/
};
typedef enum glb_stm_mode_e glb_stm_mode_t;

enum glb_stm_extend_flag_e
{
    GLB_STM_EXTEND_FLAG_COUPLE = 0x1,           /*  all couple mode */
};
typedef enum glb_stm_extend_flag_e glb_stm_extend_flag_t;

#define GLB_STM_MODE_STR(profile)         \
    ((profile) == GLB_STM_MODE_DEFAULT    ? "default" : \
     (profile) == GLB_STM_MODE_ACCESS     ? "access" : \
     (profile) == GLB_STM_MODE_IPUCV4     ? "ipucv4" : \
     (profile) == GLB_STM_MODE_VLAN       ? "vlan" : \
     (profile) == GLB_STM_MODE_IPUCV6     ? "ipucv6" : \
     (profile) == GLB_STM_MODE_NVGRE      ? "nvgre" : \
     (profile) == GLB_STM_MODE_L2VPN      ? "l2vpn" : \
     (profile) == GLB_STM_MODE_OVERLAY    ? "overlay" : \
     (profile) == GLB_STM_MODE_OF_DEFAULT ? "openflow-default" : \
     (profile) == GLB_STM_MODE_OF_HYBRID  ? "openflow-hybrid" : \
     (profile) == GLB_STM_MODE_OF_NVCLOUD ? "openflow-nvcloud" : \
     (profile) == GLB_STM_MODE_OF_IPV6 ? "openflow-ipv6" : \
     (profile) == GLB_STM_MODE_OF_TAP     ? "tap" : \
     "unknown")

enum glb_stm_key_location_e
{
    GLB_STM_INT_TCAM,         /**< [HB.GB]Location in internal TCAM*/
    GLB_STM_EXT_TCAM,         /**< [HB]Location in external TCAM*/
    GLB_STM_HASH,             /**< [HB]Location in SRAM*/
    GLB_STM_LOCATION_MAX
};
typedef enum glb_stm_key_location_e glb_stm_key_location_t; /**< ctc_ftm_key_location_t */

enum glb_stm_key_type_e
{
    GLB_STM_KEY_TYPE_IPV6_ACL0,         /**< [GB]Ipv6 ACL key */
    GLB_STM_KEY_TYPE_IPV6_ACL1,         /**< [GB]Ipv6 ACL key */

    GLB_STM_KEY_TYPE_ACL0,              /**< [GB]ACL key include MAC , IPV4, MPLS */
    GLB_STM_KEY_TYPE_ACL1,              /**< [GB]ACL key include MAC , IPV4, MPLS */
    GLB_STM_KEY_TYPE_ACL2,              /**< [GB]ACL key include MAC , IPV4, MPLS */
    GLB_STM_KEY_TYPE_ACL3,              /**< [GB]ACL key include MAC , IPV4, MPLS */

    GLB_STM_KEY_TYPE_IPV4_MCAST,        /**< [GB]IPV4 mcast key*/
    GLB_STM_KEY_TYPE_IPV6_MCAST,        /**< [GB]IPV6 mcast key*/

    GLB_STM_KEY_TYPE_VLAN_SCL,          /**< [GB]SCL VLAN key*/
    GLB_STM_KEY_TYPE_MAC_SCL,           /**< [GB]SCL MAC key*/
    GLB_STM_KEY_TYPE_IPV4_SCL,          /**< [GB]SCL IPv4 key*/

    GLB_STM_KEY_TYPE_IPV6_SCL,          /**< [GB]SCL IPv6 key*/

    GLB_STM_KEY_TYPE_FDB,               /**< [GB]FDB key*/
    GLB_STM_KEY_TYPE_IPV4_UCAST,        /**< [GB.GG]IPv4 Ucast key*/
    GLB_STM_KEY_TYPE_IPV6_UCAST,        /**< [GB.GG]IPv6 Ucast key*/

    GLB_STM_KEY_TYPE_IPV4_NAT,          /**< [GB.GG]IPv4 Nat key*/
    GLB_STM_KEY_TYPE_IPV6_NAT,          /**< [GB.GG]IPv6 Nat key*/
    GLB_STM_KEY_TYPE_IPV4_PBR,          /**< [GB.GG]IPv4 PBR key*/
    GLB_STM_KEY_TYPE_IPV6_PBR,          /**< [GB.GG]IPv6 PBR key*/

    GLB_STM_KEY_TYPE_IPV4_TUNNEL,       /**< [GB]IPv4 Tunnel key*/
    GLB_STM_KEY_TYPE_IPV6_TUNNEL,       /**< [GB]IPv6 Tunnel key*/

    /*huangxt for ggsys profile*/
    GLB_STM_KEY_TYPE_SCL0_160,          /**< [GG] SCL0, 160 Bit*/ 
    GLB_STM_KEY_TYPE_SCL0_320,          /**< [GG] SCL0, 320 Bit*/ 
    GLB_STM_KEY_TYPE_SCL0_640,          /**< [GG] SCL0, 640 Bit*/ 
    GLB_STM_KEY_TYPE_SCL1_160,          /**< [GG] SCL1, 160 Bit*/ 
    GLB_STM_KEY_TYPE_SCL1_320,          /**< [GG] SCL1, 320 Bit*/ 
    GLB_STM_KEY_TYPE_SCL1_640,          /**< [GG] SCL1, 640 Bit*/ 

    GLB_STM_KEY_TYPE_ACL0_IPE_160,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL0_IPE_320,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL0_IPE_640,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL1_IPE_160,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL1_IPE_320,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL1_IPE_640,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL2_IPE_160,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL2_IPE_320,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL2_IPE_640,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL3_IPE_160,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL3_IPE_320,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL3_IPE_640,      /**< [GG] */ 

    GLB_STM_KEY_TYPE_ACL0_EPE_160,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL0_EPE_320,      /**< [GG] */ 
    GLB_STM_KEY_TYPE_ACL0_EPE_640,      /**< [GG] */ 

    GLB_STM_KEY_TYPE_MAX
};
typedef enum glb_stm_key_type_e glb_stm_key_type_t; /**< ctc_ftm_key_type_t */

enum glb_stm_key_size_e
{
    GLB_STM_INVALID_KEY_SIZE = 0, /**<Invalid key size*/
    GLB_STM_KEY_SIZE_80_BIT = 1, /**<80 bits key size*/
    GLB_STM_KEY_SIZE_160_BIT = 2, /**<160 bits key size*/
    GLB_STM_KEY_SIZE_320_BIT = 4, /**<320 bits key size*/
    GLB_STM_KEY_SIZE_640_BIT = 8, /**<640 bits key size*/
    MAX_GLB_STM_KEY_SIZE
};
typedef enum glb_stm_key_size_e glb_stm_key_size_t; /**< ctc_ftm_key_size_t */

#ifdef GREATBELT
enum glb_stm_tbl_type_e
{
    GLB_STM_TBL_TYPE_FIB_HASH_KEY,  /**< [GB] MAC, IP key table*/
    GLB_STM_TBL_TYPE_SCL_HASH_KEY,  /**< [GB] SCL hash key table*/
    GLB_STM_TBL_TYPE_LPM_HASH_KEY,  /**< [GB] LPM hash key table*/

    GLB_STM_TBL_TYPE_FIB_HASH_AD,   /**< [GB] MAC, IP AD table*/
    GLB_STM_TBL_TYPE_SCL_HASH_AD,   /**< [GB] SCL AD table*/
    GLB_STM_TBL_TYPE_LPM_PIPE1,     /**< [GB] LPM PIPE1 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE2,     /**< [GB] LPM PIPE2 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE0,     /**< [GB] LPM PIPE0 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE3,     /**< [GB] LPM PIPE3 table*/

    GLB_STM_TBL_TYPE_MPLS,          /**< [GB] MPLS table*/

    GLB_STM_TBL_TYPE_NEXTHOP,       /**< [GB] Nexthop table*/
    GLB_STM_TBL_TYPE_FWD,           /**< [GB.GG] Fwd table*/
    GLB_STM_TBL_TYPE_MET,           /**< [GB.GG] Met table*/
    GLB_STM_TBL_TYPE_EDIT,          /**< [GB.GG] l2 and l3 edit table*/

    GLB_STM_TBL_TYPE_OAM_MEP,       /**< [GB.GG] All OAM table*/

    GLB_STM_TBL_TYPE_STATS,         /**< [GB.GG] statistics table*/
    GLB_STM_TBL_TYPE_LM,            /**< [GB.GG] OAM LM statistics table*/

    GLB_STM_TBL_TYPE_FIB0_HASH_KEY,      /**< [D2] MAC, IPDA key table*/
    GLB_STM_TBL_TYPE_DSMAC_AD,           /**< [D2] MAC AD table*/
    GLB_STM_TBL_TYPE_FIB1_HASH_KEY,      /**< [D2] NAT, IPSA key table*/
    GLB_STM_TBL_TYPE_DSIP_AD,            /**< [D2] IP AD table*/
    
    GLB_STM_TBL_TYPE_XCOAM_HASH_KEY,     /**< [D2] OAM and Egress Vlan Xlate table*/
    GLB_STM_TBL_TYPE_FLOW_HASH_KEY,      /**< [D2] Flow hash key*/
    GLB_STM_TBL_TYPE_FLOW_AD,            /**< [D2] Flow Ad table*/
    GLB_STM_TBL_TYPE_IPFIX_HASH_KEY,     /**< [D2] IPFix hash key*/
    GLB_STM_TBL_TYPE_IPFIX_AD,           /**< [D2] IPFix AD table*/
    GLB_STM_TBL_TYPE_OAM_APS,            /**< [D2] APS table*/

    GLB_STM_TBL_TYPE_MAX
};
typedef enum glb_stm_tbl_type_e glb_stm_tbl_type_t;
#else
enum glb_stm_tbl_type_e
{
    GLB_STM_TBL_TYPE_FIB_HASH_KEY,  /**< [GB] MAC, IP key table*/
    GLB_STM_TBL_TYPE_SCL_HASH_KEY,  /**< [GB.GG] SCL hash key table*/
    GLB_STM_TBL_TYPE_LPM_HASH_KEY,  /**< [GB] LPM hash key table*/

    GLB_STM_TBL_TYPE_FIB_HASH_AD,   /**< [GB] MAC, IP AD table*/
    GLB_STM_TBL_TYPE_SCL_HASH_AD,   /**< [GB.GG] SCL AD table*/
    GLB_STM_TBL_TYPE_LPM_PIPE1,     /**< [GB] LPM PIPE1 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE2,     /**< [GB] LPM PIPE2 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE0,     /**< [GB.GG] LPM PIPE0 table*/
    GLB_STM_TBL_TYPE_LPM_PIPE3,     /**< [GB] LPM PIPE3 table*/

    GLB_STM_TBL_TYPE_MPLS,          /**< [GB] MPLS table*/

    GLB_STM_TBL_TYPE_NEXTHOP,       /**< [GB.GG] Nexthop table*/
    GLB_STM_TBL_TYPE_FWD,           /**< [GB.GG] Fwd table*/
    GLB_STM_TBL_TYPE_MET,           /**< [GB.GG] Met table*/
    GLB_STM_TBL_TYPE_EDIT,          /**< [GB.GG] l2 and l3 edit table*/

    GLB_STM_TBL_TYPE_OAM_MEP,       /**< [GB.GG] All OAM table*/

    GLB_STM_TBL_TYPE_STATS,         /**< [GB.GG] statistics table*/
    GLB_STM_TBL_TYPE_LM,            /**< [GB.GG] OAM LM statistics table*/
    
    GLB_STM_TBL_TYPE_FIB0_HASH_KEY,      /**< [GG] MAC, IPDA key table*/
    GLB_STM_TBL_TYPE_DSMAC_AD,           /**< [GG] MAC AD table*/
    GLB_STM_TBL_TYPE_FIB1_HASH_KEY,      /**< [GG] NAT, IPSA key table*/
    GLB_STM_TBL_TYPE_DSIP_AD,            /**< [GG] IP AD table*/
    
    GLB_STM_TBL_TYPE_XCOAM_HASH_KEY,     /**< [GG] OAM and Egress Vlan Xlate table*/
    GLB_STM_TBL_TYPE_FLOW_HASH_KEY,      /**< [GG] Flow hash key*/
    GLB_STM_TBL_TYPE_FLOW_AD,            /**< [GG] Flow Ad table*/
    GLB_STM_TBL_TYPE_IPFIX_HASH_KEY,     /**< [GG] IPFix hash key*/
    GLB_STM_TBL_TYPE_IPFIX_AD,           /**< [GG] IPFix AD table*/
    GLB_STM_TBL_TYPE_OAM_APS,            /**< [GG] APS table*/
    
    GLB_STM_TBL_TYPE_MAX
};
typedef enum glb_stm_tbl_type_e glb_stm_tbl_type_t;
#endif /* !GOLDENGATE */

enum glb_stm_flag_fea_ctrl_e
{
    GLB_CTRL_DUAL_ACL              = (1 << 0), /**< Enable lookup acl/qos key at the same time */
    GLB_CTRL_MERGE_ACLQOS_MAC_IP   = (1 << 1), /**< Enable merge acl/qos mac/ip Key */
    GLB_CTRL_URPF_EN               = (1 << 2), /**<Enable ipuc rpf*/
    GLB_CTRL_IPV4_EN               = (1 << 3), 
    GLB_CTRL_IPV6_EN               = (1 << 4), 
    GLB_CTRL_IPMCV4_EN             = (1 << 5), 
    GLB_CTRL_IPMCV6_EN             = (1 << 6), 
    GLB_CTRL_ETHOAM_EN             = (1 << 7),
    GLB_CTRL_MPLS_EN               = (1 << 8),
    GLB_CTRL_ACL_EN                = (1 << 9),
    GLB_CTRL_QOS_EN                = (1 << 10),
    GLB_CTRL_ACLV6_EN              = (1 << 11),
    GLB_CTRL_QOSV6_EN              = (1 << 12),
    GLB_CTRL_PBR_EN                = (1 << 13),
    GLB_CTRL_PBRV6_EN              = (1 << 14),
    GLB_CTRL_TUNNEL_EN             = (1 << 15),
    GLB_CTRL_NAT_EN                = (1 << 16), /*Now only IVI*/
    GLB_CTRL_FLOW_V6_FULL_LKP      = (1 << 17),
    GLB_CTRL_NVGRE_EN              = (1 << 18), /* NVGRE_DEV. */
    GLB_CTRL_MAX,
};
typedef enum glb_stm_flag_fea_ctrl_e glb_stm_flag_fea_ctrl_t;

struct glb_stm_profile_key_info_s
{
    uint32 key_size;                        /**< [HB.GB.GG] Value = {1,2,4,8}, indicates {80b,160b,320b,640b}. glb_stm_key_size_t */
    uint32 max_key_index;                   /**< [HB.GB] Key total number. key_max_index * key_size = consumed 80b tcam entry. */
    glb_stm_key_location_t key_media;       /**< [HB.GB] glb_stm_key_location_t */
    glb_stm_key_type_t           key_id;    /**< [HB.GB.GG] Key type. glb_stm_key_type_t*/
    
    /*huangxt for ggsys profile*/
    uint32 tcam_bitmap;                         /**< [GG]Tcam Key tcam bitmap*/
    uint32 tcam_start_offset[GLB_STM_SRAM_MAX]; /**< [GG]Start Offset of TCAM*/
    uint32 tcam_entry_num[GLB_STM_SRAM_MAX];    /**< [GG]Entry number in TCAM*/
};
typedef struct glb_stm_profile_key_info_s glb_stm_profile_key_info_t;   /**< ctc_ftm_key_info_t */

struct glb_stm_tbl_info_s
{
    uint32  tbl_id;                             /**[GB]ctc_ftm_tbl_type_t*/
    uint32 mem_bitmap;                          /**[GB]Table allocation in which SRAM*/
    uint32 mem_start_offset[GLB_STM_SRAM_MAX];  /**[GB]Start Offset of SRAM*/
    uint32 mem_entry_num[GLB_STM_SRAM_MAX];     /**[GB]Entry number in SRAM*/
};
typedef struct glb_stm_tbl_info_s glb_stm_tbl_info_t;   /**< ctc_ftm_tbl_info_t */

#ifdef GREATBELT
struct glb_stm_profile_info_s
{
    glb_stm_profile_key_info_t* key_info; /**<Profile key information*/
    uint16 key_info_size;                /**< Size of key_info, multiple of sizeof(ctc_alloc_profile_key_info_t) */  

    glb_stm_tbl_info_t *tbl_info;               /**< [GB]table information  */
    uint16 tbl_info_size;                       /**< [GB]Size of tbl_info, multiple of sizeof(ctc_ftm_tbl_info_t) */

    uint32 mcast_group_number;                  /**[GB]The Mcast group Number*/
    uint32 glb_nexthop_number;                  /**[GB]The nexthop Number for egress edit mode*/
    uint16 vsi_number;                          /**[GB]The VPLS VSI number*/
    uint16 ecmp_number;                         /**[GB]The ECMP number*/ 
    uint16 bitmap_repli_enabled_vlan_number;    /**[GB] NVGRE_DEV. bitmap replication enabled vlan count. */
};
typedef struct glb_stm_profile_info_s glb_stm_profile_info_t;
#else
enum glb_ftm_spec_e
{
    GLB_FTM_SPEC_INVALID,
    GLB_FTM_SPEC_MAC,               /**<[GG] the FDB spec>*/
    GLB_FTM_SPEC_IPUC_HOST,         /**<[GG] the IP Host route spec>*/
    GLB_FTM_SPEC_IPUC_LPM,          /**<[GG] the IP LPM route spec>*/
    GLB_FTM_SPEC_IPMC,              /**<[GG] the IPMC spec>*/
    GLB_FTM_SPEC_ACL,               /**<[GG] the ACL spec>*/
    GLB_FTM_SPEC_SCL_FLOW,          /**<[GG] the flow scl spec >*/
    GLB_FTM_SPEC_ACL_FLOW,          /**<[GG] the flow acl spec>*/
    GLB_FTM_SPEC_OAM,               /**<[GG] the OAM session spec>*/
    GLB_FTM_SPEC_SCL,               /**<[GG] the SCL spec>*/
    GLB_FTM_SPEC_TUNNEL,            /**<[GG] IP Tunnel/ Nvgre /Vxlan spec>*/
    GLB_FTM_SPEC_MPLS,              /**<[GG] the MPLS spec>*/
    GLB_FTM_SPEC_VRF,               /**<[GG] the vrf spec>*/
    GLB_FTM_SPEC_L2MC,              /**<[GG] the L2MC spec>*/
    GLB_FTM_SPEC_FID,               /**<[GG] the FID spec>*/
    GLB_FTM_SPEC_IPFIX,             /**<[GG] the IPFix flow spec>*/

    GLB_FTM_SPEC_MAX
};
typedef enum glb_ftm_spec_e glb_ftm_spec_t; /**<[GG] ctc_ftm_spec_t >*/


struct glb_stm_profile_info_s
{
    glb_stm_profile_key_info_t* key_info;       /**< [GG] Profile key information*/
    uint16 key_info_size;                       /**< [GG] Size of key_info, multiple of sizeof(ctc_alloc_profile_key_info_t) */  

    glb_stm_tbl_info_t *tbl_info;               /**< [GB.GG]table information  */
    uint16 tbl_info_size;                       /**< [GB.GG]Size of tbl_info, multiple of sizeof(ctc_ftm_tbl_info_t) */

    uint32 mcast_group_number;                  /**[GB.GG]The Mcast group Number*/
    uint32 glb_nexthop_number;                  /**[GB.GG]The nexthop Number for egress edit mode*/
    uint16 vsi_number;                          /**[GB]The VPLS VSI number*/
    uint16 ecmp_number;                         /**[GB]The ECMP number*/ 
    
    uint32  gbl_met_nh_tbl_size;   /**< store global MET & Nexthop table's sram size */
    uint32  local_met_nh_tbl_size; /**< store local MET & Nexthop table's sram size */
    uint32  mpls_entry_num;        /**< Record Mpls table entries defined in profile */
    /*Modified by zhangdy for bug 19974, 2012-7-25*/
    uint32 mpls_static_label_block_num; /*static label block number*/
    /*End of modification by zhangdy, 2012-7-25*/

    uint32 vlan_num;
    /* modified by liwh for bug 20241, 2012-08-17 */
    uint32 vlan_member_port_size;
    uint32 cfm_mep_rmep_number;
    uint32 cfm_mep_lm_number;
    uint32 g8031_group_number;
    uint32 g8032_ring_number;
    uint32 g8032_member_port_number;
    uint32 l2mc_member_port_number;
    uint32 l3mc_member_port_number;
    uint32 cfm_lck;

    uint32 flex_ac_entry_number;

    uint32 mac_num;
    uint32 tcam_mac_num;
    uint32 ipv4_num;
    uint32 ipv6_num;
    uint32 lsp_num;
    uint32 pw_num;
    uint32 vsi_num;
    uint32 vrf_num;
    uint32 l2_mcast_num;
    uint32 l3_mcast_num;
    uint32 arp_num;
    uint32 l2_l3_vpn_intf_num;
    uint32 tcam_scl_ipv4_num;

    uint32 ipv6_acl_num;
    uint32 tcam_acl_num;
    uint32 hash_mac_acl_num;
    uint32 hash_ipv4_acl_num;
    uint32 user_id_mac_num;
    uint32 user_id_vlan_num;
    uint32 user_id_ipv4_num;
    uint32 ip_source_guard_percent;
    uint32 ip_based_vlan_class_percent;
    uint32 user_id_ipv6_num;
    uint32 ipv6_source_guard_percent;
    uint32 ipv6_based_vlan_class_percent;

    /* modified by yaom for service queue profile 20140911 */
    uint32 service_queue_num;

    //glb_stm_assign_tbl_info_t assign_info[GLB_STM_ASSIGN_TBL_MAX]; /*assigned table percent*/

    /*specs*/
    uint32 profile_specs[GLB_FTM_SPEC_MAX];     /**[GG]Profile specs*/
};
typedef struct glb_stm_profile_info_s glb_stm_profile_info_t;
#endif /* !GOLDENGATE */

/***************************************************************************************
*SYSTEM FEATUARE SPECIFICATION DEFINE
****************************************************************************************/
enum glb_stm_fea_list_e 
{
    GLB_UCAST_FDB              = 0,
    GLB_VLAN_INSTANCE          = 1,
    GLB_MAC_FILTER             = 2,
    GLB_MAC_BASED_VLAN_CLASS   = 3,
    GLB_IPV4_BASED_VLAN_CLASS  = 4,
    GLB_IPV4_SOURCE_GUARD      = 5,
    GLB_VLAN_MAPPING           = 6,
    GLB_REMOTE_ROUTEV4         = 7,
    GLB_HOST_ROUTEV4           = 8,
    GLB_ECMP_ROUTEV4           = 9,
    GLB_PBRV4                  = 10,
    GLB_L2MC                   = 11,
    GLB_L3MC                   = 12,
    GLB_VOICE_VLAN             = 13,
    GLB_FLOW_ENTRY             = 14,
    GLB_CFM_CHANNEL            = 15,
    GLB_CFM_LCK                = 16,
    GLB_G8031_GROUPS           = 17,
    GLB_G8032_RINGS            = 18,
    GLB_FTN                    = 19,
    GLB_ILM                    = 20,
    GLB_VPWS                   = 21,
    GLB_LSP_PE                 = 22,
    GLB_LSP_P                  = 23,
    GLB_VPLS_PEER              = 24,
    GLB_VPLS_AC                = 25,
    GLB_VSI                    = 26,
    GLB_LSP_OAM                = 27,
    GLB_PW_OAM                 = 28,
    GLB_MPLS_APS_TUNNEL        = 29,
    GLB_IPV6_BASED_VLAN_CLASS  = 30,
    GLB_IPV6_SOURCE_GUARD      = 31,   
    GLB_REMOTE_ROUTEV6         = 32,
    GLB_HOST_ROUTEV6           = 33,
    GLB_ECMP_ROUTEV6           = 34,
    GLB_PBRV6                  = 35,
    GLB_L3MC_V6                = 36,  
    GLB_FLOW_ENTRY_V6          = 37,
    GLB_IP_TUNNEL              = 38,
    GLB_IVI_PEERS              = 39,
    GLB_L2MC_V6                = 40,
    GLB_MPLS_LABLES            = 41,
    /*added by ychen in 2013-11-04 for bug 24232*/
    GLB_BFD_SESSIONS           = 42,
    GLB_DOT1X_MAC              = 43, /*Add by huangxt for 802.1X mac based*/
    GLB_NVGRE_PEERS            = 44, /* NVGRE_DEV. */
    GLB_NVGRE_VLAN_ID_MAPPING  = 45,
    GLB_NVGRE_TUNNELS          = 46,
    GLB_FEA_LIST_MAX,    
};
typedef enum glb_stm_fea_list_e glb_stm_fea_list_t;

enum glb_stm_assit_list_e
{   
    GLB_MEM_PORT_VLAN    = 0,
    GLB_MEM_PORT_L2MC    = 1,
    GLB_MEM_PORT_L3MC    = 2,
    GLB_MEM_PORT_L3MC_v6 = 3,
    GLB_MEM_PORT_G8032   = 4,    
    GLB_MEM_PORT_L2MC_V6 = 5,
    GLB_STM_ASSIT_LIST_MAX,
};
typedef enum glb_stm_assit_list_e glb_stm_assit_list_t;

struct glb_system_fea_spec_s {
    /*layer2 features*/
    uint32 ucast_fdb;
    uint16 vlan_instance;
    uint16 mac_filter;
    uint16 mac_based_vlan_class;
    uint16 ipv4_based_vlan_class;
    uint16 ipv4_source_guard;
    uint16 vlan_mapping_entry_applied_port;    
    uint16 dot1x_mac;/*Add by huangxt for 802.1X mac based*/

    /*ipv4 ucast*/
    uint32 remote_routes;
    uint16 host_routes;     
    uint16 ecmp_routes;

    /*ipv4 mcast*/
    uint16 pbr_entries; 
    uint16 l2mc_entries;    
    uint16 l3mc_entries;
    uint8 l2mc_member_ports;
    uint8 l3mc_member_ports;

    /*layer2 features*/    
    uint8 vlan_member_ports;     
    uint8 voice_vlan;

    /*acl and qos*/
    uint16 flow_entries_ingress;
    uint16 flow_entries_egress;

    /*cfm and g8031, g8032*/
    uint16 cfm_local_and_remote_meps;
    uint16 g8031_groups;
    uint16 g8032_rings;  
    uint16 g8032_member_ports_per_ring;

    /*mpls*/
    uint16 ftn_entries;
    uint16 ilm_entries;
    uint16 mpls_lables;

    /*l2vpn*/
    uint16 vpws;
    uint16 lsp_pe;
    uint16 lsp_p;
    uint16 vpls_peer; /*vpls_pw*/
    uint16 vpls_ac;
    uint16 vsi;
    uint16 lsp_oam;
    uint16 pw_oam; /*lsp_pw*/
    uint16 mpls_aps_tunnel; /*map-route&SPME*/   
    uint16 cfm_lck;
    /*ipv6 & nat*/   
    uint16 ip_tunnel; /*v6 in v4, ip tunnel shares the key spacce with ipuc remote key*/
    uint16 ivi_peers; /*NAT, should use NAT4 and NAT6 key*/
    uint16 host_v6_routes;     
    uint16 ecmp_v6_routes;
    uint16 pbr_v6_entries; 
    uint16 l3mc_v6_entries;
    uint16 l2mc_v6_entries;
    uint16 flow_v6_entries;
    uint16 ipv6_based_vlan_class;
    uint16 ipv6_source_guard;       
    uint32 remote_v6_routes;
    uint8  l3mc_v6_member_ports;  
    uint8  l2mc_v6_member_ports;
    uint8  vlan_mapping_default_entry; 
    uint8  bfd_sessions; /*added by ychen for bug 24232 in 2013-11-20*/

    /*nvgre*/
    uint16  nvgre_tunnel_id_mappings; /* limitted by tunnel id identify entry(used to identify tenant). */
    uint16  nvgre_peers; /* limitted by ip sa identify entry(used to identify peer). */
    /* Note: if later we support more peers, we must consider the met replication limit
       (hard limit is 127), and enforce a limit on per-vlan peer numbers. */
    uint16  nvgre_tunnels; /* limitted by total met number and gre encap nexthop number. */
};
typedef struct glb_system_fea_spec_s glb_system_fea_spec_t;

struct glb_stm_resouce_info_s
{
    uint32 ctrl_flag;  /*feature control flag, glb_stm_flag_fea_ctrl_t*/

    uint32 glb_nh;   
    uint32 glb_met;

    uint32 openflow_glb_nh;
    uint32 openflow_glb_met;
    /** |                             32 bit                             |
        | linkagg interface tid range  | openflow select group tid range |
    */
    uint16 openflow_linkagg_interface_max_tid;
    uint16 rsv_1;

    uint32 ucast_mac; /*ucast fdb*/
    uint32 mcast_mac; /*mcast fdb*/
    uint32 mpls; /*ilm and vpls , vpws*/
    uint32 vlan; /*vlan instance*/
    uint16 max_v4ecmp_groups; /*added by ychen in 2012-04-06*/
    uint16 max_v6ecmp_groups; /*added by ychen in 2012-04-06*/
    uint16 ivi_peers; /*added by for IVI*/
    uint16 nvgre_peers; /* NVGRE_DEVS. */
};
typedef struct glb_stm_resouce_info_s glb_stm_resouce_info_t;

struct glb_chip_info_s
{
    uint8 max_gchip;
    uint8 gchip_0;
    uint8 gchip_1;
    uint8 local_chip_num;

    uint8 port_mapping_table[56];
    uint8 port_mapping_en;
    uint8 profile_type;
    uint8 nh_dedit_mode;

    uint8 queue_num_per_network_port;
    uint8 queue_num_per_static_int_port;
    uint8 queue_num_per_fabric;
    uint8 max_internal_port_id;

    uint8 queue_num_per_internal_port;
    uint8 board_type;
    uint8 rsv1[2];

    uint16 service_num;
    uint8 queue_num_per_ingress_service;
    uint8 queue_num_per_egress_service;

    uint8 fdb_hw_learning_mode;
    uint8 logic_port_use_hw_learning;
    uint16 logic_port_num;
    uint8 fdb_no_sw_table;
    uint8 rsv2[3];
};
typedef struct glb_chip_info_s glb_chip_info_t;

struct glb_sdk_phy_maping_s
{
    uint16 mac_idx;                  /* mac index of ASIC chip */
    uint16 phy_addr;                  /* phy_addr for mido scan */
    uint16 mdio_bus_addr;             /* mdio_bus_addr for mido scan */
};
typedef struct glb_sdk_phy_maping_s glb_sdk_phy_maping_t;

/*Jqiu modify for universal datapath. 2011-02-22*/
/* SDK init need stm profile info and datapath info. */
struct glb_sdk_init_info_s
{
    //void* stm_profile_info; /*stm profile info, glb_stm_profile_info_t*/
    void* ftm_profile_info; /*stm profile info, glb_stm_profile_info_t*/
    //glb_chip_info_t* g_chip_info;   
    void* datapath_config_file;  /*datapath config file name*/
    //void* datapath_config_file2;  /*datapath config file name*/
    //uint8 ptp_quanta;
    uint8 gchip_id;
    //uint8 dal_access_type;/*dal_access_type_t*/// TODO: new add variable;
    uint8 ftm_profile_type;     /* glb_stm_mode_t */
    uint16 logic_slot_no;
    //uint8 lchip_num;
    uint16 phy_slot_no;
    //uint16 super_slot_no;
    /* qicx added for different board has different ASIC interrupt handling, 2013-07-04 */
    glb_board_type_t board_type;    
    uint8 stacking_en;
    uint8 stacking_version;
    //uint8 ptp_en;
    /*added by ychen in 2013-11-04 for bug 24232*/
    //uint16 mep_1ms_num;
    uint8 stacking_sup_slot; /*added by ychen in 2014-01-21 for bug 26942*/
    uint8 stm_mode; /* NVGRE_DEV. */
    //int gb_irq[4]; /* add for get dynamic-allocated irq numbers for GB chip, qicx, 2013-07-27 */
    //uint8 nid_1g_xg_mode_flag; /* flag for configure nid box to 1g mode or xg mode
    //                              default value: 0 means 1g mode; 
    //                              value 1 means xg mode */
    uint16 port_num;
    glb_sdk_phy_maping_t* phy_map;
#ifndef CNOS_OS
#ifdef DUET2
    uint8 acl_glb_props[GLB_ACL_PROP_MAX];
    glb_qos_service_t acl_srv_info[GLB_BOTH_DIRECTION][GLB_QOS_SRV_MAX];                                  
#endif    
#endif
};
typedef struct glb_sdk_init_info_s glb_sdk_init_info_t;

#endif /*!__GLB_STM_DEFINE_H__*/
