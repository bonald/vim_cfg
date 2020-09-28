/****************************************************************************
 * eeprom_api.c    eeprom api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-09-26.
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
#include "eeprom_api.h"
#include "eeprom_drv.h"
#define ENV_SIZE  2048
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#define	min3(a,b,c)	min(min((a),(b)), (c))




/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
/* eeprom handler */
static eeprom_handle_t **g_eeprom_hdl;
static uint32 g_eeprom_num;



/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/



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

static eeprom_handle_t* 
_get_eeprom_handle(uint32 eeprom_idx)
{
    eeprom_handle_t* p_hdl;

    if (g_eeprom_num <= eeprom_idx)
    {
        return NULL;
    }

    p_hdl = g_eeprom_hdl[eeprom_idx];

    return p_hdl;
}

/*********************************************************************
 * Name    : eeprom_mgrif_crc_serial_param
 * Purpose :  crc serial parameter
 * Input   : uint32 param         - the serial parameter
          int32 value             - the value       
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    :  Refer to uboot board.c
              0x1050  SERIAL_PARAM_ADDR crc
              0x1054  baudrate
              0x1058  databit
              0x105C  stopbit 
              0x1060  parity 

              Refer to uboot command.c
              crc_serial_param(int param, int value)
              crc = baudrate + databit * 10 + stopbits + parity; 
              0    none parity
              3    odd parity
              6    even parity

*********************************************************************/
int32
eeprom_mgrif_crc_serial_param(int32 param, int32 value)
{
    return RESULT_OK;
}

/*********************************************************************
 * Name    : eeprom_set_serial_no
 * Purpose :  set serial number  
 * Input   : uint8 * serial_no       - the pointer of serial number
          uint8 serial_len  - the serial number of length       
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_set_serial_no(uint8* p_serial_no, uint8 serial_len)
{
    return RESULT_OK;
}

/*********************************************************************
 * Name    : eeprom_get_serial_no
 * Purpose :  get serial number  
 * Input   : uint8 * serial_no       - the pointer of serial number
          uint8* serial_len  - the pointer of serial number length  
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_serial_no(uint8* p_serial_no, uint8* p_serial_len, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_serial_no);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    /* get length */
    para.len = 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = p_serial_len;
    ret += eeprom_read(p_eeprom_info->eeprom_idx, &para);

    if (MAX_SERIAL_NO_LEN < *p_serial_len)
    {
        *p_serial_len = MAX_SERIAL_NO_LEN;
    }
    
    /* get serial no */
    para.len = *p_serial_len;
    para.offset = p_eeprom_info->base_addr+1;
    para.p_val = p_serial_no;
    ret += eeprom_read(p_eeprom_info->eeprom_idx, &para);
    /*fix bug29801, cr9899, liangf, 2014-08-28*/
    p_serial_no[*p_serial_len] = '\0';
    
    if ( ret >= 0)
    { 
        DRV_LOG_DEBUG(DRV_EEPROM, "Get serial no :%s",p_serial_no);
    }

    return ret;
}

