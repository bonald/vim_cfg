
#ifndef __NCS_YANG_H__
#define __NCS_YANG_H__

/* ietf-system */
int
ncs_ietf_system_hostname_change_cb(sr_session_ctx_t *srs, const char *module_name, 
    sr_notif_event_t event, void *private_ctx);

/* hwvtep-ttp */
int
ncs_hwvtep_ttp_module_change_cb(sr_session_ctx_t *srs, const char *module_name,
        sr_notif_event_t event, void *private_ctx);

int
ncs_hwvtep_ttp_switch_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int
ncs_hwvtep_ttp_controller_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int 
ncs_hwvtep_ttp_controller_provider_cb(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx);

int
ncs_hwvtep_ttp_interface_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int 
ncs_hwvtep_ttp_interface_provider_cb(const char *xpath, sr_val_t **values, size_t *values_cnt, void *private_ctx);

int
ncs_hwvtep_ttp_interface_ref_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int
ncs_oam_aps_group_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int
ncs_oam_aps_session_list_change_cb(sr_session_ctx_t *srs, const char *xpath, 
    sr_notif_event_t event, void *private_ctx);

int
ncs_hwvtep_ttp_switch_create_capable_switch();

void
ncs_hwvtep_ttp_switch_init();

void
ncs_hwvtep_ttp_switch_reset();

int
ncs_rpc_test_cb(const char *xpath, const sr_val_t *input, const size_t input_cnt,
    sr_val_t **output, size_t *output_cnt, void *private_ctx);

void
ncs_register_cb();

#endif /* !__NCS_YANG_H__ */

