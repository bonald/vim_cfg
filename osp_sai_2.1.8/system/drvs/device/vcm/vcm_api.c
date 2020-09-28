/****************************************************************************
 * vcm_api.c    vcm api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       tongzb 
 * Date:         2017-07-24
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
#include "vcm_api.h"
#include "vcm_drv.h"
#include "vcm_ltc2991.h"
#include "vcm_bmr464.h"
#include "vcm_max20743.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/
#define VCM_ACCESS_ERROR_COUNT_MAX 3

static vcm_handle_t* g_vcm_hdl = NULL;
static uint32 g_vcm_nums = 0; 

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
int32
vcm_dev_get_volt(uint32 vcm_id, vcm_access_t type, float32* voltage)
{
    int32 ret;
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].reg_read);
    ret = g_vcm_hdl[vcm_id].get_volt(&g_vcm_hdl[vcm_id], type, voltage);
    if(ret != RESULT_OK)
    {
        g_vcm_hdl[vcm_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_vcm_hdl[vcm_id].access_err_cnt > VCM_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *voltage = 0;
            g_vcm_hdl[vcm_id].access_err_cnt=0;
        }

    }
    else
    {
        g_vcm_hdl[vcm_id].access_err_cnt=0;
    }
    return ret;
}

int32
vcm_dev_get_cur(uint32 vcm_id, vcm_access_t type, float32* current)
{
    int32 ret;
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].reg_read);
    ret = g_vcm_hdl[vcm_id].get_cur(&g_vcm_hdl[vcm_id], type, current);
    if(ret != RESULT_OK)
    {
        g_vcm_hdl[vcm_id].access_err_cnt++;
        /*Bug18147. if access continue error, return error temp to notify user.*/
        if(g_vcm_hdl[vcm_id].access_err_cnt > VCM_ACCESS_ERROR_COUNT_MAX)
        {
            ret = RESULT_OK;
            *current = 0;
            g_vcm_hdl[vcm_id].access_err_cnt=0;
        }

    }
    else
    {
        g_vcm_hdl[vcm_id].access_err_cnt=0;
    }

    return ret;
}

int32
vcm_dev_get_temp(uint32 vcm_id, vcm_access_t type, float32* tmpr)
{
    int32 ret;
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].reg_read);
    ret = g_vcm_hdl[vcm_id].get_temp(&g_vcm_hdl[vcm_id], type, tmpr);
    if(ret != RESULT_OK)
    {
        ret = RESULT_ERROR;
        *tmpr = 0;
    }
    else
    {
        ret = RESULT_OK;
    }
    return ret;
}

int32
vcm_reg_read(uint32 vcm_id, uint8 reg, uint8* value, uint8 len)
{
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].reg_read);
    return g_vcm_hdl[vcm_id].reg_read(&g_vcm_hdl[vcm_id], reg, value);
}

int32
vcm_reg_write(uint32 vcm_id, uint8 reg, uint8* value, uint8 len)
{
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].reg_read);
    return g_vcm_hdl[vcm_id].reg_write(&g_vcm_hdl[vcm_id], reg, value);
}

/*fix bug44386, liangf, 2017-06-21*/
int32
vcm_dev_trigger(uint32 vcm_id, vcm_access_t type)
{
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].trigger);
    
    return g_vcm_hdl[vcm_id].trigger(&g_vcm_hdl[vcm_id], type);
}

