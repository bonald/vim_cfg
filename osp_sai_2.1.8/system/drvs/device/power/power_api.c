/****************************************************************************
 * fan_api.c    power api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       jcao
 * Date:         2010-10-11.
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
#include "power_drv.h"
#include "power_api.h"
#include "epld_api.h"
#include "drv_log.h"
#include "ctc_i2c.h"
#include "gpio_api.h"
/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
static psu_handle_t* g_power_hdl;
static uint8 g_power_num;

static psu_id_info_t g_psu_id_info[] = {
    [PSU_MFR_GREATWALL_AC]     =    {
        .id_table          = {0x08,0x08,0x08,0x08},
        .psu_mode	       = PSU_AC,  //AC
    },
    [PSU_MFR_3Y_DC]        =    {
        .id_table          = {0x06,0x04,0x01,0x03},
        .psu_mode	       = PSU_DC,  //DC
    },
    [PSU_MFR_ETASIS_AC]    = {
        .id_table          = {0x09,0x54,0x46,0x52,0x4b,0x2d,0x47,0x33,0x35,0x36},
        .psu_mode	       = PSU_AC,  //AC
    },
    [PSU_MFR_CPR_AC]       = {
        .id_table          = {0x43,0x50,0x52,0x2d,0x34,0x30,0x31,0x31,0x2d,0x34,0x4d,0x31,0x31},
        .psu_mode	       = PSU_AC,  //AC
    },
    [PSU_MFR_CPR_AC1]       = {
        .id_table          = {0x43,0x50,0x52,0x34,0x30,0x31,0x31,0x2d,0x34,0x4d,0x31,0x37,0xcc},
        .psu_mode	       = PSU_AC,  //AC
    },
    [PSU_MFR_UM_DC]        = {
        .id_table          = {0x75,0x6d,0x34,0x30,0x30,0x64,0x30,0x31},
        .psu_mode	       = PSU_DC,  //DC
    },
    [PSU_MFR_3Y_DC_STANDARD] = {
        .id_table            = {0x59,0x4d,0x2d,0x31,0x31,0x35,0x31,0x45},
        .psu_mode	         = PSU_DC,  //DC
    },
};

static psu_mode_info_t g_psu_mode_info[] = {
	[PSU_MODE_TYPE_REG_50_01_04] =   {
    	.dev_addr          = 0x50,
		.id_reg	           = 1,
        .id_len	           = 4,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_GREATWALL_AC],
            [1]     =    &g_psu_id_info[PSU_MFR_3Y_DC],
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x58,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
    },
	[PSU_MODE_TYPE_REG_51_01_04] =   {
    	.dev_addr          = 0x51,
		.id_reg	           = 1,
        .id_len	           = 4,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_GREATWALL_AC],
            [1]     =    &g_psu_id_info[PSU_MFR_3Y_DC],
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x59,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
    },
	[PSU_MODE_TYPE_REG_5b_9a_0a] =   {
    	.dev_addr          = 0x5b,
		.id_reg	           = 0x9a,
        .id_len	           = 0xa,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_ETASIS_AC],
            [1]     =    NULL,
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x5b,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
    },
	[PSU_MODE_TYPE_REG_38_26_0d] =   {
	    .dev_addr          = 0x38,
		.id_reg	           = 0x26,
        .id_len	           = 0xd,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_CPR_AC],
            [1]     =    &g_psu_id_info[PSU_MFR_CPR_AC1],
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x3c,
            [1]     =    0x3c,
            [2]     =    0x0,
            [3]     =    0x0,
        },
    },
	[PSU_MODE_TYPE_REG_50_50_08] =   {
	    .dev_addr          = 0x50,
		.id_reg	           = 0x50,
        .id_len	           = 0x8,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_UM_DC],
            [1]     =    NULL,
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x0,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
	},
	[PSU_MODE_TYPE_REG_50_20_08] =   {
	    .dev_addr          = 0x50,
		.id_reg	           = 0x20,
        .id_len	           = 0x8,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_3Y_DC_STANDARD],
            [1]     =    NULL,
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x0,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
	},
	[PSU_MODE_TYPE_REG_51_20_08] =   {
	    .dev_addr          = 0x51,
		.id_reg	           = 0x20,
        .id_len	           = 0x8,
        .id_info	       = {
            [0]     =    &g_psu_id_info[PSU_MFR_3Y_DC_STANDARD],
            [1]     =    NULL,
            [2]     =    NULL,
            [3]     =    NULL,
        },
        .pmbus_addr = {
            [0]     =    0x0,
            [1]     =    0x0,
            [2]     =    0x0,
            [3]     =    0x0,
        },
	},
};

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
static psu_handle_t*
_get_power_handle(uint8 psu_idx)
{
    if(g_power_num <= psu_idx)
    {
        return NULL;
    }
    return &g_power_hdl[psu_idx];
}

static int32
_psu_epld_init(psu_handle_t* handle)
{
    return RESULT_OK;
}

/*shutdown: 1 mean shutdown, 0 mean undo shutdown*/
static int32
_psu_epld_set_shutdown(psu_handle_t* handle, uint8 psu_idx, uint8 shutdown)
{
    uint8 epld_idx;
    epld_idx = *((uint8 *)handle->io_hdl->p_data_epld);
    epld_set_psu_shutdown(psu_idx, epld_idx, shutdown);
    return RESULT_OK;
}

