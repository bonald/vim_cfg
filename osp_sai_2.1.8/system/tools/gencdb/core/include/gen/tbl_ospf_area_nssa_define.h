
#ifndef __TBL_OSPF_AREA_NSSA_DEFINE_H__
#define __TBL_OSPF_AREA_NSSA_DEFINE_H__

/* TBL_OSPF_AREA_NSSA field defines */
typedef enum
{
    TBL_OSPF_AREA_NSSA_FLD_KEY                  = 0 ,  /* READ */
    TBL_OSPF_AREA_NSSA_FLD_NSSA_ROLE            = 1 ,  /* RW */
    TBL_OSPF_AREA_NSSA_FLD_NO_SUMMARY           = 2 ,  /* RW */
    TBL_OSPF_AREA_NSSA_FLD_MAX                  = 3 
} tbl_ospf_area_nssa_field_id_t;

/* TBL_OSPF_AREA_NSSA defines */
typedef struct
{
    uint32               areaid;
} tbl_ospf_area_nssa_key_t;

typedef struct
{
    tbl_ospf_area_nssa_key_t key;
    uint32               nssa_role;           /* NSSA-ABR translate role */
    char                 no_summary;          /* no summary flag */
} tbl_ospf_area_nssa_t;

typedef struct
{
    ctclib_hash_t        *ospf_area_nssa_hash;
    ctclib_slist_t       *ospf_area_nssa_list;
} tbl_ospf_area_nssa_master_t;

#endif /* !__TBL_OSPF_AREA_NSSA_DEFINE_H__ */

