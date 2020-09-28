/****************************************************************************
* $Id$
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht
* Date          : 2015-01-27
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal_common.h"
#include "lcm_specific.h"
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "lcm_debug.h"
#include "lcm_error.h"
#include "lcm_log.h"
#include "lcm_card.h"
#include "eeprom_api.h"
#include "sysenv_api.h"
#include "lcm_mgt.h"
#include "ctc_api.h"
#include "ctc_chip.h"
#include "ctc_port.h"
#include "glb_distribute_system_define.h"
#include "glb_if_define.h"
#ifndef CNOS_OS
#include "oem_info.h"
#endif
int lcm_oem_decode_port_info(char buffer[], port_info_t *port_info)
{
    int len = 0;
    int unit_len = 0;
    int prefix_idx;
    
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

#define LCM_COMMON_40G_SPLIT_NUM   4
#define LCM_COMMON_10G_MERGE_NUM   4
#define LCM_COMMON_100G_SPLIT_NUM   4
#define LCM_COMMON_100G_SPLIT_NUM_2 2 /* this is for split 50G */

bool lcm_def_port_prefix = TRUE;
//lcm_card_port_split_info_t* glb_p_port_split_info;
static port_info_t glb_port_info;

static int32
lcm_check_port_validity(glb_card_t* p_card, char *temp, uint32 temp_panel_slot, uint32 temp_panel_port)
{
    uint8 port_speed_real;
    int ret = 0;

    /* 1. check validity of panel slot num and panel port num */
    if((temp_panel_port > p_card->panel_port_num) || (temp_panel_port < 1))
    {
        LCM_LOG_ERR("%% Invalid panel port num: %s.\n", temp);
        return -1;
    }
    if (temp_panel_slot != p_card->pp_ppt[temp_panel_port-1]->pslt_id)
    {
        LCM_LOG_ERR("%% Invalid panel slot num: %s.\n", temp);
        return -1;
    }
    
    port_speed_real = p_card->pp_ppt[temp_panel_port-1]->ppt_speed_real;
    /* 2. check validity of port prefix name */
    if(GLB_SUPPORT_SPEED_100G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_100G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_100G)-1]));
    }
    else if(GLB_SUPPORT_SPEED_40G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_40G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_40G)-1]));
    }
    else if(GLB_SUPPORT_SPEED_10G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_10G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_10G)-1]));
    }
    else if(GLB_SUPPORT_SPEED_25G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_25G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_25G)-1]));
    }
    else if(GLB_SUPPORT_SPEED_50G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_50G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_50G)-1]));
    }    
    else if(GLB_SUPPORT_SPEED_1G & port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_1G)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_1G)-1]));
    }
    else if(GLB_SUPPORT_SPEED_100M& port_speed_real)
    {
        ret = sal_strncmp(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_100M)-1], temp, 
            sal_strlen(glb_port_info.full_prefix[ffs(GLB_SUPPORT_SPEED_100M)-1]));
    }
    else
    {
        return -1;
    }
    if(ret)
    {
        LCM_LOG_ERR("%% Invalid port prefix: %s.\n", temp);
        return -1;
    }
    return 0;
}

int32 lcm_parse_numchg_cli(glb_card_t* p_card, char *cli_str, uint32 *panel_slot, uint32 *panel_port, uint32 *speed)
{
    char *p = NULL;
    uint32 temp_panel_slot, temp_panel_port, temp_speed;
    char *temp = NULL;

    /* 1. get panel slot num and panel port num */
    p = cli_str + sal_strlen("split interface");
    temp = p;
    while (*p && !sal_isdigit(*p))
    {
        p++;
    }
    temp++;
    if (!*p)
    {
        LCM_LOG_ERR("%% Error slot in switch cli: %s.\n", cli_str);
        return -1;
    }
    temp_panel_slot = atoi(p);
    while (*p && sal_isdigit(*p))
    {
        p++;
    }
    if (!*p)
    {
        LCM_LOG_ERR("%% Error port in switch cli: %s.\n", cli_str);
        return -1;
    }
    //goto panel_port_no
    p++;
    temp_panel_port = atoi(p);
    if (sal_strstr(cli_str, "10giga"))
    {
        temp_speed = 10;
    }
    else if (sal_strstr(cli_str, "40giga"))
    {
        temp_speed = 40;
    }
    else if (sal_strstr(cli_str, "50giga"))
    {
        temp_speed = 50;
    }
    else if (sal_strstr(cli_str, "25giga"))
    {
        temp_speed = 25;
    }    
    else
    {
        LCM_LOG_ERR("%% Error speed in split cli: %s.\n", cli_str);
        return -1;
    }
    /* 2. check port validity */
    if(lcm_check_port_validity(p_card, temp, temp_panel_slot, temp_panel_port))
    {
        return -1;
    }

    *panel_slot = temp_panel_slot;
    *panel_port = temp_panel_port;
    *speed = temp_speed;
    
    return 0;
}

