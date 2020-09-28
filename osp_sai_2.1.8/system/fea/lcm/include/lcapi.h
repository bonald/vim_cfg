#ifndef __LCAPI_H__
#define __LCAPI_H__
#include "sal_common.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "glb_stm_define.h"

typedef int32 (*lcapi_callback_t)(void* );
enum lcapi_cb_msg_type_e
{
    LCAPI_CB_HAGT_INIT_SDK,
    LCAPI_CB_HAGT_INIT_QUEUE,

    LCAPI_CB_TYPE_MAX
};
typedef enum lcapi_cb_msg_type_e lcapi_cb_msg_type_t;

struct lcapi_lcm_set_port_status_s
{    
    uint32 status; /*speed, duplex, media type or mac enable or bypass or down-hold-time*/
    uint32 lp_srcport;/* for internal loopback source port */
    uint32 lp_dstport;/* for internal loopback dst port */
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;
    uint8 pad[3];
};
typedef struct lcapi_lcm_set_port_status_s lcapi_lcm_set_port_status_t;

/* support fec, modified by liuyang 2016-5-4 */
typedef struct lcapi_lcm_set_port_fec_enable_s
{
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;
    uint8 enable;
} lcapi_lcm_set_port_fec_enable_t;
/* end of liuyang modified */

typedef struct lcapi_lcm_set_port_poe_s
{
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;    
    uint8 enable;
    uint8 force_power;
    uint8 mode;/* af/at/poh */
    uint8 priority;/* critical,high,low */
    uint32 power_limit;
} lcapi_lcm_set_port_poe_t;

/* support ffe config, modified by tongzb 2018-11-19 */
typedef struct lcapi_lcm_set_port_ffe_s
{
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;
    uint8 config;
    uint8 c0;
    uint8 c1;
    uint8 c2;
    uint8 c3;
    uint8 channel;
} lcapi_lcm_set_port_ffe_t;

/* added by qicx for trunk bug26437, PTN bug26201 for GB fiber port intr, 2013-12-18 */
struct lcapi_lcm_get_logic_port_array_s
{
    uint8 logic_port[60];
};
typedef struct lcapi_lcm_get_logic_port_array_s lcapi_lcm_get_logic_port_array_t;
/* end of qicx added */

/* support eee function, modified by liuht for bug 28298, 2014-04-21 */
typedef struct lcapi_lcm_set_eee_status_s
{
    uint32 status; /*EEE enable, budget, priority*/
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;
} lcapi_lcm_set_eee_status_t;
/* end of liuht modified */

struct lcapi_lcm_set_port_tod_mode_s
{
    uint32 mode; /*mode = 0 input, mode = 1 output*/
};
typedef struct lcapi_lcm_set_port_tod_mode_s lcapi_lcm_set_port_tod_mode_t;

struct lcapi_lcm_set_port_tod_out_put_mode_s
{
    uint32 mode; /*mode = 0 hz, mode = 1 bps*/
};
typedef struct lcapi_lcm_set_port_tod_out_put_mode_s lcapi_lcm_set_port_tod_out_put_mode_t;
struct lcapi_lcm_port_select_slot_clock_s
{
    uint32 slot; 
    uint32 port_idx;
    uint32 gport;
    uint32 mode;/*mode = 0 input, mode = 1 output*/
};
typedef struct lcapi_lcm_port_select_slot_clock_s lcapi_lcm_port_select_slot_clock_t;

struct lcapi_lcm_set_10g_port_dpll_ic_mode_s
{
    uint32 mode; /*mode = 0 shutdown, mode = 1 no shutdown*/
};
typedef struct lcapi_lcm_set_10g_port_dpll_ic_mode_s lcapi_lcm_set_10g_port_dpll_ic_mode_t;

struct lcapi_lcm_set_clock_recovery_select_s
{
    uint32 clock_recovery_select;
};
typedef struct lcapi_lcm_set_clock_recovery_select_s lcapi_lcm_set_clock_recovery_select_t;

