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
#include "cJSON.h"

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


/****************************************************************************
 *  
* Function
*
*****************************************************************************/

static void 
_chsm_mac_addr_add(uint8* mac, uint32 cnt)
{
    uint32_t value;
    uint32_t val_high, val_low;

    val_high = (mac[0]<<16)+(mac[1]<<8)+mac[2];
    val_low = (mac[3]<<16)+(mac[4]<<8)+mac[5];
    value = val_low + cnt;
    val_low = value & 0xffffff;
    if(value > 0xffffff)
    {
        val_high = val_high+1;
    }
    mac[0] = (val_high >> 16) & 0xff;
    mac[1] = (val_high >> 8) & 0xff;
    mac[2] = val_high & 0xff;
    mac[3] = (val_low >> 16) & 0xff;
    mac[4] = (val_low >> 8) & 0xff;
    mac[5] = val_low & 0xff;
}

bool 
chsm_check_mac(uint8 *mac)
{
  uint8 null_addr[MAC_ADDR_LEN]  = {0x00,0x00,0x00,0x00,0x00,0x00};
  uint8 broadcast_addr[MAC_ADDR_LEN]  = {0xff,0xff,0xff,0xff,0xff,0xff};

  if (!mac)
    return FALSE;
  
  if ((mac[0] & 1) || !sal_memcmp(null_addr, mac, MAC_ADDR_LEN)
    || !sal_memcmp(broadcast_addr, mac, MAC_ADDR_LEN))
    return FALSE;

  return TRUE;
}


int32
_chsm_port_get_mac(uint8* mac, uint32 phy_slot_no, uint32 port_idx)
{
    uint32 cnt = 0;
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    glb_chassis_type_t type = GLB_CHASSIS_6_SLOT;
#else
    glb_chassis_type_t type = GLB_CHASSIS_PIZZA_BOX;
#endif
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();

    if(NULL == p_db_chassis)
    {
        CHSM_LOG_ERR("Can't get chassis information!\n");
        return PM_E_NOT_EXIST;
    }
       
    sal_memcpy(mac, p_db_chassis->sys_mac, CHSM_MAC_ADDR_LEN);
    switch (type)
    {
        case GLB_CHASSIS_PIZZA_BOX:
#if 0
            if (!module_control_is_enable_stack())
                cnt = port_idx;
            else if (phy_slot_no == module_control_get_slot_no())
                cnt = port_idx;
            else
#endif
            cnt = phy_slot_no * GLB_MAX_PORT_NUM_PER_BOARD + port_idx;
            break;
        case GLB_CHASSIS_6_SLOT:
            cnt = (phy_slot_no - 3) * GLB_MAX_PORT_NUM_PER_BOARD + port_idx;    //TODO, maybe supervisor slot is not 1 or 2. wangw
            break;
        case GLB_CHASSIS_14_SLOT:
            if (phy_slot_no < 7)
                cnt = (phy_slot_no - 1) * GLB_MAX_PORT_NUM_PER_BOARD + port_idx;
            else
                cnt = (phy_slot_no - 3) * GLB_MAX_PORT_NUM_PER_BOARD + port_idx;
            break;
        default:
            CHSM_LOG_ERR("Invalid chassis type %d\n", type);
            break;
    }

    _chsm_mac_addr_add(mac, cnt);

    return PM_E_NONE;    
}

