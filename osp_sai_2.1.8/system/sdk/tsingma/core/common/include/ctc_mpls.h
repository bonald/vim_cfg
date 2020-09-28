/**
 @file ctc_mpls.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-03-11

 @version v2.0

   This file contains all mpls related data structure, enum, macro and proto.
*/

#ifndef _CTC_MPLS_H
#define _CTC_MPLS_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_security.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup mpls MPLS
 @{
*/

/**
 @brief  Define mpls label type
*/
enum ctc_mpls_label_type_e
{
    CTC_MPLS_LABEL_TYPE_NORMAL,      /**< [GB.GG.D2.TM] This label is a normal label */
    CTC_MPLS_LABEL_TYPE_L3VPN,       /**< [GB.GG.D2.TM] This label is a l3vpn VC label */
    CTC_MPLS_LABEL_TYPE_VPWS,        /**< [GB.GG.D2.TM] This label is a vpws VC label */
    CTC_MPLS_LABEL_TYPE_VPLS,        /**< [GB.GG.D2.TM] This label is a vpls VC label */
    CTC_MPLS_LABEL_TYPE_GATEWAY,     /**< [GG.D2.TM] This label is a VC label attach to fid and vrf */
    CTC_MPLS_MAX_LABEL_TYPE
};

/**
 @brief  Define mpls tunnel mode
*/
enum ctc_mpls_id_type_e
{
    CTC_MPLS_ID_NULL       = 0x0,       /**< [GB] NULL */
    CTC_MPLS_ID_FLOW       = 0x1,       /**< [GB.GG] The ILM use flow policer */
    CTC_MPLS_ID_VRF        = 0x2,       /**< [GB.GG.D2.TM] The ILM use vrfid, only for l3vpn vc label */
    CTC_MPLS_ID_SERVICE    = 0x4,       /**< [GB.GG.D2.TM] The ILM use qos, only for l2vpn vc label */
    CTC_MPLS_ID_APS_SELECT = 0x8,       /**< [GB.GG.D2.TM] The aps select group id, only for l2vpn vc label */
    CTC_MPLS_ID_STATS      = 0x10,      /**< [GB.GG.D2.TM] The ILM use stats */
    CTC_MPLS_MAX_ID        = 0xff
};

/**
 @brief  Define mpls pw mode
*/
enum ctc_mpls_l2vpn_pw_mode_e
{
    CTC_MPLS_L2VPN_TAGGED,      /**< [GB.GG.D2.TM] The PW mode is TAGGED */
    CTC_MPLS_L2VPN_RAW,         /**< [GB.GG.D2.TM] The PW mode is RAW */
    CTC_MPLS_MAX_L2VPN_MODE
};

/**
 @brief  Define mpls ac bind type
*/
enum ctc_mpls_ac_bind_type_e
{
    CTC_MPLS_BIND_ETHERNET,      /**< [GB] The AC bind type is port */
    CTC_MPLS_MAX_BIND_TYPE
};

/**
 @brief  Define l2vpn type
*/
enum ctc_mpls_l2vpn_type_e
{
    CTC_MPLS_L2VPN_VPWS,      /**< [GB.GG.D2.TM] The L2VPN is VPWS */
    CTC_MPLS_L2VPN_VPLS,      /**< [GB.GG.D2.TM] The L2VPN is VPLS */
    CTC_MPLS_MAX_L2VPN_TYPE
};

/**
 @brief  Define label num type
*/
enum ctc_mpls_label_size_type_e
{
    CTC_MPLS_LABEL_NUM_64_TYPE      = 0,  /**< [GB] Label space have 64 labels */
    CTC_MPLS_LABEL_NUM_128_TYPE     = 1,  /**< [GB] Label space have 128 labels */
    CTC_MPLS_LABEL_NUM_256_TYPE     = 2,  /**< [GB] Label space have 256 labels */
    CTC_MPLS_LABEL_NUM_512_TYPE     = 3,  /**< [GB] Label space have 512 labels */
    CTC_MPLS_LABEL_NUM_1K_TYPE      = 4,  /**< [GB] Label space have 1K labels */
    CTC_MPLS_LABEL_NUM_2K_TYPE      = 5,  /**< [GB] Label space have 2K labels */
    CTC_MPLS_LABEL_NUM_4K_TYPE      = 6,  /**< [GB] Label space have 4K labels */
    CTC_MPLS_LABEL_NUM_8K_TYPE      = 7,  /**< [GB] Label space have 8K labels */
    CTC_MPLS_LABEL_NUM_16K_TYPE     = 8,  /**< [GB] Label space have 16K labels */
    CTC_MPLS_LABEL_NUM_32K_TYPE     = 9,  /**< [GB] Label space have 32K labels */
    CTC_MPLS_LABEL_NUM_64K_TYPE     = 10, /**< [GB] Label space have 64K labels */
    CTC_MPLS_LABEL_NUM_128K_TYPE    = 11, /**< [GB] Label space have 128K labels */
    CTC_MPLS_LABEL_NUM_256K_TYPE    = 12, /**< [GB] Label space have 256K labels */
    CTC_MPLS_LABEL_NUM_512K_TYPE    = 13, /**< [GB] Label space have 512K labels */
    CTC_MPLS_LABEL_NUM_1024K_TYPE   = 14, /**< [GB] Label space have 1024K labels */
    CTC_MPLS_LABEL_NUM_2048K_TYPE   = 15, /**< [GB] Label space have 2048K labels */
    CTC_MPLS_LABEL_NUM_MAX_TYPE
};

/**
 @brief define ip version
*/
enum ctc_mpls_inner_pkt_type_e
{
    CTC_MPLS_INNER_IP,     /**< [GB.GG.D2.TM] IP version 4 or IP version 6 */
    CTC_MPLS_INNER_IPV4,   /**< [GB.GG.D2.TM] IP version 4 */
    CTC_MPLS_INNER_IPV6,   /**< [GB.GG.D2.TM] IP version 6 */
    CTC_MPLS_INNER_RAW,    /**< [GB.GG.D2.TM] Don't check inner packet by parser */
    MAX_MPLS_CTC_VER
};
typedef enum ctc_mpls_inner_pkt_type_e ctc_mpls_inner_pkt_type_t;

/**
 @brief define ilm flag
*/
enum ctc_mpls_ilm_flag_e
{
   CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT = 1U << 0,      /**<[GG.D2.TM] if set ,indicate the output port will get by MPLS module,not by nhid */

   CTC_MPLS_ILM_FLAG_L2VPN_OAM          = 1U << 1,      /**< [GG.D2.TM] indecate L2VPN OAM, need config l2vpn_oam_id*/
   CTC_MPLS_ILM_FLAG_SERVICE_ACL_EN     = 1U << 2,      /**< [GG.D2.TM] enable OAM feature*/
   CTC_MPLS_ILM_FLAG_SERVICE_POLICER_EN = 1U << 3,      /**< [GG] if the bit not set and policer-id isn't 0,it's flow policer*/
   CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO = 1U << 4,      /**< [GG.D2.TM] if set, acl match rule will use mpls packet outer information */
   CTC_MPLS_ILM_FLAG_BOS_LABLE  = 1U << 5,              /**< [GG.D2.TM] if set, the normal label is VC label */
   CTC_MPLS_ILM_FLAG_USE_FLEX  = 1U << 6,               /**< [D2] if set, mpls key use TCAM, can used to solve hash conflict */
   CTC_MPLS_ILM_FLAG_FLEX_EDIT = 1U << 7,              /**< [D2.TM] if set, only edit label of packet, remain eth head*/

   CTC_MPLS_ILM_FLAG_MAX
};
typedef enum ctc_mpls_ilm_flag_e ctc_mpls_ilm_flag_t;

/**
 @brief define mpls ilm oam check type
*/
enum ctc_mpls_ilm_oam_mp_chk_type_e
{
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_NONE,          /**< [D2.TM] Do not check, whether OAM Packet or Data packet*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_OAM_MEP,       /**< [D2.TM] configure MEP in the label*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_OAM_MIP,       /**< [D2.TM] configure MIP in the label*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_OAM_DISCARD,   /**< [D2.TM] If label with OAM Packet, then Discard*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_TO_CPU,        /**< [D2.TM] If label with OAM Packet, then to CPU*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_DATA_DISCARD,  /**< [D2.TM] Discard Data Packet*/
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE_MAX
};
typedef enum ctc_mpls_ilm_oam_mp_chk_type_e ctc_mpls_ilm_oam_mp_chk_type_t;

/**
 @brief define mpls ilm update type
*/
enum ctc_mpls_ilm_property_type_e
{
    CTC_MPLS_ILM_DATA_DISCARD,      /**< [GB.GG.D2.TM] Update to discard data, not OAM , for lock use. value 1, discard; 0, not discard */
    CTC_MPLS_ILM_QOS_DOMAIN,        /**< [GB.GG.D2.TM] Update Qos domain, value is qos domain, 1~15 is valid value, 0 means disable */
    CTC_MPLS_ILM_APS_SELECT,        /**< [GB.GG.D2.TM] Update Aps select info, value is ctc_mpls_ilm_t  */
    CTC_MPLS_ILM_OAM_TOCPU,         /**< [GG.D2.TM] Set Oam packet direct to CPU  */
    CTC_MPLS_ILM_ROUTE_MAC,         /**< [GG.D2.TM] Bind the L3 payload router mac with ilm, and void *value is mac_addr_t*  */
    CTC_MPLS_ILM_LLSP,              /**< [GB.GG] L-LSP enable, use lable to mapping PHB, value 0xFF means E-LSP  */
    CTC_MPLS_ILM_OAM_MP_CHK_TYPE,   /**< [D2.TM] Update OAM CHK Type, value refer to enum ctc_mpls_ilm_oam_mp_chk_type_t*/
    CTC_MPLS_ILM_ARP_ACTION,  /**< [D2.TM] If set, arp packet action refer to ctc_exception_type_t*/
    CTC_MPLS_ILM_QOS_MAP,      /**< [D2.TM] Set qos map mode, value is ctc_mpls_ilm_qos_map_t*/
    CTC_MPLS_ILM_STATS_EN,      /**< [D2.TM] Enable update stats ptr*/
    CTC_MPLS_ILM_DENY_LEARNING_EN, /**< [D2.TM] Enable deny learning*/
    CTC_MPLS_ILM_MAC_LIMIT_DISCARD_EN, /**< [D2.TM] Enable mac limit discard, only used for vpls*/
    CTC_MPLS_ILM_MAC_LIMIT_ACTION,            /**< [TM] Enable mac limit, refer to ctc_maclimit_action_t*/
    CTC_MPLS_ILM_STATION_MOVE_DISCARD_EN,  /**< [TM] Enable station move discard, only used for vpls*/
    CTC_MPLS_ILM_METADATA,             /**< [TM] Metadata*/
    CTC_MPLS_ILM_DCN_ACTION,           /**< [D2.TM] DCN mcc/scc packet action.
                                                    0: none. 1: copy to cpu. 2: decap and forward with inner packet*/
    CTC_MPLS_ILM_CID,              /**< [D2.TM] Update category id */
    CTC_MPLS_ILM_MAX
};
typedef enum ctc_mpls_ilm_property_type_e ctc_mpls_ilm_property_type_t;


/**
 @brief  Define mpls stats index
*/
struct ctc_mpls_stats_index_s
{
    uint32 stats_id;/**< [GB.D2.TM] Stats id */
    uint32 label;   /**< [GB.D2.TM] MPLS label */
    uint8 spaceid;  /**< [GB.D2.TM] Label space id */
};
typedef struct ctc_mpls_stats_index_s ctc_mpls_stats_index_t;

/**
 @brief  Define mpls parameter structure
*/
struct ctc_mpls_ilm_s
{
    uint32 label;                   /**< [GB.GG.D2.TM] In label */
    uint32 flag;                    /**< [GG.D2.TM] CTC_MPLS_ILM_FLAG_XXX */
    uint32 nh_id;                   /**< [GB.GG.D2.TM] Nexthop ID, pop mode don't need to config nexthop,in pop mode use drop nh */
    uint32 gport;                   /**< [GG.D2.TM] valid when CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT is set */
    uint32 stats_id;                /**< [GB.GG.D2.TM] Stats id */
    uint16 pwid;                    /**< [GB.GG.D2.TM] PW ID, VPLS use PW ID to bridge packets in instance*/
    union
    {
        uint16 flow_id;             /**< [GB] The ILM use flow policer */
        uint16 vrf_id;              /**< [GB.D2.TM] The ILM use vrfid, only for l3vpn vc label */
        uint16 service_id;          /**< [GB.D2.TM] The ILM use qos, only for l2vpn vc label */
        uint16 aps_select_grp_id;   /**< [GB.D2.TM] The aps select group id, only only for l2vpn vc label */
    } flw_vrf_srv_aps;

    uint8 id_type;                  /**< [GB.D2.TM] flw_vrf_srv_aps value with which type */
    uint8 spaceid;                  /**< [GB.GG.D2.TM] Label space ID */
    uint8 type;                     /**< [GB.GG.D2.TM] Label type, should be one of the ctc_mpls_label_type_e value */
    uint8 model;                    /**< [GB.GG.D2.TM] Tunnel mode, should be one of the ctc_mpls_tunnel_mode_e value */
    uint8 cwen;                     /**< [GB.GG.D2.TM] Control word enable, if label is a l2vpn vc label, the PW control word function maybe enable */
    uint8 pop;                      /**< [GB.GG.D2.TM] Whether the label is POP label or not, only used when label type is normal */
    uint8 decap;                    /**< [GB.GG.D2.TM] Whether the inner packet will decap in pop mode*/
    uint8 aps_select_protect_path;  /**< [GB.GG.D2.TM] If id_type is CTC_MPLS_ID_APS_SELECT and it is set, indicate the path is aps protect path,else the path is working path */
    uint8 logic_port_type;          /**< [GB.GG.D2.TM] The VPLS PW is a H-VPLS tunnel */
    uint8 oam_en;                   /**< [GB.GG.D2.TM] Enable OAM */
    uint8 trust_outer_exp;          /**< [GB.GG.D2.TM] Trust outer label exp, also need configure mpls property qos domain */
    uint8 inner_pkt_type;           /**< [GB.GG.D2.TM] Inner packet type used for ipv6 */
    uint8 qos_use_outer_info;       /**< [GB.GG.D2.TM] If set, use outer parse info for qos */
    uint8 out_intf_spaceid;         /**< [GB.GG.D2.TM] Interafce label space ID, pop label can generate an interface label space. */
    uint16 aps_select_grp_id;       /**< [GG.D2.TM] The aps select group id, only only for l2vpn vc label */
    uint8  svlan_tpid_index;        /**< [GB.GG.D2.TM] svlan tpid index*/
    uint8 pw_mode;                  /**< [GB.GG.D2.TM]when PW is raw mode, dsmpls.OuterVlanIsCvlan must set to 1 for eth AC won't strip the tag */
    uint16 fid;                     /**< [GB.GG.D2.TM] VPLS ID */
    uint16 vrf_id;                  /**< [GB.GG.D2.TM] The ILM use vrfid, only for l3vpn vc label */
    uint16 l2vpn_oam_id;            /**< [GG.D2.TM] vpws oam id, used in Port + Oam ID mode, refer to CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE*/
    uint16 policer_id;              /**< [GB.GG.D2.TM] policer id*/
    uint8 learn_disable;            /**< [D2.TM] VPLS learning disable, 0:learning enable, 1:learning disable */
    /*SYSTEM MODIFY by wangl for hybrid oam 2018-6-28*/
    #ifdef OFPRODUCT
    uint8   oam_check_type;
    #endif
};
typedef struct ctc_mpls_ilm_s ctc_mpls_ilm_t;

/**
 @brief  Define l2vpn pw parameter structure
*/
struct ctc_mpls_l2vpn_pw_s
{
    uint32 label;                   /**< [GB.GG.D2.TM] In label */
    uint32 flag;                    /**< [GG.D2.TM] CTC_MPLS_ILM_FLAG_XXX */
    uint8 l2vpntype;                /**< [GB.GG.D2.TM] ctc_mpls_l2vpn_type_e,L2VPN type, VPWS or VPLS */
    uint8 learn_disable;            /**< [GB.D2.TM] VPLS learning disable, 0:learning enable, 1:learning disable */
    uint8 maclimit_enable;          /**< [GB] VPLS mac limit enable*/
    uint8 space_id;                 /**< [GB.GG.D2.TM] Label space ID*/

    uint8 pw_mode;                  /**< [GB.GG.D2.TM] pw mode only used for goldengate, when PW is raw mode, dsmpls.OuterVlanIsCvlan must set to 1 for eth AC won't strip the tag */
    uint8 igmp_snooping_enable;     /**< [GB] igmpSnoopingEn */
    uint8 cwen;                     /**< [GB.GG.D2.TM] Control word enable, if label is a l2vpn vc label, the PW control word function maybe enable */
    uint8 rsv0;

    uint8 qos_use_outer_info;       /**< [GB.GG.D2.TM] If set, use outer parse info for qos */
    uint8 id_type;                  /**< [GB.D2.TM] flw_vrf_srv_aps value with which type */
    uint8 oam_en;                   /**< [GB.GG.D2.TM] Enable OAM */
    uint8 aps_select_protect_path;  /**< [GB.GG.D2.TM] If id_type is CTC_MPLS_ID_APS_SELECT and it is set, indicate the path is aps protect path,else the path is working path */

    uint16 service_id;              /**< [GB.GG.D2.TM] service id for do service acl/queueu/policer */
    uint8  service_policer_en;      /**< [GB.D2.TM] If set, service policer enable */
    uint8  service_queue_en;        /**< [GB] If set, service queue enable */
    uint8  service_aclqos_enable;   /**< [GB.D2.TM] If set, service acl enable */

    uint8  svlan_tpid_index;        /**< [GB.GG.D2.TM] svlan tpid index*/
    uint8  trust_outer_exp;         /**< [GB] Trust outer label exp */
    uint8  inner_pkt_type;          /**< [GB.GG.D2.TM] Inner packet type, refer to ctc_mpls_inner_pkt_type_t */

    uint32 stats_id;                /**< [GB.GG.D2.TM] Stats id */
    uint16 aps_select_grp_id;       /**< [GB.GG.D2.TM] The aps select group id, only only for l2vpn vc label */
    uint16 logic_port;              /**< [GB.GG.D2.TM] PW logic source port */

    uint16 l2vpn_oam_id;            /**< [GG.D2.TM] vpws oam id or vpls fid, used in Port + Oam ID mode, refer to CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE*/

    union
    {
        uint32 pw_nh_id;            /**< [GB.D2.TM] Nexthop ID of PW, should be a vpws nexthop */
        struct
        {
            uint16 fid;             /**< [GB.D2.TM] VPLS ID */
            uint8  logic_port_type; /**< [GB.D2.TM] VPLS PW is a VPLS tunnel or H-VPLS tunnel */
        } vpls_info;
    } u;


    uint32 gport;                   /**< [GG.D2.TM] valid when CTC_MPLS_ILM_FLAG_ASSIGN_OUTPUT_PORT is set */
};
typedef struct ctc_mpls_l2vpn_pw_s ctc_mpls_l2vpn_pw_t;

/**
 @brief  Define mpls label space information structure
*/
struct ctc_mpls_space_info_s
{
    uint8 enable;       /**< [GB] Whether the label space is valid */
    uint8 sizetype;     /**< [GB] The label space for GB:size =  64 * (2 ^ sizetype)*/
};
typedef struct ctc_mpls_space_info_s ctc_mpls_space_info_t;

/**
 @brief  Define mpls init information structure
*/
struct ctc_mpls_init_s
{
    ctc_mpls_space_info_t space_info[CTC_MPLS_SPACE_NUMBER];    /**< [GB] Label space information array */
    uint32 min_interface_label;                                 /**< [GB] Minimum interface label for space id > 0,
                                                                   if min_interface_label == 0, platform space size and minimum interface label is 64 * (2 ^ space_info[0].sizetype)
                                                                   if  0 < min_interface_label < 64, platform space size and minimum interface label is 64
                                                                   if  min_interface_label >= 64, platform space size is 64 and minimum interface label is min_interface_label */
};
typedef struct ctc_mpls_init_s ctc_mpls_init_t;


/**
 @brief  Define mpls property update structure
*/
struct ctc_mpls_property_s
{
    uint8 space_id;             /**< [GB.GG.D2.TM] Label space ID */
    uint8 use_flex;            /**< [D2.TM] if set, mpls key use TCAM, can used to solve hash conflict */
    uint32 label;               /**< [GB.GG.D2.TM] In label */
    uint32 property_type;       /**< [GB.GG.D2.TM] property type, refer to ctc_mpls_ilm_property_type_t */
    void *value;                /**< [GB.GG.D2.TM] property value, according to property type */
};
typedef struct ctc_mpls_property_s ctc_mpls_property_t;

enum ctc_mpls_ilm_qos_map_mode_e
{
    CTC_MPLS_ILM_QOS_MAP_DISABLE,     /**< [D2.TM] Disable qos map*/
    CTC_MPLS_ILM_QOS_MAP_ASSIGN,   /**< [D2.TM] User define priority and color, donot using exp domain*/
    CTC_MPLS_ILM_QOS_MAP_ELSP,     /**< [D2.TM] priority and color mapping from exp domain*/
    CTC_MPLS_ILM_QOS_MAP_LLSP,     /**< [D2.TM] user define priority and color maping from exp domain */
    CTC_MPLS_ILM_QOS_MAP_MAX
};
typedef enum ctc_mpls_ilm_qos_map_mode_e ctc_mpls_ilm_qos_map_mode_t;
struct ctc_mpls_ilm_qos_map_s
{
    uint8 mode;     /**< [D2.TM] refer to ctc_mpls_ilm_qos_map_mode_t*/
    uint8 exp_domain; /**< [D2.TM] exp domain*/
    uint8 priority; /**< [D2.TM] priority*/
    uint8 color; /**< [D2.TM] refer to ctc_qos_color_t*/
};
typedef struct ctc_mpls_ilm_qos_map_s ctc_mpls_ilm_qos_map_t;

/**@} end of @defgroup mpls MPLS */

#ifdef __cplusplus
}
#endif

#endif  /*_CTC_MPLS_H*/

