/****************************************************************************
* $Id$
*  Centec function information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-011
* Reason        : First Create.
****************************************************************************/

#include "laiinc.h"

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_tempfile_define.h"
#include "lcapi.h"
#include "lcm_specific.h"
#include "lcm_mgt.h"
#include "lcm_client.h"
#include "lcm_error.h"
#include "lcm_card.h"
#include "lcm_port.h"
#include "lcm_dpll.h"
#include "lcm_debug.h"
#include "lcm_srv.h"
#include "lcm_lai.h"

#define CTC_LAI_ROUTINE_TIME_INTERVAL              3000
#ifndef _GLB_UML_SYSTEM_
static ctc_task_t *g_ctc_lai_routine_timer = NULL;
#endif

lai_card_notification_t lai_card_noti;
lai_card_notification_t* p_lai_card_noti = NULL;

int lai_card_encode_portname_info(char buffer[], port_info_t *port_info)
{
    int len = 0;
    int unit_len = 0;
    int prefix_idx;

    if (!buffer || !port_info)
    {
        return -1;
    }

    buffer[len++] = PORT_INFO_CONNECTION;
    buffer[len++] = port_info->connection;

    buffer[len++] = PORT_INFO_SUB_CONNECTION;
    buffer[len++] = port_info->sub_connection;

    buffer[len++] = PORT_INFO_PREFIX_NUM;
    buffer[len++] = port_info->prefix_num;

    for(prefix_idx = 0; prefix_idx < port_info->prefix_num; prefix_idx++)
    {
        buffer[len++] = PORT_INFO_PREFIX;
        unit_len = sal_strlen (port_info->prefix[prefix_idx]);
        buffer[len++] = (char)unit_len;
        sal_memcpy(&buffer[len], port_info->prefix[prefix_idx], unit_len);
        len += unit_len;

        buffer[len++] = PORT_INFO_FULL_PREFIX;
        unit_len = sal_strlen (port_info->full_prefix[prefix_idx]);
        buffer[len++] = (char)unit_len;
        sal_memcpy(&buffer[len], port_info->full_prefix[prefix_idx], unit_len);
        len += unit_len;
    }
    buffer[len] = '\0';
    
    return 0;
}

int lai_card_decode_portname_info(char buffer[], port_info_t *port_info)
{
    int len = 0;
    int unit_len = 0;
    int prefix_idx = 0;
    
    if (buffer[len] != PORT_INFO_CONNECTION)
    {
        return -1;
    }
    port_info->connection = buffer[len+1];
    len = len + 2;

    if (buffer[len] != PORT_INFO_SUB_CONNECTION)
    {
        return -1;
    }
    port_info->sub_connection = buffer[len+1];
    len = len + 2;

    if (buffer[len] != PORT_INFO_PREFIX_NUM)
    {
        return -1;
    }
    port_info->prefix_num = buffer[len+1];
    len = len + 2;

    for(prefix_idx = 0; prefix_idx < port_info->prefix_num; prefix_idx++)
    {
        if (buffer[len] != PORT_INFO_PREFIX)
        {
            return -1;
        }  
        unit_len = buffer[len+1];
        sal_memcpy(port_info->prefix[prefix_idx], &buffer[len+2], unit_len);
        port_info->prefix[prefix_idx][unit_len] = '\0';
        len = len + unit_len + 2;

        if (buffer[len] != PORT_INFO_FULL_PREFIX)
        {
            return -1;
        }
        unit_len = buffer[len+1];
        sal_memcpy(port_info->full_prefix[prefix_idx], &buffer[len+2], unit_len);
        port_info->full_prefix[prefix_idx][unit_len] = '\0';
        len = len + unit_len + 2;
    }

    return 0;
}