/*********************************************************************
 * Name    : eeprom_get_oem_info
 * Purpose :  get oem info  
 * Input   : uint8 * p_oem_info       - the pointer of oem info
          the oem info buffer is static len MAX_EEPROM_OEM_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_oem_info(uint8* p_oem_info, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int read_len = 0;

    DRV_CTC_CHK_PTR(p_oem_info);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    read_len = MAX_EEPROM_OEM_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = read_len;
        para.offset = p_eeprom_info->base_addr+ i*read_len;
        para.p_val = p_oem_info + i*read_len;
        ret |= eeprom_read(p_eeprom_info->eeprom_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : eeprom_set_oem_info
 * Purpose :  set oem info  
 * Input   : uint8 * p_oem_info       - the pointer of oem info
          the oem info buffer is static len MAX_EEPROM_OEM_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_set_oem_info(uint8* p_oem_info, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int write_len = 0;

    DRV_CTC_CHK_PTR(p_oem_info);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    write_len = MAX_EEPROM_OEM_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = write_len;
        para.offset = p_eeprom_info->base_addr+ i*write_len;
        para.p_val = p_oem_info + i*write_len;
        ret |= eeprom_write(p_eeprom_info->eeprom_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : eeprom_get_port_info
 * Purpose : get port info  
 * Input   : uint8 * p_port_info       - the pointer of port info
             the port info buffer is static len MAX_PORT_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_port_info(uint8* p_port_info, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int read_len = 0;

    DRV_CTC_CHK_PTR(p_port_info);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    read_len = MAX_PORT_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = read_len;
        para.offset = p_eeprom_info->base_addr+ i*read_len;
        para.p_val = p_port_info + i*read_len;
        ret |= eeprom_read(p_eeprom_info->eeprom_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}

/*********************************************************************
 * Name    : eeprom_set_port_info
 * Purpose : set port info  
 * Input   : uint8 * p_port_info       - the pointer of port info
             the port info buffer is static len MAX_PORT_INFO_LEN
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
             other       = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_set_port_info(uint8* p_port_info, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = 0;
    int i = 0;
    int loop_max = 8;
    int write_len = 0;

    DRV_CTC_CHK_PTR(p_port_info);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    write_len = MAX_PORT_INFO_LEN/loop_max;
    for (i = 0; i < loop_max; i ++)
    {
        para.len = write_len;
        para.offset = p_eeprom_info->base_addr+ i*write_len;
        para.p_val = p_port_info + i*write_len;
        ret |= eeprom_write(p_eeprom_info->eeprom_idx, &para);
        usleep(20*1000);
    }
    
    return ret;
}


/*********************************************************************
 * Name    : eeprom_get_system_mac
 * Purpose :  get system mac  
 * Input   : uint8* p_buf       - the pointer of buffer 
          eeprom_info_t *p_eeprom_info       - the eeprom info                  
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_system_mac(uint8* p_buf, eeprom_info_t *p_eeprom_info)
{
    eeprom_para_t para;
    int ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_buf);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    /* get system mac to buffer */
    para.len = GLB_ETH_ADDR_LEN + 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = p_buf; 
    if (eeprom_read(p_eeprom_info->eeprom_idx, &para) < 0)
    {
        ret = RESULT_ERROR;
    }  
        
    if (RESULT_OK == ret)
    { 
        DRV_LOG_DEBUG(DRV_EEPROM, "Get system mac %02X:%02X:%02X:%02X:%02X:%02X",
                p_buf[0],p_buf[1],p_buf[2],p_buf[3],p_buf[4],p_buf[5]);
    }

    return ret;
}


/*********************************************************************
 * Name    : eeprom_set_datapath
 * Purpose :  set datapath
 * Input   : uint8 datapath       - the datapath
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_set_datapath(uint8 datapath, eeprom_info_t* p_eeprom_info)
{
    uint8 buf[1];    
    eeprom_para_t para;
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    sal_memset (buf, 0, sizeof(buf));
    para.len = 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = buf;
    
    buf[0] = datapath;  
    if (eeprom_write (p_eeprom_info->eeprom_idx, &para) < 0)
    {
        DRV_LOG_DEBUG(DRV_EEPROM, "Set datapath mode=%d fail!", datapath);   
        ret = RESULT_ERROR;
    }  
  
    return ret;
}

/*********************************************************************
 * Name    : eeprom_get_datapath
 * Purpose :  get datapath
 * Input   : 
   
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_datapath(uint8* datapath, eeprom_info_t* p_eeprom_info)
{
    eeprom_para_t para;
    uint8 buf[1];
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(datapath);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    /* get system mac to buffer */
    sal_memset (buf, 0, sizeof(buf));
    para.len = 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = buf;
    if (eeprom_read(p_eeprom_info->eeprom_idx, &para) < 0)
    {
        ret = RESULT_ERROR;
    }  
    
    *datapath = buf[0]; 
        
    if (RESULT_OK == ret)
    { 
        DRV_LOG_DEBUG(DRV_EEPROM, "Get datapath mode=%d", *datapath);              
    }

    return ret;

}


