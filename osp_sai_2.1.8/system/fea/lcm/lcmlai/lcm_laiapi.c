#include "sal_common.h"
#include "glb_hw_define.h"
#include "lcm_client.h"
#include "lcm_chsmtlk.h"
#include "lcm_debug.h"
#include "eeprom_api.h"
#include "led_api.h"
#include "bootrom_drv.h"
#include "epld_api.h"
#include "sensor_api.h"
#include "lcm_mgt.h"

#include "ctclib_sys_cmd.h"

#ifdef CNOS_OS
#include "sai.h"
#include "ctc_sai_common.h"
#include "laiinc.h"
#endif

#ifdef CNOS_OS
int32 
lcm_msg_tx_lcAttach_sock(int32 sock)
{
    lcm_clnt_t *p_clnt = lcm_get_lcm_client();
    lcm_chsm_msg_tx_lcAttach(p_clnt);
#ifndef _GLB_UML_SYSTEM_
    lcm_chsm_msg_tx_lcRebootInfo(p_clnt);
#endif
    return LCM_E_SUCCESS;
}

/* Added by liuht for bug 27036, 2014-03-20 */
/* clear reboot info file */
int32 
lcm_lai_ResetRebootInfo()
{
    return lcm_mgt_reset_reboot_info();;
}

int32
lcm_lai_SetStmMode(uint32 mode)
{
    return lcm_mgt_set_stm_mode(mode);
}

int32 
lcm_lai_set_PortInfoSet(char* info)
{
    return lcm_mgt_set_port_info(info);
}

