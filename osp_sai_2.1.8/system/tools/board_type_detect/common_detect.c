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
#include <linux/version.h>
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"
#include "detect.h"
#include "version.h"
#include <sys/ioctl.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
#include <sys/types.h>
#include <linux/fs.h>
#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include "i2c_dev.h"
#endif

#define I2C_WRITE_OP    0
#define I2C_READ_OP     1
#define I2C_RDWR        0x0707  /* Combined R/W transfer (one stop only)*/

static int32 i2c_dev0_fd = -1;
static int32 i2c_dev1_fd = -1;
struct eeprom_info g_eeprom_info;
int g_platform_type;
uint8 g_product_series;           /*board series*/
uint8 g_board_type;           /*board series*/
uint8 g_hw_ver;           /* hw_ver */
detect_info_t *glb_detect_info;

    
detect_info_t detect_info_56_57[INFO_SIZE]={/* E580/E350 board with two eeprom 0x56 and 0x57 */
 /* Here for default info,  item maybe change in some cases */
{BOARD_SERIES,          {GLB_EEPROM_I2C_BUS,    0x56, 0x0001,    2}},/* bit 7 indicate CPU on daughter card or not */
{BOARD_TYPE,            {GLB_EEPROM_I2C_BUS,    0x56, 0x0002,    2}},
{BOARD_VER,             {GLB_EEPROM_I2C_BUS,    0x56, 0x0003,    2}},
{BOARD_SERIALNO,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1220,    2}},//SERIALNO info always on 0x57
{BOARD_MATERIAL,        {GLB_EEPROM_I2C_BUS,    0x56, 0x1008,    2}},
{SYSMAC_ADDR,           {GLB_EEPROM_I2C_BUS,    0x57, 0x1240,    2}},//SYSMAC_ADDR info always on 0x57
{MGMT_CFG,              {GLB_EEPROM_I2C_BUS,    0x56, 0x1100,    2}},
{SW_VER,                {GLB_EEPROM_I2C_BUS,    0x57, 0x1502,    2}},//SW_VER info always on 0x57
{BOARD_NOR_FLASH,       {GLB_EEPROM_I2C_BUS,    0x56, 0x0004,    2}},
{BOARD_NAND_FLASH,      {GLB_EEPROM_I2C_BUS,    0x56, 0x0005,    2}},
{BOARD_DDR_TYPE,        {GLB_EEPROM_I2C_BUS,    0x56, 0x0006,    2}},//no use
{SPEED_MODE,            {GLB_EEPROM_I2C_BUS,    0x56, 0x1009,    2}},
/*add by qianj for T65C and 65DV1,begin*/
 {CUSTOM_HW_VER,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1521,    2}},
 {CUSTOM_HW_LEVEL,      {GLB_EEPROM_I2C_BUS,    0x57, 0x1523,    2}},
 {CUSTOM_SW_LEVEL,      {GLB_EEPROM_I2C_BUS,    0x57, 0x1524,    2}},
/*add by qianj for T65C and 65DV1,end*/
};

detect_info_t detect_info_57[INFO_SIZE]={/* E580/E550/LS2H board only have one eeprom 0x57 */
#ifdef _CTC_LOONGSON_    
{LS2X_CPU_POS,          {GLB_EEPROM_I2C_BUS,    0x57, 0x1010,    2}},/* bit 7 indicate CPU on daughter card or not, LS2H, LS2K */
#endif
{BOARD_SERIES,          {GLB_EEPROM_I2C_BUS,    0x57, 0x1011,    2}},/* bit 7 indicate CPU on daughter card or not , E550, E580 */
{BOARD_TYPE,            {GLB_EEPROM_I2C_BUS,    0x57, 0x1012,    2}},
{BOARD_VER,             {GLB_EEPROM_I2C_BUS,    0x57, 0x1013,    2}},
{BOARD_SERIALNO,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1220,    2}},//The first byte for serialno len
{BOARD_MATERIAL,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1008,    2}},//
{SYSMAC_ADDR,           {GLB_EEPROM_I2C_BUS,    0x57, 0x1240,    2}},
{MGMT_CFG,              {GLB_EEPROM_I2C_BUS,    0x57, 0x1100,    2}},
{SW_VER,                {GLB_EEPROM_I2C_BUS,    0x57, 0x1502,    2}}, 
{BOARD_NOR_FLASH,       {GLB_EEPROM_I2C_BUS,    0x57, 0xffff,    2}},//no use
{BOARD_NAND_FLASH,      {GLB_EEPROM_I2C_BUS,    0x57, 0xffff,    2}},//no use
{BOARD_DDR_TYPE,        {GLB_EEPROM_I2C_BUS,    0x57, 0xffff,    2}},//no use
{SPEED_MODE,            {GLB_EEPROM_I2C_BUS,    0x57, 0xffff,    2}},//no use
{LOGO_TYPE,             {GLB_EEPROM_I2C_BUS,    0x57, 0x1015,    2}},//for LS2H
{ECID,                  {GLB_EEPROM_I2C_BUS,    0x57, 0x1290,    2}}, //for LS2H
{CUSTOM_HW_VER,         {GLB_EEPROM_I2C_BUS,    0x57, 0x1521,    2}},//for OEM custom-made
{CUSTOM_HW_LEVEL,       {GLB_EEPROM_I2C_BUS,    0x57, 0x1523,    2}},//for OEM custom-made
{CUSTOM_SW_LEVEL,       {GLB_EEPROM_I2C_BUS,    0x57, 0x1524,    2}},//for OEM custom-made
};

