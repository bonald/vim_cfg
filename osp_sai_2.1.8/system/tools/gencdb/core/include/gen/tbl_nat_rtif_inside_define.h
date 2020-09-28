
#ifndef __TBL_NAT_RTIF_INSIDE_DEFINE_H__
#define __TBL_NAT_RTIF_INSIDE_DEFINE_H__

/* TBL_NAT_RTIF_INSIDE field defines */
typedef enum
{
    TBL_NAT_RTIF_INSIDE_FLD_KEY                  = 0 ,  /* READ */
    TBL_NAT_RTIF_INSIDE_FLD_MAX                  = 1 
} tbl_nat_rtif_inside_field_id_t;

/* TBL_NAT_RTIF_INSIDE defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_nat_rtif_inside_key_t;

typedef struct
{
    tbl_nat_rtif_inside_key_t key;
} tbl_nat_rtif_inside_t;

typedef struct
{
    ctclib_hash_t        *nat_rtif_inside_hash;
    ctclib_slist_t       *nat_rtif_inside_list;
} tbl_nat_rtif_inside_master_t;

#endif /* !__TBL_NAT_RTIF_INSIDE_DEFINE_H__ */

