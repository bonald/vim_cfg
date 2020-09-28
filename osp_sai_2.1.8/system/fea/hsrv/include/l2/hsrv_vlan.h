#ifndef __HSRV_SYNC_VLAN_H__
#define __HSRV_SYNC_VLAN_H__

#define HSRV_VLAN_FLAG_STATE_ENABLE (1 << 0)
#define HSRV_VLAN_FLAG_BRIDGE_ENABLE (1 << 1)
#define HSRV_VLAN_FLAG_REPLACE_DSCP (1 << 2)
#define HSRV_VLAN_FLAG_LEARNING_ENABLE (1 << 3)
/*added by ychen for set arp exception type in 2010-9-16*/
#define HSRV_VLAN_FLAG_ARP_SNOOPING_ENABLE (1 << 4) 
#define HSRV_VLAN_FLAG_VLANIF_ENABLE  (1 << 5) 
/*added by ychen for set dhcp exception type in 2010-10-19*/
#define HSRV_VLAN_FLAG_DHCP_SNOOPING_EN (1 << 6)
#define HSRV_VLAN_FLAG_DHCP_RELAY_EN    (1 << 7)


int32
hsrv_vlan_check_port_in_enable_vlan(vid_t vid, uint32 ifindex, uint32 *is_set);

int32
hsrv_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

int32
hsrv_igsp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_igsp_intf_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_vlan_add_port_to_vlan(uint16 vid, uint16 tagmode, uint64 portid);

int32
hsrv_vlan_remove_port_from_vlan(uint16 vid, uint16 tagmode, uint64 portid);

int32
hsrv_igsp_mcfdb_mrouter_list_cb(tbl_mcfdb_t *p_mcfdb, uint32 is_add);

int32
hsrv_vlan_set_dhcp_excp_type(uint16 vlan_id, uint32 flag);

int32
hsrv_vlan_stats_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
hsrv_vlan_group_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
hsrv_vlan_set_stp_instance(uint16 vid, uint32 instanceid);

#endif /* !__HSRV_SYNC_VLAN_H__ */
