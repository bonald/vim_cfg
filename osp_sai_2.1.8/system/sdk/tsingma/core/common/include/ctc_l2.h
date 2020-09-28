/**
 @file ctc_l2.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-26

 @version v2.0

   This file contains all L2 related data structure, enum, macro and proto.
*/

#ifndef _CTC_L2_H
#define _CTC_L2_H
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
 @defgroup fdb FDB
 @{
*/

/**
 @brief  define l2 FDB entry flags
*/

enum ctc_l2_flag_e
{
    CTC_L2_FLAG_IS_STATIC         = 0x00000001,             /**< [GB.GG.D2.TM][uc] unicast FDB,if set, indicate the entry is static entry, using for MAC DA*/
    CTC_L2_FLAG_DISCARD           = 0x00000002,             /**< [GB.GG.D2.TM][uc,mc] unicast/mcast static FDB, indicate the entry is discard entry,using for MAC DA*/
    CTC_L2_FLAG_SRC_DISCARD       = 0x00000004,             /**< [GB.GG.D2.TM][uc,mc] unicast/mcast static  FDB, indicate the entry is discard entry, using for MAC SA*/
    CTC_L2_FLAG_SRC_DISCARD_TOCPU = 0x00000008,             /**< [GB.GG.D2.TM][uc,mc] unicast/mcast static FDB, indicate the entry is discard entry and packet will send to CPU, using for MAC SA*/
    CTC_L2_FLAG_COPY_TO_CPU       = 0x00000010,             /**< [GB.GG.D2.TM][uc,mc] unicast/mcast static FDB, indicate packet will copy to CPU, using for MAC DA*/
    CTC_L2_FLAG_AGING_DISABLE     = 0x00000020,             /**< [GB.GG.D2.TM][uc] unicast dynamic FDB if set, indicate the entry will disable aging, but can do learning check*/
    CTC_L2_FLAG_PROTOCOL_ENTRY    = 0x00000040,             /**< [GB.GG.D2.TM][uc,mc] unicast/Mcast FDB,indicate the entry is protocal entry should send to CPU when match, using for MAC DA*/
    CTC_L2_FLAG_BIND_PORT         = 0x00000080,             /**< [GB.GG.D2.TM][uc] unicast FDB, MAC binding port/MAC +vlan binding port,when source mismatch should discard, using for MAC SA*/
    CTC_L2_FLAG_REMOTE_DYNAMIC    = 0x00000200,             /**< [GB.GG.D2.TM][uc] unicast dynamic FDB, if set, indicate the entry is remote chip learning dynamic entry, using for MAC DA*/
    CTC_L2_FLAG_SYSTEM_RSV        = 0x00000400,             /**< [GB.GG.D2.TM][uc] unicast static FDB, if set, indicate the entry is system mac, it can't be deleted by flush api, using for MAC DA*/
    CTC_L2_FLAG_UCAST_DISCARD     = 0x00001000,             /**< [GB][uc] unicast static FDB, if set, indicate the output packet will be discard for unicast*/
    CTC_L2_FLAG_KEEP_EMPTY_ENTRY  = 0x00002000,             /**< [GG.D2.TM][mc] Remove all member. api behavior. not action*/
    CTC_L2_FLAG_PTP_ENTRY         = 0x00010000,             /**< [GB][uc,mc] FDB,ptp mac address, using for MAC DA*/
    CTC_L2_FLAG_SELF_ADDRESS      = CTC_L2_FLAG_PTP_ENTRY,  /**< [GG.D2.TM][uc,mc] indicate that mac addres is switch's */
    CTC_L2_FLAG_WHITE_LIST_ENTRY  = 0x00020000,             /**< [GB.GG.D2.TM] unicast FDB with nexthop, if set, indicate this entry won't do aging but will do learning*/
    CTC_L2_FLAG_REMOVE_DYNAMIC    = 0x00040000,             /**< [GG.D2.TM] Only remove dynamic entry, using for MAC SA*/
    CTC_L2_FLAG_ASSIGN_OUTPUT_PORT    = 0x00080000,         /**< [GG.D2.TM] DestId assigned by user*/
    CTC_L2_FLAG_LEARN_LIMIT_EXEMPT    = 0x00100000,         /**< [GG] unicast FDB, if set, indicate the entry exempt from learn limit. Supported in sw learning mode.*/
    CTC_L2_FLAG_PENDING_ENTRY         = 0x00400000,         /**< [GG.D2.TM] unicast FDB, indicate the entry is pending entry, used for getting conflict entry*/

    MAX_CTC_L2_FLAG
};
typedef enum ctc_l2_flag_e ctc_l2_flag_t;



/**
 @brief  define l2 FDB entry sync flags
*/
enum ctc_l2_sync_msg_flag_e
{
    CTC_L2_SYNC_MSG_FLAG_IS_LOGIC_PORT      = 0x00000001, /**< [GB] If set, indicate gport is logic port, else is normal port*/
    CTC_L2_SYNC_MSG_FLAG_IS_AGING           = 0x00000002  /**< [GB] If set, indicate entry is aging sync, else is learning sync*/
};
typedef enum ctc_l2_sync_msg_flag_e ctc_l2_sync_msg_flag_t;


/**
 @brief define vlan property flags
*/
enum ctc_l2_dft_vlan_flag_e
{
    CTC_L2_DFT_VLAN_FLAG_UNKOWN_UCAST_DROP     = 0x00000001,   /**< [GB] Unknown unicast discard in specific VLAN*/
    CTC_L2_DFT_VLAN_FLAG_UNKOWN_MCAST_DROP     = 0x00000002,   /**< [GB] Unknown multicast discard in specific VLAN*/
    CTC_L2_DFT_VLAN_FLAG_PROTOCOL_EXCP_TOCPU   = 0x00000004,   /**< [GB,GG.D2.TM] Protocol exception to cpu in in specific VLAN*/
    CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT        = 0x00000010,   /**< [GB.GG.D2.TM] Learning use logic-port per fid*/
    CTC_L2_DFT_VLAN_FLAG_LEARNING_DISABLE      = 0x00000020,   /**< [GB.GG.D2.TM] learning disable per default entry*/
    CTC_L2_DFT_VLAN_FLAG_ASSIGN_OUTPUT_PORT    = 0x00000040,   /**< [GG.D2.TM] DestId assigned by user, this is used for member*/
    CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTH         = 0x00000100,   /**< [GB.GG.D2.TM] Port base MAC authentication, used for macsa */
    CTC_L2_DFT_VLAN_FLAG_VLAN_MAC_AUTH         = 0x00000200,   /**< [GB.GG.D2.TM] Vlan base MAC authentication, used for macsa */

    MAX_CTC_L2_DFT_VLAN_FLAG
};
typedef enum ctc_l2_dft_vlan_flag_e ctc_l2_dft_vlan_flag_t;


/**
 @brief [GG] Create fid property flag param
*/
enum ctc_l2_fid_property_e
{
    CTC_L2_FID_PROP_LEARNING,                    /**< [GG.D2.TM] Learning enable per fid*/
    CTC_L2_FID_PROP_IGMP_SNOOPING,               /**< [GG.D2.TM] Igmp snooping enable per fid*/
    CTC_L2_FID_PROP_DROP_UNKNOWN_UCAST,          /**< [GG.D2.TM] Drop unknown ucast packet enable per fid*/
    CTC_L2_FID_PROP_DROP_UNKNOWN_MCAST,          /**< [GG.D2.TM] Drop unknown mcast packet enable per fid*/
    CTC_L2_FID_PROP_DROP_BCAST,                  /**< [GG.D2.TM] Drop bcast packet enable per fid*/
    CTC_L2_FID_PROP_UNKNOWN_MCAST_COPY_TO_CPU,   /**< [GG.D2.TM] Unknown mcast packet copy to cpu enable*/
    CTC_L2_FID_PROP_UNKNOWN_UCAST_COPY_TO_CPU,   /**< [D2.TM] Unknown ucast packet copy to cpu enable*/
    CTC_L2_FID_PROP_BCAST_COPY_TO_CPU,           /**< [D2.TM] Bcast packet copy to cpu enable*/
    CTC_L2_FID_PROP_IGS_STATS_EN,                /**< [GG] Enable/Disable ingress vsi stats. value is stats_id, 0 means disable*/
    CTC_L2_FID_PROP_EGS_STATS_EN,                /**< [GG] Enable/Disable egress vsi stats. value is stats_id, 0 means disable*/
    CTC_L2_FID_PROP_BDING_MCAST_GROUP,           /**< [TM] Binding unknown mcast, unknown ucast to mcast group, the mcast group must be sequence */
};
typedef enum ctc_l2_fid_property_e  ctc_l2_fid_property_t;


/**
 @brief  define l2 FDB entry type flags
*/
enum ctc_l2_fdb_entry_op_type_e
{
    CTC_L2_FDB_ENTRY_OP_BY_VID,       /**< [GB.GG.D2.TM] operation BY FID(VLAN/VSI)*/
    CTC_L2_FDB_ENTRY_OP_BY_PORT,      /**< [GB.GG.D2.TM] operation BY PORT(VLAN/VSI)*/
    CTC_L2_FDB_ENTRY_OP_BY_MAC,       /**< [GB.GG.D2.TM] operation BY MA(VLAN/VSI)*/
    CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN, /**< [GB.GG.D2.TM] operation BY PORT + FID(VLAN/VSI)*/
    CTC_L2_FDB_ENTRY_OP_BY_MAC_VLAN,  /**< [GB.GG.D2.TM] operation BY MAC+FID(VLAN/VSI)*/
    CTC_L2_FDB_ENTRY_OP_ALL,          /**< [GB.GG.D2.TM] operation BY ALL(VLAN/VSI)*/
    /* SYSTEM MODIFIED by liwh for bug 37200, 2016-08-25 */
    CTC_L2_FDB_ENTRY_OP_HARDWARE,
    /* liwh end */
    MAX_CTC_L2_FDB_ENTRY_OP
};
typedef enum ctc_l2_fdb_entry_op_type_e ctc_l2_flush_fdb_type_t;
typedef enum ctc_l2_fdb_entry_op_type_e ctc_l2_fdb_query_type_t;

/**
 @brief define l2 FDB flush flag
*/
enum ctc_l2_fdb_entry_flag_e
{
    CTC_L2_FDB_ENTRY_STATIC,        /**< [GB.GG.D2.TM] all static fdb record, only ucast*/
    CTC_L2_FDB_ENTRY_DYNAMIC,       /**< [GB.GG.D2.TM] all dynamic fdb record, only ucast*/
    CTC_L2_FDB_ENTRY_LOCAL_DYNAMIC, /**< [GB.GG] all local chip's dynamic fdb record, only ucast*/
    CTC_L2_FDB_ENTRY_ALL,           /**< [GB.GG.D2.TM] all fdb record, only ucast*/
    CTC_L2_FDB_ENTRY_MCAST,         /**< [GB.GG.D2.TM] all mcast record*/
    CTC_L2_FDB_ENTRY_PENDING,       /**< [GG.D2.TM] all pending fdb record, only ucast.
                                                 GG: because of ASIC not support only flush pending entry.
                                                     if set, indicate that flush dynamic entry by hardware.
                                                 D2: flush all pending entry. */
    CTC_L2_FDB_ENTRY_CONFLICT,         /**< [GB.GG.D2.TM] for getting all conflict entry */
    MAX_CTC_L2_FDB_ENTRY_FLAG
};
typedef enum ctc_l2_fdb_entry_flag_e ctc_l2_flush_fdb_flag_t;
typedef enum ctc_l2_fdb_entry_flag_e ctc_l2_fdb_query_flag_t;

/**
 @brief Device-independent L2 unicast address
*/
struct ctc_l2_addr_s
{
    mac_addr_t mac;                         /**< [GB.GG.D2.TM] 802_3 MAC address*/
    uint16  mask_valid;                     /**< [GB] mac address mask valid*/
    mac_addr_t mask;                        /**< [GB] mac address mask*/

    uint16  fid;                            /**< [GB.GG.D2.TM] vid or fid, if fid == 0xFFFF, indicate vlan will be masked*/
    uint32 gport;                           /**< [GB.GG.D2.TM]
                                                    1.Use l2_add_fdb to add fdb, the gport is destination and source port;
                                                    2.Use l2_add_fdb_with_nexthop to add fdb, the gport is source port; When 
                                                      learning by loigc port, the gport is logic source port*/
    uint32  flag;                           /**< [GB.GG.D2.TM] ctc_l2_flag_t  CTC_L2_xxx flags*/

    /* the following info only use for vpls/qinq serverce queue appication/PBB*/
    uint32  assign_port;                    /**< [GG.D2.TM] using for CTC_L2_FLAG_ASSIGN_OUTPUT_PORT,
                                                    1. Use logic port learning, means destination Port
                                                    2. Else means source port & destination port*/
    uint16  cid;                            /**< [D2.TM] category id, only used for static fdb*/
    uint8   is_logic_port;                  /**< [GB.GG.D2.TM] use for dump fdb by trie sort*/
    uint8   rsv[3];
};
typedef struct ctc_l2_addr_s ctc_l2_addr_t;

/**
 @brief define FDB replace DS
*/
struct ctc_l2_replace_s
{
    ctc_l2_addr_t match_addr;       /**< [GB.GG.D2.TM] the fdb address which will be replaced when no resource for
                                                 l2_addr address, and it must be conflicted with l2_addr address */

    ctc_l2_addr_t l2_addr;          /**< [GB.GG.D2.TM] new fdb address */
    uint32 nh_id;                   /**< [GB.GG.D2.TM] indicate user fdb address with the specified nexthop id */
};
typedef struct ctc_l2_replace_s ctc_l2_replace_t;

/**
 @brief Device-independent L2 sync address
*/
struct ctc_l2_sync_msg_s
{
    mac_addr_t mac;             /**< [GB] 802_3 MAC address*/

    uint16  fid;                /**< [GB] vid or fid, if fid == 0xFFFF,indicate vlan will be masked*/
    uint32 gport;               /**< [GB] Port ID, if CTC_L2_SYNC_FLAG_IS_LOGIC_PORT set is logic port, else is gport*/
    uint32  flag;               /**< [GB] ctc_l2_sync_msg_flag_t*/
    uint32  index;              /**< [GB] fdb entry key index in chip*/
};
typedef struct ctc_l2_sync_msg_s ctc_l2_sync_msg_t;

/**
 @brief define FDB query DS
*/
struct ctc_l2_fdb_query_s
{
    mac_addr_t mac;               /**< [GB.GG.D2.TM] 802_3 MAC address*/
    uint16     fid;               /**< [GB.GG.D2.TM] vid or fid*/
    uint32     count;             /**< [GB.GG.D2.TM] (out) return fdb count*/
    uint32     gport;             /**< [GB.GG.D2.TM] FDB Port ID*/
    uint8      query_type;        /**< [GB.GG.D2.TM] ctc_l2_fdb_query_type_t*/
    uint8      query_flag;        /**< [GB.GG.D2.TM] ctc_l2_fdb_query_flag_t*/
    uint8      use_logic_port;    /**< [GB.GG.D2.TM] gport is used as logic port*/
    uint8      query_hw;          /**< [GB.GG] dump hash key by hardware, only support by hardware leaning*/
    uint8      rsv0[2];
};
typedef struct ctc_l2_fdb_query_s ctc_l2_fdb_query_t;

/**
 @brief  Store FDB entries query results,the results contain ucast fdb entries
*/
struct ctc_l2_fdb_query_rst_s
{
    ctc_l2_addr_t* buffer;      /**< [GB.GG.D2.TM] [in/out] A buffer store query results*/
    uint32  start_index;        /**< [GB.GG.D2.TM] If it is the first query, it is equal to 0, else it is equal to the last next_query_index*/
    uint32  next_query_index;   /**< [GB.GG.D2.TM] [out] return index of the next query*/
    uint8   is_end;             /**< [GB.GG.D2.TM] [out] if is_end == 1, indicate the end of query*/
    uint8   rsv;
    uint32  buffer_len;         /**< [GB.GG.D2.TM] multiple of sizeof(ctc_l2_addr_t)*/

};
typedef struct ctc_l2_fdb_query_rst_s ctc_l2_fdb_query_rst_t;

/**
 @brief  Flush FDB entries data structure
*/
struct ctc_l2_flush_fdb_s
{
    mac_addr_t mac;             /**< [GB.GG.D2.TM] 802_3 MAC address*/
    uint16 fid;                 /**< [GB.GG.D2.TM] vid or fid*/
    uint32 gport;               /**< [GB.GG.D2.TM] FDB Port ID*/
    uint8  flush_flag;          /**< [GB.GG.D2.TM] ctc_l2_flush_fdb_flag_t*/
    uint8  flush_type;          /**< [GB.GG.D2.TM] ctc_l2_flush_fdb_type_t*/
    uint8  use_logic_port;      /**< [GB.GG.D2.TM] gport is used as logic port*/
    uint8  rsv0[3];
};
typedef  struct ctc_l2_flush_fdb_s ctc_l2_flush_fdb_t;

/**@} end of @defgroup  fdb FDB */

/**
 @defgroup l2mcast L2Mcast
 @{
*/
/**
 @brief Device-independent L2 multicast address
*/
struct ctc_l2_mcast_addr_s
{
    mac_addr_t mac;                     /**< [GB.GG.D2.TM] 802_3 MAC address*/
    uint16  fid;                        /**< [GB.GG.D2.TM] vid or fid*/
    uint32  flag;                       /**< [GB.GG.D2.TM] ctc_l2_flag_t  CTC_L2_xxx flags*/
    uint16  l2mc_grp_id;                /**< [GB.GG.D2.TM] mcast group_id, it only valid when add mcast group,and unique over the switch system*/
    uint8   member_invalid;             /**< [GB.GG.D2.TM] if set, member will be invalid, maybe applied to add/remove mcast group*/
    uint8   gchip_id;                   /**< [GB.GG.D2.TM] global chip id, used for add/remove port bitmap member,gchip_id=0x1f means linkagg*/
    uint8   port_bmp_en;                /**< [GB.GG.D2.TM] if set, means add/remove members by bitmap*/
    uint8   with_nh;                    /**< [GB.GG.D2.TM] if set, packet will be edited by nhid */
    struct
    {
        uint32   mem_port;              /**< [GB.GG.D2.TM] member port if member is local member gport:gchip(8bit) + local phy port(8bit);
                                                    else if member is remote chip entry, mem_port: gchip(8bit) + mcast_profile_id(8bit) */
        uint32   nh_id;                 /**< [GB.GG.D2.TM] packet will be edited by nhid*/
        ctc_port_bitmap_t  port_bmp;    /**< [GB.GG.D2.TM] port bitmap, used for add/remove members*/
    } member;
    bool remote_chip;                   /**< [GB.GG.D2.TM] if set, member is remote chip entry*/
    uint16   cid;                       /**< [D2.TM] category id*/
    uint8    share_grp_en;              /**< [GG.D2.TM] if set, means use the share group id*/
};
typedef struct ctc_l2_mcast_addr_s ctc_l2_mcast_addr_t;
/**
 @brief Device-independent L2 default information address
*/
struct ctc_l2dflt_addr_s
{
    /* create default entry, only need flag, fid, l2mc_grp_id.*/
    uint32   flag;                      /**< [GB.GG.D2.TM] ctc_l2_dft_vlan_flag_t  CTC_L2_DFT_VLAN_FLAG_xxx flags*/
    uint16   fid;                       /**< [GB.GG.D2.TM] vid or fid*/
    uint16   l2mc_grp_id;               /**< [GB.GG.D2.TM] mcast group_id, it only valid when add default entry, and unique over the switch system*/


    uint8    with_nh;                   /**< [GB.GG.D2.TM] if set, packet will be edited by nhid*/
    uint8    gchip_id;                  /**< [GB.GG.D2.TM] global chip id, used for add/remove port bitmap member,gchip_id=0x1f means linkagg*/
    uint8    port_bmp_en;               /**< [GB.GG.D2.TM] if set, means add/remove members by bitmap*/
	uint8    rsv0;
    uint16   logic_port;                /**< [GB.GG.D2.TM] if CTC_L2_FLAG_LEARNING_USE_LOGIC_PORT set, indicate logic Port is valid*/
    uint16   cid;                       /**< [D2.TM] category id*/
    struct
    {
        uint32   mem_port;              /**< [GB.GG.D2.TM] member port if member is local member gport:gchip(8bit) +local phy port(8bit);
                                                    else if member is remote chip entry,mem_port: gchip(8bit) + mcast_profile_id(8bit))*/
        uint32   nh_id;                 /**< [GB.GG.D2.TM] packet will be edited by nhid*/
        ctc_port_bitmap_t  port_bmp;    /**< [GB.GG.D2.TM] port bitmap, used for add/remove members*/
    } member;

    uint8    remote_chip;               /**< [GB.GG.D2.TM] if set, member is remote chip entry*/
    uint8    share_grp_en;              /**< [GG.D2.TM] if set, means default-entry use the share group id*/
	uint8    rsv1[2];
};
typedef struct ctc_l2dflt_addr_s ctc_l2dflt_addr_t;


/**@} end of @defgroup  l2mcast L2Mcast */

/**
 @defgroup stp STP
 @{
 */
#define CTC_STP_NUM    0x80
#define CTC_MAX_STP_ID 0x7F
#define CTC_MIN_STP_ID 0
enum stp_state_e
{
    CTC_STP_FORWARDING  = 0x00,     /**< [GB.GG.D2.TM] stp state, packet normal forwarding*/
    CTC_STP_BLOCKING    = 0x01,     /**< [GB.GG.D2.TM] stp state, packet blocked*/
    CTC_STP_LEARNING    = 0x02,     /**< [GB.GG.D2.TM] stp state, learning enable*/
    CTC_STP_UNAVAIL      = 0x03
};
/**@} end of @defgroup  stp STP */

/**
 @brief stp mode
*/
enum ctc_l2_stp_mode_e
{
    CTC_STP_MODE_128, /**< [GG.D2.TM] 128 stp instance per port*/
    CTC_STP_MODE_64,  /**< [GG.D2.TM] 64 stp instance per port*/
    CTC_STP_MODE_32,  /**< [GG.D2.TM] 32 stp instance per port*/

    CTC_STP_MODE_MAX
};
typedef enum ctc_l2_stp_mode_e ctc_l2_stp_mode_t;

/**
 @brief l2 fdb global config information
*/
struct ctc_l2_fdb_global_cfg_s
{
    uint32 flush_fdb_cnt_per_loop;  /**< [GB.GG.D2.TM] flush flush_fdb_cnt_per_loop entries one time if flush_fdb_cnt_per_loop>0,
                                         flush all entries if flush_fdb_cnt_per_loop=0*/
    uint32 logic_port_num;          /**< [GB.GG.D2.TM] num of logic port*/
    uint16 default_entry_rsv_num;   /**< [GB.GG.D2.TM] Systems reserve the number of default entry,example:
                                         if systems support per FID default entry, and systems support FID ranges:1-4094,
                                         it's equal to 4094; if systems support FID ranges:100-200, it's equal to 100
                                         [GB.GG] use it as the max fid num*/

    uint8 hw_learn_en;              /**< [GB.GG.D2.TM] global hardware learning*/
    uint8 stp_mode;                 /**< [GG.D2.TM] stp instance mode, refer to ctc_l2_stp_mode_t*/
    uint8 static_fdb_limit_en;      /**< [GG.D2.TM] static fdb mac limit, If set, static fdb will be mac limit*/
    uint8 trie_sort_en;             /**< [GB.GG.D2.TM] if set, get fdb sort by trie*/
};
typedef struct ctc_l2_fdb_global_cfg_s ctc_l2_fdb_global_cfg_t;

#ifdef __cplusplus
}
#endif

#endif  /*_CTC_L2_H*/

