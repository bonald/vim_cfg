/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *  
* Global and Declaration
*
*****************************************************************************/
static int8 g_p_lc_attach_array[32] = {0,};
char g_current_bootromVer[MAX_BOOTROM_VER_LEN + 1];

int32 
_chsm_mgt_get_real_serial_no(uint32 slotid, char* serial_no)
{
    tbl_card_key_t key;
    tbl_card_t* p_db_card;
    
    
    CHSM_DEBUG(CHSM_CHASSIS, "CHSM  get serial no!\n");
    key.id = slotid;
    p_db_card = tbl_card_get_card(&key);
    if(p_db_card)
    {
        sal_strcpy(serial_no, p_db_card->serial_no);
        return PM_E_NONE;
    }
    else
    {
        return PM_E_NOT_EXIST;
    }
    
}


int32
_chsm_mgt_get_real_bootrom_ver(uint32 slotid, char* bootrom_ver)
{
    tbl_card_key_t key;
    tbl_card_t* p_db_card;
    
    
    CHSM_DEBUG(CHSM_CHASSIS, "CHSM  get bootrom version!\n");
    key.id = slotid;
    p_db_card = tbl_card_get_card(&key);
    if(p_db_card)
    {
        sal_strcpy(bootrom_ver, p_db_card->bootrom_ver);
        return PM_E_NONE;
    }
    else
    {
        return PM_E_NOT_EXIST;
    }
    
}

tbl_version_t*
_chsm_mgt_get_real_oem_info()
{
    tbl_version_t* version;

    version = tbl_version_get_version();
    return version;
    
    return PM_E_NONE;  
}

tbl_bootimage_t*
_chsm_mgt_get_bootcmd(void)
{
    return tbl_bootimage_get_bootimage();
}

int32
chsm_mgt_get_stm_mode()
{
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();
 
    return p_db_chassis->cur_stm_mode;   
}

int32
chsm_mgt_set_stm_mode(uint8 stm_mode)
{  
    int32 rc = 0;
    
    tbl_chassis_t* p_db_chassis = tbl_chassis_get_chassis();    
    
    if(NULL == p_db_chassis)
    {
        return PM_E_NOT_INITED;
    }
    p_db_chassis->next_stm_mode = stm_mode;
    PM_E_RETURN(tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_NEXT_STM_MODE));

    return rc;
}

inline uint16
chsm_mgt_get_slot(uint16 log_slot)
{
    return log_slot;
}

int32
chsm_mgt_state_machine_flag_init()
{
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();
    FILE *fp;
#if 0 //remove for new upm,  sw_ver info record by board_type_detect, this for support define sw_ver by hand
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if (fp == NULL)
    {
        return PM_E_NOT_INITED;
    }
    sal_fprintf(fp, "sw_ver %s\n", OSP_VERSION);

    sal_fclose(fp); 
#endif 
    p_db_chassis->slot_num = 1;
    p_db_chassis->type = CHSM_CHASSIS_PIZZA_BOX;
    if (module_control_is_enable_stack())
    {
        p_db_chassis->slot_num = 32;
    }

    fp = sal_fopen(GLB_CHASSIS_TYPE_FILE, "w+");
    if (fp == NULL)
    {
        sal_printf("open global board file failed\n");
        return 0;
    }
    
    sal_fprintf(fp, "%d", p_db_chassis->type);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
chsm_mgt_set_lc_attach_array(int8 lc_idx, int8 flag)
{
    g_p_lc_attach_array[lc_idx] = flag;
    return PM_E_NONE;
}

int8
chsm_mgt_get_lc_attach_array(int8 lc_idx)
{
    return g_p_lc_attach_array[lc_idx];
}

int32
chsm_reload_lc(int slot)
{
    return PM_E_NONE;
}


int 
chsm_recover_system(tbl_card_t *p_db_card)
{
    sal_unlink("/mnt/flash/startup-config.conf");
    /* added by tongzb for bug 40719 0x09 means CTCINTR reboot, 2016-09-12 */
    system("reboot_record 0x09 1>/dev/null 2>&1 && sync");
    chsm_reset_system(); 
    
    return PM_E_NONE;
}

int32
chsm_reset_system()
{
    /* notify to LSRV */
    tbl_card_t *p_db_card = tbl_card_get_default_card();

    /* Fix for bug 44823, send trap at first */
    lib_reboot_system_trap();
    
    p_db_card->reboot = TRUE;
    tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_REBOOT);
    sleep(2);
    
    /* reset self */
    lib_reboot_system();
    
    return PM_E_NONE;
}

char*
chsm_get_transceiver_info_type(uint8 fiber_info)
{
    switch(fiber_info)
    {
        case TRANSCEIVER_TX_POWER_LOW:
        case TRANSCEIVER_TX_POWER_HIGH:
            return "transmitted power";            
        case TRANSCEIVER_BIAS_LOW:
        case TRANSCEIVER_BIAS_HIGH:
            return "bias current";   
        case TRANSCEIVER_VCC_LOW:
        case TRANSCEIVER_VCC_HIGH:
            return "internal supply voltage";            
        case TRANSCEIVER_TEMP_LOW:
        case TRANSCEIVER_TEMP_HIGH:
            return "internal temperature"; 
        case TRANSCEIVER_RX_POWER_LOW:
        case TRANSCEIVER_RX_POWER_HIGH:
            return "received power";
    }

    return NULL;
}
   
