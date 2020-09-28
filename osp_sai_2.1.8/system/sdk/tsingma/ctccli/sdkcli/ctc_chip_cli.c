/**
 @file ctc_chip_cli.c

 @date 2012-07-09

 @version v2.0

---file comments----
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_const.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"
#include "ctc_error.h"
#include "ctc_chip.h"
#include "dal.h"

#define PRINT_CAPABLITY(c)                                                   \
if (0xFFFFFFFF == capability[CTC_GLOBAL_CAPABILITY_##c])                     \
{                                                                            \
    ctc_cli_out("%-30s : -\n", #c);                                          \
}                                                                            \
else                                                                         \
{                                                                            \
    ctc_cli_out("%-30s : %d\n", #c, capability[CTC_GLOBAL_CAPABILITY_##c]);  \
}

CTC_CLI(ctc_cli_chip_show_chip_clock,
        ctc_cli_chip_show_chip_clock_cmd,
        "show chip clock frequence",
        "show",
        "chip",
        "clock",
        "frequence")
{
    int32 ret = CLI_SUCCESS;
    uint16 freq = 0;

    if(g_ctcs_api_en)
    {
        ret = ctcs_get_chip_clock(g_api_lchip, &freq);
    }
    else
    {
        ret = ctc_get_chip_clock(&freq);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("The chip clock freq is: %dM\n", freq);

    return ret;
}

CTC_CLI(ctc_cli_chip_set_phy_scan_para,
        ctc_cli_chip_set_phy_scan_para_cmd,
        "chip set phy-scan para {(gebitmap0 GE_BITMAP0 gebitmap1 GE_BITMAP1) | (xgbitmap0 XG_BITMAP0 xgbitmap1 XG_BITMAP1) |   \
        (phybitmap0 BITMAP0 phybitmap1 BITMAP1) mdio-ctlr-type VALUE mdio-ctlr-id ID| \
        (interval INTERVAL) | (xglinkmask LINK_MASK) | (twice (enable | disable)) | control-id ID | usephy0 USEPHY0 usephy1 USEPH1 }",
        "chip module",
        "set operate",
        "auto scan operate",
        "scan parameter",
        "ge phy bit map0",
        "bit map0 value",
        "ge phy bit map1",
        "bit map1 value",
        "xg phy bit map0",
        "bit map0 value",
        "xg phy bit map1",
        "bit map1 value",
        "phy bit map0",
        "bit map0 value",
        "phy bit map1",
        "bit map1 value",
        "MDIO controller type",
        "value <0 - 1>",
        "MDIO controller ID",
        "ID",
        "scan interval",
        "interval value",
        "xg phy device link mask",
        "mask value",
        "xgphy scan twince",
        "enable",
        "disable",
        "control-id",
        "ID",
        "use phy 0",
        "use phy 0 value",
        "use phy 1",
        "use phy 1 value")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_phy_scan_ctrl_t scan_para;
    uint8 index = 0;

    sal_memset(&scan_para, 0, sizeof(ctc_chip_phy_scan_ctrl_t));

    index = CTC_CLI_GET_ARGC_INDEX("gebitmap0");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("gebitmap0", scan_para.scan_gephy_bitmap0, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_GE_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gebitmap1");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("gebitmap1", scan_para.scan_gephy_bitmap1, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_GE_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xgbitmap0");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("xgbitmap0", scan_para.scan_xgphy_bitmap0, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        scan_para.op_flag |= CTC_CHIP_XG_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xgbitmap1");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("xgbitmap1", scan_para.scan_xgphy_bitmap1, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        scan_para.op_flag |= CTC_CHIP_XG_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("phybitmap0");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("phybitmap0", scan_para.scan_phy_bitmap0, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_PHY_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("phybitmap1");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("phybitmap1", scan_para.scan_phy_bitmap1, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_PHY_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("usephy0");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("usephy0", scan_para.mdio_use_phy0, argv[index + 1]);
        scan_para.op_flag |= CTC_CHIP_USE_PHY_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("usephy1");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("usephy1", scan_para.mdio_use_phy1, argv[index + 1]);
        scan_para.op_flag |= CTC_CHIP_USE_PHY_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mdio-ctlr-type");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("mdio-ctlr-type", scan_para.mdio_ctlr_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mdio-ctlr-id");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("mdio-ctlr-id", scan_para.mdio_ctlr_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("interval", scan_para.scan_interval, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_INTERVAL_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xglinkmask");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("xglinkmask", scan_para.xgphy_link_bitmask, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        scan_para.op_flag |= CTC_CHIP_XG_LINK_MASK_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("twice");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        if (CLI_CLI_STR_EQUAL("enable", index + 1))
        {
            scan_para.xgphy_scan_twice = 1;
        }
        else
        {
            scan_para.xgphy_scan_twice = 0;
        }

        scan_para.op_flag |= CTC_CHIP_XG_TWICE_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("control-id", scan_para.ctl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_phy_scan_para(g_api_lchip, &scan_para);
    }
    else
    {
        ret = ctc_chip_set_phy_scan_para(&scan_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_phy_scan_en,
        ctc_cli_chip_set_phy_scan_en_cmd,
        "chip set phy-scan (enable | disable)",
        "chip module",
        "set operate",
        "phy scan",
        "scan enable",
        "scan disable")
{
    int32 ret       = CLI_SUCCESS;
    bool enable;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_phy_scan_en(g_api_lchip, enable);
    }
    else
    {
        ret = ctc_chip_set_phy_scan_en(enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_read_i2c,
        ctc_cli_chip_read_i2c_cmd,
        "chip read i2c (control-id ID |) (switch-id ID |) \
        (slave-id ID dev-addr ADDR offset OFF length LEN) (access-switch|) (lchip LCHIP|)",
        "chip module",
        "read operate",
        "i2c device",
        "i2c master id",
        "<0 - 1>",
        "i2c switch id",
        "set according to hardware, 0 - disable",
        "i2c slave id",
        "<0 - 95>",
        "i2c slave address",
        "ADDR",
        "i2c slave offset",
        "OFFSET",
        "read value length",
        "LENGTH",
        "access i2c switch",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    ctc_chip_i2c_read_t i2c_para;
    uint8 index = 0;

    sal_memset(&i2c_para, 0, sizeof(ctc_chip_i2c_read_t));

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("control-id", i2c_para.ctl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("switch-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("switch-id", i2c_para.i2c_switch_id , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", i2c_para.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("slave-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("slave-id", i2c_para.slave_dev_id , argv[index + 1]);
        i2c_para.slave_bitmap = (0x1 << i2c_para.slave_dev_id);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dev-addr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("dev-addr", i2c_para.dev_addr , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("offset");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("offset", i2c_para.offset , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("length");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("length", i2c_para.length , argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("access-switch");
    if (0xFF != index)
    {
        i2c_para.access_switch = 1;
    }

    i2c_para.buf_length = i2c_para.length;

    i2c_para.p_buf = (uint8*)sal_malloc(i2c_para.buf_length);
    if (NULL == i2c_para.p_buf)
    {
        return CLI_ERROR;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_i2c_read(g_api_lchip, &i2c_para);
    }
    else
    {
       ret = ctc_chip_i2c_read(&i2c_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        sal_free(i2c_para.p_buf);
        i2c_para.p_buf = NULL;
        return CLI_ERROR;
    }

    ctc_cli_out("Dev %d Data: \n", i2c_para.slave_dev_id);

    for (index = 0; index < i2c_para.buf_length; index++)
    {
        if (((index % 16) == 0) && (index != 0))
        {
            ctc_cli_out("\n");
        }

         ctc_cli_out("  0x%02x", i2c_para.p_buf[index]);
    }

    ctc_cli_out("\n");

    sal_free(i2c_para.p_buf);
    i2c_para.p_buf = NULL;

    return ret;

}

CTC_CLI(ctc_cli_chip_write_i2c,
        ctc_cli_chip_write_i2c_cmd,
        "chip write i2c (control-id ID |) (switch-id ID|) (slave-id ID \
        dev-addr ADDR offset OFFSET value VALUE) (access-switch|)(lchip LCHIP|)",
        "chip module",
        "write operate",
        "i2c device",
        "i2c master id",
        "id",
        "i2c switch id",
        "wet according to hardware 0 - disable",
        "i2c slave id",
        "id",
        "slave dev address",
        "address",
        "offset",
        "offset",
        "write value",
        "value",
        "access i2c switch",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint8 index = 0;
    ctc_chip_i2c_write_t i2c_para;

    sal_memset(&i2c_para, 0, sizeof(ctc_chip_i2c_write_t));

    index = CTC_CLI_GET_ARGC_INDEX("value");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("value", i2c_para.data, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("offset");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("offset", i2c_para.offset, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dev-addr");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("dev-addr", i2c_para.dev_addr, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("slave-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("slave-id", i2c_para.slave_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("lchip", i2c_para.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("control-id", i2c_para.ctl_id, argv[index + 1]);
    }

     index = CTC_CLI_GET_ARGC_INDEX("switch-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("switch-id", i2c_para.i2c_switch_id , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("access-switch");
    if (0xFF != index)
    {
        i2c_para.access_switch = 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_i2c_write(g_api_lchip, &i2c_para);
    }
    else
    {
       ret = ctc_chip_i2c_write(&i2c_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_i2c_scan_para,
        ctc_cli_chip_set_i2c_scan_para_cmd,
        "chip set i2c-scan para (control-id ID |) (switch-id ID |) (chip-id ID |) \
        {(dev-addr ADDR offset OFF length LEN) | (bitmap MAP0 MAP1) (interval ITVL)}",
        "chip module",
        "set operate",
        "auto scan operate",
        "scan parameter",
        "i2c master id",
        "<0 - 1>",
        "i2c switch id",
        "<1 - 16>, 0 - disable",
        "local chip id",
        "ID",
        "i2c slave address",
        "ADDR",
        "i2c slave offset",
        "OFFSET",
        "length",
        "LENGTH",
        "bitmap",
        "indicate <0-31>",
        "indicate <32-63>",
        "interval",
        "INTERVAL")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_i2c_scan_t scan_para;
    uint8 index = 0;

    sal_memset(&scan_para, 0, sizeof(ctc_chip_i2c_scan_t));

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("control-id", scan_para.ctl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("switch-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("switch-id", scan_para.i2c_switch_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("chip-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("chip-id", scan_para.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dev-addr");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT16_RANGE("dev-addr", scan_para.dev_addr, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        scan_para.op_flag |= CTC_CHIP_SFP_SCAN_REG_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("offset");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("offset", scan_para.offset, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        scan_para.op_flag |= CTC_CHIP_SFP_SCAN_REG_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("length");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("length", scan_para.length, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        scan_para.op_flag |= CTC_CHIP_SFP_SCAN_REG_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bitmap");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("bitmap", scan_para.slave_bitmap[0], argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        CTC_CLI_GET_UINT32_RANGE("bitmap", scan_para.slave_bitmap[1], argv[index + 2], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_SFP_BITMAP_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("interval", scan_para.interval, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        scan_para.op_flag |= CTC_CHIP_SFP_INTERVAL_OP;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_i2c_scan_para(g_api_lchip, &scan_para);
    }
    else
    {
       ret = ctc_chip_set_i2c_scan_para(&scan_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_i2c_scan_en,
        ctc_cli_chip_set_i2c_scan_en_cmd,
        "chip set i2c-scan (enable | disable)",
        "chip module",
        "set operate",
        "i2c scan",
        "scan enable",
        "scan disable")
{
    int32 ret       = CLI_SUCCESS;
    bool enable;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_i2c_scan_en(g_api_lchip, enable);
    }
    else
    {
       ret = ctc_chip_set_i2c_scan_en(enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_read_i2c_buffer,
        ctc_cli_chip_read_i2c_buffer_cmd,
        "chip read i2c-buffer gchip CHIP_ID length LENGTH i2c-id I2C_ID",
        "chip module",
        "read operate",
        "i2c device buffer",
        "local chip id",
        "global chip id value",
        "buffer length",
        "length value",
        "i2c_id",
        "slice id")
{
    int32 ret       = CLI_SUCCESS;
    uint32 length = 0;
    uint32 index = 0;
    ctc_chip_i2c_scan_read_t i2c_scan_read = {0};

    CTC_CLI_GET_UINT8_RANGE("gchip", i2c_scan_read.gchip, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT32_RANGE("length", length, argv[1], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT8_RANGE("i2c-id", i2c_scan_read.ctl_id, argv[2], 0, CTC_MAX_UINT8_VALUE);

    i2c_scan_read.p_buf = (uint8*)sal_malloc(length);
    if (NULL == i2c_scan_read.p_buf)
    {
        return CLI_ERROR;
    }
    i2c_scan_read.len = length;

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_read_i2c_buf(g_api_lchip, &i2c_scan_read);
    }
    else
    {
       ret = ctc_chip_read_i2c_buf(&i2c_scan_read);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        sal_free(i2c_scan_read.p_buf);
        i2c_scan_read.p_buf = NULL;
        return CLI_ERROR;
    }

    for (index = 0; index < i2c_scan_read.len; index++)
    {
        if (((index % 16) == 0) && (index != 0))
        {
            ctc_cli_out("\n");
        }
        ctc_cli_out("  0x%02x", i2c_scan_read.p_buf[index]);
    }
    ctc_cli_out("\n");

    sal_free(i2c_scan_read.p_buf);
    i2c_scan_read.p_buf = NULL;

    return ret;

}

CTC_CLI(ctc_cli_chip_set_i2c_clock_frequency,
        ctc_cli_chip_set_i2c_clock_frequency_cmd,
        "chip set i2c control-id ID freq FREQ (lchip LCHIP|)",
        "Chip module",
        "Set",
        "I2c master",
        "Control id",
        "Control id value",
        "I2c clock frequency",
        "Frequency value(K)",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint8 ctl_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint16 freq = 0;
    ctc_chip_peri_clock_t peri_clock;

    CTC_CLI_GET_UINT8("contrl id", ctl_id, argv[0]);
    CTC_CLI_GET_UINT16("freq", freq, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(&peri_clock, 0, sizeof(ctc_chip_peri_clock_t));
    peri_clock.type = CTC_CHIP_PERI_I2C_TYPE;
    peri_clock.ctl_id = ctl_id;
    peri_clock.clock_val = freq;
    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_PERI_CLOCK, &peri_clock);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_PERI_CLOCK, &peri_clock);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_show_i2c_clock_frequency,
        ctc_cli_chip_show_i2c_clock_frequency_cmd,
        "show chip i2c control-id ID freq (lchip LCHIP|)",
        "Show",
        "Chip module",
        "I2c",
        "Control id",
        "Control id value",
        "I2c clock frequency",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint8 ctl_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    ctc_chip_peri_clock_t peri_clock;

    CTC_CLI_GET_UINT8("contrl id", ctl_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(&peri_clock, 0, sizeof(ctc_chip_peri_clock_t));
    peri_clock.type = CTC_CHIP_PERI_I2C_TYPE;
    peri_clock.ctl_id = ctl_id;
    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_property(g_api_lchip, CTC_CHIP_PROP_PERI_CLOCK, &peri_clock);
    }
    else
    {
        ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_PERI_CLOCK, &peri_clock);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("I2c clock frequency is %d Khz \n", peri_clock.clock_val);

    return ret;
}

CTC_CLI(ctc_cli_chip_set_mac_led_mode,
        ctc_cli_chip_set_mac_led_mode_cmd,
        "chip set mac-led para port-id PORT_ID mode MODE { (polarity POLARITY) | \
        ((first-mode FISRT_MODE) (sec-mode SEC_MODE |)) | control-id ID | lport-en |} (lchip LCHIP|)",
        "chip module",
        "set operate",
        "mac led ",
        "parameter",
        "Port id",
        "Mac id or Lport",
        "mac led mode",
        "<0-1>",
        "polarity",
        "polarity value",
        "first led mode",
        "first mode value",
        "second led mode",
        "second mode value",
        "control-id",
        "<0 - 1>",
        "Lport enable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_led_para_t led_para;
    ctc_chip_mac_led_type_t led_type;
    uint8 index = 0;

    sal_memset(&led_para, 0, sizeof(ctc_chip_led_para_t));

    CTC_CLI_GET_UINT16("gport", led_para.port_id, argv[0]);

    CTC_CLI_GET_UINT32_RANGE("mode", led_type, argv[1], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("polarity");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("polarity", led_para.polarity, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        led_para.op_flag |= CTC_CHIP_LED_POLARITY_SET_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("first-mode");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("first-mode", led_para.first_mode, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        led_para.op_flag |= CTC_CHIP_LED_MODE_SET_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sec-mode");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT32_RANGE("sec-mode", led_para.sec_mode, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        led_para.op_flag |= CTC_CHIP_LED_MODE_SET_OP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("control-id", led_para.ctl_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lport-en");
    if (index != 0xFF)
    {
       led_para.lport_en = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", led_para.lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_mac_led_mode(g_api_lchip, &led_para, led_type);
    }
    else
    {
        ret = ctc_chip_set_mac_led_mode(&led_para, led_type);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_reset_hw,
        ctc_cli_chip_reset_hw_cmd,
        "chip reset-hw (lchip LCHIP|)",
        "Chip module",
        "Reset hardware",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret   = CLI_SUCCESS;
    uint8  lchip = 0;
    uint8  enable;
    uint8  index;
    enable = 1;


    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_RESET_HW, &enable);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_RESET_HW, &enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    return ret;
}

CTC_CLI(ctc_cli_chip_set_mem_scan_mode,
        ctc_cli_chip_set_mem_scan_mode_cmd,
        "chip mem-scan {tcam | sbe} (continuous SCAN_INTERVAL | once | stop) (lchip LCHIP|)",
        "chip module",
        "Memory scan",
        "Tcam key",
        "Single bit error",
        "Scan Continuously",
        "interval time between every scan, unit is minute, 0 means disable scan",
        "Scan once",
        "Stop scan",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret   = CLI_SUCCESS;
    uint8  index = 0, scan_tcam = 0, scan_sbe = 0, lchip = 0;
    ctc_chip_mem_scan_cfg_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_chip_mem_scan_cfg_t));
    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_property(g_api_lchip, CTC_CHIP_PROP_MEM_SCAN, &cfg);
    }
    else
    {
        ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_MEM_SCAN, &cfg);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("tcam");
    if (index != 0xFF)
    {
        scan_tcam = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sbe");
    if (index != 0xFF)
    {
        scan_sbe = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("continuous");
    if (index != 0xFF)
    {
        if (scan_tcam)
        {
            cfg.tcam_scan_mode = 1;
            CTC_CLI_GET_UINT32("interval", cfg.tcam_scan_interval, argv[index + 1]);
        }
        if (scan_sbe)
        {
            cfg.sbe_scan_mode = 1;
            CTC_CLI_GET_UINT32("interval", cfg.sbe_scan_interval, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("once");
    if (index != 0xFF)
    {
        if (scan_tcam)
        {
            cfg.tcam_scan_mode = 0;
        }
        if (scan_sbe)
        {
            cfg.sbe_scan_mode = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stop");
    if (index != 0xFF)
    {
        if (scan_tcam)
        {
            cfg.tcam_scan_mode = 2;
        }
        if (scan_sbe)
        {
            cfg.sbe_scan_mode = 2;
        }
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_MEM_SCAN, &cfg);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_MEM_SCAN, &cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_chip_set_mac_led_en,
        ctc_cli_chip_set_mac_led_en_cmd,
        "chip set mac-led (enable | disable) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "mac led",
        "enable",
        "disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    bool enable;
    uint8 lchip = 0;
    uint8 index = 0;

    if (CLI_CLI_STR_EQUAL("enable", 0))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_MAC_LED_EN, (void*)&enable);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_MAC_LED_EN, (void*)&enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_mac_led_clock_frequency,
        ctc_cli_chip_set_mac_led_clock_frequency_cmd,
        "chip set mac-led (freq FREQ | interval INTERVAL) (lchip LCHIP|)",
        "Chip module",
        "Set",
        "Mac led",
        "Mac led clock frequency",
        "frequency value(M)",
        "Mac led blink interval",
        "interval value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint16 freq = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint32 interval = 0;
    void* value = 0;
    ctc_chip_peri_clock_t peri_clock;
    uint8 type = 0;
    sal_memset(&peri_clock, 0, sizeof(ctc_chip_peri_clock_t));

    index = CTC_CLI_GET_ARGC_INDEX("freq");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("freq-id", freq, argv[index + 1]);
        peri_clock.type = CTC_CHIP_PERI_MAC_LED_TYPE;
        peri_clock.clock_val = freq;
        type = CTC_CHIP_PROP_PERI_CLOCK;
        value = &peri_clock;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("interval", interval, argv[index + 1]);
        type = CTC_CHIP_PROP_MAC_LED_BLINK_INTERVAL;
        value = &interval;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, type, value);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, type, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_show_mac_led_clock_frequency,
        ctc_cli_chip_show_mac_led_clock_frequency_cmd,
        "show chip mac-led (freq | interval) (lchip LCHIP|)",
        "Show",
        "Chip module",
        "Mac led",
        "I2c clock frequency",
        "Mac led blink interval",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    uint8 lchip = 0;
    uint8 index = 0;
    void* value = 0;
    uint8 type = 0;
    uint32 interval = 0;
    ctc_chip_peri_clock_t peri_clock;

    sal_memset(&peri_clock, 0, sizeof(ctc_chip_peri_clock_t));
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("freq");
    if (0xFF != index)
    {
        peri_clock.type = CTC_CHIP_PERI_MAC_LED_TYPE;
        type = CTC_CHIP_PROP_PERI_CLOCK;
        value = &peri_clock;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interval");
    if (0xFF != index)
    {
        type = CTC_CHIP_PROP_MAC_LED_BLINK_INTERVAL;
        value = &interval;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_property(g_api_lchip, type, value);
    }
    else
    {
        ret = ctc_chip_get_property(lchip, type, value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    if(type == CTC_CHIP_PROP_PERI_CLOCK)
    {
        ctc_cli_out("Mac led clock frequency is %d Khz \n", peri_clock.clock_val);
    }
    else
    {
        ctc_cli_out("Mac led blink interval is %u \n", interval);
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_gpio_operation,
        ctc_cli_chip_gpio_operation_cmd,
        "chip gpio gpio-id GPIO_ID (mode MODE | output VALUE | input) (lchip LCHIP|)",
        "chip module",
        "gpio",
        "gpio id",
        "<0-15>",
        "gpio mode",
        "0:input 1:output 2:special function",
        "output value",
        "0-1",
        "input value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 lchip = 0;
    ctc_chip_gpio_params_t gpio_param = {0};

    CTC_CLI_GET_UINT8_RANGE("gpio-id", gpio_param.gpio_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", gpio_param.value , argv[index + 1]);
        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_GPIO_MODE, (void*)&gpio_param);
        }
        else
        {
            ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_GPIO_MODE, (void*)&gpio_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("output");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("output", gpio_param.value , argv[index + 1]);
        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_GPIO_OUT, (void*)&gpio_param);
        }
        else
        {
            ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_GPIO_OUT, (void*)&gpio_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("input");
    if (0xFF != index)
    {
        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, CTC_CHIP_PROP_GPIO_IN, (void*)&gpio_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_GPIO_IN, (void*)&gpio_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("GPIO %d Input Signal %u \n", gpio_param.gpio_id, gpio_param.value);
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_access_mode,
        ctc_cli_chip_set_access_mode_cmd,
        "chip set access mode (PCI|I2C)",
        "chip module",
        "set operate",
        "access",
        "mode",
        "PCI access",
        "I2C access")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_access_type_t access_type;

    if (0 == sal_memcmp("PCI", argv[0], 3))
    {
        access_type = CTC_CHIP_PCI_ACCESS;
    }
    else
    {
        access_type = CTC_CHIP_I2C_ACCESS;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_access_type(g_api_lchip, access_type);
    }
    else
    {
        ret = ctc_chip_set_access_type(access_type);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_set_serdes_mode,
        ctc_cli_chip_set_serdes_mode_cmd,
        "chip set serdes SERDES mode (100base-fx|xfi|sgmii|xsgmii|qsgmii|xaui|d-xaui|xlg|cg|2dot5g|usxgmii0|usxgmii1|usxgmii2|xxvg|lg|none)(overclocking-speed VALUE|)(lport LPORT|)(lchip LCHIP |)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "serdes mode",
        "100base-fx mode",
        "xfi mode",
        "sgmii mode",
        "xsgmii mode",
        "qsgmii mode",
        "xaui mode",
        "d-xaui mode",
        "xlg mode",
        "cg mode",
        "2.5g mode",
        "usxgmii single mode",
        "usxgmii multi 1g/2.5g mode",
        "usxgmii multi 5g mode",
        "xxvg mode",
        "lg mode",
        "None",
        "Overclocking speed",
        "1:11.06G 2:12.12G 3:12.58G 4:27.27G 11:11.06G 12:12.58G",
        "Lport valid",
        "Lport value",
        "local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_info_t serdes_info;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&serdes_info, 0, sizeof(ctc_chip_serdes_info_t));
    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_info.serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    if (0 == sal_memcmp("100base-fx", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_100BASEFX_MODE;
    }
    else if (0 == sal_memcmp("xfi", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XFI_MODE;
    }
    else if (0 == sal_memcmp("sgmii", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_SGMII_MODE;
    }
    else if (0 == sal_memcmp("xsgmii", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XSGMII_MODE;
    }
    else if (0 == sal_memcmp("qsgmii", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_QSGMII_MODE;
    }
    else if (0 == sal_memcmp("xaui", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XAUI_MODE;
    }
    else if (0 == sal_memcmp("d-xaui", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_DXAUI_MODE;
    }
    else if (0 == sal_memcmp("xlg", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XLG_MODE;
    }
    else if (0 == sal_memcmp("cg", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_CG_MODE;
    }
    else if (0 == sal_memcmp("2dot5g", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_2DOT5G_MODE;
    }
    else if (0 == sal_memcmp("usxgmii0", argv[1], 8))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_USXGMII0_MODE;
    }
    else if (0 == sal_memcmp("usxgmii1", argv[1], 8))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_USXGMII1_MODE;
    }
    else if (0 == sal_memcmp("usxgmii2", argv[1], 8))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_USXGMII2_MODE;
    }
    else if (0 == sal_memcmp("xxvg", argv[1], 4))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_XXVG_MODE;
    }
    else if (0 == sal_memcmp("lg", argv[1], 2))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_LG_MODE;
    }
    else if (0 == sal_memcmp("none", argv[1], 3))
    {
        serdes_info.serdes_mode = CTC_CHIP_SERDES_NONE_MODE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

    }

    index = CTC_CLI_GET_ARGC_INDEX("overclocking-speed");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("overclocking speed", serdes_info.overclocking_speed, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lport");
    if (0xFF != index)
    {
        serdes_info.lport_valid= 1;
        CTC_CLI_GET_UINT32("lport", serdes_info.lport, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_serdes_mode(g_api_lchip, &serdes_info);
    }
    else
    {
        ret = ctc_chip_set_serdes_mode(lchip, &serdes_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_discard_same_mac_or_ip,
        ctc_cli_chip_set_discard_same_mac_or_ip_cmd,
        "chip global-cfg (discard-same-mac|discard-same-ip|discard-ttl-0|discard-mcast-macsa|discard-tcp-syn-0|discard-tcp-null| \
        discard-tcp-xmas|discard-tcp-syn-fin| discard-same-l4-port| discard-icmp-fragment|acl-change-cos-only|eloop-use-logic-destport | \
        arp-macda-check-en | arp-macsa-check-en | arp-ip-check-en | arp-check-fail-to-cpu|nh-force-bridge-dis|nh-mcast-logic-rep-en|oam-policer-en| \
        parser-outer-always-cvlan|arp-vlan-class-en|discard-macsa-0|vpws-snooping-parser|disable-egs-stk-acl|stk-with-igs-pkt-hdr) (enable|disable)",
        "Chip module",
        "Global config",
        "Discard same mac address",
        "Discard same ip address",
        "Diacard ttl 0",
        "Discard mcast mac sa",
        "Discard TCP packets with SYN equals 0",
        "Discard TCP packets with flags and sequence number equal 0",
        "Discard TCP packets with FIN, URG, PSH bits set and sequence number equals 0",
        "Discard TCP packets with SYN & FIN bits set",
        "Discard same L4 source and destination port packets",
        "Discard fragmented ICMP packets",
        "Acl vlan edit only change cos",
        "Eloop use logic dest port",
        "Drop ARP reply packets which macda not equal to target mac",
        "Drop ARP packets which macsa not equal to sender mac",
        "Drop ARP packets with invalid ip address",
        "ARP packets check fail and copy to cpu",
        "NH disable force bridge for ip-mcast when l3 match",
        "Mcast logic replication",
        "Oam policer enable",
		"Parser outer vlan is always cvlan",
        "ARP vlan class enable",
        "Discard macSa 0 packet",
        "Snooping parser",
        "Disable egress stacking acl lookup",
        "Stacking with ingress packet header",
        "Enable",
        "Disable")
{
    int32 ret       = CLI_SUCCESS;
    bool enable = FALSE;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-mac");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-ip");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-ttl-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TTL_0_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-mcast-macsa");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_MCAST_SA_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-change-cos-only");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ACL_CHANGE_COS_ONLY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-syn-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-null");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_NULL_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-xmas");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_XMAS_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-syn-fin");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-l4-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-icmp-fragment");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("eloop-use-logic-destport");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ELOOP_USE_LOGIC_DESTPORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-macda-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_MACDA_CHECK_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-macsa-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_MACSA_CHECK_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-ip-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_IP_CHECK_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-check-fail-to-cpu");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-force-bridge-dis");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_FORCE_BRIDGE_DISABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mcast-logic-rep-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_MCAST_LOGIC_REP_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("oam-policer-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_OAM_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("parser-outer-always-cvlan");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_PARSER_OUTER_ALWAYS_CVLAN_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vpws-snooping-parser");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VPWS_SNOOPING_PARSER;
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-vlan-class-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_VLAN_CLASS_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("discard-macsa-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_MACSA_0_PKT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("disable-egs-stk-acl");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_EGS_STK_ACL_DIS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stk-with-igs-pkt-hdr");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (INDEX_VALID(index))
    {
        enable = TRUE;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, type, &enable);
    }
    else
    {
        ret = ctc_global_ctl_set(type, &enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_show_discard_same_mac_or_ip,
        ctc_cli_chip_show_discard_same_mac_or_ip_cmd,
        "show chip global-cfg (discard-same-mac|discard-same-ip|discard-ttl-0|discard-mcast-macsa|discard-tcp-syn-0|discard-tcp-null| \
        discard-tcp-xmas|discard-tcp-syn-fin| discard-same-l4-port| discard-icmp-fragment| acl-change-cos-only|eloop-use-logic-destport | \
        arp-macda-check-en | arp-macsa-check-en | arp-ip-check-en | arp-check-fail-to-cpu|nh-force-bridge-dis|nh-mcast-logic-rep-en| \
        oam-policer-en|parser-outer-always-cvlan|arp-vlan-class-en|discard-macsa-0|vpws-snooping-parser|disable-egs-stk-acl|stk-with-igs-pkt-hdr)",
        "Show",
        "Chip module",
        "Global config",
        "Discard same mac address",
        "Discard same ip address",
        "Diacard ttl 0",
        "Discard mcast mac sa",
        "Discard TCP packets with SYN equals 0",
        "Discard TCP packets with flags and sequence number equal 0",
        "Discard TCP packets with FIN, URG, PSH bits set and sequence number equals 0",
        "Discard TCP packets with SYN & FIN bits set",
        "Discard same L4 source and destination port packets",
        "Discard fragmented ICMP packets",
        "Acl vlan edit only change cos",
        "Eloop use logic dest port",
        "Drop ARP reply packets which macda not equal to target mac",
        "Drop ARP packets which macsa not equal to sender mac",
        "Drop ARP packets with invalid ip address",
        "ARP packets check fail and copy to cpu",
        "NH disable force bridge for ip-mcast when l3 match",
        "Mcast logic replication",
        "Oam policer enable",
        "Parser outer vlan is always cvlan",
        "ARP vlan class enable",
        "Discard mac sa 0 packet",
        "Snooping parser",
        "Disable egress stacking acl lookup",
        "Stacking with ingress chip packet header")
{
    int32 ret       = CLI_SUCCESS;
    bool enable = FALSE;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-mac");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_MACDASA_PKT;
        ctc_cli_out("Show chip discard-same-mac info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-ip");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_IPDASA_PKT;
        ctc_cli_out("Show chip discard-same-ip info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-ttl-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TTL_0_PKT;
        ctc_cli_out("Show chip discard-ttl-0 info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-mcast-macsa");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_MCAST_SA_PKT;
        ctc_cli_out("Show chip discard-mcast-macsa info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-syn-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_SYN_0_PKT;
        ctc_cli_out("Show chip discard-tcp-syn-0 info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-null");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_NULL_PKT;
        ctc_cli_out("Show chip discard-tcp-null info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-xmas");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_XMAS_PKT;
        ctc_cli_out("Show chip discard-tcp-xmas info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-tcp-syn-fin");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_TCP_SYN_FIN_PKT;
        ctc_cli_out("Show chip discard-tcp-syn-fin info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-same-l4-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_SAME_L4_PORT_PKT;
        ctc_cli_out("Show chip discard-same-l4-port info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard-icmp-fragment");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_ICMP_FRAG_PKT;
        ctc_cli_out("Show chip discard-icmp-fragment info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-change-cos-only");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ACL_CHANGE_COS_ONLY;
        ctc_cli_out("Show chip acl-change-cos-only info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("eloop-use-logic-destport");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ELOOP_USE_LOGIC_DESTPORT;
        ctc_cli_out("Show chip eloop-use-logic-destport info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-macda-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_MACDA_CHECK_EN;
        ctc_cli_out("Show chip arp-macda-check-en info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-macsa-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_MACSA_CHECK_EN;
        ctc_cli_out("Show chip arp-macsa-check-en info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-ip-check-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_IP_CHECK_EN;
        ctc_cli_out("Show chip arp-ip-check-en info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp-check-fail-to-cpu");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_CHECK_FAIL_TO_CPU;
        ctc_cli_out("Show chip arp-check-fail-to-cpu info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-force-bridge-dis");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_FORCE_BRIDGE_DISABLE;
        ctc_cli_out("Show chip nh-force-bridge-dis info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mcast-logic-rep-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_MCAST_LOGIC_REP_EN;
        ctc_cli_out("Show chip nh-mcast-logic-rep-en info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("oam-policer-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_OAM_POLICER_EN;
        ctc_cli_out("Show chip oam-policer-en info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("parser-outer-always-cvlan");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_PARSER_OUTER_ALWAYS_CVLAN_EN;
        ctc_cli_out("Show chip parser-outer-always-cvlan info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("vpws-snooping-parser");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VPWS_SNOOPING_PARSER;
        ctc_cli_out("Show chip vpws-snooping-parser info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-vlan-class-en");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ARP_VLAN_CLASS_EN;
        ctc_cli_out("Show chip arp-vlan-class-en info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("discard-macsa-0");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_DISCARD_MACSA_0_PKT;
        ctc_cli_out("Show chip discard-macsa-0 info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("disable-egs-stk-acl");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_EGS_STK_ACL_DIS;
        ctc_cli_out("Show chip disable-egs-stk-acl info\n");
        ctc_cli_out("==================================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("stk-with-igs-pkt-hdr");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_STK_WITH_IGS_PKT_HDR_EN;
        ctc_cli_out("Show chip stk-with-igs-pkt-hdr info\n");
        ctc_cli_out("==================================\n");
    }
    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &enable);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("value : %d\n", enable?1:0);
    ctc_cli_out("==================================\n");

    return ret;

}

CTC_CLI(ctc_cli_chip_set_global_dlb_mode,
        ctc_cli_chip_set_global_dlb_mode_cmd,
        "chip global-cfg ecmp (reblance-mode (normal|first|packet) | dlb-mode (elephant|tcp|all))",
        "chip module",
        "Global config",
        "ECMP config",
        "ECMP DLB reblance mode",
        "Normal mode, inactive flow do reblance",
        "First mode, only new flow do reblance",
        "Packet mode, every 256 packets do reblance",
        "ECMP DLB flow mode",
        "Elephant flow do DLB",
        "Tcp flow do DLB",
        "All flow do DLB",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("reblance-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ECMP_REBALANCE_MODE;

        index = CTC_CLI_GET_ARGC_INDEX("normal");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_REBALANCE_MODE_NORMAL;
        }

        index = CTC_CLI_GET_ARGC_INDEX("first");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_REBALANCE_MODE_FIRST;
        }

        index = CTC_CLI_GET_ARGC_INDEX("packet");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_REBALANCE_MODE_PACKET;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("dlb-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ECMP_DLB_MODE;

        index = CTC_CLI_GET_ARGC_INDEX("elephant");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_DLB_MODE_ELEPHANT;
        }

        index = CTC_CLI_GET_ARGC_INDEX("tcp");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_DLB_MODE_TCP;
        }

        index = CTC_CLI_GET_ARGC_INDEX("all");
        if (INDEX_VALID(index))
        {
            val = CTC_GLOBAL_ECMP_DLB_MODE_ALL;
        }
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_set(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_dlb_mode,
        ctc_cli_chip_get_global_dlb_mode_cmd,
        "show chip global-cfg ecmp (reblance-mode | dlb-mode)",
        "show",
        "chip module",
        "Global config",
        "ECMP config",
        "ECMP DLB reblance mode,1:normal,2:first,3:packet",
        "ECMP DLB flow mode,1:all,2:elephant,3:tcp")
{
    int32 ret = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("reblance-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ECMP_REBALANCE_MODE;
        ctc_cli_out("Show chip ecmp reblance-mode info\n");
        ctc_cli_out("==================================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("dlb-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_ECMP_DLB_MODE;
        ctc_cli_out("Show chip ecmp dlb-mode info\n");
        ctc_cli_out("==================================\n");
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("value : %d\n", val);
    ctc_cli_out("==================================\n");

    return ret;

}

CTC_CLI(ctc_cli_chip_set_global_dlb_cfg,
        ctc_cli_chip_set_global_dlb_cfg_cmd,
        "chip global-cfg (ecmp (flow-aging-interval|flow-inactive-interval|flow-pkt-interval)|linkagg flow-inactive-interval | stacking-trunk flow-inactive-interval) value VALUE",
        "chip module",
        "Global config",
        "ECMP config",
        "ECMP DLB flow aging time, uint:ms, value 0:disable flow aging",
        "ECMP DLB flow inactive time, uint:us",
        "ECMP DLB rebalance packets interval",
        "Linkagg config",
        "Linkagg DLB flow inactive time, uint:ms",
        "Stacking trunk config",
        "Stacking trunk DLB flow inactive time, uint:ms",
        "Value",
        "Value")
{
    int32 ret       = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-aging-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_AGING_INTERVAL;
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_INACTIVE_INTERVAL;
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-pkt-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_PKT_INTERVAL;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkagg");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_LINKAGG_FLOW_INACTIVE_INTERVAL;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stacking-trunk");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_STACKING_TRUNK_FLOW_INACTIVE_INTERVAL;
        }
    }

    CTC_CLI_GET_UINT32_RANGE("value", val, argv[2], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_set(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_dlb_cfg,
        ctc_cli_chip_get_global_dlb_cfg_cmd,
        "show chip global-cfg (ecmp (flow-aging-interval|flow-inactive-interval|flow-pkt-interval)|linkagg flow-inactive-interval|stacking-trunk flow-inactive-interval)",
        "show",
        "chip module",
        "Global config",
        "ECMP config",
        "ECMP DLB flow aging time, uint:ms, value 0:disable flow aging",
        "ECMP DLB flow inactive time, uint:us",
        "ECMP DLB rebalance packets interval",
        "Linkagg config",
        "Linkagg DLB flow inactive time, uint:ms",
        "Stacking trunk config",
        "Stacking trunk DLB flow inactive time, uint:ms")
{
    int32 ret       = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-aging-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_AGING_INTERVAL;
            ctc_cli_out("Show chip ecmp flow-aging-interval(ms) info\n");
            ctc_cli_out("==================================\n");
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_INACTIVE_INTERVAL;
            ctc_cli_out("Show chip ecmp flow-inactive-interval(us) info\n");
            ctc_cli_out("==================================\n");
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-pkt-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_ECMP_FLOW_PKT_INTERVAL;
            ctc_cli_out("Show chip ecmp flow-pkt-interval info\n");
            ctc_cli_out("==================================\n");
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("linkagg");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_LINKAGG_FLOW_INACTIVE_INTERVAL;
            ctc_cli_out("Show chip linkagg flow-inactive-interval(ms) info\n");
            ctc_cli_out("==================================\n");
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stacking-trunk");
    if (INDEX_VALID(index))
    {
        index = CTC_CLI_GET_ARGC_INDEX("flow-inactive-interval");
        if (INDEX_VALID(index))
        {
            type = CTC_GLOBAL_STACKING_TRUNK_FLOW_INACTIVE_INTERVAL;
            ctc_cli_out("Show chip stacking-trunk flow-inactive-interval(ms) info\n");
            ctc_cli_out("==================================\n");
        }
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("value : %d\n", val);
    ctc_cli_out("==================================\n");
    return ret;

}

CTC_CLI(ctc_cli_chip_set_global_flow_prop,
        ctc_cli_chip_set_global_flow_prop_cmd,
        "chip global-cfg flow-property {igs-vlan-range-mode VALUE | egs-vlan-range-mode VALUE}",
        "chip module",
        "Global config",
        "Flow property config",
        "Ingress vlan range mode",
        "Value",
        "Egress vlan range mode",
        "Value")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_global_flow_property_t flow_prop;

    sal_memset(&flow_prop, 0, sizeof(ctc_global_flow_property_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_FLOW_PROPERTY, &flow_prop);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_FLOW_PROPERTY, &flow_prop);
    }

    index = CTC_CLI_GET_ARGC_INDEX("igs-vlan-range-mode");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("igs-vlan-range-mode",flow_prop.igs_vlan_range_mode,argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("egs-vlan-range-mode");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("egs-vlan-range-mode",flow_prop.egs_vlan_range_mode,argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_FLOW_PROPERTY, &flow_prop);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_FLOW_PROPERTY, &flow_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_flow_prop,
        ctc_cli_chip_get_global_flow_prop_cmd,
        "show chip global-cfg flow-property",
        "show",
        "chip module",
        "Global config",
        "Flow property config")
{
    int32 ret = CLI_SUCCESS;
    uint8 type = 0;
    ctc_global_flow_property_t flow_prop;

    sal_memset(&flow_prop, 0, sizeof(ctc_global_flow_property_t));

    type = CTC_GLOBAL_FLOW_PROPERTY;

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &flow_prop);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &flow_prop);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show chip flow-property info\n");
    ctc_cli_out("==============================\n");
    ctc_cli_out("igs-vlan-range-mode : %d\n", flow_prop.igs_vlan_range_mode);
    ctc_cli_out("egs-vlan-range-mode : %d\n", flow_prop.egs_vlan_range_mode);
    ctc_cli_out("==============================\n");

    return ret;

}

CTC_CLI(ctc_cli_chip_set_global_acl_property,
        ctc_cli_chip_set_global_acl_property_cmd,
        "chip global-cfg acl-property direction (ingress|egress|both) (priority VALUE |) "\
        "{discard-pkt-lkup-en VALUE | key-ipv6-da-addr-mode VALUE | key-ipv6-sa-addr-mode VALUE | key-cid-en VALUE" \
        "|cid-key-ipv6-da-addr-mode VALUE | cid-key-ipv6-sa-addr-mode VALUE | key-ctag-en VALUE " \
        "|l3-key-ipv6-use-compress-addr VALUE| copp-key-use-ext-mode L3TYPE_BITMAP|l2l3-ext-key-use-l2l3-key L3TYPE_BITMAP|\
        stp-blocked-pkt-lkup-en VALUE | random-log-pri VALUE|l2-type-as-vlan-num  VALUE|fwd-ext-key-l2-hdr-en VALUE}" ,
        "Chip module",
        "Global config",
        "Acl property",
        "Direction",
        "Ingress",
        "Egress",
        "Both",
        "Priority",
        "ACL priority (ACL lookup level)",
        "Discard pkt lkup en ",
        "If set,indicate the discarded packets will do ACL lookup",
        "Key ipv6 da addr mode",
        "Ipv6 address compress mode,refer to ctc_global_ipv6_addr_compress_mode_t",
        "Key ipv6 sa addr mode",
        "Ipv6 address compress mode,refer to ctc_global_ipv6_addr_compress_mode_t",
        "Key cid en",
        "If set, key enable CID",
        "Cid key ipv6 da addr mode",
        "Ipv6 address compress mode,refer to ctc_global_ipv6_addr_compress_mode_t",
        "Cid key ipv6 sa addr mode",
        "Ipv6 address compress mode,refer to ctc_global_ipv6_addr_compress_mode_t",
        "Key ctag en",
        "If set, key vlan tag will be used as CVLAN",
        "L3-key use compress-address for ipv6",
        "If set,ipv6 address use compress mode (64 bit ipaddr) in L3 Key",
        "Which L3Type's packet will use copp ext key",
        "L3 Type Bitmap, only ipv6 use copp ext key by default",
        "Force L2L3Ext Key To L2L3Key",
        "L3 Type Bitmap, only ipv4/ipv6 use L2L3Ext key by default",
        "Stp bloacked packet do ACL lookup",
        "VAlUE",
        "Random Log mapped new acl priority",
        "VAlUE",
        "L2 type as vlan number", "VALUE",
        "Forward ext key l2 header enable",
        "If set, forward ext key will include l2 header fields, else include UDF fields")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;

	uint32 value = 0;
	uint8  loop = 0;

    ctc_global_acl_property_t acl_property;
    sal_memset(&acl_property,0,sizeof(ctc_global_acl_property_t));

    /*direction*/
    acl_property.dir = CTC_BOTH_DIRECTION;
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        acl_property.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        acl_property.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Priority", acl_property.lkup_level, argv[index+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }


    index = CTC_CLI_GET_ARGC_INDEX("discard-pkt-lkup-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Discard pkt lkup en", acl_property.discard_pkt_lkup_en, argv[index+1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("key-ipv6-da-addr-mode");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Key ipv6 da addr mode", acl_property.key_ipv6_da_addr_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-ipv6-sa-addr-mode");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Key ipv6 sa addr mode", acl_property.key_ipv6_sa_addr_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-cid-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Key cid en", acl_property.key_cid_en, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid-key-ipv6-da-addr-mode");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Cid key ipv6 da addr mode", acl_property.cid_key_ipv6_da_addr_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cid-key-ipv6-sa-addr-mode");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Cid key ipv6 sa addr mode", acl_property.cid_key_ipv6_sa_addr_mode, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("key-ctag-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Key ctag en", acl_property.key_ctag_en, argv[index+1]);
    }

	index = CTC_CLI_GET_ARGC_INDEX("l3-key-ipv6-use-compress-addr");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("l3-key-ipv6-use-compress-addr", acl_property.l3_key_ipv6_use_compress_addr, argv[index+1]);
    }

	index = CTC_CLI_GET_ARGC_INDEX("copp-key-use-ext-mode");
    if(index != 0xFF)
    {
      CTC_CLI_GET_UINT32("copp-key-use-ext-mode",value, argv[index+1]);
      for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
      {
		   acl_property.copp_key_use_ext_mode[loop] = CTC_IS_BIT_SET(value,loop);
      }

    }

	index = CTC_CLI_GET_ARGC_INDEX("l2l3-ext-key-use-l2l3-key");
    if(index != 0xFF)
    {
      CTC_CLI_GET_UINT32("l2l3-ext-key-use-l2l3-key",value, argv[index+1]);
      for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
      {
		   acl_property.l2l3_ext_key_use_l2l3_key[loop] = CTC_IS_BIT_SET(value,loop);
      }

    }

    index = CTC_CLI_GET_ARGC_INDEX("stp-blocked-pkt-lkup-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Stp bloced pkt lkup en", acl_property.stp_blocked_pkt_lkup_en, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("random-log-pri");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("Random log priority", acl_property.random_log_pri, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("l2-type-as-vlan-num");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("L2 Type as vlan number en", acl_property.l2_type_as_vlan_num, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("fwd-ext-key-l2-hdr-en");
    if(index != 0xFF)
    {
       CTC_CLI_GET_UINT8("fwd-ext-key-l2-hdr-en", acl_property.fwd_ext_key_l2_hdr_en, argv[index+1]);
    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_get_global_acl_property,
        ctc_cli_chip_get_global_acl_property_cmd,
        "show chip global-cfg acl-property direction (ingress|egress) (priority VALUE |)" ,
        "show",
        "chip module",
        "Global config",
        "Acl property",
        "Direction",
        "Ingress",
        "Egress",
        "Priority",
        "ACL priority (ACL lookup level)")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;

	uint8  loop = 0;
	uint32 value = 0;

    ctc_global_acl_property_t acl_property;

    sal_memset(&acl_property,0,sizeof(ctc_global_acl_property_t));

    /*direction*/
    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        acl_property.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        acl_property.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if(index != 0xFF)
    {

       CTC_CLI_GET_UINT8("Acl priority", acl_property.lkup_level, argv[index+1]);

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_ACL_PROPERTY, &acl_property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show chip acl-property info\n");
    ctc_cli_out("==============================\n");
    ctc_cli_out("Direction: %s\n", acl_property.dir ?  "EGRESS" :"INGRESS" );
    ctc_cli_out("Acl_priority : %d\n", acl_property.lkup_level);
    ctc_cli_out("Discard pkt lkup en : %d\n", acl_property.discard_pkt_lkup_en);


    ctc_cli_out("Key ipv6 da addr mode : %d\n", acl_property.key_ipv6_da_addr_mode);
    ctc_cli_out("Key ipv6 sa addr mode : %d\n", acl_property.key_ipv6_sa_addr_mode);
    ctc_cli_out("Key cid en : %d\n", acl_property.key_cid_en);
    ctc_cli_out("Cid key ipv6 da addr mode : %d\n", acl_property.cid_key_ipv6_da_addr_mode);
    ctc_cli_out("Cid key ipv6 sa addr mode: %d\n", acl_property.cid_key_ipv6_sa_addr_mode);
    ctc_cli_out("Key ctag en : %d\n", acl_property.key_ctag_en);
    ctc_cli_out("l3-key use compress-address for ipv6 : %d\n", acl_property.l3_key_ipv6_use_compress_addr);
    ctc_cli_out("Stp blocked pkt lkup en : %d\n", acl_property.stp_blocked_pkt_lkup_en);
    ctc_cli_out("Random log mapped new acl priority : %d\n", acl_property.random_log_pri);
    ctc_cli_out("L2 Type as vlan num : %u\n", acl_property.l2_type_as_vlan_num);
    ctc_cli_out("Fwd ext key l2 header en : %u\n", acl_property.fwd_ext_key_l2_hdr_en);
    value = 0;
    for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
    {
        if(acl_property.copp_key_use_ext_mode[loop])
		{
		     	CTC_BIT_SET(value,loop);
		}
    }
    ctc_cli_out("copp-key use ext mode : 0x%x\n",value);

    value = 0;
    for(loop = 0;loop <MAX_CTC_PARSER_L3_TYPE;loop++ )
    {
        if(acl_property.l2l3_ext_key_use_l2l3_key[loop])
		{
		     	CTC_BIT_SET(value,loop);
		}

     }
     ctc_cli_out("l2l3-ext-key use l2l3-key : 0x%x\n",value);




    ctc_cli_out("==============================\n");

    return ret;

}


CTC_CLI(ctc_cli_chip_set_global_cid_property,
        ctc_cli_chip_set_global_cid_property_cmd,
        "chip global-cfg cid-property {global-cid CID|cid-pair-en|cmd-ethtype ETHTYPE|cmd-parser-en|cross-chip-cid-en|insert-cmd|\
        (reassign-cid-pri (dst-cid|) {default PRI|fwd-table PRI|flow-table PRI|global PRI|pkt PRI|interface PRI|iloop PRI})|}" ,
        "Chip module",
        "Global config",
        "CID property",
        "Global cid",
        "CID Value",
        "Enbale cid-pair function",
        "CMD EthType",
        "EthType",
        "Enable CMD Parser",
        "Cross-chip-cid enable"
        "Insert CMD Header to packet",
        "Reassign CID pipeline priority",
        "Select destination CID",
        "Default CID",
        "Priority",
        "Forward Table CID",
        "Priority",
        "Flow Table CID",
        "Priority",
        "Global CID",
        "Priority",
        "Packet CID",
        "Priority",
        "Interface CID",
        "Priority",
        "Iloop CID",
        "Priority")
{

    int32  ret = CLI_SUCCESS;

    uint8  index = 0;

    ctc_global_cid_property_t cid_property;
    sal_memset(&cid_property,0,sizeof(cid_property));

	index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (INDEX_VALID(index))
    {
	   cid_property.is_dst_cid= 1;
    }



    index = CTC_CLI_GET_ARGC_INDEX("global-cid");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("global-cid", cid_property.global_cid, argv[index+1]);
	   cid_property.global_cid_en = 1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("cid-pair-en");
    if (INDEX_VALID(index))
    {
	   cid_property.cid_pair_en= 1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("cmd-parser-en");
    if (INDEX_VALID(index))
    {
	   cid_property.cmd_parser_en= 1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("cmd-ethtype");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT16("cmd-ethtype", cid_property.cmd_ethtype, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("insert-cmd");
    if (INDEX_VALID(index))
    {
      cid_property.insert_cid_hdr_en = 1;
    }
	index = CTC_CLI_GET_ARGC_INDEX("reassign-cid-pri");
    if (INDEX_VALID(index))
    {
	   cid_property.reassign_cid_pri_en = 1;
    }

	index = CTC_CLI_GET_ARGC_INDEX("default");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("default priority", cid_property.default_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("fwd-table");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("fwd-table priority", cid_property.fwd_table_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("flow-table");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("flow-table priority", cid_property.flow_table_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("global");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("global priority", cid_property.global_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("pkt");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("pkt priority", cid_property.pkt_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("interface");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("interface priority", cid_property.if_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("iloop");
    if (INDEX_VALID(index))
    {
       CTC_CLI_GET_UINT8("iloop priority", cid_property.iloop_cid_pri, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("cross-chip-cid-en");
    if (INDEX_VALID(index))
    {
	   cid_property.cross_chip_cid_en= 1;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_CID_PROPERTY, &cid_property);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_CID_PROPERTY, &cid_property);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_get_global_cid_property,
        ctc_cli_chip_get_global_cid_property_cmd,
        "show chip global-cfg cid-property (dst-cid|)" ,
        "Show",
        "Chip module",
        "Global config",
        "CID property",
        "Select destination CID")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;
    ctc_global_cid_property_t cid_property;

    sal_memset(&cid_property,0,sizeof(ctc_global_cid_property_t));

    index = CTC_CLI_GET_ARGC_INDEX("dst-cid");
    if (INDEX_VALID(index))
    {

       cid_property.is_dst_cid = 1;

    }
    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CID_PROPERTY, &cid_property);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CID_PROPERTY, &cid_property);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("global_cid          : %d\n",cid_property.global_cid);
    ctc_cli_out("global_cid_en       : %d\n",cid_property.global_cid_en);
    ctc_cli_out("cid_pair_en         : %d\n",cid_property.cid_pair_en);
    ctc_cli_out("cmd_parser_en       : %d\n",cid_property.cmd_parser_en);
    ctc_cli_out("cmd_ethtype         : 0x%x\n",cid_property.cmd_ethtype);
	ctc_cli_out("insert CMD Header   : %d\n",cid_property.insert_cid_hdr_en);
    ctc_cli_out("is_dst_cid          : %d\n",cid_property.is_dst_cid );
	ctc_cli_out("cross chip cid en   : %d\n",cid_property.cross_chip_cid_en);
	if(!cid_property.is_dst_cid)
	{
   	   ctc_cli_out("pkt_cid_pri         : %d\n",cid_property.pkt_cid_pri);
   	   ctc_cli_out("iloop_cid_pri       : %d\n",cid_property.iloop_cid_pri);
       ctc_cli_out("global_cid_pri      : %d\n",cid_property.global_cid_pri);
       ctc_cli_out("flow_table_cid_pri  : %d\n",cid_property.flow_table_cid_pri);
       ctc_cli_out("fwd_table_cid_pri   : %d\n",cid_property.fwd_table_cid_pri);
       ctc_cli_out("if_cid_pri          : %d\n",cid_property.if_cid_pri);
       ctc_cli_out("default_cid_pri     : %d\n",cid_property.default_cid_pri);
	}
	else
	{
  	   ctc_cli_out("fwd_table_cid_pri   : %d\n",cid_property.fwd_table_cid_pri);
       ctc_cli_out("flow_table_cid_pri  : %d\n",cid_property.flow_table_cid_pri);
       ctc_cli_out("default_cid_pri     : %d\n",cid_property.default_cid_pri);
	}




    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_capability,
        ctc_cli_chip_get_global_capability_cmd,
        "show chip global-cfg chip-capability",
        "show",
        "chip module",
        "Global config",
        "Chip capability")
{
    int32 ret = CLI_SUCCESS;
    uint8 type = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    type = CTC_GLOBAL_CHIP_CAPABILITY;

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(type, capability);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show chip capability info\n");
    ctc_cli_out("======================================\n");

    PRINT_CAPABLITY(LOGIC_PORT_NUM);
    PRINT_CAPABLITY(MAX_FID);
    PRINT_CAPABLITY(MAX_VRFID);
    PRINT_CAPABLITY(MCAST_GROUP_NUM);
    PRINT_CAPABLITY(VLAN_NUM);
    PRINT_CAPABLITY(VLAN_RANGE_GROUP_NUM);
    PRINT_CAPABLITY(STP_INSTANCE_NUM);
    PRINT_CAPABLITY(LINKAGG_GROUP_NUM);
    PRINT_CAPABLITY(LINKAGG_MEMBER_NUM);
    PRINT_CAPABLITY(LINKAGG_DLB_FLOW_NUM);
    PRINT_CAPABLITY(LINKAGG_DLB_MEMBER_NUM);
    PRINT_CAPABLITY(LINKAGG_DLB_GROUP_NUM);
    PRINT_CAPABLITY(ECMP_GROUP_NUM);
    PRINT_CAPABLITY(ECMP_MEMBER_NUM);
    PRINT_CAPABLITY(ECMP_DLB_FLOW_NUM);
    PRINT_CAPABLITY(EXTERNAL_NEXTHOP_NUM);
    PRINT_CAPABLITY(GLOBAL_DSNH_NUM);
    PRINT_CAPABLITY(MPLS_TUNNEL_NUM);
    PRINT_CAPABLITY(ARP_ID_NUM);
    PRINT_CAPABLITY(L3IF_NUM);
    PRINT_CAPABLITY(OAM_SESSION_NUM);
    PRINT_CAPABLITY(NPM_SESSION_NUM);
    PRINT_CAPABLITY(APS_GROUP_NUM);
    PRINT_CAPABLITY(TOTAL_POLICER_NUM);
    PRINT_CAPABLITY(POLICER_NUM);
    ctc_cli_out("---------------------------------------\n");
    PRINT_CAPABLITY(TOTAL_STATS_NUM);
    PRINT_CAPABLITY(QUEUE_STATS_NUM);
    PRINT_CAPABLITY(POLICER_STATS_NUM);
    PRINT_CAPABLITY(SHARE1_STATS_NUM);
    PRINT_CAPABLITY(SHARE2_STATS_NUM);
    PRINT_CAPABLITY(SHARE3_STATS_NUM);
    PRINT_CAPABLITY(SHARE4_STATS_NUM);
    PRINT_CAPABLITY(ACL0_IGS_STATS_NUM);
    PRINT_CAPABLITY(ACL1_IGS_STATS_NUM);
    PRINT_CAPABLITY(ACL2_IGS_STATS_NUM);
    PRINT_CAPABLITY(ACL3_IGS_STATS_NUM);
    PRINT_CAPABLITY(ACL0_EGS_STATS_NUM);
    PRINT_CAPABLITY(ECMP_STATS_NUM);
    ctc_cli_out("---------------------------------------\n");
    PRINT_CAPABLITY(ROUTE_MAC_ENTRY_NUM);
    PRINT_CAPABLITY(MAC_ENTRY_NUM);
    PRINT_CAPABLITY(BLACK_HOLE_ENTRY_NUM);
    PRINT_CAPABLITY(HOST_ROUTE_ENTRY_NUM);
    PRINT_CAPABLITY(LPM_ROUTE_ENTRY_NUM);
    PRINT_CAPABLITY(IPMC_ENTRY_NUM);
    PRINT_CAPABLITY(MPLS_ENTRY_NUM);
    PRINT_CAPABLITY(TUNNEL_ENTRY_NUM);
    PRINT_CAPABLITY(IPFIX_ENTRY_NUM);
    PRINT_CAPABLITY(EFD_FLOW_ENTRY_NUM);
    ctc_cli_out("---------------------------------------\n");
    PRINT_CAPABLITY(L2PDU_L2HDR_PROTO_ENTRY_NUM);
    PRINT_CAPABLITY(L2PDU_MACDA_ENTRY_NUM);
    PRINT_CAPABLITY(L2PDU_MACDA_LOW24_ENTRY_NUM);
    PRINT_CAPABLITY(L2PDU_L2CP_MAX_ACTION_INDEX);
    PRINT_CAPABLITY(L3PDU_L3HDR_PROTO_ENTRY_NUM);
    PRINT_CAPABLITY(L3PDU_L4PORT_ENTRY_NUM);
    PRINT_CAPABLITY(L3PDU_IPDA_ENTRY_NUM);
    PRINT_CAPABLITY(L3PDU_MAX_ACTION_INDEX);
    ctc_cli_out("---------------------------------------\n");
    PRINT_CAPABLITY(SCL_HASH_ENTRY_NUM);
    PRINT_CAPABLITY(SCL1_HASH_ENTRY_NUM);
    PRINT_CAPABLITY(SCL_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(SCL1_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(SCL2_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(SCL3_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL_HASH_ENTRY_NUM);
    PRINT_CAPABLITY(ACL0_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL1_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL2_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL3_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL4_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL5_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL6_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL7_IGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL0_EGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL1_EGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(ACL2_EGS_TCAM_ENTRY_NUM);
    PRINT_CAPABLITY(CID_PAIR_NUM);
    PRINT_CAPABLITY(UDF_ENTRY_NUM);

    ctc_cli_out("======================================\n");

    return ret;
}

CTC_CLI(ctc_cli_chip_set_global_log_shift,
        ctc_cli_chip_set_global_log_shift_cmd,
        "chip global-cfg (igs-random-log-shift | egs-random-log-shift) value VALUE",
        "Chip module",
        "Global config",
        "Ingress random log shift",
        "Egress random log shift",
        "Value",
        "Value")
{
    int32 ret       = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;


    index = CTC_CLI_GET_ARGC_INDEX("igs-random-log-shift");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egs-random-log-shift");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT;
    }

    CTC_CLI_GET_UINT32_RANGE("value", val, argv[1], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_set(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_log_shift,
        ctc_cli_chip_get_global_log_shift_cmd,
        "show chip global-cfg (igs-random-log-shift | egs-random-log-shift)",
        "Show",
        "Chip module",
        "Global config",
        "Ingress random log shift",
        "Egress random log shift",
        "Value",
        "Value")
{
    int32 ret       = CLI_SUCCESS;
    uint32 val = 0;
    uint8 type = 0;
    uint8 index = 0;
    char string[25] = {0};


    index = CTC_CLI_GET_ARGC_INDEX("igs-random-log-shift");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT;
        sal_strcpy(string, "igs-random-log-shift");
    }

    index = CTC_CLI_GET_ARGC_INDEX("egs-random-log-shift");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT;
        sal_strcpy(string, "egs-random-log-shift");
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Show chip %-20s info:\n", string);
    ctc_cli_out("==================================\n");
    ctc_cli_out("value:%d\n", val);
    ctc_cli_out("==================================\n");

    return ret;

}

CTC_CLI(ctc_cli_chip_set_global_config,
        ctc_cli_chip_set_global_config_cmd,
        "chip global-cfg (pim-snooping-mode | vxlan-udp-dest-port | geneve-udp-dest-port|igmp-snooping-mode|tpoam-vpws-coexist|arp-derive-mode|vxlan-crypt-udp-dest-port|vxlan-policer-gid-base) value VALUE",
        "chip module",
        "Global config",
        "PIM snooping mode",
        "Vxlan udp dest port",
        "Geneve udp dest port",
        "IGMP Snooping mode",
        "TP OAM and VPWS OAM can coexist",
        "ARP derive mode",
        "Vxlan crypt udp dest port",
        "Vxlan policer group id base",
        "Value",
        "VALUE")
{
    int32 ret = CLI_SUCCESS;
    uint8 type = 0;
    uint8 index = 0;
    uint32 val = 0;

    index = CTC_CLI_GET_ARGC_INDEX("pim-snooping-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_PIM_SNOOPING_MODE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_UDP_DEST_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_GENEVE_UDP_DEST_PORT;
    }
    index = CTC_CLI_GET_ARGC_INDEX("igmp-snooping-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_IGMP_SNOOPING_MODE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("tpoam-vpws-coexist");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-derive-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-crypt-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_CRYPT_UDP_DEST_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-policer-gid-base");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_POLICER_GROUP_ID_BASE;
    }

    CTC_CLI_GET_UINT32_RANGE("value", val, argv[1], 0, CTC_MAX_UINT32_VALUE);

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_set(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_global_config,
        ctc_cli_chip_get_global_config_cmd,
        "show chip global-cfg (pim-snooping-mode | vxlan-udp-dest-port | geneve-udp-dest-port|igmp-snooping-mode|tpoam-vpws-coexist|arp-derive-mode|vxlan-crypt-udp-dest-port|vxlan-policer-gid-base)",
        "show",
        "chip module",
        "Global config",
        "PIM snooping mode",
        "Vxlan udp dest port",
        "Geneve udp dest port",
        "IGMP Snooping mode",
        "TP OAM and VPWS OAM can coexist",
        "ARP derive mode",
        "Vxlan crypt udp dest port",
        "Vxlan policer group id base")
{
    int32 ret = CLI_SUCCESS;
    uint8 type = 0;
    uint8 index = 0;
    uint32 val = 0;

    index = CTC_CLI_GET_ARGC_INDEX("pim-snooping-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_PIM_SNOOPING_MODE;
        ctc_cli_out("Show chip global-cfg pim-snooping-mode info\n");
        ctc_cli_out("==============================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_UDP_DEST_PORT;
        ctc_cli_out("Show chip global-cfg vxlan-udp-dest-port info\n");
        ctc_cli_out("==============================\n");
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_GENEVE_UDP_DEST_PORT;
        ctc_cli_out("Show chip global-cfg geneve-udp-dest-port info\n");
        ctc_cli_out("==============================\n");
    }
   index = CTC_CLI_GET_ARGC_INDEX("igmp-snooping-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_IGMP_SNOOPING_MODE;
        ctc_cli_out("Show chip global-cfg igmp-snooping-mode info\n");
        ctc_cli_out("==============================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("tpoam-vpws-coexist");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST;
        ctc_cli_out("Show chip global-cfg tpoam-vpws-coexist info\n");
        ctc_cli_out("==============================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("arp-derive-mode");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE;
        ctc_cli_out("Show chip global-cfg arp-derive-mode info\n");
        ctc_cli_out("==============================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-crypt-udp-dest-port");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_CRYPT_UDP_DEST_PORT;
        ctc_cli_out("Show chip global-cfg vxlan-crypt-udp-dest-port info\n");
        ctc_cli_out("==============================\n");
    }
    index = CTC_CLI_GET_ARGC_INDEX("vxlan-policer-gid-base");
    if (INDEX_VALID(index))
    {
        type = CTC_GLOBAL_VXLAN_POLICER_GROUP_ID_BASE;
        ctc_cli_out("Show chip global-cfg vxlan-policer-gid-base info\n");
        ctc_cli_out("==============================\n");
    }
    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, type, &val);
    }
    else
    {
        ret = ctc_global_ctl_get(type, &val);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("value : %d\n", val);
    ctc_cli_out("==============================\n");

    return ret;

}

CTC_CLI(ctc_cli_chip_set_mdio_clock_frequency,
        ctc_cli_chip_set_mdio_clock_frequency_cmd,
        "chip set mdio (gephy|xgphy) freq FREQ",
        "chip module",
        "set",
        "mdio interface",
        "gephy mdio bus",
        "xgphy mdio bus",
        "mdio clock frequency",
        "frequency value(K)")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_mdio_type_t type;
    uint16 freq = 0;

    if (CLI_CLI_STR_EQUAL("gephy", 0))
    {
        type = CTC_CHIP_MDIO_GE;
    }
    else
    {
        type = CTC_CHIP_MDIO_XG;
    }

    CTC_CLI_GET_UINT16_RANGE("freq", freq, argv[1], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_mdio_clock(g_api_lchip, type, freq);
    }
    else
    {
        ret = ctc_chip_set_mdio_clock(type, freq);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_show_mdio_clock_frequency,
        ctc_cli_chip_show_mdio_clock_frequency_cmd,
        "show chip mdio (gephy|xgphy) freq",
        "show",
        "chip module",
        "mdio interface",
        "gephy mdio bus",
        "xgphy mdio bus",
        "mdio clock frequency",
        "frequency value(K)")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_mdio_type_t type;
    uint16 freq = 0;

    if (CLI_CLI_STR_EQUAL("gephy", 0))
    {
        type = CTC_CHIP_MDIO_GE;
    }
    else
    {
        type = CTC_CHIP_MDIO_XG;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_mdio_clock(g_api_lchip, type, &freq);
    }
    else
    {
        ret = ctc_chip_get_mdio_clock(type, &freq);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Mdio clock frequency is %d Khz \n", freq);

    return ret;

}


CTC_CLI(ctc_cli_chip_show_sensor_value,
        ctc_cli_chip_show_sensor_value_cmd,
        "show chip sensor (temp|voltage) (lchip LOCAL_CHIPID |)",
        "show",
        "chip module",
        "sensor interface",
        "temperature sensor",
        "voltage sensor",
        "local chip",
        "local chip id")
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_sensor_type_t type;
    uint32 value = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    if (CLI_CLI_STR_EQUAL("temp", 0))
    {
        type = CTC_CHIP_SENSOR_TEMP;
    }
    else
    {
        type = CTC_CHIP_SENSOR_VOLTAGE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_get_chip_sensor(g_api_lchip, type, &value);
    }
    else
    {
        ret = ctc_get_chip_sensor(lchip, type, &value);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (CTC_CHIP_SENSOR_TEMP == type)
    {
        if (CTC_IS_BIT_SET(value, 31))
        {
            ctc_cli_out("Temperature is -%d C \n", value&0x7FFFFFFF);
        }
        else
        {
            ctc_cli_out("Temperature is %d C \n", value);
        }
    }
    else
    {
        ctc_cli_out("Voltage is %d mV \n", value);
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_debug_on,
        ctc_cli_chip_debug_on_cmd,
        "debug chip (chip|peri) (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "chip module",
        "chip module",
        "peripheral module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if(0 == sal_memcmp(argv[0], "chip", 4))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = CHIP_CTC;

        }
        else
        {
            typeenum = CHIP_SYS;

        }
        ctc_debug_set_flag("chip", "chip", typeenum, level, TRUE);
    }
    else if(0 == sal_memcmp(argv[0], "peri", 4))
    {
        if (0 == sal_memcmp(argv[1], "ctc", 3))
        {
            typeenum = PERI_CTC;

        }
        else
        {
            typeenum = PERI_SYS;

        }
        ctc_debug_set_flag("chip", "peri", typeenum, level, TRUE);
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_debug_off,
        ctc_cli_chip_debug_off_cmd,
        "no debug chip (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Chip Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = CHIP_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = CHIP_SYS;
    }

    ctc_debug_set_flag("chip", "chip", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_debug,
        ctc_cli_chip_show_debug_cmd,
        "show debug chip (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Chip Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = CHIP_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = CHIP_SYS;
    }
    en = ctc_debug_get_flag("chip", "chip", typeenum, &level);
    ctc_cli_out("chip:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_chip_set_serdes_prbs,
        ctc_cli_chip_set_serdes_prbs_cmd,
        "chip set serdes SERDES (prbs (tx (enable|disable)|rx) mode VALUE) (lchip LCHIP |)",
        "Chip module",
        "Set operate",
        "Serdes",
        "Serdes ID",
        "Pseudo random binary sequence",
        "Transmit prbs pattern",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        "Receive prbs pattern and check",
        "Pattern mode",
        "0-PRBS7_PLUS, 1-PRBS7_SUB, 2-PRBS15_PLUS, 3-PRBS15_SUB, 4-PRBS23_PLUS, 5-PRBS23_SUB, 6-PRBS31_PLUS, 7-PRBS31_SUB",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_prbs_t prbs_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&prbs_param, 0, sizeof(ctc_chip_serdes_prbs_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    prbs_param.serdes_id = serdes_id;

    /* prbs cfg */
    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        prbs_param.value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        prbs_param.value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
        prbs_param.mode = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (0xFF != index)
    {
        prbs_param.mode = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("mode", prbs_param.polynome_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_PRBS, (void*)&prbs_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_PRBS, (void*)&prbs_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
        ctc_cli_out("%d\n",prbs_param.value);
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_param,
        ctc_cli_chip_set_serdes_param_cmd,
        "chip set serdes SERDES ffe mode ((user-define {c0 VALUE|c1 VALUE|c2 VALUE|c3 VALUE|c4 VALUE})| (3ap {c0 VALUE|c1 VALUE|c2 VALUE|c3 VALUE})| \
        (typical (board-type (fr4|m4|m6) (trace-len (4inch|7inch|10inch|8inch|9inch|9+inch)|)))) (lchip LCHIP |)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "ffe parameter",
        "ffe cfg mode",
        "traditional mode, support copper and fiber",
        "coefficient0",
        "coefficient0 VALUE",
        "coefficient1",
        "coefficient1 VALUE",
        "coefficient2",
        "coefficient2 VALUE",
        "coefficient3",
        "coefficient3 VALUE",
        "coefficient4",
        "coefficient4 VALUE",
        "802.3ap mode, only support copper",
        "coefficient0",
        "coefficient0 VALUE",
        "coefficient1",
        "coefficient1 VALUE",
        "coefficient2",
        "coefficient2 VALUE",
        "coefficient3",
        "coefficient3 VALUE",
        "typical mode, cfg ffe by motherboard material and trace length",
        "mother board type",
        "FR4 material",
        "M4 material",
        "M6 material",
        "Trace length",
        "0~4inch",
        "4~7inch",
        "7~10inch",
        "7~8inch, only for FR4",
        "8~9inch, only for FR4",
        ">9inch, only for FR4",
        "Local chip",
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_ffe_t  ffe_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&ffe_param, 0, sizeof(ctc_chip_serdes_ffe_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    ffe_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("user-define");
    if (0xFF != index)
    {
        ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("3ap");
    if (0xFF != index)
    {
        ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_3AP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("typical");
    if (0xFF != index)
    {
        ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("m6");
    if (0xFF != index)
    {
        ffe_param.board_material = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("m4");
    if (0xFF != index)
    {
        ffe_param.board_material = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("fr4");
    if (0xFF != index)
    {
        ffe_param.board_material = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("4inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("7inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("10inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("8inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 2;
    }

    index = CTC_CLI_GET_ARGC_INDEX("9inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 3;
    }

    index = CTC_CLI_GET_ARGC_INDEX("9+inch");
    if (0xFF != index)
    {
        ffe_param.trace_len = 4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("c0");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("c0", ffe_param.coefficient[0], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("c1");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("c1", ffe_param.coefficient[1], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("c2");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("c2", ffe_param.coefficient[2], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("c3");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("c3", ffe_param.coefficient[3], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("c4");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("c4", ffe_param.coefficient[4], argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (index != 0xFF)
    {
        if (index + 1 >= argc)
        {
            ctc_cli_out("%% Insufficient  param \n\r");
            return CLI_ERROR;
        }

        CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);

    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_FFE, (void*)&ffe_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_FFE, (void*)&ffe_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_set_serdes_poly,
        ctc_cli_chip_set_serdes_poly_cmd,
        "chip set serdes SERDES poly (reverse|normal) dir (rx|tx) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "poly parameter",
        "poly reverse",
        "poly normal",
        "direction",
        "recieve dir",
        "transmit dir",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_polarity_t  polarity_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&polarity_param, 0, sizeof(ctc_chip_serdes_polarity_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    polarity_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("reverse");
    if (0xFF != index)
    {
        polarity_param.polarity_mode = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("normal");
    if (0xFF != index)
    {
        polarity_param.polarity_mode = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
        polarity_param.dir = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (0xFF != index)
    {
        polarity_param.dir = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PEOP_SERDES_POLARITY, (void*)&polarity_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PEOP_SERDES_POLARITY, (void*)&polarity_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_loopback,
        ctc_cli_chip_set_serdes_loopback_cmd,
        "chip set serdes SERDES loopback (internal|external) (enable|disable) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "loopback parameter",
        "Internal loopback",
        "External loopback",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_loopback_t  lb_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    lb_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("internal");
    if (0xFF != index)
    {
        lb_param.mode = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("external");
    if (0xFF != index)
    {
        lb_param.mode = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        lb_param.enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        lb_param.enable = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_p_flag,
        ctc_cli_chip_set_serdes_p_flag_cmd,
        "chip set serdes SERDES p-flag (enable|disable) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "P-flag",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_cfg_t  pflag_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&pflag_param, 0, sizeof(ctc_chip_serdes_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    pflag_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        pflag_param.value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        pflag_param.value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_P_FLAG, (void*)&pflag_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_P_FLAG, (void*)&pflag_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_set_serdes_peak,
        ctc_cli_chip_set_serdes_peak_cmd,
        "chip set serdes SERDES peak VALUE (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "Peaking",
        "Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_cfg_t  peak_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&peak_param, 0, sizeof(ctc_chip_serdes_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("peak-value", peak_param.value, argv[1], 0, CTC_MAX_UINT16_VALUE);

    peak_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_PEAK, (void*)&peak_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_PEAK, (void*)&peak_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_dpc,
        ctc_cli_chip_set_serdes_dpc_cmd,
        "chip set serdes SERDES dpc (enable|disable) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "Dynamic Peaking Control",
        "Enable",
        "Disable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_cfg_t  dpc_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&dpc_param, 0, sizeof(ctc_chip_serdes_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    dpc_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        dpc_param.value = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        dpc_param.value = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_DPC, (void*)&dpc_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_DPC, (void*)&dpc_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_slew_rate,
        ctc_cli_chip_set_serdes_slew_rate_cmd,
        "chip set serdes SERDES slew-rate VALUE (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "Slew rate",
        "Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_cfg_t  slew_rate_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&slew_rate_param, 0, sizeof(ctc_chip_serdes_cfg_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("slew rate", slew_rate_param.value, argv[1], 0, CTC_MAX_UINT16_VALUE);

    slew_rate_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_SLEW_RATE, (void*)&slew_rate_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_SLEW_RATE, (void*)&slew_rate_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_set_serdes_clte_rate,
        ctc_cli_chip_set_serdes_ctle_cmd,
        "chip set serdes SERDES ctle (auto-en| VALUE0 VALUE1 VALUE2) (lchip LCHIP|)",
        "chip module",
        "set operate",
        "serdes",
        "serdes id",
        "Ctle paramiter",
        "Auto Enable",
        "Value0",
        "Value1",
        "Value2",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_ctle_t  ctle_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;

    sal_memset(&ctle_param, 0, sizeof(ctc_chip_serdes_ctle_t));

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("auto-en");
    if (0xFF != index)
    {
        ctle_param.auto_en=1;
    }
    else
    {
        CTC_CLI_GET_UINT16_RANGE("ctle", ctle_param.value[0], argv[1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("ctle", ctle_param.value[1], argv[2], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("ctle", ctle_param.value[2], argv[3], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    ctle_param.serdes_id = serdes_id;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_CTLE, (void*)&ctle_param);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_CTLE, (void*)&ctle_param);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}

CTC_CLI(ctc_cli_chip_get_serdes_prop,
        ctc_cli_chip_get_serdes_prop_cmd,
        "show chip serdes SERDES (p-flag |peak |dpc |slew-rate | ffe (typical| user-define| 3ap)| loopback | polarity | ctle) (lchip LCHIP|)",
        "Show operate",
        "Chip module",
        "Serdes",
        "Serdes id",
        "P-flag",
        "Peaking",
        "Dynamic Peaking Control",
        "Slew rate",
        "FFE parameter",
        "typical mode",
        "user define mode",
        "802.3ap mode",
        "Loopback",
        "Polarity",
        "Ctle",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret       = CLI_SUCCESS;
    ctc_chip_serdes_cfg_t  cfg_param;
    ctc_chip_serdes_ffe_t ffe_param;
    ctc_chip_serdes_polarity_t pola_param;
    ctc_chip_serdes_loopback_t lb_param;
    ctc_chip_serdes_ctle_t ctle_param;
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    uint8 type = 0;
    uint8 temp = 0;
    uint8 i = 0;

    CTC_CLI_GET_UINT8_RANGE("serdes", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);

    ctc_cli_out("Show Serdes                        :  0x%x\n", serdes_id);
    ctc_cli_out("-------------------------------------------\n");

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("p-flag");
    if (0xFF != index)
    {
        sal_memset(&cfg_param, 0, sizeof(ctc_chip_serdes_cfg_t));
        type = CTC_CHIP_PROP_SERDES_P_FLAG;
        cfg_param.serdes_id = serdes_id;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&cfg_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&cfg_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "P-flag", cfg_param.value);
    }


    index = CTC_CLI_GET_ARGC_INDEX("peak");
    if (0xFF != index)
    {
        sal_memset(&cfg_param, 0, sizeof(ctc_chip_serdes_cfg_t));
        type = CTC_CHIP_PROP_SERDES_PEAK;
        cfg_param.serdes_id = serdes_id;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&cfg_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&cfg_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Peak", cfg_param.value);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dpc");
    if (0xFF != index)
    {
        sal_memset(&cfg_param, 0, sizeof(ctc_chip_serdes_cfg_t));
        type = CTC_CHIP_PROP_SERDES_DPC;
        cfg_param.serdes_id = serdes_id;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&cfg_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&cfg_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "DPC", cfg_param.value);
    }

    index = CTC_CLI_GET_ARGC_INDEX("slew-rate");
    if (0xFF != index)
    {
        sal_memset(&cfg_param, 0, sizeof(ctc_chip_serdes_cfg_t));
        type = CTC_CHIP_PROP_SERDES_SLEW_RATE;
        cfg_param.serdes_id = serdes_id;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&cfg_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&cfg_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Slew rate value", cfg_param.value);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ffe");
    if (0xFF != index)
    {
        sal_memset(&ffe_param, 0, sizeof(ctc_chip_serdes_ffe_t));
        type = CTC_CHIP_PROP_SERDES_FFE;
        ffe_param.serdes_id = serdes_id;

        index = CTC_CLI_GET_ARGC_INDEX("user-define");
        if (0xFF != index)
        {
            ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_DEFINE;
        }

        index = CTC_CLI_GET_ARGC_INDEX("3ap");
        if (0xFF != index)
        {
            ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_3AP;
        }

        index = CTC_CLI_GET_ARGC_INDEX("typical");
        if (0xFF != index)
        {
            ffe_param.mode = CTC_CHIP_SERDES_FFE_MODE_TYPICAL;
        }

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&ffe_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&ffe_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        for (i = 0; i < CTC_CHIP_FFE_PARAM_NUM; i++)
        {
            ctc_cli_out("%s%-34d:  %d\n", "FFE C",i, ffe_param.coefficient[i]);
        }
        ctc_cli_out("%-39s:  %s\n", "FFE Status",ffe_param.status?"active":"inactive");

    }

    index = CTC_CLI_GET_ARGC_INDEX("loopback");
    if (0xFF != index)
    {
        sal_memset(&lb_param, 0, sizeof(ctc_chip_serdes_loopback_t));
        type = CTC_CHIP_PROP_SERDES_LOOPBACK;
        lb_param.serdes_id = serdes_id;
        lb_param.mode = 0;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&lb_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&lb_param);
        }
        if (ret < 0)
        {
            temp = 0xff;
        }
        else
        {
            temp = lb_param.enable;
        }

        lb_param.mode = 1;
        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&lb_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&lb_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %c\n", "Internal Loopback", (0xff==temp)?'-':(temp+'0'));
        ctc_cli_out("%-34s:  %u\n", "External Loopback", lb_param.enable);
    }

    index = CTC_CLI_GET_ARGC_INDEX("polarity");
    if (0xFF != index)
    {
        sal_memset(&pola_param, 0, sizeof(ctc_chip_serdes_polarity_t));
        type = CTC_CHIP_PEOP_SERDES_POLARITY;
        pola_param.serdes_id = serdes_id;
        pola_param.dir = 0;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&pola_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&pola_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        temp = pola_param.polarity_mode;

        pola_param.dir = 1;
        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&pola_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&pola_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Rx Polarity", temp);
        ctc_cli_out("%-34s:  %u\n", "Tx Polarity", pola_param.polarity_mode);

    }

    index = CTC_CLI_GET_ARGC_INDEX("ctle");
    if (0xFF != index)
    {
        sal_memset(&ctle_param, 0, sizeof(ctc_chip_serdes_ctle_t));
        type = CTC_CHIP_PROP_SERDES_CTLE;
        ctle_param.serdes_id = serdes_id;

        if(g_ctcs_api_en)
        {
             ret = ctcs_chip_get_property(g_api_lchip, type, (void*)&ctle_param);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, (void*)&ctle_param);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Value0", ctle_param.value[0]);
        ctc_cli_out("%-34s:  %u\n", "Value1", ctle_param.value[1]);
        ctc_cli_out("%-34s:  %u\n", "Value2", ctle_param.value[2]);
    }

    return ret;

}

CTC_CLI(ctc_cli_datapath_debug_on,
        ctc_cli_datapath_debug_on_cmd,
        "debug datapath (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "datapath module",
        "CTC layer",
        "Sys layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO | CTC_DEBUG_LEVEL_FUNC | CTC_DEBUG_LEVEL_PARAM | CTC_DEBUG_LEVEL_ERROR;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DATAPATH_CTC;

    }
    else
    {
        typeenum = DATAPATH_SYS;

    }

    ctc_debug_set_flag("datapath", "datapath", typeenum, level, TRUE);
    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_datapath_debug_off,
        ctc_cli_datapath_debug_off_cmd,
        "no debug datapath (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Datapath Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DATAPATH_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DATAPATH_SYS;
    }

    ctc_debug_set_flag("datapath", "datapath", typeenum, level, FALSE);

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_datapath_show_debug,
        ctc_cli_datapath_show_debug_cmd,
        "show debug datapath (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Datapath Module",
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DATAPATH_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DATAPATH_SYS;
    }

    en = ctc_debug_get_flag("datapath", "datapath", typeenum, &level);
    ctc_cli_out("chip:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_mdio_read,
            ctc_cli_chip_mdio_read_cmd,
            "chip read mdio lchip ID (ge | xg (dev-addr ADDR)) (control-id ID |) (bus-id ID \
            phy-id ID reg-addr ADDR)",
            "chip module",
            "read operate",
            "mdio bus",
            "local chip id",
            "ID",
            "1GPHY",
            "XGPHY",
            "dev-addr",
            "ADDR",
            "control id",
            "<0 - 1>",
            "bus id",
            "MDIO bus ID",
            "phy number",
            "ID",
            "register address",
            "ADDR")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_chip_mdio_type_t type = CTC_CHIP_MDIO_GE;
    ctc_chip_mdio_para_t mdio_para = {0};


    CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("ge");
    if (0xFF != index)
    {
        type = CTC_CHIP_MDIO_GE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xg");
    if (0xFF != index)
    {
        type = CTC_CHIP_MDIO_XG;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("control-id", mdio_para.ctl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bus-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("bus-id", mdio_para.bus, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("phy-id", mdio_para.phy_addr, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("reg-addr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("reg-addr", mdio_para.reg, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dev-addr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("dev-addr", mdio_para.dev_no, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_mdio_read(g_api_lchip, type,  &mdio_para);
    }
    else
    {
        ret = ctc_chip_mdio_read(lchip, type,  &mdio_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if(CTC_CHIP_MDIO_XG == type)
        ctc_cli_out("PHY addr: 0x%x\tdevno: 0x%x\treg: 0x%x\tvalue: 0x%x\n", mdio_para.phy_addr, mdio_para.dev_no,mdio_para.reg,mdio_para.value);
    else if(CTC_CHIP_MDIO_GE == type)
        ctc_cli_out("PHY addr: 0x%x\treg: 0x%x\tvalue: 0x%x\n", mdio_para.phy_addr,mdio_para.reg, mdio_para.value);

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_chip_mdio_write,
            ctc_cli_chip_mdio_write_cmd,
            "chip write mdio lchip ID (ge | xg (dev-addr ADDR)) (control-id ID |) (bus-id ID \
            phy-id ID reg-addr ADDR value VAL)",
            "chip module",
            "read operate",
            "mdio bus",
            "local chip id",
            "ID",
            "1GPHY",
            "XGPHY",
            "dev-addr",
            "ADDR",
            "control id",
            "<0 - 1>",
            "bus id",
            "<0 - 1>",
            "phy number",
            "ID",
            "register address",
            "ADDR",
            "value",
            "VALUE")
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 index = 0xFF;
    ctc_chip_mdio_type_t type = CTC_CHIP_MDIO_GE;
    ctc_chip_mdio_para_t mdio_para = {0};


    CTC_CLI_GET_UINT8_RANGE("lchip", lchip, argv[0], 0, CTC_MAX_UINT8_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("ge");
    if (0xFF != index)
    {
        type = CTC_CHIP_MDIO_GE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("xg");
    if (0xFF != index)
    {
        type = CTC_CHIP_MDIO_XG;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("control-id", mdio_para.ctl_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("bus-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("bus-id", mdio_para.bus, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("phy-id", mdio_para.phy_addr, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("reg-addr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("reg-addr", mdio_para.reg, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("dev-addr");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("dev-addr", mdio_para.dev_no, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("value");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("value", mdio_para.value, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_mdio_write(g_api_lchip, type,  &mdio_para);
    }
    else
    {
        ret = ctc_chip_mdio_write(lchip, type,  &mdio_para);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_device_info,
        ctc_cli_chip_show_device_info_cmd,
        "show chip device information  (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Device",
        "Information",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    ctc_chip_device_info_t device_info;
    uint8 lchip = 0;
    uint8 index = 0;
    int32 ret = 0;
    sal_memset(&device_info, 0, sizeof(device_info));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if(0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("lchip id", lchip, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_property(g_api_lchip, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
    }
    else
    {
        ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_DEVICE_INFO, (void*)&device_info);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("Chip name: %s, Device Id: %d, Version Id: %d \n", device_info.chip_name, device_info.device_id, device_info.version_id);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_gport_property_prop,
        ctc_cli_chip_show_gport_property_prop_cmd,
        "show chip property serdes-to-gport SERDES",
        "Show operate",
        "Chip module",
        "property",
        "Serdes id to gport",
        "Serdes Id Value")
{
    ctc_port_serdes_info_t serdes_port;
    uint8 lchip = 0;
    int32 ret = 0;
    char* serdes_mode_name[CTC_CHIP_MAX_SERDES_MODE] = {"NONE", "XFI", "SGMII", "XSGMII", "QSGMII", "XAUI", "DXAUI", "XLG",\
            "CG", "2DOT5G", "USXGMII0", "USXGMII1", "USXGMII2", "XXVG", "LG"};

    sal_memset(&serdes_port, 0, sizeof(ctc_port_serdes_info_t));

    CTC_CLI_GET_UINT8_RANGE("serdes-to-gport", serdes_port.serdes_id,  argv[0], 0, CTC_MAX_UINT8_VALUE);

    ctc_cli_out("Show Serdes                        :  0x%04x\n", serdes_port.serdes_id);
    ctc_cli_out("-------------------------------------------\n");

    if(g_ctcs_api_en)
    {
         ret = ctcs_chip_get_property(g_api_lchip, CTC_CHIP_PROP_SERDES_ID_TO_GPORT, (void*)&serdes_port);
    }
    else
    {
        ret = ctc_chip_get_property(lchip, CTC_CHIP_PROP_SERDES_ID_TO_GPORT, (void*)&serdes_port);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("%-34s:  0x%04x\n", "Gport", serdes_port.gport);
    ctc_cli_out("%-34s:  %s\n", "serdes mode", serdes_mode_name[serdes_port.serdes_mode]);
    ctc_cli_out("%-34s:  %u\n", "overclocking", serdes_port.overclocking_speed);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_global_panel_ports,
        ctc_cli_chip_show_global_panel_ports_cmd,
        "show chip global-cfg panel-ports (lchip LCHIP|)",
        "Show",
        "Chip module",
        "Global config",
        "Panel ports",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 loop = 0;
    uint8 lchip = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 index = 0;
    ctc_global_panel_ports_t panel_ports;

    sal_memset(&panel_ports, 0, sizeof(panel_ports));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    panel_ports.lchip = lchip;

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_PANEL_PORTS, &panel_ports);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_PANEL_PORTS, &panel_ports);
    }

    if (ret >= 0 )
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
        }
        else
        {
            ret = ctc_get_gchip_id(lchip, &gchip);
        }
        ctc_cli_out("%-34s:  %u\n", "Phy Ports", panel_ports.count);
        for (loop = 0; loop < panel_ports.count; loop++ )
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, panel_ports.lport[loop]);
            ctc_cli_out("%-2d:  0x%x\n", loop, gport);
        }
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}


CTC_CLI(ctc_cli_chip_mem_bist,
        ctc_cli_chip_mem_bist_cmd,
        "chip mem-bist enable (lchip LCHIP|)",
        "Chip module",
        "Memory bist",
        "Enable",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret    = CLI_SUCCESS;
    uint8  lchip = 0;
    uint8  index = 0;
    ctc_chip_mem_bist_t mem_bist;

    sal_memset(&mem_bist, 0, sizeof(ctc_chip_mem_bist_t));
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    if (g_ctcs_api_en)
    {
        ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_MEM_BIST, (void*)&mem_bist);
    }
    else
    {
        ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_MEM_BIST, (void*)&mem_bist);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (mem_bist.status == 0)
    {
        ctc_cli_out("Memory BIST PASS\n");
    }
    else
    {
        ctc_cli_out("Memory BIST FAIL\n");
    }

    return ret;

}

#define CTC_CLI_CHIP_MON_TEMPERATURE 0x01
#define CTC_CLI_CHIP_MON_VOLTAGE     0x02
#define CTC_CLI_CHIP_MON_MAC_ERR     0x04

#define CTC_CLI_CHIP_MON_MAX_HIS     128
#define CTC_CLI_CHIP_MON_CYCLE       1*60*1000

enum ctc_cli_chip_mon_event_e
{
    CTC_CLI_CHIP_MON_EVT_NONE,
    CTC_CLI_CHIP_MON_EVT_TEMPERATURE_HIGH,
    CTC_CLI_CHIP_MON_EVT_TEMPERATURE_LOW,
    CTC_CLI_CHIP_MON_EVT_TEMPERATURE_BACK_TO_NORMAL,
    CTC_CLI_CHIP_MON_EVT_VOLT_HIGH,
    CTC_CLI_CHIP_MON_EVT_VOLT_LOW,
    CTC_CLI_CHIP_MON_EVT_VOLT_BACK_TO_NORMAL,
    CTC_CLI_CHIP_MON_EVT_RX_FCS_ERR,
    CTC_CLI_CHIP_MON_EVT_RX_MAC_OVERRUN,
    CTC_CLI_CHIP_MON_EVT_RX_BAD_63,
    CTC_CLI_CHIP_MON_EVT_RX_BAD_JUMBO,
    CTC_CLI_CHIP_MON_EVT_TX_FCS_ERR,
    CTC_CLI_CHIP_MON_EVT_TX_MAC_UNDERRUN,
    CTC_CLI_CHIP_MON_EVT_MAX
};
typedef enum ctc_cli_chip_mon_event_e ctc_cli_chip_mon_event_t;

struct ctc_cli_chip_monitor_record_s
{
    uint8  valid;
    int32  temprature;
    int32  voltage;
    ctc_mac_stats_t *mac_rx;
    ctc_mac_stats_t *mac_tx;
};
typedef struct ctc_cli_chip_monitor_record_s ctc_cli_chip_monitor_record_t;

struct ctc_cli_chip_monitor_event_s
{
    uint8 evt;
    sal_time_t timestamp;
    uint32 gport;
    union
    {
        int32 temp_value;
        int32 volt_value;
        uint64 stats_value;
    }from;
    union
    {
        int32 temp_value;
        int32 volt_value;
        uint64 stats_value;
    }to;
};
typedef struct ctc_cli_chip_monitor_event_s ctc_cli_chip_monitor_event_t;

struct ctc_chip_monitor_s
{
    uint8  task_bmp;
    uint32 cycle;
    int32  temprature_high;
    int32  temprature_low;
    int32  voltage_high;
    int32  voltage_low;
    sal_task_t* task;
    sal_mutex_t* mutex;
    ctc_cli_chip_monitor_record_t last_record;
    ctc_cli_chip_monitor_event_t evt[CTC_CLI_CHIP_MON_MAX_HIS];
    uint16 cur_evt_idx;
};
typedef struct ctc_chip_monitor_s ctc_chip_monitor_t;

ctc_chip_monitor_t* g_chip_monitor_master[CTC_MAX_LOCAL_CHIP_NUM];

#define CTC_CLI_CHIP_MON_CHECK_INIT(_lchip_)   \
    do { \
        if (_lchip_ >= CTC_MAX_LOCAL_CHIP_NUM)\
        {\
            return CLI_ERROR;\
        }\
        if (!g_chip_monitor_master[_lchip_])  \
        {  \
            uint32 _dev_id_ = 0;  \
            uint32 _capability_[CTC_GLOBAL_CAPABILITY_MAX] = {0};  \
            uint32 _max_port_num_per_chip_ = 0;  \
            if (_lchip_ >= CTC_MAX_LOCAL_CHIP_NUM) \
            { \
                return CLI_ERROR; \
            } \
            g_chip_monitor_master[_lchip_] = (ctc_chip_monitor_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_chip_monitor_t)); \
            if (!g_chip_monitor_master[_lchip_]) \
            {  \
                ctc_cli_out("%% ret = %d, %s \n", CTC_E_NO_MEMORY, ctc_get_error_desc(CTC_E_NO_MEMORY));  \
                return CLI_ERROR;  \
            }  \
            sal_memset(g_chip_monitor_master[_lchip_], 0, sizeof(ctc_chip_monitor_t));  \
            if(g_ctcs_api_en)  \
            {  \
                ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, _capability_);  \
            }  \
            else  \
            {  \
                ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, _capability_);  \
            }  \
            _max_port_num_per_chip_ = _capability_[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];  \
            g_chip_monitor_master[_lchip_]->last_record.mac_rx = (ctc_mac_stats_t*)mem_malloc(MEM_CLI_MODULE, _max_port_num_per_chip_*sizeof(ctc_mac_stats_t));  \
            if (!g_chip_monitor_master[_lchip_]->last_record.mac_rx) \
            {  \
                mem_free(g_chip_monitor_master[_lchip_]);  \
                g_chip_monitor_master[_lchip_] = NULL; \
                ctc_cli_out("%% ret = %d, %s \n", CTC_E_NO_MEMORY, ctc_get_error_desc(CTC_E_NO_MEMORY));  \
                return CLI_ERROR;  \
            }  \
            sal_memset(g_chip_monitor_master[_lchip_]->last_record.mac_rx, 0, _max_port_num_per_chip_*sizeof(ctc_mac_stats_t));  \
            g_chip_monitor_master[_lchip_]->last_record.mac_tx = (ctc_mac_stats_t*)mem_malloc(MEM_CLI_MODULE, _max_port_num_per_chip_*sizeof(ctc_mac_stats_t));  \
            if (!g_chip_monitor_master[_lchip_]->last_record.mac_tx) \
            {  \
                mem_free(g_chip_monitor_master[_lchip_]->last_record.mac_rx);  \
                mem_free(g_chip_monitor_master[_lchip_]);  \
                g_chip_monitor_master[_lchip_] = NULL; \
                ctc_cli_out("%% ret = %d, %s \n", CTC_E_NO_MEMORY, ctc_get_error_desc(CTC_E_NO_MEMORY));  \
                return CLI_ERROR;  \
            }  \
            sal_memset(g_chip_monitor_master[_lchip_]->last_record.mac_tx, 0, _max_port_num_per_chip_*sizeof(ctc_mac_stats_t));  \
            dal_get_chip_dev_id(_lchip_, &_dev_id_);  \
            g_chip_monitor_master[_lchip_]->cycle = CTC_CLI_CHIP_MON_CYCLE;  \
            switch(_dev_id_)  \
            {  \
                case DAL_HUMBER_DEVICE_ID:  \
                    g_chip_monitor_master[_lchip_]->temprature_high = 125;  \
                    g_chip_monitor_master[_lchip_]->temprature_low = -55;  \
                    g_chip_monitor_master[_lchip_]->voltage_high = 1100;  \
                    g_chip_monitor_master[_lchip_]->voltage_low = -500;  \
                    break;  \
                case DAL_GREATBELT_DEVICE_ID:  \
                    g_chip_monitor_master[_lchip_]->temprature_high = 125;  \
                    g_chip_monitor_master[_lchip_]->temprature_low = -55;  \
                    g_chip_monitor_master[_lchip_]->voltage_high = 1100;  \
                    g_chip_monitor_master[_lchip_]->voltage_low = -400;  \
                    break;  \
                case DAL_GOLDENGATE_DEVICE_ID:  \
                case DAL_GOLDENGATE_DEVICE_ID1: \
                    g_chip_monitor_master[_lchip_]->temprature_high = 125;  \
                    g_chip_monitor_master[_lchip_]->temprature_low = -55;  \
                    g_chip_monitor_master[_lchip_]->voltage_high = 1100;  \
                    g_chip_monitor_master[_lchip_]->voltage_low = -400;  \
                    break;  \
                case DAL_DUET2_DEVICE_ID:  \
                    g_chip_monitor_master[_lchip_]->temprature_high = 125;  \
                    g_chip_monitor_master[_lchip_]->temprature_low = -55;  \
                    g_chip_monitor_master[_lchip_]->voltage_high = 1100;  \
                    g_chip_monitor_master[_lchip_]->voltage_low = -400;  \
                    break;  \
                case DAL_TSINGMA_DEVICE_ID:  \
                    g_chip_monitor_master[_lchip_]->temprature_high = 125;  \
                    g_chip_monitor_master[_lchip_]->temprature_low = -55;  \
                    g_chip_monitor_master[_lchip_]->voltage_high = 1100;  \
                    g_chip_monitor_master[_lchip_]->voltage_low = -400;  \
                    break;  \
                default:  \
                    return CLI_ERROR;  \
                    break;  \
            }  \
            sal_mutex_create(&(g_chip_monitor_master[_lchip_]->mutex));  \
        }  \
    } while (0)

#define CTC_CLI_CHIP_MON_LOCK()   \
    do { \
        if (g_chip_monitor_master[lchip]->mutex) sal_mutex_lock(g_chip_monitor_master[lchip]->mutex); \
    } while (0)

#define CTC_CLI_CHIP_MON_UNLOCK() \
    do { \
        if (g_chip_monitor_master[lchip]->mutex) sal_mutex_unlock(g_chip_monitor_master[lchip]->mutex); \
    } while (0)

void
ctc_cli_chip_mon_task(void* user_param)
{
    int32 ret = 0;
    uint32 value = 0;
    int32 temp = 0;
    int32 volt = 0;
    uintptr* argv = (uintptr*)(user_param);
    uint8 lchip = (uint8)(argv[0]);
    sal_task_t* task = NULL;
    uint8 gchip = 0;
    uint32 max_port_num_per_chip = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint32 lport = 0;
    uint32 gport = 0;
    ctc_mac_stats_t stats;
    ctc_stats_mac_rec_t* last_rx = NULL;
    ctc_stats_mac_rec_t* cur_rx = NULL;
    ctc_stats_mac_snd_t* last_tx = NULL;
    ctc_stats_mac_snd_t* cur_tx = NULL;
    ctc_cli_chip_monitor_event_t* evt = NULL;

    if(g_ctcs_api_en)
    {
        ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];
    ctc_get_gchip_id(lchip, &gchip);

    while(TRUE)
    {
        CTC_CLI_CHIP_MON_LOCK();

        if (!g_chip_monitor_master[lchip]->task_bmp)
        {
            CTC_CLI_CHIP_MON_UNLOCK();
            break;
        }

        if (CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_TEMPERATURE))
        {
            ret = ctc_get_chip_sensor(lchip, CTC_CHIP_SENSOR_TEMP, &value);
            if (!ret)
            {
                if (value & 0x80000000)
                {
                    temp = 0-(int32)(value&0x7FFFFFFF);
                }
                else
                {
                    temp = value;
                }
                if (temp >= g_chip_monitor_master[lchip]->temprature_high)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_TEMPERATURE_HIGH;
                    sal_time(&(evt->timestamp));
                    evt->from.temp_value = g_chip_monitor_master[lchip]->last_record.temprature;
                    evt->to.temp_value = temp;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                else if (temp <= g_chip_monitor_master[lchip]->temprature_low)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_TEMPERATURE_LOW;
                    sal_time(&(evt->timestamp));
                    evt->from.temp_value = g_chip_monitor_master[lchip]->last_record.temprature;
                    evt->to.temp_value = temp;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                else if ((g_chip_monitor_master[lchip]->last_record.valid & CTC_CLI_CHIP_MON_TEMPERATURE) &&
                         ((g_chip_monitor_master[lchip]->last_record.temprature >= g_chip_monitor_master[lchip]->temprature_high) ||
                         (g_chip_monitor_master[lchip]->last_record.temprature <= g_chip_monitor_master[lchip]->temprature_low)))
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_TEMPERATURE_BACK_TO_NORMAL;
                    sal_time(&(evt->timestamp));
                    evt->from.temp_value = g_chip_monitor_master[lchip]->last_record.temprature;
                    evt->to.temp_value = temp;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                g_chip_monitor_master[lchip]->last_record.valid |= CTC_CLI_CHIP_MON_TEMPERATURE;
                g_chip_monitor_master[lchip]->last_record.temprature = temp;
            }
        }

        if (CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_VOLTAGE))
        {
            ret = ctc_get_chip_sensor(lchip, CTC_CHIP_SENSOR_VOLTAGE, &value);
            if (!ret)
            {
                volt = value;
                if (volt >= g_chip_monitor_master[lchip]->voltage_high)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_VOLT_HIGH;
                    sal_time(&(evt->timestamp));
                    evt->from.volt_value = g_chip_monitor_master[lchip]->last_record.voltage;
                    evt->to.volt_value = volt;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                else if (volt <= g_chip_monitor_master[lchip]->voltage_low)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_VOLT_LOW;
                    sal_time(&(evt->timestamp));
                    evt->from.volt_value = g_chip_monitor_master[lchip]->last_record.voltage;
                    evt->to.volt_value = volt;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                else if ((g_chip_monitor_master[lchip]->last_record.valid & CTC_CLI_CHIP_MON_VOLTAGE) &&
                         ((g_chip_monitor_master[lchip]->last_record.voltage >= g_chip_monitor_master[lchip]->voltage_high) ||
                         (g_chip_monitor_master[lchip]->last_record.voltage <= g_chip_monitor_master[lchip]->voltage_low)))
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_VOLT_BACK_TO_NORMAL;
                    sal_time(&(evt->timestamp));
                    evt->from.volt_value = g_chip_monitor_master[lchip]->last_record.voltage;
                    evt->to.volt_value = volt;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                g_chip_monitor_master[lchip]->last_record.valid |= CTC_CLI_CHIP_MON_VOLTAGE;
                g_chip_monitor_master[lchip]->last_record.voltage = volt;
            }
        }

        if (CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_MAC_ERR))
        {
            for (lport=0; lport<max_port_num_per_chip; lport++)
            {
                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);
                sal_memset(&stats, 0, sizeof(stats));
                stats.stats_mode = CTC_STATS_MODE_DETAIL;

                if(g_ctcs_api_en)
                {
                    ret = ctcs_stats_get_mac_stats(g_api_lchip, gport, CTC_STATS_MAC_STATS_RX, &stats);
                }
                else
                {
                    ret = ctc_stats_get_mac_stats(gport, CTC_STATS_MAC_STATS_RX, &stats);
                }
                if (ret < 0)
                {
                    continue;
                }

                last_rx = &(g_chip_monitor_master[lchip]->last_record.mac_rx[lport].u.stats_detail.stats.rx_stats);
                cur_rx = &(stats.u.stats_detail.stats.rx_stats);
                if (cur_rx->fcs_error_pkts != last_rx->fcs_error_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_RX_FCS_ERR;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_rx->fcs_error_pkts;
                    evt->to.stats_value = cur_rx->fcs_error_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                if (cur_rx->mac_overrun_pkts != last_rx->mac_overrun_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_RX_MAC_OVERRUN;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_rx->mac_overrun_pkts;
                    evt->to.stats_value = cur_rx->mac_overrun_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                if (cur_rx->bad_63_pkts != last_rx->bad_63_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_RX_BAD_63;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_rx->bad_63_pkts;
                    evt->to.stats_value = cur_rx->bad_63_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                if (cur_rx->bad_jumbo_pkts != last_rx->bad_jumbo_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_RX_BAD_JUMBO;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_rx->bad_jumbo_pkts;
                    evt->to.stats_value = cur_rx->bad_jumbo_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }

                sal_memcpy(&(g_chip_monitor_master[lchip]->last_record.mac_rx[lport]), &stats, sizeof(stats));

                sal_memset(&stats, 0, sizeof(stats));
                stats.stats_mode = CTC_STATS_MODE_DETAIL;
                if(g_ctcs_api_en)
                {
                    ret = ctcs_stats_get_mac_stats(g_api_lchip, gport, CTC_STATS_MAC_STATS_TX, &stats);
                }
                else
                {
                    ret = ctc_stats_get_mac_stats(gport, CTC_STATS_MAC_STATS_TX, &stats);
                }
                if (ret < 0)
                {
                    continue;
                }

                last_tx = &(g_chip_monitor_master[lchip]->last_record.mac_tx[lport].u.stats_detail.stats.tx_stats);
                cur_tx = &(stats.u.stats_detail.stats.tx_stats);
                if (cur_tx->fcs_error_pkts != last_tx->fcs_error_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_TX_FCS_ERR;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_tx->fcs_error_pkts;
                    evt->to.stats_value = cur_tx->fcs_error_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }
                if (cur_tx->mac_underrun_pkts != last_tx->mac_underrun_pkts)
                {
                    evt = &(g_chip_monitor_master[lchip]->evt[g_chip_monitor_master[lchip]->cur_evt_idx++]);
                    evt->evt = CTC_CLI_CHIP_MON_EVT_TX_MAC_UNDERRUN;
                    evt->gport = gport;
                    sal_time(&(evt->timestamp));
                    evt->from.stats_value = last_tx->mac_underrun_pkts;
                    evt->to.stats_value = cur_tx->mac_underrun_pkts;
                    g_chip_monitor_master[lchip]->cur_evt_idx =
                        ((g_chip_monitor_master[lchip]->cur_evt_idx<CTC_CLI_CHIP_MON_MAX_HIS) ? g_chip_monitor_master[lchip]->cur_evt_idx: 0);
                }

                sal_memcpy(&(g_chip_monitor_master[lchip]->last_record.mac_tx[lport]), &stats, sizeof(stats));
            }

            g_chip_monitor_master[lchip]->last_record.valid |= CTC_CLI_CHIP_MON_MAC_ERR;
        }

        CTC_CLI_CHIP_MON_UNLOCK();
        sal_task_sleep(g_chip_monitor_master[lchip]->cycle);
    }

    task = *(sal_task_t**)(argv[1]);
    if (task)
    {
        sal_task_destroy(task);
        *(sal_task_t**)(argv[1]) = 0;
    }
    mem_free(user_param);
}

CTC_CLI(ctc_cli_chip_monitor,
        ctc_cli_chip_monitor_cmd,
        "chip monitor (((temprature|voltage) (threshold high VALUE low VALUE|))|mac-error) (start|stop) (lchip CHIP_ID|)",
        "Chip module",
        "Chip monitor",
        "Temperature",
        "Voltage",
        "Threshold",
        "Threshold high",
        "Threshold high value",
        "Threshold low",
        "Threshold low value",
        "mac stats error",
        "Start monitor",
        "Stop monitor",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint8 index = 0;
    int32 high = 0;
    int32 low = 0;
    uint8 task = 0;
    uint8 have_thres = 0;
    uintptr* task_argv = NULL;
    char buffer[SAL_TASK_MAX_NAME_LEN]={0};

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_get_gchip_id(lchip, &gchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_CHECK_INIT(lchip);

    index = CTC_CLI_GET_ARGC_INDEX("temprature");
    if (0xFF != index)
    {
        task = CTC_CLI_CHIP_MON_TEMPERATURE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("voltage");
    if (0xFF != index)
    {
        task = CTC_CLI_CHIP_MON_VOLTAGE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac-error");
    if (0xFF != index)
    {
        task = CTC_CLI_CHIP_MON_MAC_ERR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("high");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER_RANGE("threshold high", high, argv[index+1], -2147483647, 2147483647);
        have_thres = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("low");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER_RANGE("threshold low", low, argv[index+1], -2147483647, 2147483647);
        have_thres = 1;
    }

    if (have_thres && (high <= low))
    {
        ctc_cli_out("%% Invalid threshold value \n");
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_LOCK();

    if (have_thres)
    {
        if (task == CTC_CLI_CHIP_MON_TEMPERATURE)
        {
            g_chip_monitor_master[lchip]->temprature_high = high;
            g_chip_monitor_master[lchip]->temprature_low = low;
        }
        if (task == CTC_CLI_CHIP_MON_VOLTAGE)
        {
            g_chip_monitor_master[lchip]->voltage_high = high;
            g_chip_monitor_master[lchip]->voltage_low = low;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("start");
    if (0xFF != index)
    {
        if (g_chip_monitor_master[lchip]->task_bmp)
        {
            CTC_SET_FLAG(g_chip_monitor_master[lchip]->task_bmp, task);
        }
        else
        {
            CTC_SET_FLAG(g_chip_monitor_master[lchip]->task_bmp, task);
            if (!g_chip_monitor_master[lchip]->task)
            {
                task_argv = (uintptr*)mem_malloc(MEM_CLI_MODULE, sizeof(uintptr)*2);
                if(NULL == task_argv)
                {
                    return CLI_ERROR;
                }

                task_argv[0] = lchip;
                task_argv[1] = (uintptr)(&(g_chip_monitor_master[lchip]->task));
                sal_sprintf(buffer, "system monitor-%d", lchip);
                if (0 != sal_task_create(&(g_chip_monitor_master[lchip]->task),
                                         buffer, 0, SAL_TASK_PRIO_DEF, ctc_cli_chip_mon_task, (void*)(task_argv)))
                {
                    sal_task_destroy(g_chip_monitor_master[lchip]->task);
                    g_chip_monitor_master[lchip]->task = NULL;
                    ctc_cli_out("%% Start monitor task failed \n");
                    CTC_CLI_CHIP_MON_UNLOCK();
                    return CLI_ERROR;
                }
            }
        }
    }
    else
    {
        if (CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, task))
        {
            CTC_UNSET_FLAG(g_chip_monitor_master[lchip]->task_bmp, task);
            CTC_UNSET_FLAG(g_chip_monitor_master[lchip]->last_record.valid, task);
        }
    }

    CTC_CLI_CHIP_MON_UNLOCK();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_monitor_interval,
        ctc_cli_chip_monitor_interval_cmd,
        "chip monitor interval VALUE (lchip CHIP_ID|)",
        "Chip module",
        "Chip monitor",
        "Monitor interval",
        "Interval time value in second, default is 60s",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint8 index = 0;
    uint32 cycle = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_get_gchip_id(lchip, &gchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_CHECK_INIT(lchip);

    CTC_CLI_GET_UINT32_RANGE("cycle", cycle, argv[0], 10, 600);

    CTC_CLI_CHIP_MON_LOCK();

    g_chip_monitor_master[lchip]->cycle = cycle*1000;

    CTC_CLI_CHIP_MON_UNLOCK();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_chip_monitor_status,
        ctc_cli_chip_show_chip_monitor_status_cmd,
        "show chip monitor status (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Chip monitor",
        "Chip monitor status",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_get_gchip_id(lchip, &gchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_CHECK_INIT(lchip);

    ctc_cli_out("%-15s%-10s%-10s%-10s%-10s%s\n", "Task", "ThrHi", "ThrLo", "Curr", "State", "Cycle");
    ctc_cli_out("--------------------------------------------------------------------\n");

    CTC_CLI_CHIP_MON_LOCK();

    if (g_chip_monitor_master[lchip]->last_record.valid & CTC_CLI_CHIP_MON_TEMPERATURE)
    {
        ctc_cli_out("%-15s%-10d%-10d%-10d%-10s%ds\n", "Temperature",
            g_chip_monitor_master[lchip]->temprature_high,
            g_chip_monitor_master[lchip]->temprature_low,
            g_chip_monitor_master[lchip]->last_record.temprature,
            ((CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_TEMPERATURE))? "Running": "Stoped"),
            g_chip_monitor_master[lchip]->cycle/1000);
    }
    else
    {
        ctc_cli_out("%-15s%-10d%-10d%-10s%-10s%ds\n", "Temperature",
            g_chip_monitor_master[lchip]->temprature_high,
            g_chip_monitor_master[lchip]->temprature_low,
            "-",
            ((CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_TEMPERATURE))? "Running": "Stoped"),
            g_chip_monitor_master[lchip]->cycle/1000);
    }
    if (g_chip_monitor_master[lchip]->last_record.valid & CTC_CLI_CHIP_MON_VOLTAGE)
    {
        ctc_cli_out("%-15s%-10d%-10d%-10d%-10s%ds\n", "Voltage",
            g_chip_monitor_master[lchip]->voltage_high,
            g_chip_monitor_master[lchip]->voltage_low,
            g_chip_monitor_master[lchip]->last_record.voltage,
            ((CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_VOLTAGE))? "Running": "Stoped"),
            g_chip_monitor_master[lchip]->cycle/1000);
    }
    else
    {
        ctc_cli_out("%-15s%-10d%-10d%-10s%-10s%ds\n", "Voltage",
            g_chip_monitor_master[lchip]->voltage_high,
            g_chip_monitor_master[lchip]->voltage_low,
            "-",
            ((CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_VOLTAGE))? "Running": "Stoped"),
            g_chip_monitor_master[lchip]->cycle/1000);
    }
    ctc_cli_out("%-15s%-10s%-10s%-10s%-10s%ds\n", "MacError",
        "-",
        "-",
        "-",
        ((CTC_FLAG_ISSET(g_chip_monitor_master[lchip]->task_bmp, CTC_CLI_CHIP_MON_MAC_ERR))? "Running": "Stoped"),
        g_chip_monitor_master[lchip]->cycle/1000);

    CTC_CLI_CHIP_MON_UNLOCK();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_show_chip_monitor_history,
        ctc_cli_chip_show_chip_monitor_history_cmd,
        "show chip monitor log (lchip CHIP_ID|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Chip monitor",
        "Chip monitor log",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint8 index = 0;
    uint16 idx = 0;
    uint16 cnt = CTC_CLI_CHIP_MON_MAX_HIS;
    ctc_cli_chip_monitor_event_t* p_evt = NULL;
    char str_time[64] = {0};
    char* p_time_str = NULL;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_get_gchip_id(lchip, &gchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_CHECK_INIT(lchip);

    CTC_CLI_CHIP_MON_LOCK();

    idx = g_chip_monitor_master[lchip]->cur_evt_idx;
    idx = ((idx==0) ? CTC_CLI_CHIP_MON_MAX_HIS-1: idx-1);
    while(cnt)
    {
        p_evt = &(g_chip_monitor_master[lchip]->evt[idx]);
        p_time_str = sal_ctime(&(p_evt->timestamp));
        if(NULL == p_time_str)
        {
            CTC_CLI_CHIP_MON_UNLOCK();
            return CLI_ERROR;
        }

        sal_strncpy(str_time, p_time_str, sal_strlen(p_time_str)-1);
        switch (p_evt->evt)
        {
            case CTC_CLI_CHIP_MON_EVT_TEMPERATURE_HIGH:
            {
                ctc_cli_out("[%s] Temperature high %d (threshold %d) \n", str_time, p_evt->to.temp_value, g_chip_monitor_master[lchip]->temprature_high);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_TEMPERATURE_LOW:
            {
                ctc_cli_out("[%s] Temperature low %d (threshold %d) \n", str_time, p_evt->to.temp_value, g_chip_monitor_master[lchip]->temprature_low);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_TEMPERATURE_BACK_TO_NORMAL:
            {
                ctc_cli_out("[%s] Temperature back to normal %d (threshold high %d low %d) \n", str_time, p_evt->to.temp_value, g_chip_monitor_master[lchip]->temprature_high, g_chip_monitor_master[lchip]->temprature_low);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_VOLT_HIGH:
            {
                ctc_cli_out("[%s] Voltage high %dmV (threshold %dmV) \n", str_time, p_evt->to.volt_value, g_chip_monitor_master[lchip]->voltage_high);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_VOLT_LOW:
            {
                ctc_cli_out("[%s] Voltage low %dmV (threshold %dmV) \n", str_time, p_evt->to.volt_value, g_chip_monitor_master[lchip]->voltage_low);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_VOLT_BACK_TO_NORMAL:
            {
                ctc_cli_out("[%s] Voltage back to normal %dmV (threshold high %dmV low %dmV) \n", str_time, p_evt->to.volt_value, g_chip_monitor_master[lchip]->voltage_high, g_chip_monitor_master[lchip]->voltage_low);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_RX_FCS_ERR:
            {
                ctc_cli_out("[%s] MacRX on port 0x%4.4x FCSError Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_RX_MAC_OVERRUN:
            {
                ctc_cli_out("[%s] MacRX on port 0x%4.4x Overrun Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_RX_BAD_63:
            {
                ctc_cli_out("[%s] MacRX on port 0x%4.4x Bad63 Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_RX_BAD_JUMBO:
            {
                ctc_cli_out("[%s] MacRX on port 0x%4.4x BadJumbo Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_TX_FCS_ERR:
            {
                ctc_cli_out("[%s] MacTX on port 0x%4.4x FCSError Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            case CTC_CLI_CHIP_MON_EVT_TX_MAC_UNDERRUN:
            {
                ctc_cli_out("[%s] MacRX on port 0x%4.4x Underrun Pkt changed from %"PRIu64" to %"PRIu64" \n", str_time, p_evt->gport, p_evt->from.stats_value, p_evt->to.stats_value);
                break;
            }
            default :
                break;
        }

        idx = ((idx==0) ? CTC_CLI_CHIP_MON_MAX_HIS-1: idx-1);
        cnt--;
    }

    CTC_CLI_CHIP_MON_UNLOCK();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_clear_chip_monitor_history,
        ctc_cli_chip_clear_chip_monitor_history_cmd,
        "clear chip monitor log (lchip CHIP_ID|)",
        CTC_CLI_CLEAR_STR,
        "Chip module",
        "Chip monitor",
        "Chip monitor log",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;

    ret = ctc_get_gchip_id(lchip, &gchip);
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    CTC_CLI_CHIP_MON_CHECK_INIT(lchip);

    CTC_CLI_CHIP_MON_LOCK();

    sal_memset(g_chip_monitor_master[lchip]->evt, 0, sizeof(g_chip_monitor_master[lchip]->evt));
    g_chip_monitor_master[lchip]->cur_evt_idx = 0;
    g_chip_monitor_master[lchip]->last_record.valid = 0;

    CTC_CLI_CHIP_MON_UNLOCK();

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_chip_lb_hash_select,
        ctc_cli_chip_lb_hash_select_cmd,
        "lb-hash select-id SELECT_ID (hash-select HASH_SELECT|hash-control HASH_CONTROL) value VALUE",
        "Load banlance hash",
        "Hash computer selector",
        "Value of selector",
        "LB hash select config",
        "LB hash select value,reference to ctc_lb_hash_select_t",
        "LB hash control config",
        "Ld-hash-control value,reference to ctc_lb_hash_control_t",
        "Value",
        "Value of hash_control/value of hash_select")
{
    int32 ret = CLI_SUCCESS;
	uint8 index = 0;
	ctc_lb_hash_config_t hash_cfg;
	sal_memset(&hash_cfg,0,sizeof(hash_cfg));


    CTC_CLI_GET_INTEGER("select-id", hash_cfg.sel_id, argv[0]);
	CTC_CLI_GET_INTEGER("value", hash_cfg.value, argv[3]);

	index = CTC_CLI_GET_ARGC_INDEX("hash-select");
    if (0xFF != index)
    {
        hash_cfg.cfg_type = 0;
        CTC_CLI_GET_INTEGER("hash-select", hash_cfg.hash_select, argv[index + 1]);

    }
	index = CTC_CLI_GET_ARGC_INDEX("hash-control");
    if (0xFF != index)
    {
        hash_cfg.cfg_type = 1;
        CTC_CLI_GET_INTEGER("hash-control", hash_cfg.hash_control, argv[index + 1]);

    }


	if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_set(g_api_lchip,CTC_GLOBAL_LB_HASH_KEY, &hash_cfg);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_KEY,&hash_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_chip_show_lb_hash_select,
        ctc_cli_chip_show_lb_hash_select_cmd,
        "show lb-hash select-id SELECT_ID (hash-select HASH_SELECT|hash-control HASH_CONTROL)",
        CTC_CLI_SHOW_STR,
        "Load banlance hash",
        "Hash computer selector",
        "Value of selector",
        "Hash select config",
        "Hash select value,reference to ctc_lb_hash_select_t",
        "Hash control config",
        "Hash control value,reference to ctc_lb_hash_control_t")
{
    int32 ret = CLI_SUCCESS;
	uint8 index = 0;
	ctc_lb_hash_config_t hash_cfg;
	sal_memset(&hash_cfg,0,sizeof(hash_cfg));


    CTC_CLI_GET_INTEGER("select-id", hash_cfg.sel_id, argv[0]);
	index = CTC_CLI_GET_ARGC_INDEX("hash-select");
    if (0xFF != index)
    {
        hash_cfg.cfg_type = 0;
        CTC_CLI_GET_INTEGER("hash-select", hash_cfg.hash_select, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("hash-control");
    if (0xFF != index)
    {
        hash_cfg.cfg_type = 1;
        CTC_CLI_GET_INTEGER("hash-control", hash_cfg.hash_control, argv[index + 1]);
    }
    if (g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_LB_HASH_KEY, &hash_cfg);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_LB_HASH_KEY, &hash_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("value:0x%04x\n", hash_cfg.value);
    return ret;
}
CTC_CLI(ctc_cli_chip_lb_hash_offset,
        ctc_cli_chip_lb_hash_offset_cmd,
        "lb-hash select-offset (profile-id PROFILE_ID) (ecmp|linkagg|stacking|head-ecmp|head-lag) (static|dlb-flow-set|dlb-flow-member|resilient|)\
        (unicast |non-unicast |vxlan |nvgre |mpls |ip ) (use-entropy| use-packet-head| offset OFFSET)",
        "Load banlance hash",
        "Select-offset-cfg",
        "Table DsPortBasedHashProfile index",
        "Value",
        "Ecmp module",
        "Linkagg module",
        "Stacking module",
        "Head ecmp for nexthop edit",
        "Head lag for non-unicast",
        "Static mode",
        "Dlb flow set mode",
        "Dlb flow member mode",
        "Resilient mode",
        "Unicast",
        "Non unicast (mc/bcast)",
        "Vxlan ecmp",
        "Nvgre ecmp",
        "Mpls  ecmp",
        "IP(others)ecmp",
        "Use entropy hash",
        "Use packet head hash",
        "Hash select and offset",
        "Value")
        {
            uint8 index = 0;
			int32 ret = 0;
            ctc_lb_hash_offset_t p_hash_offset;
			sal_memset(&p_hash_offset,0,sizeof(ctc_lb_hash_offset_t));

            index = CTC_CLI_GET_ARGC_INDEX("linkagg");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
            }
            index = CTC_CLI_GET_ARGC_INDEX("ecmp");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_ECMP;
            }
            index = CTC_CLI_GET_ARGC_INDEX("stacking");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_STACKING;
            }
			index = CTC_CLI_GET_ARGC_INDEX("head-ecmp");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_ECMP;
            }
			index = CTC_CLI_GET_ARGC_INDEX("head-lag");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_LAG;
            }
            index = CTC_CLI_GET_ARGC_INDEX("static");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_STATIC;
            }
            index = CTC_CLI_GET_ARGC_INDEX("dlb-flow-set");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
            }
			index = CTC_CLI_GET_ARGC_INDEX("dlb-flow-member");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_MEMBER;
            }
            index = CTC_CLI_GET_ARGC_INDEX("resilient");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_RESILIENT;

            }
            index = CTC_CLI_GET_ARGC_INDEX("unicast");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_UC;

            }
			index = CTC_CLI_GET_ARGC_INDEX("non-unicast");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_NON_UC;

            }
			index = CTC_CLI_GET_ARGC_INDEX("vxlan");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_VXLAN;

            }
			index = CTC_CLI_GET_ARGC_INDEX("nvgre");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_NVGRE;

            }
			index = CTC_CLI_GET_ARGC_INDEX("mpls");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_MPLS;

            }
			index = CTC_CLI_GET_ARGC_INDEX("ip");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_IP;

            }
            index = CTC_CLI_GET_ARGC_INDEX("profile-id");
            if (0xFF != index)
            {
                CTC_CLI_GET_INTEGER("profile-id", p_hash_offset.profile_id, argv[index + 1]);

            }

            index = CTC_CLI_GET_ARGC_INDEX("use-entropy");
            if (0xFF != index)
            {
                p_hash_offset.use_entropy_hash = 1;
            }

            index = CTC_CLI_GET_ARGC_INDEX("use-packet-head");
            if (0xFF != index)
            {
                p_hash_offset.use_packet_head_hash = 1;
            }

            index = CTC_CLI_GET_ARGC_INDEX("offset");
            if (0xFF != index)
            {
                CTC_CLI_GET_INTEGER("offset", p_hash_offset.offset, argv[index + 1]);

            }
            if (g_ctcs_api_en)
            {
                ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &p_hash_offset);
            }
            else
            {
                ret = ctc_global_ctl_set(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &p_hash_offset);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            return ret;

        }

CTC_CLI(ctc_cli_chip_show_lb_hash_offset,
        ctc_cli_chip_show_lb_hash_offset_cmd,
        "show lb-hash select-offset (profile-id PROFILE_ID) (ecmp|linkagg|stacking|head-ecmp|head-lag) (static|dlb-flow-set|dlb-flow-member|resilient|)\
        (unicast |non-unicast |vxlan |nvgre |mpls |ip )",
        CTC_CLI_SHOW_STR,
        "Load banlance hash",
        "Select-offset-cfg",
        "Table DsPortBasedHashProfile index",
        "Value",
        "Ecmp module",
        "Linkagg module",
        "Stacking module",
        "Head ecmp for nexthop edit",
        "Head lag for non-unicast",
        "Static mode",
        "Dlb flow set mode",
        "Dlb flow member mode",
        "Resilient mode",
        "Unicast",
        "Non unicast (mc/bcast)",
        "Vxlan message of ecmp",
        "Nvgre message of ecmp",
        "Mpls message 0f ecmp",
        "l3 message of ecmp")
        {
            uint8 index = 0;
			int32 ret = 0;
            ctc_lb_hash_offset_t p_hash_offset;
			sal_memset(&p_hash_offset,0,sizeof(ctc_lb_hash_offset_t));

            index = CTC_CLI_GET_ARGC_INDEX("linkagg");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_LINKAGG;
            }
            index = CTC_CLI_GET_ARGC_INDEX("ecmp");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_ECMP;
            }
            index = CTC_CLI_GET_ARGC_INDEX("stacking");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_STACKING;
            }
			index = CTC_CLI_GET_ARGC_INDEX("head-ecmp");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_ECMP;
            }
			index = CTC_CLI_GET_ARGC_INDEX("head-lag");
            if (0xFF != index)
            {
                p_hash_offset.hash_module = CTC_LB_HASH_MODULE_HEAD_LAG;
            }
            index = CTC_CLI_GET_ARGC_INDEX("static");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_STATIC;
            }
            index = CTC_CLI_GET_ARGC_INDEX("dlb-flow-set");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_FLOW_SET;
            }
			index = CTC_CLI_GET_ARGC_INDEX("dlb-flow-member");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_DLB_MEMBER;
            }
            index = CTC_CLI_GET_ARGC_INDEX("resilient");
            if (0xFF != index)
            {
                p_hash_offset.hash_mode = CTC_LB_HASH_MODE_RESILIENT;

            }
            index = CTC_CLI_GET_ARGC_INDEX("unicast");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_UC;

            }
			index = CTC_CLI_GET_ARGC_INDEX("non-unicast");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_NON_UC;

            }
			index = CTC_CLI_GET_ARGC_INDEX("vxlan");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_VXLAN;

            }
			index = CTC_CLI_GET_ARGC_INDEX("nvgre");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_NVGRE;

            }
			index = CTC_CLI_GET_ARGC_INDEX("mpls");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_MPLS;

            }
			index = CTC_CLI_GET_ARGC_INDEX("ip");
            if (0xFF != index)
            {
                p_hash_offset.fwd_type = CTC_LB_HASH_FWD_IP;

            }
            index = CTC_CLI_GET_ARGC_INDEX("profile-id");
            if (0xFF != index)
            {
                CTC_CLI_GET_INTEGER("profile-id", p_hash_offset.profile_id, argv[index + 1]);

            }
            if (g_ctcs_api_en)
            {
                ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &p_hash_offset);
            }
            else
            {
                ret = ctc_global_ctl_get(CTC_GLOBAL_LB_HASH_OFFSET_PROFILE, &p_hash_offset);
            }
            if (ret < 0)
            {
                ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
                return CLI_ERROR;
            }

            if (p_hash_offset.use_entropy_hash)
            {
                ctc_cli_out("use entropy:%d\n", 1);
            }
            else if(p_hash_offset.use_packet_head_hash)
            {
                ctc_cli_out("use packet head:%d\n", 1);
            }
            else
            {
                ctc_cli_out("hash offset:%d\n", p_hash_offset.offset);
            }

            return ret;

        }
CTC_CLI(ctc_cli_chip_set_global_overlay_decap_mode,
        ctc_cli_chip_set_global_overlay_decap_mode_cmd,
        "chip global-cfg vxlan-decap-mode MODE nvgre-decap MODE scl-id SCL_ID",
        "Chip module",
        "Global config",
        "Vxlan decap mode",
        "MODE value",
        "Nvgre decap mode",
        "MODE value",
        "Scl id", "SCL ID")
{
    int32 ret       = CLI_SUCCESS;
    ctc_global_overlay_decap_mode_t decap_mode;

    sal_memset(&decap_mode, 0, sizeof(decap_mode));

    CTC_CLI_GET_UINT8("vxlan decap mode", decap_mode.vxlan_mode, argv[0]);
    CTC_CLI_GET_UINT8("nvgre decap mode", decap_mode.nvgre_mode, argv[1]);
    CTC_CLI_GET_UINT8("scl id", decap_mode.scl_id, argv[2]);

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_set(g_api_lchip, CTC_GLOBAL_OVERLAY_DECAP_MODE, &decap_mode);
    }
    else
    {
        ret = ctc_global_ctl_set(CTC_GLOBAL_OVERLAY_DECAP_MODE, &decap_mode);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;

}
CTC_CLI(ctc_cli_chip_get_global_overlay_decap_mode,
        ctc_cli_chip_get_global_overlay_decap_mode_cmd,
        "show chip global-cfg overlay-mode scl-id SCL_ID",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "Global config",
        "overlay mode",
        "Scl id", "SCL ID")
{
    int32 ret       = CLI_SUCCESS;
    ctc_global_overlay_decap_mode_t decap_mode;

    sal_memset(&decap_mode, 0, sizeof(decap_mode));

    CTC_CLI_GET_UINT8("scl id", decap_mode.scl_id, argv[0]);

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_OVERLAY_DECAP_MODE, &decap_mode);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_OVERLAY_DECAP_MODE, &decap_mode);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("vxlan decap mode  : %u\n",decap_mode.vxlan_mode);
    ctc_cli_out("nvgre decap mode  : %u\n",decap_mode.nvgre_mode);

    return ret;

}
CTC_CLI(ctc_cli_chip_show_property,
        ctc_cli_chip_show_property_cmd,
        "show chip property (phy-scan-en | i2c-scan-en | mac-led-en | phy-scan-para (mdio-ctlr-id ID|) | i2c-scan-para (i2c-ctlr-id ID |)) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Chip module",
        "property",
        "Phy scan enable",
        "I2c scan enable",
        "Mac led enable",
        "Phy scan parameter",
        "MDIO controller ID",
        "ID",
        "I2c scan parameter",
        "i2c master id",
        "ID",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    uint8 index = 0;
    uint8 lchip = 0;
    int32 ret = CLI_SUCCESS;
    uint32 type = 0;
    bool enable = FALSE;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-scan-en");
    if (0xFF != index)
    {
        type = CTC_CHIP_PHY_SCAN_EN;
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_get_property(g_api_lchip, type, &enable);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, &enable);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Phy-scan-en", enable);
    }

    index = CTC_CLI_GET_ARGC_INDEX("i2c-scan-en");
    if (0xFF != index)
    {
        type = CTC_CHIP_I2C_SCAN_EN;
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_get_property(g_api_lchip, type, &enable);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, &enable);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "I2c-scan-en", enable);
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-led-en");
    if (0xFF != index)
    {
        type = CTC_CHIP_MAC_LED_EN;
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_get_property(g_api_lchip, type, &enable);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, &enable);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n", "Mac-led-en", enable);
    }

    index = CTC_CLI_GET_ARGC_INDEX("phy-scan-para");
    if (0xFF != index)
    {
        ctc_chip_phy_scan_ctrl_t mdio_scan_para;

        sal_memset(&mdio_scan_para, 0, sizeof(ctc_chip_phy_scan_ctrl_t));

        index = CTC_CLI_GET_ARGC_INDEX("mdio-ctlr-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("mdio control id", mdio_scan_para.mdio_ctlr_id, argv[index + 1]);
        }

        type = CTC_CHIP_PHY_SCAN_PARA;
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_get_property(g_api_lchip, type, &mdio_scan_para);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, &mdio_scan_para);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n",     "Mdio control id", mdio_scan_para.mdio_ctlr_id);
        ctc_cli_out("%-34s:  %u\n",     "Scan interval", mdio_scan_para.scan_interval);
        ctc_cli_out("%-34s:  0x%x\n",   "Mdio use phy0", mdio_scan_para.mdio_use_phy0);
        ctc_cli_out("%-34s:  0x%x\n",   "Mdio use phy1", mdio_scan_para.mdio_use_phy1);
        ctc_cli_out("%-34s:  %s\n",     "Mdio control type", mdio_scan_para.mdio_ctlr_type?"XG":"GE");
        if (CTC_CHIP_MDIO_XG == mdio_scan_para.mdio_ctlr_type)
        {
            ctc_cli_out("%-34s:  0x%x\n",   "Xgphy link bitmask", mdio_scan_para.xgphy_link_bitmask);
            ctc_cli_out("%-34s:  %u\n",     "Xgphy scan twice", mdio_scan_para.xgphy_scan_twice);
        }
        ctc_cli_out("%-34s:  0x%x\n",   "Scan phy bitmap0", mdio_scan_para.scan_phy_bitmap0);
        ctc_cli_out("%-34s:  0x%x\n",   "Scan phy bitmap1", mdio_scan_para.scan_phy_bitmap1);
    }

    index = CTC_CLI_GET_ARGC_INDEX("i2c-scan-para");
    if (0xFF != index)
    {
        ctc_chip_i2c_scan_t i2c_scan_para;

        sal_memset(&i2c_scan_para, 0, sizeof(ctc_chip_i2c_scan_t));

        index = CTC_CLI_GET_ARGC_INDEX("i2c-ctlr-id");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8("i2c control id", i2c_scan_para.ctl_id, argv[index + 1]);
        }

        type = CTC_CHIP_I2C_SCAN_PARA;
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_get_property(g_api_lchip, type, &i2c_scan_para);
        }
        else
        {
            ret = ctc_chip_get_property(lchip, type, &i2c_scan_para);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }

        ctc_cli_out("%-34s:  %u\n",     "I2c control id", i2c_scan_para.ctl_id);
        ctc_cli_out("%-34s:  %u\n",     "Slave dev addr", i2c_scan_para.dev_addr);
        ctc_cli_out("%-34s:  %u\n",     "Offset", i2c_scan_para.offset);
        ctc_cli_out("%-34s:  %u\n",     "Scan length", i2c_scan_para.length);
        ctc_cli_out("%-34s:  0x%x\n",   "Scan interval", i2c_scan_para.interval);
        ctc_cli_out("%-34s:  %u\n",     "I2c switch id", i2c_scan_para.i2c_switch_id);
        ctc_cli_out("%-34s:  0x%x\n",   "Scan bitmap0", i2c_scan_para.slave_bitmap[0]);
        ctc_cli_out("%-34s:  0x%x\n",   "Scan bitmap1", i2c_scan_para.slave_bitmap[1]);
    }

    return ret;
}

int32
ctc_chip_cli_init(void)
{
    sal_memset(g_chip_monitor_master, 0, sizeof(g_chip_monitor_master));

    install_element(CTC_SDK_MODE, &ctc_cli_chip_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_debug_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_datapath_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_datapath_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_datapath_show_debug_cmd);
    install_element(CTC_SDK_MODE,  &ctc_cli_chip_show_chip_clock_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_phy_scan_para_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_phy_scan_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_read_i2c_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_write_i2c_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_i2c_scan_para_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_i2c_scan_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_read_i2c_buffer_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_i2c_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_i2c_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_mac_led_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_mac_led_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_mac_led_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_mac_led_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_gpio_operation_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_access_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_mode_cmd);


    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_discard_same_mac_or_ip_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_discard_same_mac_or_ip_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_dlb_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_dlb_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_dlb_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_dlb_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_flow_prop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_flow_prop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_acl_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_cid_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_cid_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_capability_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_config_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_config_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_mdio_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_mdio_clock_frequency_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_sensor_value_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_param_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_poly_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_loopback_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_p_flag_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_peak_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_dpc_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_slew_rate_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_ctle_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_mem_scan_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_reset_hw_cmd);


    install_element(CTC_SDK_MODE, &ctc_cli_chip_mdio_read_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_mdio_write_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_serdes_prop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_serdes_prbs_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_device_info_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_gport_property_prop_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_mem_bist_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_global_panel_ports_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_chip_monitor_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_monitor_interval_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_chip_monitor_status_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_chip_monitor_history_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_clear_chip_monitor_history_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_chip_lb_hash_select_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_chip_lb_hash_offset_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_select_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_chip_show_lb_hash_offset_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_show_property_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_log_shift_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_log_shift_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_get_global_overlay_decap_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_chip_set_global_overlay_decap_mode_cmd);

    return 0;
}

