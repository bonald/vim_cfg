/****************************************************************************
* $Id$
* phy api layer
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : mayc
* Date          : 2010-08-18
* Reason        : First Create.
* Revision      : R0.02
* Author        : jqiu
* Date          : 2016-07-19
* Reason        : Support CL73 auto, see bug39456.
****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "phy_drv.h"
#include "phy_api.h"
#include "drv_debug.h"

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
static phy_handle_t** g_phy_hdl = NULL;
static uint32 g_port_max_nums = 0; 

phy_handle_t* get_phy_hdl(uint32 port_id)
{
    phy_handle_t* phdl;
    
    if (port_id < 0 || port_id >= g_port_max_nums)
    {
        DRV_LOG_ERR("Invalid port id %d, max port nums %d.\n", port_id, g_port_max_nums);
        return NULL;
    }
    phdl = g_phy_hdl[port_id];

    return phdl;
}


int32 phy_cfg_flowctrl(uint32 port_id, uint16 direction, uint16 enable)
{
    phy_handle_t* phdl;
    phy_info_t *pphy_info;
    glb_port_flowctrl_t admin_flowctrl;
    flowctrl_adv_ability_t ability;
    int32 ret=0;
    phy_handle_t* p_next_hdl = NULL;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_flowctrl)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phdl->phy_info;

    if(((direction == GLB_FLOWCTRL_RECV)&&(enable == pphy_info->phy_manage_info.flowctrl.recv))
        ||((direction == GLB_FLOWCTRL_SEND)&&(enable == pphy_info->phy_manage_info.flowctrl.send)))
    {
        return RESULT_OK;
    }
    
    sal_mutex_lock(phdl->phy_info.pmutex);    
    
    if(direction == GLB_FLOWCTRL_RECV)
    {
        admin_flowctrl.recv = enable;
        admin_flowctrl.send = pphy_info->phy_manage_info.flowctrl.send;
    }
    else 
    {
        admin_flowctrl.send = enable;
        admin_flowctrl.recv = pphy_info->phy_manage_info.flowctrl.recv;
    }
    flowctrl_admin_to_ability(&admin_flowctrl, &ability);
    ret = phdl->config_flowctrl(phdl, ability.symmetric_pause, ability.asymmetric_pause);
    if(ret == RESULT_OK)
    {
        pphy_info->phy_manage_info.flowctrl_ability.symmetric_pause = ability.symmetric_pause;
        pphy_info->phy_manage_info.flowctrl_ability.asymmetric_pause = ability.asymmetric_pause;
        if(direction == GLB_FLOWCTRL_SEND)
        {
            phdl->phy_info.phy_manage_info.flowctrl.send = enable;
        }
        else
        {
            phdl->phy_info.phy_manage_info.flowctrl.recv = enable;
        }
        
        if (phdl->phy_info.next_phy_hdl != NULL)
        {
            p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);

            p_next_hdl->phy_info.phy_manage_info.flowctrl_ability.symmetric_pause = ability.symmetric_pause;
            p_next_hdl->phy_info.phy_manage_info.flowctrl_ability.asymmetric_pause = ability.asymmetric_pause;
            if(direction == GLB_FLOWCTRL_SEND)
            {
                p_next_hdl->phy_info.phy_manage_info.flowctrl.send = enable;
            }
            else
            {
                p_next_hdl->phy_info.phy_manage_info.flowctrl.recv = enable;
            }
        }
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);

    return ret;
}

int32 phy_cfg_enable(uint32 port_id, int8 enable)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_enable)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, enable);
        return RESULT_ERROR;
    }
    
    /*has no attach phy, just cfg original phy*/
    p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);
    if(p_next_hdl == NULL)
    {
        if(enable == phdl->phy_info.phy_manage_info.enable)
        {
            return RESULT_OK;
        }
        sal_mutex_lock(phdl->phy_info.pmutex);    
        ret = phdl->config_enable(phdl, enable);
        if(ret == RESULT_OK)
        {
            phdl->phy_info.phy_manage_info.enable = enable;
        }
#if (!defined DUET2)&&(!defined TSINGMA)
        /*Bug40938. jqiu optimize. faster do auto when enable port.*/
        phy_do_cl73_auto(port_id);
#endif
        sal_mutex_unlock(phdl->phy_info.pmutex);
    }
    /*has attach phy, must keep original phy enable always, and only cfg attach phy here. bug23865*/
    else
    {
        /*keep original phy enable.*/
        if(0 == phdl->phy_info.phy_manage_info.enable)
        {
            sal_mutex_lock(phdl->phy_info.pmutex);    
            ret = phdl->config_enable(phdl, 1);
            if(ret == RESULT_OK)
            {
                phdl->phy_info.phy_manage_info.enable = 1;
            }
            sal_mutex_unlock(phdl->phy_info.pmutex);
        }
                
        if(p_next_hdl->config_enable != NULL)
        {
            sal_mutex_lock(p_next_hdl->phy_info.pmutex);
            ret = p_next_hdl->config_enable((phy_handle_t*)(phdl->phy_info.next_phy_hdl), enable);
            if(ret == RESULT_OK)
            {
                p_next_hdl->phy_info.phy_manage_info.enable = enable;
            }
            sal_mutex_unlock(p_next_hdl->phy_info.pmutex);
        }        
    }
    return ret;
}

