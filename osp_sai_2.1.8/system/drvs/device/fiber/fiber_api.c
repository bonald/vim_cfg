/****************************************************************************
* $Id$
* fiber module driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-09-9
* Reason        : First Create.
****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "fiber_api.h"
#include "ctc_i2c.h"
#include "genlog.h"
#include "drv_debug.h"
#include "glb_phy_define.h"
#include "phy_api.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/

#define SFP_A0_REG_LEN  95
#define SFP_A2_REG_LEN  24
#define XFP_REG_LEN     221
#define QSFP_REG_LEN    95
#define SFP_ALARM_THRESHOLD_REG 0
#define SFP_ALARM_THRESHOLD_LEN 40
#define SFP_REALTIME_LEN 10 //other 6 byte reserved for future, not read currently.
#define SFP_LENGTH_LEN 5 
#define SFP_WAVELENGTH_LEN 2
#define SFP_AD_READOUT_LEN 14
#define SFP_THRESHOLD_LEN  40


#define XFP_ALARM_THRESHOLD_REG 2
#define XFP_ALARM_THRESHOLD_LEN 56
#define XFP_WAVELEN_LEN 2
#define XFP_LENGTH_LEN 5
#define XFP_AD_READOUT_LEN 14
#define XFP_THRESHOLD_LEN  58

#define QSFP_ALARM_THRESHOLD_REG 2
#define QSFP_ALARM_THRESHOLD_LEN 56
#define QSFP_WAVELEN_LEN 2
#define QSFP_LENGTH_LEN 5
#define QSFP_AD_READOUT_LEN 64
#define QSFP_THRESHOLD_LEN  80

/*A0*/
#define SFP_COMPLIANCE_CODE_REG 3
#define SFP_EXT_COMPLIANCE_CODE_REG 36
#define SFP_NOMINAL_BIT_RATE_REG 12
#define SFP_LENGTH_REG 14
#define SFP_VENDOR_NAME_REG 20
#define SFP_VENDOR_OUI_REG 37
#define SFP_VENDOR_PN_REG 40
#define SFP_VENDOR_REV_REG 56
#define SFP_WAVELENGTH_REG 60
#define SFP_OPTION_REG 65
#define SFP_VENDOR_SN_REG 68
#define SFP_DATE_CODE_REG 84
#define SFP_DIAG_TYPE_REG 92
#define SFP_REALTIME_REG 96
#define SFP_BR_MAX_REG 66

/*A2*/
#define SFP_THRESHOLD_START_ADDR    0
#define SFP_TEMP_HIGH_ALARM_REG     (0-SFP_THRESHOLD_START_ADDR)
#define SFP_VCC_HIGH_ALARM_REG      (8-SFP_THRESHOLD_START_ADDR)
#define SFP_TX_BIAS_HIGH_ALARM_REG  (16-SFP_THRESHOLD_START_ADDR)
#define SFP_TX_PWR_HIGH_ALARM_REG   (24-SFP_THRESHOLD_START_ADDR)
#define SFP_RX_PWR_HIGH_ALARM_REG   (32-SFP_THRESHOLD_START_ADDR)
#define SFP_REALTIME_START_ADDR     96
#define SFP_REALTIME_TEMP_REG       (96-SFP_REALTIME_START_ADDR)
#define SFP_REALTIME_BIAS_REG       (100-SFP_REALTIME_START_ADDR)
#define SFP_REALTIME_TX_PWR_REG     (102-SFP_REALTIME_START_ADDR)
#define SFP_REALTIME_RX_PWR_REG     (104-SFP_REALTIME_START_ADDR)
#define SFP_REALTIME_AUX1_REG       (98-SFP_REALTIME_START_ADDR)
#define SFP_CAL_CONST_REG 56
#define SFP_TUNABLE_START_ADDR                128
#define SFP_TUNABLE_ABILITY_REG               (128-SFP_TUNABLE_START_ADDR)
#define SFP_TUNABLE_CAPABILITY_REG            (132-SFP_TUNABLE_START_ADDR)
#define SFP_TUNABLE_CHANNEL_SET_REG           144
#define SFP_TUNABLE_WAVELENGTH_SET_REG        146
#define SFP_TUNABLE_DITHER_EN_REG             (151-SFP_TUNABLE_START_ADDR)


/*A0*/
#define XFP_THRESHOLD_START_ADDR    0
#define XFP_TEMP_HIGH_ALARM_REG     (2-XFP_THRESHOLD_START_ADDR)
#define XFP_BIAS_HIGH_ALARM_REG     (18-XFP_THRESHOLD_START_ADDR)
#define XFP_TX_PWR_HIGH_ALARM_REG   (26-XFP_THRESHOLD_START_ADDR)
#define XFP_RX_PWR_HIGH_ALARM_REG   (34-XFP_THRESHOLD_START_ADDR)
#define XFP_AUX1_HIGH_ALARM_REG     (42-XFP_THRESHOLD_START_ADDR)
#define XFP_REALTIME_START_ADDR     96
#define XFP_REALTIME_TEMP_REG       (96-XFP_REALTIME_START_ADDR)
#define XFP_REALTIME_BIAS_REG       (100-XFP_REALTIME_START_ADDR)
#define XFP_REALTIME_TX_PWR_REG     (102-XFP_REALTIME_START_ADDR)
#define XFP_REALTIME_RX_PWR_REG     (104-XFP_REALTIME_START_ADDR)
#define XFP_REALTIME_AUX1_REG       (106-XFP_REALTIME_START_ADDR)
#define XFP_COMPLIANCE_CODE_REG 131
#define XFP_LENGTH_REG 142
#define XFP_VENDOR_NAME_REG 148
#define XFP_VENDOR_OUI_REG 165
#define XFP_VENDOR_PN_REG 168
#define XFP_VENDOR_REV_REG 184
#define XFP_WAVELEN_REG 186
#define XFP_VENDOR_SN_REG 196
#define XFP_DATE_CODE_REG 212
/*A0*/
#define QSFP_THRESHOLD_START_ADDR             128
#define QSFP_TEMP_HIGH_ALARM_REG              (128-QSFP_THRESHOLD_START_ADDR)
#define QSFP_VCC1_HIGH_ALARM_REG              (144-QSFP_THRESHOLD_START_ADDR)
#define QSFP_RX_PWR_HIGH_ALARM_REG            (176-QSFP_THRESHOLD_START_ADDR)
#define QSFP_TX_BIAS_HIGH_ALARM_REG           (184-QSFP_THRESHOLD_START_ADDR)
#define QSFP_TX_PWR_HIGH_ALARM_REG            (192-QSFP_THRESHOLD_START_ADDR)
#define QSFP_REALTIME_START_ADDR              22
#define QSFP_REALTIME_TEMP_REG                (22-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_VCC_REG                 (26-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL1_RX_PWR_REG     (34-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL2_RX_PWR_REG     (36-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL3_RX_PWR_REG     (38-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL4_RX_PWR_REG     (40-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL1_TX_BIAS_REG    (42-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL2_TX_BIAS_REG    (44-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL3_TX_BIAS_REG    (46-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL4_TX_BIAS_REG    (48-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL1_TX_PWR_REG     (50-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL2_TX_PWR_REG     (52-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL3_TX_PWR_REG     (54-QSFP_REALTIME_START_ADDR)
#define QSFP_REALTIME_CHANNEL4_TX_PWR_REG     (56-QSFP_REALTIME_START_ADDR)

#define QSFP_STATUS                            2
#define QSFP_BASIC_INFO_BASE                   131
#define QSFP_COMPLIANCE_CODE_REG               131-QSFP_BASIC_INFO_BASE
#define QSFP_LENGTH_REG                        142-QSFP_BASIC_INFO_BASE
#define QSFP_TECH_REG                          147-QSFP_BASIC_INFO_BASE
#define QSFP_VENDOR_NAME_REG                   148-QSFP_BASIC_INFO_BASE
#define QSFP_VENDOR_OUI_REG                    165-QSFP_BASIC_INFO_BASE
#define QSFP_VENDOR_PN_REG                     168-QSFP_BASIC_INFO_BASE
#define QSFP_VENDOR_REV_REG                    184-QSFP_BASIC_INFO_BASE
#define QSFP_WAVELEN_REG                       186-QSFP_BASIC_INFO_BASE
#define QSFP_EXT_COMPLIANCE_CODE_REG           192-QSFP_BASIC_INFO_BASE
#define QSFP_VENDOR_SN_REG                     196-QSFP_BASIC_INFO_BASE
#define QSFP_DATE_CODE_REG                     212-QSFP_BASIC_INFO_BASE

#define QSFP_POWER_INFO                         93
#define QSFP_HIGH_POWER_CLASS_EN_MASK           4
#define QSFP_EXTENDED_IDENTIFIER                129
#define QSFP_HIGH_POWER_CLASS_MASK              3

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
static uint32 g_fiber_num = 0;
static uint32 g_port_num;
static fiber_handle_t** g_fiber_hdl = NULL;
static fiber_port_info_t* g_fiber_port_info;
/* Added by liuht for bug40747, 2016-09-14 */
/* Some read/write IO such as fiber_ctc_chip_epld_write/fiber_ctc_chip_epld_read 
   must be mutex globally */
static sal_mutex_t* g_fiber_pmutex;

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
/*fix bug14824, add port fiber support check for all API */
static int32
check_port_support_fiber(uint32 port_id)
{
    if((g_port_num <= port_id) ||(g_fiber_port_info[port_id].fiber_flg == -1))
    {
        return 0;
    }
    return 1;
}

int32 
check_fiber_is_XFP(uint32 port_id)
{
    if((g_port_num <= port_id) ||(g_fiber_port_info[port_id].fiber_flg != E_FIBER_XFP))
    {
        return 0;
    }
    return 1;
}

int32 
check_fiber_is_QSFP(uint32 port_id)
{
    if((g_port_num <= port_id) ||(g_fiber_port_info[port_id].fiber_flg != E_FIBER_QSFP_P))
    {
        return 0;
    }
    return 1;
}


int32
check_fiber_support_ddm(fiber_info_t* p_fiber)
{
    if(p_fiber->ddm_support)
        return 1;
    else
        return 0;
}

/* added by tongzb for bug 43937 support tunable fiber */
int32
check_fiber_support_tunable(fiber_info_t* p_fiber)
{
    if(p_fiber->tunable_support)
        return 1;
    else
        return 0;
}

static fiber_handle_t* get_fiber_handle(uint32 port_id)
{
    uint32 fiber_id;
    fiber_handle_t* phdl;

    fiber_id = g_fiber_port_info[port_id].fiber_id;
    if (fiber_id >= g_fiber_num)
    {
        return NULL;
    }
    phdl = g_fiber_hdl[fiber_id];

    return phdl;
}

int32 fiber_read(uint32 port_id, fiber_para_t* ppara)
{
    fiber_handle_t* phdl;
    int32 ret;

    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }
    phdl = get_fiber_handle(port_id);
    
    if(NULL == phdl || NULL == ppara)
    {
        DRV_LOG_ERR("Port %d fiber handle is not created or parameter is error", port_id);
        return RESULT_ERROR;
    }
    /* Added by liuht for bug40747, 2016-09-14 */
    sal_mutex_lock(g_fiber_pmutex);
    ret = phdl->read(phdl, ppara);
    sal_mutex_unlock(g_fiber_pmutex);
    
    return ret;
}

