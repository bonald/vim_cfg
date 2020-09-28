
#ifndef __TBL_VRRP_GLOBAL_DEFINE_H__
#define __TBL_VRRP_GLOBAL_DEFINE_H__

/* TBL_VRRP_GLOBAL field defines */
typedef enum
{
    TBL_VRRP_GLOBAL_FLD_T_VRRP_TIMER         = 0 ,  /* READ */
    TBL_VRRP_GLOBAL_FLD_T_VRRP_MSEC_TIMER    = 1 ,  /* READ */
    TBL_VRRP_GLOBAL_FLD_VRRP_VERSION         = 2 ,  /* RW */
    TBL_VRRP_GLOBAL_FLD_VRRP_SESSION_COUNT   = 3 ,  /* RW */
    TBL_VRRP_GLOBAL_FLD_VRRP_MSEC_SESSION_COUNT = 4 ,  /* RW */
    TBL_VRRP_GLOBAL_FLD_BUF                  = 5 ,  /* RW */
    TBL_VRRP_GLOBAL_FLD_PKT_LEN              = 6 ,  /* RW */
    TBL_VRRP_GLOBAL_FLD_MAX                  = 7 
} tbl_vrrp_global_field_id_t;

/* TBL_VRRP_GLOBAL defines */
typedef struct
{
    ctc_task_t           *t_vrrp_timer;       /* VRRP timer */
    ctc_task_t           *t_vrrp_msec_timer;  /* VRRP timer for 100 msec*/
    uint32               vrrp_version;        /* VRRP version */
    uint32               vrrp_session_count;  /* VRRP total session number*/
    uint32               vrrp_msec_session_count; /* VRRP msec session number*/
    uint8                buf[GLB_VRRP_PKT_BUF]; /* VRRP tx packet buffer */
    uint32               pkt_len;             /* VRRP tx packet length */
} tbl_vrrp_global_t;

#endif /* !__TBL_VRRP_GLOBAL_DEFINE_H__ */