void
chsm_manage_fiber_status(tbl_port_t *p_db_port, tbl_fiber_t *p_fiber, tbl_fiber_t *p_db_fiber)
{
    char message[255] = "\0";
    uint32 alarm_flag_new = 0x0;
    uint32 warn_flag_new = 0x0;    
    uint32 alarm_result = 0x0;
    uint32 warn_result = 0x0;
    char ifname[IFNAME_SIZE] = {0};
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    uint8 i, start, end;
    uint8 channel_num, channel_idx;
    
    if (p_fiber->ddm_support != TRUE)
    {
        /* Digital diagnostic is not implemented */
        return ;
    }

    channel_num = p_fiber->channel_num;
    channel_idx = p_fiber->channel_idx;
    if(channel_idx == FIBER_VAL_TYPE_MAX2)
    {
        start = FIBER_CURRENT_VALUE1;
        end = start + channel_num - 1;
    }
    else
    {
        start = channel_idx;
        end = start;
    }

    for (i = start; i<=end;i++)
    {
        /*1. get alarm_flag_new and warn_flag_new.*/ 
        if(p_fiber->rx_pwr2[i] == GLB_FIBER_INFO_INVALID_FLAG)
        {
            alarm_flag_new &= ~(1<<TRANSCEIVER_RX_POWER_HIGH);
            alarm_flag_new &= ~(1<<TRANSCEIVER_RX_POWER_LOW);
            warn_flag_new &= ~(1<<TRANSCEIVER_RX_POWER_HIGH);
            warn_flag_new &= ~(1<<TRANSCEIVER_RX_POWER_LOW);
        }
        else
        {
            if (p_fiber->rx_pwr2[i] >= p_fiber->rx_pwr[FIBER_HIGH_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_RX_POWER_HIGH;
            }
            else if (p_fiber->rx_pwr2[i] >= p_fiber->rx_pwr[FIBER_HIGH_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_RX_POWER_HIGH;
            }
            else if (p_fiber->rx_pwr2[i] <= p_fiber->rx_pwr[FIBER_LOW_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_RX_POWER_LOW;
            }
            else if (p_fiber->rx_pwr2[i] <= p_fiber->rx_pwr[FIBER_LOW_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_RX_POWER_LOW;
            }
        }
        
        if(p_fiber->tx_pwr2[i] == GLB_FIBER_INFO_INVALID_FLAG)
        {
            alarm_flag_new &= ~(1<<TRANSCEIVER_TX_POWER_HIGH);
            alarm_flag_new &= ~(1<<TRANSCEIVER_TX_POWER_LOW);
            warn_flag_new &= ~(1<<TRANSCEIVER_TX_POWER_HIGH);
            warn_flag_new &= ~(1<<TRANSCEIVER_TX_POWER_LOW);
        }
        else
        {
            if (p_fiber->tx_pwr2[i] >= p_fiber->tx_pwr[FIBER_HIGH_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_TX_POWER_HIGH;
            }
            else if (p_fiber->tx_pwr2[i] >= p_fiber->tx_pwr[FIBER_HIGH_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_TX_POWER_HIGH;
            }
            else if (p_fiber->tx_pwr2[i] <= p_fiber->tx_pwr[FIBER_LOW_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_TX_POWER_LOW;
            }
            else if (p_fiber->tx_pwr2[i] <= p_fiber->tx_pwr[FIBER_LOW_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_TX_POWER_LOW;
            }
        }
        
        if(p_fiber->tmpr2[i] == GLB_FIBER_INFO_INVALID_FLAG)
        {
            alarm_flag_new &= ~(1<<TRANSCEIVER_TEMP_HIGH);
            alarm_flag_new &= ~(1<<TRANSCEIVER_TEMP_LOW);
            warn_flag_new &= ~(1<<TRANSCEIVER_TEMP_HIGH);
            warn_flag_new &= ~(1<<TRANSCEIVER_TEMP_LOW);
        }
        else
        {
            if (p_fiber->tmpr2[i] >= p_fiber->tmpr[FIBER_HIGH_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_TEMP_HIGH;
            }
            else if (p_fiber->tmpr2[i] >= p_fiber->tmpr[FIBER_HIGH_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_TEMP_HIGH;
            }
            else if (p_fiber->tmpr2[i] <= p_fiber->tmpr[FIBER_LOW_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_TEMP_LOW;
            }
            else if (p_fiber->tmpr2[i] <= p_fiber->tmpr[FIBER_LOW_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_TEMP_LOW;
            }
        }
        
        if(p_fiber->bias2[i] == GLB_FIBER_INFO_INVALID_FLAG)
        {
            alarm_flag_new &= ~(1<<TRANSCEIVER_BIAS_HIGH);
            alarm_flag_new &= ~(1<<TRANSCEIVER_BIAS_LOW);
            warn_flag_new &= ~(1<<TRANSCEIVER_BIAS_HIGH);
            warn_flag_new &= ~(1<<TRANSCEIVER_BIAS_LOW);
        }
        else
        {
            if (p_fiber->bias2[i] >= p_fiber->bias[FIBER_HIGH_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_BIAS_HIGH;
            }
            else if (p_fiber->bias2[i] >= p_fiber->bias[FIBER_HIGH_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_BIAS_HIGH;
            }
            else if (p_fiber->bias2[i] <= p_fiber->bias[FIBER_LOW_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_BIAS_LOW;
            }
            else if (p_fiber->bias2[i] <= p_fiber->bias[FIBER_LOW_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_BIAS_LOW;
            }
        }
        
        if(p_fiber->voltage2[i] == GLB_FIBER_INFO_INVALID_FLAG)
        {
            alarm_flag_new &= ~(1<<TRANSCEIVER_VCC_HIGH);
            alarm_flag_new &= ~(1<<TRANSCEIVER_VCC_LOW);
            warn_flag_new &= ~(1<<TRANSCEIVER_VCC_HIGH);
            warn_flag_new &= ~(1<<TRANSCEIVER_VCC_LOW);
        }
        else
        {
            if (p_fiber->voltage2[i] >= p_fiber->voltage[FIBER_HIGH_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_VCC_HIGH;
            }
            else if (p_fiber->voltage2[i] >= p_fiber->voltage[FIBER_HIGH_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_VCC_HIGH;
            }
            else if (p_fiber->voltage2[i] <= p_fiber->voltage[FIBER_LOW_ALARM_VALUE])
            {
                alarm_flag_new |= 1<<TRANSCEIVER_VCC_LOW;
            }
            else if (p_fiber->voltage2[i] <= p_fiber->voltage[FIBER_LOW_WARN_VALUE])
            {
                warn_flag_new |= 1<<TRANSCEIVER_VCC_LOW;
            }
        }

        /*2. compare the different between old and new. store real-time value.*/ 
        alarm_result   = p_db_fiber->alarm_flag[i] ^ alarm_flag_new;
        warn_result    = p_db_fiber->warn_flag[i] ^ warn_flag_new;
        p_db_fiber->alarm_flag[i] = alarm_flag_new;
        p_db_fiber->warn_flag[i]  = warn_flag_new;
        tbl_fiber_set_fiber_field(p_db_fiber, TBL_FIBER_FLD_ALARM_FLAG);
        tbl_fiber_set_fiber_field(p_db_fiber, TBL_FIBER_FLD_WARN_FLAG);

        //tbl_fiber_set_fiber_field(p_db_fiber, TBL_FIBER_FLD_ALARM_FLAG);
        //tbl_fiber_set_fiber_field(p_db_fiber, TBL_FIBER_FLD_WARN_FLAG);
            
        if_build_port_fullname_by_slot_port(ifname, p_db_port->slot_no,
            p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
        IFNAME_ETH2FULL(ifname, ifname_ext);      
        
        /*3. if alarm_result or warn_result changed, need log it.*/
        if(0 != alarm_result)
        {
            if(alarm_result & (0x1<<TRANSCEIVER_TEMP_HIGH))
            {
                sal_memset(message, 0, sizeof(message));
                if(alarm_flag_new & (0x1<<TRANSCEIVER_TEMP_HIGH))
                {
                    if((0 != warn_result) && (warn_flag_new & (0x1<<TRANSCEIVER_TEMP_HIGH)))
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "high", "alarm", "warning");
                        sal_sprintf(message,"Transceiver %s internal temperature enter to high alarm level, before is warning!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "high", "alarm", "normal");
                        sal_sprintf(message,"Transceiver %s internal temperature enter to high alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if((0 != warn_result) && (warn_flag_new & (0x1<<TRANSCEIVER_TEMP_HIGH)))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "high", "warning", "alarm");
                        sal_sprintf(message,"Transceiver %s internal temperature enter to high warning level, before is alarm!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "alarm");
                        sal_sprintf(message,"Transceiver %s internal temperature return to normal, before is high!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            
            if(alarm_result & (0x1<<TRANSCEIVER_TEMP_LOW))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_TEMP_LOW))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TEMP_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "low", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s internal temperature enter to low alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "low", "alarm", "normal");
                        sal_sprintf(message,"Transceiver %s internal temperature enter to low alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TEMP_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "low", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s internal temperature enter to low warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "alarm");
                        sal_sprintf(message,"Transceiver %s internal temperature return to normal, before is low!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }

            if(alarm_result & (0x1<<TRANSCEIVER_VCC_HIGH))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_VCC_HIGH))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "high", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s internal supply voltage enter to high alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "high", "alarm", "normal");
                             sal_sprintf(message,"Transceiver %s internal supply voltage enter to high alarm level, before is normal!\n", ifname_ext);
                   }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "high", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s internal supply voltage enter to high warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "alarm");
                        sal_sprintf(message,"Transceiver %s internal supply voltage return to normal, before is high!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            if(alarm_result & (0x1<<TRANSCEIVER_VCC_LOW))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_VCC_LOW))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "low", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s internal supply voltage enter to low alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "low", "alarm", "normal");
                            sal_sprintf(message,"Transceiver %s internal supply voltage enter to low alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "low", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s internal supply voltage enter to low warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "alarm");
                        sal_sprintf(message,"Transceiver %s internal supply voltage return to normal, before is low!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            
            if(alarm_result & (0x1<<TRANSCEIVER_BIAS_HIGH))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_BIAS_HIGH))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "high", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s bias current enter to high alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "high", "alarm", "normal");
                        sal_sprintf(message,"Transceiver %s bias current enter to high alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "high", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s bias current enter to high warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "alarm");
                        sal_sprintf(message,"Transceiver %s bias current return to normal, before is high!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            if(alarm_result & (0x1<<TRANSCEIVER_BIAS_LOW))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_BIAS_LOW))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "low", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s bias current enter to low alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "low", "alarm", "normal");
                            sal_sprintf(message,"Transceiver %s bias current enter to low alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "low", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s bias current enter to low warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "alarm");
                        sal_sprintf(message,"Transceiver %s bias current return to normal, before is low!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            
            if(alarm_result & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "high", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to high alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "high", "alarm", "normal");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to high alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "high", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to high warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "alarm");
                        sal_sprintf(message,"Transceiver %s transmitted power return to normal, before is high!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            if(alarm_result & (0x1<<TRANSCEIVER_TX_POWER_LOW))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_TX_POWER_LOW))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "low", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to low alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "low", "alarm", "normal");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to low alarm level, before is normal!\n", ifname_ext);
                    }
               }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "low", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s transmitted power enter to low warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "alarm");
                        sal_sprintf(message,"Transceiver %s transmitted power return to normal, before is low!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            
            if(alarm_result & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "high", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s received power enter to high alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "high", "alarm", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to high alarm level, before is normal!\n", ifname_ext);
                    }
               }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "high", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "alarm");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }
            if(alarm_result & (0x1<<TRANSCEIVER_RX_POWER_LOW))
            {
                if(alarm_flag_new & (0x1<<TRANSCEIVER_RX_POWER_LOW))
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "low", "alarm", "warning");
                            sal_sprintf(message,"Transceiver %s received power enter to low alarm level, before is warning!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_3_ALARM, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "low", "alarm", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low alarm level, before is normal!\n", ifname_ext);
                    }
                }
                else
                {
                    if(0 != warn_result)
                    {
                        if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_LOW))
                        {
                            logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "low", "warning", "alarm");
                            sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is alarm!\n", ifname_ext);
                        }
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "alarm");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                }
                snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
            }          
        }
        else
        {
            if(0 != warn_result)
            {
                if(warn_result & (0x1<<TRANSCEIVER_TEMP_HIGH))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_TEMP_HIGH))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "high", "warning", "normal");
                         sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is normal!\n", ifname_ext);
                   }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_HIGH), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                if(warn_result & (0x1<<TRANSCEIVER_TEMP_LOW))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_TEMP_LOW))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "low", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TEMP_LOW), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                
                if(warn_result&(0x1<<TRANSCEIVER_VCC_HIGH))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_HIGH))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "high", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_HIGH), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                if(warn_result & (0x1<<TRANSCEIVER_VCC_LOW))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_VCC_LOW))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "low", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_VCC_LOW), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                
                if(warn_result & (0x1<<TRANSCEIVER_BIAS_HIGH))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_HIGH))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "high", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_HIGH), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                if(warn_result & (0x1<<TRANSCEIVER_BIAS_LOW))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_BIAS_LOW))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "low", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_BIAS_LOW), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                
                if(warn_result & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_HIGH))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "high", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_HIGH), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                if(warn_result & (0x1<<TRANSCEIVER_TX_POWER_LOW))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_TX_POWER_LOW))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "low", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_TX_POWER_LOW), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                
                if(warn_result & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_HIGH))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "high", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to high warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_HIGH), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is high!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }
                if(warn_result & (0x1<<TRANSCEIVER_RX_POWER_LOW))
                {
                    if(warn_flag_new & (0x1<<TRANSCEIVER_RX_POWER_LOW))
                    {
                        logid_sys(LOG_TRANSCEIVER_4_WARN, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "low", "warning", "normal");
                        sal_sprintf(message,"Transceiver %s received power enter to low warning level, before is normal!\n", ifname_ext);
                    }
                    else
                    {
                        logid_sys(LOG_TRANSCEIVER_4_NORMAL, ifname_ext, chsm_get_transceiver_info_type(TRANSCEIVER_RX_POWER_LOW), "warning");
                        sal_sprintf(message,"Transceiver %s received power return to normal, before is low!\n", ifname_ext);
                    }
                    snmp_transceiver_monitor_trap(message, p_db_port->panel_port_no);
                }        
            }
        }
    }
    
}