int32 fiber_write(uint32 port_id, fiber_para_t* ppara)
{
    fiber_handle_t* phdl;
    int32 ret;

    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }
    phdl = get_fiber_handle(port_id);
    
    if(NULL == phdl || NULL == ppara)
    {
        DRV_LOG_ERR("Port %d fiber is not created or parameter is error", port_id);
        return RESULT_ERROR;
    }
    /* Added by liuht for bug40747, 2016-09-14 */
    sal_mutex_lock(g_fiber_pmutex);
    ret = phdl->write(phdl, ppara);
    sal_mutex_unlock(g_fiber_pmutex);

    return ret;
}

static int fiber_get_device_info(uint32 port_id, uint8 table, uint8 addr, uint8* buf, uint32 len)
{
    fiber_para_t fiber_para;
    int32 ret;

    fiber_para.subdev= table;
    fiber_para.offset = addr;
    fiber_para.len = len;
    fiber_para.val = buf;
    ret = fiber_read(port_id, &fiber_para);

    return ret;
}

static int32 fiber_select_page(uint32 port_id, uint8 page)
{
    fiber_para_t fiber_para;
    int32 ret = 0;
    
    fiber_para.subdev = FIBER_DEV_ADDR1;
    fiber_para.offset = 127;
    fiber_para.len = 1;
    fiber_para.val = &page;
    ret = fiber_write(port_id, &fiber_para);
    /* delay 40ms according SFF8436 */
    usleep(40000);
    return ret;
}

/* added by tongzb for bug 43937 support tunable fiber */
int32 fiber_get_wavelength_by_freq(uint32 port_id, uint32  *freq,fiber_info_t *fiber_info)
{
    int32 ret=0;
    uint8 page[1];
    uint8 read_buf[2];
    fiber_para_t fiber_read_para,page_para;

    if(fiber_info->tunable_support == 0)
    {
        DRV_LOG_ERR("SFP port %d not support tunable", port_id + 1);
        return -1;
    }

    page_para.subdev = FIBER_DEV_ADDR2;
    page_para.offset = 127;
    page_para.len = 1;
    page[0] = 2;
    page_para.val = page;
    ret = fiber_write(port_id, &page_para);
    /* delay 40ms according SFF8636 */
    usleep(40000);

    fiber_read_para.subdev = FIBER_DEV_ADDR2;
    fiber_read_para.offset = SFP_TUNABLE_CHANNEL_SET_REG;
    fiber_read_para.len = 2;
    fiber_read_para.val = read_buf;
    ret = fiber_read(port_id,&fiber_read_para);

    *freq = fiber_info->first_freq + ((double)(read_buf[0]<<8 | read_buf[1]) - 1.0)*fiber_info->step_freq;

    page[0] = 0;
    page_para.val = page;
    ret = fiber_write(port_id, &page_para);
    /* delay 40ms according SFF8636 */
    usleep(40000);
    return ret;
}

int32 fiber_set_wavelength(uint32 port_id, fiber_para_t *fiber_para, fiber_info_t *fiber_info)
{
    int32 ret=0;
    uint8 page[1];
    uint8 reg_buf[4];
    fiber_para_t page_para;

    if(fiber_info->tunable_support == 0)
    {
        DRV_LOG_ERR("SFP port %d not support tunable", port_id + 1);
        return -1;
    }

    page_para.subdev = FIBER_DEV_ADDR2;
    page_para.offset = 127;
    page_para.len = 1;
    page[0] = 2;
    page_para.val = page;
    ret = fiber_write(port_id, &page_para);
    /* delay 40ms according SFF8636 */
    usleep(40000);

    ret = fiber_write(port_id, fiber_para);
    fiber_get_device_info(port_id, FIBER_DEV_ADDR2, SFP_TUNABLE_CHANNEL_SET_REG, reg_buf, 4);
    fiber_info->wave_channel_idx = (uint32)((reg_buf[0]<<8)|reg_buf[1]);
    fiber_info->tunable_wavelength = (double)((reg_buf[2]<<8)|reg_buf[3]) * 0.05;
    
    page[0] = 0;
    page_para.val = page;
    ret = fiber_write(port_id, &page_para);
    /* delay 40ms according SFF8636 */
    usleep(40000);
    return ret;
}
int32 fiber_config_wavelength(uint32 port_id, uint32 value, fiber_tune_mode_t mode,fiber_info_t *fiber_info)
{
    int32 ret=0;
    uint32 temp;
    double freq_temp;
    fiber_para_t fiber_para;
    uint8 buf[2];

    if(fiber_info->tunable_support == 0)
    {
        DRV_LOG_ERR("SFP port %d not support tunable", port_id + 1);
        /* modified by shil for bug 44553, 2017-07-21 */
        return 0;
    }

    sal_memset(buf, 0, sizeof(buf));
    fiber_para.subdev = FIBER_DEV_ADDR2;
    fiber_para.len = 2;

    switch(mode)
    {
        case FREQ_MODE: /* through the channel to config the fiber, value = 19210 => 192.10THz*/
            if((fiber_info->tune_mode != CHANNEL_MODE)&&(fiber_info->tune_mode != WAVELENGTH_CHANNEL_MODE))
                return 0;
            freq_temp = (double)((value/100 + (value%100)*0.01));
            temp = 1 + (uint32)((freq_temp - fiber_info->first_freq)/fiber_info->step_freq);/* the channel */
            
            fiber_para.offset = SFP_TUNABLE_CHANNEL_SET_REG;
            buf[0] = (temp >> 8) & 0xff;
            buf[1] = temp & 0xff;
            fiber_para.val = buf;
            fiber_set_wavelength(port_id,&fiber_para,fiber_info);
            break;  
        case WAVELENGTH_MODE:
            if((fiber_info->tune_mode != WAVELENGTH_MODE)&&(fiber_info->tune_mode != WAVELENGTH_CHANNEL_MODE))
                return 0;
            temp = value * 20;
            fiber_para.offset = SFP_TUNABLE_WAVELENGTH_SET_REG;
            buf[0] = (temp >> 8) & 0xff;
            buf[1] = temp & 0xff;
            fiber_para.val = buf;
            fiber_set_wavelength(port_id,&fiber_para,fiber_info);
            break;
        case CHANNEL_MODE:
            if((fiber_info->tune_mode != CHANNEL_MODE)&&(fiber_info->tune_mode != WAVELENGTH_CHANNEL_MODE))
                return 0;
            fiber_para.offset = SFP_TUNABLE_CHANNEL_SET_REG;
            buf[0] = (value >> 8) & 0xff;
            buf[1] = value & 0xff;
            fiber_para.val = buf;
            fiber_set_wavelength(port_id,&fiber_para,fiber_info);
            break;    
        default:
            break;
    }
    return ret;

}

/* enable high power class depend on SFF 8636 */
static int32 fiber_high_power_class_enable(uint32 port_id, bool enable)
{
    fiber_para_t fiber_para;
    uint8 extended_identifier;
    uint8 power_info;
    int32 ret = 0;

    fiber_select_page(port_id, 0);

    /* Get class 5/6/7 power */
    fiber_para.subdev = FIBER_DEV_ADDR1;
    fiber_para.offset = QSFP_EXTENDED_IDENTIFIER;
    fiber_para.len = 1;
    fiber_para.val = &extended_identifier;
    ret = fiber_read(port_id, &fiber_para);
    if(ret<0)
    {
        return ret;
    }
    
    /* If class 5/6/7 power exceed 3.5W, need set 93.2 to 1 */
    if(extended_identifier & QSFP_HIGH_POWER_CLASS_MASK)
    {
        fiber_para.offset = QSFP_POWER_INFO;
        fiber_para.len = 1;
        fiber_para.val = &power_info;
        ret = fiber_read(port_id, &fiber_para);
        if(ret<0)
        {
            return ret;
        }
        if(enable)
            power_info |= QSFP_HIGH_POWER_CLASS_EN_MASK;
        else
            power_info &= ~QSFP_HIGH_POWER_CLASS_EN_MASK;
        
        fiber_para.val = &power_info;
        ret = fiber_write(port_id, &fiber_para);
        if(ret<0)
        {
            return ret;
        }
    }
    
    return ret;
}

int32 fiber_update_run_info_is_coper(uint32 port_id, uint8 is_coper)
{
    fiber_handle_t* phdl;

    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }
    
    phdl = get_fiber_handle(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    /*If fiber is absent, do nothing.*/
    if(FIBER_PRESENT != phdl->run_info.present)
    {
        return RESULT_OK;
    }
    phdl->run_info.is_coper = is_coper;
    
    return RESULT_OK;
}

static int32 fiber_get_cal_const(uint8 port_id, fiber_cal_const_t* buf)
{
    return fiber_get_device_info(port_id, FIBER_DEV_ADDR2, SFP_CAL_CONST_REG, (uint8 *)buf, sizeof(fiber_cal_const_t));
}

static int32 fiber_tmpr_ad_convert(int16 ad_value, fiber_info_t *fiber_info, double *p_value)
{
    fiber_cal_const_t *cal_const = &fiber_info->cal_const;
    uint16 t_slope = 0;
    int16 t_offset = 0;
    double tmpr;
    
    if (fiber_info->externally_calibrated)
    {
        /* external calibration, T = T_slope * T_ad + T_offset
         * note: slope is represented in fixed demcial way in which point is fixed between first and second byte*/
        t_slope = (((uint16)cal_const->t_slope[0])<<8)|(cal_const->t_slope[1]);
        t_offset = (((int16)cal_const->t_offset[0])<<8)|(cal_const->t_offset[1]);
        tmpr = ad_value*(((float)t_slope)/256) + t_offset;
    }
    else
    {
        tmpr = ad_value;
    }
    
    *p_value = tmpr/256;
    return 0;    
}

static int32 fiber_vcc_ad_convert(uint16 ad_value, fiber_info_t *fiber_info, double *p_value)
{
    fiber_cal_const_t *cal_const = &fiber_info->cal_const;
    uint16 v_slope = 0;
    int16 v_offset = 0;
    double vcc;
    
    if (fiber_info->externally_calibrated)
    {
         /* external calibration, V = V_slope * V_ad + V_offset
         * note: slope is represented in fixed demcial way in which point is fixed between first and second byte*/
        v_slope = (((uint16)cal_const->v_slope[0])<<8)|(cal_const->v_slope[1]);
        v_offset = (((int16)cal_const->v_offset[0])<<8)|(cal_const->v_offset[1]);
        vcc = ad_value * (((float)v_slope)/256) + v_offset;
    }
    else
    {
        vcc = ad_value;
    }
    
    /* per SFF-8472, value is in units of 100iV */
    *p_value = vcc * 0.1 * 0.001;
    return 0;    
}