detect_info_t detect_info_57_1[INFO_SIZE]={/* E550-hi3535 series, only for e550 board with cpu-hi3535, only have 0x57 on one bus */
{BOARD_SERIES,          {GLB_EEPROM_I2C_BUS,    0x57, 0x1621,    2}},/* bit 7 indicate CPU on daughter card or not */
{BOARD_TYPE,            {GLB_EEPROM_I2C_BUS,    0x57, 0x1622,    2}},
{BOARD_VER,             {GLB_EEPROM_I2C_BUS,    0x57, 0x1623,    2}},
{BOARD_SERIALNO,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1640,    2}},//The first byte for serialno len
{BOARD_MATERIAL,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1008,    2}},
{SYSMAC_ADDR,           {GLB_EEPROM_I2C_BUS,    0x57, 0x1636,    2}},
{MGMT_CFG,              {GLB_EEPROM_I2C_BUS,    0x57, 0x1100,    2}},
{SW_VER,                {GLB_EEPROM_I2C_BUS,    0x57, 0x1502,    2}}, 
{BOARD_NOR_FLASH,       {GLB_EEPROM_I2C_BUS,    0x57, 0x1600,    2}},
{BOARD_NAND_FLASH,      {GLB_EEPROM_I2C_BUS,    0x57, 0x1601,    2}},
{BOARD_DDR_TYPE,        {GLB_EEPROM_I2C_BUS,    0x57, 0x1602,    2}},
{SPEED_MODE,            {GLB_EEPROM_I2C_BUS,    0x57, 0xffff,    2}},//no use
};

int
i2c_open_dev0(void)
{
    if (i2c_dev0_fd < 0)
    {
        i2c_dev0_fd = open("/dev/i2c-0", O_RDWR);
        if (i2c_dev0_fd < 0)
        {
            return -1;
        }
    }
    return 0;
}

int
i2c_open_dev1(void)
{
    if (i2c_dev1_fd < 0)
    {
        i2c_dev1_fd = open("/dev/i2c-1", O_RDWR);
        if (i2c_dev1_fd < 0)
        {
            return -1;
        }
    }
    return 0;
}

int 
is_i2c_opened(void)
{
    if((i2c_dev0_fd < 0)&&(i2c_dev1_fd < 0))
        return -1;
    else
        return 0;
}

int32
i2c_switch_msg(struct ctc_i2c_msg* msg, uint16 addr,
                         uint8* p_buf, uint16 flag, int32 len)
{
    if(NULL == msg)
    {
        return -1;
    }

    msg->addr = addr;
    msg->buf = p_buf;
    msg->flags = flag;
    msg->len = len;
    return 0;
}

int32
i2c_transfer(uint8 i2c_bus_idx, struct i2c_rdwr_ioctl_data* msgset)
{
    int32 ret=0;
    if (NULL == msgset)
    {
        return -1;
    }
    if ((GLB_I2C_IDX_0 == i2c_bus_idx)&&(i2c_dev0_fd>0))
    {
        ret = ioctl(i2c_dev0_fd, I2C_RDWR, msgset);    
    }
    else if ((GLB_I2C_IDX_1 == i2c_bus_idx)&&(i2c_dev1_fd>0))
    {
        ret = ioctl(i2c_dev1_fd, I2C_RDWR, msgset);    
    }
    
    return ret;
}

#ifdef _CTC_X86_64_
static int32_t i2c_write_1b(int32_t i2c_fd, uint8_t buf)
{
        int32_t ret;
        ret = i2c_smbus_write_byte(i2c_fd, buf);
        return ret;
}

static int32_t i2c_write_2b(int32_t i2c_fd, uint8_t *buf)
{
        int32_t ret;
        ret = i2c_smbus_write_byte_data(i2c_fd, buf[0], buf[1]);
        return ret;
}

