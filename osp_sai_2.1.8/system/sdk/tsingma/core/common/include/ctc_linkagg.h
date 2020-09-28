/**
 @file ctc_linkagg.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-12-27

 @version v2.0

This file contains all linkagg related data structure, enum, macro and proto.

*/
#ifndef _CTC_LINKAGG_H
#define _CTC_LINKAGG_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup linkagg LINKAGG
 @{
*/
/**
 @brief  Define the mode of linkagg group
*/
enum ctc_linkagg_mode_e
{
    CTC_LINKAGG_MODE_4 = 0,     /**< [GB.GG.D2.TM] linkAgg group num is 4*/
    CTC_LINKAGG_MODE_8,         /**< [GB.GG.D2.TM] linkAgg group num is 8*/
    CTC_LINKAGG_MODE_16,        /**< [GB.GG.D2.TM] linkAgg group num is 16*/
    CTC_LINKAGG_MODE_32,        /**< [GB.GG.D2.TM] linkAgg group num is 32*/
    CTC_LINKAGG_MODE_64,        /**< [GB.GG.D2.TM] linkAgg group num is 64, this mode not support dlb*/
    CTC_LINKAGG_MODE_56,        /**< [GG.D2.TM] linkAgg group num is 56, only group 0-7 can be used for dlb*/
    CTC_LINKAGG_MODE_FLEX       /**< [D2.TM] linkagg group num is flexible */
};
typedef enum ctc_linkagg_mode_e  ctc_linkagg_mode_t;

/**
 @brief linkagg global config information
*/
struct ctc_linkagg_global_cfg_s
{
    uint8 linkagg_mode; /**< [GB.GG.D2.TM] ctc_linkagg_mode_t*/
    uint8 bind_gport_disable; /**< [D2.TM] If set indicate the gport won't be configured automatically when the member port is added. */
    uint8 rsv0[2];
};
typedef struct ctc_linkagg_global_cfg_s  ctc_linkagg_global_cfg_t;

/**
 @brief linkagg group flag enum
*/
enum ctc_linkagg_group_flag_e
{
    CTC_LINKAGG_GROUP_FLAG_RANDOM_RR = 0x00000001,          /**< [GG.D2.TM] use random rr, useful for mode CTC_LINKAGG_MODE_RR*/
    CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST = 0x00000002, /*< [D2.TM] local member first forwarding in stacking, valid for mode CTC_LINKAGG_MODE_STATIC, CTC_LINKAGG_STATIC_FAILOVER*/
    CTC_LINKAGG_GROUP_FLAG_MEM_ASCEND_ORDER = 0x00000004,        /*< [GB.GG.D2.TM] if set, indicate members of the group will be sorted in ascend order*/
    MAX_CTC_LINKAGG_GROUP_FLAG
};
typedef enum ctc_linkagg_group_flag_e ctc_linkagg_group_flag_t;

/**
 @brief linkagg group property enum
*/
enum ctc_linkagg_property_e
{
    CTC_LINKAGG_PROP_LB_HASH_OFFSET         /**< [TM] linkagg load balance hash offset */
};
typedef enum ctc_linkagg_property_e ctc_linkagg_property_t;

/**
 @brief linkagg group struct
*/
struct ctc_linkagg_group_s
{
    uint8 tid;          /**< [GB.GG.D2.TM] linkagg group id*/
    uint8 linkagg_mode; /**< [GB.GG.D2.TM] linkagg mode, refer to ctc_linkagg_group_mode_t*/
    uint16 flag;        /**< [GB.GG.D2.TM] linkagg flag, refer to ctc_linkagg_group_flag_t*/

    uint16 member_num;    /**< [D2.TM] indicate member num in this group, valid linkagg_mode is CTC_LINKAGG_MODE_FLEX */
    uint8 rsv[2];
};
typedef struct ctc_linkagg_group_s  ctc_linkagg_group_t;

/**
 @brief linkagg group mode enum
*/
enum ctc_linkagg_group_mode_e
{
    CTC_LINKAGG_MODE_STATIC,             /**< [GB.GG.D2.TM] static linkagg mode*/
    CTC_LINKAGG_MODE_STATIC_FAILOVER,    /**< [GB.GG.D2.TM] static linkagg enable failover*/
    CTC_LINKAGG_MODE_RR,                 /**< [GG.D2.TM] packet round robin linkagg mode*/
    CTC_LINKAGG_MODE_DLB,                /**< [GB.GG.D2.TM] Dynamic Load Balance linkagg mode*/
    CTC_LINKAGG_MODE_RESILIENT,          /**< [D2.TM] Resilient hash linkagg mode */
};
typedef enum ctc_linkagg_group_mode_e ctc_linkagg_group_mode_t;

/**
 @brief  ethernet hash flags
*/
/* Port Selection Criteria. -- PSC */
enum ctc_linkagg_psc_l2_header_e
{
    CTC_LINKAGG_PSC_L2_VLAN          = 0x00000001,       /**< [GB.GG.D2.TM] linkagg hash key includes vlan id*/
    CTC_LINKAGG_PSC_L2_COS           = 0x00000002,       /**< [GB.GG.D2.TM] linkagg hash key includes cos*/
    CTC_LINKAGG_PSC_L2_ETHERTYPE     = 0x00000004,       /**< [GB.GG.D2.TM] linkagg hash key includes ethertype*/
    CTC_LINKAGG_PSC_L2_DOUBLE_VLAN   = 0x00000008,       /**< [GB.GG.D2.TM] if set, linkagg hash key includes double vlan for double vlan's packet, else only includes outer vlan*/
    CTC_LINKAGG_PSC_L2_MACSA         = 0x00000010,       /**< [GB.GG.D2.TM] use macsa to compute hash*/
    CTC_LINKAGG_PSC_L2_MACDA         = 0x00000020,       /**< [GB.GG.D2.TM] use macda to compute hash*/
    CTC_LINKAGG_PSC_L2_PORT          = 0x00000040        /**< [GB.GG.D2.TM] use gport to compute hash*/
};
typedef enum  ctc_linkagg_psc_l2_header_e  ctc_linkagg_psc_l2_header_t;

/**
 @brief  pbb linkagg hash flags
*/
enum  ctc_linkagg_psc_pbb_header_e
{
    CTC_LINKAGG_PSC_PBB_BCOS          = 0x00000001,    /**< [GB.GG.D2] use stag and ctag cos to compute hash flags*/
    CTC_LINKAGG_PSC_PBB_BVLAN         = 0x00000002,    /**< [GB.GG.D2] use stag and ctag vlan id to compute hash flags*/
    CTC_LINKAGG_PSC_PBB_BMACSA        = 0x00000004,    /**< [GB.GG.D2] use macsa to compute cmac hash flags*/
    CTC_LINKAGG_PSC_PBB_BMACDA        = 0x00000008,    /**< [GB.GG.D2] use macda to compute cmac hash flags*/
    CTC_LINKAGG_PSC_PBB_ISID          = 0x00000010,    /**< [GG.D2] use isid to compute hashr*/
    CTC_LINKAGG_PSC_PBB_BCFI          = 0x00000040     /**< [GG.D2] use stag adnd ctag cfi to compute hash*/
};
typedef enum  ctc_linkagg_psc_pbb_header_e  ctc_linkagg_psc_pbb_header_t;

/**
 @brief  ip hash flags
*/
enum  ctc_linkagg_psc_ip_header_e
{
    CTC_LINKAGG_PSC_USE_IP            = 0x00000001,    /**< [GB] use ip header to compute hash flags */
    CTC_LINKAGG_PSC_IP_PROTOCOL       = 0x00000002,    /**< [GB.GG.D2.TM] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_IP_IPSA           = 0x00000004,    /**< [GB.GG.D2.TM] use ipsa to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_IP_IPDA           = 0x00000008,     /**< [GB.GG.D2.TM] use ipda to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_IP_DSCP           = 0x00000010,     /**<[GG.D2.TM] use dscp to compute hash flags*/
    CTC_LINKAGG_PSC_IPV6_FLOW_LABEL   = 0x00000020,     /**<[GG.D2.TM] use ipv6 flow label to compute hash flags*/
    CTC_LINKAGG_PSC_IP_ECN            = 0x00000040      /**<[GG.D2.TM] use ecn to compute hash flags*/
};
typedef enum  ctc_linkagg_psc_ip_header_e  ctc_linkagg_psc_ip_header_t;

/**
 @brief  layer4 hash flags
*/
enum  ctc_linkagg_psc_l4_header_e
{
    CTC_LINKAGG_PSC_USE_L4              = 0x00000001,     /**< [GB] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_L4_SRC_PORT         = 0x00000002,     /**< [GB.GG.D2.TM] use source port hash flags*/
    CTC_LINKAGG_PSC_L4_DST_PORT         = 0x00000004,     /**< [GB.GG.D2.TM] use dest port to compute hash flags*/
    CTC_LINKAGG_PSC_L4_GRE_KEY          = 0x00000008,     /**< [GB.GG.D2.TM] use gre key to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_TYPE             = 0x00000010,     /**< [GG.D2.TM] use layer4 type to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_USER_TYPE        = 0x00000020,     /**< [GG.D2.TM] use layer4 user type to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_VXLAN_L4_SRC_PORT   = 0x00000040,     /**< [GG.D2.TM] use vxlan layer4 source port to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_TCP_FLAG         = 0x00000080,     /**< [GG.D2.TM] use layer4 tcp flag to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_TCP_ECN          = 0x00000100,     /**< [GG.D2.TM] use layer4 ecn flag to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_NVGRE_VSID       = 0x00000200,     /**< [GG.D2.TM] use layer4 nvgre vsid to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_NVGRE_FLOW_ID    = 0x00000400,     /**< [GG.D2.TM] use layer4 nvgre flow id to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_L4_VXLAN_VNI        = 0x00000800      /**< [GG.D2.TM] use layer4 vxlan vni to compute linkagg hash flags*/
};
typedef enum  ctc_linkagg_psc_l4_header_e  ctc_linkagg_psc_l4_header_t;

/**
 @brief  fcoe hash flags
*/
enum  ctc_linkagg_psc_fcoe_header_e
{
    CTC_LINKAGG_PSC_USE_FCOE  = 0x00000001,     /**< [GB] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_FCOE_SID = 0x00000002,      /**< [GB.GG.D2.TM] use sid to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_FCOE_DID = 0x00000004       /**< [GB.GG.D2.TM] use did to compute linkagg hash flags*/
};
typedef enum  ctc_linkagg_psc_fcoe_header_e   ctc_linkagg_psc_fcoe_header_t;

/**
 @brief  trill hash flags
*/
enum  ctc_linkagg_psc_trill_header_e
{
    CTC_LINKAGG_PSC_USE_TRILL    = 0x00000001,              /**< [GB] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_TRILL_INGRESS_NICKNAME = 0x00000002,    /**< [GB.GG.D2.TM] use ingress nickname to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_TRILL_EGRESS_NICKNAME   = 0x00000004,   /**< [GB.GG.D2.TM] use egress nickname to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_TRILL_INNER_VID = 0x00000008            /**< [GG.D2.TM] use inner vlan id to compute linkagg hash flags*/
};
typedef enum  ctc_linkagg_psc_trill_header_e   ctc_linkagg_psc_trill_header_t;

/**
 @brief  mpls hash flags
*/
enum  ctc_linkagg_psc_mpls_header_e
{
    CTC_LINKAGG_PSC_USE_MPLS = 0x00000001,        /**< [GB] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_MPLS_PROTOCOL = 0x00000002,   /**< [GB.GG.D2.TM] use protocol to compute hash flags*/
    CTC_LINKAGG_PSC_MPLS_IPSA = 0x00000004,       /**< [GB.GG.D2.TM] use ipsa to compute linkagg hash flags*/
    CTC_LINKAGG_PSC_MPLS_IPDA = 0x00000008        /**< [GB.GG.D2.TM] use ipda to compute linkagg hash flags*/
};
typedef enum  ctc_linkagg_psc_mpls_header_e   ctc_linkagg_psc_mpls_header_t;

/**
 @brief  common hash flags
*/
enum ctc_linkagg_psc_common_type_e
{
    CTC_LINKAGG_PSC_COMMON_DEVICEINFO     = 0x00000001,   /**<[D2.TM] use deviceinfo to compute linkagg hash flags, solve hash polarization problem */
};
typedef enum ctc_linkagg_psc_common_type_e ctc_linkagg_psc_common_type_t;

/**
 @brief  select to config hash type flags
*/
enum ctc_linkagg_psc_type_e
{
    CTC_LINKAGG_PSC_TYPE_L2     = 0x00000001,   /**< [GB.GG.D2.TM] layer2 hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_IP     = 0x00000002,   /**< [GB.GG.D2.TM] layer3 ip hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_L4     = 0x00000004,   /**< [GB.GG.D2.TM] layer4 hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_PBB    = 0x00000008,   /**< [GB.GG.D2] layer2 pbb hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_MPLS   = 0x00000010,   /**< [GB.GG.D2.TM] layer3 mpls hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_FCOE   = 0x00000020,   /**< [GB.GG.D2.TM] layer3 fcoe hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_TRILL  = 0x00000040,   /**< [GB.GG.D2.TM] layer3 trill hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_COMMON  = 0x00000080,  /**< [D2.TM] common hash ctl flags*/
    CTC_LINKAGG_PSC_TYPE_UDF     = 0x00000100,  /**< [D2.TM] udf hash ctl flags,per udf entry control*/

    /* following is hash type control */
    CTC_LINKAGG_PSC_TYPE_INNER  = 0x01000000,    /**< [GG.D2.TM] if set, config inner hash key*/
};
typedef enum ctc_linkagg_psc_type_e ctc_linkagg_psc_type_t;

/**
 @brief  hash ctl fields
*/
struct ctc_linkagg_psc_s
{
    uint32  psc_type_bitmap;          /**< [GB.GG.D2.TM] psc type hash bitmap*/
    uint32  udf_id;                   /**< [D2.TM] UDF ID, is the key of udf_bitmap*/
    uint32  udf_bitmap;               /**< [D2.TM] bitmap control UDF field hash key£¬1 bit control 8 bit of udf field,total udf field is 128*/
    uint32  l2_flag;                  /**< [GB.GG.D2.TM] layer2 hash ctl flag*/
    uint32  ip_flag;                  /**< [GB.GG.D2.TM] layer3 ip hash ctl flag*/
    uint32  l4_flag;                  /**< [GB.GG.D2.TM] layer4 hash ctl flag*/
    uint32  pbb_flag;                 /**< [GB.GG.D2] layer2 pbb hash ctl flag*/
    uint32  mpls_flag;                /**< [GB.GG.D2.TM] layer3 mpls hash ctl flag*/
    uint32  fcoe_flag;                /**< [GB.GG.D2.TM] layer3 fcoe hash ctl flag*/
    uint32  trill_flag;               /**< [GB.GG.D2.TM] layer3 trill hash ctl flag*/
    uint32  common_flag;              /**<[D2.TM] Common hash ctl flag*/
};
typedef struct ctc_linkagg_psc_s ctc_linkagg_psc_t;

#ifdef __cplusplus
}
#endif

#endif
/**@} end of @defgroup   linkagg LINKAGG */

