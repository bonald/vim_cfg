/****************************************************************************
 * led_api.c    led api 
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       zhuj
 * Date:         2010-09-21.
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
#include "led_drv.h"
#include "led_api.h"
#include "ctc_api.h"
#include "ctc_chip.h"
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
/* led handler */
static led_handle_t *g_p_led_hdl[E_MAX_LED];
static mac_led_info_t* g_p_mac_led_info=NULL;
//static mac_led_api_para_t* g_p_port_led_slice0 = NULL;
//static mac_led_api_para_t* g_p_port_led_slice1 = NULL;
static uint8 g_mac_table_id=0;
//static uint8 g_mac_led_slice0_init_done = 0;
//static uint8 g_mac_led_slice1_init_done = 0;

/****************************************************************************
 *
* Functions  
*
****************************************************************************/
int32
led_api_op(glb_led_type_t type, glb_led_stat_t led_stat)
{
    int32 ret = RESULT_OK;

    if(led_stat < 0 || led_stat >=  E_LED_STAT_MAX)
    {
        DRV_LOG_ERR("Invalid parameter when control led, led stat %d.", led_stat);
        return RESULT_ERROR;    
    }

    if(g_p_led_hdl[type]==NULL)
        return ret;
    ret = g_p_led_hdl[type]->op(g_p_led_hdl[type], led_stat);
    return ret;
}

int32 
led_init(led_info_t* p_led_info)
{
    uint8 led_idx;
    
    if(NULL == p_led_info)
    {
        DRV_LOG_ERR("Led init: Invalid pointer.\n");
        return RESULT_ERROR;
    }

    for(led_idx = 0; led_idx < E_MAX_LED; led_idx++)
    {
        if(0 == p_led_info[led_idx].exist)
        {
            g_p_led_hdl[led_idx] = NULL;
            continue;
        }
        g_p_led_hdl[led_idx] = led_register(&(p_led_info[led_idx]));
        if(NULL == g_p_led_hdl[led_idx])
        {
            DRV_LOG_ERR("Led init: register led handle failed.\n");
            return RESULT_ERROR;
        }
    }
    
    return 0;
}

