/****************************************************************************
* $Id$
*  Centec QoS related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Liang Rong
* Date          : 2010-07-06 16:00
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_QOS_DEFINE_H__
#define __GLB_QOS_DEFINE_H__

#include "glb_const.h"
#include "glb_acl_define.h"
#include "glb_phy_define.h"
#include "glb_flow_define.h"
#include "glb_stats_define.h"

#define GLB_DEFAULT_QUEUE_DRR_WEIGHT 1
#define GLB_QOS_DEF_WTD_PROFILE     "TAIL_DROP"
//#define GLB_QOS_DEF_TD_10G_PROFILE     "TAIL_DROP_XGE"
#define GLB_QOS_DEF_WRED_PROFILE    "RANDOM_DROP"

#define GLB_QOS_DEF_SCHEDULER_PROFILE    "SCHEDULER_PROFILE"
#define GLB_QOS_DEF_PORT_SHAPE_PROFILE    "PORT_SHAPE_PROFILE"

#ifdef GREATBELT
#define GLB_QOS_DEF_SHAPING_PIR  10000000
#else
#define GLB_QOS_DEF_SHAPING_PIR  100000000
#endif

//#define GLB_QOS_POLICER_PROFILE_NUM     256
//#define GLB_QOS_SHAPE_PROFILE_NUM       64
/* modify by huwx for reserved 13 policer profile, copp use 13 policer*/
#ifdef OFPRODUCT
#define GLB_QOS_POLICER_PROFILE_NUM     40
#else
#define GLB_QOS_POLICER_PROFILE_NUM     243
#endif
#define GLB_QOS_POLICER_ACTION_PROFILE_NUM  60
#define GLB_QOS_MPLS_LABEL_NUM    1000
/* modify by huwx for reserved 4 shape profile, copp use 4 shape profie*/
#define GLB_QOS_SHAPE_PROFILE_NUM       60

#ifdef TSINGMA
#define GLB_QOS_DROP_PROFILE_NUM       30
#define GLB_QOS_PORT_SHAPE_PROFILE_NUM       64
 
#define GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM    63
#define GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM      15
#define GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM    GLB_QOS_POLICER_PROFILE_NUM
#define GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM     63
#elif defined DUET2
#define GLB_QOS_DROP_PROFILE_NUM       7
#define GLB_QOS_PORT_SHAPE_PROFILE_NUM       64
 
#define GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM    63
#define GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM      15
#define GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM    GLB_QOS_POLICER_PROFILE_NUM
#define GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM     15
#else
#define GLB_QOS_DROP_PROFILE_NUM       4
#define GLB_QOS_PORT_SHAPE_PROFILE_NUM       128

#define GLB_QOS_INGERSS_PORT_POLICER_PROFILE_NUM    GLB_QOS_POLICER_PROFILE_NUM
#define GLB_QOS_EGERSS_PORT_POLICER_PROFILE_NUM     GLB_QOS_POLICER_PROFILE_NUM
#define GLB_QOS_INGERSS_FLOW_POLICER_PROFILE_NUM    GLB_QOS_POLICER_PROFILE_NUM
#define GLB_QOS_EGERSS_FLOW_POLICER_PROFILE_NUM     GLB_QOS_POLICER_PROFILE_NUM
#endif

#define GLB_QOS_DOMAIN_NUM              7
#define GLB_QOS_TC_NUM            8
#define GLB_QOS_PRIORITY_NUM            64
#define GLB_QOS_PHB_PRIORITY_NUM        13

#ifdef USW
    #define GLB_QOS_DROP_PROBABILITY    61
#else
    #define GLB_QOS_DROP_PROBABILITY    19
#endif
#define GLB_QOS_DROP_THRESHOLD   600

#ifdef GREATBELT
#define GLB_QOS_DROP_MAX_THRESHOLD   6000
#elif defined GOLDENGATE
#define GLB_QOS_DROP_MAX_THRESHOLD   12286
#elif defined USW
#define GLB_QOS_DROP_MAX_THRESHOLD   10000
#else
#define GLB_QOS_DROP_MAX_THRESHOLD   12286
#endif

#define GLB_QOS_MPLS_LABEL_NUMBER  1048556 /*added by yejl for D2&TM, 2019-06-18*/

/*modified by xgu for bug 13556, 2010-11-19, bug 17820, 2012-02-06*/
#define GLB_QOS_HIGHEST_PRIORITY        12
/*modified by yanxa for bug 15464, 2011-07-14*/
#define GLB_QOS_HIGHEST_COS             7
#define GLB_QOS_COLOR_NUM               4
#define GLB_QOS_COS_NUM                 8
#define GLB_QOS_DSCP_NUM                64
/*modify by chenxw for bug 25293, 2013-10-15.*/
#define GLB_QOS_ECN_NUM 4 
#define GLB_QOS_IP_PREC_NUM             8
#define GLB_QOS_EXP_NUM                 8
#define GLB_QOS_PRI_COLOR_TABLE_SIZE    (GLB_QOS_PRIORITY_NUM * GLB_QOS_COLOR_NUM)
#define GLB_QOS_TC_COLOR_TABLE_SIZE    (GLB_QOS_TC_NUM * GLB_QOS_COLOR_NUM)
#define GLB_QOS_INVALID_FLOWID          0xFF
/* modified by cuixl for bug 22441, GB QoS development, 2013-03-12 */
/* 8 unicast queue, 4 mcast queue */
#define GLB_QOS_PORT_QUEUE_NUM          8

#define GLB_QOS_MAX_GROUP_QUEUE_NUM     8

#define GLB_QOS_QUEUE_SMART_BUFFER_NUM  7

#define GLB_QOS_GROUP_NUM          16

#define GLB_QOS_ALL_QUEUE_ID            0xFF

#define GLB_QOS_BUFFCELL_SIZE           256

#define GLB_QOS_AGP_NAME_LEN            40