#endif

/*Below cmd just used for diag*/
int32 i2c_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret;
    uint8 offset_buf[4];

#ifndef _CTC_X86_64_
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    /*fix bug20012,cr5438, change high and low address position, liangf, 2012-07-30*/
    if(alen == 1)
    {
        offset_buf[0] = (uint8)(offset&0xff);        
    }
    else if(alen == 2)
    {
        offset_buf[0] = (uint8)((offset>>8)&0xff);
        offset_buf[1] = (uint8)(offset&0xff);  
    }
    else if(alen == 3)
    { 
        offset_buf[0] = (uint8)((offset>>16)&0xff);
        offset_buf[1] = (uint8)((offset>>8)&0xff);
        offset_buf[2] = (uint8)(offset&0xff);  
    }
    else if(alen == 4)
    {
        offset_buf[0] = (uint8)((offset>>24)&0xff);  
        offset_buf[1] = (uint8)((offset>>16)&0xff);
        offset_buf[2] = (uint8)((offset>>8)&0xff);
        offset_buf[3] = (uint8)(offset&0xff);  
    }
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen);
    i2c_switch_msg(msgs+1, addr, buf, I2C_READ_OP, len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    ret = i2c_transfer(i2c_bus_idx, &msgset);    
#else
    int32 pos;
    ioctl(i2c_dev0_fd, I2C_SLAVE, addr);
    ioctl(i2c_dev0_fd, BLKFLSBUF);
    if(1 == alen)
    {
        offset_buf[0] = (uint8)(offset & 0xff);
        i2c_write_1b(i2c_dev0_fd, offset_buf[0]);
    }
    else if(2 == alen)
    {
        offset_buf[0] = (uint8)((offset >> 8) & 0xff);
        offset_buf[1] = (uint8)(offset & 0xff);
        i2c_write_2b(i2c_dev0_fd, offset_buf);
    }
    usleep(3000);
    
    for (pos = 0; pos < len; pos ++)
    {
        ioctl(i2c_dev0_fd, BLKFLSBUF);
        buf[pos] = i2c_smbus_read_byte(i2c_dev0_fd);
        usleep(3000);
    }

    ret = 0;
#endif
    return ret;
}


int32 i2c_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret, i=0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8* offset_buf;

    /*fix bug20012,cr5438, change high and low address position, liangf, 2012-07-30*/
    offset_buf = (uint8 *)malloc(sizeof(uint8)*(alen+len));
    if(alen == 1)
    {
        offset_buf[i++] = (uint8)(offset&0xff);        
    }
    else if(alen == 2)
    {
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 3)
    { 
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 4)
    {
        offset_buf[i++] = (uint8)((offset>>24)&0xff);  
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    sal_memcpy(&offset_buf[i], buf, len);
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen+len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_bus_idx, &msgset); 
    free(offset_buf);
    return ret;
}

int32 detect_i2c_read(detect_info_t *info, enum info_type type, uint8* buf,uint8 len)
{
    int i=0;
    detect_info_t item;
    for(i =0;i<INFO_SIZE;i++)
    {
        if(info[i].id == type)
        {
            item = info[i];
            break;
        }
    }
    if((i == INFO_SIZE)||(item.info.offset == 0xffff)|| (g_eeprom_info.num == 0))
    {
        for(i =0;i<len;i++)
            buf[i]=0xff;
        return -1;
    }

    return i2c_read(item.info.bus, item.info.addr, item.info.offset, item.info.alen, buf, len);
}

int32 detect_i2c_write(detect_info_t *info, enum info_type type, uint8* buf,uint8 len)
{
    int i=0;
    detect_info_t item;
    for(i =0;i<INFO_SIZE;i++)
    {
        if(info[i].id == type)
        {
            item = info[i];
            break;
        }
    }
    if((i == INFO_SIZE)||(item.info.offset == 0xffff)|| (g_eeprom_info.num == 0))
    {
        for(i =0;i<len;i++)
            buf[i]=0xff;
        return -1;
    }

    return i2c_write(item.info.bus, item.info.addr, item.info.offset, item.info.alen, buf, len);
}


uint32 get_offset_of_detect_info(detect_info_t *info, enum info_type type)
{
    int i=0;
    detect_info_t item;
    for(i =0;i<INFO_SIZE;i++)
    {
        if(info[i].id == type)
        {
            item = info[i];
            break;
        }
    }
    if((i == INFO_SIZE)||(item.info.offset == 0xffff))
    {
        return 0xffff;
    }
    return item.info.offset;
}

