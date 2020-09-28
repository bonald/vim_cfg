/****************************************************************************
 * lcm_chsmtlk.c:   lcm communicate with chassis manager 
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       tongzb
 * Date         :       2018-02-11
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#ifdef CNOS_OS
#include "sai.h"
#include "ctc_sai_common.h"
#include "laiinc.h"
#else
#include "sal_common.h"
#endif
#include "glb_hw_define.h"
#include "lcapi.h"
#include "lcm_client.h"
#include "lcm_mgt.h"
#include "lcm_msg.h"
#include "lcm_card.h"
#include "lcm_error.h"
#include "lcm_debug.h"
#include "lcm_chsmtlk.h"
#include "lcm_hagttlk.h"
#include "ctc_pci.h"
#include "eeprom_api.h"
#include "glb_stm_define.h"
#include "led_api.h"
#include "sensor_api.h"
#include "sensor_drv.h"
#include "epld_api.h"
#include "power_api.h"
#include "fan_api.h"
#include "mux_api.h"
#include "bootrom_drv.h"
#include "ctclib_show.h"
#include "fiber_api.h"
#include "epld_api.h"
#include "phy_api.h"
#include "lcm_log.h"
#include "dal.h"
#include "sysenv_api.h"
#include "poe_api.h"

#include "glb_distribute_system_define.h"

#include "glb_if_define.h"
#include "ctclib_sys_cmd.h"

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
card_init_callback_t lcm_card_init_cb;
int g_bDebug = 0;   /* debug flag : will not do keepalive if set */
/* Modified by liuht for bug 26911, 2014-03-27 */
int g_sys_led_alarm = 0;

#ifndef CNOS_OS
extern ctclib_thread_t* g_p_timer_chk_rx_keepalive;
void lcm_chk_rx_keepalive_cb(void* p_data);
LCM_DEFINE_TASK_ENCAP(lcm_chk_rx_keepalive_cb);
#endif
extern int32 lcm_msg_tx(LcmMsg_t *msg,int tx_attribute);
extern int32 lcm_notify_fiber_info(glb_port_t* p_port, lcm_fiber_notify_event_e notify_flag);