int32
chsm_mgt_set_port_info(port_info_t* port_info)
{
    tbl_ifname_info_t *p_ifname_info = tbl_ifname_info_get_ifname_info();
    int32 i = 0;

    sal_memset(p_ifname_info, 0, sizeof(tbl_ifname_info_t));
    
    for (i = 0; i < port_info->prefix_num; i++)
    {
        sal_strncpy(p_ifname_info->prefix[i], port_info->prefix[i], IFNAME_SIZE);
        sal_strncpy(p_ifname_info->full_prefix[i], port_info->full_prefix[i], IFNAME_SIZE);
    }
    tbl_ifname_info_set_ifname_info_field(p_ifname_info, TBL_IFNAME_INFO_FLD_PREFIX);
    tbl_ifname_info_set_ifname_info_field(p_ifname_info, TBL_IFNAME_INFO_FLD_FULL_PREFIX);
    
    p_ifname_info->connection = port_info->connection;
    p_ifname_info->sub_connection = port_info->sub_connection;
    p_ifname_info->prefix_num = port_info->prefix_num;
    
    tbl_ifname_info_set_ifname_info_field(p_ifname_info, TBL_IFNAME_INFO_FLD_CONNECTION);
    tbl_ifname_info_set_ifname_info_field(p_ifname_info, TBL_IFNAME_INFO_FLD_SUB_CONNECTION);
    tbl_ifname_info_set_ifname_info_field(p_ifname_info, TBL_IFNAME_INFO_FLD_PREFIX_NUM);
    
    return PM_E_NONE;
}

