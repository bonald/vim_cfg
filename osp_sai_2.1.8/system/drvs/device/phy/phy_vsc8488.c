/****************************************************************************
* $Id$
* phy vsc8488 device driver
* 
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : jqiu
* Date          : 2011-08-04
* Reason        : First Create.
****************************************************************************/
#include "sal_common.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "phy_vsc8488.h"
#include "epld_api.h"

#define VSC8488_GLB_DEV  30

#define VSC8488_GPIO0_cfg1  0x0100
#define VSC8488_GPIO1_cfg1  0x0102
#define VSC8488_GPIO1_cfg2  0x0103
#define VSC8488_GPIO2_cfg1  0x0104
#define VSC8488_GPIO3_cfg1  0x0106
#define VSC8488_GPIO4_cfg1  0x0108
#define VSC8488_GPIO5_cfg1  0x010a
#define VSC8488_GPIO6_cfg1  0x0124
#define VSC8488_GPIO7_cfg1  0x0126
#define VSC8488_GPIO8_cfg1  0x0128
#define VSC8488_GPIO8_cfg2  0x0129
#define VSC8488_GPIO9_cfg1  0x012a
#define VSC8488_GPIOA_cfg1  0x012c
#define VSC8488_GPIOB_cfg1  0x012e

int32 vsc8488_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode);
int32 vsc8488_enable(phy_handle_t* phy_handle, int8 enable);

int32 vsc8488_reg_read(phy_handle_t* phy_handle, phyreg_param_t* op_param, uint16* value)
{
    phy_handle_t *phdl;
    mdio_bus_para_t para;

    int32 ret;
    
    if(NULL == phy_handle || NULL == op_param || NULL == value)
    {
        return RESULT_ERROR;
    }
    phdl = phy_handle;

    para.xgphy_para.devno = op_param->dev_no;
    para.xgphy_para.reg = op_param->dat.regaddr16;
    para.xgphy_para.phy_addr = phdl->phy_info.phy_addr;
        
    if(NULL == phdl->mdio_hdl || NULL == phdl->mdio_hdl->read)
    {
        return RESULT_ERROR;
    }
    ret = phdl->mdio_hdl->read(phdl->mdio_hdl, &para);
    if(ret)
    {
        return RESULT_ERROR;
    }
    *value = para.xgphy_para.val;
    return RESULT_OK;
}

static int32 vsc8488_reg_write(phy_handle_t* phy_handle, phyreg_param_t* op_param, uint16 value)
{
    phy_handle_t *phdl;
    mdio_bus_para_t para;
    int32 ret;
    
    if(NULL == phy_handle || NULL == op_param)
    {
        return RESULT_ERROR;
    }
    phdl = phy_handle;
    para.xgphy_para.devno = op_param->dev_no;
    para.xgphy_para.phy_addr = phdl->phy_info.phy_addr;
    para.xgphy_para.reg = op_param->dat.regaddr16;
    para.xgphy_para.val = value;

    if(NULL == phdl->mdio_hdl || NULL == phdl->mdio_hdl->write)
    {
        return RESULT_ERROR;
    }
    ret = phdl->mdio_hdl->write(phdl->mdio_hdl, &para);
    if(ret)
    {
        return RESULT_ERROR;
    }
    
    return RESULT_OK;
}
static void _vsc8488_clean_alarm_stats(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;    
    uint16 value;
    
    /*Clear interrupts*/
    phyreg_para.dev_no = XGPHY_WIS_DEV;
    phyreg_para.dat.regaddr16 = 0xee04;
    phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    return ;
}

/* For 1G mode, 10G phy's interrupt is useless, 1G phy can support interrupt
   For 10G mode, use 10G phy interrupt */
static void _vsc8488_en_up2dn_alarm(phy_handle_t* phy_handle, glb_media_interface_t media_type)
{
    phyreg_param_t phyreg_para;
    /*Enable LS_ALARM interrupt*/
    phyreg_para.dev_no = XGPHY_WIS_DEV;
    phyreg_para.dat.regaddr16 = 0xee06; 
    if(media_type != GLB_MEDIA_1000BX)
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x0800);/*LOPC INT to WIS_INTB*/
    }
    else
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0);
    }

    return ;
}