int32 
chsm_port_create(tbl_port_t* p_port)
{
    int32 ret = 0;
    uint16 slot_idx;
    uint16 port_idx; /*panel port, start from NO.1*/
    uint16 sub_port_idx;
    uint16 logic_port_idx; 
    glb_phy_type_t phy_type = p_port->phy_type;
    char mac_str[MAC_ADDR_STR_LEN];
    char ifname[GLB_PHY_PORT_NAME_SZ];
    char ifname_ext[IFNAME_SIZE];
    char sys_cmd[256];
    uint8 phyport_flag = 0;

    slot_idx = p_port->slot_no;
    port_idx = p_port->panel_port_no;
    sub_port_idx = p_port->panel_sub_port_no;

    if (module_control_is_enable_stack())
    {
        logic_port_idx = (p_port->slot_no<< 8) | p_port->logic_port_id;
    }
    else
    {
        logic_port_idx = p_port->logic_port_id;
    }

    ret = if_build_ethname_by_slot_port(ifname, slot_idx, port_idx, sub_port_idx);
    sal_memset(sys_cmd, 0, sizeof(sys_cmd));

    phyport_flag = p_port->phyinfo_flag;
    sal_snprintf(sys_cmd, 256, "cdbctl create/cdb/interface/%s/portid/%d/mac/%s/phytype/%d/phyport_flag/%u/support_speed_flag/%u",
        ifname, logic_port_idx, cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, 
        p_port->mac), phy_type, phyport_flag, p_port->speed_capbility);
    system(sys_cmd);

    sal_snprintf(sys_cmd, 256, "cdbctl create/cdb/aclqos_if/%s", ifname);
    system(sys_cmd);

    IFNAME_ETH2FULL(ifname, ifname_ext);
    logid_sys(LOG_INTERFACE_4_CREATE, ifname_ext, mac_str, logic_port_idx);
    
    sal_udelay(10000);
    return ret;
}

static char*
_chsm_get_fiber_alarm_flag(double val, double *threshold)
{
    if (val >= threshold[FIBER_HIGH_ALARM_VALUE])
        return "++";
    else if (val >= threshold[FIBER_HIGH_WARN_VALUE])
        return "+";
    else if (val <= threshold[FIBER_LOW_ALARM_VALUE])
        return "--";
    else if (val <= threshold[FIBER_LOW_WARN_VALUE])
        return "-";
    else
        return "";
}

