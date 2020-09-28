/****************************************************************************
* $Id$
*  The header file of the clock operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : shil
* Date          : 2018-03-02
* Reason        : First Create.
****************************************************************************/
#ifndef __CLOCK_DRV__
#define __CLOCK_DRV__

#include "ctc_spi.h"
#include "ctc_i2c.h"

typedef enum
{
    E_CLOCK_SPI = 0,        /* access the clock by SPI bus */
    E_CLOCK_I2C,
} clock_io_type_t;

typedef enum 
{
    CLOCK_AD9559,
    CLOCK_CDCM6208,
    CLOCK_DS3104,
    CLOCK_IDT8T49N287,
    CLOCK_MAX,
} clock_chip_t;

struct clock_io_access_s{
    uint16 addr;             /* clock register address */ 
    uint32  *val;             /* pointer of the value buffur */
    uint32 len;             /* val length */
} ;
typedef struct clock_io_access_s clock_io_access_t;

typedef struct
{
    clock_chip_t clock_chip_type;
    clock_io_type_t clock_io_type;
    void *clock_gen;
} clock_info_t;

typedef struct clock_io_handle_s clock_io_handle_t;
struct clock_io_handle_s
{
    uint32 index;
    int32 (*close)(clock_io_handle_t *);
    int32 (*read)(const clock_io_handle_t *, clock_io_access_t *);
    int32 (*write)(const clock_io_handle_t *, clock_io_access_t *);
    void *handle;  /*handle of the low level operations */
};

typedef struct clock_handle_s clock_handle_t;
struct clock_handle_s
{
    int32 (* clock_init)(const clock_handle_t *);
    int32 (* reg_read)(const clock_handle_t *, uint16, uint32 *);
    int32 (* reg_write)(const clock_handle_t *, uint16, uint32 *);
    int32 (* clock_get_status)(const clock_handle_t *, uint32 *);
    int32 (* clock_close)(clock_handle_t *);
    int32 (* reg_read_indirect)(const clock_handle_t *, uint16, uint32 *);
    int32 (* reg_write_indirect)(const clock_handle_t *, uint16, uint32 *);
    int32 (* intr_fix_up)(const clock_handle_t *);
    clock_info_t clock_info;
    clock_io_handle_t *iohdl;
};

clock_io_handle_t *clock_io_register(clock_io_type_t type, const void *pgen);

#endif 

