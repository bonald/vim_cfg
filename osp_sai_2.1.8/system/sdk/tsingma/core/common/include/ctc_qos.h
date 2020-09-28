/**
 @file ctc_qos.h

 @date 2010-5-25

 @version v2.0

 This file contains all acl related data structure, enum, macro and proto.
*/

#ifndef _CTC_QOS_H_
#define _CTC_QOS_H_

#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_const.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/****************************************************************
*
* Data Structures
*
****************************************************************/

#define CTC_MAX_QUEUE_CLASS_NUM     4
#define CTC_RESC_DROP_PREC_NUM      4
#define CTC_DROP_PREC_NUM           4
#define CTC_RESRC_MAX_CONGEST_LEVEL_NUM             8
#define CTC_RESRC_MAX_CONGEST_LEVEL_THRD_NUM        7
#define CTC_MAX_FCDL_DETECT_NUM     2

#define CTC_QOS_SHP_TOKE_THRD_DEFAULT 0xFFFFFFFF
/**
 @defgroup qos QOS
 @{
*/

/**
 @brief QoS color
*/
enum ctc_qos_color_e
{
    CTC_QOS_COLOR_NONE,     /**< [GB.GG.D2.TM] None color */
    CTC_QOS_COLOR_RED,      /**< [GB.GG.D2.TM] Red color: the lowest drop precedence */
    CTC_QOS_COLOR_YELLOW,   /**< [GB.GG.D2.TM] Yellow color: the mild drop precedence */
    CTC_QOS_COLOR_GREEN,    /**< [GB.GG.D2.TM] Green color: the highest drop precedence */

    MAX_CTC_QOS_COLOR
};
typedef enum ctc_qos_color_e ctc_qos_color_t;

/**
 @brief qos trust type
*/
enum ctc_qos_trust_type_e
{
    CTC_QOS_TRUST_PORT        = 0,       /**< [GB.GG.D2.TM] trust Port */
    CTC_QOS_TRUST_OUTER       = 1,       /**< [GB.GG.D2.TM] copy outer */
    CTC_QOS_TRUST_COS         = 2,       /**< [GB.GG.D2.TM] trust COS */
    CTC_QOS_TRUST_DSCP        = 3,       /**< [GB.GG.D2.TM] trust DSCP */
    CTC_QOS_TRUST_IP          = 4,       /**< [GB.GG] trust IP Precedence */
    CTC_QOS_TRUST_STAG_COS    = 5,       /**< [GB.GG.D2.TM] trust stag-cos */
    CTC_QOS_TRUST_CTAG_COS    = 6,       /**< [GB.GG.D2.TM] trust ctag-cos */
    CTC_QOS_TRUST_MAX
};
typedef enum ctc_qos_trust_type_e ctc_qos_trust_type_t;

/**
 @brief Qos domain mapping type
*/
enum ctc_qos_domain_map_type_e
{
    CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR,      /**< [GB.GG.D2.TM] Mapping ingress cos +  dei to priority + color */
    CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR,     /**< [GB.GG.D2.TM] Mapping ingress dscp + ecn to priority + color */
    CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR,  /**< [GB.GG] Mapping ingress ip-precedence to priority + color */
    CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR,      /**< [GB.GG.D2.TM] Mapping ingress exp to priority + color */
    CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS,      /**< [GB.GG.D2.TM] Mapping egress  priority + color to cos + cfi*/
    CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP,     /**< [GB.GG.D2.TM] Mapping egress  priority + color to dscp*/
    CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP,      /**< [GB.GG.D2.TM] Mapping egress  priority + color to exp*/
    CTC_QOS_DOMAIN_MAP_TYPE_MAX
};
typedef enum ctc_qos_domain_map_type_e ctc_qos_domain_map_type_t;

/**
 @brief Qos domain mapping type
*/
struct ctc_qos_domain_map_s
{
    uint8    domain_id;      /**< [GB.GG.D2.TM] Domain id <0-7>, apply on port; for D2,
                                                1. cos domain: Domain id <0-7>, apply on port/nexthop
                                                2. dscp domain: Domain id <0-15>, apply on port/interface/nexthop
                                                3. exp domain: Domain id <0-15>, apply on mpls/nexthop*/
    uint8    type;           /**< [GB.GG.D2.TM] Domain mapping type, pls refer to ctc_qos_domain_map_type_t*/

    uint8    priority;       /**< [GB.GG.D2.TM] Qos priority: <0-15> */
    uint8    color;          /**< [GB.GG.D2.TM] Qos color, pls refer to ctc_qos_color_t*/

    union
    {
        struct
        {
            uint8    cos;   /**< [GB.GG.D2.TM] Vlan id cos <0-7>*/
            uint8    dei;   /**< [GB.GG.D2.TM] Vlan id dei <0-1>*/
        } dot1p;

        struct
        {
            uint8    dscp; /**< [GB.GG.D2.TM] Ip dscp <0-63>*/
            uint8    ecn;  /**< [GB.GG]       Ip ecn <0-3>*/
        } tos;

        uint8    exp;      /**< [GB.GG.D2.TM] Mpls exp <0-7>*/
        uint8    ip_prec;  /**< [GB.GG] Ip precedence <0-7>*/

    } hdr_pri;

};
typedef struct ctc_qos_domain_map_s ctc_qos_domain_map_t;


/**
 @brief Qos table mapping type
*/
enum ctc_qos_table_map_type_e
{
    CTC_QOS_TABLE_MAP_IGS_COS_TO_COS,       /**< [D2.TM] Mapping ingress cos to cos */
    CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP,      /**< [D2.TM] Mapping ingress cos to dscp */
    CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS,      /**< [D2.TM] Mapping ingress dscp to cos */
    CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP,     /**< [D2.TM] Mapping ingress dscp to dscp */
    CTC_QOS_TABLE_MAP_TYPE_MAX
};
typedef enum ctc_qos_table_map_type_e ctc_qos_table_map_type_t;

/**
 @brief Qos table mapping action type
*/
enum ctc_qos_table_map_action_type_e
{
    CTC_QOS_TABLE_MAP_ACTION_NONE,       /**< [D2.TM] None */
    CTC_QOS_TABLE_MAP_ACTION_COPY,       /**< [D2.TM] Keep original value */
    CTC_QOS_TABLE_MAP_ACTION_MAP,        /**< [D2.TM] Use value from table map */
    CTC_QOS_TABLE_MAP_ACTION_MAX
};
typedef enum ctc_qos_table_map_action_type_e ctc_qos_table_map_action_type_t;

/**
 @brief Qos table mapping
*/
struct ctc_qos_table_map_s
{
    uint8    table_map_id;   /**< [D2.TM] Table map id */
    uint8    type;           /**< [D2.TM] Table mapping type, pls refer to ctc_qos_table_map_type_t*/
    uint8    action_type;    /**< [D2.TM] Table mapping action type, pls refer to ctc_qos_table_map_action_type_t*/
    uint8    in;             /**< [D2.TM] Input value */
    uint8    out;            /**< [D2.TM] Output value*/
    uint8    rsv[3];
};
typedef struct ctc_qos_table_map_s ctc_qos_table_map_t;


/**
 @brief Qos policer type
*/
enum ctc_qos_policer_type_e
{
    CTC_QOS_POLICER_TYPE_PORT,           /**< [GB.GG.D2.TM] Port policer (support ing and egs) */
    CTC_QOS_POLICER_TYPE_FLOW,           /**< [GB.GG.D2.TM] Flow policer (support ing and egs)*/
    CTC_QOS_POLICER_TYPE_VLAN,           /**< [D2.TM]       Vlan policer (support ing and egs) */
    CTC_QOS_POLICER_TYPE_SERVICE,        /**< [GB.GG]    Service policer (only support ingress) */
    CTC_QOS_POLICER_TYPE_COPP,           /**< [D2.TM]       Control Plane Policing (only support ingress) */
    CTC_QOS_POLICER_TYPE_MFP,            /**< [TM]       Ipfix MFP  (support ing and egs)*/
    CTC_QOS_POLICER_TYPE_MAX
};
typedef enum ctc_qos_policer_type_e ctc_qos_policer_type_t;

