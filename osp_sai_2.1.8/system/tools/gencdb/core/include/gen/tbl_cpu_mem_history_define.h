
#ifndef __TBL_CPU_MEM_HISTORY_DEFINE_H__
#define __TBL_CPU_MEM_HISTORY_DEFINE_H__

/* TBL_CPU_MEM_HISTORY field defines */
typedef enum
{
    TBL_CPU_MEM_HISTORY_FLD_POLLING_INDEX        = 0 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_SAVING_INDEX         = 1 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_HOUR_START           = 2 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_DAY_START            = 3 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_HOUR_LAST_SAVE       = 4 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_DAY_LAST_SAVE        = 5 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_CPU_POLLING_SAVE     = 6 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_HOUR     = 7 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_CPU_HISTORY_DAY      = 8 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_MEM_POLLING_SAVE     = 9 ,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_HOUR     = 10,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_MEM_HISTORY_DAY      = 11,  /* READ */
    TBL_CPU_MEM_HISTORY_FLD_MAX                  = 12
} tbl_cpu_mem_history_field_id_t;

/* TBL_CPU_MEM_HISTORY defines */
typedef struct
{
    uint64               polling_index;
    uint64               saving_index;
    uint32               hour_start;
    uint32               day_start;
    uint64               hour_last_save;
    uint64               day_last_save;
    uint8                cpu_polling_save[GLB_CPU_MEM_MINUTE_SIZE];
    uint8                cpu_history_hour[GLB_CPU_MEM_HOUR_SIZE];
    uint8                cpu_history_day[GLB_CPU_MEM_DAY_SIZE];
    uint8                mem_polling_save[GLB_CPU_MEM_MINUTE_SIZE];
    uint8                mem_history_hour[GLB_CPU_MEM_HOUR_SIZE];
    uint8                mem_history_day[GLB_CPU_MEM_DAY_SIZE];
} tbl_cpu_mem_history_t;

#endif /* !__TBL_CPU_MEM_HISTORY_DEFINE_H__ */