int32 lcm_parse_switch_cli(glb_card_t* p_card, char *cli_str, uint32 *panel_slot, uint32 *panel_port, char type[32])///TODO: need modify
{
    char *p = NULL;
    uint32 temp_panel_slot, temp_panel_port;
    char *temp = NULL;

    /* 1. get panel slot num and panel port num */
    p = cli_str + sal_strlen("switch interface");
    temp = p;
    while (*p && !sal_isdigit(*p))
    {
        p++;
    }
    temp++;
    if (!*p)
    {
        LCM_LOG_ERR("%% Error slot in switch cli: %s.\n", cli_str);
        return -1;
    }
    temp_panel_slot = atoi(p);
    while (*p && sal_isdigit(*p))
    {
        p++;
    }
    if (!*p)
    {
        LCM_LOG_ERR("%% Error port in switch cli: %s.\n", cli_str);
        return -1;
    }
    //goto panel_port_no
    p++;
    temp_panel_port = atoi(p);

    if (sal_strstr(cli_str, "qsfp"))
    {
        sal_snprintf(type, 32, "%s", "qsfp");
    }
    else if (sal_strstr(cli_str, "sfp"))
    {
        sal_snprintf(type, 32, "%s", "sfp");
    }
    else
    {
        LCM_LOG_ERR("%% Error speed in switch cli: %s.\n", cli_str);
        return -1;
    }

    /* 2. check port validity */
    if(lcm_check_port_validity(p_card, temp, temp_panel_slot, temp_panel_port))
    {
        return -1;
    }

    *panel_slot = temp_panel_slot;
    *panel_port = temp_panel_port;
        
    return 0;
}

int32 lcm_get_port_info(glb_card_t* p_card)
 {

    FILE *port_tmp_fp = NULL;
    int prefix_idx;
    char buf[MAX_PORT_INFO_LEN+1];
    int ret = 0;

    port_tmp_fp = sal_fopen(GLB_PORT_CLI_INFO_FILE, "w");
    if (port_tmp_fp == NULL)
    {
        LCM_LOG_ERR("Open the interface info file failed!\n");
        return -1;
    }

    /* 1. get interface prefix and connection from eeprom(real board) or flash(uml) */
    sal_memset(buf, 0, MAX_PORT_INFO_LEN);    
#ifndef _GLB_UML_SYSTEM_
    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_PORT_INFO]);
        //jqiu modify. if board doesn't define this, use default.
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR("SYSENV port info is NULL!\n");
        }
        else
        {
            ret = sysenv_get_port_info((uint8*)buf, p_sysenv_info);
            if(ret < 0)
            {
                LCM_LOG_ERR("Get SYSENV port info fail!\n");
            }
        }
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_PORT_INFO]);
        //jqiu modify. if board doesn't define this, use default.
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR("EEPROM port info is NULL!\n");
        }
        else
        {
            ret = eeprom_get_port_info((uint8*)buf, p_eeprom_info);
            if(ret < 0)
            {
                LCM_LOG_ERR("Get EEPROM port info fail!\n");
            }
        }
    }
#else
    FILE *port_flash_fd = NULL;
    port_flash_fd = sal_fopen(GLB_UML_PORT_INFO_FILE, "r");
    if(!port_flash_fd)
    {
        LCM_LOG_ERR("Open the interface info file for read failed!");
    }
    else
    {
        sal_fgets(buf, MAX_PORT_INFO_LEN, port_flash_fd);
        sal_fclose(port_flash_fd);
    }
