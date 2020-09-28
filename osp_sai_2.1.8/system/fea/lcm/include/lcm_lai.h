
#ifndef __LCM_LAI_H__
#define __LCM_LAI_H__

#include <sys/types.h>
#include <netinet/in.h>
#include "glb_hw_define.h"
#include "glb_const.h"
#ifdef CNOS_OS
#include "laiinc.h"

int32 
lcm_lai_rx_laiTmprThreshold(int32 low, int32 high, int32 crit);
int32 
lcm_lai_rx_laiLedSet(int32 led, int32 status);

int32 
lcm_lai_rx_laiOemInfoGet(char* buf);
int32 
lcm_lai_rx_laiOemInfoSet(char* buf);
int32 
lcm_lai_rx_laiSwVerInfoSet(char* buf);
int32 
lcm_lai_get_FiberInfo(lai_fiber_info_t** fiber, glb_port_t *p_port);

int32
lcm_lai_set_Configbootcmd_onie(char* cmd);

int32  
lcm_lai_Updatebootrom(char* path);

int32  
lcm_lai_UpdateEpld(char* path);
int32 
lcm_lai_set_PortInfoSet(char* info);

int32
lcm_lai_SetStmMode(uint32 mode);

int32
lcm_lai_get_lcRebootInfo(lai_reboot_info_t** reboot_info);

int32 
lcm_lai_ResetRebootInfo();

int32
lcm_lai_Updatephyfirmware(int32 phy_port_idx, char* path);

int32 
lcm_lai_set_fan_speed_rate(uint16 bottomspeed,uint16 lowspeed,uint16 highspeed,uint16 fullspeed);

int32 
lcm_lai_set_poe_enable(char enable);

int32 
lcm_lai_set_poe_power_max(uint32 power_max);

#endif
#endif

