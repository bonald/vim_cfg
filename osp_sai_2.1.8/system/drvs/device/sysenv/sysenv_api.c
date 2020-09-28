/****************************************************************************
 * sysenv_api.c    sysenv api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2019-01-16.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_const.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "sysenv_api.h"
#include "sysenv_drv.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define ENV_SIZE  2048
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#define	min3(a,b,c)	min(min((a),(b)), (c))

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
/* sysenv handler */
static sysenv_handle_t **g_sysenv_hdl = NULL;
static uint32 g_sysenv_nums = 0;


/****************************************************************************
 *
* Functions  
*
****************************************************************************/

#ifdef CNOS_OS
extern u_int32_t ctclib_gen_crc32(u_int32_t crc, char *buf, size_t len);
#else
extern uint32 ctclib_gen_crc32(uint32 crc, char *buf, size_t len);
extern u_int32_t gen_crc32(u_int32_t crc, char *buf, size_t len);
#endif

int32 sysenv_write(uint32_t idx, sysenv_para_t* p_para)
{
    if (idx >= g_sysenv_nums || idx < 0)
        return SYSENV_E_ERROR;

    if (g_sysenv_hdl[idx] == NULL || g_sysenv_hdl[idx]->write == NULL)
        return SYSENV_E_ERROR;
        
    return g_sysenv_hdl[idx]->write(g_sysenv_hdl[idx], p_para->offset, p_para->len, p_para->p_val);
}

int32 sysenv_read(uint32_t idx, sysenv_para_t* p_para)
{
    if (idx >= g_sysenv_nums || idx < 0)
        return SYSENV_E_ERROR;

    if (g_sysenv_hdl[idx] == NULL || g_sysenv_hdl[idx]->read == NULL)
        return SYSENV_E_ERROR;
        
    return g_sysenv_hdl[idx]->read(g_sysenv_hdl[idx], p_para->offset, p_para->len, p_para->p_val);
}

int32 sysenv_close(uint32 idx)
{
    if (idx >= g_sysenv_nums || idx < 0)
        return SYSENV_E_ERROR;

    if (g_sysenv_hdl[idx] == NULL || g_sysenv_hdl[idx]->close == NULL)
        return SYSENV_E_ERROR;
        
    g_sysenv_hdl[idx]->close(g_sysenv_hdl[idx]);
    g_sysenv_hdl[idx] = NULL;

    return SYSENV_SUCCESS;
}

int32 sysenv_set_tcam_profile(uint32  mode, sysenv_info_t* p_sysenv_info)
{
    unsigned char buf[5];    
    sysenv_para_t para;
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    sal_memset (buf, 0, sizeof(buf));
    para.len = 5;
    para.offset = p_sysenv_info->base_addr;
    para.p_val = buf;  
    
    buf[0] = 0xaa; /*0x1800 should written tcam flag, for chsm to judge*/
  
    /*from 0x1801, the value is tcam mode*/
    buf[1] = mode & 0xff;
    buf[2] = (mode >> 8) & 0xff;
    buf[3] = (mode >> 16) & 0xff;
    buf[4] = (mode >> 24) & 0xff;
  
    if (sysenv_write (p_sysenv_info->sysenv_idx, &para) < 0)
    {
        DRV_LOG_DEBUG(DRV_SYSENV, "Set stm profile mode=%d fail!", mode);
        ret = RESULT_ERROR;
    }
  
    return ret;
}

int32 sysenv_get_tcam_profile(uint32* p_mode, sysenv_info_t* p_sysenv_info)
{
    sysenv_para_t para;
    uint8 buf[4];
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_mode);
    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    /* get system mac to buffer */
    sal_memset (buf, 0, sizeof(buf));
    para.len = 4;
    para.offset = p_sysenv_info->base_addr + 1;
    para.p_val = buf; 
    if (sysenv_read(p_sysenv_info->sysenv_idx, &para) < 0)
    {
        ret = RESULT_ERROR;
    }  

    *p_mode = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]; 
        
    if (RESULT_OK == ret)
    { 
        DRV_LOG_DEBUG(DRV_SYSENV, "Get stm profile mode=%d", *p_mode);              
    }

    return ret;  
}