int
ctc_get_common_epld_info(FILE *fp)
{
    unsigned char epld_ver = 0;
    unsigned short epld_time = 0;
    unsigned int epld_date = 0;
    
#ifdef _CTC_LOONGSON_
    return 0;
#endif

#if ((defined _CTC_ARM_HI3535_)||(defined _CTC_X86_64_)||(defined _CTC_ARM_LS1023A_)||(defined _CTC_ARM_CTC5236_))
    unsigned char value;
    uint16 epld_addr = EPLD_NEW_ADDRESS;

    if(GLB_SERIES_E550 == g_product_series)
    {
        if((GLB_BOARD_E550_24T16Y == g_board_type)||(GLB_BOARD_E550_24T16X == g_board_type))
        {
            if(g_hw_ver < 0x30)
                epld_addr = EPLD_OLD_ADDRESS;
        }
        if ((GLB_BOARD_E550_24X8Y2C == g_board_type) || (GLB_BOARD_E550_48T4Y2Q == g_board_type)
            || (GLB_BOARD_E550_28M4Y2C == g_board_type) || (GLB_BOARD_E550_48M4Y2C== g_board_type))
        {
            epld_addr = EPLD_OLD_ADDRESS;
        }
    }
    /* get epld version */
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_VER, 1, &epld_ver, 1);

    /* get epld data */
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_DATA_OFFSET, 1, &value, 1);
    epld_date = value;
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_DATA_OFFSET+1, 1, &value, 1);
    epld_date <<= 8;
    epld_date |= value;
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_DATA_OFFSET+2, 1, &value, 1);
    epld_date <<= 8;
    epld_date |= value;
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_DATA_OFFSET+3, 1, &value, 1);
    epld_date <<= 8;
    epld_date |= value;

    /* get epld time */
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_TIME_OFFSET, 1, &value, 1);
    epld_time = value;
    i2c_read(GLB_EPLD_I2C_BUS, epld_addr, EPLD_TIME_OFFSET+1, 1, &value, 1);
    epld_time <<= 8;
    epld_time |= value;
#else
    unsigned char *local_bus_base;
    int mem_fd;
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0) {
        printf("open /dev/mem failed");
        return -1;
    }
    local_bus_base = (unsigned char*)mmap(NULL, 0x1000, PROT_READ|PROT_WRITE,
            MAP_SHARED, mem_fd, GB_NORMAL_EPLD_LOCAL_BUS_ADDR);/* IO for EPLD */
    if(local_bus_base < 0)
    {
        printf("Mmap epld error\n");
        close(mem_fd);
        return -1;
    }

    epld_ver = *(local_bus_base + EPLD_VER);
    epld_date = *(local_bus_base + EPLD_DATA_OFFSET);
    epld_date <<= 8;
    epld_date |= *(local_bus_base + EPLD_DATA_OFFSET+1);
    epld_date <<= 8;
    epld_date |= *(local_bus_base + EPLD_DATA_OFFSET+2);
    epld_date <<= 8;
    epld_date |= *(local_bus_base + EPLD_DATA_OFFSET+3);
    epld_time = *(local_bus_base + EPLD_TIME_OFFSET);
    epld_time <<= 8;
    epld_time |= *(local_bus_base + EPLD_TIME_OFFSET+1);
    munmap(local_bus_base, 0x1000);
#endif
    fprintf(fp, GLB_EPLD_VERSION_STRING "  %x\n", epld_ver);
    fprintf(fp, GLB_EPLD_DATE_STRING "  %x\n", epld_date);
    fprintf(fp, GLB_EPLD_TIME_STRING "  %x\n", epld_time);

    return 0;
}

int
ctc_get_platform_type()
{
    int ret = 0;
    char strCmd[256];
    int platform_type = 0;//default unsupport ONIE
    
#ifdef _CTC_LOONGSON_  //don't support ONIE
    g_platform_type = platform_type;
    return ret;
#endif    

    sal_memset(strCmd, 0, 256);
    if(g_eeprom_info.num != 0)
    {
#ifdef CNOS_OS    
        sal_sprintf(strCmd, "/bin/onie-syseeprom -g 0x21 1>/dev/null 2>&1");//check product name for judge the ONIE support
#else
        sal_sprintf(strCmd, "/usr/bin/onie-syseeprom -g 0x21 1>/dev/null 2>&1");//check product name for judge the ONIE support
#endif
        ret = system(strCmd);
        if(!ret)
        {
            platform_type = 1;
        }
    }
    g_platform_type = platform_type;
    return ret;
}

