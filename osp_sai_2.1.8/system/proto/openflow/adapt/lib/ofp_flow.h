#ifndef __OFP_FLOW_H__
#define __OFP_FLOW_H__

enum ofp_flow_id_e
{
    OFP_FLOW_ID_DEFAULT_FLOW = 1,       /**< default flow id */
    OFP_FLOW_ID_GROUP_FLOW = 2,         /**< group flow id */
    OFP_FLOW_ID_MNGT_DFT_FLOW = 3,      /**< flow id of flows for management */
    OFP_FLOW_ID_SFLOW_IGS = 4,          /**< flow id for ingress sflow */
    OFP_FLOW_ID_SFLOW_EGS = 5,          /**< flow id for egress sflow */

    OFP_FLOW_ID_MNGT_LOCAL = 8,         /**< flow id of flows with action: local */
    OFP_FLOW_ID_CPU_UNKNOWN = 10,       /**< flow id for undefined reason to cpu */
    OFP_FLOW_ID_START_OFFSET = 11       /**< start offset id */
};

#define OFP_INVALID_STATS_ID 0xFFFFFFFF /**< should be CTC_STATS_MAX_STATSID */
#define OFP_IDLE_TIMEOUT_TIMER_INTERVAL 5        /**< in seconds */

#endif /* !__OFP_FLOW_H__ */
