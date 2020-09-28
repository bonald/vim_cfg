#ifndef __HAGT_OPENFLOW_STATS_H__
#define __HAGT_OPENFLOW_STATS_H__

/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_api.h"

#include "glb_openflow_define.h"

#include "hagt_inc.h"
#include "hagt.h"
/****************************************************************************************
 *
 * Globals
 *
 ****************************************************************************************/
#define HAGT_OPENFLOW_FLOW_STATS_MAX_ITEM 2
#define HAGT_OPENFLOW_METER_STATS_MAX_ITEM 6 /* Modifed by weizj for meter */
#define HAGT_OPENFLOW_GROUP_STATS_MAX_ITEM 2

/* This value is calculated based on the following algorithm:
   We have 1s timer to poll flow stats from ASIC, and we support up to 4.5K flows
   for default profile, and we poll the stats hash db consecutively, if we want to
   support 5s accuracy for flow idle timeout, we must support to poll 4.5K/5s means
   900/s at least. 
   Consider the delay introduced by the multi process message passing, we set
   this roughly to 1000. */
#define HAGT_OPENFLOW_FLOW_STATS_POLLING_ONE_ROUND  1000

/* We only support limited meter entry, so we can safely try to poll 1024
   one round because the emtpy hash bucket does not consume much cpu time.
   Added by weizj for meter */
#define HAGT_OPENFLOW_METER_STATS_POLLING_ONE_ROUND  1024

/* We only support limited group entry, so we can safely try to poll 1024
   one round because the emtpy hash bucket does not consume much cpu time. */
#define HAGT_OPENFLOW_GROUP_STATS_POLLING_ONE_ROUND  1024

#define HAGT_OPENFLOW_FLOW_STATS_POLLING_INTERVAL 1
#define HAGT_OPENFLOW_METER_STATS_POLLING_INTERVAL 1
#define HAGT_OPENFLOW_GROUP_STATS_POLLING_INTERVAL 1

#define HAGT_OPENFLOW_UINT64_TO_OCTET_STR(data, str_s, str_d)                \
do{                                                                 \
    int32 i;                                                        \
    str_s.buf = str_d;                                              \
    str_s.size = sizeof(uint64);                                    \
    for(i = 0; i < sizeof(uint64); i++)                             \
    {                                                               \
        str_d[i] = (uint8)((data >> (i*8))&0xFF);                  \
    }                                                               \
}while(0)

/****************************************************************************************
 *
 * Function
 *
 ****************************************************************************************/

int32
hagt_openflow_add_stats(Hsrv2AgtMsg_t* p_msg_data);

int32
hagt_openflow_del_stats(Hsrv2AgtMsg_t* p_msg_data);

int32
hagt_openflow_clear_stats(Hsrv2AgtMsg_t* p_msg_data);

int32
hagt_openflow_stats_init(void);

uint32
hagt_openflow_stats_get_ctc_stats_id(uint32 openflow_stats_id);

int32
hagt_openflow_stats_register_callback(void);

#endif /*__HAGT_OPENFLOW_STATS_H__*/
