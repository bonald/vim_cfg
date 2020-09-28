
#ifndef __STBL_LAG_MEMPORT_DEFINE_H__
#define __STBL_LAG_MEMPORT_DEFINE_H__

/* STBL_LAG_MEMPORT field defines */
typedef enum
{
    STBL_LAG_MEMPORT_FLD_KEY                  = 0 ,  /* READ */
    STBL_LAG_MEMPORT_FLD_PORT_OID             = 1 ,  /* READ */
    STBL_LAG_MEMPORT_FLD_TID                  = 2 ,  /* READ */
    STBL_LAG_MEMPORT_FLD_MAX                  = 3 
} stbl_lag_memport_field_id_t;

/* STBL_LAG_MEMPORT defines */
typedef struct
{
    uint32               gport;
} stbl_lag_memport_key_t;

typedef struct
{
    stbl_lag_memport_key_t key;
    sai_object_id_t      port_oid;
    uint32               tid;
} stbl_lag_memport_t;

typedef struct
{
    ctclib_hash_t        *memport_hash;
} stbl_lag_memport_master_t;

#endif /* !__STBL_LAG_MEMPORT_DEFINE_H__ */