int32
_chsm_fiber_show_one(tbl_fiber_t *p_db_fiber, uint8 detail, FILE *fp)
{
    char show_name[COMM_NAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 wavelength = 0;
    uint32 i = 0;
    uint32 start = 0;
    uint32 end = 0;
    int32 length[5];

    sal_memset(length, 0, sizeof(length));
    
    IFNAME_ETH2FULL(p_db_fiber->interface_name, ifname_ext);
    sal_fprintf(fp, "Port %s transceiver info: \n", ifname_ext);
    sal_fprintf(fp, "Transceiver Type: %s\n", p_db_fiber->fiber_type_name);
    sal_fprintf(fp, " Transceiver Vendor Name : %s\n", p_db_fiber->name);
    sal_fprintf(fp, " Transceiver PN          : %s\n", p_db_fiber->pn);
    sal_fprintf(fp, " Transceiver S/N         : %s\n", p_db_fiber->sn);
    wavelength = ((p_db_fiber->wavelength[0]<<8) | p_db_fiber->wavelength[1]);
    if (wavelength)
    {
        sal_fprintf(fp, "Transceiver Output Wavelength: %u nm\n", wavelength);
    }
    else
    {
        sal_fprintf(fp, "Transceiver Output Wavelength: N/A\n");
    }
    
    for (i = 0; i < 5; i++)
    {
        length[i] = p_db_fiber->length[i];
    }
    
    if (length[0]!=0 || length[1]!=0 || length[2]!=0 || length[3]!=0 || length[4]!=0)
    {
        /* SFF-8472 */
        if (p_db_fiber->fiber_flag == E_FIBER_SFP_P)
        {
            sal_fprintf(fp, "Supported Link Type and Length: \n");
            if (length[0])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u km\n", length[0]);
            }
            if (length[1])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u m\n", length[1]*100);
            }    
            if (length[2])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode fiber: %u m\n", length[2]*10);
            }          
            if (length[3])
            {
                sal_fprintf(fp, "      Link Length for 62.5/125um multi-mode fiber: %u m\n", length[3]*10);
            }          
            /* the length of copper cable or active cable is different in units */
            if (length[4])
            {
                if (sal_strncmp("25GBASE_SR", p_db_fiber->fiber_type_name, 10))
                {
                    sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", length[4]*10);        
                }
                else
                {
                    sal_fprintf(fp, "      Link Length for copper: %u m\n", length[4]);
                }
            }
        }
        /* SFF-8636 */
        /* for bug 45607 */
        else if (p_db_fiber->fiber_flag == E_FIBER_QSFP_P)
        {
            sal_fprintf(fp, "Supported Link Type and Length: \n");
            if (length[0])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u km\n", length[0]);
            }
            if (length[1])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM3 fiber: %u m\n", length[1]*2);
            }    
            if (length[2])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM2 fiber: %u m\n", length[2]);
            }          
            if (length[3])
            {
                sal_fprintf(fp, "      Link Length for 62.5/125um multi-mode OM1 fiber: %u m\n", length[3]);
            }       
            /* the length of copper cable or active cable is different in units */
            if (length[4])
            {
                if (p_db_fiber->is_copper)
                {
                    sal_fprintf(fp, "      Link Length for copper: %u m\n", length[4]);
                }
                else
                {
                    sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", length[4]*2);
                }
            }
        }
    }

    if (detail)
    {
        if (0 == p_db_fiber->ddm_support)
        {
            sal_fprintf(fp, "Digital diagnostic is not implemented.\n");
            sal_fprintf(fp, "\n");
            return PM_E_NONE;
        }
        
        if(p_db_fiber->channel_idx == FIBER_VAL_TYPE_MAX2)
        {
            start = FIBER_CURRENT_VALUE1;
            end = start + p_db_fiber->channel_num - 1;
        }
        else
        {
            start = p_db_fiber->channel_idx;
            end = start;
        }
        
        for (i = start; i <= end; i++)
        {
            if (start != end)
            {
                sal_sprintf(show_name, "%s chan%d", ifname_ext, i+1);
            }
            else
            {
                sal_sprintf(show_name, "%s", ifname_ext);
            }
        
            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "Transceiver is %s calibrated.\n", p_db_fiber->externally_calibrated ? "externally":"internally");
            sal_fprintf(fp, "mA: milliamperes, dBm: decibels (milliwatts), NA or N/A: not applicable. \n");
            sal_fprintf(fp, "++ : high alarm, +  : high warning, -  : low warning, -- : low alarm. \n");
            sal_fprintf(fp, "The threshold values are calibrated. \n");

            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
            sal_fprintf(fp, "                Temperature       Threshold   Threshold   Threshold   Threshold\n");
            sal_fprintf(fp, "Port            (Celsius)         (Celsius)   (Celsius)   (Celsius)   (Celsius)\n");
            sal_fprintf(fp, "---------------+-----------------+-----------+-----------+-----------+---------\n");
            sal_fprintf(fp, "%-16s", show_name);
            sal_fprintf(fp, "%-11.2f", p_db_fiber->tmpr2[i]);
            sal_fprintf(fp, "%-7s", _chsm_get_fiber_alarm_flag(p_db_fiber->tmpr2[i], p_db_fiber->tmpr)); /*flag*/
            sal_fprintf(fp, "%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                p_db_fiber->tmpr[FIBER_HIGH_ALARM_VALUE], 
                p_db_fiber->tmpr[FIBER_HIGH_WARN_VALUE], 
                p_db_fiber->tmpr[FIBER_LOW_WARN_VALUE], 
                p_db_fiber->tmpr[FIBER_LOW_ALARM_VALUE]);
         
            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
            sal_fprintf(fp, "                Voltage           Threshold   Threshold   Threshold   Threshold\n");
            sal_fprintf(fp, "Port            (Volts)            (Volts)     (Volts)     (Volts)     (Volts) \n");
            sal_fprintf(fp, "---------------+-----------------+-----------+-----------+-----------+---------\n");
            sal_fprintf(fp, "%-16s", show_name);
            sal_fprintf(fp, "%-11.2f",  p_db_fiber->voltage2[i]);
            sal_fprintf(fp, "%-7s", _chsm_get_fiber_alarm_flag(p_db_fiber->voltage2[i], p_db_fiber->voltage)); /*flag*/
            sal_fprintf(fp, "%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                p_db_fiber->voltage[FIBER_HIGH_ALARM_VALUE], 
                p_db_fiber->voltage[FIBER_HIGH_WARN_VALUE], 
                p_db_fiber->voltage[FIBER_LOW_WARN_VALUE], 
                p_db_fiber->voltage[FIBER_LOW_ALARM_VALUE]);           
            
            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                  High Alarm  High Warn   Low Warn    Low Alarm\n");
            sal_fprintf(fp, "                Current           Threshold   Threshold   Threshold   Threshold\n");
            sal_fprintf(fp, "Port            (milliamperes)      (mA)        (mA)        (mA)        (mA)   \n");
            sal_fprintf(fp, "---------------+-----------------+-----------+-----------+-----------+---------\n");
            sal_fprintf(fp, "%-16s", show_name);
            sal_fprintf(fp, "%-11.2f",  p_db_fiber->bias2[i]);
            sal_fprintf(fp, "%-7s", _chsm_get_fiber_alarm_flag(p_db_fiber->bias2[i], p_db_fiber->bias)); /*flag*/
            sal_fprintf(fp, "%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                p_db_fiber->bias[FIBER_HIGH_ALARM_VALUE], 
                p_db_fiber->bias[FIBER_HIGH_WARN_VALUE], 
                p_db_fiber->bias[FIBER_LOW_WARN_VALUE], 
                p_db_fiber->bias[FIBER_LOW_ALARM_VALUE]);
            
            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                Optical           High Alarm  High Warn   Low Warn    Low Alarm\n");
            sal_fprintf(fp, "                Transmit Power    Threshold   Threshold   Threshold   Threshold\n");
            sal_fprintf(fp, "Port             (dBm)              (dBm)       (dBm)       (dBm)       (dBm)  \n");
            sal_fprintf(fp, "---------------+-----------------+-----------+-----------+-----------+---------\n");
            sal_fprintf(fp, "%-16s", show_name);
            sal_fprintf(fp, "%-11.2f",  p_db_fiber->tx_pwr2[i]);
            sal_fprintf(fp, "%-7s", _chsm_get_fiber_alarm_flag(p_db_fiber->tx_pwr2[i], p_db_fiber->tx_pwr)); /*flag*/
            sal_fprintf(fp, "%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                p_db_fiber->tx_pwr[FIBER_HIGH_ALARM_VALUE], 
                p_db_fiber->tx_pwr[FIBER_HIGH_WARN_VALUE], 
                p_db_fiber->tx_pwr[FIBER_LOW_WARN_VALUE], 
                p_db_fiber->tx_pwr[FIBER_LOW_ALARM_VALUE]);

            sal_fprintf(fp, "-------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                Optical           High Alarm  High Warn   Low Warn    Low Alarm\n");
            sal_fprintf(fp, "                Receive Power     Threshold   Threshold   Threshold   Threshold\n");
            sal_fprintf(fp, "Port             (dBm)              (dBm)       (dBm)       (dBm)       (dBm)  \n");
            sal_fprintf(fp, "---------------+-----------------+-----------+-----------+-----------+---------\n");
            sal_fprintf(fp, "%-16s", show_name);
            sal_fprintf(fp, "%-11.2f",  p_db_fiber->rx_pwr2[i]);
            sal_fprintf(fp, "%-7s", _chsm_get_fiber_alarm_flag(p_db_fiber->rx_pwr2[i], p_db_fiber->rx_pwr)); /*flag*/
            sal_fprintf(fp, "%-12.2f%-12.2f%-12.2f%-9.2f\n", 
                p_db_fiber->rx_pwr[FIBER_HIGH_ALARM_VALUE], 
                p_db_fiber->rx_pwr[FIBER_HIGH_WARN_VALUE], 
                p_db_fiber->rx_pwr[FIBER_LOW_WARN_VALUE], 
                p_db_fiber->rx_pwr[FIBER_LOW_ALARM_VALUE]);
        }
    }

    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-05*/
