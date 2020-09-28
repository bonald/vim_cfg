#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include "cli.h"
#include "clish.h"
#include "lcsh_lcmtlk.h"
#include "lcapi_lcm_lcsh_msg.h"
#include "ctclib_show.h"
#include "diag_cli.h"
#include "glb_const.h"
#include "lcsh_client.h"
#include "glb_macro.h"
#include <inttypes.h>

#define BOARD_MAX_PORT 60


CLI(diag_read_gephy,
    diag_read_gephy_cmd,
    "read gephy PORTID ADDRESS (<1-64>|)",
    CLI_READ_DESC,
    CLI_GEPHY_DESC,
    CLI_PORT_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 port = 0;
    uint32 address = 0;
    uint32 count = 1;
    uint32 ii;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    if(argc >= 3)
    {
        CLI_GET_INTEGER_RANGE(CLI_COUNT_DESC, count, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);
    }

    read_req.opcode = REG_GEPHY_R;
    read_req.chip = port;
    read_req.addr = address;
    read_req.count = count;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read gephy fail.\n");
        return CLI_ERROR;
    }
    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for(ii = 0; ii < count; ii++)
    {
        cli_out(cli, "\t0x%08x:\t0x%08x\n", buf[ii].addr, buf[ii].value);
    }
    return CLI_SUCCESS;
}

CLI(diag_write_gephy,
    diag_write_gephy_cmd,
    "write gephy PORTID ADDRESS VALUE",
    CLI_WRITE_DESC,
    CLI_GEPHY_DESC,
    CLI_PORT_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_SHORT_VALUE_DESC)
{
    int32 ret = 0;
    uint32 port = 0;
    uint32 address = 0;
    uint32 value = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_SHORT_VALUE_DESC, value, argv[2], 0, 0xFFFF);

    write_req.opcode = REG_GEPHY_W;
    write_req.chip = port;
    write_req.addr = address;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write gephy fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_read_xgphy,
    diag_read_xgphy_cmd,
    "read xgphy PORTID DEVICEID ADDRESS (<1-64>|)",
    CLI_READ_DESC,
    CLI_XGPHY_DESC,
    "Index of panel port",
    "Device_id",
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 port = 0;
    uint32 device = 0;
    uint32 address = 0;
    uint32 count = 1;
    uint32 ii;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_INTEGER_RANGE("Deviceid", device, argv[1], 0, 0x3f);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[2], 0, 0xFFFF);
    address = device * 0x10000 + address;
    if(argc >= 4)
    {
        CLI_GET_INTEGER_RANGE(CLI_COUNT_DESC, count, argv[3], 1, LCAPI_LCM_LCSH_REG_NUM);
    }
    
    read_req.opcode = REG_XGPHY_R;
    read_req.chip = port;
    read_req.addr = address;
    read_req.count = count;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Read xgphy fail.\n");
        return CLI_ERROR;
    }
    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for(ii = 0; ii < count; ii++)
    {
        cli_out(cli, "\t0x%08x:\t0x%08x\n", buf[ii].addr, buf[ii].value);
    }
    return CLI_SUCCESS;
}

CLI(diag_write_xgphy,
    diag_write_xgphy_cmd,
    "write xgphy PORTID DEVICE ADDRESS VALUE",
    CLI_WRITE_DESC,
    CLI_XGPHY_DESC,
    CLI_CHIP_IDX_DESC,
    "Device id",
    CLI_ADDRESS_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int32 ret = 0;
    uint32 port = 0;
    uint32 device = 0;
    uint32 address = 0;
    uint32 value = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_INTEGER_RANGE("Deviceid", device, argv[1], 0, 0x3f);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[2], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_SHORT_VALUE_DESC, value, argv[3], 0, 0xFFFF);
    address = device * 0x10000 + address;

    write_req.opcode = REG_XGPHY_W;
    write_req.chip = port;
    write_req.addr = address;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write xgphy fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_speed,
    diag_config_port_speed_cmd,
    "conport speed PORTID SPEED",
    CLI_CONFIG_PORT,
    "port speed",
    CLI_PORT_IDX_DESC,
    "10M | 100M | 1G | 10G | auto")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    conport_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    con_req.port_id = port;
   
    con_req.opcode = CONFIG_SPEED;
    if(!sal_strcmp(argv[1], "10M"))
    {
        con_req.speed = GLB_SPEED_10M;
    }    
    else if(!sal_strcmp(argv[1], "100M"))
    {
        con_req.speed = GLB_SPEED_100M;
    }
    else if(!sal_strcmp(argv[1], "1G"))
    {
        con_req.speed = GLB_SPEED_1G;
    }
    else if(!sal_strcmp(argv[1], "10G"))
    {
        con_req.speed = GLB_SPEED_10G;
    }   
    else if(!sal_strcmp(argv[1], "auto"))
    {
        con_req.speed = GLB_SPEED_AUTO;
    }
    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    sal_memcpy(st_req.msg, &con_req, sizeof(conport_t));
    st_req.msg_len = sizeof(conport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config speed fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_duplex,
    diag_config_port_duplex_cmd,
    "conport duplex PORTID DUPLEX",
    CLI_CONFIG_PORT,
    "port duplex",
    CLI_PORT_IDX_DESC,
    "auto | half | full")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    conport_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    con_req.port_id = port;

    con_req.opcode = CONFIG_DUPLEX;
    if(!sal_strcmp(argv[1], "auto"))
    {
        con_req.duplex = GLB_DUPLEX_AUTO;
    }    
    else if(!sal_strcmp(argv[1], "half"))
    {
        con_req.duplex = GLB_DUPLEX_HALF;
    }
    else if(!sal_strcmp(argv[1], "full"))
    {
        con_req.duplex = GLB_DUPLEX_FULL;
    }
 
    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    sal_memcpy(st_req.msg, &con_req, sizeof(conport_t));
    st_req.msg_len = sizeof(conport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config duplex fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_enable,
    diag_config_port_enable_cmd,
    "conport enable PORTID ENABLE",
    CLI_CONFIG_PORT,
    "enable or diable",
    CLI_PORT_IDX_DESC,
    "enable | disable")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    conport_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    con_req.port_id = port;
    con_req.opcode = CONFIG_ENABLE;
    if(!sal_strcmp(argv[1], "enable"))
    {
        con_req.enable = 1;
    }    
    else if(!sal_strcmp(argv[1], "disable"))
    {
        con_req.enable = 0;
    }
 
    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    sal_memcpy(st_req.msg, &con_req, sizeof(conport_t));
    st_req.msg_len = sizeof(conport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config enable fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_lb,
    diag_config_port_lb_cmd,
    "conport lb PORTID LBACK",
    CLI_CONFIG_PORT,
    "config loopback mode",
    CLI_PORT_IDX_DESC,
    "none | near | far")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    conport_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    con_req.port_id = port;
    con_req.opcode = CONFIG_LB;
    if(!sal_strcmp(argv[1], "none"))
    {
        con_req.loopback = GLB_LB_NONE;
    }    
    else if(!sal_strcmp(argv[1], "near"))
    {
        con_req.loopback = GLB_LB_PHY_NEAR;
    }    
    else if(!sal_strcmp(argv[1], "far"))
    {
        con_req.loopback = GLB_LB_PHY_FAR;
    }

    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    sal_memcpy(st_req.msg, &con_req, sizeof(conport_t));
    st_req.msg_len = sizeof(conport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config loopback mode fail.");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_media,
    diag_config_port_media_cmd,
    "conport media PORTID MEDIA",
    CLI_CONFIG_PORT,
    "config media type",
    CLI_PORT_IDX_DESC,
    "RJ45 | SFP | AUTO")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    conport_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    con_req.port_id = port;
    con_req.opcode = CONFIG_MEDIA;
    if(!sal_strcmp(argv[1], "RJ45"))
    {
        con_req.media_type = GLB_PORT_TYPE_COPPER;
    }    
    else if(!sal_strcmp(argv[1], "SFP"))
    {
        con_req.media_type = GLB_PORT_TYPE_FIBER;
    }    
    else if(!sal_strcmp(argv[1], "AUTO"))
    {
        con_req.media_type = GLB_PORT_TYPE_AUTO;
    }
    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PORT;
    sal_memcpy(st_req.msg, &con_req, sizeof(conport_t));
    st_req.msg_len = sizeof(conport_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config media fail.\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* added by tongzb for group xfp/sfp/qsfp 2016-07-08 */
CLI(diag_read_fiber,
    diag_read_fiber_cmd,
    "read fiber (basic|extend|phy) PORTID ADDRESS (<1-64>|)",
    CLI_READ_DESC,
    CLI_FIBER_DESC,
    "basic register",
    "extend register(just for sfp)",
    "phy register(just for sfp)",
    CLI_PORT_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    reg_values_t* buf;
    read_regs_reg_t read_req;
    lcapi_lcm_lcsh_msg_t st_rcv;
    int ret = 0;
    uint32 ii = 0;
    uint32 port = 0;
    uint32 address = 0;
    uint32 count = 1;

    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[1], 1, BOARD_MAX_PORT);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[2], 0, 0xFFFF);
    if(argc >= 4)
    {
        CLI_GET_INTEGER_RANGE(CLI_COUNT_DESC, count, argv[3], 1, LCAPI_LCM_LCSH_REG_NUM);
    }
    /* set the subdev */
    if ((sal_strncmp (argv[0], "basic", 5) == 0))
    {
        address |= 0x100;
    }
    else if ((sal_strncmp (argv[0], "extend", 6) == 0))
    {
        address |= 0x200;
    }
    else if((sal_strncmp (argv[0], "phy", 3) == 0))
    {
        address |= 0x300;
        count = count*2;
    }

    read_req.opcode = REG_FIBER_R;
    read_req.chip = port;
    read_req.addr = address;
    read_req.count = count;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Read fiber fail.\n");
        return CLI_ERROR;
    }
    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for(ii = 0; ii < count; ii++)
    {
        cli_out(cli, "\t0x%08x:\t0x%08x\n", buf[ii].addr, buf[ii].value);
    }

    return CLI_SUCCESS;
}