int32 
chsm_mgt_create_version_info_file(tbl_card_t* p_card)
{
    tbl_version_t ver;
    FILE *fp;

    sal_memset(&ver, 0, sizeof(ver));
    sal_strcpy(ver.version, OSP_VERSION);
    sal_strcpy(ver.company, p_card->company_name);
    sal_strcpy(ver.package, p_card->package_name);
    sal_strcpy(ver.product, p_card->product_name);
    sal_strcpy(ver.hw_type, p_card->hardware_type);
    
    tbl_version_set_version_field(&ver, TBL_VERSION_FLD_VERSION);
    tbl_version_set_version_field(&ver, TBL_VERSION_FLD_COMPANY);
    tbl_version_set_version_field(&ver, TBL_VERSION_FLD_PACKAGE);
    tbl_version_set_version_field(&ver, TBL_VERSION_FLD_PRODUCT);
    tbl_version_set_version_field(&ver, TBL_VERSION_FLD_HW_TYPE);

    fp = sal_fopen(OEM_INFO_FILE, "w+");
    if(NULL == fp)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Open oem information file error.");
        return -1;
    }
    
    if (p_card)
    {
        sal_fprintf(fp, "%s %s\n", OEM_COMPANY_FULL_NAME, p_card->company_name);
        sal_fprintf(fp, "%s %s\n", OEM_PACKAGE_NAME, p_card->package_name);
        sal_fprintf(fp, "%s %s\n", OEM_PRODUCT_NAME, p_card->product_name);
        sal_fprintf(fp, "%s %s\n", OEM_HARDWARE_TYPE, p_card->hardware_type);
        sal_fprintf(fp, "%s %s\n", OEM_SNMP_ENTERPRISE_OID, p_card->enterprise_oid);
        sal_fprintf(fp, "%s %s\n", OEM_CHASSIS_TYPE, p_card->hardware_type);
    }
    else
    {
        sal_fprintf(fp, "%s %s\n", OEM_COMPANY_FULL_NAME, "Unknown");
        sal_fprintf(fp, "%s %s\n", OEM_PACKAGE_NAME, "Unknown");
        sal_fprintf(fp, "%s %s\n", OEM_PRODUCT_NAME, "Unknown");
        sal_fprintf(fp, "%s %s\n", OEM_HARDWARE_TYPE, "Unknown");
        sal_fprintf(fp, "%s %s\n", OEM_SNMP_ENTERPRISE_OID, "27975");
        sal_fprintf(fp, "%s %s\n", OEM_CHASSIS_TYPE, "Unknown");
    }
    
    sal_fclose(fp);
    return 0;
}

int32
chsm_mgt_parse_next_bootimage(char* bootcmd, tbl_bootimage_t *p_boot)
{
#ifdef _CTC_X86_64_
    FILE *fp = NULL;
    char *ptr_serverip = NULL;
    char *ptr_image = NULL;
    char buf[CMD_BUF_256];

    /* set default value */
    p_boot->mode = BOOT_MODE_FLASH;
    p_boot->image[0] = CMD_ZERO_CHAR;
    p_boot->serverip[0] = CMD_ZERO_CHAR;
    p_boot->ipaddr[0] = CMD_ZERO_CHAR;
    
    fp = sal_fopen(CHSM_DEBIAN_BOOT_CONFIG_FILE, "r");
    if (fp)
    {
        sal_fgets(buf, M_FULLPATH_MAX_LEN, fp);
        if (sal_strstr(buf, CHSM_DEBIAN_UIMAGE_PATH))
        {    
            p_boot->mode = BOOT_MODE_FLASH;
        }
        else
        {
            ptr_serverip = sal_strstr(buf, "tftp://");
            if (ptr_serverip)
            {
                ptr_serverip += sal_strlen("tftp://");
                ptr_image = sal_strstr(ptr_serverip, "/");
                if (ptr_image)
                {
                    ptr_image[0] = CMD_ZERO_CHAR;
                    ptr_image += sal_strlen("/");
                    sal_strncpy(p_boot->serverip, ptr_serverip, 16);
                    lib_strip_newline(ptr_image);
                    sal_strncpy(p_boot->image, ptr_image, 128);
                }
            }
            p_boot->mode = BOOT_MODE_TFTP;
        }
        sal_fclose(fp);
    }
#else
    int32 i=0;
    int32 bootcmdlen;
    char *ptr_image;
    char *ptr_serverip;
    char *ptr_ipaddr;
  
    /*bootcmd: bootcmd=boot_flash
     *bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x&ipaddr=x.x.x.x
     *bootcmd: bootcmd=boot_tftp6 xxxxOS-xx.x.x.bin&serverip6=x:x:x:x&ip6addr=x:x:x:x*/    
    ptr_image = sal_strstr(bootcmd, "boot_tftp");
    bootcmdlen = strlen(bootcmd);

    if(ptr_image != NULL)
    {
        if(sal_strstr(bootcmd, "boot_tftp6") != NULL)
        {
            ptr_serverip = sal_strstr(bootcmd, "serverip6=");
            ptr_ipaddr = sal_strstr(bootcmd, "ip6addr=");
            while(i<bootcmdlen)
            {
                if((bootcmd[i]=='&') || (bootcmd[i]=='\n'))
                    bootcmd[i]='\0';
                i++;
            }
            p_boot->mode = BOOT_MODE_TFTP6;
            sal_strcpy(p_boot->image, ptr_image+11);
            if(ptr_serverip != NULL)
            {
                sal_strcpy(p_boot->serverip, ptr_serverip+10);
            }
            if(ptr_ipaddr != NULL)
            {
                sal_strcpy(p_boot->ipaddr, ptr_ipaddr+8);
            }            
        }
        else
        {
            ptr_serverip = sal_strstr(bootcmd, "serverip=");
            ptr_ipaddr = sal_strstr(bootcmd, "ipaddr=");
            while(i<bootcmdlen)
            {
                if((bootcmd[i]=='&') || (bootcmd[i]=='\n'))
                    bootcmd[i]='\0';
                i++;
            }            
            p_boot->mode = BOOT_MODE_TFTP;
            sal_strcpy(p_boot->image, ptr_image+10);
            if(ptr_serverip != NULL)
            {
                sal_strcpy(p_boot->serverip, ptr_serverip+9);
            }
            if(ptr_ipaddr != NULL)
            {
                sal_strcpy(p_boot->ipaddr, ptr_ipaddr+7);
            }            
        }
    }
    else
    {
        p_boot->mode = BOOT_MODE_FLASH;
    }
#endif

    sal_strncpy(p_boot->bootcmd, "UNKNOWN", M_FULLPATH_MAX_LEN);
    
    return 0;
}

