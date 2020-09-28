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
#include "glb_phy_define.h"
#include "ctclib_show.h"

static char* port_up_state_str(uint32 admin_up, uint32 oper_up, uint32 present)
{
    switch(admin_up)
    {
    case 0:
        return "admin down";

    default:
        if (oper_up)
        {
            return "up";
        }
        else
        {
            return "down";
        }
    }
}

static char* port_duplex_str(uint32 admin_duplex, uint32 oper_duplex)
{
    switch(admin_duplex)
    {
        case GLB_DUPLEX_AUTO:
            {
                switch(oper_duplex)
                {
                    case GLB_DUPLEX_HALF:
                        return "a-half";
                    case GLB_DUPLEX_FULL:
                        return "a-full";
                    default :
                        return "auto";
                }
            }
        case GLB_DUPLEX_HALF:
            return "half";
        case GLB_DUPLEX_FULL:
            return "full";
        default :
            return "Unknown";
    }
}

static char* port_speed_str(uint32 admin_speed, uint32 oper_speed)
{
    switch(admin_speed)
    {
        case GLB_SPEED_AUTO:
            {
                switch(oper_speed)
                {
                    case GLB_SPEED_10M:
                        return "a-10";

                    case GLB_SPEED_100M:
                        return "a-100";

                    case GLB_SPEED_1G:
                        return "a-1000";

                    case GLB_SPEED_2_5G:
                        return "a-2500";

                    case GLB_SPEED_5G:
                        return "a-10000";

                    case GLB_SPEED_10G:
                        return "a-10000";
                        
                    case GLB_SPEED_40G:
                        return "a-40000";
                        
                    case GLB_SPEED_100G:
                        return "a-100000";  
                        
                    default :
                        return "auto";
                }
            }
            
        case GLB_SPEED_10M:
            return "10";

        case GLB_SPEED_100M:
            return "100";

        case GLB_SPEED_1G:
            return "1000";

        case GLB_SPEED_2_5G:
            return "2500";

        case GLB_SPEED_5G:
            return "5000";
            
        case GLB_SPEED_10G:
            return "10000";
            
        case GLB_SPEED_40G:
            return "40000";
            
        case GLB_SPEED_100G:
            return "100000"; 
            
        default :
            return "Unknown";
    }
}

static char* port_loopback_str(uint32 admin_lb)
{
    switch(admin_lb)
    {
        case GLB_LB_PHY_NEAR:
            return "near";

        case GLB_LB_PHY_FAR:
            return "far";

        case GLB_LB_NONE:
            return "none";
            
        default :
            return "Unknown";
    }
}