/***************************************************************************************************
 * Name         : mac_led_info_register 
 * Purpose      : register mac led information          
 * Input        : p_mac_led_info         
 * Output       : g_p_mac_led_info              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32 mac_led_info_register(mac_led_info_t* p_mac_led_info)
{
    uint8 table_idx;
    uint8 table_num;
    uint8 mac_num;
    mac_led_api_para_t** mac_led_api_para;
    
    if(!p_mac_led_info)
    {
        g_p_mac_led_info = NULL;
        return -1;
    }

    table_num = p_mac_led_info->table_num;
    mac_num = p_mac_led_info->mac_num;
    
    if(table_num == 0)
    {
        return RESULT_OK;
    }
    
    g_p_mac_led_info = (mac_led_info_t*)DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(mac_led_info_t));
    if(!g_p_mac_led_info)
    {
        DRV_LOG_ERR("Allocate pointer to mac_led_info_t fail.\n");
        return RESULT_ERROR;
    }

    g_p_mac_led_info->polarity = p_mac_led_info->polarity;
    g_p_mac_led_info->mac_num = mac_num;
    g_p_mac_led_info->table_num = table_num;
    g_p_mac_led_info->slice0_mac_num = p_mac_led_info->slice0_mac_num;

    mac_led_api_para = (mac_led_api_para_t **)DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(mac_led_api_para_t *)*table_num);
    if(!mac_led_api_para)
    {
        DRV_LOG_ERR("Allocate pointer to mac_led_api_para_t* fail.\n");
        return RESULT_ERROR;
    }

    for(table_idx = 0; table_idx < table_num; table_idx++)
    {
        mac_led_api_para[table_idx] = (mac_led_api_para_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(mac_led_api_para_t)*mac_num);
        if(!mac_led_api_para[table_idx])
        {
            DRV_LOG_ERR("Allocate pointer to mac_led_api_para_t fail table_idx %d.\n", table_idx);
            return RESULT_ERROR;
        }
        sal_memcpy(mac_led_api_para[table_idx], p_mac_led_info->mac_led_api_para[table_idx], sizeof(mac_led_api_para_t)*mac_num);
    }

    g_p_mac_led_info->mac_led_api_para = mac_led_api_para;

    return 0;
}

/***************************************************************************************************
 * Name         : mac_led_info_modify 
 * Purpose      : modify mac led information          
 * Input        : port_led         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32 mac_led_info_modify(mac_led_api_para_t* port_led)
{
    uint8 table_id, mac_idx;
    mac_led_api_para_t* pp_mac_led_api_para;

    if(NULL == g_p_mac_led_info)
    {
        DRV_LOG_ERR("Management mac led: g_p_mac_led_info is NULL.\n");
        return RESULT_ERROR;
    }

    for(table_id = 0; table_id < g_p_mac_led_info->table_num; table_id++)
    {
        pp_mac_led_api_para = g_p_mac_led_info->mac_led_api_para[table_id];
        for(mac_idx = 0; mac_idx < g_p_mac_led_info->mac_num; mac_idx++)
        {
            if((pp_mac_led_api_para[mac_idx].port_id == port_led->port_id)
            &&(pp_mac_led_api_para[mac_idx].lchip == port_led->lchip)
            &&(pp_mac_led_api_para[mac_idx].ctl_id == port_led->ctl_id))
            {
                pp_mac_led_api_para[mac_idx].mode = port_led->mode;
                break;
            }
        }
    }

    return 0;
}

int32 mac_led_mode_set(mac_led_api_para_t *port_led)
{
    int32 ret = 0;
    ctc_chip_led_para_t led_para;
    ctc_chip_mac_led_type_t led_type = CTC_CHIP_USING_ONE_LED;
    
    led_para.ctl_id = port_led->ctl_id;
#ifdef TSINGMA     
    led_para.lport_en = 0;
#endif
    led_para.lchip = port_led->lchip;
    led_para.op_flag = CTC_CHIP_LED_MODE_SET_OP;
    /* modified by liuht for bug 27331, 2014-04-22 */
    led_para.port_id = port_led->port_id;
    /* end of liuht modified */
    if(port_led->mode == LED_MODE_2_RXLNK_BIACT)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_RXLINK_MODE;
        /* Modified by liuht to change activity led mode for bug25680, 2013-11-12 */
        led_para.sec_mode = CTC_CHIP_BIACTIVITY_MODE;
    }
    /* added by liuht for bug 29099, 2014-06-23 */
    else if(port_led->mode == LED_MODE_1_RXLNK_BIACT)
    {
        led_type = CTC_CHIP_USING_ONE_LED;
        led_para.first_mode = CTC_CHIP_RXLINK_BIACTIVITY_MODE;
    }     
    else if (port_led->mode == LED_MODE_2_OFF_RXLNKBIACT)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
        led_para.sec_mode = CTC_CHIP_RXLINK_BIACTIVITY_MODE;
    }
    /* end of liuht added */
    else if(port_led->mode == LED_MODE_2_RXLNKBIACT_OFF)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_RXLINK_BIACTIVITY_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    else if(port_led->mode == LED_MODE_2_FORCE_OFF)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    else if(port_led->mode == LED_MODE_1_FORCE_ON)
    {
        led_type = CTC_CHIP_USING_ONE_LED;
        led_para.first_mode = CTC_CHIP_FORCE_ON_MODE;
    }
    else if(port_led->mode == LED_MODE_1_FORCE_OFF)
    {
        led_type = CTC_CHIP_USING_ONE_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    else if(port_led->mode == LED_MODE_2_OFF_BIACT)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
        led_para.sec_mode = CTC_CHIP_BIACTIVITY_MODE;
    }
    else if(port_led->mode == LED_MODE_2_BIACT_OFF)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_BIACTIVITY_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    else if(port_led->mode == LED_MODE_1_BIACT)
    {
        led_type = CTC_CHIP_USING_ONE_LED;
        led_para.first_mode = CTC_CHIP_BIACTIVITY_MODE;
    }
    else if(port_led->mode == LED_MODE_2_OFF_ON)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_ON_MODE;
    }
    else if(port_led->mode == LED_MODE_2_ON_OFF)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_ON_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    /* Added by liuht for bug41782, 2016-11-23 */
    else if(port_led->mode == LED_MODE_2_TXLNK_RXLNKBIACT)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_RXLINK_BIACTIVITY_MODE;
        led_para.sec_mode = CTC_CHIP_TXLINK_MODE;
    }
    else if(port_led->mode == LED_MODE_2_TXLNK_BIACT)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_BIACTIVITY_MODE;
        led_para.sec_mode = CTC_CHIP_TXLINK_MODE;
    }
    else if(port_led->mode == LED_MODE_2_TXLNK_OFF)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_FORCE_OFF_MODE;
        led_para.sec_mode = CTC_CHIP_TXLINK_MODE;
    }
    else if(port_led->mode == LED_MODE_2_RXLNKBIACT_TXLNK)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_TXLINK_MODE;
        led_para.sec_mode = CTC_CHIP_RXLINK_BIACTIVITY_MODE;
    }
    else if(port_led->mode == LED_MODE_2_BIACT_TXLNK)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_TXLINK_MODE;
        led_para.sec_mode = CTC_CHIP_BIACTIVITY_MODE;
    }
    else if(port_led->mode == LED_MODE_2_OFF_TXLNK)
    {
        led_type = CTC_CHIP_USING_TWO_LED;
        led_para.first_mode = CTC_CHIP_TXLINK_MODE;
        led_para.sec_mode = CTC_CHIP_FORCE_OFF_MODE;
    }
    ret = ctc_chip_set_mac_led_mode(&led_para, led_type);    
    return ret;
}