/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
int32 phy_cfg_master_slave_mode(uint32 port_id, glb_port_master_slave_t master_slave)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_master_slave)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, master_slave);
        return RESULT_ERROR;
    }
    if(master_slave == phdl->phy_info.phy_manage_info.master_slave)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->config_master_slave(phdl, master_slave);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.master_slave= master_slave;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}
/* End of liuht modified */

int32 phy_cfg_speed(uint32 port_id, glb_port_speed_t speed)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_speed)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, speed);
        return RESULT_ERROR;
    }
    /* for support 10/100/1000 copper_sfp, need keep the phdl->phy_info.pmutex out of p_next_hdl->phy_info.pmutex */
    sal_mutex_lock(phdl->phy_info.pmutex);
#if (!defined DUET2)&&(!defined TSINGMA)
    //sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->config_speed(phdl, speed);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.speed = speed;
    }
    //sal_mutex_unlock(phdl->phy_info.pmutex);
#else
    glb_port_speed_t tmp;
    tmp = phdl->phy_info.phy_manage_info.speed;
    //sal_mutex_lock(phdl->phy_info.pmutex);
    /* set the manage info first, because config_speed interface need this info for update cl73 ability */
    phdl->phy_info.phy_manage_info.speed = speed;
    ret = phdl->config_speed(phdl, speed);
    if(ret != RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.speed = tmp;/* if speed set fail, roll back speed */
    }
    //sal_mutex_unlock(phdl->phy_info.pmutex);
#endif
    /* store config speed to p_next_hdl for bug 37166 */
    p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);
    if(p_next_hdl)
    {
        p_next_hdl->phy_info.phy_manage_info.speed = speed;
        /*bug39540 jqiu 2016-07-15. sync copper SFP mac side auto cfg with MAC cfg.*/
        if(p_next_hdl->config_speed != NULL)
        {
            sal_mutex_lock(p_next_hdl->phy_info.pmutex);
            ret = p_next_hdl->config_speed((phy_handle_t*)(phdl->phy_info.next_phy_hdl), speed);
            sal_mutex_unlock(p_next_hdl->phy_info.pmutex);
        }
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);     
    return ret;
}

int32 phy_cfg_fec_enable(uint32 port_id, uint8 fec_en)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_fec_en)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, fec_en);
        return RESULT_ERROR;
    }
    if(fec_en == phdl->phy_info.phy_manage_info.fec_en)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    /*This sequence should not be changed. jqiu*/
    phdl->phy_info.phy_manage_info.fec_en = fec_en;
    ret = phdl->config_fec_en(phdl, fec_en);    
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 phy_cfg_duplex(uint32 port_id, glb_port_duplex_t duplex)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_duplex)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, duplex);
        return RESULT_ERROR;
    }
    /* for support 10/100/1000 copper_sfp, need keep the phdl->phy_info.pmutex out of p_next_hdl->phy_info.pmutex */
    sal_mutex_lock(phdl->phy_info.pmutex); 
#if (!defined DUET2)&&(!defined TSINGMA)
    //sal_mutex_lock(phdl->phy_info.pmutex);    
    ret = phdl->config_duplex(phdl, duplex);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.duplex = duplex;
    }
    //sal_mutex_unlock(phdl->phy_info.pmutex);
#else
    glb_port_duplex_t tmp;
    tmp = phdl->phy_info.phy_manage_info.duplex;
    //sal_mutex_lock(phdl->phy_info.pmutex);
    /* set the manage info first, because config_speed interface need this info for update cl73 ability */
    phdl->phy_info.phy_manage_info.duplex = duplex;
    ret = phdl->config_duplex(phdl, duplex);
    if(ret != RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.duplex = tmp;/* if duplex set fail, roll back duplex */
    }
    //sal_mutex_unlock(phdl->phy_info.pmutex);
