
#ifndef __HSRV_SYNC_STORM_CONTROL_H__
#define __HSRV_SYNC_STORM_CONTROL_H__


int32
hsrv_interface_storm_control_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_interface_set_storm_control_broadcast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl);

int32
hsrv_interface_set_storm_control_multicast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl);

int32
hsrv_interface_set_storm_control_unicast(tbl_interface_t* p_if, ds_storm_control_t *p_stormctl);

uint32
hsrv_interface_storm_control_get_level_rate(tbl_interface_t* p_if, uint32 level);

uint32
hsrv_interface_storm_control_get_link_speed(uint8 speed);

#endif /* !__HSRV_SYNC_STORM_CONTROL_H__ */

