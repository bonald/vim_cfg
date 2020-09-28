
/****************************************************************************
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Centec
 * Date         :       2015-08-06
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "chsm_inc.h"

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
#define CHSM_ROUTINE_TIME_INTERVAL              3000
#define CHSM_COLD_SYS_INFO_TIME_INTERVAL      600000  /* 10 minutes */

//static ctc_task_t *g_chsm_routine_timer = NULL;
/* polling system running info for online diagnositc, like CPU and memory  usage */
//static ctc_task_t *g_chsm_cold_sys_info_timer = NULL;/* for bug 45620, disable cold log */
static uint32 g_chsm_start_phase2 = FALSE;
/****************************************************************************
 *  
* Function
*
*****************************************************************************/

extern int32
chsm_mgt_state_machine_flag_init();

#if 0 /* for bug 45620, disable cold log */
static void
_chsm_cold_sys_info_timer_handle()
{    
    g_chsm_cold_sys_info_timer = ctc_task_add_timer_msec(TASK_PRI_LOW, _chsm_cold_sys_info_timer_handle, 
                                                NULL, CHSM_COLD_SYS_INFO_TIME_INTERVAL);
    cold_item_sys_info_monitor();
}
#endif

static int32
_chsm_get_current_bootimage()
{
    tbl_bootimage_t boot;
    char ary_tmp[M_FULLPATH_MAX_LEN];
    char cur_tmp[M_FULLPATH_MAX_LEN] = {'\0'};
    char *chr_p = NULL;
    char *chr_p1 = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&boot, 0, sizeof(boot));

#ifdef _CTC_X86_64_
    fp = sal_fopen(CHSM_DEBIAN_BOOT_CONFIG_FILE, "r");
    if (!fp)
    {
        sal_strncpy(boot.current_image, "UNKNOWN", M_FULLPATH_MAX_LEN);
        tbl_bootimage_set_bootimage_field(&boot, TBL_BOOTIMAGE_FLD_CURRENT_IMAGE);
        return PM_E_FAIL;
    }
    sal_fgets(ary_tmp, M_FULLPATH_MAX_LEN, fp);
    
    if ((chr_p = sal_strstr(ary_tmp, CHSM_DEBIAN_UIMAGE_PATH)) != NULL)
    {
        rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, cur_tmp, M_FULLPATH_MAX_LEN);
        if (rc < 0)
        {
            sal_strncpy(boot.current_image, "UNKNOWN", M_FULLPATH_MAX_LEN);
        }
        else
        {
            sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "%s/%s",
                M_BOOT_IMAGE_PATH_SHOW, cur_tmp);
        }
    }
    else if ((chr_p = sal_strstr(ary_tmp, "tftp://")) != NULL)
    {
        sal_strncpy(boot.current_image, "TftpFile://", M_FULLPATH_MAX_LEN);
        chr_p += sal_strlen("tftp://");
        if ((chr_p1 = sal_strstr(chr_p, "/")) != NULL)
        {
            chr_p1[0] = ':';
        }
        lib_strip_newline(chr_p);
        sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "TftpFile://%s", chr_p);
    }
#else
    (void)chr_p1;

    fp = sal_fopen("/proc/cmdline", "r");
    if (!fp)
    {
        sal_strncpy(boot.current_image, "tftp://10.10.38.160/chenyqfx.r", M_FULLPATH_MAX_LEN);
        tbl_bootimage_set_bootimage_field(&boot, TBL_BOOTIMAGE_FLD_CURRENT_IMAGE);
        return PM_E_FAIL;
    }
    sal_fgets(ary_tmp, M_FULLPATH_MAX_LEN, fp);

    if ((chr_p = sal_strstr(ary_tmp, "bootimage=default")) != NULL)
    {
        rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, cur_tmp, M_FULLPATH_MAX_LEN);
        if (rc < 0)
        {
            sal_strncpy(boot.current_image, "UNKNOWN", M_FULLPATH_MAX_LEN);
        }
    }
    else if((chr_p = sal_strstr(ary_tmp, "bootimage=uImage")) != NULL )
    {
        rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, cur_tmp, M_FULLPATH_MAX_LEN);
        if(rc < 0)
        {
            sal_strncpy(boot.current_image, "uImage", M_FULLPATH_MAX_LEN);
        }
    }
    else
    {
        chr_p = NULL;
        chr_p = sal_strstr(ary_tmp, "bootimage=");

        if (chr_p != NULL)
        {
            chr_p += sal_strlen("bootimage=");
            sal_strncpy(cur_tmp, chr_p, M_FULLPATH_MAX_LEN);
            chr_p = cur_tmp;
            if ((chr_p = sal_strchr(chr_p, ' ')) != NULL)
            {
                *chr_p = CMD_ZERO_CHAR;
            }
        }
        else
        {
            sal_strncpy(boot.current_image, "UNKNOWN", M_FULLPATH_MAX_LEN);
        }
    }
    if (sal_strncmp(boot.current_image, "UNKNOWN", sal_strlen("UNKNOWN")))
    {
        if (!sal_strncasecmp(cur_tmp, "TftpFile", sal_strlen("TftpFile")))
        {
            sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "%s", cur_tmp);
        }
        else if (!sal_strncasecmp(cur_tmp, "Tftp6File", sal_strlen("Tftp6File")))
        {
            sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "%s", cur_tmp);
        }
        else if(!sal_strncasecmp(cur_tmp, "ConsoleFile", sal_strlen("ConsoleFile")))
        {
            sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "%s", cur_tmp);
        }
        else
        {
            sal_snprintf(boot.current_image, M_FULLPATH_MAX_LEN, "%s/%s",
                M_BOOT_IMAGE_PATH_SHOW, cur_tmp);
        }
    }