#endif

    /* 2. parse interface prefix and connection from original code */
    ret = lcm_oem_decode_port_info(buf, &glb_port_info);
    if(ret < 0)
    {
        /* default interface prefix and connection */
        glb_port_info.connection = '-';
        glb_port_info.sub_connection = '/';
        glb_port_info.prefix_num = GLB_PHT_PORT_TYPE_MAX;
        for(prefix_idx = 0; prefix_idx < glb_port_info.prefix_num; prefix_idx++)
        {        
            sal_strncpy(glb_port_info.prefix[prefix_idx], GLB_DEF_PHY_PORT_PREFIX, sal_strlen(GLB_DEF_PHY_PORT_PREFIX));
            sal_strncpy(glb_port_info.full_prefix[prefix_idx], GLB_DEF_PHY_PORT_PREFIX, sal_strlen(GLB_DEF_PHY_PORT_PREFIX));
        }
    }
    /* 3. save interface prefix and connection to file */
    sal_fprintf(port_tmp_fp, "port_connection %c sub_port_connection %c \n", 
        glb_port_info.connection, glb_port_info.sub_connection);
    
    for(prefix_idx = 0; prefix_idx < glb_port_info.prefix_num; prefix_idx++)
    {        
        if (!sal_strcasecmp(glb_port_info.prefix[prefix_idx], GLB_DEF_PHY_PORT_PREFIX))
        {
            continue;
        }
        sal_fprintf(port_tmp_fp, "prefix %s full_prefix %s \n", 
            glb_port_info.prefix[prefix_idx], glb_port_info.full_prefix[prefix_idx]);
    }
    sal_fclose(port_tmp_fp);   

    return 0;
 }

