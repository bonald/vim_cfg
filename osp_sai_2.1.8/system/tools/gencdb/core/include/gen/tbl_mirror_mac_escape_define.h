
#ifndef __TBL_MIRROR_MAC_ESCAPE_DEFINE_H__
#define __TBL_MIRROR_MAC_ESCAPE_DEFINE_H__

/* TBL_MIRROR_MAC_ESCAPE field defines */
typedef enum
{
    TBL_MIRROR_MAC_ESCAPE_FLD_KEY                  = 0 ,  /* READ */
    TBL_MIRROR_MAC_ESCAPE_FLD_MAX                  = 1 
} tbl_mirror_mac_escape_field_id_t;

/* TBL_MIRROR_MAC_ESCAPE defines */
typedef mirror_mac_escape_key_t tbl_mirror_mac_escape_key_t;

typedef struct
{
    tbl_mirror_mac_escape_key_t key;
} tbl_mirror_mac_escape_t;

typedef struct
{
    ctclib_slist_t       *mirror_mac_escape_list;
} tbl_mirror_mac_escape_master_t;

#endif /* !__TBL_MIRROR_MAC_ESCAPE_DEFINE_H__ */

