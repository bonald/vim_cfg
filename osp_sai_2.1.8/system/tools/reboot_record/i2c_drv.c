#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_tempfile_define.h"

#include "i2c_drv.h"

static int32 i2c_dev0_fd = -1;
static int32 i2c_dev1_fd = -1;


static int i2c_open_dev0(void)
{
    if (i2c_dev0_fd < 0)
    {
        i2c_dev0_fd = open("/dev/i2c-0", O_RDWR);
        if (i2c_dev0_fd < 0)
        {
            return -1;
        }
    }
    return 0;
}

static int i2c_open_dev1(void)
{
    if (i2c_dev1_fd < 0)
    {
        i2c_dev1_fd = open("/dev/i2c-1", O_RDWR);
        if (i2c_dev1_fd < 0)
        {
            return -1;
        }
    }
    return 0;
}

static int32 i2c_switch_msg(struct ctc_i2c_msg* msg, uint16 addr,
                         uint8* p_buf, uint16 flag, int32 len)
{
    if(NULL == msg)
    {
        return -1;
    }

    msg->addr = addr;
    msg->buf = p_buf;
    msg->flags = flag;
    msg->len = len;
    return 0;
}

static int32 i2c_transfer(uint8 i2c_bus_idx, struct i2c_rdwr_ioctl_data* msgset)
{
    int32 ret=0;
    if (NULL == msgset)
    {
        return -1;
    }
    if (GLB_I2C_IDX_0 == i2c_bus_idx)
    {
        ret = ioctl(i2c_dev0_fd, I2C_RDWR, msgset);    
    }
    else if (GLB_I2C_IDX_1 == i2c_bus_idx)
    {
        ret = ioctl(i2c_dev1_fd, I2C_RDWR, msgset);    
    }
    return ret;
}

int32 i2c_write(uint8 i2c_bus_idx, uint16 addr, uint32 offset, uint8 alen, uint8* buf, uint8 len)
{
    int32 ret, i=0;
    struct i2c_rdwr_ioctl_data msgset;    
    struct ctc_i2c_msg msgs[2];
    uint8 offset_buf[4];

    if(i2c_dev0_fd < 0 && i2c_bus_idx == 0)
    {
        i2c_open_dev0();
    }

    if(i2c_dev1_fd < 0 && i2c_bus_idx == 1)
    {
        i2c_open_dev1();
    }
    
    if(alen == 1)
    {
        offset_buf[i++] = (uint8)(offset&0xff);        
    }
    else if(alen == 2)
    {
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 3)
    { 
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    else if(alen == 4)
    {
        offset_buf[i++] = (uint8)((offset>>24)&0xff);  
        offset_buf[i++] = (uint8)((offset>>16)&0xff);
        offset_buf[i++] = (uint8)((offset>>8)&0xff);
        offset_buf[i++] = (uint8)(offset&0xff);  
    }
    sal_memcpy(&offset_buf[i], buf, len);
    i2c_switch_msg(msgs, addr, offset_buf, I2C_WRITE_OP, alen+len);
                           
    msgset.msgs = msgs;
    msgset.nmsgs = 1;
    ret = i2c_transfer(i2c_bus_idx, &msgset);    
    return ret;
}