/**
 @brief Qos policer mode
*/
enum ctc_qos_policer_mode_e
{
    CTC_QOS_POLICER_MODE_STBM,       /**< [D2.TM]          Single Token Bucket Meter */
    CTC_QOS_POLICER_MODE_RFC2697,    /**< [GB.GG.D2.TM] RFC2697 srTCM */
    CTC_QOS_POLICER_MODE_RFC2698,    /**< [GB.GG.D2.TM] RFC2698 trTCM */
    CTC_QOS_POLICER_MODE_RFC4115,    /**< [GB.GG.D2.TM]    RFC4115 mode enhanced trTCM*/
    CTC_QOS_POLICER_MODE_MEF_BWP,    /**< [GB.GG.D2.TM]    MEF BWP(bandwidth profile) */
    CTC_QOS_POLICER_MODE_MAX
};
typedef enum ctc_qos_policer_mode_e ctc_qos_policer_mode_t;


/**
 @brief Qos policer level
*/
enum ctc_qos_policer_level_e
{
    CTC_QOS_POLICER_LEVEL_NONE,        /**< [D2.TM] None level */
    CTC_QOS_POLICER_LEVEL_0,           /**< [D2.TM] Policer level 0*/
    CTC_QOS_POLICER_LEVEL_1,           /**< [D2.TM] Policer level 1 */
    CTC_QOS_POLICER_LEVEL_MAX
};
typedef enum ctc_qos_policer_level_e ctc_qos_policer_level_t;

/**
 @brief Qos policer 2 level color merge mode
*/
enum ctc_qos_policer_color_merge_mode_e
{
    CTC_QOS_POLICER_COLOR_MERGE_AND,    /**< [D2.TM] 2 level policer color merge,the final color is low priority color */
    CTC_QOS_POLICER_COLOR_MERGE_OR,     /**< [D2.TM] 2 level policer color merge,the final color is high priority color */
    CTC_QOS_POLICER_COLOR_MERGE_DUAL,   /**< [D2.TM] 2 level policer color merge,if the micro color is green,the final color is green.the other situation is low priority color*/
};
typedef enum ctc_qos_policer_color_merge_mode_e ctc_qos_policer_color_merge_mode_t;

/**
 @brief Qos policer parameter
*/
struct ctc_qos_policer_param_s
{
    uint8  policer_mode;      /**< [GB.GG.D2.TM] policer mode refer to ctc_qos_policer_mode_t*/

    uint8  cf_en;             /**< [GB.GG.D2.TM]  Only for MEF BWP, couple flag enable*/
    uint8  is_color_aware;    /**< [GB.GG.D2.TM]  is_color_aware = 1 for is_color_aware, is_color_aware = 0 for Color-blind mode */

    uint8  use_l3_length;     /**< [GB.GG.D2.TM]  Use packet length from layer 3 header for metering */
    uint8  stats_en;          /**< [GB.GG.D2.TM]  enable policer stats for three color */
    uint8  stats_mode;        /**< [D2.TM]           stats mode, stats_mode = 0 for color-aware stats, stats_mode = 1 for color-blind stats */
    uint8  drop_color;        /**< [GB.GG.D2.TM]  the drop color after policing  */
    uint8  color_merge_mode;  /**< [D2.TM] 2 level policer color merge mode refer to ctc_qos_policer_color_merge_mode_t */

    uint32 cir;  /**< [GB.GG.D2.TM]  Committed Information Rate (unit :kbps; if set pps_en, unit :pps)*/
    uint32 cbs;  /**< [GB.GG.D2.TM]  Committed Burst Size, equivalent to the C-bucket size (unit :kb; if set pps_en, unit :pkt),
                                     cbs = CTC_MAX_UINT32_VALUE is max bucket size*/

    uint32 pir;  /**< [GB.GG.D2.TM]   PIR(RFC2698) or EIR(RFC4115,BWP) (unit :kbps; if set pps_en, unit :pps)*/
    uint32 pbs;  /**< [GB.GG.D2.TM]   PBS(RFC2698,)or EBS (RFC2697, RFC4115,BWP)(unit :kb; if set pps_en, unit :pkt),
                                      pbs = CTC_MAX_UINT32_VALUE is max bucket size*/
};
typedef struct ctc_qos_policer_param_s ctc_qos_policer_param_t;

/**
 @brief Qos policer Hierarchical Bandwidth Profile.
   1.1, when bwp is applied to MEF10.3,the relationship between the cos index
        and the cos level is as follows :
         Cos level    Cos index
        -------------------------
          3            1
          2            2
          1            3
        ------------------------
   1.2, when bwp is applied to triple_play mode,the cos index =0 is total policer
        service;
*/
struct ctc_qos_policer_hbwp_s
{
    uint8  cos_index;     /**< [GB.GG.D2.TM] Cos Level */
    uint8  sf_en;         /**< [GB.GG.D2.TM] Share flag enable*/
    uint8  triple_play;   /**< [GG] if set, cos_index = 0 regard as total policer */
    uint8  rsv;

    uint8  sp_en;         /**< [GB.GG] Strict schedule enable*/
    uint8  cf_total_dis;  /**< [GB.GG.D2.TM] Coupling total disable */
    uint16 weight;        /**< [GB.GG] Weight value<0-0x3FF>, only support cos 1 weight, cos 0 is (0x3FF - Weight)*/

    uint32 cir_max;      /**< [GB.GG.D2.TM] Max Cir rate,(unit:kbps; if set pps_en, unit :pps)*/
    uint32 pir_max;      /**< [GB.GG.D2.TM] Max Eir rate,(unit:kbps; if set pps_en, unit :pps)*/
};
typedef struct ctc_qos_policer_hbwp_s ctc_qos_policer_hbwp_t;

/**
 @brief Qos policer action flag
*/
enum ctc_qos_policer_action_flag_e
{
    CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID    = 1U << 0,  /**< [D2.TM] Green priority valid*/
    CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID   = 1U << 1,  /**< [D2.TM] Yellow priority valid*/
    CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID      = 1U << 2,  /**< [D2.TM] Red priority valid*/
    CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN              = 1U << 3,  /**< [D2.TM] Drop green */
    CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW             = 1U << 4,  /**< [D2.TM] Drop yellow */
    CTC_QOS_POLICER_ACTION_FLAG_DROP_RED                = 1U << 5  /**< [D2.TM] Drop red */
};
typedef enum ctc_qos_policer_action_flag_e ctc_qos_policer_action_flag_t;

/**
 @brief Qos policer action
*/
struct ctc_qos_policer_action_s
{
    uint32 flag;                /**< [D2.TM] qos action flag, refer to ctc_qos_policer_action_flag_t*/
    uint8  prio_green;          /**< [D2.TM] Green priority*/
    uint8  prio_yellow;         /**< [D2.TM] Yellow priority*/
    uint8  prio_red;            /**< [D2.TM] Red priority*/
    uint8  rsv;
};
typedef struct ctc_qos_policer_action_s ctc_qos_policer_action_t;


/**
 @brief Qos policer data structure
*/
struct ctc_qos_policer_s
{
    uint8  type;    /**< [GB.GG.D2.TM]  Policer type, refer to ctc_qos_policer_type_t*/
    uint8  dir;     /**< [GB.GG.D2.TM]  Ingress or egress , refer to ctc_direction_t, for Port policer and Vlan policer */
    uint8  enable;  /**< [GB.GG.D2.TM]  Policer enable or disable*/
    uint8  hbwp_en; /**< [GB.GG.D2.TM]     MEF BWP enable or disable*/

    uint8  level;   /**< [D2.TM]           Policer level, refer to ctc_qos_policer_level_t*/
    uint8  pps_en;  /**< [D2.TM]           Policer PPS mode enable*/

    union
    {
        uint16 policer_id;      /**< [GB.GG.D2.TM]  Flow policer and HBWP id <1-0xFFFF>*/
        uint16 service_id;      /**< [GB]  Service ID <1-0xFFFF>*/
        uint16 vlan_id;         /**< [D2.TM]  Vlan ID */
        uint32 gport;           /**< [GB.GG.D2.TM]  Global port*/
    } id;

