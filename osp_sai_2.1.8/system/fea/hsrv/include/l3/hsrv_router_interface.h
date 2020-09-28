#ifndef __HSRV_SYNC_ROUTER_INTERFACE_H__
#define __HSRV_SYNC_ROUTER_INTERFACE_H__

int32
rif_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hsrv_router_interface_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

/* modified by liwh for bug 47781, 2018-09-17 */
int32
hsrv_router_interface_after_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
/* liwh end */

#endif /* !__HSRV_SYNC_ROUTER_INTERFACE_H__ */