#define GLB_QOS_FLOW_COUNT_ID_NUM       63
#define GLB_QOS_GROUP_COUNT_ID_NUM      255
#define GLB_QOS_FLOW_RULE_PRIORITY_MAX  0xFFFFFFFF
/*moidfy by chenxw for acl&qos resource mgr, 2013-09-12.*/
#define GLB_QOS_ACL_TCAM_BLOCK          0
#define GLB_QOS_ACL_MAC_KEY_MERGE_IPV6  1
#define GLB_QOS_ACL_MAC_KEY_MERGE_MPLS  2 
#define GLB_QOS_ACL_MAC_KEY_MERGE_IPV4   3
#define GLB_QOS_ACL_PORT_OP_NEQ        3
#define GLB_QOS_ACL_BLOCK_ENABLE_LOOKUP  0xF
#define GLB_QOS_ACL_QOS_PORT_VLAN_PRI      (1 << 6)
#define GLB_QOS_FLOW_DEFAULT_COUNT    63

#define GLB_QOS_DROP_PREC_NUM           4

#define GLB_QOS_QUEUE_WDRR_WEIGHT_MTU_DEF 1600

#define GLB_QOS_MAX_HSRV2AGT_FLOW_RULE_NUM    10
#define GLB_QOS_MAX_HSRV2AGT_MAP_TABLE_SIZE   32
#define GLB_QOS_MAX_HAGT2SRV_PEND_FLOW_NUM    20

/* modified by jiangz for bug 15627, 2011-09-03 */
#define GLB_QOS_DEFAULT_DROP_THRESH_1G  0xE0
/*modified by xgu for bug 16265, 2011-11-09*/
#define GLB_QOS_DEFAULT_WRED_MIN_THRESH_1G 0x10
#define GLB_QOS_DEFAULT_WRED_MAX_THRESH_1G 0x1E0
#define GLB_QOS_DEFAULT_DROP_DELTA_1G   0x10
#define GLB_QOS_DEFAULT_WRED_MIN_THRESH_DELTA_1G   8
#define GLB_QOS_DEFAULT_DROP_THRESH_10G 0x240
#define GLB_QOS_DEFAULT_DROP_DELTA_10G  0x10

#define GLB_QOS_DEFAULT_MONITOR_LOG_WEIGHT 15

/* modified by cuixl for bug 27208, 2014-04-01 */
/* modification start */
#define GLB_QOS_ACL_GET_GROUP_ID(stats_id, group_id) \
    (group_id = ((stats_id >> 22) & 0xFF))
/* modification end */


/* modified by rongl for bug 14061, 2011-03-18 */
/* modified by cuixl for bug 22441, 2013-04-17 */
/*sdk support max threshold as SYS_MAX_DROP_THRD*/
/* #define SYS_MAX_DROP_THRD (12 * 1024 - 1)*/
/*modify by chenxw for bug 25286, 2013-12-02.*/
#define GLB_QOS_MAX_DROP_THRESHOLD 12287
#define GLB_QOS_MAX_WRED_THRESHOLD 12287

#define GLB_QOS_DEFAULT_CLASSID            0
#define GLB_QOS_QID_BIT_NUMBER          5

#define GLB_QOS_QID_BIT_MASK            0x1F

#define GLB_QOS_THRESHOLD_BIT_NUMBER    3
#define GLB_QOS_THRESHOLD_BIT_MASK      0x7

#define GLB_QOS_DEFAULT_COS             0
#define GLB_QOS_DEFAULT_DSCP            0
#define GLB_QOS_DEFAULT_EXP             0
#define GLB_QOS_DEFAULT_IP_PREC         0

#define GLB_QOS_INIFINITY_DRR_WEIGHT     0xFF
#define GLB_QOS_UNSET_DRR_WEIGHT     0xFE

#define GLB_QOS_DRR_WEIGHT_MAX           100000

/* modified by cuixl for bug22441, GB QoS development, 2013-03-14 */
/*modification start*/
/* this value means disable shape(port)*/
#define GLB_QOS_ESPECAIL_BURST 0xFFFFFFFF

/*modification end*/


/*modified by xgu for bug 16265, 2011-11-09*/
#define GLB_QOS_MAX_DROP_PROBABILITY_VALUE  65535
#define GLB_QOS_MAX_DROP_PROBABILITY_BIT    16
#define GLB_QOS_DEFAULT_WRED_DROP_PROB 1024
/* modified by chenxw for bug 25816 , 2013-08-28 */
#define GLB_QOS_MAX_DROP_PROBABILITY_CTC 127


#define GLB_QOS_ACL_STATSID_MASK        0xFFFFF
/*added by wangl for GB qos acl*/
#define GLB_QOS_ACL_GET_ENTRY_ID(rule_id, label, entry_id) \
    (entry_id = (rule_id << 2) + (label << 22))
 
/*modified by xgu for bug 16265, 2011-11-09*/
#define GLB_QOS_DEFAULT_WRED_MIN_THRESH(speed_mode, drop_prec)   \
    (GLB_QOS_DEFAULT_WRED_MIN_THRESH_1G + \
        (drop_prec) * GLB_QOS_DEFAULT_WRED_MIN_THRESH_DELTA_1G)
#define GLB_QOS_DEFAULT_WRED_MAX_THRESH(speed_mode, drop_prec)   \
    (GLB_QOS_DEFAULT_WRED_MAX_THRESH_1G + \
        (drop_prec) * GLB_QOS_DEFAULT_DROP_DELTA_1G)

#define GLB_QOS_DEFAULT_WTD_THRESH(speed_mode, drop_prec) \
    (GLB_SPEED_10G == (speed_mode)) ? \
        (GLB_QOS_DEFAULT_DROP_THRESH_10G + (drop_prec) * GLB_QOS_DEFAULT_DROP_DELTA_10G) : \
        (GLB_QOS_DEFAULT_DROP_THRESH_1G + \
                (drop_prec) * GLB_QOS_DEFAULT_DROP_DELTA_1G)

/*Modified by yanxa, for bug 17961, 2012-02-29*/
#define GLB_QOS_DEFAULT_TH3_WRED_MIN_THRESH(speed_mode, th2_thresh)   \
    (((th2_thresh) + GLB_QOS_DEFAULT_WRED_MIN_THRESH_DELTA_1G) >= GLB_QOS_MAX_WRED_THRESHOLD ? \
     GLB_QOS_MAX_WRED_THRESHOLD : ((th2_thresh) + GLB_QOS_DEFAULT_WRED_MIN_THRESH_DELTA_1G))
