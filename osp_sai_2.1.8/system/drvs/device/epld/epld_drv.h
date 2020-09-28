/****************************************************************************
* $Id$
*  The header file of the epld operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Jian Zhu
* Date          : 2008-08-30 
* Reason        : First Create.
****************************************************************************/
#ifndef __EPLD_DRV__
#define __EPLD_DRV__

#include "ctc_spi.h"
#include "ctc_i2c.h"

#define EPLD_REG_DESC_MAXNUM    256 /*replace 64 with 128 for e810*/

#define EPLD_DATA_LENTH 1    /* data length (byte)*/  
#define EPLD_ADDRESS_LENTH 1  /* address length (byte)*/ 

/*from hardware team*/
#define EPLD_SPI_READ  0x1
#define EPLD_SPI_WRITE 0x0

/*from seoul G24EU epld spec*/
#define EPLD_ACCESS_MODE 0x8
#define EPLD_SPI_SWITCH 0x9

typedef struct epld_reg_s
{
    uint8 reg_offset;                         /* reg addr*/
    int32 start_bit;                          /* the start bit */
    int32 end_bit;                            /* the end bit */
    int32 item_bitwidth;                      /* how many bits desc one items */
    int32 reg_valid;
} epld_reg_t;

typedef enum epld_bus_type_e
{
    EPLD_PCI_TYPE = 0, 
    EPLD_LOCALBUS_TYPE,
    EPLD_SPI_TYPE,
    EPLD_I2C_TYPE,
} epld_bus_type_t;

typedef struct epld_info_s
{
    uintptr_t base_addr;
    epld_bus_type_t epld_bus_type;
    epld_reg_t reg_desc[EPLD_REG_DESC_MAXNUM];
    /*add by jcao for seoul G24EU board*/
    spi_gen_t spi_gen; /* spi bus infomation*/
    i2c_gen_t i2c_gen; /*i2c busormation*/
    uint8 switch_value;
    uint16 diag_level;
    uint16 req_diag_level;
} epld_info_t;

typedef struct epld_access_s{
    uint32 addr;      /* the address in epld to be access */
    uint32 val;
} epld_access_t;

typedef struct epld_handle_s epld_handle_t;
struct epld_handle_s
{
    int32 (*item_read)(const epld_handle_t *phdl, uint32 op_reg, int32 *value);
    int32 (*item_write)(const epld_handle_t *phdl, uint32 op_reg, int32 value);
    int32 (*read)(const epld_handle_t *, epld_access_t *);
    int32 (*write)(const epld_handle_t *, epld_access_t *);
    int32 (*get_item_width)(const epld_handle_t *phdl, uint32 op_reg);
    void *info;
    /*add by jcao for seoul G24EU board*/
    void *handle;  /*gpio spi handle */
};


#define EPLD_PRINT(level, fmt, args...)                      \
{                                                              \
    FILE * fp_console = NULL;                                  \
    fp_console = fopen("/dev/console", "w+");                  \
    sal_fprintf(fp_console, "[%s] "fmt"\n", #level, ##args);   \
    fclose(fp_console);                                        \
}


epld_handle_t * epld_dev_register(void *info);
#endif /* !__EPLD_DRV__ */

