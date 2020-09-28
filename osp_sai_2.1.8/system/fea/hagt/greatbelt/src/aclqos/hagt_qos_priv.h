/****************************************************************************
 * hagt_qos_priv.h :  Macro, data structure for hal agent qos module
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision :   R0.01
 * Author   :   rongl
 * Date     :   2010-09-15
 ****************************************************************************/

#ifndef __HAGT_QOS_PRIV_H__
#define __HAGT_QOS_PRIV_H__

/*== hagt qos sanity check related macros ==*/

#define HAGT_QOS_PTR_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HAGT_QOS_LOG_ERR("Invaid pointer, %s()@%d", __FUNCTION__, __LINE__); \
        return(HAGT_E_INVALID_PTR); \
    } \
}while(0)


#define HAGT_QOS_MEM_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    { \
        HAGT_QOS_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); \
        return(HAGT_E_NO_MEMORY); \
    } \
}while(0)


/*== hagt qos memory related macros ==*/

#define HAGT_QOS_CALLOC(size) HAGT_CALLOC(MEM_FEA_HAGT_QOS, size)
#define HAGT_QOS_MALLOC(size) HAGT_MALLOC(MEM_FEA_HAGT_QOS, size)
#define HAGT_QOS_FREE(ptr) \
do { \
    HAGT_FREE(MEM_FEA_HAGT_QOS, ptr);  \
    ptr = NULL; \
}while(0)

/*== hagt qos log & debug related macros ==*/

#define HAGT_QOS_LOG_EMERG(fmt, args...)   HAGT_LOG_EMERG(fmt, ##args)
#define HAGT_QOS_LOG_ALERT(fmt, args...)   HAGT_LOG_ALERT(fmt, ##args)
#define HAGT_QOS_LOG_CRIT(fmt, args...)    HAGT_LOG_CRIT(fmt, ##args)
#define HAGT_QOS_LOG_ERR(fmt, args...)     HAGT_LOG_ERR(fmt, ##args)
#define HAGT_QOS_LOG_WARN(fmt, args...)    HAGT_LOG_WARN(fmt, ##args)
#define HAGT_QOS_LOG_NOTICE(fmt, args...)  HAGT_LOG_NOTICE(fmt, ##args)
#define HAGT_QOS_LOG_INFO(fmt, args...)    HAGT_LOG_INFO(fmt, ##args)

/* for log as qos module */
#define HAGT_LOG_QOS_WARN(fmt, args...) log_sys(M_MOD_QOS, E_WARNING, fmt, ##args)
#if 0
#define HAGT_QOS_PROCESS_DEBUG_FUNC() \
do { \
    HAGT_LOG_DEBUG(qos, process, DEBUG_QOS_PRO, "%s()\n", __FUNCTION__); \
}while(0)

#define HAGT_QOS_CLASS_DEBUG_FUNC() \
do { \
    HAGT_LOG_DEBUG(qos, class, DEBUG_QOS_CLS, "%s()\n", __FUNCTION__); \
}while(0)


#define HAGT_QOS_POLICER_DEBUG_FUNC() \
do { \
    HAGT_LOG_DEBUG(qos, policer, DEBUG_QOS_PLC, "%s()\n", __FUNCTION__); \
}while(0)

#define HAGT_QOS_QUEUE_DEBUG_FUNC() \
do { \
    HAGT_LOG_DEBUG(qos, queue, DEBUG_QOS_QUE, "%s()\n", __FUNCTION__); \
}while(0)

#define HAGT_QOS_PROCESS_DEBUG_INFO(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, process, DEBUG_QOS_PRO, fmt, ##args); \
}while(0)

#define HAGT_QOS_CLASS_DEBUG_INFO(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, class, DEBUG_QOS_CLS, fmt, ##args); \
}while(0)

#define HAGT_QOS_POLICER_DEBUG_INFO(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, policer, DEBUG_QOS_PLC, fmt, ##args); \
}while(0)

