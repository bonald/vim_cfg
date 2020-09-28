
#ifndef __TBL_MACFILTER_DEFINE_H__
#define __TBL_MACFILTER_DEFINE_H__

/* TBL_MACFILTER field defines */
typedef enum
{
    TBL_MACFILTER_FLD_KEY                  = 0 ,  /* READ */
    TBL_MACFILTER_FLD_MAX                  = 1 
} tbl_macfilter_field_id_t;

/* TBL_MACFILTER defines */
typedef struct
{
    mac_addr_t           mac;
} tbl_macfilter_key_t;

typedef struct
{
    tbl_macfilter_key_t  key;
} tbl_macfilter_t;

typedef struct
{
    ctclib_slist_t       *macflt_list;
} tbl_macfilter_master_t;

#endif /* !__TBL_MACFILTER_DEFINE_H__ */

