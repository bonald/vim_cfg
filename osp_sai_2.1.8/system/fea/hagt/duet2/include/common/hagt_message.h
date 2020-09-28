#ifndef __HAGT_MESSAGE_PUB_H__
#define __HAGT_MESSAGE_PUB_H__

typedef int32 (*HAGT_MESSAGE_CB_FUNC)(Hsrv2AgtMsg_t *); 


#define HAGT2SRV_MSG_SEND(msg) \
    hagt_message_send_msg_to_hsrv(msg)

extern int32 hagt_message_set_msg_callback(opHsrv2Agt_PR msg_type, HAGT_MESSAGE_CB_FUNC func);
extern int32 hagt_message_module_start(void **pp_data);
extern int32 hagt_message_send_msg_to_hsrv(Hagt2SrvMsg_t *p_msg);
extern int32 hagt_message_init_module();
extern int hagt_message_reinit(void* arg);
int32 hagt_message_ucast_is_ready();
#endif /*!__HAGT_MESSAGE_PUB_H__*/
