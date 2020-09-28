#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "detect.h"

/* 
 * According to new EEPROM allocation specification, there will be TWO EEPROMs(I2C addr: 0x56, 0x57)
 * on board, CPU both on motherboard and daughter card MUST obey the agreement!
 * 
 * ########################  CPU on daughter card  ############################
 * EEPROM0(I2C addr: 0x56) stores CPU-subsystem-related information:
 *    1) CPU exist on daughter card or on mother board   (offset: 0x1[7])
 *    2) NOR Flash type register                   (offset: 0x4)
 *    3) NAND Flash type register                  (offset: 0x5)
 *    4) DDR type register                         (offset: 0x6)
 *    5) CPU daughter card version                 (offset: 0x7)
 *    6) CPU daughter card serial number           (offset: 0x1220~0x122C)
 *
 * EEPROM1(I2C addr: 0x57) stores Product-related information:
 *    1) Product series register                   (offset: 0x1011)
 *    2) Board type register                       (offset: 0x1012)
 *    3) Board version register                    (offset: 0x1013)
 *    4) Management MAC address                    (offset: 0x1000~0x1005)
 *    5) Board serial number                       (offset: 0x1220~0x122C)
 *    6) System MAC addresses                      (offset: 0x1240~0x1247)
 * 
 *
 * ########################  CPU on mother board  ############################
 * EEPROM0(I2C addr: 0x56) stores CPU-subsystem-related information:
 *    1) CPU exist on daughter card or on mother board   (offset: 0x1[7])
 *    2) Product series register                   (offset: 0x1[6:0])
 *    3) Board type register                       (offset: 0x2)
 *    4) Board version register                    (offset: 0x3)
 *    5) NOR Flash type register                   (offset: 0x4)
 *    6) NAND Flash type register                  (offset: 0x5)
 *    7) DDR type register                         (offset: 0x6)
 *
 * EEPROM1(I2C addr: 0x57) stores Product-related information:
 *    1) Management MAC address                    (offset: 0x1000~0x1005)
 *    2) Board serial number                       (offset: 0x1220~0x122C)
 *    3) System MAC addresses                      (offset: 0x1240~0x1247)
 * 
 * 
 */
/*Every GB board infomation get from EEPROM 0x56 or 0x57. jqiu modify 2013-08-10
  First check EEPROM 0x56, board info at address 0x0-0x6
  If 0x56 is not exist, then check EEPROM 0x57, board info at address 0x1284-0x128a
  If 0x57 is not exist, then default as GB demo board.
*/
#define TMP_INFO_FILE              "/tmp/tmp_board_info"

int ctc_memory_size_array[CTC_MEMORY_MAX_TYPES] =
{
    CTC_MEMORY_SIZE_1G,
    CTC_MEMORY_SIZE_2G,
    CTC_MEMORY_SIZE_512,
    CTC_MEMORY_SIZE_4G,
    CTC_MEMORY_SIZE_256,
    CTC_MEMORY_SIZE_8G,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
    CTC_MEMORY_RESERVE,
};

long get_mem_type(long size)
{
    if(size<=256)
        return CTC_MEMORY_SIZE_256;
    else if(size<=512)
        return CTC_MEMORY_SIZE_512;
    else if(size<=1024)
        return CTC_MEMORY_SIZE_1G;
    else if(size<=2048)
        return CTC_MEMORY_SIZE_2G;
    else if(size<=4096)
        return CTC_MEMORY_SIZE_4G;
    else if(size<=8192)
        return CTC_MEMORY_SIZE_8G;
    else 
        return CTC_MEMORY_SIZE_1G;
}

