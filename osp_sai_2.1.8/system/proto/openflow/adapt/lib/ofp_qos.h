/**
 *  Copyright (C) 2011, 2012 CentecNetworks, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file
 * @brief This file defines qos data structure used in adapter layer
 */
#ifndef __OFP_QOS_H__
#define __OFP_QOS_H__

/******************************************************************************
* Header Files 
******************************************************************************/

/*******************************************************************
*
*Structures and macros, enums
*
********************************************************************/
#define OFP_QOS_PRIORITY_NUM            64
#define OFP_QOS_COLOR_NUM               OFP_QOS_COLOR_MAX
#define OFP_QOS_COS_NUM                 8
#define OFP_QOS_DSCP_NUM                64
#define OFP_QOS_TOS_NUM                 256
#define OFP_QOS_IP_PREC_NUM             8
#define OFP_QOS_EXP_NUM                 8
#define OFP_QOS_DROP_PREC_NUM           4
/* Modified by Yan Xing'an, 2014-01-06 */
/* Queue confirm class is 7, exceed class <0-6> can be configured*/
#define OFP_QOS_CLASS_NUM               7
#define OFP_QOS_MAP_NUM                 8
#define OFP_QOS_QUEUE_NUM               8

#define OFP_QOS_DEFAULT_DOMAIN          0
#define OFP_QOS_DEFAULT_COS             0
#define OFP_QOS_DEFAULT_TRUST           OFP_QOS_TRUST_NONE
#define OFP_QOS_DEFAULT_REPLACE_COS     0
#define OFP_QOS_DEFAULT_REPLACE_DSCP    0
#define OFP_QOS_DEFAULT_STATUS          true

#define OFP_QOS_DEFAULT_DROP_THRESH_10G 0x240
#define OFP_QOS_DEFAULT_DROP_THRESH_1G  0xE0
#define OFP_QOS_DEFAULT_DROP_DELTA_10G  0x10
#define OFP_QOS_DEFAULT_DROP_DELTA_1G   0x10

#define OFP_QOS_QUEUE_SHAPE_PROFILE_MAX    60
#define OFP_QOS_GROUP_SHAPE_PROFILE_MAX    30
#define OFP_QOS_QUEUE_PORT_NUM_MAX         60

#define OFP_QOS_DEFAULT_WTD_THRESH(speed_mode, drop_prec) \
    (GLB_SPEED_10G == (speed_mode)) ? \
        (OFP_QOS_DEFAULT_DROP_THRESH_10G + (drop_prec) * OFP_QOS_DEFAULT_DROP_DELTA_10G) : \
        (OFP_QOS_DEFAULT_DROP_THRESH_1G + \
                (drop_prec) * OFP_QOS_DEFAULT_DROP_DELTA_1G)

/* Default queue class for enabled qos */
#define OFP_QOS_DEFAULT_EN_Q_CLS(qid)   \
    OFP_QOS_DEFAULT_EXCEED_Q_CLS(qid)

/* Default queue class for disabled qos */
#define OFP_QOS_DEFAULT_DIS_Q_CLS(qid)  \
    OFP_QOS_DEFAULT_EXCEED_Q_CLS(qid)

#define OFP_QOS_DEFAULT_CONFIRM_Q_CLS(qid)   \
    ((qid) == 0 ?   7 :                 \
     (qid) == 1 ?   7 :                 \
     (qid) == 2 ?   7 :                 \
     (qid) == 3 ?   7 :                 \
     (qid) == 4 ?   7 :                 \
     (qid) == 5 ?   7 :                 \
     (qid) == 6 ?   7 :                 \
     (qid) == 7 ?   7 :                 \
     0)

#define OFP_QOS_DEFAULT_EXCEED_Q_CLS(qid)  \
    ((qid) == 0 ?   0 :                 \
     (qid) == 1 ?   0 :                 \
     (qid) == 2 ?   1 :                 \
     (qid) == 3 ?   1 :                 \
     (qid) == 4 ?   2 :                 \
     (qid) == 5 ?   2 :                 \
     (qid) == 6 ?   3 :                 \
     (qid) == 7 ?   3 :                 \
     0)

/*  TOS/COS/IP-PREC/EXP to qid */
#define OFP_QOS_DEFAULT_TOS_QID(tos)                        \
    ( (tos) * OFP_QOS_QUEUE_NUM / OFP_QOS_TOS_NUM)

