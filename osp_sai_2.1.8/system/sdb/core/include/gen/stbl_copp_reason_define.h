
#ifndef __STBL_COPP_REASON_DEFINE_H__
#define __STBL_COPP_REASON_DEFINE_H__

/* STBL_COPP_REASON field defines */
typedef enum
{
    STBL_COPP_REASON_FLD_KEY                  = 0 ,  /* READ */
    STBL_COPP_REASON_FLD_CHANNEL              = 1 ,  /* READ */
    STBL_COPP_REASON_FLD_TRAP_ID              = 2 ,  /* READ */
    STBL_COPP_REASON_FLD_MAX                  = 3 
} stbl_copp_reason_field_id_t;

/* STBL_COPP_REASON defines */
typedef struct
{
    uint32               reason_id;
} stbl_copp_reason_key_t;

typedef struct
{
    stbl_copp_reason_key_t key;
    uint32               channel;
    uint32               trap_id;
} stbl_copp_reason_t;

typedef struct
{
    stbl_copp_reason_t   *array[GLB_COPP_REASON_MAX];
} stbl_copp_reason_master_t;

#endif /* !__STBL_COPP_REASON_DEFINE_H__ */

