
#ifndef __TBL_NS_ROUTE_DEFINE_H__
#define __TBL_NS_ROUTE_DEFINE_H__

/* TBL_NS_ROUTE field defines */
typedef enum
{
    TBL_NS_ROUTE_FLD_KEY                  = 0 ,  /* READ */
    TBL_NS_ROUTE_FLD_FLAG                 = 1 ,  /* RW */
    TBL_NS_ROUTE_FLD_IS_INBAND            = 2 ,  /* RW */
    TBL_NS_ROUTE_FLD_IFNAME               = 3 ,  /* RW */
    TBL_NS_ROUTE_FLD_CNT                  = 4 ,  /* RW */
    TBL_NS_ROUTE_FLD_MAX                  = 5 
} tbl_ns_route_field_id_t;

/* TBL_NS_ROUTE defines */
typedef ns_route_key_t tbl_ns_route_key_t;

typedef struct
{
    tbl_ns_route_key_t   key;
    uint32               flag;                /* reference to ns_route_flag_t */
    uint32               is_inband;
    char                 ifname[IFNAME_SIZE]; /* "default" or interface */
    int32                cnt;                 /* used cnt */
} tbl_ns_route_t;

typedef struct
{
    ctclib_hash_t        *ns_route_hash;
    ctclib_slist_t       *ns_route_list;
} tbl_ns_route_master_t;

#endif /* !__TBL_NS_ROUTE_DEFINE_H__ */

