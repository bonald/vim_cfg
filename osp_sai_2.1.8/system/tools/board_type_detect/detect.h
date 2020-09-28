#ifndef __DETECT_H__
#define __DETECT_H__

/*
 * I2C Message - used for trpure i2c ansaction, also from /dev interface
 */
struct ctc_i2c_msg {
    uint16 addr;
    uint16 flags;
    uint16 len;
    uint8 *buf;
};

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
    struct ctc_i2c_msg *msgs;   /* pointers to i2c_msgs */
    int32 nmsgs;                /* number of i2c_msgs */
};

struct eeprom_info {
    uint8 num;
    uint8 addr[2];
};

#if (defined _CTC_ARM_LS1023A_)
#define GLB_EPLD_I2C_BUS GLB_I2C_IDX_1
#define GLB_EEPROM_I2C_BUS GLB_I2C_IDX_1
#elif (defined _CTC_ARM_CTC5236_)
#define GLB_EPLD_I2C_BUS GLB_I2C_IDX_0
#define GLB_EEPROM_I2C_BUS GLB_I2C_IDX_0
#elif (defined _CTC_LOONGSON_)
#define GLB_EEPROM_I2C_BUS GLB_I2C_IDX_1
#else
#define GLB_EPLD_I2C_BUS GLB_I2C_IDX_0
#define GLB_EEPROM_I2C_BUS GLB_I2C_IDX_0
#endif

enum info_type{
#ifdef _CTC_LOONGSON_
    LS2X_CPU_POS,
#endif    
    BOARD_SERIES = 1,   /* bit 7 indicate CPU on daughter card or not, for E550, E580 */
    BOARD_TYPE,
    BOARD_VER,
    BOARD_SERIALNO,
    BOARD_MATERIAL,    
    SYSMAC_ADDR,
    MGMT_CFG,
    SW_VER,    
    BOARD_NOR_FLASH,
    BOARD_NAND_FLASH,
    BOARD_DDR_TYPE,
    SPEED_MODE,
    LOGO_TYPE, //LS2H 
    ECID,  //LS2H license control
    /*add by qianj for T65C and 65DV1,begin*/
    CUSTOM_HW_VER, 
    CUSTOM_HW_LEVEL,
    CUSTOM_SW_LEVEL,
    /*add by qianj for T65C and 65DV1,end*/
    INFO_MAX
};

#define INFO_SIZE   INFO_MAX
typedef struct info_access_s{
    uint8 bus;      /* i2c bus no */
    uint16 addr;    /* i2c address */
    uint32 offset;  /* reg offset */
    uint8 alen;     /* address len */
} info_access_t;

typedef struct detect_info_s{
    enum info_type id;
    info_access_t info;
}detect_info_t;

/* Flash/Dram can has at most 16 kinds of types */
#define CTC_MEMORY_SIZE_32   32
#define CTC_MEMORY_SIZE_64   64
#define CTC_MEMORY_SIZE_128  128
#define CTC_MEMORY_SIZE_256  256
#define CTC_MEMORY_SIZE_512  512
#define CTC_MEMORY_SIZE_1G   1024
#define CTC_MEMORY_SIZE_2G   2048
#define CTC_MEMORY_SIZE_4G   4096
#define CTC_MEMORY_SIZE_8G   8192
#define CTC_MEMORY_RESERVE   0
#define CTC_MEMORY_MAX_TYPES 16

#define GB_EEPROM0_ADDR                       0x56
#define GB_EEPROM1_ADDR                       0x57

#define GLB_MTD_TMP_FILE   "/tmp/ctc_mtd_info"
#define GLB_MEM_TMP_FILE   "/tmp/ctc_memory_info"

#if 0
#define GB_EEPROM0_ADDR                       0x56
#define GB_EEPROM0_TEST_OFFSET                 0x0
#define GB_EEPROM0_BOARD_SERIES_OFFSET         0x1  /* bit 7 indicate CPU on daughter card or not */
#define GB_EEPROM0_BOARD_TYPE_OFFSET           0x2
#define GB_EEPROM0_BOARD_VER_OFFSET            0x3
#define GB_EEPROM0_BOARD_NOR_FLASH_OFFSET      0x4
#define GB_EEPROM0_BOARD_NAND_FLASH_OFFSET     0x5
#define GB_EEPROM0_BOARD_DDR_TYPE_OFFSET       0x6
#define GB_EEPROM0_MGMT_CFG_OFFSET          0x1100
#define GB_EEPROM0_BOARD_MATERIAL_OFFSET    0x1008
#define GB_EEPROM0_SPEED_MODE_OFFSET        0x1009

#define GB_EEPROM1_ADDR                       0x57
#ifndef _CTC_ARM_LS1023A_
#define GB_EEPROM1_BOARD_SERIES_OFFSET      0x1011
#define GB_EEPROM1_BOARD_TYPE_OFFSET        0x1012
#define GB_EEPROM1_BOARD_VER_OFFSET         0x1013
#define GB_EEPROM1_SYSMAC_ADDR              0x1240
#define GB_EEPROM1_MGMT_CFG_OFFSET          0x1248
#define GB_EEPROM1_BOARD_SERIALNO           0x1220
#define GB_EEPROM1_SW_VER_OFFSET            0x1502
#else
#define GB_EEPROM1_BOARD_SERIES_OFFSET      0x1621
#define GB_EEPROM1_BOARD_TYPE_OFFSET        0x1622
#define GB_EEPROM1_BOARD_VER_OFFSET         0x1013 ///TODO:
#define GB_EEPROM1_SYSMAC_ADDR              0x1636
#define GB_EEPROM1_MGMT_CFG_OFFSET          0x1100
#define GB_EEPROM1_BOARD_SERIALNO           0x1640
#define GB_EEPROM1_SW_VER_OFFSET            0x1502///TODO:
#define GB_EEPROM1_BOARD_MATERIAL_OFFSET    0x1008
#endif
#endif
#define GB_NORMAL_EPLD_LOCAL_BUS_ADDR      0xffb00000

#define EPLD_OLD_ADDRESS                        0x58
#define EPLD_NEW_ADDRESS                        0x36

#define EPLD_VER                            0x1
#define EPLD_DATA_OFFSET                    0xf0
#define EPLD_TIME_OFFSET                    0xf4

int ctc_get_board_info();
int ctc_get_emulation_board_info();
int ctc_get_common_mgt_info(FILE *fp);
int ctc_get_common_epld_info(FILE *fp);
int32 i2c_read(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 detect_i2c_read(detect_info_t *info, enum info_type type, uint8* buf,uint8 len);
int32 i2c_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);
int32 detect_i2c_write(detect_info_t *info, enum info_type type, uint8* buf,uint8 len);

uint32 get_offset_of_detect_info(detect_info_t *info, enum info_type type);
int i2c_open_dev0(void);
int i2c_open_dev1(void);
int is_i2c_opened(void);

int ctc_get_platform_type();
int ctc_detect_info_init(uint8 cli_print);

int ctc_get_onie_eeprom_info(FILE *fp);
int ctc_get_onie_bootrom_ver(FILE *fp);
int ctc_get_onie_bootimage(FILE *fp);
int ctc_get_normal_board_info(FILE *fp);
int ctc_get_normal_sf_board_info(FILE * fp);
int ctc_get_normal_bootromver(FILE *fp);
int ctc_get_normal_mem_info(FILE *fp);

extern struct eeprom_info g_eeprom_info;
extern int g_platform_type;
extern uint8 g_product_series;
extern uint8 g_board_type;
extern uint8 g_hw_ver;
extern detect_info_t *glb_detect_info;
#endif /* !__RMT_H__ */