#endif
    /* store config speed to p_next_hdl for bug 37166 */
    p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);
    if(p_next_hdl)
    {
        p_next_hdl->phy_info.phy_manage_info.duplex = duplex;
        /*bug39540 jqiu 2016-07-15. sync copper SFP mac side auto cfg with MAC cfg.*/
        if(p_next_hdl->config_duplex != NULL)
        {
            sal_mutex_lock(p_next_hdl->phy_info.pmutex);
            ret = p_next_hdl->config_duplex((phy_handle_t*)(phdl->phy_info.next_phy_hdl), duplex);
            sal_mutex_unlock(p_next_hdl->phy_info.pmutex);
        }
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);    
    return ret;
}

int32 phy_cfg_unidir(uint32 port_id, glb_port_unidir_t unidir)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_unidir)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, unidir);
        return RESULT_ERROR;
    }
    if(unidir == phdl->phy_info.phy_manage_info.unidir)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->config_unidir(phdl, unidir);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.unidir = unidir;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 phy_cfg_loopback(uint32 port_id, glb_lb_phy_t lb_mode)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_loopback)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, lb_mode);
        return RESULT_ERROR;
    }
    if(lb_mode == phdl->phy_info.phy_manage_info.lb_mode)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);    
    ret = phdl->config_loopback(phdl, lb_mode);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.lb_mode = lb_mode;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 phy_cfg_medium(uint32 port_id, glb_media_interface_t media_type)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_medium)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, media_type);
        return RESULT_ERROR;
    }
    if(media_type == phdl->phy_info.phy_manage_info.media_type)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);    
    ret = phdl->config_medium(phdl, phdl->phy_info.phy_manage_info.mac_if, media_type);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.media_type = media_type;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}


int32 phy_cfg_workmode(uint32 port_id, phy_work_mode_t mode)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_phy)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, mode);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);

    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 phy_cfg_syncE(uint32 port_id, int8 enable)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->enable_syncE)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, enable);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    
    phdl->phy_info.phy_manage_info.syncE_enable = enable;
    ret = phdl->enable_syncE(phdl, enable);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;    
}

int32 phy_cfg_clock(uint32 port_id, glb_vtss_phy_clock_conf_t phy_clk_conf)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_clock)
    {
        DRV_LOG_ERR("Invalid port id %d, %s.\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    
    phdl->phy_info.phy_manage_info.phy_clk_conf.phy_clk_port = phy_clk_conf.phy_clk_port;
    phdl->phy_info.phy_manage_info.phy_clk_conf.src = phy_clk_conf.src;
    phdl->phy_info.phy_manage_info.phy_clk_conf.freq = phy_clk_conf.freq;
    phdl->phy_info.phy_manage_info.phy_clk_conf.squelch = phy_clk_conf.squelch;
    ret = phdl->config_clock(phdl, phy_clk_conf);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;    
}

int32 phy_cfg_bypass(uint32 port_id, int8 enable)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl )
    {
        DRV_LOG_ERR("Invalid port id %d, %s. bypass %d\n", port_id, __FUNCTION__, enable);
        return RESULT_ERROR;
    }

    phdl->phy_info.phy_manage_info.bypass = enable;
    
    return ret;    
}

/*
*  mode: 0-disable tx, 1-enable tx, 2-check rx 
*  polynome_type: 0-PRBS7_+, 1-PRBS7_-, 2-PRBS15_+, 3-PRBS15_-, 4-PRBS23_+, 5-PRBS23_-, 6-PRBS31_+, 7-PRBS31_-
*  result: when check rx, this is the check result for prbs check
*/
int32 phy_cfg_prbs(uint32 port_id, uint8 mode,uint8 polynome_type,uint8 *result)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->config_prbs)
    {
        DRV_LOG_ERR("Invalid port id %d, %s.\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->config_prbs(phdl, mode,polynome_type,result);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;    
}

/*add by chenjr for bug 53838*/
int32 phy_cfg_led_mode(uint32 port_id,uint8 first_led_mode,uint8 second_led_mode)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        DRV_LOG_ERR("Invalid port id %d, %s.\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->config_led_mode(phdl,first_led_mode,second_led_mode);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;    
}

int32 gephy_reg_read(uint32 port_id, uint8 offset, uint16* val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_read)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }        
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    phyreg_para.dat.regaddr8 = offset;
    
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->reg_read(phdl, &phyreg_para, val);  
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 gephy_reg_write(uint32 port_id, uint8 offset, uint16 val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_write)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }   
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    phyreg_para.dat.regaddr8 = offset;

    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->reg_write(phdl, &phyreg_para, val);      
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 gephy_page_reg_read(uint32 port_id, uint8 phy_page, uint8 offset, uint16* val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_read)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }     
    
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    
    sal_mutex_lock(phdl->phy_info.pmutex);
    
    phyreg_para.dat.regaddr8 = 0x1f;
    ret = phdl->reg_write(phdl, &phyreg_para, phy_page);   
    
    phyreg_para.dat.regaddr8 = offset;
    ret = phdl->reg_read(phdl, &phyreg_para, val);  
    
    phyreg_para.dat.regaddr8 = 0x1f;
    ret = phdl->reg_write(phdl, &phyreg_para, PHY_PAGE_STANDARD);      

    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

