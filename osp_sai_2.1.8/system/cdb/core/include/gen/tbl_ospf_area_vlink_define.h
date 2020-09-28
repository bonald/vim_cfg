
#ifndef __TBL_OSPF_AREA_VLINK_DEFINE_H__
#define __TBL_OSPF_AREA_VLINK_DEFINE_H__

/* TBL_OSPF_AREA_VLINK field defines */
typedef enum
{
    TBL_OSPF_AREA_VLINK_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF_AREA_VLINK_FLD_AUTH_FLAG            = 1 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_AUTH_TYPE            = 2 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_AUTH_KEY             = 3 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_MD5_ID               = 4 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_MD5_KEY              = 5 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_HELLO_INTERVAL       = 6 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_DEAD_INTERVAL        = 7 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_RETRANSMIT_INTERVAL  = 8 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_TRANSMIT_DELAY       = 9 ,  /* RW */
    TBL_OSPF_AREA_VLINK_FLD_MAX                  = 10
} tbl_ospf_area_vlink_field_id_t;

/* TBL_OSPF_AREA_VLINK defines */
typedef ospf_area_vlink_key_t tbl_ospf_area_vlink_key_t;

typedef struct
{
    tbl_ospf_area_vlink_key_t key;
    char                 auth_flag;           /* authentication flag */
    uint32               auth_type;           /* Authehntication type */
    char                 auth_key[10];        /* simple password */
    uint32               md5_id;              /* Cryptographic key ID */
    char                 md5_key[20];         /* MD5 authentication key */
    uint32               hello_interval;
    uint32               dead_interval;
    uint32               retransmit_interval;
    uint32               transmit_delay;
} tbl_ospf_area_vlink_t;

typedef struct
{
    ctclib_hash_t        *ospf_area_vlink_hash;
    ctclib_slist_t       *ospf_area_vlink_list;
} tbl_ospf_area_vlink_master_t;

#endif /* !__TBL_OSPF_AREA_VLINK_DEFINE_H__ */

