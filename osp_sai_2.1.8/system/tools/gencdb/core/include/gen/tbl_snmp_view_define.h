
#ifndef __TBL_SNMP_VIEW_DEFINE_H__
#define __TBL_SNMP_VIEW_DEFINE_H__

/* TBL_SNMP_VIEW field defines */
typedef enum
{
    TBL_SNMP_VIEW_FLD_KEY                  = 0 ,  /* RW */
    TBL_SNMP_VIEW_FLD_TYPE                 = 1 ,  /* RW */
    TBL_SNMP_VIEW_FLD_MASK                 = 2 ,  /* RW */
    TBL_SNMP_VIEW_FLD_ACCESS_CNT           = 3 ,  /* RW */
    TBL_SNMP_VIEW_FLD_REFCNT               = 4 ,  /* READ */
    TBL_SNMP_VIEW_FLD_MAX                  = 5 
} tbl_snmp_view_field_id_t;

/* TBL_SNMP_VIEW defines */
typedef struct
{
    snmp_view_key_t      view;
} tbl_snmp_view_key_t;

typedef struct
{
    tbl_snmp_view_key_t  key;
    uint8                type;                /* include or exclude */
    char                 mask[SNMP_MASK_SIZE]; /* subtree mask hex string */
    uint8                access_cnt;          /* access view reference cnt */
    uint8                refcnt;              /* reference count */
} tbl_snmp_view_t;

typedef struct
{
    ctclib_slist_t       *snmp_view_list;
} tbl_snmp_view_master_t;

#endif /* !__TBL_SNMP_VIEW_DEFINE_H__ */

