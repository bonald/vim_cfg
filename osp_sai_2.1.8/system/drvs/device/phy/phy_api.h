#ifndef __PHY_API_H__
#define __PHY_API_H__

#include "sal_common.h"
#include "phy_drv.h"

#define PHY_LED_ON 1
#define PHY_LED_OFF 0
/* support eee function, modified by liuht for bug 28298, 2014-04-21 */
typedef enum eee_enable_e
{
    EEE_ENABLE = 1,
    EEE_DISABLE,
    EEE_MAX,
} eee_enable_t;
/* end of liuht modified */

typedef struct
{
    uint8 access_fail;
    uint8 port_id;
    uint8 down;
}phy_diag_result_t;

/**********************************************************************
 *  Name:       : phy_cfg_enable
 *  Purpose     : enable or disable phy, actually power down or up phy;
 *  Input       : port id; zero as disable; None-zero as enable;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ***********************************************************************/
int32 phy_cfg_enable(uint32 port_id, int8 enable);

/*********************************************************
 *  Name:       : phy_cfg_flowctrl
 *  Purpose     : config phy flow cntrol;
 *  Input       : port id; direction, ability;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ********************************************************/
int32 phy_cfg_flowctrl(uint32 port_id, uint16 direction, uint16 enable);


/*********************************************************
 *  Name:       : phy_cfg_speed
 *  Purpose     : config phy speed;
 *  Input       : port id; speed;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ********************************************************/
int32 phy_cfg_speed(uint32 port_id, glb_port_speed_t speed);

/*added by liuyang 2016-5-5*/
/*********************************************************
 *  Name:       : phy_cfg_fec_enable
 *  Purpose     : config phy fec;
 *  Input       : port id; fec;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ********************************************************/
int32 phy_cfg_fec_enable(uint32 port_id, uint8 fec_en);
/* Modified by liuht for bug 28155, 2014-04-17 */
/* Support force master or slave mode */
/*********************************************************
 *  Name:       : phy_cfg_master_slave_mode
 *  Purpose     : config phy master-slave mode;
 *  Input       : port id; master-slave mode;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ********************************************************/
int32 phy_cfg_master_slave_mode(uint32 port_id, glb_port_master_slave_t master_slave);
/* End of liuht modified */

/**********************************************************
 *  Name:       : phy_cfg_duplex
 *  Purpose     : config phy duplex;
 *  Input       : port id; duplex;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 phy_cfg_duplex(uint32 port_id, glb_port_duplex_t duplex);

/**********************************************************
 *  Name:       : phy_cfg_unidir
 *  Purpose     : config phy unidir;
 *  Input       : port id; unidir;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 phy_cfg_unidir(uint32 port_id, glb_port_unidir_t enable);

/******************************************************
 *  Name:       : phy_cfg_loopback
 *  Purpose     : config phy loopback mode;
 *  Input       : port id; loopback mode;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 ******************************************************/
int32 phy_cfg_loopback(uint32 port_id, glb_lb_phy_t lb_mode);

/*********************************************************
 *  Name:       : phy_cfg_medium
 *  Purpose     : config phy media type;
 *  Input       : port id; media type;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 *********************************************************/
int32 phy_cfg_medium(uint32 port_id, glb_media_interface_t media_type);
/*********************************************************
 *  Name:       : phy_cfg_flowctrl
 *  Purpose     : config phy flow control ability;
 *  Input       : port id; flow control direction, flow control enable;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 *********************************************************/
int32 phy_cfg_flowctrl(uint32 port_id, uint16 direction, uint16 ability);

/*********************************************************
 *  Name:       : phy_cfg_syncE
 *  Purpose     : config phy syncE function;
 *  Input       : port id; enable flag: zero as disable; None-zero as enable;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 *********************************************************/
int32 phy_cfg_syncE(uint32 port_id, int8 enable);

/*********************************************************
 *  Name:       : phy_cfg_clock
 *  Purpose     : config phy clock function;
 *  Input       : port id; phy_clk_conf;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 *********************************************************/
int32 phy_cfg_clock(uint32 port_id, glb_vtss_phy_clock_conf_t phy_clk_conf);

int32 phy_cfg_bypass(uint32 port_id, int8 enable);

