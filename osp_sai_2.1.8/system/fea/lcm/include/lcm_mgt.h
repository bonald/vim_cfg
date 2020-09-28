#ifndef __LCM_MGT_H__
#define __LCM_MGT_H__

#include "glb_hw_define.h"

#ifndef SEP
#define SEP(desc)       1
#endif
#include "lcm_card.h"

typedef enum
{
    LCM_FIBER_INSERT,
    LCM_FIBER_REMOVE,
}lcm_fiber_notify_event_e;

struct lcm_mgt_stack_config
{
    int master;
};


#ifdef CNOS_OS
#define DATAPATH_PROFILE_PATH    CTC_ETC_BASE_DIR"/etc/spec/"
#else 
#define DATAPATH_PROFILE_PATH         SWITCH_PREFIX"/etc/datapath_profile/"
#endif
#define GREATBELT_DEMO_MB_DATAPATH_NAME "GreatBelt_Demo_MB.txt"
#define GREATBELT_DEMO_48T8XG_DATAPATH_NAME "GreatBelt_Demo_48T8XG.txt"
#define GREATBELT_DEMO_24SFP8XG_DATAPATH_NAME "GreatBelt_Demo_24SFP8XG.txt"
#define GREATBELT_DEMO_4XG8XG_DATAPATH_NAME "GreatBelt_Demo_4XG8XG.txt"
/*modify by weij, tmp for 48+4, review by qicx*/
#define E350_48T4XG_DATAPATH_NAME  "E350_48T4XG.txt"
#define E350_48P4X_DATAPATH_NAME  "E350_48P4X.txt"
#define E350_24P4X_DATAPATH_NAME  "E350_24P4X.txt"

/* added by liuht for e350 48+12, 2013-11-05 */
#define E350_48T4X2Q_DATAPATH_NAME  "E350_48T4X2Q.txt"
/* added by liuht for e350-8t12xg, 2013-10-11 */
#define E350_8T12X_DATAPATH_NAME  "E350_8T12XG.txt"
#define E350_8TS12X_DATAPATH_NAME  "E350_8TS12XG.txt"
#define E350_65DV1_DATAPATH_NAME "E350_65DV1.txt"

#define E350_24X_DATAPATH_NAME  "E350_24XG.txt"
#define E350_24X_DATAPATH_NAME2  "E350_24XG2.txt"
#define E350_PEGATRON_28P4X_DATAPATH_NAME "E350_PEGATRON.txt"
#define PTN722_DATAPATH_NAME  "Ptn722.txt"
#define PTN722_1G_DATAPATH_NAME  "Ptn722_1G.txt"
#define NID_DATAPATH_NAME  "ACT_NID.txt"
#define E350_HF_12T4S8X_DATAPATH_NAME  "E350_HF_12T4S8X.txt"/* support HF 2018-09-18*/ 

#define GOLDENDATE_24Q_SFP_DATAPATH_NAME "GoldGate_24q_sfp.txt"
#define GOLDENDATE_24Q_DATAPATH_NAME "GoldGate_24q.txt"
#define E580_48X2Q4Z_DATAPATH_NAME    "E580_48X2Q4Z.txt"
#define E580_48X6Q_DATAPATH_NAME    "E580_48X6Q.txt"
#define E580_32X2Q_DATAPATH_NAME    "E580_32X2Q.txt"
#define E580_WQT48XT4Q_DATAPATH_NAME "E580_WQT48XT4Q.txt"

#define E580_48X2Q4Z_100G_DATAPATH_NAME    "E580_48X2Q4Z_100G.txt"

#define E550_24X8Y2C_DATAPATH_NAME    "E550_24X8Y2C.txt"
#define E550_48T4Y2Q_DATAPATH_NAME    "E550_48T4Y2Q.txt"
#define E550_28M4Y2C_DATAPATH_NAME    "E550_28M4Y2C.txt"
#define E550_48M4Y2C_DATAPATH_NAME    "E550_48M4Y2C.txt"
#define E550_24T16Y_DATAPATH_NAME    "E550_24T16Y.txt"
#define E550_24T16X_DATAPATH_NAME    "E550_24T16X.txt"
#define E530_24X2C_D_DATAPATH_NAME    "E530_24X2C_D.txt"
#define E530_48T4X2Q_D_DATAPATH_NAME    "E530_48T4X2Q_D.txt"
#define E530_48T4X_DATAPATH_NAME    "E530_48T4X.txt"

#define E530_48X_CHIP0_DATAPATH_NAME    "E530_48X_chip0.txt"
#define E530_48X_CHIP1_DATAPATH_NAME    "E530_48X_chip1.txt"
#define E530_24X2C_DATAPATH_NAME    "E530_24X2C.txt"
#define E530_24X2Q_DATAPATH_NAME    "E530_24X2Q.txt"
#define E530_48S4X_DATAPATH_NAME    "E530_48S4X.txt"