/*********************************************************************
 * Name    : sysenv_get_oem_info
 * Purpose :  get oem info  
 * Input   : uint8 * p_oem_info       - the pointer of oem info
          the oem info buffer is static len MAX_SYSENV_OEM_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
sysenv_get_oem_info(uint8* p_oem_info, sysenv_info_t *p_sysenv_info)
{
    sysenv_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int read_len = 0;

    DRV_CTC_CHK_PTR(p_oem_info);
    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    read_len = MAX_SYSENV_OEM_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = read_len;
        para.offset = p_sysenv_info->base_addr+ i*read_len;
        para.p_val = p_oem_info + i*read_len;
        ret |= sysenv_read(p_sysenv_info->sysenv_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : sysenv_set_oem_info
 * Purpose :  set oem info  
 * Input   : uint8 * p_oem_info       - the pointer of oem info
          the oem info buffer is static len MAX_SYSENV_OEM_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
sysenv_set_oem_info(uint8* p_oem_info, sysenv_info_t *p_sysenv_info)
{
    sysenv_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int write_len = 0;

    DRV_CTC_CHK_PTR(p_oem_info);
    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    write_len = MAX_SYSENV_OEM_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = write_len;
        para.offset = p_sysenv_info->base_addr+ i*write_len;
        para.p_val = p_oem_info + i*write_len;
        ret |= sysenv_write(p_sysenv_info->sysenv_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : sysenv_get_port_info
 * Purpose : get port info  
 * Input   : uint8 * p_port_info       - the pointer of port info
             the port info buffer is static len MAX_PORT_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
sysenv_get_port_info(uint8* p_port_info, sysenv_info_t *p_sysenv_info)
{
    sysenv_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int read_len = 0;

    DRV_CTC_CHK_PTR(p_port_info);
    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    read_len = MAX_PORT_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = read_len;
        para.offset = p_sysenv_info->base_addr+ i*read_len;
        para.p_val = p_port_info + i*read_len;
        ret |= sysenv_read(p_sysenv_info->sysenv_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : sysenv_set_port_info
 * Purpose : set port info  
 * Input   : uint8 * p_port_info       - the pointer of port info
             the port info buffer is static len MAX_PORT_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
sysenv_set_port_info(uint8* p_port_info, sysenv_info_t *p_sysenv_info)
{
    sysenv_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int write_len = 0;

    DRV_CTC_CHK_PTR(p_port_info);
    DRV_CTC_CHK_PTR(p_sysenv_info);
    SYSENV_ITEM_CHK(p_sysenv_info);
    
    write_len = MAX_PORT_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = write_len;
        para.offset = p_sysenv_info->base_addr+ i*write_len;
        para.p_val = p_port_info + i*write_len;
        ret |= sysenv_write(p_sysenv_info->sysenv_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*modify for bug 14775. jqiu 2011-05-13*/
static int32 search_sysenv_str(char* buf, uint buf_len, char *str, int* len)
{
    uint i=0;
    while(i<buf_len)
    {
        if((buf[i]==0)&&(buf[i+1]!=0))
        {
            if(sal_strncmp(&buf[i+1], str, sal_strlen(str))==0)
            {
                *len = sal_strlen(&buf[i+1]);
                return i+1;
            }
        }
        i++;
    }
    return -1;
}


/* only used by sysenv with eeprom, ENV_SIZE == 2048 */
/* for sysenv with spiflash, bootcmd is managed by fw_printenv/fw_setenv */
/*modify for bug 14775. jqiu 2011-05-13*/
/*bootcmd: bootcmd=boot_flash
  bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x&ipaddr=x.x.x.x
  bootcmd: bootcmd=boot_tftp6 xxxxOS-xx.x.x.bin&serverip6=x:x:x:x&ip6addr=x:x:x:x*/