static int32 fiber_curr_ad_convert(uint16 ad_value, fiber_info_t *fiber_info, double *p_value)
{
    fiber_cal_const_t *cal_const = &fiber_info->cal_const;
    uint16 current_slope = 0;
    int16 current_offset = 0;
    double current;
    
    if (fiber_info->externally_calibrated)
    {
        /* external calibration, I = I_slope * I_ad + I_offset
         * note: slope is represented in fixed demcial way in which point is fixed between first and second byte*/
        current_slope = ((uint16)(cal_const->tx_i_slope[0])<<8)|(cal_const->tx_i_slope[1]);  
        current_offset = ((int16)(cal_const->tx_i_offset[0])<<8)|(cal_const->tx_i_offset[1]);          
        current = ad_value * (((float)current_slope)/256) + current_offset;
    }
    else
    {
        current = ad_value;
    }

    /* per SFF-8472, value is in units of 2uA */
    *p_value = current * 0.002;

    return 0;    
}

static int32 fiber_txpwr_ad_convert(uint16 ad_value, fiber_info_t *fiber_info, double *p_value)
{
    fiber_cal_const_t *cal_const = &fiber_info->cal_const;
    uint16 tx_pwr_slope = 0;
    int16 tx_pwr_offset = 0;
    double tx_pwr = 0.0;
    
    if (fiber_info->externally_calibrated)
    {
        /* external calibration, TxP = TxP_slope * TxP_ad + TxP_offset
         * note: slope is represented in fixed demcial way in which point is fixed between first and second byte*/
        tx_pwr_slope = ((uint16)(cal_const->tx_pwr_slope[0])<<8)|(cal_const->tx_pwr_slope[1]);
        tx_pwr_offset = ((int16)(cal_const->tx_pwr_offset[0])<<8)|(cal_const->tx_pwr_offset[1]);
        tx_pwr = ((double)(ad_value * tx_pwr_slope))/256.0;
        tx_pwr = tx_pwr + tx_pwr_offset;                
        //tx_pwr = pwr*(((double)tx_pwr_slope)/256) + tx_pwr_offset;
    }
    else
    {
        tx_pwr = ad_value;
    }

    /* per SFF-8472, value is in units of 0.1iW */    
    if (tx_pwr <= 0) /* Modfied by liuht for bug42884, 2017-0306 */
    {
        /* 0iW ~ -40dBm */
        tx_pwr = -40.0;
    }
    else
    {
        /* converting from iW to dBm */
        tx_pwr = 10*log10(tx_pwr*0.0001);
    } 

    *p_value = tx_pwr;
    return 0;    
}

static int32 fiber_rxpwr_ad_convert(uint16 ad_value, fiber_info_t *fiber_info, double *p_value)
{
    fiber_cal_const_t *cal_const = &fiber_info->cal_const;
    uint16 pwr = ad_value;
    uint32 rx_pwr_int0 = 0;
    uint32 rx_pwr_int1 = 0;
    uint32 rx_pwr_int2 = 0;
    uint32 rx_pwr_int3 = 0;
    uint32 rx_pwr_int4 = 0;
    float rx_pwr0 = 0.0;
    float rx_pwr1 = 0.0;
    float rx_pwr2 = 0.0;
    float rx_pwr3 = 0.0;
    float rx_pwr4 = 0.0;
    double rx_pwr = 0.0;
    
    if (fiber_info->externally_calibrated)
    {
         /* external calibration, RxP = RxP(4)*RxP_ad^4 + RxP(3)*RxP_ad^3 + RxP(2)*RxP_ad^2 +RxP(1)*RxP_ad + RxP(0) */
        rx_pwr_int0 = ((uint32)(cal_const->rx_pwr0[0])<<24)|
                      ((uint32)(cal_const->rx_pwr0[1])<<16)|
                      ((uint32)(cal_const->rx_pwr0[2])<<8)|
                      ((uint32)(cal_const->rx_pwr0[3])<<0);
        rx_pwr_int1 = ((uint32)(cal_const->rx_pwr1[0])<<24)|
                      ((uint32)(cal_const->rx_pwr1[1])<<16)|
                      ((uint32)(cal_const->rx_pwr1[2])<<8)|
                      ((uint32)(cal_const->rx_pwr1[3])<<0);
        rx_pwr_int2 = ((uint32)(cal_const->rx_pwr2[0])<<24)|
                      ((uint32)(cal_const->rx_pwr2[1])<<16)|
                      ((uint32)(cal_const->rx_pwr2[2])<<8)|
                      ((uint32)(cal_const->rx_pwr2[3])<<0);
        rx_pwr_int3 = ((uint32)(cal_const->rx_pwr3[0])<<24)|
                      ((uint32)(cal_const->rx_pwr3[1])<<16)|
                      ((uint32)(cal_const->rx_pwr3[2])<<8)|
                      ((uint32)(cal_const->rx_pwr3[3])<<0);
        rx_pwr_int4 = ((uint32)(cal_const->rx_pwr4[0])<<24)|
                      ((uint32)(cal_const->rx_pwr4[1])<<16)|
                      ((uint32)(cal_const->rx_pwr4[2])<<8)|
                      ((uint32)(cal_const->rx_pwr4[3])<<0);  

        sal_memcpy(&rx_pwr0, &rx_pwr_int0, sizeof(uint32));
        sal_memcpy(&rx_pwr1, &rx_pwr_int1, sizeof(uint32));
        sal_memcpy(&rx_pwr2, &rx_pwr_int2, sizeof(uint32));
        sal_memcpy(&rx_pwr3, &rx_pwr_int3, sizeof(uint32));
        sal_memcpy(&rx_pwr4, &rx_pwr_int4, sizeof(uint32));
       
        rx_pwr = (rx_pwr4*pwr*pwr*pwr*pwr) + (rx_pwr3*pwr*pwr*pwr) + (rx_pwr2*pwr*pwr) 
                + (rx_pwr1*pwr) + rx_pwr0;
    }
    else
    {
        rx_pwr = pwr;
    }

    /* per SFF-8472, value is in units of 0.1iW */    
    if (rx_pwr <= 0) /* Modfied by liuht for bug42884, 2017-0306 */
    {
        /* 0iW ~ -40dBm */
        rx_pwr = -40.0;
    }
    else
    {
        /* converting from iW to dBm */
        rx_pwr = 10*log10(rx_pwr*0.0001);
    } 
    *p_value = rx_pwr;
    return 0;    
}

static int32 fiber_update_alarm_threshold(uint32 port_id, fiber_info_t* p_fiber)
{
    uint8 buf[80]; //large than SFP(40) or XFP(56) or QSFP(72) threshold len.
    uint8 *p_buf;
    uint8 addr;
    int16 ad_value = 0;
    int16 s_ad_value = 0;
    uint8 page, table_id, len;
    int8 tmpr_reg,vcc_reg,bias_reg,txpwr_reg,rxpwr_reg;
    int ret = 0;
    int i = 0;

    sal_memset(buf, 0, sizeof(buf));
    if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_XFP)
    {
        table_id = FIBER_DEV_ADDR1;
        tmpr_reg = XFP_TEMP_HIGH_ALARM_REG;
        vcc_reg = XFP_AUX1_HIGH_ALARM_REG;
        bias_reg = XFP_BIAS_HIGH_ALARM_REG;
        txpwr_reg = XFP_TX_PWR_HIGH_ALARM_REG;
        rxpwr_reg = XFP_RX_PWR_HIGH_ALARM_REG;
        addr = XFP_THRESHOLD_START_ADDR;
        len = XFP_THRESHOLD_LEN;
    }
    else if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
    {
        table_id = FIBER_DEV_ADDR1;
        page = 3;
        tmpr_reg = QSFP_TEMP_HIGH_ALARM_REG;
        vcc_reg = QSFP_VCC1_HIGH_ALARM_REG;
        bias_reg = QSFP_TX_BIAS_HIGH_ALARM_REG;
        txpwr_reg = QSFP_TX_PWR_HIGH_ALARM_REG;
        rxpwr_reg = QSFP_RX_PWR_HIGH_ALARM_REG;
        addr = QSFP_THRESHOLD_START_ADDR;
        len = QSFP_THRESHOLD_LEN;
        fiber_select_page(port_id, page);
    }
    else
    {
        table_id = FIBER_DEV_ADDR2;
        tmpr_reg = SFP_TEMP_HIGH_ALARM_REG;
        vcc_reg = SFP_VCC_HIGH_ALARM_REG;
        bias_reg = SFP_TX_BIAS_HIGH_ALARM_REG;
        txpwr_reg = SFP_TX_PWR_HIGH_ALARM_REG;
        rxpwr_reg = SFP_RX_PWR_HIGH_ALARM_REG;
        addr = SFP_THRESHOLD_START_ADDR;
        len = SFP_THRESHOLD_LEN;
    }
    ret = fiber_get_device_info(port_id, table_id, addr, buf, len);    
    if (ret)
    {
        sal_memset(p_fiber->tmpr, 0, sizeof(p_fiber->tmpr));
        sal_memset(p_fiber->voltage, 0, sizeof(p_fiber->voltage));
        sal_memset(p_fiber->bias, 0, sizeof(p_fiber->bias));
        sal_memset(p_fiber->tx_pwr, 0, sizeof(p_fiber->tx_pwr));
        sal_memset(p_fiber->rx_pwr, 0, sizeof(p_fiber->rx_pwr));
    }
    else
    {        
        /* get temperature threshold */
        for (i = FIBER_HIGH_ALARM_VALUE; i <= FIBER_LOW_WARN_VALUE; i++)
        {
            if(tmpr_reg<0)
            {
                p_fiber->tmpr[i] = -1;
                continue;
            }
            p_buf = &(buf[i*2 + tmpr_reg]);            
            s_ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
            fiber_tmpr_ad_convert(s_ad_value, p_fiber, &p_fiber->tmpr[i]);
        }

        /* get supply voltage threshold */
        for (i = FIBER_HIGH_ALARM_VALUE; i <= FIBER_LOW_WARN_VALUE; i++)
        {
            if(vcc_reg<0)
            {
                p_fiber->voltage[i] = -1;
                continue;
            }
            p_buf = &(buf[i*2 + vcc_reg]);
            ad_value = ((uint16)p_buf[0]<<8)|p_buf[1];
            fiber_vcc_ad_convert(ad_value, p_fiber, &p_fiber->voltage[i]);
        }

        /* get bias threshold */
        for (i = FIBER_HIGH_ALARM_VALUE; i <= FIBER_LOW_WARN_VALUE; i++)
        {
            if(bias_reg<0)
            {
                p_fiber->bias[i] = -1;
                continue;
            }
            p_buf = &(buf[i*2 + bias_reg]);
            ad_value = ((uint16)p_buf[0]<<8)|p_buf[1];
            fiber_curr_ad_convert(ad_value, p_fiber, &p_fiber->bias[i]);
        }

        /* get tx power threshold */
        for (i = FIBER_HIGH_ALARM_VALUE; i <= FIBER_LOW_WARN_VALUE; i++)
        {
            if(txpwr_reg<0)
            {
                p_fiber->tx_pwr[i] = -1;
                continue;
            }
            p_buf = &(buf[i*2 + txpwr_reg]);
            ad_value = ((uint16)p_buf[0]<<8)|p_buf[1];
            fiber_txpwr_ad_convert(ad_value, p_fiber, &p_fiber->tx_pwr[i]);
        }

        /* get rx power threshold */
        for (i = FIBER_HIGH_ALARM_VALUE; i <= FIBER_LOW_WARN_VALUE; i++)
        {
            if(rxpwr_reg<0)
            {
                p_fiber->rx_pwr[i] = -1;
                continue;
            }
            p_buf = &(buf[i*2 + rxpwr_reg]);
            ad_value = ((uint16)p_buf[0]<<8)|p_buf[1];
            fiber_rxpwr_ad_convert(ad_value, p_fiber, &p_fiber->rx_pwr[i]);
        }
    }

    if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
    {
        page = 0;
        fiber_select_page(port_id, page); 
    }
    
    return 0;
}