#endif

    sal_fclose(fp);
    rc = tbl_bootimage_set_bootimage_field(&boot, TBL_BOOTIMAGE_FLD_CURRENT_IMAGE);

    return rc;
}

static int32 
_chsm_manage_if_init()
{
#ifdef _KERNEL_VER_2_6_
    int32 vrfid = CHSM_MANG_VRF;
    manage_if_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (manage_if_fd < 0)
    {
        return CHSM_E_INIT_FAILED;
    }
    
    sal_setsockopt(manage_if_fd, SOL_SOCKET, SO_VRF, &vrfid, sizeof(vrfid));
#else
#endif    
    return PM_E_NONE;
}


int32
chsm_set_default_tmpr_threshold(void)
{

    tbl_card_t *p_db_card = NULL;
    
    p_db_card = tbl_card_get_default_card();
    
    p_db_card->tmpr_low = CHSM_DEFAULT_LOW_TMPR;
    p_db_card->tmpr_high = CHSM_DEFAULT_HIGH_TMPR;
    p_db_card->tmpr_critical = CHSM_DEFAULT_CRIT_TMPR;
    
    if (module_control_is_enable_stack())
    {
#if 0    
        for (i = 1; i < p_chassis->lc_num; i ++)
        {
        }
#endif
    }
    else
    {
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_TMPR_LOW);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_TMPR_HIGH);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_TMPR_CRITICAL);
    }

    return PM_E_NONE;
}

int32
chsm_set_default_fan_speed(void)
{

    tbl_card_t *p_db_card = NULL;
    
    p_db_card = tbl_card_get_default_card();
    
    p_db_card->fan_speed_bottom= DEFAULT_FAN_RATE_BOTTOM;
    p_db_card->fan_speed_low= DEFAULT_FAN_RATE_LOW;
    p_db_card->fan_speed_high= DEFAULT_FAN_RATE_HIGH;
    p_db_card->fan_speed_full= DEFAULT_FAN_RATE_FULL;
    
    if (module_control_is_enable_stack())
    {
#if 0    
        for (i = 1; i < p_chassis->lc_num; i ++)
        {
        }
#endif
    }
    else
    {
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_FAN_SPEED_BOTTOM);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_FAN_SPEED_LOW);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_FAN_SPEED_HIGH);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_FAN_SPEED_FULL);
    }

    return PM_E_NONE;
}

int32
chsm_set_default_sys_poe(void)
{

    tbl_card_t *p_db_card = NULL;
    
    p_db_card = tbl_card_get_default_card();
    
    p_db_card->glb_poe_enable = DEFAULT_POE_ENALBE;
    p_db_card->glb_poe_power_max = DEFAULT_POE_POWER_MAX;
    
    if (module_control_is_enable_stack())
    {
#if 0    
        for (i = 1; i < p_chassis->lc_num; i ++)
        {
        }
#endif
    }
    else
    {
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_GLB_POE_ENABLE);
        tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_GLB_POE_POWER_MAX);
    }

    return PM_E_NONE;
}


