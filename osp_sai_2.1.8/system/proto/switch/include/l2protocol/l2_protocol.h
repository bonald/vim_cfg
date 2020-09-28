#ifndef __L2_PROTOCOL_H__
#define __L2_PROTOCOL_H__


/* Macro to convert G8131 FSM Request State value to string */
#define L2_PROTOCOL_ACTION_STR(ACTION)                                 \
  ((ACTION) == GLB_L2_PROTO_TUNNEL ?   "tunnel" :                   \
   (ACTION) == GLB_L2_PROTO_PEER ?   "peer" :                   \
   (ACTION) == GLB_L2_PROTO_DISCARD ?     "discard" :             \
   (ACTION) == GLB_L2_PROTO_FORWARD ?     "forward" :                     \
                                      "Invalid")
int32
l2_protocol_init(void);

int32
l2_protocol_start(switch_master_t *master);

int32
l2_protocol_cmd_process_l2_protocol(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
l2_protocol_add_l2_protocol(tbl_l2_protocol_t* p_l2_protocol);

int32
l2_protocol_set_l2_protocol_group_mac(tbl_l2_protocol_t* p_l2_protocol);

int32
l2_protocol_del_user_define_protocol_on_interface(tbl_l2_protocol_t* p_db_l2protocol);

int32
l2_protocol_cmd_show_l2_protocol_process(char **argv, int32 argc, cfg_cmd_para_t *para);


#endif /* !__L2_PROTOCOL_H__ */