static int32
_psu_epld_get_status(psu_handle_t* handle, uint8 psu_idx, psu_status_t* status)
{
    uint8 epld_idx, psu_type;
    epld_psu_status_t epld_psu_status = {0};
    
    psu_type = handle->p_data->psu_type;
    epld_idx = *((uint8 *)handle->io_hdl->p_data_epld);
    epld_get_psu_status(psu_idx, &epld_psu_status, epld_idx, psu_type);
    /* deleted by liuht for bug 24525,e350 don't support psu fan, 2013-10-24 */	   	
#if 0	
    status->psu_fan = epld_psu_status.psu_fan;
#endif
    status->psu_mode = epld_psu_status.psu_mode;
    status->psu_absent = epld_psu_status.psu_absent;
    status->psu_work_status = epld_psu_status.psu_work_status;
    status->psu_alert = epld_psu_status.psu_alert;
    /* deleted by liuht for bug 24525,e350 don't support psu shutdown, 2013-10-24 */	
#if 0	
    status->shutdown = epld_psu_status.psu_shutdown;
#endif
    status->psu_type = handle->p_data->psu_type;
	status->fixed_status = handle->p_data->fixed_status;
    return RESULT_OK;
}

/* added by liuht for bug 24525,2013-10-24 */
static int32
_psu_reg_read(psu_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    psu_access_t access;
    int ret;

    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->read(handle->io_hdl, &access);
    if(ret < 0)
    {
        //DRV_LOG_ERR("psu reg read fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

/* Added by liuht  for bug26772 to write power eeprom, 2013-01-11 */
static int32
_psu_reg_write(psu_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    psu_access_t access;
    int ret;

    access.reg = reg;
    access.val = value;
    access.len = len;
    ret = handle->io_hdl->write(handle->io_hdl, &access);
    if(ret < 0)
    {
        DRV_LOG_ERR("psu reg write fail. reg 0x%x", reg);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_psu_attr_mod(psu_handle_t* handle, uint8 dev_addr)
{
    int ret;

    ret = handle->io_hdl->mattr(handle->io_hdl, dev_addr);
    if(ret < 0)
    {
        DRV_LOG_ERR("modify power attr dev_addr 0x%x", dev_addr);
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

static int32
_psu_i2c_get_pmbus_addr(psu_handle_t* handle, uint8* addr)
{
    uint8 buf[PSU_ID_LEN_MAX];
    uint8 i, j;
    uint8 dev_addr;
    uint8 id_match = 0;
    uint8 id_reg, id_len;
    uint32 psu_mode_type;
    psu_id_info_t* p_psu_id_info;
    int ret = RESULT_OK;

    psu_mode_type = handle->p_data->psu_mode_type;
    for(i=0; i<PSU_MODE_TYPE_REG_MAX; i++)
    {
        if((psu_mode_type>>i) & 0x1)
        {
            dev_addr = g_psu_mode_info[i].dev_addr;
            id_reg = g_psu_mode_info[i].id_reg;
            id_len = g_psu_mode_info[i].id_len;
            ret = handle->attr_mod(handle, dev_addr);
            if(ret < 0)
            {
                continue;
            }
            sal_memset(buf, 0, PSU_ID_LEN_MAX);
            ret = handle->reg_read(handle, id_reg, buf, id_len);
            if(ret < 0)
            {
                continue;
            }

            for(j=0; j<PSU_ID_TABLE_LEN_MAX; j++)
            {
                p_psu_id_info = g_psu_mode_info[i].id_info[j];
                if(!p_psu_id_info)
                    continue;

                if(sal_strncmp((char*)p_psu_id_info->id_table, (char*)buf, id_len) == 0)    
                {
                    *addr = g_psu_mode_info[i].pmbus_addr[j];
                    id_match = 1;
                    break;
                }
            }
        }
        if(id_match)
        {
            break;
        }
    }
    if(!id_match)
    {
        *addr = 0x0;
    }

    return RESULT_OK;
}

static int32
_psu_i2c_get_mode(psu_handle_t* handle, uint8 psu_idx, uint8* mode)
{
    uint8 buf[PSU_ID_LEN_MAX];
    uint8 i, j;
    uint8 dev_addr;
    uint8 id_match = 0;
    uint8 id_reg, id_len;
    uint32 psu_mode_type;
    psu_id_info_t* p_psu_id_info;
    int ret = RESULT_OK;

    psu_mode_type = handle->p_data->psu_mode_type;
    for(i=0; i<PSU_MODE_TYPE_REG_MAX; i++)
    {
        if((psu_mode_type>>i) & 0x1)
        {
            dev_addr = g_psu_mode_info[i].dev_addr;
            id_reg = g_psu_mode_info[i].id_reg;
            id_len = g_psu_mode_info[i].id_len;
            ret = handle->attr_mod(handle, dev_addr);
            if(ret < 0)
            {
                continue;
            }
            sal_memset(buf, 0, PSU_ID_LEN_MAX);
            ret = handle->reg_read(handle, id_reg, buf, id_len);
            if(ret < 0)
            {
                continue;
            }

            for(j=0; j<PSU_ID_TABLE_LEN_MAX; j++)
            {
                p_psu_id_info = g_psu_mode_info[i].id_info[j];
                if(!p_psu_id_info)
                    continue;

                if(sal_strncmp((char*)p_psu_id_info->id_table, (char*)buf, id_len) == 0)    
                {
                    *mode = p_psu_id_info->psu_mode;
                    id_match = 1;
                    break;
                }
            }
        }
        if(id_match)
        {
            break;
        }
    }
    if(!id_match)
    {
        *mode = PSU_AC;
    }

    return RESULT_OK;
}

static int32
_psu_i2c_search_addr(psu_handle_t* handle, uint8* addr)
{
    uint8 buf[PSU_ID_LEN_MAX];
    uint8 i, j;
    uint8 dev_addr;
    uint8 id_match = 0;
    uint8 id_reg, id_len;
    uint32 psu_mode_type;
    psu_id_info_t* p_psu_id_info;
    int ret = RESULT_OK;
    i2c_handle_t *i2c_hdl = (i2c_handle_t*)(handle->io_hdl->handle);
    *addr = ((i2c_gen_t *)(i2c_hdl->data))->addr;

    psu_mode_type = handle->p_data->psu_mode_type;
    for(i=0; i<PSU_MODE_TYPE_REG_MAX; i++)
    {
        if((psu_mode_type>>i) & 0x1)
        {
            dev_addr = g_psu_mode_info[i].dev_addr;
            id_reg = g_psu_mode_info[i].id_reg;
            id_len = g_psu_mode_info[i].id_len;
            ret = handle->attr_mod(handle, dev_addr);
            if(ret < 0)
            {
                continue;
            }
            sal_memset(buf, 0, PSU_ID_LEN_MAX);
            ret = handle->reg_read(handle, id_reg, buf, id_len);
            if(ret < 0)
            {
                continue;
            }

            for(j=0; j<PSU_ID_TABLE_LEN_MAX; j++)
            {
                p_psu_id_info = g_psu_mode_info[i].id_info[j];
                if(!p_psu_id_info)
                    continue;

                if(sal_strncmp((char*)p_psu_id_info->id_table, (char*)buf, id_len) == 0)    
                {
                    *addr = dev_addr;
                    id_match = 1;
                    break;
                }
            }
        }
        if(id_match)
        {
            break;
        }
    }

    return RESULT_OK;
}

static int32
_psu_pmbus_get_status(psu_handle_t* handle, uint8 reg, uint8* value, uint8 len)
{
    uint8 psu_pmbus_addr = 0;
    int32 ret = RESULT_OK;


    if (len == 0 || len >4)
        return RESULT_ERROR;
    DRV_CTC_CHK_PTR(handle);
    
    _psu_i2c_get_pmbus_addr(handle, &psu_pmbus_addr);
    if (psu_pmbus_addr == 0x0)
        return RESULT_ERROR;
    
    ret = handle->attr_mod(handle, psu_pmbus_addr);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }
    ret = handle->reg_read(handle, reg, value, len);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

static int32
_psu_i2c_epld_get_status(psu_handle_t* handle, uint8 psu_idx, psu_status_t* status)
{
    uint32 ret = RESULT_OK;
    uint8 epld_idx, psu_type;
    epld_psu_status_t epld_psu_status;

    /* 1. get psu status from epld */
    psu_type = handle->p_data->psu_type;
    epld_idx = *((uint8 *)handle->io_hdl->p_data_epld);
    epld_get_psu_status(psu_idx, &epld_psu_status, epld_idx, psu_type);    
    status->psu_absent = epld_psu_status.psu_absent;
    status->psu_work_status = epld_psu_status.psu_work_status;
    status->psu_alert = epld_psu_status.psu_alert;
    status->psu_type = handle->p_data->psu_type;
    status->fixed_status = handle->p_data->fixed_status;
    
    /*2. get psu mode from psu eeprom when power supply ok twice*/
    if(status->psu_work_status == 0)
    {
        if(handle->p_data->psu_ok_cnt<2)
            handle->p_data->psu_ok_cnt++;
    }
    else
    {
        handle->p_data->psu_ok_cnt=0;
    }

    if(handle->p_data->psu_ok_cnt<2)
    {
        handle->p_data->psu_mode = PSU_UNKNOWN;
        status->psu_mode = PSU_UNKNOWN;
    }
    else if(handle->p_data->psu_mode == PSU_UNKNOWN)
    {
        _psu_i2c_get_mode(handle,psu_idx,&(status->psu_mode));
        handle->p_data->psu_mode = status->psu_mode;
    }
    else
    {
        status->psu_mode = handle->p_data->psu_mode;
    }

    return ret;
}

static int32
_psu_i2c_gpio_get_status(psu_handle_t* handle, uint8 psu_idx, psu_status_t* status)
{
    int32 ret;
    psu_data_gpio *psu_data;
    uint8 value;
    
    /*1. get psu status by gpio*/
    psu_data = handle->io_hdl->p_data_gpio;
    /*raw value 1 means absent.*/
    ret = gpio_get_scan_special_bit_value(psu_data->present_chip, psu_data->present_no, &value);
    if(value)
    {
        status->psu_absent = psu_data->present_reverse?0:1;
    }
    else
    {
        status->psu_absent = psu_data->present_reverse?1:0;
    }
    /*raw value 0 means good.*/
    ret = gpio_get_scan_special_bit_value(psu_data->workstate_chip, psu_data->workstate_no, &value);
    if(value)
    {
        status->psu_work_status = psu_data->workstate_reverse?0:1;
    }
    else
    {
        status->psu_work_status = psu_data->workstate_reverse?1:0;
    }
    /*raw value 1 means alert.*/
    ret = gpio_get_scan_special_bit_value(psu_data->alert_chip, psu_data->alert_no, &value);
    if(value)
    {
        status->psu_alert = psu_data->alert_reverse?0:1;
    }
    else
    {
        status->psu_alert = psu_data->alert_reverse?1:0;
    }
    status->psu_type = handle->p_data->psu_type;
    status->fixed_status = handle->p_data->fixed_status;
    
    /*2. get psu mode from psu eeprom when power supply ok twice*/
    if(status->psu_work_status == 0)
    {
        if(handle->p_data->psu_ok_cnt<2)
            handle->p_data->psu_ok_cnt++;
    }
    else
    {
        handle->p_data->psu_ok_cnt=0;
    }

    if(handle->p_data->psu_ok_cnt<2)
    {
        handle->p_data->psu_mode = PSU_UNKNOWN;
        status->psu_mode = PSU_UNKNOWN;
    }
    else if(handle->p_data->psu_mode == PSU_UNKNOWN)
    {
        _psu_i2c_get_mode(handle,psu_idx,&(status->psu_mode));
        handle->p_data->psu_mode = status->psu_mode;
    }
    else
    {
        status->psu_mode = handle->p_data->psu_mode;
    }
    return ret;
}

static int32
_psu_gpio_get_status(psu_handle_t* handle, uint8 psu_idx, psu_status_t* status)
{
    int32 ret;
    psu_data_gpio *psu_data;
    uint8 value;
    
    /*1. get psu status by gpio*/
    psu_data = handle->io_hdl->p_data_gpio;
    /*raw value 1 means absent.*/
    ret = gpio_get_scan_special_bit_value(psu_data->present_chip, psu_data->present_no, &value);
    if(value)
    {
        status->psu_absent = psu_data->present_reverse?0:1;
    }
    else
    {
        status->psu_absent = psu_data->present_reverse?1:0;
    }
    /*raw value 0 means good.*/
    ret = gpio_get_scan_special_bit_value(psu_data->workstate_chip, psu_data->workstate_no, &value);
    if(value)
    {
        status->psu_work_status = psu_data->workstate_reverse?0:1;
    }
    else
    {
        status->psu_work_status = psu_data->workstate_reverse?1:0;
    }
    /*raw value 1 means alert.*/
    ret = gpio_get_scan_special_bit_value(psu_data->alert_chip, psu_data->alert_no, &value);
    if(value)
    {
        status->psu_alert = psu_data->alert_reverse?0:1;
    }
    else
    {
        status->psu_alert = psu_data->alert_reverse?1:0;
    }
    status->psu_type = handle->p_data->psu_type;
    status->fixed_status = handle->p_data->fixed_status;   /* add by chenjr for bug 53516*/
    
    /*2. get psu mode from psu eeprom when power supply ok twice*/
    if(status->psu_work_status == 0)
    {
        if(handle->p_data->psu_ok_cnt<2)
            handle->p_data->psu_ok_cnt++;
    }
    else
    {
        handle->p_data->psu_ok_cnt=0;
    }

    if(handle->p_data->psu_ok_cnt<2)
    {
        handle->p_data->psu_mode = PSU_UNKNOWN;
        status->psu_mode = PSU_UNKNOWN;
    }
    else if(handle->p_data->psu_mode == PSU_UNKNOWN)
    {
        /*raw value 1 means DC.*/
        ret = gpio_get_scan_special_bit_value(psu_data->power_mode_chip, psu_data->power_mode_no, &value);
        /* for bug 53249, by tongzb, fix power status info, 2019-08-22 */
        if(value)
        {
            status->psu_mode = psu_data->power_mode_reverse?PSU_AC:PSU_DC;
        }
        else
        {
            status->psu_mode = psu_data->power_mode_reverse?PSU_DC:PSU_AC;
        }        
        handle->p_data->psu_mode = status->psu_mode;
    }
    else
    {
        status->psu_mode = handle->p_data->psu_mode;
    }
    return ret;
}

static int32
_psu_EPLD_register_driver(psu_handle_t* handle)
{
    handle->psu_init = _psu_epld_init;
    handle->set_shutdown = _psu_epld_set_shutdown;
    handle->get_status = _psu_epld_get_status;
    return RESULT_OK;
}

/* added by liuht for bug 24525,2013-10-24 */
static int32
_psu_I2C_EPLD_register_driver(psu_handle_t* handle)
{
    handle->psu_init = _psu_epld_init;
    handle->set_shutdown = _psu_epld_set_shutdown;
    handle->reg_read = _psu_reg_read;
    handle->reg_write = _psu_reg_write;
    handle->attr_mod = _psu_attr_mod;
    handle->get_status = _psu_i2c_epld_get_status;
    return RESULT_OK;
}
static int32
_psu_I2C_GPIO_register_driver(psu_handle_t* handle)
{
    handle->psu_init = NULL;
    handle->set_shutdown = NULL;
    handle->reg_read = _psu_reg_read;
    handle->reg_write = _psu_reg_write;
    handle->attr_mod = _psu_attr_mod;
    handle->get_status = _psu_i2c_gpio_get_status;
    return RESULT_OK;    
}
static int32
_psu_GPIO_register_driver(psu_handle_t* handle)
{
    handle->psu_init = NULL;
    handle->set_shutdown = NULL;
    handle->get_status = _psu_gpio_get_status;
    return RESULT_OK;    
}

/*shutdown: 1 mean shutdown, 0 mean undo shutdown*/
int32 
psu_set_shutdown(uint8 psu_idx, uint8 shutdown)
{    
    psu_handle_t* p_hdl = NULL;

    p_hdl = _get_power_handle(psu_idx);
    
    DRV_CTC_CHK_PTR(p_hdl);
    if(NULL == p_hdl->set_shutdown)
        return RESULT_OK;
    return p_hdl->set_shutdown(p_hdl, psu_idx, shutdown);
}

int32 
psu_get_status(uint8 psu_idx, psu_status_t * p_psu_status)
{
    psu_handle_t* p_hdl = NULL;

    p_hdl = _get_power_handle(psu_idx);
    
    DRV_CTC_CHK_PTR(p_hdl);
    DRV_CTC_CHK_PTR(p_psu_status);
    
    return p_hdl->get_status(p_hdl, psu_idx, p_psu_status);
}

/* Added by liuht  for bug26772 to read power eeprom, 2013-01-11 */
int32
psu_reg_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len)
{
    if(psu_id >= g_power_num)
    {
        DRV_LOG_ERR("Psu id %d is error!, max is %d\n", psu_id, g_power_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_power_hdl[psu_id].reg_read);
    return g_power_hdl[psu_id].reg_read(&g_power_hdl[psu_id], reg, value, len);
}

/* Added by liuht  for bug26772 to write power eeprom, 2013-01-11 */
int32
psu_reg_write(uint32 psu_id, uint8 reg, uint8 value, uint8 len)
{
    if(psu_id >= g_power_num)
    {
        DRV_LOG_ERR("Psu id %d is error!, max is %d\n", psu_id, g_power_num);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_power_hdl[psu_id].reg_write);
    return g_power_hdl[psu_id].reg_write(&g_power_hdl[psu_id], reg, &value, len);
}

int32
psu_pmbus_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len)
{
    uint8 psu_pmbus_addr = 0;
    int32 ret = RESULT_OK;
    
    if(psu_id >= g_power_num)
    {
        DRV_LOG_ERR("Psu id %d is error!, max is %d\n", psu_id, g_power_num);
        return RESULT_ERROR;
    }
    
    _psu_i2c_get_pmbus_addr(&g_power_hdl[psu_id], &psu_pmbus_addr);
    if (psu_pmbus_addr == 0x0)
    {
        DRV_LOG_DEBUG(DRV_POWER,"Can not match pmbus addr for PSU\n");  
        return RESULT_ERROR;
    }
    ret = g_power_hdl[psu_id].attr_mod(&g_power_hdl[psu_id], psu_pmbus_addr);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_POWER,"Match pmbus addr for PSU : 0x%x\n",psu_pmbus_addr);  
    ret = g_power_hdl[psu_id].reg_read(&g_power_hdl[psu_id], reg, value, len);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }    
    return RESULT_OK;
}

int32
psu_normal_read(uint32 psu_id, uint8 reg, uint8* value, uint8 len)
{
    uint8 psu_real_addr = 0;
    int32 ret = RESULT_OK;    
    if(psu_id >= g_power_num)
    {
        DRV_LOG_ERR("Psu id %d is error!, max is %d\n", psu_id, g_power_num);
        return RESULT_ERROR;
    }
    
    _psu_i2c_search_addr(&g_power_hdl[psu_id], &psu_real_addr);
    
    ret = g_power_hdl[psu_id].attr_mod(&g_power_hdl[psu_id], psu_real_addr);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }
    DRV_LOG_DEBUG(DRV_POWER,"Match/Default slave addr for PSU : 0x%x\n",psu_real_addr);  

    ret = g_power_hdl[psu_id].reg_read(&g_power_hdl[psu_id], reg, value, len);
    if(ret < 0)
    {
        return RESULT_ERROR;
    }

    return RESULT_OK;    
}

int32
psu_pmbus_get_status(uint32 psu_id, psu_status_t * p_psu_status)
{
    uint8 buf[4] = {0};
    int32 ret = RESULT_OK;
    
    if(psu_id >= g_power_num)
    {
        DRV_LOG_ERR("Psu id %d is error!, max is %d\n", psu_id, g_power_num);
        return RESULT_ERROR;
    }

    p_psu_status->psu_pmbus_status.support_pmbus = g_power_hdl[psu_id].p_data->support_pmbus;
    if (p_psu_status->psu_pmbus_status.support_pmbus == 0)
    {
        return RESULT_OK;
    }

    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x78, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_byte = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }

    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x79, buf, 2);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_word = buf[0] | (buf[1] << 8);
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7a, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_vout= buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7b, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_iout = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7c, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_input = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7d, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_temperature = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7e, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_cml = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x7f, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_other = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x80, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_mfr_specific = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x81, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_fans_1_2 = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    
    ret = _psu_pmbus_get_status(&g_power_hdl[psu_id], 0x82, buf, 1);
    if (ret == RESULT_OK)
    {
        p_psu_status->psu_pmbus_status.status_fans_3_4 = buf[0];
    }
    else
    {
        goto DUMP_ERR_EXIT;
    }
    return RESULT_OK;

DUMP_ERR_EXIT:
    p_psu_status->psu_pmbus_status.status_byte = 0xde;
    p_psu_status->psu_pmbus_status.status_word = 0xadbe;
    p_psu_status->psu_pmbus_status.status_vout = 0xef;
    p_psu_status->psu_pmbus_status.status_iout = 0xde;
    p_psu_status->psu_pmbus_status.status_input = 0xad;
    p_psu_status->psu_pmbus_status.status_temperature = 0xbe;
    p_psu_status->psu_pmbus_status.status_cml = 0xef;
    p_psu_status->psu_pmbus_status.status_other = 0xde;
    p_psu_status->psu_pmbus_status.status_mfr_specific = 0xad;
    p_psu_status->psu_pmbus_status.status_fans_1_2 = 0xbe;
    p_psu_status->psu_pmbus_status.status_fans_3_4 = 0xef;

    return RESULT_OK;
}

