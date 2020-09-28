/****************************************************************************
 * api_epld.c    epld api of chsm
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Zhu Jian
 * Date:         2008-7-21.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_macro.h"
#include "ctclib_debug.h"
#include "drv_debug.h"
#include "epld_drv.h"
#include "epld_api.h"
#include "drv_debug.h"
#include "ctclib_sys_cmd.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
/* the max number of the EPLD in one board  */
#define EPLD_MAX_NUM 2
/* two system power: primary and reserved  */
#define EPLD_SYS_PSU_MAX 2
/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
#ifndef _CTC_LOONGSON_
static epld_handle_t* g_epld_hdl[EPLD_MAX_NUM];
static epld_info_t g_epld_info[EPLD_MAX_NUM];
#endif

/****************************************************************************
 *
* Functions  
*
****************************************************************************/    

/*****************************************************************************
 * Name         :   epld_get_fiber_present
 * Purpose      :   check if the fiber is present
 * Input        :   
 *              :    
 * Output       :   
 * Return       :   
 * Note         :   
 *****************************************************************************/
int32 epld_get_fiber_present(uint32 chip_no, uint32 *fiber_present)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 val = 0;
    int32 bit_len1, bit_len2, bit_len3, bit_len4, bit_len5, bit_len6, bit_len7;

    bit_len1 = epld_item_get_width(0, EPLD_SFP_PRESENT1);
    bit_len2 = epld_item_get_width(0, EPLD_SFP_PRESENT2);
    bit_len3 = epld_item_get_width(0, EPLD_SFP_PRESENT3);
    bit_len4 = epld_item_get_width(0, EPLD_SFP_PRESENT4);
    bit_len5 = epld_item_get_width(0, EPLD_SFP_PRESENT5);
    bit_len6 = epld_item_get_width(0, EPLD_SFP_PRESENT6);
    bit_len7 = epld_item_get_width(0, EPLD_SFP_PRESENT7);

    if(bit_len1 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT1, &val);        
        *fiber_present = !((val>>chip_no) &0x1);         
    }
    else if(bit_len1+bit_len2 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT2, &val);
        *fiber_present = !((val>>(chip_no-bit_len1))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT3, &val);
        *fiber_present = !((val>>(chip_no-bit_len1-bit_len2))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT4, &val);
        *fiber_present = !((val>>(chip_no-bit_len1-bit_len2-bit_len3))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT5, &val);
        *fiber_present = !((val>>(chip_no-bit_len1-bit_len2-bit_len3-bit_len4))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5+bit_len6 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT6, &val);
        *fiber_present = !((val>>(chip_no-bit_len1-bit_len2-bit_len3-bit_len4-bit_len5))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5+bit_len6 +bit_len7 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_PRESENT7, &val);
        *fiber_present = !((val>>(chip_no-bit_len1-bit_len2-bit_len3-bit_len4-bit_len5-bit_len6))&0x1);
    }
    else
    {
        DRV_LOG_ERR("Get fiber present: chip_no %d is out of range!", chip_no);
        ret = -1;
    }
    DRV_LOG_DEBUG(DRV_EPLD, " Get fiber present: chip_no [%d], val[0x%x], present[%d]\n",chip_no,val, *fiber_present);
    return ret;    
#endif
}

/*****************************************************************************
 * Name         :   epld_get_fiber_los
 * Purpose      :   check if the fiber signal is los
 * Input        :   
 *              :    
 * Output       :   
 * Return       :   
 * Note         : If the port has not register EPLD los signal, will return -1, and won't change los value. bug25338 
 *****************************************************************************/
int32 epld_get_fiber_los(uint32 chip_no, uint32 *los)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 val = 0;
    int32 bit_len1, bit_len2, bit_len3, bit_len4, bit_len5, bit_len6;

    bit_len1 = epld_item_get_width(0, EPLD_SFP_LOS1);
    bit_len2 = epld_item_get_width(0, EPLD_SFP_LOS2);
    bit_len3 = epld_item_get_width(0, EPLD_SFP_LOS3);
    bit_len4 = epld_item_get_width(0, EPLD_SFP_LOS4);
    bit_len5 = epld_item_get_width(0, EPLD_SFP_LOS5);
    bit_len6 = epld_item_get_width(0, EPLD_SFP_LOS6);

    if(bit_len1 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS1, &val);        
        *los = ((val>>chip_no) &0x1);         
    }
    else if(bit_len1+bit_len2 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS2, &val);
        *los = ((val>>(chip_no-bit_len1))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS3, &val);
        *los = ((val>>(chip_no-bit_len1-bit_len2))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS4, &val);
        *los = ((val>>(chip_no-bit_len1-bit_len2-bit_len3))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS5, &val);
        *los = ((val>>(chip_no-bit_len1-bit_len2-bit_len3-bit_len4))&0x1);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5+bit_len6 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_LOS6, &val);
        *los = ((val>>(chip_no-bit_len1-bit_len2-bit_len3-bit_len4-bit_len5))&0x1);
    }
    else
    {        
        ret = -1;
    }
    DRV_LOG_DEBUG(DRV_EPLD, " Get fiber los: chip_no [%d], val[0x%x], los[%d]\n",chip_no,val, *los);
    return ret;    
#endif
}


int32 
epld_set_sfp_enable(uint32 chip_no, int32 enable)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret;
    int32 val;
    int32 bit_len1, bit_len2, bit_len3, bit_len4, bit_len5, bit_len6;

    bit_len1 = epld_item_get_width(0, EPLD_SFP_DISABLE1);
    bit_len2 = epld_item_get_width(0, EPLD_SFP_DISABLE2);
    bit_len3 = epld_item_get_width(0, EPLD_SFP_DISABLE3);
    bit_len4 = epld_item_get_width(0, EPLD_SFP_DISABLE4);
    bit_len5 = epld_item_get_width(0, EPLD_SFP_DISABLE5);
    bit_len6 = epld_item_get_width(0, EPLD_SFP_DISABLE6);

    if(bit_len1==0)
    {
        return 0;
    }
    
    if(bit_len1 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE1, &val);
        if(!enable)
        {
            val |= 0x1<<chip_no;
        }
        else
        {
            val &= ~(0x1<<chip_no);
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE1, val);
    }
    else if(bit_len1+bit_len2 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE2, &val);
        if(!enable)
        {
            val |= 0x1<<(chip_no-bit_len1);
        }
        else
        {
            val &= ~(0x1<<(chip_no-bit_len1));
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE2, val);
    }
    else if(bit_len1+bit_len2+bit_len3 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE3, &val);
        if(!enable)
        {
            val |= 0x1<<(chip_no-bit_len1-bit_len2);
        }
        else
        {
            val &= ~(0x1<<(chip_no-bit_len1-bit_len2));
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE3, val);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE4, &val);
        if(!enable)
        {
            val |= 0x1<<(chip_no-bit_len1-bit_len2-bit_len3);
        }
        else
        {
            val &= ~(0x1<<(chip_no-bit_len1-bit_len2-bit_len3));
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE4, val);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE5, &val);
        if(!enable)
        {
            val |= 0x1<<(chip_no-bit_len1-bit_len2-bit_len3-bit_len4);
        }
        else
        {
            val &= ~(0x1<<(chip_no-bit_len1-bit_len2-bit_len3-bit_len4));
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE5, val);
    }
    else if(bit_len1+bit_len2+bit_len3+bit_len4+bit_len5+bit_len6 > chip_no)
    {
        ret = epld_item_read(0, EPLD_SFP_DISABLE6, &val);
        if(!enable)
        {
            val |= 0x1<<(chip_no-bit_len1-bit_len2-bit_len3-bit_len4-bit_len5);
        }
        else
        {
            val &= ~(0x1<<(chip_no-bit_len1-bit_len2-bit_len3-bit_len4-bit_len5));
        }
        ret = epld_item_write(0, EPLD_SFP_DISABLE6, val);
    }
    else
    {
        DRV_LOG_ERR("Set fiber enable: chip_no %d is out of range!", chip_no);
        ret = -1;
    }
    DRV_LOG_DEBUG(DRV_EPLD, " Set fiber enable: chip_no [%d] val[0x%x]\n",chip_no,val);
    return ret;
#endif
}

/* Added by liuht for bug41782, 2016-11-23 */
/* get link status from epld, used for 100G mode */
/* Bug44674, walkaround EPLD4.0 bug*/
#ifndef _CTC_LOONGSON_
static int32 g_fault_status[4]={0,0,0,0};
#endif
int32 
epld_get_txlink_status(uint32 port_no, int *status)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 val = 0;
    int32 bit_len1;
    int32 fault_status;

    /* port_no start from 1, and 0 means invalid */
    if(port_no == 0)
        return 0;

    port_no = port_no - 1;
    //bit_len1 = epld_item_get_width(0, EPLD_TXLNK_STAT);
    /*Force bit len as 4, GG only has  4 100G port. Temply code.*/
    bit_len1 = 4;
    if(bit_len1 > port_no)
    {
        ret = epld_item_read(0, EPLD_TXLNK_STAT, &val);
        
        fault_status = ((val>>port_no) &0x1);
        /*When fault status keep same in two cycle, use it as real status.*/
        if(fault_status == g_fault_status[port_no])
        {        
            if(fault_status)
                *status = 0;
            else
                *status = 1;
        }
        DRV_LOG_DEBUG(DRV_EPLD, " Get txlink status: CG_no [%d], val[0x%x], old[%d], up[%d]\n",
                                                port_no,val, g_fault_status[port_no], *status);
        g_fault_status[port_no] = fault_status;
    }    
    return ret;    
