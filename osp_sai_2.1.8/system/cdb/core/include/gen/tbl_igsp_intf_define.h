
#ifndef __TBL_IGSP_INTF_DEFINE_H__
#define __TBL_IGSP_INTF_DEFINE_H__

#include "gen/ds_mrouter_port_define.h"
#include "gen/ds_query_define.h"
#include "gen/ds_join_group_define.h"

/* TBL_IGSP_INTF field defines */
typedef enum
{
    TBL_IGSP_INTF_FLD_KEY                  = 0 ,  /* READ */
    TBL_IGSP_INTF_FLD_ACL_NAME             = 1 ,  /* RW */
    TBL_IGSP_INTF_FLD_ENABLE               = 2 ,  /* RW */
    TBL_IGSP_INTF_FLD_FAST_LEAVE           = 3 ,  /* RW */
    TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN      = 4 ,  /* RW */
    TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT  = 5 ,  /* RW */
    TBL_IGSP_INTF_FLD_REPORT_SUPPRESS      = 6 ,  /* RW */
    TBL_IGSP_INTF_FLD_VERSION              = 7 ,  /* RW */
    TBL_IGSP_INTF_FLD_QUERIER_ENABLE       = 8 ,  /* RW */
    TBL_IGSP_INTF_FLD_QUERIER_OPERATE      = 9 ,  /* RW */
    TBL_IGSP_INTF_FLD_QUERY_INTERVAL       = 10,  /* RW */
    TBL_IGSP_INTF_FLD_ADMIN_OTHER_QUERY_INTERVAL = 11,  /* RW */
    TBL_IGSP_INTF_FLD_OTHER_QUERY_INTERVAL = 12,  /* RW */
    TBL_IGSP_INTF_FLD_QUERIER_MAX_RESPONSE_TIME = 13,  /* RW */
    TBL_IGSP_INTF_FLD_QUERIER_CONFIG_ADDRESS = 14,  /* RW */
    TBL_IGSP_INTF_FLD_QUERIER_OPER_ADDRESS = 15,  /* READ */
    TBL_IGSP_INTF_FLD_OTHER_QUERIER_ADDRESS = 16,  /* READ */
    TBL_IGSP_INTF_FLD_VLAN_IF_ADDRESS      = 17,  /* READ */
    TBL_IGSP_INTF_FLD_LMQI                 = 18,  /* RW */
    TBL_IGSP_INTF_FLD_LMQC                 = 19,  /* RW */
    TBL_IGSP_INTF_FLD_MAX_MEMBER_NUMBER    = 20,  /* RW */
    TBL_IGSP_INTF_FLD_CURR_GROUP_MEMBER    = 21,  /* RW */
    TBL_IGSP_INTF_FLD_GROUP_MEMBER_INTERVAL = 22,  /* RW */
    TBL_IGSP_INTF_FLD_ROBUSTNESS_VAR       = 23,  /* RW */
    TBL_IGSP_INTF_FLD_MROUTER_AGING_INTERVAL = 24,  /* RW */
    TBL_IGSP_INTF_FLD_STARTUP_QUERY_COUNT  = 25,  /* READ */
    TBL_IGSP_INTF_FLD_T_QUERIER            = 26,  /* READ */
    TBL_IGSP_INTF_FLD_T_OTHER_QUERIER      = 27,  /* READ */
    TBL_IGSP_INTF_FLD_MROUTER_LIST         = 28,  /* SUB */
    TBL_IGSP_INTF_FLD_QUERY_LIST           = 29,  /* SUB */
    TBL_IGSP_INTF_FLD_JOIN_LIST            = 30,  /* SUB */
    TBL_IGSP_INTF_FLD_MAX                  = 31
} tbl_igsp_intf_field_id_t;

/* TBL_IGSP_INTF defines */
typedef struct
{
    vid_t                vid;
} tbl_igsp_intf_key_t;

typedef struct
{
    tbl_igsp_intf_key_t  key;
    char                 acl_name[ACL_NAME_FULL_NAME_SIZE+1];
    uint8                enable;
    uint8                fast_leave;
    uint8                discard_unknown;
    uint8                discard_unknown_act;
    uint8                report_suppress;
    uint8                version;
    uint8                querier_enable;      /* querier - enable */
    uint8                querier_operate;     /* querier - operate enable */
    uint32               query_interval;      /* querier - query internal */
    uint32               admin_other_query_interval; /* querier - admin other query internal */
    uint32               other_query_interval; /* querier - other query internal */
    uint32               querier_max_response_time; /* querier - max-response-time */
    addr_ipv4_t          querier_config_address; /* querier - config address */
    addr_ipv4_t          querier_oper_address; /* querier - operate address */
    addr_ipv4_t          other_querier_address; /* querier - other address */
    addr_ipv4_t          vlan_if_address;     /* vlan interface's address */
    uint32               lmqi;                /* querier - last member query interval */
    uint32               lmqc;                /* querier - last member query count */
    uint32               max_member_number;   /* max group member count */
    uint32               curr_group_member;   /* current group member count */
    uint32               group_member_interval;
    uint32               robustness_var;
    uint32               mrouter_aging_interval;
    uint32               startup_query_count; /* current group member count */
    ctc_task_t           *t_querier;
    ctc_task_t           *t_other_querier;
    cdb_reference_list_t mrouter_list;        /* list of mrouter port */
    cdb_reference_list_t query_list;          /* list of query group */
    cdb_reference_list_t join_list;           /* list of host join group */
} tbl_igsp_intf_t;

typedef struct
{
    tbl_igsp_intf_t      *igsp_array[GLB_MAX_VLAN_ID];
} tbl_igsp_intf_master_t;

#endif /* !__TBL_IGSP_INTF_DEFINE_H__ */