/****************************************************************************
 *  
* Function
*
*****************************************************************************/
int32 
lcm_chsm_msg_tx_lcAttach(lcm_clnt_t* clnt)
{
#ifndef CNOS_OS
    LcmMsg_t msg;
    int32 ret = 0;
    glb_card_t* p_card = NULL;
    uint32 stm_mode = 0;
    int32 stm_ext_flag = 0;
    int32 i = 0;
    lcapi_hagt_get_sdk_profile_t lcapi_sdk_profile;
    struct SDKTableType *p_msg_item = NULL;
    
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm send lcAttach : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    lcm_mgt_get_sw_info(p_card);
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    sal_memset(&lcapi_sdk_profile, 0, sizeof(lcapi_sdk_profile));

    msg.op.present = op_PR_lcAttach;
    msg.op.lcAttach.logicSlot = p_card->logic_slot_no;
    msg.op.lcAttach.phySlot= p_card->phy_slot_no;
    msg.op.lcAttach.portNum = p_card->port_num;
    
    msg.op.lcAttach.lcType.series = p_card->board_type.series;
    msg.op.lcAttach.lcType.type = p_card->board_type.type;
    msg.op.lcAttach.hwVer = p_card->hw_ver;
    msg.op.lcAttach.epldVer = p_card->epld_ver;
    msg.op.lcAttach.epldDate = p_card->epld_date;
    msg.op.lcAttach.epldTime = p_card->epld_time;

    msg.op.lcAttach.flashSize = p_card->flash_size;
    msg.op.lcAttach.dramSize = p_card->dram_size;
    //msg.op.lcAttach.tcamType = p_card->tcam_type;
    //msg.op.lcAttach.sramType = p_card->sram_type;
    msg.op.lcAttach.swVer.buf = (uint8 *)p_card->sw_ver;
    msg.op.lcAttach.swVer.size = strlen(p_card->sw_ver);
    msg.op.lcAttach.serialNo.buf = (uint8 *)p_card->serial_no;
    msg.op.lcAttach.serialNo.size = strlen(p_card->serial_no);
    msg.op.lcAttach.bootromVer.buf = (uint8 *)p_card->bootrom_ver;
    msg.op.lcAttach.bootromVer.size = strlen(p_card->bootrom_ver); 
    msg.op.lcAttach.extCardType = p_card->daughter_card_type;
    msg.op.lcAttach.extCardVer = p_card->daughter_card_ver;
    /*fix bug35857, cr11635, modify prompt message, liangf, 2015-11-02*/
    msg.op.lcAttach.cpuType = p_card->cpu_type;    
    msg.op.lcAttach.datapathMode = p_card->datapath_mode;
    msg.op.lcAttach.platformType = p_card->platform_type;

    msg.op.lcAttach.tmprLimit.boardLow = p_card->tmpr_limit.board_low;
    msg.op.lcAttach.tmprLimit.boardHigh = p_card->tmpr_limit.board_high;
    msg.op.lcAttach.tmprLimit.boardCrit = p_card->tmpr_limit.board_crit;   
    ret = lcm_ftm_read_profile_mode(&stm_mode);
    if (ret < 0)
    {
        LCM_LOG_ERR( "Lcm attach get stm profile mode : failed to get stm mode, use default mode.");
        stm_mode = 0;
    }
    stm_ext_flag = lcm_ftm_get_profile_ext_flag (p_card->board_type);
    
    msg.op.lcAttach.stmMode = stm_mode; 
    msg.op.lcAttach.stmExtFlag = stm_ext_flag;

    lcapi_sdk_profile.profile_mode = stm_mode;
    lcapi_sdk_profile.profile_ext_flag = stm_ext_flag;
    LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_GET_SDK_PROFILE, &lcapi_sdk_profile);

    p_msg_item = LCM_CALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(struct SDKTableType) * GLB_STM_SPEC_TBL_MAX);
    if (!p_msg_item)
    {
        LCM_LOG_ERR("Lcm send lcAttach : no memory.");
        return LCM_E_NO_MEMORY;
    }
    msg.op.lcAttach.sdkGlbeInfo.mcastGroupNum = lcapi_sdk_profile.sdk_profile.sdk_glb_info.mcast_group_number;
    msg.op.lcAttach.sdkGlbeInfo.glbNexthopNum = lcapi_sdk_profile.sdk_profile.sdk_glb_info.glb_nexthop_number;
    msg.op.lcAttach.sdkGlbeInfo.mac = lcapi_sdk_profile.sdk_profile.sdk_glb_info.mac;
    for (i = 0; i < GLB_STM_SPEC_TBL_MAX; i++)
    {
        p_msg_item[i].size = lcapi_sdk_profile.sdk_profile.spec_table[i].entry_size;
        p_msg_item[i].number = lcapi_sdk_profile.sdk_profile.spec_table[i].total_num;
        ASN_SET_ADD(&(msg.op.lcAttach.sdkTable.list), &p_msg_item[i]);
    }
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send lcAttach");
    LCM_LOG_BRINGUP("Line card send lcAttach");

    ret = lcm_msg_tx(&msg,0);
    asn_set_empty(&(msg.op.lcAttach.sdkTable.list));
    LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_msg_item);
    p_msg_item = NULL;
    if (ret < 0)
    {
        return ret;
    }

#endif
    return LCM_E_SUCCESS;
}

int32 
lcm_chsm_msg_tx_lcDeattach(lcm_clnt_t* clnt)
{
    LcmMsg_t msg;
    int32 logic_slot;

    LCM_MGT_CHK_SLOT(logic_slot);
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcDeattach;
    msg.op.lcDeattach.slot = logic_slot;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send lcDeattach");

    return lcm_msg_tx(&msg,0);
}

#ifndef CNOS_OS
void
lcm_chk_rx_keepalive_cb(void* p_data)
{
    lcm_clnt_t *clnt = (lcm_clnt_t *)p_data;
    if (!g_bDebug)
    {
        clnt->keepalive_cnt++;
        if (KEEPALIVE_TIME_OUT_COUNT <= clnt->keepalive_cnt)
        {
            LCM_LOG_ERR( "connect to supervisor timeout %d times, reload ...\n", clnt->keepalive_cnt); 
            lcm_clnt_close(clnt);
            /* added by tongzb for bug 40719 0x04 means LCMDOG reboot, 2016-09-12 */
#ifdef HAVE_DEBIAN
            system("/switch/usr/sbin/reboot_record 0x04 1>/dev/null 2>&1 && sync");            
#else
            system("/usr/sbin/reboot_record 0x04 1>/dev/null 2>&1 && sync");            
#endif
            lcm_reload();
        }
    }
    return;
}
#endif

