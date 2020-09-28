#ifndef __SAL_TASK_H__
#define __SAL_TASK_H__

/**
 * @file sal_task.h
 */

/**
 * @defgroup task Tasks
 * @{
 */

#if defined(_SAL_LINUX_KM)

#define SAL_TASK_PRIO_HIGH      1
#define SAL_TASK_PRIO_LOW       139
#define SAL_TASK_PRIO_RT_HIGH   1
#define SAL_TASK_PRIO_RT_LOW    99
#define SAL_TASK_PRIO_NICE_HIGH 100
#define SAL_TASK_PRIO_NICE_LOW  139
#define SAL_TASK_PRIO_DEF       120

struct sal_task
{
    struct task_struct* ktask;
    char name[64];
    void (* start)(void*);
    void* arg;
    struct completion started;
    struct completion done;
    int prio;
    pid_t tid;
    uint64_t cpu_mask;
    uint32_t task_type;       /**<[TM]  the type of the task, reference to sal_task_type_t*/
};

#elif defined(_SAL_LINUX_UM)

#define SAL_TASK_PRIO_HIGH      1
#define SAL_TASK_PRIO_LOW       139
#define SAL_TASK_PRIO_RT_HIGH   1
#define SAL_TASK_PRIO_RT_LOW    99
#define SAL_TASK_PRIO_NICE_HIGH 100
#define SAL_TASK_PRIO_NICE_LOW  139
#define SAL_TASK_PRIO_DEF       120

struct sal_task
{
    pthread_t pth;
    char name[64];
    void (* start)(void*);
    void* arg;
    int prio;
    int done;
    pid_t tid;
    uint64_t cpu_mask;
    uint32_t task_type;       /**<[TM]  the type of the task, reference to sal_task_type_t*/
};

#elif defined(_SAL_VXWORKS)

#define SAL_TASK_PRIO_HIGH      0
#define SAL_TASK_PRIO_LOW       255
#define SAL_TASK_PRIO_DEF       128
#define SAL_TASK_PRIO_NICE_HIGH 100
#define SAL_TASK_PRIO_NICE_LOW  139

typedef struct vx_task_s
{
    char thread_type;
}* vx_task_t;

struct sal_task
{
    char name[64];
    void (* start)(void*);
    int prio;
    int tid;
    vx_task_t pthread;
    uint64_t cpu_mask;
    uint32_t task_type;       /**<[TM]  the type of the task, reference to sal_task_type_t*/
};

#endif


/** Task Object */
typedef struct sal_task sal_task_t;

#ifdef __cplusplus
extern "C"
{
#endif

#define SAL_DEF_TASK_STACK_SIZE     (128 * 1024)

#define CTC_SAL_TASK_MAX_NUM    64
#define SAL_TASK_MAX_NAME_LEN   64

static int g_ctc_sal_task_count = 0;
static sal_task_t *g_ctc_sal_task_array[CTC_SAL_TASK_MAX_NUM] = {NULL};

/** system time */
typedef struct sal_systime_s
{
    unsigned int tv_sec;     /* seconds */
    unsigned int tv_usec;    /* microseconds */
} sal_systime_t;

enum sal_task_type_e
{
    SAL_TASK_TYPE_MAIN,
    SAL_TASK_TYPE_PACKET,
    SAL_TASK_TYPE_STATS,
    SAL_TASK_TYPE_ALPM,
    SAL_TASK_TYPE_LINK_SCAN,
    SAL_TASK_TYPE_ECC_SCAN,
    SAL_TASK_TYPE_FDB,
    SAL_TASK_TYPE_INTERRUPT,
    SAL_TASK_TYPE_OTHERS,
    SAL_TASK_TYPE_MAX
};
typedef enum sal_task_type_e sal_task_type_t;

/**
 * Create a new task
 *
 * @param[out] ptask
 * @param[in]  name
 * @param[in]  stack_size
 * @param[in]  start
 * @param[in]  arg
 *
 * @return
 */
int ctc_sal_task_create(sal_task_t** ptask,
                          char* name,
                          size_t stack_size,
                          int prio,
                          void (* start)(void*),
                          void* arg);

/**
 * Destroy the task
 *
 * @param[in] task
 */
void ctc_sal_task_destroy(sal_task_t* task);

/**
 * Exit the current executing task
 */
#if defined(_SAL_LINUX_KM)
void ctc_sal_task_exit(sal_task_t *task);
#elif defined(_SAL_LINUX_UM)
void ctc_sal_task_exit(void);
#elif defined(_SAL_VXWORKS)
void ctc_sal_task_exit(int rc);
#endif

/**
 * Sleep for <em>msec</em> milliseconds
 *
 * @param[in] msec
 */
void ctc_sal_task_sleep(uint32_t msec);

/**
 * Yield the processor voluntarily to other tasks
 */
void ctc_sal_task_yield(void);

/**
 * Get current time
 *
 * @param tv
 */
void ctc_sal_gettime(sal_systime_t* tv);

/**
 * Get uptime since last startup
 *
 * @param ts
 */
void ctc_sal_getuptime(struct timespec* ts);

void ctc_sal_udelay(uint32 usec);

void ctc_sal_delay(uint32 sec);

/**
 * Set task's priority
 *
 * @param priority
 */
int
ctc_sal_task_set_priority(int32 priority);

/**
 * Get task's priority 1-99 for realtime SCHED_RR, 100-139 for SCHED_OTHER
 *
 * @param tv
 */
int
ctc_sal_task_get_priority(int32* p_priority);

/**
 * Set thread cpu mask & priority. tid 0 means current task.
 *
 */
int
ctc_sal_task_set_affinity_prio(pid_t tid, uint64 cpu_mask, int32 priority);

/**
 * Get thread cpu mask & priority. tid 0 means current task.
 *
 */
int
ctc_sal_task_get_affinity_prio(pid_t tid, uint64* p_cpu_mask, int32* p_priority);

void
ctc_sal_task_dump(void);

/**
 * Get task  list,
 * @param count: array buffer length
 * @array: pointer for a array buffer
 * @return: 0 success, -1 array is NULL, when count<task number, buffer is not enought, return value is -(task number).
 */
int
ctc_sal_get_task_list(uint8 lchip,  uint8 count, sal_task_t* array);


static INLINE int
_sal_task_add_to_db(sal_task_t* p_task)
{
    if (g_ctc_sal_task_count >= CTC_SAL_TASK_MAX_NUM)
    {
        return 0;
    }

    g_ctc_sal_task_array[g_ctc_sal_task_count++] = p_task;

    return 0;
}

static INLINE int
_sal_task_del_from_db(sal_task_t* p_task)
{
    int i = 0;
    int find = FALSE;

    for (i = 0; i < g_ctc_sal_task_count; i++)
    {
        if (p_task == g_ctc_sal_task_array[i])
        {
            find = TRUE;
            break;
        }
    }

    if (!find)
    {
        return -1;
    }

    if (i < g_ctc_sal_task_count-1)
    {
        g_ctc_sal_task_array[i] = g_ctc_sal_task_array[g_ctc_sal_task_count-1];
    }
    g_ctc_sal_task_array[--g_ctc_sal_task_count] = NULL;

    return 0;
}

#ifdef __cplusplus
}
#endif

/**@}*/ /* End of @defgroup task */

#endif /* !__SAL_TASK_H__ */

