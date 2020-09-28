#ifndef __HAGT_STATS_PRIV_H__
#define __HAGT_STATS_PRIV_H__

#define HAGT_STATS_LOG_EMERG(fmt, args...)   HAGT_LOG_EMERG(fmt, ##args)
#define HAGT_STATS_LOG_ALERT(fmt, args...)   HAGT_LOG_ALERT(fmt, ##args)
#define HAGT_STATS_LOG_CRIT(fmt, args...)    HAGT_LOG_CRIT(fmt, ##args)
#define HAGT_STATS_LOG_ERR(fmt, args...)     HAGT_LOG_ERR(fmt, ##args)
#define HAGT_STATS_LOG_WARN(fmt, args...)    HAGT_LOG_WARN(fmt, ##args)
#define HAGT_STATS_LOG_NOTICE(fmt, args...)  HAGT_LOG_NOTICE(fmt, ##args)
#define HAGT_STATS_LOG_INFO(fmt, args...)    HAGT_LOG_INFO(fmt, ##args)

#define HAGT_STATS_MAX_ITEM 64
#if 0
#define HAGT_DEBUG_STATS(fmt, args...)                                            \
    HAGT_LOG_DEBUG(stats, stats, DEBUG_STATS, fmt, ##args)

#define HAGT_DEBUG_STATS_FUNC()                                                   \
    HAGT_LOG_DEBUG(stats, stats, DEBUG_STATS, "Enter into %s.", __FUNCTION__)
#else
#define HAGT_DEBUG_STATS(fmt, args...)                                            
#define HAGT_DEBUG_STATS_FUNC()
#endif
#define HAGT_STATS_CALLOC(size)                                                   \
    HAGT_CALLOC(MEM_FEA_HAGT_STATS, (size))

#define HAGT_STATS_MALLOC(size)                                                   \
    HAGT_MALLOC(MEM_FEA_HAGT_STATS, (size))

#define HAGT_STATS_FREE(ptr)                                                      \
    HAGT_FREE(MEM_FEA_HAGT_STATS, ptr)

#define HAGT_STATS_WAIT_TIMER 1000 /*1s*/

/*modified by weij for bug 17826, 2012-01-12*/
#define HAGT_UINT64_TO_OCTET_STR(data, str_s, str_d)                \
do{                                                                 \
    int32 i;                                                        \
    str_s.buf = str_d;                                              \
    str_s.size = sizeof(uint64);                                    \
    for(i = 0; i < sizeof(uint64); i++)                             \
    {                                                               \
        str_d[i] = (uint8)((data >> (i*8))&0xFF);                  \
    }                                                               \
}while(0)


#define HAGT_STATS_CHECK_TYPE(type)                                             \
do{                                                                             \
    if(type >= GLB_STATS_TYPE_MAX)                                              \
    {                                                                           \
        HAGT_LOG_ERR("Invalid Stats Type, %s:%d", __FUNCTION__, __LINE__);      \
        return HAGT_E_INVALID_PARAM;                                            \
    }                                                                           \
}while(0)

#define HAGT_STATS_PTR_CHECK(ptr)                                               \
do {                                                                            \
    if (NULL == ptr)                                                            \
    {                                                                           \
        HAGT_LOG_ERR("Invalid Pointer, %s:%d", __FUNCTION__, __LINE__);         \
        return HAGT_E_INVALID_PTR;                                              \
    }                                                                           \
} while(0)

#define HAGT_STATS_MEM_PTR_CHECK(ptr)                                           \
do {                                                                            \
    if (NULL == ptr)                                                            \
    {                                                                           \
        HAGT_LOG_ERR("No memory, %s:%d", __FUNCTION__, __LINE__);               \
        return HAGT_E_NO_MEMORY;                                                \
    }                                                                           \
} while(0)

struct hagt_stats_master_s
{
    hagt_stast_pm_data_t pm_data[GLB_STATS_TYPE_MAX];
};
typedef struct hagt_stats_master_s hagt_stats_master_t;

#endif /*__HAGT_STATS_PRIV_H__*/