#endif
}

/* Added by liuht for bug41782, 2016-11-23 */
/* set port split flag to epld, used for 100G mode */
int32
epld_set_port_split_flag(uint32 port_no, int split)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 val = 0;
    int32 bit_len1 = 0;

    /* port_no start from 1, and 0 means invalid */
    if(port_no == 0)
        return 0;

    port_no = port_no - 1;
    bit_len1 = epld_item_get_width(0, EPLD_PORT_SPLIT);
    if(bit_len1 > port_no)
    {
        ret = epld_item_read(0, EPLD_PORT_SPLIT, &val);
        if(split)
        {
            val |= 0x1<<port_no;
        }
        else
        {
            val &= ~(0x1<<port_no);
        }
        ret = epld_item_write(0, EPLD_PORT_SPLIT, val);
    }
    DRV_LOG_DEBUG(DRV_EPLD, " Set split flag: chip_no [%d] val[0x%x]\n",port_no,val);
    return ret;
#endif

}

/*shutdown: 1 mean shutdown, 0 mean undo shutdown*/
int32 
epld_set_psu_shutdown(uint8 psu_idx, uint8 epld_index, uint8 shutdown)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 status;
    int32 ret = 0;

    ret = epld_item_read(epld_index, EPLD_PSU_SHUTDOWN, &status);
    if(shutdown)
    {
        status |= 0x1<<psu_idx;
    }
    else
    {
        status &= 0x1<<psu_idx;
    }
    ret += epld_item_write(epld_index, EPLD_PSU_SHUTDOWN, status);

    return ret;
#endif
}

/*fan status: 1 mean fail, 0 mean ok;
  psu type: 1 mean AC, 0 mean DC
  psu status: 1 mean ok, 0 mean fail
  psu absent: 1 mean absent, 0 mean present
  psu shutdown: 1 mean shutdown, 0 mean enable*/
int32 
epld_get_psu_status(uint8 psu_idx, void * p_psu_status, uint8 epld_index, uint8 psu_type)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 status;
    int32 ret = 0;
    epld_psu_status_t* p_epld_psu_status = NULL;     

    p_epld_psu_status = (epld_psu_status_t*)p_psu_status;

    DRV_CTC_CHK_PTR(p_epld_psu_status);
    if(PSU_SHOW_FULL_STATUS == psu_type)
    {
         /* deleted by liuht for bug 24525,2013-10-24 */    
#if 0    
        ret += epld_item_read(epld_index, EPLD_PSU_FAN_STATUS, &status);
        p_epld_psu_status->psu_fan = (status>>psu_idx)&0x1;
#endif		
        ret += epld_item_read(epld_index, EPLD_PSU_TYPE, &status);
        p_epld_psu_status->psu_mode = (status>>psu_idx)&0x1;
        if(psu_idx < EPLD_SYS_PSU_MAX)
        {
            /* modified by liuht for bug 24525,to adapt e350 system. 2013-10-24 */ 
            ret += epld_item_read(epld_index, EPLD_PSU_WORK_STATUS, &status);	
	     #if 0
	     p_epld_psu_status->psu_work_status = (status>>psu_idx)&0x1;
	     #endif	 
            p_epld_psu_status->psu_work_status = ((status>>(EPLD_SYS_PSU_MAX - psu_idx - 1))&0x1)^0x1;
        }
        else
        {
            /* third power: POE PSU */
            ret += epld_item_read(0, EPLD_PSU_POE_WORK_STATUS, &status);
            p_epld_psu_status->psu_work_status = status & 0x1;
        }
	  /* modified by liuht for bug 24525,2013-10-24 */
        ret += epld_item_read(epld_index, EPLD_PSU_PRESENT_STATUS, &status);
        p_epld_psu_status->psu_absent = (status>>(EPLD_SYS_PSU_MAX - psu_idx - 1))&0x1;
        ret += epld_item_read(epld_index, EPLD_PSU_ALERT_STATUS, &status);
        p_epld_psu_status->psu_alert = ((status>>(EPLD_SYS_PSU_MAX - psu_idx - 1))&0x1)^0x1;	

         /* deleted by liuht for bug 24525,2013-10-24 */    		
#if 0		
        ret += epld_item_read(epld_index, EPLD_PSU_SHUTDOWN, &status);
        p_epld_psu_status->psu_shutdown = (status>>psu_idx)&0x1;
#endif		
    }
    else if(PSU_SHOW_GOOD_SIG == psu_type)
    {        
        if(psu_idx < EPLD_SYS_PSU_MAX)
        {
            /* modified by liuht for bug 24525,2013-10-24 */ 
            ret += epld_item_read(epld_index, EPLD_PSU_WORK_STATUS, &status);
            p_epld_psu_status->psu_work_status = ((status>>(EPLD_SYS_PSU_MAX - psu_idx - 1))&0x1)^0x1;
        }
        else
        {
            /* third power: POE PSU */
            ret += epld_item_read(0, EPLD_PSU_POE_WORK_STATUS, &status);
            p_epld_psu_status->psu_work_status = status & 0x1;
        }
    }
    else if(PSU_SHOW_HF == psu_type)
    {        
        if(psu_idx == 0)
        {
            /* status is 1, means led is off, psu is absent */
            ret += epld_item_read(epld_index, EPLD_PSU1_LED, &status);
            p_epld_psu_status->psu_absent = status?1:0;
            p_epld_psu_status->psu_work_status = status?PSU_FAIL:PSU_OK;
        }
        else if(psu_idx == 1)
        {
            /* status is 1, means led is off, psu is absent */
            ret += epld_item_read(epld_index, EPLD_PSU2_LED, &status);
            p_epld_psu_status->psu_absent = status?1:0;            
            p_epld_psu_status->psu_work_status = status?PSU_FAIL:PSU_OK;
        }
        p_epld_psu_status->psu_mode = PSU_UNKNOWN;
    }    
    return ret;
#endif
}

int32
epld_get_poe_psu_status(uint8* psu_status)
{
#ifdef _CTC_LOONGSON_
    *psu_status = 0;
    return 0;
#else
    int32 status;
    int32 ret = 0;

    ret += epld_item_read(0, EPLD_PSU_POE_WORK_STATUS, &status);
    *psu_status = status & 0x1;

    return ret;
#endif
}

int32
epld_get_poe_present(uint8* present)
{
#ifdef _CTC_LOONGSON_
    *present = 0;
    return 0;
#else
    int32 status;
    int32 ret = 0;

    ret += epld_item_read(0, EPLD_POE_PRESENT, &status);
    *present = status & 0x1;

    return ret;
#endif
}

/* type detail: see fan_module_type_e. */
int32
epld_get_fan_type(uint8 fan_module, uint8* type, uint8 epld_index)
{
#ifdef _CTC_LOONGSON_
    *type = 0;
    return 0;
#else
    int32 value;
    int32 ret = 0;
    DRV_CTC_CHK_PTR(type);
    ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_TYPE, &value);
    *type = (uint8)value;
    return ret;
#endif
}

/*present value : 1 mean present, 0 mean absent.*/
int32
epld_get_fan_present(uint8 fan_module, uint8* present, uint8 epld_index)
{
#ifdef _CTC_LOONGSON_
    *present = 0;
    return 0;
#else
    int32 value;
    int32 ret = 0;
    
    DRV_CTC_CHK_PTR(present);
    ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_PRESENT, &value);
    /* in hw spec, 0 mean present, so here need do translate.*/
    if(value == 0)
        *present = 1;
    else
        *present = 0;
    return ret;
#endif
}

/*Input value power_ctrl: 1 mean power on, 0 mean power off.*/
int32
epld_set_fan_power(uint8 fan_module, uint8 power_ctrl, uint8 epld_index)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;

    ret = epld_item_write(epld_index, EPLD_FAN_MODULE0_POWER_CTRL, power_ctrl);
    return ret;
#endif
}

/*status: 1 mean ok, 0 mean fail. each bit show 1 fan status*/
int32
epld_get_fan_status(uint8 fan_module, uint8* status, uint8 epld_index)
{
#ifdef _CTC_LOONGSON_
    *status = 0;
    return 0;
#else
    int32 value;
    int32 ret = 0;
    ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_STATUS, &value);
    *status = (uint8)value;
    return ret;    
#endif 
}

/* Added by liuht for bug26671, 2014-02-14 */
int32
epld_set_fan_speed(uint8 fan_module, uint8 speed, uint8 epld_index, uint8 reg)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    ret = epld_item_write(epld_index, EPLD_FAN_MODULE0_SET_SPEED, speed);
    return ret; 
#endif
}

/* Added by liuht for bug26671, 2014-02-14 */
int32
epld_get_fan_speed(uint8 fan_module, uint8* speed, uint8 epld_index, uint8 reg)
{
#ifdef _CTC_LOONGSON_
    *speed = 0;
    return 0;
#else
    int32 ret = 0;
    int32 value = 0;	
    if(0x27 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED1_LOW, &value);
    }
    else if(0x28 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED1_HIGH, &value);
    }
    else if(0x29 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED2_LOW, &value);
    }
    else if(0x30 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED2_HIGH, &value);
    }
    else if(0x31 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED3_LOW, &value);
    }
    else if (0x32 == reg)
    {
        ret = epld_item_read(epld_index, EPLD_FAN_MODULE0_GET_SPEED3_HIGH, &value);
    }	
    else
    {
        ret = epld_item_read(epld_index, EPLD_FAN_STATUS, &value);
    }		
    *speed = (uint8)value;	
    return ret;
#endif
}

