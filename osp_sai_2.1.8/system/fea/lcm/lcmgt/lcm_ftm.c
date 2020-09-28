/****************************************************************************
 * lcm_ftm.c  :   calculate the key and nexthop information for sdk init
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       ychen
 * Date         :       2013-07-22
 * Reason       :       First Create.
 ****************************************************************************/
/****************************************************************************
* Header Files
****************************************************************************/
#include "sal_common.h"
#include "lcm_debug.h"
#include "lcm_error.h"
#include "glb_stm_define.h"
#include "glb_hw_define.h"
#include "glb_macro.h"
#include "glb_eeprom_define.h"
#include "eeprom_api.h"
#include "lcm_mgt.h"
#include "sysenv_api.h"
/******************************************************************************
*Defines and Macros
********************************************************************************/
#define LCM_FTM_EMPTY_LINE(C)     ((C) == '\0' ||(C) == '\r' || (C) == '\n' )
#define LCM_FTM_WHITE_SPACE(C)    ((C) == '\t' || (C) == ' ')

#define LCM_FTM_LINE_LEN_MAX                128
#define LCM_FTM_MAX_KEY_LEN                  64
/**************************************************************************
*Globals
****************************************************************************/
uint32 g_stm_mode = GLB_STM_MODE_DEFAULT;
uint32 g_stm_extend_flag = 0;

/*========================================================================================
*FUNCTIONS 
==========================================================================================*/
#ifdef _GLB_UML_SYSTEM_
/*******************************************************************************************
 * name            :_lcm_ftm_string_atrim
 * pupose          :trim left and right space
 * input           :p_input---input string
 *                 :
 * output          :p_output--output string
 * return          :N/A
 * note:
*******************************************************************************************/
static int32
_lcm_ftm_string_atrim(char* p_output, const char* p_input)
{
    char* p = NULL;  
  
    GLB_PTR_VALID_CHECK(p_output, LCM_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_input, LCM_E_INVALID_PTR);
  
    /*trim left space*/
    while(*p_input != '\0')
    {
        if(LCM_FTM_WHITE_SPACE(*p_input))
        {
            ++p_input;
        }
        else
        {
            break;
        }
    }  
    sal_strcpy(p_output, p_input);  
    /*trim right space*/
    p = p_output + sal_strlen(p_output) - 1;
  
    while(p >= p_output)
    {
      /*skip empty line*/
        if(LCM_FTM_WHITE_SPACE(*p) || ('\r' == (*p)) || ('\n' == (*p)))
        {
            --p;
        }
        else
        {
            break;
        }
    }
  
    *(++p) = '\0';
  
    return LCM_E_SUCCESS;
}

/*******************************************************************************************
 * name            :_lcm_ftm_read_size_info
 * pupose          : read key size
 * input           :p_line---input string
 *                 :
 * output          :p_para--tcam key size defined in glb_stm_key_size_t or just entry number
 * return          :N/A
 * note:           :
******************************************************************************************/
static int32
_lcm_ftm_read_size_info(const char* p_line, void* p_para)
{
    char*   ch = NULL;
    uint32* p_size = (uint32*)p_para;
    
    GLB_PTR_VALID_CHECK(p_line, LCM_E_INVALID_PTR);
    GLB_PTR_VALID_CHECK(p_para, LCM_E_INVALID_PTR); 
    
    ch = sal_strstr(p_line, "=");
    if(NULL == ch)
    {
        return LCM_E_INVALID_PARAM;
    }
    else
    {
        ch++;
    }
    
    while(sal_isspace(*ch))
    {
      ch++;
    }
  
    sal_sscanf(ch, "%u", p_size);
  
    return LCM_E_SUCCESS;
}
#endif /*_GLB_UML_SYSTEM_*/



/********************************************************************************************
* name            :_stm_alloc_read_profile_mode
* pupose          :get profile mode
* input           :N/A
* output          :p_mode----profile mode defined in stm_profile_mode_t
* return          :NONE
* note:
*******************************************************************************************/
int32 
lcm_ftm_read_profile_mode(uint32* p_mode)
{
    int32 ret = LCM_E_SUCCESS;
#ifndef _GLB_UML_SYSTEM_
    uint32 stm_mode = GLB_STM_MODE_DEFAULT;
#endif /*_GLB_UML_SYSTEM_*/
    
    GLB_PTR_VALID_CHECK(p_mode, LCM_E_INVALID_PTR);
    LCM_LOG_BRINGUP("Line card read ftm profile mode from device");
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("LCM read stm mode: card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    if (p_card->use_sysenv_api)
    {
        sysenv_info_t* p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_TCAM_PROFILE]);
        //jqiu modify. if board doesn't define this, use default.
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR("LCM read stm mode: sysenv alloc is NULL.");
        }
        else
        {
            ret = sysenv_get_tcam_profile(&stm_mode, p_sysenv_info);
            if (ret < 0)
            {
                LCM_LOG_ERR("LCM read stm mode: failed to get stm mode, use default mode.");
                stm_mode = GLB_STM_MODE_DEFAULT;
            }
        }
    }
    else
    {
        eeprom_info_t* p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE]);
        //jqiu modify. if board doesn't define this, use default.
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR("LCM read stm mode: eeprom alloc is NULL.");
        }
        else
        {
            ret = eeprom_get_tcam_profile(&stm_mode, p_eeprom_info);
            if (ret < 0)
            {
                LCM_LOG_ERR("LCM read stm mode: failed to get stm mode, use default mode.");
                stm_mode = GLB_STM_MODE_DEFAULT;
            }
        }
    }
    /*huangxt for bug 32166*/
    /* Modified by Yan Xing'an, for openflow */
    if ((GLB_STM_MODE_DEFAULT != stm_mode) 
        && (GLB_STM_MODE_ACCESS != stm_mode)
        && (GLB_STM_MODE_IPUCV4 != stm_mode)
        && (GLB_STM_MODE_IPUCV6 != stm_mode)
        && (GLB_STM_MODE_L2VPN != stm_mode)
#ifdef CNOS_OS   
        && (GLB_STM_MODE_OF_IPV6 != stm_mode)
        && (GLB_STM_MODE_TAP_IPV4_IPV6 != stm_mode)
#endif
        )
    {
        stm_mode = GLB_STM_MODE_DEFAULT;
    }