#define GLB_QOS_DEFAULT_TH3_WRED_MAX_THRESH(speed_mode, th2_thresh)   \
    (((th2_thresh) + GLB_QOS_DEFAULT_DROP_DELTA_1G) >= GLB_QOS_MAX_WRED_THRESHOLD ? \
     GLB_QOS_MAX_WRED_THRESHOLD : ((th2_thresh) + GLB_QOS_DEFAULT_DROP_DELTA_1G))
#define GLB_QOS_DEFAULT_TH3_WTD_THRESH(speed_mode, th2_thresh)   \
    (GLB_SPEED_10G == (speed_mode)) \
        ? ((((th2_thresh) + GLB_QOS_DEFAULT_DROP_THRESH_10G) >=  GLB_QOS_MAX_WRED_THRESHOLD) ? \
           GLB_QOS_MAX_WRED_THRESHOLD : ((th2_thresh) + GLB_QOS_DEFAULT_DROP_THRESH_10G))\
        : ((((th2_thresh) + GLB_QOS_DEFAULT_DROP_DELTA_1G) >=  GLB_QOS_MAX_WRED_THRESHOLD) ? \
           GLB_QOS_MAX_WRED_THRESHOLD : ((th2_thresh) + GLB_QOS_DEFAULT_DROP_DELTA_1G))
#define GLB_QOS_DEFAULT_TH3_WRED_DROP_PROB  0
/* modified by cuixl for bug 22441, 2013-04-17 */
/*sdk support max threshold as SYS_MAX_DROP_THRD*/
/* #define SYS_MAX_DROP_THRD (16 * 1024 - 1)*/
/*modify by chenxw for bug 25286, 2013-12-02.*/
#ifdef _CENTEC_
#define GLB_QOS_MAX_TH3_WRED_MAX_THRESH 12287
#define GLB_QOS_MIN_TH3_WRED_MIN_THRESH 12286
#endif /*_CENTEC_*/

// when qos is disable, queue 0 of each channel should be 352, 368, 384(DEFAULT), 400(RESERVED)
#define GLB_NON_QOS_DEFAULT_DROP_THRESH  0x160
#define GLB_NON_QOS_DEFAULT_DROP_DELTA   0x10
#define GLB_NON_QOS_DEFAULT_WTD_THRESH(drop_prec) \
    (GLB_NON_QOS_DEFAULT_DROP_THRESH + (drop_prec) * GLB_NON_QOS_DEFAULT_DROP_DELTA)

#define GLB_QOS_POLICER_DEFAULT_BURST   128000
#define GLB_QOS_POLICER_DEFAULT_PPS_BURST   1500
#ifdef USW
#define GLB_QOS_POLICER_PPS_DEFAULT_RATE   140000000    
#else
#define GLB_QOS_POLICER_PPS_DEFAULT_RATE   0    
#endif

#ifdef GREATBELT
#define GLB_QOS_POLICER_DEFAULT_RATE   10000000    
#else
#define GLB_QOS_POLICER_DEFAULT_RATE   100000000    
#endif
#define GLB_QOS_SHAPE_DEFAULT_RATE   0x7FFFFFFF
#define GLB_QOS_DROP_DEFAULT_THRD   0x7FFFFFFF    

#define GLB_POLICER_PROFILE_FLAGS_AGP                       (1 << 0)
#define GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE            (GLB_POLICER_PROFILE_FLAGS_AGP              << 1)
#define GLB_POLICER_PROFILE_FLAGS_COLOR_MODE                (GLB_POLICER_PROFILE_FLAGS_ALGORITHM_MODE   << 1)
#define GLB_POLICER_PROFILE_FLAGS_CIR                       (GLB_POLICER_PROFILE_FLAGS_COLOR_MODE       << 1)
#define GLB_POLICER_PROFILE_FLAGS_CBS                       (GLB_POLICER_PROFILE_FLAGS_CIR              << 1)
#define GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR                (GLB_POLICER_PROFILE_FLAGS_CBS              << 1)
#define GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS                (GLB_POLICER_PROFILE_FLAGS_EIR_OR_PIR       << 1)
#define GLB_POLICER_PROFILE_FLAGS_DROP_COLOR                (GLB_POLICER_PROFILE_FLAGS_EBS_OR_PBS       << 1)
#define GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH             (GLB_POLICER_PROFILE_FLAGS_DROP_COLOR       << 1)
#define GLB_POLICER_PROFILE_FLAGS_STATS                     (GLB_POLICER_PROFILE_FLAGS_USE_L3_LENGTH    << 1)
/* this flag is used for dump only */
#define GLB_POLICER_PROFILE_FLAGS_DUMP_STATS                (GLB_POLICER_PROFILE_FLAGS_STATS            << 1)   


#define GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR         (1 << 0)
#define GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR         (GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR << 1)
#define GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE      (GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR << 1)
#define GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT      (GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE << 1)

#define GLB_PORT_SHAPE_PROFILE_FLAGS_PIR         (1 << 0)

#define GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD     (1 << 0)
#define GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD     (GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD    << 1)
#define GLB_DROP_PROFILE_FLAGS_GREEN_PROB         (GLB_DROP_PROFILE_FLAGS_GREEN_MIN_THRD    << 1)
#define GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD    (GLB_DROP_PROFILE_FLAGS_GREEN_PROB        << 1)
#define GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD    (GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD   << 1)
#define GLB_DROP_PROFILE_FLAGS_YELLOW_PROB        (GLB_DROP_PROFILE_FLAGS_YELLOW_MIN_THRD   << 1)
#define GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD       (GLB_DROP_PROFILE_FLAGS_YELLOW_PROB       << 1)
#define GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD       (GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD      << 1)
#define GLB_DROP_PROFILE_FLAGS_RED_PROB           (GLB_DROP_PROFILE_FLAGS_RED_MIN_THRD      << 1)
#define GLB_DROP_PROFILE_FLAGS_RANDOM_DETECT      (GLB_DROP_PROFILE_FLAGS_RED_PROB      << 1)