static int32 fiber_update_realtime(uint32 port_id, fiber_info_t* p_fiber)
{
    uint8 buf[64];
    uint8 *p_buf;
    uint8 addr;
    int16 ad_value = 0;
    int16 s_ad_value = 0;
    int32 table_id, len, tmpr_reg,vcc_reg,bias_reg[FIBER_VAL_TYPE_MAX2],txpwr_reg[FIBER_VAL_TYPE_MAX2],rxpwr_reg[FIBER_VAL_TYPE_MAX2];
    uint8 i;
    int ret = 0;

    sal_memset(buf, 0, sizeof(buf));
    if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_XFP)
    {
        table_id = FIBER_DEV_ADDR1;
        tmpr_reg = XFP_REALTIME_TEMP_REG;
        vcc_reg = XFP_REALTIME_AUX1_REG;
        bias_reg[FIBER_CURRENT_VALUE1] = XFP_REALTIME_BIAS_REG;
        bias_reg[FIBER_CURRENT_VALUE2] = -1;
        bias_reg[FIBER_CURRENT_VALUE3] = -1;
        bias_reg[FIBER_CURRENT_VALUE4] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE1] = XFP_REALTIME_TX_PWR_REG;
        txpwr_reg[FIBER_CURRENT_VALUE2] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE3] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE4] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE1] = XFP_REALTIME_RX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE2] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE3] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE4] = -1;
        addr = XFP_REALTIME_START_ADDR;
        len = XFP_AD_READOUT_LEN;
    }
    else if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
    {
        table_id = FIBER_DEV_ADDR1;
        tmpr_reg = QSFP_REALTIME_TEMP_REG;
        vcc_reg = QSFP_REALTIME_VCC_REG;
        bias_reg[FIBER_CURRENT_VALUE1] = QSFP_REALTIME_CHANNEL1_TX_BIAS_REG;
        bias_reg[FIBER_CURRENT_VALUE2] = QSFP_REALTIME_CHANNEL2_TX_BIAS_REG;
        bias_reg[FIBER_CURRENT_VALUE3] = QSFP_REALTIME_CHANNEL3_TX_BIAS_REG;
        bias_reg[FIBER_CURRENT_VALUE4] = QSFP_REALTIME_CHANNEL4_TX_BIAS_REG;
        txpwr_reg[FIBER_CURRENT_VALUE1] = QSFP_REALTIME_CHANNEL1_TX_PWR_REG;
        txpwr_reg[FIBER_CURRENT_VALUE2] = QSFP_REALTIME_CHANNEL2_TX_PWR_REG;
        txpwr_reg[FIBER_CURRENT_VALUE3] = QSFP_REALTIME_CHANNEL3_TX_PWR_REG;
        txpwr_reg[FIBER_CURRENT_VALUE4] = QSFP_REALTIME_CHANNEL4_TX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE1] = QSFP_REALTIME_CHANNEL1_RX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE2] = QSFP_REALTIME_CHANNEL2_RX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE3] = QSFP_REALTIME_CHANNEL3_RX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE4] = QSFP_REALTIME_CHANNEL4_RX_PWR_REG;
        addr = QSFP_REALTIME_START_ADDR;
        len = QSFP_AD_READOUT_LEN;
    }
    else
    {
        table_id = FIBER_DEV_ADDR2;
        tmpr_reg = SFP_REALTIME_TEMP_REG;
        vcc_reg = SFP_REALTIME_AUX1_REG;
        bias_reg[FIBER_CURRENT_VALUE1] = SFP_REALTIME_BIAS_REG;
        bias_reg[FIBER_CURRENT_VALUE2] = -1;
        bias_reg[FIBER_CURRENT_VALUE3] = -1;
        bias_reg[FIBER_CURRENT_VALUE4] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE1] = SFP_REALTIME_TX_PWR_REG;
        txpwr_reg[FIBER_CURRENT_VALUE2] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE3] = -1;
        txpwr_reg[FIBER_CURRENT_VALUE4] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE1] = SFP_REALTIME_RX_PWR_REG;
        rxpwr_reg[FIBER_CURRENT_VALUE2] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE3] = -1;
        rxpwr_reg[FIBER_CURRENT_VALUE4] = -1;
        addr = SFP_REALTIME_START_ADDR;
        len = SFP_AD_READOUT_LEN;
    }
    ret = fiber_get_device_info(port_id, table_id, addr, buf, len);    
    if (ret)
    {
        p_fiber->tmpr2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tmpr2[FIBER_CURRENT_VALUE2] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tmpr2[FIBER_CURRENT_VALUE3] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tmpr2[FIBER_CURRENT_VALUE4] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->voltage2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->voltage2[FIBER_CURRENT_VALUE2] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->voltage2[FIBER_CURRENT_VALUE3] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->voltage2[FIBER_CURRENT_VALUE4] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->bias2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->bias2[FIBER_CURRENT_VALUE2] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->bias2[FIBER_CURRENT_VALUE3] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->bias2[FIBER_CURRENT_VALUE4] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tx_pwr2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tx_pwr2[FIBER_CURRENT_VALUE2] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tx_pwr2[FIBER_CURRENT_VALUE3] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->tx_pwr2[FIBER_CURRENT_VALUE4] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->rx_pwr2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->rx_pwr2[FIBER_CURRENT_VALUE2] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->rx_pwr2[FIBER_CURRENT_VALUE3] = GLB_FIBER_INFO_INVALID_FLAG;
        p_fiber->rx_pwr2[FIBER_CURRENT_VALUE4] = GLB_FIBER_INFO_INVALID_FLAG;
    }
    else
    {
        /* get real time temperature */
        if(tmpr_reg<0)
        {
            p_fiber->tmpr2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        }
        p_buf = &(buf[tmpr_reg]);            
        s_ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
        fiber_tmpr_ad_convert(s_ad_value, p_fiber, &p_fiber->tmpr2[FIBER_CURRENT_VALUE1]);
        p_fiber->tmpr2[FIBER_CURRENT_VALUE2] = p_fiber->tmpr2[FIBER_CURRENT_VALUE1];
        p_fiber->tmpr2[FIBER_CURRENT_VALUE3] = p_fiber->tmpr2[FIBER_CURRENT_VALUE1];
        p_fiber->tmpr2[FIBER_CURRENT_VALUE4] = p_fiber->tmpr2[FIBER_CURRENT_VALUE1];

        /* get real time supply voltage */
        if(vcc_reg<0)
        {
            p_fiber->voltage2[FIBER_CURRENT_VALUE1] = GLB_FIBER_INFO_INVALID_FLAG;
        }
        p_buf = &(buf[vcc_reg]);
        ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
        fiber_vcc_ad_convert(ad_value, p_fiber, &p_fiber->voltage2[FIBER_CURRENT_VALUE1]);
        p_fiber->voltage2[FIBER_CURRENT_VALUE2] = p_fiber->voltage2[FIBER_CURRENT_VALUE1];
        p_fiber->voltage2[FIBER_CURRENT_VALUE3] = p_fiber->voltage2[FIBER_CURRENT_VALUE1];
        p_fiber->voltage2[FIBER_CURRENT_VALUE4] = p_fiber->voltage2[FIBER_CURRENT_VALUE1];

        /* get real time bias */
        for (i = FIBER_CURRENT_VALUE1; i <= FIBER_CURRENT_VALUE4; i++)
        {
            if(bias_reg[i]<0)
            {
                p_fiber->bias2[i] = GLB_FIBER_INFO_INVALID_FLAG;
                continue;
            }
            p_buf = &(buf[bias_reg[i]]);            
            s_ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
            fiber_curr_ad_convert(s_ad_value, p_fiber, &p_fiber->bias2[i]);
        }

        /* get real time tx power */
        for (i = FIBER_CURRENT_VALUE1; i <= FIBER_CURRENT_VALUE4; i++)
        {
            if(txpwr_reg[i]<0)
            {
                p_fiber->tx_pwr2[i] = GLB_FIBER_INFO_INVALID_FLAG;
                continue;
            }
            p_buf = &(buf[txpwr_reg[i]]);            
            s_ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
            fiber_txpwr_ad_convert(s_ad_value, p_fiber, &p_fiber->tx_pwr2[i]);
        }

        /* get real time rx power */
        for (i = FIBER_CURRENT_VALUE1; i <= FIBER_CURRENT_VALUE4; i++)
        {
            if(rxpwr_reg[i]<0)
            {
                p_fiber->rx_pwr2[i] = GLB_FIBER_INFO_INVALID_FLAG;
                continue;
            }
            p_buf = &(buf[rxpwr_reg[i]]);            
            s_ad_value = ((int16)p_buf[0]<<8)|p_buf[1];
            fiber_rxpwr_ad_convert(s_ad_value, p_fiber, &p_fiber->rx_pwr2[i]);
        }
    }

    p_fiber->updated = TRUE;
    
    return 0;
}

int32 fiber_update_state(uint32 port_id, fiber_info_t* p_fiber)
{
    int32 ret;
    if(!p_fiber)
    {
        return -1;
    }
    if(!check_port_support_fiber(port_id))
    {
        return -1;
    }
    if(p_fiber->ddm_support)
    {
        ret = fiber_update_realtime(port_id, p_fiber);
        return ret;
    }
    return -1;
}

