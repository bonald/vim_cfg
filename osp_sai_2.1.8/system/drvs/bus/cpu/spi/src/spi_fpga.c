/****************************************************************************
* $Id$
*  FPGA SPI implement
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
#include "glb_macro.h"
#include "ctc_spi.h"
#include "spi_fpga.h"
#include "drv_debug.h"
#include "spi_err.h"

/****************************************************************************
 *
* Defines and Macros
* 
****************************************************************************/
#define SPI_FPGA_TIMEOUT 0x7ff
#define STATUS_OFF  0x08
#define CMD_OFF     0x10
#define	READ_OFF    0x18
#define WRITE_OFF   0x14


/****************************************************************************
 *
* Global and Declaration
* 
****************************************************************************/

/* for operation safety, add mutex for read and write , each time only one spi device W/R can be excuted */

 sal_mutex_t *g_spi_fpga_mutex;


/****************************************************************************
 *
* Function
* 
****************************************************************************/
/*********************************************************************
 * Name    : spi_fpga_close
 * Purpose :  free memory and pointer
 * Input   : spi_handle_t *phdl       - the handler of the spi bus
          
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_fpga_close(spi_handle_t *phdl)
{      
    DRV_CTC_CHK_PTR(phdl);

    sal_mutex_destroy(g_spi_fpga_mutex);
    
    if (NULL != phdl->data)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl->data);
        phdl->data = NULL;
    }
    DRV_FREE(CTCLIB_MEM_DRIVER_SPI_INFO,phdl);
    phdl = NULL;
    
    
    return SPI_SUCCESS;
}

/*********************************************************************
 * Name    : spi_fpga_read
 * Purpose :  read spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
spi_fpga_cmd_uint32_val_uint8_read(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint32 *status_addr;
    volatile uint32 *cmd_addr;
    volatile uint8 *read_addr;    
    uint32 cmd;

    spi_gen_t *spi_pgen = NULL;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    spi_pgen = (spi_gen_t *)phdl->data;
        
    status_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr+ STATUS_OFF);
    cmd_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + CMD_OFF);
    read_addr = (uint8 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + READ_OFF);

    sal_mutex_lock (g_spi_fpga_mutex);         
    timeout = SPI_FPGA_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    cmd = ppara->addr;
    *cmd_addr = GLB_CTC_SWAP32(cmd);

    timeout = SPI_FPGA_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    *ppara->val = *read_addr;
 
    sal_mutex_unlock (g_spi_fpga_mutex); 
    
    DRV_LOG_DEBUG(DRV_BUS_SPI,"spi_fpga_read addr %d, val %d",
                            ppara->addr, *(ppara->val)); 

    return SPI_SUCCESS;
}

/*********************************************************************
 * Name    : spi_fpga_write
 * Purpose :  write spi bus
 * Input   : const spi_handle_t *phdl       - the handler of the spi bus
          spi_op_para_t *ppara     - some info about spi bus layer operation
                                       
 * Output  : N/A
 * Return  : SPI_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
spi_fpga_cmd_uint32_val_uint8_write(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint32 *status_addr;
    volatile uint32 *cmd_addr;
    volatile uint8 *write_addr;
    spi_gen_t *spi_pgen = NULL;
    uint32 cmd;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    DRV_LOG_DEBUG(DRV_BUS_SPI,"spi_fpga_write addr %d, val pointer %p",
                            ppara->addr, ppara->val); 
   
    spi_pgen = (spi_gen_t *)phdl->data;

    status_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + STATUS_OFF);
    cmd_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr +  CMD_OFF);
    write_addr = (uint8 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + WRITE_OFF);

    sal_mutex_lock (g_spi_fpga_mutex); 
    
    timeout = SPI_FPGA_TIMEOUT;

    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_WRITE; 
    }   
    *write_addr = (uint8)(*(ppara->val));   

    cmd = ppara->addr;
    *cmd_addr = GLB_CTC_SWAP32(cmd);

    sal_mutex_unlock (g_spi_fpga_mutex); 
    
    return SPI_SUCCESS;
}

int32 
spi_fpga_cmd_uint16_val_uint32_read(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint32 *status_addr;
    volatile uint16 *cmd_addr;
    volatile uint32 *read_addr;    

    uint32 value_swap;    
    uint16 cmd_swap;
    
    spi_gen_t *spi_pgen = NULL;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    spi_pgen = (spi_gen_t *)phdl->data;
        
    status_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr+ STATUS_OFF);
    cmd_addr = (uint16 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + CMD_OFF);
    read_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + READ_OFF);

    sal_mutex_lock (g_spi_fpga_mutex); 
    
    timeout = SPI_FPGA_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    cmd_swap = (uint16)ppara->addr;
    *cmd_addr = GLB_CTC_SWAP16(cmd_swap);

    timeout = SPI_FPGA_TIMEOUT; 
    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_READ;
    }   
    value_swap = *read_addr;
    *ppara->val = GLB_CTC_SWAP32(value_swap);

    sal_mutex_unlock (g_spi_fpga_mutex); 
    
    DRV_LOG_DEBUG(DRV_BUS_SPI,"spi_fpga_read addr %d, val %d",
                            ppara->addr, *(ppara->val)); 

    return SPI_SUCCESS;
}


int32 
spi_fpga_cmd_uint16_val_uint32_write(const spi_handle_t *phdl, spi_op_para_t *ppara)
{
    uint32 timeout;
    volatile uint32 *status_addr;
    volatile uint16 *cmd_addr;
    volatile uint32 *write_addr;
    spi_gen_t *spi_pgen = NULL;
    uint32 value_swap;
    uint16 cmd_swap;
        
    DRV_CTC_CHK_PTR(phdl);
    DRV_CTC_CHK_PTR(ppara);
    
    DRV_LOG_DEBUG(DRV_BUS_SPI,"spi_fpga_write addr %d, val pointer %p",
                            ppara->addr, ppara->val); 
   
    spi_pgen = (spi_gen_t *)phdl->data;

    status_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + STATUS_OFF);
    cmd_addr = (uint16 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr +  CMD_OFF);
    write_addr = (uint32 *)((uintptr_t)spi_pgen->spi_info.spi_fpga_info.baseaddr + WRITE_OFF);

    sal_mutex_lock (g_spi_fpga_mutex); 
    
    timeout = SPI_FPGA_TIMEOUT;

    while(((*status_addr & spi_pgen->spi_info.spi_fpga_info.busy) != 0) && timeout)
    {
        timeout--;
    }   
    if(0 == timeout)
    {
        return SPI_E_WRITE; 
    }   
    value_swap = *ppara->val;
    *write_addr = GLB_CTC_SWAP32(value_swap);
    cmd_swap = (uint16)ppara->addr;
    *cmd_addr = GLB_CTC_SWAP16(cmd_swap);

    sal_mutex_unlock (g_spi_fpga_mutex); 
    
    return SPI_SUCCESS;
}

/*********************************************************************
 * Name    : spi_fpga_create_handle
 * Purpose :  create low level I/O handle
 * Input   : const spi_gen_t *spi_info     - some info about the spi bus implement
 
 * Output  : N/A
 * Return  : spi bus handle
           
 * Note    : N/A
*********************************************************************/