#define GLB_ACLQOS_IF_FLAGS_DOMAIN                         (1 << 0)
#define GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE   (GLB_ACLQOS_IF_FLAGS_DOMAIN << 1)   
#define GLB_ACLQOS_IF_FLAGS_INPUT_POLICER              (GLB_ACLQOS_IF_FLAGS_PORT_SHAPE_PROFILE << 1)
#define GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER           (GLB_ACLQOS_IF_FLAGS_INPUT_POLICER << 1)
#define GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE  (GLB_ACLQOS_IF_FLAGS_OUTPUT_POLICER << 1)  
#define GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE   (GLB_ACLQOS_IF_FLAGS_QUEUE_SHAPE_PROFILE << 1) 
#define GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE       (GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_PROFILE << 1)
#define GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP       (GLB_ACLQOS_IF_FLAGS_QUEUE_DROP_MODE << 1)
#define GLB_ACLQOS_IF_FLAGS_REPLACE_COS       (GLB_ACLQOS_IF_FLAGS_REPLACE_DSCP << 1)
#define GLB_ACLQOS_IF_FLAGS_TRUST                   (GLB_ACLQOS_IF_FLAGS_REPLACE_COS << 1)
#define GLB_ACLQOS_IF_FLAGS_PORT_TC               (GLB_ACLQOS_IF_FLAGS_TRUST << 1)

#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER     (1 << 0)
#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER    (GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER << 1)
#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER    (GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER << 1)
#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER           (GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_INPUT_POLICER << 1)
#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE                        (GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_OUTPUT_POLICER << 1)
#define GLB_ACLQOS_IF_STATS_FLAGS_PORT_CLEAR_QUEUE                    (GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE << 1)

#define GLB_MONITOR_LATENCY_RANGE_ID_MAX    8

typedef enum
{
    GLB_QOS_UPDATE_OP_ADD,
    GLB_QOS_UPDATE_OP_DEL,
    GLB_QOS_UPDATE_OP_ATTACH,
    GLB_QOS_UPDATE_OP_DEATTACH,
    GLB_QOS_UPDATE_OP_SET,
    GLB_QOS_UPDATE_OP_UNSET,
    GLB_QOS_UPDATE_OP_MAX
} glb_qos_update_handle_op_type_t;

typedef enum
{
    GLB_QOS_PORT_QUEUE_UPDATE_DEFCOS,
    GLB_QOS_PORT_QUEUE_UPDATE_DOMAIN,
    GLB_QOS_PORT_QUEUE_UPDATE_TRUST,
    GLB_QOS_PORT_QUEUE_UPDATE_REPLACE,
    GLB_QOS_PORT_QUEUE_UPDATE_SHAPE,
    GLB_QOS_PORT_QUEUE_UPDATE_ATTACH_PMAP,
    GLB_QOS_PORT_QUEUE_UPDATE_DEATTACH_PMAP,
    GLB_QOS_PORT_QUEUE_UPDATE_POLICER,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_STATS_EN,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_SHAPE,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_CLASS,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_WEIGHT,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_DROP_MODE,
    GLB_QOS_PORT_QUEUE_UPDATE_QUEUE_ECN,
    GLB_QOS_PORT_QUEUE_UPDATE_MAX
} glb_qos_port_queue_update_type_t;

typedef enum
{
    GLB_QOS_GLOBAL_UPDATE_QOS_ENABLE,
    GLB_QOS_GLOBAL_UPDATE_PHB_ENABLE,
    GLB_QOS_GLOBAL_UPDATE_PORT_FIRST,
    GLB_QOS_GLOBAL_UPDATE_POLICER_STATS,
    GLB_QOS_GLOBAL_UPDATE_MAX
} glb_qos_global_update_type_t;

typedef enum 
{
    GLB_QOS_POLICER_MODE_RFC2697 = 0,
    GLB_QOS_POLICER_MODE_RFC2698,
    GLB_QOS_POLICER_MODE_RFC4115,
    GLB_QOS_POLICER_MODE_BWP,
    GLB_QOS_POLICER_MODE_MAX
} glb_qos_policer_mode_t;

typedef enum 
{
    GLB_QOS_POLICER_TYPE_POLICER = 0,
    GLB_QOS_POLICER_TYPE_AGP,
    GLB_QOS_POLICER_TYPE_MAX
} glb_qos_policer_type_t;

typedef enum 
{
    GLB_QOS_POLICER_COLOR_BLIND,
    GLB_QOS_POLICER_COLOR_AWARE,
    GLB_QOS_POLICER_COLOR_MAX
} glb_qos_policer_color_mode_t;


enum glb_qos_phb_type_e
{
    GLB_QOS_PHB_CS1 = 0,
    GLB_QOS_PHB_DF,
    GLB_QOS_PHB_AF11,
    GLB_QOS_PHB_AF12,
    GLB_QOS_PHB_AF13,
    GLB_QOS_PHB_AF21,
    GLB_QOS_PHB_AF22,
    GLB_QOS_PHB_AF23,
    GLB_QOS_PHB_AF31,
    GLB_QOS_PHB_AF32,
    GLB_QOS_PHB_AF33,
    GLB_QOS_PHB_AF41,
    GLB_QOS_PHB_AF42,
    GLB_QOS_PHB_AF43,
    GLB_QOS_PHB_CS2,
    GLB_QOS_PHB_CS3,
    GLB_QOS_PHB_CS6,
    GLB_QOS_PHB_CS7,
    GLB_QOS_PHB_CS4,
    GLB_QOS_PHB_CS5,
    GLB_QOS_PHB_EF,
    GLB_QOS_PHB_MAX
};
typedef enum glb_qos_phb_type_e glb_qos_phb_type_t;