int32
_chsm_fiber_show_one_json(tbl_fiber_t *p_db_fiber, uint8 detail, FILE *fp, cJSON *port_cj)
{
    char show_name[COMM_NAME_SIZE];
    char ifname_ext[IFNAME_EXT_SIZE];
    uint32 wavelength = 0;
    uint32 i = 0;
    uint32 start = 0;
    uint32 end = 0;
    int32 length[5];
    cJSON *sup_link = NULL;
    cJSON *diag_info = NULL;
    cJSON *tips = NULL;
    char str_buf[48] = "";

    sal_memset(length, 0, sizeof(length));
    
    IFNAME_ETH2FULL(p_db_fiber->interface_name, ifname_ext);
    cJSON_AddStringToObject(port_cj, "Port", ifname_ext);
    cJSON_AddStringToObject(port_cj, "Transceiver Type", p_db_fiber->fiber_type_name);
    cJSON_AddStringToObject(port_cj, "Transceiver Vendor Name", p_db_fiber->name);
    cJSON_AddStringToObject(port_cj, "ransceiver PN", p_db_fiber->pn);
    cJSON_AddStringToObject(port_cj, "Transceiver S/N", p_db_fiber->sn);
    wavelength = ((p_db_fiber->wavelength[0]<<8) | p_db_fiber->wavelength[1]);
    if (wavelength)
    {
        cJSON_AddNumberToObject(port_cj, "Transceiver Output Wavelength(nm)", wavelength);
    }
    else
    {
        cJSON_AddStringToObject(port_cj, "Transceiver Output Wavelength(nm)", "N/A");
    }
    
    for (i = 0; i < 5; i++)
    {
        length[i] = p_db_fiber->length[i];
    }
    
    if (length[0]!=0 || length[1]!=0 || length[2]!=0 || length[3]!=0 || length[4]!=0)
    {
        /* SFF-8472 */
        if (p_db_fiber->fiber_flag == E_FIBER_SFP_P)
        {
            sup_link = cJSON_CreateObject();
	     cJSON_AddItemToObject(port_cj, "Supported Link Type and Length", sup_link);
            if (length[0])
            {
		  cJSON_AddNumberToObject(sup_link, "link Length for 9/125um single mode fiber(km)", length[0]);
            }
            if (length[1])
            {
		  cJSON_AddNumberToObject(sup_link, "Link Length for 9/125um single mode fiber(m)", length[1]*100);
            }    
            if (length[2])
            {
		  cJSON_AddNumberToObject(sup_link, "Link Length for 50/125um multi-mode OM2 fiber(m)", length[2]*10);
            }          
            if (length[3])
            {
		  cJSON_AddNumberToObject(sup_link, "Link Length for 62.5/125um multi-mode OM3 fiber(m)", length[3]*10);
            }          
            /* the length of copper cable or active cable is different in units */
            if (length[4])
            {
                if (sal_strncmp("25GBASE_SR", p_db_fiber->fiber_type_name, 10))
                {
		      cJSON_AddNumberToObject(sup_link, "Link Length for 50/125um multi-mode OM4 fiber(m)", length[4]*10);
                }
                else
                {
		      cJSON_AddNumberToObject(sup_link, "Link Length for copper(m)",length[4]);
                }
            }
        }
        /* SFF-8636 */
        /* for bug 45607 */
        else if (p_db_fiber->fiber_flag == E_FIBER_QSFP_P)
        {
            sup_link = cJSON_CreateObject();
	     cJSON_AddItemToObject(port_cj, "Supported Link Type and Length", sup_link);
            if (length[0])
            {
	         cJSON_AddNumberToObject(sup_link, "link Length for 9/125um single mode fiber(km)", length[0]);
            }
            if (length[1])
            {
		  cJSON_AddNumberToObject(sup_link, "Link Length for 50/125um multi-mode OM3 fiber(m)", length[1]*2);
            }    
            if (length[2])
            {
                cJSON_AddNumberToObject(sup_link, "Link Length for 50/125um multi-mode OM2 fiber(m)", length[2]);
            }          
            if (length[3])
            {
                cJSON_AddNumberToObject(sup_link, "Link Length for 62.5/125um multi-mode OM1 fiber(m)", length[3]);
            }       
            /* the length of copper cable or active cable is different in units */
            if (length[4])
            {
                if (p_db_fiber->is_copper)
                {
                    cJSON_AddNumberToObject(sup_link, "Link Length for copper(m)", length[4]);
                }
                else
                {
                    cJSON_AddNumberToObject(sup_link, "Link Length for 50/125um multi-mode OM4 fiber(m)", length[4]*2);
                }
            }
        }
    }

    if (detail)
    {
        diag_info = cJSON_CreateObject();
        cJSON_AddItemToObject(port_cj, "Diagnostic Iinformation" , diag_info);
        tips = cJSON_CreateObject();
        cJSON_AddItemToObject(diag_info, "Unit and Symbol Description" , tips);
        if (0 == p_db_fiber->ddm_support)
        {
            cJSON_AddStringToObject(diag_info, "Digital diagnostic", "not implemented");
	     cJSON_DeleteItemFromObject(diag_info, "Unit and Symbol Description");
            return PM_E_NONE;
        }
        
        if(p_db_fiber->channel_idx == FIBER_VAL_TYPE_MAX2)
        {
            start = FIBER_CURRENT_VALUE1;
            end = start + p_db_fiber->channel_num - 1;
        }
        else
        {
            start = p_db_fiber->channel_idx;
            end = start;
        }
        
        for (i = start; i <= end; i++)
        {
            if (start != end)
            {
                sal_sprintf(show_name, "%s chan%d", ifname_ext, i+1);
            }
            else
            {
                sal_sprintf(show_name, "%s", ifname_ext);
            }


	     cJSON_AddStringToObject(tips, "Transceiver calibrated", p_db_fiber->externally_calibrated ? "externally":"internally");
	     cJSON_AddStringToObject(tips, "mA", "milliamperes");
	     cJSON_AddStringToObject(tips, "dBm", "decibels (milliwatts)");
	     cJSON_AddStringToObject(tips, "NA or N/A", "not applicable");
	     cJSON_AddStringToObject(tips, "++", "high alarm");
	     cJSON_AddStringToObject(tips, "+", "high warning"); 
	     cJSON_AddStringToObject(tips, "--", "low alarm");
	     cJSON_AddStringToObject(tips, "-", "low warning");
	     cJSON_AddStringToObject(tips, "The threshold values", "calibrated");
		 
            cJSON_AddStringToObject(diag_info, "Port", show_name);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tmpr2[i]);
	     cJSON_AddStringToObject(diag_info, "Temperature (Celsius)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tmpr[FIBER_HIGH_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Alarm Threshold (Celsius)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tmpr[FIBER_HIGH_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Warn Threshold (Celsius)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tmpr[FIBER_LOW_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Alarm Threshold (Celsius)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tmpr[FIBER_LOW_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Warn Threshold (Celsius)", str_buf);
      
			
            cJSON_AddStringToObject(diag_info, "Port", show_name);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->voltage2[i]);
	     cJSON_AddStringToObject(diag_info, "Voltage (Volts)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->voltage[FIBER_HIGH_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Alarm Threshold (Volts)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->voltage[FIBER_HIGH_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Warn Threshold (Volts)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->voltage[FIBER_LOW_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Alarm Threshold (Volts)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->voltage[FIBER_LOW_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Warn Threshold (Volts)", str_buf); 
			 

	     cJSON_AddStringToObject(diag_info, "Port", show_name);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->bias2[i]);
	     cJSON_AddStringToObject(diag_info, "Current (milliamperes)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->bias[FIBER_HIGH_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Alarm Threshold (mA)", str_buf);
            sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->bias[FIBER_HIGH_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Warn Threshold (mA)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->bias[FIBER_LOW_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Alarm Threshold (mA)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->bias[FIBER_LOW_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Warn Threshold (mA)", str_buf);
            
	     cJSON_AddStringToObject(diag_info, "Port", show_name);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tx_pwr2[i]);
	     cJSON_AddStringToObject(diag_info, "Optical Transmit Power (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tx_pwr[FIBER_HIGH_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Alarm Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tx_pwr[FIBER_HIGH_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Warn Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tx_pwr[FIBER_LOW_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Alarm Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->tx_pwr[FIBER_LOW_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Warn Threshold (dBm)", str_buf);

	     cJSON_AddStringToObject(diag_info, "Port", show_name);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->rx_pwr2[i]);
	     cJSON_AddStringToObject(diag_info, "Optical Receive Power (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->rx_pwr[FIBER_HIGH_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Alarm Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->rx_pwr[FIBER_HIGH_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "High Warn Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->rx_pwr[FIBER_LOW_ALARM_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Alarm Threshold (dBm)", str_buf);
	     sal_snprintf(str_buf, sizeof(str_buf), "%.2f", p_db_fiber->rx_pwr[FIBER_LOW_WARN_VALUE]);
	     cJSON_AddStringToObject(diag_info, "Low Warn Threshold (dBm)", str_buf); 
        }
    }

    sal_fprintf(fp, "\n");
    
    return PM_E_NONE;
}
/*end zhush*/

