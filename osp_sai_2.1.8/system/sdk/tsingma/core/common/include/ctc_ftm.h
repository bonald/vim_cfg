/**
 @file ctc_ftm.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-11

 @version v2.0

   This file contains all memory allocation related data structure, enum, macro and proto.
*/

#ifndef _CTC_FTM_H
#define _CTC_FTM_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define MAX_CTC_FTM_KEY_TYPE_SIZE (2 * CTC_FTM_KEY_TYPE_MAX)

#define CTC_FTM_SRAM_MAX (32)

/**
 @defgroup ftm FTM
 @{
*/

/**
 @brief Define key type
*/

enum ctc_ftm_key_type_e
{
    CTC_FTM_KEY_TYPE_IPV6_ACL0,           /**< [GB]Ipv6 ACL key */
    CTC_FTM_KEY_TYPE_IPV6_ACL1,           /**< [GB]Ipv6 ACL key */

    CTC_FTM_KEY_TYPE_SCL0,                 /**< [GG.D2.TM]SCL key */
    CTC_FTM_KEY_TYPE_SCL1,                 /**< [GG.D2.TM]SCL key */
    CTC_FTM_KEY_TYPE_ACL0,                 /**< [GB.GG.D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL1,                 /**< [GB.GG.D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL2,                 /**< [GB.GG.D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL3,                 /**< [GB.GG.D2.TM]ACL key include MAC , IPV4, MPLS */

    CTC_FTM_KEY_TYPE_ACL0_EGRESS,       /**< [GB.GG.D2.TM]Egress ACL key include MAC , IPV4, IPV6, MPLS */

    CTC_FTM_KEY_TYPE_IPV4_MCAST,        /**< [GB]IPV4 mcast key*/
    CTC_FTM_KEY_TYPE_IPV6_MCAST,        /**< [GB]IPV6 mcast key*/

    CTC_FTM_KEY_TYPE_VLAN_SCL,          /**< [GB]SCL VLAN key*/
    CTC_FTM_KEY_TYPE_MAC_SCL,           /**< [GB]SCL MAC key*/
    CTC_FTM_KEY_TYPE_IPV4_SCL,          /**< [GB]SCL IPv4 key*/

    CTC_FTM_KEY_TYPE_IPV6_SCL,          /**< [GB]SCL IPv6 key*/

    CTC_FTM_KEY_TYPE_FDB,               /**< [GB]FDB key*/
    CTC_FTM_KEY_TYPE_IPV4_UCAST,        /**< [GB.GG]IPv4 Ucast key*/
    CTC_FTM_KEY_TYPE_IPV6_UCAST,        /**< [GB.GG]IPv6 Ucast key*/

    CTC_FTM_KEY_TYPE_IPV4_NAT,          /**< [GB.GG.D2.TM]IPv4 Nat key*/
    CTC_FTM_KEY_TYPE_IPV6_NAT,          /**< [GB.GG]IPv6 Nat key*/
    CTC_FTM_KEY_TYPE_IPV4_PBR,          /**< [GB.GG]IPv4 PBR key*/
    CTC_FTM_KEY_TYPE_IPV6_PBR,          /**< [GB.GG]IPv6 PBR key*/

    CTC_FTM_KEY_TYPE_IPV4_TUNNEL,       /**< [GB]IPv4 Tunnel key*/
    CTC_FTM_KEY_TYPE_IPV6_TUNNEL,       /**< [GB]IPv6 Tunnel key*/

    CTC_FTM_KEY_TYPE_ACL4,              /**< [D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL5,              /**< [D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL6,              /**< [D2.TM]ACL key include MAC , IPV4, MPLS */
    CTC_FTM_KEY_TYPE_ACL7,              /**< [D2.TM]ACL key include MAC , IPV4, MPLS */