#define HAGT_QOS_QUEUE_DEBUG_INFO(fmt, args...) \
do { \
    HAGT_LOG_DEBUG(qos, queue, DEBUG_QOS_QUE, fmt, ##args); \
}while(0)
#else
#define HAGT_QOS_PROCESS_DEBUG_FUNC() 

#define HAGT_QOS_CLASS_DEBUG_FUNC() 


#define HAGT_QOS_POLICER_DEBUG_FUNC() 

#define HAGT_QOS_QUEUE_DEBUG_FUNC() 

#define HAGT_QOS_PROCESS_DEBUG_INFO(fmt, args...) 

#define HAGT_QOS_CLASS_DEBUG_INFO(fmt, args...)

#define HAGT_QOS_POLICER_DEBUG_INFO(fmt, args...) 

#define HAGT_QOS_QUEUE_DEBUG_INFO(fmt, args...)

#endif


/*== hagt qos private info macros ==*/
/*modified by xgu for bug 16265, 2011-11-09*/
/*factor = log2[2^30 / (maxth - minth) * maxp]*/
#define HAGT_QOS_MAP_WRED_DROP_PROB(ctc_prob, hagt_prob, hagt_max_min_gap)   \
do {                                                                        \
    uint8  factor;                                                          \
    for (factor = 0; factor < 16; factor++)                                 \
    {                                                                       \
        (ctc_prob) = factor;                                                \
        if (factor + GLB_QOS_MAX_DROP_PROBABILITY_BIT >= 30)                \
        {                                                                   \
            if ((hagt_prob) <= ((1 << (factor + 1 +                         \
                GLB_QOS_MAX_DROP_PROBABILITY_BIT- 30)) * (hagt_max_min_gap)))\
            {                                                               \
                break;                                                      \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            if (((hagt_prob) * (1 << (30 - factor -                         \
                GLB_QOS_MAX_DROP_PROBABILITY_BIT))) <= (hagt_max_min_gap))   \
            {                                                               \
                break;                                                      \
            }                                                               \
        }                                                                   \
    }                                                                       \
}while (0)

#define HAGT_QOS_MAP_RATE_KBIT2BYTE(kbps) ((kbps) * (1000 / 8))

/*modify by chenxw for bug 22441, 2013-07-24.*/
#define HAGT_QOS_MAP_RATE_BYTE2KBIT(byte) ((byte) / 125)

#define HAGT_QOS_INIT_MAP_FUNC_CB(dir, type, func) \
do { \
    g_pst_hagt_qos_master->map_func_cb[dir][type] = func; \
}while(0)

/*removed by wangl for GB QOS ACL 2013-1-29*/
#if 0
#define HAGT_QOS_INIT_ACTION_OP_FUNC_CB(op_action, func) \
do { \
    g_pst_hagt_qos_master->action_op_cb[op_action] = func; \
}while(0)
#endif

typedef int32 (*HAGT_QOS_MAP_FUNC)(HalMsgQosPriColorMap_t** pp_map_tbl, int32 size);
typedef int32 (*HAGT_QOS_ACTION_UPDATE_FUNC)(uint32 label, HalMsgQosFlowEntryId_t* p_entry_id, \
                                             HalMsgQosFlowAction_t* p_action);


#define HAGT_QOS_NOTIFY_PEND_FLOW_ENTRY_INTERVAL   2000    /* 2000ms */
#define HAGT_QOS_RETRY_SET_QUEUE_CLASS_INTERVAL    500    /* 500ms */

struct hagt_qos_pend_flow_entry_s
{
    ctclib_list_node_t node;
    
    uint32 label;

    uint32 rule_id;

    uint8  has_notified;
    uint8  pend_type; /* glb_qos_pend_type_t */
    uint8  reserved[2];
};
typedef struct hagt_qos_pend_flow_entry_s hagt_qos_pend_flow_entry_t;


/* hagt qos master */
struct hagt_qos_master_s 
{
/* modified by cuixl for bug 22441, 2013-03-18 */
    /* Now all map settings use one function(except GLB_QOS_PRI_COLOR_MAP_TYPE_QID_TID) */
    /* so comment off this field */
#if 0    
    HAGT_QOS_MAP_FUNC map_func_cb[GLB_BOTH_DIRECTION][GLB_QOS_PRI_COLOR_MAP_TYPE_MAX];
#endif    
    
/* We don't do resource check before, so we need store these failed entries when resource is ok*/
/* Now we do resource check, so these fields is no need any more */
/*modification start*/
#if 0
    ctclib_list_t pend_flow_entry_list;
    sal_mutex_t   *p_pend_flow_entry_mutex;
#endif    
/*modification end*/

    /*modified by yanxa, for bug 14870, 2011-06-09*/
    // TODO:refer to below, added by cuixl 2013-03-18 
    /* refer to ctc_greatbelt_stats_init(), now all policer and queue stats is enabled*/
    uint8 policer_stats_support_flag;
    uint8 queue_stats_support_flag;
    uint8 flow_proc_flag;
    uint8 port_queue_num;
    
};
typedef struct hagt_qos_master_s hagt_qos_master_t;

/* modified by cuixl for bug 22441, 2013-03-18 */
/*modification start*/
/* move to hagt_qos.c */
#if 0
extern hagt_qos_master_t *g_pst_hagt_qos_master;
#define HAGT_QOS_PORT_QUEUE_NUM g_pst_hagt_qos_master->port_queue_num
#endif
/*modification end*/

#endif /* __HAGT_QOS_PRIV_H__ */

