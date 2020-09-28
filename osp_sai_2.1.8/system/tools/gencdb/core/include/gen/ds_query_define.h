
#ifndef __DS_QUERY_DEFINE_H__
#define __DS_QUERY_DEFINE_H__

/* DS_QUERY field defines */
typedef enum
{
    DS_QUERY_FLD_KEY                  = 0 ,  /* READ */
    DS_QUERY_FLD_MAX_RESP_TIME        = 1 ,  /* READ */
    DS_QUERY_FLD_VID                  = 2 ,  /* READ */
    DS_QUERY_FLD_RESPONSE_TIMER       = 3 ,  /* READ */
    DS_QUERY_FLD_MAX                  = 4 
} ds_query_field_id_t;

/* DS_QUERY defines */
typedef struct
{
    addr_ipv4_t          group;
} ds_query_key_t;

typedef struct
{
    ds_query_key_t       key;
    uint32               max_resp_time;
    vid_t                vid;
    ctc_task_t           *response_timer;
} ds_query_t;

#endif /* !__DS_QUERY_DEFINE_H__ */