/* Added by kcao for bug 8045 2009-01-19: fix Agilent HFBR 57M5AP SFP first Byte read ZERO problem */
static int32 sfp_get_chip_info_from_buf(uint32 port_id, fiber_info_t* p_fiber)
{
    uint8 buf[SFP_A0_REG_LEN];
    double temp1,temp2;
    fiber_para_t fiber_para;
    uint8 page;

    sal_memset(buf, 0, sizeof(buf));

    if (fiber_get_device_info(port_id, FIBER_DEV_ADDR1, 0, buf, SFP_A0_REG_LEN) < 0)
    {
        p_fiber->ddm_support = 0;
        DRV_LOG_ERR("SFP port %d get MSA failed", port_id + 1);
        return -1;
    }
    p_fiber->speed = buf[SFP_NOMINAL_BIT_RATE_REG];
    p_fiber->br_max = buf[SFP_BR_MAX_REG];
    sal_memcpy(p_fiber->name, &buf[SFP_VENDOR_NAME_REG], FIBER_VENDOR_NAME_LEN);
    sal_memcpy(p_fiber->oui, &buf[SFP_VENDOR_OUI_REG], FIBER_VENDOR_OUI_LEN);
    sal_memcpy(p_fiber->pn, &buf[SFP_VENDOR_PN_REG], FIBER_VENDOR_PN_LEN);
    sal_memcpy(p_fiber->rev, &buf[SFP_VENDOR_REV_REG], FIBER_VENDOR_REV_LEN);
    sal_memcpy(p_fiber->sn, &buf[SFP_VENDOR_SN_REG], FIBER_VENDOR_SN_LEN);

    p_fiber->tunable_support = buf[SFP_OPTION_REG] & 0x40;/* for bug 43937 */
    p_fiber->date_code.year[0] = buf[SFP_DATE_CODE_REG];
    p_fiber->date_code.year[1] = buf[SFP_DATE_CODE_REG+1];
    p_fiber->date_code.year[2] = '\0';
    p_fiber->date_code.month[0] = buf[SFP_DATE_CODE_REG+2];
    p_fiber->date_code.month[1] = buf[SFP_DATE_CODE_REG+3];
    p_fiber->date_code.month[2] = '\0';
    p_fiber->date_code.day[0] = buf[SFP_DATE_CODE_REG+4];
    p_fiber->date_code.day[1] = buf[SFP_DATE_CODE_REG+5];
    p_fiber->date_code.day[2] = '\0';
    p_fiber->date_code.lot[0] = buf[SFP_DATE_CODE_REG+6];
    p_fiber->date_code.lot[1] = buf[SFP_DATE_CODE_REG+7];
    p_fiber->date_code.lot[2] = '\0';    
    p_fiber->ext_compliance_code = buf[SFP_EXT_COMPLIANCE_CODE_REG];
    sal_memcpy(p_fiber->compliance_code, &buf[SFP_COMPLIANCE_CODE_REG], FIBER_COMPLIANCE_CODE_LEN);
    sal_memcpy(p_fiber->wavelength, &buf[SFP_WAVELENGTH_REG], SFP_WAVELENGTH_LEN);
    sal_memcpy(p_fiber->length, &buf[SFP_LENGTH_REG], SFP_LENGTH_LEN);
    if((buf[SFP_DIAG_TYPE_REG]&0x40) != 0)
    {
        if(fiber_get_cal_const(port_id, &(p_fiber->cal_const)) == RESULT_OK)
        {
            p_fiber->ddm_support = 1;
            if((buf[SFP_DIAG_TYPE_REG]&0x10) != 0)
            {
                p_fiber->externally_calibrated = 1;
            }
        }
    }
    
    if(p_fiber->ddm_support)
    {
        fiber_update_alarm_threshold(port_id, p_fiber);
        fiber_update_realtime(port_id, p_fiber);
    }
    p_fiber->channel_num = 1;
    
    if(p_fiber->tunable_support)
    {
        sal_memset(buf, 0, sizeof(buf));
        fiber_para.subdev = FIBER_DEV_ADDR2;
        fiber_para.offset = 127;
        fiber_para.len = 1;
        page = 2;
        fiber_para.val = &page;
        fiber_write(port_id, &fiber_para);
        /* delay 40ms according SFF8636 */
        usleep(40000);
        p_fiber->tune_mode = TUNE_NONE_MODE;
        if (fiber_get_device_info(port_id, FIBER_DEV_ADDR2, SFP_TUNABLE_START_ADDR, buf, SFP_A2_REG_LEN) < 0)
        {
            p_fiber->tunable_support = 0;
            DRV_LOG_ERR("SFP port %d get TUNABLE failed", port_id + 1);
            return -1;
        }
        page = 0;
        fiber_para.val = &page;
        fiber_write(port_id, &fiber_para);
        /* delay 40ms according SFF8636 */
        usleep(40000);

        p_fiber->dither_support = buf[SFP_TUNABLE_ABILITY_REG] & 0x04;
        if((buf[SFP_TUNABLE_ABILITY_REG] & 0x1) && (buf[SFP_TUNABLE_ABILITY_REG] & 0x2))
            p_fiber->tune_mode = WAVELENGTH_CHANNEL_MODE;
        else if(buf[SFP_TUNABLE_ABILITY_REG] & 0x1)
            p_fiber->tune_mode = WAVELENGTH_MODE;
        else if(buf[SFP_TUNABLE_ABILITY_REG] & 0x2)
            p_fiber->tune_mode = CHANNEL_MODE;
        temp1= (double)((buf[SFP_TUNABLE_CAPABILITY_REG]<<8 )|buf[SFP_TUNABLE_CAPABILITY_REG+1]);
        temp2= (double)((buf[SFP_TUNABLE_CAPABILITY_REG+2]<<8 )|buf[SFP_TUNABLE_CAPABILITY_REG+3]);
        p_fiber->first_freq = temp1 + temp2*0.0001;
        
        temp1= (double)((buf[SFP_TUNABLE_CAPABILITY_REG+4]<<8 )|buf[SFP_TUNABLE_CAPABILITY_REG+5]);
        temp2= (double)((buf[SFP_TUNABLE_CAPABILITY_REG+6]<<8 )|buf[SFP_TUNABLE_CAPABILITY_REG+7]);
        p_fiber->last_freq = temp1 + temp2*0.0001;
        
        temp2= (double)((buf[SFP_TUNABLE_CAPABILITY_REG+8]<<8 )|buf[SFP_TUNABLE_CAPABILITY_REG+9]);
        p_fiber->step_freq = temp2*0.0001;
        if(p_fiber->step_freq == 0)
        {
            p_fiber->tunable_support = 0;
            DRV_LOG_ERR("SFP port %d get TUNABLE invalid step freq", port_id + 1);
            return -1;
        }
        p_fiber->wave_channels = 1 + (uint32)((p_fiber->last_freq - p_fiber->first_freq)/p_fiber->step_freq);
        p_fiber->wave_channel_idx = (uint32)((buf[SFP_TUNABLE_CHANNEL_SET_REG-SFP_TUNABLE_START_ADDR]<<8)|
                    buf[SFP_TUNABLE_CHANNEL_SET_REG-SFP_TUNABLE_START_ADDR  +1]);
        p_fiber->tunable_wavelength = (double)((buf[SFP_TUNABLE_WAVELENGTH_SET_REG-SFP_TUNABLE_START_ADDR]<<8)|
                    buf[SFP_TUNABLE_WAVELENGTH_SET_REG-SFP_TUNABLE_START_ADDR  +1]) * 0.05;

        p_fiber->dither_support = buf[SFP_TUNABLE_DITHER_EN_REG] & 0x01;

    }


    return 0;
}

static int32 xfp_get_chip_info_from_buf(uint32 port_id, fiber_info_t* p_fiber)
{
    uint8 buf[XFP_REG_LEN];
    uint16 wavelength;
    
    sal_memset(buf, 0, sizeof(buf));

    if (fiber_get_device_info(port_id, FIBER_DEV_ADDR1, 0, buf, XFP_REG_LEN) < 0)
    {
        DRV_LOG_ERR("XFP port %d get MSA failed", port_id + 1);
        return -1;
    }
    sal_memcpy(p_fiber->name, &buf[XFP_VENDOR_NAME_REG], FIBER_VENDOR_NAME_LEN);
    sal_memcpy(p_fiber->oui, &buf[XFP_VENDOR_OUI_REG], FIBER_VENDOR_OUI_LEN);
    sal_memcpy(p_fiber->pn, &buf[XFP_VENDOR_PN_REG], FIBER_VENDOR_PN_LEN);
    sal_memcpy(p_fiber->rev, &buf[XFP_VENDOR_REV_REG], FIBER_VENDOR_REV_LEN);
    sal_memcpy(p_fiber->sn, &buf[XFP_VENDOR_SN_REG], FIBER_VENDOR_SN_LEN);

    p_fiber->date_code.year[0] = buf[XFP_DATE_CODE_REG];
    p_fiber->date_code.year[1] = buf[XFP_DATE_CODE_REG+1];
    p_fiber->date_code.year[2] = '\0';
    p_fiber->date_code.month[0] = buf[XFP_DATE_CODE_REG+2];
    p_fiber->date_code.month[1] = buf[XFP_DATE_CODE_REG+3];
    p_fiber->date_code.month[2] = '\0';
    p_fiber->date_code.day[0] = buf[XFP_DATE_CODE_REG+4];
    p_fiber->date_code.day[1] = buf[XFP_DATE_CODE_REG+5];
    p_fiber->date_code.day[2] = '\0';
    p_fiber->date_code.lot[0] = buf[XFP_DATE_CODE_REG+6];
    p_fiber->date_code.lot[1] = buf[XFP_DATE_CODE_REG+7];
    p_fiber->date_code.lot[2] = '\0';
    sal_memcpy(p_fiber->compliance_code, &buf[XFP_COMPLIANCE_CODE_REG], FIBER_COMPLIANCE_CODE_LEN);
    wavelength = (buf[XFP_WAVELEN_REG]<<8) + buf[XFP_WAVELEN_REG+1];
    wavelength = wavelength/20;
    p_fiber->wavelength[0] = ((wavelength>>8)&0xff);
    p_fiber->wavelength[1] = (wavelength & 0xff);
    sal_memcpy(p_fiber->length, &buf[XFP_LENGTH_REG], XFP_LENGTH_LEN);

    /*For XFP, all support ddm, and just use internally calibrated.*/
    p_fiber->ddm_support = 1;
    p_fiber->externally_calibrated = 0;
    p_fiber->channel_num = 1;
    fiber_update_alarm_threshold(port_id, p_fiber);
    fiber_update_realtime(port_id, p_fiber);
    return 0;
}