int32 mac_led_update(mac_led_api_para_t* port_led)
{
    uint8 port = 0;
    uint16 slice0_lchip0_port = 0,slice1_lchip0_port = 0;
    uint16 slice0_lchip1_port = 0,slice1_lchip1_port = 0;
    ctc_chip_mac_led_mapping_t led_map_slice0_lchip0,led_map_slice1_lchip0;
    ctc_chip_mac_led_mapping_t led_map_slice0_lchip1,led_map_slice1_lchip1;
    sal_memset(&led_map_slice0_lchip0, 0, sizeof(ctc_chip_mac_led_mapping_t));
    sal_memset(&led_map_slice1_lchip0, 0, sizeof(ctc_chip_mac_led_mapping_t));
    sal_memset(&led_map_slice0_lchip1, 0, sizeof(ctc_chip_mac_led_mapping_t));
    sal_memset(&led_map_slice1_lchip1, 0, sizeof(ctc_chip_mac_led_mapping_t));

    for(port = 0; port < g_p_mac_led_info->mac_num; port++)
    {
        if(0 == port_led[port].lchip)
        {
            if(0 == port_led[port].ctl_id)
            {
                slice0_lchip0_port++;
            }
            else if(1 == port_led[port].ctl_id)
            {
                slice1_lchip0_port++;
            }
        }
        else if(1 == port_led[port].lchip)
        {
            if(0 == port_led[port].ctl_id)
            {
                slice0_lchip1_port++;
            }
            else if(1 == port_led[port].ctl_id)
            {
                slice1_lchip1_port++;
            }            
        }
    }   

    led_map_slice0_lchip0.mac_led_num = slice0_lchip0_port;
    led_map_slice0_lchip0.ctl_id = 0;
    led_map_slice0_lchip0.lchip = 0;
    led_map_slice0_lchip0.p_mac_id = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(uint8)*slice0_lchip0_port);

    led_map_slice1_lchip0.mac_led_num = slice1_lchip0_port;
    led_map_slice1_lchip0.ctl_id = 1;
    led_map_slice1_lchip0.lchip = 0;
    led_map_slice1_lchip0.p_mac_id = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(uint8)*slice1_lchip0_port);

    led_map_slice0_lchip1.mac_led_num = slice0_lchip1_port;
    led_map_slice0_lchip1.ctl_id = 0;
    led_map_slice0_lchip1.lchip = 1;
    led_map_slice0_lchip1.p_mac_id = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(uint8)*slice0_lchip1_port);

    led_map_slice1_lchip1.mac_led_num = slice1_lchip1_port;
    led_map_slice1_lchip1.ctl_id = 1;
    led_map_slice1_lchip1.lchip = 1;
    led_map_slice1_lchip1.p_mac_id = DRV_MALLOC(CTCLIB_MEM_DRIVER_LED_INFO, sizeof(uint8)*slice1_lchip1_port);

    slice0_lchip0_port = 0;
    slice1_lchip0_port = 0;
    slice0_lchip1_port = 0;
    slice1_lchip1_port = 0;

    for(port = 0; port < g_p_mac_led_info->mac_num; port++)
    {
        mac_led_mode_set(&port_led[port]);
        if(0 == port_led[port].lchip)
        {
            if(0 == port_led[port].ctl_id)
            {
                led_map_slice0_lchip0.p_mac_id[slice0_lchip0_port++] = port_led[port].port_id;
            }
            else if(1 == port_led[port].ctl_id)
            {
                led_map_slice1_lchip0.p_mac_id[slice1_lchip0_port++] = port_led[port].port_id;
            }
        }
        else if(1 == port_led[port].lchip)
        {
            if(0 == port_led[port].ctl_id)
            {
                led_map_slice0_lchip1.p_mac_id[slice0_lchip1_port++] = port_led[port].port_id;
            }
            else if(1 == port_led[port].ctl_id)
            {
                led_map_slice1_lchip1.p_mac_id[slice1_lchip1_port++] = port_led[port].port_id;
            }          
        }        
    }
    /* set mac led mapping */
    if(slice0_lchip0_port != 0)
    {
        ctc_chip_set_mac_led_mapping(&led_map_slice0_lchip0);
    }
    if(slice1_lchip0_port != 0)
    {
        ctc_chip_set_mac_led_mapping(&led_map_slice1_lchip0);
    }
    if(slice0_lchip1_port != 0)
    {
        ctc_chip_set_mac_led_mapping(&led_map_slice0_lchip1);
    }
    if(slice1_lchip1_port != 0)
    {
        ctc_chip_set_mac_led_mapping(&led_map_slice1_lchip1);
    }

    DRV_FREE(CTCLIB_MEM_DRIVER_LED_INFO, led_map_slice0_lchip0.p_mac_id);
    DRV_FREE(CTCLIB_MEM_DRIVER_LED_INFO, led_map_slice1_lchip0.p_mac_id);
    DRV_FREE(CTCLIB_MEM_DRIVER_LED_INFO, led_map_slice0_lchip1.p_mac_id);
    DRV_FREE(CTCLIB_MEM_DRIVER_LED_INFO, led_map_slice1_lchip1.p_mac_id);

    return 0;
}