    ctc_qos_policer_hbwp_t hbwp;       /**< [GB.GG.D2.TM]   MEF Bandwidth Profile*/
    ctc_qos_policer_param_t policer;   /**< [GB.GG.D2.TM] Policer parameter info*/
    ctc_qos_policer_action_t action;   /**< [D2.TM] Policer action*/
};
typedef struct ctc_qos_policer_s ctc_qos_policer_t;

/**
 @brief Qos policer stats info
*/
struct ctc_qos_policer_stats_info_s
{
    uint64 confirm_pkts;        /**< [GB.GG.D2.TM] Total number of packets with color green*/
    uint64 confirm_bytes;       /**< [GB.GG.D2.TM] Total bytes of packets with color green*/
    uint64 exceed_pkts;         /**< [GB.GG.D2.TM] Total number of packets with color yellow*/
    uint64 exceed_bytes;        /**< [GB.GG.D2.TM] Total bytes of packets with color yellow*/
    uint64 violate_pkts;        /**< [GB.GG.D2.TM] Total number of packets with color red*/
    uint64 violate_bytes;       /**< [GB.GG.D2.TM] Total bytes of packets with color red*/
};
typedef struct ctc_qos_policer_stats_info_s ctc_qos_policer_stats_info_t;

/**
 @brief Qos policer stats structure
*/
struct ctc_qos_policer_stats_s
{
    uint8 type;         /**< [GB.GG.D2.TM]ctc_qos_policer_type_t*/
    uint8 dir;          /**< [GB.GG.D2.TM]ctc_direction_t */
    uint8 hbwp_en;      /**< [GB.GG.D2.TM] MEF BWP enable or disable*/
    uint8 cos_index;    /**< [GB.GG.D2.TM] Cos Level*/

    union
    {
        uint16 policer_id;      /**< [GB.GG.D2.TM] Flow policer and BWP id <00xFFFF>*/
        uint16 service_id;      /**< [GB] Service ID <0xFFFF>*/
        uint16 vlan_id;         /**< [D2.TM]  Vlan ID */
        uint32 gport;           /**< [GB.GG.D2.TM] Global port*/
    } id;
	ctc_qos_policer_stats_info_t stats; /**< [GB.GG.D2.TM] Stats parameter*/

};
typedef struct ctc_qos_policer_stats_s ctc_qos_policer_stats_t;



/**
 @brief Qos global configure type
*/
enum ctc_qos_glb_cfg_type_e
{
    CTC_QOS_GLB_CFG_POLICER_EN,             /**< [GB.GG.D2.TM] Global policer update enable*/
    CTC_QOS_GLB_CFG_POLICER_STATS_EN,       /**< [GB.GG] Global policer statst enable*/
    CTC_QOS_GLB_CFG_POLICER_IPG_EN,         /**< [GB.GG.D2.TM] Global policer ipg enable*/
    CTC_QOS_GLB_CFG_POLICER_SEQENCE_EN,     /**< [GB.GG] Global policer port and flow sequential enable*/
    CTC_QOS_GLB_CFG_POLICER_FLOW_FIRST_EN,  /**< [GB.GG] Global policer port and flow plicer priority,
                                                         high 16 bit is dir, low 16bit is enable value*/

    CTC_QOS_GLB_CFG_RESRC_MGR_EN,           /**< [GB.GG.D2.TM] Global resource manger enable */
    CTC_QOS_GLB_CFG_QUE_SHAPE_EN,           /**< [GB.GG.D2.TM] Global queue shaping enable*/
    CTC_QOS_GLB_CFG_GROUP_SHAPE_EN,         /**< [GB.GG.D2.TM] Global group shaping enable*/
    CTC_QOS_GLB_CFG_PORT_SHAPE_EN,          /**< [GB.GG.D2.TM] Global port shaping  enable*/
    CTC_QOS_GLB_CFG_SHAPE_IPG_EN,           /**< [GB.GG.D2.TM]    Global queue shaping ipg enable*/
    CTC_QOS_GLB_CFG_QUE_STATS_EN,           /**< [GB.GG]    Global queue stats enable*/
    CTC_QOS_GLB_CFG_PHB_MAP,                /**< [GB.GG.D2.TM]    Priority mapped to phb*/
    CTC_QOS_GLB_CFG_REASON_SHAPE_PKT_EN,    /**< [GB.GG.D2.TM] Cpu Reason shape base on packet*/
    CTC_QOS_GLB_CFG_POLICER_HBWP_SHARE_EN,  /**< [GB.GG]    HBWP using global share mode*/
    CTC_QOS_GLB_CFG_QUEUE_DROP_MONITOR_EN,       /**< [GG.D2.TM]    Monitoring span on drop,per port enable*/
    CTC_QOS_GLB_CFG_TRUNCATION_LEN,         /**< [GG]  The length for packet truncation.*/
    CTC_QOS_GLB_CFG_POLICER_MARK_ECN_EN,    /**< [GG]  TCP ECN field will be mark when do policer*/
    CTC_QOS_GLB_CFG_SCH_WRR_EN,             /**< [GG.D2.TM]  Schedule WRR mode enable*/
    CTC_QOS_GLB_CFG_TABLE_MAP,              /**< [D2.TM]  QoS table map*/

	CTC_QOS_GLB_CFG_ECN_EN,                 /**< [D2.TM] Global ecn enable*/
    CTC_QOS_GLB_CFG_SHAPE_PKT_EN,      /**< [D2.TM] Global network port ,group,queue shaping pps enable*/
    CTC_QOS_GLB_CFG_FCDL_INTERVAL,          /**< [D2.TM] Global fc/pfc deadlock-detect interval time .unit :ms*/
    CTC_QOS_GLB_CFG_POLICER_PORT_STBM_EN,   /**< [D2.TM] Global port policer support stbm mode£¬if set£¬port policer will not support other mode*/
    CTC_QOS_GLB_CFG_MAX
};
typedef enum ctc_qos_glb_cfg_type_e ctc_qos_glb_cfg_type_t;

/**
 @brief Qos Phb map type
*/
enum ctc_qos_phb_map_type_e
{
   CTC_QOS_PHB_MAP_PRI,      /**< [GB.GG.D2.TM] Map priority to phb*/
   CTC_QOS_PHB_MAP_MAX
};
typedef enum ctc_qos_phb_map_type_e ctc_qos_phb_map_type_t;


struct ctc_qos_phb_map_s
{
    uint8 map_type;      /**< [GB.GG.D2.TM] Map type, refer to ctc_qos_phb_map_type_t*/
    uint8 priority;      /**< [GB.GG.D2.TM] Qos priority: <0-15>*/
    uint8 cos_index;     /**< [GB.GG.D2.TM] Regard as cos index */
};
typedef struct ctc_qos_phb_map_s ctc_qos_phb_map_t;


struct ctc_qos_queue_id_s
{
    uint8  queue_type; /**< [GB.GG.D2.TM] Queue type, refer to ctc_queue_type_t*/
    uint8  rsv;
    uint16 queue_id;   /**< [GB.GG.D2.TM] Queue id */

    uint32 gport;      /**< [GB.GG.D2.TM] Global port*/
    uint16 service_id; /**< [GB.D2.TM]    Service identification*/
    uint16 cpu_reason; /**< [GB.GG.D2.TM] Cpu reason*/

};
typedef struct ctc_qos_queue_id_s ctc_qos_queue_id_t;

/**
 @brief monitor drop configure
*/
struct ctc_qos_queue_drop_monitor_s
{
     uint32 src_gport;               /**< [GG.D2.TM] The source port of monitor drop, network port */
     uint32 dst_gport;               /**< [GG.D2.TM] The globally unique destination port of monitor drop, network port */
     uint8 enable;                   /**< [GG.D2.TM] Queue drop monitor source port enable*/
     uint8 rsv[3];
};
typedef struct ctc_qos_queue_drop_monitor_s ctc_qos_queue_drop_monitor_t;

