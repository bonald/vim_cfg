
#ifndef __TBL_MEM_SUMMARY_DEFINE_H__
#define __TBL_MEM_SUMMARY_DEFINE_H__

/* TBL_MEM_SUMMARY field defines */
typedef enum
{
    TBL_MEM_SUMMARY_FLD_TOTAL                = 0 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_USED                 = 1 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_FREE                 = 2 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_BUFFER               = 3 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_CACHED               = 4 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_MEM_HIGH             = 5 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_CPU_HIGH             = 6 ,  /* READ */
    TBL_MEM_SUMMARY_FLD_CPU_HIGH_THRESHOLD   = 7 ,  /* RW */
    TBL_MEM_SUMMARY_FLD_CPU_LOW_THRESHOLD    = 8 ,  /* RW */
    TBL_MEM_SUMMARY_FLD_MAX                  = 9 
} tbl_mem_summary_field_id_t;

/* TBL_MEM_SUMMARY defines */
typedef struct
{
    uint32               total;
    uint32               used;
    uint32               free;
    uint32               buffer;
    uint32               cached;
    uint32               mem_high;
    uint32               cpu_high;
    uint32               cpu_high_threshold;
    uint32               cpu_low_threshold;
} tbl_mem_summary_t;

#endif /* !__TBL_MEM_SUMMARY_DEFINE_H__ */