/* Added by kcao 2017-06-12 for bug 44321 */
int32 
chsm_mgt_sync_update_flash_image()
{
    tbl_bootimage_t *p_db_boot = tbl_bootimage_get_bootimage();

    /* for bug 45826, modify for show boot right */
    char szFullName[M_FULLPATH_MAX_LEN + 1];
    char* pFileName = NULL;
    int32 ret = 0;
    szFullName[0] = '\0';
    szFullName[M_FULLPATH_MAX_LEN] = '\0';
    ret = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, szFullName, M_FULLPATH_MAX_LEN);
    if (ret < 0)
        return 0;
    
    szFullName[ret] = '\0';
    if ((pFileName = strrchr(szFullName, '/')) == NULL)
    {
        pFileName = szFullName;
    }
    else
    {
        pFileName++;
        if ('\0' == pFileName)
            return 0;
    }

    sal_strncpy(p_db_boot->image, pFileName, SYS_INFO_SIZE);
    tbl_bootimage_set_bootimage_field(p_db_boot, TBL_BOOTIMAGE_FLD_IMAGE);
        
    return 0;
}

int32 
chsm_mgt_sync_next_bootimage_to_db(tbl_bootimage_t *p_boot)
{
    if(p_boot->mode == BOOT_MODE_FLASH)
    {
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_MODE);
        chsm_mgt_sync_update_flash_image();
    }
    else
    {
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_MODE);
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IMAGE);
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_SERVERIP);
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_IPADDR);
        tbl_bootimage_set_bootimage_field(p_boot, TBL_BOOTIMAGE_FLD_BOOTCMD);
    }

    return 0;
}

int32 
chsm_mgt_handle_next_bootimage(char* bootcmd)
{
    tbl_bootimage_t p_boot;
    
    chsm_mgt_parse_next_bootimage(bootcmd, &p_boot);
    chsm_mgt_sync_next_bootimage_to_db(&p_boot);

    return 0;
}


int32
chsm_mgt_card_init(tbl_card_t* p_db_card)
{
    uint16 max_port_idx = 0, max_slot_idx=0;
    FILE *interface_info_fd = NULL;
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();


    interface_info_fd = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if (interface_info_fd == NULL)
    {
        CHSM_LOG_ERR("Open the interface info file failed!\n");
        return PM_E_FAIL;
    }
    
    if (module_control_is_enable_stack())
    {
        max_slot_idx = 31;
        max_port_idx = 56;
    }
    else
    {
        max_slot_idx = p_db_chassis->slot_num;
        max_port_idx = p_db_card->port_num;
    }
    
    sal_fprintf(interface_info_fd, "slot 0 %d port %d\n", max_slot_idx, max_port_idx);

#if 0 //remove for new upm, for bootrom info has been record by boot_type_detect
    if (NULL != p_db_chassis) 
    {
        if (p_db_card->bootrom_ver[0] != '\0')
        {
            sal_fprintf(interface_info_fd, "bootrom %s\n", p_db_card->bootrom_ver);
        }
        else
        {
            sal_fprintf(interface_info_fd, "bootrom UNKNOWN\n");
        }
    }
#endif
    sal_fclose(interface_info_fd); 

    return PM_E_NONE;
}

int32
chsm_manage_sensor_tempr_board(tbl_sensor_t* sensor)
{
    tbl_sensor_master_t *p_master = tbl_sensor_get_master();
    tbl_sensor_t* p_db_sensor = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    int32 ret = PM_E_NONE;   
    int32 sensor_value_high = -255;
    int32 sensor_value_low  = 255;
    int32 sensor_value = 0;
    uint8 sensor_alarm = FALSE;
    uint8 sensor_critical = FALSE;
    static uint8 sensor_alarm_orig = FALSE;
    static uint8 sensor_critical_orig = FALSE;

    /* 1. check sensor pointer*/
    if (NULL == sensor)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. store highest and lowest sensor value*/

    CTCLIB_SLIST_LOOP(p_master->sensor_list, p_db_sensor, listnode)
    {
        if (p_db_sensor->is_chip)
        {
            continue;
        }
        
        if (sensor_value_high < p_db_sensor->temperature)
        {
            sensor_value_high = p_db_sensor->temperature;
        }
        if (sensor_value_low > p_db_sensor->temperature)
        {
            sensor_value_low = p_db_sensor->temperature;
        }
    }

    //log_sys(M_MOD_CHSM, E_ERROR, "chsm_manage_sensor_tempr_board tmpr_low: %d, tmpr_high : %d, alarm: %d, critical: %d",
    //    sensor_value_low, sensor_value_high, sensor_alarm_orig, sensor_critical_orig);

    /* 3. check current sensor is alarm or normal*/
    if (FALSE == sensor_alarm_orig)
    {
        if ((sensor_value_high >= sensor->tmpr_high)
          ||(sensor_value_low <= sensor->tmpr_low))
        {
            sensor_alarm = TRUE;
        }
        else
        {
            sensor_alarm = FALSE;
        }
    }
    else
    {
        if ((sensor_value_high <= sensor->tmpr_high - CHSM_TMPR_HYST)
            &&(sensor_value_low >= sensor->tmpr_low+ CHSM_TMPR_HYST))
        {
            sensor_alarm = FALSE;
        }
        else
        {
            sensor_alarm = TRUE;
        }
    }
    
    /* 4. check current sensor is critical */
    if (sensor_value_high + CHSM_TMPR_HYST >= sensor->tmpr_critical)
    {
        sensor_critical = TRUE;
    }
    else
    {
        sensor_critical = FALSE;
    }
    
    /* 5. when board sensor alarm change, log it.*/
    if (sensor_alarm != sensor_alarm_orig)
    {
        if (sensor_alarm)
        {
            sensor_value = (sensor_value_high >= sensor->tmpr_high) ? sensor_value_high : sensor_value_low;
            logid_sys(LOG_TEMPERATURE_3_ALARM, "board", sensor_value);
            snmp_temp_trap(FALSE, CHSM_FRAM_INDEX, sensor_value);
        }
        else
        {
            /* for bug 52002, by tongzb, change the log of temperature alarm, 2019-09-03 */
            logid_sys(LOG_TEMPERATURE_4_NORMAL, "board");          
            snmp_temp_trap(TRUE, CHSM_FRAM_INDEX, sensor_value_high);
        }
        sensor_alarm_orig = sensor_alarm;
    }

    /* 6. when board sensor critical change, log it.*/
    if (sensor_critical != sensor_critical_orig)
    {
        if (sensor_critical)
        {
            CHSM_LOG_WARN("System notice: board temperature %d will reach the critical threshold, system will be shutdown\n", sensor_value_high);
            system(SWITCH_PREFIX"/usr/sbin/logsync.sh 1>/dev/null 2>&1");
        }
        else
        {
            CHSM_LOG_WARN("System notice: board temperature %d return to no critical\n",sensor_value_high);
        }
        sensor_critical_orig = sensor_critical;
    }

    return ret;
}

