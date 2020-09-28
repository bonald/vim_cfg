/**
 @file ctc_monitor.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-12-27

 @version v2.0

This file contains all monitor related data structure, enum, macro and proto.

*/
#ifndef _CTC_MONITOR_H
#define _CTC_MONITOR_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_qos.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define CTC_MONITOR_LATENCY_THRD_LEVEL 8 /**<[GG.D2] Support 8 latency threshold levels*/
/**
 @defgroup monitor MONITOR
 @{
*/

/**
@brief  monitor type
*/
enum ctc_monitor_type_e
{
    CTC_MONITOR_BUFFER,    /**<[GG.D2.TM]  monitor buffer count*/
    CTC_MONITOR_LATENCY,   /**<[GG.D2.TM]  monitor latency*/
};
typedef enum ctc_monitor_type_e ctc_monitor_type_t;

enum ctc_monitor_event_state_e
{
    CTC_MONITOR_EVENT_STATE_CLEAR,   /**<[D2.TM]  event clear*/
    CTC_MONITOR_EVENT_STATE_OCCUR    /**<[D2.TM]  event occur */
};
typedef enum ctc_monitor_event_state_e ctc_monitor_event_state_t;

/**
@brief  monitor info type
*/
enum ctc_monitor_buffer_type_e
{
    CTC_MONITOR_BUFFER_PORT,    /**<[GG.D2.TM]  Port based buffer monitor*/
    CTC_MONITOR_BUFFER_SC,      /**<[GG.D2.TM]  Sc based buffer monitor*/
    CTC_MONITOR_BUFFER_QUEUE,   /**<[D2.TM]  Queue based buffer monitor*/
    CTC_MONITOR_BUFFER_TOTAL,   /**<[GG.D2.TM]  Total based buffer monitor*/
    CTC_MONITOR_BUFFER_C2C,     /**<[TM]  C2c based buffer monitor*/
};
typedef enum  ctc_monitor_buffer_type_e ctc_monitor_buffer_type_t;

/**
@brief  monitor config type
*/
enum ctc_monitor_config_type_e
{
    CTC_MONITOR_CONFIG_MON_INFORM_EN,     /**<[GG.D2.TM]  Buffer/latency event enable, per port enable*/
    CTC_MONITOR_CONFIG_MON_INFORM_MIN,    /**<[GG.D2.TM]  Buffer/latency event min thrd,  per port control
                                                  for buffer is bufcnt thrd<0-MaxBuffer> 16 multiple(such as 0,16,32,...),
                                                  for GG latency is thrd level<0-7>,for D2 latency is threshold time <ns>*/
    CTC_MONITOR_CONFIG_MON_INFORM_MAX,    /**<[GG.D2.TM]  Buffer/latency event max thrd,  per port control
                                                  for buffer is bufcnt thrd<0-MaxBuffer> 16 multiple(such as 0,16,32,...),
                                                  for GG latency is thrd level<0-7>,for D2 latency is threshold time <ns>*/

    CTC_MONITOR_CONFIG_LOG_EN,            /**<[GG.D2.TM]  Buffer/Latency log packet to cpu, latency per port enable, buffer global enable*/
    CTC_MONITOR_CONFIG_LOG_THRD_LEVEL,    /**<[GG.D2.TM]  Latency log thrd level<0-7>, per port control,
                                                  buffer log use buffer hig thrd condition, so don't use this api */

    CTC_MONITOR_CONFIG_MON_SCAN_EN,       /**<[GG.D2.TM]   Buffer/Latency stats timer scan enable, per port enable*/
    CTC_MONITOR_CONFIG_MON_INTERVAL,      /**<[GG.D2.TM]   Buffer/Latency stats timer scan inteval, global control*/

    CTC_MONITOR_CONFIG_LANTENCY_DISCARD_EN,            /**<[D2.TM]  Latency discard packet, latency per port enable, only for disable 8 levels all*/
    CTC_MONITOR_CONFIG_LANTENCY_DISCARD_THRD_LEVEL,    /**<[D2.TM]  Latency discard thrd level<0-7>, per port control, per level enable*/

    CTC_MONITOR_RETRIEVE_LATENCY_STATS,       /**<[D2.TM]  Retrieve Latency Stats */
    CTC_MONITOR_RETRIEVE_MBURST_STATS,        /**<[D2.TM]   Retrieve mBurst Stats,the mBurst stats takes effect only when the buffer event is enabled. */
};
typedef enum ctc_monitor_config_type_e ctc_monitor_config_type_t;