int32
vcm_dev_init(uint32 vcm_id)
{
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    DRV_CTC_CHK_PTR(g_vcm_hdl[vcm_id].vcm_init);
    return g_vcm_hdl[vcm_id].vcm_init(&g_vcm_hdl[vcm_id]);
}
int32 vcm_get_stauts(uint32 vcm_id, vcm_cfg_t *cfg, vcm_status_t* status)
{
    vcm_handle_t * vcm_hdl;
    vcm_info_t * vcm_info;
    vcm_access_t vcm_para;
    uint8 i = 0;
    float32 value;
    int32 ret;
    if(vcm_id >= g_vcm_nums)
    {
        DRV_LOG_ERR("Vcm id %d is error!, max is %d\n", vcm_id, g_vcm_nums);
        return RESULT_ERROR;
    }
    vcm_hdl = &g_vcm_hdl[vcm_id];
    vcm_info = &vcm_hdl->vcm_info;
    switch(vcm_info->vcm_chip_type)
    {
        case VCM_LTC2991_CMS:
        case VCM_LTC2991_LMS:
        case VCM_LTC2991_IMS:
            /* channel no for ltc2991 */
            /* channel_no   type            
                    0       LTC2991_SINGLE_V1_VOLT,
                    1       LTC2991_SINGLE_V2_VOLT,
                    2       LTC2991_SINGLE_V3_VOLT,
                    3       LTC2991_SINGLE_V4_VOLT,
                    4       LTC2991_SINGLE_V5_VOLT,
                    5       LTC2991_SINGLE_V6_VOLT,
                    6       LTC2991_SINGLE_V7_VOLT,
                    7       LTC2991_SINGLE_V8_VOLT,        
                    8       LTC2991_VCC_VOLT,
            */ 
            for(i=0; i<MAX_VCM_CHANNEL_NUM; i++ )
            {
                if((cfg->channel_cfg[i].flag & GLB_PIN_VOLT) || (cfg->channel_cfg[i].flag & GLB_PIN_CURR))
                {
                    vcm_para.channel_no = i;
                    ret = vcm_dev_get_volt(vcm_id,vcm_para,&value);
                    if(ret != RESULT_OK)
                    {
                        return RESULT_ERROR;
                    }
                    status->volt[i] = value * cfg->channel_cfg[i].volt_split_factor;
                }
            }
            /* fix bug44386, liangf, 2017-06-21 */
            vcm_para.channel_no = 9; /* LTC2991_MAX_VOLTAGE */
            vcm_dev_trigger(vcm_id, vcm_para);  
            
            for(i=0; i<MAX_VCM_CHANNEL_NUM; i++ )
            {
                if(cfg->channel_cfg[i].flag & GLB_PIN_CURR)
                {
                    float32 volt1,volt2;
                    volt1 = status->volt[cfg->channel_cfg[i].curr_depend_pin];
                    volt2 = status->volt[i];
                    if (volt1 > volt2)
                        status->curr[i] = (volt1 - volt2)/vcm_info->vcm_rsense;
                    else
                        status->curr[i] = (volt2 - volt1)/vcm_info->vcm_rsense;
                }
            }

            break;
        case VCM_BMR464:
        case VCM_MAX20743:
            for(i=0; i<MAX_VCM_CHANNEL_NUM; i++ )
            {
                if(cfg->channel_cfg[i].flag & GLB_PIN_VOLT_OUT)
                {
                    vcm_para.channel_no = i;
                    vcm_para.type = VOUT;
                    ret = vcm_dev_get_volt(vcm_id,vcm_para,&value);
                    if(ret != RESULT_OK)
                    {
                        return RESULT_ERROR;
                    }
                    status->volt[i] = value;
                }
                else if(cfg->channel_cfg[i].flag & GLB_PIN_VOLT_IN)
                {
                    vcm_para.channel_no = i;
                    vcm_para.type = VIN;
                    ret = vcm_dev_get_volt(vcm_id,vcm_para,&value);
                    if(ret != RESULT_OK)
                    {
                        return RESULT_ERROR;
                    }
                    status->volt[i] = value;
                }
                else if(cfg->channel_cfg[i].flag & GLB_PIN_CURR_OUT)
                {
                    vcm_para.channel_no = i;
                    vcm_para.type = COUT;
                    ret = vcm_dev_get_cur(vcm_id,vcm_para,&value);
                    if(ret != RESULT_OK)
                    {
                        return RESULT_ERROR;
                    }
                    status->curr[i] = value;
                }
                else if(cfg->channel_cfg[i].flag & GLB_PIN_TMPR)
                {
                    vcm_para.channel_no = i;
                    vcm_para.type = TMPR;
                    ret = vcm_dev_get_temp(vcm_id,vcm_para,&value);
                    if(ret != RESULT_OK)
                    {
                        return RESULT_ERROR;
                    }
                    status->tmpr[i] = value;
                }                
            }
            break;
        default:
            DRV_LOG_ERR("Unsupport vcm chip type %d!\n", vcm_info->vcm_chip_type);
            return RESULT_ERROR;
    }
    
    return ret;
}

/*********************************************************************
 * Name    : vcm_init
 * Purpose :  init some data structure and register vcm dev
 * Input   : i2c_gen_t *i2c_gen     - some info about the way of vcm'i2c bus implement
          uint32 num         - the number of vcm
 * Output  : N/A
 * Return  : RESULT_OK   = SUCCESS
           other           = ErrCode
 * Note    : N/A
*********************************************************************/
int32
vcm_init(vcm_info_t* vcm_info, uint32 num)
{
    int32 i = 0;

    g_vcm_hdl = (vcm_handle_t* )DRV_MALLOC(CTCLIB_MEM_DRIVER_VCM_INFO, sizeof(vcm_handle_t)*num);
    if(NULL == g_vcm_hdl)
    {
        DRV_LOG_ERR("vcm alloc handler fail!\n");
        return RESULT_ERROR;
    }

    /* register vcm dev */
    for (i = 0; i < num; i++)
    {
        switch(vcm_info[i].vcm_chip_type)
        {
            case VCM_LTC2991_CMS:
            case VCM_LTC2991_LMS:
            case VCM_LTC2991_IMS:
                vcm_ltc2991_register_driver(&g_vcm_hdl[i], &vcm_info[i]);
                break;
            case VCM_BMR464:
                vcm_bmr464_register_driver(&g_vcm_hdl[i], &vcm_info[i]);
                break;
            case VCM_MAX20743:
                vcm_max20743_register_driver(&g_vcm_hdl[i], &vcm_info[i]);
                break;
            default:
                DRV_LOG_ERR("Unsupport vcm chip type %d!\n", vcm_info[i].vcm_chip_type);
                return RESULT_ERROR;
        }
    }

    g_vcm_nums = num;
    return RESULT_OK;
}

