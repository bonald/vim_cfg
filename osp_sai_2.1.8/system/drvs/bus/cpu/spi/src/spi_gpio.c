/****************************************************************************
* $Id$
*  Software SPI implement
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-08-04
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "sal_common.h"
#include "genlog.h"

#include "ctc_spi.h"
#include "spi_gpio.h"
#include "drv_debug.h"
#include "spi_err.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
    
#define CTC_GPIO_SPI_MAJOR       115
#define CTC_GPIO_SPI_RW        _IO(CTC_GPIO_SPI_MAJOR, 15)  /* Combined R/W transfer (one stop only)*/
#define GPIO_SPI_DEV_NAME_MAX_LEN 25

/*
 * SPI Message - used for trpure spi ansaction, also from /dev interface
 */ 
struct ctc_spi_msg {
    uint16 addr;
    uint16 alen; /* addr len byte num, max is 2 */
    uint16 op_flags;
    uint16 len; /* buf byte num, max is 4 */
    uint32 *tx_buf;
    uint32 *rx_buf;
};

/* This is the structure as used in the ioctl call */
struct spi_rdwr_ioctl_data {
    struct ctc_spi_msg *msgs;   /* pointers to i2c_msgs */
};

/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/* for gpio operation safety, add mutex for read and write , each time only one spi device W/R can be excuted */
sal_mutex_t *g_spi_gpio_mutex;
static int32 gpio_spi_fd = -1;

/****************************************************************************
 *
* Function
* 
****************************************************************************/

/*********************************************************************
 * Name    : spi_gpio_read
 * Purpose :  read spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_gpio_read(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    spi_gen_t *spi_pgen = NULL;
    int32 ret;
    int32 spi_fd;
    struct spi_rdwr_ioctl_data spi_access;
    struct ctc_spi_msg spi_msg;
    uint32 tx_buf[1];
    uint32 rx_buf[1];
    uint32 *value =NULL;
    uint8 i = 0; 
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    spi_pgen = (spi_gen_t *)phdl->data;
    spi_fd = spi_pgen->spi_info.spi_gpio_info.fd;
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));

    spi_msg.addr = ppara->addr;
    spi_msg.alen = spi_pgen->alen;
    spi_msg.len = spi_pgen->len;
    spi_msg.op_flags = 1; /* 1 for read , 0 for write */
    spi_msg.rx_buf = rx_buf;
    spi_msg.tx_buf = tx_buf;

    spi_access.msgs = &spi_msg;
     /* for gpio operation safety, add read and write mutex, each time only one spi device W/R can be excuted */
    sal_mutex_lock (g_spi_gpio_mutex);        
    ret = ioctl(spi_fd, CTC_GPIO_SPI_RW, &spi_access);
    sal_mutex_unlock (g_spi_gpio_mutex);

    if(ret < 0)
    {
        ret = -1;
    }
    else
    {
        *(ppara->val) = 0;
        value = ppara->val;
        for(i = 0;i < (spi_pgen->len);i++)
        {
            //value[i] = rx_buf[0] & (0xff<<((spi_pgen->len - i - 1) * 8));
            value[i] = (rx_buf[0] >>( i * 8))& 0xff;
        }
        ret = 0;
    }

    return ret;
}

/*********************************************************************
 * Name    : spi_gpio_write
 * Purpose :  write spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_gpio_write(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    spi_gen_t *spi_pgen = NULL;
    int32 ret;
    int32 spi_fd;
    struct spi_rdwr_ioctl_data spi_access;
    struct ctc_spi_msg spi_msg;
    uint32 tx_buf[1];
    uint32 rx_buf[1];
    uint32 *value =NULL;
    uint8 i = 0; 
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    spi_pgen = (spi_gen_t *)phdl->data;
    spi_fd = spi_pgen->spi_info.spi_gpio_info.fd;
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    value = ppara->val;

    spi_msg.addr = ppara->addr;
    spi_msg.alen = spi_pgen->alen;
    spi_msg.len = spi_pgen->len;
    spi_msg.op_flags = 0; /* 1 for read , 0 for write */
    
    for(i = 0;i < (spi_pgen->len) ;i++)
    {
        tx_buf[0] |= (value[i]&0xff)<<(i*8);        
    }
    spi_msg.rx_buf = rx_buf;
    spi_msg.tx_buf = tx_buf;

    spi_access.msgs = &spi_msg;
    /* for gpio operation safety, add read and write mutex, each time only one spi device W/R can be excuted */
    sal_mutex_lock (g_spi_gpio_mutex);        
    ret = ioctl(spi_fd, CTC_GPIO_SPI_RW, &spi_access);
    sal_mutex_unlock (g_spi_gpio_mutex);

    if(ret < 0)
        ret = -1;
    else
        ret = 0;
    
    return ret;    
}
int32
spi_gpio_close(spi_handle_t *phdl)
{      
    DRV_CTC_CHK_PTR(phdl);
    
//    sal_mutex_destroy(g_spi_gpio_mutex);
        
    if (NULL != phdl->data)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl->data);
        phdl->data = NULL;
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl);
    phdl = NULL;
    
    return SPI_SUCCESS;
}