CLI(diag_write_fiber,
    diag_write_fiber_cmd,
    "write fiber (basic|extend|phy) PORTID ADDRESS VALUE",
    CLI_WRITE_DESC,
    CLI_FIBER_DESC,
    "basic register",
    "extend register(just for sfp)",
    "phy register(just for sfp)",
    CLI_PORT_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_SHORT_VALUE_DESC)
{
    write_regs_reg_t write_req;
    lcapi_lcm_lcsh_msg_t st_rcv;

    int ret = 0;
    uint32 port = 0;
    uint32 address = 0;
    uint32 value;

    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[1], 1, BOARD_MAX_PORT);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[2], 0, 0xFFFF);
	CLI_GET_HEX_UINT32_RANGE(CLI_SHORT_VALUE_DESC, value, argv[3], 0, 0xffff);

    /* set the subdev */
    if ((sal_strncmp (argv[0], "basic", 5) == 0))
    {
        address |= 0x100;
    }
    else if ((sal_strncmp (argv[0], "extend", 6) == 0))
    {
        address |= 0x200;
    }
    else if((sal_strncmp (argv[0], "phy", 3) == 0))
    {
        address |= 0x300;
    }

    write_req.opcode = REG_FIBER_W;
    write_req.chip = port;
    write_req.addr = address;
	write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write fiber %s fail.\n",argv[0]);
        return CLI_ERROR;
    }
	cli_out(cli, "Write fiber %s success.\n",argv[0]);
    return CLI_SUCCESS;
}
/* added end by tongzb for group xfp/sfp/qsfp */

/* added by tongzb for bug 43937 */
CLI(diag_fiber_config_wavelength,
    diag_fiber_config_wavelength_cmd,
    "config fiber (wavelength|channel) PORTID VALUE",
    "config fiber parameter",
    CLI_FIBER_DESC,
    "change freq",
    "change channel",
    CLI_PORT_IDX_DESC,
    CLI_SHORT_VALUE_DESC)
{
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    int ret = 0;
    uint32 port = 0;
    uint32 value;
    fiber_conf_t fiber_conf;

    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[1], 1, BOARD_MAX_PORT);
	CLI_GET_HEX_UINT32_RANGE(CLI_SHORT_VALUE_DESC, value, argv[2], 0, 0xffff);
    //CLI_GET_UINT32(CLI_SHORT_VALUE_DESC,value,argv[2]);
    
    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    /* set the para */
    fiber_conf.port_id = port;
    fiber_conf.value = value;
    if (sal_strncmp (argv[0], "wavelength", 10) == 0)
    {
        fiber_conf.opmode = CONFIG_WAVELENGTH;
    }
    else if (sal_strncmp (argv[0], "channel", 7) == 0)
    {
        fiber_conf.opmode = CONFIG_CHANNEL;
    }
    
    st_req.msg_type = LCAPI_LCM_LCSH_CHANGE_WAVELENGTH;
    sal_memcpy(st_req.msg, &fiber_conf, sizeof(fiber_conf_t));
    st_req.msg_len = sizeof(fiber_conf_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config fiber wavelength fail.\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
/* added end by tongzb */
CLI(diag_read_epld,
    diag_read_epld_cmd,
    "read epld <0-1> REG (<1-64>|)",
    CLI_READ_DESC,
    CLI_EPLD_DESC,    
    CLI_EPLD_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 count = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_EPLD_IDX_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFFFFFF);
    if(argc >= 3)
    {
        CLI_GET_INTEGER_RANGE(CLI_ADDRESS_DESC, count, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);
    }

    read_req.opcode = REG_EPLD_R;
    read_req.chip = chip;
    read_req.addr = address;
    read_req.count = count;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read EPLD fail\n");
        return CLI_ERROR;
    }

    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < count; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\n", buf[i].addr, buf[i].value);
    }

    return CLI_ERROR;
}

CLI(diag_write_epld,
    diag_write_epld_cmd,
    "write epld <0-1> REG VALUE",
    CLI_WRITE_DESC,
    CLI_EPLD_DESC,    
    CLI_EPLD_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int32 ret = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 value = 0; 
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_EPLD_IDX_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xFF);

    write_req.opcode = REG_EPLD_W;
    write_req.chip = chip;
    write_req.addr = address;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write EPLD fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_load_xgphy,
    diag_load_xgphy_cmd,
    "load xgphy PORTID",
    "Load fireware",
    CLI_XGPHY_DESC,
    "Index of panel port")
{
    int32 ret = 0;
    uint32 port = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    read_req.opcode = REG_XGPHY_LOAD;
    read_req.chip = port;

    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret || LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "load xgphy fail\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CLI(diag_read_sensor,
    diag_read_sensor_cmd,
    "read sensor <0-5> REG LEN",
    CLI_READ_DESC,
    CLI_SENSOR_DESC,    
    CLI_SENSOR_IDX_DESC,
    CLI_SENSOR_REG_DESC,
    CLI_SENSOR_REG_LEN_DESC)
{
    int32 ret = 0;
    uint32 sensor_idx = 0;
    uint32  reg = 0;
    uint32  len;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_SENSOR_IDX_DESC, sensor_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_SENSOR_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_SENSOR_REG_LEN_DESC, len, argv[2], 0, 0xff);
    
    read_req.opcode = REG_SENSOR_R;
    read_req.chip = sensor_idx;
    read_req.addr= reg;
    read_req.count = len;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read sensor register fail.\n");
        return CLI_ERROR;
    }
    
    buf = (reg_values_t* )(st_rcv.msg);
    cli_out (cli, "0x%02x:\t0x%02x\n",  reg, buf[0].value);
    
    return CLI_SUCCESS;
}

CLI(diag_write_sensor,
    diag_write_sensor_cmd,
    "write sensor <0-5> REG VALUE",
    CLI_WRITE_DESC,
    CLI_SENSOR_DESC,    
    CLI_SENSOR_IDX_DESC,
    CLI_SENSOR_REG_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t sensor_idx = 0;
    uint32_t reg = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_SENSOR_IDX_DESC, sensor_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_SENSOR_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xffff);

    write_req.opcode = REG_SENSOR_W;
    write_req.chip = sensor_idx;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write sensor register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_sensor_temp,
    diag_read_sensor_temp_cmd,
    "read sensor <0-5> type TYPE",
    CLI_READ_DESC,
    CLI_SENSOR_DESC,    
    CLI_SENSOR_IDX_DESC,
    CLI_SENSOR_TYPE_DESC,
    CLI_SENSOR_TYPE_DESC)
{
    int32 ret = 0;
    uint32 sensor_idx = 0;
    uint32  type = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_SENSOR_IDX_DESC, sensor_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_SENSOR_TYPE_DESC, type, argv[1], 0, 0xff);
    
    read_req.opcode = REG_SENSOR_TMP_R;
    read_req.chip = sensor_idx;
    read_req.addr= type;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read sensor register fail.\n");
        return CLI_ERROR;
    }
    
    buf = (reg_values_t* )(st_rcv.msg);
    cli_out (cli, "0x%02x:\t%d\n", type, (int8)(buf[0].value));
    
    return CLI_SUCCESS;
}

CLI(diag_write_sensor_temp,
    diag_write_sensor_temp_cmd,
    "write sensor <0-5> type TYPE VALUE",
    CLI_WRITE_DESC,
    CLI_SENSOR_DESC,    
    CLI_SENSOR_IDX_DESC,
    CLI_SENSOR_TYPE_DESC,
    CLI_SENSOR_TYPE_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t sensor_idx = 0;
    uint32_t type = 0;
    int32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_SENSOR_IDX_DESC, sensor_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_SENSOR_TYPE_DESC, type, argv[1], 0, 0xff);
    CLI_GET_INTEGER(CLI_BYTE_VALUE_DESC, value, argv[2]);

    write_req.opcode = REG_SENSOR_TMP_W;
    write_req.chip = sensor_idx;
    write_req.addr= type;
    write_req.value[0] = (uint32_t)value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write sensor register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_gpio,
    diag_read_gpio_cmd,
    "read gpio <0-4> REG LEN",
    CLI_READ_DESC,
    CLI_GPIO_CHIP_DESC,    
    CLI_GPIO_CHIP_IDX_DESC,
    CLI_GPIO_CHIP_REG_DESC,
    CLI_GPIO_CHIP_REG_LEN_DESC)
{
    int32 ret = 0;
    uint32 gpio_chip_idx = 0;
    uint32  reg = 0;
    uint32  len;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_GPIO_CHIP_IDX_DESC, gpio_chip_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_GPIO_CHIP_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_GPIO_CHIP_REG_LEN_DESC, len, argv[2], 0, 0xff);
    
    read_req.opcode = REG_GPIO_CHIP_R;
    read_req.chip = gpio_chip_idx;
    read_req.addr= reg;
    read_req.count = len;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read gpio chip register fail.\n");
        return CLI_ERROR;
    }
    
    buf = (reg_values_t* )(st_rcv.msg);
    cli_out (cli, "0x%02x:\t0x%02x\n",  reg, buf[0].value);
    
    return CLI_SUCCESS;
}

