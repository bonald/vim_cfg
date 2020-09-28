/****************************************************************************
 * param_check.c: Common functions to check parameter
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     : R0.01
 * Author       : Alexander Liu
 * Date         : 2006-11-27
 * Reason       : First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/statfs.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "reboot_record.h"
#include "i2c_drv.h"
#include "localbus_drv.h"

/****************************************************************************
 *  
* Defines and Macros
*
*****************************************************************************/
glb_board_type_t glb_boardtype;
uint8 glb_hwver;
int cmd_str2num (const char *str, int *ret)
{
  int i;
  int len;
  int digit = 0;
  u_int32_t limit, remain, hex = 0;
  int minus = 0;
  u_int32_t max = 0xffffffff;
  u_int32_t total = 0;

  /* Sanify check. */
  if (str == NULL || ret == NULL)
        return -1;

  /* First set return value as error. */
  *ret = -1;

  len = strlen (str);
  if(*str == '+')
    {
        str++;
        len--;
    }
  else if(*str == '-')
    {
        str++;
        len--;
        minus = 1;
        max = max / 2 + 1;
    }
  else if(*str == '0')
    {
        str++;
        len--;
        if(*str =='x')
        {
            str++;
            len--;	
            hex = 1;
        }
        else if(*str =='\0')
        {
            *ret = 0;
            return 0;
        }
        else
        {
            return 0;
        }
    }

  limit = max / 10;
  remain = max % 10;

  if(len < 1 || len > 10)
        return -1;

  for(i = 0; i < len; i++)
    {
        if(*str < '0' || (*str > '9' && *str < 'a') || *str >'f')
            return -1;

        if(*str <= '9')
            digit = (*str++) - '0';
        else if(*str >= 'a')
            digit = (*str++) - 'a' + 10;

        if(total  > limit || (total == limit && digit > remain))
            return -1;
        if(hex == 1)
            total = total * 16 + digit;
        else
            total = total * 10 + digit;
    }

  *ret = 0;
  if(minus && (total == 0))
        return -1;
  if(minus)
        return -total;
  else
        return total;
}
int get_board_type(reboot_device_t *reboot_dev)
{
#ifdef _GLB_UML_SYSTEM_
    return 0;
#endif
    FILE *fp;
    uint32 val;
    char buf[256];

    /* HW information is in file /tmp/ctcos_board_info, now parse it */
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL == fp)
    {
        glb_boardtype.type = 0xff;
        glb_boardtype.series = GLB_SERIES_MAX;
        return -1;
    }
    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, GLB_PRODUCT_SERIES_STRING, 14))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            glb_boardtype.series = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_BOARD_TYPE_STRING, 10))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            glb_boardtype.type = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_HW_VERSION_STRING, 6))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            glb_hwver = (uint8)val;
        }        
        if(!sal_strncmp(buf, "rebootinfo_type", 15))
        {
            sal_sscanf(buf, "%s %d", tmp, &val);
            reboot_dev->type = (uint8)val;
        }
        if(!sal_strncmp(buf, "rebootepld_addr", 15))
        {
            sal_sscanf(buf, "%s %d", tmp, &val);
            reboot_dev->epld_addr = (uint32)val;
        }        
    }
    sal_fclose(fp);
    return 0;
}
int32 init_reboot_device(reboot_device_t *reboot_dev)
{
    int ret;
    reboot_dev->type = REBOOT_UNSUPPORT;
    
    /* For all board, need config reboot info in board init file by 
    *  "lcm_common_config_reboot_record_type(REBOOT_EPLD,0xf6);" or 
    *  "cm_common_config_reboot_record_type(REBOOT_FILE_EPLD,0xf6);"
    */    
    ret = get_board_type(reboot_dev);
    if(ret != 0)
        return -1;

    return 0;
}

int write_device(reboot_device_t *reboot_dev, uint8 value)
{
    FILE *fp;
#if (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
    uint16 epld_addr = 0x36;
#if (defined _CTC_ARM_CTC5236_)
#define GLB_I2C_IDX GLB_I2C_IDX_0
#elif (defined _CTC_ARM_LS1023A_)
#define GLB_I2C_IDX GLB_I2C_IDX_1
#endif
#endif
    switch (reboot_dev->type)
    {
        case REBOOT_FILE_EPLD:
            fp = sal_fopen(REBOOT_RECORD_FILE, "w");
            if(NULL == fp)
            {
                return -1;
            }
            sal_fprintf(fp ,"rebootinfo %d\n",value);
            sal_fflush(fp);
            sal_fclose(fp);
        case REBOOT_EPLD:
#ifdef _CTC_ARM_HI3535_
            i2c_write(0, 0x58, reboot_dev->epld_addr, 1, &value, 1);
#elif (defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)
            if(GLB_SERIES_E550 == glb_boardtype.series)
            {
                if((GLB_BOARD_E550_24T16Y == glb_boardtype.type)||(GLB_BOARD_E550_24T16X == glb_boardtype.type))
                {
                    if(glb_hwver < 0x30)
                        epld_addr = 0x58;
                }
            }
            i2c_write(GLB_I2C_IDX, epld_addr, reboot_dev->epld_addr, 1, &value, 1);
#else
            ctc_localbus_write(reboot_dev->epld_addr, value);
#endif
            break;

        case REBOOT_EPLD_EEPROM:
            break;
        case REBOOT_UNSUPPORT:
            fprintf(stderr, "error: this board no support reboot info record\n");
            exit (1);            
            break;    
        default:
            break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int32 value;
    int ret;
    reboot_device_t *reboot_dev;

    if (argc != 2)
    {
        fprintf(stderr, "error: %s just need one pram VALUE!\n", argv[0]);
        exit (1);
    }
    value = cmd_str2num(argv[1],&ret);
    reboot_dev = (reboot_device_t *)malloc(sizeof(reboot_device_t));
    if (NULL == reboot_dev)
    {
        fprintf(stderr, "malloc reboot device fail!\n");
        exit (1);
    }    
    ret = init_reboot_device(reboot_dev);
    if (0 != ret)
    {
        fprintf(stderr, "init reboot device fail!\n");
        free(reboot_dev);
        exit (2);
    }
    printf("write reboot info value %d!\n",value);
    ret = write_device( reboot_dev, (uint8)value);
    if (0 != ret)
    {
        fprintf(stderr, "write reboot info fail!\n");
        free(reboot_dev);
        exit (3);
    }

    printf("\nrecord reboot success!\n");
    fflush(stdout);
    exit (0);
}