int32
_chsm_port_dump_one_transceiver_info(tbl_port_t* p_db_port, uint8 detail, tbl_iter_args_t *pargs, FILE *fp, uint8 raw)
{
    tbl_fiber_t* p_fiber = NULL;

    if (p_db_port)
    {
        p_fiber = tbl_fiber_get_fiber((tbl_fiber_key_t*)&p_db_port->key);
        if(NULL == p_fiber)
        {
            return PM_E_NONE;
        }

        if (raw)
        {
            tbl_fiber_dump_one(p_fiber, pargs);
        }
        else
        {
            _chsm_fiber_show_one(p_fiber, detail, fp);
        }
    }
    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-05*/
int32
_chsm_port_dump_one_transceiver_info_json(tbl_port_t* p_db_port, uint8 detail, tbl_iter_args_t *pargs, FILE *fp, uint8 raw, cJSON *root)
{
    tbl_fiber_t* p_fiber = NULL;
    cJSON *port_cj = NULL;

    if (p_db_port)
    {
        p_fiber = tbl_fiber_get_fiber((tbl_fiber_key_t*)&p_db_port->key);
        if(NULL == p_fiber)
        {
            return PM_E_NONE;
        }

        if (raw)
        {
            tbl_fiber_dump_one(p_fiber, pargs);
        }
        else
        {
            port_cj = cJSON_CreateObject();
            cJSON_AddItemToObject(root, "Port transceiver info", port_cj);
            _chsm_fiber_show_one_json(p_fiber, detail, fp, port_cj);
        }
    }
    return PM_E_NONE;
}
/*end zhush*/

int32
chsm_port_dump_transceiver(char* ifname, uint16 slot, uint16 port, uint16 sub_port, uint8 detail, cfg_cmd_para_t *para, uint8 raw)
{
    tbl_port_master_t* p_master = tbl_port_get_master();
    tbl_port_t* p_db_port = NULL;
    FILE *fp = NULL;
    int32 is_match = 0;
    tbl_iter_args_t args;
    field_parse_t field;
    uint32 port_idx = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    args.argv[0] = &field;
    args.argv[1] = fp;

    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (GLB_CARD_PORT_NUM_MAX == port)
        {
            is_match = TRUE;
        }
        else
        {
            is_match = (slot == p_db_port->slot_no) 
                    && (port == p_db_port->panel_port_no)
                    && (sub_port == p_db_port->panel_sub_port_no);
        }

        if (0xff == slot || is_match)
        {
            _chsm_port_dump_one_transceiver_info(p_db_port, detail, &args, fp, raw);
        }
    }

    sal_fclose(fp); 

    return PM_E_NONE;
}