/**
 @brief Policer level select
*/
struct ctc_qos_policer_level_select_s
{
     uint8 ingress_port_level;           /**< [D2.TM] Ingress port policer level,refer to ctc_qos_policer_level_t*/
     uint8 ingress_vlan_level;           /**< [D2.TM] Ingress vlan policer level,refer to ctc_qos_policer_level_t*/
     uint8 egress_port_level;            /**< [D2.TM] Egress port policer level,refer to ctc_qos_policer_level_t*/
     uint8 egress_vlan_level;            /**< [D2.TM] Egress vlan policer level,refer to ctc_qos_policer_level_t*/
};
typedef struct ctc_qos_policer_level_select_s ctc_qos_policer_level_select_t;

/**
 @brief Qos global configure
*/
struct ctc_qos_glb_cfg_s
{
    uint16 cfg_type;                 /**< [GB.GG.D2.TM] Config type, refer to ctc_qos_glb_cfg_type_t*/
    uint16 rsv;
    union
    {
        uint32 value;                /**< [GB.GG.D2.TM] Config value*/
        ctc_qos_phb_map_t phb_map;   /**< [GB.GG.D2.TM] Phb mapping configure*/
        ctc_qos_queue_drop_monitor_t drop_monitor; /**< [GG.D.TM2] Config monitor drop*/
        ctc_qos_table_map_t table_map; /**< [D2.TM] QoS table map*/
    } u;
};
typedef struct ctc_qos_glb_cfg_s ctc_qos_glb_cfg_t;

enum ctc_qos_inter_cn_e
{
    CTC_QOS_INTER_CN_DROP,                /**< [D2.TM] Responsive wred dropping */
    CTC_QOS_INTER_CN_NON_DROP,            /**< [D2.TM] Non responsive wred dropping */
    CTC_QOS_INTER_CN_NON_CE,              /**< [D2.TM] Support ECN and not congestion experienced*/
    CTC_QOS_INTER_CN_CE,                  /**< [D2.TM] Support ECN and congestion experienced */
    CTC_QOS_INTER_CN_MAX
};
typedef enum ctc_qos_inter_cn_e ctc_qos_inter_cn_t;

/**
 @brief Qos drop mode
*/
enum ctc_queue_drop_mode_s
{
    CTC_QUEUE_DROP_WTD,     /**< [GB.GG.D2.TM] WTD drop mode */
    CTC_QUEUE_DROP_WRED,    /**< [GB.GG.D2.TM] WRED drop mode */

    MAX_CTC_QUEUE_DROP
};
typedef enum ctc_queue_drop_mode_s ctc_queue_drop_mode_t;

/**
 @brief Qos queue drop
*/
struct ctc_qos_queue_drop_s
{
    ctc_queue_drop_mode_t mode;          /**< [GB.GG.D2.TM] Queue Drop Mode, type of CTC_QUEUE_DROP_XXX */

    uint16 min_th[CTC_DROP_PREC_NUM];    /**< [GB.D2.TM] WRED min threshold */
    uint16 max_th[CTC_DROP_PREC_NUM];    /**< [GB.GG.D2.TM] WRED max threshold, or WTD drop threshold */
    uint16 drop_prob[CTC_DROP_PREC_NUM]; /**< [GB.GG.D2.TM] WRED max drop probability */
    uint8  is_coexist;                   /**< [D2.TM] WTD and WRED coexist */
    uint16 ecn_mark_th;                  /**< [GG] If ecn_mark_th equal 0, ecn mark disable. else mean queue ecn mark threshold.
                                                  ecn_mark_th should less than max_th*/
    uint16 ecn_th[CTC_DROP_PREC_NUM];    /**< [D2.TM] Queue ecn mark threshold.ecn_th should less than max_th*/
    uint8 drop_factor[CTC_DROP_PREC_NUM];    /**< [D2.TM] f:0-1/128, 1-1/64, 2-1/32, 3-1/16, 4-1/8, 5-1/4, 6-1/2, 7-1, 8-2, 9-4, 10-8
                                                  threshold = (f / (1 + f)) * remainCnt (f = 1/128 ~ 8) */
    uint8 is_dynamic;                    /**< [D2.TM] WTD drop threshold is dynamic*/
    uint8 rsv[3];
};
typedef struct ctc_qos_queue_drop_s ctc_qos_queue_drop_t;




/**
 @brief Qos queue drop data structure
*/
struct ctc_qos_drop_s
{
    ctc_qos_queue_id_t queue;   /**< [GB.GG.D2.TM] Generate queue id information*/
    ctc_qos_queue_drop_t drop;  /**< [GB.GG.D2.TM] Drop info, refer to ctc_qos_queue_drop_t*/
};
typedef struct ctc_qos_drop_s ctc_qos_drop_t;

/**
 @brief Qos queue stats info
*/
struct ctc_qos_queue_stats_info_s
{
    uint64 deq_packets;     /**< [GB.GG.D2.TM] De-queue packet number */
    uint64 deq_bytes;       /**< [GB.GG.D2.TM] De-queue packet bytes */
    uint64 drop_packets;    /**< [GB.GG.D2.TM] Dropped packet number */
    uint64 drop_bytes;      /**< [GB.GG.D2.TM] Dropped packet bytes */
};
typedef struct ctc_qos_queue_stats_info_s ctc_qos_queue_stats_info_t;

/**
 @brief Qos queue stats data structure
*/
struct ctc_qos_queue_stats_s
{
    ctc_qos_queue_id_t queue;   /**< [GB.GG.D2.TM] Generate queue id information*/

    uint8 stats_en;             /**< [GB.GG.D2.TM] */

    ctc_qos_queue_stats_info_t stats;   /**< [GB.GG.D2.TM] */

};
typedef struct ctc_qos_queue_stats_s ctc_qos_queue_stats_t;


/**
 @brief Qos queue packet len adjust data structure
*/
struct ctc_qos_queue_pkt_adjust_s
{
    ctc_qos_queue_id_t queue;     /**< [GB.GG.D2.TM] Generate queue id information*/
    uint8 adjust_len;             /**< [GB.GG.D2.TM] packet length adjust for shaping,
                                               if MSB set, mean minus packet length */
};
typedef struct ctc_qos_queue_pkt_adjust_s ctc_qos_queue_pkt_adjust_t;

/**
 @brief Qos queue type
*/
enum ctc_queue_type_e
{
    CTC_QUEUE_TYPE_SERVICE_INGRESS,     /**< [GB.D2.TM] Ingress service queue */
    CTC_QUEUE_TYPE_NETWORK_EGRESS,      /**< [GB.GG.D2.TM] Egress network queue */
    CTC_QUEUE_TYPE_ILOOP,               /**< [GG.D2.TM] Ingress loopback queue */
    CTC_QUEUE_TYPE_NORMAL_CPU,          /**< [GB] Normal CPU queue */
    CTC_QUEUE_TYPE_OAM,                 /**< [GB.GG.D2.TM] OAM queue */
    CTC_QUEUE_TYPE_ELOOP,               /**< [GB.GG.D2.TM] Egress loopback queue */
    CTC_QUEUE_TYPE_EXCP_CPU,            /**< [GB.GG.D2.TM] Exception CPU queue */
    CTC_QUEUE_TYPE_FABRIC,              /**< [GB]    Fabric queue */
    CTC_QUEUE_TYPE_INTERNAL_PORT,       /**< [GB.GG.D2.TM] Internal port queue */

    MAX_CTC_QUEUE_TYPE
};
typedef enum ctc_queue_type_e ctc_queue_type_t;

