
#ifndef __TBL_CPU_TRAFFIC_GROUP_DEFINE_H__
#define __TBL_CPU_TRAFFIC_GROUP_DEFINE_H__

/* TBL_CPU_TRAFFIC_GROUP field defines */
typedef enum
{
    TBL_CPU_TRAFFIC_GROUP_FLD_KEY                  = 0 ,  /* READ */
    TBL_CPU_TRAFFIC_GROUP_FLD_RATE                 = 1 ,  /* RW */
    TBL_CPU_TRAFFIC_GROUP_FLD_MAX                  = 2 
} tbl_cpu_traffic_group_field_id_t;

/* TBL_CPU_TRAFFIC_GROUP defines */
typedef struct
{
    uint32               class_id;
} tbl_cpu_traffic_group_key_t;

typedef struct
{
    tbl_cpu_traffic_group_key_t key;
    uint64               rate;
} tbl_cpu_traffic_group_t;

typedef struct
{
    tbl_cpu_traffic_group_t *array[GLB_CPU_TRAFFIC_GROUP_MAX];
} tbl_cpu_traffic_group_master_t;

#endif /* !__TBL_CPU_TRAFFIC_GROUP_DEFINE_H__ */

