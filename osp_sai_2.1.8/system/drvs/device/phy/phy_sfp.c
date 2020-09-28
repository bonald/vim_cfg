/****************************************************************************
* $Id$
* phy SFP device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jqiu 
* Date          : 2013-11-15
* Reason        : First Create for bug 23865
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_sfp.h"
#include "fiber_drv.h"
#include "fiber_api.h"
#define MARVELL_88E1111_PART 0x0cc0
#define MARVELL_88E1112_PART 0x0c90
#define COPPER_SFP_UNKNOWN_PART 0xffff

/*Marvell 88E1111/1112 phy support many page in phy register, this function used to select page*/
static int32 sfp_select_phy_page(uint32 port_id, uint8 page)
{
    fiber_para_t sfp_para;
    uint8 data[2];
    int32 ret = 0;
    
    data[0] = 0;
    data[1] = page;
    sfp_para.subdev = FIBER_DEV_ADDR3;
    sfp_para.offset = 22;
    sfp_para.len = 2;
    sfp_para.val = data;
    ret = fiber_write(port_id, &sfp_para);
    return ret;
}

int32 phy_sfp_get_chip_type(uint16 port_id, uint16* part_num, uint8* revision)
{
    uint16 reg3;
    fiber_para_t sfp_para;
    uint8 data[2];
    int32 ret=RESULT_OK;

    sfp_para.subdev = FIBER_DEV_ADDR3;
    sfp_para.len = 2;
    sfp_para.val = data;    
    sfp_para.offset = 3;
    ret = fiber_read(port_id, &sfp_para);
    reg3 = (data[0]<<8) + data[1];
    *part_num = reg3 & 0xfff0;
    *revision = reg3 & 0xf;
    if(ret != RESULT_OK)
    {
        *part_num = COPPER_SFP_UNKNOWN_PART;
    }    
    return RESULT_OK;
}

/*This only do for sfp phy which need be managed.*/
int32 phy_sfp_init_chip(phy_info_t* pphy_info, uint16 part_num)
{    
    uint16 port_id;
    fiber_para_t sfp_para;
    uint8 data[2];
    int ret=RESULT_OK;

    /*adjust Marvell phy tx/rx fifo to support 9216 byte jumbo frame wirespeed on 100M bug 11039*/
    port_id = pphy_info->port_num;    
    sfp_para.subdev = FIBER_DEV_ADDR3;
    sfp_para.len = 2;
    sfp_para.val = data;    
    data[0]=0x50;
    data[1]=0x78;
    sfp_para.offset = 16;    
    ret += fiber_write(port_id, &sfp_para);
    return ret;
}

/*Check this copper SFP work mode, and decide whether it need manage status by I2C.*/
int32 phy_sfp_need_manage_status(uint16 port_id, uint16 part_num)
{
    fiber_para_t sfp_para;
    uint8 data[2];
    int32 ret = RESULT_OK;
    uint16 mode;
    
    if(part_num == MARVELL_88E1111_PART)
    {
        sfp_para.subdev = FIBER_DEV_ADDR3;
        sfp_para.len = 2;
        sfp_para.val = data;    
        sfp_para.offset = 27;
        ret = fiber_read(port_id, &sfp_para);
        if(ret < 0)
        {
            return 0;
        }
        mode = data[1]&0xf;
        /*media 1000BASE-T, mac auto || no-auto*/
        if((mode == 0x8)||(mode == 0xc))
        {
            return 0;
        }
        /*media 10/100/1000BASE-T, mac no clock || clock*/
        else if((mode == 0x4)||(mode == 0x0))
        {
            return 1;
        }
        else /* other mode*/
        {
            return 0;
        }
    }
    else if(part_num == MARVELL_88E1112_PART)
    {
        /*read out work mode*/
        sfp_select_phy_page(port_id, 2);
        sfp_para.subdev = FIBER_DEV_ADDR3;
        sfp_para.len = 2;
        sfp_para.val = data;    
        sfp_para.offset = 16;
        ret = fiber_read(port_id, &sfp_para);
        if(ret < 0)
        {
            return 0;
        }
        mode = ((data[0]&0x3)<<1) | ((data[1]&0x80)>>7);
        /*media 1000BASE-T*/
        if(mode == 0x1)
        {
            return 0;
        }
        /*media 10/100/1000BASE-T*/
        else if((mode == 0x2)||(mode == 0x3)||(mode == 0x5))
        {
            return 1;
        }
        else/*other mode*/
        {
            return 0;
        }
    }
    return 0;
}

