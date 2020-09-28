
#ifndef __TBL_TAP_FIVE_TUPLE_DEFINE_H__
#define __TBL_TAP_FIVE_TUPLE_DEFINE_H__

/* TBL_TAP_FIVE_TUPLE field defines */
typedef enum
{
    TBL_TAP_FIVE_TUPLE_FLD_KEY                  = 0 ,  /* READ */
    TBL_TAP_FIVE_TUPLE_FLD_ACL_ENTRY_ID         = 1 ,  /* RW */
    TBL_TAP_FIVE_TUPLE_FLD_STATS_ID             = 2 ,  /* RW */
    TBL_TAP_FIVE_TUPLE_FLD_PACKET_COUNT         = 3 ,  /* RW */
    TBL_TAP_FIVE_TUPLE_FLD_BYTE_COUNT           = 4 ,  /* RW */
    TBL_TAP_FIVE_TUPLE_FLD_MAX                  = 5 
} tbl_tap_five_tuple_field_id_t;

/* TBL_TAP_FIVE_TUPLE defines */
typedef tap_five_tuple_key_t tbl_tap_five_tuple_key_t;

typedef struct
{
    tbl_tap_five_tuple_key_t key;
    uint32               acl_entry_id;
    uint32               stats_id;
    uint64               packet_count;
    uint64               byte_count;
} tbl_tap_five_tuple_t;

typedef struct
{
    ctclib_hash_t        *tap_five_tuple_hash;
    ctclib_slist_t       *tap_five_tuple_list;
} tbl_tap_five_tuple_master_t;

#endif /* !__TBL_TAP_FIVE_TUPLE_DEFINE_H__ */