enum glb_qos_global_enable_flag_e
{
    GLB_QOS_GLOBAL_FLAG_GLB_EN             = 0x0001,
    GLB_QOS_GLOBAL_FLAG_POLICER_EN         = 0x0002,
    GLB_QOS_GLOBAL_FLAG_FLOW_FIRST         = 0x0004,
    GLB_QOS_GLOBAL_FLAG_QUEUE_STATS_EN     = 0x0008,
    GLB_QOS_GLOBAL_FLAG_POLICER_STATS_EN   = 0x0010,
    GLB_QOS_GLOBAL_FLAG_POLICER_IPG_EN     = 0x0020,
    GLB_QOS_GLOBAL_FLAG_SHAPING_IPG_EN     = 0x0040,
    /*modify by chenxw for bug 25492, 2013-11-29.*/
    GLB_QOS_GLOBAL_FLAG_SMART_BUFFER_EN     = 0x0080,    
};

enum glb_qos_flow_action_flag_e
{
    GLB_QOS_FLOW_ACTION_FLAG_TRUST         = 0x0001,
    GLB_QOS_FLOW_ACTION_FLAG_PRIORITY      = 0x0002,
    GLB_QOS_FLOW_ACTION_FLAG_POLICER       = 0x0004,
    GLB_QOS_FLOW_ACTION_FLAG_STATS         = 0x0008,
    GLB_QOS_FLOW_ACTION_FLAG_REDIRECT      = 0x0010,
    GLB_QOS_FLOW_ACTION_FLAG_LOG           = 0x0020,
    GLB_QOS_FLOW_ACTION_AGG_POLICER        = 0X0040,
/* modified by cuixl for bug 22442, GB QoS development, 2013-03-15 */
/*modification start*/
    GLB_QOS_FLOW_ACTION_DOMAIN             = 0X0080,
    GLB_QOS_FLOW_ACTION_DSCP               = 0X0100,
    GLB_QOS_FLOW_ACTION_STAG_COS           = 0X0200,
    GLB_QOS_FLOW_ACTION_CTAG_COS           = 0X0400,
    GLB_QOS_FLOW_ACTION_TAP                = 0X0800,
    GLB_QOS_FLOW_ACTION_MIRROR             = 0X1000,
/*modification end*/
};

enum glb_qos_flow_process_flag_e
{
    GLB_QOS_FLOW_PROCESSING_IPV4     = 0x0001,
    GLB_QOS_FLOW_PROCESSING_IPV6     = 0x0002,
    GLB_QOS_FLOW_PROCESSING_MPLS     = 0x0004,
};

/* Priority-Color mapping table type */
enum glb_qos_pri_color_map_type_e
{
    GLB_QOS_PRI_COLOR_MAP_TYPE_COS = 0,
    GLB_QOS_PRI_COLOR_MAP_TYPE_DSCP,
    GLB_QOS_PRI_COLOR_MAP_TYPE_TOS,
    GLB_QOS_PRI_COLOR_MAP_TYPE_IP_PREC,
    GLB_QOS_PRI_COLOR_MAP_TYPE_EXP,
    GLB_QOS_PRI_COLOR_MAP_TYPE_QID_TID,/*Mapped queueId and ThresholdId*/
    
    GLB_QOS_PRI_COLOR_MAP_TYPE_MAX
};
typedef enum glb_qos_pri_color_map_type_e glb_qos_pri_color_map_type_t;

/* modified by cuixl for bug 22441, GB QoS development, 2013-03-08 */
/*modification start*/
enum glb_qos_igs_map_type_e
{
    GLB_QOS_IGS_MAP_TYPE_COS = 0,
    GLB_QOS_IGS_MAP_TYPE_DSCP,
    GLB_QOS_IGS_MAP_TYPE_IP_PREC,
    GLB_QOS_IGS_MAP_TYPE_EXP,   
    GLB_QOS_IGS_MAP_TYPE_MAX
};
typedef enum glb_qos_igs_map_type_e glb_qos_igs_map_type_t;

/*modification end*/

/* QoS port attribute type */
enum glb_qos_port_attr_type_e
{
    GLB_QOS_PORT_ATTR_TYPE_TRUST_STATE = 0,
    GLB_QOS_PORT_ATTR_TYPE_DEFAULT_COS,
    GLB_QOS_PORT_ATTR_TYPE_REPLACE_COS,
    GLB_QOS_PORT_ATTR_TYPE_REPLACE_CTAG_COS,
    GLB_QOS_PORT_ATTR_TYPE_REPLACE_DSCP,
    GLB_QOS_PORT_ATTR_TYPE_DOMAIN,

    GLB_QOS_PORT_ATTR_TYPE_MAX
};
typedef enum glb_qos_port_attr_type_e glb_qos_port_attr_type_t;


/* QoS trust state */
enum glb_qos_trust_state_e
{
    GLB_QOS_TRUST_PORT,         /* Trust port default CoS */
    GLB_QOS_TRUST_OUTER,        /* Trust outer priority and color, for tunnel or loopback */
    GLB_QOS_TRUST_COS,          /* Trust packet CoS field */
    GLB_QOS_TRUST_TOS,          /* Trust packet CoS field */
    GLB_QOS_TRUST_DSCP,         /* Trust IP DSCP field */
    GLB_QOS_TRUST_IP_PREC,      /* Trust IP precedence field */
    GLB_QOS_TRUST_COS_INNER,    /* Trust packet inner CoS field */
    GLB_QOS_TRUST_COS_OUTER,    /* Trust packet stag CoS field */    

    GLB_QOS_TRUST_MAX
};
typedef enum glb_qos_trust_state_e glb_qos_trust_state_t;

/* QoS repace state */
enum glb_qos_replace_state_e
{
    GLB_QOS_REPLACE_NONE,        
    GLB_QOS_REPLACE_COS,      
    GLB_QOS_REPLACE_DSCP_EXP,         
    GLB_QOS_REPLACE_STAG_COS,      
    GLB_QOS_REPLACE_CTAG_COS,   

    GLB_QOS_REPLACE_MAX
};
typedef enum glb_qos_replace_state_e glb_qos_replace_state_t;