int32
chsm_manage_sensor_tempr_chip(tbl_sensor_t* sensor)
{
    int32 ret = PM_E_NONE;   
    int32 chip_value_high = -255;
    int32 chip_value_low  = 255;
    int32 chip_value = 0;
    uint8 chip_alarm = FALSE;
    uint8 chip_critical = FALSE;
    static uint8 chip_alarm_orig = FALSE;
    static uint8 chip_critical_orig = FALSE;

    /* 1. check sensor pointer*/
    if (NULL == sensor)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. has only one chip */
    if (chip_value_high < sensor->temperature)
    {
        chip_value_high = sensor->temperature;
    }
    if (chip_value_low > sensor->temperature)
    {
        chip_value_low = sensor->temperature;
    }

    //log_sys(M_MOD_CHSM, E_ERROR, "chsm_manage_sensor_tempr_chip tmpr_low: %d, tmpr_high : %d, alarm: %d, critical: %d",
    //    chip_value_low, chip_value_high, chip_alarm_orig, chip_critical_orig);
    
    /* 3. check current chip tempr is alarm or normal*/
    if (FALSE == chip_alarm_orig)
    {
        if ((chip_value_high >= sensor->tmpr_high)
            ||(chip_value_low <= sensor->tmpr_low))
        {
            chip_alarm = TRUE;
        }
        else
        {
            chip_alarm = FALSE;
        }
    }
    else
    {
        if((chip_value_high <= sensor->tmpr_high - CHSM_TMPR_HYST)
            &&(chip_value_low >= sensor->tmpr_low + CHSM_TMPR_HYST))
        {
            chip_alarm = FALSE;
        }
        else
        {
            chip_alarm = TRUE;
        }
    }

    /* 4. check current chip tempr is critical */
    if (chip_value_high+ CHSM_TMPR_HYST >= sensor->tmpr_critical)
    {
        chip_critical = TRUE;
    }
    else
    {
        chip_critical = FALSE;
    }

    /* 5. when chip sensor alarm change, log it.*/
    if (chip_alarm != chip_alarm_orig)
    {
        if (chip_alarm)
        {
            chip_value = (chip_value_high >= sensor->tmpr_high) ? chip_value_high : chip_value_low;
            logid_sys(LOG_TEMPERATURE_3_ALARM, "chip", chip_value);
            snmp_temp_trap(FALSE, CHSM_FRAM_INDEX, chip_value);
        }
        else
        {
            /* for bug 52002, by tongzb, change the log of temperature alarm, 2019-09-03 */
            logid_sys(LOG_TEMPERATURE_4_NORMAL, "chip");
            snmp_temp_trap(TRUE, CHSM_FRAM_INDEX, chip_value_high);
        }
        chip_alarm_orig = chip_alarm;
    }
    
    /* 6. when chip sensor critical change, log it.*/
    if (chip_critical != chip_critical_orig)
    {
        if (chip_critical)
        {
            CHSM_LOG_WARN("System notice: chip temperature %d will reach the critical theshold, system will be shutdown\n", chip_value_high);
            system(SWITCH_PREFIX"/usr/sbin/logsync.sh 1>/dev/null 2>&1");
        }
        else
        {
            CHSM_LOG_WARN("System notice: chip temperature %d return to no critical\n", chip_value_high);
        }
        chip_critical_orig = chip_critical;
    }

    return ret;
}

/* Added by kcao for bug 39110, -255 is for chip_temp trigger board log, then separate board and chip process function */
int32
chsm_manage_sensor_tempr(tbl_sensor_t* sensor)
{
    if (sensor->is_chip)
    {
        chsm_manage_sensor_tempr_chip(sensor);
    }
    else
    {
        chsm_manage_sensor_tempr_board(sensor);
    }
    
    return PM_E_NONE;
}

/* for bug 45858, For E350-48T4X fan1 and fan2 order is reverse*/
static int32 chsm_build_fan_index(char *index_str, tbl_fan_t* fan)
{
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    uint32 is_fan_reverse = 0;
    uint8 model_no, fan_no;
        
    if (sal_strstr(p_db_card->product_name, "350"))
    {
        if (0 == sal_strcmp(p_db_card->hardware_name, "48T4X"))
        {
            is_fan_reverse = 1;
        }
    }
    model_no = fan->tray+1;
    fan_no = (fan->key.id & 0xff) +1;
    if(is_fan_reverse)
    {
        if(fan_no == 1)
            fan_no += 1;
        else if(fan_no == 2)
            fan_no -= 1;
    }
    sal_snprintf(index_str, PASSWD_SIZE, "%d-%d",model_no,fan_no);
    return PM_E_NONE;
}
/*log and alarm when fan status change.*/
int32
chsm_manage_fan_status(tbl_fan_t* fan)
{
    char index_str[CMD_BUF_16];

    chsm_build_fan_index(index_str, fan);

    /*fan status change, log*/
    if (!sal_strncmp(fan->status, "FAIL", GLB_NAME_INFO_STR_MAX))
    {
        logid_sys(LOG_FAN_3_FAIL, index_str, fan->slot);
        snmp_fan_trap(FALSE, fan->key.id+1); 
    }
    else
    {
        logid_sys(LOG_FAN_4_OK, index_str, fan->slot);
        snmp_fan_trap(TRUE, fan->key.id+1); 
    }
    
    return PM_E_NONE;
}