static void _vsc8488_en_dn2up_alarm(phy_handle_t* phy_handle, glb_media_interface_t media_type)
{
    phyreg_param_t phyreg_para;
    /*Enable LS_ALARM interrupt*/
    phyreg_para.dev_no = XGPHY_WIS_DEV;
    phyreg_para.dat.regaddr16 = 0xee06; 
    if(media_type != GLB_MEDIA_1000BX)
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);/*RXLOL INT to WIS_INTB*/
    }
    else
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0);
    }

    return ;
}

/*Disable interrupt, used when set phy loopback.*/
static void _vsc8488_dis_alarm(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;
    /*Disable LS_ALARM interrupt*/
    phyreg_para.dev_no = XGPHY_WIS_DEV;
    phyreg_para.dat.regaddr16 = 0xee06; 
    phy_handle->reg_write(phy_handle, &phyreg_para, 0);

    return ;
}

static void _xgphy_vsc8488_status_get(phy_handle_t* phy_handle, glb_phy_state_t* phy_state_val)
{
    phyreg_param_t phyreg_para;    
    uint16 pcs_status;
    uint16 lopc_status;
#if 1    
    /*Get SFP+ LOS signal*/
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = 0xa200;        
    phy_handle->reg_read(phy_handle, &phyreg_para, &lopc_status);    
    /*when near loopback, link only decide by pcs_status.*/
    if(((lopc_status&0x2)!=0)&&(phy_handle->phy_info.phy_manage_info.lb_mode!=GLB_LB_PHY_NEAR))
    {
        phy_state_val->link_up = 0;
    }
    else
    {
        /*Get PCS status*/
        phyreg_para.dev_no = XGPHY_PCS_DEV;
        phyreg_para.dat.regaddr16 = 0x20;        
        phy_handle->reg_read(phy_handle, &phyreg_para, &pcs_status);    
        if((pcs_status&0x1000)!=0)
        {
            phy_state_val->link_up = 1;
        }
        else
        {
            phy_state_val->link_up = 0;
        }
    }
#else 
    /*check link down to up */
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = 0x8;
    phy_handle->reg_read(phy_handle, &phyreg_para, &pma_status);
    if((pma_status&0x0400) != 0)
    {
        phy_handle->reg_read(phy_handle, &phyreg_para, &pma_status);
    }
    /*check link up to down*/
    phyreg_para.dev_no = XGPHY_PCS_DEV;
    phyreg_para.dat.regaddr16 = 0x8;
    phy_handle->reg_read(phy_handle, &phyreg_para, &pcs_status);
    if((pcs_status&0x0400) != 0)
    {
        phy_handle->reg_read(phy_handle, &phyreg_para, &pcs_status);
    }
    /*Both PMA and PCS have no recv fault, then link up*/
    if(((pma_status&0x0400) == 0)&&((pcs_status&0x0400) == 0))
        phy_state_val->link_up = 1;
    else
        phy_state_val->link_up = 0;/*link down */
//    DRV_LOG_ERR("vsc8488 get %d status pma %x pcs %x", phy_handle->phy_info.phy_addr, pma_status, pcs_status);
#endif
    phy_state_val->speed = GLB_SPEED_10G;
    phy_state_val->duplex = GLB_DUPLEX_FULL;
    phy_state_val->linkup_media = GLB_PORT_TYPE_FIBER;
    phy_state_val->flowctrl.send = phy_handle->phy_info.phy_manage_info.flowctrl.send;
    phy_state_val->flowctrl.recv = phy_handle->phy_info.phy_manage_info.flowctrl.recv;

    return;
}


