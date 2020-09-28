
#ifndef __DS_CONNECTED_V6_DEFINE_H__
#define __DS_CONNECTED_V6_DEFINE_H__

/* DS_CONNECTED_V6 field defines */
typedef enum
{
    DS_CONNECTED_V6_FLD_KEY                  = 0 ,  /* READ */
    DS_CONNECTED_V6_FLD_DESTINATION          = 1 ,  /* RW */
    DS_CONNECTED_V6_FLD_FLAGS_SECONDARY      = 2 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_ANYCAST        = 3 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_VIRTUAL        = 4 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_DUPLICATE      = 5 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL      = 6 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_LINKLOCAL   = 7 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_AUTO_LINKLOCAL = 8 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_GLOBAL_EUI64 = 9 ,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_DAD_IN_PROGRESS = 10,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_NA_IN_PROGRESS = 11,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_CONFIGURED  = 12,  /* READ */
    DS_CONNECTED_V6_FLD_FLAGS_V6_REAL        = 13,  /* READ */
    DS_CONNECTED_V6_FLD_INSTALLED            = 14,  /* READ */
    DS_CONNECTED_V6_FLD_IFNAME               = 15,  /* RW */
    DS_CONNECTED_V6_FLD_IFINDEX              = 16,  /* RW */
    DS_CONNECTED_V6_FLD_T_TIMER              = 17,  /* READ */
    DS_CONNECTED_V6_FLD_DAD_NS_SEND          = 18,  /* RW */
    DS_CONNECTED_V6_FLD_UNSOLICITED_NA       = 19,  /* RW */
    DS_CONNECTED_V6_FLD_MAX                  = 20
} ds_connected_v6_field_id_t;

/* DS_CONNECTED_V6 defines */
typedef struct
{
    prefix_t             address;
} ds_connected_v6_key_t;

typedef struct
{
    ds_connected_v6_key_t key;
    prefix_t             destination;
    uint32               flags;
    uint8                installed;
    char                 ifname[IFNAME_SIZE];
    uint32               ifindex;
    ctc_task_t           *t_timer;
    uint16               dad_ns_send;
    uint16               unsolicited_na;
} ds_connected_v6_t;

#endif /* !__DS_CONNECTED_V6_DEFINE_H__ */