#define SG8348C_NOBKPORT_DATAPATH_NAME     "SG8348C_NOBKPORT.txt"
#define T65C_8T64X6Q_DATAPATH_NAME "T65C_8T64X6Q.txt"

#define TM2_EMULATION_DATAPATH_NAME "TM2_emulation.txt"

/* Added by liuht for bug27036, 2014-02-27 */
#define REBOOT_INFO_SAVE_PATH  "/mnt/flash/reboot-info/reboot_info.log"
#define REBOOT_INFO_SAVE_PATH_TMP  "/mnt/flash/reboot-info/reboot_info_tmp.log"

#define LCM_POLLING_ENVIRONMENT_TIMER 3000  //scan environment every 3 seconds

#define LCM_MGT_CHK_SLOT(_SLOT)                                                     \
do {                                                                            \
    if (((_SLOT) = lcm_mgt_get_logic_slot()) == GLB_INVALID_SLOT_NO)                    \
    {                                                                           \
        return LCM_E_INVALID_SLOT;                                                              \
    }                                                                           \
}while(0)

typedef int32 (*lcm_mgt_callback_t)(void);

glb_card_t* lcm_mgt_get_card();
int16 lcm_mgt_get_logic_slot();
void lcm_mgt_set_priority_flag(int32 flag);
void lcm_mgt_set_err_reboot_flag(int32 flag);
int32 lcm_mgt_monitor_lc(void);
int32 lcm_mgt_init(void);
int32 lcm_mgt_post_init(void);
int32 lcm_mgt_reset_chipset(void* pv_arg);
//int32 lcm_mgt_enable_asic_normal_intr(void* pv_arg);
//int32 lcm_mgt_enable_asic_fatal_intr(void* pv_arg);
int32 lcm_mgt_bypass_timers(void* pv_arg);
int32 lcm_mgt_get_bypass_timers();
#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
//int32 lcm_mgt_enable_foam_normal_intr(void* pv_arg);
#endif /*FOAM_SUPPORT*/
int32 lcm_mgt_asic_fatal(void* pv_arg);
int32 lcm_reload(void);
int32 lcm_mgt_get_active_sup_slot(void* pv_arg);
int32 lcm_mgt_get_sysmac(void* pv_arg);
int lcm_mgt_get_if_hwaddr (char *ifname, unsigned char *hwaddr,
               int *hwaddr_len);
int32 lcm_mgt_reload(void* pv_arg);
int32 lcm_mgt_get_sw_info(glb_card_t* p_card);
/* Added by liuht for bug27036, 2014-02-27 */
int32 lcm_mgt_mount_reboot_info(glb_card_t* p_card);
int32 lcm_mgt_unmount_reboot_info(glb_card_t* p_card);
/* Added by liuht for bug 27788, 2014-03-24 */
void lcm_mgt_get_current_bootromver(char *bootrom_ver);
int32 lcm_mgt_set_led_flag_to_kernal(int* alarm);
int32 lcm_mgt_set_stm_mode(uint32 mode);
int32 lcm_mgt_set_tmpr_threshold(int32 low, int32 high, int32 crit);
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
int32 lcm_mgt_smartcfg_set(unsigned char mode);
#endif
int32 lcm_mgt_oem_info_get(char* buf);
int32 lcm_mgt_oem_info_set(uint8* buf);
int32 lcm_mgt_set_port_info(char* info);
int32 lcm_mgt_led_set(int32 led, int32 status);
int32 lcm_mgt_reset_reboot_info(void);
int32 lcm_mgt_update_bootrom(char* path);
int32 lcm_mgt_update_epld(char* path);
int32 lcm_mgt_update_phy_firmware(int32 phy_port_idx, char* path);
int32 lcm_mgt_set_configbootcmd_exec(char* cmd);
int32 lcm_mgt_set_configbootcmd_normal(char* cmd);
int32 lcm_mgt_mgmt_cfg_set(uint8 *buf);
int32 lcm_mgt_config_fan_speed_rate(uint16 bottomspeed,uint16 lowspeed,uint16 highspeed,uint16 fullspeed);
int32 lcm_mgt_set_swver(uint8* buf);
void lcm_mgt_get_current_serialno(char *serial_no);
int32 lcm_mgt_config_sys_poe_enable(uint8 enable);
int32 lcm_mgt_config_sys_poe_power_max(uint32 power_max);
int32 lcm_set_timers_mask(uint8 timer_id, uint8 enable);
int32 lcm_get_timers_mask(uint8 timer_id, uint8 *status);
int32 lcm_phy_init_custom_init_callback(lcm_phy_custom_init_callback_t func);
int32 lcm_check_timers_mask(uint8 type);
int32 lcm_stack_is_enable();
int32 lcm_stack_self_slotid();
int32 lcm_stack_master_slotid();
int32 lcm_ftm_read_profile_mode(uint32* p_mode);
int32 lcm_ftm_get_profile_ext_flag (glb_board_type_t type);
int32 lcm_mgt_get_sup_slot();

#endif
