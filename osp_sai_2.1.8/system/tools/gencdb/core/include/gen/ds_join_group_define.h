
#ifndef __DS_JOIN_GROUP_DEFINE_H__
#define __DS_JOIN_GROUP_DEFINE_H__

/* DS_JOIN_GROUP field defines */
typedef enum
{
    DS_JOIN_GROUP_FLD_KEY                  = 0 ,  /* READ */
    DS_JOIN_GROUP_FLD_VID                  = 1 ,  /* READ */
    DS_JOIN_GROUP_FLD_TYPE                 = 2 ,  /* READ */
    DS_JOIN_GROUP_FLD_MAX                  = 3 
} ds_join_group_field_id_t;

/* DS_JOIN_GROUP defines */
typedef struct
{
    addr_ipv4_t          group;
} ds_join_group_key_t;

typedef struct
{
    ds_join_group_key_t  key;
    vid_t                vid;
    uint8                type;                /* igmp_type_t */
} ds_join_group_t;

#endif /* !__DS_JOIN_GROUP_DEFINE_H__ */