int32 phy_sfp_enable(phy_handle_t* phy_handle, int8 enable)
{
    uint16 port_id;
    fiber_para_t sfp_para;
    uint8 data[2];
    int ret=RESULT_OK;

    port_id = phy_handle->phy_info.port_num;
    /*go to page 0 to cfg phy enable*/
    sfp_select_phy_page(port_id, 0);
    /*read register 0 value*/
    sfp_para.subdev = FIBER_DEV_ADDR3;
    sfp_para.len = 2;
    sfp_para.val = data;    
    sfp_para.offset = 0;
    ret += fiber_read(port_id, &sfp_para);
    if(enable)
        data[0] &= 0xf7;
    else
        data[0] |= 0x8;
    DRV_LOG_DEBUG(DRV_PHY,   "phy sfp port %d enable %d config offset 0x%x val 0x%x%x", 
        port_id,enable, sfp_para.offset,data[0],data[1]);    
    ret += fiber_write(port_id, &sfp_para);
    if(phy_handle->phy_info.part_num == MARVELL_88E1111_PART)
    {
        /*return to page 1 default*/
        sfp_select_phy_page(port_id, 1);
    }
    return ret;
}

/*Bug39540. For copper SFP, speed/duplex set only config mac side, and media side just do auto-neg*/
int32 phy_sfp_conf_set(phy_handle_t* phy_handle, uint8 speed, uint8 duplex)
{
    int32 ret=RESULT_OK;
    uint16 port_id;
    fiber_para_t sfp_para;
    uint8 data[2];

    port_id = phy_handle->phy_info.port_num;
    if((speed == GLB_SPEED_AUTO)||(duplex == GLB_DUPLEX_AUTO))
    {
        DRV_LOG_DEBUG(DRV_PHY,   "phy sfp port %d reset auto-neg", port_id);
        /*enable mac side auto-neg*/
        if(phy_handle->phy_info.part_num == MARVELL_88E1111_PART)
        {
            /*init on page 1, needn't change page.*/
            sfp_para.subdev = FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 0;
            data[0] = 0x90;
            data[1] = 0x40;
            ret += fiber_write(port_id, &sfp_para);
        }
        else
        {
            /*init on page 0, need change to page 2*/
            ret += sfp_select_phy_page(port_id, 2);
            sfp_para.subdev = FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 0;
            data[0] = 0x90;
            data[1] = 0x40;
            ret += fiber_write(port_id, &sfp_para);
            ret += sfp_select_phy_page(port_id, 0);
        }
    }
    else
    {
        DRV_LOG_DEBUG(DRV_PHY,   "phy sfp port %d disable auto-neg", port_id);
        /*disable mac side auto-neg*/
        if(phy_handle->phy_info.part_num == MARVELL_88E1111_PART)
        {
            /*init on page 1, needn't change page.*/
            sfp_para.subdev = FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 0;
            data[0] = 0x80;
            data[1] = 0x40;
            ret += fiber_write(port_id, &sfp_para);
        }
        else
        {
            /*init on page 0, need change to page 2*/
            ret += sfp_select_phy_page(port_id, 2);
            sfp_para.subdev = FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 0;
            data[0] = 0x80;
            data[1] = 0x40;
            ret += fiber_write(port_id, &sfp_para);
            ret += sfp_select_phy_page(port_id, 0);
        }
    }
    /* Modified by tongzb for bug 50889 */
    if(phy_handle->phy_info.phy_manage_info.enable == 0)
    {
        phy_sfp_enable(phy_handle,phy_handle->phy_info.phy_manage_info.enable);
    }      
    return ret;
}