int32 ctc_mac_led_init(uint8 lchip)
{
    ctc_chip_led_para_t led_para;
    uint32 enable = 0;
    //lchip = g_p_mac_led_info->mac_led_api_para[0][0].lchip;///TODO:

    /* set mac led polarity */
    led_para.lchip = lchip;
    led_para.ctl_id = 0;
#ifdef TSINGMA     
    led_para.lport_en = 0;
#endif
    led_para.op_flag = CTC_CHIP_LED_POLARITY_SET_OP;
    led_para.polarity = g_p_mac_led_info->polarity;
    ctc_chip_set_mac_led_mode(&led_para, CTC_CHIP_USING_ONE_LED);//One led or two led here both ok, it not use
#ifndef TSINGMA     
    led_para.ctl_id = 1;   
    ctc_chip_set_mac_led_mode(&led_para, CTC_CHIP_USING_ONE_LED);//One led or two led here both ok, it not use
#endif
    /* set mac led enable */
    enable = TRUE;
    ctc_chip_set_property(lchip, CTC_CHIP_MAC_LED_EN, (uint32*)&enable);

    return 0;
}

/*bug 23820, support mac led.*/
int32 mac_led_init()
{
    uint8 port = 0;
    uint8 lchip0_init =0;
    uint8 lchip1_init =0;
    mac_led_api_para_t* port_led;
    /* update mac led status, init mac led with table 0 para */
    mac_led_update(g_p_mac_led_info->mac_led_api_para[0]);

    port_led = g_p_mac_led_info->mac_led_api_para[0];
    for(port = 0; port < g_p_mac_led_info->mac_num; port++)
    {
        if((0 == port_led[port].lchip)&&(lchip0_init == 0))
        {
            ctc_mac_led_init(port_led[port].lchip);
            lchip0_init = 1;
        }
        else if((1 == port_led[port].lchip)&&(lchip1_init == 0))
        {
            ctc_mac_led_init(port_led[port].lchip);
            lchip1_init = 1;
        }
    }    

    return 0;
}

