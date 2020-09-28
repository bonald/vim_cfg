
#ifndef __TBL_SFLOW_COUNTER_PORT_DEFINE_H__
#define __TBL_SFLOW_COUNTER_PORT_DEFINE_H__

/* TBL_SFLOW_COUNTER_PORT field defines */
typedef enum
{
    TBL_SFLOW_COUNTER_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_SFLOW_COUNTER_PORT_FLD_LEFT_INTERVAL        = 1 ,  /* RW */
    TBL_SFLOW_COUNTER_PORT_FLD_SEQUENCE             = 2 ,  /* RW */
    TBL_SFLOW_COUNTER_PORT_FLD_MAX                  = 3 
} tbl_sflow_counter_port_field_id_t;

/* TBL_SFLOW_COUNTER_PORT defines */
typedef struct
{
    uint32               ifindex;
} tbl_sflow_counter_port_key_t;

typedef struct
{
    tbl_sflow_counter_port_key_t key;
    uint32               left_interval;       /* left interval for next report, msec */
    uint32               sequence;            /* sflow packet sequence */
} tbl_sflow_counter_port_t;

typedef struct
{
    ctclib_slist_t       *counter_port_list;
} tbl_sflow_counter_port_master_t;

#endif /* !__TBL_SFLOW_COUNTER_PORT_DEFINE_H__ */