/*add for bug 49601*/
int32
lcm_common_stack_save_port_info_file(FILE *port_info_fd)
{
    int i = 0;
    FILE *startup_config_fp = NULL;
    uint32 panel_slot_no = 0;
    char buf[256];     
    char board_type[32] = {0};

    startup_config_fp = sal_fopen(GLB_STARTUP_CONFIG_FILE_PATH, "r");
    if (startup_config_fp)
    {
        while (sal_fgets(buf, 128, startup_config_fp))
        {
            if (2 != sal_sscanf(buf, "stack-switch %d type %s", &panel_slot_no, board_type))
            {
                continue;
            }
            if (!sal_strncmp (board_type, "20Q4Z", strlen("20Q4Z")))
            {
                for (i = 1; i <= GLB_E580_24Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "32X2Q", strlen("32X2Q")))
            {
                for (i = 1; i <= GLB_E580_32X2Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "32X", strlen("32X")))
            {
                for (i = 1; i <= GLB_E580_32X_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "48X6Q", strlen("48X6Q")))
            {
                for (i = 1; i <= GLB_E580_48X6Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "48X2Q4Z", strlen("48X2Q4Z")))
            {
                for (i = 1; i <= GLB_E580_48X6Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "24X2C-D", strlen("24X2C-D")))
            {
                for (i = 1; i <= GLB_E580_24Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "48T4X2Q-D", strlen("48T4X2Q-D")))
            {
                for (i = 1; i <= GLB_E580_48X6Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
            else if (!sal_strncmp (board_type, "24X8Y2C", strlen("24X8Y2C")))
            {
                for (i = 1; i <= GLB_E580_24Q_MAX_PORT; i++)
                {
                    sal_fprintf(port_info_fd, "port %d %d %d %d\n", 1,  panel_slot_no, i, 0);
                }
            }
        }
        sal_fclose(startup_config_fp);
    }
    return LCM_E_SUCCESS;
}


/***************************************************************************************************
 * Name         : lcm_common_parse_port_numchg_info 
 * Purpose      : parse port split/merge info           
 * Input        :    
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32
//lcm_common_parse_port_split_info(glb_card_t* p_card, lcm_card_port_split_info_t* port_split_info)
lcm_common_parse_port_numchg_info(glb_card_t* p_card)
{
    FILE *startup_config_fp = NULL;
    uint32 panel_slot_no, panel_port_no, speed;
    uint8 tmp_port_num;
    char buf[256];     
    glb_panel_port_t* ppt_info;

    tmp_port_num = p_card->panel_port_num;

    /*get port prefix from eeprom (uml from flash), should be used to parse/check split cli*/
    lcm_get_port_info(p_card);
    
    startup_config_fp = sal_fopen(GLB_STARTUP_CONFIG_FILE_PATH, "r");
    if (NULL == startup_config_fp)
    {
        startup_config_fp = sal_fopen(GLB_FACTORY_CONFIG_FILE_PATH, "r");
    }
    if (startup_config_fp)
    {
        while (sal_fgets(buf, 128, startup_config_fp))
        {
            if (sal_strncmp(buf, "split interface", sal_strlen("split interface"))
                && sal_strncmp(buf, "merge interface", sal_strlen("merge interface")))
            {
                continue;
            }
            if (lcm_parse_numchg_cli(p_card, buf, &panel_slot_no, &panel_port_no, &speed))
            {
                continue;
            }
            ppt_info = p_card->pp_ppt[panel_port_no - 1];
            if((panel_port_no > p_card->panel_port_num) || (panel_port_no < 1))
            {
                continue;
            }
            if (panel_slot_no != ppt_info->pslt_id)
            {
                continue;
            }
            if((ppt_info->op == GLB_PPT_OP_SPLIT) && (speed == 10) && (ppt_info->op_ability & GLB_PPT_OP_SPLIT_10G))
            {
                ppt_info->ppt_numchg_type = GLB_PPT_NUMCHG_TYPE_10G;
                ppt_info->ppt_numchg_num = LCM_COMMON_40G_SPLIT_NUM;
                /*When 40G port split to 10G port, it lose 40G ability.*/
                if(ppt_info->disable_speed_change)
                    ppt_info->ppt_speed_ability = GLB_SUPPORT_SPEED_10G;
                else
                    ppt_info->ppt_speed_ability = (GLB_SUPPORT_SPEED_10G | GLB_SUPPORT_SPEED_1G);
                ppt_info->ppt_speed_real= GLB_SPEED_10G;
                tmp_port_num += (LCM_COMMON_40G_SPLIT_NUM - 1);
            }
            else if((ppt_info->op == GLB_PPT_OP_SPLIT) && (speed == 25) && (ppt_info->op_ability & GLB_PPT_OP_SPLIT_25G))
            {
                ppt_info->ppt_numchg_type = GLB_PPT_NUMCHG_TYPE_25G;
                ppt_info->ppt_numchg_num = LCM_COMMON_100G_SPLIT_NUM;
                /*When 100G port split to 25G port, it lose 100G ability.*/
                if(ppt_info->disable_speed_change)
                    ppt_info->ppt_speed_ability = GLB_SUPPORT_SPEED_25G;
                else
                    ppt_info->ppt_speed_ability = (GLB_SUPPORT_SPEED_25G |GLB_SUPPORT_SPEED_10G);
                ppt_info->ppt_speed_real= GLB_SPEED_25G;
                tmp_port_num += (LCM_COMMON_100G_SPLIT_NUM - 1);
            }
            else if((ppt_info->op == GLB_PPT_OP_SPLIT) && (speed == 50) && (ppt_info->op_ability & GLB_PPT_OP_SPLIT_50G))
            {
                ppt_info->ppt_numchg_type = GLB_PPT_NUMCHG_TYPE_50G;
                ppt_info->ppt_numchg_num = LCM_COMMON_100G_SPLIT_NUM_2;
                /*When 100G port split to 50G port, it only support 50G ability.*/
                if(ppt_info->disable_speed_change)
                    ppt_info->ppt_speed_ability = GLB_SUPPORT_SPEED_50G;
                else
                    ppt_info->ppt_speed_ability = (GLB_SUPPORT_SPEED_50G);
                ppt_info->ppt_speed_real= GLB_SPEED_50G;
                tmp_port_num += (LCM_COMMON_100G_SPLIT_NUM_2 - 1);
            }
            else if ((ppt_info->op == GLB_PPT_OP_MERGE) && (speed == 40)  && (ppt_info->op_ability & GLB_PPT_OP_SPLIT_40G))
            {
                ppt_info->ppt_numchg_type = GLB_PPT_NUMCHG_TYPE_40G;
                ppt_info->ppt_numchg_num = LCM_COMMON_10G_MERGE_NUM;
                ppt_info->ppt_speed_ability = GLB_SUPPORT_SPEED_40G;
                ppt_info->ppt_speed_real= GLB_SPEED_40G;
                tmp_port_num -= (LCM_COMMON_40G_SPLIT_NUM - 1);
            }
        }
        sal_fclose(startup_config_fp);
    }
    p_card->port_num = tmp_port_num;  /* get real port num */
    
    return 0;
}

int32
lcm_common_save_port_info_file(glb_card_t* p_card)
{
    uint16 port_id = 0;
    FILE *port_info_fd = NULL;
    glb_port_t *p_port = NULL;
    uint8 is_default = FALSE;

    /* format of port_info_FILE file: port prefix_idx, slot_no, panel_port_no, panel_sub_port_no
     * prefix_idx        : prefix index saved in GLB_PORT_CLI_INFO_FILE file
     * slot_no           : slot no
     * panel_port_no     : panel port no
     * panel_sub_port_no : sub pannel port no
    */
    port_info_fd = sal_fopen(GLB_PORT_INFO_FILE, "w");
    if (port_info_fd == NULL)
    {
        LCM_LOG_ERR("Open the interface info file failed!\n");
        return -1;
    }

    if (!sal_strcasecmp(glb_port_info.prefix[0], GLB_DEF_PHY_PORT_PREFIX))
    {
        is_default = TRUE;
    }

    for (port_id = 0; port_id < p_card->port_num; port_id++)
    {
        p_port = p_card->pp_port[port_id];
        if(GLB_SUPPORT_SPEED_100G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_100G)), 
                    p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_40G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_40G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_50G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_50G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_25G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_25G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_10G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_10G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_5G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_5G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_2_5G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_2_5G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_1G & p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_1G)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else if(GLB_SUPPORT_SPEED_100M& p_port->port_speed_real)
        {
            sal_fprintf(port_info_fd, "port %d %d %d %d\n", (is_default? 1 : ffs(GLB_SUPPORT_SPEED_100M)), 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
        else
        {
            sal_fprintf(port_info_fd, "port 1 %d %d %d\n", 
                p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
        }
    }

    if (lcm_stack_is_enable())
    {
        lcm_common_stack_save_port_info_file(port_info_fd);
    }

    sal_fclose(port_info_fd); 
    return 0;
}


int32
lcm_common_ppt_map_port(glb_card_t* p_card, lcm_card_port_panel_mapping_t* port_panel_mapping)
{
    uint8 ppt_id;
    uint8 port_id=0;
    uint8 panel_subport_no=0;   

    for (ppt_id = 0; ppt_id < p_card->panel_port_num; ppt_id++)
    {
        if (GLB_PPT_NUMCHG_TYPE_NONE == p_card->pp_ppt[ppt_id]->ppt_numchg_type)
        {
            port_panel_mapping[port_id].panel_slot_no = p_card->pp_ppt[ppt_id]->pslt_id;
            port_panel_mapping[port_id].panel_port_no = p_card->pp_ppt[ppt_id]->ppt_idx;
            if ((GLB_PPT_OP_SPLIT == p_card->pp_ppt[ppt_id]->op)
                || (GLB_PPT_OP_NONE == p_card->pp_ppt[ppt_id]->op))
            {
                port_panel_mapping[port_id].panel_subport_no = 0;
            }
            else if (GLB_PPT_OP_MERGE == p_card->pp_ppt[ppt_id]->op)
            {
                port_panel_mapping[port_id].panel_subport_no = -1;  ///TODO: need be handled in chsm
            }
            port_panel_mapping[port_id].port_numchg_type = p_card->pp_ppt[ppt_id]->ppt_numchg_type;
            port_id++;
        }
        else
        {
            if (GLB_PPT_OP_SPLIT == p_card->pp_ppt[ppt_id]->op)
            {
                for(panel_subport_no = 0; panel_subport_no < p_card->pp_ppt[ppt_id]->ppt_numchg_num; panel_subport_no++)
                {
                    port_panel_mapping[port_id].panel_slot_no = p_card->pp_ppt[ppt_id]->pslt_id;
                    port_panel_mapping[port_id].panel_port_no = p_card->pp_ppt[ppt_id]->ppt_idx;
                    port_panel_mapping[port_id].panel_subport_no = panel_subport_no+1;
                    port_panel_mapping[port_id].port_numchg_type = p_card->pp_ppt[ppt_id]->ppt_numchg_type;
                    port_id++;
                }
            }
            else if (GLB_PPT_OP_MERGE == p_card->pp_ppt[ppt_id]->op)
            {
                port_panel_mapping[port_id].panel_slot_no = p_card->pp_ppt[ppt_id]->pslt_id;
                if (p_card->pp_ppt[ppt_id]->ppt_idx % 4 == 0)
                {
                    port_panel_mapping[port_id].panel_port_no = p_card->pp_ppt[ppt_id]->ppt_idx;
                    port_panel_mapping[port_id].panel_subport_no = -1; ///TODO: need be handled in chsm
                    port_panel_mapping[port_id].port_numchg_type = p_card->pp_ppt[ppt_id]->ppt_numchg_type;
                    port_id++;
                }
                else
                {
                    continue;
                }
            }
        }
    }

    return LCM_E_SUCCESS;
}
/*Bug39456 jqiu 2016-07-19, support CL73 auto.*/
int32
lcm_common_get_defaultspeed_from_speedability(uint8 speed_ability, glb_port_speed_t*  speed, glb_port_duplex_t* duplex)
{
    if(speed_ability & GLB_SUPPORT_SPEED_100G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_FULL;
    }
    else if(speed_ability & GLB_SUPPORT_SPEED_40G)
    {
        *speed = GLB_SPEED_40G;
        *duplex = GLB_DUPLEX_FULL;
    }
    else if(speed_ability & GLB_SUPPORT_SPEED_50G)
    {
        *speed = GLB_SPEED_50G;
        *duplex = GLB_DUPLEX_AUTO;
    }
    else if(speed_ability & GLB_SUPPORT_SPEED_25G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }    
    else if(speed_ability & GLB_SUPPORT_SPEED_10G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }
    else if(speed_ability & GLB_SUPPORT_SPEED_5G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }
    else if(speed_ability & GLB_SUPPORT_SPEED_2_5G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }    
    else if(speed_ability & GLB_SUPPORT_SPEED_1G)
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }
    else
    {
        *speed = GLB_SPEED_AUTO;
        *duplex = GLB_DUPLEX_AUTO;
    }
    return LCM_E_SUCCESS;
}

int32
lcm_common_get_defaultfec_from_speedability(uint16 speed_ability, uint32*  fec_ability)
{
    *fec_ability = 0;
    if((speed_ability & GLB_SUPPORT_SPEED_100G)||(speed_ability & GLB_SUPPORT_SPEED_50G)
    ||(speed_ability & GLB_SUPPORT_SPEED_25G))
    {
#if (defined DUET2) || (defined TSINGMA)
        /* open all FEC ability default */
        *fec_ability |= CTC_PORT_CL73_FEC_ABILITY;
        *fec_ability |= CTC_PORT_CL73_FEC_REQUESTED;
        *fec_ability |= CTC_PORT_CL73_25G_RS_FEC_REQUESTED;
        *fec_ability |= CTC_PORT_CL73_25G_BASER_FEC_REQUESTED;
        *fec_ability |= CTC_PORT_CSTM_RS_FEC_ABILITY;
        *fec_ability |= CTC_PORT_CSTM_RS_FEC_REQUESTED;
        *fec_ability |= CTC_PORT_CSTM_BASER_FEC_ABILITY;
        *fec_ability |= CTC_PORT_CSTM_BASER_FEC_REQUESTED;
#endif
    }
    
    return LCM_E_SUCCESS;
}

int32
lcm_common_get_board_material(uint8 *bm)
{
    FILE *fp;
    uint32 val;
    char buf[256];
    *bm = 0xff;
    /* GLB_BOARD_INFO_FILE this file be writen at board detect tool */
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL == fp)
    {
        return LCM_E_FILE_OPEN;
    }

    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, GLB_BOARD_MATERIAL_STRING, 14))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            *bm = (uint8)val;
        }
    }

    sal_fclose(fp);

    if(*bm == 0xff)
    {
        return LCM_E_INVALID_PARAM;
    }
    return LCM_E_SUCCESS;
}