static int32 
lcm_chsm_msg_rx_chsmLcAttachAck(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    CTC_TASK_GET_MASTER
    glb_card_t *p_card = NULL;
    
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv lcAttach ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    lcm_msg_rx_chsmLcAttachAck(msg);

#ifndef CNOS_OS
    if (msg->op.chsmLcAttachAck.debugEn && g_p_timer_chk_rx_keepalive == NULL)
    {
        g_p_timer_chk_rx_keepalive = CTC_TASK_ADD_TIME_MSEC(lcm_chk_rx_keepalive_cb, p_card, KEEPALIVE_INTERVAL);
    }
#endif
    return LCM_E_SUCCESS;
}

int32 
lcm_chsm_msg_tx_lcChsmInitSdkAck(int32 error_code)
{
    uint32 port_id;
    LcmMsg_t msg;    
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    struct stat stat_buf;

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card tx init SDK ack message.");
    LCM_LOG_BRINGUP("Line card tx init SDK ack message.");

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm send init SDK ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
 
    for(port_id = 0; port_id < p_card->port_num; port_id++)
    {
        p_port = p_card->pp_port[port_id];
        if (!p_port)
        {
            continue;
        }
        lcm_msg_tx_lcPortInfo(p_port,0);
        if (p_port->p_fiber)
        {
            lcm_notify_fiber_info(p_port, LCM_FIBER_INSERT);
        }
        //wangw , for stacking system, temp code
        if (port_id % 5 == 0 && (!stat("/tmp/startup_cfg_done", &stat_buf)))
        {
            sleep(1);
        }
    }
    LCM_LOG_BRINGUP("Line card tx port info.");
    
    sal_memset(&msg, 0, sizeof(LcmMsg_t));
    msg.op.present = op_PR_lcChsmInitSdkAck;
    msg.op.lcChsmInitSdkAck.errCode = error_code;
#ifndef _GLB_UML_SYSTEM_    
    msg.op.lcChsmInitSdkAck.sysMac.buf = p_card->sysmac;
    msg.op.lcChsmInitSdkAck.sysMac.size = GLB_ETH_ADDR_LEN + 1;
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
    uint8 scfgFlags = 0;
    lcm_mgt_smartcfg_set(scfgFlags);
    msg.op.lcChsmInitSdkAck.scfgFlags = scfgFlags;
#endif /* !HAVE_SMARTCFG */
#endif       
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card send init SDK ack.");
    LCM_LOG_BRINGUP("Line card send init SDK ack.");

    return lcm_msg_tx(&msg,0);
}