#define OFP_QOS_DEFAULT_COS_QID(cos)                        \
    ( (cos) * OFP_QOS_QUEUE_NUM / OFP_QOS_COS_NUM)

#define OFP_QOS_DEFAULT_IP_PREC_QID(ip_prec)                \
    ( (ip_prec) * OFP_QOS_QUEUE_NUM / OFP_QOS_IP_PREC_NUM)

#define OFP_QOS_DEFAULT_EXP_QID(exp)                        \
    ( (exp) * OFP_QOS_QUEUE_NUM / OFP_QOS_EXP_NUM)

/* Use priority to generate queue-id */
#define OFP_QOS_DEFAULT_PRI_COLOR_QID(pri, color)           \
    ( (pri) * OFP_QOS_QUEUE_NUM / OFP_QOS_PRIORITY_NUM)

#define OFP_QOS_DEFAULT_PRI_COLOR_TID(pri, color)           \
    ((color) == OFP_QOS_COLOR_RED    ? 0 :                  \
     (color) == OFP_QOS_COLOR_YELLOW ? 1 :                  \
     (color) == OFP_QOS_COLOR_GREEN  ? 2 :                  \
     3)

/* qid to TOS/COS/EXP */
#define OFP_QOS_DEFAULT_QID_TOS(qid)                        \
    ( (qid) * OFP_QOS_TOS_NUM / OFP_QOS_QUEUE_NUM)

#define OFP_QOS_DEFAULT_QID_COS(qid)                        \
    ( (qid) * OFP_QOS_COS_NUM / OFP_QOS_QUEUE_NUM)

#define OFP_QOS_DEFAULT_QID_EXP(qid)                        \
    ( (qid) * OFP_QOS_EXP_NUM / OFP_QOS_QUEUE_NUM)

#define OFP_QOS_DEFAULT_QID_PRI(qid)                        \
    ( (qid) * OFP_QOS_PRIORITY_NUM / OFP_QOS_QUEUE_NUM)

#define OFP_QOS_DEFAULT_QID_COLOR(qid)                      \
    ( OFP_QOS_COLOR_GREEN )

#define OFP_QOS_TOS_2_DSCP(tos)                             \
    ( (tos) >> 2) 

#define OFP_QOS_DSCP_2_TOS(dscp)                            \
    ( (dscp) << 2)

enum ofp_qos_queue_profile_check_result_e
{
    OFP_QUEUE_RESULT_SUCCESS     = 0,
    OFP_QUEUE_RESULT_NOT_SUPPORT = 1,
    OFP_QUEUE_RESULT_Q_PRF_FULL  = 2,
    OFP_QUEUE_RESULT_QG_PRF_FULL = 3,
    OFP_QUEUE_RESULT_Q_PORT_FULL = 4,

    OFP_QUEUE_RESULT_MAX
};
typedef enum ofp_qos_queue_profile_check_result_e ofp_qos_queue_profile_check_result_t;

enum ofp_qos_queue_drop_mode_s
{
    OFP_QOS_QUEUE_DROP_MODE_WTD,
    OFP_QOS_QUEUE_DROP_MODE_WRED,

    OFP_QOS_QUEUE_DROP_MODE_MAX
};
typedef enum ofp_qos_queue_drop_mode_s ofp_qos_queue_drop_mode_t;

/**
 @brief QoS color
*/
enum ofp_qos_color_e
{
    OFP_QOS_COLOR_NONE,     /**< None color */
    OFP_QOS_COLOR_RED,      /**< Red color: the lowest drop precedence */
    OFP_QOS_COLOR_YELLOW,   /**< Yellow color: the mild drop precedence */
    OFP_QOS_COLOR_GREEN,    /**< Green color: the highest drop precedence */

    OFP_QOS_COLOR_MAX
};
typedef enum ofp_qos_color_e ofp_qos_color_t;

#define OFP_QOS_COLOR_TO_STR(color)                     \
    ((color) == OFP_QOS_COLOR_RED    ? "red" :          \
     (color) == OFP_QOS_COLOR_YELLOW ? "yellow" :       \
     (color) == OFP_QOS_COLOR_GREEN  ? "green" :        \
     "unknown")