int32
sysenv_set_bootcmd(sysenv_info_t* p_sysenv_info, char* bootcmd)
{
    sysenv_para_t para;
    uint8* src_buf;
    uint8* dst_buf;
    uint16 loop_max, read_len, i, j, k, len;
    uint8 erase_flag_ipv4=0, tmp,bootcmd_flag=0;
    uint32 crc =0;
    int ret=0;
    uint8 is_tftp6 = 0;
    uint8 erase_flag_ipv6=0;
    
    DRV_CTC_CHK_PTR(p_sysenv_info);
    DRV_CTC_CHK_PTR(bootcmd);
    
    read_len = 32;
    loop_max = ENV_SIZE/read_len;
    src_buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, ENV_SIZE+1);
    if(src_buf == NULL)
        return -1;
    dst_buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, ENV_SIZE+M_FULLPATH_MAX_LEN);
    if(dst_buf == NULL)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, src_buf);
        return -1;    
    }
    for (k = 0; k< loop_max; k++)
    {
        para.len = read_len;
        para.offset = p_sysenv_info->base_addr+ k*read_len;
        para.p_val = src_buf + k*read_len;
        ret |= sysenv_read(p_sysenv_info->sysenv_idx, &para);
        usleep(10*1000);
    }

    if(sal_strstr(bootcmd, "bootcmd=boot_flash") == NULL)
    {
        /*need remove old serverip and ipaddr*/
        erase_flag_ipv4 = 1;
        erase_flag_ipv6 = 1;
    }
    if(sal_strstr(bootcmd, "bootcmd=boot_tftp6") == NULL)
    {
        is_tftp6 = 0;
        erase_flag_ipv6 = 0;
    }
    else
    {
        is_tftp6 = 1;
        erase_flag_ipv4 = 0;
    }
    tmp = src_buf[3];
    src_buf[3] = 0;
    i = 4;
    /*first 4 byte is crc check.*/
    for(k = 4; k < ENV_SIZE; k++)
    {
        /* at string start*/
        if((src_buf[k-1]==0)&&(src_buf[k]!=0))
        {
            if(is_tftp6 == 0)
            {
                /* if ipaddr, judge if need to skip this string*/
                if(sal_strncmp((char*)&src_buf[k], "ipaddr=", 7)==0)
                {
                    if(erase_flag_ipv4 == 1)
                    {
                        while((src_buf[k]!='\0')&&(k<ENV_SIZE))
                        {                    
                            k++;
                        }                    
                        continue;
                    }                
                }
                /* if serverip, skip this string*/
                if(sal_strncmp((char*)&src_buf[k], "serverip=", 9)==0)
                {
                    if(erase_flag_ipv4 == 1)
                    {
                        while((src_buf[k]!='\0')&&(k<ENV_SIZE))
                        {                    
                            k++;
                        }                    
                        continue;
                    }                
                }
            }
            else /* boot from tftp6 */
            {
                /* if ipaddr, judge if need to skip this string*/
                if(sal_strncmp((char*)&src_buf[k], "ip6addr=", 8)==0)
                {
                    if(erase_flag_ipv6 == 1)
                    {
                        while((src_buf[k]!='\0')&&(k<ENV_SIZE))
                        {                    
                            k++;
                        }                    
                        continue;
                    }                
                }
                /* if serverip, skip this string*/
                if(sal_strncmp((char*)&src_buf[k], "serverip6=", 10)==0)
                {
                    if(erase_flag_ipv6 == 1)
                    {
                        while((src_buf[k]!='\0')&&(k<ENV_SIZE))
                        {                    
                            k++;
                        }                    
                        continue;
                    }                
                }                
            }
            /* if bootcmd, skip this string*/
            if(sal_strncmp((char*)&src_buf[k], "bootcmd=", 8)==0)
            {
                /* skip this string */
                while((src_buf[k]!='\0')&&(k<ENV_SIZE))
                {                    
                    k++;
                }
                /*if first time process bootcmd string, add new string, else just skip this string.*/
                if(bootcmd_flag == 0)
                {
                    len = sal_strlen(bootcmd);
                    for(j=0; j<len; j++)
                    {
                        dst_buf[i] = bootcmd[j];
                        if(bootcmd[j]=='&')
                        {
                            dst_buf[i] = '\0';
                        }
                        i++;
                    }
                    dst_buf[i++] = '\0';
                    bootcmd_flag = 1;
                }                
                continue;
            }
            dst_buf[i++] = src_buf[k];
        }
        else /* */
        {
            dst_buf[i++] = src_buf[k];
        }
    }
    src_buf[3] = tmp;
    crc = ctclib_gen_crc32(0, (char *)&dst_buf[4], ENV_SIZE-4);
    dst_buf[0] = (crc>>24)&0xff;
    dst_buf[1] = (crc>>16)&0xff;
    dst_buf[2] = (crc>>8)&0xff;
    dst_buf[3] = crc&0xff;
    /*found out the last diff byte, then just need to write into sysenv until this byte*/
    k = ENV_SIZE-1; /*fix bug38213, liangf, 2016-05*/
    while((src_buf[k] == dst_buf[k]) && (k>0))
    {
        k--;
    }
    /*found out the first diff byte, then just need to write into sysenv from this byte*/
    j = 4;
    while((src_buf[j] == dst_buf[j]) && (j<k))
    {
        j++;
    }
    /*Fix bug 14941. jqiu 2011-05-18.*/
    /* k!=0 or (k==0 and byte 0 isn't equal),  crc has changed, need update.*/
    if((k != 0)||(src_buf[0] != dst_buf[0]))
    {
        para.len = 4;
        para.offset = p_sysenv_info->base_addr;
        para.p_val = (uint8 *)&crc;
        ret |= sysenv_write(p_sysenv_info->sysenv_idx, &para);
        usleep(10*1000);
    }
    /* j>4 or (j==4 and byte 4 isn't equal), data has changed, need update.*/
    if((j > 4)||(src_buf[4] != dst_buf[4]))
    {
        /*select the start addr from begin of 32 times*/
        while(j%32 != 0)
        {
            j--;
        }
        
        while(j < k)
        {
            para.len = 32;
            para.offset = p_sysenv_info->base_addr+j;
            para.p_val = (uint8 *)&dst_buf[j];
            ret = sysenv_write(p_sysenv_info->sysenv_idx, &para);
            usleep(10*1000);
            j = j+32;
        }
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, src_buf);
    DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, dst_buf);
    return ret;
}