int32
chsm_manage_psu_run_status(tbl_psu_t* psu)
{
    /* absent need not report fail */
    if (psu->absent)
    {
        return PM_E_NONE;
    }

    if (sal_strncmp(psu->run_status, "OK", GLB_NAME_INFO_STR_MAX))
    {
        logid_sys(LOG_PSU_3_ABNORMAL, psu->name, psu->slot, (psu->key.id + 1));
        if (psu->support_pmbus)
        {
            logid_sys(LOG_PSU_4_PMBUS, psu->name, psu->slot, (psu->key.id + 1),
                            psu->pmbus_status_byte, psu->pmbus_status_word, psu->pmbus_status_vout,
                            psu->pmbus_status_iout, psu->pmbus_status_input, psu->pmbus_status_temperature,
                            psu->pmbus_status_cml, psu->pmbus_status_other, psu->pmbus_status_mfr_specific,
                            psu->pmbus_status_fans_1_2, psu->pmbus_status_fans_3_4);
        }
        snmp_power_trap(CHSM_SNMP_POWER_FAIL, psu->key.id+1); 
    }
    else
    {
        logid_sys(LOG_PSU_4_NORMAL, psu->name, psu->slot, (psu->key.id + 1));
        snmp_power_trap(CHSM_SNMP_POWER_NOR, psu->key.id+1); 
    }
    
    return PM_E_NONE;
}

int32
chsm_manage_psu_alert_status(tbl_psu_t* psu)
{
    /* absent need not report alert */
    if (psu->absent)
    {
        return PM_E_NONE;
    }
    
    if(!sal_strncmp(psu->alert_status, "ALERT", GLB_NAME_INFO_STR_MAX))
    {
        logid_sys(LOG_PSU_3_ALERT, psu->name, psu->slot, (psu->key.id + 1));
        if (psu->support_pmbus)
        {
            logid_sys(LOG_PSU_4_PMBUS, psu->name, psu->slot, (psu->key.id + 1),
                            psu->pmbus_status_byte, psu->pmbus_status_word, psu->pmbus_status_vout,
                            psu->pmbus_status_iout, psu->pmbus_status_input, psu->pmbus_status_temperature,
                            psu->pmbus_status_cml, psu->pmbus_status_other, psu->pmbus_status_mfr_specific,
                            psu->pmbus_status_fans_1_2, psu->pmbus_status_fans_3_4);
        }
        snmp_power_trap(CHSM_SNMP_POWER_ALERT, psu->key.id+1); 
    }
    else
    {
        logid_sys(LOG_PSU_4_NOALERT, psu->name, psu->slot, (psu->key.id + 1));
        snmp_power_trap(CHSM_SNMP_POWER_NO_ALERT, psu->key.id+1); 
    }
    
    return PM_E_NONE;
}

int32
chsm_manage_psu_absent_status(tbl_psu_t* psu)
{
    if (psu->absent)
    {
        logid_sys(LOG_PSU_3_ABSENT, psu->name, psu->slot, (psu->key.id + 1));
        snmp_power_trap(CHSM_SNMP_POWER_ABSENT, psu->key.id+1);
    }
    else
    {
        logid_sys(LOG_PSU_4_PRESENT, psu->name, psu->slot, (psu->key.id + 1));
    }

    return PM_E_NONE;
}

