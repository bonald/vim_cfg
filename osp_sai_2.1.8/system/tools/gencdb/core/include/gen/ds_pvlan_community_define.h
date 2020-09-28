
#ifndef __DS_PVLAN_COMMUNITY_DEFINE_H__
#define __DS_PVLAN_COMMUNITY_DEFINE_H__

/* DS_PVLAN_COMMUNITY field defines */
typedef enum
{
    DS_PVLAN_COMMUNITY_FLD_KEY                  = 0 ,  /* READ */
    DS_PVLAN_COMMUNITY_FLD_GROUP_ID             = 1 ,  /* RW */
    DS_PVLAN_COMMUNITY_FLD_PORTS                = 2 ,  /* RW */
    DS_PVLAN_COMMUNITY_FLD_MAX                  = 3 
} ds_pvlan_community_field_id_t;

/* DS_PVLAN_COMMUNITY defines */
typedef struct
{
    vid_t                community_vid;
} ds_pvlan_community_key_t;

typedef struct
{
    ds_pvlan_community_key_t key;
    uint8                group_id;
    cdb_reference_list_t ports;               /* list of community ports */
} ds_pvlan_community_t;

#endif /* !__DS_PVLAN_COMMUNITY_DEFINE_H__ */