uintptr_t 
epld_localbus_addr_get(void)
{
#ifdef _CTC_LOONGSON_
    return -1;
#else
    uint8 * epld_base = NULL;
    int32 mem_fd = 0;
        
    mem_fd = sal_open("/dev/mem", O_RDWR | O_SYNC);
    if(mem_fd < 0) {
        DRV_LOG_ERR("epld_localbus_addr_get open /dev/mem failed");
        return -1;
    } 
//#ifdef _CTC_OCTEON_CN50XX_   // TODO: need to adapt P1020 CPU
//#define CTC_EPLD_LOCAL_BUS_PHY_ADDR 0x17000000
//#else
//#define CTC_EPLD_LOCAL_BUS_PHY_ADDR 0x60000000
//#endif
#ifdef _CTC_NID_
#define CTC_EPLD_LOCAL_BUS_PHY_ADDR 0xfe800000   /* modified by qicx for NID */
#else
//TODO_LIUHT
//#define CTC_EPLD_LOCAL_BUS_PHY_ADDR 0xd0000000   /* modified by qicx for EPLD localbus addr of BlueFox */
#define CTC_EPLD_LOCAL_BUS_PHY_ADDR 0xffb00000
#endif
    epld_base = (uint8 *)mmap(NULL, 3*1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, CTC_EPLD_LOCAL_BUS_PHY_ADDR);/* IO for EPLD */
    if(epld_base < 0)
    {
        DRV_LOG_ERR("epld_localbus_addr_get mmap epld error");
        close(mem_fd);
        return -1;
    }
    return (uintptr_t)epld_base;
#endif
}

/* 
 * Used for GreatBelt system 
 * Return value: 
 *   1: All slot cards are present and with same type.
 *   0: the opposite status.
 */
int32
epld_get_slot_card_status(uint8 epld_idx, uint8* slot_card_type)
{
#ifdef _CTC_LOONGSON_
    *slot_card_type = 0;
    return 0;
#else
    int32 value0 = 0;
    int32 value1 = 0;
    int32 value2 = 0;
    int32 ret = 0;
    
    ret = epld_item_read(epld_idx, EPLD_SLOT0_TYPE, &value0);
    ret += epld_item_read(epld_idx, EPLD_SLOT1_TYPE, &value1);
    ret += epld_item_read(epld_idx, EPLD_SLOT2_TYPE, &value2);
    if(value0 != GLB_SLOT_CARD_NULL)
    {
        *slot_card_type = value0;
    }
    else if(value1 != GLB_SLOT_CARD_NULL)
    {
        *slot_card_type = value1;
    }
    else
    {
        *slot_card_type = value2;
    }
    return 1;
    if((0 == ret)                           /* epld read successfully */
        && (value0 == value1) 
        && (value1 == value2)               /* all three slot cards are the same type */
        && (GLB_SLOT_CARD_NULL != value0)   /* all three slot cards are present */
        && (GLB_SLOT_CARD_2SFPP_SFI != value0)  /* all three slot cards are not SFP+_SFI card */
        )
    {
        *slot_card_type = value0;   /* 16T card or 8SFP card, CANNOT be SFP+_SFI card */
        return 1;
    }
    else if((0 == ret)                      /* epld read successfully */
        && (value0 == value1)               /* the first and second slot cards are the same type */
        && (GLB_SLOT_CARD_NULL == value2)   /* the third slot card is absent */
        && (GLB_SLOT_CARD_2SFPP_SFI == value1))  /* the first and second slot cards are SFP+_SFI card */
    {
        *slot_card_type = value1;   /* SFP+_SFI card only */
        return 1;
    }
    else if((0 == ret)                      /* epld read successfully */
        && (value0 == value1) && (value1 == value2)
        && (GLB_SLOT_CARD_NULL == value0))  /* all slot cards are absent */
    {
        *slot_card_type = GLB_SLOT_CARD_NULL;
        return 1;
    }
    else
    {
        *slot_card_type = GLB_SLOT_CARD_UNSUPPORT;
        return 0;
    }
#endif
}

/*****************************************************************************
 * Name         :   epld_get_info
 * Purpose      :   get the epld epld info for specify board
 * Input         :   board_type
 *                 :   epld_version 
 * Output       :   
 * Return       :   epld epld info
 * Note         :   
 *****************************************************************************/
epld_info_t* epld_get_info(glb_board_type_t* board_type, uint8 epld_version)
{
#ifndef _CTC_LOONGSON_
    return &g_epld_info[0];
#else
    return NULL;
#endif
}


/*****************************************************************************
 * Name         :   epld_set_reg_desc
 * Purpose      :   set epld_dev's reg_desc's value 
 * Input        :   reg_offset          
 *              :   start_bit
 *              :   endbit
 *              :   item_bitwidth
 * Output       :   reg_desc
 * Return       :   Success 
 * Note         :
 *****************************************************************************/
int32 epld_set_reg_desc(struct epld_reg_s* reg_desc, uint8 reg_offset, int32 start_bit,
                              int32 end_bit, int32 item_bitwidth)
{
    if( NULL == reg_desc )
    {
        return -1;
    }
    
    reg_desc->reg_offset = reg_offset;
    reg_desc->start_bit = start_bit;
    reg_desc->end_bit = end_bit;
    reg_desc->item_bitwidth = item_bitwidth;
    reg_desc->reg_valid = 0x1;

    return 0;
}

/* add 'is_gbdemo' flag for some features that gbdemo board owns only, added by qicx, 2013-07-08 */
int32
epld_cfg_phy_interrupt_mask(int32 mask_en, uint8 is_gbdemo)
{    
#ifdef _CTC_LOONGSON_
    return 0;
#else
    if(mask_en)
    {
        if (is_gbdemo)
        {
            epld_item_write(0, EPLD_SLOT0_PHY_INT_MASK, 0xff);
            epld_item_write(0, EPLD_SLOT1_PHY_INT_MASK, 0xff);
            epld_item_write(0, EPLD_SLOT2_PHY_INT_MASK, 0xff);
        }
        else
        {
            epld_item_write(0, EPLD_PHY_INT_MASK1, 0xff);
            epld_item_write(0, EPLD_PHY_INT_MASK2, 0xff);
        }
    }
    else
    {
        if (is_gbdemo)
        {
            epld_item_write(0, EPLD_SLOT0_PHY_INT_MASK, 0x0);
            epld_item_write(0, EPLD_SLOT1_PHY_INT_MASK, 0x0);
            epld_item_write(0, EPLD_SLOT2_PHY_INT_MASK, 0x0);
        }
        else
        {
            epld_item_write(0, EPLD_PHY_INT_MASK1, 0x0);
            epld_item_write(0, EPLD_PHY_INT_MASK2, 0x0);
        }
    }

    return 0;
#endif
}

/* Added by liuht for bug 27657, 2014-03-25 */
int32
epld_cfg_power_interrupt_mask(int32 mask_en)
{    
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret;
    
    if(mask_en)
    {
        ret = epld_item_write(0, EPLD_POWER_INTR_MASK, 0x1);
    }
    else
    {
        ret = epld_item_write(0, EPLD_POWER_INTR_MASK, 0x0);
    }

    return ret;
#endif
}

int32
epld_cfg_poe_interrupt_mask(int32 mask_en)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret;
    
    if(mask_en)
    {
        ret = epld_item_write(0, EPLD_POE_INT_MASK, 0x1);
    }
    else
    {
        ret = epld_item_write(0, EPLD_POE_INT_MASK, 0x0);
    }

    return ret;
#endif
}
int32
epld_cfg_dpll_interrupt_mask(int32 mask_en)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    if(mask_en)
    {
        epld_item_write(0, EPLD_DPLL_INT_MASK, 0x1);
    }
    else
    {
        epld_item_write(0, EPLD_DPLL_INT_MASK, 0x0);
    }

    return 0;
#endif
}
/* This is a switch to select channel to XGMac or 1G phy. bug 16317 jqiu 2011-09-20. support vitesse 10G phy*/
int32
epld_cfg_phy_switch_smi(uint8 idx, uint8 chnl_id, uint8 enable)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 value;
    uint32 mask;
    epld_item_read(idx, EPLD_XGPHY_SMI_SWITCH, &value);
    if(enable)
    {
        value |= (1<<chnl_id);
    }
    else
    {
        mask = ~(1<<chnl_id);
        value &= mask;
    }
    epld_item_write(idx, EPLD_XGPHY_SMI_SWITCH, value);
    return 0;
#endif
}

/* Added by liuht for bug27036, 2014-02-27 */
/*****************************************************************************
 * Name         :   epld_check_power_reboot
 * Purpose      :   check if power reboot
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   0 : power reboot 1 : no power reboot
 * Note           :   N/A
 *****************************************************************************/
int32
epld_check_power_reboot()
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 value;
	
    epld_item_read(0, EPLD_REBOOT_POWER_RST, &value);
    if(0x55 != value)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
#endif
}

/* Added by liuht for bug27036, 2014-02-27 */
/*****************************************************************************
 * Name         :   epld_check_manual_reboot
 * Purpose      :   check if manual reboot
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   0 : manual reboot 1 : no manual reboot
 * Note           :   N/A
 *****************************************************************************/
int32
epld_check_manual_reboot()
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 value;
    ret = epld_item_read(0, EPLD_REBOOT_MANUAL_OTHER_RST, &value);
    if(ret)
    {
        DRV_LOG_ERR("check epld manual reboot fail!\n");
        return RESULT_ERROR;
    }
    if(0xaa == value)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
#endif
}