/**********************************************************
 *  Name:       : xgphy_reg_read
 *  Purpose     : read 10G phy register
 *  Input       : port id,dev no,register offset,val buffer
 *  Output      : register value
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 xgphy_reg_read(uint32 port_id, uint8 dev, uint16 offset, uint16* val);


/**********************************************************
 *  Name:       : xgphy_reg_write
 *  Purpose     : write 10G phy register
 *  Input       : port id,dev no,register offset,value
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 xgphy_reg_write(uint32 port_id, uint8 dev, uint16 offset, uint16 val);


/**********************************************************
 *  Name:       : gephy_reg_read
 *  Purpose     : read 1G phy register
 *  Input       : port id,register offset,val buffer
 *  Output      : register value
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 gephy_reg_read(uint32 port_id, uint8 offset, uint16* val);


/**********************************************************
 *  Name:       : gephy_reg_write
 *  Purpose     : write 1G phy register
 *  Input       : port id,register offset,value
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 gephy_reg_write(uint32 port_id, uint8 offset, uint16 val);

/**********************************************************
 *  Name:       : gephy_page_reg_read
 *  Purpose     : read 1G phy register from non standard page
 *  Input       : port id,phy page,register offset,val buffer
 *  Output      : register value
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 gephy_page_reg_read(uint32 port_id, uint8 phy_page, uint8 offset, uint16* val);


/**********************************************************
 *  Name:       : gephy_page_reg_write
 *  Purpose     : write 1G phy register to non standard page
 *  Input       : port id, phy page, register offset,value
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 gephy_page_reg_write(uint32 port_id, uint8 phy_page, uint8 offset, uint16 val);

/**********************************************************
 *  Name:       : phy_init
 *  Purpose     : Init phy;
 *  Input       : phy handle array,port number;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : N/A
 **********************************************************/
int32 phy_init(phy_handle_t** pphdl, uint32 max_port_nums);

int32 phy_get_link_poll(uint32 port_id, glb_phy_state_t* phy_val, phy_state_change_t* phy_change);
int32 phy_get_link_interrupt(uint8 port_id, glb_phy_state_t* phy_val, phy_state_change_t* phy_change);
int32 phy_check_no_phy(uint32 port_id);

/**********************************************************
 *  Name:       : phy_attach_copper_sfp_phy
 *  Purpose     : Add new copper SFP phy handle to original phy;
 *  Input       : port number;
 *  Output      : N/A
 *  Return      : RESULT_ERROR or RESULT_OK
 *  Note        : If the copper SFP phy needn't phy handle, attach NULL.
 **********************************************************/
int32 phy_attach_copper_sfp_phy(uint32 port_id);
int32 phy_deattach_copper_sfp_phy(uint32 port_id);
int32 phy_check_attach_phy(uint32 port_id);

/**********************************************************
 *  Name:       : get_phy_hdl
 *  Purpose     : get phy handle;
 *  Input       : port id;
 *  Output      : N/A
 *  Return      : phy_handle_t
 *  Note        : N/A
 **********************************************************/
phy_handle_t* get_phy_hdl(uint32 port_id);

int32 phy_set_vct_mod(uint32 port_id);
int32 phy_get_cable_info(uint32 port_id, glb_port_cable_info_t* cable_info);
int32 phy_re_init(uint32 port_id);
/* support eee function, modified by liuht for bug 28298, 2014-04-21 */
int32 phy_cfg_eee_capability(uint32, uint32);
int32 phy_config_phy_sig_test_mode(uint32 port_id, uint8 serdes_id, uint8 mode);
int32 phy_update_clause73_ability(uint32 port_id, uint32 ability);
int32 phy_get_clause73_ability(uint32 port_id, uint32* ability);
int32 phy_update_fec_ability(uint32 port_id, uint32 ability);
int32 phy_update_fiber_ability(uint32 port_id, uint8 fiber_ability, uint8 is_DAC);
int32 phy_get_fiber_ability(uint32 port_id, uint8* fiber_ability, uint8* is_DAC);
int32 phy_update_fec_en(uint32 port_id, uint8 fec_en);
int32 phy_get_fec_en(uint32 port_id, uint8* fec_en);
int32 phy_do_cl73_auto(uint32 port_id);
int32 phy_load_firmware(uint32 port_id, const char *pathname);
int32 phy_check_firmware_version(uint32 port_id, const char *pathname);
int32 phy_diagnostic_test_and_report(void);
/* end of liuht modified */
int32 phy_config_link_tolerable_time(uint32 port_id, uint32 time);
int32 phy_get_link_tolerable_time(uint32 port_id);
int32 phy_cfg_prbs(uint32 port_id, uint8 mode,uint8 polynome_type,uint8 *result);
int32 phy_cfg_led_mode(uint32 port_id,uint8 first_led_mode,uint8 second_led_mode);
#if (defined DUET2) || (defined TSINGMA)
int32 phy_get_autoneg_ability(uint32 port_id, uint32* ability);
#endif
#endif /* !__PHY_API_H__ */

