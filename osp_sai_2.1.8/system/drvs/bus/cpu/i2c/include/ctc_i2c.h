#ifndef __CTC_I2C_H__
#define __CTC_I2C_H__

#include "sal_common.h"


#define I2C_WRITE_OP    0
#define I2C_READ_OP     1

/* ----- commands for the ioctl like i2c_command call:
 * note that additional calls are defined in the algorithm and hw 
 *      dependent layers - these can be listed here, or see the 
 *      corresponding header files.
 */

#define I2C_RDWR        0x0707  /* Combined R/W transfer (one stop only)*/

/*
 * I2C Message - used for trpure i2c ansaction, also from /dev interface
 */ 
struct ctc_i2c_msg {
    uint16 addr;     
    uint16 flags;           
    uint16 len;              
    uint8 *buf;              
};

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
    struct ctc_i2c_msg *msgs;   /* pointers to i2c_msgs */
    int32 nmsgs;              /* number of i2c_msgs */
};

typedef enum
{
    E_I2C_GPIO = 0,        /* the I2C interface is implemented by two GPIO pins */
    E_I2C_CPM,              /* the I2C interface is implemented by I2C controller in CPU cpm */
    E_I2C_CPM_RAW,       /* the I2C interface is implemented by I2C controller in CPU cpm for support raw transfer for PoE device*/
    E_I2C_GB,              /* the I2C interface is implemented by I2C controller in Greatbelt */
    E_I2C_SMBUS,              /* the I2C interface is implemented by CPU SMBus */
    E_I2C_SUSI,         /* the I2C driver implemented by Advantech SUSI */
    E_I2C_GG,              /* the I2C interface is implemented by I2C controller in GG */
} i2c_type_e;

typedef enum
{
    I2C_BR_GPIO = 0,        /* the I2C interface is implemented by two GPIO pins */
    I2C_BR_CPM,             /* the I2C interface is implemented by I2C controller in CPU cpm */
    I2C_BR_FPGA,             /* the brige is realized in FPAG */
    I2C_BR_EPLD,             /* the brige is realized in EPLD and EPLD just a switch, not a real I2C bridge */
    I2C_BR_SMBUS,              /* the I2C interface is implemented by CPU SMBus */
    I2C_BR_SUSI,            /* the I2C driver implemented by Advantech SUSI */
} i2c_br_type_e;

struct i2c_bridge_s
{
    i2c_br_type_e i2c_br_type;  /* i2c bridge type */ 
    uint8 channel;         /* channel of the i2c dev in bridge */
    uint8 bridge_addr;       /* bridge address of the i2c dev*/ 
    int32 fd;
};

typedef struct i2c_bridge_s i2c_bridge_t;

struct i2c_gen_s
{
    i2c_type_e i2c_type;    /* i2c interface type */ 
    uint8 addr;            /* i2c component address */ 
    uint8 bridge_flag;     /* if access the i2c dev through bridge */            
    uint32 alen;             /* address length (byte)*/   
    i2c_bridge_t p_br;      /* bridge of the i2c dev */   
    uint8 i2c_bus_idx;      /* Added by qicx for bug 21474: 2012-11-27 */
    uint8 slave_bitmap;     /* used for ctc chip i2c interface. add by jqiu*/  
    uint8 lchip;            /* used for ctc chip i2c interface.add by qianj,2017-08-30*/
};
typedef struct i2c_gen_s i2c_gen_t;

/* define the structure including i2c operation paramaters */
struct i2c_op_para_s
{    
    uint32 offset;           /* internal address of the i2c component */    
    uint8 *p_val;              /* pointer of the value buffur */
    uint32 len;              /* data length (byte)*/    
#ifdef _CTC_X86_64_
    uint32 width;           /* register width (byte), added by shil */
#endif
};
typedef struct i2c_op_para_s i2c_op_para_t;


typedef struct i2c_handle_s i2c_handle_t;
struct i2c_handle_s
{
    int32 (*close)(i2c_handle_t *);
    int32 (*read)(const i2c_handle_t *, i2c_op_para_t *);
    int32 (*write)(const i2c_handle_t *, i2c_op_para_t *);
    void * data;
};

i2c_handle_t* i2c_create_handle( i2c_gen_t *i2c_pgen);
int32 i2c_open(i2c_type_e type);

#endif