/* For QSFP+ 10 Gbs 4X Pluggable Transceiver */
static int32 qsfp_get_chip_info_from_buf(uint32 port_id, fiber_info_t* p_fiber)
{
    uint8 buf[QSFP_REG_LEN+1];
    uint8 status=0xff;
    uint16 wavelength;
    uint8 tech_reg;
    
    sal_memset(buf, 0, sizeof(buf));

    /* Modified by liuht for bug 32777, 2015-04-15 */
    /* get qsfp status information */
    if (fiber_get_device_info(port_id, FIBER_DEV_ADDR1, QSFP_STATUS, &status, 1) < 0)
    {
        DRV_LOG_ERR("QSFP+ port %d get status info failed", port_id + 1);
        return -1;
    }

    /* get qsfp msa */
    if (fiber_get_device_info(port_id, FIBER_DEV_ADDR1, QSFP_BASIC_INFO_BASE, buf, QSFP_REG_LEN) < 0)
    {
        DRV_LOG_ERR("QSFP+ port %d get MSA failed", port_id + 1);
        return -1;
    }
    sal_memcpy(p_fiber->name, &buf[QSFP_VENDOR_NAME_REG], FIBER_VENDOR_NAME_LEN);
    sal_memcpy(p_fiber->oui, &buf[QSFP_VENDOR_OUI_REG], FIBER_VENDOR_OUI_LEN);
    sal_memcpy(p_fiber->pn, &buf[QSFP_VENDOR_PN_REG], FIBER_VENDOR_PN_LEN);
    sal_memcpy(p_fiber->rev, &buf[QSFP_VENDOR_REV_REG], 2);
    sal_memcpy(p_fiber->sn, &buf[QSFP_VENDOR_SN_REG], FIBER_VENDOR_SN_LEN);

    p_fiber->date_code.year[0] = buf[QSFP_DATE_CODE_REG];
    p_fiber->date_code.year[1] = buf[QSFP_DATE_CODE_REG+1];
    p_fiber->date_code.year[2] = '\0';
    p_fiber->date_code.month[0] = buf[QSFP_DATE_CODE_REG+2];
    p_fiber->date_code.month[1] = buf[QSFP_DATE_CODE_REG+3];
    p_fiber->date_code.month[2] = '\0';
    p_fiber->date_code.day[0] = buf[QSFP_DATE_CODE_REG+4];
    p_fiber->date_code.day[1] = buf[QSFP_DATE_CODE_REG+5];
    p_fiber->date_code.day[2] = '\0';
    p_fiber->date_code.lot[0] = buf[QSFP_DATE_CODE_REG+6];
    p_fiber->date_code.lot[1] = buf[QSFP_DATE_CODE_REG+7];
    p_fiber->date_code.lot[2] = '\0';
    p_fiber->ext_compliance_code = buf[QSFP_EXT_COMPLIANCE_CODE_REG];
    sal_memcpy(p_fiber->compliance_code, &buf[QSFP_COMPLIANCE_CODE_REG], FIBER_COMPLIANCE_CODE_LEN);
    tech_reg = (buf[QSFP_TECH_REG]>>4)&0xf;
    /* if tech_reg >9, means coper cable */
    if(tech_reg > 9)
    {
        p_fiber->wavelength[0] = 0;
        p_fiber->wavelength[1] = 0;
        p_fiber->is_copper = 1;
        fiber_update_run_info_is_coper(port_id, 1);
    }
    else
    {
        wavelength = (buf[QSFP_WAVELEN_REG]<<8) + buf[QSFP_WAVELEN_REG+1];
        wavelength = wavelength/20;
        p_fiber->wavelength[0] = ((wavelength>>8)&0xff);
        p_fiber->wavelength[1] = (wavelength & 0xff);
        p_fiber->is_copper = 0;
        fiber_update_run_info_is_coper(port_id, 0);
    }

    sal_memcpy(p_fiber->length, &buf[QSFP_LENGTH_REG], QSFP_LENGTH_LEN);

    /* Added by liuht for bug41698, 2016-11-17 */
    /* enable high power class depend on SFF 8636 */
    fiber_high_power_class_enable(port_id, 1);

    /* if not support paging, means not support ddm */
    if(status&0x4)
    {
        p_fiber->ddm_support = 0;
    }
    /* modified by liuht for bug 33716, 2015-06-06 */
    /* if cober cable, means not support ddm  */
    else if((0 == p_fiber->wavelength[0]) && (0 == p_fiber->wavelength[1]))
    {
        p_fiber->ddm_support = 0;
    }
    else
    {
        p_fiber->ddm_support = 1;
    }

    p_fiber->externally_calibrated = 0;
    p_fiber->channel_num = 4;
    if(p_fiber->ddm_support)
    {
        fiber_update_alarm_threshold(port_id, p_fiber);
        fiber_update_realtime(port_id, p_fiber);
    }

    return 0;
}

/*Modify by jqiu for bug 15855. 2011-08-25*/
int32 fiber_get_fiber_type(uint32 port_id, fiber_info_t* fiber_info)
{    
    /*For XFP transceiver */
    if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_XFP)
    {
        /*base on SFF Committee INF-8077i Rev 4.5. current just support LAN, not support WAN*/
        if((fiber_info->compliance_code[0]&XFP_10GBASE_SR)==XFP_10GBASE_SR)
            fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_SR;
        else if((fiber_info->compliance_code[0]&XFP_10GBASE_LR)==XFP_10GBASE_LR)
            fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_LR;
        else if((fiber_info->compliance_code[0]&XFP_10GBASE_ER)==XFP_10GBASE_ER)
            fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_ER;
        else if((fiber_info->compliance_code[0]&XFP_10GBASE_LRM)==XFP_10GBASE_LRM)
            fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_LRM;
        else 
            fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
        return 0;
    }
    else if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
    {
        /* modified by liuht for bug 33741, 2015-06-04 */
        /* parse external compliance code first */
        if((fiber_info->compliance_code[0]&QSFP_COMPLIANCE_EXT)==QSFP_COMPLIANCE_EXT)
        {
            switch(fiber_info->ext_compliance_code&0xff)
            {
                case QSFP_100G_AOC:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100G_AOC;
                    break;
                case QSFP_100GBASE_SR4:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_SR4;
                    break;
                case QSFP_100GBASE_LR4:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_LR4;
                    break;
                case QSFP_100GBASE_ER4:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_ER4;
                    break;
                case QSFP_100GBASE_SR10:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_SR10;
                    break;
                case QSFP_100G_CWDM4_MSA_FEC:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100G_CWDM4_MSA_FEC;
                    break;
                case QSFP_100G_PSM4_SMF:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100G_PSM4_SMF;
                    break;
                case QSFP_100G_ACC:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100G_ACC;
                    break;
                case QSFP_100G_CWDM4_MSA_NOFEC:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC;
                    break;
                case QSFP_100GBASE_CR4:
                case QSFP_100GBASE_CR4_DEF:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_CR4;
                    break;
                case QSFP_40GBASE_ER4:
                    fiber_info->fiber_type = GLB_PHY_TYPE_40GBASE_ER4;
                    break;
                case QSFP_4X10GBASE_SR:
                    fiber_info->fiber_type = GLB_PHY_TYPE_4X10GBASE_SR;
                    break;
                case QSFP_40G_PSM4_SMF:
                    fiber_info->fiber_type = GLB_PHY_TYPE_40G_PSM4_SMF;
                    break;
                default:
                    fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
                    break;
            }
        }
        else if((fiber_info->compliance_code[0]&QSFP_40GBASE_CR4)==QSFP_40GBASE_CR4)
            fiber_info->fiber_type = GLB_PHY_TYPE_40GBASE_CR4;
        else if((fiber_info->compliance_code[0]&QSFP_40GBASE_SR4)==QSFP_40GBASE_SR4)
            fiber_info->fiber_type = GLB_PHY_TYPE_40GBASE_SR4;
        else if((fiber_info->compliance_code[0]&QSFP_40GBASE_LR4)==QSFP_40GBASE_LR4)
            fiber_info->fiber_type = GLB_PHY_TYPE_40GBASE_LR4;
        else if((fiber_info->compliance_code[0]&QSFP_40GBASE_XLPPI)==QSFP_40GBASE_XLPPI)
            fiber_info->fiber_type = GLB_PHY_TYPE_40GBASE_XLPPI;
        /* for bug 51535, remove compatible for INNOLIGHT fiber*/
        //else if(fiber_info->compliance_code[0]==QSFP_100GBASE_DEF)
        //    fiber_info->fiber_type = GLB_PHY_TYPE_100GBASE_SR4;
        else 
            fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
        return 0;
    }
    /*For SFP or SFP+ transceiver*/    
    else
    {
        /* support 10G base-ZR and 1G base-ZR for bug 37823£¬37891£¬44540 */
        if(80 == fiber_info->length[0])
        {
            /* 10G base-ZR*/
            if(0x64 <= fiber_info->speed)
            {
                fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_ZR;
                return 0;
            }
            else
            {
                fiber_info->fiber_type = GLB_PHY_TYPE_1000BASE_ZX;
                return 0;
            }
        }
        if (fiber_info->ext_compliance_code != 0)
        {
            switch(fiber_info->ext_compliance_code&0xff)
            {
                case SFP_25G_AOC_BER_H:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H;
                    break;                    
                case SFP_25G_AOC_BER_L:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L;
                    break;
                case SFP_25GBASE_SR:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_SR;
                    break;
                case SFP_25GBASE_LR:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_LR;
                    break;
                case SFP_25GBASE_ER:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_ER;
                    break;
                case SFP_25G_ACC_BER_H:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H;
                    break;                    
                case SFP_25G_ACC_BER_L:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L;
                    break;
                case SFP_25GBASE_CR_L:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_CR_L;
                    break;
                case SFP_25GBASE_CR_S:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_CR_S;
                    break;                    
                case SFP_25GBASE_CR_N:
                    fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_CR_N;
                    break;
                default:
                    fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
                    break;
            }
            if(fiber_info->fiber_type != GLB_PHY_TYPE_UNKNOWN)
                return 0;
        }        
        /* FFh indicates the bit rate is greater than 25.0Gb/s, address 66 is used
        to determine bit rate.*/
        if((0xff == fiber_info->speed) && (fiber_info->fiber_type == GLB_PHY_TYPE_UNKNOWN))
        {
            /* if the value of address 66 is bigger than 0x70, then the bit rate is greater than 25.0Gb/s */
            /* A value of 00h indicates that this field is not specified */
            if((fiber_info->br_max < 0x70) &&(fiber_info->br_max > 0x00))
            {
                /*SFP+ Cable Technology. 0x08 is active cable, 0x04 is passive cable.*/
                if((fiber_info->compliance_code[5] & 0x0c) != 0)
                {
                    if((fiber_info->compliance_code[5] & SFP_P_25GBASE_PASSIVE_CABLE) != 0)
                        fiber_info->fiber_type = GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE;
                    return 0;
                }
            }
        }
        /*10G Ethernet Compliance Codes*/
        if((fiber_info->compliance_code[0]&0xf0) != 0)
        {
            switch(fiber_info->compliance_code[0]&0xf0)
            {
                case SFP_P_10GBASE_SR:
                    fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_SR;
                    break;
                case SFP_P_10GBASE_LR:
                    fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_LR;
                    break;
                case SFP_P_10GBASE_LRM:
                    fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_LRM;
                    break;
                case SFP_P_10GBASE_ER:
                    fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_ER;
                    break;
                default:
                    fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
                    break;
            }
            return 0;
        }
        /*SFP+ Cable Technology. bug33507. Some cable both support 10G SFP+ and 1G SFP, parse 10G first.*/
        else if((fiber_info->compliance_code[5]&0x0c) != 0)
        {
            if((fiber_info->compliance_code[5]&SFP_P_10GBASE_ACTIVE_CABLE) != 0)
                fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER;
            else
                fiber_info->fiber_type = GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER;
            return 0;
        }
        /*Ethernet Compliance Codes*/
        else if(fiber_info->compliance_code[3] != 0)
        {
            switch(fiber_info->compliance_code[3])
            {
                case SFP_ETH_1000BASE_SX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_1000BASE_SX;
                    break;
                case SFP_ETH_1000BASE_LX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_1000BASE_LX;
                    break;
                case SFP_ETH_1000BASE_CX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_1000BASE_CX;
                    break;
                case SFP_ETH_1000BASE_T:
                    fiber_info->fiber_type = GLB_PHY_TYPE_1000BASE_T_SFP;
                    break;
                case SFP_ETH_100BASE_LX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100BASE_LX;
                    break;
                case SFP_ETH_100BASE_FX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_100BASE_FX;
                    break;
                case SFP_ETH_BASE_BX10:
                    fiber_info->fiber_type = GLB_PHY_TYPE_BASE_BX10;
                    break; 
                case SFP_ETH_BASE_PX:
                    fiber_info->fiber_type = GLB_PHY_TYPE_BASE_PX;
                    break;
                default:
                    fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
                    break;
            }
            return 0;
        }        
        /*SONET Compliance Codes. here only support OC-3. 155M as 100M */
        else if((fiber_info->compliance_code[2]&0x07) != 0)
        {
            fiber_info->fiber_type = GLB_PHY_TYPE_100BASE_FX;
            return 0;
        }
        fiber_info->fiber_type = GLB_PHY_TYPE_UNKNOWN;
        return 0;
    } 
}