/* Added by liuht for bug27036, 2014-02-27 */
/*****************************************************************************
 * Name         :   epld_check_other_reboot
 * Purpose      :   check if other reboot
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   0 : other reboot 1 : no other reboot
 * Note           :   N/A
 *****************************************************************************/
int32
epld_check_other_reboot()
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    int32 value;
    ret = epld_item_read(0, EPLD_REBOOT_MANUAL_OTHER_RST, &value);
    if(ret)
    {
        DRV_LOG_ERR("check epld other reboot fail!\n");
        return RESULT_ERROR;
    }	
    if(0x5a == value)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
#endif
}

/* Added by tongzb for bug 40719, 2016-09-12 */
/*****************************************************************************
 * Name         :   epld_get_reboot_info
 * Purpose      :   get reboot info in epld
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   reboot info
 * Note           :   N/A
 *****************************************************************************/
int32
epld_get_reboot_info(void)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 value;
    epld_item_read(0, EPLD_REBOOT_POWER_RST, &value);
    return value;
#endif
}
/* Added by tongzb for bug 40719, 2016-09-12 */
/*****************************************************************************
 * Name         :   epld_reset_reboot_info
 * Purpose      :   reset reboot info in epld
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   0
 * Note           :   N/A
 *****************************************************************************/
int32
epld_reset_reboot_info(void)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0; 
    ret = epld_item_write(0, EPLD_REBOOT_POWER_RST, 0xfe);
    if(ret)
    {
        DRV_LOG_ERR("reset epld reboot info fail!\n");
        return RESULT_ERROR;
    }
    return RESULT_OK;
#endif
}

/* Added by liuht for bug27036, 2014-02-27 */
/*****************************************************************************
 * Name         :   epld_reset_reboot_flags
 * Purpose      :   reset reboot flag
 * Input          :   N/A
 * Output        :   N/A
 * Return        :   0
 * Note           :   N/A
 *****************************************************************************/
int32
epld_reset_reboot_flags()
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    ret = epld_item_write(0, EPLD_REBOOT_POWER_RST, 0x55);
    ret |= epld_item_write(0, EPLD_REBOOT_MANUAL_OTHER_RST, 0x5a);	
    if(ret)
    {
        DRV_LOG_ERR("reset epld reboot flags fail!\n");
        return RESULT_ERROR;
    }	
    return RESULT_OK;
#endif
}

/* set linecard power on ctrl. lc_id start from 0, and 0xff means all linecards.*/
int32
epld_cfg_lc_power_on(uint8 idx, uint8 lc_id, uint8 enable)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 value;
    if(lc_id == 0xff)
    {
        if(enable)
        {
            value = 0xff;            
        }
        else
        {
            value = 0x0;
        }
        epld_item_write(idx, EPLD_LC_PWR_ON_SLOT_A, value);
        epld_item_write(idx, EPLD_LC_PWR_ON_SLOT_B, value);
    }
    else
    {
        if(lc_id < 8)
        {
            epld_item_read(idx, EPLD_LC_PWR_ON_SLOT_A, &value);
            if(enable)
            {
                value |= (1<<lc_id);
            }
            else
            {
                value &= ~(1<<lc_id);
            }
            epld_item_write(idx, EPLD_LC_PWR_ON_SLOT_A, value);
        }
        else
        {
            epld_item_read(idx, EPLD_LC_PWR_ON_SLOT_B, &value);
            if(enable)
            {
                value |= (1<<(lc_id-8));
            }
            else
            {
                value &= ~(1<<(lc_id-8));
            }
            epld_item_write(idx, EPLD_LC_PWR_ON_SLOT_B, value);
        }
    }
    return 0;
#endif
}
  
int32 
epld_read(uint8 idx, epld_para_t *ppara)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    epld_access_t epld_access;
    int32 ret = 0;
    
    if(NULL == g_epld_hdl[idx] || NULL == ppara)
    {
        return -1;
    }
    epld_access.addr = ppara->addr;
        
    ret = g_epld_hdl[idx]->read(g_epld_hdl[idx], &epld_access);
    ppara->val = epld_access.val;
    DRV_LOG_DEBUG(DRV_EPLD, "epld read : addr: 0x%x val:0x%x\n",ppara->addr,epld_access.val);
    return ret; 
#endif
}

int32 
epld_write(uint8 idx, epld_para_t *ppara)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    epld_access_t epld_access;

    if(NULL == g_epld_hdl[idx] || NULL == ppara)
    {
        return -1;
    }
    DRV_LOG_DEBUG(DRV_EPLD, "epld write : value: 0x%x addr:0x%x\n",ppara->val,ppara->addr);
    epld_access.addr = ppara->addr;
    epld_access.val = ppara->val;
    
    return g_epld_hdl[idx]->write(g_epld_hdl[idx], &epld_access);
#endif
}

int32 
epld_item_read(uint8 idx, uint32 op_reg, int32* value)
{    
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret;
    int32 val;
    if((NULL == g_epld_hdl[idx]) || (op_reg > EPLD_REG_MAX_NUM) || (NULL == value))
    {
        return -1;
    }
    
    ret = g_epld_hdl[idx]->item_read(g_epld_hdl[idx], op_reg, &val);
    *value = val;

    return ret;
#endif
}

int32
epld_item_write(uint8 idx, uint32 op_reg, int32 value)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    if(NULL == g_epld_hdl[idx] || op_reg > EPLD_REG_MAX_NUM)
    {
        return -1;
    }    
    
    return g_epld_hdl[idx]->item_write(g_epld_hdl[idx], op_reg, value);
#endif
}

int32
epld_item_write_mask(uint8 idx, uint32 op_reg, int32 value, int32 mask)
{
    int32 ret = RESULT_OK;
    int32 val;
    ret = epld_item_read(idx, op_reg, &val);
    value = (value & mask )|(val & ~mask);
    ret += epld_item_write(idx, op_reg, value);
    return ret;
}

/*Return value: the width of the item.*/
int32
epld_item_get_width(uint8 idx, uint32 op_reg)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    if(NULL == g_epld_hdl[idx] || op_reg > EPLD_REG_MAX_NUM)
    {
        return 0;
    }    
    
    return g_epld_hdl[idx]->get_item_width(g_epld_hdl[idx], op_reg);
#endif
}

void
epld_update_reg_info(epld_info_t * p_epld_info)
{
#ifdef _CTC_LOONGSON_
    return;
#else
    sal_memcpy((int8 *)g_epld_hdl[0]->info, (int8 *)p_epld_info, sizeof(epld_info_t));
#endif
}

int32
_epld_bootup_diag_test(uint32 idx, uint32 op_reg, epld_diag_result_t* diag_result)
{
    int32 var = 0;
    int32 test_var = 0x1;
    int32 org_val = 0;
    int32 ret1 = RESULT_OK, ret2 = RESULT_OK;    

    DRV_CTC_CHK_PTR(diag_result);

    ret1 = epld_item_read(idx, op_reg, &org_val);
    if(ret1)
    {
        diag_result->rd_fail = 1;
        ret2 = RESULT_ERROR;
    }

    ret1 = epld_item_write(idx, op_reg, test_var);   
    if(ret1)
    {
        diag_result->wr_fail = 1;
        ret2 = RESULT_ERROR;
    }
   
    ret1 = epld_item_read(idx, op_reg, &var);
    if(ret1)
    {
        diag_result->rd_fail = 1;
        ret2 = RESULT_ERROR;
    }
    
    if(test_var != var)
    {
        diag_result->cmp_fail = 1;
        ret2 = RESULT_ERROR;
    }
    ret1 = epld_item_write(idx, op_reg, org_val);
    if(ret1)
    {
        diag_result->wr_fail = 1;
        ret2 = RESULT_ERROR;
    }

    return ret2;
}

int32
epld_bootup_diag_and_report(uint32 idx)
{
#ifndef _GLB_UML_SYSTEM_  
    epld_diag_result_t diag_result;
    int32 result = 0;

    sal_memset(&diag_result, 0, sizeof(epld_diag_result_t));
    result = _epld_bootup_diag_test(idx, EPLD_TEST, &diag_result);
    if (result != RESULT_OK)
    {
        if(diag_result.rd_fail)
        {
            DRV_DIAG_LOG("EPLD diag read fail!\n");    
        }
        if(diag_result.wr_fail)
        {
            DRV_DIAG_LOG("EPLD diag write fail!\n");    
        }
        if(diag_result.cmp_fail)
        {
            DRV_DIAG_LOG("EPLD diag cmp fail!\n");    
        }    
        DRV_DIAG_LOG("EPLD diag fail!\n");
        return RESULT_OK;
    }
    else
    {
        DRV_DIAG_LOG("EPLD diag pass!\n");
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}

/*****************************************************************************
 * Name         :   epld_update
 * Purpose      :   Update epld image
 * Input        :   the update tar file name          
 * Output       :   
 * Return       :   Success 
 * Note         :
 *****************************************************************************/
int32 epld_update(char* src_file, uint8 epld_type)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    char cmd[M_FULLPATH_MAX_LEN];

    if(epld_type == GLB_EPLD_TYPE_VME)
    {
        /* extract gzipped src_file*/
        sal_sprintf(cmd, "tar zxf %s\n", src_file);
#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
        system(cmd);
#else
        ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)cmd);
#endif
        /* update the epld */
        sal_sprintf(cmd, "/vme/vme /vme/algo.vme vme/data.vme\n");
#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
        system(cmd);
#else
        ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)cmd);