int32 
lcm_chsm_msg_rx_chsmInitSdk(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    int32 ret = LCM_E_SUCCESS;
    glb_card_t* p_card;
    glb_sdk_init_info_t sdk_info;
    uint8 stm_mode;
    uint16 port_id;
    static int g_bSDKinit = FALSE; 
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "############lcm_msg_rx_chsmInitSdk 0");
    LCM_LOG_BRINGUP("############lcm_msg_rx_chsmInitSdk 0");
    if (g_bSDKinit == TRUE)
    {
        lcm_chsm_msg_tx_lcChsmInitSdkAck(ret);
        return 0;
    }

    LCM_LOG_DEBUG(LCM_CHSMTLK, "############lcm_msg_rx_chsmInitSdk 1");
    LCM_LOG_BRINGUP("############lcm_msg_rx_chsmInitSdk 1");
    g_bSDKinit = TRUE;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv chsm init sdk : card is NULL.");
        return LCM_E_INVALID_PTR;
    } 
    /*read memory profile*/
    stm_mode = msg->op.chsmInitSdk.profileMode;
    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card rx init SDK message, stm_mode=%d.", stm_mode);
    LCM_LOG_INFO("Line card rx init SDK message, stm_mode=%d.\n", stm_mode);
    LCM_LOG_DEBUG(LCM_CHSMTLK, "############lcm_msg_rx_chsmInitSdk 2");
    LCM_LOG_BRINGUP("############lcm_msg_rx_chsmInitSdk 2, stm_mode=%d\n", stm_mode);

    sal_memset(&sdk_info, 0, sizeof(sdk_info));
    /* modify for vxlan by chenxw, 2014-05-29. */
    sdk_info.stm_mode = stm_mode; /* VXLAN */
    
    LCM_LOG_DEBUG(LCM_CHSMTLK, "############lcm_msg_rx_chsmInitSdk 3");
    LCM_LOG_BRINGUP("############lcm_msg_rx_chsmInitSdk 3");
    /*send profile info and datapath info to sdk*/    
    /*Jqiu modify for universal datapath. 2011-02-22*/
    sdk_info.datapath_config_file = p_card->cfg_profile_name;
    if(lcm_stack_is_enable())
    {
        sdk_info.stacking_en = TRUE;
        sdk_info.stacking_version = 0;
        sdk_info.gchip_id = lcm_stack_self_slotid();
        sdk_info.stacking_sup_slot = lcm_stack_master_slotid();/*added by ychen in 2014-01-21 for bug 26942*/
    }
    else
    {
        sdk_info.stacking_en = FALSE;
        sdk_info.stacking_version = 0;
        sdk_info.gchip_id = 0;
        sdk_info.stacking_sup_slot = 0;
    }
    sdk_info.logic_slot_no = p_card->logic_slot_no;
    sdk_info.phy_slot_no = p_card->phy_slot_no;
    /* qicx added for different board has different ASIC interrupt handling, 2013-07-04 */
    sdk_info.board_type.series = p_card->board_type.series;
    sdk_info.board_type.type = p_card->board_type.type;
    sdk_info.port_num = p_card->port_num;
    sdk_info.phy_map = (glb_sdk_phy_maping_t* )LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(glb_sdk_phy_maping_t)*sdk_info.port_num);
    for(port_id = 0; port_id < (p_card->port_num); port_id++)
    {
        sdk_info.phy_map[port_id].mdio_bus_addr = p_card->pp_port[port_id]->mdio_bus_addr;
        sdk_info.phy_map[port_id].phy_addr = p_card->pp_port[port_id]->phy_addr;
        sdk_info.phy_map[port_id].mac_idx = p_card->pp_port[port_id]->mac_idx;
    }    

    LCM_LOG_DEBUG(LCM_CHSMTLK, "############lcm_msg_rx_chsmInitSdk 4");
    LCM_LOG_BRINGUP("############lcm_msg_rx_chsmInitSdk 4");
    /* call functiong : hagt_modules_init */
    ret = LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_INIT_HAGT, &sdk_info);
    if(ret < 0)
    {
        FILE* fp;
        fp = fopen("/dev/console", "w+");
        fprintf(fp, "Init SDK fail! Error code %d.\n", ret);
        fclose(fp);
        LCM_LOG_ERR("Lcm init hagt and SDK fail.");
        ret = LCM_E_INIT_FAILED;
        goto err_out;
    } 

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Line card init SDK done.");
    LCM_LOG_BRINGUP("############Line card init SDK done");
    ret = lcm_mgt_post_init();
    if(ret == LCM_E_INIT_FAILED)
        goto err_out;

#ifndef MANU_DIAG
    lcm_chsm_msg_tx_lcChsmInitSdkAck(ret);
#endif

    if (p_card->bootup_diag_level >= E_DIAG_LEVEL_MINIMAL)
    {
        FILE * fp_console = NULL;
        fp_console = fopen("/dev/console", "w+");
        sal_fprintf(fp_console, "SDK diag pass!\n");
        fclose(fp_console);
        FILE * fp_log = NULL;
        fp_log = fopen("/tmp/.bootup_diag_log", "a");
        sal_fprintf(fp_log, "SDK diag pass!\n");
        fclose(fp_log);      
        log_diag(M_MOD_LCM, E_WARNING, "SDK diag pass!\n");
    }
    return ret;
err_out:
    if (p_card->bootup_diag_level >= E_DIAG_LEVEL_MINIMAL)
    {
        FILE * fp_console = NULL;
        fp_console = fopen("/dev/console", "w+");
        sal_fprintf(fp_console, "SDK diag fail!\n");
        fclose(fp_console);
        FILE * fp_log = NULL;
        fp_log = fopen("/tmp/.bootup_diag_log", "a");
        sal_fprintf(fp_log, "SDK diag fail!\n");
        fclose(fp_log);  
        log_diag(M_MOD_LCM, E_WARNING, "SDK diag fail!\n");
    }
    return ret;
}

