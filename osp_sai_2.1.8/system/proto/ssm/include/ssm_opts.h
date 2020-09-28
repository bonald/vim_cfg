#ifndef __SSM_OPTS_H__
#define __SSM_OPTS_H__


enum  ssm_dpll_ic_e
{
    SSM_DPLL_IC1 = 1,
    SSM_DPLL_IC2,
    SSM_DPLL_IC3,
    SSM_DPLL_IC4,
    SSM_DPLL_IC8 = 8
};
typedef enum ssm_dpll_ic_e ssm_dpll_ic_t;

enum  ssm_dpll_ic_to_ifindex_e
{
    SSM_DPLL_INVALID_IFINDEX = 0,
    SSM_DPLL_IC4_TO_IFINDEX = 20,
    SSM_DPLL_IC3_TO_IFINDEX = 24,
    SSM_DPLL_IC2_TO_IFINDEX,
    SSM_DPLL_IC1_TO_IFINDEX,
    SSM_DPLL_IC8_TO_IFINDEX  = 9999
};
typedef enum ssm_dpll_ic_to_ifindex_e ssm_dpll_ic_to_ifindex_t;

int32
ssm_opts_set_holdover_enable(uint32 holdover_en);

int32
ssm_opts_selected_ifindex(tbl_ssm_port_t* p_ssm_if_temp);

int32
ssm_opts_set_current_quality_master_only(void);

int32
ssm_opts_set_holdover_state(uint32 no_holdoff);

int32
ssm_opts_refresh_property(tbl_ssm_port_t* p_db_ssm_if, uint32 enable);

int32
ssm_opts_recheck_device(void);

int32
ssm_opts_set_synce_global_enable(uint32 synce_enable);

int32
ssm_opts_set_synce_ssm_enable(uint32 ssm_enable);

int32
ssm_opts_set_synce_clock_id();

uint32
ssm_port_is_up(tbl_ssm_port_t* p_db_ssm_if);

void
ssm_switch_mode_trigger();

int32
ssm_opts_check_all_receive_timer();

int32
ssm_opts_refresh_send_timer(tbl_ssm_port_t* p_db_ssm_if);

int32
ssm_opts_refresh_timeout_timer(tbl_ssm_port_t* p_db_ssm_if);

void
ssm_opts_timeout_trigger(void *p_data);

void
ssm_opts_wtr_select_sync_if_trigger(void *p_data);

void
ssm_opts_holdoff_select_sync_if_trigger(void *p_data);

int32
ssm_opts_receive_clock_status(tbl_ssm_global_t* p_ssm_glb);

#endif /* !__SSM_OPTS_H__ */

