
#ifndef __CTC_TTP_DRV_SCL_H__
#define __CTC_TTP_DRV_SCL_H__

#define CTC_SCL_ENTRY_ID_MIN    20
#define CTC_SCL_ENTRY_ID_NUM   200

typedef struct
{
    uint32 ipsa;
    uint32 ipda;
    uint32 vni;
}vxlan_tunnel_key_t;

typedef struct
{
    vxlan_tunnel_key_t key;
    uint32 ref_cnt;
}vxlan_tunnel_t;

int32
ctc_ttp_drv_scl_entry_id_alloc(uint32 *entry_id);

int32
ctc_ttp_drv_scl_entry_id_free(uint32 entry_id);

int32
ctc_ttp_drv_scl_vlan_mapping_add(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_terminal_mac_flow_add(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_entry_remove(hsrv_openflow_ttp_flow_t *p_flow); 

int32
ctc_ttp_drv_scl_vxlan_tunnel_add(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_vxlan_tunnel_del(hsrv_openflow_ttp_flow_t *p_flow);

int32
ctc_ttp_drv_scl_set_port_default_entry(uint32 gport);

int32
ctc_ttp_drv_scl_unset_port_default_entry(uint32 gport);

int32
ctc_ttp_drv_scl_init();

#endif