/* BUG52293 load default stm mode for Hybrid&TAP Product */
#ifdef TAPPRODUCT
#ifdef GREATBELT
    if (GLB_STM_MODE_DEFAULT != stm_mode && GLB_STM_MODE_TAP_IPV4_IPV6 != stm_mode)
#else
    if (GLB_STM_MODE_DEFAULT != stm_mode)
#endif
    {
        stm_mode = GLB_STM_MODE_DEFAULT;
    }
#endif

#ifdef OFPRODUCT
    if (GLB_STM_MODE_DEFAULT != stm_mode && GLB_STM_MODE_OF_IPV6 != stm_mode)
    {
        stm_mode = GLB_STM_MODE_DEFAULT;
    }
#endif

    *p_mode = stm_mode;
	printf("zhw test: stm_mode is :%d\n", stm_mode);
	*p_mode = GLB_STM_MODE_IPUCV6;
	printf("zhw test: p_mode is :%d\n", *p_mode);
#else
    char file_name[LCM_FTM_LINE_LEN_MAX];
    char string[LCM_FTM_LINE_LEN_MAX];
    char line[LCM_FTM_LINE_LEN_MAX];
    FILE* fp = NULL;    

    /*get file path*/
#ifndef CNOS_OS   
    sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH_UML, GLB_STM_PROFILE_MODE);
#else
    sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH, GLB_STM_PROFILE_MODE);
#endif
    /*OPEN FILE*/
    fp = sal_fopen(file_name, "r");
    
    if((NULL == fp) || sal_feof(fp))
    {
#ifndef CNOS_OS
        /*get file path*/
        sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH, GLB_STM_PROFILE_MODE);

        /*OPEN FILE*/
        fp = sal_fopen(file_name, "r");
        
        if((NULL == fp) || sal_feof(fp))
        {
            return LCM_E_FILE_OPEN;
        }
#else
        return LCM_E_FILE_OPEN;
#endif
    }

    /*get profile mode*/
    while(!sal_feof(fp))
    {
        sal_memset(string, 0, sizeof(string));
        sal_fgets(string, LCM_FTM_LINE_LEN_MAX, fp);

        /*comment line*/
        if('#' == string[0])
        {
          continue;
        }
        
        /*trim left and right space*/
        sal_memset(line, 0, sizeof(line));        
        ret = _lcm_ftm_string_atrim(line, string);

        if(LCM_FTM_EMPTY_LINE(line[0]))
        {
          continue;
        }    

        if (!sal_strncmp("[PROFILE_MODE]", line, sal_strlen("[PROFILE_MODE]")))
        {
            ret = _lcm_ftm_read_size_info(line, (void*)p_mode);
            break;
        }
    }

    /*close file*/
    sal_fclose(fp);
    fp = NULL;
#endif /*_GLB_UML_SYSTEM_*/

#ifdef OPENFLOW_V580
    /* On OSP-OF product phase 1, only OF default mode is supported. */
    *p_mode = GLB_STM_MODE_OF_DEFAULT;
#endif

    g_stm_mode = *p_mode;
    LCM_LOG_BRINGUP("Line card read ftm profile mode %d",g_stm_mode);

    return ret;
}

int32 
lcm_ftm_get_profile_ext_flag (glb_board_type_t type)
{
    FILE *startup_config_fd = NULL;
    char buf[256];

    if (GLB_SERIES_E550 == type.series
        && GLB_BOARD_E550_24T16X == type.type)
    {
        GLB_SET_FLAG(g_stm_extend_flag, GLB_STM_EXTEND_FLAG_COUPLE);
    }
    else
    {
        startup_config_fd = sal_fopen(GLB_STARTUP_CONFIG_FILE_PATH, "r");
        if (startup_config_fd)
        {
            while (sal_fgets(buf, 128, startup_config_fd))
            { 
                 if (!sal_strncmp(buf, "stm working-mode resource-couple", sal_strlen("stm working-mode resource-couple")))
                {
                    GLB_SET_FLAG(g_stm_extend_flag, GLB_STM_EXTEND_FLAG_COUPLE);
                    break;
                }
                else if (!(sal_strncmp(buf, "interface", sal_strlen("interface"))))
                {
                    break;
                }
            }

            sal_fclose(startup_config_fd);
        }
    }
  
    return g_stm_extend_flag;
}

