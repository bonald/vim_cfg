
#ifndef __DS_MROUTER_PORT_DEFINE_H__
#define __DS_MROUTER_PORT_DEFINE_H__

/* DS_MROUTER_PORT field defines */
typedef enum
{
    DS_MROUTER_PORT_FLD_KEY                  = 0 ,  /* READ */
    DS_MROUTER_PORT_FLD_TYPE                 = 1 ,  /* READ */
    DS_MROUTER_PORT_FLD_UPTIME               = 2 ,  /* READ */
    DS_MROUTER_PORT_FLD_EXPIRE_TIMER         = 3 ,  /* READ */
    DS_MROUTER_PORT_FLD_MAX                  = 4 
} ds_mrouter_port_field_id_t;

/* DS_MROUTER_PORT defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} ds_mrouter_port_key_t;

typedef struct
{
    ds_mrouter_port_key_t key;
    uint32               type;                /* igmp_mrouter_type_t */
    sal_time_t           uptime;
    ctc_task_t           *expire_timer;
    void                 *igsp_if;            /* refer to tbl_igsp_intf_t */
} ds_mrouter_port_t;

#endif /* !__DS_MROUTER_PORT_DEFINE_H__ */