static int 
lcm_chsm_msg_rx_chsmKeepalive(lcm_clnt_t* clnt, LcmMsg_t *msg)
{
    int32 ret;
    clnt->keepalive_cnt = 0;
    ret = lcm_msg_tx_lcChsmKeepaliveAck();
    if(ret < 0)
    {
        return ret;
    }
    clnt->send_keepalive_cnt++;
    clnt->recv_keepalive_cnt++;
    return 0;
}

static int32 
lcm_chsm_msg_rx_chsmReloadLc(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmReloadLc(msg);
}

#ifndef _GLB_DISTRIBUTE_SYSTEM_  
static int32
lcm_chsm_msg_rx_chsmSetStmMode(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_mgt_set_stm_mode(msg->op.chsmSetStmMode.stmMode);
}
#endif /*_GLB_DISTRIBUTE_SYSTEM_*/

int32 
lcm_chsm_msg_tx_lcFiberAttach(lcm_clnt_t* clnt,    glb_port_t *p_port)
{
    return lcm_msg_tx_lcFiberAttach(p_port,0);
}

int32 
lcm_chsm_msg_tx_lcFiberDeAttach(lcm_clnt_t* clnt,    glb_port_t *p_port)
{
    return lcm_msg_tx_lcFiberDeAttach(p_port,0);
}

int32
lcm_chsm_msg_tx_lcFiberUpdate(lcm_clnt_t* clnt,    glb_port_t *p_port)
{
    return lcm_msg_tx_lcFiberUpdate(p_port,0);
}

int32
lcm_chsm_msg_tx_lcPortInfo(lcm_clnt_t* clnt, glb_port_t* p_port)
{
    return lcm_msg_tx_lcPortInfo(p_port,0);    
}

/* Added by liuht for bug 27788, 2014-03-24 */
int32 
lcm_chsm_msg_tx_lcUptimeGetAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcUptimeGetAck(0);    
}
/* add end by liuht */

int32 
lcm_chsm_msg_rx_chsmTmprThreshold(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmTmprThreshold(msg);
}

static int32 
lcm_chsm_msg_rx_chsmUptimeGet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_tx_lcUptimeGetAck(0);
}

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
static int32
lcm_chsm_msg_rx_chsmSmartCfgSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;   
    return lcm_msg_rx_chsmSmartCfgSet(msg);
}
#endif /* !HAVE_SMARTCFG */

int32 
lcm_chsm_msg_tx_lcOemInfo(lcm_clnt_t* clnt) ///TODO:here need change the msg type define
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_tx_lcOemInfo(0);
}

static int32 
lcm_chsm_msg_rx_chsmOemInfoSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmOemInfoSet(msg);
}

static int32 
lcm_chsm_msg_rx_chsmPortInfoSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmPortInfoSet(msg);
}

static int32 
lcm_chsm_msg_rx_chsmLedSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmLedSet(msg);
}

static int32 
lcm_chsm_msg_rx_chsmLedAlarmFlagSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    LCM_LOG_DEBUG(LCM_CHSMTLK, "rx set led alarm flag message.");

#ifdef _CTC_LOONGSON_2H_    
    g_sys_led_alarm = msg->op.chsmLedAlarmFlagSet.ledAlarmFlag;
    lcm_mgt_set_led_flag_to_kernal(&g_sys_led_alarm);
#endif	

    return LCM_E_SUCCESS;
}

int32 
lcm_chsm_msg_tx_lcChsmTmprGetAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcChsmTmprGetAck(0);
}

int32 
lcm_chsm_msg_rx_chsmTmprGet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0; 
    return lcm_msg_rx_chsmTmprGet(msg);
}

int32 
lcm_chsm_msg_tx_lcchsmGetFanStatusAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmGetFanStatusAck(0);
}


int32 
lcm_chsm_msg_rx_chsmGetFanStatus(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmGetFanStatus(msg);
}

int32 
lcm_chsm_msg_tx_lcchsmGetPsuStatusAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmGetPsuStatusAck(0);
}

