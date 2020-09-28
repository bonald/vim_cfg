/****************************************************************************
* $Id$
*  The header file of the mux device operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht
* Date          : 2014-12-15 
* Reason        : First Create.
****************************************************************************/
#ifndef __MUX_DRV__
#define __MUX_DRV__

#include "ctc_i2c.h"

typedef enum
{
    MUX_I2C = 0,        /* access the mux device by I2C bus */
    MUX_MAX
} mux_io_type_e;

/* define the structure including i2c operation paramaters */
struct mux_access_s{
    uint8 reg;         /* the type of the mux device register */     
    uint8* val;        /* pointer of the value buffur */
    uint8 len;
};
typedef struct mux_access_s mux_access_t;

typedef struct mux_io_handle_s mux_io_handle_t;
struct mux_io_handle_s 
{
    int32 (*close)(mux_io_handle_t *);
    int32 (*read)(const mux_io_handle_t *, mux_access_t *);
    int32 (*write)(const mux_io_handle_t *, mux_access_t *);
    void* handle;      /*handle of the low level operations */
};

mux_io_handle_t *mux_io_register(mux_io_type_e type, const void *p_data);

#endif 

