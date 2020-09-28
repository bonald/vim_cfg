/****************************************************************************
 * $Id$
 *  spi err
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Jcao
 * Date          : 2010-08-07 
 * Reason        : First Create.
 ****************************************************************************/
#ifndef __SPI_ERR_H__
#define __SPI_ERR_H__

enum spi_error_e
{
    SPI_SUCCESS = 0,
    SPI_E_ERROR = -999,
    SPI_E_NO_MEMORY,
    SPI_E_NOT_INIT,
    SPI_E_INIT_FAILED,
    SPI_E_TIMEOUT,    
    SPI_E_READ,    
    SPI_E_WRITE,
    /* parameter check error */
    SPI_E_INVALID_PARAM,
    SPI_E_INVALID_PTR,
    SPI_E_INVALID_INDEX,    
    SPI_E_INVALID_LENGTH,
    SPI_E_INVALID_CHIP,

};
typedef enum spi_error_e spi_error_t;


#endif /*!__SPI_ERR_H__*/