CLI(diag_write_gpio,
    diag_write_gpio_cmd,
    "write gpio <0-3> REG VALUE",
    CLI_WRITE_DESC,
    CLI_GPIO_CHIP_DESC,    
    CLI_GPIO_CHIP_IDX_DESC,
    CLI_GPIO_CHIP_REG_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t gpio_chip_idx = 0;
    uint32_t reg = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_GPIO_CHIP_IDX_DESC, gpio_chip_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_GPIO_CHIP_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xffff);

    write_req.opcode = REG_GPIO_CHIP_W;
    write_req.chip = gpio_chip_idx;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write gpio chip register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_mux,
    diag_read_mux_cmd,
    "read mux <0-1> REG LEN",
    CLI_READ_DESC,
    CLI_MUX_CHIP_DESC,    
    CLI_MUX_CHIP_IDX_DESC,
    CLI_MUX_CHIP_REG_DESC,
    CLI_MUX_CHIP_REG_LEN_DESC)
{
    int32 ret = 0;
    uint32 mux_chip_idx = 0;
    uint32  reg = 0;
    uint32  len;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_MUX_CHIP_IDX_DESC, mux_chip_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_MUX_CHIP_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_MUX_CHIP_REG_LEN_DESC, len, argv[2], 0, 0xff);
    
    read_req.opcode = REG_MUX_CHIP_R;
    read_req.chip = mux_chip_idx;
    read_req.addr= reg;
    read_req.count = len;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read mux chip register fail.\n");
        return CLI_ERROR;
    }
    
    buf = (reg_values_t* )(st_rcv.msg);
    cli_out (cli, "0x%02x:\t0x%02x\n",  reg, buf[0].value);
    
    return CLI_SUCCESS;
}

CLI(diag_write_mux,
    diag_write_mux_cmd,
    "write mux <0-1> REG VALUE",
    CLI_WRITE_DESC,
    CLI_MUX_CHIP_DESC,    
    CLI_MUX_CHIP_IDX_DESC,
    CLI_MUX_CHIP_REG_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t mux_chip_idx = 0;
    uint32_t reg = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_MUX_CHIP_IDX_DESC, mux_chip_idx, argv[0], 0, 5);
    CLI_GET_HEX_UINT32_RANGE(CLI_MUX_CHIP_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xffff);

    write_req.opcode = REG_MUX_CHIP_W;
    write_req.chip = mux_chip_idx;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write mux chip register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}
CLI(diag_read_fan,
    diag_read_fan_cmd,
    "read fan MODULE REG",
    CLI_READ_DESC,
    CLI_FAN_DESC,
    CLI_FAN_IDX_DESC,
    CLI_FAN_REG_DESC)
{
    int32 ret = 0;
    uint32 fan_idx = 0;
    uint32  reg = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_FAN_IDX_DESC, fan_idx, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_FAN_REG_DESC, reg, argv[1], 0, 0xff);
    
    read_req.opcode = REG_FAN_R;
    read_req.chip = fan_idx;
    read_req.addr= reg;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read fan register fail.\n");
        return CLI_ERROR;
    }
    
    buf = (reg_values_t* )(st_rcv.msg);
    cli_out (cli, "0x%02x:\t0x%02x\n",  reg, buf[0].value);
    
    return CLI_SUCCESS;
}

/* added by tongzb 2010-08-10 */
CLI(diag_read_fan_rpm,
    diag_read_fan_rpm_cmd,
    "read fan MODULE adt7470rpm",
    CLI_READ_DESC,
    CLI_FAN_DESC,
    CLI_FAN_IDX_DESC,
    "get the real time rpm")
{
    int32 ret = 0;
    uint32 fan_idx = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    uint32 tach[8];
	int32 i,rpm=-1;
	char reg[8] = {0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31};
    CLI_GET_INTEGER_RANGE(CLI_FAN_IDX_DESC, fan_idx, argv[0], 0, 1);
    
    read_req.opcode = REG_FAN_R;
    read_req.chip = fan_idx;
	for(i=0;i<8;i++)
    {
        read_req.addr = reg[i];
        ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Read fan register fail.\n");
            return CLI_ERROR;
        }
        buf = (reg_values_t* )(st_rcv.msg);
	    tach[i] = buf[0].value;
        cli_out (cli, "0x%02x:\t0x%02x\n",  read_req.addr, tach[i]);
    }
    for(i = 0; i < 4; i++)
    {
        rpm = (uint16)(90000*60/((tach[i*2+1]<<8)+tach[i*2]));
		cli_out (cli, "tach %d rpm : %d\n",i, rpm);
    }
	
    return CLI_SUCCESS;
}

CLI(diag_write_fan,
    diag_write_fan_cmd,
    "write fan MODULE REG VALUE",
    CLI_WRITE_DESC,
    CLI_FAN_DESC,    
    CLI_FAN_IDX_DESC,
    CLI_FAN_REG_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t fan_idx = 0;
    uint32_t reg = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_FAN_IDX_DESC, fan_idx, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_FAN_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xff);

    write_req.opcode = REG_FAN_W;
    write_req.chip = fan_idx;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write fan register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_fan_speed,
    diag_read_fan_speed_cmd,
    "read fan MODULE speed",
    CLI_READ_DESC,
    CLI_FAN_DESC,
    CLI_FAN_IDX_DESC,
    "get the real rpm of fan")
{
    int32 ret = 0;
    uint32 fan_idx = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    uint8 i;
    CLI_GET_INTEGER_RANGE(CLI_FAN_IDX_DESC, fan_idx, argv[0], 0, 1);
    
    read_req.opcode = REG_FAN_SPEED_R;
    read_req.chip = fan_idx;
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read fan register fail.\n");
        return CLI_ERROR;
    }    

    buf = (reg_values_t* )(st_rcv.msg);
	for(i=0;i<4;i++)
    {
        cli_out (cli, "Fan %d real rpm: %d\n",  buf[i].addr, buf[i].value);
    }
	
    return CLI_SUCCESS;
}

CLI(diag_write_fan_speed,
    diag_write_fan_speed_cmd,
    "write fan MODULE speed-table VALUE1 VALUE2 VALUE3 VALUE4",
    CLI_WRITE_DESC,
    CLI_FAN_DESC,    
    CLI_FAN_IDX_DESC,
    "fan speed level",
    "bottom speed level from 0 to 100, step 10",
    "low speed level from 0 to 100, step 10",
    "high speed level from 0 to 100, step 10",
    "full speed level from 0 to 100, step 10")
{
    int ret = 0;
    uint32_t fan_idx = 0;
    uint32_t value_bottom = 0;
    uint32_t value_low = 0;
    uint32_t value_high = 0;
    uint32_t value_full = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_FAN_IDX_DESC, fan_idx, argv[0], 0, 1);
    CLI_GET_INTEGER_RANGE(CLI_BYTE_VALUE_DESC, value_bottom, argv[1], 0, 100);
    CLI_GET_INTEGER_RANGE(CLI_BYTE_VALUE_DESC, value_low, argv[2], 0, 100);
    CLI_GET_INTEGER_RANGE(CLI_BYTE_VALUE_DESC, value_high, argv[3], 0, 100);
    CLI_GET_INTEGER_RANGE(CLI_BYTE_VALUE_DESC, value_full, argv[4], 0, 100);

    write_req.opcode = REG_FAN_SPEED_W;
    write_req.chip = fan_idx;
    write_req.value[0] = value_bottom;
    write_req.value[1] = value_low;
    write_req.value[2] = value_high;
    write_req.value[3] = value_full;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write fan register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


CLI(diag_read_power,
    diag_read_power_cmd,
    "read power MODULE REG LEN (|pmbus)",
    CLI_READ_DESC,
    CLI_POWER_DESC,
    CLI_POWER_IDX_DESC,
    CLI_POWER_REG_DESC,
    CLI_REG_DATA_LEN,
    "read by pmbus mode")
{
    uint32 i;
    uint32 data_len;
    int32 ret = 0;
    uint32 power_idx = 0;
    uint32  reg = 0;
    uint32 len = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_POWER_IDX_DESC, power_idx, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_POWER_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_REG_DATA_LEN, len, argv[2], 0, 0xff);
    
    read_req.opcode = REG_PSU_R;
    read_req.chip = power_idx;
    read_req.addr= reg;
    read_req.count= len;

    if(argc == 4)
    {
        read_req.chip |= 0x80;
        read_req.count= 2;
    }
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read power register fail.\n");
        return CLI_ERROR;
    }

    data_len = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < (data_len > len ? len : data_len); i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\n", buf[i].addr, buf[i].value);
    }
    
    return CLI_SUCCESS;
}