int32
lcm_common_get_slot_no(uint16 * logic_slot_no, uint16 * phy_slot_no)
{
    FILE *fp;
    char buf[BUFSIZ];
    fp = sal_fopen(GLB_SLOT_NO_FILE, "r");///TODO:
    if (fp)
    {
        sal_fgets (buf, BUFSIZ, fp);
        *logic_slot_no = atoi(buf);
        *phy_slot_no = *logic_slot_no;
        sal_fclose(fp);
    }
    else
    {
        *logic_slot_no = 1;
        *phy_slot_no = 0;
    }
    return LCM_E_SUCCESS;
}

int32
lcm_common_init_cpu_info()
{
    FILE * fp;

    fp = sal_fopen(GLB_CPU_IF_INFO_FILE, "w+");
    if(fp ==NULL)
    {
        return LCM_E_FILE_OPEN;
    }
    sal_fprintf(fp, "MGMT_IF eth0\n");
    sal_fprintf(fp, "CPU_IF NOTUSE\n");
    sal_fclose(fp);
    return LCM_E_SUCCESS;
}

int32
lcm_common_config_ptp_enable(int enable)
{
    FILE * fp;

    fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if(fp ==NULL)
    {
        return LCM_E_FILE_OPEN;
    }
    if(enable)
    {
        sal_fprintf(fp, "ptp enable\n");
    }
    else
    {
        sal_fprintf(fp, "ptp disable\n");
    }
    sal_fclose(fp);
    return LCM_E_SUCCESS;
}