/*void fiber_get_type_str(fiber_info_t *fiber_info, char *type_str)   fiber_api.c*/
void 
lcm_port_get_fiber_type_str(lai_fiber_info_t *fiber_info, char *type_str)
{   
    if (!fiber_info)
        return;
    switch(fiber_info->fiber_type)
    {
        case GLB_PHY_TYPE_BASE_PX:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "BASE-PX");
            break;
        case GLB_PHY_TYPE_BASE_BX10:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "BASE-BX10");  
            break;
        case GLB_PHY_TYPE_100BASE_FX:
            if(((fiber_info->compliance_code[3] >> 5 )& 0x1) != 0)
                snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100BASE-FX");  
            else if (((fiber_info->compliance_code[2] >> 2 )& 0x1) == 0x1)
            {
                if(((fiber_info->compliance_code[1] >> 3 )& 0x3) == 0x3)
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET LR-3");
                else if (((fiber_info->compliance_code[1] >> 3 )& 0x3) == 0x1)
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET LR-2");
                else
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET LR-1");
            }
            else if (((fiber_info->compliance_code[2] >> 1 )& 0x1) == 0x1)
            {
                if (((fiber_info->compliance_code[1] >> 3 )& 0x3) == 0x1)
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET IR-2");
                else
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET IR-1");
            }
            else
            {
                if (((fiber_info->compliance_code[1] >> 3 )& 0x3) == 0x2)
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 single-mode SONET SR-1");
                else
                    snprintf(type_str, FIBER_TRANS_TYPE_LEN, "OC-3 SONET SR");        
            }
            break;
        case GLB_PHY_TYPE_100BASE_LX:
            /*Fix bug 20083. update SFP string.*/
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100BASE-LX/LX10");
            break;
        case GLB_PHY_TYPE_1000BASE_T_SFP:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "1000BASE-T_SFP");
            break;
        case GLB_PHY_TYPE_1000BASE_CX:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "1000BASE-CX");
            break;
        case GLB_PHY_TYPE_1000BASE_LX:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "1000BASE-LX");
            break;
        case GLB_PHY_TYPE_1000BASE_SX:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "1000BASE-SX");
            break;
        case GLB_PHY_TYPE_10GBASE_ER:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Base-ER");
            break;
        case GLB_PHY_TYPE_10GBASE_LRM:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Base-LRM");
            break;
        case GLB_PHY_TYPE_10GBASE_LR:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Base-LR");
            break;
        case GLB_PHY_TYPE_10GBASE_SR:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Base-SR");
            break;
        case GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Passive Copper");  
            break;
        case GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "10G Active Copper");
            break;
        case GLB_PHY_TYPE_40GBASE_CR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40GBASE_CR4");
            break;
        case GLB_PHY_TYPE_40GBASE_SR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40GBASE_SR4");
            break;
        case GLB_PHY_TYPE_40GBASE_LR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40GBASE_LR4");
            break;
        case GLB_PHY_TYPE_40GBASE_XLPPI:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40GBASE_XLPPI");
            break;
        case GLB_PHY_TYPE_100G_AOC:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100G_AOC");
            break;
        case GLB_PHY_TYPE_100GBASE_SR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100GBASE_SR4");
            break;
        case GLB_PHY_TYPE_100GBASE_LR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100GBASE_LR4");
            break;
        case GLB_PHY_TYPE_100GBASE_ER4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100GBASE_ER4");
            break;
        case GLB_PHY_TYPE_100GBASE_SR10:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100GBASE_SR10");
            break;
        case GLB_PHY_TYPE_100G_CWDM4_MSA_FEC:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100G_CWDM4_MSA_FEC");
            break;
        case GLB_PHY_TYPE_100G_PSM4_SMF:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100G_PSM4_SMF");
            break;
        case GLB_PHY_TYPE_100G_ACC:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100G_ACC");
            break;
        case GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100G_CWDM4_MSA_NOFEC");
            break;
        case GLB_PHY_TYPE_100GBASE_CR4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "100GBASE_CR4");
            break;
        case GLB_PHY_TYPE_40GBASE_ER4:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40GBASE_ER4");
            break;
        case GLB_PHY_TYPE_4X10GBASE_SR:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "4X10GBASE_SR");
            break;
        case GLB_PHY_TYPE_40G_PSM4_SMF:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "40G_PSM4_SMF");
            break;
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GAUI_C2M_AOC_BER_H");
            break;
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GAUI_C2M_AOC_BER_L");
            break;            
        case GLB_PHY_TYPE_25GBASE_SR:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_SR");
            break;
        case GLB_PHY_TYPE_25GBASE_LR:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_LR");
            break;
        case GLB_PHY_TYPE_25GBASE_ER:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_ER");
            break;
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GAUI_C2M_ACC_BER_H");
            break;
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GAUI_C2M_ACC_BER_L");
            break;            
        case GLB_PHY_TYPE_25GBASE_CR_L:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_CR_CA_L");
            break;
        case GLB_PHY_TYPE_25GBASE_CR_S:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_CR_CA_S");
            break;
        case GLB_PHY_TYPE_25GBASE_CR_N:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_CR_CA_N");
            break;
        case GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE:
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "25GBASE_PASSIVE_CABLE");
            break;       
        case GLB_PHY_TYPE_UNKNOWN: /*same with default*/
        default:    
            snprintf(type_str, FIBER_TRANS_TYPE_LEN, "Unknown");  
            break;
    }
  
    return ;
    
}