CLI(diag_write_power,
    diag_write_power_cmd,
    "write power MODULE REG VALUE",
    CLI_WRITE_DESC,
    CLI_POWER_DESC,    
    CLI_POWER_IDX_DESC,
    CLI_POWER_REG_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int ret = 0;
    uint32_t power_idx = 0;
    uint32_t reg = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_POWER_IDX_DESC, power_idx, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_POWER_REG_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xff);

    write_req.opcode = REG_PSU_W;
    write_req.chip = power_idx;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write power register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}



CLI(diag_read_vsc3308,
    diag_read_vsc3308_cmd,
    "read vsc3308 CHIP_ID REG (<1-64>|)",
    CLI_READ_DESC,
    CLI_VSC3308_DESC,
	CLI_VSC3308_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
	uint32 i = 0;
    uint32 reg = 0;
    uint32 chip = 0;
	uint32 count = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;

    CLI_GET_HEX_UINT32_RANGE(CLI_VSC3308_DESC, chip, argv[0], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_VSC3308_DESC, reg, argv[1], 0, 0xff);
    if(argc >= 3)
    {
        CLI_GET_INTEGER_RANGE(CLI_ADDRESS_DESC, count, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);
    }
    
    read_req.opcode = REG_VSC3308_R;
    read_req.chip = chip;
    read_req.addr= reg;
    read_req.count = count;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read VSC3308 register fail.\n");
        return CLI_ERROR;
    }
    
    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < count; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\n", buf[i].addr, buf[i].value);
	}
    
    return CLI_SUCCESS;
}

CLI(diag_write_vsc3308,
    diag_write_vsc3308_cmd,
    "write vsc3308 CHIP_ID REG VALUE",
    CLI_READ_DESC,
    CLI_VSC3308_DESC,
    CLI_VSC3308_IDX_DESC,
    CLI_ADDRESS_DESC,
    "Write data")
{
    int ret = 0;
    uint32_t reg = 0;
    uint32_t chip = 0;
    uint32_t value = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_HEX_UINT32_RANGE(CLI_VSC3308_DESC, chip, argv[0], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_VSC3308_DESC, reg, argv[1], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xff);

    write_req.opcode = REG_VSC3308_W;
    write_req.chip = chip;
    write_req.addr= reg;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Write VSC3308 register fail.\n");
        return CLI_ERROR;
    }
    return RESULT_OK;
}

CLI(diag_config_vsc3308_input_output_pin,
    diag_config_vsc3308_input_output_pin_cmd,
    "config vsc3308 CHIP_ID input INPUT_PIN output OUTPUT_PIN",
    "Configure",
    CLI_VSC3308_DESC,  
    CLI_VSC3308_IDX_DESC,
    "Input",
    "Input Pin <0-7>",
    "Output",
    "Output Pin <0-7>")
{
    int ret = 0;
    uint32 chip = 0;
    uint32 input_pin, output_pin;/* modified by cuixl, from uint8 to uint32*/
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;

    CLI_GET_HEX_UINT32_RANGE(CLI_VSC3308_DESC, chip, argv[0], 0, 0xff);
    CLI_GET_INTEGER_RANGE("Input pin", input_pin, argv[1], 0, 7);
    CLI_GET_INTEGER_RANGE("Output pin", output_pin, argv[2], 0, 7);

    write_req.opcode = REG_VSC3308_CONF;
    write_req.chip = chip;
    write_req.addr = input_pin;
    write_req.addr_1 = output_pin;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Configure VSC3308 register fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_config_phy_clock,
    diag_config_phy_clock_cmd,
    "config phy-clock PORTID CLK_PORT CLK-SEL CLK-FREQ CLK-SQUELCH",
    "Configure",
    "PHY clock",
    CLI_PORT_IDX_DESC,
    "Clock port <0-1>",
    CLI_PHY_CLK_SEL,
    CLI_PHY_CLK_FREQ,
    CLI_PHY_CLK_SQUELCH)
{
    int ret = 0;
    uint32 port = 0;
    uint32 phy_clk_port;
    uint32 phy_clk_sel;
    uint32 phy_clk_freq;
    uint32 phy_clk_squelch;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;

    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_INTEGER_RANGE("Clock port <0-1>", phy_clk_port, argv[1], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_PHY_CLK_SEL, phy_clk_sel, argv[2], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_PHY_CLK_FREQ, phy_clk_freq, argv[3], 0, 0xff);
    CLI_GET_HEX_UINT32_RANGE(CLI_PHY_CLK_SQUELCH, phy_clk_squelch, argv[4], 0, 0xff);

    write_req.opcode = REG_PHY_CLK_CONF;
    write_req.chip = port;
    write_req.addr = phy_clk_port;
    write_req.value[0] = (phy_clk_sel<<16)|(phy_clk_freq)<<8|phy_clk_squelch;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli, "Configure phy clock fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}


CLI(diag_read_eeprom,
    diag_read_eeprom_cmd,
    "read eeprom <0-1> REG (<1-64>|)",
    CLI_READ_DESC,
    CLI_EEPROM_DESC,    
    CLI_EEPROM_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 count = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_EEPROM_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    if(argc >= 3)
    {
        CLI_GET_INTEGER_RANGE(CLI_ADDRESS_DESC, count, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);
    }

    read_req.opcode = REG_EEPROM_R;
    read_req.chip = chip;
    read_req.addr = address;
    read_req.count = count;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read eeprom fail\n");
        return CLI_ERROR;
    }

    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < count; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\t(%c)\n", buf[i].addr, buf[i].value, buf[i].value);
    }

    return CLI_ERROR;
}

CLI(diag_write_eeprom,
    diag_write_eeprom_cmd,
    "write eeprom <0-1> REG VALUE",
    CLI_WRITE_DESC,
    CLI_EEPROM_DESC,    
    CLI_EEPROM_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int32 ret = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 value; 
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_EEPROM_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xFF);

    write_req.opcode = REG_EEPROM_W;
    write_req.chip = chip;
    write_req.addr = address;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write eeprom fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_sysenv,
    diag_read_sysenv_cmd,
    "read sysenv <0-1> REG (<1-64>|)",
    CLI_READ_DESC,
    CLI_SYSENV_DESC,    
    CLI_SYSENV_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 count = 1;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_SYSENV_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_SYSENV_DESC, address, argv[1], 0, 0xFFFF);
    if(argc >= 3)
    {
        CLI_GET_INTEGER_RANGE(CLI_ADDRESS_DESC, count, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);
    }

    read_req.opcode = REG_SYSENV_R;
    read_req.chip = chip;
    read_req.addr = address;
    read_req.count = count;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read sysenv fail\n");
        return CLI_ERROR;
    }

    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < count; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\t(%c)\n", buf[i].addr, buf[i].value, buf[i].value);
    }

    return CLI_ERROR;
}

CLI(diag_write_sysenv,
    diag_write_sysenv_cmd,
    "write sysenv <0-1> REG VALUE",
    CLI_WRITE_DESC,
    CLI_SYSENV_DESC,    
    CLI_SYSENV_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_BYTE_VALUE_DESC)
{
    int32 ret = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 value; 
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_SYSENV_DESC, chip, argv[0], 0, 1);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xFF);

    write_req.opcode = REG_SYSENV_W;
    write_req.chip = chip;
    write_req.addr = address;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write sysenv fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_read_i2c,
    diag_read_i2c_cmd,
    "read i2c I2C_BUS_IDX ADDR REG REGLEN DATALEN (|(BRGADDR BRGCHANNEL))",
    CLI_READ_DESC,
    "read i2c or i2c-bridge",    
    "Index of i2c bus",
    "Addr of i2c",
    "Register in i2c component",
    "Register length",
    "Read data length",
    "i2c bridge addr",
    "i2c bridge channel")
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 address;/* modified by cuixl, from uint8 to uint32*/
    uint32 reg; 
    uint32 reg_len;/* modified by cuixl, from uint8 to uint32*/
    uint32 data_len;/* modified by cuixl, from uint8 to uint32*/
    /* Added by qicx for bug 21474: 2012-11-27 */
    uint32 i2c_bus_idx;
    lcapi_lcm_lcsh_msg_t st_rcv;/* modified by cuixl, from uint8 to uint32*/
    read_regs_reg_t read_req;
    reg_values_t* buf;
    uint32 i2c_bridge_addr=0;
    uint32 i2c_bridge_channel=0; 
    write_regs_reg_t write_req;

    /* Added by qicx for bug 21474: 2012-11-27 */
    CLI_GET_HEX_UINT32_RANGE("Index of i2c bus", i2c_bus_idx, argv[0], 0, 0xFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE("Register", reg, argv[2], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the register", reg_len, argv[3], 0, 0xFFFF);
    CLI_GET_INTEGER_RANGE("Length of the data", data_len, argv[4], 1, LCAPI_LCM_LCSH_REG_NUM);
    if (argc > 5)
    {
        CLI_GET_HEX_UINT32_RANGE("addr of the i2c bridge", i2c_bridge_addr, argv[5], 0, 0xFF);
        CLI_GET_HEX_UINT32_RANGE("the i2c brigde channel", i2c_bridge_channel, argv[6], 0, 0xF);
    }
    
    if (argc > 5)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = (0x1<<i2c_bridge_channel);
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }

    read_req.opcode = REG_I2C_R;
    read_req.chip = address;
    read_req.addr = reg;
    /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
    read_req.addr_1 = (reg_len<<8) | i2c_bus_idx;
    read_req.count = data_len;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read IIC fail\n");
        goto read_i2c_out;
    }

    data_len = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < data_len; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\n", buf[i].addr, buf[i].value);
    }