/* QoS color */
enum glb_qos_color_e
{
    GLB_QOS_COLOR_NONE,
    GLB_QOS_COLOR_RED,      /* Red color: the lowest drop precedence */
    GLB_QOS_COLOR_YELLOW,   /* Yellow color: the mild drop precedence */
    GLB_QOS_COLOR_GREEN,    /* Green color: the highest drop precedence */

    GLB_QOS_COLOR_MAX
};
typedef enum glb_qos_color_e glb_qos_color_t;


/* QoS threshold */
enum glb_qos_threshold_e
{
    GLB_QOS_THRESHOLD_0,    /* The lowest drop precedence threshold */
    GLB_QOS_THRESHOLD_1,    /* The mild drop precedence threshold */
    GLB_QOS_THRESHOLD_2,    /* The high drop precedence threshold */
    GLB_QOS_THRESHOLD_3,    /* The highest drop precedence threshold resvered for critical packet */

    GLB_QOS_THRESHOLD_MAX
};
typedef enum glb_qos_threshold_e glb_qos_threshold_t;

enum glb_qos_acl_stats_update_e
{
    GLB_QOS_ACL_STATS_UPDATE_HOLD,
    GLB_QOS_ACL_STATS_UPDATE_DEL,
    GLB_QOS_ACL_STATS_UPDATE_ADD
};
typedef enum glb_qos_acl_stats_update_e glb_qos_acl_stats_update_f;

#define  GLB_QOS_DEFULT_ENABLE_SHAPING_QID 7
#define GLB_QOS_DEFAULT_QUEUE_SHAPE_PER    30

#define GLB_QOS_PORT_DEFAULT_COS          0
#define GLB_QOS_PORT_DEFAULT_DOMAIN       0
#define GLB_QOS_PORT_DEFAULT_TRUST        GLB_QOS_TRUST_COS

/* color action */
struct glb_qos_color_action_s
{
    uint8 drop_packet;  /* 0:transmit packet; 1:drop packet */
    uint8 tc_valid;         /* indicate tc if valid */
    uint8 tc;                  /* tc:0-7 */
};
typedef struct glb_qos_color_action_s glb_qos_color_action_t;

/* priority color */
struct glb_qos_pri_color_s
{
    uint8  pri;
    uint8  color;
    uint8  reserved[2];
};
typedef struct glb_qos_pri_color_s glb_qos_pri_color_t;


/* priority color mapping */
struct glb_qos_pri_color_map_s
{
    uint8  domain;
    uint8  value;/*For qid-tid: qid have higher 5 bits, tid have lower 3bits */
    uint8  pri;
    uint8  color;
};
typedef struct glb_qos_pri_color_map_s glb_qos_pri_color_map_t;
#if 0
struct glb_qos_pri_color_map_s
{
    union {
        uint8  domain;
        uint8  queue_num;/* for qid-tid, queue_num indicates queue num on port or service */
    }u;
    uint8  value;/*For qid-tid: qid have higher 4 bits, tid have lower 3bits, most significant indicates mcast or ucast queue; */
    uint8  pri;
    uint8  color;
};
typedef struct glb_qos_pri_color_map_s glb_qos_pri_color_map_t;
#endif

/* QoS policer */
struct glb_qos_policer_s
{
    uint8  is_srtcm;
    uint8  is_color_blind;
    uint8  drop_color;
    uint8  use_l3_length;

    /*Added by yanxa, for bug 17215, 2011-12-31*/
    uint8  is_stats_en;
/* modified by cuixl for bug 22441, 2013-03-14 */
/*modification start*/
    uint8  mode;
#define GLB_QOS_POLICER_MODE_RFC2697                 0
#define GLB_QOS_POLICER_MODE_RFC2698                 1
#define GLB_QOS_POLICER_MODE_RFC4115                 2
#define GLB_QOS_POLICER_MODE_BWP                     3
    //uint8  resved[3];
/*modification end*/
    
    uint32 cir;
    uint32 cbs;
    uint32 pir;
    uint32 pbs;    
};
typedef struct glb_qos_policer_s glb_qos_policer_t;

struct glb_qos_flow_action_flag_s 
{
    uint32 policer:1,
           agp:1,
           priority:1,
           trust:1,
           stats:1,
           redirect:1,
           log:1,
/* modified by cuixl for bug 22441, 2013-03-15 */
/*modification start*/
           domain:1,
           dscp:1,
           stag_cos:1,
           ctag_cos:1,
           tap:1,
/*modification end*/           
           reserved:20;
};
typedef struct glb_qos_flow_action_flag_s glb_qos_flow_action_flag_t;

#define GLB_QOS_INVALID_NH_ID 0xffffffff

struct glb_qos_flow_action_s 
{
    glb_qos_flow_action_flag_t flag;/* added by yanxa, bug 15492, 2011-07-18 */

    char  sz_agp_name[GLB_QOS_AGP_NAME_LEN+4];
    
    glb_qos_policer_t policer;
/* modified by cuixl for bug 22441, 2013-03-18 */
/* because GB chip supports flow policer and aggregater flow policer simultaneously */
/*modification start*/
    glb_qos_policer_t agp;
/*modification end*/
    
    uint8  priority;
    uint8  color;
    uint8  trust;
    uint8  log_id;

    uint32 log_weight;
    uint32 redirect_ifindex;
    uint32 tap_id;
    uint32 tap_vlan;
/* modified by cuixl for bug 22441, 2013-03-15 */
/*modification start*/
    uint8 domain;
    uint8 dscp;
    uint8 cos;
/*modification end*/
    
};
typedef struct glb_qos_flow_action_s glb_qos_flow_action_t;


struct glb_qos_flow_action_update_param_s
{
    uint32 op_action;     /* GLB_QOS_FLOW_ACTION_OP_XXX */
    char  sz_agp_name[GLB_QOS_AGP_NAME_LEN+4];
    
    glb_qos_policer_t policer;
/* modified by cuixl for bug 22441, 2013-03-18 */
/* because GB chip supports flow policer and aggregater flow policer simultaneously */
/*modification start*/
    glb_qos_policer_t agp;
/*modification end*/
    
