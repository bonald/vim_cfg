
#ifndef __STBL_L2MC_DEFINE_H__
#define __STBL_L2MC_DEFINE_H__

/* STBL_L2MC field defines */
typedef enum
{
    STBL_L2MC_FLD_KEY                  = 0 ,  /* READ */
    STBL_L2MC_FLD_GROUP_ID             = 1 ,  /* READ */
    STBL_L2MC_FLD_MAX                  = 2 
} stbl_l2mc_field_id_t;

/* STBL_L2MC defines */
typedef sai_l2mc_entry_t stbl_l2mc_key_t;

typedef struct
{
    stbl_l2mc_key_t      key;
    uint32               group_id;            /* L2 mcast group ID */
} stbl_l2mc_t;

typedef struct
{
    ctclib_hash_t        *l2mc_hash;
} stbl_l2mc_master_t;

#endif /* !__STBL_L2MC_DEFINE_H__ */

