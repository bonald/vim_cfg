
#ifndef __TBL_SSM_PORT_DEFINE_H__
#define __TBL_SSM_PORT_DEFINE_H__

/* TBL_SSM_PORT field defines */
typedef enum
{
    TBL_SSM_PORT_FLD_KEY                  = 0 ,  /* READ */
    TBL_SSM_PORT_FLD_IFINDEX              = 1 ,  /* READ */
    TBL_SSM_PORT_FLD_SYNCE_ENABLE         = 2 ,  /* RW */
    TBL_SSM_PORT_FLD_SSM_PDU_DIRECT       = 3 ,  /* RW */
    TBL_SSM_PORT_FLD_PRIORITY             = 4 ,  /* RW */
    TBL_SSM_PORT_FLD_REPLACE_CLOCKID_EN   = 5 ,  /* RW */
    TBL_SSM_PORT_FLD_RECEIVE_PDU_ENABLE   = 6 ,  /* RW */
    TBL_SSM_PORT_FLD_SEND_QUALITY         = 7 ,  /* RW */
    TBL_SSM_PORT_FLD_RECEIVE_QUALITY      = 8 ,  /* RW */
    TBL_SSM_PORT_FLD_FORCE_TX_QUALITY     = 9 ,  /* RW */
    TBL_SSM_PORT_FLD_FORCE_RX_QUALITY     = 10,  /* RW */
    TBL_SSM_PORT_FLD_DNU_GROUP_ID         = 11,  /* RW */
    TBL_SSM_PORT_FLD_HOP                  = 12,  /* RW */
    TBL_SSM_PORT_FLD_RECEIVE_CLOCK_ID     = 13,  /* RW */
    TBL_SSM_PORT_FLD_SSM_TIMEOUT_TIMER    = 14,  /* READ */
    TBL_SSM_PORT_FLD_SSM_SEND_TIMER       = 15,  /* READ */
    TBL_SSM_PORT_FLD_MAX                  = 16
} tbl_ssm_port_field_id_t;

/* TBL_SSM_PORT defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_ssm_port_key_t;

typedef struct
{
    tbl_ssm_port_key_t   key;
    uint32               ifindex;             /* interface ifindex*/
    uint32               synce_enable;        /* Port synce enable */
    uint32               ssm_pdu_direct;      /* Ssm pdu direction */
    uint32               priority;            /* Port synce priority */
    uint32               replace_clockid_en;  /* replace clockid status on port */
    uint32               receive_pdu_enable;  /* Port receive pdu or not */
    uint32               send_quality;        /* Send pdu quality */
    uint32               receive_quality;     /* Receive pdu quality */
    uint32               force_tx_quality;    /* forcible tx quality value */
    uint32               force_rx_quality;    /* forcible rx quality value */
    uint32               dnu_group_id;        /* DNU group ID */
    uint32               hop;                 /* SSM extend tlv hop*/
    ssm_clock_id_t       receive_clock_id;    /* Received clock ID*/
    ctc_task_t           *ssm_timeout_timer;  /* Ssm timeout timer */
    ctc_task_t           *ssm_send_timer;     /* Ssm send timer */
} tbl_ssm_port_t;

typedef struct
{
    ctclib_hash_t        *ssm_port_hash;
    ctclib_slist_t       *ssm_port_list;
} tbl_ssm_port_master_t;

#endif /* !__TBL_SSM_PORT_DEFINE_H__ */

