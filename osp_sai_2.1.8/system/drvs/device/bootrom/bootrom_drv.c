#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>      
#include "genlog.h"
#include "drv_debug.h"
#include "ctclib_crc.h"
#include "ctclib_sys_cmd.h"

#include "glb_tempfile_define.h"
#include "glb_hw_define.h"
typedef enum
{
    E_CPU_P1010,
    E_CPU_P1014,    
}E_CPU_TYPE;


int check_boot_bin(char* src_file, uint8 type)
{
    FILE* fd;
    int ret, file_len;
    char buffend[21];
    char* buff;
    uint32 crc, store_crc;
    struct stat stat_buf;

    if(!sal_stat(src_file, &stat_buf))
    {
        file_len = stat_buf.st_size;
    }
    else
    {
        DRV_LOG_ERR("Get src file stat failed!\n");
        return -1;
    }
    /* Modified by liuht for bug 25654, 2013-11-08 */	
    if(file_len < 0x80000)
    {
        DRV_LOG_ERR("Boot src file length is error!\n");
        return -1;
    }
    
    fd = sal_fopen(src_file, "r");
    if(fd == NULL) 
    {
        DRV_LOG_ERR("Open boot src file failed!\n");
        return -1;
    }
    sal_fseek(fd, -16, SEEK_END);
    ret = sal_fread(buffend, 1, 16, fd);
    if(ret != 16) 
    {        
        DRV_LOG_ERR("read file descriptor error, ret %d\n", ret);
        goto ERROR;
    }
    
    /* Added by liuht for bug44055, 2017-0601 */
    /*CPU type check. */
    if(E_BOOTROM_TYPE_512K == type)
    {
        if(0 != sal_strncmp(buffend, "p101xcrc", 8))
        {
            DRV_LOG_ERR("Wrong bootrom version for bootrom type 512k.\n");
            goto ERROR;
        }
    }
    else if(E_BOOTROM_TYPE_4M== type)
    {
        if(0 != sal_strncmp(buffend, "p1010crc", 8))
        {
            DRV_LOG_ERR("Wrong bootrom version for bootrom type 4M.\n");
            goto ERROR;
        }
    }
    else
    {
        DRV_LOG_ERR("Unknow bootrom type.\n");
        goto ERROR;
    }

    sal_fseek(fd, 0, SEEK_SET);
    /* for bug 51578, can not malloc more than 128k mem from XXX_MALLOC, instead sal_malloc  */
    //buff = (char *)DRV_MALLOC(CTCLIB_MEM_DRIVER_BOOTROM_INFO, file_len);
    buff = sal_malloc(file_len);
    if(buff == NULL)
    {
        DRV_LOG_ERR("Malloc memory to calculate crc failed!\n");
        goto ERROR;
    }
    ret = sal_fread(buff, 1, file_len-16, fd);
    if(ret != (file_len-16))
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Read file length error!ret %d length %d\n",ret, file_len-16);
        goto ERROR;
    }
    /*CRC checksum check. */    
    /*last checkinfo not in crc calculate.*/
    crc = ctclib_gen_crc32(0, buff, file_len-16);    
    store_crc = (((uint8)buffend[11])<<24)+(((uint8)buffend[10])<<16)+(((uint8)buffend[9])<<8)+((uint8)buffend[8]);
    if(crc != store_crc)
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Crc check failed.\n");
        goto ERROR;
    }
    sal_fclose(fd);
    //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
    sal_free(buff);
    return 0;
ERROR:
    sal_fclose(fd);
    return -1;
}

