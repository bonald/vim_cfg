
#ifndef __TBL_CPU_LIMIT_DEFINE_H__
#define __TBL_CPU_LIMIT_DEFINE_H__

/* TBL_CPU_LIMIT field defines */
typedef enum
{
    TBL_CPU_LIMIT_FLD_KEY                  = 0 ,  /* READ */
    TBL_CPU_LIMIT_FLD_PERCENT              = 1 ,  /* RW */
    TBL_CPU_LIMIT_FLD_MAX                  = 2 
} tbl_cpu_limit_field_id_t;

/* TBL_CPU_LIMIT defines */
typedef struct
{
    char                 name[COMM_NAME_SIZE];
} tbl_cpu_limit_key_t;

typedef struct
{
    tbl_cpu_limit_key_t  key;
    uint32               percent;
} tbl_cpu_limit_t;

typedef struct
{
    ctclib_slist_t       *cpu_limit_list;
} tbl_cpu_limit_master_t;

#endif /* !__TBL_CPU_LIMIT_DEFINE_H__ */

