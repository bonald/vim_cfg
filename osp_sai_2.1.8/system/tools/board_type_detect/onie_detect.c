#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "detect.h"

#define ONIE_INFO_FILE              "/tmp/tmp_board_info"
#define ONIE_PRODUCT_NAME_STRING    "Product Name"
#define ONIE_DEVICE_VER_STRING      "Device Version"
#define ONIE_PLATFORM_NAME_STRING   "Platform Name"
#define ONIE_MAC_ADDR_STRING        "MAC Addresses"
#define ONIE_MANU_DATE_STRING       "Manufacture Date"
#define ONIE_SERIAL_NUMBER_STRING   "Serial Number"
#define ONIE_BASE_MAC_ADDR_STRING   "Base MAC Address"

static void 
ctc_mac_addr_add(uint8* mac, uint32 cnt)
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

/***********************************************************
*Product Name         0x21  10 XX
*Device Version       0x26   1 XX
*Platform Name        0x28  28 XX
*MAC Addresses        0x2A   2 XX
*Serial Number        0x23  12 XX
*Base MAC Address     0x24   6 XX
*Manufacture Date     0x25  19 XX
*CRC-32               0xFE   4 XX
***********************************************************/
int
ctc_get_onie_eeprom_info(FILE *fp)
{
    char buf[256];
    char strCmd[256];
    uint32 tlv_code;
    uint32 tlv_len;
    char tlv_value[256];
    uint8 product_series;           /*board series*/
    uint8 board_type;           /*board type*/
    uint8 hw_ver = 0;                   /*board version*/
    int nand_flash_type;          /*nand flash type*/
    int dram_type;                /*ddr type*/
    uint8 sysmac[6];
    char sn[18];
    int i;
    FILE *tmp_fp;
    
    /* 1 Get board info from onie eeprom */
    sal_memset(strCmd, 0, 256);
#ifdef CNOS_OS    
    sal_sprintf(strCmd, "/bin/onie-syseeprom 1>%s 2>&1", ONIE_INFO_FILE);
#else
    sal_sprintf(strCmd, "/usr/bin/onie-syseeprom 1>%s 2>&1", ONIE_INFO_FILE);
#endif    
    system(strCmd);
    tmp_fp = sal_fopen(ONIE_INFO_FILE, "r");
    if(NULL == tmp_fp)
    {
        product_series = GLB_SERIES_E580;
        board_type = GLB_BOARD_E580_24Q;
        nand_flash_type = CTC_MEMORY_SIZE_2G;
        dram_type = CTC_MEMORY_SIZE_1G;
    }
    while(sal_fgets(buf, 256, tmp_fp) != NULL)
    {
        if(!sal_strncmp(buf, ONIE_PRODUCT_NAME_STRING, sal_strlen(ONIE_PRODUCT_NAME_STRING)))
        {
            sal_sscanf(buf, ONIE_PRODUCT_NAME_STRING"%x %d %s", &tlv_code, &tlv_len, tlv_value);            
            if(!sal_strncmp(tlv_value, "E580-20Q4Z", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_24Q;
            }
            else if(!sal_strncmp(tlv_value, "E580-48X2Q4Z", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_48X2Q4Z;
            }
            else if(!sal_strncmp(tlv_value, "E580-48X6Q", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_48X6Q;
            }
            else if(!sal_strncmp(tlv_value, "E580-32X", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_32X;
            }
            else if(!sal_strncmp(tlv_value, "E580-32X2Q", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_32X2Q;
            }  
            else if(!sal_strncmp(tlv_value, "E580-48XT6Q", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_48XT6Q;
            }
            else if(!sal_strncmp(tlv_value, "E580-WQT48XT4Q", tlv_len))
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_WQT48XT4Q;
            }
            /*phicomm board*/
            else if(!sal_strncmp(tlv_value, "P580-48X6Q", tlv_len))
            {
                product_series = GLB_SERIES_P580;
                board_type = GLB_BOARD_P580_48X6Q;
            }            
            else if(!sal_strncmp(tlv_value, "P580-32X6Q", tlv_len))
            {
                product_series = GLB_SERIES_P580;
                board_type = GLB_BOARD_P580_32X6Q;
            }
            else if(!sal_strncmp(tlv_value, "T65C_8T64X6Q", tlv_len))
            {
                product_series = GLB_SERIES_P580;
                board_type = GLB_BOARD_T65C_8T64X6Q;
            }
            else
            {
                product_series = GLB_SERIES_E580;
                board_type = GLB_BOARD_E580_24Q;
            }

            g_product_series = product_series;
            g_board_type = board_type;
        }
        else if(!sal_strncmp(buf, ONIE_PLATFORM_NAME_STRING, sal_strlen(ONIE_PLATFORM_NAME_STRING)))
        {
            sal_sscanf(buf, ONIE_PLATFORM_NAME_STRING"%x %d %s", &tlv_code, &tlv_len, tlv_value);
            if(!sal_strncmp(tlv_value, "powerpc-centec_e580_20q4z-r0", tlv_len)
                ||!sal_strncmp(tlv_value, "powerpc-centec_e580_48x2q4z-r0", tlv_len)
                ||!sal_strncmp(tlv_value, "powerpc-centec_e580_48x6q-r0", tlv_len)
                ||!sal_strncmp(tlv_value, "powerpc-centec_e580_32x2q-r0", tlv_len)
                ||!sal_strncmp(tlv_value, "powerpc-centec_e580_48xt6q-r0", tlv_len)
                ||!sal_strncmp(tlv_value, "powerpc-centec_t65c_8t64x6q-r0", tlv_len))
            {
                nand_flash_type = CTC_MEMORY_SIZE_2G;
                dram_type = CTC_MEMORY_SIZE_1G;
            }
            else
            {
                nand_flash_type = CTC_MEMORY_SIZE_2G;
                dram_type = CTC_MEMORY_SIZE_1G;
            }
        }
        else if(!sal_strncmp(buf, ONIE_BASE_MAC_ADDR_STRING, sal_strlen(ONIE_BASE_MAC_ADDR_STRING)))
        {
            uint32 tmp[7];
            sal_sscanf(buf, ONIE_BASE_MAC_ADDR_STRING"%x %d %02X:%02X:%02X:%02X:%02X:%02X", &tlv_code, &tlv_len, 
                &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);
            for(i = 0; i < 6; i++)
            {
                sysmac[i] = (uint8)tmp[i];
            }
            tmp[6]='\0';
            ctc_mac_addr_add(sysmac, 1);
        }
        else if(!sal_strncmp(buf, ONIE_SERIAL_NUMBER_STRING, sal_strlen(ONIE_SERIAL_NUMBER_STRING)))
        {
            sal_sscanf(buf, ONIE_SERIAL_NUMBER_STRING"%x %d %s", &tlv_code, &tlv_len, sn);
        }
        else if(!sal_strncmp(buf, ONIE_DEVICE_VER_STRING, sal_strlen(ONIE_DEVICE_VER_STRING)))
        {
            uint32 tmp;
            sal_sscanf(buf, ONIE_DEVICE_VER_STRING"%x %d %d", &tlv_code, &tlv_len, &tmp);
            hw_ver = tmp;
        }
    }
    sal_fclose(tmp_fp);

    fprintf(fp, GLB_PRODUCT_SERIES_STRING "  %x\n", product_series);
    fprintf(fp, GLB_BOARD_TYPE_STRING "  %x\n", board_type);
    fprintf(fp, GLB_HW_VERSION_STRING "  %x\n", hw_ver);
    fprintf(fp, GLB_HW_FLASH_SIZE "  %x\n", nand_flash_type);
    fprintf(fp, GLB_HW_DRAM_SIZE  "  %x\n", dram_type);
    fprintf(fp, GLB_ROOT_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    fprintf(fp, GLB_SYS_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    /*Need to remove a space, otherwise the ECID decoding tool will report an error*/
    fprintf(fp, GLB_SERIES_NUM  " %s\n", sn);
    fprintf(fp, GLB_PLATFORM_TYPE  "  %x\n", g_platform_type);

    return 0;
}

int
ctc_get_onie_bootrom_ver(FILE *fp)
{
    char strCmd[256];
    FILE *tmp_fp = NULL;
    char buf[128];
    char bootrom_ver[32];
    char *p;
    
    sal_memset(strCmd, 0, 256);
#ifdef CNOS_OS    
#ifdef HAVE_DEBIAN
    sal_sprintf(strCmd, "fw_printenv ver>>%s 2>&1", ONIE_INFO_FILE);
#else
    sal_sprintf(strCmd, "/bin/fw_printenv ver>>%s 2>&1", ONIE_INFO_FILE);
#endif
#else
#ifdef HAVE_DEBIAN
    sal_sprintf(strCmd, "fw_printenv ver>>%s 2>&1", ONIE_INFO_FILE);
#else
    sal_sprintf(strCmd, "/usr/sbin/fw_printenv ver>>%s 2>&1", ONIE_INFO_FILE);
#endif
#endif        

    system(strCmd);

    sal_memset(bootrom_ver, 0, 32);
    sal_strncpy(bootrom_ver, "UNKNOWN", sal_strlen("UNKNOWN"));
    tmp_fp = sal_fopen(ONIE_INFO_FILE, "r");
    if(tmp_fp)
    {
        sal_memset(buf, 0, 128);
        while(sal_fgets(buf, 128, tmp_fp) != NULL)
        {   
            
            if((p = sal_strstr(buf, "ONIE master-")))
            {
                p += sal_strlen("ONIE master-"); 
                sal_sprintf(bootrom_ver, "ONIE-master-%s", p);
                bootrom_ver[24] = '\0';
                break;
            }
            else if((p = sal_strstr(buf, "- V")))
            {
                p += sal_strlen("- V"); 
                sal_strncpy(bootrom_ver, p, 5);
                bootrom_ver[5] = '\0';
                break;
            }
            memset(buf, 0, 128);
        }
        sal_fclose(tmp_fp);
    }
    fprintf(fp, "bootrom %s\n", bootrom_ver);

    return 0;
}

int
ctc_get_onie_bootimage(FILE *fp)
{
    char strCmd[256];
    FILE *tmp_fp = NULL;
    char buf[128];
    char bootimage[128];
    char serverip[128];
    char ipaddr[128];
    char *p;
    
    sal_memset(strCmd, 0, 256);
#ifdef CNOS_OS    
#ifdef HAVE_DEBIAN
    sal_sprintf(strCmd, "fw_printenv hw_load serverip ipaddr >>%s 2>&1", ONIE_INFO_FILE);
#else
    sal_sprintf(strCmd, "/bin/fw_printenv hw_load serverip ipaddr >>%s 2>&1", ONIE_INFO_FILE);
#endif
#else
#ifdef HAVE_DEBIAN
    sal_sprintf(strCmd, "fw_printenv hw_load serverip ipaddr >>%s 2>&1", ONIE_INFO_FILE);
#else
    sal_sprintf(strCmd, "/usr/sbin/fw_printenv hw_load serverip ipaddr >>%s 2>&1", ONIE_INFO_FILE);
#endif
#endif
    system(strCmd);

    sal_memset(bootimage, 0, 128);
    sal_memset(serverip, 0, 128);
    sal_memset(ipaddr, 0, 128);
    sal_strncpy(bootimage, "boot_flash", sal_strlen("boot_flash"));
    tmp_fp = sal_fopen(ONIE_INFO_FILE, "r");
    if(tmp_fp)
    {
        sal_memset(buf, 0, 128);
        while(sal_fgets(buf, 128, tmp_fp) != NULL)
        {   
            if((p = sal_strstr(buf, "hw_load=tftpboot")) != NULL)
            {
                p += sal_strlen("hw_load=tftpboot"); 
                p++;
                sal_strncpy(bootimage, p, sal_strlen(p)-1);
                bootimage[sal_strlen(p)-1]='\0';
            }
            else if((p = sal_strstr(buf, "serverip=")) != NULL)
            {
                sal_strncpy(serverip, p, sal_strlen(p)-1);
                serverip[sal_strlen(p)-1]='\0';
            }
            else if((p = sal_strstr(buf, "ipaddr=")) != NULL)
            {
                sal_strncpy(ipaddr, p, sal_strlen(p)-1);
                ipaddr[sal_strlen(p)-1]='\0';
            }
            memset(buf, 0, 128);
        }
        sal_fclose(tmp_fp);
    }
  /*bootcmd: bootcmd=boot_flash
    bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x*ipaddr=x.x.x.x*/ 
    if(sal_strncmp(bootimage, "boot_flash", sal_strlen("boot_flash"))==0)
    {
        fprintf(fp, "bootimage bootcmd=boot_flash\n");
    }
    else
    {
        fprintf(fp, "bootimage bootcmd=boot_tftp %s&%s&%s\n", bootimage, serverip, ipaddr);
    }

    return 0;
}