#ifdef _CTC_ARM_HI3535_
static int check_hi3535_boot_bin(char* src_file)
{
    FILE* fd;
    int ret, file_len;
    char buffend[21];
    char* buff;
    uint32 crc, store_crc;
    struct stat stat_buf;

    if(!sal_stat(src_file, &stat_buf))
    {
        file_len = stat_buf.st_size;
    }
    else
    {
        DRV_LOG_ERR("Get src file stat failed!\n");
        return -1;
    }
    
    fd = sal_fopen(src_file, "r");
    if(fd == NULL) 
    {
        DRV_LOG_ERR("Open boot src file failed!\n");
        return -1;
    }
    sal_fseek(fd, -13, SEEK_END);
    ret = sal_fread(buffend, 1, 13, fd);
    if(ret != 13) 
    {        
        DRV_LOG_ERR("read file descriptor error, ret %d\n", ret);
        goto ERROR;
    }
    if(0 != sal_strncmp(buffend, "hi3535crc", 9))
    {
        DRV_LOG_ERR("Wrong bootrom version.\n");
        goto ERROR;
    }

    sal_fseek(fd, 0, SEEK_SET);
    /* for bug 51920, can not malloc more than 128k mem from XXX_MALLOC, instead sal_malloc  */
    //buff = (char *)DRV_MALLOC(CTCLIB_MEM_DRIVER_BOOTROM_INFO, file_len);
    buff = sal_malloc(file_len);
    if(buff == NULL)
    {
        DRV_LOG_ERR("Malloc memory to calculate crc failed!\n");
        goto ERROR;
    }
    ret = sal_fread(buff, 1, file_len-13, fd);
    if(ret != (file_len-13))
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Read file length error!ret %d length %d\n",ret, file_len-13);
        goto ERROR;
    }
    /*CRC checksum check. */    
    /*last checkinfo not in crc calculate.*/
    crc = ctclib_gen_crc32(0, buff, file_len-13);    
    store_crc = (((uint8)buffend[12])<<24)+(((uint8)buffend[11])<<16)+(((uint8)buffend[10])<<8)+((uint8)buffend[9]);
    if(crc != store_crc)
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Crc check failed.\n");
        goto ERROR;
    }
    sal_fclose(fd);
    DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
    //sal_free(buff);
    return 0;
ERROR:
    sal_fclose(fd);
    return -1;
}
#endif /* !_CTC_ARM_HI3535_ */

#ifdef _CTC_ARM_LS1023A_
static int check_ls1023a_boot_bin(char* src_file)
{
    FILE* fd;
    int ret, file_len;
    char buffend[21];
    char* buff;
    uint32 crc, store_crc;
    struct stat stat_buf;

    if(!sal_stat(src_file, &stat_buf))
    {
        file_len = stat_buf.st_size;
    }
    else
    {
        DRV_LOG_ERR("Get src file stat failed!\n");
        return -1;
    }
    
    fd = sal_fopen(src_file, "r");
    if(fd == NULL) 
    {
        DRV_LOG_ERR("Open boot src file failed!\n");
        return -1;
    }
    sal_fseek(fd, -14, SEEK_END);
    ret = sal_fread(buffend, 1, 14, fd);
    if(ret != 14) 
    {        
        DRV_LOG_ERR("read file descriptor error, ret %d\n", ret);
        goto ERROR;
    }
    if(0 != sal_strncmp(buffend, "ls1023acrc", 10))
    {
        DRV_LOG_ERR("Wrong bootrom version.\n");
        goto ERROR;
    }

    sal_fseek(fd, 0, SEEK_SET);
    /* for bug 51920, can not malloc more than 128k mem from XXX_MALLOC, instead sal_malloc  */
    //buff = (char *)DRV_MALLOC(CTCLIB_MEM_DRIVER_BOOTROM_INFO, file_len);
    buff = sal_malloc(file_len);
    if(buff == NULL)
    {
        DRV_LOG_ERR("Malloc memory to calculate crc failed!\n");
        goto ERROR;
    }
    ret = sal_fread(buff, 1, file_len-14, fd);
    if(ret != (file_len-14))
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Read file length error!ret %d length %d\n",ret, file_len-14);
        goto ERROR;
    }
    /*CRC checksum check. */    
    /*last checkinfo not in crc calculate.*/
    crc = ctclib_gen_crc32(0, buff, file_len-14);    
    store_crc = (((uint8)buffend[13])<<24)+(((uint8)buffend[12])<<16)+(((uint8)buffend[11])<<8)+((uint8)buffend[10]);
    if(crc != store_crc)
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Crc check failed.\n");
        goto ERROR;
    }
    sal_fclose(fd);
    //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
    sal_free(buff);
    return 0;
ERROR:
    sal_fclose(fd);
    return -1;
}
#endif 

