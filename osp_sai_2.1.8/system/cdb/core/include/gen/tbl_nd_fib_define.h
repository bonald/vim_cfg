
#ifndef __TBL_ND_FIB_DEFINE_H__
#define __TBL_ND_FIB_DEFINE_H__

/* TBL_ND_FIB field defines */
typedef enum
{
    TBL_ND_FIB_FLD_KEY                  = 0 ,  /* READ */
    TBL_ND_FIB_FLD_IFINDEX              = 1 ,  /* RW */
    TBL_ND_FIB_FLD_IFNAME               = 2 ,  /* RW */
    TBL_ND_FIB_FLD_MAC_ADDR             = 3 ,  /* READ */
    TBL_ND_FIB_FLD_STATE                = 4 ,  /* RW */
    TBL_ND_FIB_FLD_PERMANENT            = 5 ,  /* RW */
    TBL_ND_FIB_FLD_DYNAMIC              = 6 ,  /* RW */
    TBL_ND_FIB_FLD_ACTIVE               = 7 ,  /* RW */
    TBL_ND_FIB_FLD_ROUTER               = 8 ,  /* RW */
    TBL_ND_FIB_FLD_FDB_AGE              = 9 ,  /* RW */
    TBL_ND_FIB_FLD_UPTIME               = 10,  /* READ */
    TBL_ND_FIB_FLD_PROBES               = 11,  /* RW */
    TBL_ND_FIB_FLD_T_TIMER              = 12,  /* READ */
    TBL_ND_FIB_FLD_IPACKETS             = 13,  /* RW */
    TBL_ND_FIB_FLD_OPACKETS             = 14,  /* RW */
    TBL_ND_FIB_FLD_ROUTE_REF            = 15,  /* RW */
    TBL_ND_FIB_FLD_IS_STATIC_ROUTE_DUP  = 16,  /* READ */
    TBL_ND_FIB_FLD_STAIC_ROUTE_DUP_IPV6_NH = 17,  /* READ */
    TBL_ND_FIB_FLD_MAX                  = 18
} tbl_nd_fib_field_id_t;

/* TBL_ND_FIB defines */
typedef nd_prefix_key_t tbl_nd_fib_key_t;

typedef struct
{
    tbl_nd_fib_key_t     key;
    uint32               ifindex;             /* interface index */
    char                 ifname[IFNAME_SIZE]; /* interface name */
    mac_addr_t           mac_addr;            /* nd entry mac address */
    uint32               state;               /* nd entry state */
    uint32               flags;               /* nd entry flags */
    sal_time_t           uptime;              /* time stamp as to when the entry was last created */
    uint32               probes;              /* numbers have probed */
    ctc_task_t           *t_timer;            /* nd entry timer */
    uint32               ipackets;            /* the number of incoming ND packets */
    uint32               opackets;            /* the number of outgoing ND packets */
    uint32               route_ref;           /* number of active route relation to this neigh */
    uint32               is_static_route_dup;
    addr_ipv6_t          staic_route_dup_ipv6_nh;
} tbl_nd_fib_t;

typedef struct
{
    ctclib_hash_t        *neighbor_hash;
    ctclib_slist_t       *neighbor_list;
} tbl_nd_fib_master_t;

#endif /* !__TBL_ND_FIB_DEFINE_H__ */