/**
@brief  monitor configure
*/
struct ctc_monitor_config_s
{
    uint8   monitor_type;  /**<[GG.D2.TM] monitor type, refert to ctc_monitor_type_t*/
    uint8   cfg_type;      /**<[GG.D2.TM] property type, refert to ctc_monitor_config_type_t*/
    uint8   buffer_type;   /**<[D2.TM] information type of event for TM, stats for D2,TM .refer to ctc_monitor_buffer_type_t*/
    uint8   sc;            /**<[TM] monitor buffer sc number*/
    uint8   dir;           /**<[D2.TM] direction 0:ingress 1:egress.refer to ctc_direction_t, only for scan*/
    uint8 level;         /**<[D2.TM] latecny threshold level or mburst threshold <0-7>*/
    uint32  gport;        /**<[GG.D2.TM] port of monitor*/
    uint32  value;        /**<[GG.D2.TM] configure value*/
};
typedef struct ctc_monitor_config_s ctc_monitor_config_t;

/**
@brief  monitor buffer wartermark
*/
struct ctc_monitor_watermark_buffer_s
{
    uint8  dir;             /**<[D2.TM] direction 0:ingress 1:egress.refer to ctc_direction_t*/
    uint8  buffer_type;     /**<[D2.TM] information type of buffer.refer to ctc_monitor_buffer_type_t*/
    uint8  sc;              /**<[D2.TM] monitor buffer sc number*/
    uint8  rsv;
    uint32 max_uc_cnt;      /**<[GG.D2.TM]  record max unicast buffer cnt*/
    uint32 max_mc_cnt;      /**<[GG.D2.TM]  record max multicast buffer cnt*/
    uint32 max_total_cnt;   /**<[GG.D2.TM]  record max total buffer cnt*/
};
typedef struct ctc_monitor_watermark_buffer_s ctc_monitor_watermark_buffer_t;

/**
@brief  monitor latency wartermark
*/
struct ctc_monitor_watermark_latency_s
{
    uint64 max_latency;     /**<[GG.D2.TM]  record max latency*/
};
typedef struct ctc_monitor_watermark_latency_s ctc_monitor_watermark_latency_t;

/**
@brief  monitor wartermark
*/
struct ctc_monitor_watermark_s
{
    uint8 monitor_type;        /**<[GG.D2.TM] monitor type, refert to ctc_monitor_type_t*/
    uint32 gport;              /**<[GG.D2.TM] port of monitor*/

    union
    {
        ctc_monitor_watermark_buffer_t buffer;    /**<[GG.D2.TM] buffer watermark*/
        ctc_monitor_watermark_latency_t latency;  /**<[GG.D2.TM] latency watermark*/
    }u;
};
typedef struct ctc_monitor_watermark_s ctc_monitor_watermark_t;

/**
@brief  monitor message type
*/
enum ctc_monitor_msg_type_e
{
     CTC_MONITOR_MSG_EVENT,        /**<[GG.D2.TM]  buffer/latency event message*/
     CTC_MONITOR_MSG_STATS,        /**<[GG.D2.TM]  buffer/latency  stats message*/
     CTC_MONITOR_MSG_MBURST_STATUS   /**<[D2.TM]  mburst(micro burst) stats message*/
};
typedef enum ctc_monitor_msg_type_e ctc_monitor_msg_type_t;

/**
@brief  buffer monitor event
*/
struct ctc_monitor_buffer_event_s
{
    uint32 gport;           /**<[GG.D2.TM] port of monitor*/
    uint8  buffer_type;      /**<[TM] event buffer type ctc_monitor_buffer_type_t*/
    uint8  sc;              /**<[TM] monitor buffer sc number */
    uint8 event_state;      /**<[GG.D2.TM] CTC_MONITOR_EVENT_STATE_xx  */
    uint8 threshold_level;  /**<[GG.D2.TM] threshold level when buffer event overcome  */
    uint32 uc_cnt;          /**<[GG.D2.TM] unicast buffer cell count*/
    uint32 mc_cnt;          /**<[GG.D2.TM] multicast  buffer cell count*/
    uint32 buffer_cnt;      /**<[TM] monitor buffer count*/
};
typedef struct ctc_monitor_buffer_event_s ctc_monitor_buffer_event_t;

/**
@brief  buffer monitor stats
*/
struct ctc_monitor_buffer_stats_s
{
    uint8 buffer_type;          /**<[GG.D2.TM] information type of stats.refer to ctc_monitor_buffer_type_t*/
    uint8 dir;                  /**<[D2.TM] direction 0:ingress 1:egress.refer to ctc_direction_t*/
    uint8 rsv;
    uint8 sc;                   /**<[GG.D2.TM] monitor buffer sc number, Default pool buffer*/
    uint32 gport;               /**<[GG.D2.TM] global phyical port of monitor*/
    ctc_qos_queue_id_t queue_id;/**<[D2.TM] Queue id*/
    uint32 buffer_cnt;          /**<[GG.D2.TM] monitor buffer count*/
};
typedef struct ctc_monitor_buffer_stats_s ctc_monitor_buffer_stats_t;