read_i2c_out:    
    if (argc > 5)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = 0;
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }

    return CLI_ERROR;
}

CLI(diag_write_i2c,
    diag_write_i2c_cmd,
    "write i2c I2C_BUS_IDX ADDR REG REGLEN DATA DATALEN (|(BRGADDR BRGCHANNEL))",
    CLI_WRITE_DESC,
    "write i2c or i2c-bridge",    
    "Index of i2c bus",
    "Addr of i2c",
    "Register in i2c component",
    "Register length",
    "Write data",
    "Data length, unit is byte",
    "i2c bridge addr",
    "i2c bridge channel")
{
    int32 ret = 0;
    uint32 address;/* modified by cuixl, from uint8 to uint32*/
    uint32 reg; 
    uint32 reg_len;/* modified by cuixl, from uint8 to uint32*/
    uint32 data_len;/* modified by cuixl, from uint8 to uint32*/
    /* Added by qicx for bug 21474: 2012-11-27 */
    uint32 i2c_bus_idx;/* modified by cuixl, from uint8 to uint32*/
    uint32 value; /* modified by cuixl, from uint8 to uint32*/
    uint32 i2c_bridge_addr=0;
    uint32 i2c_bridge_channel=0;  
    
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;

    /* Added by qicx for bug 21474: 2012-11-27 */
    CLI_GET_HEX_UINT32_RANGE("Index of i2c bus", i2c_bus_idx, argv[0], 0, 0xFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE("Register", reg, argv[2], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the register", reg_len, argv[3], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE("Value", value, argv[4], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the value", data_len, argv[5], 0, 0xFF);

    if (argc > 6)
    {
        CLI_GET_HEX_UINT32_RANGE("addr of the i2c bridge", i2c_bridge_addr, argv[6], 0, 0xFF);
        CLI_GET_HEX_UINT32_RANGE("the i2c brigde channel", i2c_bridge_channel, argv[7], 0, 0xF);
    }
    
    if (argc > 6)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = (0x1<<i2c_bridge_channel);
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }

    write_req.opcode = REG_I2C_W;
    write_req.chip = address;
    write_req.addr = reg;
    /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
    write_req.addr_1 = (reg_len<<16) | (data_len<<8) | (i2c_bus_idx);
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write i2c fail.\n");
        goto write_i2c_out;
    }
write_i2c_out:    
    if (argc > 6)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = 0;
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }    
    return CLI_SUCCESS;
}

CLI(diag_transfer_i2c_raw,
    diag_transfer_i2c_raw_cmd,
    "transfer i2c-raw I2C_BUS_IDX ADDR (read|write DATA) DATALEN (| bridge BRGADDR BRGCHANNEL) (|checksum CHECKBYTE)",
    "transfer i2c data",
    "transfer data from i2c or i2c-bridge",
    "Index of i2c bus",
    "Addr of i2c",
    "read i2c",
    "write i2c",    
    "data for transfer,eg:0x123456789...",
    "data length",
    "through i2c bridge",
    "i2c bridge addr",
    "i2c bridge channel",
    "add checksum",
    "num of byte for checksum")
{
    int32 ret = 0;
    uint32 address;/* modified by cuixl, from uint8 to uint32*/
    uint32 data_len=0;/* modified by cuixl, from uint8 to uint32*/
    /* Added by qicx for bug 21474: 2012-11-27 */
    uint32 i2c_bus_idx = 0;/* modified by cuixl, from uint8 to uint32*/
    uint32 value[LCSH_I2C_BUF_LEN]; /* modified by cuixl, from uint8 to uint32*/
    uint32 i2c_bridge_addr=0;
    uint32 i2c_bridge_channel=0;
    uint8 is_bridge = 0;
    uint8 access_type = 0;
    reg_values_t* buf = NULL;
    uint32 i = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    uint8 argc_cnt = 0;
    uint32 checksum = 0;
    int8 checknum = 0;

    /* Added by qicx for bug 21474: 2012-11-27 */
    CLI_GET_HEX_UINT32_RANGE("Index of i2c bus", i2c_bus_idx, argv[0], 0, 0xFF);
    CLI_GET_HEX_UINT32_RANGE("Addr of i2c", address, argv[1], 0, 0xFFFF);

    if (0 == strncmp(argv[2], "read", 4))
    {
        access_type = 0;
        CLI_GET_HEX_UINT32_RANGE("Length of the value", data_len, argv[3], 0, LCSH_I2C_BUF_LEN);
        argc_cnt = 4;
    }
    else if (0 == strncmp(argv[2], "write", 5))
    {
        access_type = 1;
        CLI_GET_HEX_UINT32_RANGE("Length of the value", data_len, argv[4], 0, LCSH_I2C_BUF_LEN);
        if(cmd_hexstr2uint_array(argv[3],value,data_len))
        {
            cli_out(cli,"Invalid parameter.\n");
            return CLI_ERROR;
        }
        argc_cnt = 5;
    }

    if((argc > argc_cnt) &&(0 == strncmp(argv[argc_cnt], "bridge", 6)))
    {
        CLI_GET_HEX_UINT32_RANGE("addr of the i2c bridge", i2c_bridge_addr, argv[argc_cnt+1], 0, 0xFF);
        CLI_GET_HEX_UINT32_RANGE("the i2c brigde channel", i2c_bridge_channel, argv[argc_cnt+2], 0, 0xF);
        argc_cnt += 3;
        is_bridge = 1;
    }

    if((argc > argc_cnt) &&(0 == strncmp(argv[argc_cnt], "checksum", 8)))
    {
        CLI_GET_HEX_UINT32_RANGE("addr of the i2c bridge", checknum, argv[argc_cnt+1], 0, 16);
        argc_cnt += 2;
    }
    
    if(checknum)
    {
        for (i = 0; i < data_len; i++)
        {
            checksum += value[i];
        }
        value[i++] = (checksum>>8)&0xff;
        value[i++] = checksum&0xff;
        data_len = i;
    }
    
    if (is_bridge)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = (0x1<<i2c_bridge_channel);
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }

    write_req.opcode = REG_I2C_RAW;
    write_req.chip = address;
    write_req.addr = access_type;
    /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
    write_req.addr_1 = (data_len<<8) | (i2c_bus_idx);
    sal_memcpy(write_req.value, value, sizeof(value));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if(access_type)/* print write */
    {
        cli_out (cli, "WRITE:\n");
        for (i = 0; i < data_len; i++)
        {
            cli_out (cli, "\tbyte %2d:\t0x%02x\n", i, write_req.value[i]);
        }
    }
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"transfer i2c fail.\n");
        goto transf_out;
    }
    data_len = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    if(data_len)
        cli_out (cli, "READ:\n");
    for (i = 0; i < data_len; i++)
    {
        cli_out (cli, "\tbyte %2d:\t0x%02x\n", buf[i].addr, buf[i].value);
    }    

transf_out:
    if (is_bridge)
    {
        write_req.opcode = REG_I2C_W;
        write_req.chip = i2c_bridge_addr;
        write_req.addr = 0x0;
        /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
        write_req.addr_1 = (1<<16) | (1<<8) | (i2c_bus_idx);
        write_req.value[0] = 0;
        ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
        if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
        {
            cli_out(cli,"Write i2c bridge fail.\n");
            return CLI_ERROR;
        }        
    }    
    return CLI_SUCCESS;
}

CLI(diag_read_clock,
    diag_read_clock_cmd,
    "read clock <0-7> REG (direct|indirect) (<1-64>|)",
    CLI_READ_DESC,
    CLI_CLOCK_DESC,    
    CLI_CLOCK_IDX_DESC,
    CLI_ADDRESS_DESC,
    CLI_ACCESS_DIRECT_DESC,
    CLI_ACCESS_INDIRECT_DESC,
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 count = 1;
    uint8 indirect = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_regs_reg_t read_req;
    reg_values_t* buf;
    
    CLI_GET_INTEGER_RANGE(CLI_CLOCK_IDX_DESC, chip, argv[0], 0, 0);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFF);
    if (0 == sal_strncmp(argv[2], "direct", 6))
    {
        indirect = 0;
    }
    else
    {
        indirect = 1;
    }
    
    if(argc >= 4)
    {
        CLI_GET_INTEGER_RANGE(CLI_ADDRESS_DESC, count, argv[3], 1, LCAPI_LCM_LCSH_REG_NUM);
    }

    read_req.opcode = REG_CLOCK_R;
    read_req.chip = chip;
    read_req.addr = address;
    read_req.count = count;
    read_req.indirect= indirect;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read clock fail\n");
        return CLI_ERROR;
    }

    count = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < count; i++)
    {
        cli_out (cli, "\t0x%08x:\t0x%08x\n", buf[i].addr, buf[i].value);
    }

    return CLI_ERROR;
}