#endif
    }
    else if(epld_type == GLB_EPLD_TYPE_JAM)
    {
        /* extract gzipped src_file*/
        sal_sprintf(cmd, "tar zxf %s\n", src_file);
#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
        system(cmd);
#else
        ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)cmd);
#endif
        /* update the epld */
        sal_sprintf(cmd, "/jam/jam -aPROGRAM -dDO_REAL_TIME_ISP=1 -dDO_BYPASS_UFM=1 jam/data.jam \n");
#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
        system(cmd);
#else
        ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)cmd);
#endif
    }
    else
    {
        return -1;
    }

    return 0;
#endif
}

/*****************************************************************************
 * Name         :   epld_init
 * Purpose      :   init epld dev handler
 * Input        :   idx (the epld index of all the boards epld version, corresponding to enum epld_dev)          
 * Output       :   
 * Return       :   Success 
 * Note         :
 *****************************************************************************/
int32 
epld_init(epld_info_t* p_epld_info)
{        
#ifdef _CTC_LOONGSON_
    return 0;
#else
    static int i=-1;
    int32 ret = 0;

    DRV_CTC_CHK_PTR(p_epld_info);
    
    g_epld_hdl[++i] = epld_dev_register((void *)p_epld_info);
    if(NULL == g_epld_hdl[i])
    {
        return -1;
    }

    if ((p_epld_info->diag_level >= p_epld_info->req_diag_level)
    && (p_epld_info->req_diag_level >= E_DIAG_LEVEL_MINIMAL))
    {
        ret = epld_bootup_diag_and_report(i);
        if (ret != 0)
        {
            return -1;
        }
    }

    return 0;
#endif
}

#define ACCESS_TIMEOUT 0x100000

#define CHIP_ACCESS_OK              0
#define CHIP_READ_FAIL              -1
#define CHIP_WRITE_FAIL             -2
#define CHIP_ACCESS_TIMEOUT             -3
#define CHIP_ACK_ERRO             -4
#define CHIP_VALUE_NOT_EQUAL             -5
#define CHIP_BUS_BUSY             -6

#if 0
int32
epld_shim_read(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 *value)
{
    uint32 ack;
    int32 ret = 0;
    epld_para_t epld_para;
    uint32 timeout;
    uint8 temp[4];
    DRV_CTC_CHK_PTR(value);     
    
    sal_memset(&epld_para, 0, sizeof(epld_para_t));

/*************************************************
    mw.b 0xd0020010 chipId(1-5)
    mw.b 0xd0020011 0x1
    mw.b (0xd0200003+rdAddr) 0x00
    md.b 0xd0020019 1(ackCnt should increase)
    md.b 0xd0020012 1(should be 0)
    md.b 0xd0020014 1(rdData[7:0])
    md.b 0xd0020015 1(rdData[15:8])
    md.b 0xd0020016 1(rdData[23:16])
    md.b 0xd0020017 1(rdData[31:24])
 
**************************************************/    
    epld_para.addr = 0x20019;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    ack = epld_para.val;
    
    epld_para.addr = 0x20010;
    epld_para.val = fpga_idx;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x20011;
    epld_para.val = 0x1;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200003 + addr;
    epld_para.val = 0x0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
#if 0    
    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20019 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((!(ack == epld_para.val))&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }
#endif
    epld_para.addr = 0x20012;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_VALUE_NOT_ZERO;  
    }

    epld_para.addr = 0x20014;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[0] = epld_para.val;
    
    epld_para.addr = 0x20015;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[1] = epld_para.val;
    
    epld_para.addr = 0x20016;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[2] = epld_para.val;
    
    epld_para.addr = 0x20017;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[3] = epld_para.val;
    
    *value = (temp[3]<<24)|(temp[2]<<16)|(temp[1] <<8)|temp[0];
    DRV_LOG_DEBUG(DRV_EPLD, "epld shim read : addr: 0x%x value:0x%x\n",addr,*value);
    return ret;
} 

