/**
 @file ctc_usw_qos.h

 @date 2012-09-03

 @version v2.0

\p
The differentiated service (DS) is an architecture that specifies a simple and scalable
mechanism for classifying and managing network traffic and providing QoS on modern IP
networks. DS relies on mechanisms to classify and mark packets as belonging to a specific
class at the boundary of DS domain, and perform differentiated packet forwarding and
treatment, which is referred to as PHB (per-hop behavior), based on the marked class at DS
interior node.
\p
\b
DS is a coarse-grained, class-based mechanism for traffic management. In contrast, IntServ is
a fine-grained, flow-based mechanism. However, DS has better scalability than IntServ as it
doesn¡¯t require maintaining per-flow state on routers along the traffic path. The DS is widely
implemented in modern Internet routers and switch, Tte DS model consists of the following components:
\p
\b
\B Classification
\pp
     Classifier distinguishes one kind of traffic from another (based on CoS, DSCP,EXP,ACL). 
     SCL and ACL are able to use for this purpose. For more detailed information, please
     refer to scl/acl module.
\p
\b
\B Metering (Policing)
\pp
    Policers decide whether a packet is in profile or out of profile by comparing the rate of the
    traffic to the configured policer, and the centec QOS supports the following  algorithms defined
    by IETF RFC or MEF standard.
      \dd RFC 2697, SrTCM
      \dd RFC 2698, TrTCM
      \dd RFC 4115, modified TrTCM
      \dd MEF BWP (bandwidth profile)
      \dd STBM, Single Token Bucket Meter
\pp
   Also the Centec Qos support three type and 2-level policing model, the three types policing:
     \dd port policing
     \dd flow policing  (Micro/Macro Flow)
     \dd vlan policing
     \dd copp policing
\pp
   and the 2-level policing according to different application mode:
   \dd  Port policer           (ingress /egress)
   \dd  Micro Flow policer    (ingress /egress)
   \dd  Micro Flow policer  + Port policer (ingress /egress)
   \dd  Micro Flow policer  + Macro Flow policer (ingress /egress)
   \dd  Micro Flow policer  + Service policer (ingress)
   \dd  MEF 10.3: Per UNI/Per EVC :Port policer + Service policer  (ingress )
   \dd  MEF 10.3: Per UNI/Per EVC Per Cos:Port policer + Service policer(ingress )
   \dd  Port policer and vlan policer is the same level, vlan policer take effect

\pp
    you can refer to the API ctc_qos_set_policer() to see more details.
\p
\b
 \B Priority Mapping & Re-Marking
 \pp
     Priority Mapping maps original priorities of packets to internal priorities and is implemented
     in the inbound direction. Traffic re-marking maps internal priorities to packet priorities and is
     implemented in the outbound direction, it sets  or modifies the packet priority to relay QoS
     information to the connected device.  you can refer to the API ctc_qos_set_domain_map() to
     see more details.
\p
\b
\B Congestion management:
\pp
     Provides means to manage and control traffic when traffic congestion occurs.
     Packets sent from one interface are placed into multiple queues that are marked with different
     priorities. The packets are sent based on the priorities. Different queue scheduling mechanisms
     are designed for different situations and lead to different results. the Centec implements congestion
     management by hierarchical shaping and scheduling. Traffic manger totally supports total 1280 queues,
     128 groups and 64 ports. Up to 3-stage shaping and 2-stage scheduling can be performed to regulate the
     traffic output rate at queue level, group level and port level.

\ppp
        1)Queue management
      \ddd  Networks port support 10 queues,8 Ucast Queue/1 Mcast Queue/1 Span Queue
      \ddd  To CPU packet support 128 queues
      \ddd  Stacking Port support 16 queues,8 Ucast Queue,8 CPU packet
\ppp
         You can refer to the API ctc_qos_set_queue() to see more details.
\ppp
         2) Traffic  shaping
\ppp
          The Centec Qos support 3-stage shaping:
	 \ddd   Queue Shaping
	 \ddd   Group shaping
	 \ddd   Port shaping
\ppp
         You can refer to the API ctc_qos_qos_set_shape() to see more details.

\ppp
        3) Traffic scheduling
\ppp
         The Centec Qos support 2-stage scheduling:
     \dd   SP + WFQ scheduling in a group
     \dd   WFQ scheduling between multiple groups in a physical port
\ppp
        You can refer to the API ctc_qos_set_sched() to see more details.
\b
\p
\B Congestion avoidance
\pp
     Congestion avoidance is a flow control technique used to relieve network overload. A system configured
     with congestion avoidance monitors network resources such as queues and memory buffers. When congestion
     occurs or aggravates, the system discards packets.
     The Centec Qos support two drop policies:
   \dd   Tail drop
   \dd   Weighted Random Early Detection (WRED)
\pp
    You can refer to the API ctc_qos_set_drop_scheme() to see more details.

\S ctc_qos.h:ctc_qos_color_t
\S ctc_qos.h:ctc_qos_trust_type_t
\S ctc_qos.h:ctc_qos_domain_map_type_t
\S ctc_qos.h:ctc_qos_glb_cfg_type_t
\S ctc_qos.h:ctc_qos_policer_type_t
\S ctc_qos.h:ctc_qos_policer_mode_t
\S ctc_qos.h:ctc_queue_drop_mode_t
\S ctc_qos.h:ctc_qos_queue_cfg_type_t
\S ctc_qos.h:ctc_qos_shape_type_t
\S ctc_qos.h:ctc_qos_sched_type_t

\S ctc_qos.h:ctc_qos_domain_map_t
\S ctc_qos.h:ctc_qos_policer_t
\S ctc_qos.h:ctc_qos_glb_cfg_t
\S ctc_qos.h:ctc_qos_drop_t
\S ctc_qos.h:ctc_qos_queue_cfg_t
\S ctc_qos.h:ctc_qos_shape_t
\S ctc_qos.h:ctc_qos_sched_t
\S ctc_qos.h:ctc_qos_global_cfg_t

*/

