/**
 @file ctc_usw_chip.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-10-19

 @version v2.0

 The file contains all chip APIs of ctc layer
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_usw_chip.h"
#include "sys_usw_chip.h"
#include "sys_usw_peri.h"
#include "sys_usw_datapath.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

/**
 @brief Initialize the chip module and set the local chip number of the linecard
*/
int32
ctc_usw_chip_init(uint8 lchip, uint8 lchip_num)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = lchip_num;

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_chip_init(lchip, lchip_num));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to deinit chip
*/
int32
ctc_usw_chip_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_chip_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to init datapath
*/
int32
ctc_usw_datapath_init(uint8 lchip_id, void* p_global_cfg)
{
    ctc_datapath_global_cfg_t global_cfg;
    uint8 index = 0;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    uint8 lchip = lchip_id;

    LCHIP_CHECK(lchip);
    if (p_global_cfg == NULL)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_datapath_global_cfg_t));

        for (index = 0; index < 64; index++)
        {
            global_cfg.serdes[index].mode = CTC_CHIP_SERDES_XFI_MODE;
        }

        global_cfg.core_frequency_a = 600;
        global_cfg.core_frequency_b = 500;
        /*SYSTEM MODIFY, Added by weij for enable macsec default, 2017-09-11*/
        global_cfg.dot1ae_enable = TRUE;
        p_global_cfg = &global_cfg;
        lchip_id = 0;
    }


    lchip_start = lchip;
    lchip_end = lchip + 1;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 0)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_init(lchip, ((ctc_datapath_global_cfg_t*)p_global_cfg)+lchip_id));
        CTC_ERROR_RETURN(sys_usw_peri_init(lchip, 1));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to deinit datapath
*/
int32
ctc_usw_datapath_deinit(uint8 lchip)
{
    CTC_ERROR_RETURN(sys_usw_peri_deinit(lchip));
    CTC_ERROR_RETURN(sys_usw_datapath_deinit(lchip));

    return CTC_E_NONE;
}

/**
 @brief The function is to get the local chip number of the linecard
*/
int32
ctc_usw_get_local_chip_num(uint8 lchip, uint8* lchip_num)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(lchip_num);
    *lchip_num = sys_usw_get_local_chip_num();

    return CTC_E_NONE;
}

/**
 @brief The function is to set chip's global chip id
*/
int32
ctc_usw_set_gchip_id(uint8 lchip, uint8 gchip_id)
{

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_set_gchip_id(lchip, gchip_id));

    return CTC_E_NONE;
}

/**
 @brief The function is to get chip's global chip id
*/
int32
ctc_usw_get_gchip_id(uint8 lchip, uint8* gchip_id)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, gchip_id));

    return CTC_E_NONE;
}

/**
 @brief The function is to set chip's global cnfig
*/
int32
ctc_usw_set_chip_global_cfg(uint8 lchip, ctc_chip_global_cfg_t* chip_cfg)
{
    CTC_PTR_VALID_CHECK(chip_cfg);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_SET_API_LCHIP(lchip, chip_cfg->lchip);

    CTC_ERROR_RETURN(sys_usw_set_chip_global_cfg(lchip, chip_cfg));

    return CTC_E_NONE;
}

/**
 @brief The function is to get chip's frequency
*/
int32
ctc_usw_get_chip_clock(uint8 lchip, uint16* freq)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(freq);

    CTC_ERROR_RETURN(sys_usw_get_chip_clock(lchip, freq));

    return CTC_E_NONE;
}

/**
 @brief Set port to mdio and port to phy mapping
*/
int32
ctc_usw_chip_set_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_set_phy_mapping(lchip, phy_mapping_para));

    return CTC_E_NONE;
}

/**
 @brief Get port to mdio and port to phy mapping
*/
int32
ctc_usw_chip_get_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_get_phy_mapping(lchip, phy_mapping_para));

    return CTC_E_NONE;
}
/**
 @brief control gephy scan
*/
int32
ctc_usw_chip_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_scan_para);


    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_phy_scan_para(lchip, p_scan_para));
    }

    return CTC_E_NONE;
}

/**
 @brief read phy scan enablle
*/
int32
ctc_usw_chip_set_phy_scan_en(uint8 lchip, bool enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_phy_scan_en(lchip, enable));

    return CTC_E_NONE;
}

/**
 @brief read by i2c master interface
*/
int32
ctc_usw_chip_i2c_read(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    CTC_PTR_VALID_CHECK(p_i2c_para);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_SET_API_LCHIP(lchip, p_i2c_para->lchip);

    CTC_ERROR_RETURN(sys_usw_peri_i2c_read(lchip, p_i2c_para));

    return CTC_E_NONE;
}

/**
 @brief write sfp register by i2c master interface
*/
int32
ctc_usw_chip_i2c_write(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    CTC_PTR_VALID_CHECK(p_i2c_para);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_SET_API_LCHIP(lchip, p_i2c_para->lchip);

    CTC_ERROR_RETURN(sys_usw_peri_i2c_write(lchip, p_i2c_para));

    return CTC_E_NONE;
}