/*Modify by jqiu for bug 15855. 2011-08-25*/
void fiber_update_fiber_info(fiber_info_t* fiber_info)
{
    /*For copper or copper cable, wavelength is useless.*/
    if((fiber_info->fiber_type == GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER)
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER)
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_25GBASE_CR_L) /* for bug 45717 */
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_25GBASE_CR_S) 
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_25GBASE_CR_N) 
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE)         
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_1000BASE_T_SFP)
        ||(fiber_info->fiber_type == GLB_PHY_TYPE_UNKNOWN))
    {
        fiber_info->wavelength[0] = 0;
        fiber_info->wavelength[1] = 0;
    }

    /*Bug33507, clear length when unknown fiber.*/
    if(fiber_info->fiber_type == GLB_PHY_TYPE_UNKNOWN)
    {
        fiber_info->length[4] = 0;
    }
}

/* for bug 47844, when fiber info is invalid, we need keep it to default info */
void fiber_check_string_info(fiber_info_t* fiber_info)
{
    int i;
    char invalid[FIBER_VENDOR_NAME_LEN]="N/A             ";
    for(i=0;i<FIBER_VENDOR_NAME_LEN;i++)
    {
        if(!sal_isprint(fiber_info->name[i]))
        {
            sal_memcpy(fiber_info->name, invalid, FIBER_VENDOR_NAME_LEN);
            break;
        }
    }
    for(i=0;i<FIBER_VENDOR_PN_LEN;i++)
    {
        if(!sal_isprint(fiber_info->pn[i]))
        {
            sal_memcpy(fiber_info->pn, invalid, FIBER_VENDOR_NAME_LEN);
            break;
        }
    }
    for(i=0;i<FIBER_VENDOR_SN_LEN;i++)
    {
        if(!sal_isprint(fiber_info->sn[i]))
        {
            sal_memcpy(fiber_info->sn, invalid, FIBER_VENDOR_NAME_LEN);
            break;
        }
    }    
}

int32 fiber_get_chip_info(uint32 port_id, fiber_info_t *p_fiber)
{
    int32 ret;

    if(!check_port_support_fiber(port_id))
    {
        return -1;
    }
    if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_XFP)
    {
        ret = xfp_get_chip_info_from_buf(port_id, p_fiber);
    }
    else if(g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
    {
        ret = qsfp_get_chip_info_from_buf(port_id, p_fiber);
    }
    else
    {
        ret = sfp_get_chip_info_from_buf(port_id, p_fiber);
    }
    if(ret >= 0)
    {
        p_fiber->fiber_flag = g_fiber_port_info[port_id].fiber_flg;
        fiber_get_fiber_type(port_id, p_fiber);
        fiber_update_fiber_info(p_fiber);
        fiber_check_string_info(p_fiber);    
    }
    return ret;
}

/* check current value exceeding the limit or not */
static char* fiber_get_alarm_flag(float val, float val_high_alarm, float val_high_warn, float val_low_warn, float val_low_alarm)
{
    if (val >= val_high_alarm)
        return "++";
    else if (val >= val_high_warn)
        return "+";
    else if (val <= val_low_alarm)
        return "--";
    else if (val <= val_low_warn)
        return "-";
    else
        return "";
}

/* dump fiber digital diag information according to SFF-8472 */
void fiber_dump_diag_info(char *port_name, fiber_info_t* fiber_info, FILE *fp)
{
    double val = 0.0;
    double val_high_alarm = 0.0;
    double val_low_alarm = 0.0;
    double val_high_warn = 0.0;
    double val_low_warn = 0.0;
    uint8 channel_num, channel_idx;
    uint8 i, start, end;
    char show_name[64];
       
    if (fiber_info->ddm_support != 1)
    {
        sal_fprintf(fp, "Digital diagnostic is not implemented.\n");
    }
    else
    {
        channel_num = fiber_info->channel_num;
        channel_idx = fiber_info->channel_idx;

        if(channel_idx == FIBER_VAL_TYPE_MAX2)
        {
            start = FIBER_CURRENT_VALUE1;
            end = start + channel_num - 1;
        }
        else
        {
            start = channel_idx;
            end = start;
        }
        for(i=start; i<=end; i++)
        {
            if(start != end)
            {
                sal_sprintf(show_name, "%s chan %d", port_name, i+1);
            }
            else
            {
                sal_sprintf(show_name, "%s", port_name);
            }
            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "Transceiver is %s calibrated.\n", fiber_info->externally_calibrated ? "externally":"internally");
            sal_fprintf(fp, "mA: milliamperes, dBm: decibels (milliwatts), NA or N/A: not applicable. \n");
            sal_fprintf(fp, "++ : high alarm, +  : high warning, -  : low warning, -- : low alarm. \n");
            sal_fprintf(fp, "The threshold values are calibrated. \n");
            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                    High Alarm  High Warn   Low Warn    Low Alarm \n");
            sal_fprintf(fp, "                Temperature         Threshold   Threshold   Threshold   Threshold  \n");
            sal_fprintf(fp, "Port            (Celsius)           (Celsius)   (Celsius)   (Celsius)   (Celsius)  \n");
            sal_fprintf(fp, "--------------- ------------------  ----------  ----------  ----------  ---------- \n");
            val             = fiber_info->tmpr2[i];
            val_high_alarm  = fiber_info->tmpr[FIBER_HIGH_ALARM_VALUE];
            val_low_alarm   = fiber_info->tmpr[FIBER_LOW_ALARM_VALUE];
            val_high_warn   = fiber_info->tmpr[FIBER_HIGH_WARN_VALUE];
            val_low_warn    = fiber_info->tmpr[FIBER_LOW_WARN_VALUE];
            sal_fprintf(fp, "%-18s%-13.2f%-7s%-12.2f%-12.2f%-12.2f%-12.2f\n", show_name, val, \
                fiber_get_alarm_flag(val, val_high_alarm, val_high_warn, val_low_warn, val_low_alarm),\
                val_high_alarm, val_high_warn, val_low_warn, val_low_alarm);
            
            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                    High Alarm  High Warn   Low Warn    Low Alarm \n");
            sal_fprintf(fp, "                Voltage             Threshold   Threshold   Threshold   Threshold \n");
            sal_fprintf(fp, "Port            (Volts)              (Volts)     (Volts)     (Volts)     (Volts)  \n");
            sal_fprintf(fp, "--------------- ------------------  ----------  ----------  ----------  ----------\n");
            val             = fiber_info->voltage2[i];
            val_high_alarm  = fiber_info->voltage[FIBER_HIGH_ALARM_VALUE];
            val_low_alarm   = fiber_info->voltage[FIBER_LOW_ALARM_VALUE];
            val_high_warn   = fiber_info->voltage[FIBER_HIGH_WARN_VALUE];
            val_low_warn    = fiber_info->voltage[FIBER_LOW_WARN_VALUE];            
            sal_fprintf(fp, "%-18s%-13.2f%-7s%-12.2f%-12.2f%-12.2f%-12.2f\n", show_name, val, \
                fiber_get_alarm_flag(val, val_high_alarm, val_high_warn, val_low_warn, val_low_alarm),\
                val_high_alarm, val_high_warn, val_low_warn, val_low_alarm);
            
            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                                    High Alarm  High Warn   Low Warn    Low Alarm \n");
            sal_fprintf(fp, "                Current             Threshold   Threshold   Threshold   Threshold   \n");
            sal_fprintf(fp, "Port            (milliamperes)        (mA)        (mA)        (mA)        (mA)   \n");
            sal_fprintf(fp, "--------------- ------------------  ----------  ----------  ----------  ----------\n");
            val             = fiber_info->bias2[i];
            val_high_alarm  = fiber_info->bias[FIBER_HIGH_ALARM_VALUE];
            val_low_alarm   = fiber_info->bias[FIBER_LOW_ALARM_VALUE];
            val_high_warn   = fiber_info->bias[FIBER_HIGH_WARN_VALUE];
            val_low_warn    = fiber_info->bias[FIBER_LOW_WARN_VALUE];    
            sal_fprintf(fp, "%-18s%-13.2f%-7s%-12.2f%-12.2f%-12.2f%-12.2f\n", show_name, val, \
                fiber_get_alarm_flag(val, val_high_alarm, val_high_warn, val_low_warn, val_low_alarm),\
                val_high_alarm, val_high_warn, val_low_warn, val_low_alarm);

            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                Optical             High Alarm  High Warn   Low Warn    Low Alarm  \n");
            sal_fprintf(fp, "                Transmit Power      Threshold   Threshold   Threshold   Threshold    \n");
            sal_fprintf(fp, "Port             (dBm)                (dBm)       (dBm)       (dBm)       (dBm)   \n");
            sal_fprintf(fp, "--------------- ------------------  ----------  ----------  ----------  ---------- \n");
            val             = fiber_info->tx_pwr2[i];
            val_high_alarm  = fiber_info->tx_pwr[FIBER_HIGH_ALARM_VALUE];
            val_low_alarm   = fiber_info->tx_pwr[FIBER_LOW_ALARM_VALUE];
            val_high_warn   = fiber_info->tx_pwr[FIBER_HIGH_WARN_VALUE];
            val_low_warn    = fiber_info->tx_pwr[FIBER_LOW_WARN_VALUE];   
            sal_fprintf(fp, "%-18s%-13.2f%-7s%-12.2f%-12.2f%-12.2f%-12.2f\n", show_name, val, \
                fiber_get_alarm_flag(val, val_high_alarm, val_high_warn, val_low_warn, val_low_alarm),\
                val_high_alarm, val_high_warn, val_low_warn, val_low_alarm);

            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");
            sal_fprintf(fp, "                Optical             High Alarm  High Warn   Low Warn    Low Alarm    \n");
            sal_fprintf(fp, "                Receive Power       Threshold   Threshold   Threshold   Threshold \n");
            sal_fprintf(fp, "Port             (dBm)                (dBm)       (dBm)       (dBm)       (dBm) \n");
            sal_fprintf(fp, "--------------- ------------------  ----------  ----------  ----------  ---------- \n");
            val             = fiber_info->rx_pwr2[i];
            val_high_alarm  = fiber_info->rx_pwr[FIBER_HIGH_ALARM_VALUE];
            val_low_alarm   = fiber_info->rx_pwr[FIBER_LOW_ALARM_VALUE];
            val_high_warn   = fiber_info->rx_pwr[FIBER_HIGH_WARN_VALUE];
            val_low_warn    = fiber_info->rx_pwr[FIBER_LOW_WARN_VALUE];  
            sal_fprintf(fp, "%-18s%-13.2f%-7s%-12.2f%-12.2f%-12.2f%-12.2f\n", show_name, val, \
                fiber_get_alarm_flag(val, val_high_alarm, val_high_warn, val_low_warn, val_low_alarm),\
                val_high_alarm, val_high_warn, val_low_warn, val_low_alarm);
            sal_fprintf(fp, "----------------------------------------------------------------------------------\n");  
        }      
    }    
    return;    
}

