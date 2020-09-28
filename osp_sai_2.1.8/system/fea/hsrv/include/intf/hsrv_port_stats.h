#ifndef __HSRV_SYNC_PORT_STATS_H__
#define __HSRV_SYNC_PORT_STATS_H__


#define  LOG_THRESHOLD_DISCARD_RESUME   0
#define  LOG_THRESHOLD_DISCARD_OVER     1

extern int32 
hsrv_port_stats_init();

int32
hsrv_port_stats_clear_stats(uint32 ifindex);

int32
hsrv_port_stats_get_all_stats(uint32 ifindex);

int32
hsrv_port_stats_rate_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
        void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_PORT_STATS_H__ */
