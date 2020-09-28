/****************************************************************************
* $Id$
*  spi cpu implement
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Chenxi Qi
* Date          : 2012-10-29
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "sal_common.h"
#include "genlog.h"

#include "ctc_spi.h"
#include "spi_cpu.h"
#include "drv_debug.h"
#include "spi_err.h"

#ifndef _CTC_LOONGSON_2F_
#include <linux/spi/spidev.h>
/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
//#define SI_MASK  0x1
//#define BIT_WIDTH_8  8
#define SPI_DEV_NAME_MAX_LEN 15


/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/* for gpio operation safety, add mutex for read and write , each time only one spi device W/R can be excuted */

sal_mutex_t *g_spi_cpu_mutex=NULL;

/****************************************************************************
 *
* Function
* 
****************************************************************************/
/*********************************************************************
 * Name    : spi_cpu_close
 * Purpose :  free memory and pointer
 * Input   : spi_handle_t *phdl       - the handler of the spi bus
          
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
spi_cpu_close(spi_handle_t *phdl)
{      
    DRV_CTC_CHK_PTR(phdl);
    
//    sal_mutex_destroy(g_spi_cpu_mutex);
        
    if (NULL != phdl->data)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl->data);
        phdl->data = NULL;
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl);
    phdl = NULL;
    
    return SPI_SUCCESS;
}

/*********************************************************************
 * Name    : spi_cpu_read
 * Purpose :  read spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
spi_cpu_read(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    spi_gen_t *spi_pgen = NULL;
    int32 ret;
    int32 spi_fd;
    uint8 *addr =NULL;
    uint8 *value = NULL;
    uint8 count = 0; 
	uint8		txbuf[7];
    uint8		rxbuf[7];
    struct spi_ioc_transfer	xfer[1];  

    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
	memset(xfer, 0, sizeof(xfer));
	memset(rxbuf, 0, sizeof(rxbuf));
    memset(txbuf, 0, sizeof(txbuf));
    
    spi_pgen = (spi_gen_t *)phdl->data;
    spi_fd = spi_pgen->spi_info.spi_cpu_info.fd;

    addr = (uint8*)(&ppara->addr);   
/* fix bug42271, support VPX-3U16T, liangf, 2017-01-24 */
#if (defined _CTC_LOONGSON_ || defined _CTC_ARM_CTC5236_)
    for(count = 0;count < (spi_pgen->alen);count++)
    {
        txbuf[count] = addr[(spi_pgen->alen-1) - count];
    }
#else
    for(count = 0;count < (spi_pgen->alen);count++)
    {
        txbuf[count] = addr[(spi_pgen->alen)+count];
    }
#endif    
    xfer[0].tx_buf = (unsigned long)txbuf;    /* Write address */
/*Bug29433 work around, resolve l2switch access problem. 2014-09-03*/    
#if 1
    if(spi_pgen->len == 4)
    {
        xfer[0].len = spi_pgen->alen + spi_pgen->len + 1;
    }
    else
    {
        xfer[0].len = spi_pgen->alen + spi_pgen->len;
    }
#else
    xfer[0].len = spi_pgen->alen + spi_pgen->len;
#endif
	xfer[0].rx_buf = (unsigned long)rxbuf;    /* Read value */

    /* for operation safety, add read and write mutex, each time only one spi device W/R can be excuted */
    sal_mutex_lock (g_spi_cpu_mutex);
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
    sal_mutex_unlock (g_spi_cpu_mutex);
    if(ret < 0)
    {
        ret = 1;  
    }
    else
    {   
#if (defined _CTC_LOONGSON_ || defined _CTC_ARM_CTC5236_)
        *(ppara->val) = 0;
        value = (uint8*)(ppara->val);
        for (count = 0; count < spi_pgen->len; count++)
        {
            value[spi_pgen->len -1 - count] = rxbuf[spi_pgen->alen + count];        
       }
#else        
        *(ppara->val) = 0;
        value = (uint8*)(ppara->val);
        if((spi_pgen->len)==1)
        {
            value[3] = rxbuf[(spi_pgen->alen) ];
        }
        else if((spi_pgen->len)==4)
        {
/*Bug29433 work around, resolve l2switch access problem. 2014-09-03*/            
#if 1
            for(count = 1;count < (spi_pgen->len);count++)
            {
                value[count] = rxbuf[(spi_pgen->alen) +count];
            }
            value[0] = rxbuf[(spi_pgen->alen) +4];
#else
            for(count = 0;count < (spi_pgen->len);count++)
            {
                value[count] = rxbuf[(spi_pgen->alen) +count];
            }
#endif            
        }
#endif        
        ret = 0;    
    }
    return ret;
}