CLI(diag_write_clock,
    diag_write_clock_cmd,
    "write clock <0-7> REG VALUE (direct|indirect)",
    CLI_WRITE_DESC,
    CLI_CLOCK_DESC,    
    CLI_CLOCK_IDX_DESC,
    CLI_ADDRESS_DESC,    
    CLI_BYTE_VALUE_DESC,
    CLI_ACCESS_DIRECT_DESC,
    CLI_ACCESS_INDIRECT_DESC)
{
    int32 ret = 0;
    uint32 chip = 0;
    uint32 address = 0;
    uint32 value; 
    uint8 indirect = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;
    
    CLI_GET_INTEGER_RANGE(CLI_CLOCK_IDX_DESC, chip, argv[0], 0, 0);
    CLI_GET_HEX_UINT32_RANGE(CLI_ADDRESS_DESC, address, argv[1], 0, 0xFFF);
    CLI_GET_HEX_UINT32_RANGE(CLI_BYTE_VALUE_DESC, value, argv[2], 0, 0xFF);
    if (0 == sal_strncmp(argv[3], "direct", 6))
    {
        indirect = 0;
    }
    else
    {
        indirect = 1;
    }
    

    write_req.opcode = REG_CLOCK_W;
    write_req.chip = chip;
    write_req.addr = address;
    write_req.value[0] = value;
    write_req.indirect= indirect;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write clock fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_set_time,
    diag_set_time_cmd,
    "clock set datetime HH:MM:SS <1-12> <1-31> <2000-2037>",
    "Manage the system clock",
    "Set the time and date",
    "Datetime set",
    "Current time",
    "Month of the year",
    "Day of the month",
    "Year"
)
{
    char year[10] = {0};
    char month[10] = {0};
    char day[10] = {0};
    char hour[10] = {0};
    char min[10] = {0};
    char sec[10] = {0};
    char strcmd[256] = {0};
    int ret = 0;

    if(strlen(argv[0]) != 8)
    {
        cli_out(cli, "Invalid HH:MM:SS\n");
        return CLI_ERROR;
    }
    if(strlen(argv[1]) != 2)
    {
        cli_out(cli, "Invalid month\n");
        return CLI_ERROR;
    }
    if(strlen(argv[2]) != 2)
    {
        cli_out(cli, "Invalid day\n");
        return CLI_ERROR;
    }
    if(strlen(argv[3]) != 4)
    {
        cli_out(cli, "Invalid year\n");
        return CLI_ERROR;
    }

    sal_strncpy(year,  argv[3], 4);
    year[4] = '\0';
    sal_strncpy(month, argv[1], 2);
    month[2] = '\0';
    sal_strncpy(day,   argv[2], 2);
    day[2] = '\0';
    sal_strncpy(hour,  &argv[0][0], 2);
    hour[2] = '\0';
    sal_strncpy(min,   &argv[0][3], 2);
    min[2] = '\0';
    sal_strncpy(sec,   &argv[0][6], 2);
    sec[2] = '\0';

    sal_memset(strcmd, 0, 256);
    sal_sprintf(strcmd, "date -s %s%s%s%s%s%s", month, day, hour, min, sec, year);
    ret = system(strcmd);
    if(ret)
    {
        cli_out(cli, "set clock to system fail!\n");
        return CLI_ERROR;
    }

    sal_memset(strcmd, 0, 256);
    /* modify by wulx for bug 49666, 2019-03-06 */
    sal_sprintf(strcmd, "hwclock -wu");
    ret = system(strcmd);
    if(ret)
    {
        cli_out(cli, "sync clock to rtc fail!\n");
        return CLI_ERROR;
    }
    
    return CLI_ERROR;
}

CLI(diag_show_time,
    diag_show_time_cmd,
    "show clock",
    "show",
    "clock"
)
{
    system("hwclock -r");

    return CLI_ERROR;
}

CLI(diag_read_spi,
    diag_read_spi_cmd,
    "read spi REG REGLEN DATALEN",
    CLI_READ_DESC,
    "SPI",
    "Register in spi component",
    "Register length",
    "Read data length")
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 reg; 
    uint32 reg_len;/* modified by cuixl, from uint8 to uint32*/
    uint32 data_len;/* modified by cuixl, from uint8 to uint32*/
    lcapi_lcm_lcsh_msg_t st_rcv;/* modified by cuixl, from uint8 to uint32*/
    read_regs_reg_t read_req;
    reg_values_t* buf;

    CLI_GET_HEX_UINT32_RANGE("Register", reg, argv[0], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the register", reg_len, argv[1], 0, 0xFFFF);
    CLI_GET_INTEGER_RANGE("Length of the data", data_len, argv[2], 1, LCAPI_LCM_LCSH_REG_NUM);

    read_req.opcode = REG_SPI_R;
    read_req.chip = 0;
    read_req.addr = reg;
    read_req.addr_1 = reg_len;
    read_req.count = data_len;
    
    ret = lcsh_lcm_msg_tx_read_regs(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read IIC fail\n");
        return CLI_ERROR;
    }

    data_len = (st_rcv.msg_len - LCAPI_LCM_LCSH_HEAD_SIZE)/sizeof(reg_values_t);
    buf = (reg_values_t* )(st_rcv.msg);
    for (i = 0; i < data_len; i++)
    {
        cli_out (cli, "\t0x%04x:\t0x%02x\n", buf[i].addr, buf[i].value);
    }

    return CLI_ERROR;
}