int
ctc_eeprom_detect()
{
    uint8 num=0;

/* ls2h, ls2k just have one eeprom, 0x57*/
#ifdef _CTC_LOONGSON_
    g_eeprom_info.addr[num++] = GB_EEPROM1_ADDR;
    g_eeprom_info.num = num;
#else
    int ret;
    uint8 val;

    /* There is three scenes here.
    ** only eeprom 0x56 on one i2c bus (for now, there is no such board with only 0x56 eeprom)
    ** only eeprom 0x57 on one i2c bus (0x57 eeprom always on baseboard)
    ** has two eeprom 0x56 and 0x57 on the same bus (when have two eeprom, 0x56 is on cpu board)
    */
    ret = i2c_read(GLB_EEPROM_I2C_BUS, GB_EEPROM0_ADDR, 0, 2, &val, 1);
    if(ret>=0)
    {
        g_eeprom_info.addr[num] = GB_EEPROM0_ADDR;
        num++;
    }
    ret = i2c_read(GLB_EEPROM_I2C_BUS, GB_EEPROM1_ADDR, 0, 2, &val, 1);
    if(ret>=0)
    {
        g_eeprom_info.addr[num] = GB_EEPROM1_ADDR;
        num++;
    }
    g_eeprom_info.num = num;
#endif    
    return 0;
}

int ctc_board_match(int board_series,int board_type)
{
    if((g_board_type == board_type)||(g_product_series == board_series))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int
ctc_get_common_mgt_info(FILE *fp)
{
    uint8 mgmtcfg[24];
    unsigned char mgmtuser[9];
    unsigned char mgmtpin[9];
    struct in_addr addr;
    struct in_addr addr_mgmtif;
    int ret = 0;
    int i;

#ifndef CTC_ENV_IN_SPI_FLASH
    ret = detect_i2c_read(glb_detect_info,MGMT_CFG,mgmtcfg,sizeof(mgmtcfg));
    if (ret < 0)
    {
        printf("Read mgmtcfg info failed, Please check it!\n");
    }
#else    
    /* get info from file */
    int mtd_fd = -1;
    mtd_fd = open(GLB_MTD_TMP_FILE, O_SYNC|O_RDONLY);
    if(mtd_fd < 0)
    {
       printf("open mtd info file failed\n");
       return 0;
    } 
    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,MGMT_CFG),SEEK_SET);
    sal_read(mtd_fd,mgmtcfg,sizeof(mgmtcfg));
    if(mtd_fd >= 0)
    {
        sal_close(mtd_fd);
    }        
#endif
    if (0xFF == mgmtcfg[0] && 0xFF == mgmtcfg[23]) {
        /* EEPROM for MGMT CFG uninitialized */
        memset(mgmtcfg, 0x00, sizeof(mgmtcfg));   
        /* for bug 37280 modified by liuyang 2016-2-26*/
        if (g_product_series == GLB_SERIES_P580)  
        {
            /*modified by liuyang 2016-2-18*/
            memcpy(mgmtcfg, "admin", strlen("admin"));
            memcpy(mgmtcfg + 8, "admin", strlen("admin"));
            inet_aton("192.168.0.100", &addr);
            inet_aton("0.0.0.0", &addr_mgmtif);
            memcpy(mgmtcfg + 16, &addr, sizeof(addr));
            memcpy(mgmtcfg + 20, &addr_mgmtif, sizeof(addr_mgmtif));
        }
    }

    memset(mgmtuser, 0x00, sizeof(mgmtuser));
    memset(mgmtpin, 0x00, sizeof(mgmtpin));
    for (i = 0; i < 8; i++) {
        if ('\0' == mgmtcfg[i]) {
            break;
        }

        mgmtuser[i] = mgmtcfg[i];
    }
    mgmtuser[i] = '\0';

    for (i = 0; i < 8; i++) {
        if ('\0' == mgmtcfg[i + 8]) {
            break;
        }

        mgmtpin[i] = mgmtcfg[i + 8];
    }
    mgmtpin[i] = '\0';
    memcpy(&addr, mgmtcfg + 16, sizeof(addr));
    memcpy(&addr_mgmtif, mgmtcfg + 20, sizeof(addr_mgmtif));

    fprintf(fp, "mgmtuser %s\n", mgmtuser);
    fprintf(fp, "mgmtpin \"%s\"\n", mgmtpin);
    fprintf(fp, "mgmtip %s\n", inet_ntoa(addr)); 
    fprintf(fp, "mgmtifip %s\n", inet_ntoa(addr_mgmtif));
    return ret;
}

