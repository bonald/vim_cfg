
#ifndef __TBL_RIPNG_PASSIVE_IF_DEFINE_H__
#define __TBL_RIPNG_PASSIVE_IF_DEFINE_H__

/* TBL_RIPNG_PASSIVE_IF field defines */
typedef enum
{
    TBL_RIPNG_PASSIVE_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIPNG_PASSIVE_IF_FLD_MAX                  = 1 
} tbl_ripng_passive_if_field_id_t;

/* TBL_RIPNG_PASSIVE_IF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_ripng_passive_if_key_t;

typedef struct
{
    tbl_ripng_passive_if_key_t key;
} tbl_ripng_passive_if_t;

typedef struct
{
    ctclib_hash_t        *ripng_passive_if_hash;
    ctclib_slist_t       *ripng_passive_if_list;
} tbl_ripng_passive_if_master_t;

#endif /* !__TBL_RIPNG_PASSIVE_IF_DEFINE_H__ */

