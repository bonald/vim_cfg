
#ifndef __TBL_QOS_PORT_SHAPE_PROFILE_DEFINE_H__
#define __TBL_QOS_PORT_SHAPE_PROFILE_DEFINE_H__

/* TBL_QOS_PORT_SHAPE_PROFILE field defines */
typedef enum
{
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_KEY                  = 0 ,  /* RW */
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_NAME                 = 1 ,  /* RW */
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR                  = 2 ,  /* RW */
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_REF_CNT              = 3 ,  /* RW */
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_FLAGS_PIR            = 4 ,  /* READ */
    TBL_QOS_PORT_SHAPE_PROFILE_FLD_MAX                  = 5 
} tbl_qos_port_shape_profile_field_id_t;

/* TBL_QOS_PORT_SHAPE_PROFILE defines */
typedef struct
{
    uint32               id;
} tbl_qos_port_shape_profile_key_t;

typedef struct
{
    tbl_qos_port_shape_profile_key_t key;
    char                 name[QOS_NAME_SIZE+1];
    uint32               pir;                 /* kbits/s */
    uint32               ref_cnt;
    uint32               flags;               /* GLB_PORT_SHAPE_PROFILE_FLAGS_PIR */
} tbl_qos_port_shape_profile_t;

typedef struct
{
    tbl_qos_port_shape_profile_t *qos_port_shape_profile_array[GLB_QOS_PORT_SHAPE_PROFILE_NUM];
} tbl_qos_port_shape_profile_master_t;

#endif /* !__TBL_QOS_PORT_SHAPE_PROFILE_DEFINE_H__ */