static int ctc_get_common_sw_ver(FILE *fp)
{
    int ret=0;
    unsigned char sw_ver[MAX_SWVER_LEN+1];

#ifndef CTC_ENV_IN_SPI_FLASH
    unsigned char sw_ver_len = 0;
    /*fix bug39335, support to define sw_ver, liangf, 2016-07-06*/
    ret = detect_i2c_read(glb_detect_info,SW_VER,&sw_ver_len,1);

    if ((0 != sw_ver_len) && (0xff != sw_ver_len))
    {
        sw_ver_len = (sw_ver_len>MAX_SWVER_LEN ) ? MAX_SWVER_LEN : sw_ver_len;
        ret += detect_i2c_read(glb_detect_info,SW_VER,sw_ver,sw_ver_len+1);
        sw_ver[0] = ' ';
        //change by qianj,2019-04-04
        sw_ver[sw_ver_len + 1] = '\0';
        fprintf(fp, "%s%s\n", GLB_SW_VERSION_STRING, sw_ver);
    }
#else
    /* get info from file */
    int mtd_fd = -1;
    mtd_fd = open(GLB_MTD_TMP_FILE, O_SYNC|O_RDONLY);
    if(mtd_fd < 0)
    {
       printf("open mtd info file failed\n");
       return 0;
    } 
    lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,SW_VER),SEEK_SET);
    sal_read(mtd_fd,sw_ver,sizeof(sw_ver));
    if(mtd_fd >= 0)
    {
        sal_close(mtd_fd);
    }
    if ((0 != sw_ver[0]) && (0xff != sw_ver[0]))
    {
        sw_ver[0] = (sw_ver[0]>MAX_SWVER_LEN ) ? MAX_SWVER_LEN : sw_ver[0];
        sw_ver[sw_ver[0]+1] = '\0';
        sw_ver[0] = ' ';        
        fprintf(fp, "%s%s\n", GLB_SW_VERSION_STRING, sw_ver);
    }    
#endif    
    else
    {
        fprintf(fp, "%s %s\n", GLB_SW_VERSION_STRING, CENTEC_VERSION);
    }

    return ret;
}

static int ctc_get_common_logo_type(FILE *fp)
{
    int ret=0;

#ifdef _CTC_LOONGSON_
    uint8 logo_type = 0x1;  //no logo, default value
   
    ret = detect_i2c_read(glb_detect_info,LOGO_TYPE,&logo_type,1);
    logo_type = (logo_type>>3) & 0x1;
    fprintf(fp, GLB_LOGO_TYPE_STRING " %x\n", (logo_type==1) ? 0: 1);
#endif

    return ret;
}

static int ctc_get_common_mtdinfo(FILE *fp)
{
    int ret=0;

#ifdef _CTC_LOONGSON_
    uint8 bootmtd = -1;     /* for old ls2h board, do not support bootmtd */

    /*
    when the bit in eeprom is 0 , need return 0 ,means do not support bootmtd
    when the bit in eeprom is 1 , need return 1 ,means do support bootmtd
    change it in pmon by:    setv bootmtd enable/disable.
    */
    ret = detect_i2c_read(glb_detect_info,LS2X_CPU_POS,&bootmtd,1);
    bootmtd = bootmtd & 0x1;
    fprintf(fp, GLB_BOOTMTD_STRING " %x\n", bootmtd);
#endif

    return ret;
}

#define ECID_LEN 32
static int ctc_get_common_ecid(FILE *fp)
{
    int ret=0;

#ifdef _CTC_LOONGSON_
    int i = 0;
    uint8 ecid[ECID_LEN] = {0};

    ret = detect_i2c_read(glb_detect_info,ECID,ecid,ECID_LEN);
    /*write ecid*/
    fprintf(fp, "ecid ");
    for(i = 0; i < ECID_LEN; i++) 
    {
        fprintf(fp, "%02X", ecid[i]);
    }
    fprintf(fp, "\n");
#endif

    return ret;
}