CLI(diag_write_spi,
    diag_write_spi_cmd,
    "write spi REG REGLEN DATA DATALEN",
    CLI_WRITE_DESC,
    "SPI",    
    "Register in spi component",
    "Register length",
    "Write data",
    "Data length, unit is byte")
{
    int32 ret = 0;
    uint32 reg; 
    uint32 reg_len;/* modified by cuixl, from uint8 to uint32*/
    uint32 data_len;/* modified by cuixl, from uint8 to uint32*/
    uint32 value; /* modified by cuixl, from uint8 to uint32*/
    
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_regs_reg_t write_req;

    CLI_GET_HEX_UINT32_RANGE("Register", reg, argv[0], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the register", reg_len, argv[1], 0, 0xFFFF);
    CLI_GET_HEX_UINT32_RANGE("Value", value, argv[2], 0, 0xFFFFFFFF);
    CLI_GET_HEX_UINT32_RANGE("Length of the value", data_len, argv[3], 0, 0xFF);

    write_req.opcode = REG_SPI_W;
    write_req.chip = 0;
    write_req.addr = reg;
    /* increase one more parameter "i2c_bus_idx(8-bit variable)", use the last 8-bit of "req_msg->addr_1" */
    write_req.addr_1 = (reg_len<<8) | data_len;
    write_req.value[0] = value;
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    
    ret = lcsh_lcm_msg_tx_write_regs(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Write i2c fail.\n");
        return CLI_ERROR;
    }
    return CLI_SUCCESS;
}

CLI(diag_lcm_bringup_log,
    diag_lcm_bringup_log_cmd,
    "bringup log (enable|disable)",
    CLI_LCM_BRINGUP_LOG,
    "Log",
    "Enable",
    "Disable")
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;

    bringuplog_t bringuplog_req;

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if (0 == strncmp(argv[0], "enable", 6))
    {
        bringuplog_req.enable = 1;
    }
    else
    {
        bringuplog_req.enable = 0;
    }
    
    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_BRINGUP_LOG;
    sal_memcpy(st_req.msg, &bringuplog_req, sizeof(bringuplog_t));
    st_req.msg_len = sizeof(bringuplog_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config bringup log fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

typedef enum 
{
    LCM_CARD = 1<<0,
    LCM_LCMGT = 1<<1,
    LCM_CHSMTLK = 1<<2,
    LCM_LCSH = 1<<3,
    LCM_DRVS = 1<<4,
    LCM_CLIENT= 1<<5,
    LCM_DEBUG_MAX= 1<<6
}E_LCM_DEBUG_TYPE;

CLI(diag_lcm_log_debug,
    diag_lcm_log_debug_cmd,
    "lcm debug (card | lcmgt | chsmtlk | lcsh | drvs | client | debug | all) (enable|disable)",
    CLI_LCM_LOG_DEBUG,
    "Debug",
    "Card",
    "Lcmgt",
    "Chsmtlk",
    "Lcsh",
    "Drvs",
    "Client",
    "Debug",
    "All",
    "Enable",
    "Disable")
{
    int32 ret = 0;

    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;
    
    int32 debug_type = 0;
    lcmdebuglog_t lcmdebuglog_req;

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if (0 == strncmp(argv[0], "card", 4))
    {
        GLB_SET_FLAG(debug_type, LCM_CARD);
    }
    else if (0 == strncmp(argv[0], "lcmgt", 5))
    {
        GLB_SET_FLAG(debug_type, LCM_LCMGT);
    }
    else if (0 == strncmp(argv[0], "chsmtlk", 7))
    {
        GLB_SET_FLAG(debug_type, LCM_CHSMTLK);
    }
    else if (0 == strncmp(argv[0], "lcsh", 4))
    {
        GLB_SET_FLAG(debug_type, LCM_LCSH);
    }
    else if (0 == strncmp(argv[0], "drvs", 4))
    {
        GLB_SET_FLAG(debug_type, LCM_DRVS);
    }
    else if (0 == strncmp(argv[0], "client", 6))
    {
        GLB_SET_FLAG(debug_type, LCM_CLIENT);
    }
    else if (0 == strncmp(argv[0], "debug", 5))
    {
        GLB_SET_FLAG(debug_type, LCM_DEBUG_MAX);
    }    
    else if (0 == strncmp(argv[0], "all", 3))
    {
        GLB_SET_FLAG(debug_type, LCM_CARD | LCM_LCMGT | LCM_CHSMTLK | LCM_LCSH | LCM_DRVS|LCM_CLIENT);
    }
    lcmdebuglog_req.debug_type = debug_type;

    if (0 == strncmp(argv[1], "enable", 6))
    {
        lcmdebuglog_req.enable = 1;
    }
    else
    {
        lcmdebuglog_req.enable = 0;
    }

    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_LCM_DEBUG_LOG;
    sal_memcpy(st_req.msg, &lcmdebuglog_req, sizeof(lcmdebuglog_t));
    st_req.msg_len = sizeof(lcmdebuglog_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config lcm debug log fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

typedef enum 
{
    DRV_BUS_I2C = 1<<0,
    DRV_BUS_SPI = 1<<1,
    DRV_BUS_MDIO = 1<<2,
    DRV_PHY = 1<<3,
    DRV_EPLD = 1<<4,
    DRV_FIBER = 1<<5,
    DRV_SENSOR = 1<<6,
    DRV_GPIO = 1<<7,
    DRV_MUX = 1<<8,
    DRV_FAN = 1<<9,
    DRV_POWER = 1<<10,
    DRV_EEPROM = 1<<11,
    DRV_CLOCK = 1<<12,
    DRV_VCM = 1<<13,
    DRV_LED = 1<<14,
    DRV_LTC2991 = 1<<15,
    DRV_LTC4151 = 1<<16,
    DRV_VSC3308 = 1<<17,
    DRV_SYSENV = 1<<18,
    DRV_POE = 1<<19,
    DRV_DEBUG_MAX = 1<<20
}E_DRV_DEBUG_TYPE;

CLI(diag_drv_log_debug,
    diag_drv_log_debug_cmd,
    "drv debug (i2c | spi | mdio | phy | epld | fiber | sensor | gpio | mux | fan | power | eeprom | clock | vcm | led | ltc2991 | ltc4151 | vsc3308 | sysenv | poe | debug | all ) (enable|disable)",
    CLI_LCM_LOG_DEBUG,
    "Debug",
    "i2c",
    "spi",
    "mdio",
    "phy",
    "epld",
    "fiber",
    "sensor",
    "gpio",
    "mux",
    "fan",
    "power",
    "eeprom",
    "clock",
    "vcm",
    "led",
    "ltc2991",
    "ltc4151",
    "vsc3308",
    "sysenv",
    "poe",
    "debug",
    "All",
    "Enable",
    "Disable")
{
    int32 ret = 0;

    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;
    
    int64 debug_type = 0;
    lcmdebuglog_t lcmdebuglog_req;

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));

    if (0 == strncmp(argv[0], "i2c", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_BUS_I2C);
    }
    else if (0 == strncmp(argv[0], "spi", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_BUS_SPI);
    }
    else if (0 == strncmp(argv[0], "mdio", 4))
    {
        GLB_SET_FLAG(debug_type, DRV_BUS_MDIO);
    }
    else if (0 == strncmp(argv[0], "phy", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_PHY);
    }
    else if (0 == strncmp(argv[0], "epld", 4))
    {
        GLB_SET_FLAG(debug_type, DRV_EPLD);
    }
    else if (0 == strncmp(argv[0], "fiber", 5))
    {
        GLB_SET_FLAG(debug_type, DRV_FIBER);
    }
    else if (0 == strncmp(argv[0], "sensor", 6))
    {
        GLB_SET_FLAG(debug_type, DRV_SENSOR);
    }
    else if (0 == strncmp(argv[0], "gpio", 4))
    {
        GLB_SET_FLAG(debug_type, DRV_GPIO);
    }
    else if (0 == strncmp(argv[0], "mux", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_MUX);
    }
    else if (0 == strncmp(argv[0], "fan", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_FAN);
    }    
    else if (0 == strncmp(argv[0], "power", 5))
    {
        GLB_SET_FLAG(debug_type, DRV_POWER);
    }
    else if (0 == strncmp(argv[0], "eeprom", 6))
    {
        GLB_SET_FLAG(debug_type, DRV_EEPROM);
    }    
    else if (0 == strncmp(argv[0], "clock ", 5))
    {
        GLB_SET_FLAG(debug_type, DRV_CLOCK);
    }
    else if (0 == strncmp(argv[0], "vcm", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_VCM);
    }
    else if (0 == strncmp(argv[0], "led", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_LED);
    }
    else if (0 == strncmp(argv[0], "ltc2991", 7))
    {
        GLB_SET_FLAG(debug_type, DRV_LTC2991);
    }
    else if (0 == strncmp(argv[0], "ltc4151", 7))
    {
        GLB_SET_FLAG(debug_type, DRV_LTC4151);
    }  
    else if (0 == strncmp(argv[0], "vsc3308", 6))
    {
        GLB_SET_FLAG(debug_type, DRV_VSC3308);
    }      
    else if (0 == strncmp(argv[0], "sysenv", 6))
    {
        GLB_SET_FLAG(debug_type, DRV_SYSENV);
    } 
    else if (0 == strncmp(argv[0], "poe", 3))
    {
        GLB_SET_FLAG(debug_type, DRV_POE);
    }    
    else if (0 == strncmp(argv[0], "debug", 5))
    {
        GLB_SET_FLAG(debug_type, DRV_DEBUG_MAX);
    }     
    else if (0 == strncmp(argv[0], "all", 3))
    {
        //GLB_SET_FLAG(debug_type, LCM_CARD | LCM_LCMGT | LCM_CHSMTLK | LCM_LCSH | LCM_DRVS|LCM_CLIENT);
        debug_type = 0xffffffff;
    }
    lcmdebuglog_req.debug_type = debug_type;

    if (0 == strncmp(argv[1], "enable", 6))
    {
        lcmdebuglog_req.enable = 1;
    }
    else
    {
        lcmdebuglog_req.enable = 0;
    }

    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_DRV_DEBUG_LOG;
    sal_memcpy(st_req.msg, &lcmdebuglog_req, sizeof(lcmdebuglog_t));
    st_req.msg_len = sizeof(lcmdebuglog_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config lcm debug log fail.\n");
        return CLI_ERROR;
    }
    
    return CLI_SUCCESS;
}

static uint64_t reg_str2uint64_t(char *str)
{
    uint64_t ret = 0;
    if (strlen(str) > 2 &&  str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        sscanf(str, "%" PRIx64 "\n", &ret);
    }
    else
    {
        sscanf(str, "%" PRIu64 "\n", &ret);
    }
    return ret;
}

CLI(diag_read_soc_reg,
    diag_read_soc_reg_cmd,
    "read soc-reg BASEADDR OFFSET WIDTH COUNT",
    CLI_READ_DESC,
    "soc reg",    
    "base address",
    "offset",
    "reg width bytes",
    CLI_COUNT_DESC)
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    read_soc_reg_t read_req;
    FILE *fp = NULL;
    char buf[128] = {0};

    read_req.baseaddr = reg_str2uint64_t(argv[0]);
    read_req.offset = reg_str2uint64_t(argv[1]);
    read_req.width = reg_str2uint64_t(argv[2]);
    read_req.count = reg_str2uint64_t(argv[3]);
    
    ret = lcsh_lcm_msg_tx_read_soc_reg(&read_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read SOC REG fail\n");
        return CLI_ERROR;
    }

    fp = fopen("/tmp/.soc_out", "r");
    if (fp == NULL)
    {
        cli_out(cli, "Open .soc_out file failed!\n");
        return CLI_ERROR;
    }
    while (fgets(buf, 128, fp) != NULL)
    {
        cli_out(cli, buf);
    }
    fclose(fp);
    
    return CLI_SUCCESS;
}

CLI(diag_write_soc_reg,
    diag_write_soc_reg_cmd,
    "write soc-reg BASEADDR OFFSET WIDTH VALUE",
    CLI_WRITE_DESC,
    "soc reg",    
    "base address",
    "offset",
    "reg width bytes",
    "write value")
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_rcv;
    write_soc_reg_t write_req;
    FILE *fp = NULL;
    char buf[128] = {0};

    write_req.baseaddr = reg_str2uint64_t(argv[0]);
    write_req.offset = reg_str2uint64_t(argv[1]);
    write_req.width = reg_str2uint64_t(argv[2]);
    write_req.value = reg_str2uint64_t(argv[3]);
    
    ret = lcsh_lcm_msg_tx_write_soc_reg(&write_req, &st_rcv);
    if (ret|| LCAPI_LCM_LCSH_OK != st_rcv.errcode)
    {
        cli_out(cli,"Read SOC REG fail\n");
        return CLI_ERROR;
    }

    fp = fopen("/tmp/.soc_out", "r");
    if (fp == NULL)
    {
        cli_out(cli, "Open .soc_out file failed!\n");
        return CLI_ERROR;
    }
    while (fgets(buf, 128, fp) != NULL)
    {
        cli_out(cli, buf);
    }
    fclose(fp);
    
    return CLI_SUCCESS;
}