    CTC_FTM_KEY_TYPE_ACL1_EGRESS,       /**< [GB.D2.TM]Egress ACL key include MAC , IPV4, IPV6, MPLS */
    CTC_FTM_KEY_TYPE_ACL2_EGRESS,       /**< [GB.D2.TM]Egress ACL key include MAC , IPV4, IPV6, MPLS */

    CTC_FTM_KEY_TYPE_IPV6_UCAST_HALF,             /**< [D2.TM] Lpm Tcam Ipv6 half key*/
    CTC_FTM_KEY_TYPE_ACL3_EGRESS,       /**< [GB]Egress ACL key include MAC , IPV4, MPLS */

    CTC_FTM_KEY_TYPE_IPV6_ACL0_EGRESS,   /**< [GB]Egress Ipv6 ACL key */
    CTC_FTM_KEY_TYPE_IPV6_ACL1_EGRESS,   /**< [GB]Egress Ipv6 ACL key */

    CTC_FTM_KEY_TYPE_SCL2,                 /**< [TM]SCL key */
    CTC_FTM_KEY_TYPE_SCL3,                 /**< [TM]SCL key */

    CTC_FTM_KEY_TYPE_MAX
};
typedef enum ctc_ftm_key_type_e ctc_ftm_key_type_t;

/**
 @brief Define key size
*/
enum ctc_ftm_key_size_e
{
    CTC_FTM_KEY_SIZE_INVALID = 0,   /**< [GB]Invalid key size*/
    CTC_FTM_KEY_SIZE_80_BIT  = 1,   /**< [GB.D2.TM]80 bits key size*/
    CTC_FTM_KEY_SIZE_160_BIT = 2,   /**< [GB.GG.D2.TM]160 bits key size*/
    CTC_FTM_KEY_SIZE_320_BIT = 4,   /**< [GB.GG.D2.TM]320 bits key size*/
    CTC_FTM_KEY_SIZE_640_BIT = 8,   /**< [GB.GG.D2.TM]640 bits key size*/
    CTC_FTM_KEY_SIZE_MAX
};
typedef enum ctc_ftm_key_size_e ctc_ftm_key_size_t;

/**
 @brief Profile key information
*/
struct ctc_ftm_key_info_s
{
    uint8  key_size;                    /**< [GB.GG]Value = {1,2,4,8}, indicates {80b,160b,320b,640b}. */
    uint32 max_key_index;               /**< [GB]Key total number. key_max_index * key_size = consumed 80b tcam entry. */
    uint8  key_media;                   /**< [GB]ctc_ftm_key_location_t*/
    uint8  key_id;                      /**< [GB.GG]Key type*/

    uint32 tcam_bitmap;                         /**< [GG]Tcam Key tcam bitmap*/
    uint32 tcam_start_offset[CTC_FTM_SRAM_MAX]; /**< [GG]Start Offset of TCAM*/
    uint32 tcam_entry_num[CTC_FTM_SRAM_MAX];    /**< [GG]Entry number in TCAM*/
};
typedef struct ctc_ftm_key_info_s ctc_ftm_key_info_t;

/**
 @brief Define table type
*/
enum ctc_ftm_tbl_type_e
{

    CTC_FTM_TBL_TYPE_LPM_PIPE0,     /**< [GB.GG.D2.TM] LPM PIPE0 table*/

    CTC_FTM_TBL_TYPE_NEXTHOP,       /**< [GB.GG.D2.TM] Nexthop table*/
    CTC_FTM_TBL_TYPE_FWD,           /**< [GB.GG.D2.TM] Fwd table*/
    CTC_FTM_TBL_TYPE_MET,           /**< [GB.GG.D2.TM] Met table*/
    CTC_FTM_TBL_TYPE_EDIT,          /**< [GB.GG.D2.TM] l2 and l3 edit table*/

    CTC_FTM_TBL_TYPE_OAM_MEP,       /**< [GB.GG.D2.TM] All OAM table*/