static char* port_phy_type_str(uint32 is_combo, glb_phy_type_t phy_type)
{
    if (is_combo)
    {
        switch(phy_type)
        {
        case GLB_PHY_TYPE_1000BASE_T:
            return "C-1000BASE_T";   
            
        case GLB_PHY_TYPE_10BASE_T:
            return "C-10BASE_T";

        case GLB_PHY_TYPE_100BASE_TX:
            return "C-100BASE_TX";

        case GLB_PHY_TYPE_BASE_PX:
            return "C-100BASE_PX";

        case GLB_PHY_TYPE_BASE_BX10:
            return "C-100BASE_BX10";
            
        case GLB_PHY_TYPE_100BASE_FX:
            return "C-100BASE_FX";

        case GLB_PHY_TYPE_100BASE_LX:
            return "C-100BASE_LX";
            
        case GLB_PHY_TYPE_1000BASE_T_SFP:
            return "C-1000BASE_T_SFP";
            
        case GLB_PHY_TYPE_1000BASE_CX:
            return "C-1000BASE_CX";    
            
        case GLB_PHY_TYPE_1000BASE_LX:
            return "C-1000BASE_LX";
            
        case GLB_PHY_TYPE_1000BASE_ZX:
            return "C-1000BASE_ZX";        
            
        case GLB_PHY_TYPE_1000BASE_SX:
            return "C-1000BASE_SX";

        case GLB_PHY_TYPE_10GBASE_ER:
            return "C-10GBASE_ER";

        case GLB_PHY_TYPE_10GBASE_LRM:
            return "C-10GBASE_LRM";

        case GLB_PHY_TYPE_10GBASE_LR:
            return "C-10GBASE_LR";
            
        case GLB_PHY_TYPE_10GBASE_SR:
            return "C-10GBASE_SR";

        case GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER:
            return "C-10GBASE_ACTIVE_COPPER";

        case GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER:
            return "C-10GBASE_PASSIVE_COPPER";

        case GLB_PHY_TYPE_40GBASE_CR4:
            return "C-40GBASE_CR4";

        case GLB_PHY_TYPE_40GBASE_SR4:
            return "C-40GBASE_SR4";

        case GLB_PHY_TYPE_40GBASE_LR4:
            return "C-40GBASE_LR4";

        case GLB_PHY_TYPE_40GBASE_XLPPI:
            return "C-40GBASE_XLPPI";

        case GLB_PHY_TYPE_100G_AOC:
            return "C-100G_AOC";

        case GLB_PHY_TYPE_100GBASE_SR4:
            return "C-100GBASE_SR4";

        case GLB_PHY_TYPE_100GBASE_LR4:
            return "C-100GBASE_LR4";

        case GLB_PHY_TYPE_100GBASE_ER4:
            return "C-100GBASE_ER4";

        case GLB_PHY_TYPE_100GBASE_SR10:
            return "C-100GBASE_SR10";

        case GLB_PHY_TYPE_100G_CWDM4_MSA_FEC:
            return "C-100G_CWDM4_MSA_FEC";

        case GLB_PHY_TYPE_100G_PSM4_SMF:
            return "C-100G_PSM4_SMF";

        case GLB_PHY_TYPE_100G_ACC:
            return "C-100G_ACC";

        case GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC:
            return "C-100G_CWDM4_MSA_NOFEC";

        case GLB_PHY_TYPE_100GBASE_CR4:
            return "C-100GBASE_CR4";

        case GLB_PHY_TYPE_40GBASE_ER4:
            return "C-40GBASE_ER4";

        case GLB_PHY_TYPE_4X10GBASE_SR:
            return "C-4X10GBASE_SR";

        case GLB_PHY_TYPE_40G_PSM4_SMF:
            return "C-40G_PSM4_SMF";

        case GLB_PHY_TYPE_2G5BASE_T:
            return "C-2.5GBASE_T";

        case GLB_PHY_TYPE_5GBASE_T:
            return "C-5GBASE_T";

        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H:
            return "C-25GAUI_C2M_AOC_BER_H";
        
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L:
            return "C-25GAUI_C2M_AOC_BER_L";
        
        case GLB_PHY_TYPE_25GBASE_SR:
            return "C-25GBASE_SR";
        
        case GLB_PHY_TYPE_25GBASE_LR:
            return "C-25GBASE_LR";
        
        case GLB_PHY_TYPE_25GBASE_ER:
            return "C-25GBASE_ER";
        
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H:
            return "C-25GAUI_C2M_ACC_BER_H";
            
        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L:
            return "C-25GAUI_C2M_ACC_BER_L";            
        
        case GLB_PHY_TYPE_25GBASE_CR_L:
            return "C-25GBASE_CR_CA_L";
            
        case GLB_PHY_TYPE_25GBASE_CR_S:
            return "C-25GBASE_CR_CA_S";
            
        case GLB_PHY_TYPE_25GBASE_CR_N:
            return "C-25GBASE_CR_CA_N";
            
        case GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE:
            return "C-25GBASE_CR_PASSIVE_CABLE";     


        default :
            return "C-Unknown";
        }
    }
    else
    {
        switch(phy_type)
        {
        case GLB_PHY_TYPE_1000BASE_T:
            return "1000BASE_T";  
            
        case GLB_PHY_TYPE_10BASE_T:
            return "10BASE_T";

        case GLB_PHY_TYPE_100BASE_TX:
            return "100BASE_TX";

        case GLB_PHY_TYPE_BASE_PX:
            return "100BASE_PX";

        case GLB_PHY_TYPE_BASE_BX10:
            return "100BASE_BX10";
            
        case GLB_PHY_TYPE_100BASE_FX:
            return "100BASE_FX";

        case GLB_PHY_TYPE_100BASE_LX:
            return "100BASE_LX";
            
        case GLB_PHY_TYPE_1000BASE_T_SFP:
            return "1000BASE_T_SFP";
            
        case GLB_PHY_TYPE_1000BASE_CX:
            return "1000BASE_CX";    
            
        case GLB_PHY_TYPE_1000BASE_LX:
            return "1000BASE_LX";

        case GLB_PHY_TYPE_1000BASE_ZX:
            return "1000BASE_ZX";
            
        case GLB_PHY_TYPE_1000BASE_SX:
            return "1000BASE_SX";

        case GLB_PHY_TYPE_10GBASE_ER:
            return "10GBASE_ER";

        case GLB_PHY_TYPE_10GBASE_LRM:
            return "10GBASE_LRM";

        case GLB_PHY_TYPE_10GBASE_LR:
            return "10GBASE_LR";
            
        case GLB_PHY_TYPE_10GBASE_SR:
            return "10GBASE_SR";

        case GLB_PHY_TYPE_10GBASE_ACTIVE_COPPER:
            return "10GBASE_ACTIVE_COPPER";

        case GLB_PHY_TYPE_10GBASE_PASSIVE_COPPER:
            return "10GBASE_PASSIVE_COPPER";

        case GLB_PHY_TYPE_40GBASE_CR4:
            return "40GBASE_CR4";

        case GLB_PHY_TYPE_40GBASE_SR4:
            return "40GBASE_SR4";

        case GLB_PHY_TYPE_40GBASE_LR4:
            return "40GBASE_LR4";

        case GLB_PHY_TYPE_40GBASE_XLPPI:
            return "40GBASE_XLPPI";

        case GLB_PHY_TYPE_100G_AOC:
            return "100G_AOC";

        case GLB_PHY_TYPE_100GBASE_SR4:
            return "100GBASE_SR4";

        case GLB_PHY_TYPE_100GBASE_LR4:
            return "100GBASE_LR4";

        case GLB_PHY_TYPE_100GBASE_ER4:
            return "100GBASE_ER4";

        case GLB_PHY_TYPE_100GBASE_SR10:
            return "100GBASE_SR10";

        case GLB_PHY_TYPE_100G_CWDM4_MSA_FEC:
            return "100G_CWDM4_MSA_FEC";

        case GLB_PHY_TYPE_100G_PSM4_SMF:
            return "100G_PSM4_SMF";

        case GLB_PHY_TYPE_100G_ACC:
            return "100G_ACC";

        case GLB_PHY_TYPE_100G_CWDM4_MSA_NOFEC:
            return "100G_CWDM4_MSA_NOFEC";

        case GLB_PHY_TYPE_100GBASE_CR4:
            return "100GBASE_CR4";

        case GLB_PHY_TYPE_40GBASE_ER4:
            return "40GBASE_ER4";

        case GLB_PHY_TYPE_4X10GBASE_SR:
            return "4X10GBASE_SR";

        case GLB_PHY_TYPE_40G_PSM4_SMF:
            return "40G_PSM4_SMF";

        case GLB_PHY_TYPE_2G5BASE_T:
            return "2.5GBASE_T";

        case GLB_PHY_TYPE_5GBASE_T:
            return "5GBASE_T";

        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_H:
            return "25GAUI_C2M_AOC_BER_H";
            
        case GLB_PHY_TYPE_25GAUI_C2M_AOC_BER_L:
            return "25GAUI_C2M_AOC_BER_L";            

        case GLB_PHY_TYPE_25GBASE_SR:
            return "25GBASE_SR";

        case GLB_PHY_TYPE_25GBASE_LR:
            return "25GBASE_LR";

        case GLB_PHY_TYPE_25GBASE_ER:
            return "25GBASE_ER";

        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_H:
            return "25GAUI_C2M_ACC_BER_H";

        case GLB_PHY_TYPE_25GAUI_C2M_ACC_BER_L:
            return "25GAUI_C2M_ACC_BER_L";            

        case GLB_PHY_TYPE_25GBASE_CR_L:
            return "25GBASE_CR_CA_L";
            
        case GLB_PHY_TYPE_25GBASE_CR_S:
            return "25GBASE_CR_CA_S";
            
        case GLB_PHY_TYPE_25GBASE_CR_N:
            return "25GBASE_CR_CA_N";
            
        case GLB_PHY_TYPE_25GBASE_PASSIVE_CABLE:
            return "25GBASE_CR_PASSIVE_CABLE";

        default :
            return "Unknown";
        }
    }
}

