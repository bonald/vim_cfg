/****************************************************************************
* $Id$
*  The header file of the power api.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-10-11 
* Reason        : First Create.
****************************************************************************/
#ifndef __POWER_API_H__
#define __POWER_API_H__
#include "glb_hw_define.h"
#include "power_drv.h"
#include "power_api.h"
/* added by liuht for bug 24525,2013-10-24 */
#define PSU_I2C_OFFSET_WIDTH 1
#define PSU_ID_TABLE_LEN_MAX 4
#define PSU_ID_LEN_MAX 32
#define DIAG_MAX_PSU_NUM 2

typedef struct
{
    uint8 id_table[PSU_ID_LEN_MAX];
    uint8 psu_mode;/* 1 mean AC, 0 mean DC*/
}psu_id_info_t;

typedef struct
{
    uint8 dev_addr;
    uint8 id_reg; 
    uint8 id_len;
    psu_id_info_t* id_info[PSU_ID_TABLE_LEN_MAX];
    uint8 pmbus_addr[PSU_ID_TABLE_LEN_MAX];
}psu_mode_info_t;

typedef enum
{
    PSU_MFR_GREATWALL_AC = 0, //0x50
    PSU_MFR_3Y_DC,            //0x50 this is customized for accton     
    PSU_MFR_ETASIS_AC,        //0x5b
    PSU_MFR_CPR_AC,           //0x38
    PSU_MFR_CPR_AC1,           //0x38
    PSU_MFR_UM_DC,            //0x50
    PSU_MFR_3Y_DC_STANDARD,   //this is 3Y standard DC 
    PSU_MFR_MODE_UNKNOWN
}psu_mfr_e;

/* PSU_MODE_TYPE_REG_devaddr_idreg_idlen */
typedef enum
{
    PSU_MODE_TYPE_REG_50_01_04 = 0,
    PSU_MODE_TYPE_REG_51_01_04,
    PSU_MODE_TYPE_REG_5b_9a_0a,
    PSU_MODE_TYPE_REG_38_26_0d,
    PSU_MODE_TYPE_REG_50_50_08,
    PSU_MODE_TYPE_REG_50_20_08,
    PSU_MODE_TYPE_REG_51_20_08,
    PSU_MODE_TYPE_REG_MAX,
}psu_mode_type_reg_e;

typedef enum
{
    PSU_EPLD,    /*PSU manage by epld.*/
    /* added by liuht for bug 24525,2013-10-24 */	
    PSU_I2C_EPLD,      /* PSU manage by epld, type get by i2c.*/
    PSU_I2C_GPIO,        /*PSU manage by gpio, type get by i2c.*/
    PSU_GPIO,        /*PSU manage by gpio, type get by gpio.*/
}psu_chip_e;

/* added by liuht for bug 24525,2013-10-24 */	
typedef struct
{
    void *p_data_epld;
    void *p_data_i2c;
    void *p_data_gpio;
}psu_private_t;

typedef struct
{
    psu_chip_e chip_type; /* psu chip type*/
    psu_io_type_e io_type; /*psu access type*/
    psu_type_e psu_type; /* psu cli show type */
    psu_mode_type_reg_e psu_mode_type;
    uint8 support_pmbus;
    uint8 psu_num; /*chip manage psu num*/ 
    uint8 fixed_status;  /* add by chenjr for bug 53516*/
}psu_chip_t;

typedef struct
{
    uint8 psu_type;
    uint8 psu_mode;
    uint8 psu_ok_cnt;
    uint32 psu_mode_type;
    uint8 support_pmbus;
    uint8 fixed_status;   /* add by chenjr for bug 53516*/
}psu_private_data_t;

typedef struct
{
    uint8 access_fail;
    uint8 absent;	
    uint8 status_fail;
    uint8 alert;
}psu_diag_result_t;

typedef struct psu_handle_s psu_handle_t;
struct psu_handle_s
{
    int32 (*psu_init)(psu_handle_t*);
    int32 (*set_shutdown)(psu_handle_t*, uint8, uint8);
    /* added by liuht for bug 24525,2013-10-24 */		
    int32 (* reg_read)(psu_handle_t*, uint8 , uint8* , uint8 );	
    int32 (* reg_write)(psu_handle_t*, uint8 , uint8* , uint8 );
    int32 (* attr_mod)(psu_handle_t*, uint8 );
    psu_private_data_t* p_data;
    int32 (*get_status)(psu_handle_t*, uint8, psu_status_t*);
    psu_io_handle_t* io_hdl;
};

/* modified by liuht for bug 24525,2013-10-24 */
#if 0
int32 psu_init(void** p_data, psu_chip_t* type, uint8 power_num);
#endif
int32 psu_init(psu_private_t* p_data, psu_chip_t* type, uint8 power_num);
int32 psu_set_shutdown(uint8 psu_idx, uint8 shutdown);
int32 psu_get_status(uint8 psu_idx, psu_status_t * p_psu_status);
int32 psu_diagnostic_test(uint8 *psu_num, psu_diag_result_t* diag_result);
int32 psu_reg_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len);
int32 psu_reg_write(uint32 psu_id, uint8 reg, uint8 value, uint8 len);
int32 psu_diagnostic_test_and_report(uint8 psu_num);
int32 psu_pmbus_get_status(uint32 psu_id, psu_status_t * p_psu_status);
int32 psu_pmbus_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len);
int32 psu_normal_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len);
#endif 