int32 
lcm_chsm_msg_rx_chsmGetPsuStatus(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmGetPsuStatus(msg);
}

int32 
lcm_chsm_msg_tx_lcchsmGetVcmCfgAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmGetVcmCfgAck(0);
}

int32 
lcm_chsm_msg_rx_chsmGetVcmCfg(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmGetVcmCfg(msg);
}

int32 
lcm_chsm_msg_tx_lcchsmGetVcmStatusAck(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmGetVcmStatusAck(0);
}

int32 
lcm_chsm_msg_rx_chsmGetVcmStatus(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmGetVcmStatus(msg);
}

/* clear reboot info file */
int32 
lcm_chsm_msg_rx_chsmResetRebootInfo(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_mgt_reset_reboot_info();
}

int32
lcm_chsm_msg_tx_lcChsmVCT(lcm_clnt_t *clnt, uint32 port_id, glb_port_cable_info_t* p_cable_info)
{
    return lcm_msg_tx_lcChsmVCT(port_id,p_cable_info,0);
}

int32
lcm_chsm_msg_rx_chsmSetVctMod(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmSetVctMod(msg);   
}
 
int32
lcm_chsm_msg_tx_lcPortSpeedMode(lcm_clnt_t* clnt, glb_port_t* p_port)
{
    return lcm_msg_tx_lcPortSpeedMode(p_port,0);    
}

int32 
lcm_chsm_msg_rx_chsmConfigSpeedMode(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmConfigSpeedMode(msg);
}

static int32  
lcm_chsm_msg_rx_chsmUpdatebootrom(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmUpdatebootrom(msg);
}

static int32  
lcm_chsm_msg_rx_chsmUpdateEpld(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmUpdateEpld(msg);
}

int32 lcm_chsm_msg_rx_chsmLoadPhyFirmware(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;
    return lcm_msg_rx_chsmLoadPhyFirmware(msg);
}

int32
lcm_chsm_msg_rx_chsmConfigbootcmd(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmConfigbootcmd(msg);
}

/*fix bug39335, support to define sw_ver, liangf, 2016-07-06*/
int32
lcm_chsm_msg_rx_chsmSetSwVer(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmSetSwVer(msg);
}

/* Added by liuht for bug27036, 2014-02-27 */
int32
lcm_chsm_msg_tx_lcRebootInfo(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcRebootInfo(0);
}

static int 
lcm_chsm_msg_rx_chsmSetKeepaliveEn(lcm_clnt_t *clnt, LcmMsg_t *msg)
{
    g_bDebug = msg->op.chsmSetKeepaliveEn.enable;
    clnt->keepalive_cnt = 0;
    clnt->send_keepalive_cnt = 0;
    clnt->recv_keepalive_cnt = 0;
    return 0;
}

int32
lcm_chsm_msg_tx_lcchsmReset(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmReset(0);
}

int32
lcm_chsm_msg_tx_lcchsmDyingGasp(lcm_clnt_t* clnt)
{
    return lcm_msg_tx_lcchsmDyingGasp(0);
}

#ifdef HAVE_STACK    
extern struct lcm_mgt_stack_config g_lcm_stack;
int lcm_chsm_msg_rx_chsmStackingMaster(lcm_clnt_t *clnt, LcmMsg_t *msg)
{    
    uint32_t master = 0;
    
    master = msg->op.chsmStackingMaster.master;
    g_lcm_stack.master = master;
    log_sys(M_MOD_LCM, E_INFORMATIONAL, "%s master slot %d", __FUNCTION__, master);
    return 0;
}

int lcm_chsm_msg_rx_chsmStackingTxTrunk(lcm_clnt_t *clnt, LcmMsg_t *msg)
{    
    lcapi_hagt_set_stack_swtich_tx_trunk_t tx_info;
    
    tx_info.member = msg->op.chsmStackingTxTrunk.member;
    tx_info.trunk = msg->op.chsmStackingTxTrunk.trunk;
    log_sys(M_MOD_LCM, E_INFORMATIONAL, "%s master member %d trunk %d", __FUNCTION__, tx_info.member, tx_info.trunk);

    LCM2HAGT_CALLBACK_SEND(LCM2HAGT_CB_SET_STACK_SWITCH_TX_TRUNK, &tx_info);
    return 0;
}
#endif