/**
@brief  latency monitor event
*/
struct ctc_monitor_latency_event_s
{
    uint32 gport;       /**<[GG.D2.TM] global phyical port of monitor*/
    uint32 source_port; /**<[GG.D2.TM] source port of packet*/
    uint64 latency;     /**<[GG.D2.TM] packet latency through switch,latency in nanoseconds*/
    uint8  event_state; /**<[GG.D2.TM] CTC_MONITOR_EVENT_STATE_xx  */
    uint8  level;        /**<[GG.D2.TM] congestion level of smart buffer*/
    uint8  port;         /**<[GG.D2.TM] global source phy port of monitor such as linkagg memeber port*/
    uint8  rsv;
};
typedef struct ctc_monitor_latency_event_s ctc_monitor_latency_event_t;

/**
@brief  latency monitor stats
*/
struct ctc_monitor_latency_stats_s
{
    uint32 gport;  /**<[GG.D2.TM] port of monitor*/
    uint16 rsv;
    uint32 threshold_cnt[CTC_MONITOR_LATENCY_THRD_LEVEL];   /**<[GG.D2.TM] latency threshold count per per level <0-7>*/
};
typedef struct ctc_monitor_latency_stats_s ctc_monitor_latency_stats_t;
typedef struct ctc_monitor_latency_stats_s ctc_monitor_mburst_stats_t;


/**
@brief  monitor message
*/
struct ctc_monitor_msg_s
{
    uint8 monitor_type; /**<[GG.D2.TM] monitor type, refert to ctc_monitor_type_t*/
    uint8 msg_type;     /**<[GG.D2.TM] message  type, refert to ctc_monitor_msg_type_t*/
    uint8 rsv[2];

    uint64 timestamp;    /**<[GG.D2.TM] timestamp of message*/

    union
    {
        ctc_monitor_buffer_event_t buffer_event;      /**<[GG.D2.TM] buffer event information*/
        ctc_monitor_buffer_stats_t  buffer_stats;      /**<[GG.D2.TM] buffer stats information*/
        ctc_monitor_latency_event_t latency_event;    /**<[GG.D2.TM] latency event information*/
        ctc_monitor_latency_stats_t latency_stats;    /**<[GG.D2.TM] latency stats information*/
	    ctc_monitor_mburst_stats_t  mburst_stats;    /**<[D2.TM] Mburst stats information*/
    }  u;

};
typedef struct ctc_monitor_msg_s ctc_monitor_msg_t;


/**
@brief  monitor profile of hardware
*/
struct ctc_monitor_data_s
{
    uint16 msg_num;     /**<[GG.D2.TM] message numbers*/
    uint16 rsv;
    ctc_monitor_msg_t*  p_msg; /**<[GG.D2.TM] message of event or stats*/
};
typedef struct ctc_monitor_data_s ctc_monitor_data_t;

/**
@brief  callback of monitor for protocol stack
*/
typedef void (* ctc_monitor_fn_t)(ctc_monitor_data_t * info, void* userdata) ;  /**<Callback function to get data from DMA*/

/*Configure the lower boundary of each bin for latency durations histogram,the upper
  boundary of the bin is the lower boundary of next bin. The upper boundary of the last bin is infinity.
  */
struct ctc_monitor_latency_threshold_s
{
    uint8 level;      /**< [GG.D2.TM] latecny threshold level <0-7>*/
    uint8 rsv[3];
    uint32 threshold; /**< [GG.D2.TM] the latecny threshold of specific level,latency in nanoseconds*/
};
typedef struct ctc_monitor_latency_threshold_s ctc_monitor_latency_thrd_t;
typedef struct ctc_monitor_latency_threshold_s ctc_monitor_mburst_thrd_t;

/**
@brief  monitor global config type
*/
enum ctc_monitor_glb_config_type_e
{
    CTC_MONITOR_GLB_CONFIG_LATENCY_THRD,     /**<[GG.D2.TM]  latency level threshold*/
    CTC_MONITOR_GLB_CONFIG_MBURST_THRD,      /**<[D2.TM]  micro burst level threshold*/
    CTC_MONITOR_GLB_CONFIG_BUFFER_LOG_MODE,  /**<[GG.D2.TM]  buffer log mode, 0: causer, 1: victim*/
};
typedef enum ctc_monitor_glb_config_type_e ctc_monitor_glb_config_type_t;


/**
@brief  monitor global config
*/
struct ctc_monitor_glb_cfg_s
{
    uint8 cfg_type;                         /**<[GG.D2.TM] configre type refer to ctc_monitor_glb_config_type_t*/
    union
    {
        uint32 value;                       /**< [GG.D2.TM] configre value according to ctc_monitor_glb_config_type_t*/
        ctc_monitor_latency_thrd_t  thrd;   /**< [GG.D2.TM] configre latecny threshold*/
        ctc_monitor_mburst_thrd_t mburst_thrd; /**< [D2.TM] configre mirco burst threshold*/
    }u;
};
typedef struct ctc_monitor_glb_cfg_s ctc_monitor_glb_cfg_t;

#ifdef __cplusplus
}
#endif

#endif
/**@} end of @defgroup   monitor MONITOR */

