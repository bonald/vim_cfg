/****************************************************************************
* $Id$
*  The header file of the poe(Power of Ethernet) operation.
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : tongzb
* Date          : 2019-05-27
* Reason        : First Create.
****************************************************************************/
#ifndef _POE_DRV__
#define _POE_DRV__

#include "ctc_i2c.h"
#include "glb_hw_define.h"

#define POE_TASK_INTERVAL 200
#define POE_PD69200_MSG_LIST_SIZE 100

typedef enum 
{
    POE_PD69200,
    POE_MAX,
}poe_chip_t;

typedef enum
{
    POE_IO_I2C = 0, 
    POE_IO_MAX,
}poe_io_type_e;

typedef enum
{
    POE_WRITE = 0, 
    POE_READ =1, 
}poe_io_dir_e;

typedef enum
{
    POE_OP_SET_GLB_EN = 0, 
    POE_OP_SET_GLB_POWER,
    POE_OP_SET_CHN_EN,
    POE_OP_SET_CHN_FORCE,
    POE_OP_SET_CHN_MODE,
    POE_OP_SET_CHN_PRI,
    POE_OP_SET_CHN_PPL,
    POE_OP_SET_CHN_PD,

    POE_OP_SET_SYS_MASK,
    POE_OP_SET_PM_MODE,
    POE_OP_SET_SYS_STAT,

    POE_OP_GET_GLB_POWER,/* for global power status */
    POE_OP_GET_POWER_STAT,/* for all channel operation ON/OFF, kind of global data */
    POE_OP_GET_DEV_STAT,/* for channel temperature */
    POE_OP_GET_CHN_CLASS,/* for channel class */
    POE_OP_GET_CHN_PARA,/* for channel power status */
    
    POE_OP_MAX,
}poe_operate_type_e;

typedef enum poe_key_type_e
{
    POE_KEY_COMMAND = 0x0,
    POE_KEY_PROGRAM = 0x1,
    POE_KEY_REQUEST = 0x2,
    POE_KEY_TELEMETRY = 0x3,
    POE_KEY_TEST = 0x4,
    POE_KEY_REPORT = 0x52,
    POE_KEY_MAX
} poe_key_type_t;

typedef enum
{
    ECHO_POE_RESET,/* 0 */
    ECHO_POE_RESTORE,
    ECHO_POE_SAVE,
    ECHO_POE_SET_USER_DEF,
    ECHO_POE_SET_SYS_STAT,
    ECHO_POE_GET_SYS_STAT,
    ECHO_POE_GET_SYS_STAT2,
    ECHO_POE_SET_INT_MASK,
    ECHO_POE_GET_INT_MASK,
    ECHO_POE_SET_IND_MASK,
    ECHO_POE_GET_IND_MASK,/* 10 */
    ECHO_POE_SET_OK_LED,
    ECHO_POE_GET_OK_LED,
    ECHO_POE_SET_POWER_OK,
    ECHO_POE_GET_POWER_OK,
    ECHO_POE_SET_SYS_MASK,
    ECHO_POE_GET_SYS_MASK,
    ECHO_POE_GET_VERSION,    
    ECHO_POE_SET_DEVICE_TSH,
    ECHO_POE_GET_DEVICE_STAT,/* for get poe Temperature */
    ECHO_POE_SET_DEVICE_REG, /* 20 */
    ECHO_POE_GET_DEVICE_REG,
    ECHO_POE_SET_TEM_MAT,
    ECHO_POE_GET_TEM_MAT,
    ECHO_POE_SET_GLB_MAT,
    ECHO_POE_GET_GLB_MAT,

    ECHO_POE_SET_GLB_EN, /* for set poe global enable */
    ECHO_POE_SET_CHN_EN,/* for set poe  enable */
    ECHO_POE_SET_CHN_MODE,/* for set poe   mode */
    ECHO_POE_SET_CHN_PD,/* for set poe Legacy detect */
    ECHO_POE_SET_CHN_FORCE,/* for set poe   force power */ /* 30 */
    ECHO_POE_SET_CHN_PPL,/* for set poe  power limit */
    ECHO_POE_SET_CHN_PRI,/* for set poe  priority */
    ECHO_POE_SET_PM_MODE,
    ECHO_POE_GET_GLB_POWER,/* for get poe   global power status */
    ECHO_POE_SET_GLB_POWER,/* for set poe global max-power */

    ECHO_POE_GET_CHN_POWER_STAT, /* for get all channel delivering power status */
    ECHO_POE_GET_CHN_CLASS,/* for get   channel class by channel */
    ECHO_POE_GET_CHN_PARA,/* for get channel power consump,current,voltage */
    ECHO_POE_MAX = 0xff,
}poe_echo_t;