int ctc_detect_info_init(uint8 cli_print)
{
    uint8 i=0;     
    ctc_eeprom_detect();
#ifdef _CTC_ARM_HI3535_ 
    glb_detect_info = detect_info_57_1;
#else
    glb_detect_info = detect_info_57;
#endif
    if(g_eeprom_info.num == 0)
    {
       /* for bug 48319, remove this log on e550 board */
#ifndef _CTC_ARM_LS1023A_
#ifndef _CTC_ARM_CTC5236_
       printf("There is no eeprom device( 0x56 or 0x57 ) on i2c bus, Please check it!\n");
#endif
#endif
    }
    else if(g_eeprom_info.num == 1)
    {
        if(cli_print)
        {
            printf("There is one eeprom device( 0x57 ) on i2c bus.\n");
        }
    }
    else if(g_eeprom_info.num == 2)
    {
        glb_detect_info = detect_info_56_57;
        if(cli_print)
        {
            printf("There is two eeprom device( 0x56/0x57 ) on i2c bus.\n");
        }        
    }
    /* for fix up CPU on daughter card */
#ifndef CTC_ENV_IN_SPI_FLASH    /* when env in spi flash, do not need to modify board info location */
    uint8 cpu_location=0xff;
#ifdef _CTC_LOONGSON_
    detect_i2c_read(glb_detect_info,LS2X_CPU_POS,&cpu_location,1);
#else
    detect_i2c_read(glb_detect_info,BOARD_SERIES,&cpu_location,1);
#endif
    if (cpu_location & 0x80)   /* CPU on daughter card, board info store in EEPROM1 */
    {
        for(i =0;i<INFO_SIZE;i++)
        {
            if(glb_detect_info[i].id == BOARD_SERIES)
            {
                glb_detect_info[i].info.addr = 0x57;
                glb_detect_info[i].info.offset = 0x1011;
            }
            else if(glb_detect_info[i].id == BOARD_TYPE)
            {
                glb_detect_info[i].info.addr = 0x57;
                glb_detect_info[i].info.offset = 0x1012;
            }
            else if(glb_detect_info[i].id == BOARD_VER)
            {
                glb_detect_info[i].info.addr = 0x57;
                glb_detect_info[i].info.offset = 0x1013;
            }
        }
        if(cli_print)
        {
            printf("This board CPU on daughter card, fixed up board info location.\n");
        }
    }
#endif    
    if(cli_print)
    {
        printf("Info_Name\t Bus_index\taddr\toffset\n");
#ifdef _CTC_LOONGSON_
        for(i =0;i<INFO_SIZE;i++)
        {
            if(glb_detect_info[i].id == LS2X_CPU_POS)
            {
                printf("CPU POSITION\t ");
            }
#else
        for(i =0;i<INFO_SIZE-1;i++)
        {
#endif
            if(glb_detect_info[i].id == BOARD_SERIES)
            {
                printf("BOARD_SERIES\t ");
            }
            if(glb_detect_info[i].id == BOARD_TYPE)
            {
                printf("BOARD_TYPE\t ");
            }
            if(glb_detect_info[i].id == BOARD_VER)
            {
                printf("BOARD_VER\t ");
            }
            if(glb_detect_info[i].id == BOARD_SERIALNO)
            {
                printf("BOARD_SERIALNO\t ");
            }
            if(glb_detect_info[i].id == BOARD_MATERIAL)
            {
                printf("BOARD_MATERIAL\t ");
            }
            if(glb_detect_info[i].id == SYSMAC_ADDR)
            {
                printf("SYSMAC_ADDR\t ");
            }
            if(glb_detect_info[i].id == MGMT_CFG)
            {
                printf("MGMT_CFG\t ");
            }
            if(glb_detect_info[i].id == BOARD_NOR_FLASH)
            {
                printf("BOARD_NOR_FLASH\t ");
            }
            if(glb_detect_info[i].id == BOARD_NAND_FLASH)
            {
                printf("BOARD_NAND_FLASH ");
            }
            if(glb_detect_info[i].id == BOARD_DDR_TYPE)
            {
                printf("BOARD_DDR_TYPE\t ");
            }
            if(glb_detect_info[i].id == SPEED_MODE)
            {
                printf("SPEED_MODE\t ");
            }
            if(glb_detect_info[i].id == SW_VER)
            {
                printf("SW_VER\t\t ");
            }   
            if(glb_detect_info[i].id == LOGO_TYPE)
            {
                printf("LOGO_TYPE\t ");
            }    
            if(glb_detect_info[i].id == ECID)
            {
                printf("ECID\t\t ");
            }    
            
            printf("%d\t%x\t\t%x\n",glb_detect_info[i].info.bus,glb_detect_info[i].info.addr,glb_detect_info[i].info.offset);
        }
    }
    return 0;
}

int
ctc_get_crsc_board_info(FILE *fp)
{    
    unsigned char custom_hw_ver[2];
    uint8 custom_hw_level = 0;
    uint8 custom_sw_level = 0;
    
    if (ctc_board_match(GLB_SERIES_P580,GLB_BOARD_T65C_8T64X6Q)
      ||ctc_board_match(GLB_SERIES_E350,GLB_BOARD_E350_65DV1)
      ||ctc_board_match(GLB_SERIES_P580,GLB_BOARD_P580_32X6Q)
      ||ctc_board_match(GLB_SERIES_E530,GLB_BOARD_E530_48T4X))
    {    
#ifndef CTC_ENV_IN_SPI_FLASH        
        detect_i2c_read(glb_detect_info,CUSTOM_HW_VER,custom_hw_ver,2);        
        detect_i2c_read(glb_detect_info,CUSTOM_HW_LEVEL,&custom_hw_level,1);
        detect_i2c_read(glb_detect_info,CUSTOM_SW_LEVEL,&custom_sw_level,1);
#else
        /* get info from file */
        int mtd_fd = -1;
        mtd_fd = open(GLB_MTD_TMP_FILE, O_SYNC|O_RDONLY);
        if(mtd_fd < 0)
        {
           printf("open mtd info file failed\n");
           return 0;
        } 
        lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,CUSTOM_HW_VER),SEEK_SET);
        sal_read(mtd_fd,custom_hw_ver,sizeof(custom_hw_ver));
        lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,CUSTOM_HW_LEVEL),SEEK_SET);
        sal_read(mtd_fd,&custom_hw_level,sizeof(custom_hw_level));
        lseek(mtd_fd, get_offset_of_detect_info(glb_detect_info,CUSTOM_SW_LEVEL),SEEK_SET);
        sal_read(mtd_fd,&custom_sw_level,sizeof(custom_sw_level));
        
        if(mtd_fd >= 0)
        {
            sal_close(mtd_fd);
        }
