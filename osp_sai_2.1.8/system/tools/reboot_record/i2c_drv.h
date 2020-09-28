#ifndef __CTC_I2C_DRV_H__
#define __CTC_I2C_DRV_H__

#define I2C_WRITE_OP    0
#define I2C_READ_OP     1
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
    int32 nmsgs;               /* number of i2c_msgs */
};

int32 i2c_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len);

#endif
