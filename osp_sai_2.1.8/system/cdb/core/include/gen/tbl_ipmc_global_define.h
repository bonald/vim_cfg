
#ifndef __TBL_IPMC_GLOBAL_DEFINE_H__
#define __TBL_IPMC_GLOBAL_DEFINE_H__

/* TBL_IPMC_GLOBAL field defines */
typedef enum
{
    TBL_IPMC_GLOBAL_FLD_MCAST_ROUTE_EN       = 0 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_REGISTER_SUPPRESS_TIME = 1 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_PIM_PACKET_PROCESS   = 2 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_KEEP_ALIVE_TIME      = 3 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_RP_KEEP_ALIVE_TIME   = 4 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_JOIN_PRUNE_INTERVAL  = 5 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_SWITCHOVER_MODE      = 6 ,  /* RW */
    TBL_IPMC_GLOBAL_FLD_MAX                  = 7 
} tbl_ipmc_global_field_id_t;

/* TBL_IPMC_GLOBAL defines */
typedef struct
{
    uint32               mcast_route_en;
    uint32               register_suppress_time;
    uint32               pim_packet_process;
    uint32               keep_alive_time;
    uint32               rp_keep_alive_time;
    uint32               join_prune_interval;
    uint32               switchover_mode;     /* IMMEDIATE or INFINITY */
} tbl_ipmc_global_t;

#endif /* !__TBL_IPMC_GLOBAL_DEFINE_H__ */