#endif
        fprintf(fp, GLB_CUSTOM_HW_VERSION_STRING " %x.%x.%x\n", custom_hw_ver[0]&0xf,
            (custom_hw_ver[1]>>4)&0xf, custom_hw_ver[1]&0xf); 
        fprintf(fp, GLB_CUSTOM_HW_LEVEL_STRING " %x\n", custom_hw_level);
        fprintf(fp, GLB_CUSTOM_SW_LEVEL_STRING " %x\n", custom_sw_level); 
    }
    return 0;
}

int ctc_get_board_info()
{
    FILE *fp;
    uint8 bm = 0xff;/* default value */
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "w+");
    if(fp == NULL)
    {
       printf("open global board file failed\n");
       return 0;
    } 

    ctc_detect_info_init(0);
    ctc_get_platform_type();
    if(g_platform_type)
    {
        ctc_get_onie_eeprom_info(fp);
        ctc_get_onie_bootrom_ver(fp);
        ctc_get_onie_bootimage(fp);
        if (ctc_board_match(GLB_SERIES_E580,GLB_BOARD_E580_48X6Q)
          ||ctc_board_match(GLB_SERIES_E580,GLB_BOARD_E580_48X2Q4Z))
        {
            detect_i2c_read(glb_detect_info,BOARD_MATERIAL,&bm,1);
            fprintf(fp, GLB_BOARD_MATERIAL_STRING "  %x\n", bm);
        }
    }
    else
    {
#ifndef CTC_ENV_IN_SPI_FLASH        
        ctc_get_normal_board_info(fp);
#else
        ctc_get_normal_sf_board_info(fp);
#endif
        ctc_get_normal_mem_info(fp);
        ctc_get_normal_bootromver(fp);
    }
    //add by qianj,2019-07-05 for crsc
    ctc_get_crsc_board_info(fp);
    
    ctc_get_common_epld_info(fp);
    ctc_get_common_mgt_info(fp);
    ctc_get_common_sw_ver(fp);
    ctc_get_common_logo_type(fp);
    ctc_get_common_mtdinfo(fp);
    ctc_get_common_ecid(fp);
    
    if(fp)
    {
        sal_fclose(fp);
    }
    
    return 0;
}

int ctc_get_emulation_board_info()
{
    FILE *fp;
    
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "w+");
    if(fp == NULL)
    {
       printf("open global board file failed\n");
       return 0;
    }

    g_product_series = GLB_SERIES_EMU;
    g_board_type     = GLB_BOARD_TM2_EMU;
    
    fprintf(fp, GLB_PRODUCT_SERIES_STRING "  %x\n", g_product_series);
    fprintf(fp, GLB_BOARD_TYPE_STRING "  %x\n", g_board_type);
    fprintf(fp, GLB_HW_VERSION_STRING "  10\n");
    fprintf(fp, GLB_ROOT_MAC " 00:1e:08:00:00:00\n");
    fprintf(fp, GLB_SYS_MAC " 00:1e:08:00:00:01\n");
    fprintf(fp, GLB_PLATFORM_TYPE  "  1\n");
    fprintf(fp, GLB_BOARD_MATERIAL_STRING "  ff\n");   
    
    fprintf(fp, GLB_SERIES_NUM "E000GD000000\n");
    
    fprintf(fp, GLB_EPLD_VERSION_STRING "  1\n");
    fprintf(fp, GLB_EPLD_DATE_STRING "  20190911\n");
    fprintf(fp, GLB_EPLD_TIME_STRING "  0900\n");
    sal_fclose(fp);
    return 0;
}


