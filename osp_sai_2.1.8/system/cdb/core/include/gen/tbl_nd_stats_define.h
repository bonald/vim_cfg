
#ifndef __TBL_ND_STATS_DEFINE_H__
#define __TBL_ND_STATS_DEFINE_H__

/* TBL_ND_STATS field defines */
typedef enum
{
    TBL_ND_STATS_FLD_RCV                  = 0 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_ERROR            = 1 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_TOOSHORT         = 2 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_INVALID          = 3 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_INCONSISTENT     = 4 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_RS               = 5 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_RA               = 6 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_NS               = 7 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_NA               = 8 ,  /* RW */
    TBL_ND_STATS_FLD_RCV_REDIRECT         = 9 ,  /* RW */
    TBL_ND_STATS_FLD_OVERFLOW_FAILED      = 10,  /* RW */
    TBL_ND_STATS_FLD_RCV_MISS             = 11,  /* RW */
    TBL_ND_STATS_FLD_RCV_MISS_CONFLICT    = 12,  /* RW */
    TBL_ND_STATS_FLD_RCV_STATIC_CONFLICT  = 13,  /* RW */
    TBL_ND_STATS_FLD_ALLOCS               = 14,  /* RW */
    TBL_ND_STATS_FLD_DESTROYS             = 15,  /* RW */
    TBL_ND_STATS_FLD_ERROR                = 16,  /* RW */
    TBL_ND_STATS_FLD_INTF_INVALID         = 17,  /* RW */
    TBL_ND_STATS_FLD_RES_FAILED           = 18,  /* RW */
    TBL_ND_STATS_FLD_RCV_PROBES_MCAST     = 19,  /* RW */
    TBL_ND_STATS_FLD_RCV_PROBES_UCAST     = 20,  /* RW */
    TBL_ND_STATS_FLD_SND_SOLICITED_NA     = 21,  /* RW */
    TBL_ND_STATS_FLD_SND_UNSOLICITED_NA   = 22,  /* RW */
    TBL_ND_STATS_FLD_SND_DAD_NS           = 23,  /* RW */
    TBL_ND_STATS_FLD_MAX                  = 24
} tbl_nd_stats_field_id_t;

/* TBL_ND_STATS defines */
typedef struct
{
    uint64               rcv;                 /* number of received packets */
    uint64               rcv_error;           /* number of received error packets */
    uint64               rcv_tooshort;        /* number of received too short packets */
    uint64               rcv_invalid;         /* number of received invalid packets */
    uint64               rcv_inconsistent;    /* number of received inconsistent packets */
    uint64               rcv_rs;              /* number of received rs packets */
    uint64               rcv_ra;              /* number of received ra packets */
    uint64               rcv_ns;              /* number of received ns packets */
    uint64               rcv_na;              /* number of received na packets */
    uint64               rcv_redirect;        /* number of received redirect packets */
    uint64               overflow_failed;     /* number of received packets discard when overflow */
    uint64               rcv_miss;            /* number of kernel trigger nd learning */
    uint64               rcv_miss_conflict;   /* number of no matched ifc when kernel trigger nd learning */
    uint64               rcv_static_conflict; /* number of static nd entry exist when kernel trigger nd learning */
    uint64               allocs;              /* number of allocated neighs */
    uint64               destroys;            /* number of destroyed neighs */
    uint64               error;               /* number of nd entry state error */
    uint64               intf_invalid;        /* number of invalid interface from kernel */
    uint64               res_failed;          /* number of failed resolutions */
    uint64               rcv_probes_mcast;    /* number of received mcast ipv6 */
    uint64               rcv_probes_ucast;    /* number of received ucast ipv6 */
    uint64               snd_solicited_na;    /* number of solicited NA */
    uint64               snd_unsolicited_na;  /* number of unsolicited NA */
    uint64               snd_dad_ns;          /* number of DAD NS */
} tbl_nd_stats_t;

#endif /* !__TBL_ND_STATS_DEFINE_H__ */