/*********************************************************************
 * Name    : eeprom_set_tcam_profile
 * Purpose :  set tcam profile
 * Input   : uint8* p_buf       - the buff
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_set_tcam_profile(uint32 mode, eeprom_info_t* p_eeprom_info)
{
    unsigned char buf[5];    
    eeprom_para_t para;
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    sal_memset (buf, 0, sizeof(buf));
    para.len = 5;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = buf;  
    
    buf[0] = 0xaa; /*0x1800 should written tcam flag, for chsm to judge*/
  
    /*from 0x1801, the value is tcam mode*/
    buf[1] = mode & 0xff;
    buf[2] = (mode >> 8) & 0xff;
    buf[3] = (mode >> 16) & 0xff;
    buf[4] = (mode >> 24) & 0xff;
  
    if (eeprom_write (p_eeprom_info->eeprom_idx, &para) < 0)
    {
        DRV_LOG_DEBUG(DRV_EEPROM, "Set stm profile mode=%d fail!", mode);   
        ret = RESULT_ERROR;
    }  
  
    return ret;
}

/*********************************************************************
 * Name    : eeprom_get_tcam_profile
 * Purpose :  get tcam profile
 * Input   :  uint8* p_buf       - the buff
   
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_get_tcam_profile(uint32* p_mode, eeprom_info_t* p_eeprom_info)
{
    eeprom_para_t para;
    uint8 buf[4];
    int32 ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_mode);
    DRV_CTC_CHK_PTR(p_eeprom_info);
    /*modified by jcao for bug 14259, 2011-05-19*/
    EEPROM_ITEM_CHK(p_eeprom_info);
    
    /* get system mac to buffer */
    sal_memset (buf, 0, sizeof(buf));
    para.len = 4;
    para.offset = p_eeprom_info->base_addr + 1;
    para.p_val = buf; 
    if (eeprom_read(p_eeprom_info->eeprom_idx, &para) < 0)
    {
        ret = RESULT_ERROR;
    }  

    *p_mode = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]; 
        
    if (RESULT_OK == ret)
    { 
        DRV_LOG_DEBUG(DRV_EEPROM, "Get stm profile mode=%d", *p_mode);              
    }

    return ret;  
}
/*modify for bug 14775. jqiu 2011-05-13*/
static int32 search_eeprom_str(char* buf, uint buf_len, char *str, int* len)
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

/*modify for bug 14775. jqiu 2011-05-13*/
/*bootcmd: bootcmd=boot_flash
  bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x&ipaddr=x.x.x.x
  bootcmd: bootcmd=boot_tftp6 xxxxOS-xx.x.x.bin&serverip6=x:x:x:x&ip6addr=x:x:x:x
  bootcmd: bootcmd=bootmtd */
  
int32
eeprom_set_bootcmd(eeprom_info_t* p_eeprom_info, char* bootcmd)
{
    eeprom_para_t para;
    uint8* src_buf;
    uint8* dst_buf;
    uint16 loop_max, read_len, i, j, k, len;
    uint8 erase_flag_ipv4=0, tmp,bootcmd_flag=0;
    uint32 crc =0;
    int ret=0;
    uint8 is_tftp6 = 0;
    uint8 erase_flag_ipv6=0;
    
    DRV_CTC_CHK_PTR(p_eeprom_info);
    DRV_CTC_CHK_PTR(bootcmd);
    
    read_len = 32;
    loop_max = ENV_SIZE/read_len;
    src_buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_EEPROM_INFO, ENV_SIZE+1);
    if(src_buf == NULL)
        return -1;
    dst_buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_EEPROM_INFO, ENV_SIZE+M_FULLPATH_MAX_LEN);
    if(dst_buf == NULL)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, src_buf);
        return -1;    
    }
    for (k = 0; k< loop_max; k++)
    {
        para.len = read_len;
        para.offset = p_eeprom_info->base_addr+ k*read_len;
        para.p_val = src_buf + k*read_len;
        ret |= eeprom_read(p_eeprom_info->eeprom_idx, &para);
        usleep(10*1000);
    }

    if((sal_strstr(bootcmd, "bootcmd=boot_flash") == NULL) && (sal_strstr(bootcmd, "bootcmd=bootmtd") == NULL))
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
    /*found out the last diff byte, then just need to write into eeprom until this byte*/
    k = ENV_SIZE-1; /*fix bug38213, liangf, 2016-05*/
    while((src_buf[k] == dst_buf[k]) && (k>0))
    {
        k--;
    }
    /*found out the first diff byte, then just need to write into eeprom from this byte*/
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
        para.offset = p_eeprom_info->base_addr;
        para.p_val = (uint8 *)&crc;
        ret |= eeprom_write(p_eeprom_info->eeprom_idx, &para);
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
            para.offset = p_eeprom_info->base_addr+j;
            para.p_val = (uint8 *)&dst_buf[j];
            ret = eeprom_write(p_eeprom_info->eeprom_idx, &para);
            usleep(10*1000);
            j = j+32;
        }
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, src_buf);
    DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, dst_buf);
    return ret;
}