/**
 @brief Qos queue configure type
*/
enum ctc_qos_queue_cfg_type_e
{
    CTC_QOS_QUEUE_CFG_SERVICE_BIND,             /**< [GB.GG.D2.TM] Service bind information*/
    CTC_QOS_QUEUE_CFG_PRI_MAP,                  /**< [GB.GG.D2.TM]    Priority to queue select configure*/
    CTC_QOS_QUEUE_CFG_QUEUE_NUM,                /**< [GB.GG]    Queue number set for normal queue*/
    CTC_QOS_QUEUE_CFG_QUEUE_REASON_MAP,         /**< [GB.GG.D2.TM]    Cpu reason mapping queue*/
    CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST,        /**< [GB.GG.D2.TM]    Cpu reason set destination*/
    CTC_QOS_QUEUE_CFG_QUEUE_REASON_PRIORITY,    /**< [GB.GG]       Cpu reason set priority*/
    CTC_QOS_QUEUE_CFG_QUEUE_REASON_MODE,        /**< [GB]       Cpu reason set mode DMA/ETH to cpu*/
    CTC_QOS_QUEUE_CFG_QUEUE_REASON_MISC,        /**< [GG]      Set Cpu reason parameter*/
    CTC_QOS_QUEUE_CFG_QUEUE_STATS_EN,           /**< [GB.GG.D2.TM]    Queue stats enale configure*/
    CTC_QOS_QUEUE_CFG_SCHED_WDRR_MTU,           /**< [GB.GG.D2.TM] Schedule wdrr mtu size configure*/
    CTC_QOS_QUEUE_CFG_LENGTH_ADJUST,            /**< [GB.GG.D2.TM]    Adjust length for shaping, the length can be negative*/
    CTC_QOS_QUEUE_CFG_MAX
};
typedef enum ctc_qos_queue_cfg_type_e ctc_qos_queue_cfg_type_t;

/**
 @brief Qos service id opcode
*/
enum ctc_qos_service_id_opcode_s
{
    CTC_QOS_SERVICE_ID_CREATE                   = 0, /**< [GB.D2.TM] Create service id with queue */
    CTC_QOS_SERVICE_ID_DESTROY                  = 1, /**< [GB.D2.TM] Destroy service id */
    CTC_QOS_SERVICE_ID_BIND_DESTPORT            = 3, /**< [GB.D2.TM] Service id binding with queue */
    CTC_QOS_SERVICE_ID_UNBIND_DESTPORT          = 4, /**< [GB.D2.TM] Service id unbinding with queue */
    MAX_CTC_QOS_SERVICE_ID_OPCODE
};
typedef enum ctc_qos_service_id_opcode_s ctc_qos_service_id_opcode_t;

/**
 @brief Qos service information
*/
struct ctc_qos_service_info_s
{
    uint16 flag;        /**< [GB.D2.TM] ctc_qos_queue_cfg_type_t */
    uint8  opcode;      /**< [GB.D2.TM] ctc_qos_service_id_opcode_t */
    uint8  dir;         /**< [TM] Direction, refer to ctc_direction_t */
    uint16 service_id;  /**< [GB.D2.TM] Service identifciation*/
    uint32 gport;       /**< [GB.D2.TM] Service bind port*/
};
typedef struct ctc_qos_service_info_s ctc_qos_service_info_t;

/**
 @brief Qos priority maping
*/
struct ctc_qos_pri_map_s
{
    uint8 priority;             /**< [GB.GG.D2.TM] priority <0-15> */
    uint8 color;                /**< [GB.GG.D2.TM] color 1-red 2-yellow 3-green */
    uint8 queue_select;         /**< [GB.GG.D2.TM] mapped queue select <0-15>; for D2 TM, mapped queue select <0-7> */
    uint8 drop_precedence;      /**< [GB.GG.D2.TM] mapped drop precedence <0-3> */
};
typedef struct ctc_qos_pri_map_s ctc_qos_pri_map_t;

/**
 @brief Qos queue number
*/
struct ctc_qos_queue_number_s
{
    uint8 queue_type;   /**< [GB] Queue type, refer to ctc_queue_type_t*/
    uint8 queue_number; /**< [GB] Queue number binding*/

};
typedef struct ctc_qos_queue_number_s ctc_qos_queue_number_t;

/**
 @brief Qos reason queue map when cpu reason 's destination is local cpu
*/
struct ctc_qos_queue_cpu_reason_map_s
{
    uint16 cpu_reason;     /**< [GB.GG.D2.TM] Cpu reason type, refer to ctc_pkt_cpu_reason_t*/
    uint8 reason_group;   /**< [GB.GG.D2.TM] Cpu reason group */
    uint8 queue_id;       /**< [GB.GG.D2.TM] Cpu reason queue id*/
};
typedef struct ctc_qos_queue_cpu_reason_map_s ctc_qos_queue_cpu_reason_map_t;

/**
 @brief Qos reason queue destination
*/
struct ctc_qos_queue_cpu_reason_dest_s
{
    uint16  cpu_reason;   /**< [GB.GG.D2.TM] Cpu reason type, refer to ctc_pkt_cpu_reason_t*/
    uint8  dest_type;    /**< [GB.GG.D2.TM] Cpu reason type, refer to ctc_pkt_cpu_reason_dest_t*/
    uint32 dest_port;    /**< [GB.GG.D2.TM] Value means enable (dest_port != 0) or disable (dest_port == 0) if dest_type is CTC_PKT_CPU_REASON_DISCARD_CANCEL_TO_CPU,
                                  remote cpu port if dest_type is CTC_PKT_CPU_REASON_TO_REMOTE_CPU */
	uint32 nhid;        /**< [GB.GG.D2.TM] nhid if dest_type is CTC_PKT_CPU_REASON_TO_NHID */
};
typedef struct ctc_qos_queue_cpu_reason_dest_s ctc_qos_queue_cpu_reason_dest_t;

/**
 @brief Qos reason priority
*/
struct ctc_qos_queue_cpu_reason_priority_s
{
    uint16 cpu_reason;     /**< [GB] Cpu reason type, refer to ctc_pkt_cpu_reason_t */
    uint8 cos;            /**< [GB] Cpu reason cos, will mapping to channel ID for DMA, or cos in CPUMAC header */
    uint8 rsv[1];
};
typedef struct ctc_qos_queue_cpu_reason_priority_s ctc_qos_queue_cpu_reason_priority_t;

/**
 @brief Qos reason
*/
struct ctc_qos_group_cpu_reason_mode_s
{
    uint8 group_type;       /**< [GB]  Cpu reason group type, refer to ctc_pkt_cpu_reason_group_type_t */
    uint8 group_id;         /**< [GB]  Cpu reason group <0-15>, valid if group_type is CTC_PKT_CPU_REASON_GROUP_EXCEPTION */
    uint8 mode;             /**< [GB]  The mode for packet to cpu, refer to ctc_pkt_mode_t */
    uint8 rsv;
};
typedef struct ctc_qos_group_cpu_reason_mode_s ctc_qos_group_cpu_reason_mode_t;


/**
 @brief Qos reason
*/
struct ctc_qos_group_cpu_reason_misc_s
{
    uint16 cpu_reason;       /**< [GG] Cpu reason type, refer to ctc_pkt_cpu_reason_t */
    uint8 truncation_en;     /**< [GG] Packet will be truncated. The truncation length refer to CTC_QOS_GLB_CFG_TRUNCATION_LEN for GG */
    uint8 rsv[1];
};
typedef struct ctc_qos_group_cpu_reason_misc_s ctc_qos_group_cpu_reason_misc_t;

/**
 @brief Qos queue configure
*/
struct ctc_qos_queue_cfg_s
{
    uint8 type; /**< [GB.GG.D2.TM] Queue configure type, refer to ctc_qos_queue_cfg_type_t*/
    uint8 rsv[3];
    union
    {
        ctc_qos_service_info_t srv_queue_info;  /**< [GB.D2.TM] Service queue info*/
        ctc_qos_pri_map_t  pri_map;             /**< [GB.GG.D2.TM] Priority to queue maping*/
        ctc_qos_queue_number_t queue_num;       /**< [GB]    Queue number config*/
        ctc_qos_queue_cpu_reason_map_t reason_map;   /**< [GB.GG.D2.TM]    Cpu reason mapping queue*/
        ctc_qos_queue_cpu_reason_dest_t reason_dest; /**< [GB.GG.D2.TM]    Cpu reason set destination*/
        ctc_qos_queue_cpu_reason_priority_t reason_pri; /**< [GB]    Cpu reason set priority*/
        ctc_qos_group_cpu_reason_mode_t     reason_mode; /**< [GB]    Cpu reason set mode DMA/ETH*/
		ctc_qos_group_cpu_reason_misc_t    reason_misc;  /**< [GG.D2.TM]    cpu reason parameter */
        ctc_qos_queue_stats_t stats;            /**< [GB.GG.D2.TM]    Queue stats enable */
        ctc_qos_queue_pkt_adjust_t pkt;         /**< [GB.GG.D2.TM]    Adjust the packet length which equalize the shaping rate*/
        uint32 value_32;                        /**< [GB.GG.D2.TM] Uint32 value according to ctc_qos_queue_cfg_type_t */
    } value;
};
typedef struct ctc_qos_queue_cfg_s ctc_qos_queue_cfg_t;