int32 gephy_page_reg_write(uint32 port_id, uint8 phy_page, uint8 offset, uint16 val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_write)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }   
                            
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));

    sal_mutex_lock(phdl->phy_info.pmutex);

    phyreg_para.dat.regaddr8 = 0x1f;
    ret = phdl->reg_write(phdl, &phyreg_para, phy_page);   
    
    phyreg_para.dat.regaddr8 = offset;
    ret = phdl->reg_write(phdl, &phyreg_para, val);      
    
    phyreg_para.dat.regaddr8 = 0x1f;
    ret = phdl->reg_write(phdl, &phyreg_para, PHY_PAGE_STANDARD);  
    
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}


int32 xgphy_reg_read(uint32 port_id, uint8 dev, uint16 offset, uint16* val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_read)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }
    
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    phyreg_para.dev_no = dev;
    phyreg_para.dat.regaddr16 = offset;

    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->reg_read(phdl, &phyreg_para, val);  
    sal_mutex_unlock(phdl->phy_info.pmutex);

    return ret;
}

int32 xgphy_reg_write(uint32 port_id, uint8 dev, uint16 offset, uint16 val)
{
    phyreg_param_t phyreg_para;
    phy_handle_t* phdl;
    int32 ret;
    
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->reg_write)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, offset);
        return RESULT_ERROR;
    }
    
    memset(&phyreg_para, 0, sizeof(phyreg_param_t));
    phyreg_para.dev_no = dev;
    phyreg_para.dat.regaddr16 = offset;
    
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->reg_write(phdl, &phyreg_para, val);      
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return ret;
}

/**********************************************************
 *  Name:       : phy_get_link_poll
 *  Purpose     : poll phy status
 *  Input       : port id,phy_state_t,phy_state_change_t
 *  Output      : current status: link up or link down
                                  current speed
                                  current duplex
                  status change flag: up to down or down to up
                                      speed changed
                                      duplex changed
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : used in a timer;
 **********************************************************/
int32 phy_get_link_poll(uint32 port_id, glb_phy_state_t* phy_val, phy_state_change_t* phy_change)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->get_link_poll)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. \n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    
    /*phy need be mutexed because next_phy_hdl may be modified by copper sfp attach/deattach.bug28514*/
    if (!sal_mutex_try_lock(phdl->phy_info.pmutex))
    {
        return RESULT_ERROR; 
    }
    
    /*has no attach phy, use original phy link polling*/
    p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);
    if(p_next_hdl == NULL)
    { 
        phdl->get_link_poll(phdl, phy_val, phy_change); 
    }

    /*has attach phy bug23865.*/
    else
    {
        if(p_next_hdl->get_link_poll != NULL)
        {
            if (!sal_mutex_try_lock(p_next_hdl->phy_info.pmutex))
            {
                sal_mutex_unlock(phdl->phy_info.pmutex);
                return RESULT_ERROR;        
            }
            p_next_hdl->get_link_poll(p_next_hdl, phy_val, phy_change);            
            sal_mutex_unlock(p_next_hdl->phy_info.pmutex);
        }
    }    
    sal_mutex_unlock(phdl->phy_info.pmutex);
    return RESULT_OK;
}

/* modified by liuht for bug 26641, 2014-0422 */
/* get current phy status */
/**********************************************************
 *  Name:       : phy_get_cur_status
 *  Purpose     : get phy status
 *  Input       : port id,phy_state_t
 *  Output      : current status: link up or link down
                                  current speed
                                  current duplex
 *  Return      : RESULT_ERROR or RESULT_OK
 **********************************************************/
int32 phy_get_cur_status(uint32 port_id, glb_phy_state_t* phy_val)
{
    phy_handle_t* phdl;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->get_cur_status)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. \n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    if (!sal_mutex_try_lock(phdl->phy_info.pmutex))
    {
        return RESULT_ERROR;        
    }
    phdl->get_cur_status(phdl, phy_val);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return RESULT_OK;
}
/* end of liuht modified */

/**********************************************************
 *  Name:       : phy_get_link_interrupt
 *  Purpose     : poll phy status
 *  Input       : port id,phy_state_t,phy_state_change_t
 *  Output      : current status: link up or link down
                                  current speed
                                  current duplex
                  status change flag: up to down or down to up
                                      speed changed
                                      duplex changed
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : used for phy interrupt
 **********************************************************/
