/****************************************************************************
 * clock_ds3104.c   ds3104 access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2011-02-17.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "ctc_spi.h"
#include "drv_debug.h"
#include "clock_drv.h"
#include "clock_ds3104.h"
#include "epld_drv.h"


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
void *epld_base;

#ifndef _GLB_UML_SYSTEM_
static void gpio_delay()
{ 
    volatile int32 loop = 0x10;
    while(loop--)
        ;
}
#endif
/****************************************************************************
 *
* Functions  
*
****************************************************************************/
/*********************************************************************
 * Name    : _clock_ds3104_close
 * Purpose :  free memory and pointer
 * Input   : clock_handle_t *phdl       - the handler of the clock
          
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ds3104_close(clock_handle_t* phdl)
{
    return phdl->iohdl->close(phdl->iohdl);
}


/*********************************************************************
 * Name    : _clock_ds3104_write
 * Purpose :  write ds3104 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ds3104_write(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
    
    access.addr= ((addr) << 1) | (DS3104_SPI_ACCESS_MODE) | (DS3104_SPI_WRITE<<15);
    access.val = val;    

    DRV_LOG_DEBUG(DRV_CLOCK, "_clock_ds3104_write addr %d, val pointer %p",access.addr, access.val);
    
    return phdl->iohdl->write(phdl->iohdl, &access);
}


/*********************************************************************
 * Name    : ds3104_write_indirect
 * Purpose :  write ds3104 register indirect
 * Input   : uint32 idx         - the index of ds3104, usually index = 0
          uint16 addr           - the address of ds3104 internal register 
          uint32 val            - write value 
                              
 * Output  : N/A
 * Return  : ds3104_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
_clock_ds3104_write_indirect(const clock_handle_t *phdl, uint16 addr, uint32  *val)
{
    /*switch cpu epld to ds3104 */
    *((uint8 *)(epld_base + EPLD_SPI_SWITCH)) = 0x2; 
    gpio_delay();
    *((uint8 *)(epld_base + 0xa)) = 0x1; 
    *((uint8 *)(epld_base + 0xa)) = 0x0; 

    return _clock_ds3104_write(phdl, addr, val);
}

/*********************************************************************
 * Name    : _clock_ds3104_read
 * Purpose :  read ds3104 register
 * Input   : const clock_handle_t *phdl       - the handler of the clock
          clock_access_t *paccess     - some info pass to spi bus layer
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ds3104_read(const clock_handle_t *phdl, uint16 addr, uint32 *val)
{
    clock_io_access_t access;
      
    access.addr= ((addr) << 1) | (DS3104_SPI_ACCESS_MODE) | (DS3104_SPI_READ<<15);
    access.val = val;

    DRV_LOG_DEBUG(DRV_CLOCK,"_clock_ds3104_read addr %d, val pointer %p",access.addr, access.val);    
    
    return phdl->iohdl->read(phdl->iohdl, &access);
}


/*********************************************************************
 * Name    : _clock_ds3104_read_indirect
 * Purpose :  read ds3104 register indirect
 * Input   : uint32 idx         - the index of ds3104, usually index = 0
          uint16 addr           - the address of ds3104 internal register 
          uint32 *val           - the pointer of read value
                              
 * Output  : N/A
 * Return  : ds3104_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
_clock_ds3104_read_indirect(const clock_handle_t *phdl, uint16 addr, uint32  *val)
{
    /*switch cpu epld to ds3104 */
    *((uint8 *)(epld_base + EPLD_SPI_SWITCH)) = 0x2; 
    gpio_delay();
    *((uint8 *)(epld_base + 0xa)) = 0x1; 
    *((uint8 *)(epld_base + 0xa)) = 0x0; 

    return _clock_ds3104_read(phdl, addr, val);
}

/****************************************************************************
 *
* Functions  
*
****************************************************************************/