    uint8  priority;
    uint8  color;
    uint8  trust;
    uint8  log_id;
    uint32 log_weight;
    uint32 redirect_ifindex;
    uint32 tap_id;
    uint32 tap_vlan;
/* modified by cuixl for bug 22441, 2013-03-14 */
/*modification start*/
    uint8 domain;
    uint8 dscp;
    uint8 cos;
/*modification end*/
    
};
typedef struct glb_qos_flow_action_update_param_s glb_qos_flow_action_update_param_t;


struct glb_qos_flow_rule_s
{
    uint32 seq_num;
    glb_flow_rule_t rule;
};
typedef struct glb_qos_flow_rule_s glb_qos_flow_rule_t;


struct glb_qos_flow_rule_group_s
{
    char  sz_group_name[ACL_NAME_FULL_NAME_SIZE+4];  /* ACL name */
    
    uint8  group_type;     /* acl type: GLB_FLOW_RULE_TYPE_XXX */
    uint8  rule_num;
    uint8  has_ext_mac_rule;
    uint8  reserved;
    
    glb_qos_flow_rule_t rule_group[GLB_QOS_MAX_HSRV2AGT_FLOW_RULE_NUM];
    glb_flow_mac_rule_t ext_mac_rule;     /* mac key part: used for match-all class-map when 
                                             mac and ip key are required to merge together */
};
typedef struct glb_qos_flow_rule_group_s glb_qos_flow_rule_group_t;


enum glb_qos_flow_action_op_code_e
{
    GLB_QOS_FLOW_ACTION_OP_SET_PRI_COLOR = 0,
    GLB_QOS_FLOW_ACTION_OP_UNSET_PRI_COLOR,
    GLB_QOS_FLOW_ACTION_OP_SET_TRUST,
    GLB_QOS_FLOW_ACTION_OP_UNSET_TRUST,
    GLB_QOS_FLOW_ACTION_OP_ADD_IND_POLICER,
    GLB_QOS_FLOW_ACTION_OP_DEL_IND_POLICER,
    GLB_QOS_FLOW_ACTION_OP_MOD_IND_POLICER,
    GLB_QOS_FLOW_ACTION_OP_ADD_AGG_POLICER,
    GLB_QOS_FLOW_ACTION_OP_DEL_AGG_POLICER,
    GLB_QOS_FLOW_ACTION_OP_SET_LOG,
    GLB_QOS_FLOW_ACTION_OP_UNSET_LOG,
    GLB_QOS_FLOW_ACTION_OP_ENABLE_STATS,
    GLB_QOS_FLOW_ACTION_OP_DISABLE_STATS,
    GLB_QOS_FLOW_ACTION_OP_SET_REDIRECT,
    GLB_QOS_FLOW_ACTION_OP_UNSET_REDIRECT,
/* modified by cuixl for bug 22441, 2013-03-14 */
/*modification start*/
    GLB_QOS_FLOW_ACTION_OP_SET_DOMAIN,
    GLB_QOS_FLOW_ACTION_OP_UNSET_DOMAIN,
    GLB_QOS_FLOW_ACTION_OP_SET_DSCP,
    GLB_QOS_FLOW_ACTION_OP_UNSET_DSCP,
    GLB_QOS_FLOW_ACTION_OP_SET_STAG_COS,
    GLB_QOS_FLOW_ACTION_OP_SET_CTAG_COS,
    GLB_QOS_FLOW_ACTION_OP_UNSET_COS,    
/*modification end*/
    GLB_QOS_FLOW_ACTION_OP_SET_TAP,
    GLB_QOS_FLOW_ACTION_OP_UNSET_TAP,
    GLB_QOS_FLOW_ACTION_OP_MAX
};

enum glb_qos_flow_op_code_e
{
    GLB_QOS_FLOW_OP_ADD_FLOW_RULE = 0,
    GLB_QOS_FLOW_OP_DEL_FLOW_RULE,
    GLB_QOS_FLOW_OP_VALID_FLOW_RULE,
    GLB_QOS_FLOW_OP_INVALID_FLOW_RULE,
    GLB_QOS_FLOW_OP_ADD_RULE_GROUP,
    GLB_QOS_FLOW_OP_DEL_RULE_GROUP,
    GLB_QOS_FLOW_OP_ADD_FLOW_ENTRY,
    GLB_QOS_FLOW_OP_DEL_FLOW_ENTRY,
    GLB_QOS_FLOW_OP_MOD_FLOW_ACTION,

    GLB_QOS_FLOW_OP_MAX
};


struct glb_qos_flow_update_param_s
{
    uint32 ifindex;
    
    uint8  dir;
    uint8  op_flow;     /* GLB_QOS_FLOW_TABLE_OP_XXX */
    uint8  op_action;   /* GLB_QOS_FLOW_ACTION_OP_XXX, used for 
                           op_code = GLB_QOS_FLOW_OP_MOD_FLOW_ACTION */
    uint8  reserved;
    
    char  sz_flow_name[ACL_NAME_FULL_NAME_SIZE+4];
    char  sz_group_name[ACL_NAME_FULL_NAME_SIZE+4];
    
    uint32 seq_num;
};
typedef struct glb_qos_flow_update_param_s glb_qos_flow_update_param_t;



/* queue shape paramters */
struct glb_qos_queue_shape_s
{
    uint32 cir;
    uint32 cbs;
    uint32 pir;
    uint32 pbs;
};
typedef struct glb_qos_queue_shape_s glb_qos_queue_shape_t;


/* port shape parameters */
struct glb_qos_port_shape_s
{
    uint32 pir;
    uint32 pbs;
};
typedef struct glb_qos_port_shape_s glb_qos_port_shape_t;
typedef struct glb_qos_port_shape_s glb_qos_group_shape_t;


typedef enum 
{
    GLB_QOS_QUEUE_DROP_MODE_WTD = 0,
    GLB_QOS_QUEUE_DROP_MODE_WRED,

    GLB_QOS_QUEUE_DROP_MODE_MAX
} glb_qos_queue_drop_mode_t;