static int32 
lcm_chsm_msg_rx_chsmMgmtCfgSet(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    clnt->keepalive_cnt = 0;   
    return lcm_msg_rx_chsmMgmtCfgSet(msg);
}

int32 
lcm_chsm_msg_rx_chsmFanSpeedRate(lcm_clnt_t* clnt, LcmMsg_t* msg)
{
    return lcm_msg_rx_chsmFanSpeedRate(msg);
}

lcm_clnt_cb_t lcm_clnt_cb_tbl[LCM_SRV_CB_TBL_SZ] = {
    [op_PR_chsmLcAttachAck]      = lcm_chsm_msg_rx_chsmLcAttachAck,
    [op_PR_chsmKeepalive]        = lcm_chsm_msg_rx_chsmKeepalive,        
    [op_PR_chsmReloadLc]         = lcm_chsm_msg_rx_chsmReloadLc,
#ifndef _GLB_DISTRIBUTE_SYSTEM_   
    [op_PR_chsmSetStmMode]       = lcm_chsm_msg_rx_chsmSetStmMode,
#endif 
    [op_PR_chsmInitSdk]          = lcm_chsm_msg_rx_chsmInitSdk,
    [op_PR_chsmUptimeGet]        = lcm_chsm_msg_rx_chsmUptimeGet,
    [op_PR_chsmTmprThreshold]    = lcm_chsm_msg_rx_chsmTmprThreshold,
#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */    
    [op_PR_chsmSmartCfgSet]      = lcm_chsm_msg_rx_chsmSmartCfgSet,
#endif    
    [op_PR_chsmOemInfoSet]       = lcm_chsm_msg_rx_chsmOemInfoSet,
    [op_PR_chsmPortInfoSet]      = lcm_chsm_msg_rx_chsmPortInfoSet,
    [op_PR_chsmLedSet]           = lcm_chsm_msg_rx_chsmLedSet,
    [op_PR_chsmLedAlarmFlagSet]  = lcm_chsm_msg_rx_chsmLedAlarmFlagSet,        
    [op_PR_chsmTmprGet]          = lcm_chsm_msg_rx_chsmTmprGet,
    [op_PR_chsmGetFanStatus]     = lcm_chsm_msg_rx_chsmGetFanStatus,
    [op_PR_chsmGetPsuStatus]     = lcm_chsm_msg_rx_chsmGetPsuStatus,
    [op_PR_chsmGetVcmCfg]        = lcm_chsm_msg_rx_chsmGetVcmCfg,
    [op_PR_chsmGetVcmStatus]     = lcm_chsm_msg_rx_chsmGetVcmStatus,   
    [op_PR_chsmResetRebootInfo]  = lcm_chsm_msg_rx_chsmResetRebootInfo,  
    [op_PR_chsmSysSetVCT]        = lcm_chsm_msg_rx_chsmSetVctMod,  
    [op_PR_chsmConfigSpeedMode]  = lcm_chsm_msg_rx_chsmConfigSpeedMode,
    [op_PR_chsmUpdatebootrom]    = lcm_chsm_msg_rx_chsmUpdatebootrom,
    /* added by tongzb for load aqr phy firmware */
    [op_PR_chsmLoadPhyFirmware]  = lcm_chsm_msg_rx_chsmLoadPhyFirmware,
    [op_PR_chsmUpdateEpld]       = lcm_chsm_msg_rx_chsmUpdateEpld,
    [op_PR_chsmConfigbootcmd]    = lcm_chsm_msg_rx_chsmConfigbootcmd,
    [op_PR_chsmSetSwVer]         = lcm_chsm_msg_rx_chsmSetSwVer, 
    [op_PR_chsmSetKeepaliveEn]   = lcm_chsm_msg_rx_chsmSetKeepaliveEn,
#ifdef HAVE_STACK
    [op_PR_chsmStackingMaster]   = lcm_chsm_msg_rx_chsmStackingMaster,
    [op_PR_chsmStackingTxTrunk]  = lcm_chsm_msg_rx_chsmStackingTxTrunk,
#endif
    [op_PR_chsmMgmtCfgSet]       = lcm_chsm_msg_rx_chsmMgmtCfgSet,
    [op_PR_chsmFanSpeedRate]     = lcm_chsm_msg_rx_chsmFanSpeedRate,
};