/*********************************************************************
 * Name    : _clock_ds3104_init
 * Purpose :  configuration ds3104
 * Input   : uint32 idx         - the index of ds3104, usually index = 0
          uint32 clock_type     - the flag of clock type, such as richmod type, 
                                  humber type and 10G phy type etc
                              
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
_clock_ds3104_init(const clock_handle_t *phdl)
{
    int32 ret = 0;
    uint32 tmp;

    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x1ff, &tmp);

    tmp = 0x3;
    ret += _clock_ds3104_write(phdl, 0x1a8, &tmp);
    tmp = 0x2;
    ret += _clock_ds3104_write(phdl, 0x1ac, &tmp);
                        
    tmp = 0x1;
    ret += _clock_ds3104_write(phdl, 0x1cc, &tmp);
                        
    tmp = 0xc;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0x7b;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x8;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xdf;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xc;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0x79;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x8;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xf5;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xad;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x02;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xd5;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x16;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xdb;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x02;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xcd;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x1;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xdf;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x02;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xce;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x16;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
    /*58*/              
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xb7;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xce;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x51;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x01;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xc2;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xce;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x51;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xc9;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xcf;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xcc;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xd0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xce;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x03;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0xcd;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x01;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*106*/             
    tmp = 0xe;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x5f;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd1;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x51;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xa4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xd1;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x51;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x1;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    tmp = 0xf0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x11;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x8;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*122*/             
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x4;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*126*/             
    tmp = 0xe0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x2;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x16;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*130*/             
    tmp = 0x94;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xf;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x1;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*134*/             
    tmp = 0x3;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xb;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*138*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x45;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*142*/             
    tmp = 0xd1;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x44;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*146*/             
    tmp = 0xcd;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xa1;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x44;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*150*/             
    tmp = 0xae;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x2;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x16;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*154*/             
    tmp = 0xd;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0x35;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x7;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
     /*159*/            
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xff;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xa5;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0xe;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*164*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x7f;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xa9;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x3;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*168*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xff;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x52;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x7;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*172*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x3f;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x7e;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x4c;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*176*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xf0;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x2;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*180*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xed;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x36;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*184*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xb4;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xdb;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*188*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x55;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x55;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x5;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*192*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x44;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x44;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*196*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xfe;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0xff;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x3f;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*200*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0xc8;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x8f;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x9;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*204*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x181, &tmp);
    tmp = 0x11;
    ret += _clock_ds3104_write(phdl, 0x182, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x183, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x184, &tmp);
                        
    /*208*/             
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x180, &tmp);
                        
    /*customer config reidx,gister*/
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x20, &tmp);
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x21, &tmp);
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x22, &tmp);
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x23, &tmp);
    tmp = 0xc1;
    ret += _clock_ds3104_write(phdl, 0x24, &tmp);
                        
    tmp = 0xc0;
    ret += _clock_ds3104_write(phdl, 0x25, &tmp);
    tmp = 0x01;
    ret += _clock_ds3104_write(phdl, 0x27, &tmp);
    tmp = 0x01;
    ret += _clock_ds3104_write(phdl, 0x28, &tmp);
                        
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x4b, &tmp);
    tmp = 0x21;
    ret += _clock_ds3104_write(phdl, 0x18, &tmp);
    tmp = 0x43;
    ret += _clock_ds3104_write(phdl, 0x19, &tmp);
    tmp = 0x65;
    ret += _clock_ds3104_write(phdl, 0x1a, &tmp);
    tmp = 0x70;
    ret += _clock_ds3104_write(phdl, 0x1b, &tmp);
    tmp = 0x08;
    ret += _clock_ds3104_write(phdl, 0x1c, &tmp);
                        
    tmp = 0x10;
    ret += _clock_ds3104_write(phdl, 0x4b, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x18, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x19, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x1a, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x1b, &tmp);
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x1c, &tmp);
                        
    tmp = 0x0;
    ret += _clock_ds3104_write(phdl, 0x4b, &tmp);
                        
    tmp = 0x4a;
    ret += _clock_ds3104_write(phdl, 0x65, &tmp);
    tmp = 0x01;
    ret += _clock_ds3104_write(phdl, 0x4f, &tmp);
                        
    tmp = 0x54;
    ret += _clock_ds3104_write(phdl, 0x60, &tmp);
    tmp = 0x05;
    ret += _clock_ds3104_write(phdl, 0x61, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x62, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x63, &tmp);
                        
                        
    tmp = 0x99;
    ret += _clock_ds3104_write(phdl, 0x41, &tmp);
    tmp = 0x00;
    ret += _clock_ds3104_write(phdl, 0x42, &tmp);
                        
    tmp = 0x0e;
    ret += _clock_ds3104_write(phdl, 0x69, &tmp);
    tmp = 0x22;
    ret += _clock_ds3104_write(phdl, 0x73, &tmp);
    tmp = 0xe5;
    ret += _clock_ds3104_write(phdl, 0x74, &tmp);         
    tmp = 0x80;
    ret += _clock_ds3104_write(phdl, 0x44, &tmp);

    return ret;
}


/*********************************************************************
 * Name    : clock_ds3104_register_driver
 * Purpose :  create a clock ds3104 device handler
 * Input   : clock_handle_t *spi_phdl       - the handler of the clock          
                                       
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
          
 * Note    : N/A
*********************************************************************/
int32
clock_ds3104_register_driver(clock_handle_t* hdl, clock_info_t* clock_info)
{
    clock_handle_t* clock_hdl;
    DRV_CTC_CHK_PTR(clock_info);
    
    spi_gen_t* spi_pgen = (spi_gen_t*)clock_info->clock_gen;
    spi_pgen->alen = DS3104_ADDRESS_LENTH;
    spi_pgen->len = DS3104_DATA_LENTH;
    
    clock_hdl = hdl;
    memcpy(&clock_hdl->clock_info, clock_info, sizeof(clock_info_t));
    clock_hdl->iohdl = clock_io_register(clock_info->clock_io_type, clock_info->clock_gen);
    
    clock_hdl->clock_close = _clock_ds3104_close;
    clock_hdl->reg_read = _clock_ds3104_read;
    clock_hdl->reg_write = _clock_ds3104_write;
    clock_hdl->reg_read_indirect = _clock_ds3104_read_indirect;
    clock_hdl->reg_write_indirect = _clock_ds3104_write_indirect;
    clock_hdl->clock_init = _clock_ds3104_init;
    
    return 0;
}

