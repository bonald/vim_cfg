#ifndef __HSRV_IFDB_CDB_H__
#define __HSRV_IFDB_CDB_H__

#define HSRV_CDB_PORT_ID_TO_OPENFLOW_PORT_ID(objid)    (objid & 0xFFFFFFFF)

#define HSRV_OPENFLOW_PORT_ID_TO_CDB_PORT_ID(objid)    (objid + 0x100000000ULL)

typedef int32 (*HSRV_IFDB_LISTLOOP_CB_FUNC)(uint32 ifindex, void* pv_arg);

extern int32
hsrv_ifdb_get_portid(uint32 ifindex, uint16* p_portid);

extern int32
hsrv_ifdb_get_agg_portid(uint32 ifindex, uint16* p_portid);

extern int32
hsrv_ifdb_get_ifindex_by_gportid(uint16 gportid, uint32* p_ifindex);

extern int32
hsrv_ifdb_new_l3ifid(glb_if_type_t iftype, uint32* p_l3ifid);

extern int32
hsrv_openflow_cdb_bond_update_slave(uint32 bond_ifindex, uint32 slave_ifindex, bool enable);

extern int32
hsrv_openflow_port_get_label(uint32 ifindex, uint32* p_label);

extern int32
_hsrv_openflow_get_group_id_by_ifindex_class_id(uint32 ifindex, uint32 class_id, uint32 dirction);

extern uint32 
hsrv_openflow_phyport_get_link_status(uint32 ifindex);

extern uint32 
hsrv_openflow_agg_get_link_status(uint32 ifindex);

extern uint32
hsrv_openflow_phyport_is_openflow_port(uint32 ifindex);

extern uint32
hsrv_openflow_phyport_is_openflow_port_by_portid(uint64 port_id);

extern int32
hsrv_ifdb_do_listloop(glb_if_type_t iftype, HSRV_IFDB_LISTLOOP_CB_FUNC func, void* pv_arg);

extern bool
hsrv_openflow_port_is_slave(uint32 ifindex);

extern int32
hsrv_openflow_port_set_instance_enable(uint32 ifindex, uint32 instance_enable);

extern int32
hsrv_openflow_port_add_to_bridge_by_cdb(uint32 ifindex, uint32 enable);

extern int32
hsrv_openflow_nexthop_id_opf_alloc_by_sai(uint32* p_nexthop_id);

extern int32
hsrv_openflow_nexthop_id_opf_release_by_sai(uint32 nexthop_id);

extern int32
hsrv_openflow_get_port_mac_by_gport(uint32 gport, uint8 *mac);

extern uint32
hsrv_ifdb_get_agg_gport_by_tid(uint8 tid);

extern int32
hsrv_openflow_cdb_bond_remove(uint32 ifindex);

extern bool
hsrv_openflow_cdb_port_is_bond(uint32 ifindex);

#endif