int32
epld_shim_write(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 value)
{
    int32 ret = 0;
    epld_para_t epld_para;
    uint32 timeout;
    uint8 temp[4];
    uint32 ack;
/*************************************************    
    mw.b 0xd0020010 chipId(1-5)
    mw.b 0xd0020011 0x0
    mw.b (0xd0200000+wrAddr) wrData[7:0]
    mw.b (0xd0200001+wrAddr) wrData[15:8]
    mw.b (0xd0200002+wrAddr) wrData[23:16]
    mw.b (0xd0200003+wrAddr) wrData[31:24]
    md.b 0xd0020019 1(ackCnt should increase)
    md.b 0xd0020012 1(should be 0)     
**************************************************/        
    sal_memset(&epld_para, 0, sizeof(epld_para_t));

    epld_para.addr = 0x20019;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    ack = epld_para.val;
    
    epld_para.addr = 0x20010;
    epld_para.val = fpga_idx;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x20011;
    epld_para.val = 0x0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x200000 + addr;
    epld_para.val = value&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x200001 + addr;
    epld_para.val = (value>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200002 + addr;
    epld_para.val = (value>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200003 + addr;
    epld_para.val = (value>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
#if 0
    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20019 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((!(ack == epld_para.val))&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }
#endif
    epld_para.addr = 0x20012;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_VALUE_NOT_ZERO;  
    }
    return ret;
} 
#else
int32
epld_shim_read(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 *value)
{
//    uint32 ack;
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    epld_para_t epld_para;
    uint32 timeout;
    uint8 temp[4];
    DRV_CTC_CHK_PTR(value);     
    
    sal_memset(&epld_para, 0, sizeof(epld_para_t));

/*************************************************
    mw.b 0xd0020010            chipId(1-5)                //select fpga chip
    mw.b 0xd0020011            0x1                        //read type
    mw.b (0xd0200003+rdAddr)   0x00                       //read trigger
    md.b 0xd0020013 1          (accessBusy, should be 0)  //access busy status
    if (accessBusy == 1) read again

    md.b 0xd0020019 1          (read ackCnt >0)
    if(ackCnt ==0) read again

    md.b 0xd0020012 1          (should be 0)              //ack error if not be 0

    md.b 0xd0020014 1(rdData[7:0])
    md.b 0xd0020015 1(rdData[15:8])
    md.b 0xd0020016 1(rdData[23:16])
    md.b 0xd0020017 1(rdData[31:24])

    note:
    rdAddr:0 -> 4 -> 8 .... step is 4
 
**************************************************/        
    epld_para.addr = 0x20010;
    epld_para.val = fpga_idx;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x20011;
    epld_para.val = 0x1;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200003 + addr;
    epld_para.val = 0x0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

  
    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20013 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((1 == epld_para.val)&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_BUS_BUSY;
    }
#if 0   
    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20019 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((0 == epld_para.val)&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }


    epld_para.addr = 0x20012;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_ACK_ERRO;  
    }
#endif
    epld_para.addr = 0x20014;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[0] = epld_para.val;
    
    epld_para.addr = 0x20015;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[1] = epld_para.val;
    
    epld_para.addr = 0x20016;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[2] = epld_para.val;
    
    epld_para.addr = 0x20017;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[3] = epld_para.val;
    
    *value = (temp[3]<<24)|(temp[2]<<16)|(temp[1] <<8)|temp[0];
    DRV_LOG_DEBUG(DRV_EPLD, "epld shim read : addr: 0x%x value:0x%x\n",addr,*value);
    return ret;
#endif
} 

int32
epld_shim_write(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 value)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    epld_para_t epld_para;
    uint32 timeout;
//    uint8 temp[4];
//    uint32 ack;
/*************************************************    
    mw.b 0xd0020010            chipId(1-5)                //select fpga chip
    mw.b 0xd0020011            0x0                        //write type
    mw.b (0xd0200000+wrAddr)   wrData[7:0]
    mw.b (0xd0200001+wrAddr)   wrData[15:8]
    mw.b (0xd0200002+wrAddr)   wrData[23:16]
    mw.b (0xd0200003+wrAddr)   wrData[31:24]              //write tigger
    md.b 0xd0020013 1          (accessBusy, should be 0)  //access busy status
    if (accessBusy == 1) read again

    md.b 0xd0020012 1          (should be 0)              //ack error if not be 0

    note:
    wrAddr:0 -> 4 -> 8 .... step is 4     
**************************************************/        
    sal_memset(&epld_para, 0, sizeof(epld_para_t));
    
    epld_para.addr = 0x20010;
    epld_para.val = fpga_idx;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x20011;
    epld_para.val = 0x0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x200000 + addr;
    epld_para.val = value&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x200001 + addr;
    epld_para.val = (value>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200002 + addr;
    epld_para.val = (value>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x200003 + addr;
    epld_para.val = (value>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20013 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((1 == epld_para.val)&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_BUS_BUSY;
    }
#if 0
    timeout = ACCESS_TIMEOUT;
    
    epld_para.addr = 0x20019 ;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    /* polling */
    while((!(ack == epld_para.val))&&(--timeout))
    {
        ret = epld_read(chip, &epld_para);
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }
#endif
#if 0
    epld_para.addr = 0x20012;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_ACK_ERRO;  
    }
#endif    
    return ret;
#endif
} 

#endif

#define FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F              -100
#define FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F              -101
#define FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F             -102
#define FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F             -103
#define FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F             -104
#define CMD_ERROR             -105

#if 0
int32
epld_sup_read(uint8 chip, uint32 addr, uint32 *value)
{
    int32 ret = 0;
    epld_para_t epld_para;
    uint8 temp_status[4];
    uint8 temp[4];
    uint8 fifoStatus;
    uint32 timeout;
/*************************************************        
    mw.b 0xd0002010 0x80
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00
**************************************************/     
    sal_memset(&epld_para, 0, sizeof(epld_para_t));
    epld_para.addr = 0x2010;
    epld_para.val = 0x80;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
/*************************************************            
    mw.b 0xd0002014 rdAddr[7:0]
    mw.b 0xd0002015 rdAddr[15:8]
    mw.b 0xd0002016 rdAddr[23:16]
    mw.b 0xd0002017 rdAddr[31:24]
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = addr&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (addr>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (addr>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (addr>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    mw.b 0xd0002010 0xa0
    mw.b 0xd0002011 0x0
    mw.b 0xd0002012 0x0
    mw.b 0xd0002013 0x0
    mw.b 0xd0002014 0x0
    mw.b 0xd0002015 0x0
    mw.b 0xd0002016 0x0
    mw.b 0xd0002017 0x0
**************************************************/     
    epld_para.addr = 0x2010;
    epld_para.val = 0xa0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2014;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    //fifo depth
    md.b 0xd0002000 1(fifoStatus1[7:0])
    md.b 0xd0002001 1(fifoStatus 1[15:8])
    md.b 0xd0002002 1(fifoStatus 1[23:16])
    md.b 0xd0002003 1(fifoStatus 1[31:24])
    if (fifoStatus [5:0] < 2) read again
   
**************************************************/     
POLL:
    epld_para.addr = 0x2000;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    fifoStatus = epld_para.val;

    
    epld_para.addr = 0x2001;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2002;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2003;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    timeout = ACCESS_TIMEOUT;
            
    /* polling */
    while(((fifoStatus < 2))&&(--timeout))
    {
        goto POLL;
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }

/*************************************************            
//status and read data
   md.b 0xd0002020 1
   md.b 0xd0002021 1
   md.b 0xd0002022 1
   md.b 0xd0002023 1
   md.b 0xd0002024 1(rdAddr[7:0])
   md.b 0xd0002025 1(rdAddr[15:8])
   md.b 0xd0002026 1(rdAddr[23:16])
   md.b 0xd0002027 1(rdAddr[31:24]) 
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************            
   md.b 0xd0002020 1(rdStatus2[7:0])
   md.b 0xd0002021 1(rdStatus2 [15:8])
   md.b 0xd0002022 1(rdStatus2 [23:16])
   md.b 0xd0002023 1(rdStatus2 [31:24])
   if ((rdStatus2 [6] !== 1'b0) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F
     (rdStatus2 [3] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F
     (rdStatus2 [5:4] !== 2) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F
     (rdStatus2 [7] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F
     (rdStatus2 [1:0] !== 2'b00) || //FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F
     (rdStatus2 [8] !== 1'b0)  // CMD_ERROR)
     Report error   
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[0] = epld_para.val;

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[1] = epld_para.val;

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[2] = epld_para.val;

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[3] = epld_para.val;

    if((temp_status[0]&0x03) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F;
    }

    if((temp_status[0]&0x08) != 0x08)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F;
    }

    if((temp_status[0]&0x30) != 0x20)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F;
    }

    if((temp_status[0]&0x40) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F;
    }

    if((temp_status[0]&0x80) != 0x80)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F;
    }

    if((temp_status[1]&0x01) != 0x00)
    {
        return  CMD_ERROR;
    }
    
/*************************************************            
    md.b 0xd0002024 1(rdData[7:0])
    md.b 0xd0002025 1(rdData[15:8])
    md.b 0xd0002026 1(rdData[23:16])
    md.b 0xd0002027 1(rdData[31:24])   
**************************************************/         
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[0] = epld_para.val;
    
    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[1] = epld_para.val;
    
    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[2] = epld_para.val;
    
    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[3] = epld_para.val;
    
    *value = (temp[3]<<24)|(temp[2]<<16)|(temp[1] <<8)|temp[0];
    DRV_LOG_DEBUG(DRV_EPLD, "epld sup read : addr: 0x%x value:0x%x\n",addr,*value);
    return ret;
} 

int32
epld_sup_write(uint8 chip, uint32 addr, uint32 value)
{
    int32 ret = 0;
    epld_para_t epld_para;
    uint8 temp_status[4];
    uint8 temp[4];
    uint8 fifoStatus;
    uint32 timeout;
/*************************************************            
    mw.b 0xd0002010 0x80
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00    
**************************************************/     
    sal_memset(&epld_para, 0, sizeof(epld_para_t));
    epld_para.addr = 0x2010;
    epld_para.val = 0x80;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
/*************************************************                
    mw.b 0xd0002014 wrAddr[7:0]
    mw.b 0xd0002015 wrAddr[15:8]
    mw.b 0xd0002016 wrAddr[23:16]
    mw.b 0xd0002017 wrAddr[31:24]    
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = addr&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (addr>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (addr>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (addr>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************               
    mw.b 0xd0002010 0x90
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00   
**************************************************/     
    epld_para.addr = 0x2010;
    epld_para.val = 0x90;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
/*************************************************               
    mw.b 0xd0002014 wrData[7:0]
    mw.b 0xd0002015 wrData[15:8]
    mw.b 0xd0002016 wrData[23:16]
    mw.b 0xd0002017 wrData[31:24]    
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = value&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (value>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (value>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (value>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    //fifo depth
    md.b 0xd0002000 1(fifoStatus1[7:0])
    md.b 0xd0002001 1(fifoStatus 1[15:8])
    md.b 0xd0002002 1(fifoStatus 1[23:16])
    md.b 0xd0002003 1(fifoStatus 1[31:24])
    if (fifoStatus [5:0] < 2) read again  
**************************************************/     
POLL:
    epld_para.addr = 0x2000;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    fifoStatus = epld_para.val;

    
    epld_para.addr = 0x2001;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2002;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2003;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    timeout = ACCESS_TIMEOUT;
            
    /* polling */
    while(((fifoStatus < 2))&&(--timeout))
    {
        goto POLL;
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }

/*************************************************            
    //status and read data
    md.b 0xd0002020 1
    md.b 0xd0002021 1
    md.b 0xd0002022 1
    md.b 0xd0002023 1
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************            
    md.b 0xd0002024 1(wrAddr[7:0])
    md.b 0xd0002025 1(wrAddr[15:8])
    md.b 0xd0002026 1(wrAddr[23:16])
    md.b 0xd0002027 1(wrAddr[31:24])
**************************************************/   
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************              
    md.b 0xd0002020 1(wrStatus2[7:0])
    md.b 0xd0002021 1(wrStatus2 [15:8])
    md.b 0xd0002022 1(wrStatus2 [23:16])
    md.b 0xd0002023 1(wrStatus2 [31:24])
    if ((wrStatus2 [6] !== 1'b0) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F
      (wrStatus2 [3] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F
      (wrStatus2 [5:4] !== 1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F
      (wrStatus2 [7] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F
      (wrStatus2 [1:0] !== 2'b00) || //FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F
      (wrStatus2 [8] !== 1'b0)  // CMD_ERROR)
      Report error  
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[0] = epld_para.val;

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[1] = epld_para.val;

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[2] = epld_para.val;

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[3] = epld_para.val;

    if((temp_status[0]&0x03) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F;
    }

    if((temp_status[0]&0x08) != 0x08)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F;
    }

    if((temp_status[0]&0x30) != 0x10)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F;
    }

    if((temp_status[0]&0x40) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F;
    }

    if((temp_status[0]&0x80) != 0x80)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F;
    }

    if((temp_status[1]&0x01) != 0x00)
    {
        return  CMD_ERROR;
    }
    
/*************************************************              
    md.b 0xd0002024 1(wrData[7:0])
    md.b 0xd0002025 1(wrData[15:8])
    md.b 0xd0002026 1(wrData [23:16])
    md.b 0xd0002027 1(wrData [31:24])  
**************************************************/         
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    return ret;
} 
#else

int32
epld_sup_read(uint8 chip, uint32 addr, uint32 *value)
{
#ifdef _CTC_LOONGSON_
    *value = 0;
    return 0;
#else
    int32 ret = 0;
    epld_para_t epld_para;
    uint8 temp_status[4];
    uint8 temp[4];
    uint8 fifoStatus;
    uint32 timeout;
/*************************************************        
    mw.b 0xd0002010 0x80 //command start flag
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00
**************************************************/     
    sal_memset(&epld_para, 0, sizeof(epld_para_t));
    epld_para.addr = 0x2010;
    epld_para.val = 0x80;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
/*************************************************            
    mw.b 0xd0002014 rdAddr[7:0]
    mw.b 0xd0002015 rdAddr[15:8]
    mw.b 0xd0002016 rdAddr[23:16]
    mw.b 0xd0002017 rdAddr[31:24]
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = addr&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (addr>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (addr>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (addr>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    mw.b 0xd0002010 0xa0 //read operation type
    mw.b 0xd0002011 0x0
    mw.b 0xd0002012 0x0
    mw.b 0xd0002013 0x0

    mw.b 0xd0002014 0x0
    mw.b 0xd0002015 0x0
    mw.b 0xd0002016 0x0
    mw.b 0xd0002017 0x0 //read trigger
**************************************************/     
    epld_para.addr = 0x2010;
    epld_para.val = 0xa0;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2014;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    //fifo depth
    md.b 0xd0002000 1(fifoStatus 1[7:0])
    md.b 0xd0002001 1(fifoStatus 1[15:8])
    md.b 0xd0002002 1(fifoStatus 1[23:16])
    md.b 0xd0002003 1(fifoStatus 1[31:24])
    if (fifoStatus [5:0] < 2) read again
**************************************************/     
POLL:
    epld_para.addr = 0x2000;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    fifoStatus = epld_para.val;

    
    epld_para.addr = 0x2001;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2002;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2003;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    timeout = ACCESS_TIMEOUT;
            
    /* polling */
    while((fifoStatus < 2)&&(--timeout))
    {
        goto POLL;
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }

/*************************************************            
//status and read data
    md.b 0xd0002020 1
    md.b 0xd0002021 1
    md.b 0xd0002022 1
    md.b 0xd0002023 1

    md.b 0xd0002024 1(rdAddr[7:0])
    md.b 0xd0002025 1(rdAddr[15:8])
    md.b 0xd0002026 1(rdAddr[23:16])
    md.b 0xd0002027 1(rdAddr[31:24]) 
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************            
    md.b 0xd0002020 1(rdStatus2 [7:0])
    md.b 0xd0002021 1(rdStatus2 [15:8])
    md.b 0xd0002022 1(rdStatus2 [23:16])
    md.b 0xd0002023 1(rdStatus2 [31:24])
    if ((rdStatus2 [6] !== 1'b0)        || //FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F
    (rdStatus2   [3] !== 1'b1)      || //FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F
    (rdStatus2   [5:4] !== 2'b10)   || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F
    (rdStatus2   [7] !== 1'b1)      || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F
    (rdStatus2   [1:0] !== 2'b00)   || //FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F
    (rdStatus2   [8] !== 1'b0))        // CMD_ERROR)
    Report error
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[0] = epld_para.val;

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[1] = epld_para.val;

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[2] = epld_para.val;

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[3] = epld_para.val;

    if((temp_status[0]&0x03) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F;
    }

    if((temp_status[0]&0x08) != 0x08)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F;
    }

    if((temp_status[0]&0x30) != 0x20)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F;
    }

    if((temp_status[0]&0x40) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F;
    }

    if((temp_status[0]&0x80) != 0x80)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F;
    }

    if((temp_status[1]&0x01) != 0x00)
    {
        return  CMD_ERROR;
    }
    
/*************************************************            
    md.b 0xd0002024 1(rdData[7:0])
    md.b 0xd0002025 1(rdData[15:8])
    md.b 0xd0002026 1(rdData[23:16])
    md.b 0xd0002027 1(rdData[31:24])   
**************************************************/         
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[0] = epld_para.val;
    
    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[1] = epld_para.val;
    
    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[2] = epld_para.val;
    
    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    temp[3] = epld_para.val;

    
    epld_para.addr = 0x2040;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_ACK_ERRO;  
    }
    
    *value = (temp[3]<<24)|(temp[2]<<16)|(temp[1] <<8)|temp[0];
    DRV_LOG_DEBUG(DRV_EPLD, "epld sup read : addr: 0x%x value:0x%x\n",addr,*value);
    return ret;
#endif
} 

int32
epld_sup_write(uint8 chip, uint32 addr, uint32 value)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    epld_para_t epld_para;
    uint8 temp_status[4];
//    uint8 temp[4];
    uint8 fifoStatus;
    uint32 timeout;
/*************************************************            
    mw.b 0xd0002010 0x80
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00    
**************************************************/     
    sal_memset(&epld_para, 0, sizeof(epld_para_t));
    epld_para.addr = 0x2010;
    epld_para.val = 0x80;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
/*************************************************                
    mw.b 0xd0002014 wrAddr[7:0]
    mw.b 0xd0002015 wrAddr[15:8]
    mw.b 0xd0002016 wrAddr[23:16]
    mw.b 0xd0002017 wrAddr[31:24]    
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = addr&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (addr>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (addr>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (addr>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************               
    mw.b 0xd0002010 0x90
    mw.b 0xd0002011 0x00
    mw.b 0xd0002012 0x00
    mw.b 0xd0002013 0x00   
**************************************************/     
    epld_para.addr = 0x2010;
    epld_para.val = 0x90;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2011;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2012;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2013;
    epld_para.val = 0x00;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
/*************************************************               
    mw.b 0xd0002014 wrData[7:0]
    mw.b 0xd0002015 wrData[15:8]
    mw.b 0xd0002016 wrData[23:16]
    mw.b 0xd0002017 wrData[31:24]    
**************************************************/     

    epld_para.addr = 0x2014;
    epld_para.val = value&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2015;
    epld_para.val = (value>>8)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }
    
    epld_para.addr = 0x2016;
    epld_para.val = (value>>16)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

    epld_para.addr = 0x2017;
    epld_para.val = (value>>24)&0xff;
    ret = epld_write(chip, &epld_para);
    if(ret)
    {
        return CHIP_WRITE_FAIL;
    }

/*************************************************            
    //fifo depth
    md.b 0xd0002000 1(fifoStatus1[7:0])
    md.b 0xd0002001 1(fifoStatus 1[15:8])
    md.b 0xd0002002 1(fifoStatus 1[23:16])
    md.b 0xd0002003 1(fifoStatus 1[31:24])
    if (fifoStatus [5:0] < 2) read again  
**************************************************/     
POLL:
    epld_para.addr = 0x2000;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    fifoStatus = epld_para.val;

    
    epld_para.addr = 0x2001;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2002;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    
    epld_para.addr = 0x2003;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    timeout = ACCESS_TIMEOUT;
            
    /* polling */
    while(((fifoStatus < 2))&&(--timeout))
    {
        goto POLL;
    }

    if(!timeout)
    {
        return CHIP_ACCESS_TIMEOUT;
    }

/*************************************************            
    //status and read data
    md.b 0xd0002020 1
    md.b 0xd0002021 1
    md.b 0xd0002022 1
    md.b 0xd0002023 1
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************            
    md.b 0xd0002024 1(wrAddr[7:0])
    md.b 0xd0002025 1(wrAddr[15:8])
    md.b 0xd0002026 1(wrAddr[23:16])
    md.b 0xd0002027 1(wrAddr[31:24])
**************************************************/   
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
/*************************************************              
    md.b 0xd0002020 1(wrStatus2[7:0])
    md.b 0xd0002021 1(wrStatus2 [15:8])
    md.b 0xd0002022 1(wrStatus2 [23:16])
    md.b 0xd0002023 1(wrStatus2 [31:24])
    if ((wrStatus2 [6] !== 1'b0) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F
      (wrStatus2 [3] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F
      (wrStatus2 [5:4] !== 1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F
      (wrStatus2 [7] !== 1'b1) || //FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F
      (wrStatus2 [1:0] !== 2'b00) || //FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F
      (wrStatus2 [8] !== 1'b0)  // CMD_ERROR)
      Report error  
**************************************************/   
    epld_para.addr = 0x2020;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[0] = epld_para.val;

    epld_para.addr = 0x2021;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[1] = epld_para.val;

    epld_para.addr = 0x2022;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[2] = epld_para.val;

    epld_para.addr = 0x2023;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    temp_status[3] = epld_para.val;

    if((temp_status[0]&0x03) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_EXTENDED_DATA_F;
    }

    if((temp_status[0]&0x08) != 0x08)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ACK_F;
    }

    if((temp_status[0]&0x30) != 0x10)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_TYPE_F;
    }

    if((temp_status[0]&0x40) != 0x00)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_ERROR_ACK_F;
    }

    if((temp_status[0]&0x80) != 0x80)
    {
        return FPGA_SUP_RESULTMEM_CFG_RESULT_CMD_START_F;
    }

    if((temp_status[1]&0x01) != 0x00)
    {
        return  CMD_ERROR;
    }
    
/*************************************************              
    md.b 0xd0002024 1(wrData[7:0])
    md.b 0xd0002025 1(wrData[15:8])
    md.b 0xd0002026 1(wrData [23:16])
    md.b 0xd0002027 1(wrData [31:24])  
**************************************************/         
    epld_para.addr = 0x2024;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2025;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2026;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }
    
    epld_para.addr = 0x2027;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    epld_para.addr = 0x2040;
    ret = epld_read(chip, &epld_para);
    if(ret)
    {
        return CHIP_READ_FAIL;
    }

    if(epld_para.val != 0)
    {
        return CHIP_ACK_ERRO;  
    }
    
    return ret;
