/**
 @file glb_macro.h
 
 @date 2010-07-01
 
 @version v1.0
 
The file define  all CNOS module's  common Macros .

*/
    
#ifndef __GLB_TEMPFILE_DEFINE_H__
#define __GLB_TEMPFILE_DEFINE_H__

#define GLB_MEMCHK_STATE_FILE           "/tmp/memchk_state"

/*Hareware related temporary file define*/
#define GLB_CHASSIS_TYPE_FILE         "/tmp/ctcos_chassis_type"
#define GLB_BOARD_INFO_FILE           "/tmp/ctcos_board_info"

/* IF_SPLITTING */
#define GLB_PORT_INFO_FILE           "/tmp/ctcos_port_info"
#define GLB_PORT_CLI_INFO_FILE           "/tmp/ctcos_port_cli_info"
#define GLB_UML_PORT_INFO_FILE           "/mnt/flash/glb_port_info"

#define GLB_HAL_HSRV_SOCKET_PATH               "/tmp/.shal_msg"
#define GLB_HAL_HSRV_HI_PRIO_SOCKET_PATH       "/tmp/.shal_hi_prio_msg"

#define GLB_CHSM_LCM_SOCK_PATH          "/tmp/.chsm_lcm_ctrl"

#define GLB_CLI_SOCK_PATH               "/tmp/.klish_cli"

#define GLB_LCM_LCSH_SOCK_PATH          "/tmp/.lcm_lcsh_ctrl"

#define GLB_SERIAL_ATTR                 "/tmp/.serial_attr"

/*Global hsrv done file define*/
#define GLB_HSRV_DONE_FILE      "/tmp/glb_hsrv_done"
#define GLB_SDK_DONE_FILE       "/tmp/glb_sdk_done"
#define STARTUP_CFG_DONE_FILE   "/tmp/startup_cfg_done"

#define RPCAPI_COUNTER_FILE      "/tmp/rpcapi_counter"

/*file for dump*/
#define GLB_DUMP_FILE   "/tmp/dumpfile"

#define GLB_SNMP_EN_FILE    "/tmp/snmp_en"
#define GLB_WEBUI_EN_FILE   "/tmp/webui_en"

/*file for peth dump*/
#define GLB_PETH_DUMP_FILE "/tmp/.glb_peth_dump_file"

/*file for IPC cli parameter*/
#define GLB_CLI_SHOW_OUT_FILE "/tmp/ipc_cli_show_out"
#define GLB_PARA_DUMP_FILE "/tmp/ipc_para_dump"

/*file for EPLD update*/
#define GLB_UPDATE_EPLD_FLAG        "/mnt/flash/glb_update_epld"
#define GLB_UPDATE_EPLD_DONE        "/tmp/glb_update_epld_done"
#define GLB_UPDATE_EPLD_ERASE_DONE  "/tmp/glb_update_epld_erase_done"
#define GLB_UPDATE_EPLD_PROG_DONE        "/tmp/glb_update_epld_prog_done"

#define GLB_PETH_IPC_SOCKET_PATH               "/tmp/.peth_ipc_msg"
/*file for bootrom update*/
#define GLB_UPDATE_BOOTROM_DONE        "/tmp/glb_update_bootrom_done"

#define GLB_LOAD_PHY_FIRMWARE_DONE        "/tmp/glb_load_phy_firmware_done"
#define GLB_LOAD_PHY_FIRMWARE_FAIL        "/tmp/glb_load_phy_firmware_fail"

#ifdef MANU_DIAG
/* for manufacture */
#define DIAG_STR_BOOTUP_LOG_PATH "/tmp/bootup_diagnostic.log"
#define DIAG_MANU_LOG_PATH    "/mnt/flash/cold"
#endif

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
#define GLB_SMARTCFG_FILE               "/tmp/glb_scfg_file"
#endif /* !HAVE_SMARTCFG */

#define GLB_POE_PORT_CFG_DONE_NAME "/tmp/glb_poe_port_cfg_done"
#define GLB_POE_SYS_CFG_DONE_NAME "/tmp/glb_poe_sys_cfg_done"
/*jqiu added for storing eth No. info, to adapt different board. bug 24393. 2013-08-10*/
#define GLB_CPU_IF_INFO_FILE "/tmp/.glb_cpu_if_info"

#define GLB_DUMP_HW_FDB_FILE      "/tmp/glb_dump_hw_fdb_file"
/*fix bug32715, liangf, 2015-04-15*/
#define GLB_BOOT_CMD_INFO_FILE  "/tmp/glb_boot_cmd_info_file"

/*file for cnos_config*/
#define GLB_CNOS_CONFIG_FILE   "/tmp/cnos_config_file"

/* add by wangyb for bug 44518, 2017-08-07*/
#define GLB_CONFIG_MGMT_INFO  "/tmp/cnos_config_mgmt_info"
#define GLB_CONFIG_MGMT_LINKLOCAL "/tmp/cnos_config_mgmt_linklocal"
#define GLB_CONFIG_MGMT_ETHTOOL "/tmp/cnos_config_mgmt_ethtool"
#define GLB_MGMT_ARP_INFO       "/tmp/cnos_mgmt_arp_info"
#define GLB_MGMT_NEIGHBOR_INFO       "/tmp/cnos_mgmt_neighbor_info"
#define GLB_CONFIG_MGMT_IPV4_ADDR "/tmp/cnos_config_mgmt_ipv4_addr"
#define GLB_CONFIG_MGMT_IPV6_ADDR "/tmp/cnos_config_mgmt_ipv6_addr"

#define WB_RESTORING_FILE       "/tmp/wb_restoring"

#endif /*!__GLB_TEMPFILE_DEFINE_H__*/