CLI(lc_show_lc,
    lc_show_lc_cmd,
    "show test",
    "Show running information",
    "just test")
{
    int32 ret = 0;
    lcapi_lcm_lcsh_test1_msg_t lcapi_lcm_lcsh_test1_msg;

    sal_memset(&lcapi_lcm_lcsh_test1_msg, 0, sizeof(lcapi_lcm_lcsh_test1_msg));
    ret = lcsh_lcm_msg_tx_test1(&lcapi_lcm_lcsh_test1_msg);
    if (ret)
    {
        cli_out(cli,"get info fail :( -> %d", ret);
    }
    cli_out(cli,"int = %-9d, string = %-15s\n",lcapi_lcm_lcsh_test1_msg.int_value, lcapi_lcm_lcsh_test1_msg.str_value);    
    return CLI_SUCCESS;
}

CLI(show_port,
    show_port_cmd,
    "show port",
    "Show running information",
    "Line card")
{
    int32 ret = 0;
    lcapi_lcm_lcsh_msg_t st_req;
    showport_t rcv_rcv;
    int32 port_num, ii;
    portdesc_t* port_desc;
    
    sal_memset(&st_req, 0, sizeof(lcapi_lcm_lcsh_msg_t));
    sal_memset(&rcv_rcv, 0, sizeof(showport_t));
    
    rcv_rcv.msg = sal_malloc(4096);
    
    ret = lcsh_lcm_msg_tx_get_port_info(&st_req, &rcv_rcv);
    if (ret)
    {
        cli_out(cli,"get info fail :( -> %d", ret);
        sal_free(rcv_rcv.msg);
        return CLI_ERROR;
    }
    port_num = rcv_rcv.port_num;
    port_desc = (portdesc_t* )rcv_rcv.msg;
    
    cli_out(cli,"%-9s %-10s %-7s %-7s %-8s %-12s %-16s\n",
        "Port(P-B)",
        "Status",
        "Duplex",
        "Speed",
        "Loopback",
        "Type",
        "Desc");
    
    cli_out(cli, "======================================================================\n");

    for(ii = 0; ii < port_num; ii++)
    {
        cli_out(cli,"%-2d - %-2d %-10s %-7s %-7s %-8s %-12s\n",
            port_desc[ii].portno,port_desc[ii].mac_id,
            port_up_state_str(port_desc[ii].admin.up, port_desc[ii].curstatus.up, port_desc[ii].present),
            port_duplex_str(port_desc[ii].admin.duplex, port_desc[ii].curstatus.duplex),
            port_speed_str(port_desc[ii].admin.speed, port_desc[ii].curstatus.speed),
            port_loopback_str(port_desc[ii].admin.loopback),
            port_phy_type_str(port_desc[ii].is_combo, port_desc[ii].phy_type));
    }

    sal_free(rcv_rcv.msg);
    
    return CLI_SUCCESS;
}

CLI(lc_show_transceivers_info,
    lc_show_transceivers_info_cmd,
    "show transceiver (detail|)",
    CLI_SHOW_STR,
    "Transceiver",
    "Detail information")
{
    int ret = 0;
    uint8 detail = 0;
    
    if (argc == 0)
    {
        detail = 0;  
    }
    else if (argc == 1)
    {
        if (!sal_strncmp (argv[0], "detail", 6))
        {
            detail = 1;
        }
    }
    ret = lcsh_lcm_msg_tx_show_transceiver(detail);    
    if (ret)
    {
        cli_out(cli,"Show transceiver failed.\n");
    }
    CTCLIB_DUMP_FINISH(cli);
    return CLI_SUCCESS;
}   

int
lcsh_cli_init(struct cli_tree *cli_tree)
{
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &lc_show_lc_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &show_port_cmd);
    cli_install_gen(cli_tree, EXEC_MODE, PRIVILEGE_NORMAL, 0, &lc_show_transceivers_info_cmd);

    return 0;
}

