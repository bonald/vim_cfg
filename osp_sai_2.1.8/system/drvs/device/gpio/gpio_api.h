/****************************************************************************
* $Id$
*  The header file of the gpio api.
*
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht
* Date          : 2014-12-06 
* Reason        : First Create.
****************************************************************************/
#ifndef __GPIO_API_H__
#define __GPIO_API_H__
#include "gpio_drv.h"

/* Common Definition */
#define PCA95X5_OFFSET_WIDTH 1
#define PCA95X5_VALUE_LEN 1
#define PCA95X5_PORTS_PER_BANK 8

/* Specical Definition */
#define PCA9505_OFFSET_WIDTH 1
#define PCA9505_VALUE_LEN 1
#define PCA9505_PORTS_PER_BANK 8
#define PCA9505_BANK_NUM 5

#define PCA9505_INPUT_PORT_REG_BANK0 0x0
#define PCA9505_INPUT_PORT_REG_BANK1 0x1
#define PCA9505_INPUT_PORT_REG_BANK2 0x2
#define PCA9505_INPUT_PORT_REG_BANK3 0x3
#define PCA9505_INPUT_PORT_REG_BANK4 0x4

#define PCA9505_OUTPUT_PORT_REG_BANK0 0x8
#define PCA9505_OUTPUT_PORT_REG_BANK1 0x9
#define PCA9505_OUTPUT_PORT_REG_BANK2 0xa
#define PCA9505_OUTPUT_PORT_REG_BANK3 0xb
#define PCA9505_OUTPUT_PORT_REG_BANK4 0xc

#define PCA9505_POLARITY_INVE_REG_BANK0 0x10
#define PCA9505_POLARITY_INVE_REG_BANK1 0x11
#define PCA9505_POLARITY_INVE_REG_BANK2 0x12
#define PCA9505_POLARITY_INVE_REG_BANK3 0x13
#define PCA9505_POLARITY_INVE_REG_BANK4 0x14

#define PCA9505_DIR_CTRL_REG_BANK0 0x18
#define PCA9505_DIR_CTRL_REG_BANK1 0x19
#define PCA9505_DIR_CTRL_REG_BANK2 0x1a
#define PCA9505_DIR_CTRL_REG_BANK3 0x1b
#define PCA9505_DIR_CTRL_REG_BANK4 0x1c

#define PCA9505_INT_MASK_REG_BANK0 0x20
#define PCA9505_INT_MASK_REG_BANK1 0x21
#define PCA9505_INT_MASK_REG_BANK2 0x22
#define PCA9505_INT_MASK_REG_BANK3 0x23
#define PCA9505_INT_MASK_REG_BANK4 0x24

#define PCA9535_BANK_NUM 2

#define PCA9535_INPUT_PORT_REG_BANK0 0x0
#define PCA9535_INPUT_PORT_REG_BANK1 0x1

#define PCA9535_OUTPUT_PORT_REG_BANK0 0x2
#define PCA9535_OUTPUT_PORT_REG_BANK1 0x3

#define PCA9535_POLARITY_INVE_REG_BANK0 0x4
#define PCA9535_POLARITY_INVE_REG_BANK1 0x5

#define PCA9535_DIR_CTRL_REG_BANK0 0x6
#define PCA9535_DIR_CTRL_REG_BANK1 0x7

typedef enum 
{
    GPIO_PCA9505,
    GPIO_PCA9506,
    GPIO_PCA9535,
    GPIO_AT9555,
    GPIO_MAX,
}gpio_chip_e;

typedef struct 
{
    gpio_chip_e chip_type;  /* gpio device chip type*/
    gpio_io_type_e io_type; /* gpio device access type*/
    uint8 scan_group_bitmap; /*pca9505 has 5 group, this note which group need be scan.*/
}gpio_chip_t;

typedef struct 
{
    uint8 rd_fail;
    uint8 wr_fail;
    uint8 cmp_fail;
}gpio_diag_result_t;

typedef struct gpio_handle_s gpio_handle_t;
struct gpio_handle_s
{
    int32 (* reg_read)(gpio_handle_t*, uint8 , uint8* , uint8 );
    int32 (* reg_write)(gpio_handle_t*, uint8 , uint8* , uint8 );
    gpio_chip_e chip_type;  /* gpio device chip type*/
    uint8 scan_group_bitmap; /*pca9505 has 5 group, every group has 8 bits.*/
    uint8 scan_value[PCA9505_BANK_NUM]; /*if scan gpio chip periodly, save the real-time scan value.*/
    gpio_io_handle_t *io_hdl;
};

int32 gpio_reg_read(uint32 chip_id, uint8 reg, uint8* value, uint8 len);
int32 gpio_reg_write(uint32 chip_id, uint8 reg, uint8* value, uint8 len);
int32 gpio_init(void** p_data, gpio_chip_t* type, uint32 num);
int32 gpio_get_scan_special_bit_value(uint8 chip, uint8 no, uint8 *value);
void gpio_scan_real_time_value();
int32 gpio_set_special_bit(uint8 chip, uint8 no, uint8 value);
void gpio_set_scan_group(uint8 chip,  uint8 value);
int32 gpio_get_diag_bank(uint8 chip_id);
#endif 