#endif
} 
#endif

int32 
epld_shim_test(uint32 loop)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    uint32 random_val[1024];
    uint32 temp_val[1024];
    uint32 count;
    uint32 addr = 0x70000,temp_addr0,addr1;
    uint8 fpga_idx;

    for(;loop> 0;loop--)
    {
        for(fpga_idx = 1; fpga_idx< 6;fpga_idx++ )
        {
            for(count = 0,temp_addr0 = 0;count<1024;count++)
            {
                random_val[count] = (uint32)random();
                temp_addr0 = count*4;
                addr1 = addr + temp_addr0;
                ret = epld_shim_write(0, fpga_idx, addr1, random_val[count]); 
                if(ret != 0)
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "shim test : write fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,random_val[count]);
                    return ret;
                }
                else
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "shim test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,random_val[count]);
                }
            }

            for(count = 0,temp_addr0 = 0;count<1024;count++)
            {
                temp_addr0 = count*4;
                addr1 = addr + temp_addr0;
                ret = epld_shim_read(0, fpga_idx, addr1, &temp_val[count]);  
                if(ret != 0)
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "shim test : read fpga %d addr: 0x%x temp value:0x%x fail\n",fpga_idx,addr,temp_val[count]);
                    return ret;
                }
                else
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "shim test : write fpga %d addr: 0x%x temp value:0x%x\n",fpga_idx,addr,temp_val[count]);
                }
            }

            for(count = 0;count<1024;count++)
            {        
                if(temp_val[count] != random_val[count])
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "shim test : cmp fpga %d count: 0x%x random value:0x%x temp value:0x%x fail\n",fpga_idx,count,random_val[count],temp_val[count]);
                    return CHIP_VALUE_NOT_EQUAL;
                }        
            }
        }
    }    
        
    return ret; 
