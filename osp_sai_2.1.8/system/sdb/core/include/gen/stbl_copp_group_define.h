
#ifndef __STBL_COPP_GROUP_DEFINE_H__
#define __STBL_COPP_GROUP_DEFINE_H__

/* STBL_COPP_GROUP field defines */
typedef enum
{
    STBL_COPP_GROUP_FLD_KEY                  = 0 ,  /* READ */
    STBL_COPP_GROUP_FLD_POLICE_OID           = 1 ,  /* READ */
    STBL_COPP_GROUP_FLD_POLICE_SSH_OID       = 2 ,  /* READ */
    STBL_COPP_GROUP_FLD_POLICE_TELNET_OID    = 3 ,  /* READ */
    STBL_COPP_GROUP_FLD_POLICE_MLAG_OID      = 4 ,  /* READ */
    STBL_COPP_GROUP_FLD_QUEUE_ID             = 5 ,  /* READ */
    STBL_COPP_GROUP_FLD_CTC_REASON_ID        = 6 ,  /* READ */
    STBL_COPP_GROUP_FLD_CHANNEL              = 7 ,  /* READ */
    STBL_COPP_GROUP_FLD_MAX                  = 8 
} stbl_copp_group_field_id_t;

/* STBL_COPP_GROUP defines */
typedef struct
{
    uint32               group_id;
} stbl_copp_group_key_t;

typedef struct
{
    stbl_copp_group_key_t key;
    sai_object_id_t      police_oid;
    sai_object_id_t      police_ssh_oid;
    sai_object_id_t      police_telnet_oid;
    sai_object_id_t      police_mlag_oid;
    uint32               queue_id;
    uint32               ctc_reason_id;
    uint32               channel;
} stbl_copp_group_t;

typedef struct
{
    stbl_copp_group_t    *array[GLB_COPP_GROUP_MAX];
} stbl_copp_group_master_t;

#endif /* !__STBL_COPP_GROUP_DEFINE_H__ */

