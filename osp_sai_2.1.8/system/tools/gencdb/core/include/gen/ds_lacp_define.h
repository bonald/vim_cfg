
#ifndef __DS_LACP_DEFINE_H__
#define __DS_LACP_DEFINE_H__

/* DS_LACP field defines */
typedef enum
{
    DS_LACP_FLD_ACTOR_PORT_PRIORITY  = 0 ,  /* RW */
    DS_LACP_FLD_ACTOR_OPER_PORT_KEY  = 1 ,  /* READ */
    DS_LACP_FLD_ACTOR_PORT_NUMBER    = 2 ,  /* READ */
    DS_LACP_FLD_SHORT_TIMEOUT        = 3 ,  /* RW */
    DS_LACP_FLD_ACTOR_OPER_PORT_STATE = 4 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_SYSTEM_PRIORITY = 5 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_SYSTEM = 6 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_KEY    = 7 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_PORT_PRIORITY = 8 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_PORT_NUMBER = 9 ,  /* READ */
    DS_LACP_FLD_PARTNER_ADMIN_PORT_STATE = 10,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_SYSTEM_PRIORITY = 11,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_SYSTEM  = 12,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_KEY     = 13,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_PORT_PRIORITY = 14,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_PORT_NUMBER = 15,  /* READ */
    DS_LACP_FLD_PARTNER_OPER_PORT_STATE = 16,  /* READ */
    DS_LACP_FLD_TX_LACP_COUNT        = 17,  /* READ */
    DS_LACP_FLD_TX_ERROR_COUNT       = 18,  /* READ */
    DS_LACP_FLD_RX_LACP_COUNT        = 19,  /* READ */
    DS_LACP_FLD_RX_ERROR_COUNT       = 20,  /* READ */
    DS_LACP_FLD_MLAG_ID              = 21,  /* READ */
    DS_LACP_FLD_NTT                  = 22,  /* READ */
    DS_LACP_FLD_LACP_ENABLED         = 23,  /* READ */
    DS_LACP_FLD_READY_N              = 24,  /* READ */
    DS_LACP_FLD_PORT_MOVED           = 25,  /* READ */
    DS_LACP_FLD_PERIODIC_TX_STATE    = 26,  /* READ */
    DS_LACP_FLD_RECEIVE_STATE        = 27,  /* READ */
    DS_LACP_FLD_MUX_STATE            = 28,  /* READ */
    DS_LACP_FLD_SELECTED             = 29,  /* READ */
    DS_LACP_FLD_PDU                  = 30,  /* READ */
    DS_LACP_FLD_CURRENT_WHILE_TIMER  = 31,  /* READ */
    DS_LACP_FLD_PERIODIC_TIMER       = 32,  /* READ */
    DS_LACP_FLD_WAIT_WHILE_TIMER     = 33,  /* READ */
    DS_LACP_FLD_MAX                  = 34
} ds_lacp_field_id_t;

/* DS_LACP defines */
typedef struct
{
    uint16               actor_port_priority;
    uint16               actor_oper_port_key;
    uint16               actor_port_number;
    uint8                short_timeout;
    uint8                actor_oper_port_state;
    uint16               partner_admin_system_priority;
    mac_addr_t           partner_admin_system;
    uint16               partner_admin_key;
    uint16               partner_admin_port_priority;
    uint16               partner_admin_port_number;
    uint8                partner_admin_port_state;
    uint16               partner_oper_system_priority;
    mac_addr_t           partner_oper_system;
    uint16               partner_oper_key;
    uint16               partner_oper_port_priority;
    uint16               partner_oper_port_number;
    uint8                partner_oper_port_state;
    uint32               tx_lacp_count;
    uint32               tx_error_count;
    uint32               rx_lacp_count;
    uint32               rx_error_count;
    uint32               mlag_id;             /* MLAG ID */
    uint8                ntt;
    uint8                lacp_enabled;
    uint8                ready_n;
    uint8                port_moved;
    uint8                periodic_tx_state;   /* lacp_pertx_state_t */
    uint8                receive_state;       /* lacp_rx_state_t */
    uint8                mux_state;           /* lacp_mux_state_t */
    uint8                selected;            /* lacp_selected_t */
    void                 *pdu;
    ctc_task_t           *current_while_timer;
    ctc_task_t           *periodic_timer;
    ctc_task_t           *wait_while_timer;
} ds_lacp_t;

#endif /* !__DS_LACP_DEFINE_H__ */

