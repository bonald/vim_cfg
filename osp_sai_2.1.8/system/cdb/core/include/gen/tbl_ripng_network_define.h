
#ifndef __TBL_RIPNG_NETWORK_DEFINE_H__
#define __TBL_RIPNG_NETWORK_DEFINE_H__

/* TBL_RIPNG_NETWORK field defines */
typedef enum
{
    TBL_RIPNG_NETWORK_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIPNG_NETWORK_FLD_MAX                  = 1 
} tbl_ripng_network_field_id_t;

/* TBL_RIPNG_NETWORK defines */
typedef prefix_t tbl_ripng_network_key_t;

typedef struct
{
    tbl_ripng_network_key_t key;
} tbl_ripng_network_t;

typedef struct
{
    ctclib_hash_t        *ripng_network_hash;
    ctclib_slist_t       *ripng_network_list;
} tbl_ripng_network_master_t;

#endif /* !__TBL_RIPNG_NETWORK_DEFINE_H__ */

