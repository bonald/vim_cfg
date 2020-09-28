
#ifndef __TBL_OSPF6_AREA_RANGE_DEFINE_H__
#define __TBL_OSPF6_AREA_RANGE_DEFINE_H__

/* TBL_OSPF6_AREA_RANGE field defines */
typedef enum
{
    TBL_OSPF6_AREA_RANGE_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF6_AREA_RANGE_FLD_ADVERTISE            = 1 ,  /* READ */
    TBL_OSPF6_AREA_RANGE_FLD_MAX                  = 2 
} tbl_ospf6_area_range_field_id_t;

/* TBL_OSPF6_AREA_RANGE defines */
typedef ospf6_area_range_key_t tbl_ospf6_area_range_key_t;

typedef struct
{
    tbl_ospf6_area_range_key_t key;
    uint32               advertise;
} tbl_ospf6_area_range_t;

typedef struct
{
    ctclib_hash_t        *ospf6_area_range_hash;
    ctclib_slist_t       *ospf6_area_range_list;
} tbl_ospf6_area_range_master_t;

#endif /* !__TBL_OSPF6_AREA_RANGE_DEFINE_H__ */