spi_handle_t *
spi_fpga_create_handle(const spi_gen_t *spi_info)
{
    spi_handle_t *phdl = NULL;

    phdl = (spi_handle_t *)(spi_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_handle_t));
    if (NULL == phdl)
    {
        DRV_LOG_ERR( "spi_fpga_create_handle can't malloc memory!\n");
        goto err_out;
    }

    phdl->data = (spi_gen_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_SPI_INFO ,sizeof(spi_gen_t));
    if (NULL == phdl->data)
    {
        DRV_LOG_ERR( "spi_fpga_create_handle can't malloc memory!\n");
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->data, (int8 *)spi_info, sizeof(spi_gen_t));

    sal_mutex_create(&g_spi_fpga_mutex);
    
    if(spi_info->spi_info.spi_fpga_info.fpga_type== CMD_UINT32_VAL_UINT8)
    {
        phdl->read = spi_fpga_cmd_uint32_val_uint8_read;
        phdl->write = spi_fpga_cmd_uint32_val_uint8_write;
    }
    else if(spi_info->spi_info.spi_fpga_info.fpga_type == CMD_UINT16_VAL_UINT32)
    {
        phdl->read = spi_fpga_cmd_uint16_val_uint32_read;
        phdl->write = spi_fpga_cmd_uint16_val_uint32_write;
    }
    else
    {
        goto err_out;
    }
       
    phdl->close = spi_fpga_close;
    
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

    sal_mutex_destroy(g_spi_fpga_mutex);
    
    return NULL;
}

