/**
 @file ctc_alloc.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-11

 @version v2.0

   This file contains all memory allocation related data structure, enum, macro and proto.
*/

#ifndef _CTC_ALLOC_H
#define _CTC_ALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define MAX_CTC_ALLOC_KEY_TYPE_SIZE (2 * MAX_CTC_ALLOC_KEY_TYPE)

/**
 @defgroup allocation ALLOCATION
 @{
*/

/**
 @brief Define allocation key location
*/
enum ctc_alloc_key_location_e
{
    CTC_ALLOC_INT_TCAM,         /**<Location in internal TCAM*/
    CTC_ALLOC_EXT_TCAM,         /**<Location in external TCAM*/
    CTC_ALLOC_HASH,             /**<Location in SRAM*/
    MAX_CTC_ALLOC_KEY_LOCATION
};
typedef enum ctc_alloc_key_location_e ctc_alloc_key_location_t;

/**
 @brief Define key type
*/
enum ctc_key_type_e
{
    CTC_ACL_IPV6_KEY = 0,       /**<Acl ipv6 key*/
    CTC_ACL_MAC_IPV4_KEY,       /**<Acl mac ipv4 mpls key*/
    CTC_IPV4_UCAST_ROUTE_KEY,   /**<Ipv4 unicast route key*/
    CTC_IPV4_MCAST_ROUTE_KEY,   /**<Ipv4 multicast route key*/
    CTC_IPV4_NAT_KEY,           /**<Ipv4 nat key*/
    CTC_IPV4_PBR_DUALDA_KEY,    /**<Ipv4 pbr key*/
    CTC_IPV6_UCAST_ROUTE_KEY,   /**<Ipv6 unicast route key*/
    CTC_USER_ID_MAC_KEY,        /**<User id mac key*/
    CTC_USER_ID_VLAN_KEY,       /**<User id vlan key*/
    CTC_IPV6_MCAST_ROUTE_KEY,   /**<Ipv6 multicast route key*/
    CTC_IPV6_NAT_KEY,           /**<Ipv6 nat key*/
    CTC_MAC_KEY,                /**<Mac key*/
    CTC_IPV6_PBR_DUALDA_KEY,    /**<Ipv6 pbr key*/
    CTC_USER_ID_IPV4_KEY,       /**<User id ipv4 key*/
    CTC_USER_ID_IPV6_KEY,       /**<User id ipv6 key*/
    CTC_QOS_MAC_IPV4_KEY,       /**<Qos mac ipv4 mpls key*/
    CTC_QOS_IPV6_KEY,           /**<Qos ipv6 key*/
    CTC_OAM_KEY,                /**<Oam key*/
    MAX_CTC_ALLOC_KEY_TYPE
};
typedef enum ctc_key_type_e ctc_key_type_t;

/**
 @brief Define key size
*/
enum ctc_key_size_e
{
    CTC_INVALID_KEY_SIZE = 0,   /**<Invalid key size*/
    CTC_KEY_SIZE_80_BIT = 1,    /**<80 bits key size*/
    CTC_KEY_SIZE_160_BIT = 2,   /**<160 bits key size*/
    CTC_KEY_SIZE_320_BIT = 4,   /**<320 bits key size*/
    CTC_KEY_SIZE_640_BIT = 8,   /**<640 bits key size*/
    MAX_CTC_KEY_SIZE
};
typedef enum ctc_key_size_e ctc_key_size_t;

/**
 @brief Define TCAM type
*/
enum ctc_alloc_tcam_type_e
{
    CTC_ALLOC_TCAM_TYPE_NL9K = 2, /**< Netlogic 9000 TCAM type*/
    MAX_CTC_ALLOC_TCAM_TYPE
};
typedef enum ctc_alloc_tcam_type_e ctc_alloc_tcam_type_t;

/**
 @brief Profile key information
*/
struct ctc_alloc_profile_key_info_s
{
    uint32 key_size;                    /**< Value = {1,2,4,8}, indicates {80b,160b,320b,640b}. */
    uint32 max_key_index;               /**< Key total number. key_max_index * key_size = consumed 80b tcam entry. */
    ctc_alloc_key_location_t key_media; /**< CTC_ALLOC_XXX*/
    ctc_key_type_t           key_id;    /**< Key type*/
};
typedef struct ctc_alloc_profile_key_info_s ctc_alloc_profile_key_info_t;

/**
 @brief Profile information
*/
struct ctc_alloc_profile_info_s
{
    ctc_alloc_profile_key_info_t* key_info; /**<Profile key information*/
    uint16 key_info_size;                   /**< Size of key_info, multiple of sizeof(ctc_alloc_profile_key_info_t) */

    /* tcam/sram configuration */
    uint8  is_ipuc_sa_enable;           /**< Enable ipuc rpf*/
    uint8  is_aclqos_dual_lookup;       /**< Enable lookup acl/qos key at the same time */
    uint8  is_merge_aclqos_mac_ip_key;  /**< Enable merge acl/qos mac/ip Key */
    uint8  is_mpls_store_ext_sram;      /**< Mpls table store in external sram or not */
    uint8  is_dyntbl_store_ext_sram;    /**< Dynamic table store in external sram or not */
    uint8  is_metnh_store_ext_sram;     /**< MET & Nexthop table store in external sram or not*/

    uint32  gbl_met_nh_tbl_size;        /**< store global MET & Nexthop table's sram size */
    uint32  local_met_nh_tbl_size;      /**< store local MET & Nexthop table's sram size */
    uint32 ip_tunnel_tbl_size;          /**< store IP-tunnel table's sram size*/
    uint32 mpls_entry_num;              /**< Record Mpls table entries defined in profile */

    /* hardware information is listed below */
    uint32 hash_physical_size;          /**< Optional hash size, 48K or 96K */
    uint32 ass_dyn_sram_size;           /**< Internal Sram for associate and dynamic table */

    uint32 external_tcam_type;          /**< External TCAM hardware type, default NL9K */
    uint32 external_tcam_size;          /**< External external tcam size, 256K */
    uint32 external_sram_size;          /**< External sram size */

    uint32 policer_tbl_size;            /**< store policer table's sram size*/
    uint32 fwd_stats_tbl_size;          /**< store forward statistics table's sram size*/

    uint8 is_ext_qdr_enable;            /**< Enable external QDR*/
};
typedef struct ctc_alloc_profile_info_s ctc_alloc_profile_info_t;

/**@} end of @defgroup allocation ALLOCATION  */

#ifdef __cplusplus
}
#endif

#endif

