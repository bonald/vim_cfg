/****************************************************************************
* $Id$
*  The header file of the vcm(voltage and current monitor) operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb
* Date          : 2017-07-24
* Reason        : First Create.
****************************************************************************/
#ifndef _VCM_DRV__
#define _VCM_DRV__

#include "ctc_i2c.h"

#define VCM_LTC2991_OFFSET_WIDTH 1
#define VCM_BMR_464_OFFSET_WIDTH    1
#define VCM_MAX20743_OFFSET_WIDTH    1

typedef enum
{
    VCM_I2C = 0, 
    VCM_EPLD,
}vcm_io_type_e;

typedef enum 
{
    VCM_LTC2991_CMS,
    VCM_LTC2991_LMS,
    VCM_LTC2991_IMS,
    VCM_BMR464,
    VCM_MAX20743,
    VCM_MAX,
}vcm_chip_t;

/* define the structure including i2c operation paramaters */
struct vcm_io_access_s{
    uint8 reg;         /* the type of the temperature ltc2991 register */     
    uint8* val;              /* pointer of the value buffur */
    uint8 len;
};
typedef struct vcm_io_access_s vcm_io_access_t;

typedef struct vcm_io_handle_s vcm_io_handle_t;
struct vcm_io_handle_s 
{
    int32 (*close)(vcm_io_handle_t *);
    int32 (*read)(const vcm_io_handle_t *, vcm_io_access_t *);
    int32 (*write)(const vcm_io_handle_t *, vcm_io_access_t *);
    void* handle;      /*handle of the low level operations */
};

typedef struct
{
    vcm_chip_t vcm_chip_type;
    vcm_io_type_e vcm_io_type;
    float32 vcm_rsense; /* for calculate the current by voltage, need by ltc2991 */
    void * vcm_gen;
}vcm_info_t;

typedef enum vcm_access_type_e
{
    VOUT,
    VIN,
    COUT,
    TMPR,
} vcm_access_type_e_t;

typedef struct
{
    vcm_access_type_e_t type;
    
    /* channel no for ltc2991 */
    /* channel_no   type            
            0       LTC2991_SINGLE_V1_VOLT,
            1       LTC2991_SINGLE_V2_VOLT,
            2       LTC2991_SINGLE_V3_VOLT,
            3       LTC2991_SINGLE_V4_VOLT,
            4       LTC2991_SINGLE_V5_VOLT,
            5       LTC2991_SINGLE_V6_VOLT,
            6       LTC2991_SINGLE_V7_VOLT,
            7       LTC2991_SINGLE_V8_VOLT,        
            8       LTC2991_VCC_VOLT,
    */
    uint8 channel_no;
}vcm_access_t;

typedef struct vcm_handle_s vcm_handle_t;
struct vcm_handle_s
{
    int32 (* vcm_init)(vcm_handle_t*);
    int32 (* get_temp)(vcm_handle_t* ,vcm_access_t, float32*);
    int32 (* get_volt)(vcm_handle_t*, vcm_access_t, float32* );
    int32 (* get_cur)(vcm_handle_t*, vcm_access_t, float32* );    
    int32 (* reg_read)(vcm_handle_t*, uint8, uint8* );
    int32 (* reg_write)(vcm_handle_t*, uint8, uint8* );
    int32 (* trigger)(vcm_handle_t*, vcm_access_t);
    uint8 access_err_cnt;
    vcm_info_t vcm_info;
    vcm_io_handle_t *io_hdl;
};

vcm_io_handle_t *vcm_io_register(vcm_io_type_e type, void *p_data);
#endif 

