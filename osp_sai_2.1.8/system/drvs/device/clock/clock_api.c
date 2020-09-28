/****************************************************************************
 * clock_api.c    clock api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       shil
 * Date:         2018-03-02.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "clock_api.h"
#include "clock_drv.h"
#include "clock_ad9559.h"
#include "clock_cdcm6208.h"
#include "clock_ds3104.h"
#include "clock_idt8t49n287.h"
#include "glb_synce_define.h"

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
/* clock handler */
static clock_handle_t *g_clock_hdl;
static uint32 g_clock_nums = 0;


/****************************************************************************
 *
* Functions  
*
****************************************************************************/

int32
clock_dev_init(uint32 idx)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
        
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].clock_init);

    g_clock_hdl[idx].clock_init(&g_clock_hdl[idx]);

    return 0;
}

int32 clock_get_status(uint32 idx, uint32 *state)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].clock_get_status);

    g_clock_hdl[idx].clock_get_status(&g_clock_hdl[idx], state);

    return 0;
}

int32 clock_intr_fixup(uint32 idx)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    if(NULL != g_clock_hdl[idx].intr_fix_up)
    {
        g_clock_hdl[idx].intr_fix_up(&g_clock_hdl[idx]);
    }

    return 0;
}

/*********************************************************************
 * Name    : clock_read
 * Purpose :  read clock register
 * Input   : uint32 idx         - the index of clock, usually index = 0
          uint16 addr           - the address of clock internal register 
          uint32 *val           - the pointer of read value
                              
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
clock_read(uint32 idx, uint16 addr, uint32  *val)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(val);
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].reg_read);
    
    return g_clock_hdl[idx].reg_read(&g_clock_hdl[idx], addr, val);
}

int32 
clock_read_indirect(uint32 idx, uint16 addr, uint32 *val)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(val);
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].reg_read_indirect);
    
    return g_clock_hdl[idx].reg_read_indirect(&g_clock_hdl[idx], addr, val);
}

/*********************************************************************
 * Name    : clock_write
 * Purpose :  write clock register
 * Input   : uint32 idx         - the index of clock, usually index = 0
          uint16 addr           - the address of clock internal register 
          uint32 val            - write value 
                              
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32 
clock_write(uint32 idx, uint16 addr, uint32  val)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].reg_write);
    
    return g_clock_hdl[idx].reg_write(&g_clock_hdl[idx], addr, &val);
}

int32 
clock_write_indirect(uint32 idx, uint16 addr, uint32 val)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].reg_write_indirect);
    
    return g_clock_hdl[idx].reg_write_indirect(&g_clock_hdl[idx], addr, &val);
}


/*********************************************************************
 * Name    : clock_close
 * Purpose :  free memory and pointer
 * Input   : uint32 idx     - the index of clock, usually index = 0
          
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/

int32
clock_close(uint32 idx)
{
    if (idx >= g_clock_nums || idx < 0)
        return CLOCK_E_INVALID_INDEX;
       
    DRV_CTC_CHK_PTR(&g_clock_hdl[idx]);
    DRV_CTC_CHK_PTR(g_clock_hdl[idx].clock_close);
    
    return g_clock_hdl[idx].clock_close(&g_clock_hdl[idx]);
}

/*********************************************************************
 * Name    : clock_init
 * Purpose :  init some data structure and config clock
 * Input   : spi_gen_t *spi_gen     - some info about the way of clock'spi bus implement
          uint32 num         - the number of clock
 * Output  : N/A
 * Return  : CLOCK_SUCCESS   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32 
clock_init(clock_info_t *clock_info, uint32 num)
{
    int32 i = 0;
    
    DRV_CTC_CHK_PTR(clock_info);
    
    g_clock_hdl = (clock_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_CLOCK_INFO, sizeof(clock_handle_t)*num);
    g_clock_nums = num;
    
    if(NULL == g_clock_hdl)
    {
        DRV_LOG_ERR("clock alloc handler fail!\n");
        return CLOCK_E_ERROR;
    }

    sal_memset(g_clock_hdl, 0, sizeof(clock_handle_t)*num);
    
    for (i = 0; i < num; i++)
    {
        switch(clock_info[i].clock_chip_type)
        {
            case CLOCK_AD9559:
                clock_ad9559_register_driver(&g_clock_hdl[i], &clock_info[i]);
                clock_dev_init(i);
                break;
            case CLOCK_CDCM6208:
                clock_cdcm6208_register_driver(&g_clock_hdl[i], &clock_info[i]);
                break;
            case CLOCK_DS3104:
                clock_ds3104_register_driver(&g_clock_hdl[i], &clock_info[i]);
                clock_dev_init(i);
            case CLOCK_IDT8T49N287:
                clock_idt8t49n287_register_driver(&g_clock_hdl[i], &clock_info[i]);
                clock_dev_init(i);                
		break;
            default:
                DRV_LOG_ERR("Unsupport clock chip type %d!\n", clock_info[i].clock_chip_type);
                return CLOCK_E_ERROR;
        }
    }
    return CLOCK_SUCCESS;
}

