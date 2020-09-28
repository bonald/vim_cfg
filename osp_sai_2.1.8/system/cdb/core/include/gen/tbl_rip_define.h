
#ifndef __TBL_RIP_DEFINE_H__
#define __TBL_RIP_DEFINE_H__

/* TBL_RIP field defines */
typedef enum
{
    TBL_RIP_FLD_KEY                  = 0 ,  /* READ */
    TBL_RIP_FLD_VERSION              = 1 ,  /* READ */
    TBL_RIP_FLD_ALLOW_ECMP           = 2 ,  /* READ */
    TBL_RIP_FLD_TIMERS               = 3 ,  /* READ */
    TBL_RIP_FLD_DEFAULT_INFORMATION  = 4 ,  /* READ */
    TBL_RIP_FLD_REDISTRIBUTE         = 5 ,  /* READ */
    TBL_RIP_FLD_RED_METRIC           = 6 ,  /* READ */
    TBL_RIP_FLD_DEFAULT_METRIC       = 7 ,  /* READ */
    TBL_RIP_FLD_PASSIVE_IF_DEFAULT   = 8 ,  /* READ */
    TBL_RIP_FLD_MAX                  = 9 
} tbl_rip_field_id_t;

/* TBL_RIP defines */
typedef struct
{
    uint32               id;
} tbl_rip_key_t;

typedef struct
{
    tbl_rip_key_t        key;
    uint8                version;
    uint8                allow_ecmp;
    rip_timers_t         timers;
    uint32               default_information;
    uint32               redistribute;
    uint32               red_metric[RT_ROUTE_MAX];
    uint32               default_metric;
    uint8                passive_if_default;
} tbl_rip_t;

typedef struct
{
    ctclib_hash_t        *rip_hash;
    ctclib_slist_t       *rip_list;
} tbl_rip_master_t;

#endif /* !__TBL_RIP_DEFINE_H__ */

