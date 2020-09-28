
#ifndef __TBL_CPU_TRAFFIC_DEFINE_H__
#define __TBL_CPU_TRAFFIC_DEFINE_H__

/* TBL_CPU_TRAFFIC field defines */
typedef enum
{
    TBL_CPU_TRAFFIC_FLD_KEY                  = 0 ,  /* READ */
    TBL_CPU_TRAFFIC_FLD_RATE                 = 1 ,  /* RW */
    TBL_CPU_TRAFFIC_FLD_CLASS_ID             = 2 ,  /* RW */
    TBL_CPU_TRAFFIC_FLD_MAX                  = 3 
} tbl_cpu_traffic_field_id_t;

/* TBL_CPU_TRAFFIC defines */
typedef struct
{
    uint32               reason_id;
} tbl_cpu_traffic_key_t;

typedef struct
{
    tbl_cpu_traffic_key_t key;
    uint64               rate;
    uint32               class_id;
} tbl_cpu_traffic_t;

typedef struct
{
    tbl_cpu_traffic_t    *array[GLB_CPU_TRAFFIC_MAX];
} tbl_cpu_traffic_master_t;

#endif /* !__TBL_CPU_TRAFFIC_DEFINE_H__ */

