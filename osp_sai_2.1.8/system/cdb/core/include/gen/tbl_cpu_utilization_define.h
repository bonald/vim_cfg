
#ifndef __TBL_CPU_UTILIZATION_DEFINE_H__
#define __TBL_CPU_UTILIZATION_DEFINE_H__

/* TBL_CPU_UTILIZATION field defines */
typedef enum
{
    TBL_CPU_UTILIZATION_FLD_KEY                  = 0 ,  /* READ */
    TBL_CPU_UTILIZATION_FLD_CPU_USAGE            = 1 ,  /* READ */
    TBL_CPU_UTILIZATION_FLD_MAX                  = 2 
} tbl_cpu_utilization_field_id_t;

/* TBL_CPU_UTILIZATION defines */
typedef struct
{
    char                 name[COMM_NAME_SIZE];
} tbl_cpu_utilization_key_t;

typedef struct
{
    tbl_cpu_utilization_key_t key;
    double               cpu_usage;
} tbl_cpu_utilization_t;

typedef struct
{
    ctclib_hash_t        *cpu_utilize_hash;
    ctclib_slist_t       *cpu_utilize_list;
} tbl_cpu_utilization_master_t;

#endif /* !__TBL_CPU_UTILIZATION_DEFINE_H__ */