int32
chsm_sensor_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_sensor_key_t* p_sensor_key = NULL;
    tbl_sensor_t *p_sensor = NULL;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_sensor = (tbl_sensor_t*)p_tbl;
        CHSM_LOG_NOTICE("Sensor %s in, temperature %s is %d\n", 
            p_sensor->name, p_sensor->position, p_sensor->temperature);
        break;

    case CDB_OPER_DEL:
        p_sensor_key = (tbl_sensor_key_t*)p_tbl;
        if(p_sensor_key)
        {
            p_sensor = tbl_sensor_get_sensor(p_sensor_key);
            CHSM_LOG_NOTICE("Sensor %s out, temperature %s is %d\n", 
            p_sensor->name, p_sensor->position, p_sensor->temperature);
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

/* Added by kcao for bug 39110, should check after new tempr stored in local CDB */
int32
chsm_sensor_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_sensor_t *p_sensor = NULL;
    int32 rc = 0;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_sensor = (tbl_sensor_t*)p_tbl;
        switch (field_id)
        {
            case TBL_SENSOR_FLD_TEMPERATURE:
                PM_E_RETURN(chsm_manage_sensor_tempr(p_sensor));
                break;

            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_fan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    char index_str[CMD_BUF_16];
    tbl_fan_key_t* p_fan_key = NULL;
    tbl_fan_t *p_fan = NULL;
    int32 rc = 0;

    CHSM_DEBUG_FUNC();

    sal_memset(index_str, 0, sizeof(index_str));
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fan = (tbl_fan_t*)p_tbl;
        chsm_build_fan_index(index_str, p_fan);
        /* Deleted by kcao for bug 43790 2017-04-25 */
        /* logid_sys(LOG_FAN_4_PRESENT, index_str, p_fan->slot); */
        snmp_fan_present_trap(TRUE, p_fan->key.id+1);
        break;

    case CDB_OPER_DEL:
        p_fan_key = (tbl_fan_key_t*)p_tbl;
        if (p_fan_key)
        {
            p_fan = tbl_fan_get_fan(p_fan_key);
            chsm_build_fan_index(index_str, p_fan);
            /* Deleted by kcao for bug 43790 2017-04-25 */
            /* logid_sys(LOG_FAN_3_ABSENT, index_str, p_fan->slot); */
            snmp_fan_present_trap(FALSE, p_fan->key.id+1); 
        }
        break;

    case CDB_OPER_SET:
        p_fan = (tbl_fan_t*)p_tbl;
        switch (field_id)
        {
            case TBL_FAN_FLD_STATUS:
                PM_E_RETURN(chsm_manage_fan_status(p_fan));
                break;

            case TBL_FAN_FLD_PERCENT:
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}


int32
chsm_psu_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_psu_t *p_psu = NULL;
    int32 rc = 0;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_psu = (tbl_psu_t*)p_tbl;
        logid_sys(LOG_PSU_4_PRESENT, p_psu->name, p_psu->slot, (p_psu->key.id + 1));
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_psu = (tbl_psu_t*)p_tbl;
        switch (field_id)
        {
            case TBL_PSU_FLD_RUN_STATUS:
                PM_E_RETURN(chsm_manage_psu_run_status(p_psu));
                break;

            case TBL_PSU_FLD_ALERT_STATUS:
                PM_E_RETURN(chsm_manage_psu_alert_status(p_psu));
                break;

            case TBL_PSU_FLD_ABSENT:
                PM_E_RETURN(chsm_manage_psu_absent_status(p_psu));
                break;

            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_vcm_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_vcm_key_t* p_vcm_key = NULL;
    tbl_vcm_t *p_vcm = NULL;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vcm = (tbl_vcm_t*)p_tbl;
        CHSM_LOG_NOTICE("Vcm %s in.\n", p_vcm->slot);
        break;

    case CDB_OPER_DEL:
        p_vcm_key = (tbl_vcm_key_t*)p_tbl;
        if(p_vcm_key)
        {
            p_vcm = tbl_vcm_get_vcm(p_vcm_key);
            CHSM_LOG_NOTICE("Vcm %s out.\n", p_vcm->slot);
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_poe_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_poe_key_t* p_poe_key = NULL;
    tbl_poe_t *p_poe = NULL;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_poe = (tbl_poe_t*)p_tbl;
        CHSM_LOG_NOTICE("Poe %s in.\n", p_poe->slot);
        break;

    case CDB_OPER_DEL:
        p_poe_key = (tbl_poe_key_t*)p_tbl;
        if(p_poe_key)
        {
            p_poe = tbl_poe_get_poe(p_poe_key);
            CHSM_LOG_NOTICE("Poe %s out.\n", p_poe->slot);
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}


int32
chsm_card_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_card_key_t* p_card_key = NULL;
    tbl_card_t *p_card = NULL;
    tbl_card_t *p_db_card = NULL;
    FILE *fp = NULL;
    int32 rc = 0;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_card = (tbl_card_t*)p_tbl;
        CHSM_LOG_WARN("System notice: card %s index %d is inserted!\n", 
            p_card->hardware_name, p_card->key.id);
        
        /*Init related board file*/
        PM_E_RETURN(chsm_mgt_card_init(p_card));
        PM_E_RETURN(chsm_mgt_create_version_info_file(p_card));
        PM_E_RETURN(chsm_mgt_handle_next_bootimage(p_card->bootcmd));
        PM_E_RETURN(chsm_start_phase2());
        break;

    case CDB_OPER_DEL:
        p_card_key = (tbl_card_key_t*)p_tbl;
        if (p_card_key)
        {
            p_db_card = tbl_card_get_card(p_card_key);
            
            CHSM_LOG_WARN("System notice: card %s index %d is removed!\n", 
            p_db_card->hardware_name, p_db_card->key.id);
        }
        break;

    case CDB_OPER_SET:
        p_card = (tbl_card_t*)p_tbl;
        switch (field_id)
        {
            case TBL_CARD_FLD_STATUS:
                if(GLB_FLAG_ISSET(p_card->status, GLB_CARD_STATUS_READY))
                {
                    /*card init finish, continue startup in pizzabox*/
                     if ((fp = sal_fopen(FEA_INIT_DONE_FILE, "w+")) != NULL)
                     {
                         sal_fclose(fp);
                     }
                }
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_port_key_t* p_port_key = NULL;
    tbl_port_t *p_port = NULL;
    tbl_port_t *p_db_port = NULL;
    int32 rc = 0;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_port = (tbl_port_t*)p_tbl;
        PM_E_RETURN(chsm_port_create(p_port));
        break;

    case CDB_OPER_DEL:
        p_port_key = (tbl_port_key_t*)p_tbl;
        if (p_port_key)
        {
            p_db_port = tbl_port_get_port(p_port_key);
            
            CHSM_LOG_WARN("System notice: port %s index %d is removed!\n", 
            p_db_port->name, p_db_port->key.port_id);
        }
        break;

    case CDB_OPER_SET:
        p_port = (tbl_port_t*)p_tbl;
        switch (field_id)
        {
            case TBL_PORT_FLD_MEDIA:
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_fiber_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_fiber_key_t* p_fiber_key = NULL;
    tbl_fiber_t *p_fiber = NULL;
    tbl_fiber_t *p_db_fiber = NULL;
    tbl_port_t * p_db_port = NULL;
    uint8 slot_no = 0;
    uint8 pannel_port_no = 0;
    uint8 panel_sub_port_no = 0;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fiber = (tbl_fiber_t*)p_tbl;
        IFNAME_ETH2FULL(p_fiber->interface_name, ifname_ext);
        if_get_slot_port_by_ethname(p_fiber->interface_name, &slot_no, &pannel_port_no, &panel_sub_port_no);
        p_db_port = tbl_port_get_port_by_slot_port(slot_no, pannel_port_no, panel_sub_port_no);
        if (NULL == p_db_port)
        {
            CHSM_LOG_ERR("System notice: fiber %s  %s is inserted, but no port exist!", 
                ifname_ext, p_fiber->fiber_type_name);
            return PM_E_NONE;
        }
        logid_sys(LOG_TRANSCEIVER_4_INSERT, ifname_ext, p_fiber->fiber_type_name);        
        snmp_transceiver_trap(TRUE, p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
        break;

    case CDB_OPER_DEL:
        p_fiber_key = (tbl_fiber_key_t*)p_tbl;
        if (p_fiber_key)
        {
            p_db_fiber = tbl_fiber_get_fiber(p_fiber_key);
        }
        if (NULL == p_db_fiber)
        {
            return PM_E_NONE;
        }
        IFNAME_ETH2FULL(p_db_fiber->interface_name, ifname_ext);
        if_get_slot_port_by_ethname(p_db_fiber->interface_name, &slot_no, &pannel_port_no, &panel_sub_port_no);
        p_db_port = tbl_port_get_port_by_slot_port(slot_no, pannel_port_no, panel_sub_port_no);
        if (NULL == p_db_port)
        {
            CHSM_LOG_ERR("System notice: fiber %s %s is inserted, but no port exist!", 
                ifname_ext, p_fiber->fiber_type_name);
            return PM_E_NONE;
        }
        logid_sys(LOG_TRANSCEIVER_4_REMOVE, ifname_ext);
        snmp_transceiver_trap(FALSE, p_db_port->slot_no, p_db_port->panel_port_no, p_db_port->panel_sub_port_no);
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_fiber_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_fiber_t *p_fiber = NULL;
    tbl_fiber_t *p_db_fiber = NULL;
    tbl_port_t * p_db_port = NULL;
    tbl_port_key_t port_key;

    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_fiber = (tbl_fiber_t*)p_tbl;
        if(!p_fiber)
        {
            return PM_E_INVALID_PARAM;
        }
        p_db_fiber= tbl_fiber_get_fiber(&p_fiber->key);
        port_key.port_id = p_fiber->key.port_id;
        
        p_db_port = tbl_port_get_port(&port_key);
        if(!p_db_port || !p_db_fiber)
        {
            return PM_E_NOT_EXIST;
        }
        /*now the fiber manage function use raw data and do not need check item here*/
        chsm_manage_fiber_status(p_db_port, p_fiber, p_db_fiber);
        
        switch (field_id)
        {
            case TBL_FIBER_FLD_ALARM_FLAG: 
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
chsm_chassis_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_chassis_t* p_chassis = NULL;
    tbl_chassis_t* p_db_chassis = NULL;
    CHSM_DEBUG_FUNC();
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_chassis = (tbl_chassis_t*)p_tbl;
        if(!p_chassis)
        {
            return PM_E_INVALID_PARAM;
        }
        
        switch (field_id)
        {
            case TBL_CHASSIS_FLD_CUR_STM_MODE: 
                /*set next_stm_mode == cur_stm_mode*/
                p_db_chassis = tbl_chassis_get_chassis();
                p_db_chassis->next_stm_mode = p_chassis->cur_stm_mode;
                tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_NEXT_STM_MODE);           
                break;
               
            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

