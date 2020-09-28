
#ifndef __TBL_FEA_GLOBAL_DEFINE_H__
#define __TBL_FEA_GLOBAL_DEFINE_H__

/* TBL_FEA_GLOBAL field defines */
typedef enum
{
    TBL_FEA_GLOBAL_FLD_ROUTER_ID            = 0 ,  /* RW */
    TBL_FEA_GLOBAL_FLD_PORT_STATS_MS        = 1 ,  /* RW */
    TBL_FEA_GLOBAL_FLD_MAX                  = 2 
} tbl_fea_global_field_id_t;

/* TBL_FEA_GLOBAL defines */
typedef struct
{
    uint64               router_id;
    uint32               port_stats_ms;
} tbl_fea_global_t;

#endif /* !__TBL_FEA_GLOBAL_DEFINE_H__ */