int lai_card_decode_oem_info(char buffer[], lai_oem_info_t *oem_info)
{
    int len = 0;
    int unit_len = 0;
    char year;
    int old_year = 0;
    int new_year = 0;
    
    /*get company_name*/
    if (buffer[len] != LAI_OEM_TYPE_COMPANY_NAME)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    /* 
    ** year format should like xxxx/xxxx-xxxx
    */
    year = buffer[len+6];
    new_year = sal_atoi(LAI_OEM_CENTEC_YEAR);
    if ('-' == year && unit_len > 9)
    {
        sal_memcpy(&oem_info->company_name, &buffer[len+2], 9);
        oem_info->company_name[9] = '\0';
        old_year = sal_atoi(&oem_info->company_name[5]);
        if (old_year < new_year)
            sal_memcpy(&oem_info->company_name[5], LAI_OEM_CENTEC_YEAR, 4);
        sal_memcpy(&oem_info->company_name[9], &buffer[len+11], unit_len - 9);
    }
    else if (' ' == year && unit_len > 4)
    {
        sal_memcpy(oem_info->company_name, &buffer[len+2], unit_len);
        oem_info->company_name[4] = '\0';
        old_year = sal_atoi(oem_info->company_name);
        if (old_year < new_year)
        {
            sal_memcpy(&oem_info->company_name, LAI_OEM_CENTEC_YEAR, 4);
        }    
        oem_info->company_name[4] = ' ';
    }
    else
    {
        sal_memcpy(oem_info->company_name, &buffer[len+2], unit_len);
    }
    oem_info->company_name[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get package_name*/
    if (buffer[len] != LAI_OEM_TYPE_PACKAGE_NAME)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    sal_memcpy(oem_info->package_name, &buffer[len+2], unit_len);
    oem_info->package_name[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get product_name*/
    if (buffer[len] != LAI_OEM_TYPE_PRODUCT_NAME)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    sal_memcpy(oem_info->product_name, &buffer[len+2], unit_len);
    oem_info->product_name[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get hardware_type*/
    if (buffer[len] != LAI_OEM_TYPE_HARDWARE_TYPE)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    sal_memcpy(oem_info->hardware_type, &buffer[len+2], unit_len);
    oem_info->hardware_type[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get snmp_enterprise_oid*/
    if (buffer[len] != LAI_OEM_TYPE_SNMP_ENTERPRISE_OID)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    sal_memcpy(oem_info->snmp_enterprise_oid, &buffer[len+2], unit_len);
    oem_info->snmp_enterprise_oid[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get chassis_type*/
    if (buffer[len] != LAI_OEM_TYPE_CHASSIS_TYPE)
    {
        return -1;
    }

    unit_len =buffer[len+1];
    sal_memcpy(oem_info->chassis_type, &buffer[len+2], unit_len);
    oem_info->chassis_type[unit_len] = '\0';
    len = len + unit_len + 2;
    /*support OEM external card name*/
    /*get 4sfp card name*/
    if(buffer[len] != LAI_OEM_TYPE_4SFP_CARD_NAME)
    {
        oem_info->four_sfp_name[0] = '\0';
        oem_info->two_sfpp_name[0] = '\0';
        oem_info->four_sfpp_name[0] = '\0';
        return 0;
    }
    unit_len =buffer[len+1];
    sal_memcpy(oem_info->four_sfp_name, &buffer[len+2], unit_len);
    oem_info->four_sfp_name[unit_len] = '\0';
    len = len + unit_len + 2;    

    /*get 2sfpp card name*/
    if(buffer[len] != LAI_OEM_TYPE_2SFPP_CARD_NAME)
    {
        oem_info->four_sfp_name[0] = '\0';
        oem_info->two_sfpp_name[0] = '\0';
        oem_info->four_sfpp_name[0] = '\0';
        return 0;        
    }
    unit_len =buffer[len+1];
    sal_memcpy(oem_info->two_sfpp_name, &buffer[len+2], unit_len);
    oem_info->two_sfpp_name[unit_len] = '\0';
    len = len + unit_len + 2;

    /*get 4sfpp card name*/
    if(buffer[len] != LAI_OEM_TYPE_4SFPP_CARD_NAME)
    {
        oem_info->four_sfp_name[0] = '\0';
        oem_info->two_sfpp_name[0] = '\0';
        oem_info->four_sfpp_name[0] = '\0';
        return 0;        
    }
    unit_len =buffer[len+1];
    sal_memcpy(oem_info->four_sfpp_name, &buffer[len+2], unit_len);
    oem_info->four_sfpp_name[unit_len] = '\0';
    len = len + unit_len + 2;
    return 0;
}


int lai_card_encode_oem_info(char buffer[], lai_oem_info_t *oem_info)
{
    int len = 0;
    int unit_len = 0;

    if (!buffer || !oem_info)
    {
        return -1;
    }

    buffer[len++] = LAI_OEM_TYPE_COMPANY_NAME;
    unit_len = sal_strlen (oem_info->company_name);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->company_name, unit_len);
    len += unit_len;
    
    buffer[len++] = LAI_OEM_TYPE_PACKAGE_NAME;
    unit_len = sal_strlen (oem_info->package_name);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->package_name, unit_len);
    len += unit_len;

    buffer[len++] = LAI_OEM_TYPE_PRODUCT_NAME;
    unit_len = sal_strlen (oem_info->product_name);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->product_name, unit_len);
    len += unit_len;

    buffer[len++] = LAI_OEM_TYPE_HARDWARE_TYPE;
    unit_len = sal_strlen (oem_info->hardware_type);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->hardware_type, unit_len);
    len += unit_len;

    buffer[len++] = LAI_OEM_TYPE_SNMP_ENTERPRISE_OID;
    unit_len = sal_strlen (oem_info->snmp_enterprise_oid);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->snmp_enterprise_oid, unit_len);
    len += unit_len;

    buffer[len++] = LAI_OEM_TYPE_CHASSIS_TYPE;
    unit_len = sal_strlen (oem_info->chassis_type);
    buffer[len++] = (char)unit_len;
    sal_memcpy(&buffer[len], oem_info->chassis_type, unit_len);
    len += unit_len;

    buffer[len] = '\0';
    if(sal_strncmp(oem_info->four_sfp_name, LAI_OEM_DEFAULT_NAME, sizeof(LAI_OEM_DEFAULT_NAME)) != 0)
    {
        buffer[len++] = LAI_OEM_TYPE_4SFP_CARD_NAME;
        unit_len = sal_strlen (oem_info->four_sfp_name);
        buffer[len++] = (char)unit_len;
        sal_memcpy(&buffer[len], oem_info->four_sfp_name, unit_len);
        len += unit_len;
    }

    if(sal_strncmp(oem_info->two_sfpp_name, LAI_OEM_DEFAULT_NAME, sizeof(LAI_OEM_DEFAULT_NAME)) != 0)
    {
        buffer[len++] = LAI_OEM_TYPE_2SFPP_CARD_NAME;
        unit_len = sal_strlen (oem_info->two_sfpp_name);
        buffer[len++] = (char)unit_len;
        sal_memcpy(&buffer[len], oem_info->two_sfpp_name, unit_len);
        len += unit_len;
    }

    if(sal_strncmp(oem_info->four_sfpp_name, LAI_OEM_DEFAULT_NAME, sizeof(LAI_OEM_DEFAULT_NAME)) != 0)
    {
        buffer[len++] = LAI_OEM_TYPE_4SFPP_CARD_NAME;
        unit_len = sal_strlen (oem_info->four_sfpp_name);
        buffer[len++] = (char)unit_len;
        sal_memcpy(&buffer[len], oem_info->four_sfpp_name, unit_len);
        len += unit_len;
    }
    return 0;
}