int32 
lcm_lai_get_FiberInfo(lai_fiber_info_t** fiber, glb_port_t *p_port)
{
    glb_card_t* p_card = NULL;
    char type_str[FIBER_TRANS_TYPE_LEN]={0};
 
    p_card = lcm_mgt_get_card();

    (*fiber)->slot_id = p_card->phy_slot_no;
    (*fiber)->port_id = p_port->port_idx - 1;
    (*fiber)->oid = p_port->port_idx - 1 + (p_card->phy_slot_no << 16);
    (*fiber)->pannel_port_id = p_port->panel_port_no;
    (*fiber)->pannel_sub_port_id = p_port->panel_sub_port_no;

    if(NULL == p_port || NULL == p_port->p_fiber)
    {
        return LCM_E_SUCCESS;
    }

    if (((fiber_info_t*)p_port->p_fiber)->updated)
    {
        (*fiber)->updated = TRUE;
        ((fiber_info_t*)p_port->p_fiber)->updated = FALSE;
    }
    else
    {
        (*fiber)->updated = FALSE;
        return LCM_E_SUCCESS;
    }
    
    (*fiber)->fiber_type = (((fiber_info_t*)p_port->p_fiber)->fiber_type);
    (*fiber)->fiber_flag = (((fiber_info_t*)p_port->p_fiber)->fiber_flag);
    (*fiber)->is_copper = (((fiber_info_t*)p_port->p_fiber)->is_copper);
    (*fiber)->channel_idx = (((fiber_info_t*)p_port->p_fiber)->channel_idx);
    (*fiber)->channel_num = (((fiber_info_t*)p_port->p_fiber)->channel_num);
    sal_memcpy((*fiber)->name, (((fiber_info_t*)p_port->p_fiber)->name),FIBER_VENDOR_NAME_LEN+1);
    sal_strncpy((*fiber)->pn, (((fiber_info_t*)p_port->p_fiber)->pn),FIBER_VENDOR_NAME_LEN+1);    
    sal_strncpy((*fiber)->sn, (((fiber_info_t*)p_port->p_fiber)->sn),FIBER_VENDOR_NAME_LEN+1); 
    sal_strncpy((*fiber)->rev, (((fiber_info_t*)p_port->p_fiber)->rev),FIBER_VENDOR_REV_LEN+1);
    (*fiber)->speed = (((fiber_info_t*)p_port->p_fiber)->speed);
    (*fiber)->externally_calibrated = (((fiber_info_t*)p_port->p_fiber)->externally_calibrated);
    (*fiber)->wavelength[0] = ((fiber_info_t*)p_port->p_fiber)->wavelength[0];
    (*fiber)->wavelength[1] = ((fiber_info_t*)p_port->p_fiber)->wavelength[1];
    sal_memcpy((*fiber)->length, (((fiber_info_t*)p_port->p_fiber)->length),
         sizeof(((fiber_info_t*)p_port->p_fiber)->length)); 
    sal_memcpy((*fiber)->compliance_code, (((fiber_info_t*)p_port->p_fiber)->compliance_code),
        sizeof(((fiber_info_t*)p_port->p_fiber)->compliance_code)); 

    (*fiber)->ddm_support = ((fiber_info_t*)p_port->p_fiber)->ddm_support;
    if((*fiber)->ddm_support)
    {
        (*fiber)->tx_pwr2[0] = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE1];
        (*fiber)->tx_pwr2[1] = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE2];
        (*fiber)->tx_pwr2[2]       = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE3];
        (*fiber)->tx_pwr2[3]  = ((fiber_info_t*)p_port->p_fiber)->tx_pwr2[FIBER_CURRENT_VALUE4];
        (*fiber)->tx_pwr[0] = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_ALARM_VALUE];
        (*fiber)->tx_pwr[1] = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_ALARM_VALUE];
        (*fiber)->tx_pwr[2] = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_HIGH_WARN_VALUE];
        (*fiber)->tx_pwr[3]  = ((fiber_info_t*)p_port->p_fiber)->tx_pwr[FIBER_LOW_WARN_VALUE];

        (*fiber)->rx_pwr2[0] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE1];
        (*fiber)->rx_pwr2[1] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE2];
        (*fiber)->rx_pwr2[2] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE3];
        (*fiber)->rx_pwr2[3] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr2[FIBER_CURRENT_VALUE4];
        (*fiber)->rx_pwr[0] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_ALARM_VALUE];
        (*fiber)->rx_pwr[1] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_ALARM_VALUE];
        (*fiber)->rx_pwr[2] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_HIGH_WARN_VALUE];
        (*fiber)->rx_pwr[3] = ((fiber_info_t*)p_port->p_fiber)->rx_pwr[FIBER_LOW_WARN_VALUE];

        (*fiber)->tmpr2[0] = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE1];
        (*fiber)->tmpr2[1] = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE2];
        (*fiber)->tmpr2[2] = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE3];
        (*fiber)->tmpr2[3] = ((fiber_info_t*)p_port->p_fiber)->tmpr2[FIBER_CURRENT_VALUE4];
        (*fiber)->tmpr[0] = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_ALARM_VALUE];
        (*fiber)->tmpr[1] = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_ALARM_VALUE];
        (*fiber)->tmpr[2] = ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_HIGH_WARN_VALUE];
        (*fiber)->tmpr[3]= ((fiber_info_t*)p_port->p_fiber)->tmpr[FIBER_LOW_WARN_VALUE];

        (*fiber)->voltage2[0] = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE1];
        (*fiber)->voltage2[1] = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE2];
        (*fiber)->voltage2[2] = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE3];
        (*fiber)->voltage2[3] = ((fiber_info_t*)p_port->p_fiber)->voltage2[FIBER_CURRENT_VALUE4];
        (*fiber)->voltage[0] = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_ALARM_VALUE];
        (*fiber)->voltage[1] = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_ALARM_VALUE];
        (*fiber)->voltage[2] = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_HIGH_WARN_VALUE];
        (*fiber)->voltage[3] = ((fiber_info_t*)p_port->p_fiber)->voltage[FIBER_LOW_WARN_VALUE];

        (*fiber)->bias2[0] = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE1];
        (*fiber)->bias2[1] = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE2];
        (*fiber)->bias2[2] = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE3];
        (*fiber)->bias2[3] = ((fiber_info_t*)p_port->p_fiber)->bias2[FIBER_CURRENT_VALUE4];
        (*fiber)->bias[0] = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_ALARM_VALUE];
        (*fiber)->bias[1] = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_ALARM_VALUE];
        (*fiber)->bias[2]  = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_HIGH_WARN_VALUE];
        (*fiber)->bias[3] = ((fiber_info_t*)p_port->p_fiber)->bias[FIBER_LOW_WARN_VALUE];
        
    }

    lcm_port_get_fiber_type_str(*fiber, type_str);
    sal_strcpy((*fiber)->fiber_type_name, type_str);
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send Fiber attach on port %d.", p_port->port_idx);

    return LCM_E_SUCCESS;
}