static int32 
_chsm_load_default_cfg()
{
    tbl_chassis_t *p_db_chassis = tbl_chassis_get_chassis();
    tbl_card_t     *p_db_card = tbl_card_get_default_card();
    char sys_cmd[256];

    char mac_str[MAC_ADDR_STR_LEN];
    p_db_chassis->mac_num = p_db_card->port_num + 1; /* add 1 for route-mac */

    /* set route-mac */
    sal_snprintf(sys_cmd, 256, "cdbctl update/cdb/sys/sys_global/route_mac/%s/mac_num/%u",
        cdb_mac_val2str(mac_str, MAC_ADDR_STR_LEN, p_db_chassis->sys_mac), 
        p_db_chassis->mac_num);
    system(sys_cmd);

#ifndef TAPPRODUCT
    /* add instance 0 */
    sal_snprintf(sys_cmd, 256, "cdbctl create/cdb/l2/mstp_instance/0");
    system(sys_cmd);

/* modified by liwh for bug 54358, 2019-11-13 
   hybrid use instance 64 (HSRV_OF_INSTANCE), system use 0-63, needn't add instance 1 */
//#ifdef OFPRODUCT
    /* add instance 1 */
    /*sal_snprintf(sys_cmd, 256, "cdbctl create/cdb/l2/mstp_instance/1");
    system(sys_cmd);
#endif*/
/* liwh end */

    /* L2+ and Openflow need vlan 1 */
    /* create default vlan 1 */
    sal_snprintf(sys_cmd, 256, "cdbctl create/cdb/l2/vlan/1");
    system(sys_cmd);
#endif /* !TAPPRODUCT */

    chsm_set_default_tmpr_threshold();
    chsm_set_default_fan_speed();
    chsm_set_default_sys_poe();
    
#ifdef _GLB_UML_SYSTEM_ 
{
    tbl_fan_t fan;
    tbl_psu_t psu;
    tbl_sensor_t sensor;

    sal_memset(&fan, 0, sizeof(fan));
    fan.key.id = 1;
    fan.slot = 0;
    fan.tray = 1;
    fan.percent = 50;
    fan.absent = 0;
    sal_strcpy(fan.status,"OK");
    sal_strcpy(fan.direction, "F2B");
    sal_strcpy(fan.name,"FAN1");
    sal_strcpy(fan.mode,"AUTO");
    tbl_fan_add_fan(&fan);

    sal_memset(&fan, 0, sizeof(fan));
    fan.key.id = 2;
    fan.slot = 1;
    fan.tray = 1;
    fan.percent = 95;
    fan.absent = 0;
    sal_strcpy(fan.status,"OK");
    sal_strcpy(fan.direction, "F2B");
    sal_strcpy(fan.name,"FAN2");
    sal_strcpy(fan.mode,"AUTO");
    tbl_fan_add_fan(&fan);

    sal_memset(&psu, 0, sizeof(psu));
    psu.absent = 0;
    psu.slot = 0;
    psu.key.id = 2;
    sal_strcpy(psu.name,"PSU2");
    sal_strcpy(psu.alert_status,"NO ALERT");
    sal_strcpy(psu.run_status,"OK");
    sal_strcpy(psu.mode,"DC48V");
    tbl_psu_add_psu(&psu);
    
    sal_memset(&psu, 0, sizeof(psu));
    psu.absent = 0;
    psu.slot = 1;
    psu.key.id = 5;
    sal_strcpy(psu.name,"PSU2");
    sal_strcpy(psu.alert_status,"NO ALERT");
    sal_strcpy(psu.run_status,"OK");
    sal_strcpy(psu.mode,"DC48V");
    tbl_psu_add_psu(&psu);

    sal_memset(&sensor, 0, sizeof(sensor));
    sensor.key.id  = 3;
    sensor.slot = 0;
    sensor.temperature = 100;
    sal_strcpy(sensor.name,"SENSOR3");
    sal_strcpy(sensor.position,"CHIP1");
    tbl_sensor_add_sensor(&sensor);

    sal_memset(&sensor, 0, sizeof(sensor));
    sensor.key.id  = 5;
    sensor.slot = 0;
    sensor.temperature = 200;
    sal_strcpy(sensor.name,"SENSOR5");
    sal_strcpy(sensor.position,"BOARD1");
    tbl_sensor_add_sensor(&sensor);
}
    
#endif


    return PM_E_NONE;
}

static int32
_chsm_start_phase1()
{
    CHSM_DEBUG(CHSM_CHASSIS, "CHSM start phase1 begin!");

    chsm_mgt_state_machine_flag_init();
    _chsm_manage_if_init();

    _chsm_get_current_bootimage();

    return PM_E_NONE;
}

int32
chsm_start_phase2()
{
    if (g_chsm_start_phase2)
    {
        return PM_E_NONE;
    }
    g_chsm_start_phase2 = TRUE;
    
    CHSM_DEBUG(CHSM_CHASSIS, "CHSM start phase2 begin!");

    _chsm_load_default_cfg();

    chsm_stm_mgr_init();

    /* for bug 45620, disable cold log */
    //_chsm_cold_sys_info_timer_handle();

    CHSM_DEBUG(CHSM_CHASSIS, "CHSM start phase2 end!");

    return PM_E_NONE;
}


int32
chsm_init(uint16 daemon_mode)
{ 
    lib_master_t *lib_master = lib_init(PM_ID_CHSM);
    lib_master->task_mst = ctc_task_init();
    chsm_master_t *master = (chsm_master_t*)XCALLOC(MEM_PM_CHSM_MASTER, sizeof(chsm_master_t));
    lib_master->proto_mst = master;
#ifdef LDB_SYNC_TO_RDB
    rdb_connect();
#endif
    ipc_connect_bhm();

    /* client to CDS */
    ctc_sock_register_rx_fn(chsm_svr_process_msg);
    ipc_connect_ccs_cds(10);
    
    //chsm_svr_init();
    ipc_connect_snmp_trap();

    /* for bug 45620, disable cold log */
    //cold_init();
    chsm_cmd_init();
    chsm_debug_start();

    _chsm_start_phase1(master);
    ctc_task_main();
   
    return PM_E_NONE;
}

