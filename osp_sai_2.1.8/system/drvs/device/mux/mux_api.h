/****************************************************************************
* $Id$
*  The header file of the mux api.
*
 * Copyright:    (c)2014 Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : liuht
* Date          : 2014-12-15 
* Reason        : First Create.
****************************************************************************/
#ifndef __MUX_API_H__
#define __MUX_API_H__
#include "mux_drv.h"

#define SN65LVCP114_OFFSET_WIDTH 1

typedef enum
{
    MUX_PORTA = 0,
    MUX_PORTB,
    MUX_PORTC,
    MUX_PORT_MAX,
}mux_port_e;

typedef enum 
{
    MUX_SN65LVCP114,
    MUX_DEV_MAX,
}mux_chip_e;

typedef enum 
{
    MUX_CONFIG_AS_SWITCH,
    MUX_CONFIG_AS_BUFF,
    MUX_CONFIG_MAX,
}mux_cfg_e;

typedef struct 
{
    mux_chip_e chip_type;  /* mux device chip type*/
    mux_io_type_e io_type; /* mux device access type*/
    mux_cfg_e cfg_type;    /* mux device config type */
    uint8 port_num;
    uint8 lane_num;
}mux_chip_t;

typedef struct
{
    uint8 channel;
    mux_port_e port;
    uint8 lane;
    uint8 out_inverse;
    int32 gain;
    int32 eq;

}mux_info_t;

typedef struct
{
    mux_port_e port;
    uint8 lane;
    uint8 out_inverse;
}mux_channel_t;

typedef struct
{
    uint8 rd_fail;
    uint8 wr_fail;
    uint8 cmp_fail;
}mux_diag_result_t;

typedef struct mux_handle_s mux_handle_t;
struct mux_handle_s
{
    int32 (* mux_init)(mux_handle_t* );
    int32 (* reg_read)(mux_handle_t*, uint8 , uint8* , uint8 );
    int32 (* reg_write)(mux_handle_t*, uint8 , uint8* , uint8 );
    int32 (* lane_select)(mux_handle_t*, uint8, uint8);
    int32 (* channel_select)(mux_handle_t*, uint8);
    int32 (* output_inverse)(mux_handle_t*, uint8, uint8, uint8);
    int32 (* gain)(mux_handle_t*, uint8, uint8, uint8);
    int32 (* eq)(mux_handle_t*, uint8, uint8, uint8);
    uint8 access_err_cnt;
    mux_io_handle_t *io_hdl;
    void* info;           /* Additional data info for mux */
};

int32 mux_reg_read(uint32 chip_id, uint8 reg, uint8* value, uint8 len);
int32 mux_reg_write(uint32 chip_id, uint8 reg, uint8* value, uint8 len);
int32 mux_lane_select(uint32 chip_id, uint8 lane, uint8 port);
int32 mux_channel_select(uint32 chip_id, uint8 channel);
int32 mux_init(void** p_data, mux_chip_t* type, mux_info_t** mux_info, uint8 chip_num);
int32 mux_dev_init(uint32 mux_id);

#endif 