/* Added by liuht for bug27036, 2014-02-27 */
int32
lcm_lai_get_lcRebootInfo(lai_reboot_info_t** reboot_info)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    int32 i = 0;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Lcm tx lc get reboot info : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    if(0 == p_card->support_reboot_info)
    {
        return LCM_E_SUCCESS;
    }
    if(NULL == reboot_info)
    {
        return LCM_E_INVALID_PARAM;
    }

    (*reboot_info)->reboot_times = p_card->reboot_times;
    for (i = 0; i < p_card->reboot_times; i++)
    {
        sal_memcpy((*reboot_info)->reboot_type[i], p_card->reboot_info[i].reboot_type, sizeof(p_card->reboot_info[i].reboot_type));
        sal_memcpy((*reboot_info)->reboot_time[i], p_card->reboot_info[i].reboot_time, sizeof(p_card->reboot_info[i].reboot_time));
    }

#endif 
    return LCM_E_SUCCESS;
}

int32 
lcm_lai_rx_laiOemInfoGet(char* buf)
{
    return lcm_mgt_oem_info_get(buf);
}


int32 
lcm_lai_rx_laiOemInfoSet(char* buf)
{
    return lcm_mgt_oem_info_set((uint8 *)buf);
}

int32 
lcm_lai_rx_laiSwVerInfoSet(char* buf)
{
    return lcm_mgt_set_swver((uint8 *)buf);
}

int32 
lcm_lai_rx_laiLedSet(int32 led, int32 status)
{
    return lcm_mgt_led_set(led, status);
}

int32  
lcm_lai_Updatebootrom(char* path)
{
    return lcm_mgt_update_bootrom(path);
}

int32
lcm_lai_Updatephyfirmware(int32 phy_port_idx, char* path)
{
    return lcm_mgt_update_phy_firmware(phy_port_idx, path);
}

int32  
lcm_lai_UpdateEpld(char* path)
{
    return lcm_mgt_update_epld(path);
}

int32
lcm_lai_set_Configbootcmd_onie(char* cmd)
{
    return lcm_mgt_set_configbootcmd_exec(cmd);
}

int32
lcm_lai_set_Configbootcmd_normal(char* cmd)
{
#if (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    return lcm_mgt_set_configbootcmd_exec(cmd);
#else
    return lcm_mgt_set_configbootcmd_normal(cmd);
#endif
}

int32 
lcm_lai_rx_laiTmprThreshold(int32 low, int32 high, int32 crit)
{
    return lcm_mgt_set_tmpr_threshold(low, high, crit);
}

int32 
lcm_lai_set_fan_speed_rate(uint16 bottomspeed,uint16 lowspeed,uint16 highspeed,uint16 fullspeed)
{
    return lcm_mgt_config_fan_speed_rate(bottomspeed, lowspeed, highspeed, fullspeed);
}