int
ctc_get_normal_board_info(FILE *fp)
{
    uint8 product_series;           /*board series*/
    uint8 board_type;           /*board type*/
    uint8 hw_ver;                 /*board version*/
    uint8 bm;
    //uint8 nand_flash_type=0;              /*nor flash type*/
    //uint8 dram_type=0;                /*ddr type*/
    uint8 sysmac[6];
    int32 ret = 0;
    char serial_no[MAX_SERIAL_NO_LEN+1];
    int i,len=0;
    
    if(g_eeprom_info.num != 0)
    {
        ret += detect_i2c_read(glb_detect_info,BOARD_SERIES,&product_series, 1);
        ret += detect_i2c_read(glb_detect_info,BOARD_TYPE,&board_type, 1);
        ret += detect_i2c_read(glb_detect_info,BOARD_VER,&hw_ver, 1);
        ret += detect_i2c_read(glb_detect_info,SYSMAC_ADDR,sysmac, 6);
        //ret += detect_i2c_read(glb_detect_info,BOARD_NAND_FLASH,&nand_flash_type, 1);
        //if(nand_flash_type==0xff)
        //    nand_flash_type=0;
        //ret += detect_i2c_read(glb_detect_info,BOARD_DDR_TYPE,&dram_type, 1);
        //if(dram_type==0xff)
        //    dram_type=0;
        ret += detect_i2c_read(glb_detect_info,BOARD_SERIALNO,(uint8 *)serial_no,sizeof(serial_no));
        ret += detect_i2c_read(glb_detect_info,BOARD_MATERIAL,&bm,1);     
    }
    g_product_series = product_series;
    g_board_type     = board_type;
    fprintf(fp, GLB_PRODUCT_SERIES_STRING "  %x\n", product_series);
    fprintf(fp, GLB_BOARD_TYPE_STRING "  %x\n", board_type);
    fprintf(fp, GLB_HW_VERSION_STRING "  %x\n", hw_ver);
    //fprintf(fp, GLB_HW_FLASH_SIZE "  %x\n", ctc_memory_size_array[nand_flash_type]);
    //fprintf(fp, GLB_HW_DRAM_SIZE  "  %x\n", ctc_memory_size_array[dram_type]);
    fprintf(fp, GLB_ROOT_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    fprintf(fp, GLB_SYS_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    fprintf(fp, GLB_PLATFORM_TYPE  "  %x\n", g_platform_type);
    fprintf(fp, GLB_BOARD_MATERIAL_STRING "  %x\n", bm);   
    
    len = serial_no[0];
    for(i=1;i<=len;i++)
    {
        if((( serial_no[i] >= '0') && (serial_no[i] <= '9'))
         ||(( serial_no[i] >= 'A') && (serial_no[i] <= 'Z'))
         ||(( serial_no[i] >= 'a') && (serial_no[i] <= 'z')))
        {
            continue;
        }
        else
            break;
    }
    serial_no[0] = ' '; /* for the first byte is serialno len */
    serial_no[i] = '\0';
    fprintf(fp, GLB_SERIES_NUM "%s\n", serial_no); /*for license, there just be one space*/

    return 0;
}

int ctc_get_normal_sf_board_info(FILE * fp)
{
    uint8 product_series;           /*board series*/
    uint8 board_type;           /*board type*/
    uint8 hw_ver;                 /*board version*/  
    uint8 bm;
    uint8 sysmac[6];
    int32 ret = 0;
    char serial_no[MAX_SERIAL_NO_LEN+1];
    int i;
    char len;

    /* get info from file */
    int mtd_fd = -1;
    char buf[256];
    mtd_fd = open(GLB_MTD_TMP_FILE, O_SYNC|O_RDONLY);
    if(mtd_fd < 0)
    {
       printf("open mtd info file failed\n");
       return 0;
    } 
    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,BOARD_SERIES),SEEK_SET);
    sal_read(mtd_fd,buf,sizeof(buf));
    product_series = buf[0];
    board_type =buf[1];
    hw_ver = buf[2];
    //printf("board_series is 0x%x\nboard_type is 0x%x\nboard_version is 0x%x\n",buf[0],buf[1],buf[2]);

    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,BOARD_MATERIAL),SEEK_SET);
    sal_read(mtd_fd,buf,sizeof(buf));
    bm = buf[0];
    //printf("board_material is 0x%x\n",buf[0]);

    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,SYSMAC_ADDR),SEEK_SET);
    sal_read(mtd_fd,buf,sizeof(buf));
    for (i=0;i<6;i++)
    {
        sysmac[i]=buf[i];
    }
    //printf("sysmac is %02x:%02x:%02x:%02x:%02x:%02x count is %d\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6]);
    
    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,BOARD_SERIALNO),SEEK_SET);
    sal_read(mtd_fd,buf,sizeof(buf));
    sal_sscanf(buf, "%c%s",&len,serial_no);
    serial_no[len+1]='\0';
    //printf("serialno is %s len is %d\n",serial_no,len);

    for(i=0;i<len;i++)
    {
        if((( serial_no[i] >= '0') && (serial_no[i] <= '9'))
         ||(( serial_no[i] >= 'A') && (serial_no[i] <= 'Z'))
         ||(( serial_no[i] >= 'a') && (serial_no[i] <= 'z')))
        {
            continue;
        }
        else
            break;
    }
    serial_no[i] = '\0';
    sal_close(mtd_fd);

    g_product_series = product_series;
    g_board_type     = board_type;
    g_hw_ver = hw_ver;

	/*add by zhw for tmp*/
	product_series = GLB_SERIES_E530;
	board_type     = GLB_BOARD_E530_48T4X;
	g_product_series = product_series;
    g_board_type = board_type;
	sysmac[0]=0x0;
	sysmac[1]=0x9;
	sysmac[2]=0x6;
	sysmac[3]=0x27;
	sysmac[4]=0x3A;
	sysmac[5]=0x3A;
	
    /* save info to file */
    fprintf(fp, GLB_PRODUCT_SERIES_STRING "  %x\n", product_series);
    fprintf(fp, GLB_BOARD_TYPE_STRING "  %x\n", board_type);
    fprintf(fp, GLB_HW_VERSION_STRING "  %x\n", hw_ver);
    fprintf(fp, GLB_ROOT_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    fprintf(fp, GLB_SYS_MAC " %02X:%02X:%02X:%02X:%02X:%02X\n", 
        sysmac[0], sysmac[1], sysmac[2], sysmac[3], sysmac[4], sysmac[5]);
    fprintf(fp, GLB_PLATFORM_TYPE  "  %x\n", g_platform_type);
    fprintf(fp, GLB_BOARD_MATERIAL_STRING "  %x\n", bm);
    fprintf(fp, GLB_SERIES_NUM " %s\n", serial_no);


    char strCmd[256];
    FILE *tmp_fp = NULL;
    char bootimage[256];
    char serverip[128];
    char ipaddr[128];
    char tmp1[128];
    
    sal_memset(strCmd, 0, 256);
    sal_sprintf(strCmd, "fw_printenv 2>&1 >%s", TMP_INFO_FILE);
    system(strCmd);

    sal_memset(bootimage, 0, 256);
    sal_memset(serverip, 0, 128);
    sal_memset(ipaddr, 0, 128);
    tmp_fp = sal_fopen(TMP_INFO_FILE, "r");
    if(tmp_fp)
    {
        sal_memset(buf, 0, 256);
        while(sal_fgets(buf, 256, tmp_fp) != NULL)
        {   
            if(!sal_strncmp(buf, "bootcmd", 7))
            {
                sal_sscanf(buf, "%s %s", tmp1, bootimage);
            }
            if(!sal_strncmp(buf, "ipaddr", 6))
            {
                sal_sscanf(buf, "%s", ipaddr);
            }
            if(!sal_strncmp(buf, "serverip", 8))
            {
                sal_sscanf(buf, "%s", serverip);
            }            
            memset(buf, 0, 258);
        }
        sal_fclose(tmp_fp);
    }
    /*bootcmd: bootcmd=boot_flash
      bootcmd: bootcmd=boot_tftp xxxxOS-xx.x.x.bin&serverip=x.x.x.x*ipaddr=x.x.x.x*/ 
    if(sal_strncmp(tmp1, "bootcmd=boot_flash", sal_strlen("bootcmd=boot_flash"))==0)
    {
      fprintf(fp, "bootimage bootcmd=boot_flash\n");
    }
    else
    {
      fprintf(fp, "bootimage bootcmd=boot_tftp %s&%s&%s\n", bootimage, serverip, ipaddr);
    }

    return ret;
}