#if 1
static int _xgphy_vsc8488_load_firmware(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;
    int ret=0;
    FILE* fp;
    uint8 tmpbuf[16];
    uint16 data;
    int count = 0;
    int ii, jj=0;

    DRV_LOG_DEBUG(DRV_PHY, "Vsc8488 init, start");
    fp = sal_fopen("/etc/vsc8488_firmware_mdio.bin", "rb");
    if(fp < 0) 
    {
        DRV_LOG_ERR("Open /etc/vsc8488_firmware_mdio.bin failed.");
        return RESULT_ERROR;
    }

    phyreg_para.dev_no = 0x1f;
    phyreg_para.dat.regaddr16 = 0x0000;
    while((count = sal_fread(tmpbuf, 1, 16, fp))>0)
    {
        for(ii=0; ii<count/4; ii++)
        {
            data = (tmpbuf[ii*4+3]<<8) | tmpbuf[ii*4+2];
            phyreg_para.dat.regaddr16 = jj++;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, data);
            data = (tmpbuf[ii*4+1]<<8) | tmpbuf[ii*4];
            phyreg_para.dat.regaddr16 = jj++;
            ret += phy_handle->reg_write(phy_handle, &phyreg_para, data);
        }        
    }    
    while(jj < 0x4000)
    {
        phyreg_para.dat.regaddr16 = jj++;
        ret += phy_handle->reg_write(phy_handle, &phyreg_para, 0);
    }
    sal_fclose(fp);
    return RESULT_OK;
}

#endif

static void _vsc8488_config_XG_mode(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;
    uint16 value=1, count;
    
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = 0x8017;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0004);/* PMA TX Rate control. CMU Varclk sel bit 16 */
    phyreg_para.dat.regaddr16 = 0x8018;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0001);
    phyreg_para.dat.regaddr16 = 0x8019;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0008);
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = 0x7fe3;
    count = 0;
    while((value!=0)&&(count < 10))
    {
        phy_handle->reg_read(phy_handle, &phyreg_para, &value);
        count++;
    }
    /*request firmware to run 1G */
    phyreg_para.dat.regaddr16 = 0x7fe4;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x2);
    
    phyreg_para.dat.regaddr16 = 0x7fe3;
    if(phy_handle->phy_info.phy_addr%2)
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x8094);
    }
    else
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x8194);
    }    
}

static void _vsc8488_config_1G_mode(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;
    uint16 value=1, count;
    
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = 0x8017;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x001a);/* PMA TX Rate control. CMU Varclk sel bit 16 */        
    phyreg_para.dat.regaddr16 = 0x8018;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x00c0);
    phyreg_para.dat.regaddr16 = 0x8019;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0004);
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = 0x7fe3;
    count = 0;
    while((value!=0)&&(count < 10))
    {
        phy_handle->reg_read(phy_handle, &phyreg_para, &value);
        count++;
    }
    /*request firmware to run 1G */
    phyreg_para.dat.regaddr16 = 0x7fe4;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8);
    
    phyreg_para.dat.regaddr16 = 0x7fe3;
    if(phy_handle->phy_info.phy_addr%2)
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x8094);
    }
    else
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x8194);
    }    
}

/*media_type: cfg work mode of vsc8488, include SFP/SFP+ 1GMode, SFP/SFP+ 10GMode, and so on*/
int32 vsc8488_phy_reset(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
        
    if(media_if == GLB_MEDIA_1000BX)
    {
        _vsc8488_config_1G_mode(phy_handle);
#ifdef CNOS_OS
        epld_cfg_phy_switch_smi(0, phy_handle->phy_info.switch_chnl_id, 0);
#endif
    }
    else
    {
        _vsc8488_config_XG_mode(phy_handle);
#ifdef CNOS_OS
        epld_cfg_phy_switch_smi(0, phy_handle->phy_info.switch_chnl_id, 1);
#endif
    }
    return RESULT_OK;
}