void fiber_dump_basic_info(char *port_name, fiber_info_t* fiber_info, FILE *fp)
{
    uint16_t wave_length = 0;
    
    sal_fprintf(fp, "Transceiver Type: %d\n", fiber_info->fiber_type);
    sal_fprintf(fp, " Transceiver Vendor Name : %s\n", fiber_info->name);
    sal_fprintf(fp, " Transceiver PN          : %s\n", fiber_info->pn);
    sal_fprintf(fp, " Transceiver S/N         : %s\n", fiber_info->sn);
    /* for bug 43937 added by tongzb */
    if(fiber_info->tunable_support)
    {
        sal_fprintf(fp, "Tunable mode:");
        if(fiber_info->tune_mode == WAVELENGTH_CHANNEL_MODE)
            sal_fprintf(fp, "wavelength/channel\n");
        else if(fiber_info->tune_mode == WAVELENGTH_MODE)
            sal_fprintf(fp, "wavelength\n");
        else if(fiber_info->tune_mode == CHANNEL_MODE)
            sal_fprintf(fp, "channel\n");
        else
            sal_fprintf(fp, "\n");
        sal_fprintf(fp, "Transceiver Output Wavelength: %.2f nm\n", fiber_info->tunable_wavelength);
        sal_fprintf(fp,"Transceiver DWDM channel NO: %d/%d\n",fiber_info->wave_channel_idx,fiber_info->wave_channels);
        sal_fprintf(fp,"Transceiver DWDM frequency: %.2f\n",\
            (fiber_info->wave_channel_idx * fiber_info->step_freq) + fiber_info->first_freq - fiber_info->step_freq);
    }
    else
    {
        /*fix bug 15496. jqiu 2011-07-19*/
        wave_length = (((uint16_t)fiber_info->wavelength[0])<<8) | fiber_info->wavelength[1];
        if(wave_length)
        {
            sal_fprintf(fp, "Transceiver Output Wavelength: %u nm\n", wave_length);
        }
        else
        {
            sal_fprintf(fp, "Transceiver Output Wavelength: N/A\n");
        }
    }
    /*Modify by jqiu for bug 15855. 2011-08-25*/
    if(fiber_info->length[0]!=0 || fiber_info->length[1]!=0 || fiber_info->length[2]!=0
        || fiber_info->length[3]!=0 || fiber_info->length[4]!=0)
    {
        /* SFF-8472 */
        if(fiber_info->fiber_flag == E_FIBER_SFP_P)
        {
            sal_fprintf(fp, "Supported Link Type and Length: \n");
            if (fiber_info->length[0])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u km\n", fiber_info->length[0]);
            }
            else if (fiber_info->length[1])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u m\n", fiber_info->length[1]*100);
            }    
            if (fiber_info->length[2])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM2 fiber: %u m\n", fiber_info->length[2]*10);
            }          
            if (fiber_info->length[3])
            {
                sal_fprintf(fp, "      Link Length for 62.5/125um multi-mode OM1 fiber: %u m\n", fiber_info->length[3]*10);
            }       
            /* the length of copper cable or active cable is different in units */
            if (fiber_info->length[4])
            {
                if(GLB_PHY_TYPE_25GBASE_SR == fiber_info->fiber_type)
                {
                    sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", fiber_info->length[4]*10);        
                }
                else
                {
                    sal_fprintf(fp, "      Link Length for copper: %u m\n", fiber_info->length[4]);
                }
            }
        }
        /* SFF-8636 */
        /* for bug 45607 */
        else if(fiber_info->fiber_flag == E_FIBER_QSFP_P)
        {
            sal_fprintf(fp, "Supported Link Type and Length: \n");
            if (fiber_info->length[0])
            {
                sal_fprintf(fp, "      Link Length for 9/125um single mode fiber: %u km\n", fiber_info->length[0]);
            }
            if (fiber_info->length[1])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM3 fiber: %u m\n", fiber_info->length[1]*2);
            }    
            if (fiber_info->length[2])
            {
                sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM2 fiber: %u m\n", fiber_info->length[2]);
            }          
            if (fiber_info->length[3])
            {
                sal_fprintf(fp, "      Link Length for 62.5/125um multi-mode OM1 fiber: %u m\n", fiber_info->length[3]);
            }       
            /* the length of copper cable or active cable is different in units */
            if (fiber_info->length[4])
            {
                if(fiber_info->is_copper)
                {
                    sal_fprintf(fp, "      Link Length for copper: %u m\n", fiber_info->length[4]);
                }
                else
                {
                    sal_fprintf(fp, "      Link Length for 50/125um multi-mode OM4 fiber: %u m\n", fiber_info->length[4]*2);
                }
            }
        }
    }
    return;
}

/*bug32802, scan fiber present, absent to present change in this func. */
int32 fiber_get_present(uint32 port_id, uint8* status)
{
    fiber_handle_t* phdl;
    int32 ret;
    uint32 present;

    *status = FIBER_ABSENT;
    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }
    phdl = get_fiber_handle(port_id);
    if(NULL == phdl || NULL == phdl->fiber_present)
    {
        return RESULT_ERROR;
    }    
    sal_mutex_lock(g_fiber_pmutex);
    ret = phdl->fiber_present(g_fiber_port_info[port_id].fiber_id, phdl, &present);
    sal_mutex_unlock(g_fiber_pmutex);    
    if(ret == RESULT_ERROR)
    {
        return ret;
    }    
    if(present)
    {
        switch(phdl->run_info.present)
        {
            case FIBER_ABSENT:
                phdl->run_info.present = FIBER_INIT1;
                break;
            case FIBER_INIT1:
                phdl->run_info.present = FIBER_INIT2;
                break;
            case FIBER_INIT2:
                phdl->run_info.present = FIBER_PRESENT;
                break;
            case FIBER_PRESENT:
                phdl->run_info.present = FIBER_PRESENT;
                break;
            default:
                phdl->run_info.present = FIBER_ABSENT;
                break;                
        }
    }
    else
    {
        phdl->run_info.present = FIBER_ABSENT;
    }

    if((g_fiber_port_info[port_id].fiber_flg == E_FIBER_QSFP_P)
        && (phdl->run_info.present != FIBER_PRESENT))
    {
        phdl->run_info.tx_enable = 1;//QSFP module tx disable is lost when enabled
    }
    
    *status = phdl->run_info.present;
    return RESULT_OK;
}

int32 fiber_get_los(uint32 port_id, uint8* status, fiber_info_t* p_fiber)
{
    fiber_handle_t* phdl;
    uint32 los;

    *status = 1;
    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }    
    phdl = get_fiber_handle(port_id);
    if(NULL == phdl || NULL == phdl->fiber_los)
    {
        return RESULT_ERROR;
    }
    /*If fiber is absent, los is always true*/
    if(FIBER_PRESENT != phdl->run_info.present)
    {
        return RESULT_OK;
    }
    if(fiber_is_direct_attach_cable(p_fiber))
        los = 0;
    else
        phdl->fiber_los(g_fiber_port_info[port_id].fiber_id, phdl, &los);    
    *status = los?1:0;
    return RESULT_OK;
}

int32 fiber_set_enable(uint32 port_id, uint8 tx_enable)
{
    fiber_handle_t* phdl;
    int32 ret;

    if(!check_port_support_fiber(port_id))
    {
        return RESULT_ERROR;
    }
    
    phdl = get_fiber_handle(port_id);
    if(NULL == phdl || NULL == phdl->fiber_enable)
    {
        return RESULT_ERROR;
    }
    /*If fiber is absent, do nothing.*/
    if(FIBER_PRESENT != phdl->run_info.present)
    {
        return RESULT_OK;
    }
    /*if fiber orig tx_enable cfg is different as new, do it.*/
    if(phdl->run_info.tx_enable != tx_enable)
    {
        ret = phdl->fiber_enable(g_fiber_port_info[port_id].fiber_id, phdl, tx_enable);
        if(ret == RESULT_OK)
        {
            phdl->run_info.tx_enable = tx_enable;
        }
    }
    return RESULT_OK;
}

int32 fiber_is_direct_attach_cable(fiber_info_t* p_fiber)
{
    if(NULL == p_fiber)
    {
        //DRV_LOG_ERR("fiber_is_direct_attach_cable: Invalid parameter.\n");
        return 0;
    }
    /* For AOC and DAC, the wavelength is 0 */
    if((p_fiber->wavelength[0] ==0)&&(p_fiber->wavelength[1] ==0)
        && (p_fiber->fiber_type != GLB_PHY_TYPE_100G_AOC))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* fix bug44604, liangf, 2017-07-06 */
int32 fiber_is_mellanox_cable(fiber_info_t* p_fiber)
{
    if(NULL == p_fiber)
    {
        //DRV_LOG_ERR("fiber_is_mellanox_cable: Invalid parameter.\n");
        return 0;
    }
    if ((0 == sal_strncmp(p_fiber->name, "Mellanox", 8)) && (p_fiber->fiber_type == GLB_PHY_TYPE_40GBASE_XLPPI))
    {
        return 1;
    }
    else
    {
        return 0;
    }        
}

int32 fiber_init(uint32 fiber_num, uint32 port_num, fiber_port_info_t* fiber_port_info)
{
    fiber_handle_t** phdl;
    uint32 port_id;
    uint32 fiber_id;

    if(NULL == fiber_port_info)
    {
        DRV_LOG_ERR("Fiber init: Invalid parameter.\n");
        return RESULT_ERROR;
    }
    phdl = (fiber_handle_t** )DRV_MALLOC(CTCLIB_MEM_DRIVER_FIBER_INFO, sizeof(fiber_handle_t* ) * fiber_num);
    if(NULL == phdl)
    {
        DRV_LOG_ERR("Malloc fiber handle failed, out of memory.\n");
        return RESULT_ERROR;
    }
    /* Added by liuht for bug40747, 2016-09-14 */
    if(sal_mutex_create(&g_fiber_pmutex))
    {
        return RESULT_ERROR;    
    }
    g_fiber_hdl = phdl;
    g_fiber_port_info = fiber_port_info;
    g_fiber_num = fiber_num;
    g_port_num = port_num;
    for(port_id = 0; port_id < port_num; port_id++)
    {
        if(-1 == fiber_port_info[port_id].fiber_flg)
        {
            continue;
        }
        fiber_id = fiber_port_info[port_id].fiber_id;
        if(fiber_id >= fiber_num)
        {
            DRV_LOG_ERR("FIBER init: Invalid fiber id.\n");
            DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, phdl);
            phdl = NULL;
            return RESULT_ERROR;
        }
        
        phdl[fiber_id] = fiber_create_handle(port_id, &fiber_port_info[port_id]);
        if(NULL == phdl[fiber_id])
        {
            DRV_FREE(CTCLIB_MEM_DRIVER_FIBER_INFO, phdl);
            phdl = NULL;
            return RESULT_ERROR;
        }
        /*bug32802, add run_info struct, init to default value*/
        phdl[fiber_id]->run_info.present = FIBER_ABSENT;
        phdl[fiber_id]->run_info.tx_enable = 1;//tx diff with port_cfg.enable will trig fiber cfg the tx_enable real
#ifdef BRINGUP_IMAGE  
        phdl[fiber_id]->run_info.present = FIBER_PRESENT;
        phdl[fiber_id]->run_info.tx_enable = 0;
        fiber_set_enable(port_id, 1);
#else            
        /* After register fiber handler, disable fiber. */
        /*must after g_port_num = port_num, in this func will check param. */
        //fiber_set_enable(port_id, 0);
        
#endif
    }

    return RESULT_OK;

}