int 
ctc_get_normal_mem_info(FILE *fp)
{
    long nand_flash_type=0;              /*nor flash type*/
    long dram_type=0;                /*ddr type*/
    /* get info from file */
    FILE *mem_fd;
    char mem_buf[256];
    long flash_size = 0, mem_size = 0;
    mem_fd = sal_fopen(GLB_MEM_TMP_FILE, "r");
    if(mem_fd == NULL)
    {
       printf("open mem info file failed\n");
       return 0;
    } 
    while(fgets(mem_buf, 256, mem_fd) != NULL)
    {
        uint8 tmp[256];
        char size[20];
        if(!sal_strncmp(mem_buf, "flash", 5))
        {
            sal_sscanf(mem_buf, "%s %s", tmp, size);
            if(( size[0] >= '0') && (size[0] <= '9'))
            {
                flash_size = strtoul(size,NULL,10);
            }
            else
            {
                flash_size=CTC_MEMORY_RESERVE;
            }
        }
        if(!sal_strncmp(mem_buf, "memory", 6))
        {
            sal_sscanf(mem_buf, "%s %s", tmp, size);
            if(( size[0] >= '0') && (size[0] <= '9'))
            {
                mem_size = strtoul(size,NULL,10);
            }
            else
            {
                mem_size=CTC_MEMORY_RESERVE;
            }
        }        
    }
    if(mem_fd)
    {
        sal_fclose(mem_fd);
    }  
    nand_flash_type =get_mem_type(flash_size);
    dram_type =get_mem_type(mem_size);
    fprintf(fp, GLB_HW_FLASH_SIZE "  %lx\n", nand_flash_type);
    fprintf(fp, GLB_HW_DRAM_SIZE  "  %lx\n", dram_type);
    return 0;    
}

int
ctc_get_normal_bootromver(FILE *fp)
{
    char bootrom_ver[32];
    char ary_tmp[256];
    char *chr_p = NULL;
    FILE * fd_proc;
    char cur_tmp[256] = {'\0'};

    sal_memset(bootrom_ver, 0, 32);
    fd_proc = sal_fopen("/proc/cmdline", "r");
    if(!fd_proc)
    {
        sal_strncpy(bootrom_ver, "UNKNOWN", MAX_BOOTROM_VER_LEN);
        return 0;
    }
    sal_fgets(ary_tmp, 256, fd_proc);

    chr_p = sal_strstr(ary_tmp, "U-Boot ");

    if(chr_p != NULL)
    {
        chr_p += sal_strlen("U-Boot ");
        sal_strncpy(cur_tmp, chr_p, 256);
        chr_p = cur_tmp;
        if((chr_p = strchr(chr_p, ' ')) != NULL)
            *chr_p = '\0';

        sal_strncpy(bootrom_ver, cur_tmp, MAX_BOOTROM_VER_LEN);
    }
    else
    {
        sal_strncpy(bootrom_ver, "UNKNOWN", MAX_BOOTROM_VER_LEN);
    }
    sal_fclose(fd_proc);
    
    sal_fprintf(fp, "bootrom %s\n", bootrom_ver);

    return 0;
}