int32 vsc8488_config_medium(phy_handle_t* phy_handle, uint8 mac_if, uint8 media_if)
{
    int32 ret = RESULT_OK;
    phy_info_t *pphy_info;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY,
        "VSC8488 phy addr %d config medium mac_if %d media_if %d", pphy_info->phy_addr, mac_if, media_if);
    ret = vsc8488_phy_reset(phy_handle, mac_if, media_if);
    /*After medium config, speed/duplex/loopback need re-config. */
    if(GLB_LB_NONE != pphy_info->phy_manage_info.lb_mode)
    {
        ret |= vsc8488_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
    }
    /*Fix bug30023. After medium config, enable need re-config*/
    if(0 == pphy_info->phy_manage_info.enable)
    {
        ret |= vsc8488_enable(phy_handle, 0);
    }
    return ret;
}

int32 vsc8488_config_flowctrl(phy_handle_t* phy_handle, uint8 symmetric, uint8 asymmetric)
{
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

int32 vsc8488_init(phy_handle_t* phy_handle)
{
    int ret=RESULT_OK;
    uint16 value=0, cnt=0;

    phyreg_param_t phyreg_para;
    
    if(NULL == phy_handle || NULL == phy_handle->reg_write)
    {
        return RESULT_ERROR;
    }
    
    if(phy_handle->phy_info.phy_addr % 2)
    {
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = 0x2; 
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x80);
        _xgphy_vsc8488_load_firmware(phy_handle);
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = 0x2; 
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x0);
    }

    /*soft-reset to init phy*/
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = XGPHY_REG_CONTROL_REG; 
    phy_handle->reg_write(phy_handle, &phyreg_para, 0xa040);
    /* wait until reset finished. timeout is 0.5s */
    while((value & 0x8000) != 0 && (cnt < 50))
    {
        sal_udelay(10);
        cnt ++;
        phy_handle->reg_read(phy_handle, &phyreg_para, &value);
    }
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x2840);    

    phyreg_para.dev_no = XGPHY_WIS_DEV;
    phyreg_para.dat.regaddr16 = 0x0007;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0000); /* Disable WAN MODE*/
    /*cfg Global Refclk.*/    
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = 0x7f10;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0188); /* Global XREFCLK from CMU clkgen     */
    phyreg_para.dat.regaddr16 = 0x7f11;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0002);/* Global Refclk CMU 14Mhz BW control */            
    phyreg_para.dat.regaddr16 = 0x3; /*Failover to normal operation*/
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x0020);
    
    /*cfg GPIO0 SFPP0_MODABS*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO0_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8400); 
    /*cfg GPIO9 SFPP1_MODABS*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO9_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8400);

    /*cfg GPIO1 SFPP0_TXDIS*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO1_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);
    /*cfg GPIO8 SFPP1_TXDIS*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO8_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);        

    /*cfg GPIO2 SLOT_XG_PHY_INT1_N*/
    /*Because xgphy two channel use same INT to CPU, so here AND two channel INT to WIS_INTB*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO2_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x82);

    /*cfg GPIO6 SLOT_XG_PHY_INT1_N*/
    /*Because xgphy two channel use same INT to CPU, so here AND two channel INT to WIS_INTB
      and just need one INT, this gpio is useless for INT. */
//    phyreg_para.dev_no = VSC8488_GLB_DEV;
//    phyreg_para.dat.regaddr16 = VSC8488_GPIO6_cfg1;
//    phy_handle->reg_write(phy_handle, &phyreg_para, 0x82);
    
    /*cfg GPIO3 SFPP0_LED*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO3_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);
    /*cfg GPIO7 SFPP1_LED*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO7_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);
    /*cfg RX_LED_MODE */
    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = 0xa100;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x1f0);

    /*cfg GPIO4 SFPP0_SDA*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO4_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8404);
    /*cfg GPIO5 SFPP0_SCL*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIO5_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8404);    
    /*cfg GPIO10 SFPP1_SDA*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIOA_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8404);
    /*cfg GPIO11 SFPP1_SCL*/
    phyreg_para.dev_no = VSC8488_GLB_DEV;
    phyreg_para.dat.regaddr16 = VSC8488_GPIOB_cfg1;
    phy_handle->reg_write(phy_handle, &phyreg_para, 0x8404);

    ret = vsc8488_phy_reset(phy_handle, phy_handle->phy_info.phy_manage_info.mac_if, phy_handle->phy_info.phy_manage_info.media_type);
    return ret;
}