int32 raw_gpio_spi_read(uint32 addr, uint32 alen, uint8 *buf, uint32 len)
{
#if defined( _CTC_ARM_HI3535_) || defined(_CTC_ARM_LS1023A_) || defined(_CTC_ARM_CTC5236_)
    int32 ret; 
    int i;
    struct spi_rdwr_ioctl_data spi_access;
    struct ctc_spi_msg spi_msg;
    uint32 tx_buf[1];
    uint32 rx_buf[1];
    uint8 *value =NULL;
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));
    
    spi_msg.addr = addr;
    spi_msg.alen = alen;
    spi_msg.len = len;
    spi_msg.op_flags = 1; /* 1 for read , 0 for write */
    spi_msg.rx_buf = rx_buf;
    spi_msg.tx_buf = tx_buf;
    
    spi_access.msgs = &spi_msg;
    ret = ioctl(gpio_spi_fd, CTC_GPIO_SPI_RW, &spi_access);
    if(ret < 0)
    {
        ret = -1;
    }
    else
    {
        value = buf;
        for(i = 0;i < (len);i++)
        {
            value[i] = (rx_buf[0] >>( i * 8))& 0xff;
        }
        ret = 0;
    }
    return ret;
#else
    return 0;
#endif
}

int32 raw_gpio_spi_write(uint32 addr, uint32 alen, uint32 value, uint32 len)
{
#if defined( _CTC_ARM_HI3535_) || defined(_CTC_ARM_LS1023A_) || defined(_CTC_ARM_CTC5236_)
    int32 ret; 
    struct spi_rdwr_ioctl_data spi_access;
    struct ctc_spi_msg spi_msg;
    uint32 tx_buf[1];
    uint32 rx_buf[1];
    memset(rx_buf, 0, sizeof(rx_buf));
    memset(tx_buf, 0, sizeof(tx_buf));

    spi_msg.addr = addr;
    spi_msg.alen = alen;
    spi_msg.len = len;
    spi_msg.op_flags = 0; /* 1 for read , 0 for write */
    
    tx_buf[0] = value;
    spi_msg.rx_buf = rx_buf;
    spi_msg.tx_buf = tx_buf;

    spi_access.msgs = &spi_msg;
    ret = ioctl(gpio_spi_fd, CTC_GPIO_SPI_RW, &spi_access);
    if(ret < 0)
        ret = -1;
    else
        ret = 0;

    return ret;

#else
    return 0;
#endif
}

/*********************************************************************
 * Name    : spi_gpio_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const spi_gen_t *spi_info     - some info about the spi bus implement
 
 * Output  : N/A
 * Return  : spi bus handle
           
 * Note    : N/A
*********************************************************************/

spi_handle_t *spi_gpio_create_handle(const spi_gen_t *spi_info)
{
    spi_handle_t *phdl = NULL;    
    spi_gen_t  *spi_pgen;
    char spi_dev[GPIO_SPI_DEV_NAME_MAX_LEN];

    phdl = (spi_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_handle_t));
    if (NULL == phdl)
    {
        DRV_LOG_ERR( "spi_gpio_create_handle spi handle null pointer!\n");
        goto err_out;
    }
    sal_memset(spi_dev, 0, GPIO_SPI_DEV_NAME_MAX_LEN);
#if defined( _CTC_ARM_HI3535_) || defined(_CTC_ARM_LS1023A_) || defined(_CTC_ARM_CTC5236_)
    sal_sprintf(spi_dev, "/dev/ctc_gpio_spi");
#endif
    /*resolve data overflow. jqiu modify. bug 13956. 2011-01-22*/
    phdl->data = (spi_gen_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_gen_t));
    if (NULL == phdl->data)
    {
        DRV_LOG_ERR( "spi_gpio_create_handle spi handle data null pointer!\n");
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->data, (int8 *)spi_info, sizeof(spi_gen_t));
    spi_pgen = ((spi_gen_t *)phdl->data);

    if(g_spi_gpio_mutex == NULL)
        sal_mutex_create(&g_spi_gpio_mutex);
    spi_pgen->spi_info.spi_gpio_info.fd = sal_open(spi_dev, O_RDWR);
    if (spi_pgen->spi_info.spi_gpio_info.fd < 0)
    {
        DRV_LOG_ERR("opening GPIO_SPI device failed\n");
        return NULL;
    }
    gpio_spi_fd = spi_pgen->spi_info.spi_gpio_info.fd;

    phdl->close = spi_gpio_close;
    phdl->read = spi_gpio_read;
    phdl->write = spi_gpio_write;
   
    return phdl;
    
err_out:
    if (NULL != phdl)
    {
        sal_mutex_destroy(g_spi_gpio_mutex);
        
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl->data);
            phdl->data = NULL;
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl);
        phdl = NULL;
    }

    return NULL;
}