#endif
}

int32 
epld_sup_test(uint32 loop)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    uint32 random_val[1024];
    uint32 temp_val[1024];
    uint32 count;
    uint32 addr = 0;

    for(;loop> 0;loop--)
    {
        for(count = 0,addr = 0;count<1024;count++)
        {
            random_val[count] = (uint32)random();
            addr = count*4;
            ret = epld_sup_write(0, addr, random_val[count]); 
            if(ret != 0)
            {
                DRV_LOG_DEBUG(DRV_EPLD, "sup test : write addr: 0x%x random value:0x%x fail\n",addr,random_val[count]);
                return ret;
            }
            else
            {
                DRV_LOG_DEBUG(DRV_EPLD, "sup test : write addr: 0x%x random value:0x%x\n",addr,random_val[count]);
            }
        }

        for(count = 0,addr = 0;count<1024;count++)
        {
            addr = count*4;
            ret = epld_sup_read(0, addr, &temp_val[count]);  
            if(ret != 0)
            {
                DRV_LOG_DEBUG(DRV_EPLD, "sup test : read addr: 0x%x random value:0x%x fail\n",addr,temp_val[count]);
                return ret;
            }
            else
            {
                DRV_LOG_DEBUG(DRV_EPLD, "sup test : write addr: 0x%x random value:0x%x\n",addr,temp_val[count]);
            }
        }

        for(count = 0;count<1024;count++)
        {        
            if(temp_val[count] != random_val[count])
            {
                DRV_LOG_DEBUG(DRV_EPLD, "shim sup : cmp addr: 0x%x random value:0x%x fail\n",addr,temp_val[count]);
                return CHIP_VALUE_NOT_EQUAL;
            }        
        }            
    }
    
        
    return ret; 
#endif
}

#if 0
int32 
epld_sup_shim_test(uint32 loop)
{
    int32 ret = 0;
    uint32 loop1,loop2;
    for(loop1=0;loop1<1000;loop1++)
    {
        for(loop2=0;loop2<1800;loop2++)
        {
            ret = epld_shim_test();
            if(ret != 0)
            {                
                return ret;
            }
        }
    }

    for(loop1=0;loop1<1000;loop1++)
    {
        for(loop2=0;loop2<1800;loop2++)
        {
            ret = epld_sup_test();
            if(ret != 0)
            {                
                return ret;
            }
        }
    }
}
#else
int32 
epld_sup_shim_test(uint32 loop)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
   
    ret = epld_shim_test(loop);
    if(ret != 0)
    {                
        return ret;
    }
      
    ret = epld_sup_test(loop);
    if(ret != 0)
    {                
        return ret;
    }

    return ret;
#endif
}

#endif

int32 
epld_ddr_test(uint32 loop,uint8 fpga_idx)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    uint32 random_val[1024];
    uint32 temp_val[1024*16];
    
    
    uint32 count;
    uint32 addr = 0;
    uint32 value = 0;
    uint32 temp = 0;
    uint32 temp_count;
    uint32 temp_addr;
    for(;loop> 0;loop--)
    {
        {
#if 0            
            addr = 0x2801c;
            value = 1;
            ret = epld_shim_write(0, fpga_idx, addr, value); 
            if(ret != 0)
            {
                DRV_LOG_DEBUG(DRV_EPLD, "ddr test : switch channel to ddr fail\n");
                return ret;
            }
#endif            
            for(count = 0,addr = 0;count<1024;count++)
            {
                random_val[count] = (uint32)random();
                addr = 0x50048;
                ret = epld_shim_write(0, fpga_idx, addr, random_val[count]); 
                if(ret != 0)
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,random_val[count]);
                    return ret;
                }
                else
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,random_val[count]);
                }

                value = count*4;
                addr = 0x50044;
                ret = epld_shim_write(0, fpga_idx, addr, value); 
                if(ret != 0)
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,random_val[count]);
                    return ret;
                }
                else
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,random_val[count]);
                }
            }

            for(count = 0,addr = 0;count<1024;count++)
            {
                addr = count*4;
                
                temp = 0x50044;
                value = ((1<<31)|addr);
                ret = epld_shim_write(0, fpga_idx, temp, value); 
                if(ret != 0)
                {
                    DRV_LOG_DEBUG(DRV_EPLD, "ddr test : switch channel to ddr fail\n");
                    return ret;
                }
                usleep(1000);

                for(temp_count = 0,temp_addr = 0x50000;temp_count<16;temp_count++)
                {
                    temp_addr +=temp_count*4;
                    ret = epld_shim_read(0, fpga_idx, temp_addr, &temp_val[count *16+ temp_count]);  
                    if(ret != 0)
                    {
                        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : read fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,temp_val[count]);
                        return ret;
                    }
                    else
                    {
                        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,temp_val[count]);
                    }
                    }                
                
            }

            for(count = 0;count<1024;count++)
            {   
                for(temp_count = 0;temp_count<16;temp_count++)
                {
                    if(temp_val[count *16+ temp_count] != random_val[count])
                    {
                        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : cmp fpga %d random value:0x%x fail\n",fpga_idx,temp_val[count *16+ temp_count]);
                        return CHIP_VALUE_NOT_EQUAL;
                    }                      
                }                
               
            }
#if 0
            addr = 0x2801c;
            value = 0;
            ret = epld_shim_write(0, fpga_idx, addr, value); 
            if(ret != 0)
            {
                DRV_LOG_DEBUG(DRV_EPLD, "ddr test : switch channel to shim fail\n");
                return ret;
            }
#endif            
        }
    }    
        
    return ret; 
#endif
}

int32 
epld_download_fpga(uint8 board_idx,uint8 fpga_idx,uint8 count)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    epld_para_t epld_para;
    char szCmd[256];
    
    if(count == 1)
    {
        /*download 5 fpga image*/        
        epld_para.addr = 0x20009;
        epld_para.val = 0x01;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_1.bin /dev/mtd0",board_idx);
        system(szCmd);

        epld_para.addr = 0x20009;
        epld_para.val = 0x01;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_2.bin /dev/mtd1",board_idx);
        system(szCmd);

        epld_para.addr = 0x20009;
        epld_para.val = 0x11;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_3.bin /dev/mtd0",board_idx);
        system(szCmd);

        epld_para.addr = 0x20009;
        epld_para.val = 0x0;
        ret = epld_write(0, &epld_para);        
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_4.bin /dev/mtd0",board_idx);
        system(szCmd);

        epld_para.addr = 0x20009;
        epld_para.val = 0x0;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_5.bin /dev/mtd1",board_idx);
        system(szCmd);
    }
    
    if(fpga_idx ==1)
    {
        epld_para.addr = 0x20009;
        epld_para.val = 0x01;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_1.bin /dev/mtd0",board_idx);
        system(szCmd);
    }
    else if(fpga_idx ==2)
    {
        epld_para.addr = 0x20009;
        epld_para.val = 0x01;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_2.bin /dev/mtd1",board_idx);
        system(szCmd);
    }
    else if(fpga_idx ==3)
    {
        epld_para.addr = 0x20009;
        epld_para.val = 0x11;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_3.bin /dev/mtd0",board_idx);
        system(szCmd);
    }
    else if(fpga_idx ==4)
    {
        epld_para.addr = 0x20009;
        epld_para.val = 0x0;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_4.bin /dev/mtd0",board_idx);
        system(szCmd);
    }
    else if(fpga_idx ==5)
    {
        epld_para.addr = 0x20009;
        epld_para.val = 0x0;
        ret = epld_write(0, &epld_para);
        snprintf(szCmd, 256, "flashcp -v /data/asic/goldengate/%d/fpga_5.bin /dev/mtd1",board_idx);
        system(szCmd);
    }
    return ret;
#endif
}

int32 
epld_ddr_read(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 *value)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    uint32 temp_addr;
    uint32 temp_value;
    
    temp_addr = 0x50044;
    temp_value = ((1<<31)|addr);
    ret = epld_shim_write(0, fpga_idx, temp_addr, temp_value); 
    if(ret != 0)
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : switch channel to ddr fail\n");
        return ret;
    }
    usleep(1000);
    
    temp_addr = 0x50000;
    ret = epld_shim_read(0, fpga_idx, temp_addr + addr, value);  
    if(ret != 0)
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : read fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,*value);
        return ret;
    }
    else
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,*value);
    }               


    return ret; 
#endif
}

int32 
epld_ddr_write(uint8 chip, uint8 fpga_idx,uint32 addr, uint32 value)
{
#ifdef _CTC_LOONGSON_
    return 0;
#else
    int32 ret = 0;
    uint32 temp_addr;
    
    temp_addr = 0x50048;
    ret = epld_shim_write(0, fpga_idx, temp_addr, value); 
    if(ret != 0)
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,temp_addr,value);
        return ret;
    }
    else
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,value);
    }

    temp_addr = 0x50044;
    ret = epld_shim_write(0, fpga_idx, temp_addr, addr); 
    if(ret != 0)
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x fail\n",fpga_idx,addr,value);
        return ret;
    }
    else
    {
        DRV_LOG_DEBUG(DRV_EPLD, "ddr test : write fpga %d addr: 0x%x random value:0x%x\n",fpga_idx,addr,value);
    }
       

    return ret; 
#endif
}