int32 vsc8488_enable(phy_handle_t* phy_handle, int8 enable)
{
    phyreg_param_t phyreg_para;    
    phy_info_t* pphy_info;
    
    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }

    pphy_info = &phy_handle->phy_info;
    DRV_LOG_DEBUG(DRV_PHY, "VSC8488 phy addr %d enable: speed %d duplex %d", 
        pphy_info->phy_addr, pphy_info->phy_manage_info.speed, 
        pphy_info->phy_manage_info.duplex);

    phyreg_para.dev_no = XGPHY_PMA_DEV;
    phyreg_para.dat.regaddr16 = XGPHY_REG_CONTROL_REG;       
    if(enable)
    {
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x2040);
        if(pphy_info->phy_manage_info.lb_mode != GLB_LB_NONE)
        {
            vsc8488_config_loopback(phy_handle, pphy_info->phy_manage_info.lb_mode);
        }

        /*Clear interrupts and enable interrupt*/
        _vsc8488_clean_alarm_stats(phy_handle);
        _vsc8488_en_up2dn_alarm(phy_handle, phy_handle->phy_info.phy_manage_info.media_type);
    }
    else
    {
        phyreg_para.dat.regaddr16 = XGPHY_REG_CONTROL_REG;
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x2840);
    }
    return RESULT_OK;
}

static int32 vsc8488_linkup_setting(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;

    /*when near loopback, will continue report interrupt for Signal los, so ignore it.*/
    if(phy_handle->phy_info.phy_manage_info.lb_mode==GLB_LB_PHY_NEAR)
    {
        _vsc8488_dis_alarm(phy_handle);
    }
    else
    {
        /*When linkup, enable up->down interrupt */
        _vsc8488_en_dn2up_alarm(phy_handle, phy_handle->phy_info.phy_manage_info.media_type);
    }
    if(phy_handle->phy_info.phy_addr % 2 == 0)
    {
        /*cfg GPIO3 SFPP0_LED*/
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = VSC8488_GPIO3_cfg1;
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x1);
    }
    else
    {
        /*cfg GPIO7 SFPP1_LED*/
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = VSC8488_GPIO7_cfg1;
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x9);
    }
    
    return RESULT_OK;
}

static int32 vsc8488_linkdown_setting(phy_handle_t* phy_handle)
{
    phyreg_param_t phyreg_para;

    /*when near loopback, will continue report interrupt for Signal los, so ignore it.*/
    if(phy_handle->phy_info.phy_manage_info.lb_mode==GLB_LB_PHY_NEAR)
    {
        _vsc8488_dis_alarm(phy_handle);
    }
    else
    {
        _vsc8488_en_up2dn_alarm(phy_handle, phy_handle->phy_info.phy_manage_info.media_type);
    }
    if(phy_handle->phy_info.phy_addr % 2 == 0)
    {
        /*cfg GPIO3 SFPP0_LED*/
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = VSC8488_GPIO3_cfg1;
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);
    }
    else
    {
        /*cfg GPIO7 SFPP1_LED*/
        phyreg_para.dev_no = VSC8488_GLB_DEV;
        phyreg_para.dat.regaddr16 = VSC8488_GPIO7_cfg1;
        phy_handle->reg_write(phy_handle, &phyreg_para, 0x1000);
    }
    return RESULT_OK;
}