int32 phy_get_link_interrupt(uint8 port_id, glb_phy_state_t* phy_val, phy_state_change_t* phy_change)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->get_link_interupt)
    {
        DRV_LOG_ERR("Invalid port id %d, %s.\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    phdl->get_link_interupt(phdl, phy_val, phy_change);
    /*Bug42054. If phy has attach phy, do nothing in interrupt, wait polling update.*/
    p_next_hdl = (phy_handle_t*)(phdl->phy_info.next_phy_hdl);
    if(p_next_hdl != NULL)
    {
        phy_change->action = -1;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return RESULT_OK;
}

int32 phy_check_no_phy(uint32 port_id)
{
    phy_handle_t* phdl;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        DRV_LOG_ERR("Check phy, invalid port id %d, %s.\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    if(phdl->phy_info.phy_device_type == PORT_PHY_NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
int32 phy_re_init(uint32 port_id)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->phy_init)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. %d\n", port_id, __FUNCTION__, phy_init);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->phy_init(phdl);
    sal_mutex_unlock(phdl->phy_info.pmutex);    
    return ret;
}

/* Modified by liuht to support eee for bug 28298, 2014-05-16 */
int32 phy_cfg_eee_capability(uint32 port_id, uint32 enable)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->enable_eee)
    {
        DRV_LOG_ERR("Invalid port id %d, %s\n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    if(enable == phdl->phy_info.phy_manage_info.eee_enable)
    {
        return RESULT_OK;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->enable_eee(phdl, enable);
    if(ret == RESULT_OK)
    {
        phdl->phy_info.phy_manage_info.eee_enable = enable;
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);    
    return ret;
}
/* end of liuht modified */

int32 phy_init(phy_handle_t** pphdl, uint32 max_port_nums)
{
    int32 port_num;
    int32 ret;
#ifdef GREATBELT
    uint16 val;
    uint16 access_phy_fail_flag = 0;
    uint16 mdio_reset_count = 0;
#endif

    if(NULL == pphdl)
    {
        DRV_LOG_ERR("PHY init param invalid.\n");
        return RESULT_ERROR;
    }    

    /*the high 16 bits means the port num which not used on the first phy chip,the low 16 bits mean the real port num.
    modify  to adapter LS2H-SG8348C,which prior some port on the first phy chip is not used,guhg,2016-04-25*/
    g_phy_hdl = &pphdl[max_port_nums >> 16];
    g_port_max_nums = max_port_nums & 0xff;
    max_port_nums = ((max_port_nums &0xff) + (max_port_nums >> 16));

#ifdef GREATBELT
    /* Added by liuht for bug24800, 2014-02-11 */
    while(1)
    {
        for(port_num = 0; port_num < g_port_max_nums; port_num++)
        {
            if(PORT_PHY_NULL != g_phy_hdl[port_num]->phy_info.phy_device_type)
            {
                ret = gephy_reg_read(port_num, 2, &val);
                if(ret < 0)
                {
                    access_phy_fail_flag = 1;
                    extern int32 sys_greatbelt_chip_reset_mdio(uint8 lchip);
                    sys_greatbelt_chip_reset_mdio(0);
                    mdio_reset_count++;
                    break;
                }
            }
        }
        if((access_phy_fail_flag == 0) || (mdio_reset_count >= 5))
        {
            break;
        }
        else
        {
            access_phy_fail_flag = 0;
        }
    }
#endif    
    for(port_num = 0; port_num < max_port_nums; port_num++)
    {
        if(NULL != pphdl[port_num]->phy_init)
        {
            ret = pphdl[port_num]->phy_init(pphdl[port_num]);
            if(ret)
            { 
                DRV_LOG_ERR("PHY init failed, port %d\n", port_num+1);
                DRV_CONSOLE_PRINT("PHY init failed, port %d. ret %d\n", port_num+1, ret); 
            }
        }
    }
    
    //for mv88e1680 init
    for (port_num = 0; port_num < g_port_max_nums; port_num++)
    {
        if(NULL != g_phy_hdl[port_num]->phy_post_init)
        {
            ret = g_phy_hdl[port_num]->phy_post_init(g_phy_hdl[port_num]);
            if(ret)
            { 
                DRV_LOG_ERR("PHY post init failed, port %d\n", port_num+1);
                DRV_CONSOLE_PRINT("PHY post init failed, port %d. ret %d\n", port_num+1, ret); 
            }
        }
    }

    return RESULT_OK;
}

int32 phy_set_vct_mod(uint32 port_id)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if (NULL == phdl || NULL == phdl->phy_set_vct_mod)
    {
        DRV_LOG_ERR("Invalid port id %d. \n", port_id);
        return RESULT_ERROR;
    }
    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->phy_set_vct_mod(phdl);
    sal_mutex_unlock(phdl->phy_info.pmutex);

    return ret;  
}

int32
phy_get_cable_info(uint32 port_id, glb_port_cable_info_t * p_cable_info)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if (NULL == phdl || NULL == phdl->phy_get_vct_info)
    {
        DRV_LOG_ERR("Invalid port id %d. \n", port_id);
        return GLB_VCT_ERROR;
    }

    sal_mutex_lock(phdl->phy_info.pmutex);
    ret = phdl->phy_get_vct_info(phdl, p_cable_info);
    if (ret)
    {        
        sal_mutex_unlock(phdl->phy_info.pmutex);
        return ret;
    }

    /* get current speed  */
    if (p_cable_info->port_link_up)
    {
        p_cable_info->cable_speed = phdl->phy_info.phy_stat_flag.speed;
    }
    else
    {
        p_cable_info->cable_speed = GLB_SPEED_MAX;
    }
    p_cable_info->port_enable  = phdl->phy_info.phy_manage_info.enable;
    sal_mutex_unlock(phdl->phy_info.pmutex);
    return GLB_VCT_OK;
}
/*bug23865 support copper SFP*/
/**********************************************************
 *  Name:       : phy_attach_copper_sfp_phy
 *  Purpose     : Add new copper SFP phy handle to original phy;
 *  Input       : port number;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : If the copper SFP phy needn't phy handle, attach NULL.
 **********************************************************/
int32 phy_attach_copper_sfp_phy(uint32 port_id)
{
    phy_info_t phy_info;
    phy_handle_t* copper_sfp_phdl;
    phy_handle_t* phdl;
        
    phdl = get_phy_hdl(port_id);
    if (NULL == phdl)
    {
        DRV_LOG_ERR("Invalid port id %d. \n", port_id);
        return RESULT_ERROR;
    }
    /*phy need be mutexed because link polling will use next_phy_hdl in another thread. bug28514*/
    sal_mutex_lock(phdl->phy_info.pmutex);
    memset(&phy_info, 0, sizeof(phy_info));
    phy_info.phy_device_type = PORT_PHY_SFP;
    phy_info.port_num = port_id;    
    memcpy(&(phy_info.phy_manage_info), &(phdl->phy_info.phy_manage_info), sizeof(phy_manage_info_t));
    memcpy(&(phy_info.phy_stat_flag), &(phdl->phy_info.phy_stat_flag), sizeof(glb_phy_state_t));
    copper_sfp_phdl = phy_dev_register(&phy_info);
    if(copper_sfp_phdl != NULL)
    {
        DRV_LOG_DEBUG(DRV_PHY, "Port %d attach copper SFP phy handle",port_id) 
    }
    phdl->phy_info.next_phy_hdl = copper_sfp_phdl;
    sal_mutex_unlock(phdl->phy_info.pmutex);
    return RESULT_OK;
}

int32 phy_deattach_copper_sfp_phy(uint32 port_id)
{
    phy_handle_t* phdl;
    phy_handle_t* p_next_hdl;
    
    phdl = get_phy_hdl(port_id);    
    if (NULL == phdl)
    {
        DRV_LOG_ERR("Invalid port id %d. \n", port_id);
        return RESULT_ERROR;
    }
    /*phy need be mutexed because link polling will use next_phy_hdl in another thread. bug28514, bug30441*/
    sal_mutex_lock(phdl->phy_info.pmutex);
    if(phdl->phy_info.next_phy_hdl != NULL)
    {
        p_next_hdl = (phy_handle_t* )(phdl->phy_info.next_phy_hdl);
        DRV_LOG_DEBUG(DRV_PHY, "Port %d remove copper SFP phy handle",port_id)         
        memcpy(&(phdl->phy_info.phy_stat_flag), \
            &(p_next_hdl->phy_info.phy_stat_flag), sizeof(phdl->phy_info.phy_stat_flag));
        phy_dev_unregister(phdl->phy_info.next_phy_hdl);
        phdl->phy_info.next_phy_hdl = NULL;
#if 0        
        /*clear original phy stat info to default down*/
        memset(&(phdl->phy_info.phy_stat_flag), 0, sizeof(phdl->phy_info.phy_stat_flag));
#endif
    }
    sal_mutex_unlock(phdl->phy_info.pmutex);
    return RESULT_OK;
}

/*Check whether phy has attach a new copper SFP phy handle successfully*/
int32 phy_check_attach_phy(uint32 port_id)
{
    phy_handle_t* phdl;
    
    phdl = get_phy_hdl(port_id);
    if (NULL == phdl)
    {
        DRV_LOG_ERR("Invalid port id %d. \n", port_id);
        return 0;
    }
    if(phdl->phy_info.next_phy_hdl == NULL)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/*bug30363 jqiu add 2014-10-23 for customer request*/
int32 phy_config_phy_sig_test_mode(uint32 port_id, uint8 serdes_id, uint8 mode)
{
    phy_handle_t* phdl;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl || NULL == phdl->get_cur_status)
    {
        DRV_LOG_ERR("Invalid port id %d, %s. \n", port_id, __FUNCTION__);
        return RESULT_ERROR;
    }
    if (!sal_mutex_try_lock(phdl->phy_info.pmutex))
    {
        return RESULT_ERROR;        
    }
    phdl->config_phy_sig_test_mode(phdl, serdes_id, mode);
    sal_mutex_unlock(phdl->phy_info.pmutex);
    
    return RESULT_OK;
}

int32 phy_update_clause73_ability(uint32 port_id, uint32 ability)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    phdl->phy_info.phy_manage_info.cl73_speed_ability = ability;
    return ret;
}