int lai_card_encode_sw_ver_info(char *buffer, lai_sw_version_info_t *sw_ver_info)
{
    int len = 0;
    int unit_len = 0;

    if (!buffer || !sw_ver_info)
    {
        return -1;
    }

    unit_len = sal_strlen (sw_ver_info->sw_version);
    sal_memcpy(buffer, sw_ver_info->sw_version, unit_len);
    len += unit_len;
   
    buffer[len] = '\0';

    return 0;
}

int lai_get_bootcmd(char *bootcmd)
{
    FILE *fd = NULL;
    int i=0;
    int bootcmdlen;
    char buf[LAI_NAME_INFO_STR_MAX+1];
    
    fd = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(fd)
    {
        while(sal_fgets(buf, 256, fd) != NULL)
        {
            if(sal_strstr(buf, "bootimage") != NULL)
            {
                sal_strncpy(bootcmd, buf, M_FULLPATH_MAX_LEN);
            }
        }
        sal_fclose(fd);
    }

    /* delete '\0' */
    bootcmdlen = strlen(bootcmd);
    while(i<bootcmdlen)
    {
        if(bootcmd[i]=='\n')
            bootcmd[i]='\0';
        i++;
    }

    bootcmd[bootcmdlen] = '\0';

    return LCM_E_SUCCESS;    
}