CLI(diag_config_port_prbs,
    diag_config_port_prbs_cmd,
    "port PORTID prbs MODE (enable|disable|check)",
    "configure port prbs",
    CLI_PORT_IDX_DESC,
    "prbs mode",
    "0-PRBS7_+, 1-PRBS7_-, 2-PRBS15_+, 3-PRBS15_-, 4-PRBS23_+, 5-PRBS23_-, 6-PRBS31_+, 7-PRBS31_-",    
    "enable Tx prbs",
    "disable Tx prbs",
    "check Rx prbs")
{
    int32 ret = 0;
    uint32 port = 0;
    int8 prbs_mode = 0;
    uint8 * buf;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;
    char* pattern[10] = {"PRBS7+","PRBS7-","PRBS15+","PRBS15-","PRBS23+","PRBS23-","PRBS31+","PRBS31-", "PRBS9", "Squareware"};
    char print_buf[128],print_buf1[128];

    conprbs_t con_req;
    
    CLI_GET_INTEGER_RANGE(CLI_PORT_IDX_DESC, port, argv[0], 1, BOARD_MAX_PORT);
    CLI_GET_INTEGER_RANGE("PRBS mode type", prbs_mode, argv[1], 0, 7);

    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&print_buf, 0, sizeof(print_buf));
    sal_memset(&print_buf1, 0, sizeof(print_buf1));
    con_req.port_id = port;
   
    con_req.polynome_type = prbs_mode;
    if(!sal_strcmp(argv[2], "enable"))
    {
        con_req.opcode = CONFIG_PRBS_TX_ENABLE;
    }
    else if(!sal_strcmp(argv[2], "disable"))
    {
        con_req.opcode = CONFIG_PRBS_TX_DISABLE;
    }
    else if(!sal_strcmp(argv[2], "check"))
    {
        con_req.opcode = CONFIG_PRBS_RX_CHECK;
    }

    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_PRBS;
    sal_memcpy(st_req.msg, &con_req, sizeof(conprbs_t));
    st_req.msg_len = sizeof(conprbs_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config prbs fail.\n");
        return CLI_ERROR;
    }

    buf = (uint8* )(st_rcv.msg);
    sal_sprintf(print_buf,"\n%-12s%-6s%-10s%-9s%-9s\n", "Port_ID", "Dir", "Pattern", "Enable", "Result");
    sal_strcat(print_buf,"--------------------------------------------\n");
    
    if(con_req.opcode == CONFIG_PRBS_RX_CHECK)
    {
        cli_out(cli, "PRBS mode %d check %s\n", prbs_mode, buf[0]?"pass":"failed");
        sal_sprintf(print_buf1,"%-12d%-6s%-10s%-9s%-9s\n", port, "RX", pattern[prbs_mode], "  -", buf[0]?"Pass":"Failed");
        cli_out(cli, "%s%s", print_buf,print_buf1);        
    }
    else
    {
         if(con_req.opcode == CONFIG_PRBS_TX_ENABLE)
         {
            sal_sprintf(print_buf1,"%-12d%-6s%-10s%-9s%-9s\n", port, "TX", pattern[prbs_mode], "Yes", "  -");
            cli_out(cli, "%s%s", print_buf,print_buf1);  
         }
         else
         {
            sal_sprintf(print_buf1,"%-12d%-6s%-10s%-9s%-9s\n", port, "TX", pattern[prbs_mode], "No", "  -");
            cli_out(cli, "%s%s", print_buf,print_buf1);  
         }
    }
    return CLI_SUCCESS;
}

enum timer_mask_s
{
    ALL_TIMER=0,
    PORT_TIMER=1,
    FIBER_TIMER=2,
    ENV_TIMER=3,
    PORT_INTR=4
};

CLI(diag_config_timer_ctl,
    diag_config_timer_ctl_cmd,
    "timer-ctl (all |port-timer|fiber-timer|env-timer|port-intr) (enable|disable|show)",
    "configure timer task",
    "all timer configure",
    "port_timer configure",
    "fiber_timer configure",
    "env_timer configure",
    "port-intr configure",
    "enable timer",
    "disable timer",
    "show timer configure"
    )
{
    int32 ret = 0;
    uint32 * buf;
    lcapi_lcm_lcsh_msg_t st_rcv;
    lcapi_lcm_lcsh_msg_t st_req;
    char print_buf[128];

    contimer_t con_req;
    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&st_rcv, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&print_buf, 0, sizeof(print_buf));
    if(!sal_strcmp(argv[0], "all"))
    {
       con_req.timer_type = ALL_TIMER;
    }
    else if(!sal_strcmp(argv[0], "port-timer"))
    {
       con_req.timer_type = PORT_TIMER;
    }
    else if(!sal_strcmp(argv[0], "fiber-timer"))
    {
       con_req.timer_type = FIBER_TIMER;
    }
    else if(!sal_strcmp(argv[0], "env-timer"))
    {
       con_req.timer_type = ENV_TIMER;
    }
    else if(!sal_strcmp(argv[0], "port-intr"))
    {
       con_req.timer_type = PORT_INTR;
    }

    if(!sal_strcmp(argv[1], "enable"))
    {
        con_req.opcode = 1;
    }
    else if(!sal_strcmp(argv[1], "disable"))
    {
        con_req.opcode = 0;
    }
    else if(!sal_strcmp(argv[1], "show"))
    {
        con_req.opcode = 2;
    }

    st_req.msg_type = LCAPI_LCM_LCSH_CONFIG_TIMER;
    sal_memcpy(st_req.msg, &con_req, sizeof(contimer_t));
    st_req.msg_len = sizeof(contimer_t) + LCAPI_LCM_LCSH_HEAD_SIZE;
    ret = lcsh_clnt_send_msg(&st_req, &st_rcv);
    if (ret)
    {
        cli_out(cli, "Config prbs fail.\n");
        return CLI_ERROR;
    }

    buf = (uint32* )(st_rcv.msg);
    sal_sprintf(print_buf,"\n%-12s%-6s\n", "Timer", "Status");
    
    if(con_req.opcode == 2)
    {
        if(con_req.timer_type == 1)
            cli_out(cli, "%s%-12s%-6s\n", print_buf,"port_timer",buf[0]&(1<<PORT_TIMER)?"enable":"disable");
        else if(con_req.timer_type == 2)
            cli_out(cli, "%s%-12s%-6s\n", print_buf,"fiber_timer",buf[0]&(1<<FIBER_TIMER)?"enable":"disable");
        else if(con_req.timer_type == 3)
            cli_out(cli, "%s%-12s%-6s\n", print_buf,"env_timer",buf[0]&(1<<ENV_TIMER)?"enable":"disable");
        else if(con_req.timer_type == 4)
            cli_out(cli, "%s%-12s%-6s\n", print_buf,"port_intr",buf[0]&(1<<PORT_INTR)?"enable":"disable");        
        else
        {
            cli_out(cli, "%s%-12s%-6s\n%-12s%-6s\n%-12s%-6s\n%-12s%-6s\n", print_buf,
                                               "port_timer",buf[0]&(1<<PORT_TIMER)?"enable":"disable",
                                               "fiber_timer",buf[0]&(1<<FIBER_TIMER)?"enable":"disable",
                                               "env_timer",buf[0]&(1<<ENV_TIMER)?"enable":"disable",
                                               "port_intr",buf[0]&(1<<PORT_INTR)?"enable":"disable");
        }
    }
    return CLI_SUCCESS;
}

int32
diag_lcsh_cli_init(struct cli_tree *cli_tree)
{
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_gephy_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_gephy_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_xgphy_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_xgphy_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_clock_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_clock_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_speed_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_duplex_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_enable_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_lb_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_media_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_epld_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_epld_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_fiber_cmd);/* added by tongzb for group xfp/sfp/qsfp 2016-07-08 */
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_fiber_cmd);/* added by tongzb for group xfp/sfp/qsfp 2016-07-08 */
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_fiber_config_wavelength_cmd);/* for bug 43937 */   
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_load_xgphy_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_sensor_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_sensor_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_sensor_temp_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_sensor_temp_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_gpio_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_gpio_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_mux_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_mux_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_fan_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_fan_rpm_cmd);/* tongzb */
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_fan_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_fan_speed_cmd);/* tongzb */
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_fan_speed_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_power_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_power_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_vsc3308_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_vsc3308_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_vsc3308_input_output_pin_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_phy_clock_cmd);
    
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_eeprom_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_eeprom_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_sysenv_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_sysenv_cmd);

    //cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_i2c_bridge_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_i2c_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_i2c_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_transfer_i2c_raw_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_set_time_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_show_time_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_spi_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_spi_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_lcm_bringup_log_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_lcm_log_debug_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_drv_log_debug_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_read_soc_reg_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_write_soc_reg_cmd);

    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_port_prbs_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &diag_config_timer_ctl_cmd);

    return 0;
}
