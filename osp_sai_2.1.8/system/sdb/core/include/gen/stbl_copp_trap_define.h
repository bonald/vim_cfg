
#ifndef __STBL_COPP_TRAP_DEFINE_H__
#define __STBL_COPP_TRAP_DEFINE_H__

/* STBL_COPP_TRAP field defines */
typedef enum
{
    STBL_COPP_TRAP_FLD_KEY                  = 0 ,  /* READ */
    STBL_COPP_TRAP_FLD_GROUP_OID            = 1 ,  /* READ */
    STBL_COPP_TRAP_FLD_CTC_REASON_ID        = 2 ,  /* READ */
    STBL_COPP_TRAP_FLD_CHANNEL              = 3 ,  /* READ */
    STBL_COPP_TRAP_FLD_MAX                  = 4 
} stbl_copp_trap_field_id_t;

/* STBL_COPP_TRAP defines */
typedef struct
{
    uint32               trap_id;
} stbl_copp_trap_key_t;

typedef struct
{
    stbl_copp_trap_key_t key;
    sai_object_id_t      group_oid;
    uint32               ctc_reason_id;
    uint32               channel;
} stbl_copp_trap_t;

typedef struct
{
    stbl_copp_trap_t     *array[GLB_COPP_TRAP_MAX];
} stbl_copp_trap_master_t;

#endif /* !__STBL_COPP_TRAP_DEFINE_H__ */