int32 led_cfg_port_mode(mac_led_api_para_t* port_led)
{
    int32 ret = 0;
    /* here do mac led mode set for real time show */
    ret = mac_led_mode_set(port_led);
    mac_led_info_modify(port_led);
    return ret;
}

/***************************************************************************************************
 * Name         : led_mgt_port_led 
 * Purpose      : configure mac led depand on g_p_mac_led_info        
 * Input        : N/A         
 * Output       : N/A              
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
int32 led_mgt_port_led()
{
    int ret = 0;
    //uint8 port  = 0;
    //uint8 port0 = 0;
    //uint8 port1 = 0;
    mac_led_api_para_t* p_mac_led_api_para = NULL;

    if(NULL == g_p_mac_led_info)
    {
        DRV_LOG_ERR("Management mac led: g_p_mac_led_info is NULL.\n");
        return RESULT_ERROR;
    }
    
    if(g_mac_table_id >= g_p_mac_led_info->table_num)
    {
        g_mac_table_id = 0;
    }
    
    p_mac_led_api_para = g_p_mac_led_info->mac_led_api_para[g_mac_table_id++];
    if(!p_mac_led_api_para)
    {
        g_mac_table_id = 0;
        return 0;
    }
    ret = mac_led_update(p_mac_led_api_para);
    //ret = mac_led_init(g_p_port_led_slice0, g_p_mac_led_info->slice0_mac_num);
    //ret += mac_led_init(g_p_port_led_slice1, g_p_mac_led_info->mac_num - g_p_mac_led_info->slice0_mac_num);

    return ret;
}

/*Bug35259, jqiu modify 2016-09-13.*/
int32 led_linkup_process(uint8 lchip, uint16 gport)
{
/* for bug 46827, added back by tongzb 2018.04.17 */
#ifdef GOLDENGATE
extern int32
sys_goldengate_port_get_mac_status(uint8 lchip, uint16 gport, uint32* p_value);
extern int32
sys_goldengate_port_reset_mac_en(uint8 lchip, uint16 gport, uint32 value);

    uint32 value;
    sys_goldengate_port_get_mac_status(lchip, gport, &value);
    if(value)
    {
        sys_goldengate_port_reset_mac_en(lchip, gport, 1);
        sys_goldengate_port_reset_mac_en(lchip, gport, 0);
    }
#endif
    return 0;
}

