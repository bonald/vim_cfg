
#ifndef __TBL_CPU_MIRROR_GLOBAL_DEFINE_H__
#define __TBL_CPU_MIRROR_GLOBAL_DEFINE_H__

/* TBL_CPU_MIRROR_GLOBAL field defines */
typedef enum
{
    TBL_CPU_MIRROR_GLOBAL_FLD_STRATEGY             = 0 ,  /* RW */
    TBL_CPU_MIRROR_GLOBAL_FLD_BUF_SIZE             = 1 ,  /* RW */
    TBL_CPU_MIRROR_GLOBAL_FLD_BUF_LIST             = 2 ,  /* READ */
    TBL_CPU_MIRROR_GLOBAL_FLD_WRITE_FILE           = 3 ,  /* RW */
    TBL_CPU_MIRROR_GLOBAL_FLD_FLASH_SIZE_THRESHOLD = 4 ,  /* RW */
    TBL_CPU_MIRROR_GLOBAL_FLD_BUF_MUTEX            = 5 ,  /* READ */
    TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_FP           = 6 ,  /* READ */
    TBL_CPU_MIRROR_GLOBAL_FLD_CAPTURE_TIMER        = 7 ,  /* READ */
    TBL_CPU_MIRROR_GLOBAL_FLD_MAX                  = 8 
} tbl_cpu_mirror_global_field_id_t;

/* TBL_CPU_MIRROR_GLOBAL defines */
typedef struct
{
    uint32               strategy;            /* strategy: replace or drop */
    uint32               buf_size;
    ctclib_slist_t       *buf_list;
    uint32               write_file;
    uint64               flash_size_threshold;
    sal_mutex_t          *buf_mutex;
    void                 *capture_fp;
    ctc_task_t           *capture_timer;
} tbl_cpu_mirror_global_t;

#endif /* !__TBL_CPU_MIRROR_GLOBAL_DEFINE_H__ */

