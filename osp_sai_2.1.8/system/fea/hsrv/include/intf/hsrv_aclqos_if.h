#ifndef __HSRV_SYNC_ACLQOS_IF_H__
#define __HSRV_SYNC_ACLQOS_IF_H__

int32
hsrv_aclqos_if_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_aclqos_if_process_aclqos_if_stats(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hsrv_qos_get_queue_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats);

#endif 