enum ofp_qos_trust_e
{
    OFP_QOS_TRUST_NONE,
    OFP_QOS_TRUST_PORT,
    OFP_QOS_TRUST_OUTER,
    OFP_QOS_TRUST_COS,
    OFP_QOS_TRUST_TOS,
    OFP_QOS_TRUST_IP_PREC,
    OFP_QOS_TRUST_COS_INNER,

    OFP_QOS_TRUST_MAX
};
typedef enum ofp_qos_trust_e ofp_qos_trust_t;

#define OFP_QOS_TRUST_TO_STR(trust)                     \
    ((trust) == OFP_QOS_TRUST_NONE      ? "none"      : \
     (trust) == OFP_QOS_TRUST_OUTER     ? "outer"     : \
     (trust) == OFP_QOS_TRUST_COS       ? "cos"       : \
     (trust) == OFP_QOS_TRUST_TOS       ? "tos"       : \
     (trust) == OFP_QOS_TRUST_IP_PREC   ? "ip-prec"   : \
     (trust) == OFP_QOS_TRUST_COS_INNER ? "cos inner" : \
     "unknown")

enum ofp_qos_replace_type_e
{
    OFP_QOS_REPLACE_COS,
    OFP_QOS_REPLACE_DSCP,
    OFP_QOS_REPLACE_MAX
};
typedef enum ofp_qos_replace_type_e ofp_qos_replace_type_t;

struct ofp_qos_qid_map_e
{
    uint8_ofp value;    /* COS/TOS/IP-PREC/EXP */
    uint8_ofp queue_id;
};
typedef struct ofp_qos_qid_map_e ofp_qos_qid_map_t;

enum ofp_qos_map_type_e
{
    OFP_QOS_IGS_MAP_COS_QID,
    OFP_QOS_IGS_MAP_TOS_QID,
    OFP_QOS_IGS_MAP_DSCP_QID,
    OFP_QOS_IGS_MAP_IP_PREC_QID,
    OFP_QOS_IGS_MAP_EXP_QID,
    OFP_QOS_IGS_MAP_ALL,

    OFP_QOS_EGS_MAP_QID_COS,
    OFP_QOS_EGS_MAP_QID_TOS,
    OFP_QOS_EGS_MAP_QID_DSCP,
    OFP_QOS_EGS_MAP_QID_EXP,
    OFP_QOS_EGS_MAP_ALL,

    OFP_QOS_PRI_COLOR_QID,
    OFP_QOS_QID_PRI_COLOR,

    OFP_QOS_MAP_MAX,
};
typedef enum ofp_qos_map_type_e ofp_qos_map_type_t;

struct ofp_qos_pri_color_s
{
    uint8_ofp priority;
    uint8_ofp color;
};
typedef struct ofp_qos_pri_color_s ofp_qos_pri_color_t;

struct ofp_qos_queue_dest_s
{
    uint8_ofp queue_id;
    uint8_ofp drop_prec;
};
typedef struct ofp_qos_queue_dest_s ofp_qos_queue_dest_t;

struct ofp_qos_map_table_s
{
    uint8_ofp cos_qid_map_tbl[OFP_QOS_COS_NUM];         /* CoS-to-queue_id map */
    uint8_ofp ip_prec_qid_map_tbl[OFP_QOS_IP_PREC_NUM]; /* ip_prec-to-queue_id map */
    uint8_ofp tos_qid_map_tbl[OFP_QOS_TOS_NUM];         /* TOS-to-queue_id map */  
    uint8_ofp exp_qid_map_tbl[OFP_QOS_EXP_NUM];         /* EXP-to-queue_id map */

    /* qid-cos map */
    uint8_ofp qid_cos_map_tbl[OFP_QOS_QUEUE_NUM];
    /* qid-to-DSCP map */    
    uint8_ofp qid_tos_map_tbl[OFP_QOS_QUEUE_NUM];
    /* qid-to-EXP map */
    uint8_ofp qid_exp_map_tbl[OFP_QOS_QUEUE_NUM];

    ofp_qos_pri_color_t qid_pri_color_map_tbl[OFP_QOS_QUEUE_NUM];
    ofp_qos_queue_dest_t pri_color_qid_map_tbl[OFP_QOS_PRIORITY_NUM][OFP_QOS_COLOR_NUM];
};
typedef struct ofp_qos_map_table_s ofp_qos_map_table_t;

#endif /* !__OFP_QOS_H__ */
