/****************************************************************************
* $Id$
*  The header file of the vsc3308 api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-11-05 
* Reason        : First Create.
****************************************************************************/
#ifndef __VSC3308_API_H__
#define __VSC3308_API_H__
#include "vsc3308_drv.h"

#define VSC3308_OFFSET_WIDTH 1

typedef enum
{
    VSC3308_USED_AS_SWITCH,    /* VSC3308 used as a switch, e.g. on motherboard */
    VSC3308_USED_AS_BUFFER,    /* VSC3308 used as a buffer, e.g. on slot card */
}vsc3308_chip_e;

typedef struct
{
    vsc3308_chip_e chip_used_type; /*vsc3308 chip type*/
}vsc3308_chip_t;

typedef struct vsc3308_handle_s vsc3308_handle_t;
struct vsc3308_handle_s
{
    int32 (* vsc3308_init)(vsc3308_handle_t* );
    int32 (* get_status)(vsc3308_handle_t* , uint8*);
    int32 (* reg_read)(vsc3308_handle_t*, uint8 , uint8* , uint8 );
    int32 (* reg_write)(vsc3308_handle_t*, uint8 , uint8 , uint8 );
    vsc3308_io_handle_t *io_hdl;
};

int32 vsc3308_dev_init(uint8 vsc3308_idx);
int32 vsc3308_init(void **p_data, vsc3308_chip_t* chip_used_type, uint8 chip_num);
int32 vsc3308_read(uint8 chip_id, uint8 reg, uint8* value);
int32 vsc3308_write(uint8 chip_id, uint8 reg, uint8 value);
int32 vsc3308_config_input_output_pin(uint8 chip_id, uint8 input_pin, uint8 output_pin);

#endif 

