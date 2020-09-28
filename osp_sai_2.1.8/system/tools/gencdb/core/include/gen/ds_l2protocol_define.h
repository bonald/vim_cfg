
#ifndef __DS_L2PROTOCOL_DEFINE_H__
#define __DS_L2PROTOCOL_DEFINE_H__

/* DS_L2PROTOCOL field defines */
typedef enum
{
    DS_L2PROTOCOL_FLD_KEY                  = 0 ,  /* READ */
    DS_L2PROTOCOL_FLD_IS_STANDARD_PRO      = 1 ,  /* RW */
    DS_L2PROTOCOL_FLD_L2PRO_ACTION         = 2 ,  /* RW */
    DS_L2PROTOCOL_FLD_L2PRO_VLAN           = 3 ,  /* RW */
    DS_L2PROTOCOL_FLD_L2PRO_EVC_NAME       = 4 ,  /* RW */
    DS_L2PROTOCOL_FLD_MAX                  = 5 
} ds_l2protocol_field_id_t;

/* DS_L2PROTOCOL defines */
typedef struct
{
    char                 l2pro_name[GLB_L2_PROTOCOL_NAME_SIZE+1];
} ds_l2protocol_key_t;

typedef struct
{
    ds_l2protocol_key_t  key;
    uint32               is_standard_pro;     /* If is_standard_pro is 1, then it is standard protocol*/
    uint32               l2pro_action;        /* glb_l2_proto_action_t */
    vid_t                l2pro_vlan;          /* Vlan id */
    char                 l2pro_evc_name[GLB_VLAN_EVC_NAME_LEN+1]; /* EVC name */
} ds_l2protocol_t;

#endif /* !__DS_L2PROTOCOL_DEFINE_H__ */

