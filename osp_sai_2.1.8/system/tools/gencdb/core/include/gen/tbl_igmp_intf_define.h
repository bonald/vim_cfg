
#ifndef __TBL_IGMP_INTF_DEFINE_H__
#define __TBL_IGMP_INTF_DEFINE_H__

/* TBL_IGMP_INTF field defines */
typedef enum
{
    TBL_IGMP_INTF_FLD_KEY                  = 0 ,  /* READ */
    TBL_IGMP_INTF_FLD_QUERIER_TIMEOUT      = 1 ,  /* RW */
    TBL_IGMP_INTF_FLD_QUERY_INTERVAL       = 2 ,  /* RW */
    TBL_IGMP_INTF_FLD_ROBUSTNESS_VARIABLE  = 3 ,  /* RW */
    TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL_INDEX = 4 ,  /* RW */
    TBL_IGMP_INTF_FLD_LAYER3_PROTOCOL      = 5 ,  /* RW */
    TBL_IGMP_INTF_FLD_ENTRY_VALID          = 6 ,  /* RW */
    TBL_IGMP_INTF_FLD_ACTION_INDEX         = 7 ,  /* RW */
    TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_COUNT = 8 ,  /* RW */
    TBL_IGMP_INTF_FLD_LAST_MEMBER_QUERY_INTERVAL = 9 ,  /* RW */
    TBL_IGMP_INTF_FLD_GLOBAL_LIMIT_NUMBER  = 10,  /* RW */
    TBL_IGMP_INTF_FLD_MAX                  = 11
} tbl_igmp_intf_field_id_t;

/* TBL_IGMP_INTF defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_igmp_intf_key_t;

typedef struct
{
    tbl_igmp_intf_key_t  key;
    uint32               querier_timeout;     /* querier - enable */
    uint32               query_interval;      /* querier - query internal */
    uint32               robustness_variable;
    uint32               layer3_protocol_index; /*igmp on parameter*/
    uint32               layer3_protocol;     /*igmp on parameter*/
    uint32               entry_valid;         /*igmp on parameter*/
    uint32               action_index;        /*igmp on parameter*/
    uint32               last_member_query_count;
    uint32               last_member_query_interval;
    uint32               global_limit_number; /*global groups limit number,1-8192*/
} tbl_igmp_intf_t;

typedef struct
{
    ctclib_hash_t        *igmp_intf_hash;
    ctclib_slist_t       *igmp_intf_list;
} tbl_igmp_intf_master_t;

#endif /* !__TBL_IGMP_INTF_DEFINE_H__ */