int32 phy_get_clause73_ability(uint32 port_id, uint32* ability)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    *ability = phdl->phy_info.phy_manage_info.cl73_speed_ability;    
    return ret;
}

int32 phy_update_fec_ability(uint32 port_id, uint32 ability)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    phdl->phy_info.phy_manage_info.fec_ability = ability;
    return ret;
}

int32 phy_update_fiber_ability(uint32 port_id, uint8 fiber_ability, uint8 is_DAC)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    phdl->phy_info.phy_manage_info.fiber_ability = fiber_ability;
    phdl->phy_info.phy_manage_info.is_DAC = is_DAC;
    
    return ret;
}
int32 phy_get_fiber_ability(uint32 port_id, uint8* fiber_ability, uint8* is_DAC)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    *fiber_ability = phdl->phy_info.phy_manage_info.fiber_ability;
    *is_DAC = phdl->phy_info.phy_manage_info.is_DAC;
    
    return ret;
}

int32 phy_update_fec_en(uint32 port_id, uint8 fec_en)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    phdl->phy_info.phy_manage_info.fec_en = fec_en;
    
    return ret;
}
int32 phy_get_fec_en(uint32 port_id, uint8* fec_en)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    *fec_en = phdl->phy_info.phy_manage_info.fec_en;
    
    return ret;
}