    CTC_FTM_TBL_TYPE_STATS,         /**< [GB] statistics table*/
    CTC_FTM_TBL_TYPE_LM,            /**< [GB] OAM LM statistics table*/
    CTC_FTM_TBL_TYPE_SCL_HASH_KEY,  /**< [GB] SCL hash key table*/
    CTC_FTM_TBL_TYPE_SCL_HASH_AD,   /**< [GB] SCL AD table*/

    CTC_FTM_TBL_TYPE_FIB_HASH_KEY,  /**< [GB.TM] MAC table*/
    CTC_FTM_TBL_TYPE_LPM_HASH_KEY,  /**< [GB] LPM hash key table*/
    CTC_FTM_TBL_TYPE_FIB_HASH_AD,   /**< [GB.TM] MAC table*/
    CTC_FTM_TBL_TYPE_LPM_PIPE1,     /**< [GB.D2.TM] LPM PIPE1 table*/
    CTC_FTM_TBL_TYPE_LPM_PIPE2,     /**< [GB] LPM PIPE2 table*/
    CTC_FTM_TBL_TYPE_LPM_PIPE3,     /**< [GB] LPM PIPE3 table*/
    CTC_FTM_TBL_TYPE_MPLS,          /**< [GB] MPLS table*/

    CTC_FTM_TBL_TYPE_FIB0_HASH_KEY,      /**< [GG.D2.TM] MAC, IPDA key table*/
    CTC_FTM_TBL_TYPE_DSMAC_AD,           /**< [GG.D2.TM] MAC AD table*/
    CTC_FTM_TBL_TYPE_FIB1_HASH_KEY,      /**< [GG.D2.TM] NAT, IPSA key table*/
    CTC_FTM_TBL_TYPE_DSIP_AD,            /**< [GG.D2.TM] IP AD table*/

    CTC_FTM_TBL_TYPE_XCOAM_HASH_KEY,     /**< [GG.D2.TM] OAM and Egress Vlan Xlate table*/
    CTC_FTM_TBL_TYPE_FLOW_HASH_KEY,      /**< [GG.D2.TM] Flow hash key*/
    CTC_FTM_TBL_TYPE_FLOW_AD,            /**< [GG.D2.TM] Flow Ad table*/
    CTC_FTM_TBL_TYPE_IPFIX_HASH_KEY,     /**< [GG.D2.TM] IPFix hash key*/
    CTC_FTM_TBL_TYPE_IPFIX_AD,           /**< [GG.D2.TM] IPFix AD table*/
    CTC_FTM_TBL_TYPE_OAM_APS,            /**< [GG.D2.TM] APS table*/

    CTC_FTM_TBL_TYPE_SCL1_HASH_KEY,  /**< [TM] SCL1 hash key table*/
    CTC_FTM_TBL_TYPE_SCL1_HASH_AD,   /**< [TM] SCL1 AD table*/

    CTC_FTM_TBL_TYPE_MAX
};
typedef enum ctc_ftm_tbl_type_e ctc_ftm_tbl_type_t;

enum ctc_ftm_spec_e
{
    CTC_FTM_SPEC_INVALID,    
    CTC_FTM_SPEC_MAC,               /**<[GG.D2] the FDB spec>*/
    CTC_FTM_SPEC_IPUC_HOST,         /**<[GG.D2.TM] the IP Host route spec>*/
    CTC_FTM_SPEC_IPUC_LPM,          /**<[GG.D2] the IP LPM route spec>*/
    CTC_FTM_SPEC_IPMC,              /**<[GG.D2.TM] the IPMC spec>*/
    CTC_FTM_SPEC_ACL,               /**<[GG.D2] the ACL spec>*/
    CTC_FTM_SPEC_SCL_FLOW,          /**<[GG.D2.TM] the flow scl spec >*/
    CTC_FTM_SPEC_ACL_FLOW,          /**<[GG.D2] the flow acl spec>*/
    CTC_FTM_SPEC_OAM,               /**<[GG.D2] the OAM session spec>*/
    CTC_FTM_SPEC_SCL,               /**<[GG.D2.TM] the ingress SCL spec>*/
    CTC_FTM_SPEC_TUNNEL,            /**<[GG.D2.TM] IP Tunnel/ Nvgre /Vxlan spec>*/
    CTC_FTM_SPEC_MPLS,              /**<[GG.D2] the MPLS spec>*/
    CTC_FTM_SPEC_L2MC,              /**<[GG.D2.TM] the L2MC spec>*/
    CTC_FTM_SPEC_IPFIX,             /**<[GG.D2] the IPFix flow spec>*/
    CTC_FTM_SPEC_NAPT,              /**<[D2.TM]    the NAPT spec>*/
    