/*Bug39540 jqiu 2016-07-15. support cfg copper sfp auto-neg of mac side */
int32 phy_sfp_speed(phy_handle_t* phy_handle, uint8 speed)
{
    int32 ret = 0;
    phy_info_t *pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    ret = phy_sfp_conf_set(phy_handle, speed, pphy_info->phy_manage_info.duplex);
    return ret;    
}
/*Bug39540 jqiu 2016-07-15. support cfg copper sfp auto-neg of mac side */
int32 phy_sfp_duplex(phy_handle_t* phy_handle, uint8 duplex)
{
    int32 ret = 0;
    phy_info_t *pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    ret = phy_sfp_conf_set(phy_handle, pphy_info->phy_manage_info.speed, duplex);
    return ret;    
}

int32 phy_sfp_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t *phy_val)
{
    uint16 port_id;
    fiber_para_t sfp_para;
    uint8 data[2];
    int ret = 0;

    port_id = phy_handle->phy_info.port_num;
    if(phy_handle->phy_info.phy_manage_info.enable)
    {
        if(phy_handle->phy_info.part_num == MARVELL_88E1111_PART)
        {
            /*read link status and speed duplex, in page 1*/
            sfp_para.subdev= FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 4;
            ret = fiber_read(port_id, &sfp_para);
            if(ret)
            {
                DRV_LOG_ERR("SFP get phy status err");
                phy_val->link_up = GLB_LINK_DOWN;
            }
            else
            {
                phy_val->link_up = ((data[0]&0x80) == 0) ? GLB_LINK_DOWN : GLB_LINK_UP;
            }
            
            if((data[0]&0x10) == 0)
            {
                phy_val->duplex = GLB_DUPLEX_HALF;
            }
            else
            {
                phy_val->duplex = GLB_DUPLEX_FULL;
            }
            if((data[0]&0xc) == 0)
            {
                phy_val->speed = GLB_SPEED_10M;
            }
            else if((data[0]&0xc) == 4)
            {
                phy_val->speed = GLB_SPEED_100M;
            }
            else
            {
                phy_val->speed = GLB_SPEED_1G;
            }
        }
        else if(phy_handle->phy_info.part_num == MARVELL_88E1112_PART)
        {            
            /*read link status and speed duplex, in page 0*/
            sfp_para.subdev= FIBER_DEV_ADDR3;
            sfp_para.len = 2;
            sfp_para.val = data;    
            sfp_para.offset = 17;
            ret = fiber_read(port_id, &sfp_para);
            if(ret)
            {
                DRV_LOG_ERR("SFP get phy status err");
                phy_val->link_up = GLB_LINK_DOWN;
            }
            else
            {
                phy_val->link_up = ((data[0]&0x4) == 0) ? GLB_LINK_DOWN : GLB_LINK_UP;
            }
            if((data[0]&0xc0) == 0)
            {
                phy_val->speed = GLB_SPEED_10M;
            }
            else if((data[0]&0xc0) == 0x40)
            {
                phy_val->speed = GLB_SPEED_100M;
            }
            else
            {
                phy_val->speed = GLB_SPEED_1G;
            }

            if((data[0]&0x20) == 0)
            {
                phy_val->duplex = GLB_DUPLEX_HALF;
            }
            else
            {
                phy_val->duplex = GLB_DUPLEX_FULL;
            }
        }
        else
        {
            phy_val->link_up = GLB_LINK_DOWN;
            phy_val->speed = GLB_SPEED_AUTO;
            phy_val->duplex = GLB_DUPLEX_AUTO;
        }
    }
    else
    {
        phy_val->link_up = GLB_LINK_DOWN;
        phy_val->speed = GLB_SPEED_AUTO;
        phy_val->duplex = GLB_DUPLEX_AUTO;
    }

    /* Added by liuht for bug 37166, 2016-02-18 */
    /* If config speed is 10G, link down is desired */
    if(GLB_SPEED_10G == phy_handle->phy_info.phy_manage_info.speed)
        phy_val->link_up = GLB_LINK_DOWN;
    
    phy_val->linkup_media = GLB_PORT_TYPE_FIBER;
    phy_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;
    phy_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    return ret;
}

