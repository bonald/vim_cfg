
#ifndef __TBL_ACL_L4_PORT_RANGE_DEFINE_H__
#define __TBL_ACL_L4_PORT_RANGE_DEFINE_H__

/* TBL_ACL_L4_PORT_RANGE field defines */
typedef enum
{
    TBL_ACL_L4_PORT_RANGE_FLD_KEY                  = 0 ,  /* READ */
    TBL_ACL_L4_PORT_RANGE_FLD_PORT_MIN             = 1 ,  /* RW */
    TBL_ACL_L4_PORT_RANGE_FLD_PORT_MAX             = 2 ,  /* RW */
    TBL_ACL_L4_PORT_RANGE_FLD_CNT                  = 3 ,  /* RW */
    TBL_ACL_L4_PORT_RANGE_FLD_IS_SRC               = 4 ,  /* RW */
    TBL_ACL_L4_PORT_RANGE_FLD_MAX                  = 5 
} tbl_acl_l4_port_range_field_id_t;

/* TBL_ACL_L4_PORT_RANGE defines */
typedef struct
{
    uint32               id;
} tbl_acl_l4_port_range_key_t;

typedef struct
{
    tbl_acl_l4_port_range_key_t key;
    uint16               port_min;
    uint16               port_max;
    uint32               cnt;
    uint8                is_src;              /* src_port: 1, dest_port: 0 */
} tbl_acl_l4_port_range_t;

typedef struct
{
    tbl_acl_l4_port_range_t *acl_l4_port_range_array[GLB_ACL_L4PORT_ACE_NUM_MAX];
} tbl_acl_l4_port_range_master_t;

#endif /* !__TBL_ACL_L4_PORT_RANGE_DEFINE_H__ */