#ifdef _CTC_ARM_CTC5236_
static int check_tsingma_boot_bin(char* src_file)
{
    FILE* fd;
    int ret, file_len;
    char buffend[21];
    char* buff;
    uint32 crc, store_crc;
    struct stat stat_buf;

    if(!sal_stat(src_file, &stat_buf))
    {
        file_len = stat_buf.st_size;
    }
    else
    {
        DRV_LOG_ERR("Get src file stat failed!\n");
        return -1;
    }
    
    fd = sal_fopen(src_file, "r");
    if(fd == NULL) 
    {
        DRV_LOG_ERR("Open boot src file failed!\n");
        return -1;
    }
    sal_fseek(fd, -14, SEEK_END);
    ret = sal_fread(buffend, 1, 14, fd);
    if(ret != 14) 
    {        
        DRV_LOG_ERR("read file descriptor error, ret %d\n", ret);
        goto ERROR;
    }
    if(0 != sal_strncmp(buffend, "tsingmacrc", 10))
    {
        DRV_LOG_ERR("Wrong bootrom version.\n");
        goto ERROR;
    }

    sal_fseek(fd, 0, SEEK_SET);
    /* for bug 51920, can not malloc more than 128k mem from XXX_MALLOC, instead sal_malloc  */
    //buff = (char *)DRV_MALLOC(CTCLIB_MEM_DRIVER_BOOTROM_INFO, file_len);
    buff = sal_malloc(file_len);
    if(buff == NULL)
    {
        DRV_LOG_ERR("Malloc memory to calculate crc failed!\n");
        goto ERROR;
    }
    ret = sal_fread(buff, 1, file_len-14, fd);
    if(ret != (file_len-14))
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Read file length error!ret %d length %d\n",ret, file_len-14);
        goto ERROR;
    }
    /*CRC checksum check. */    
    /*last checkinfo not in crc calculate.*/
    crc = ctclib_gen_crc32(0, buff, file_len-14);    
    store_crc = (((uint8)buffend[13])<<24)+(((uint8)buffend[12])<<16)+(((uint8)buffend[11])<<8)+((uint8)buffend[10]);
    if(crc != store_crc)
    {
        //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
        sal_free(buff);
        DRV_LOG_ERR("Crc check failed.\n");
        goto ERROR;
    }
    sal_fclose(fd);
    //DRV_FREE(CTCLIB_MEM_DRIVER_BOOTROM_INFO, buff);
    sal_free(buff);
    return 0;
ERROR:
    sal_fclose(fd);
    return -1;
}
#endif 

static int update_bootrom_in_flash(char* src_file, uint8 bootrom_type)
{
    char sys_cmd[256];
    struct stat stat_buf;
    int ret = 0;
    char spi_flash_dev_name[20];

#ifndef _CTC_LOONGSON_
#ifdef _CTC_ARM_HI3535_
    if( check_hi3535_boot_bin(src_file) != 0)
#elif (defined _CTC_ARM_LS1023A_)
    if( check_ls1023a_boot_bin(src_file) != 0)
#elif (defined _CTC_ARM_CTC5236_)
    if( check_tsingma_boot_bin(src_file) != 0)                
#else
    if( check_boot_bin(src_file, bootrom_type) != 0)
#endif
    {
        DRV_LOG_ERR("check bootrom bin failed\n");
        return -1;
    }
#endif
    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    sal_memset(spi_flash_dev_name, 0, sizeof(spi_flash_dev_name));
    if (!sal_stat(src_file, &stat_buf))
    {
        if (!S_ISREG (stat_buf.st_mode))
        {
            DRV_LOG_ERR("Bootrom image is not existed!\n");
            return -1;
        }
    }

/*fix bug40026, update pmon on line, liangf, 2016-09-07 */
#ifdef _CTC_LOONGSON_
    /*Whether bootmtd is supported or not, copy PMON to the corresponding partition of SPI flash*/
    sal_sprintf(sys_cmd, "cat /proc/mtd | grep spi_pmon |awk -F \": \" {'print $1'} > %s",GLB_SPI_FLASH_MTD_INFO_FILE);
    system(sys_cmd);
    sal_memset(sys_cmd, 0, sizeof(sys_cmd));
    FILE* fd = NULL;

    fd = sal_fopen(GLB_SPI_FLASH_MTD_INFO_FILE, "r");
    if(fd == NULL) 
    {
        DRV_LOG_ERR("Open glb_spi_flash_info_file failed!\n");
        return -1;
    }
    sal_fseek(fd, 0, SEEK_SET);
    /*strlen("mtdx") == 4*/
    ret = sal_fread(spi_flash_dev_name, 4, 1, fd);
    sal_fclose(fd);
    if(ret < 0)
    {
        DRV_LOG_ERR("read glb_spi_flash_info_file failed!\n");
        return -1;
    }
    sal_sprintf(sys_cmd, "flashcp %s /dev/%s && touch %s", src_file, spi_flash_dev_name, GLB_UPDATE_BOOTROM_DONE);
#else    
    /*For clone new progress, done file must be created after flashcp done. merge from 6.2 */
    sal_sprintf(sys_cmd, "flashcp %s /dev/mtd0 && touch %s", src_file,GLB_UPDATE_BOOTROM_DONE);
#endif

#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
    ret = system(sys_cmd);
#else
    ret = ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)sys_cmd);
#endif

    return ret;
}

int update_bootrom(char* src_file, uint8 bootrom_type)
{
    if(src_file == NULL)
    {
        DRV_LOG_ERR("Uboot bin file name is NULL!\n");
        return -1;
    }

    return update_bootrom_in_flash(src_file, bootrom_type);

}