/*********************************************************************
 * Name    : psu_init
 * Purpose :  init some data structure 
 * Input   : psu_chip_t power_type    - operate power type
          uint8 power_num         - the number of power
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
psu_init(psu_private_t* p_data, psu_chip_t* type, uint8 power_num)
{
    int32 i = 0;
    psu_private_data_t* private_data;

    DRV_CTC_CHK_PTR(p_data);
    DRV_CTC_CHK_PTR(type);
    
    g_power_hdl = (psu_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO,sizeof(psu_handle_t)*power_num);
    if(NULL == g_power_hdl)
    {
        DRV_LOG_ERR("Power alloc handler fail!\n");
        return RESULT_ERROR;
    }
    for (i = 0; i < power_num; i++)
    {
        private_data = (psu_private_data_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_POWER_INFO, sizeof(psu_private_data_t));
        if(NULL == private_data)
        {
            DRV_LOG_ERR("Power alloc private data fail!\n");
            return RESULT_ERROR;
        }

        g_power_hdl[i].p_data = private_data;
        private_data->psu_type = type[i].psu_type;
        private_data->psu_mode_type = type[i].psu_mode_type;	
        private_data->support_pmbus = type[i].support_pmbus;
        private_data->fixed_status= type[i].fixed_status;  /* add by chenjr for bug 53516*/
        
        private_data->psu_mode = PSU_UNKNOWN;	
        private_data->psu_ok_cnt = 0;
        g_power_hdl[i].io_hdl = psu_io_register(type[i].io_type, &p_data[i]);
        if(NULL == g_power_hdl[i].io_hdl)
        {
            DRV_LOG_ERR("Power alloc io handler fail!\n");
            return RESULT_ERROR;
        }
        switch(type[i].chip_type)
        {
            case PSU_EPLD:
                _psu_EPLD_register_driver(&g_power_hdl[i]);
                break;
            case PSU_I2C_EPLD:
                _psu_I2C_EPLD_register_driver(&g_power_hdl[i]);
                break;
            case PSU_I2C_GPIO:
                _psu_I2C_GPIO_register_driver(&g_power_hdl[i]);
                break;
            case PSU_GPIO:
                _psu_GPIO_register_driver(&g_power_hdl[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport power chip type %d!\n", type[i].chip_type);
                DRV_LOG_USER(E_ERROR, DRV_3_PSU_TYPE_UNKNOWN, type[i].chip_type);
                break;
        }
    }
    g_power_num = power_num;

    //psu_status_t p_psu_status;
    //psu_get_status(0, &p_psu_status);
    //psu_get_status(1, &p_psu_status);
    
    return RESULT_OK;
}

