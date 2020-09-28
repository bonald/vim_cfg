
#ifndef __PTP_API_H__
#define __PTP_API_H__

#define PTP_API_PTR_CHECK(_v) GLB_PTR_VALID_CHECK(_v, PTP_E_API_INVALID_PARAM)//PTP_PTR_INVALID_RETURN

#define PTP_API_INVALID_INTERFACE_RET(p_ptp_port) \
do \
{ \
    if (ptp_ops_ifp_invalid(p_ptp_port) || (NULL == p_ptp_port) || (0 != p_ptp_port->lag_id)) \
    { \
        return PTP_E_API_INVALID_INTERFACE; \
    } \
} \
while (0)

int32 
ptp_api_current_slave_port_not_null(char* ifname);

int32
ptp_api_set_hsrv_adjust_offset(int64 offset);
int32
ptp_api_set_hsrv_device_type(uint8 hsrv_device_type);
int32
ptp_api_set_hsrv_reset_tx_ts();
int32
ptp_api_set_hsrv_update_system_time(int16 utc_offset);
int32
ptp_api_set_port_hsrv_path_delay(tbl_ptp_port_t *p_db_ptp_port, int64 path_delay);

#ifndef ptp_cmd_related
int32
ptp_api_set_device_type(ptp_device_type_t type);

int32
ptp_api_set_default_ds_two_step_flag(uint32 two_step_flag);

int32
ptp_api_get_default_ds_two_step_flag(uint32* p_two_step);

int32
ptp_api_set_time_prop_intern_current_utc_offset(int16 current_utc_offset);

int32
ptp_api_get_time_prop_intern_current_utc_offset(int16* current_utc_offset);

int32
ptp_api_set_default_ds_slave_only(uint8 slave_only);

int32
ptp_api_get_default_ds_slave_only(uint8* slave_only);

int32
ptp_api_set_default_ds_domain_number(uint8 domain_number);

int32
ptp_api_get_default_ds_domain_number(uint8* p_domain);

int32
ptp_api_set_system_time(uint32 set_system_time);

int32
ptp_api_get_system_time(uint32* set_system_time);

int32
ptp_api_set_time_prop_intern_leap61(uint8 leap61);

int32
ptp_api_get_time_prop_intern_leap61(uint8* p_leap61);

int32
ptp_api_set_time_prop_intern_leap59(uint8 leap59);

int32
ptp_api_get_time_prop_intern_leap59(uint8* p_leap59);

int32
ptp_api_set_default_ds_primary_domain(uint8 primary_domain);

int32
ptp_api_get_default_ds_primary_domain(uint8* primary_domain);

int32
ptp_api_set_default_ds_priority1(uint8 priority1);

int32
ptp_api_get_default_ds_priority1(uint8* p_priority1);

int32
ptp_api_set_default_ds_priority2(uint8 priority2);

int32
ptp_api_get_default_ds_priority2(uint8* p_priority2);

int32
ptp_api_set_time_prop_intern_time_source(uint8 time_source);

int32
ptp_api_get_time_prop_intern_time_source(uint8* p_time_source);

int32
ptp_api_set_default_ds_clk_accuracy(uint8 clk_accuracy);

int32
ptp_api_get_default_ds_clk_accuracy(uint8* p_clock_accuracy);

int32
ptp_api_set_default_ds_clk_class(uint8 clk_class);

int32
ptp_api_get_default_ds_clk_class(uint8* p_clock_class);

int32
ptp_api_set_clock_id(glb_ptp_clock_id_t clock_id);

int32
ptp_api_set_global_enable(uint32 enable);

int32
ptp_api_get_global_enable(uint32* p_enable);

int32
ptp_api_set_port_port_ds_log_announce_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_announce_interval);

int32
ptp_api_get_port_port_ds_log_announce_interval(tbl_ptp_port_t *p_port, int8* log_announce_interval);

int32
ptp_api_set_port_port_ds_log_sync_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_sync_interval);

int32
ptp_api_get_port_port_ds_log_sync_interval(tbl_ptp_port_t *p_port, int8* log_sync_interval);

int32
ptp_api_set_port_port_ds_log_min_delay_req_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_min_delay_req_interval);

int32
ptp_api_get_port_port_ds_log_min_delay_req_interval(tbl_ptp_port_t *p_port, int8* log_min_delay_req_interval);

int32
ptp_api_set_port_port_ds_log_min_pdelay_req_interval(tbl_ptp_port_t *p_db_ptp_port, int8 log_min_pdelay_req_interval);

int32
ptp_api_get_port_port_ds_log_min_pdelay_req_interval(tbl_ptp_port_t *p_port, int8* log_min_pdelay_req_interval);

int32
ptp_api_set_port_port_ds_announce_receipt_timeout(tbl_ptp_port_t *p_db_ptp_port, uint8 announce_receipt_timeout);

int32
ptp_api_get_port_port_ds_announce_receipt_timeout(tbl_ptp_port_t *p_port, uint8* announce_receipt_timeout);

int32
ptp_api_set_port_asymmetry_correction(tbl_ptp_port_t *p_db_ptp_port, int64 asymmetry_correction);

int32
ptp_api_set_port_ingress_latency(tbl_ptp_port_t *p_db_ptp_port, int32 ingress_latency);

int32
ptp_api_set_port_egress_latency(tbl_ptp_port_t *p_db_ptp_port, int32 egress_latency);

int32
ptp_api_get_port_latency_correction(tbl_ptp_port_t *p_port, int32* ingress_latency, int32* egress_latency);

int32
ptp_api_set_port_port_ds_delay_mechanism(tbl_ptp_port_t *p_db_ptp_port, uint8 delay_mechanism);

int32
ptp_api_get_port_port_ds_delay_mechanism(tbl_ptp_port_t *p_port, ptp_delay_mechanism_t* p_mechanism);

int32
ptp_api_set_port_vlan_info(tbl_ptp_port_t *p_db_ptp_port, uint32 vlan_id, uint8 cos);

int32
ptp_api_get_port_vlan_info(tbl_ptp_port_t *p_port, ptp_vlan_info_t* p_info);

int32
ptp_api_set_port_enable(tbl_ptp_port_t *p_db_ptp_port, uint32 enable);

int32
ptp_api_get_port_enable(tbl_ptp_port_t *p_port , uint32* enable);
#endif
/* global APIs */
int32
ptp_api_set_glb_enable(uint32 enable);

uint64
ptp_api_get_global_enable_time();

/* port APIs */
int32
ptp_api_sync_add_port(tbl_interface_t *p_db_if);

int32
ptp_api_sync_del_port(tbl_ptp_port_t *p_db_ptp_port);

int32
ptp_api_sync_set_port_up_running(tbl_interface_t *p_db_if, tbl_interface_t *p_if);

int32
ptp_api_set_port_enable(tbl_ptp_port_t *p_db_ptp_port, uint32 enable);

#endif /* !__PTP_API_H__ */