/*bootcmd: bootcmd=boot_flash
  bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x*ipaddr=x.x.x.x
  bootcmd: bootcmd=boot_tftp6 xxxxOS-xx.x.x.bin&serverip6=x:x:x:x&ip6addr=x:x:x:x*/
int32
eeprom_get_bootcmd(eeprom_info_t* p_eeprom_info,char* bootcmd)
{
    eeprom_para_t para;
    char* buf;
    int32 ret = RESULT_OK;
    int len=0, read_len, loop_max, k;
    char boot_image[128+18+1]; //there include 'bootcmd=boot_tftp ' 18 byte.
    char serverip[64+1];
    char ipaddr[64+1];

    DRV_CTC_CHK_PTR(p_eeprom_info);
    DRV_CTC_CHK_PTR(bootcmd);

    sal_memset(boot_image, 0, sizeof(boot_image));
    sal_memset(serverip, 0, sizeof(serverip));
    sal_memset(ipaddr, 0, sizeof(ipaddr));
    buf = DRV_CALLOC(CTCLIB_MEM_DRIVER_EEPROM_INFO, ENV_SIZE+1);
    if(buf == NULL)
    {
        DRV_LOG_ERR("eeprom calloc memory fail!\n");
        return RESULT_ERROR;
    }
    read_len = 32;
    loop_max = ENV_SIZE/read_len;
    for (k = 0; k< loop_max; k++)
    {
        para.len = read_len;
        para.offset = p_eeprom_info->base_addr+ k*read_len;
        para.p_val = (uint8 *)buf + k*read_len;
        ret |= eeprom_read(p_eeprom_info->eeprom_idx, &para);
        usleep(10*1000);
    }

    ret=search_eeprom_str(buf, ENV_SIZE, "bootcmd=",&len);
    if(ret<=0)
    {
        DRV_LOG_ERR("read bootcmd fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return RESULT_ERROR;
    }
    
    sal_memcpy(boot_image, &buf[ret], len>146?146:len);
    if(sal_strstr(boot_image, "boot_flash") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return 0;
    }
    /* added by tongzb for bug 42163 support bootmtd 2017-01-01 */
    else if(sal_strstr(boot_image, "bootmtd") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return 0;
    }
    else if(sal_strstr(boot_image, "boot_tftp6") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        /*need to find serverip and ipaddr*/
        ret=search_eeprom_str(buf, ENV_SIZE, "serverip6=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(serverip, &buf[ret], len>128?128:len);
            sal_strcat(bootcmd, serverip);
        }
        ret=search_eeprom_str(buf, ENV_SIZE, "ip6addr=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(ipaddr, &buf[ret], len>128?128:len);
            sal_strcat(bootcmd, ipaddr);
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return 0;
    }
    else if(sal_strstr(boot_image, "boot_tftp") != NULL)
    {
        sal_strcpy(bootcmd, boot_image);
        /*need to find serverip and ipaddr*/
        ret=search_eeprom_str(buf, ENV_SIZE, "serverip=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(serverip, &buf[ret], len>64?64:len);
            sal_strcat(bootcmd, serverip);
        }
        ret=search_eeprom_str(buf, ENV_SIZE, "ipaddr=",&len);
        if(ret > 0)
        {
            sal_strcat(bootcmd, "&");
            sal_memcpy(ipaddr, &buf[ret], len>64?64:len);
            sal_strcat(bootcmd, ipaddr);
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return 0;
    }
    else
    {
        DRV_LOG_ERR("read bootcmd fail!\n");
        DRV_FREE(CTCLIB_MEM_DRIVER_EEPROM_INFO, buf);
        return RESULT_ERROR;
    }    
}




/*********************************************************************
 * Name    : eeprom_get_chassis_type
 * Purpose :  get chassis type
 * Input   : glb_chassis_type_t* chassis_type   ---chassis type
   
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
eeprom_get_chassis_type(glb_chassis_type_t* chassis_type)
{
    return RESULT_OK;
}

/*********************************************************************
 * Name    : eeprom_read
 * Purpose :  read eeprom register
 * Input   : uint32 idx         - the index of eeprom
          eeprom_para_t *p_para     - the parmeter       
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
eeprom_read(uint32_t idx, eeprom_para_t* p_para)
{
    eeprom_access_t eeprom_access;
    eeprom_handle_t* p_hdl = NULL;

    p_hdl = _get_eeprom_handle(idx);
    
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl);
    
    eeprom_access.len = p_para->len;
    eeprom_access.offset = p_para->offset;
    eeprom_access.p_val = p_para->p_val;
    
    return p_hdl->read(p_hdl, &eeprom_access);
}



/*********************************************************************
 * Name    : eeprom_write
 * Purpose :  write eeprom register
 * Input   : uint32 idx         - the index of eeprom
          eeprom_para_t *ppara     - the parmeter       
                              
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : Be attention!!!!!
            If you want to write data into eeprom which length is larger than 1, you must be 
            attention that the start addr is start from 32 times. because when eeprom write 
            the internal address just increase for low 5 bits. if not for 32 times, maybe roll back.
            below copy from eeprom datasheet.
The data word address lower 5 bits are internally incremented
following the receipt of each data word. The higher
data word address bits are not incremented, retaining the
memory page row location. When the word address, internally
generated, reaches the page boundary, the following
byte is placed at the beginning of the same page. If more
than 32 data words are transmitted to the EEPROM, the
data word address will ¡°roll over¡± and previous data will be
overwritten.            
*********************************************************************/

int32 
eeprom_write(uint32_t idx, eeprom_para_t* p_para)
{
    eeprom_access_t eeprom_access;
    eeprom_handle_t* p_hdl = NULL;

    p_hdl = _get_eeprom_handle(idx);
    
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl);
    
    eeprom_access.len = p_para->len;
    eeprom_access.offset = p_para->offset;
    eeprom_access.p_val = p_para->p_val;
    
    return p_hdl->write(p_hdl, &eeprom_access);
}

/*********************************************************************
 * Name    : eeprom_close
 * Purpose :  free memory and pointer
 * Input   : uint32 idx     - the index of eeprom
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
eeprom_close(uint32 idx)
{
    eeprom_handle_t* p_hdl = NULL;

    p_hdl = _get_eeprom_handle(idx);
    
    DRV_CTC_CHK_PTR(p_hdl);   
    
    return p_hdl->close(p_hdl);
}

int32 _eeprom_bootup_diag_test(eeprom_info_t *p_eeprom_info, eeprom_diag_result_t* diag_result)
{
    int32 ret1 = RESULT_OK, ret2 = RESULT_OK;
    eeprom_para_t para;
    uint8 value2= 0x55;
    uint8 value1 =0;
   
    DRV_CTC_CHK_PTR(p_eeprom_info);
    DRV_CTC_CHK_PTR(diag_result);
   
    para.len = 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = &value2;
    ret1 = eeprom_write(p_eeprom_info->eeprom_idx, &para);
    if (ret1 < 0)
    {
       diag_result->wr_fail = 1;
       ret2 = RESULT_ERROR;
    }

    sal_task_sleep(10);
    
    para.len = 1;
    para.offset = p_eeprom_info->base_addr;
    para.p_val = &value1;
    ret1 = eeprom_read(p_eeprom_info->eeprom_idx, &para);
    if (ret1 < 0)
    {
       diag_result->rd_fail = 1;
       ret2 = RESULT_ERROR;
    }
 
    if(value1 != value2)
    {
        diag_result->cmp_fail = 1;
        ret2 = RESULT_ERROR;
    }   
    
    return ret2;
}

int32 eeprom_bootup_diag_and_report(eeprom_info_t *p_eeprom_info)
{
#ifndef _GLB_UML_SYSTEM_    
    eeprom_diag_result_t diag_result;
    uint32 result = 0;

    sal_memset(&diag_result, 0, sizeof(eeprom_diag_result_t));
    result = _eeprom_bootup_diag_test(p_eeprom_info, &diag_result); 
    if (result != RESULT_OK)
    {
        if(diag_result.rd_fail)
        {
            DRV_DIAG_LOG("EEPROM%d diag read fail!\n", p_eeprom_info->eeprom_idx);    
        }
        if(diag_result.wr_fail)
        {
            DRV_DIAG_LOG("EEPROM%d diag write fail!\n", p_eeprom_info->eeprom_idx);    
        }
        if(diag_result.cmp_fail)
        {
            DRV_DIAG_LOG("EEPROM%d diag cmp fail!\n", p_eeprom_info->eeprom_idx);    
        }    
        DRV_DIAG_LOG("EEPROM%d diag fail!\n", p_eeprom_info->eeprom_idx);
        return RESULT_ERROR;
    }
    else
    {
        DRV_DIAG_LOG("EEPROM%d diag pass!\n", p_eeprom_info->eeprom_idx);
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}

/*fix bug39335, define sw_ver, liangf, 2016-07-06*/
int32
eeprom_set_sw_ver(eeprom_info_t *p_eeprom_info, uint8* p_sw_ver, uint8 sw_ver_len)
{
    eeprom_para_t para;
    uint8 sw_ver[MAX_SWVER_LEN+1];
    int ret = RESULT_OK;

    DRV_CTC_CHK_PTR(p_sw_ver);
    EEPROM_ITEM_CHK(p_eeprom_info);
        
    if (0 == sw_ver_len)
    {
        sw_ver[0] = sw_ver_len;
        para.len = 1;
        para.offset = p_eeprom_info->base_addr;
        para.p_val = sw_ver;
    }
    else
    {
        sw_ver[0] = sw_ver_len;
        sal_memcpy(sw_ver+1, p_sw_ver, sw_ver_len);            
        para.len = sw_ver_len+1;
        para.offset = p_eeprom_info->base_addr;
        para.p_val = sw_ver;
    }
    
    ret += eeprom_write(p_eeprom_info->eeprom_idx, &para);
    
    if (ret < 0)
    { 
        DRV_LOG_DEBUG(DRV_EEPROM, "eeprom_set_sw_ver write fail!");   
    }

    return ret;
}

/*********************************************************************
 * Name    : eeprom_init
 * Purpose :  init some data structure 
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of eepromn'i2c bus implement
          uint32 num         - the number of eeprom
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_init(i2c_gen_t* i2c_gen, uint32 num)
{
    int32 i = 0;

    DRV_CTC_CHK_PTR(i2c_gen);   
    
    g_eeprom_hdl = (eeprom_handle_t** )DRV_MALLOC(CTCLIB_MEM_DRIVER_EEPROM_INFO, sizeof(eeprom_handle_t* )*num);
    if(NULL == g_eeprom_hdl)
    {
        DRV_LOG_ERR("eeprom alloc handler fail!\n");
        return RESULT_ERROR;
    }

    /* register eeprom dev */
    for (i = 0; i < num; i++)
    {
        g_eeprom_hdl[i] = eeprom_register(E_EEPROM_I2C, (void *)&i2c_gen[i]);
    }
    g_eeprom_num = num;
    return RESULT_OK;
}