/**
 @brief write sfp register by i2c master interface
*/
int32
ctc_usw_chip_read_i2c_buf(uint8 lchip, ctc_chip_i2c_scan_read_t* p_i2c_scan_read)
{
    CTC_PTR_VALID_CHECK(p_i2c_scan_read);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(p_i2c_scan_read->gchip, lchip);

    CTC_ERROR_RETURN(sys_usw_peri_read_i2c_buf(lchip, p_i2c_scan_read));

    return CTC_E_NONE;
}

/**
 @brief set phy auto scan enable
*/
int32
ctc_usw_chip_set_i2c_scan_en(uint8 lchip, bool enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_i2c_scan_en(lchip, enable));

    return CTC_E_NONE;
}

/**
 @brief control i2c scan
*/
int32
ctc_usw_chip_set_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_i2c_para);
    CTC_SET_API_LCHIP(lchip, p_i2c_para->lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_i2c_scan_para(lchip, p_i2c_para));
    }

    return CTC_E_NONE;
}

/**
 @brief set mac led mode interface
*/
int32
ctc_usw_chip_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type)
{
    CTC_PTR_VALID_CHECK(p_led_para);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_SET_API_LCHIP(lchip, p_led_para->lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_mode(lchip, p_led_para, led_type, 0));

    return CTC_E_NONE;
}

/**
 @brief set mac led mapping
*/
int32
ctc_usw_chip_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map)
{
    CTC_PTR_VALID_CHECK(p_led_map);

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_SET_API_LCHIP(lchip, p_led_map->lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_mapping(lchip, p_led_map));

    return CTC_E_NONE;
}

/**
 @brief set mac led enable
*/
int32
ctc_usw_chip_set_mac_led_en(uint8 lchip, bool enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_en(lchip, enable));

    return CTC_E_NONE;
}

/**
 @brief set gpio interface
*/
int32
ctc_usw_chip_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_gpio_mode(lchip, gpio_id, mode));

    return CTC_E_NONE;
}

/**
 @brief set gpio output value
*/
int32
ctc_usw_chip_set_gpio_output(uint8 lchip, uint8 gpio, uint8 out_para)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_set_gpio_output(lchip, gpio, out_para));

    return CTC_E_NONE;
}

/**
 @brief set gpio output value
*/
int32
ctc_usw_chip_get_gpio_input(uint8 lchip, uint8 gpio, uint8* in_value)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    CTC_PTR_VALID_CHECK(in_value);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_peri_get_gpio_input(lchip, gpio, in_value));

    return CTC_E_NONE;
}

/**
 @brief switch chip access type
*/
int32
ctc_usw_chip_set_access_type(uint8 lchip, ctc_chip_access_type_t access_type)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_chip_set_access_type(lchip, access_type));
    }
    return CTC_E_NONE;
}


#if 0
/**
 @brief get chip access type
*/
int32
ctc_usw_chip_get_access_type(uint8 lchip, ctc_chip_access_type_t* p_access_type)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    CTC_PTR_VALID_CHECK(p_access_type);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_chip_get_access_type(lchip, p_access_type));
    }

    return CTC_E_NONE;
}
#endif

/**
 @brief set chip dynamic switch mode
*/
int32
ctc_usw_chip_set_serdes_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_set_serdes_mode(lchip, p_serdes_info));

    return CTC_E_NONE;
}

/**
 @brief Mdio read interface
*/
int32
ctc_usw_chip_mdio_read(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_mdio_read(lchip, type, p_para));

    return CTC_E_NONE;
}

/**
 @brief Mdio read interface
*/
int32
ctc_usw_chip_mdio_write(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_mdio_write(lchip, type, p_para));

    return CTC_E_NONE;
}

/**
 @brief get mdio clock frequency
*/
int32
ctc_usw_chip_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_peri_set_mdio_clock(lchip, type, freq));
    }

    return CTC_E_NONE;
}

/**
 @brief get mdio clock frequency
*/
int32
ctc_usw_chip_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* p_freq)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;
    LCHIP_CHECK(lchip);
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    CTC_PTR_VALID_CHECK(p_freq);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_peri_get_mdio_clock(lchip, type, p_freq));
    }

    return CTC_E_NONE;
}
/**
 @brief set chip property
*/
int32
ctc_usw_chip_set_property(uint8 lchip,  ctc_chip_property_t chip_prop,  void* p_value)
{
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_set_property(lchip, chip_prop, p_value));

    return CTC_E_NONE;
}

/**
 @brief get chip property
*/
int32
ctc_usw_chip_get_property(uint8 lchip,  ctc_chip_property_t chip_prop,  void* p_value)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_peri_get_property(lchip, chip_prop, p_value));

    return CTC_E_NONE;
}

/**
 @brief get chip sensor
*/
int32
ctc_usw_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_CHIP);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_get_chip_sensor(lchip, type, p_value));

    return CTC_E_NONE;
}