/* queue drop scheme & parameters */
struct glb_qos_queue_drop_s
{
    uint8  mode;                         /* GLB_QOS_QUEUE_DROP_MODE_XXX */
    uint8  weight;                       /* WRED weight */
    uint8  reserved[2];
    uint16 min_th[GLB_QOS_DROP_PREC_NUM];    /* WRED min threshold */
    uint16 max_th[GLB_QOS_DROP_PREC_NUM];    /* WRED max threshold, or WTD drop threshold */
    uint16 drop_prob[GLB_QOS_DROP_PREC_NUM]; /* WRED max drop probability */
};
typedef struct glb_qos_queue_drop_s glb_qos_queue_drop_t;

struct glb_qos_port_init_cfg_s
{
    uint32 ifindex;
    
    uint8  attr_value[GLB_QOS_PORT_ATTR_TYPE_MAX];
    uint8  reserved[2];
    
    uint32 wdrr_weight;
};
typedef struct glb_qos_port_init_cfg_s glb_qos_port_init_cfg_t;


enum glb_qos_port_cfg_flag_e
{
    GLB_QOS_PORT_CFG_FLAG_DOMAIN           = 1 << 0,
    GLB_QOS_PORT_CFG_FLAG_TRUST            = 1 << 1,
    GLB_QOS_PORT_CFG_FLAG_COS              = 1 << 2,
    GLB_QOS_PORT_CFG_FLAG_REPLACE_COS      = 1 << 3,
    GLB_QOS_PORT_CFG_FLAG_REPLACE_DSCP     = 1 << 4,
    GLB_QOS_PORT_CFG_FLAG_IGS_PORT_POLICER = 1 << 5,
    GLB_QOS_PORT_CFG_FLAG_EGS_PORT_POLICER = 1 << 6,
    GLB_QOS_PORT_CFG_FLAG_PORT_SHAPE       = 1 << 7,
    GLB_QOS_PORT_CFG_FLAG_QUEUE_DROP       = 1 << 8,
    GLB_QOS_PORT_CFG_FLAG_QUEUE_CLASS      = 1 << 9,
    GLB_QOS_PORT_CFG_FLAG_WDRR_WEIGHT      = 1 << 10,
    GLB_QOS_PORT_CFG_FLAG_QUEUE_SHAPE      = 1 << 11
};


struct glb_qos_port_deinit_cfg_s
{
    uint32 ifindex;

    uint32 flag;    /* GLB_QOS_PORT_CFG_XXX */

    uint8  domain;
    uint8  trust_state;
    uint8  default_cos;
    
    uint32 wdrr_weight;
    uint32 queue_wdrr_weight_bmp;

    uint32 queue_class_bmp;
    uint32 queue_classid;/*each queue have 2 bits classid*/
    glb_qos_queue_drop_t queue_drop;
    uint32 queue_drop_bmp;
    
    glb_qos_port_shape_t port_shape;
    uint32 queue_shape_bmp;
};
typedef struct glb_qos_port_deinit_cfg_s glb_qos_port_deinit_cfg_t;

enum glb_qos_pend_type_e
{
    GLB_QOS_PEND_TYPE_ADD_FAIL       = 1,
    GLB_QOS_PEND_TYPE_SET_STATS_FAIL = 2,
    GLB_QOS_PEND_TYPE_SET_PLC_FAIL   = 3
    /* other pend type */
};
typedef enum glb_qos_pend_type_e glb_qos_pend_type_t;
/* modified by cuixl for bug 22441, 2013-03-18 */
/*modification start*/
enum glb_qos_profile_type_s
{
  GLB_QOS_PROFILE_BASIC,
  GLB_QOS_PROFILE_ENTERPRISE,
  GLB_QOS_PROFILE_ENTERPRISE_ADVANCE,
  GLB_QOS_PROFILE_MAX,  
};
typedef enum glb_qos_profile_type_s glb_qos_profile_type_t;

struct glb_qos_queue_map_s
{
    char *profile_name;
    uint8 queue_num;
};
typedef struct glb_qos_queue_map_s glb_qos_queue_map_t;

/* modified by cuixl for bug 22441, 2013-03-11 */
// TODO: this file directory needs to be confirmed on board and UML
//#define GLB_QOS_QUEUE_MAP_PROFILE   "/centec_switch/etc/tmp/qos_queue_map"
// TODO: following stm prefer is better?
/*modify by chenxw for bug 25873, 2013-12-03.*/
#if 0
#define GLB_QOS_PROFILE_FILE   "/mnt/flash/qos_profile"
#else
#define GLB_QOS_PROFILE_FILE   "/mnt/flash/.qos_profile"
#endif
#define GLB_QOS_IS_MCAST_QUEUE(qid)   ((qid > 7) && (qid < 12))
#define GLB_QOS_UCAST_QUEUE_NUM     8
#define GLB_QOS_MCAST_QUEUE_NUM     4
#define GLB_QOS_GEN_MCAST_QID(ucast_qid)   ((ucast_qid) / (GLB_QOS_UCAST_QUEUE_NUM / GLB_QOS_MCAST_QUEUE_NUM))
#define GLB_QOS_GEN_TID(color)   (((color) + 2) % 3)

/*modification end*/

#define GLB_MONITOR_LATENCY_RANGE_ID_MAX    8

#define GLB_MONITOR_LATENCY_RANGE_0_DEF_VALUE  0
#define GLB_MONITOR_LATENCY_RANGE_1_DEF_VALUE  800
#define GLB_MONITOR_LATENCY_RANGE_2_DEF_VALUE  1000
#define GLB_MONITOR_LATENCY_RANGE_3_DEF_VALUE  1200
#define GLB_MONITOR_LATENCY_RANGE_4_DEF_VALUE  1400
#define GLB_MONITOR_LATENCY_RANGE_5_DEF_VALUE  1800
#define GLB_MONITOR_LATENCY_RANGE_6_DEF_VALUE  2000
#define GLB_MONITOR_LATENCY_RANGE_7_DEF_VALUE  4096

#define GLB_BUFFER_EVENT_LOW_THRESHOLD_DEFAULT  16
#define GLB_BUFFER_EVENT_HIGH_THRESHOLD_DEFAULT 64

#endif /*!__GLB_QOS_DEFINE_H__*/

