
#ifndef __HSRV_MESSAGE_H__
#define __HSRV_MESSAGE_H__

/* hsrv message statistics */
typedef struct hsrv_msg_cnt_s
{
     uint64 curr_cnt;
     uint64 last_cnt;
} hsrv_msg_cnt_t;

typedef struct hsrv_hagt_info_s 
{
    uint32 phy_slotid;
    void *p_hagt_session_info;
} hsrv_hagt_info_t;

typedef int (*HSRV_CFG_SYNC_CB_FUNC)(void *); 
typedef int (*HSRV_MESSAGE_CB_FUNC)(void *);

typedef struct hsrv_message_master_s
{
    int32 fei_sock;
    int32 fei_sock_high;
    HSRV_MESSAGE_CB_FUNC hsrv_hal_msg_cb[FEI_MSG_TYPE_MAX]; /*Callback functions for hal to hal server message*/
    HSRV_MESSAGE_CB_FUNC hagt2srv_msg_cb[opHagt2Srv_PR_hagt2SrvMsgMax]; /*Callback functions for hal to hal server message*/
} hsrv_message_master_t;

#define HSRV2SWITCH_MSG_SEND_RESP(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, FALSE, PM_ID_SWITCH)

#define HSRV2SWITCH_MSG_SEND_NOTIFY(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, TRUE, PM_ID_SWITCH)

/* Use normal socket in a coordinated way with ovs-vswitchd */
#define HSRV2OPENFLOW_MSG_SEND_NOTIFY_NORMAL(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, FALSE, PM_ID_OPENFLOW)

#define HSRV2OPENFLOW_MSG_SEND_RESP(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, FALSE, PM_ID_OPENFLOW)
    
#define HSRV2OPENFLOW_MSG_SEND_NOTIFY(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, TRUE, PM_ID_OPENFLOW)

#define HSRV2CHSM_MSG_SEND_RESP(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, FALSE, PM_ID_CHSM)

#define HSRV2CHSM_MSG_SEND_NOTIFY(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, TRUE, PM_ID_CHSM)

#define HSRV2ROUTED_MSG_SEND_NOTIFY(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, TRUE, PM_ID_ROUTED)

#define HSRV2AGT_MSG_SEND(msg) \
    hsrv_message_send_msg_to_hagt(msg)

#define HSRV2PTP_MSG_SEND_RESP(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, FALSE, PM_ID_PTP)

#define HSRV2OPM_MSG_SEND(cmd, req, req_len) \
    hsrv_message_send_msg_to_hal(cmd, req, req_len, TRUE, PM_ID_OPM)

/*This api is just used for db sync, commented by xgu, 2011-2-21*/
#define HSRV2AGT_UCAST_MSG_SEND(hagt_session, msg) \
    hsrv_message_send_ucast_msg_to_hagt(hagt_session, msg)

int32
hsrv_message_module_start(void **pp_msg_master);

int32
hsrv_message_set_cb_from_fei(hsrv_msg_type_t msg_type, HSRV_MESSAGE_CB_FUNC cb);

int32
hsrv_message_send_msg_to_hal(uint32 cmd, void *req, uint32 req_len, uint32 high, pm_id_t pm_id);

int32
hsrv_message_set_cb_from_hagt(opHagt2Srv_PR msg_type, HSRV_MESSAGE_CB_FUNC pf_func);

int32
hsrv_message_send_msg_to_hagt(Hsrv2AgtMsg_t *p_msg);

int32
hsrv_message_send_ucast_msg_to_hagt(hsrv_hagt_info_t *p_hagt_info, 
                                    Hsrv2AgtMsg_t *p_msg);

int32
hsrv_message_get_curr_valid_chip(uint32 *p_chip_bmp);

#endif /* !__HSRV_MESSAGE_H__ */