int32 
lcm_lai_set_poe_enable(char enable)
{
    return lcm_mgt_config_sys_poe_enable(enable);
}

int32 
lcm_lai_set_poe_power_max(uint32 power_max)
{
    return lcm_mgt_config_sys_poe_power_max(power_max);
}


int32
hagt_modules_init(void *arg);
int32
hsrv_switch_init();
extern int32
hagt_modules_start();
extern int32
hsrv_modeule_openflow_init();

extern void hsrv_packet_event_notification(
        void *buffer, 
        sai_size_t buffer_size, 
        uint32_t attr_count,
        const sai_attribute_t *attr_list);

extern void hsrv_cpu_packet_debug(
        void *buffer, 
        sai_size_t buffer_size, 
        uint32_t attr_count,
        const sai_attribute_t *attr_list);

extern void hsrv_interface_port_state_change_notification_cb(
    uint32 count,
    sai_port_oper_status_notification_t *noti_data);

extern void hsrv_fdb_event_notification_cb(
    _In_ uint32_t count,
    _In_ sai_fdb_event_notification_data_t *data);

extern void hsrv_switch_monitor_event_notification_cb(
    _In_ sai_object_id_t  port_oid,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list);

int32 
lcm_InitSdk(uint32 profile_type)
{
    int32 ret = LCM_E_SUCCESS;
    glb_card_t* p_card;     
    sai_switch_api_t*  switch_api;
    sai_switch_notification_t switch_notifications;
    glb_sdk_phy_maping_t* phy_map;
    uint32 port_id = 0;
    //g_bSDKinit = TRUE;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lcm_InitSdk : card is NULL.");
        return LCM_E_INVALID_PTR;
    } 

    ret = sai_api_initialize(0,NULL);
    if(ret)
    {
        LCM_LOG_BRINGUP("lcm_InitSdk init fail. ret = %d\n", ret);
    }

    sal_memset(&switch_notifications, 0, sizeof(switch_notifications));
    switch_notifications.on_packet_event = (void *)hsrv_packet_event_notification;
    switch_notifications.on_cpu_packet_debug = (void *)hsrv_cpu_packet_debug;
    switch_notifications.on_port_state_change = (void *)hsrv_interface_port_state_change_notification_cb;
    switch_notifications.on_fdb_event = (void *)hsrv_fdb_event_notification_cb;
    switch_notifications.on_monitor_event = (void *)hsrv_switch_monitor_event_notification_cb;
    ret = sai_api_query(SAI_API_SWITCH,(void**)&switch_api);
    if(SAI_STATUS_SUCCESS != ret)
    {
        LCM_LOG_BRINGUP("%% %s, ret= %d \n", "fail query sai_api_switch", ret);
        return 0;
    }
    //LCM_PRINT_CON("%% %s, profile id = %d \n", "sai_api_switch", profile_type);
    phy_map = (glb_sdk_phy_maping_t* )LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(glb_sdk_phy_maping_t)*p_card->port_num);
    for(port_id = 0; port_id < (p_card->port_num); port_id++)
    {
        phy_map[port_id].mdio_bus_addr = p_card->pp_port[port_id]->mdio_bus_addr;
        phy_map[port_id].phy_addr = p_card->pp_port[port_id]->phy_addr;
        phy_map[port_id].mac_idx = p_card->pp_port[port_id]->mac_idx;
    }    

    ret = switch_api->initialize_switch(profile_type,NULL,p_card->cfg_profile_name,&switch_notifications,p_card->port_num,(void*)phy_map);
    if(SAI_STATUS_SUCCESS != ret)
    {
        LCM_LOG_BRINGUP("%% %s, ret = %d\n", "fail  sai_api_switch initialize_switch", ret);
        //return 0;
    }
    hagt_modules_start();
    hsrv_modeule_openflow_init();

    ret = lcm_mgt_post_init();
    if(ret != LCM_E_SUCCESS)
    {
        LCM_LOG_BRINGUP("%% lcm init card callback failed!\n");
        return ret;
    }

    ret = hsrv_switch_init();
    if(SAI_STATUS_SUCCESS != ret)
    {
        LCM_LOG_BRINGUP("%% %s, ret = %d\n", "fail  hsrv_switch_init", ret);
        return 0;
    }

    return ret;
}
#endif