typedef struct lcapi_lcm_set_poe_status_s
{
    uint32 status; /*POE enable, budget, priority*/
    uint8 slot_idx; /*slot id in eth-X-X, for bug 14686.*/
    uint8 port_idx; /*from 1 to max_port_num, eth-X-X*/
    uint8 subport_idx;
} lcapi_lcm_set_poe_status_t;

struct lcapi_lcm_set_flowctrl_s
{
    uint8 slot_idx;
    uint8 port_idx;
    uint8 subport_idx;
    uint16 direction;
    uint16 enable;
};
typedef struct lcapi_lcm_set_flowctrl_s lcapi_lcm_set_flowctrl_t;

struct lcapi_lcm_get_port_num_s
{
    uint8 port_num;  /*total port nums in the line card*/    
    uint8 pad[3];
};
typedef struct lcapi_lcm_get_port_num_s lcapi_lcm_get_port_num_t;

struct lcapi_lcm_get_port_status_s
{
    uint8 slot_no;
    uint8 port_no;
    uint8 sub_port_no;
    
    uint32             enable;      
    glb_port_duplex_t  duplex;
    glb_port_speed_t   speed;
    glb_port_master_slave_t    master_slave; /* Modified by liuht for bug 28155, 2014-04-17 */
    uint32             phy_type;
    glb_port_flowctrl_t  flowctrl;
    /* Modified by liuht to support eee for bug 28298, 2014-05-16 */
    glb_port_eee_status_t eee_status;
    /* End of liuht modified */
    /* Modified by liuht for bug 29005, 2014-06-17 */
    uint32 port_status_change;
    /* End of liuht modified */
    uint32 fec_status;
    uint32 cl73_ability;    
};
typedef struct lcapi_lcm_get_port_status_s lcapi_lcm_get_port_status_t;

struct lcapi_lcm_port_info_s
{
    uint8 slot_no;
    uint8 port_no;
    uint8 sub_port_no;
};
typedef struct lcapi_lcm_port_info_s lcapi_lcm_port_info_t;

struct lcapi_lcm_reset_chipset_s
{
    uint8 type;  /* reset pin type glb_reset_type_t */
    uint8 flag;  /* reset pin flag (0,1) */
};
typedef struct lcapi_lcm_reset_chipset_s lcapi_lcm_reset_chipset_t;

/*Support log abnormal int, bug25780. 2013-11-12*/
struct lcapi_lcm_asic_abnormal_s
{
    uint8 fatal_type; 
    uint16 size;
    uint8* buf; 
};
typedef struct lcapi_lcm_asic_abnormal_s lcapi_lcm_asic_abnormal_t;

struct lcapi_hagt_set_force_tx_en_s
{    
    uint16 gport;   /* global port index */
    uint8 flag;     /* 1 enable, 0 diable */
    uint8 pad[1];
};
typedef struct lcapi_hagt_set_force_tx_en_s lcapi_hagt_set_force_tx_en_t;

struct lcapi_hagt_set_mac_en_s
{    
    uint16 gport;   /* global port index */
    uint8 flag;     /* 1 enable, 0 diable */
    uint8 pad[1];
};
typedef struct lcapi_hagt_set_mac_en_s lcapi_hagt_set_mac_en_t;

struct lcapi_hagt_set_mac_rx_s
{    
    uint16 gport;    /* global port index */
    uint8  flag;     /* 1 enable, 0 diable */
    uint8  pad[1];
};
typedef struct lcapi_hagt_set_mac_rx_s lcapi_hagt_set_mac_rx_t;

struct lcapi_hagt_set_mac_speed_s
{    
    uint16 gport;   /* global port index */
    glb_port_speed_t speed_mode;     /* speed mode */
};
typedef struct lcapi_hagt_set_mac_speed_s lcapi_hagt_set_mac_speed_t;

