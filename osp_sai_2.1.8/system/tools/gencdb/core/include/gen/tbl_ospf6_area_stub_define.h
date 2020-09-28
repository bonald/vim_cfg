
#ifndef __TBL_OSPF6_AREA_STUB_DEFINE_H__
#define __TBL_OSPF6_AREA_STUB_DEFINE_H__

/* TBL_OSPF6_AREA_STUB field defines */
typedef enum
{
    TBL_OSPF6_AREA_STUB_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF6_AREA_STUB_FLD_NO_SUMMARY           = 1 ,  /* RW */
    TBL_OSPF6_AREA_STUB_FLD_MAX                  = 2 
} tbl_ospf6_area_stub_field_id_t;

/* TBL_OSPF6_AREA_STUB defines */
typedef struct
{
    uint32               areaid;
} tbl_ospf6_area_stub_key_t;

typedef struct
{
    tbl_ospf6_area_stub_key_t key;
    char                 no_summary;          /* no summary flag */
} tbl_ospf6_area_stub_t;

typedef struct
{
    ctclib_hash_t        *ospf6_area_stub_hash;
    ctclib_slist_t       *ospf6_area_stub_list;
} tbl_ospf6_area_stub_master_t;

#endif /* !__TBL_OSPF6_AREA_STUB_DEFINE_H__ */