    CTC_FTM_SPEC_MAX
};
typedef enum ctc_ftm_spec_e ctc_ftm_spec_t;

/**
 @brief Define tble type
*/
enum ctc_ftm_flag_e
{
    CTC_FTM_FLAG_COUPLE_EN              = (1 << 10), /**< [GG.D2.TM] If set couple mode Enable*/
    CTC_FTM_FLAG_MAX
};
typedef enum ctc_ftm_flag_e ctc_ftm_flag_t;

/**
 @brief Define profile type
*/
enum ctc_ftm_profile_type_e
{
    CTC_FTM_PROFILE_0,          /**< [GB.GG]Default profile*/
    CTC_FTM_PROFILE_1,          /**< [GB]Enterprise profile*/
    CTC_FTM_PROFILE_2,          /**< [GB]Bridge profile*/
    CTC_FTM_PROFILE_3,          /**< [GB]Ipv4 host profile*/
    CTC_FTM_PROFILE_4,          /**< [GB]Ipv4 lpm profile*/
    CTC_FTM_PROFILE_5,          /**< [GB]Ipv6 profile*/
    CTC_FTM_PROFILE_6,          /**< [GB]Metro profile*/
    CTC_FTM_PROFILE_7,          /**< [GB]VPWS ridge profile*/
    CTC_FTM_PROFILE_8,          /**< [GB]VPLS profile*/
    CTC_FTM_PROFILE_9,          /**< [GB]L3VPN profile*/
    CTC_FTM_PROFILE_10,         /**< [GB]PTN profile*/
    CTC_FTM_PROFILE_11,         /**< [GB]PON profile*/

    CTC_FTM_PROFILE_USER_DEFINE,/**< [GB.GG.D2.TM]FLEX profile, accoring key_info and tbl_info*/

    CTC_FTM_PROFILE_MAX
};
typedef enum ctc_ftm_profile_type_e ctc_ftm_profile_type_t;

/**
 @brief Profile table information
*/
struct ctc_ftm_tbl_info_s
{
    uint32 tbl_id;                                                 /**[GB.GG.D2.TM]ctc_ftm_tbl_type_t*/
    uint32 mem_bitmap;                                             /**[GB.GG.D2.TM]Table allocation in which SRAM*/
    uint32 mem_start_offset[CTC_FTM_SRAM_MAX];                     /**[GB.GG.D2.TM]Start Offset of SRAM*/
    uint32 mem_entry_num[CTC_FTM_SRAM_MAX];                        /**[GB.GG.D2.TM]Entry number in SRAM*/
};
typedef struct ctc_ftm_tbl_info_s ctc_ftm_tbl_info_t;

/**
 @brief Profile misc information
*/
struct ctc_ftm_misc_info_s
{
    uint32 mcast_group_number;                           /**[GB.GG.D2.TM]The Mcast group Number*/
    uint32 glb_nexthop_number;                           /**[GB.GG.D2.TM]The nexthop Number for egress edit mode*/
    uint16 vsi_number;                                   /**[GB]The VPLS VSI number*/
    uint16 ecmp_number;                                  /**[GB]The ECMP number*/
    uint32 profile_specs[CTC_FTM_SPEC_MAX];              /**[GG.D2.TM]Profile specs*/
    uint8  ip_route_mode;                                /**[D2.TM] refer to ctc_ftm_route_mode_t*/
    uint8  scl_mode;                                     /**[TM] Scl access mode, 0: scl hash 0/1 individual, 1: hash0/1 mixed*/
    uint8  rsv[2];

};
typedef struct ctc_ftm_misc_info_s ctc_ftm_misc_info_t;

