/**
 @file ctc_learning_aging.h

 @date 2010-2-25

 @version v2.0

---file comments----
*/

#ifndef CTC_LEARNING_AGING_H_
#define CTC_LEARNING_AGING_H_
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"

/**
 @defgroup learning_aging  LEARNING_AGING
 @{
*/

#define CTC_LEARNING_CACHE_MAX_INDEX          32     /**< [GB.GG.D2.TM] learning cache size*/

#define CTC_AGING_FIFO_DEPTH                  32     /**< [GB] aging fifo size*/
#define MAX_CTC_ENTRY_NUM_IN_AGING_INDEX      16     /**< [GB] a aging entry  contains  the max num of  MAC entries*/

/**
 @brief enum type about aging table
*/
enum ctc_aging_table_type_e
{
    CTC_AGING_TBL_MAC,         /**< [GB.GG.D2.TM] MAC Aging table*/
    CTC_AGING_TBL_SCL,         /**< [GB] SCL Aging table*/
    MAX_CTC_AGING_TBL

};
typedef enum ctc_aging_table_type_e ctc_aging_table_type_t;

/**
 @brief enum type about aging status
*/
struct ctc_aging_status_s
{
    uint16    reserved;
};
typedef struct ctc_aging_status_s ctc_aging_status_t;

/**
 @brief learning cache entry node struct
*/
struct ctc_learning_cache_entry_s
{
    uint32 mac_sa_0to31;       /**< [GB.GG.D2.TM] MacSa lower 32 bits*/

    uint32 cmac_sa_0to31;      /**< [GB.GG.D2.TM] CmacSa Lower 32 bits,  used in PBB*/

    uint16 mac_sa_32to47;      /**< [GB.GG.D2.TM] MacSa upper 16 bits*/
    uint16 cmac_sa_32to47;     /**< [GB.GG.D2.TM] CMacSa upper 16 bits, used in PBB*/

    mac_addr_t mac;            /**< [GG.D2.TM] 802_3 MAC address*/

    uint16 logic_port;         /**< [GB.GG.D2.TM] Logic source port*/
    uint16 fid;                /**< [GB.GG.D2.TM] Fid or vsiid*/

    uint16 cvlan_id;           /**< [GB.GG.D2.TM]Source packet cvlan*/
    uint16 svlan_id;           /**< [GB.GG.D2.TM]Source packet svlan*/

    uint16 mapped_vlan_id;     /**< [GB] Mapped Forwarding VlanID*/
    uint32 global_src_port;    /**< [GB.GG.D2.TM] Global Source port which packet in*/

    uint8 is_logic_port;       /**< [GB.GG.D2.TM] If set, using logic_port for learning*/
    uint8 is_ether_oam;        /**< [GB.GG.D2.TM] If set, indicate packet is Ethernet OAM*/
    uint8 ether_oam_md_level;  /**< [GB.GG.D2.TM] Ethernet OAM MD Levl*/
    uint8 is_hw_sync;          /**< [GG.D2.TM]  If set, indicate this fdb is hw learning mode, else software learning mode*/
};
typedef struct ctc_learning_cache_entry_s ctc_learning_cache_entry_t;

/**
 @brief struct type to store the aging cache content
*/
struct ctc_learning_cache_s
{
    ctc_learning_cache_entry_t learning_entry[CTC_LEARNING_CACHE_MAX_INDEX];    /**< [GB.GG.D2.TM] Learning cache entry*/
    uint32 entry_num;                                                           /**< [GB.GG.D2.TM] Learning cache entry num*/
    uint32 sync_mode;                                                           /**< [GG.D2.TM] 0:Using Fifo to sync data; 1: Using Dma to sync data*/
};
typedef struct ctc_learning_cache_s ctc_learning_cache_t;

/**
 @brief aging cache entry node struct
*/
struct ctc_aging_info_entry_s
{
    mac_addr_t mac;             /**< [GG.D2.TM] 802_3 MAC address*/
    uint16 fid;                 /**< [GG.D2.TM] Fid or vsiid*/
    uint16  vrf_id;             /**< [GG.D2.TM] Vrf ID of the route*/
    union
    {
        ip_addr_t ipv4;         /**< [GG] IPv4 destination address*/
        ipv6_addr_t ipv6;       /**< [GG] IPv6 destination address*/
    } ip_da;
    uint8  aging_type;          /**< [GG.D2.TM] refer to ctc_aging_type_t*/
    uint8  masklen;             /**< [GG.D2.TM] Mask length of destination*/
    uint8 is_hw_sync;           /**< [GG.D2.TM] If set, indicate this entry is hw aging mode, else software aging mode*/
    uint8  is_logic_port;       /**< [D2.TM] If set, using logic_port*/
    uint32 gport;               /**< [D2.TM] Global Source port*/
};
typedef struct ctc_aging_info_entry_s ctc_aging_info_entry_t;

