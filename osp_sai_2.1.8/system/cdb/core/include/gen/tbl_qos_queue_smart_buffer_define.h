
#ifndef __TBL_QOS_QUEUE_SMART_BUFFER_DEFINE_H__
#define __TBL_QOS_QUEUE_SMART_BUFFER_DEFINE_H__

/* TBL_QOS_QUEUE_SMART_BUFFER field defines */
typedef enum
{
    TBL_QOS_QUEUE_SMART_BUFFER_FLD_KEY                  = 0 ,  /* RW */
    TBL_QOS_QUEUE_SMART_BUFFER_FLD_SMART_BUFFER_ENABLE  = 1 ,  /* RW */
    TBL_QOS_QUEUE_SMART_BUFFER_FLD_MAX                  = 2 
} tbl_qos_queue_smart_buffer_field_id_t;

/* TBL_QOS_QUEUE_SMART_BUFFER defines */
typedef struct
{
    uint32               queue_id;
} tbl_qos_queue_smart_buffer_key_t;

typedef struct
{
    tbl_qos_queue_smart_buffer_key_t key;
    uint32               smart_buffer_enable;
} tbl_qos_queue_smart_buffer_t;

typedef struct
{
    tbl_qos_queue_smart_buffer_t *qos_queue_smart_buffer_array[GLB_QOS_QUEUE_SMART_BUFFER_NUM];
} tbl_qos_queue_smart_buffer_master_t;

#endif /* !__TBL_QOS_QUEUE_SMART_BUFFER_DEFINE_H__ */