/**
 @brief Qos shape type
*/
enum ctc_qos_shape_type_e
{
    CTC_QOS_SHAPE_PORT,   /**< [GB.GG.D2.TM] Port shaping*/
    CTC_QOS_SHAPE_QUEUE,  /**< [GB.GG.D2.TM] Queue shaping*/
    CTC_QOS_SHAPE_GROUP,  /**< [GB.GG.D2.TM] Group shaping*/
    CTC_QOS_SHAPE_MAX
};
typedef enum ctc_qos_shape_type_e ctc_qos_shape_type_t;

/**
 @brief Qos queue shape
*/
struct ctc_qos_shape_queue_s
{
    ctc_qos_queue_id_t queue; /**< [GB.GG.D2.TM] Generate queue id
                                 information*/

    uint8  enable;            /**< [GB.GG.D2.TM] Queue shaping enable*/
    uint8  rsv0[3];

    uint32  pir;              /**< [GB.GG.D2.TM] Max rate,
                                 Peak Information Rate (unit :kbps/pps)*/

    uint32  pbs;             /**< [GB.GG.D2.TM] Peak Burst Size, (unit :kb/pkt)
                                if set CTC_QOS_SHP_TOKE_THRD_DEFAULT,
                                default value will be set */

    uint32  cir;  /**< [GB.GG.D2.TM]  Committed Information Rate (unit :kbps/pps),
                       CIR = 0 is CIR Fail; cir = CTC_MAX_UINT32_VALUE is CIR pass*/
    uint32  cbs;  /**< [GB.GG.D2.TM]  Committed Burst Size, (unit :kb/pkt),
                        if set CTC_QOS_SHP_TOKE_THRD_DEFAULT,
                        default value will be set */

};
typedef struct ctc_qos_shape_queue_s ctc_qos_shape_queue_t;

/**
 @brief Qos group shape
*/
struct ctc_qos_shape_group_s
{
    uint16 service_id;      /**< [GB]  Service identification*/
    uint8  group_type;      /*[GB]group type, refer to ctc_qos_sched_group_type_t*/
    uint32 gport;           /**< [GB.GG]     Dest Port*/
    ctc_qos_queue_id_t queue;   /**<[GB.GG.D2.TM] Generate queue id information*/

    uint8  enable;          /**< [GB.GG.D2.TM]  Group shaping enable*/
    uint8 rsv0[3];
    uint32  pir;            /**< [GB.GG.D2.TM] group shaping commit rate,(unit :kbps/pps)*/
    uint32  pbs;            /**< [GB.GG.D2.TM] group shaping bucket size,(unit :kb/pkt),
                                  if set CTC_QOS_SHP_TOKE_THRD_DEFAULT,
                                  default value will be set*/
};
typedef struct ctc_qos_shape_group_s ctc_qos_shape_group_t;

/**
 @brief Qos port shape
*/
struct ctc_qos_shape_port_s
{
    uint32 gport;       /**< [GB.GG.D2.TM] Global port*/
    uint8  enable;      /**< [GB.GG.D2.TM] Port shaping enable*/
    uint8  sub_dest_id; /**< [GG] only valid for local cpu port, range <0-3> */
    uint16 rsv;

    uint32  pir;       /**< [GB.GG.D2.TM] Peak Information Rate,
                            (unit :kbps/pps)*/
    uint32  pbs;       /**< [GB.GG.D2.TM]  Peak Burst Size,(unit :kb/pkt),
                         if set CTC_QOS_SHP_TOKE_THRD_DEFAULT,
                         default value will be set */

    uint32  ecn_mark_rate; /*[GG.TM] ECN will be marked when output port rate reach ecn_mark_rate, (unit :kbps)*/

};
typedef struct ctc_qos_shape_port_s ctc_qos_shape_port_t;

/**
 @brief Qos shape data structure
*/
struct ctc_qos_shape_s
{
    uint8 type;        /**< [GB.GG.D2.TM] Shaping type, refer to ctc_qos_shape_type_t*/
    uint8 rsv0[3];

    union
    {
        ctc_qos_shape_queue_t queue_shape; /**< [GB.GG.D2.TM] Queue shaping*/
        ctc_qos_shape_group_t group_shape; /**< [GB.GG.D2.TM] Group shaping*/
        ctc_qos_shape_port_t port_shape;   /**< [GB.GG.D2.TM] Port shaping*/
    } shape;
};
typedef struct ctc_qos_shape_s ctc_qos_shape_t;

/**
 @brief Qos schedule type
*/
enum ctc_qos_sched_type_e
{
    CTC_QOS_SCHED_QUEUE,        /**< [GB.GG.D2.TM] Queue level schedule*/
    CTC_QOS_SCHED_GROUP,        /**< [GB.GG.D2.TM]    Group level schedule*/
    CTC_QOS_SCHED_MAX
};
typedef enum ctc_qos_sched_type_e ctc_qos_sched_type_t;

/**
 @brief Qos schedule group type
*/
enum ctc_qos_sched_group_type_e
{
    CTC_QOS_SCHED_GROUP_PORT,       /**< [GB] Schedule is port group*/
    CTC_QOS_SCHED_GROUP_SERVICE,    /**< [GB] Schedule is service group*/
    CTC_QOS_SCHED_GROUP_MAX
};
typedef enum ctc_qos_sched_group_type_e ctc_qos_sched_group_type_t;

/**
 @brief Qos schedule configure type
*/
enum ctc_qos_sched_cfg_type_e
{
    CTC_QOS_SCHED_CFG_CONFIRM_WEIGHT,   /**< [GB.GG.D2.TM]      Confirm (green color) weight*/
    CTC_QOS_SCHED_CFG_EXCEED_WEIGHT,    /**< [GB.GG.D2.TM]  Exceed (yellow color) weight*/
    CTC_QOS_SCHED_CFG_CONFIRM_CLASS,    /**< [GB.GG.D2.TM]   Confirm (green color) class*/
    CTC_QOS_SCHED_CFG_EXCEED_CLASS,     /**< [GB.GG.D2.TM]  Exceed (yellow color) class*/
    CTC_QOS_SCHED_CFG_SUB_GROUP_ID,     /**< [D2.TM]  Add queue class to sub group*/
    CTC_QOS_SCHED_CFG_SUB_GROUP_WEIGHT, /**< [D2.TM]  Sub group weight */
    CTC_QOS_SCHED_CFG_MAX
};
typedef enum ctc_qos_sched_cfg_type_e ctc_qos_sched_cfg_type_t;

/**
 @brief Qos queue level schedule
*/
struct ctc_qos_sched_queue_s
{
    uint8  cfg_type;            /**< [GB.GG.D2.TM] Configure type, refer to ctc_qos_sched_cfg_type_t */

    ctc_qos_queue_id_t queue;   /**< [GB.GG.D2.TM] Generate queue id information*/

    uint16 confirm_weight;      /**< [GB] Comfirmed weight for green packet*/
    uint16 exceed_weight;       /**< [GB.GG.D2.TM]  Exceeded  weight for yellow packet*/

    uint8 confirm_class;        /**< [GB.GG] queue class for green packet */
    uint8 exceed_class;         /**< [GB.GG.D2.TM]    queue class for yellow packet */

};
typedef struct ctc_qos_sched_queue_s ctc_qos_sched_queue_t;

/**
 @brief Qos group level schedule
*/
struct ctc_qos_sched_group_s
{
    uint32 gport;               /**< [GB] Global port(input)*/
    uint16 service_id;          /**< [GB] Service identifaication(input)*/
    uint8 group_type;           /**< [GB]Scheudle type, refer to ctc_qos_sched_group_type_t*/
    uint8 cfg_type;             /**< [GB.GG.D2.TM]Configure type, refer to ctc_qos_sched_cfg_type_t*/

