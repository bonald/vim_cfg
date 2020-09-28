#ifndef __HAGT_SFLOW_PRIV_H__
#define __HAGT_SFLOW_PRIV_H__

#define HAGT_SFLOW_PORT_LOG_STATS_INTERVAL 3600000/* 3s */

#define HAGT_SFLOW_CALLOC(size)                                                 \
    HAGT_CALLOC(CTCLIB_MEM_SFLOW_MODULE, (size))

#define HAGT_SFLOW_MALLOC(size)                                                 \
    HAGT_MALLOC(CTCLIB_MEM_SFLOW_MODULE, (size))

#define HAGT_SFLOW_FREE(ptr)                                                    \
    HAGT_FREE(CTCLIB_MEM_SFLOW_MODULE, ptr)

#define HAGT_DEBUG_SFLOW(fmt, args...)                                          \

#define HAGT_DEBUG_SFLOW_FUNC()                                                 

#define HAGT_SFLOW_MAX_LOG_THRESHOLD   0x7fff
#define HAGT_SFLOW_WEIGHT_MAP_TO_THRESHOLD(weight, threshold)                   \
do{                                                                             \
    if(weight > 15)                                                             \
    {                                                                           \
        return HAGT_E_INVALID_PARAM;                                            \
    }                                                                           \
    threshold = 1 << weight;                                                    \
    threshold = threshold > HAGT_SFLOW_MAX_LOG_THRESHOLD ? HAGT_SFLOW_MAX_LOG_THRESHOLD : threshold; \
} while(0)
    

#define HAGT_SFLOW_PTR_CHECK(ptr)                                               \
do {                                                                            \
    if (NULL == ptr)                                                            \
    {                                                                           \
        HAGT_LOG_ERR("Invalid Pointer, %s:%d", __FUNCTION__, __LINE__);         \
        return HAGT_E_INVALID_PTR;                                              \
    }                                                                           \
} while(0)

#define HAGT_SFLOW_MEM_PTR_CHECK(ptr)                                           \
do {                                                                            \
    if (NULL == ptr)                                                            \
    {                                                                           \
        HAGT_LOG_ERR("No memory, %s:%d", __FUNCTION__, __LINE__);               \
        return HAGT_E_NO_MEMORY;                                                \
    }                                                                           \
} while(0)

struct hagt_sflow_master_s
{
    glb_stats_t* p_port_log_stats; /*CTC_MAX_LOCAL_CHIP_NUM * CTC_MAX_HUMBER_PHY_PORT*/
};
typedef struct hagt_sflow_master_s hagt_sflow_master_t;

#endif /* __HAGT_SFLOW_PRIV_H__ */