/**
 @brief struct type to store the aging fifo content
*/
struct ctc_aging_fifo_info_s
{
    uint32 aging_index_array[CTC_AGING_FIFO_DEPTH];             /**< [GB.GG.D2.TM] Aging FIFO index*/
    uint8  fifo_idx_num;                                        /**< [GB.GG.D2.TM] entry number*/
    uint8  tbl_type[CTC_AGING_FIFO_DEPTH];                      /**< [GB.GG.D2.TM] need aging table type, ctc_aging_table_type_t*/
    ctc_aging_info_entry_t aging_entry[CTC_AGING_FIFO_DEPTH];   /**< [GG.D2.TM] Aging info, Only usefull in dma aging mode*/
    uint32 sync_mode;                                           /**< [GG.D2.TM] 0:Using Fifo to sync data; 1: Using Dma to sync data*/
};
typedef struct ctc_aging_fifo_info_s ctc_aging_fifo_info_t;

/**
 @brief learning aging info for GB hardware learning mode when using software
*/
struct ctc_learn_aging_info_s
{
    uint32 key_index;               /**< [GB] learning entry key index*/

    mac_addr_t   mac;               /**< [GB] mac address*/
    uint16 vsi_id;                  /**< [GB] vsi id*/

    uint16 damac_index;             /**< [GB] Ad index */
    uint8 is_mac_hash;              /**< [GB] whether using mac hash*/
    uint8 valid;                    /**< [GB] entry is valid*/
    uint8 is_aging;                 /**< [GB] is aging flag*/
};
typedef struct ctc_learn_aging_info_s  ctc_learn_aging_info_t;

/**
 @brief node of recv_list
*/
struct ctc_hw_learn_aging_fifo_s
{
    uint16   entry_num;                                         /**< [GB] hardware leaning or aging entry number */
    ctc_learn_aging_info_t    learn_info[CTC_AGING_FIFO_DEPTH]  /**< [GB] hardware leaning entry or aging information*/;
};
typedef struct ctc_hw_learn_aging_fifo_s  ctc_hw_learn_aging_fifo_t;

/**
 @brief define Learning action
*/
enum ctc_learning_action_e
{
    CTC_LEARNING_ACTION_ALWAYS_CPU = 1,                     /**< [GB] MAC Learning by CPU*/
    CTC_LEARNING_ACTION_CACHE_FULL_TO_CPU,                  /**< [GB] MAC Learning by cache ,but cache full send to cpu*/
    CTC_LEARNING_ACTION_CACHE_ONLY,                         /**< [GB] MAC Learning by cache*/
    CTC_LEARNING_ACTION_DONLEARNING,                        /**< [GB] Dont  Learning*/
    CTC_LEARNING_ACTION_MAC_TABLE_FULL,                     /**< [GB.GG.D2.TM] IpeLearningCtl mac tabel full*/
    CTC_LEARNING_ACTION_MAC_HASH_CONFLICT_LEARNING_DISABLE  /**< [GB] MAC conflict learning disable */
};
typedef enum ctc_learning_action_e ctc_learning_action_t;

/**
 @brief define Learning action and learning cache threshold
*/
struct ctc_learning_action_info_s
{
    ctc_learning_action_t action;       /**< [GB.GG.D2.TM] define Learning action*/
    uint32    value;                    /**< [GB.GG.D2.TM] learning cache_threshold ,for IpeLearning*/
};
typedef struct ctc_learning_action_info_s ctc_learning_action_info_t;

/**
 @brief define MAC Aging property associated type data struct
*/
enum ctc_aging_prop_e
{
    CTC_AGING_PROP_FIFO_THRESHOLD  =  1,    /**< [GB] aging fifo threshold  ; value: 0x0--0xFF*/
    CTC_AGING_PROP_INTERVAL,                /**< [GB.GG.D2.TM] aging interval(second) ; value :0x0--0xFFFFFFFF*/
    CTC_AGING_PROP_STOP_SCAN_TIMER_EXPIRED, /**< [GB.GG.D2.TM] when one aging timer finish, if set, aging timer will stop scan; value: TRUE or FALSE*/
    CTC_AGING_PROP_AGING_SCAN_EN,           /**< [GB.GG.D2.TM] aging timer scan enable*/

    CTC_AGING_PROP_MAX
};
typedef enum ctc_aging_prop_e ctc_aging_prop_t;

/**
 @brief define learn & Aging  global config information
*/
struct ctc_learn_aging_global_cfg_s
{
    uint8 mac_aging_dis;      /**< [GB] if set,will disable mac aging*/
    uint8 scl_aging_en;       /**< [GB] if set,will enable scl aging*/
    uint8 ipmc_aging_en;      /**< [GB] if set,will enable ipmc aging*/
    uint8 hw_mac_learn_en;    /**< [GB] if set,will enable hw mac learning*/
    uint8 hw_mac_aging_en;    /**< [GB] if set,will enable hw mac aging*/
    uint8 hw_scl_aging_en;    /**< [GB] if set,will enable hw scl aging*/
};
typedef struct ctc_learn_aging_global_cfg_s ctc_learn_aging_global_cfg_t;


/**
 @brief define aging type
*/
enum ctc_aging_type_e
{
    CTC_AGING_TYPE_MAC,             /**< [GG.D2.TM] mac aging*/
    CTC_AGING_TYPE_IPV4_UCAST,      /**< [GG] ipuc ucast host route aging*/
    CTC_AGING_TYPE_IPV6_UCAST,      /**< [GG] ipuc ipv6 ucast host route aging*/

    CTC_AGING_TYPE_MAX
};
typedef enum ctc_aging_type_e ctc_aging_type_t;

/**@} end of @defgroup   learning_aging  LEARNING_AGING*/

#ifdef __cplusplus
}
#endif

#endif