typedef enum
{
    POE_REPORT_SUCCESS,
    POE_REPORT_CMD_CHK_FAIL,/* checksum is wrong and command not executed*/
    POE_REPORT_CMD_SUB_INVALID,/* invalid subject field and command not executed*/
    POE_REPORT_CMD_DATA_INVALID,/* invalid data field and command not executed*/
    POE_REPORT_CMD_KEY_UNKNOWN,/* invalid key field and command not executed*/
    POE_TELEMETRY_CHK_PASS,
    POE_TELEMETRY_CHK_FAIL,

    POE_MAG_UNKNOWN_KEY,
    POE_MSG_MAX,
}poe_msg_status_t;

typedef union
{
    uint8 buf[15];
    struct
    {
        uint8 key;
        uint8 echo;
        uint8 sub;
        uint8 sub1;
        uint8 sub2;
        uint8 data5;
        uint8 data6;
        uint8 data7;
        uint8 data8;
        uint8 data9;
        uint8 data10;
        uint8 data11;
        uint8 data12;
        uint8 csumh;
        uint8 csuml;
    }data;
}poe_msg_t;

typedef struct poe_msg_buff_s
{
    uint8 rdwr;
    uint8 op_type;
    uint8 channel;/* channel index for msg transfer to */
    uint16 checksum;
    uint32 status;
    poe_msg_t msg;
}poe_msg_buff_t;

typedef struct poe_msg_list_s
{
    poe_msg_buff_t tx_msg_buf[POE_PD69200_MSG_LIST_SIZE];
    poe_msg_buff_t rx_msg_buf[1];
    uint32 tx_cnt;
    uint32 tx_push;
    uint32 tx_poll;
    uint32 rx_cnt;
}poe_msg_list_t;

/* define the structure including i2c operation paramaters */
struct poe_io_access_s{  
    uint8* val;  /* pointer of the value buffur */
    uint8 len;
};
typedef struct poe_io_access_s poe_io_access_t;

typedef struct poe_io_handle_s poe_io_handle_t;
struct poe_io_handle_s 
{
    int32 (*close)(poe_io_handle_t *);
    int32 (*read)(const poe_io_handle_t *, poe_io_access_t *);
    int32 (*write)(const poe_io_handle_t *, poe_io_access_t *);
    void* handle;      /*handle of the low level operations */
};

typedef struct
{
    uint16 channel_id;
    uint8 mgmt_id;/* poe manage id */
    uint32 status;
    uint8 class;
    uint32 cur_consump; /* system total real time comsumption power (milli-watt)*/
    uint32 current;
    uint32 voltage;
    int32 temper;/* temperature per port*/
}poe_channel_info_t;

typedef struct
{
    uint8 pse_id;
    uint8 power_bank_id;/* get it when init poe */
    uint32 cur_consump;   /* system total real time comsumption power (watt)*/
    poe_channel_info_t ch_info[MAX_POE_PORT_NUM];
}poe_module_info_t;

typedef struct
{
    uint8 index;/* default is 0, change it when special driver register, using in special device,like PD69200 */    
    poe_chip_t chip_type;
    poe_io_type_e io_type;
    poe_module_info_t info;
    void * p_data_i2c;
    void * p_data_epld;
    void * p_data_gpio;
}poe_info_t;

typedef struct poe_handle_s poe_handle_t;
struct poe_handle_s
{
    int32 (* init)(poe_handle_t*);   
    int32 (* read)(poe_handle_t*,poe_msg_buff_t *);
    int32 (* write)(poe_handle_t*,poe_msg_buff_t *);
    int32 (* operation)(poe_handle_t*,uint8, uint16 ,void *,int8);
    int32 (* task)(poe_handle_t*);
    poe_info_t poe_info;
    poe_io_handle_t *io_hdl;
};

poe_io_handle_t *poe_io_register(poe_io_type_e type, void *p_data);
#endif 

