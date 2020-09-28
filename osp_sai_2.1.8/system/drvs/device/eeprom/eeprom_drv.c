/****************************************************************************
 * eeprom_drv.c   eeprom access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-09-26.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_spi.h"
#include "drv_debug.h"
#include "eeprom_drv.h"


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



/*********************************************************************
 * Name    : eeprom_i2c_read
 * Purpose :  read a i2c type eeprom register
 * Input   : const eeprom_handle_t *p_hdl       - the handler of the eeprom
          eeprom_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_i2c_read(const eeprom_handle_t* p_hdl, eeprom_access_t* p_para)
{
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl->handle);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_para->len;
    i2c_para.offset = p_para->offset;
    i2c_para.p_val = p_para->p_val;
    
    return i2c_phdl->read(i2c_phdl, &i2c_para);
}



/*********************************************************************
 * Name    : eeprom_i2c_write
 * Purpose :  write a i2c type eeprom register
 * Input   : const eeprom_handle_t *p_hdl       - the handler of the eeprom
          eeprom_access_t *p_para     - some info pass to i2c bus layer
                                       
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
eeprom_i2c_write(const eeprom_handle_t* p_hdl, eeprom_access_t* p_para)
{
    i2c_handle_t *i2c_phdl = NULL;
    i2c_op_para_t i2c_para;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_para);
    DRV_CTC_CHK_PTR(p_hdl->handle);
    
    i2c_phdl = (i2c_handle_t *)p_hdl->handle;
    i2c_para.len = p_para->len;
    i2c_para.offset = p_para->offset;
    i2c_para.p_val = p_para->p_val;
   
    return i2c_phdl->write(i2c_phdl, &i2c_para);
}



/*********************************************************************
 * Name    : eeprom_i2c_close
 * Purpose :  free memory and pointer
 * Input   : eeprom_handle_t *phdl       - the handler of the eeprom
          
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
eeprom_i2c_close(eeprom_handle_t *p_hdl)
{
    int32 ret = 0;
    i2c_handle_t *i2c_phdl = NULL;

    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_hdl->handle);

    i2c_phdl = (i2c_handle_t *)p_hdl->handle;

    ret = i2c_phdl->close(i2c_phdl);
    DRV_FREE( CTCLIB_MEM_DRIVER_EEPROM_INFO, p_hdl);
    i2c_phdl = NULL;

    return ret;
}



static eeprom_handle_t *
_eeprom_i2c_create_handle(i2c_handle_t *i2c_phdl)
{
    eeprom_handle_t *p_hdl = NULL; 
    
    p_hdl = (eeprom_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_EEPROM_INFO,sizeof(eeprom_handle_t));
    if (NULL == p_hdl)
    {
        goto err_out;
    }

    p_hdl->handle = (void *)i2c_phdl;

    p_hdl->close = eeprom_i2c_close;
    p_hdl->read = eeprom_i2c_read;
    p_hdl->write = eeprom_i2c_write;
    
    return p_hdl;
    
err_out:
    if (NULL != p_hdl)
    {
        DRV_FREE( CTCLIB_MEM_DRIVER_EEPROM_INFO, p_hdl);
        p_hdl = NULL;
    }

    return NULL;
}




static eeprom_handle_t *
_eeprom_i2c_register(i2c_gen_t *i2c_pgen)
{
    i2c_handle_t *i2c_phdl = NULL;
    
    /* the i2c eeprom address len is fixed and we save them in i2c bus handle */
//    i2c_pgen->alen = EEPROM_OFFSET_WIDTH;

    i2c_phdl = i2c_create_handle(i2c_pgen);
    return _eeprom_i2c_create_handle( i2c_phdl);      
}


/****************************************************************************
 * Name	: eeprom_register
 * Purpose: register a eeprom device handler, initialize the handler
 * Input	:  eeprom type, the pointer to i2c general infomation struct
 * Output:  
 * Return:  the handler of the eeprom
 * Note	:
****************************************************************************/	
eeprom_handle_t *
eeprom_register(eeprom_type_t type, const void *p_gen)
{
    i2c_gen_t *i2c_pgen = (i2c_gen_t *)p_gen;
    
    DRV_CTC_CHK_PTR_NULL(p_gen);
    
    switch(type)
    {
        case E_EEPROM_I2C:
            return _eeprom_i2c_register(i2c_pgen);

        default:
            break;        
    }
    
    return NULL;
}