/* only used by sysenv with eeprom, ENV_SIZE == 2048 */
/* for sysenv with spiflash, bootcmd is managed by fw_printenv/fw_setenv */
/*bootcmd: bootcmd=boot_flash
  bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x*ipaddr=x.x.x.x
  bootcmd: bootcmd=boot_tftp6 xxxxOS-xx.x.x.bin&serverip6=x:x:x:x&ip6addr=x:x:x:x*/
int32
sysenv_get_bootcmd(sysenv_info_t* p_sysenv_info,char* bootcmd)
{
    sysenv_para_t para;
    char* buf;
    int32 ret = RESULT_OK;
    int len=0, read_len, loop_max, k;
    char boot_image[128+18+1]; //there include 'bootcmd=boot_tftp ' 18 byte.
    char serverip[64+1];
    char ipaddr[64+1];

    DRV_CTC_CHK_PTR(p_sysenv_info);
    DRV_CTC_CHK_PTR(bootcmd);

    sal_memset(boot_image, 0, sizeof(boot_image));
    sal_memset(serverip, 0, sizeof(serverip));
    sal_memset(ipaddr, 0, sizeof(ipaddr));
    buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, ENV_SIZE+1);
    if(buf == NULL)
    {
        DRV_LOG_ERR("sysenv calloc memory fail!\n");
        return RESULT_ERROR;
    }
    read_len = 32;
    loop_max = ENV_SIZE/read_len;
    for (k = 0; k< loop_max; k++)
    {
        para.len = read_len;
        para.offset = p_sysenv_info->base_addr+ k*read_len;
        para.p_val = (uint8 *)buf + k*read_len;
        ret |= sysenv_read(p_sysenv_info->sysenv_idx, &para);
        usleep(10*1000);
    }

    ret=search_sysenv_str(buf, ENV_SIZE, "bootcmd=",&len);
    if(ret<=0)
    {
        DRV_LOG_ERR("read bootcmd fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, buf);
        return RESULT_ERROR;
    }
    
    sal_memcpy(boot_image, &buf[ret], len>146?146:len);
    if(sal_strstr(boot_image, "boot_flash") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, buf);
        return 0;
    }
    else if(sal_strstr(boot_image, "boot_tftp6") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        /*need to find serverip and ipaddr*/
        ret=search_sysenv_str(buf, ENV_SIZE, "serverip6=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(serverip, &buf[ret], len>128?128:len);
            sal_strcat(bootcmd, serverip);
        }
        ret=search_sysenv_str(buf, ENV_SIZE, "ip6addr=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(ipaddr, &buf[ret], len>128?128:len);
            sal_strcat(bootcmd, ipaddr);
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, buf);
        return 0;
    }
    else if(sal_strstr(boot_image, "boot_tftp") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        /*need to find serverip and ipaddr*/
        ret=search_sysenv_str(buf, ENV_SIZE, "serverip=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(serverip, &buf[ret], len>64?64:len);
            sal_strcat(bootcmd, serverip);
        }
        ret=search_sysenv_str(buf, ENV_SIZE, "ipaddr=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(ipaddr, &buf[ret], len>64?64:len);
            sal_strcat(bootcmd, ipaddr);
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, buf);
        return 0;
    }
    else
    {
        DRV_LOG_ERR("read bootcmd fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_SYSENV_INFO, buf);
        return RESULT_ERROR;
    }    
}