    ctc_qos_queue_id_t queue;   /**< [GB.GG.D2.TM] Generate queue id information*/

    uint8 queue_class;          /**< [GB.GG.D2.TM] Queue class include confirm class and exceed class*/
    uint8 class_priority;       /**< [GB.GG.D2.TM] Class priority*/

    uint8 weight;               /**< [GB.GG.D2.TM] Schedule weight*/
    uint8 sub_group_id;         /**< [D2.TM] Sub group id*/
};
typedef struct ctc_qos_sched_group_s ctc_qos_sched_group_t;

/**
 @brief Qos schedule data structure
*/
struct ctc_qos_sched_s
{
    uint8 type;         /**< [GB.GG.D2.TM] Scheudle type, refer to ctc_qos_sched_type_t */
    uint8 rsv0[3];

    union
    {
        ctc_qos_sched_queue_t     queue_sched;      /**< [GB.GG.D2.TM] Queue level schedule*/
        ctc_qos_sched_group_t     group_sched;      /**< [GB.GG.D2.TM]    Group level schedule*/
    } sched;
};
typedef struct ctc_qos_sched_s ctc_qos_sched_t;

/**
 @brief Qos ingress resource pools type
*/
enum ctc_qos_igs_resrc_pool_type_e
{
    CTC_QOS_IGS_RESRC_DEFAULT_POOL,      /**< [GB.GG.D2.TM] Default pool */
    CTC_QOS_IGS_RESRC_NON_DROP_POOL,     /**< [GB.GG.D2.TM] Non-drop pool */
    CTC_QOS_IGS_RESRC_MIN_POOL,          /**< [GB.GG.D2.TM] Min guarantee pool */
    CTC_QOS_IGS_RESRC_C2C_POOL,          /**< [GB.GG.D2.TM] CPU to CPU pool */
    CTC_QOS_IGS_RESRC_ROUND_TRIP_POOL,   /**< [GG] Round trip pool */
    CTC_QOS_IGS_RESRC_CONTROL_POOL,      /**< [GB.GG.D2.TM] Control pool, from cpu or to cpu */

    CTC_QOS_IGS_RESRC_POOL_MAX
};
typedef enum ctc_qos_igs_resrc_pool_type_e ctc_qos_igs_resrc_pool_type_t;

/**
 @brief Qos egress resource pools type
*/
enum ctc_qos_egs_resrc_pool_type_e
{
    CTC_QOS_EGS_RESRC_DEFAULT_POOL,      /**< [GB.GG.D2.TM] Default pool, support congest level */
    CTC_QOS_EGS_RESRC_NON_DROP_POOL,     /**< [GB.GG.D2.TM] Non-drop pool */
    CTC_QOS_EGS_RESRC_SPAN_POOL,         /**< [GG.D2.TM] SPAN and mcast pool, support congest level */
    CTC_QOS_EGS_RESRC_CONTROL_POOL,      /**< [GB.GG.D2.TM] Control pool, from cpu or to cpu */
    CTC_QOS_EGS_RESRC_MIN_POOL,          /**< [GB.GG.D2.TM] Min guarantee pool */
    CTC_QOS_EGS_RESRC_C2C_POOL,          /**< [GB.GG.D2.TM] CPU to CPU pool */

    CTC_QOS_EGS_RESRC_POOL_MAX
};
typedef enum ctc_qos_egs_resrc_pool_type_e ctc_qos_egs_resrc_pool_type_t;

/**
 @brief Qos resource configure type
*/
enum ctc_qos_resrc_cfg_type_e
{
    CTC_QOS_RESRC_CFG_POOL_CLASSIFY,        /**< [GB.GG.D2.TM] Priority => pool */
    CTC_QOS_RESRC_CFG_PORT_MIN,             /**< [GB.GG.D2.TM] Port min guarantee threshold */
    CTC_QOS_RESRC_CFG_QUEUE_DROP,           /**< [GB.GG.D2.TM] Config queue drop profile */
    CTC_QOS_RESRC_CFG_FLOW_CTL,             /**< [GB.GG.D2.TM] Config queue flow control threshold */
    CTC_QOS_RESRC_CFG_FCDL_DETECT,          /**< [D2.TM] Config queue flow control deadlock-detect*/
    CTC_QOS_RESRC_CFG_MAX
};
typedef enum ctc_qos_resrc_cfg_type_e ctc_qos_resrc_cfg_type_t;

/**
 @brief Qos resource classify pool
*/
struct ctc_qos_resrc_classify_pool_s
{
    uint32 gport;               /**< [GB.GG.D2.TM] Global port */
    uint8  priority;            /**< [GB.GG.D2.TM] Qos priority: <0-15>*/
    uint8  dir;                 /**< [GB.GG.D2.TM] Direction, refer to ctc_direction_t */

    uint8  pool;                /**< [GB.GG.D2.TM] refer to ctc_qos_igs_resrc_pool_type_t and ctc_qos_egs_resrc_pool_type_t */
};
typedef struct ctc_qos_resrc_classify_pool_s ctc_qos_resrc_classify_pool_t;

/**
 @brief Qos resource port min guarante threshold config
*/
struct ctc_qos_resrc_port_min_s
{
    uint32 gport;               /**< [GB.GG.D2.TM] Global port */
    uint8  dir;                 /**< [GB.GG.D2.TM] Direction, refer to ctc_direction_t */

    uint16 threshold;           /**< [GB.GG.D2.TM] Port min guarantee threshold, uint is buffer cell <0-255> */
};
typedef struct ctc_qos_resrc_port_min_s ctc_qos_resrc_port_min_t;

struct ctc_qos_resrc_fc_s
{
    uint32 gport;               /**< [GB.GG.D2.TM] Global port */
    uint8  priority_class;      /**< [GB.GG.D2.TM] Pfc priority class <0-7>, value should be priority/8, for D2 TM, value should be priority/2 */
    uint8 is_pfc;               /**< [GB.GG.D2.TM] Is pfc */

    uint16 xon_thrd;            /**< [GB.GG.D2.TM] Xon threshold which triger send xon pause frame */
    uint16 xoff_thrd;           /**< [GB.GG.D2.TM] Xoff threshold which triger send xoff pause frame */
    uint16 drop_thrd;           /**< [GB.GG.D2.TM] Drop threshold which packet will be drop  */
};
typedef struct ctc_qos_resrc_fc_s ctc_qos_resrc_fc_t;

struct ctc_qos_fcdl_event_s
{
    uint32 gport;                             /**< [D2.TM] Global port */
    uint8  state[CTC_MAX_FCDL_DETECT_NUM];    /**< [D2.TM] Flowctl deadlock-detect  state*/
};
typedef struct ctc_qos_fcdl_event_s ctc_qos_fcdl_event_t;

struct ctc_qos_resrc_fcdl_detect_s
{
    uint32 gport;                                  /**< [D2.TM]  Global port */
    uint8  priority_class[CTC_MAX_FCDL_DETECT_NUM];/**< [D2.TM]  Pfc deadlock-detect  priority class, value should be priority/2*/
    uint8  enable;                                 /**< [D2.TM]  Pfc/fc deadlock-detect  enable*/
    uint8  valid_num;                              /**< [D2.TM]  Valid num for priority_class*/
    uint8  detect_mult;                            /**< [D2.TM]  Pfc/fc deadlock-detect multiplier */
};
typedef struct ctc_qos_resrc_fcdl_detect_s ctc_qos_resrc_fcdl_detect_t;


typedef ctc_qos_drop_t  ctc_qos_drop_array[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];
/**
 @brief Qos resource mangement data structure
*/
struct ctc_qos_resrc_s
{
    uint8  cfg_type;            /**< [GB.GG.D2.TM] Configure type, refer to ctc_qos_resrc_cfg_type_t*/

    union
    {
        ctc_qos_resrc_classify_pool_t pool;     /**< [GB.GG.D2.TM] classify pool */
        ctc_qos_resrc_port_min_t port_min;      /**< [GB.GG.D2.TM] set ingress/egress port guarantee min threshold */
        ctc_qos_drop_array queue_drop;          /**< [GB.GG] set queue drop of 8 congest level */
        ctc_qos_resrc_fc_t flow_ctl;            /**< [GB.GG.D2.TM] set flow Control resource threshold */
        ctc_qos_resrc_fcdl_detect_t fcdl_detect;/**< [D2.TM] set flow Control deadlock-detect */
    }u;
};
typedef struct ctc_qos_resrc_s ctc_qos_resrc_t;

