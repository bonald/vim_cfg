#ifndef __LACP_PDU_H__
#define __LACP_PDU_H__

/* See Figure 43-18. */
typedef struct
{
    uint32 response;
    uint16 port;
    uint8  system[MAC_ADDR_LEN];
    uint32 transaction_id;
} lacp_marker_pdu_t;

typedef struct
{
    uint16  system_priority;
    uint8   system[MAC_ADDR_LEN];
    uint8   state;
    uint16  key;
    uint16  port_priority;
    uint16  port;
} lacp_info_t;

typedef struct
{
    uint8       subtype;
    uint8       version_number;
    lacp_info_t actor;
    lacp_info_t partner;
    uint16      collector_max_delay;
} lacp_pdu_t;

int32
lacp_pdu_tx_lacp(tbl_interface_t *p_db_if);

int32
lacp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

#endif /* !__LACP_PDU_H__ */