int32 vsc8488_link_poll(phy_handle_t* phy_handle, glb_phy_state_t *phy_val, 
                            phy_state_change_t *phy_change)
{
    phy_info_t *phy_info;
    
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }
       
    phy_info = &phy_handle->phy_info;


    /*get pcs status*/
    _xgphy_vsc8488_status_get(phy_handle, phy_val);
     
    /*Link status in changed*/
    if(phy_info->phy_stat_flag.link_up != phy_val->link_up)
    {
        if(phy_val->link_up == GLB_LINK_UP)     /* link down to link up */
        {
            phy_change->action = 1;
            if(phy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
                phy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
            {
                phy_change->flowctrl_change = 1;
            }
            DRV_LOG_DEBUG(DRV_PHY, "VSC8488 phy addr %d link up",phy_info->phy_addr); 
            vsc8488_linkup_setting(phy_handle);
        }
        else
        {
            phy_change->action = 0;  /* link down to link up */
            DRV_LOG_DEBUG(DRV_PHY, "VSC8488 phy addr %d link down",phy_info->phy_addr); 
            vsc8488_linkdown_setting(phy_handle);
        }        
    }
    /*No change*/
    else
    {
        phy_change->action = -1;
        if(phy_val->link_up == GLB_LINK_UP)
        {
            if(phy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
                phy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
            {
                phy_change->flowctrl_change = 1;
            }
        }
    }   

    phy_info->phy_stat_flag.link_up = phy_val->link_up;
    phy_info->phy_stat_flag.speed = phy_val->speed;
    phy_info->phy_stat_flag.duplex = phy_val->duplex;
    phy_info->phy_stat_flag.linkup_media = phy_val->linkup_media;
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        phy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }

    return RESULT_OK;
}