int32 
_psu_diagnostic_test(uint8 psu_num, psu_diag_result_t* diag_result)
{
    uint8 idx;
    int32 ret1 = RESULT_OK, ret2 = RESULT_OK;
    psu_status_t p_psu_status[MAX_PSU_NUM];

    DRV_CTC_CHK_PTR(diag_result);

    for(idx = 0; idx < psu_num; idx++)
    {
        ret1 = psu_get_status(idx, &p_psu_status[idx]);
        if(ret1)
        {
            diag_result[idx].access_fail = 1;
            ret2 = RESULT_ERROR;
            continue;
        }
        if(p_psu_status[idx].psu_absent)
        {
            diag_result[idx].absent = 1;
            //ret2 = RESULT_ERROR;
            continue;
        }
        if(p_psu_status[idx].psu_work_status)
        {
            diag_result[idx].status_fail = 1;
            ret2 = RESULT_ERROR;
        }
        if(p_psu_status[idx].psu_alert)
        {
            diag_result[idx].alert = 1;
            ret2 = RESULT_ERROR;
        }  
    }

    return ret2;
}


int32
psu_diagnostic_test_and_report(uint8 psu_num)
{
#ifndef _GLB_UML_SYSTEM_  
    uint8 i;
    int32 result = 0;
    psu_diag_result_t diag_result[DIAG_MAX_PSU_NUM];

    sal_memset(diag_result, 0, sizeof(psu_diag_result_t)*DIAG_MAX_PSU_NUM);
    result = _psu_diagnostic_test(psu_num, diag_result);
    if (result != RESULT_OK)
    {
        for(i = 0; i < psu_num; i++)
        {
            if(diag_result[i].access_fail)
            {
                DRV_DIAG_LOG("Power %d access fail!\n", i);
            }
            else if(diag_result[i].absent)
            {
                DRV_DIAG_LOG("Power %d absent!\n", i);
            }
            else
            {
                if (diag_result[i].status_fail)
                {
                    DRV_DIAG_LOG("Power %d work status fail!\n", i);
                }
                if (diag_result[i].alert)
                {
                    DRV_DIAG_LOG("Power %d alert!\n", i);
                }
            }

            if (diag_result[i].access_fail || diag_result[i].status_fail || diag_result[i].alert)
            {
                DRV_DIAG_LOG("Power %d diag fail!\n", i);
            }
            else
            {
                DRV_DIAG_LOG("Power %d diag pass!\n", i);
            } 
        }
        return RESULT_ERROR;
    }
    else
    {
        for (i = 0; i < psu_num; i++)
        {
                DRV_DIAG_LOG("Power %d diag pass!\n", i);
        }
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}

