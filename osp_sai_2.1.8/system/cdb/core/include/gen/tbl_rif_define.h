
#ifndef __TBL_RIF_DEFINE_H__
#define __TBL_RIF_DEFINE_H__

/* TBL_RIF field defines */
typedef enum
{
    TBL_RIF_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIF_FLD_RIF_ID               = 1 ,  /* RW */
    TBL_RIF_FLD_HOSTIF_ID            = 2 ,  /* RW */
    TBL_RIF_FLD_MAX                  = 3 
} tbl_rif_field_id_t;

/* TBL_RIF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_rif_key_t;

typedef struct
{
    tbl_rif_key_t        key;
    uint64               rif_id;
    uint64               hostif_id;
} tbl_rif_t;

typedef struct
{
    ctclib_hash_t        *rif_hash;
    ctclib_slist_t       *if_list;
} tbl_rif_master_t;

#endif /* !__TBL_RIF_DEFINE_H__ */

