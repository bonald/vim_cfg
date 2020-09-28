/****************************************************************************
 * ctc_spi.c   SPI devices access and getting the information
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-08-04.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_spi.h"
#include "spi_gpio.h"
#include "spi_fpga.h"
#include "spi_epld.h"
#include "spi_cpu.h"
#include "drv_debug.h"
#include "spi_err.h"


/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/



/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/


/****************************************************************************
 *
* Functions  
*
****************************************************************************/
/*********************************************************************
 * Name    : spi_create_handle
 * Purpose :  create low level I/O handle
 * Input   : spi_gen_t *spi_gen     - some info about the way of ad9517'spi bus implement
 
 * Output  : N/A
 * Return  : spi bus handle
           
 * Note    : N/A
*********************************************************************/

spi_handle_t *
spi_create_handle (const spi_gen_t *spi_pgen)
{
        spi_handle_t *phdl = NULL;
          
        DRV_CTC_CHK_PTR_NULL(spi_pgen);
        
        switch (spi_pgen->spi_type)
        {
#if defined( _CTC_ARM_HI3535_) || defined(_CTC_ARM_LS1023A_) || defined(_CTC_ARM_CTC5236_)
            case E_SPI_GPIO:
                phdl = (spi_handle_t *)spi_gpio_create_handle(spi_pgen);
                break;
#endif
            case E_SPI_FPGA:
                phdl = (spi_handle_t *)spi_fpga_create_handle(spi_pgen);
                break;    
            case E_SPI_EPLD:
                phdl = (spi_handle_t *)spi_epld_create_handle(spi_pgen);
                break;    
            case E_SPI_CPU:
                phdl = (spi_handle_t *)spi_cpu_create_handle(spi_pgen);
                break;
            default:
                DRV_LOG_ERR( "spi_create_handle don't support this type!\n");
                break;
        }
        
        return phdl;
}