int32
lcm_common_config_syncE_enable(int enable)
{
    FILE * fp;

    fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if(fp ==NULL)
    {
        return LCM_E_FILE_OPEN;
    }
    if(enable)
    {
        sal_fprintf(fp, "syncE enable\n");
    }
    else
    {
        sal_fprintf(fp, "syncE disable\n");
    }
    sal_fclose(fp);
    return LCM_E_SUCCESS;
}

int32
lcm_common_config_reboot_record_type(int type,int32 epld_addr)
{
    FILE * fp;

    fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
    if(fp ==NULL)
    {
        return LCM_E_FILE_OPEN;
    }
    if(type == REBOOT_EPLD)
    {
        sal_fprintf(fp, "rebootinfo_type %d\n",REBOOT_EPLD);
        sal_fprintf(fp, "rebootepld_addr %d\n",epld_addr);
    }
    else if(type == REBOOT_EPLD_EEPROM)
    {
        sal_fprintf(fp, "rebootinfo_type %d\n",REBOOT_EPLD_EEPROM);
        sal_fprintf(fp, "rebootepld_addr %d\n",epld_addr);
    }
    else if(type == REBOOT_FILE_EPLD)
    {
        sal_fprintf(fp, "rebootinfo_type %d\n",REBOOT_FILE_EPLD);
        sal_fprintf(fp, "rebootepld_addr %d\n",epld_addr);
    }    
    sal_fclose(fp);
    return LCM_E_SUCCESS;
}

