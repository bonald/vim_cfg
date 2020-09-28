
#ifndef __LLDP_H__
#define __LLDP_H__
#include "proto_debug.h"

#define LLDP_LOG_DEBUG(typeenum, fmt, args...) \
do {\
    PM_DEBUG_OUT_INFO(LLDP, lldp, typeenum, fmt, ##args);\
}while(0)

#define LLDP_DEBUG_FUNC(typeenum) \
    LLDP_LOG_DEBUG(typeenum, "Enter into %s.", __FUNCTION__)

#define LLDP_DEBUG(typeenum, fmt, args...) \
do {\
    LLDP_LOG_DEBUG(typeenum, fmt, ##args);\
}while(0)

/* lldp PDU APIs */
int32
lldp_pdu_tx(tbl_lldp_if_t *p_db_lldp_if);
int32
lldp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg);

int32
lldp_pdu_tx_disable(tbl_lldp_if_t *p_db_lldp_if);

/* lldp interface APIs */
int32
lldp_if_tx_init(tbl_lldp_if_t *p_db_lldp_if);
int32
lldp_if_tx_deinit(tbl_lldp_if_t *p_db_lldp_if);
int32
lldp_if_update_tx_ttl(tbl_lldp_if_t *p_db_lldp_if);
int32
lldp_if_update_neigh(tbl_lldp_if_t *p_db_lldp_if, ds_lldp_neighbour_t *p_neigh);
int32
lldp_if_create(tbl_lldp_if_t *p_lldp_if);
int32
lldp_if_set_mode(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_t *p_db_lldp_if);
int32
lldp_if_destory(tbl_lldp_if_t *p_db_lldp_if);

int32
_lldp_if_del_neigh(tbl_lldp_if_t *p_db_lldp_if, ds_lldp_neighbour_t *p_db_neigh);

int32
_lldp_if_del_all_neigh(tbl_lldp_if_t *p_db_lldp_if);

/* lldp init/deinit APIs */
int32
lldp_init();
int32
lldp_start(switch_master_t *master);
int32
lldp_stop(switch_master_t *master);

/* lldp cmd APIs */
int32
lldp_cmd_process_lldp_global(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
lldp_cmd_process_lldp_if(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
lldp_cmd_process_show_local(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
lldp_cmd_process_show_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
lldp_cmd_process_show_statistics(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
lldp_cmd_process_clear_statistics(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* __LLDP_H__ */