/**
 @brief Profile information
*/
struct ctc_ftm_profile_info_s
{
    uint32 flag;                                 /**< [GG.D2.TM]Ftm flag refer to ctc_ftm_flag_t*/
    ctc_ftm_key_info_t* key_info;                /**< [GB.GG.D2.TM]Profile key information*/
    uint16 key_info_size;                        /**< [GB.GG.D2.TM]Size of key_info, multiple of sizeof(ctc_ftm_key_info_t) */
    uint8 profile_type;                          /**< [GB.GG.D2.TM]Profile type, refer to ctc_ftm_profile_type_t*/
    uint8  rsv0;

    ctc_ftm_tbl_info_t *tbl_info;               /**< [GB.GG.D2.TM]table information  */
    uint16 tbl_info_size;                       /**< [GB.GG.D2.TM]Size of tbl_info, multiple of sizeof(ctc_ftm_tbl_info_t) */
    uint16 rsv1;

    ctc_ftm_misc_info_t  misc_info;             /**< [GB.GG.D2.TM]Misc info */
};
typedef struct ctc_ftm_profile_info_s ctc_ftm_profile_info_t;

enum ctc_ftm_hash_poly_type_e
{
    CTC_FTM_HASH_POLY_TYPE_0,           /**< [GG]x11+x9+1*/
    CTC_FTM_HASH_POLY_TYPE_1,           /**< [GG.D2.TM]x11+x2+1*/
    CTC_FTM_HASH_POLY_TYPE_2,           /**< [GG]x12+x+1*/
    CTC_FTM_HASH_POLY_TYPE_3,           /**< [GG.D2.TM]x12+x8+x2+x+1*/
    CTC_FTM_HASH_POLY_TYPE_4,           /**< [GG.D2.TM]x12+x6+x4+x+1*/
    CTC_FTM_HASH_POLY_TYPE_5,           /**< [GG.D2.TM]x13+x4+x3+x+1*/
    CTC_FTM_HASH_POLY_TYPE_6,           /**< [GG.D2.TM]x13+x5+x2+x+1*/
    CTC_FTM_HASH_POLY_TYPE_7,           /**< [GG.D2.TM]x13+x7+x6+x+1*/
    CTC_FTM_HASH_POLY_TYPE_8,           /**< [GG]x13+x7+x3+x+1*/
    CTC_FTM_HASH_POLY_TYPE_9,           /**< [GG]x13+x8+x6+x+1*/
    CTC_FTM_HASH_POLY_TYPE_10,          /**< [GG]x14+x5+x3+x+1*/
    CTC_FTM_HASH_POLY_TYPE_11,          /**< [GG]x14+x8+x4+x+1*/
    CTC_FTM_HASH_POLY_TYPE_12,          /**< [GG]x14+x5+x4+x3+1*/
    CTC_FTM_HASH_POLY_TYPE_13,          /**< [GG]x14+x7+x5+x3+1*/
    CTC_FTM_HASH_POLY_TYPE_14,          /**< [D2.TM]x9+x4+1*/
    CTC_FTM_HASH_POLY_TYPE_15,          /**< [D2.TM]x11+x4+x2+x+1*/
    CTC_FTM_HASH_POLY_TYPE_16,          /**< [D2.TM]x11+x5+x3+x+1*/
    CTC_FTM_HASH_POLY_TYPE_17,          /**< [D2.TM]x12+x7+x4+x3+1*/
    CTC_FTM_HASH_POLY_TYPE_18,          /**< [D2.TM]x10+x4+x3+x1+1*/
    CTC_FTM_HASH_POLY_TYPE_19,          /**< [D2.TM]x10+x5+x2+x1+1*/
    CTC_FTM_HASH_POLY_TYPE_20,          /**< [TM]x11+x6+x2+x+1*/
    CTC_FTM_HASH_POLY_TYPE_21,          /**< [TM]x11+x6+x5+x+1*/
    CTC_FTM_HASH_POLY_TYPE_22,          /**< [TM]x13+x6+x4+x+1*/
    CTC_FTM_HASH_POLY_TYPE_23,          /**< [TM]x14+x6+x4+x3+x2+x1+1*/
    CTC_FTM_HASH_POLY_TYPE_24,          /**< [TM]x14+x6+x5+x4+x3+x1+1*/
    CTC_FTM_HASH_POLY_TYPE_25,          /**< [TM]x14+x7+x5+x3+x2+x1+1*/
    CTC_FTM_HASH_POLY_TYPE_26,          /**< [TM]x14+x7+x5+x4+x3+x1+1*/



