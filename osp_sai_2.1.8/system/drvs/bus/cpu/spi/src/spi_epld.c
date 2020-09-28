/****************************************************************************
* $Id$
*  EPLD SPI implement
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jcao
* Date          : 2010-08-25
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files
* 
****************************************************************************/
#include "ctc_spi.h"
#include "spi_epld.h"
#include "drv_debug.h"
#include "spi_err.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#define SPI_EPLD_TIMEOUT    0x7ff
#define EPLD_STATUS_OFF  0x10
#define EPLD_CMD_H_OFF     0x11
#define EPLD_CMD_L_OFF     0x12
#define	EPLD_READ_OFF    0x14
#define EPLD_WRITE_OFF   0x13


/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/* for operation safety, add mutex for read and write , each time only one spi device W/R can be excuted */

 sal_mutex_t *g_spi_epld_mutex;


/****************************************************************************
 *
* Function
* 
****************************************************************************/
/*********************************************************************
 * Name    : spi_epld_close
 * Purpose :  free memory and pointer
 * Input   : spi_handle_t *phdl       - the handler of the spi bus
          
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_epld_close(spi_handle_t *phdl)
{      
    DRV_CTC_CHK_PTR(phdl);

    sal_mutex_destroy(g_spi_epld_mutex);
    
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
 * Name    : spi_epld_read
 * Purpose :  read spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_epld_read(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint8 *status_addr;
    volatile uint8 *cmd_addr_h;
    volatile uint8 *cmd_addr_l;
    volatile uint8 *read_addr;    
    uint16 cmd;        
    spi_gen_t *spi_pgen = NULL;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    spi_pgen = (spi_gen_t *)phdl->data;
        
    status_addr = (spi_pgen->spi_info.spi_epld_info.baseaddr+ EPLD_STATUS_OFF);
    cmd_addr_h = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_CMD_H_OFF);
    cmd_addr_l = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_CMD_L_OFF);
    read_addr = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_READ_OFF);

    sal_mutex_lock (g_spi_epld_mutex); 
     
    timeout = SPI_EPLD_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_epld_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    cmd = (uint16_t)ppara->addr;
    
    *cmd_addr_h = ((cmd & 0xff00) >> 8);    
    *cmd_addr_l = (cmd & 0xff);

    timeout = SPI_EPLD_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_epld_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    *ppara->val = *read_addr;
 
    sal_mutex_unlock (g_spi_epld_mutex); 
    
    DRV_LOG_DEBUG(DRV_BUS_SPI, "spi_epld_read addr %d, val %d",
                            ppara->addr, *(ppara->val)); 

    return SPI_SUCCESS;
}

/*********************************************************************
 * Name    : spi_epld_write
 * Purpose :  write spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_epld_write(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint8 *status_addr;
    volatile uint8 *cmd_addr_h;
    volatile uint8 *cmd_addr_l;
    volatile uint8 *write_addr;
    spi_gen_t *spi_pgen = NULL;
    uint16_t cmd;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    DRV_LOG_DEBUG(DRV_BUS_SPI, "spi_epld_write addr %d, val pointer %p",
                            ppara->addr, ppara->val); 
   
    spi_pgen = (spi_gen_t *)phdl->data;

    status_addr = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_STATUS_OFF);
    cmd_addr_h = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_CMD_H_OFF);
    cmd_addr_l = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_CMD_L_OFF);
    write_addr = (spi_pgen->spi_info.spi_epld_info.baseaddr + EPLD_WRITE_OFF);
    
    sal_mutex_lock (g_spi_epld_mutex); 
    timeout = SPI_EPLD_TIMEOUT;

    while(((*status_addr & spi_pgen->spi_info.spi_epld_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_WRITE; 
    }   
    *write_addr = (uint8)(*(ppara->val));   

    cmd = (uint16_t)(ppara->addr);
   
    *cmd_addr_h = ((cmd & 0xff00) >> 8);
    *cmd_addr_l = (cmd & 0xff);
    sal_mutex_unlock (g_spi_epld_mutex); 
    return SPI_SUCCESS;
}



/*********************************************************************
 * Name    : spi_epld_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const spi_gen_t *spi_info     - some info about the spi bus implement
 
 * Output  : N/A
 * Return  : spi bus handle
           
 * Note    : N/A
*********************************************************************/

spi_handle_t *
spi_epld_create_handle(const spi_gen_t *spi_info)
{
    spi_handle_t *phdl = NULL;

    phdl = (spi_handle_t *)(spi_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO, 
                                          sizeof(spi_handle_t));
    if (NULL == phdl)
    {
        DRV_LOG_ERR( "spi_epld_create_handle can't malloc memory!\n");
        goto err_out;
    }

    phdl->data = (spi_gen_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO, 
                                          sizeof(spi_gen_t));
    if (NULL == phdl->data)
    {
        DRV_LOG_ERR( "spi_epld_create_handle can't malloc memory!\n");
        goto err_out;
    }

    memcpy((int8 *)phdl->data, (int8 *)spi_info, sizeof(spi_gen_t));

    sal_mutex_create(&g_spi_epld_mutex);
   
    phdl->read = spi_epld_read;
    phdl->write = spi_epld_write;
    
    phdl->close = spi_epld_close;
    
    return phdl;
    
err_out:
    if (NULL != phdl)
    {        
        if (NULL != phdl->data)
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl->data);
            phdl->data = NULL;
        }
        
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO, phdl);
        phdl = NULL;
    }
    sal_mutex_destroy(g_spi_epld_mutex);
    return NULL;
}

