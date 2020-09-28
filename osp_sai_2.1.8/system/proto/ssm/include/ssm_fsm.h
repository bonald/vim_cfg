#ifndef __SSM_FSM_H__
#define __SSM_FSM_H__

#define SSM_FSM_ORDER_NUM_STU 2
#define SSM_FSM_ORDER_NUM_ST2 4
#define SSM_FSM_ORDER_NUM_ST3E 8

int32
_ssm_fsm_select_best_port_slave(void);

int32
 _ssm_fsm_select_best_port_standard(void);

int32
ssm_fsm_select_best_port(uint32 device_role);

int32
ssm_fsm_select_clock(void);

int32
ssm_fsm_select_non_ssm(uint32 device_role);

uint32
_ssm_fsm_get_compared_port_quality(tbl_ssm_port_t* p_ssm_if);

tbl_ssm_port_t *
ssm_get_tbl_ssm_by_ifindex(uint32 ifindex);

#endif /* !__SSM_FSM_H__ */