int32 phy_config_link_tolerable_time(uint32 port_id, uint32 time)
{
    phy_handle_t* phdl;
    int32 ret = 0;

    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    phdl->phy_info.phy_manage_info.link_tolerable_time = time;
    
    return ret;
}

int32 phy_get_link_tolerable_time(uint32 port_id)
{
    phy_handle_t* phdl;
    phdl = get_phy_hdl(port_id);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    
    return phdl->phy_info.phy_manage_info.link_tolerable_time;
}


extern int32 phy_null_do_cl73_auto(phy_handle_t* phy_handle);
int32 phy_do_cl73_auto(uint32 port_id)
{
    phy_handle_t* phdl;
    
    phdl = get_phy_hdl(port_id);    
    phy_null_do_cl73_auto(phdl);
    return 0;
}

extern int aqr40x_load_firmware(phy_handle_t* phy_handle, const char *pathname);
int32 phy_load_firmware(uint32 port_id, const char *pathname)
{
    phy_handle_t* phdl;
    uint32 ret = RESULT_ERROR;
    phdl = get_phy_hdl(port_id);
    if((PORT_PHY_AQR407 == phdl->phy_info.phy_device_type)||(PORT_PHY_AQR411 == phdl->phy_info.phy_device_type))
    {
        if(NULL == phdl)
        {
            DRV_LOG_ERR("Invalid port id %d, %s. \n", port_id, __FUNCTION__);
            return RESULT_ERROR;
        }
        if (!sal_mutex_try_lock(phdl->phy_info.pmutex))
        {
            return RESULT_ERROR;        
        }
        ret = aqr40x_load_firmware(phdl,pathname);
        sal_mutex_unlock(phdl->phy_info.pmutex); 
    }
    
    return ret;
}
extern int check_firmware_version(phy_handle_t* phy_handle,firmware_ver_t *version);
int32 phy_check_firmware_version(uint32 port_id, const char *pathname)
{
    phy_handle_t* phdl;
    firmware_ver_t version;
    uint32 ret = RESULT_ERROR;
    phdl = get_phy_hdl(port_id);
    if(PORT_PHY_AQR407 == phdl->phy_info.phy_device_type)
    {
        if(NULL == phdl)
        {
            DRV_LOG_ERR("Invalid port id %d, %s. \n", port_id, __FUNCTION__);
            return RESULT_ERROR;
        }
        if (!sal_mutex_try_lock(phdl->phy_info.pmutex))
        {
            return RESULT_ERROR;        
        }
        ret = check_firmware_version(phdl,&version);
        sal_mutex_unlock(phdl->phy_info.pmutex); 
    }
    
    return ret;
}

