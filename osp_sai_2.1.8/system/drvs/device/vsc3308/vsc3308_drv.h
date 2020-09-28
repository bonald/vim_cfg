/****************************************************************************
* $Id$
*  The header file of the vsc3308 operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-11-05 
* Reason        : First Create.
****************************************************************************/
#ifndef __VSC3308_DRV_H__
#define __VSC3308_DRV_H__

#include "ctc_i2c.h"

/* define the structure including i2c operation paramaters */
struct vsc3308_access_s
{
    uint8 reg;     /* vsc3308 register*/   
    uint8 *val;    /* pointer of the value buffur */
    uint8 len;
} ;
typedef struct vsc3308_access_s vsc3308_access_t;

typedef struct vsc3308_io_handle_s vsc3308_io_handle_t;
struct vsc3308_io_handle_s
{
    int32 (*read)(const vsc3308_io_handle_t *, vsc3308_access_t *);
    int32 (*write)(const vsc3308_io_handle_t *, vsc3308_access_t *);
    void* handle; /* handle of the low level operations*/
    void* p_data; /*private data for this handle*/
};
vsc3308_io_handle_t* vsc3308_io_register(const void *p_data);

#endif 