/*Added by zhush for bug 52768, 2019-07-05*/
int32
chsm_port_dump_transceiver_json(char* ifname, uint16 slot, uint16 port, uint16 sub_port, uint8 detail, cfg_cmd_para_t *para, uint8 raw)
{
    tbl_port_master_t* p_master = tbl_port_get_master();
    tbl_port_t* p_db_port = NULL;
    FILE *fp = NULL;
    int32 is_match = 0;
    tbl_iter_args_t args;
    field_parse_t field;
    uint32 port_idx = 0;
    cJSON *root = NULL;
    char *out = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    args.argv[0] = &field;
    args.argv[1] = fp;
    root = cJSON_CreateObject();
	
    for (port_idx = 0; port_idx < GLB_CARD_PORT_NUM_MAX; port_idx++)
    {
        p_db_port = p_master->port_array[port_idx];
        if (NULL == p_db_port)
        {
            continue;
        }

        if (GLB_CARD_PORT_NUM_MAX == port)
        {
            is_match = TRUE;
        }
        else
        {
            is_match = (slot == p_db_port->slot_no) 
                    && (port == p_db_port->panel_port_no)
                    && (sub_port == p_db_port->panel_sub_port_no);
        }

        if (0xff == slot || is_match)
        {          
		_chsm_port_dump_one_transceiver_info_json(p_db_port, detail, &args, fp, raw, root);
        }
    }

    out = cJSON_Print(root);
    sal_fprintf(fp, out);
    sal_fprintf(fp,"\n");
    sal_fclose(fp);

    cJSON_Delete(root);
    cJSON_free(out);
    return PM_E_NONE;
}
/*end zhush*/