#ifndef __HAGT_MESSAGE_PRIV_H__
#define __HAGT_MESSAGE_PRIV_H__

#define HAGT_MSG_LOG(fmt, args...) 

#if 0
enum hagt_message_session_type_e
{
    HAGT_MESSAGE_SESSION_MIN = 0,
        
    HAGT_MESSAGE_SESSION_RMT,/*This type of message is used for normal config*/
    HAGT_MESSAGE_SESSION_TCP,/*This type of  message is just used for db sync*/
    
    HAGT_MESSAGE_SESSION_MAX
};
typedef enum hagt_message_session_type_e hagt_message_session_type_t;

struct hagt_message_node_s
{
    ctclib_list_node_t head;
    Hsrv2AgtMsg_t *msg;
};
typedef struct hagt_message_node_s hagt_message_node_t;

struct hagt_message_session_s
{
    int32   sock;
    uint32 msg_id;
    uint16 session_type;
    uint16 phy_slot_id;/*Physical slot number*/
    bool has_init;
    ctclib_thread_t *p_thread_read;
    ctclib_thread_t *p_thread_connect;
    ctclib_thread_t *p_thread_send_sync;
    
#define RE_CONNECT_INTERVAL 1000 /*1s*/
    uint8 ibuf[GLB_HSRV2AGT_MSG_BUF_LEN];
    int32  ibuf_len;       
    Hsrv2AgtMsg_t *msg;
};
typedef struct hagt_message_session_s hagt_message_session_t;
#endif
struct hagt_message_info_s
{
#if 0
    /*Socket information*/
    hagt_message_session_t hagt_msg_ucast;
    hagt_message_session_t hagt_msg_rmt;/*rmt message*/
#endif
    bool db_syning;
    ctclib_list_t pending_msg_list;
    uint32 rmt_initseq;
    HAGT_MESSAGE_CB_FUNC hsrv2agt_msg_cb[opHsrv2Agt_PR_hsrv2AgtMsgMax];/*msg callback function for hal_agent, op_PR_halMsgMax*/
    sal_mutex_t *pst_send_mutex;
};
typedef struct hagt_message_info_s hagt_message_info_t;

#endif /*!__HAGT_MESSAGE_PRIV_H__*/
