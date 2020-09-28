
#ifndef __TBL_L2_PROTOCOL_DEFINE_H__
#define __TBL_L2_PROTOCOL_DEFINE_H__

/* TBL_L2_PROTOCOL field defines */
typedef enum
{
    TBL_L2_PROTOCOL_FLD_KEY                  = 0 ,  /* READ */
    TBL_L2_PROTOCOL_FLD_PROTOCOL_MAC         = 1 ,  /* RW */
    TBL_L2_PROTOCOL_FLD_GROUP_MAC            = 2 ,  /* RW */
    TBL_L2_PROTOCOL_FLD_IS_STANDARD_PRO      = 3 ,  /* RW */
    TBL_L2_PROTOCOL_FLD_PROTOCOL_INDEX       = 4 ,  /* RW */
    TBL_L2_PROTOCOL_FLD_MAX                  = 5 
} tbl_l2_protocol_field_id_t;

/* TBL_L2_PROTOCOL defines */
typedef struct
{
    char                 name[GLB_L2_PROTOCOL_NAME_SIZE+1];
} tbl_l2_protocol_key_t;

typedef struct
{
    tbl_l2_protocol_key_t key;
    mac_addr_t           protocol_mac;        /* Protocol mac address */
    mac_addr_t           group_mac;           /* Group mac adresss */
    uint32               is_standard_pro;     /* when is_standard_pro is 1, then is l2 protocol is standard protocol*/
    uint32               protocol_index;      /* l2 protocol index*/
} tbl_l2_protocol_t;

typedef struct
{
    ctclib_hash_t        *l2_protocol_hash;
    ctclib_slist_t       *l2_protocol_list;
} tbl_l2_protocol_master_t;

#endif /* !__TBL_L2_PROTOCOL_DEFINE_H__ */