    CTC_FTM_POLY_TYPE_MAX
};
typedef enum ctc_ftm_hash_poly_type_e ctc_ftm_hash_poly_type_t;

struct ctc_ftm_hash_poly_s
{
    uint32 mem_id;                                              /**< [GG.D2.TM]hash memory id */
    uint8  poly_num;                                            /**< [GG.D2.TM]number of available hash poly corresponding to specific memory id */
    uint8  rsv[3];
    uint32 cur_poly_type;                                       /**< [GG.D2.TM]the currenttly-used hash poly type corresponding to the specific memory id*/
    uint32 poly_type[CTC_FTM_POLY_TYPE_MAX];                    /**< [GG.D2.TM]hash poly type, refer to ctc_ftm_hash_poly_type_t*/
};
typedef struct ctc_ftm_hash_poly_s ctc_ftm_hash_poly_t;

enum ctc_ftm_route_mode_e
{
    CTC_FTM_ROUTE_MODE_DEFAULT,                                        /**< [D2.TM] with rpf check, ipda and ipsa share one ram, no linerate*/
    CTC_FTM_ROUTE_MODE_WITHOUT_RPF_LINERATE,                           /**< [D2.TM] without rpf check, linerate */
    CTC_FTM_ROUTE_MODE_WITH_RPF_LINERATE,                              /**< [D2.TM] with rpf check, linerate*/
    CTC_FTM_ROUTE_MODE_WITH_PUB_LINERATE,                              /**< [D2.TM] support public route, without rpf check, linerate*/
    CTC_FTM_ROUTE_MODE_WITH_PUB_AND_RPF,                               /**< [D2.TM] support public route and rpf check, no linerate*/
};
typedef enum ctc_ftm_route_mode_e ctc_ftm_route_mode_t;

enum ctc_ftm_mem_change_mode_e
{
    CTC_FTM_MEM_CHANGE_NONE,            /**< [TM] No change memory*/
    CTC_FTM_MEM_CHANGE_MAC,             /**< [TM] Only change mac table*/
    CTC_FTM_MEM_CHANGE_REBOOT,          /**< [TM] Change memory by reboot*/
    CTC_FTM_MEM_CHANGE_RECOVER          /**< [TM] Change memory and recover basic config*/
};
typedef enum ctc_ftm_mem_change_mode_e ctc_ftm_mem_change_mode_t;
struct ctc_ftm_change_profile_s
{
    uint8 recover_en;                                                  /**< [TM] if set, sdk will recover basic config such as port,vlan,l3if*/
    ctc_ftm_profile_info_t* old_profile;                               /**< [TM] Must be exactly the same as the current memory profile*/
    ctc_ftm_profile_info_t* new_profile;                               /**< [TM] New memory profile which to be changed*/
    uint8 change_mode;                                                 /**< [TM] Out change mode, refer to ctc_ftm_mem_change_mode_t */
};
typedef struct ctc_ftm_change_profile_s ctc_ftm_change_profile_t;
/**@} end of @defgroup ftm FTM  */

#ifdef __cplusplus
}
#endif

#endif
