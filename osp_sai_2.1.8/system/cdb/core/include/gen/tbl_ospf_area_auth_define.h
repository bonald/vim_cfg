
#ifndef __TBL_OSPF_AREA_AUTH_DEFINE_H__
#define __TBL_OSPF_AREA_AUTH_DEFINE_H__

/* TBL_OSPF_AREA_AUTH field defines */
typedef enum
{
    TBL_OSPF_AREA_AUTH_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF_AREA_AUTH_FLD_AUTH_TYPE            = 1 ,  /* READ */
    TBL_OSPF_AREA_AUTH_FLD_MAX                  = 2 
} tbl_ospf_area_auth_field_id_t;

/* TBL_OSPF_AREA_AUTH defines */
typedef struct
{
    uint32               areaid;
} tbl_ospf_area_auth_key_t;

typedef struct
{
    tbl_ospf_area_auth_key_t key;
    uint32               auth_type;
} tbl_ospf_area_auth_t;

typedef struct
{
    ctclib_hash_t        *ospf_area_auth_hash;
    ctclib_slist_t       *ospf_area_auth_list;
} tbl_ospf_area_auth_master_t;

#endif /* !__TBL_OSPF_AREA_AUTH_DEFINE_H__ */

