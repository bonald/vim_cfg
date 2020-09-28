
#ifndef __TBL_SYS_LOAD_DEFINE_H__
#define __TBL_SYS_LOAD_DEFINE_H__

/* TBL_SYS_LOAD field defines */
typedef enum
{
    TBL_SYS_LOAD_FLD_SYS_LOAD_EN          = 0 ,  /* RW */
    TBL_SYS_LOAD_FLD_CLI_BACK_PRESSURE_EN = 1 ,  /* RW */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_NOTICE_THRESHOLD = 2 ,  /* RW */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_WARNING_THRESHOLD = 3 ,  /* RW */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_SWITCH = 4 ,  /* READ */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_ROUTED = 5 ,  /* READ */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_CDS  = 6 ,  /* READ */
    TBL_SYS_LOAD_FLD_CDB_QUEUE_DEPTH_FEA  = 7 ,  /* READ */
    TBL_SYS_LOAD_FLD_MAX                  = 8 
} tbl_sys_load_field_id_t;

/* TBL_SYS_LOAD defines */
typedef struct
{
    uint32               sys_load_en;
    uint32               cli_back_pressure_en;
    uint32               cdb_queue_notice_threshold;
    uint32               cdb_queue_warning_threshold;
    uint32               cdb_queue_depth_switch;
    uint32               cdb_queue_depth_routed;
    uint32               cdb_queue_depth_cds;
    uint32               cdb_queue_depth_fea;
} tbl_sys_load_t;

#endif /* !__TBL_SYS_LOAD_DEFINE_H__ */