lai_status_t
ctc_lai_get_card_info(
    _In_ lai_object_id_t oid,
    _Out_ lai_card_info_t* card)
{
    glb_card_t* p_card = NULL; 
    char version[LAI_NAME_INFO_STR_MAX] = {0};

#ifndef _GLB_UML_SYSTEM_
    char buf[LAI_NAME_INFO_STR_MAX];
    lai_oem_info_t oem_info;
    int ret;
#endif
    if(NULL == card)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    p_card= lcm_mgt_get_card();

    sal_memset(card, 0, sizeof(lai_card_info_t));

    card->oid = p_card->phy_slot_no;
    card->slot = p_card->phy_slot_no; 
    card->port_num = p_card->port_num;
    card->flash_size = p_card->flash_size;
    card->sdram_size = p_card->dram_size;
    card->platform_type = p_card->platform_type;
    card->tmpr_low = p_card->tmpr_cfg.low;
    card->tmpr_high = p_card->tmpr_cfg.high;
    card->tmpr_crit = p_card->tmpr_cfg.crit;
    card->tmpr_chip_low_alarm = p_card->tmpr_cfg.chip_low_alarm;
    card->tmpr_chip_low = p_card->tmpr_cfg.chip_low;
    card->tmpr_chip_high = p_card->tmpr_cfg.chip_high;
    card->tmpr_chip_alarm = p_card->tmpr_cfg.chip_alarm;
    card->tmpr_chip_crit = p_card->tmpr_cfg.chip_crit;
    card->profile_type = p_card->profile_type;
    card->board_series = p_card->board_type.series;
    card->board_type = p_card->board_type.type;
    
    if(p_card->card_state == BOARD_READY)
    {
        card->status = LAI_CARD_STATUS_READY;
    }
    else
    {
        card->status = LAI_CARD_STATUS_INITED;
    }

    card->caps |= LAI_CARD_CAPS_SUPPORT_ONIE;
    if(p_card->support_reboot_info)
    {
        card->caps |= LAI_CARD_CAPS_SUPPORT_SAVE_REBOOT_INFO;
    }
    sal_memcpy(card->mac, p_card->sysmac, MAC_ADDR_LEN);

    sal_strncpy(card->serial_number, p_card->serial_no, LAI_NAME_INFO_STR_MAX);
    sal_strncpy(card->bootrom_version, p_card->bootrom_ver, LAI_NAME_INFO_STR_MAX);

    sal_sprintf(version, "%x.%x",(p_card->epld_ver>>4)&0x0f, p_card->epld_ver&0x0f);
    sal_strncpy(card->epld_version, version, LAI_NAME_INFO_STR_MAX);  
    card->epld_status = p_card->epld_status;

    sal_sprintf(version, "%x.%x",(p_card->hw_ver>>4)&0x0f, p_card->hw_ver&0x0f);
    sal_strncpy(card->hardware_version, version, LAI_NAME_INFO_STR_MAX);

    /* Added by liuht for bug44094, 2017-0601 */
    lai_get_bootcmd(card->bootcmd);
    
#ifndef _GLB_UML_SYSTEM_
    sal_memset(buf, 0, sizeof(buf));
    ret = lcm_lai_rx_laiOemInfoGet(buf);
    if(ret)
    {
        sal_strncpy(card->product_name, "E580", LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card->vendor, "Centec Networks", LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card->package_name, "OSP System", LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card->hardware_name, "E580", LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card->snmp_enterprise_oid, "10027", LAI_NAME_INFO_STR_MAX);
    }
    else
    {
        sal_memset(&oem_info,0, sizeof(oem_info));
        ret = lai_card_decode_oem_info(buf, &oem_info);
        if(ret)
        {
            sal_strncpy(card->product_name, "E580", LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->vendor, "Centec Networks", LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->package_name, "OSP System", LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->hardware_name, "E580", LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->snmp_enterprise_oid, "10027", LAI_NAME_INFO_STR_MAX);
        }
        else
        {
            sal_strncpy(card->product_name, oem_info.product_name, LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->vendor, oem_info.company_name, LAI_NAME_INFO_STR_MAX);
            sal_strncpy(card->package_name, oem_info.package_name, LAI_NAME_INFO_STR_MAX);
            if((card->board_series == GLB_SERIES_E350)&&(card->board_type == GLB_BOARD_E350_65DV1))
            {    
                if(p_card->daughter_card_type == E350_65DV1_4SFP_8T)
                {
                    sal_strncpy(card->hardware_name, "CZ.T-T65D-36Q", MAX_HARDWARE_VER_LEN);
                }
                else if(p_card->daughter_card_type == E350_65DV1_6SFP_PLUS)
                {
                    sal_strncpy(card->hardware_name, "CZ.T-T65D-30W", MAX_HARDWARE_VER_LEN);
                }
                else
                {
                    sal_strncpy(card->hardware_name, "CZ.T-T65D-24Q", MAX_HARDWARE_VER_LEN);
                }

                
                //sal_strncpy(card->cst_company, card->vendor, LAI_NAME_INFO_STR_MAX);   /* maybe large than 64 chars */
                sal_strncpy(card->cst_company, "2019 China Railway Signal & Communication Co., Ltd", LAI_NAME_INFO_STR_MAX);   /* maybe large than 64 chars */
                sal_strncpy(card->cst_hw_type, oem_info.hardware_type, LAI_CONFIG_INFO_STR_MAX);
                sal_strncpy(card->cst_hw_info, p_card->daughter_card_name, DAU_CARD_NAME);
                sal_strncpy(card->cst_hw_ver, p_card->custom_hw_ver, CUSTOM_HW_VERSION);
                sal_strncpy(card->cst_sw_ver, p_card->custom_sw_ver, CUSTOM_SW_VERSION);
                card->is_cst = TRUE;
            }
            else if((card->board_series == GLB_SERIES_P580)&&
                ((card->board_type == GLB_BOARD_T65C_8T64X6Q) || (card->board_type == GLB_BOARD_P580_32X6Q)))
            {    
                sal_strncpy(card->hardware_name, oem_info.hardware_type, LAI_NAME_INFO_STR_MAX);
                //sal_strncpy(card->cst_company, card->vendor, LAI_NAME_INFO_STR_MAX);   /* maybe large than 64 chars */
                sal_strncpy(card->cst_company, "2019 China Railway Signal & Communication Co., Ltd", LAI_NAME_INFO_STR_MAX);   /* maybe large than 64 chars */
                sal_strncpy(card->cst_hw_type, oem_info.hardware_type, LAI_CONFIG_INFO_STR_MAX);
                sal_strncpy(card->cst_hw_info, p_card->daughter_card_name, DAU_CARD_NAME);
                sal_strncpy(card->cst_hw_ver, p_card->custom_hw_ver, CUSTOM_HW_VERSION);
                sal_strncpy(card->cst_sw_ver, p_card->custom_sw_ver, CUSTOM_SW_VERSION);
                card->is_cst = TRUE;
            }
            else
            {
                sal_strncpy(card->hardware_name, oem_info.hardware_type, LAI_NAME_INFO_STR_MAX);
            }
            /* support OEM custom-made, for customer */
            if(sal_strstr(oem_info.company_name,"China Railway Signal"))
            {
                sal_strncpy(card->hardware_name, oem_info.hardware_type, LAI_NAME_INFO_STR_MAX);
                sal_strncpy(card->cst_company, "2019 China Railway Signal & Communication Co., Ltd", LAI_NAME_INFO_STR_MAX);   /* maybe large than 64 chars */
                sal_strncpy(card->cst_hw_type, oem_info.hardware_type, LAI_CONFIG_INFO_STR_MAX);
                sal_strncpy(card->cst_hw_info, p_card->daughter_card_name, DAU_CARD_NAME);
                sal_strncpy(card->cst_hw_ver, p_card->custom_hw_ver, CUSTOM_HW_VERSION);
                sal_strncpy(card->cst_sw_ver, p_card->custom_sw_ver, CUSTOM_SW_VERSION);
                card->is_cst = TRUE;
            }            
            sal_strncpy(card->snmp_enterprise_oid, oem_info.snmp_enterprise_oid, LAI_NAME_INFO_STR_MAX);
        }
    }
#else
    card->flash_size = 99;
    card->sdram_size = 88;
    sal_strcpy(card->epld_version,"epld0.1");
    sal_strcpy(card->hardware_version, "uml1.0");
    sal_strcpy(card->product_name,"CNOS");
    sal_strcpy(card->vendor,"Centec Networks (suzhou) Co., Ltd.");
    sal_strcpy(card->package_name,"OSP Software");
    sal_strcpy(card->hardware_name,"UML");
    sal_strcpy(card->country_code,"086");
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_set_oem_info(
    _In_ lai_object_id_t oid,
    _In_ lai_oem_info_t* oem_info)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    char buf[LAI_NAME_INFO_STR_MAX];
    int ret = 0;

    p_card= lcm_mgt_get_card();
    if (oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    sal_memset(buf, 0, sizeof(buf));
    ret = lai_card_encode_oem_info(buf, oem_info);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    ret = lcm_lai_rx_laiOemInfoSet(buf);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_set_sw_ver_info(
    _In_ lai_object_id_t oid,
    _In_ lai_sw_version_info_t* sw_vers_info)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    char buf[LAI_MAX_SWVER_LEN + 1];
    int ret = 0;

    p_card= lcm_mgt_get_card();
    if (oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    sal_memset(buf, 0, sizeof(buf));
    ret = lai_card_encode_sw_ver_info(buf, sw_vers_info);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    ret = lcm_lai_rx_laiSwVerInfoSet(buf);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_set_ifname_info(
    _In_ lai_object_id_t oid,
    _In_ lai_portname_info_t* info)
{
    char buf[LAI_NAME_INFO_STR_MAX];
    port_info_t portname_info;
    int ret = 0;
    int prefix_idx = 0;
    int unit_len = 0;

    if(NULL == info)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memset(&portname_info, 0, sizeof(portname_info));
    portname_info.connection = info->connection;
    portname_info.sub_connection = info->sub_connection;
    portname_info.prefix_num = info->prefix_num;

    for(prefix_idx = 0; prefix_idx < info->prefix_num; prefix_idx++)
    {
        unit_len = sal_strlen (info->prefix[prefix_idx]);
        sal_memcpy(portname_info.prefix[prefix_idx], info->prefix[prefix_idx], unit_len);

        unit_len = sal_strlen (info->full_prefix[prefix_idx]);
        sal_memcpy(portname_info.full_prefix[prefix_idx], info->full_prefix[prefix_idx], unit_len);
    }
    
    ret = lai_card_encode_portname_info(buf, &portname_info);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }

    ret = lcm_lai_set_PortInfoSet(buf);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    return LAI_STATUS_SUCCESS;
}

extern int32
lcm_lai_set_Configbootcmd_onie(char* cmd);
extern int32
lcm_lai_set_Configbootcmd_normal(char* cmd);

lai_status_t
ctc_lai_set_bootcmd(
    _In_ lai_object_id_t oid,
    _In_ char* cmd)
{
#ifndef _GLB_UML_SYSTEM_
#ifndef _CTC_X86_64_
    int ret = 0;
    glb_card_t* p_card = NULL; 

    p_card= lcm_mgt_get_card();
    
    if(NULL == cmd)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    if(p_card->platform_type)
    {
        ret = lcm_lai_set_Configbootcmd_onie(cmd);
    }
    else
    {
        ret = lcm_lai_set_Configbootcmd_normal(cmd);
    }
    
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
#endif
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_set_stmmode(
    _In_ lai_object_id_t oid,
    _In_ uint32_t mode)
{
//#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    ret = lcm_lai_SetStmMode(mode);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
//#endif    
    return LAI_STATUS_SUCCESS;
}



lai_status_t
ctc_lai_get_reboot_info(
    _In_ lai_object_id_t oid,
    _In_ lai_reboot_info_t* reboot_info)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    ret = lcm_lai_get_lcRebootInfo(&reboot_info);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif    
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_reset_reboot_info(
    _In_ lai_object_id_t oid)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    ret = lcm_lai_ResetRebootInfo();
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif    
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_update_bootrom(
    _In_ lai_object_id_t oid,
    _In_ char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    if(NULL == path)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    ret = lcm_lai_Updatebootrom(path);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_update_phy_firmware(
    _In_ lai_object_id_t oid,
    _In_ int32_t phy_port_idx,
    _In_ char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    if(NULL == path)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }

    ret = lcm_lai_Updatephyfirmware(phy_port_idx, path);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_set_fan_speed(
    _In_ lai_object_id_t oid,
    _In_ int32_t bottom, 
    _In_ int32_t low,
    _In_ int32_t high,
    _In_ int32_t full)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    int32 ret = 0;

    p_card= lcm_mgt_get_card();

    if(oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    
    ret = lcm_lai_set_fan_speed_rate(bottom, low, high, full);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_set_poe_enable(
    _In_ lai_object_id_t oid,
    _In_ char enable
    )
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    int32 ret = 0;

    p_card= lcm_mgt_get_card();

    if(oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    
    ret = lcm_lai_set_poe_enable(enable);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_set_poe_power_max(
    _In_ lai_object_id_t oid,
    _In_ uint32_t power_max
    )
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    int32 ret = 0;

    p_card= lcm_mgt_get_card();

    if(oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    
    ret = lcm_lai_set_poe_power_max(power_max);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    
#endif
    return LAI_STATUS_SUCCESS;
}

lai_status_t
ctc_lai_reboot(
    _In_ lai_object_id_t oid,
    _In_ char* cmd)
{
    lcm_reload();

    return LAI_STATUS_SUCCESS;
}
 
lai_status_t
ctc_lai_update_epld(
    _In_ lai_object_id_t oid,
    _In_ char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;

    if(NULL == path)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    
    ret = lcm_lai_UpdateEpld(path);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
#endif
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_set_tmpr_info(
    _In_ lai_object_id_t oid,
    _In_ int32_t low, 
    _In_ int32_t high, 
    _In_ int32_t critical)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL; 
    int32 ret = 0;

    p_card= lcm_mgt_get_card();

    if(oid != p_card->phy_slot_no)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    
    ret = lcm_lai_rx_laiTmprThreshold(low, high, critical);
    if(ret)
    {
        return LAI_STATUS_FAILURE;
    }
    
#endif
    return LAI_STATUS_SUCCESS;
}


int32
ctc_lai_process_sys_led_event(uint8 type, uint8 stat)
{
#ifndef _GLB_UML_SYSTEM_
    static uint8 event[SYS_EVENT_MAX];
    static uint8 old_alarm = 0xff;
    uint8 i, alarm = 0;
    
    event[type] = stat;
    for (i=0; i<SYS_EVENT_MAX; i++)
    {
        if(event[i] == SYS_LED_ALARM)
        {
            alarm = 1;
            break;
        }
    }
    if (old_alarm != alarm)
    {
        if(alarm == 1)
        {   
            lcm_lai_rx_laiLedSet(E_SYS_LED, E_LED_SYS_ALARM);
            lcm_lai_rx_laiLedSet(E_SYS_LED, E_LED_SYS_ABNORMAL);
        }
        else
        {
            lcm_lai_rx_laiLedSet(E_SYS_LED, E_LED_SYS_RUN);
            lcm_lai_rx_laiLedSet(E_SYS_LED, E_LED_SYS_NO_ALARM);
        }
        old_alarm = alarm;
    }
#endif
    return 0;
}


int32
ctc_lai_manage_tmpr_status(glb_card_t* p_card)
{
    int32 ret = 0;
    uint8 tmpr_idx;    
    int32 sensor_value_high=-255, chip_value_high=-255;
    int32 sensor_value_low=255, chip_value_low=255;
    uint8 sensor_alarm=0, chip_alarm=0;
    uint8 sensor_critical=0, chip_critical=0;
    static uint8 sensor_alarm_orig=0, chip_alarm_orig=0;
    static uint8 sensor_critical_orig=0, chip_critical_orig=0;

    /*1. store real-time value, and the highest value */        
    for (tmpr_idx=0; tmpr_idx < p_card->sensor_num; tmpr_idx++)
    {

        /*1.2 store highest and lowest sensor value*/
        if ((p_card->tmpr_status[tmpr_idx].pos != SENSOR_CHIP0)
            && (p_card->tmpr_status[tmpr_idx].pos != SENSOR_CHIP1))
        {
            if(sensor_value_high < p_card->tmpr_status[tmpr_idx].tmpr_val)
            {
                sensor_value_high = p_card->tmpr_status[tmpr_idx].tmpr_val;
            }
            if(sensor_value_low > p_card->tmpr_status[tmpr_idx].tmpr_val)
            {
                sensor_value_low = p_card->tmpr_status[tmpr_idx].tmpr_val;
            }
        }
        else
        {
            if(chip_value_high < p_card->tmpr_status[tmpr_idx].tmpr_val)
            {
                chip_value_high = p_card->tmpr_status[tmpr_idx].tmpr_val;
            }
            if(chip_value_low > p_card->tmpr_status[tmpr_idx].tmpr_val)
            {
                chip_value_low = p_card->tmpr_status[tmpr_idx].tmpr_val;
            }
        }
    }
        
    /*2. check current sensor is alarm or normal*/
    if (sensor_alarm_orig == 0)
    {
        if ((sensor_value_high >= p_card->tmpr_cfg.high)
          ||(sensor_value_low <= p_card->tmpr_cfg.low))
        {
            sensor_alarm = 1;
        }
        else
        {
            sensor_alarm = 0;
        }
    }
    else
    {
        /*bug 32974, add low temperature alarm to normal process. jqiu 2015-04-25*/
        if((sensor_value_high <= p_card->tmpr_cfg.high - BOARD_TMPR_HYST)
            &&(sensor_value_low >= p_card->tmpr_cfg.low + BOARD_TMPR_HYST))
        {
            sensor_alarm = 0;
        }
        else
        {
            sensor_alarm = 1;
        }
    }
    /*3. check current sensor is critical */
    if (sensor_value_high + BOARD_TMPR_HYST >= p_card->tmpr_cfg.crit)
    {
        sensor_critical = 1;
    }
    else
    {
        sensor_critical = 0;
    }
    /*4. check current chip tempr is alarm or normal*/
    if (chip_alarm_orig == 0)
    {
        if((chip_value_high >= GG_CHIP_TMPR_ALARM)
            ||(chip_value_low <= GG_CHIP_TMPR_LOW_ALARM))
        {
            chip_alarm = 1;
        }
        else
        {
            chip_alarm = 0;
        }
    }
    else
    {
        if((chip_value_high <= GG_CHIP_TMPR_ALARM-BOARD_TMPR_HYST)
            &&(chip_value_low >= GG_CHIP_TMPR_LOW_ALARM+BOARD_TMPR_HYST))
        {
            chip_alarm = 0;
        }
        else
        {
            chip_alarm = 1;
        }
    }
    /*5. check current chip tempr is critical */
    if(chip_value_high+GG_CHIP_TMPR_HYST >= GG_CHIP_TMPR_CRITICAL)
    {
        chip_critical = 1;
    }
    else
    {
        chip_critical = 0;
    }
    
    /* 6. when board sensor alarm change, log it.*/
    if (sensor_alarm != sensor_alarm_orig)
    {
        sensor_alarm_orig = sensor_alarm;
    }

    /* 7. when board sensor critical change, log it.*/
    if (sensor_critical != sensor_critical_orig)
    {
        sensor_critical_orig = sensor_critical;
    }

    /* 8. when chip sensor alarm change, log it.*/
    if (chip_alarm != chip_alarm_orig)
    {
        chip_alarm_orig = chip_alarm;
    }
    
    /* 9. when chip sensor critical change, log it.*/
    if (chip_critical != chip_critical_orig)
    {
        chip_critical_orig = chip_critical;
    }

    /*10. after check all temperature, process alarm event*/
    if ((chip_alarm) || (sensor_alarm))
    {
        ret = ctc_lai_process_sys_led_event(SYS_EVENT_TEMP, SYS_LED_ALARM);
    }
    else
    {
        ret = ctc_lai_process_sys_led_event(SYS_EVENT_TEMP, SYS_LED_NORMAL);
    }
    
    return ret;
}

/*fan status management.*/
int32
ctc_lai_manage_fan_status(glb_card_t* p_card)
{
#ifndef _GLB_UML_SYSTEM_
    uint8 fan_module, i, alarm=0;
    int32 ret = 0;
    glb_led_stat_t fan_led_stat=E_LED_SYS_NO_ALARM;

    for (fan_module=0; fan_module < p_card->fan_module_num; fan_module++)
    {
        /*4. check fan led event*/
        if(FAN_PRESENT != p_card->fan_status[fan_module].present)
        {
            alarm = 1;
            fan_led_stat = E_LED_SYS_ABSENT;
        }
        else
        {
            fan_led_stat = E_LED_SYS_NO_ALARM;
            for(i=0; i<p_card->fan_status[fan_module].num; i++)
            {
                if((p_card->fan_status[fan_module].status & (1<<i)) == 0)
                {
                    alarm = 1;
                    fan_led_stat = E_LED_SYS_ALARM;
                }
            }
        }
    }

    /*For many fan event, just open led one time. */
    lcm_lai_rx_laiLedSet(E_FAN_LED, fan_led_stat);
    
    /*need to set system alarm flag*/
    if(alarm)
    {
        ret = ctc_lai_process_sys_led_event(SYS_EVENT_FAN, SYS_LED_ALARM);
    }
    else
    {
        ret = ctc_lai_process_sys_led_event(SYS_EVENT_FAN, SYS_LED_NORMAL);
    }
    (void)ret;
#endif
    return 0;
}


int32
ctc_lai_manage_psu_status(glb_card_t *p_card)
{
#ifndef _GLB_UML_SYSTEM_
    uint8 psu_idx;
    uint8 absent[MAX_PSU_NUM] = {0},alarm[MAX_PSU_NUM]={0};
    uint8 sys_alarm = 0;

    for (psu_idx = 0; psu_idx < p_card->psu_module_num; psu_idx++)
    {
        if(PSU_SHOW_FULL_STATUS == p_card->psu_status[psu_idx].psu_type)
        {
            /* modified by liuht for bug44462, 2017-0706 */
            /* update system led and psu led only when psu is present */
            if(!p_card->psu_status[psu_idx].psu_absent) 
            {
                absent[psu_idx] = 1;
                /* 2.3 check power led event */
                if(p_card->psu_status[psu_idx].psu_work_status || ((p_card->psu_status[psu_idx].fixed_status != 1) && p_card->psu_status[psu_idx].psu_alert ))
                {
                    alarm[psu_idx] = 1;
                }
                else
                {
                    alarm[psu_idx] = 0;
                }
            }
            else
            {
                absent[psu_idx] = 0;
                alarm[psu_idx] = 0;
            }
        }
        else if(PSU_SHOW_GOOD_SIG == p_card->psu_status[psu_idx].psu_type)
        {
            /* update system led and psu led only when psu is present */
            if(!p_card->psu_status[psu_idx].psu_absent) 
            {            
                absent[psu_idx] = 1;
                /* 1. if work status change, need log it */
                if(p_card->psu_status[psu_idx].psu_work_status)
                {
                    alarm[psu_idx] = 1;
                }
                else
                {
                    alarm[psu_idx] = 0;
                }
            }
            else
            {
                absent[psu_idx] = 0;
                alarm[psu_idx] = 0;
            }            
        }
    }

    for (psu_idx = 0; psu_idx < p_card->psu_module_num; psu_idx++)
    {
        if(p_card->psu_led_mode == GLB_PSU_LED_MODE_MULTI)
        {
            if(absent[psu_idx] == 0)
            {
                /* for bug 45771, off psu led when psu absent */
                if(psu_idx == 0)
                {
                    lcm_lai_rx_laiLedSet(E_PSU1_LED, E_LED_SYS_ABSENT);
                }
                else
                {
                    lcm_lai_rx_laiLedSet(E_PSU2_LED, E_LED_SYS_ABSENT);
                }
            }
            else
            {
                if(psu_idx == 0)
                {
                    lcm_lai_rx_laiLedSet(E_PSU1_LED, alarm[psu_idx]?E_LED_SYS_ALARM:E_LED_SYS_NO_ALARM);
                }
                else
                {
                    lcm_lai_rx_laiLedSet(E_PSU2_LED, alarm[psu_idx]?E_LED_SYS_ALARM:E_LED_SYS_NO_ALARM);
                }
            }
        }
    }

    for (psu_idx = 0; psu_idx < MAX_PSU_NUM; psu_idx++)
    {
        if(alarm[psu_idx])
        {
            sys_alarm = 1;
            break;
        }
    }
    if(sys_alarm)
    {
        if(p_card->psu_led_mode == GLB_PSU_LED_MODE_ONE)/* when only have one psu led, it act like the sysled */
            lcm_lai_rx_laiLedSet(E_PSU1_LED, E_LED_SYS_ALARM);
        ctc_lai_process_sys_led_event(SYS_EVENT_PSU, SYS_LED_ALARM);
    }
    else
    {
        if(p_card->psu_led_mode == GLB_PSU_LED_MODE_ONE)/* when only have one psu led, it act like the sysled */
            lcm_lai_rx_laiLedSet(E_PSU1_LED, E_LED_SYS_NO_ALARM);
        ctc_lai_process_sys_led_event(SYS_EVENT_PSU, SYS_LED_NORMAL);
    }

    /*4. store real-time value */
    //TODU, DATA update cb here    
#endif
    return LAI_STATUS_SUCCESS;
}

/*polling temperature, fan, psu status, and config alarm*/
static void
ctc_lai_routine_timer_handle(void *p_data)
{
    glb_card_t* p_card = NULL; 

    if(lcm_mgt_get_bypass_timers())
        return;

    p_card= lcm_mgt_get_card();
    if(NULL == p_card)
    {
        return;
    }
    p_card->card_state = BOARD_READY;
    
#ifndef _GLB_UML_SYSTEM_
    static uint32 count = 0;

    g_ctc_lai_routine_timer = ctc_task_add_timer_msec(TASK_PRI_NORMAL, ctc_lai_routine_timer_handle,
        NULL, CTC_LAI_ROUTINE_TIME_INTERVAL);
    /*For update interval from 10s to 3s, here read temperature every third times to
    reduce i2c access freq. 3s interval guarantee fan pull out/in event can be checked. PSU is epld access. so just
    update to 3s. fan is i2c access, when fan is normal, still need reduce i2c access, this will be done in lcm.*/
    
        
    if(count % 3 == 0)
    {
       ctc_lai_manage_tmpr_status(p_card);
           
    }
    ctc_lai_manage_fan_status(p_card);
    ctc_lai_manage_psu_status(p_card);
    count++;

#endif
    return;

}



lai_status_t
ctc_card_init()
{
    extern int32 lc_start();
    extern int32 lcm_mgt_init();
    /*For center forward system, just call hagt function to init hagt db*/
    /*For distribute system, these moudles will be start on linecard*/
    lc_start();
    lcm_mgt_init();
    ctc_lai_process_sys_led_event(SYS_EVENT_RUN, SYS_LED_NORMAL);

    ctc_lai_routine_timer_handle(NULL);

    return LAI_STATUS_SUCCESS;
}


static lai_card_api_t      g_lai_card_api_func = {
    .get_card_info      = ctc_lai_get_card_info,
    .set_oem_info       = ctc_lai_set_oem_info,
    .set_tmpr_info      = ctc_lai_set_tmpr_info,
    .set_bootcmd        = ctc_lai_set_bootcmd,
    .set_stmmode        = ctc_lai_set_stmmode,
    .set_ifname_info    = ctc_lai_set_ifname_info,
    .get_reboot_info    = ctc_lai_get_reboot_info,
    .reset_reboot_info  = ctc_lai_reset_reboot_info,
    .update_epld        = ctc_lai_update_epld,
    .update_bootrom     = ctc_lai_update_bootrom,
    .reboot             = ctc_lai_reboot,
    .update_phy_firmware = ctc_lai_update_phy_firmware,
    .set_fan_speed      = ctc_lai_set_fan_speed,
    .set_poe_enable      = ctc_lai_set_poe_enable,
    .set_poe_power_max      = ctc_lai_set_poe_power_max,
    .set_sw_version_info = ctc_lai_set_sw_ver_info,
    .init               = ctc_card_init,
};

static ctc_lai_api_reg_info_t g_card_api_reg_info = {
    .id         = LAI_API_CARD,
    .init_func  = NULL,
    .exit_func  = NULL,
    .api_method_table = &g_lai_card_api_func,
    .init_status      = INITIALIZED,
};


lai_status_t
ctc_lai_card_noti_init(lai_card_notification_t* noti_cb)
{
    p_lai_card_noti = &lai_card_noti;

    if(NULL ==  noti_cb)
    {
        return LAI_STATUS_INVALID_PARAMETER;
    }
    sal_memcpy(&lai_card_noti, noti_cb, sizeof(lai_card_noti));
    
    return LAI_STATUS_SUCCESS;
}


lai_status_t
ctc_lai_card_init(lai_card_notification_t* noti_cb)
{
    lai_api_reg_register_fn(&g_card_api_reg_info);

    ctc_lai_card_noti_init(noti_cb);
    
    return LAI_STATUS_SUCCESS;
}


