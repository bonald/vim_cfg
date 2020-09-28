
#ifndef __TBL_STATIC_RT_CNT_DEFINE_H__
#define __TBL_STATIC_RT_CNT_DEFINE_H__

/* TBL_STATIC_RT_CNT field defines */
typedef enum
{
    TBL_STATIC_RT_CNT_FLD_KEY                  = 0 ,  /* READ */
    TBL_STATIC_RT_CNT_FLD_REFER_CNT            = 1 ,  /* READ */
    TBL_STATIC_RT_CNT_FLD_HAS_BLACKHOLE        = 2 ,  /* READ */
    TBL_STATIC_RT_CNT_FLD_NH_GROUP             = 3 ,  /* RW */
    TBL_STATIC_RT_CNT_FLD_MAX                  = 4 
} tbl_static_rt_cnt_field_id_t;

/* TBL_STATIC_RT_CNT defines */
typedef route_node_key_with_ad_t tbl_static_rt_cnt_key_t;

typedef struct
{
    tbl_static_rt_cnt_key_t key;
    uint32               refer_cnt;
    uint32               has_blackhole;
    nexthop_ecmp_routed_t nh_group;
} tbl_static_rt_cnt_t;

typedef struct
{
    ctclib_hash_t        *rt_cnt_hash;
} tbl_static_rt_cnt_master_t;

#endif /* !__TBL_STATIC_RT_CNT_DEFINE_H__ */