int32 phy_sfp_get_link_poll(phy_handle_t* phy_handle, glb_phy_state_t* phy_val, 
                            phy_state_change_t* phy_change)
{
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    pphy_info = &phy_handle->phy_info;
    phy_sfp_get_cur_status(phy_handle, phy_val);
    
    /*link change from down to up*/
    if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up != GLB_LINK_DOWN))
    {
        phy_change->action = 1;
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
        DRV_LOG_DEBUG(DRV_PHY, "phy sfp addr %d link up",pphy_info->port_num) 
    }
    /*link change from up to down*/
    else if((pphy_info->phy_stat_flag.link_up != phy_val->link_up)
        &&(phy_val->link_up == GLB_LINK_DOWN))
    {
        phy_change->action = 0;
        DRV_LOG_DEBUG(DRV_PHY, "phy sfp addr %d link down",pphy_info->port_num) 
    }
    /*link always up*/
    else if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_change->action = -1;
        if(pphy_info->phy_stat_flag.speed != phy_val->speed)
        {            
            phy_change->speed_change = 1;
        }
        if(pphy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
            pphy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
        {
            phy_change->flowctrl_change = 1;
        }
    }
    /*link always down*/
    else
    {
        phy_change->action = -1;
    }
    pphy_info->phy_stat_flag.link_up = phy_val->link_up;
    pphy_info->phy_stat_flag.speed = phy_val->speed;
    pphy_info->phy_stat_flag.duplex = phy_val->duplex;
    pphy_info->phy_stat_flag.linkup_media = phy_val->linkup_media;
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        pphy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        pphy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return RESULT_OK;

}


phy_handle_t* phy_sfp_dev_register(phy_info_t* pphy_info)
{
    phy_handle_t* phdl = NULL;
    uint16 part_num;
    uint8 revision;
    int32 ret;
    
    if(NULL == pphy_info)
    {
        return NULL;
    }
    phy_sfp_get_chip_type(pphy_info->port_num, &part_num, &revision);
    ret = phy_sfp_need_manage_status(pphy_info->port_num, part_num);    
    /*If copper sfp needn't manage by i2c, needn't creat phy handle, return null*/
    if(ret == 0)
    {
        DRV_LOG_DEBUG(DRV_PHY, "phy sfp addr %d is serdes interface",pphy_info->port_num) 
        sal_mutex_destroy(pphy_info->pmutex);
        return NULL;
    } 
    DRV_LOG_DEBUG(DRV_PHY, "phy sfp addr %d is sgmii interface",pphy_info->port_num) 
    phdl = (phy_handle_t* )DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(phy_handle_t));
    if(NULL == phdl)
    {
        DRV_LOG_ERR("malloc failed");
        sal_mutex_destroy(pphy_info->pmutex);
        return NULL;
    }
    phy_sfp_init_chip(pphy_info, part_num);
    /*return to default use page.*/
    if(part_num == MARVELL_88E1111_PART)
    {
        sfp_select_phy_page(pphy_info->port_num, 1);
    }
    else
    {
        sfp_select_phy_page(pphy_info->port_num, 0);
    }
    sal_memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));
    phdl->phy_info.part_num = part_num;
    phdl->phy_info.phy_revision = revision;
    phdl->config_enable = phy_sfp_enable;
    phdl->config_speed = phy_sfp_speed;
    phdl->config_duplex = phy_sfp_duplex;
    phdl->get_link_poll = phy_sfp_get_link_poll;
    return phdl;
}