#ifndef _CTC_USW_QOS_H
#define _CTC_USW_QOS_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_const.h"
#include "ctc_parser.h"
#include "ctc_qos.h"

/**
 @addtogroup qos QOS
 @{
*/

/**
 @brief     Init qos module.

 @param[in] lchip    local chip id

 @param[in] p_glb_parm      Init parameter

 @remark[D2.TM]     Init qos module.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_init(uint8 lchip, void* p_glb_parm);

/**
 @brief De-Initialize qos module

 @param[in] lchip    local chip id

 @remark[D2.TM]  User can de-initialize the qos configuration

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_deinit(uint8 lchip);

/**
 @brief     set qos global configure.

 @param[in] lchip    local chip id

 @param[out] p_glb_cfg    point to  global configure

 @remark[D2.TM]    Qos global configure such as resource manger, policer, shaping global enable

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg);

/**
 @brief     get qos global configure.

 @param[in] lchip    local chip id

 @param[in] p_glb_cfg    point to  global configure

 @remark[D2.TM]    Get global configure such as resource manger, policer, shaping global enable

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_global_config(uint8 lchip, ctc_qos_glb_cfg_t* p_glb_cfg);

/**
 @brief     set qos domain mapping

 @param[in] lchip    local chip id

 @param[in] p_domain_map    point to domain mapping

 @remark[D2.TM]    Set Priority Mapping & Re-Marking

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);

/**
 @brief     get qos domain mapping

 @param[in] lchip    local chip id

 @param[in|out] p_domain_map    point to  domain mapping

 @remark[D2.TM]   Get ingress mapping and egress mapping

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_domain_map(uint8 lchip, ctc_qos_domain_map_t* p_domain_map);

/**
 @brief     set qos policer

 @param[in] lchip    local chip id

 @param[in] p_policer    point to  policer

 @remark[D2.TM]   Create a policer entry,include port, flow, vlan and MEF BWP policer.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_policer(uint8 lchip, ctc_qos_policer_t* p_policer);
/**
 @brief     get qos policer

 @param[in] lchip    local chip id

 @param[in] p_policer    point to  policer

 @remark[D2.TM]   Get a policer entry,include port, flow, vlan and MEF BWP policer.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_policer(uint8 lchip, ctc_qos_policer_t* p_policer);
/**
 @brief     set qos queue configure

 @param[in] lchip    local chip id

 @param[in] p_que_cfg    point to  queue configure

 @remark[D2.TM]   Set queue configure,include priority to queue maping,
           protocol packet to cpu,queue stats,and so on.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_queue(uint8 lchip, ctc_qos_queue_cfg_t* p_que_cfg);

/**
 @brief     set qos shaping

 @param[in] lchip    local chip id

 @param[in] p_shape    point to  shaping configure

 @remark[D2.TM]   Configure traffic shaping parameters,include port, queue, group shaping.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_shape(uint8 lchip, ctc_qos_shape_t* p_shape);

/**
 @brief     get qos shaping

 @param[in] lchip    local chip id

 @param[in] p_shape    point to  shaping configure

 @remark[D2.TM]   Get traffic shaping parameters,include port, queue, group shaping.

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_shape(uint8 lchip, ctc_qos_shape_t* p_shape);

/**
 @brief     set qos scheduling

 @param[in] lchip    local chip id

 @param[in] p_sched    point to  schedule configure

 @remark[D2.TM]   Configure traffic scheduling parameters ,include port, queue, group shaping.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_sched(uint8 lchip, ctc_qos_sched_t* p_sched);
/**
 @brief     set qos scheduling

 @param[in] lchip    local chip id

 @param[in] p_sched    point to  schedule configure

 @remark[D2.TM]   Configure traffic scheduling parameters ,include port, queue, group shaping.
 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_sched(uint8 lchip, ctc_qos_sched_t* p_sched);

/**
 @brief     get qos drop scheme

 @param[in] lchip    local chip id

 @param[in] p_drop    point to  drop configure

 @remark[D2.TM]  Get  TD/WRED Parameters

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop);

/**
 @brief     get qos drop scheme

 @param[in] lchip    local chip id

 @param[in|out] p_drop    point to  drop configure

 @remark[D2.TM]   Get TD/WRED Parameters

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_drop_scheme(uint8 lchip, ctc_qos_drop_t* p_drop);

/**
 @brief     set qos resource management

 @param[in] lchip    local chip id

 @param[in] p_resrc    point to  resource configure

 @remark[D2.TM]  Configure  resource management parameters

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_set_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);
/**
 @brief     get qos resource management

 @param[in] lchip    local chip id

 @param[in] p_resrc    point to  resource configure

 @remark[D2.TM]  Get  resource management parameters

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_get_resrc(uint8 lchip, ctc_qos_resrc_t* p_resrc);

/**
 @brief     query resource pool stats

 @param[in] lchip    local chip id

 @param[in|out] p_stats    point to pool stats

 @remark[D2.TM]  Get resource pool stats

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_query_pool_stats(uint8 lchip, ctc_qos_resrc_pool_stats_t* p_stats);

/**
 @brief     query queue stats

 @param[in] lchip    local chip id

 @param[in|out] p_queue_stats    point to  queue stats

 @remark[D2.TM]   Get queue stats include dequeue and drop stats

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_query_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats);

/**
 @brief     clear queue stats

 @param[in] lchip    local chip id

 @param[in] p_queue_stats    point to  queue stats

 @remark[D2.TM]   Clear the queue stats and reset to zero

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_clear_queue_stats(uint8 lchip, ctc_qos_queue_stats_t* p_queue_stats);

/**
 @brief     query policer stats

 @param[in] lchip    local chip id

 @param[in|out] p_policer_stats    point to  policer stats

 @remark[D2.TM]   Get stats of green,yellow,red color stats

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_query_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats);

/**
 @brief     clear policer stats

 @param[in] lchip    local chip id

 @param[in] p_policer_stats    point to  policer stats

 @remark[D2.TM]   Reset the policers to zero

 @return CTC_E_XXX
*/
extern int32
ctc_usw_qos_clear_policer_stats(uint8 lchip, ctc_qos_policer_stats_t* p_policer_stats);

/**@} end of @addtogroup qos QOS*/

#ifdef __cplusplus
}
#endif

#endif