int32 vsc8488_link_interrupt(phy_handle_t* phy_handle, glb_phy_state_t *phy_val, 
                                  phy_state_change_t *phy_change)
{
    phy_info_t *phy_info;
   
    if(NULL == phy_handle || NULL == phy_val || NULL == phy_change)
    {
        return RESULT_ERROR;
    }

    _vsc8488_clean_alarm_stats(phy_handle);
    
    phy_info = &phy_handle->phy_info;
    
    _xgphy_vsc8488_status_get(phy_handle, phy_val);
    
    DRV_LOG_DEBUG(DRV_PHY, "vsc8488_link_interrupt id %d link up %d", phy_info->phy_addr, phy_info->phy_stat_flag.link_up); 
    /*Link status in changed*/
    if(phy_info->phy_stat_flag.link_up != phy_val->link_up)
    {
        if(phy_val->link_up == GLB_LINK_UP)     /* link down to link up */
        {
            phy_change->action = 1;
            if(phy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
                phy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
            {
                phy_change->flowctrl_change = 1;
            }
            vsc8488_linkup_setting(phy_handle);
        }
        else
        {
            phy_change->action = 0;  /* link up to link down */
            vsc8488_linkdown_setting(phy_handle);
        }        
    }
    /*No change*/
    else
    {
        phy_change->action = -1;
        if(phy_val->link_up == GLB_LINK_UP)
        {
            if(phy_info->phy_stat_flag.flowctrl.send != phy_val->flowctrl.send ||
                phy_info->phy_stat_flag.flowctrl.recv != phy_val->flowctrl.recv)
            {
                phy_change->flowctrl_change = 1;
            }
        }
    }   

    phy_info->phy_stat_flag.link_up = phy_val->link_up;
    phy_info->phy_stat_flag.speed = phy_val->speed;
    phy_info->phy_stat_flag.duplex = phy_val->duplex;
    phy_info->phy_stat_flag.linkup_media = phy_val->linkup_media;
    /*When link is down, not update flow ctrl*/
    if(phy_val->link_up == GLB_LINK_UP)
    {
        phy_info->phy_stat_flag.flowctrl.send = phy_val->flowctrl.send;
        phy_info->phy_stat_flag.flowctrl.recv = phy_val->flowctrl.recv;
    }
    
    return RESULT_OK;
}

/* modified by liuht for bug 26641, 2014-0422 */
/* get current phy status */
int32 vsc8488_get_cur_status(phy_handle_t* phy_handle, glb_phy_state_t* phy_val)
{
    _xgphy_vsc8488_status_get(phy_handle, phy_val);
    return 0;
}
/* end of liuht modified */

int32 vsc8488_config_loopback(phy_handle_t* phy_handle, uint8 lb_mode)
{
    phyreg_param_t phyreg_para;    
    uint16 value;

    if(NULL == phy_handle)
    {
        return RESULT_ERROR;
    }
    switch (lb_mode)
    {
    case GLB_LB_NONE:
        /*clear near loopback*/
        phyreg_para.dev_no = 1;
        phyreg_para.dat.regaddr16 = 0x0;
        phy_handle->reg_read(phy_handle, &phyreg_para, &value); 
        value &= 0xfffe;
        phy_handle->reg_write(phy_handle, &phyreg_para, value); 
        /*clear far loopback*/
        phyreg_para.dat.regaddr16 = 0x8012;
        phy_handle->reg_read(phy_handle, &phyreg_para, &value); 
        value |= 0x10; 
        phy_handle->reg_write(phy_handle, &phyreg_para, value); 
        if(phy_handle->phy_info.phy_stat_flag.link_up)
        {
            _vsc8488_en_dn2up_alarm(phy_handle, phy_handle->phy_info.phy_manage_info.media_type);
        }
        else
        {
            _vsc8488_en_up2dn_alarm(phy_handle, phy_handle->phy_info.phy_manage_info.media_type);
        }
        break;
    case GLB_LB_PHY_FAR: /*Here use PMA far loopback*/
        phyreg_para.dev_no = 1;
        phyreg_para.dat.regaddr16 = 0x8012;
        phy_handle->reg_read(phy_handle, &phyreg_para, &value); 
        value &= 0xffef; 
        phy_handle->reg_write(phy_handle, &phyreg_para, value); 
        break;
    case GLB_LB_PHY_NEAR:/*Here use PMA near loopback*/
        phyreg_para.dev_no = 1;
        phyreg_para.dat.regaddr16 = 0x0;
        phy_handle->reg_read(phy_handle, &phyreg_para, &value); 
        value |= 1; 
        phy_handle->reg_write(phy_handle, &phyreg_para, value);
        _vsc8488_dis_alarm(phy_handle);
        break;
    default:
        return RESULT_ERROR;
    }    
    return RESULT_OK;
}

phy_handle_t* phy_vsc8488_dev_register(phy_info_t* pphy_info)
{
    phy_handle_t* phdl = NULL;
    mdio_handle_t* p_mdio_hdl = NULL;
    mdio_info_t mdio_info;

    sal_memset(&mdio_info, 0, sizeof(mdio_info));
    
    if(NULL == pphy_info)
    {
        return NULL;
    }
    
    phdl = (phy_handle_t* )DRV_CALLOC(CTCLIB_MEM_DRIVER_PHY_INFO, sizeof(phy_handle_t));
    if(NULL == phdl)
    {
        DRV_LOG_ERR("malloc failed");
        return NULL;
    }

    sal_memcpy(&phdl->phy_info, pphy_info, sizeof(phy_info_t));

    phdl->phy_init = vsc8488_init;
    phdl->config_enable = vsc8488_enable;
    phdl->config_loopback = vsc8488_config_loopback;
    phdl->config_medium = vsc8488_config_medium;
    phdl->config_flowctrl = vsc8488_config_flowctrl;
    phdl->get_link_poll = vsc8488_link_poll;    
    phdl->get_link_interupt = vsc8488_link_interrupt;
    phdl->get_cur_status = vsc8488_get_cur_status; /* modified by liuht for bug 26641, 2014-0422 */
    phdl->reg_read = vsc8488_reg_read;
    phdl->reg_write = vsc8488_reg_write;

    mdio_info.base_addr = pphy_info->base_addr;
    mdio_info.bus_type  = pphy_info->mdio_bus;
    p_mdio_hdl = mdio_create_handle(&mdio_info);
    
    if( NULL == p_mdio_hdl)
    {
        goto err_out;
    }
    
    phdl->mdio_hdl = p_mdio_hdl;
 
    return phdl;
    
err_out:
    if (NULL != phdl)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_PHY_INFO, phdl);
        phdl = NULL;
    }

    return NULL;
}

