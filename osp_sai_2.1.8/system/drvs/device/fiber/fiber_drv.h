#ifndef __FIBER_DRV_H__
#define __FIBER_DRV_H__

#include "sal_common.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"

#define FIBER_DEV_ADDR1   0x50
#define FIBER_DEV_ADDR2   0x51
#define FIBER_DEV_ADDR3   0x56

#define CMD_STATUS_NO_CMD       0
#define CMD_STATUS_CMD_SUCCESS  1
#define CMD_STATUS_CMD_IN_PROG  2
#define CMD_STATUS_CMD_FAIL     3

/* define the structure including fiber operation paramaters */
typedef struct fiber_para_s{
    uint32 subdev;                     /*I2C address. For XFP, memory map at 50, for SFP, memory map at 50,51 */
    uint32 offset;                   /* the address of the fiber dev register */
    uint32 len;                      /* the length of read/write */
    uint8* val;                     /* value pointer*/
}fiber_para_t;

/*support multi GB chip system*/
typedef struct fiber_ctc_gen_s{
    uint32 slave_bitmap;
    uint8 lchip;
    uint8 bus_id;
    uint8 channel;
    uint8 epld_switch_reg;
    uint8 enable_chip;
    uint8 enable_reg; /* This is for support two or more epld */
    uint8 enable_no;
}fiber_ctc_gen_t;

typedef enum
{
    //E_FIBER_I2C_EPLD = 0,           /* access the FIBER by I2C bus, EN/PRESENT by EPLD */
    //E_FIBER_I2C_VSC8658,            /* access the FIBER by I2C bus, EN/PRESENT by vsc8658 GPIO */
    //E_FIBER_PHY_QT2225,             /* access the FIBER by QT2225 phy, EN/PRESENT by QT2225 GPIO */ 
    //E_FIBER_PHY_VSC8488,            /* access the FIBER by VSC8488 phy. EN/PRESENT by vsc8488 GPIO */ 
    //E_FIBER_CTC_CHIP,               /* access the FIBER by Centec chip I2C master. EN/PRESENT/LOS by EPLD */ 
    //E_FIBER_SFP_CTC_CHIP_GPIO,      /* access the FIBER by Centec chip I2C master. EN/PRESENT/LOS by GPIO */ 
    //E_FIBER_QSFP_CTC_CHIP_GPIO,     /* access the FIBER by Centec chip I2C master. PRESENT by GPIO. EN/LOS by QSFP+ reg */
    //E_FIBER_QSFP_I2C_GPIO,          /* access the FIBER by CPU I2C master, PRESENT by GPIO. EN/LOS by QSFP+ reg */

    E_FIBER_ASIC_ASIC,       /* Fiber chip select and access by Centec chip i2c Master*/ 
    E_FIBER_EPLD_ASIC,       /* Fiber chip select by EPLD and access by Centec chip i2c Master  */
    E_FIBER_BRIDGE_CPU,      /* Fiber chip select by i2c bridge and access by CPU i2c Master  */     
} fiber_access_mode_t;

typedef enum
{
    E_FIBER_EN_GPIO,       /* Fiber enable by GPIO */ 
    E_FIBER_EN_EPLD,       /* Fiber enable by epld */
    E_FIBER_EN_MAX,
} fiber_enable_mode_t;

typedef struct fiber_cs_para1_s
{
    uint8   lchip;
	uint32  slave_bitmap;
    uint8   epld_switch_reg;/* parameter just for E_FIBER_EPLD_ASIC */
}fiber_cs_para1_t;

typedef struct fiber_cs_para2_s
{
    uint8 bridge_chan;
    uint8 bridge_addr;
	uint8 bridge_type;
}fiber_cs_para2_t;

typedef struct fiber_en_para1_s
{
    uint8 enable_chip;/* gpio index */
    uint8 enable_no;
}fiber_en_para1_t;

typedef struct fiber_en_para2_s
{
    uint8 enable_chip;/* epld index */
    uint8 enable_reg;
    uint8 enable_no;
}fiber_en_para2_t;

typedef struct fiber_port_info_s{
    int8 fiber_flg;             /* -1 mean not fiber, other get from fiber_device_t */
    fiber_access_mode_t mode;   /* fiber select and fiber access */
    fiber_enable_mode_t en_mode; /* fiber enable mode */
    uint8 bus_id;
    union
    {
        fiber_cs_para1_t asic;     /* parameter for E_FIBER_ASIC_ASIC/E_FIBER_EPLD_ASIC */
        fiber_cs_para2_t br_cpu;     /* parameter for E_FIBER_BRIDGE_CPU */
    }cs;                            /* Fiber access io parameter */
    union
    {
        fiber_en_para1_t en_gpio;  /* enable fiber by gpio */
        fiber_en_para2_t en_epld;  /* enable fiber by epld */
    }en;
    uint32 fiber_id;                  /* fiber chip index, 0 based, bitno in sfp present and sfp disable*/
    uint8 fiber_channel;
}fiber_port_info_t;
    
typedef struct fiber_handle_s fiber_handle_t;
struct fiber_handle_s
{
    int32 (*read)(fiber_handle_t* , fiber_para_t* );    
    int32 (*write)(fiber_handle_t* , fiber_para_t* );
    int32 (*fiber_present)(uint32, fiber_handle_t* , uint32* );
    int32 (*fiber_enable)(uint32, fiber_handle_t* , uint32);
    int32 (*fiber_los)(uint32, fiber_handle_t* , uint32*);
    void* phdl_dev1;     /* The hdl of i2c bus or 10G phy, access A0 */  
    void* phdl_dev2;     /* The hdl of i2c bus or 10G phy  access A2*/  
    void* phdl_dev3;     /* The hdl of i2c bus or 10G phy  access AC*/  
    void* phdl_dev4;     /* The hdl of 1G phy hdl*/ 
    fiber_running_info_t run_info;
    void* data;          /* Additional data info for fiber module */
};

fiber_handle_t* fiber_create_handle(uint32 port_id, fiber_port_info_t* p_info);

#endif /* __FIBER_DRV_H__ */

