
#ifndef __TBL_RIPNG_DEFINE_H__
#define __TBL_RIPNG_DEFINE_H__

/* TBL_RIPNG field defines */
typedef enum
{
    TBL_RIPNG_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIPNG_FLD_ALLOW_ECMP           = 1 ,  /* READ */
    TBL_RIPNG_FLD_TIMERS               = 2 ,  /* READ */
    TBL_RIPNG_FLD_DEFAULT_INFORMATION  = 3 ,  /* READ */
    TBL_RIPNG_FLD_REDISTRIBUTE         = 4 ,  /* READ */
    TBL_RIPNG_FLD_RED_METRIC           = 5 ,  /* READ */
    TBL_RIPNG_FLD_DEFAULT_METRIC       = 6 ,  /* READ */
    TBL_RIPNG_FLD_MAX                  = 7 
} tbl_ripng_field_id_t;

/* TBL_RIPNG defines */
typedef struct
{
    uint32               id;
} tbl_ripng_key_t;

typedef struct
{
    tbl_ripng_key_t      key;
    uint8                allow_ecmp;
    rip_timers_t         timers;
    uint32               default_information;
    uint32               redistribute;
    uint32               red_metric[RT_ROUTE_MAX];
    uint32               default_metric;
} tbl_ripng_t;

typedef struct
{
    ctclib_hash_t        *ripng_hash;
    ctclib_slist_t       *ripng_list;
} tbl_ripng_master_t;

#endif /* !__TBL_RIPNG_DEFINE_H__ */