struct lcapi_hagt_set_mac_flowctrl_s
{
    uint16 gport;
    glb_port_flowctrl_t flowctrl;
};
typedef struct lcapi_hagt_set_mac_flowctrl_s lcapi_hagt_set_mac_flowctrl_t;

struct lcapi_lcm_get_clock_status_s
{
    uint32 dpll_state;
    uint32 select_ifindex;
};
typedef struct lcapi_lcm_get_clock_status_s lcapi_lcm_get_clock_status_t;

struct lcapi_lcm_get_active_sup_slot_s
{
    uint32 slot; 
};
typedef struct lcapi_lcm_get_active_sup_slot_s lcapi_lcm_get_active_sup_slot_t;

struct lcapi_lcm_get_sysmac_s
{
    uint8 sysmac[6]; 
};
typedef struct lcapi_lcm_get_sysmac_s lcapi_lcm_get_sysmac_t;

struct lcapi_hagt_set_stack_swtich_tx_trunk_s
{    
    uint32 member; 
    uint32 trunk; 
};
typedef struct lcapi_hagt_set_stack_swtich_tx_trunk_s lcapi_hagt_set_stack_swtich_tx_trunk_t;

struct lcapi_hagt_update_stack_port_s
{    
    uint32 gport; 
    uint32 linkup; 
};
typedef struct lcapi_hagt_update_stack_port_s lcapi_hagt_update_stack_port_t;

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
struct lcapi_hagt_set_mac_eee_s
{
    uint32 gport;
    uint32 eee_capability;
};
typedef struct lcapi_hagt_set_mac_eee_s lcapi_hagt_set_mac_eee_t;
/* End of liuht modified */
/* modified by yaom for 10G remote fault 20140410 */
struct lcapi_lcm_get_remote_fault_status_s
{
    uint16 gport;   /* global port index */
    uint8  pad[2];
    bool* is_error ;
};
typedef struct lcapi_lcm_get_remote_fault_status_s lcapi_lcm_get_remote_fault_status_t;

struct lcapi_hagt_set_rx_peaking_s
{
    uint8 chip;
    uint8 serdes;
    uint8 enable;
    uint8 pad;
};
typedef struct lcapi_hagt_set_rx_peaking_s lcapi_hagt_set_rx_peaking_t;

/*Bug 33637. re-cfg FFE param when DAC/Fiber change.*/
struct lcapi_hagt_set_ffe_s
{
    uint8 chip;
    uint8 serdes;
    uint8 mode;
    uint8 board_material;
    uint8 trace_len;
    uint8 c0;
    uint8 c1;
    uint8 c2;
    uint8 c3;
};
typedef struct lcapi_hagt_set_ffe_s lcapi_hagt_set_ffe_t;

struct lcapi_hagt_set_lb_s
{
    uint8 chip;
    uint8 serdes;
    uint8 mode;    //0 for internal loopback, 1 for external loopback
    uint8 enable;
};
typedef struct lcapi_hagt_set_lb_s lcapi_hagt_set_lb_t;

struct lcapi_hagt_set_polarity_s
{
    uint8 chip;
    uint8 serdes;
    uint8 dir;    //0 for rx polarity, 1 for tx polarity
    uint8 polarity_mode;
};
typedef struct lcapi_hagt_set_polarity_s lcapi_hagt_set_polarity_t;

#ifndef CNOS_OS
struct lcapi_hagt_get_sdk_profile_s
{
    uint8 profile_mode;
    uint32 profile_ext_flag;;
    stm_sdk_profile_info_t sdk_profile;
};
typedef struct lcapi_hagt_get_sdk_profile_s lcapi_hagt_get_sdk_profile_t;
#endif

/* modified by kcao for bug 13571, 2010-11-22 : use memory management */
#define LCAPI_MALLOC(type,size) XMALLOC(type,size)
#define LCAPI_CALLOC(type,size) XCALLOC(type,size)
#define LCAPI_FREE(type, ptr)   XFREE(type,ptr)

#endif