/**
 @brief Qos resource pool stats type
*/
enum ctc_qos_resrc_pool_stats_type_e
{
    CTC_QOS_RESRC_STATS_IGS_POOL_COUNT,         /**< [GB.GG.D2.TM] ingress pool instant count */
    CTC_QOS_RESRC_STATS_IGS_TOTAL_COUNT,        /**< [GB.GG.D2.TM] ingress total instant count */
    CTC_QOS_RESRC_STATS_EGS_POOL_COUNT,         /**< [GB.GG.D2.TM] egress pool instant count */
    CTC_QOS_RESRC_STATS_EGS_TOTAL_COUNT,        /**< [GB.GG.D2.TM] egress total instant count */
    CTC_QOS_RESRC_STATS_QUEUE_COUNT,            /**< [GB.GG.D2.TM] queue instant count */

    CTC_QOS_RESRC_STATS_COUNT_MAX
};
typedef enum ctc_qos_resrc_pool_stats_type_e ctc_qos_resrc_pool_stats_type_t;

/**
 @brief Qos resource pool stats
*/
struct ctc_qos_resrc_pool_stats_s
{
    uint8  type;                /**< [GB.GG.D2.TM] stats type, refer to ctc_qos_resrc_pool_stats_type_t */
    uint8  rsv[3];

    ctc_qos_queue_id_t queue;   /**< [GB.GG.D2.TM] Generate queue id information*/
    uint32 gport;               /**< [GB.GG.D2.TM] Global port */
    uint8  priority;            /**< [GB.GG.D2.TM] Qos priority: <0-15> */
    uint8  pool;                /**< [GB.GG.D2.TM] refer to ctc_qos_igs_resrc_pool_type_t and ctc_qos_egs_resrc_pool_type_t */

    uint16 count;               /**< [GB.GG.D2.TM] instant count */
};
typedef struct ctc_qos_resrc_pool_stats_s ctc_qos_resrc_pool_stats_t;

/**
 @brief Qos resource mangement drop profile
*/
struct ctc_qos_resrc_drop_profile_s
{
    uint8  congest_level_num;       /**< [GB.GG] congest level num <1-8> */
    uint16 congest_threshold[CTC_RESRC_MAX_CONGEST_LEVEL_THRD_NUM];         /**< [GB.GG] congest threshold for different level */
    ctc_qos_queue_drop_t queue_drop[CTC_RESRC_MAX_CONGEST_LEVEL_NUM];       /**< [GB.GG] queue threshold for different level */
};
typedef struct ctc_qos_resrc_drop_profile_s ctc_qos_resrc_drop_profile_t;

/**
 @brief Qos resource profile
*/
enum ctc_qos_resrc_pool_mode_e
{
    CTC_QOS_RESRC_POOL_DISABLE,          /**< [GB.GG.D2.TM] Disable resource managent. For ingress only have default pool; for egress, have default and control pool */
    CTC_QOS_RESRC_POOL_MODE1,            /**< [GB.GG.D2.TM] disable span and non-drop pool */
    CTC_QOS_RESRC_POOL_MODE2,            /**< [GB.GG.D2.TM] all pool used, and some pool map is done */
    CTC_QOS_RESRC_POOL_USER_DEFINE,      /**< [GB.GG.D2.TM] User define mode */

    CTC_QOS_RESRC_POOL_MODE_MAX
};
typedef enum ctc_qos_resrc_pool_mode_e ctc_qos_resrc_pool_mode_t;

/**
 @brief  qos resrc init config
*/
struct ctc_qos_resrc_pool_cfg_s
{
    uint32 igs_pool_size[CTC_QOS_IGS_RESRC_POOL_MAX];   /**< [GB.GG.D2.TM] Assign igs pools size, for user define */
    uint32 egs_pool_size[CTC_QOS_EGS_RESRC_POOL_MAX];   /**< [GB.GG.D2.TM] Assign egs pools size, for user define */
    uint8  igs_pool_mode;                   /**< [GB.GG.D2.TM] Select resource profile, refert to ctc_qos_resrc_pool_mode_t */
    uint8  egs_pool_mode;                   /**< [GB.GG.D2.TM] Select resource profile, refert to ctc_qos_resrc_pool_mode_t */
    ctc_qos_resrc_drop_profile_t drop_profile[CTC_QOS_EGS_RESRC_POOL_MAX];  /**< [GB.GG]  Only default|non-drop|span|control pool support drop profile */
};
typedef struct ctc_qos_resrc_pool_cfg_s ctc_qos_resrc_pool_cfg_t;

/**
 @brief  qos resrc init config
*/
enum ctc_qos_port_queue_num_e
{
    CTC_QOS_PORT_QUEUE_NUM_8       = 8,  /**< [GG.D2.TM] 8 queue mode*/
    CTC_QOS_PORT_QUEUE_NUM_16      = 16, /**< [GG.D2.TM] 16 queue mode*/
    CTC_QOS_PORT_QUEUE_NUM_4_BPE,        /**< [GG] 4 queue mode for 480 port extender, must configurate by BPE before use*/
    CTC_QOS_PORT_QUEUE_NUM_8_BPE,        /**< [GG] 8 queue mode for 240 port extender, must configurate by BPE before use*/
    MAX_CTC_PORT_QUEUE_NUM_MAX
};
typedef enum ctc_qos_port_queue_num_e ctc_qos_port_queue_num_t;

/**
 @brief  qos global config
*/
struct ctc_qos_global_cfg_s
{
    uint8  queue_num_per_network_port;      /**< [GB.GG.D2.TM] queue number per network port, refer to ctc_qos_port_queue_num_t*/
    uint8  queue_num_per_internal_port;     /**< [GB.D2.TM] queue number per internal port, for D2, mean extend port queue num*/
    uint8  queue_num_per_cpu_reason_group;  /**< [GB] queue number per cpu reason group */
    uint8  queue_aging_time;                /**< [GB] queue aging interval (uint:s), default  aging time is 30s*/
    uint8  max_cos_level;                   /**< [GG.D2.TM] the max number of cos level in MEF BWP */
    uint16 policer_num;                     /**< [GG] the max number of policer, support 1K/2K/4K/8K, default 4K
                                                      8K support 1G rate,  4K support 10G rate,
                                                      2K support 40G rate, 1K support 100G rate */
    uint8  queue_num_for_cpu_reason;        /**< [GG.D2.TM] the queue number for cpu reason, support 128/64/32, default 128*/
    uint8  cpu_queue_shape_profile_num;     /**< [GB.GG.D2.TM] cpu queue shape profile number, default 0*/
    uint16 ingress_vlan_policer_num;        /**< [D2.TM] the max number of ingress vlan policer, default 0*/
    uint16 egress_vlan_policer_num;         /**< [D2.TM] the max number of egress vlan policer, default 0*/
    ctc_qos_resrc_pool_cfg_t resrc_pool;    /**< [GB.GG.D2.TM] resource pool init value */
    uint8  policer_merge_mode;              /**< [TM] 2 level policer merge compatible GG*/
    uint8 service_queue_mode;               /**< [D2.TM] service queue mode, default 0,0:logic scr port + dstport enq
                                                      1:service id + dstport enq ,2:logic port +dsport enq,logic port can be src and dst and only support in TM*/
    uint8  queue_num_per_ingress_service;   /**< [GB] queue number per ingress service */
    ctc_qos_policer_level_select_t  policer_level;/**< [D2.TM] QoS policer vlan and port level*/
    uint8  priority_mode;                   /**< [GB,GG] priority mode,default 0,0:support 0-15 priority
                                                         1:support 0-63 priority,if GG or GB stacking with the chip which is not GG or GB, must select mode 0*/
};
typedef struct ctc_qos_global_cfg_s ctc_qos_global_cfg_t;

/**@} end of @defgroup qos QOS */

#ifdef __cplusplus
}
#endif

#endif