void _phy_cfg_all_enable(int8 enable);
void _phy_cfg_all_loopback(glb_lb_phy_t lb_mode);
int32 _phy_check_link_status(uint32 port_id);


int32
_phy_diagnostic_test(phy_diag_result_t *diag_result)
{
    uint32 port;
    uint32 ret1 = RESULT_OK, ret2 = RESULT_OK;
    glb_lb_phy_t loopback ;  //0:LB_NONE   1:LB_PHY_FAR     2:LB_PHY_NEAR	
    phy_handle_t** phy_hdl = g_phy_hdl;
    uint8 phy_test_count = 0;
    uint8 phy_test_fail_flag = 0;

    DRV_CTC_CHK_PTR(diag_result);

    _phy_cfg_all_enable(1);
    sleep(2);
    loopback = GLB_LB_PHY_NEAR_DIAG;   
    _phy_cfg_all_loopback(loopback);   
    sleep(3); 
    /* check for 5 times */
    while(1)
    {
        phy_test_fail_flag = 0;
        for(port = 0; port < g_port_max_nums; port++)
        {
            if(PORT_PHY_NULL != phy_hdl[port]->phy_info.phy_device_type)
            {
                ret1 = _phy_check_link_status(port);
                ret1 = _phy_check_link_status(port);
                if(ret1 == 0)
                {                       
                    if(phy_test_count >= 5)
                    {
                        diag_result->down = 1;
                        diag_result->port_id = port;
                        ret2 = RESULT_ERROR;
                        goto finish;
                    }
                    else
                    {
                        phy_test_count++;
                        phy_test_fail_flag = 1;
                        sleep(1);
                        break;
                    }
                }
            }
        }

        if(0 == phy_test_fail_flag)
        {
            goto finish;
        }            
    }

finish:
    _phy_cfg_all_enable(0);
    loopback = GLB_LB_NONE;
    _phy_cfg_all_loopback(loopback); 
    return ret2;
}

void 
_phy_cfg_all_enable(int8 enable)
{
    uint32 port_id;
    for(port_id = 0; port_id < g_port_max_nums; port_id++)
    {
        if(PORT_PHY_NULL != g_phy_hdl[port_id]->phy_info.phy_device_type)
        {
            phy_cfg_enable(port_id, enable);
        }
    }
}

/* modified by liuht for bug 26641, 2014-0422 */
void
_phy_cfg_all_loopback(glb_lb_phy_t lb_mode)
{
    uint32 port_id;
    for(port_id = 0; port_id < g_port_max_nums; port_id++)
    {
        if(PORT_PHY_NULL != g_phy_hdl[port_id]->phy_info.phy_device_type)
        {
            phy_cfg_loopback(port_id, lb_mode);
        }
    }
}

/* modified by liuht for bug 26641, 2014-0422 */
int32
_phy_check_link_status(uint32 port_id)
{
    glb_phy_state_t phy_val;

    if(PORT_PHY_NULL != g_phy_hdl[port_id]->phy_info.phy_device_type)
    {
        memset(&phy_val,0,sizeof(glb_phy_state_t));
        phy_get_cur_status(port_id, &phy_val);
        return phy_val.link_up;
    }
    else
    {
        return -1;
    }
}

int32
phy_diagnostic_test_and_report(void)
{
#ifndef _GLB_UML_SYSTEM_   
    int32 result=0;
    phy_diag_result_t diag_result;

    sal_memset(&diag_result, 0, sizeof(phy_diag_result_t));
    result = _phy_diagnostic_test(&diag_result);
    if(result != 0)
    {
        if(diag_result.access_fail)
        {
            DRV_DIAG_LOG("PHY diag access fail!\n");	
        }
        else if(diag_result.down)
        {
            DRV_DIAG_LOG("PHY diag port %d link down\n", diag_result.port_id);	
        }
        return RESULT_ERROR;
    }
    else
    {
        DRV_DIAG_LOG("Phy diag pass!\n");
        return RESULT_OK;
    }
#else
    return RESULT_OK;
#endif
}

#if (defined DUET2) || (defined TSINGMA)
int32 phy_get_autoneg_ability(uint32 port_id, uint32* ability)
{
    if (g_phy_hdl[port_id]->get_autoneg_ability != NULL)
    {
        g_phy_hdl[port_id]->get_autoneg_ability(g_phy_hdl[port_id], ability);
    }

    return 0;
}
#endif