/*********************************************************************
 * Name    : spi_cpu_write
 * Purpose :  write spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_cpu_write(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    spi_gen_t *spi_pgen = NULL;    
    int32 ret = 0;
    int32 spi_fd = 0;
    uint8 *addr =NULL;
    uint8 *value = NULL;
    uint8 count = 0; 
	uint8		txbuf[7];
    struct spi_ioc_transfer	xfer[1];  
    
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
	memset(xfer, 0, sizeof(xfer));
    memset(txbuf, 0, sizeof(txbuf));
    
    spi_pgen = (spi_gen_t *)phdl->data;
    spi_fd = spi_pgen->spi_info.spi_cpu_info.fd;

    addr = (uint8*)(&ppara->addr);
    value = (uint8 *)ppara->val;
/* fix bug42271, support VPX-3U16T, liangf, 2017-01-24 */
#if (defined _CTC_LOONGSON_ || defined _CTC_ARM_CTC5236_)
    for (count = 0;count < spi_pgen->alen; count++)
    {
        txbuf[count] = addr[spi_pgen->alen -1 -count];        
    }

    for (count = 0; count < spi_pgen->len; count++)
    {
        txbuf[spi_pgen->alen + count] = value[spi_pgen->len -1 - count];        
   }
#else    
    for(count = 0;count < (spi_pgen->alen) ;count++)
    {
        txbuf[count] = addr[(spi_pgen->alen)+count];        
    }

    for(count = 0;count < (spi_pgen->len) ;count++)
    {
        txbuf[(spi_pgen->alen) +count] = value[4 -(spi_pgen->len) +count];        
    }
#endif    
	xfer[0].tx_buf = (unsigned long)txbuf;    /* Write address */
	xfer[0].len = spi_pgen->alen + spi_pgen->len;

    /* for operation safety, add read and write mutex, each time only one spi device W/R can be excuted */
    sal_mutex_lock (g_spi_cpu_mutex);
	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
    sal_mutex_unlock (g_spi_cpu_mutex);
    
    if(ret < 0)
    {
        ret = 1;  
    }
    else
    {   
        ret = 0;    
    }
    return ret;    
}
#endif
/*********************************************************************
 * Name    : spi_cpu_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const spi_gen_t *spi_info     - some info about the spi bus implement
 
 * Output  : N/A
 * Return  : spi bus handle
           
 * Note    : N/A
*********************************************************************/

spi_handle_t *spi_cpu_create_handle(const spi_gen_t *spi_info)
{
#ifndef _CTC_LOONGSON_2F_
    spi_handle_t *phdl = NULL;    
    spi_gen_t  *spi_pgen;
    char spi_dev[SPI_DEV_NAME_MAX_LEN];

    phdl = (spi_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_handle_t));
    if (NULL == phdl)
    {
        DRV_LOG_ERR( "spi_cpu_create_handle spi handle null pointer!\n");
        goto err_out;
    }

    sal_memset(spi_dev, 0, SPI_DEV_NAME_MAX_LEN);
/* fix bug42271, support VPX-3U16T, liangf, 2017-01-24 */
#ifdef _CTC_LOONGSON_
    sal_sprintf(spi_dev, "/dev/spidev0.%d", spi_info->spi_info.spi_cpu_info.chip_sel);
#else
#ifdef _CTC_ARM_CTC5236_
    sal_sprintf(spi_dev, "/dev/spidev0.%d", spi_info->spi_info.spi_cpu_info.chip_sel);
#else
#ifdef _CTC_NID_  /* Chani modified for AD9559 driver, this will be kept compatibility later, 2013-11-01 */
    sal_sprintf(spi_dev, "/dev/spidev28672.%d", spi_info->spi_info.spi_cpu_info.chip_sel);
#else
    sal_sprintf(spi_dev, "/dev/spidev2.%d", spi_info->spi_info.spi_cpu_info.chip_sel);
#endif
#endif
#endif
    phdl->data = (spi_gen_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_gen_t));
    if (NULL == phdl->data)
    {
        DRV_LOG_ERR( "spi_cpu_create_handle spi handle data null pointer!\n");
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->data, (int8 *)spi_info, sizeof(spi_gen_t));
    spi_pgen = ((spi_gen_t *)phdl->data);

    if(g_spi_cpu_mutex == NULL)
    sal_mutex_create(&g_spi_cpu_mutex);

    spi_pgen->spi_info.spi_cpu_info.fd = sal_open(spi_dev, O_RDWR);
    if (spi_pgen->spi_info.spi_cpu_info.fd < 0)
    {
        DRV_LOG_ERR("opening SPI device failed\n");
        return NULL;
    }

    phdl->close = spi_cpu_close;
    phdl->read = spi_cpu_read;
    phdl->write = spi_cpu_write;

    return phdl;

err_out:
    if (NULL != phdl)
    {
        sal_mutex_destroy(g_spi_cpu_mutex);
        
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl->data);
            phdl->data = NULL;
        }
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl);
        phdl = NULL;
    }
#endif
    return NULL;
}

