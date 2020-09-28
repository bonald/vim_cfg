#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_monitor.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2013-10-28

 @version v2.0

\p
Monitor tracks port congestion and queuing latency with real-time reporting. With this application
layer event export, external applications can predict impending congestion and latency. This enables the
application layer to make traffic routing decisions with visibility into the network layer.
With monitor, network operations teams and administrators have near real-time visibility into the
network, enabling early detection of microbursts. Monitor continually monitors congestion, allowing for
rapid detection of congestion and sending of application layer messages.

\b
\p
Monitor have two ways to track congestion and latency. One is buffer usage, another is packet latency through switch.

\p
1)Buffer Monitor:
\pp
Buffer Monitor provides congestion data by continuously monitoring each port¡¯s output queue lengths. When
the length of an output queue exceeds the upper threshold for that port, it will  generate an
over-threshold event. When the length of an output queue is below the upper threshold for that port, it will  generate an
below-threshold event.
\pp
You can enable buffer monitor and change report period through API:
\pp
 ctc_monitor_set_config();

\p
2)Latency Monitor:
\pp
Latency Monitor provides latency data by continuously monitoring packet latency through switch. When
the latency of packet exceeds the upper threshold level for that port, it will  generate an
over-threshold event. it continues to report an over-threshold state every microseconds until
all queue lengths for that port pass below the lower threshold.
\pp
You can enable latency monitor and change report period through API:
\pp
 ctc_monitor_set_config();

\b
\p
Monitor can log waterwark of max buffer usage and latency which can analyze each port's network burst and latency.
\p
You can get and clear watermark through API:
\p
 ctc_monitor_get_watermark();
\p
 ctc_monitor_clear_watermark();

\b
\p
Monitor can log packet to cpu when the length of an output queue exceeds the upper threshold for that port or
the latency of packet exceeds the upper threshold level. This function can help user to analyze the congestion.
For buffer congestion, it support two modes. one is causer mode which can help user to analyze the congestion causer,
another is victim mode which can help user to analyze the congestion victims.
\p
You can change mode through API:
\p
 ctc_monitor_set_global_config();


\S ctc_monitor.h:ctc_monitor_config_type_t
\S ctc_monitor.h:ctc_monitor_config_t
\S ctc_monitor.h:ctc_monitor_watermark_t
\S ctc_monitor.h:ctc_monitor_glb_config_type_t
\S ctc_monitor.h:ctc_monitor_glb_cfg_t
*/

#ifndef _CTC_USW_MONITOR_H
#define _CTC_USW_MONITOR_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "ctc_monitor.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
/**
 @addtogroup monitor MONITOR
 @{
*/

/**
 @brief Intialize monitor module

 @param[in] lchip    local chip id

 @param[in] p_global_cfg to initialize monitor

 @remark[D2.TM]
  Initialize the monitor moudle.

 @return CTC_E_XXX

*/

extern int32
ctc_usw_monitor_init(uint8 lchip, void* p_global_cfg);

/**
 @brief De-Initialize monitor module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the monitor configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_monitor_deinit(uint8 lchip);



/**
 @brief Set monitor configure

 @param[in] lchip    local chip id

 @param[in] p_cfg  monitor configure such moninitor/log enable

 @remark[D2.TM]
  Enable buffer and latency event inform and cpu log function.
  Refer to ctc_monitor_config_type_t.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_monitor_set_config(uint8 lchip, ctc_monitor_config_t* p_cfg);



/**
 @brief Get monitor configure

 @param[in] lchip    local chip id

 @param[out] p_cfg  monitor configure such moninitor/log enable

 @remark[D2.TM]
 Get the buffer and latnecy configure information.
 Refer to ctc_monitor_config_type_t.


 @return CTC_E_XXX

*/
 extern int32
ctc_usw_monitor_get_config(uint8 lchip, ctc_monitor_config_t* p_cfg);

/**
 @brief Register callback function to get monitor data from DMA

 @param[in] lchip    local chip id

 @param[in] callback point to callback function

 @param[in] userdata point to user data

 @remark[D2.TM]
 Callback fucntion to get the message of buffer and latency event and stats

 @return CTC_E_XXX

*/
extern int32
ctc_usw_monitor_register_cb(uint8 lchip, ctc_monitor_fn_t callback,void* userdata);


/**
 @brief Get the watermark of monitor such max latency or max buffer cnt

 @param[in] lchip    local chip id

 @param[in|out] p_watermark  point to node which get the watermark of monitor such max latency or max buffer cnt

 @remark[D2.TM]
 Get the buffer and latency watermark which log the max buffer usage and latency.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_monitor_get_watermark(uint8 lchip, ctc_monitor_watermark_t* p_watermark);


/**
 @brief Clear the watermark of monitor such max latency or max buffer cnt

 @param[in] lchip    local chip id

 @param[in] p_watermark  point to node which get the watermark of monitor such max latency or max buffer cnt

 @remark[D2.TM]

 Clear the buffer and latency watermark which log the max buffer usage and latency.

 @return CTC_E_XXX

*/
extern int32
ctc_usw_monitor_clear_watermark(uint8 lchip, ctc_monitor_watermark_t* p_watermark);



/**
 @brief Set monitor global configure

 @param[in] lchip    local chip id

 @param[in] p_cfg  monitor global configure

 @remark[D2.TM]

 Buffer and latency global configure include latency level threshold and buffer log mode.
 refer to ctc_monitor_glb_config_type_t;

 @return CTC_E_XXX

*/
 extern int32
ctc_usw_monitor_set_global_config(uint8 lchip, ctc_monitor_glb_cfg_t* p_cfg);




/**@} end of @addgroup monitor */

#ifdef __cplusplus
}
#endif

#endif

#endif