int32
sysenv_set_sw_ver(sysenv_info_t *p_sysenv_info, uint8* p_sw_ver, uint8 sw_ver_len)
{
    sysenv_para_t para;
    uint8 sw_ver[MAX_SWVER_LEN+1];
    int ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_sw_ver);
    DRV_CTC_CHK_PTR(p_sysenv_info);
        
    if (0 == sw_ver_len)
    {
        sw_ver[0] = sw_ver_len;
        para.len = 1;
        para.offset = p_sysenv_info->base_addr;
        para.p_val = sw_ver;
    }
    else
    {
        sw_ver[0] = sw_ver_len;
        sal_memcpy(sw_ver+1, p_sw_ver, sw_ver_len);            
        para.len = sw_ver_len+1;
        para.offset = p_sysenv_info->base_addr;
        para.p_val = sw_ver;
    }
    
    ret += sysenv_write(p_sysenv_info->sysenv_idx, &para);
    
    if (ret < 0)
    { 
        DRV_LOG_DEBUG(DRV_SYSENV, "sysenv_set_sw_ver write fail!");   
    }

    return ret;
}

int32
sysenv_init(sysenv_chip_info_t *sysenv_chip_info, uint32 num)
{
    int32 i = 0;
    
    DRV_CTC_CHK_PTR(sysenv_chip_info);
    
    g_sysenv_hdl = (sysenv_handle_t **)DRV_MALLOC(CTCLIB_MEM_DRIVER_SYSENV_INFO, sizeof(sysenv_handle_t *)*num);
    g_sysenv_nums = num;
    
    if(NULL == g_sysenv_hdl)
    {
        DRV_LOG_ERR("sysenv alloc handler fail!\n");
        return SYSENV_E_ERROR;
    }

    sal_memset(g_sysenv_hdl, 0, sizeof(sysenv_handle_t *)*num);
    
    for (i = 0; i < num; i++)
    {
        g_sysenv_hdl[i] = sysenv_register_driver(&sysenv_chip_info[i]);
    }
    return SYSENV_SUCCESS;
}

