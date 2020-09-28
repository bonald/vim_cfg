
#ifndef __STBL_VLAN_DEFINE_H__
#define __STBL_VLAN_DEFINE_H__

/* STBL_VLAN field defines */
typedef enum
{
    STBL_VLAN_FLD_KEY                  = 0 ,  /* READ */
    STBL_VLAN_FLD_LEARN_DISABLE        = 1 ,  /* READ */
    STBL_VLAN_FLD_STP_OID              = 2 ,  /* READ */
    STBL_VLAN_FLD_STATS_ID_L2_IN       = 3 ,  /* RW */
    STBL_VLAN_FLD_STATS_ID_L2_OUT      = 4 ,  /* RW */
    STBL_VLAN_FLD_STATS_ID_L3_IN       = 5 ,  /* RW */
    STBL_VLAN_FLD_STATS_ID_L3_OUT      = 6 ,  /* RW */
    STBL_VLAN_FLD_MAX                  = 7 
} stbl_vlan_field_id_t;

/* STBL_VLAN defines */
typedef struct
{
    vid_t                vid;
} stbl_vlan_key_t;

typedef struct
{
    stbl_vlan_key_t      key;
    uint32               learn_disable;
    sai_object_id_t      stp_oid;             /* STP instance OID */
    uint32               stats_id_l2_in;
    uint32               stats_id_l2_out;
    uint32               stats_id_l3_in;
    uint32               stats_id_l3_out;
} stbl_vlan_t;

typedef struct
{
    stbl_vlan_t          *array[GLB_MAX_VLAN_ID];
} stbl_vlan_master_t;

#endif /* !__STBL_VLAN_DEFINE_H__ */