int32
lcm_common_parse_bootup_diag(glb_card_t* p_card)
{
    char buf[1024];
    FILE *fp;
   
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init bootup diag begin.");
    LCM_LOG_BRINGUP( "Lcm init bootup diag begin.");
    fp = sal_fopen(GLB_STARTUP_CONFIG_FILE_PATH, "r");
    if(NULL == fp)
    {
        p_card->bootup_diag_level = E_DIAG_LEVEL_NO;
        LCM_LOG_DEBUG(LCM_CARD, "Lcm init bootup diag end.");
        return LCM_E_SUCCESS;
    }

    p_card->bootup_diag_level = E_DIAG_LEVEL_NO;   
    while(sal_fgets(buf, 1024, fp))
    {
        if(NULL != sal_strstr(buf, "diagnostic bootup level"))
        {
            if(NULL != sal_strstr(buf, "complete"))
            {
                 p_card->bootup_diag_level = E_DIAG_LEVEL_COMPLETE;
            }
            else if(NULL != sal_strstr(buf, "minimal"))
            {
                p_card->bootup_diag_level = E_DIAG_LEVEL_MINIMAL;
            }
            else
            {
                p_card->bootup_diag_level = E_DIAG_LEVEL_NO;
            }
            break;
        }
    }
    sal_fclose(fp);
    
    LCM_LOG_DEBUG(LCM_CARD, "Lcm init bootup diag end.");
    LCM_LOG_BRINGUP( "Lcm init bootup diag end.");

    return LCM_E_SUCCESS;

}

