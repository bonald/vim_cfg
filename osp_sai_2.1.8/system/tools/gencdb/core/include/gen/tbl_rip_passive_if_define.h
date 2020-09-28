
#ifndef __TBL_RIP_PASSIVE_IF_DEFINE_H__
#define __TBL_RIP_PASSIVE_IF_DEFINE_H__

/* TBL_RIP_PASSIVE_IF field defines */
typedef enum
{
    TBL_RIP_PASSIVE_IF_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIP_PASSIVE_IF_FLD_MAX                  = 1 
} tbl_rip_passive_if_field_id_t;

/* TBL_RIP_PASSIVE_IF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_rip_passive_if_key_t;

typedef struct
{
    tbl_rip_passive_if_key_t key;
} tbl_rip_passive_if_t;

typedef struct
{
    ctclib_hash_t        *rip_passive_if_hash;
    ctclib_slist_t       *rip_passive_if_list;
} tbl_rip_passive_if_master_t;

#endif /* !__TBL_RIP_PASSIVE_IF_DEFINE_H__ */

