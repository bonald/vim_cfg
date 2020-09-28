/****************************************************************************
 * lcm_mgt.c   :    linecard manager core functions
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision     :       R0.01
 * Author       :       Zhuj
 * Date         :       2010-08-11
 * Reason       :       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#ifdef CNOS_OS
#include "proto.h"
#include "cdb_ds.h"
#include "laiinc.h"
#endif
#include "sal_common.h"
#include "glb_hw_define.h"
#include "glb_if_define.h"
#include "glb_tempfile_define.h"
#include "lcapi.h"
#include "lcm_specific.h"
#include "lcm_mgt.h"
#include "lcm_msg.h"
#include "lcm_client.h"
#include "lcm_error.h"
#include "lcm_card.h"
#include "lcm_port.h"
#include "lcm_dpll.h"
#include "lcm_debug.h"
#include "lcm_srv.h"
#include "epld_api.h"
#include "ctc_pci.h"
#include "phy_api.h"
#include "fiber_api.h"
#include "poe_api.h"
//#include "fpga_api.h"
#include "led_api.h"
#include "bootrom_drv.h"
#include "sensor_api.h"
#include "ctc_api.h"
#include "lcm_hagttlk.h"
#include "lcm_log.h"
#include "lcm_lai.h"
#include "ctclib_sys_cmd.h"
#include "glb_distribute_system_define.h"
#include "eeprom_api.h"
#include "fan_api.h"
#include "gpio_api.h"
#include "power_api.h"
#include "vcm_api.h"
#include "sysenv_api.h"
#include <linux/if.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#ifdef _CTC_LOONGSON_
#include "kernel_monitor.h"
#endif
#include "lcm_chsmtlk.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
extern int32 lcm_ftm_read_profile_mode(uint32* p_mode);
extern int32 ctc_sai_port_set_mac(uint32 gport, mac_addr_t mac_addr);
extern char* lcm_mgt_get_combo_info_goldengate_demo();

extern int32 if_build_port_shortname_by_slot_port (char ifname[IFNAMSIZ], uint16 slot_id, uint16 port_id, uint16 sub_port_id);
extern int32 if_build_port_fullname_by_slot_port (char ifname[GLB_PHY_PORT_FULL_NAME_SZ], uint16 slot_id, uint16 port_id, uint16 sub_port_id);

extern int sys_reboot(void);

#define LCM_LOW_PRIO_QUEUE_DEPTH_MAX  8
/* for bug 50154, reduce fiber timer circle, from 1s to 500ms */
#define LCM_LOW_PRIO_QUEUE_TIMEOUT  500     /* ms */
#define LCM_LOW_PRIO_TASK_INTERVAL  500      /* ms */
struct lcm_low_prio_task_s
{
    sal_task_t* p_task;
    sal_async_queue_t*  p_queue;
};
typedef struct lcm_low_prio_task_s lcm_low_prio_task_t;

#define POE_CARD_STATUS_TIME 6000
#define POE_PORT_SYS_CFG_TIME 1000


/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
static glb_card_t g_card;
static int32 g_err_reboot_flag;
static int32 g_priority_flag;
static lcm_low_prio_task_t g_lcm_low_prio_task;
static int32 g_bypass_timers;
static int32 g_timers_mask = 0xff;
lcm_mgt_callback_t lcm_mgt_poe_present_callback_func;
lcm_mgt_callback_t lcm_mgt_poe_absent_callback_func;
int g_active_sup_slot = 1; /*wangw_debug, temp 1*/
#ifndef _GLB_UML_SYSTEM_
static uint8 g_reboot_times = 0;
static int32 g_int_init = 0;
#endif

card_init_callback_t lcm_card_init_cb;
#ifdef HAVE_STACK
struct lcm_mgt_stack_config g_lcm_stack = {0};
#endif

#ifndef _GLB_UML_SYSTEM_
void lcm_manage_environment_timer(void* p_data);
//void lcm_poe_port_sys_cfg_timer(void *p_data);

LCM_DEFINE_TASK_ENCAP(lcm_manage_environment_timer)
//LCM_DEFINE_TASK_ENCAP(lcm_poe_port_sys_cfg_timer)
#endif

void lcm_mgt_wakeup_low_priority_task (void* p_data);
LCM_DEFINE_TASK_ENCAP(lcm_mgt_wakeup_low_priority_task)

void lcm_mgt_ctc_dpll_intr(void* p_data);
void lcm_mgt_ctc_phy_intr(void* p_data);
void lcm_mgt_ctc_reset_intr(void* p_data);
void lcm_mgt_ctc_power_intr(void* p_data);
//void lcm_mgt_ctc_poe_intr(void* p_data);

LCM_DEFINE_TASK_ENCAP(lcm_mgt_ctc_dpll_intr)
LCM_DEFINE_TASK_ENCAP(lcm_mgt_ctc_phy_intr)
LCM_DEFINE_TASK_ENCAP(lcm_mgt_ctc_reset_intr)
LCM_DEFINE_TASK_ENCAP(lcm_mgt_ctc_power_intr)
//LCM_DEFINE_TASK_ENCAP(lcm_mgt_ctc_poe_intr)

lcm_phy_custom_init_callback_t  lcm_phy_custom_init_cb = NULL;

/****************************************************************************
*
* Function
*
*****************************************************************************/
#if 0
#ifndef _GLB_UML_SYSTEM_
poe_port_deal_item_cb poe_port_deal_item_cb_tbl[POE_PORT_DEAL_ITEM_MAX]=
{
    [POE_PORT_DEAL_ITEM_ENABLE] = poe_port_enable_item,
};

poe_sys_deal_item_cb poe_sys_deal_item_cb_tbl[POE_SYS_DEAL_ITEM_MAX]=
{
    [POE_SYS_DEAL_ITEM_PM] = poe_set_sys_pm_item,
    [POE_SYS_DEAL_ITEM_LEGACY] = poe_set_sys_legacy_cap_item,
};
#endif
#endif

int32
lcm_phy_init_custom_init_callback(lcm_phy_custom_init_callback_t func)
{
    lcm_phy_custom_init_cb = func;
    return LCM_E_SUCCESS;
}

int32
lcm_mgt_get_active_sup_slot(void* pv_arg)
{
    lcapi_lcm_get_active_sup_slot_t *msg;

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);
    msg = (lcapi_lcm_get_active_sup_slot_t *)pv_arg;

    if(lcm_stack_is_enable())
    {
        msg->slot = lcm_stack_master_slotid();
    }
    else
    {
        msg->slot = g_active_sup_slot;
    }
    return LCM_E_SUCCESS;
}

static int32
_lcm_shutdown_all_ports(glb_card_t* p_card)
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#else
    int32 ret = 0;
    uint32 port_id ;

    for (port_id = 0;port_id < p_card->port_num; port_id++)
        {
            ret += phy_cfg_enable(port_id, 0);
            /*Fix bug 17269. when reboot, cause link partner link flap. jqiu 2011-12-31*/
            fiber_set_enable(port_id, 0);
        }
    return ret;
#endif
}

int32
lcm_mgt_get_sw_info(glb_card_t* p_card)
{
    FILE *fp;
    char buf[256];

    if (p_card == NULL)
    {
        return LCM_E_SUCCESS;
    }
    
    memset(p_card->sw_ver, 0, sizeof(p_card->sw_ver));
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL == fp)
    {
        return LCM_E_FILE_OPEN;
    }

    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, GLB_SW_VERSION_STRING, 6))
        {
            sal_sscanf(buf, "%s %s", tmp, p_card->sw_ver);
        }
        
     }

    sal_fclose(fp);
    return LCM_E_SUCCESS;
}

/*****************************************************************************
 * Name        : _lcm_mgt_init_hw_info
 * Purpose     : initialize hw information
 * Input       : card infomation
 * Output      : N/A
 * Return      : SUCCESS
 * Note        :
*****************************************************************************/
static int32
_lcm_mgt_init_hw_info(glb_card_t* p_card)
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#endif

    FILE *fp;
    uint32 val;
    char buf[256];

    p_card->tmpr_cfg.low = DEFAULT_LOW_TMPR;
    p_card->tmpr_cfg.high = DEFAULT_HIGH_TMPR;
    p_card->tmpr_cfg.crit = DEFAULT_CRIT_TMPR;
#ifdef CNOS_OS
    p_card->tmpr_cfg.chip_low_alarm = GG_CHIP_TMPR_LOW_ALARM;
    p_card->tmpr_cfg.chip_low = GG_CHIP_TMPR_LOW;
    p_card->tmpr_cfg.chip_high = GG_CHIP_TMPR_HIGH;
    p_card->tmpr_cfg.chip_alarm = GG_CHIP_TMPR_ALARM;
    p_card->tmpr_cfg.chip_crit = GG_CHIP_TMPR_CRITICAL;
#endif
    /* HW information is in file /tmp/ctcos_board_info, now parse it */

    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL == fp)
    {
        p_card->board_type.type = 0xff;
        p_card->board_type.series = GLB_SERIES_MAX;
        p_card->epld_ver = 0;
        p_card->fpga_ver = 0;
        return LCM_E_FILE_OPEN;
    }

    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, GLB_PRODUCT_SERIES_STRING, 14))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->board_type.series = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_BOARD_TYPE_STRING, 10))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->board_type.type = (uint8)val;
        }
#ifndef _CTC_NID_
        else if(!sal_strncmp(buf, GLB_HW_VERSION_STRING, 6))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->hw_ver = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_HW_FLASH_SIZE, 10))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->flash_size = val;
        }
        else if(!sal_strncmp(buf, GLB_HW_DRAM_SIZE, 10))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->dram_size = val;
        }
#endif
        else if(!sal_strncmp(buf, GLB_EPLD_VERSION_STRING, 8))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->epld_ver = (uint8)val;
        }
        else if(!sal_strncmp(buf, GLB_EPLD_DATE_STRING, 9))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->epld_date = val;
        }
#ifndef _CTC_NID_
        else if(!sal_strncmp(buf, GLB_EPLD_TIME_STRING, 9))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->epld_time = (uint16)val;
        }
#endif
        else if(!sal_strncmp(buf, GLB_SW_VERSION_STRING, 6))
        {
            sal_sscanf(buf, "%s %s", tmp, p_card->sw_ver);
        }
        else if(!sal_strncmp(buf, GLB_SERIES_NUM, 8))
        {
            sal_sscanf(buf, "%s %s", tmp, p_card->serial_no);
        }
        else if(!sal_strncmp(buf, GLB_BOOTROM_VER, 7))
        {
            sal_sscanf(buf, "%s %s", tmp, p_card->bootrom_ver);
        }
        else if(!sal_strncmp(buf, GLB_PLATFORM_TYPE, 13))
        {
            sal_sscanf(buf, "%s %x", tmp, &val);
            p_card->platform_type = val;
        }
        else if(!sal_strncmp(buf, GLB_SYS_MAC, 6))
        {
            uint32 tmp2[6];
            int  i;
            sal_sscanf(buf, "%s %02X:%02X:%02X:%02X:%02X:%02X", tmp, &tmp2[0], &tmp2[1], &tmp2[2], &tmp2[3], &tmp2[4], &tmp2[5]);
            for(i = 0; i < 6; i++)
            {
                p_card->sysmac[i] = (uint8)tmp2[i];
            }
            p_card->sysmac[GLB_ETH_ADDR_LEN] = '\0';
        }
     }

    LCM_LOG_DEBUG(LCM_LCMGT,   "Get board series %x, type %x, hw version %x",
                  p_card->board_type.series, p_card->board_type.type, p_card->hw_ver);
    LCM_LOG_BRINGUP("board %s support onie",(p_card->platform_type == 1)?"":"not");
    LCM_LOG_BRINGUP("Get board series %x, type %x, hw version %x",
                  p_card->board_type.series, p_card->board_type.type, p_card->hw_ver);

    sal_fclose(fp);

    return LCM_E_SUCCESS;
}

int32 
lcm_mgt_int_init()
{
#ifdef _GLB_UML_SYSTEM_
    return 0;
#else
    //CTC_TASK_GET_MASTER

    if(g_int_init == 0)
    {
        if(g_card.phy_interrupt_mode == GLB_PHY_INTERRUPT_MODE_EPLD)
        {
            epld_cfg_phy_interrupt_mask(0, g_card.is_gbdemo);
        }
        else if(g_card.phy_interrupt_mode == GLB_PHY_INTERRUPT_MODE_CPU)
        {
            epld_cfg_phy_interrupt_mask(0, g_card.is_gbdemo);
        } 
        
        g_int_init = 1;
    }
    
    return 0;
#endif    
}

/***************************************************************************************************
 * Name         : lcm_mgt_ctc_phy_intr
 * Purpose      : manage ctc hw interrupt events
 * Input        :
 * Output       : N/A
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
void
lcm_mgt_ctc_phy_intr(void* p_data)
{
#ifdef _GLB_UML_SYSTEM_
    return;
#else
    CTC_TASK_GET_MASTER

    LCM_LOG_DEBUG(LCM_LCMGT, "handle ctc hw interrupt events.");
#ifndef CNOS_OS
    int32 ctc_phy_fd = get_ctc_phy_fd();
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_phy_intr, NULL, ctc_phy_fd);
#endif    
    lcm_port_status_intr();
    return;
#endif    
}

void
lcm_mgt_ctc_reset_intr(void* p_data)
{
#ifdef _GLB_UML_SYSTEM_
    return;
#else
    CTC_TASK_GET_MASTER
    LCM_LOG_DEBUG(LCM_LCMGT, "handle reset interrupt events.");
#ifndef CNOS_OS 
    int32 ctc_reset_fd = get_ctc_reset_fd();
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_reset_intr, NULL, ctc_reset_fd);
#endif
    /* notify CHSM of reset operation */
    lcm_msg_tx_lcchsmReset(0);

    return;
#endif
}

/* Modified by liuht for bug 27657, 2014-03-25 */
#ifndef _GLB_UML_SYSTEM_
static int power_intr_flag = 0;
#endif
int32
lcm_mgt_handle_power_intr()
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#else
    time_t timep;
    struct stat stat_buf;
    FILE *fp_console = NULL;
    FILE *fp_pts = NULL;
    FILE *fp_log = NULL;	
    int i = 0;
    char buf[32];

#ifndef CNOS_OS
    lcm_msg_tx_lcchsmDyingGasp(0);
#endif
    
    time(&timep);
    strncpy(buf,ctime(&timep),24);
    buf[24] = '\0';
    /* sync log to syslog */
    if(power_intr_flag == 0)
    {
        fp_log = sal_fopen("/mnt/flash/cold/power.log",  "a+");
        sal_fprintf(fp_log, "%s The power supply of device has been broken down, please check!\n", buf);
        sal_fclose(fp_log);
        power_intr_flag = 1;
    }
    /* Print log to console */
    fp_console = sal_fopen("/dev/console",  "a+");
    sal_fprintf(fp_console, "PSU Down!\n");
    sal_fclose(fp_console);

    /* Print log to all tty */
    for(i = 0; i < 8; i++)
    {
        sal_sprintf(buf, "/dev/pts/%d", i);
        buf[10] = '\0';
        if (stat(buf, &stat_buf) == 0)
        {
            fp_pts = sal_fopen(buf,  "a+"); 
            if(fp_pts)
            {
                sal_fprintf(fp_pts, "The power supply of device has been broken down, please check!\n");
                sal_fclose(fp_pts);
            }
        }
    }

    // TODO: send msg to upper level
    return LCM_E_SUCCESS;
#endif
}

/* Modified by liuht for bug 27657, 2014-03-25 */
void
lcm_mgt_ctc_power_intr(void* p_data)
{
#ifdef _GLB_UML_SYSTEM_
    return;
#else
    CTC_TASK_GET_MASTER
    
    LCM_LOG_DEBUG(LCM_LCMGT, "handle power interrupt events.");
#ifndef CNOS_OS
    int32 ctc_power_fd = get_ctc_power_fd();
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_power_intr, NULL, ctc_power_fd);
#endif
    lcm_mgt_handle_power_intr();

    return;
#endif
}


/***************************************************************************************************
 * Name         : lcm_mgt_ctc_dpll_intr
 * Purpose      : manage ctc dpll interrupt events
 * Input        :
 * Output       : N/A
 * Return       : LCM_E_SUCCESS
 * Note         : N/A
***************************************************************************************************/
void
lcm_mgt_ctc_dpll_intr(void* p_data)
{
#ifdef _GLB_UML_SYSTEM_
    return;
#else
    CTC_TASK_GET_MASTER

    LCM_LOG_DEBUG(LCM_LCMGT, "handle ctc dpll interrupt events.");
#ifndef CNOS_OS 
    int32 ctc_dpll_fd = get_ctc_dpll_fd();
    if (ctc_dpll_fd < 0)
    {
        LCM_LOG_ERR("lcm_mgt_ctc_dpll_intr : get dpll fd failed.");
    }
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_dpll_intr, NULL, ctc_dpll_fd);
#endif
    lcm_dpll_status_intr();
    /*get local line card*/
    glb_card_t* p_card = NULL; 
    p_card= lcm_mgt_get_card();
    if(NULL == p_card)
    {
        LCM_LOG_ERR("Pointer to card is invalid.");
        return ;
    }
    if(p_card->dpll_interrupt_mode == GLB_PHY_INTERRUPT_MODE_EPLD)
    {
        epld_cfg_dpll_interrupt_mask(0);
    }
    else if(p_card->dpll_interrupt_mode == GLB_PHY_INTERRUPT_MODE_CPU)
    {
        ctc_en_dpll_irq(NULL);
    }
    else if(p_card->dpll_interrupt_mode == GLB_PHY_INTERRUPT_MODE_NULL)
    {
        ctc_en_dpll_irq(NULL);//default cpu interrupt mode
    }

    return;
#endif
}

#ifdef CNOS_OS
void 
lcm_mac_addr_add(uint8* mac, uint32 cnt)
{
    uint32_t value;
    uint32_t val_high, val_low;

    val_high = (mac[0]<<16)+(mac[1]<<8)+mac[2];
    val_low = (mac[3]<<16)+(mac[4]<<8)+mac[5];
    value = val_low + cnt;
    val_low = value & 0xffffff;
    if(value > 0xffffff)
    {
        val_high = val_high+1;
    }
    mac[0] = (val_high >> 16) & 0xff;
    mac[1] = (val_high >> 8) & 0xff;
    mac[2] = val_high & 0xff;
    mac[3] = (val_low >> 16) & 0xff;
    mac[4] = (val_low >> 8) & 0xff;
    mac[5] = val_low & 0xff;
}

static int32
lcm_port_get_mac(uint8*mac, uint32 pannel_port_no)
{
    sal_memcpy(mac, g_card.sysmac, GLB_ETH_ADDR_LEN);
#ifdef TAPPRODUCT
    /* Added by kcao 2017-02-28 for consume one MAC for TAP */
#else
    lcm_mac_addr_add(mac, pannel_port_no);
#endif

    return LCM_E_SUCCESS;    
}

void
lcm_mgt_get_combo_info(FILE* fp)
{
#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = lcm_mgt_get_card();
    glb_port_t* p_port = NULL;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ];

    char *combo_str = "None";

    if (GLB_SERIES_E580 == p_card->board_type.series)
    {
        if (GLB_BOARD_E580_24Q == p_card->board_type.type)
        {
            //combo_str = lcm_mgt_get_combo_info_goldengate_demo();
            sal_fprintf(fp,"%s\n", combo_str);
        }
    }
    else if (GLB_SERIES_E350 == p_card->board_type.series)
    {
        if (GLB_BOARD_E350_8TS12X == p_card->board_type.type)
        {
            int port_idx = 0;

            const char *media_type_strs[3] =
            {
                "auto",
                "rj45",
                "sfp"
            };

            sal_fprintf (fp, "\n%-14s %-10s %-25s\n", "Interface", "Admin", "OperType");
            sal_fprintf (fp, "--------------+----------+-------------------------\n");

            for(port_idx = 0; port_idx < p_card->port_num; port_idx++)
            {
                p_port = p_card->pp_port[port_idx];

                if(p_port->is_combo)
                {
                    sal_memset(port_name, 0 , sizeof(port_name));
                    if_build_port_shortname_by_slot_port(port_name, p_port->panel_slot_no, 
                                             p_port->panel_port_no, p_port->panel_sub_port_no);
                    sal_fprintf(fp,"%-14s %-10s %-25s\n", port_name, 
                                cdb_enum_val2str(media_type_strs, 3, p_port->port_cfg.media),
                                cdb_enum_val2str(glb_phy_type_strs, GLB_PHY_TYPE_MAX, p_port->phy_type));
                }
            }
        }
    }
#endif

    return;
}
#endif

int32 lcm_match_series_and_type(uint8 series, uint8 type)
{
    glb_card_t* p_card;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("lcm start : card is NULL.");
    }

    if((series == p_card->board_type.series) 
        && (type == p_card->board_type.type))
    {
        return 1;
    }

    return 0;
}

/*****************************************************************************
 * Name        : lcm_mgt_init_lc
 * Purpose     : initialize linecard
 * Input       : pointer of global card info
 * Output      : N/A
 * Return      : SUCCESS
 * Note        : only used on chassis system
*****************************************************************************/
static int32
_lcm_mgt_init_lc(glb_card_t* p_card)
{
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm managment init linecard begin.");
#ifdef _GLB_DISTRIBUTE_SYSTEM_
#ifdef _GLB_UML_SYSTEM_
    LCM_IF_ERROR_RETURN(lcm_init_sw_emu(p_card)); 
#endif // _GLB_UML_SYSTEM_ end

#else // !_GLB_DISTRIBUTE_SYSTEM_
#ifdef _GLB_UML_SYSTEM_
    LCM_IF_ERROR_RETURN(lcm_init_sw_emu(p_card));
#else // !_GLB_UML_SYSTEM_   

    LCM_IF_ERROR_RETURN(lcm_common_parse_bootup_diag(p_card));

    /****************************** E350 series *****************************/
    if(lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_48T4XG)
        ||lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_48T4X2Q)
        ||lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_V350_48P4X)
        ||lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_V350_24P4X))
    {
        /* Modify by liuht for now don't support E350-48T4X2Q for bug25659, 2013-11-28 */
        /* Modified by liuht to distinguish e350-48t4xg and e350-48t12xg, 2013-11-05 */           
        //LCM_IF_ERROR_RETURN(lcm_init_e350_48t4xg(p_card));
    }
#ifdef GREATBELT
    else if(lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_PEGATRON))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e350_pegatron(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_8TS12X))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_e350_8ts12x(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_65DV1))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e350_65dv1(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E350,GLB_BOARD_E350_HF_12T4S8X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e350_hf_12t4s8x(p_card));
    }    
#endif    
    /****************************** E580/P580 series *****************************/
    else if(lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_24Q))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_goldengate_demo(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_48X2Q4Z))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_e580_48x2q4z(p_card));
    } 
    else if(lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_48X6Q))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_e580_48x6q(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_32X2Q)
        ||lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_32X))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_e580_32x2q(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_P580,GLB_BOARD_P580_32X6Q))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_p580_32x6q(p_card));
    }    
    else if(lcm_match_series_and_type(GLB_SERIES_E580,GLB_BOARD_E580_WQT48XT4Q))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_e580_wqt48xt4q(p_card));
    }   
    else if(lcm_match_series_and_type(GLB_SERIES_P580,GLB_BOARD_T65C_8T64X6Q))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_t65c_8t64x6q(p_card));
    }   
    else if(lcm_match_series_and_type(GLB_SERIES_SG8348C,GLB_BOARD_SG8348C))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_sg8348c(p_card));
    }     
    /****************************** E550 series *********************************/
#ifdef DUET2
    else if(lcm_match_series_and_type(GLB_SERIES_E550,GLB_BOARD_E550_24X8Y2C))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e550_24x8y2c(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E550,GLB_BOARD_E550_48T4Y2Q))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e550_48t4y2q(p_card));
    }    
    else if(lcm_match_series_and_type(GLB_SERIES_E550,GLB_BOARD_E550_24T16Y)
        ||lcm_match_series_and_type(GLB_SERIES_E550,GLB_BOARD_E550_24T16X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e550_24t16y(p_card));
    }    
#endif
#ifdef TSINGMA
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_24X2C_D))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c_d(p_card));
    } 
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_48T4X2Q_D))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x2q_d(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_48X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_48x(p_card));
    }
    else if((lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_24X2C))
        ||(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_24X2Q)))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_24x2c(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_48T4X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_48S4X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_48s4x(p_card));
    }
    else if(lcm_match_series_and_type(GLB_SERIES_E530,GLB_BOARD_E530_48P4X))
    {
        LCM_IF_ERROR_RETURN(lcm_init_e530_48t4x(p_card));
    }    
#endif
    else if(lcm_match_series_and_type(GLB_SERIES_EMU, GLB_BOARD_TM2_EMU))
    {
        //LCM_IF_ERROR_RETURN(lcm_init_tm2_emu(p_card));
    }
    else
    {
        LCM_LOG_USER(E_WARNING,LCM_4_UNSUPPORT_BOARD_SERIES_TYPE,p_card->board_type.series,p_card->board_type.type);
        LCM_LOG_WARN("Unsupport board series %d type %d",p_card->board_type.series,p_card->board_type.type);
        return LCM_E_INVALID_BOARD;
    }
#endif // _GLB_UML_SYSTEM_ end
#endif // _GLB_DISTRIBUTE_SYSTEM_ end
#ifdef CNOS_OS
/*add port mac to port data*/
#ifdef _GLB_UML_SYSTEM_
    int32 fd;
    int32 ret;
    fd = sal_open("/dev/urandom", O_RDONLY);
    if(fd < 0)
    {
        g_card.sysmac[0] = 0x00;
        g_card.sysmac[1] = 0x00;
        g_card.sysmac[2] = 0x0a;
        g_card.sysmac[3] = 0x00;
        g_card.sysmac[4] = 0x00;
        g_card.sysmac[5] = 0x00;
    }
    ret = sal_read(fd, g_card.sysmac, 6);
    if(ret < 0)
    {
        g_card.sysmac[0] = 0x00;
        g_card.sysmac[1] = 0x00;
        g_card.sysmac[2] = 0x0a;
        g_card.sysmac[3] = 0x00;
        g_card.sysmac[4] = 0x00;
        g_card.sysmac[5] = 0x00;
    }
    else
    {
        g_card.sysmac[0] &= 0xfe;
        g_card.sysmac[5] = 0x0;
    }
    sal_close(fd);
#endif
    uint8 mac[GLB_ETH_ADDR_LEN];
    int i;
    for(i=0;i<g_card.port_num;i++)
    {
        lcm_port_get_mac(mac, g_card.pp_port[i]->port_idx);
        sal_memcpy(g_card.pp_port[i]->mac,mac, GLB_ETH_ADDR_LEN); 
    }
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm managment init linecard end.");
#endif
    return LCM_E_SUCCESS;
}

/* Added by liuht for bug27036, 2014-02-27 */
/***************************************************************************************************
 * Name         : lcm_mgt_mount_reboot_info 
 * Purpose      : mount reboot information to line card         
 * Input          : p_card
 * Output        : N/A              
 * Return        : LCM_E_SUCCESS
 * Note           : N/A
***************************************************************************************************/
int32
lcm_mgt_mount_reboot_info(glb_card_t* p_card)
{
#ifndef _GLB_UML_SYSTEM_
    uint8 i = 0;
    uint8 count = 0;
    char buf[256];
    char time1[16],time2[16];
    FILE *fp = NULL; 
    if(p_card->support_reboot_info)
    {
        if(0 == p_card->reboot_times)
        {
            return LCM_E_INVALID_PARAM;
        }
        g_reboot_times = p_card->reboot_times;
        count = p_card->reboot_times;	
    
        fp = sal_fopen(REBOOT_INFO_SAVE_PATH, "a+");
        if(NULL == fp)
        {
            return LCM_E_FILE_OPEN;
        }
        if(p_card->reboot_times > REBOOT_TIMES_MAX)
        {
            p_card->reboot_times = REBOOT_TIMES_MAX;
        }
        p_card->reboot_info = (reboot_info_t *)LCM_MALLOC(CTCLIB_MEM_LCM_MODULE, sizeof(reboot_info_t)*p_card->reboot_times);
        if(NULL == p_card->reboot_info)
        {
            return LCM_E_NO_MEMORY;
        }
        sal_fseek(fp, 0, SEEK_SET);
        memset(buf, 0, 256);
        while(fgets(buf, 256, fp))
        {
            if((count > 0) && (count <= REBOOT_TIMES_MAX))
            {
                sal_sscanf(buf, "%s %s %s", p_card->reboot_info[i].reboot_type, time1, time2);
                sal_sprintf(p_card->reboot_info[i].reboot_time, "%s %s", time1, time2);	
                i++;
            }
            count--;
            memset(buf, 0, 256);
        }
        sal_fclose(fp);
    }
#endif
    return LCM_E_SUCCESS;
}

#if 0
int32
lcm_mgt_unmount_reboot_info(glb_card_t* p_card)
{
#ifndef _GLB_UML_SYSTEM_
    if(p_card->support_reboot_info)
    {
        p_card->reboot_times = g_reboot_times;
#ifndef CNOS_OS
        /* Modified by liuht for bug 27925, 2014-03-31 */
        if(p_card->reboot_info)
        {
            sal_free(p_card->reboot_info);
        }
        p_card->reboot_info = NULL;
#endif
    }
#endif
    return LCM_E_SUCCESS;
}
#endif


/* Added by liuht for bug27036, 2014-02-27 */
/***************************************************************************************************
 * Name         : lcm_mgt_get_reboot_info_reboot_time 
 * Purpose      : get reboot time           
 * Input          : reboot_time
 * Output        : N/A              
 * Return        : LCM_E_SUCCESS
 * Note           : N/A
***************************************************************************************************/
int32
lcm_mgt_get_reboot_info_reboot_time(char *reboot_time)
{
#ifndef _GLB_UML_SYSTEM_
    char year[5], month[3], day[3],hour[3],minute[3], second[3];

    time_t timep;
    struct tm *p;

    time(&timep);
    p=localtime(&timep);

    sal_sprintf(year, "%d", (1900 + p->tm_year));	
    if(1+p->tm_mon >= 10)
    {  
        sal_sprintf(month, "%d", (1+p->tm_mon));		
    }
    else
    {   
        sal_sprintf(month, "%c%d", '0',(1+p->tm_mon));		
    }
    if(p->tm_mday >= 10)
    { 
        sal_sprintf(day, "%d", p->tm_mday);    			
    }
    else
    {
        sal_sprintf(day, "%c%d", '0', p->tm_mday);    		
    }
    if(p->tm_hour >= 10)
    { 
        sal_sprintf(hour, "%d", p->tm_hour);    		
    }
    else
    {  
        sal_sprintf(hour, "%c%d", '0', p->tm_hour);    	
    }
    if(p->tm_min >= 10)
    {  
        sal_sprintf(minute, "%d", p->tm_min);    		
    }
    else
    {  
        sal_sprintf(minute, "%c%d", '0', p->tm_min);    		
    }
    if(p->tm_sec>= 10)
    {  
        sal_sprintf(second, "%d", p->tm_sec);    		
    }
    else
    {  
        sal_sprintf(second, "%c%d", '0', p->tm_sec);    		
    }	
    sal_sprintf(reboot_time, "%s-%s-%s %s:%s:%s", 
		year, 
		month, 
		day, 
		hour, 
		minute,
		second);
#endif	
    return LCM_E_SUCCESS ;
}

/* Added by liuht for bug27036, 2014-02-27 */
/***************************************************************************************************
 * Name         : lcm_mgt_get_reboot_info 
 * Purpose      : get reboot information           
 * Input          : reboot_info
 * Output        : N/A              
 * Return        : LCM_E_SUCCESS
 * Note           : N/A
***************************************************************************************************/
int32
 lcm_mgt_get_reboot_info(reboot_info_t *reboot_info)
{
#ifndef _GLB_UML_SYSTEM_
    int reboot_flag = 0;
    char reboot_time[32];	

    lcm_mgt_get_reboot_info_reboot_time(reboot_time);
    sal_strncpy(reboot_info->reboot_time, reboot_time, 32);

    /* added by tongzb for bug 40719, 2016-09-12 */
    /* modified by tongzb for bug 47469, 2016-09-12 */
    FILE *fp = NULL;
    uint32 val;
    char buf[256];
    uint8 reboot_type = REBOOT_EPLD;
    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL != fp)
    {
        while(fgets(buf, 256, fp) != NULL)
        {
            uint8 tmp[256];
        
            if(!sal_strncmp(buf, "rebootinfo_type", 15))
            {
                sal_sscanf(buf, "%s %d", tmp, &val);
                reboot_type = (uint8)val;
            }
        }
        sal_fclose(fp);        
    }

    if(reboot_type == REBOOT_EPLD)
    {
        reboot_flag = epld_get_reboot_info();
        epld_reset_reboot_info();
        LCM_LOG_BRINGUP("get reboot info from epld is %d",reboot_flag);
    }
    else if(reboot_type == REBOOT_FILE_EPLD)
    {
        reboot_flag = epld_get_reboot_info();
        epld_reset_reboot_info();
        /* for bug 47940,when reboot by hardware watchdog, the epld do not poweroff
         * so we use epld info for record hardware watchdog reboot type */
        LCM_LOG_BRINGUP("get reboot info from epld is %d",reboot_flag);
        if(reboot_flag != 0xfe)
        {
            fp = sal_fopen(REBOOT_RECORD_FILE, "rw+");
            if(NULL != fp)
            {
                while(fgets(buf, 256, fp) != NULL)
                {
                    uint8 tmp[256];
                
                    if(!sal_strncmp(buf, "rebootinfo", 10))
                    {
                        sal_sscanf(buf, "%s %d", tmp, &val);
                        reboot_flag = (uint8)val;
                    }
                }
                sal_fseek(fp,0,SEEK_SET);
                sal_fprintf(fp ,"rebootinfo %d\n",0x00);
                sal_fflush(fp);
                sal_fclose(fp);
                LCM_LOG_BRINGUP("get reboot info from file is %d",reboot_flag);
            }
        }
    }
    /*********************************************************
            reboot_type           reboot_flag
            POWER                      0x00
            MANUAL                     0x01
            HIGH-TMPR                  0x02
            BHMDOG                     0x03
            LCMDOG                     0x04
            SCHEDULE                   0x05
            SNMP-RELOAD                0x06
            HALFAIL                    0x07
            ABNORMAL                   0x08
            CTCINTR                    0x09
            LCATTACH                   0x0a
            MANUAL-FAST                0x0b(CNOS)
            MANUAL-HITLESS             0x0c(CNOS)
            OTHER                      0xfe
            UNKNOW                     other
    *********************************************************/
    switch (reboot_flag)
    {
        case 0x01:
            sal_memcpy(reboot_info->reboot_type, "MANUAL", sizeof("MANUAL"));
            break;
        case 0x02:
            sal_memcpy(reboot_info->reboot_type, "HIGH-TMPR", sizeof("HIGH-TMPR"));
            break;      
        case 0x03:
            sal_memcpy(reboot_info->reboot_type, "BHMDOG", sizeof("BHMDOG"));
            break;
        case 0x04:
            sal_memcpy(reboot_info->reboot_type, "LCMDOG", sizeof("LCMDOG"));
            break;
        case 0x05:
            sal_memcpy(reboot_info->reboot_type, "SCHEDULE", sizeof("SCHEDULE"));
            break;
        case 0x06:
            sal_memcpy(reboot_info->reboot_type, "SNMP-RELOAD", sizeof("SNMP-RELOAD"));
            break;
        case 0x07:
            sal_memcpy(reboot_info->reboot_type, "HALFAIL", sizeof("HALFAIL"));
            break;  
        case 0x08:
            sal_memcpy(reboot_info->reboot_type, "ABNORMAL", sizeof("ABNORMAL"));
            break;
        case 0x09:
            sal_memcpy(reboot_info->reboot_type, "CTCINTR", sizeof("CTCINTR")); 
            break;
        case 0x0a:
            sal_memcpy(reboot_info->reboot_type, "LCATTACH", sizeof("LCATTACH")); 
            break;
#ifdef CNOS_OS            
        case 0x0b:
            sal_memcpy(reboot_info->reboot_type, "MANUAL-FAST", sizeof("MANUAL-FAST"));
            break;
        case 0x0c:
            sal_memcpy(reboot_info->reboot_type, "MANUAL-HITLESS", sizeof("MANUAL-HITLESS"));
            break;
#endif            
        case 0xfe:
            sal_memcpy(reboot_info->reboot_type, "OTHER", sizeof("OTHER"));
            break;
        case 0x00:/* for now ,all support board reset value is 0x00 2016-09-07*/
            sal_memcpy(reboot_info->reboot_type, "POWER", sizeof("POWER")); 
            break;
        default:
            sal_memcpy(reboot_info->reboot_type, "UNKNOWN", sizeof("UNKNOWN"));
            break;
                
    }
#endif
    return LCM_E_SUCCESS;
}

/* Added by liuht for bug27036, 2014-02-27 */
/***************************************************************************************************
 * Name         : lcm_mgt_save_reboot_info 
 * Purpose      : save reboot information           
 * Input          : p_card reboot_info
 * Output        : N/A              
 * Return        : LCM_E_FILE_OPEN | LCM_E_NO_MEMORY | LCM_E_SUCCESS
 * Note           : N/A
***************************************************************************************************/
int32
 lcm_mgt_save_reboot_info(glb_card_t* p_card, reboot_info_t *reboot_info)
{
#ifndef _GLB_UML_SYSTEM_
    /* Modified by liuht for bug 27925, 2014-03-31 */
    FILE *fp = NULL;
    FILE *fp_tmp = NULL;
    char buf[256];
    long int count = 0;

    fp = sal_fopen(REBOOT_INFO_SAVE_PATH, "a+");
    if(NULL == fp)
    {
        return LCM_E_FILE_OPEN;
    }
	
    sal_fprintf(fp ,"%-15s %-25s\n",
        reboot_info->reboot_type, 
        reboot_info->reboot_time);
    sal_fflush(fp);
    sal_fseek(fp, 0, SEEK_SET);
    while(sal_fgets(buf, 256, fp))
    {
        count++;
    }
	
    if(count > 200)
    {
        fp_tmp = sal_fopen(REBOOT_INFO_SAVE_PATH_TMP, "a+");
        if(NULL == fp_tmp)
        {
            return LCM_E_FILE_OPEN;
        }
        sal_fseek(fp, 0, SEEK_SET);
        while(count > 200)
        {
            sal_fgets(buf, 256, fp);
            count--;
        }
        while(sal_fgets(buf, 256, fp))
        {
            sal_fputs(buf, fp_tmp);
        }
        sal_fclose(fp);
        sal_fclose(fp_tmp);
        sal_unlink(REBOOT_INFO_SAVE_PATH);
        sal_rename(REBOOT_INFO_SAVE_PATH_TMP, REBOOT_INFO_SAVE_PATH);
    }
    else
    {
        sal_fclose(fp);
    }
    p_card->reboot_times = (uint8)count;
#endif
    return LCM_E_SUCCESS;
}

/* Added by liuht for bug27036, 2014-02-27 */
/***************************************************************************************************
 * Name         : lcm_mgt_reboot_info 
 * Purpose      : management reboot information           
 * Input          : p_card
 * Output        : N/A              
 * Return        : ret
 * Note           : N/A
***************************************************************************************************/
int32
_lcm_mgt_reboot_info(glb_card_t* p_card)
{
#ifndef _GLB_UML_SYSTEM_
    int ret = 0;
    reboot_info_t reboot_info;
    if(p_card->support_reboot_info)
    {
        LCM_LOG_BRINGUP("get reboot infomation from device/file\n");
        ret = lcm_mgt_get_reboot_info(&reboot_info);
        if(ret < 0)
        {
            p_card->reboot_times = 0;
            LCM_LOG_USER(E_ERROR,LCM_4_GET_REBOOT_INFO, "Get");
            LCM_LOG_ERR("Get reboot infomation fail.");
            return LCM_E_SUCCESS;
        }
        ret = lcm_mgt_save_reboot_info(p_card, &reboot_info);
        if(ret < 0)	
        {
            p_card->reboot_times= 0;
            LCM_LOG_USER(E_ERROR,LCM_4_GET_REBOOT_INFO, "Save");
            LCM_LOG_ERR("Save reboot infomation fail.");
            return LCM_E_SUCCESS;
        }
    }
    else
    {
        LCM_LOG_BRINGUP("board do not support reboot-info\n");
    }
    ret = lcm_mgt_mount_reboot_info(p_card);
#endif
    return LCM_E_SUCCESS;
}

void
lcm_mgt_wakeup_low_priority_task (void* p_data)
{
    CTC_TASK_GET_MASTER
    int ret = 0;

    CTC_TASK_ADD_TIME_MSEC(lcm_mgt_wakeup_low_priority_task, NULL, LCM_LOW_PRIO_TASK_INTERVAL);

    ret = sal_async_queue_put(g_lcm_low_prio_task.p_queue, NULL);
    if(ret)
    {
        LCM_LOG_DEBUG(LCM_LCMGT, "Low priority task queue write fail, error code is %d.", ret);
    }

    return;
}

#ifndef _GLB_UML_SYSTEM_
/*Lcm get all sensor temperature and store, if reach critical, shutdown power.*/
int32
lcm_manage_tmpr_status()
{
    glb_card_t* p_card = NULL; 
    int16 board_temp, board_temp_max=0;    
    uint32 chip_temp = 0;
    uint8 i, sensor_id=0, position=SENSOR_CPU, shutdown_flag=0;
    
    p_card= lcm_mgt_get_card();
    /*1. Get real-time temperature value, and select the max temp value.*/
    for(i=0; i<p_card->sensor_num; i++)
    {
        if(RESULT_OK == sensor_dev_get_temp(i, CURRENT_TEMP, &board_temp))
        {
            p_card->tmpr_status[i].tmpr_val = board_temp;
            sensor_dev_get_position(i, &position);
            p_card->tmpr_status[i].pos = position;
            if(board_temp_max<board_temp)
            {
                board_temp_max = board_temp;
                sensor_id = i;
                /*fix bug36556, if p_card->tmpr_cfg.crit is default value 0, then use BOARD_TMPR_FULL */
                /* added by tongzb for bug 43301 */
                if (p_card->tmpr_cfg.crit<=0)
                {
                    /* if customized critical limitation is valid, then use it */
                    if (p_card->tmpr_limit.board_crit>0)
                    {
                        p_card->tmpr_cfg.crit = p_card->tmpr_limit.board_crit;
                    }
                    else
                    {
                        p_card->tmpr_cfg.crit = BOARD_TMPR_FULL;
                    }
                }
                if(board_temp_max >= p_card->tmpr_cfg.crit)
                {
                     shutdown_flag = 1; 
                     break;
                }
            }
        }
    }
    for(i=0; i<p_card->chip_sensor_num; i++)
    {
        if(CTC_E_NONE == ctc_get_chip_sensor(i, CTC_CHIP_SENSOR_TEMP, &chip_temp))
        {
            p_card->tmpr_status[p_card->sensor_num+i].tmpr_val = chip_temp;
            p_card->tmpr_status[p_card->sensor_num+i].pos = SENSOR_CHIP0+i;
        }
    }
    /*2. check if chip tmpr reach critical*/
    for(i=0; i<p_card->chip_sensor_num; i++)
    {
        if(p_card->tmpr_status[p_card->sensor_num+i].tmpr_val >= GG_CHIP_TMPR_CRITICAL)
        {
#ifdef _CTC_ARM_CTC5236_
            if(p_card->tmpr_status[p_card->sensor_num+i].tmpr_val >= -40
              && p_card->tmpr_status[p_card->sensor_num+i].tmpr_val <= 125)
            {
                shutdown_flag = 1;
            }
#else
            shutdown_flag = 1;
#endif            
        }
    }
    /*3. use sensor critical function to shutdown power.*/
    if(shutdown_flag)
    {
        /* added by tongzb for bug 40719 0x02 means HIGH-TMPR reboot, 2016-09-12 */
        system("reboot_record 0x02 1>/dev/null 2>&1 && sync");
        if(p_card->sensor_num)
            sensor_dev_set_temp(sensor_id, CRIT_TEMP, board_temp_max-GG_CHIP_TMPR_CRITICAL_HYST);
        else
            reboot(RB_AUTOBOOT);
    }
    return LCM_E_SUCCESS;    
}

int32
lcm_adjust_fan_speed()
{
    glb_card_t* p_card = NULL;     
    int32 chip_tmpr=0, board_tmpr=0;
    uint8 i, cfg_speed_level, orig_speed_level;
    uint8 chip_cfg_speed_level=0,board_cfg_speed_level=0;
    uint8 speed_rate;
    uint16 speed_real;
    static tmpr_level_t level;

    p_card= lcm_mgt_get_card();
    /* get low/high/full temperature limit */
    if (!level.low && !level.high && !level.full)
    {
        sensor_dev_get_level(0, &level);
        if (!level.low && !level.high && !level.full)
        {
            level.low  = BOARD_TMPR_LOW;
            level.high = BOARD_TMPR_HIGH;
            level.full = BOARD_TMPR_FULL;
        }
    }    
    /*1 get highest temperature*/
    for(i=0; i<(p_card->chip_sensor_num+p_card->sensor_num); i++)
    {        
        if((p_card->tmpr_status[i].pos == SENSOR_CHIP0)
            ||(p_card->tmpr_status[i].pos == SENSOR_CHIP1))
        {
            if(chip_tmpr < p_card->tmpr_status[i].tmpr_val)
            {
                chip_tmpr = p_card->tmpr_status[i].tmpr_val;
            }
        }
        else
        {
            if(board_tmpr < p_card->tmpr_status[i].tmpr_val)
            {
                board_tmpr = p_card->tmpr_status[i].tmpr_val;
            }
        }        
    }
    /*2 judge temperature and get new speed level*/
    fan_get_speed_level(0, &orig_speed_level);
    /*2.1 support chip sensor, adjust fan speed depend on chip sensor*/
    if(p_card->chip_sensor_num > 0)
    {
        if(chip_tmpr >= GG_CHIP_TMPR_ALARM)
        {
            chip_cfg_speed_level = FAN_SPEED_FULL;
        }
        else if(chip_tmpr >= (GG_CHIP_TMPR_ALARM-GG_CHIP_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_FULL)
            {
                chip_cfg_speed_level = FAN_SPEED_FULL;
            }
            else
            {
                chip_cfg_speed_level = FAN_SPEED_HIGH;
            }
        }
        else if(chip_tmpr >= GG_CHIP_TMPR_HIGH)
        {
            chip_cfg_speed_level = FAN_SPEED_HIGH;
        }
        else if(chip_tmpr >= (GG_CHIP_TMPR_HIGH-GG_CHIP_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_HIGH)
            {
                chip_cfg_speed_level = FAN_SPEED_HIGH;
            }
            else
            {
                chip_cfg_speed_level = FAN_SPEED_LOW;
            }
        }
        else if(chip_tmpr >= GG_CHIP_TMPR_LOW)
        {
            chip_cfg_speed_level = FAN_SPEED_LOW;
        }
        else if(chip_tmpr >= (GG_CHIP_TMPR_LOW-GG_CHIP_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_LOW)
            {
                chip_cfg_speed_level = FAN_SPEED_LOW;
            }
            else
            {
                chip_cfg_speed_level = FAN_SPEED_BOTTOM;
            }
        }
        else
        {
            chip_cfg_speed_level = FAN_SPEED_BOTTOM;
        }
    }
    /*2.2 adjust fan speed depend on board sensor*/
    if(p_card->sensor_num > 0)
    {
        if(board_tmpr >= level.full)
        {
            board_cfg_speed_level = FAN_SPEED_FULL;
        }
        else if(board_tmpr >= (level.full-BOARD_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_FULL)
            {
                board_cfg_speed_level = FAN_SPEED_FULL;
            }
            else
            {
                board_cfg_speed_level = FAN_SPEED_HIGH;
            }
        }
        else if(board_tmpr >= level.high)
        {
            board_cfg_speed_level = FAN_SPEED_HIGH;
        }
        else if(board_tmpr >= (level.high-BOARD_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_HIGH)
            {
                board_cfg_speed_level = FAN_SPEED_HIGH;
            }
            else
            {
                board_cfg_speed_level = FAN_SPEED_LOW;
            }
        }
        else if(board_tmpr >= level.low)
        {
            board_cfg_speed_level = FAN_SPEED_LOW;
        }
        else if(board_tmpr >= (level.low-BOARD_TMPR_HYST))
        {
            if(orig_speed_level == FAN_SPEED_LOW)
            {
                board_cfg_speed_level = FAN_SPEED_LOW;
            }
            else
            {
                board_cfg_speed_level = FAN_SPEED_BOTTOM;
            }
        }
        else
        {
            board_cfg_speed_level = FAN_SPEED_BOTTOM;
        }
    }
    /*3. set new speed and store it.*/
    cfg_speed_level = chip_cfg_speed_level > board_cfg_speed_level? chip_cfg_speed_level: board_cfg_speed_level;    
    for(i=0; i<p_card->fan_module_num; i++)
    {
        if(FAN_SPEED_SW == p_card->fan_status[i].speed_adjust)
        {
            fan_config_speed_level(i, cfg_speed_level);
            p_card->fan_status[i].speed_level = cfg_speed_level;
            fan_get_speed_rate_by_level(i, cfg_speed_level, &speed_rate);
            p_card->fan_status[i].speed_rate = speed_rate;
        }
        else if(FAN_SPEED_HW == p_card->fan_status[i].speed_adjust)
        {
            fan_get_real_speed(i, &speed_real);
            p_card->fan_status[i].speed_rate = (uint8)speed_real;
        }
    }
    return LCM_E_SUCCESS;
}

int32
lcm_manage_fan_status()
{
    glb_card_t* p_card = NULL; 
    uint8 i, value, present, status, fan_num,speed_adjust;

    p_card= lcm_mgt_get_card();
    /*1. get real-time fan present status */
    for(i=0; i<p_card->fan_module_num; i++)
    {
        if(RESULT_OK != fan_get_present(i, &value))
        {
            value = 0;
        }
        if(value == 0)
        {
             present = FAN_ABSENT;
        }
        else if(FAN_ABSENT == p_card->fan_status[i].present)
        {
            present = FAN_INIT;
        }
        else
        {
            present = FAN_PRESENT;
        }
        if(present == FAN_PRESENT)
        {
            /*Fan insert, status change from absent to present, re-init fan, and get fan basic info.*/
            if(p_card->fan_status[i].present == FAN_INIT)
            {
                fan_dev_reinit(i);
            }
            p_card->fan_status[i].present = present;
            fan_get_speed_adjust(i, &speed_adjust);
            p_card->fan_status[i].speed_adjust = speed_adjust;
            fan_get_num(i, &fan_num);
            p_card->fan_status[i].num = fan_num;
        }
        p_card->fan_status[i].present = present;
    }
    
    /*2. get real-time fan status */
    for(i=0; i<p_card->fan_module_num; i++)
    {
        if(p_card->fan_status[i].present != FAN_PRESENT)
        {
            continue;
        }        
        fan_get_status(i, &status, &fan_num);
        p_card->fan_status[i].status = status;        
    }
    /*3. adjust fan speed depend on temperature*/
    if((p_card->fan_module_num > 0) && speed_adjust)
    {
        lcm_adjust_fan_speed();
    }
    
    return LCM_E_SUCCESS;
}

int32
lcm_manage_psu_status()
{    
    int32 ret;
    glb_card_t* p_card = NULL; 
    psu_status_t psu_status;
    uint8 i;
    
    p_card = lcm_mgt_get_card();
    for (i = 0; i < p_card->psu_module_num; i++)
    {
        ret = psu_get_status(i, &psu_status);
        if (ret < 0)
        {
            LCM_LOG_DEBUG(LCM_CHSMTLK, "Get line card psu %d status fail", i);
            continue;
        }
        psu_status.psu_health_status_change = (psu_status.psu_alert != p_card->psu_status[i].psu_alert
                                                               || psu_status.psu_work_status != p_card->psu_status[i].psu_work_status);
        if (psu_status.psu_health_status_change)
        {
            psu_pmbus_get_status(i, &psu_status);
        }
        sal_memcpy(&(p_card->psu_status[i]), &psu_status, sizeof(psu_status_t));
        /* remove log for bug 46731 by tongzb 2018-03-29 */
        //LCM_LOG_DEBUG(LCM_CHSMTLK, "line card psu %d psuType %d psuPg %d psu_absent %d psu_alert %d",
        //                   i,psu_status.psu_type,psu_status.psu_work_status, psu_status.psu_absent, psu_status.psu_alert);
    }
    return LCM_E_SUCCESS;
}

int32
lcm_manage_vcm_status()
{
    int32 ret = 0;
    glb_card_t* p_card = NULL;   
    vcm_status_t status[MAX_VCM_MODULE_NUM];    
    uint32 i;
    
    p_card= lcm_mgt_get_card(); 
    
    if(0 == p_card->vcm_module_num)
        return LCM_E_SUCCESS;
    for (i = 0; i < p_card->vcm_module_num; i++)
    {
        ret = vcm_get_stauts(i, &p_card->vcm_cfg[i], &status[i]);
        if (ret < 0)
        {
            continue;
        }
        p_card->vcm_status[i] = status[i];
    }
    return LCM_E_SUCCESS;
}

int32
lcm_manage_poe_status()
{
    ///TODO: add for poe status, need update after by tongzb
    glb_card_t* p_card = NULL;
    glb_port_t* p_port = NULL;
    glb_poe_sys_info_t *poe_sys = NULL;
    glb_poe_port_info_t *poe_port = NULL;
    char port_name[GLB_PHY_PORT_FULL_NAME_SZ] = {0};
    uint8 i = 0,j = 0;
    poe_module_info_t poe_module;
    static uint8 scan_cnt = 0;
    static uint8 scan_pos = 0;
    static uint8 is_power[MAX_PSE_NUM] = {0};
    
    p_card= lcm_mgt_get_card(); 
    
    for(i=0;i<p_card->poe_module_num;i++)
    {
        poe_power_status_update(i);
    }
    
    for(i=0;i<p_card->poe_module_num;i++)
    {
        poe_sys = &p_card->poe_sys_info[i];
        poe_get_status(i,&poe_module);
        poe_sys->aver_consump = (poe_sys->cur_consump+poe_module.cur_consump)/2;
        poe_sys->peak_consump = poe_sys->cur_consump>poe_module.cur_consump?poe_sys->cur_consump:poe_module.cur_consump;
        poe_sys->cur_consump = poe_module.cur_consump;

        if(poe_sys->cur_consump > (poe_sys->const_power_limit*9/10))
        {
            log_sys(M_MOD_LCM, E_NOTICE, "power consumption is too high");
        }
        
        for (j = 0; j < p_card->port_num; j++)
        {
            p_port = p_card->pp_port[j];
            poe_port = &p_port->poe_info;
            if(p_port->poe_support&&poe_port->pse_id == i)
            {
                poe_port->class = poe_module.ch_info[poe_port->pse_channel_id].class;
                poe_port->cur_consump = poe_module.ch_info[poe_port->pse_channel_id].cur_consump;
                poe_port->aver_consump = poe_module.ch_info[poe_port->pse_channel_id].cur_consump;
                poe_port->peak_consump = poe_module.ch_info[poe_port->pse_channel_id].cur_consump;
                poe_port->current = poe_module.ch_info[poe_port->pse_channel_id].current;
                poe_port->voltage = poe_module.ch_info[poe_port->pse_channel_id].voltage;
                poe_port->temper = poe_module.ch_info[poe_port->pse_channel_id].temper;
                if_build_port_shortname_by_slot_port(port_name, p_port->panel_slot_no, p_port->panel_port_no, p_port->panel_sub_port_no);
                if(poe_port->status != poe_module.ch_info[poe_port->pse_channel_id].status)
                {
                    poe_status_update(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,GLB_POE_HIGH_PRI);
                    log_sys(M_MOD_LCM, E_NOTICE, "Interface %s power change to %s",
                                port_name,poe_module.ch_info[poe_port->pse_channel_id].status?"ON":"OFF");
                }
                poe_port->status = poe_module.ch_info[poe_port->pse_channel_id].status;
            }
        }
    }

    for (i = scan_pos; i < p_card->port_num; i++,scan_pos++)
    {
        p_port = p_card->pp_port[i];
        poe_port = &p_port->poe_info;
        is_power[poe_port->pse_id] = 0;
        if((p_port->poe_support)&&(poe_port->status))
        {
            is_power[poe_port->pse_id] = 1;
            if(scan_cnt == 1)
            {
                scan_cnt = 0;
                break;
            }
            poe_status_update(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,GLB_POE_LOW_PRI);
            scan_cnt++;
        }
    }
    if(scan_pos == p_card->port_num)
        scan_pos = 0;    

    for(i=0;i<p_card->poe_module_num;i++)
    {
        if(is_power[i])
            poe_glb_status_update(i);
    }

    return LCM_E_SUCCESS;
}

void
lcm_add_lai_cb(void)
{
#ifdef CNOS_OS
    lai_psu_info_t lai_psu;
    lai_thermal_info_t lai_thermal;
    lai_fan_info_t lai_fan;
    lai_vcm_info_t lai_vcm;
    lai_poe_info_t lai_poe;
    
    lai_object_id_t oid = 0;
    int i=0, j=0;
    static int hw_ready = 0;
    /*add fan lai cb*/
    /*because fan is found after hw init a while, so add it alwayse here*/
    if(p_lai_card_noti && p_lai_card_noti->on_fan_event)
    {
        for(i = 0; i<g_card.fan_module_num; i++)
        {
            for(j=0; j<g_card.fan_status[i].num; j++)
            {
                oid = (g_card.phy_slot_no << 16) + (i<<8) + j;
                
                ctc_lai_get_fan_info(oid, &lai_fan);
                
                p_lai_card_noti->on_fan_event(LAI_FAN_EVENT_ADD, &lai_fan);
            }
        }
    }

    if(hw_ready)
    {
        return;
    }
    /*add psu lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_psu_event)
    {
        for(i = 0; i<g_card.psu_module_num; i++)
        {
            oid = (g_card.phy_slot_no << 16) + i;
            
            ctc_lai_get_psu_info(oid, &lai_psu);
            
            p_lai_card_noti->on_psu_event(LAI_PSU_EVENT_ADD, &lai_psu);
        }
    }

    /*add thermal lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_thermal_event)
    {
        for (i = 0; i < g_card.sensor_num + g_card.chip_sensor_num; i++)
        {
            oid = (g_card.phy_slot_no << 16) + i;
            
            ctc_lai_get_thermal_info(oid, &lai_thermal);
            
            p_lai_card_noti->on_thermal_event(LAI_THERMAL_EVENT_ADD, &lai_thermal);
        }
    }

    /*add vcm lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_vcm_event)
    {
        for (i = 0; i < g_card.vcm_module_num; i++)
        {
            oid = (g_card.phy_slot_no << 16) + i;
            
            ctc_lai_get_vcm_info(oid, &lai_vcm);
            
            p_lai_card_noti->on_vcm_event(LAI_VCM_EVENT_ADD, &lai_vcm);
        }
    }
    /*add poe lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_poe_event)
    {
        for (i = 0; i < g_card.poe_module_num; i++)
        {
            oid = (g_card.phy_slot_no << 16) + i;
            
            ctc_lai_get_poe_info(oid, &lai_poe);
            
            p_lai_card_noti->on_poe_event(LAI_POE_EVENT_ADD, &lai_poe);
        }
    }

    g_card.card_state = BOARD_READY;
    hw_ready = 1;
#endif
}


/*Fix bug 32404, support manage environment. include sensor, fan, psu.*/
void
lcm_manage_environment_timer(void* p_data)
{
    if(lcm_mgt_get_bypass_timers())
        return;

    CTC_TASK_GET_MASTER

    CTC_TASK_ADD_TIME_MSEC(lcm_manage_environment_timer, NULL, LCM_POLLING_ENVIRONMENT_TIMER);
    /*2. manage temperature. get real-time chip and PCB temperature.*/
    lcm_manage_tmpr_status();
    /*3. manage fan status and adjust speed. */
    lcm_manage_fan_status();
    /*4. manage psu status*/
    lcm_manage_psu_status();
    /*5. manage vcm status*/
    lcm_manage_vcm_status();
    /*6. manage poe status*/
    if(lcm_check_timers_mask(ENV_TIMER))
        lcm_manage_poe_status();


#ifdef CNOS_OS
    lcm_add_lai_cb();
#endif
    return;
}

static int32
lcm_fiber_port_link_status_change_isr(uint8 gchip, void* p_data)
{
    uint16 gport = 0;
    ctc_port_link_status_t *link_status;
    
    link_status = (ctc_port_link_status_t *)p_data;
    gport = link_status->gport;    
    /* for bug 45850, remove link stats log from syslog */
    //LCM_LOG_WARN("gport %d interrupt event.", gport);
    LCM_LOG_DEBUG(LCM_LCMGT,"gport %d interrupt event.", gport);
    lcm_port_poll_port_status(&gport);
    
    return LCM_E_SUCCESS;
}

#endif

/*****************************************************************************
 * Name        : lcm_mgt_monitor_lc
 * Purpose     : run the threads(timer or event) for monitoring the linecard
 * Input       : N/A
 * Output      : N/A
 * Return      : If success
 * Note        : Should be called after the line card actually
               : init done function(lcm_card_init_cb).
*****************************************************************************/
int32
lcm_mgt_monitor_lc(void)
{
    CTC_TASK_GET_MASTER

#ifndef _GLB_UML_SYSTEM_
#ifndef _GGEMU_ // TODO: removed by xgu for gg emulation board, 2014-6-3
#ifndef _GLB_DISTRIBUTE_SYSTEM_
    int32 ctc_phy_fd = get_ctc_phy_fd();
    int32 ctc_dpll_fd = -1;
    int32 ctc_reset_fd = get_ctc_reset_fd();
    int32 ctc_power_fd = get_ctc_power_fd();
#endif
#endif

    glb_card_t* p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm mgt monitor lc : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

#endif

#ifdef _GLB_DISTRIBUTE_SYSTEM_
#else // !_GLB_DISTRIBUTE_SYSTEM_
    CTC_TASK_ADD_TIME_MSEC(lcm_mgt_wakeup_low_priority_task, NULL, LCM_LOW_PRIO_TASK_INTERVAL);
#ifndef _GGEMU_ // TODO: removed by xgu for gg emulation board, 2014-6-3
    #ifdef _GLB_UML_SYSTEM_

    #else // !_GLB_UML_SYSTEM_
    /* modify default port timer interval from 3s to 1s, qicx, 2013-12-18 */
    CTC_TASK_ADD_TIME_MSEC(lcm_port_port_status_timer, NULL, 200);
    LCM_IF_ERROR_RETURN(lcm_mgt_int_init());
    /* jqiu modified for bug38657 40G/100G auto-neg. 2016-05-23*/
#if (!defined DUET2)&&(!defined TSINGMA)
    CTC_TASK_ADD_TIME_MSEC(lcm_port_cl73_auto_timer, NULL, 2000);
#endif
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_phy_intr, NULL, ctc_phy_fd);
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_reset_intr, NULL, ctc_reset_fd);
    /* Modified by liuht for bug 27657, 2014-03-25 */
    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_power_intr, NULL, ctc_power_fd);
    //ctclib_thread_add_timer_msec(p_master, lcm_fiber_port_status_timer, NULL, 1000);
    /* if any port split, this timer management port led */
    if(p_card->split_flag)
    {
        CTC_TASK_ADD_TIME_MSEC(lcm_port_led_cfg, NULL, 5000);
    }
    CTC_TASK_ADD_TIME_MSEC(lcm_manage_environment_timer, NULL, LCM_POLLING_ENVIRONMENT_TIMER);
    /* fix bug42763, XGSW-EA-10 dpll interrupt, liangf, 2017-05-17 */
    if (p_card->dpll_num > 0)
    {
        ctc_dpll_fd = get_ctc_dpll_fd();
        if (ctc_dpll_fd > 0)
        {
            LCM_LOG_DEBUG(LCM_LCMGT, "register ctc dpll interrupt handle.");
            CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_dpll_intr, NULL, ctc_dpll_fd);
        }
    }

    #if 0

    /* modified by yaom for 10G remote fault detection 20120926 */
    CTC_TASK_ADD_TIME_MSEC(lcm_port_remote_fault_timer, NULL, 1000);

    CTC_TASK_ADD_READ_HIGH(lcm_mgt_ctc_dpll_intr, NULL, ctc_dpll_fd);

    if(p_card->poe_port_num)
    {
        CTC_TASK_ADD_TIME_MSEC(lcm_poe_port_sys_cfg_timer, NULL, 1000);
    }
    #endif

    ctc_interrupt_register_event_cb(CTC_EVENT_PORT_LINK_CHANGE, lcm_fiber_port_link_status_change_isr);
    
    #endif // _GLB_UML_SYSTEM_ end
#endif
#endif // _GLB_DISTRIBUTE_SYSTEM_ end
    LCM_LOG_BRINGUP("############Line card monitor start Done");

    return LCM_E_SUCCESS;
}

static void 
_lcm_mgt_low_priority_time_handle()
{
    #ifndef _GLB_UML_SYSTEM_  
    lcm_fiber_port_status_timer();
    gpio_scan_real_time_value();
    #endif
    return ;
}

/*****************************************************************************
 * Name        : _lcm_mgt_monitor_low_priority_task
 * Purpose     : run the threads for monitoring linecard low priority tasks
 * Input       : N/A
 * Output      : N/A
 * Return      : N/A
 * Note        : Must not use ctclib_thread related functions
*****************************************************************************/
static void
_lcm_mgt_monitor_low_priority_task(void* p_arg)
{
    int ret;
    uint32 queue_param = 0;
#ifndef _GLB_UML_SYSTEM_
    glb_card_t* p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("card is NULL.");
        return;
    }
#endif

    while(1)
    {
        ret = sal_async_queue_get(g_lcm_low_prio_task.p_queue,
                                    LCM_LOW_PRIO_QUEUE_TIMEOUT, (void **)&queue_param);
        if (ETIMEDOUT == ret)
        {
            continue;
        }

        if (0 != ret)
        {
            LCM_LOG_DEBUG(LCM_LCMGT, "Low priority task queue read fail, error code is %d.", ret);
            continue;
        }
        _lcm_mgt_low_priority_time_handle();
    }

    return;
}

glb_card_t*
lcm_mgt_get_card()
{
    return &g_card;
}

int16
lcm_mgt_get_logic_slot()
{
    return g_card.logic_slot_no;
}

void
lcm_mgt_set_err_reboot_flag(int32 flag)
{
    g_err_reboot_flag = flag;
}

int32
lcm_mgt_get_err_reboot_flag(void)
{
    return g_err_reboot_flag;
}

void
lcm_mgt_set_priority_flag(int32 flag)
{
    g_priority_flag = flag;
}

int32
lcm_mgt_get_priority_flag(void)
{
    return g_priority_flag;
}

#if 0
int32
lcm_mgt_enable_asic_normal_intr(void* pv_arg)
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#endif
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm enabel asic normal interrupt.");
    return hw_asic_en_normal_int();
}

int32
lcm_mgt_enable_asic_fatal_intr(void* pv_arg)
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#endif
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm enabel asic fatal interrupt.");
    return hw_asic_en_fatal_int();
}

#ifdef FOAM_SUPPORT /* for bug14682, foam 1588 syncif development, 2011-04-25 */
int32
lcm_mgt_enable_foam_normal_intr(void* pv_arg)
{
#ifdef _GLB_UML_SYSTEM_
    return LCM_E_SUCCESS;
#endif
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm enabel foam normal interrupt.");
    return hw_foam_en_normal_int();
}
#endif /*FOAM_SUPPORT*/
#endif
/*****************************************************************************
 * Name        : lcm_mgt_asic_fatal
 * Purpose     : handle asic fatal interrupt
 * Input       : lcapi_lcm_asic_abnormal_t
 * Output      : N/A
 * Return      : N/A
 * Note        : N/A
*****************************************************************************/
int32
lcm_mgt_asic_fatal(void* pv_arg)
{
#if 0    
    lcapi_lcm_asic_abnormal_t* asic_fatal;
    lcm_clnt_t *p_lcm_clnt = (lcm_clnt_t *)lcm_get_lcm_client();

    if(NULL == pv_arg)
    {
        LCM_LOG_ERR("handle asic fatal interrupt: invalid pointer.");
        return LCM_E_INVALID_PTR;
    }

    asic_fatal = (lcapi_lcm_asic_abnormal_t* )pv_arg;
    if(p_lcm_clnt != NULL)
    {
        lcm_msg_tx_lcAsicFatal(p_lcm_clnt, asic_fatal->fatal_type,asic_fatal->buf, asic_fatal->size);
    }
#endif
    return LCM_E_SUCCESS;
}
/*****************************************************************************
 * Name        : lcm_mgt_bypass_timers
 * Purpose     : enable/disable the bypass timers
 * Input       : enable flag
 * Output      : N/A
 * Return      : N/A
 * Note        : N/A
*****************************************************************************/
int32
lcm_mgt_bypass_timers(void* pv_arg)
{
    g_bypass_timers = *(int32 *)pv_arg;

    return 0;
}

/*****************************************************************************
 * Name        : lcm_mgt_get_bypass_timers
 * Purpose     : get the bypass timers flag
 * Input       : N/A
 * Output      : N/A
 * Return      : N/A
 * Note        : N/A
*****************************************************************************/
int32
lcm_mgt_get_bypass_timers()
{
    return g_bypass_timers;
}
/*****************************************************************************
 * Name         : lcm_reload()
 * Purpose      : The function reload line card
 * Input        : N/A
 * Output       : N/A
 * Return       : ErrCode
 * Note         : The ports' PHY must be disabled during line card reloading
*****************************************************************************/
int32
lcm_reload(void)
{
    int32 ret = 0;
    int32 bypass = 1;
    glb_card_t* p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm reload : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    /* Bypass some timers before reboot */
    lcm_mgt_bypass_timers(&bypass);

    ret = _lcm_shutdown_all_ports(p_card);
#if 0
    if(p_card->poe_port_num)
    {
        ret += _lcm_disable_poe(p_card);
    }
#endif    
#ifndef CNOS_OS
#ifdef _GLB_UML_SYSTEM_
    system("halt -f");
#endif
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    reboot(RB_AUTOBOOT);
#else
    if (lcm_stack_is_enable()
        && lcm_stack_master_slotid() != lcm_stack_self_slotid())
    {
        reboot(RB_AUTOBOOT);
    }
#endif
#endif
    return ret;
}

int32
lcm_mgt_get_bootcmd(void)
{
    if(g_card.platform_type == 0)
    {
        char bootcmd[M_FULLPATH_MAX_LEN+1];
        sal_memset(bootcmd, 0, M_FULLPATH_MAX_LEN+1);
        int ret = 0;

#ifndef _GLB_UML_SYSTEM_
#ifndef CTC_ENV_IN_SPI_FLASH
        FILE *fp;
        if (g_card.use_sysenv_api)
        {
            sysenv_info_t *p_sysenv_info = NULL;
            p_sysenv_info = &(g_card.p_sysenv_alloc[SYSENV_BOOTCMD]);
            if(NULL == p_sysenv_info)
            {
                return LCM_E_INVALID_PTR;
            }
            ret = sysenv_get_bootcmd(p_sysenv_info, bootcmd);
            if(ret < 0)
            {
                return LCM_E_HW_UPDATEBTROM_GEN;
            }
        }
        else
        {
            eeprom_info_t *p_eeprom_info = NULL;
            p_eeprom_info = &(g_card.p_eeprom_alloc[EEPROM_BOOTCMD]);
            if(NULL == p_eeprom_info)
            {
                return LCM_E_INVALID_PTR;
            }
            ret = eeprom_get_bootcmd(p_eeprom_info, bootcmd);
            if(ret < 0)
            {
                return LCM_E_HW_UPDATEBTROM_GEN;
            }
        }
        fp = sal_fopen(GLB_BOARD_INFO_FILE, "a+");
        if(fp)
        {
            sal_fprintf(fp, "bootimage %s\n", bootcmd);
            sal_fclose(fp);
        }        
#endif        
#endif

        return ret;
    }
    return LCM_E_SUCCESS;
}

int32
lcm_mgt_lai_init_card(void)
{
    int32 ret = 0;

#ifdef CNOS_OS    
    /*Here, profile type should be got from local card epprom for pizzax, 
      and should be got by subscribe CHASSIS Table, so later initsdk should be moved 
      to wait_chassis_version_check_and_get_profile_type call*/
    uint32 profile_type = 0;
    int i = 0;
    lai_card_info_t lai_card;
    lai_port_info_t lai_port;
    lai_object_id_t oid = 0;

/*mod by hansf for profile type*/
#if 0
#ifndef _GLB_UML_SYSTEM_
    //lcm_ftm_read_profile_mode(&profile_type);
    profile_type = 0;
#else
    profile_type = 0;
#endif
#endif
    lcm_ftm_read_profile_mode(&profile_type);
    g_card.profile_type = profile_type;
/*end by hansf*/

    LCM_LOG_BRINGUP("################lcm_mgt_init 5.1.");
    extern int32 lcm_InitSdk(uint32 profile_type);
    ret = lcm_InitSdk(profile_type);
#ifndef _GLB_UML_SYSTEM_  
    if (ret == 0 && g_card.bootup_diag_level >= E_DIAG_LEVEL_MINIMAL)
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
    else if (ret != 0 && g_card.bootup_diag_level >= E_DIAG_LEVEL_MINIMAL)
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
#else
    (void)ret;
#endif

    LCM_LOG_BRINGUP("################lcm_mgt_init 5.2.");
    /*set card status ready*/
    /*New OSP system decouple chsm and lcm, init sdk here derectly*/
    /*add card lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_card_event)
    {
        oid = g_card.phy_slot_no;
        LCM_IF_ERROR_RETURN(ctc_lai_get_card_info(oid, &lai_card));
        p_lai_card_noti->on_card_event(LAI_CARD_EVENT_ADD, &lai_card);
    }

    if (lcm_phy_custom_init_cb != NULL)
    {
        if (!strncmp(lai_card.hardware_name, "ATTO", 4))
        {
            lcm_phy_custom_init_cb(LCM_PHY_CUSTOM_ATTO);
        }
        else
        {
            //for other custom
        }
    }

    LCM_LOG_BRINGUP("################lcm_mgt_init 5.3.");
    /*add port lai cb*/
    if(p_lai_card_noti && p_lai_card_noti->on_port_event)
    {
        for(i = 0; i<g_card.port_num; i++)
        {
            oid = (g_card.phy_slot_no<<16) + i;
            
            LCM_IF_ERROR_RETURN(ctc_lai_get_port_info(oid, &lai_port));
            
            /*add by yejl to fix bug37838, set the phy-port's mac to sdk*/
            LCM_IF_ERROR_RETURN(ctc_sai_port_set_mac(lai_port.portid, lai_port.mac));
            
            p_lai_card_noti->on_port_event(LAI_PORT_EVENT_ADD, &lai_port);
        }
    }
#endif

    if (g_card.do_phy_bootup_diag != 0)
    {
#ifdef CNOS_OS   
#ifndef _GLB_UML_SYSTEM_
        if (g_card.phy_intr_en != 0)
        {
            lcm_port_status_intr();
        }
#endif
#endif
    }
    
    return ret;
}

/*****************************************************************************
 * Name        : int32 lcm_mgt_init
 * Purpose     : intialize the linecard manager
 * Input       : N/A
 * Output      : N/A
 * Return      : If success
 * Note        : N/A
*****************************************************************************/
int32
lcm_mgt_init(void)
{
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm managment init begin.");
    LCM_LOG_BRINGUP("Lcm managment init begin.");

    sal_memset(&g_card, 0, sizeof(glb_card_t));
    g_card.card_state = BOARD_FIRST_CREATED;
    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 0.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 0.");
    LCM_IF_ERROR_RETURN(lcm_debug_init());
    LCM_IF_ERROR_RETURN(_lcm_mgt_init_hw_info(&g_card));
    LCM_IF_ERROR_RETURN(_lcm_mgt_init_lc(&g_card));
#ifndef _GLB_UML_SYSTEM_	
    /* Added by liuht for bug27036, 2014-02-27 */	
    LCM_IF_ERROR_RETURN(_lcm_mgt_reboot_info(&g_card));	
#endif
    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 1.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 1.");


    if (0 == sal_async_queue_create(&g_lcm_low_prio_task.p_queue,
                                    LCM_LOW_PRIO_QUEUE_DEPTH_MAX))
    {
        /* modified by kcao for bug 13571, 2010-11-22 : reduce thread stack memory */
        if (0 != sal_task_create(&g_lcm_low_prio_task.p_task,
                                "lcm_monitor",
                                256*1024, SAL_TASK_PRIO_DEF+10, _lcm_mgt_monitor_low_priority_task, NULL))
        {
            LCM_LOG_CRIT( "Create lcm low priority monitoring thread fail.");
        }
    }
    else
    {
        LCM_LOG_CRIT("Create lcm low priority task queue fail.");
    }

    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 2.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 2.");
    LCM_IF_ERROR_RETURN(lcm_clnt_start()); 
#ifndef CNOS_OS
    lcm_clnt_t * lcm_clnt = lcm_get_lcm_client();

    if (!lcm_stack_is_enable())
    {
        //in lcm_clnt_start,this function fill be called 
        LCM_IF_ERROR_RETURN(lcm_chsm_msg_tx_lcAttach(lcm_clnt));
    }

    LCM_IF_ERROR_RETURN(lcm_chsm_msg_tx_lcRebootInfo(lcm_clnt));
    LCM_IF_ERROR_RETURN(lcm_chsm_msg_tx_lcOemInfo(lcm_clnt));   //here start by lcm, so chsm donot need to send oemGet info
    //LCM_IF_ERROR_RETURN(lcm_msg_tx_lcchsmGetBootcmdAck(lcm_clnt));//here send bootcmd info to chsm
#endif

    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 3.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 3.");

    LCM_IF_ERROR_RETURN(lcm_srv_start());
    
    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 4.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 4.");
    LCM_IF_ERROR_RETURN(lcm_mgt_get_bootcmd());

#ifdef CNOS_OS    
    LCM_LOG_DEBUG(LCM_LCMGT, "################lcm_mgt_init 5.");
    LCM_LOG_BRINGUP("################lcm_mgt_init 5.");
    LCM_IF_ERROR_RETURN(lcm_mgt_lai_init_card());
#endif

    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm managment init end.");
    LCM_LOG_BRINGUP("Lcm managment init end.");

    return LCM_E_SUCCESS;
}

int32
lcm_card_init_callback(card_init_callback_t func)
{
    lcm_card_init_cb = func;
    return LCM_E_SUCCESS;
}

int32 lcm_mgt_post_init(void)
{
    int32 ret = LCM_E_SUCCESS;

#ifndef _GLB_UML_SYSTEM_
    /*1. init devices depend on centec sdk*/
    ret = lcm_card_init_cb();
    if(ret < 0)
    {
        LCM_LOG_BRINGUP("Lcm init hagt and SDK fail, ret %d.", ret);
        return LCM_E_INIT_FAILED;
    }
    LCM_LOG_BRINGUP("############Line card init board callback done");
#endif /*!_GLB_UML_SYSTEM_*/

#ifndef MANU_DIAG
    /*2. start lcm mgt monitor: timer and interrupt handler*/
    lcm_mgt_monitor_lc();
#endif
    return ret;
}

int32
lcm_mgt_poe_present_callback(lcm_mgt_callback_t func)
{
    lcm_mgt_poe_present_callback_func = func;
    return LCM_E_SUCCESS;
}

int32
lcm_mgt_poe_absent_callback(lcm_mgt_callback_t func)
{
    lcm_mgt_poe_absent_callback_func = func;
    return LCM_E_SUCCESS;
}

int32
lcm_mgt_get_virt_sysmac(uint8 sys_mac[6])
{
    int32 fd = -1;
    int32 ret = -1;

    fd = sal_open("/dev/urandom", O_RDONLY);
    if(fd < 0)
    {
        sys_mac[0] = 0x00;
        sys_mac[1] = 0x00;
        sys_mac[2] = 0x0a;
        sys_mac[3] = 0x00;
        sys_mac[4] = 0x00;
        sys_mac[5] = 0x00;
        return 0;
    }
    ret = sal_read(fd, sys_mac, GLB_ETH_ADDR_LEN);
    if(ret < 0)
    {
        sys_mac[0] = 0x00;
        sys_mac[1] = 0x00;
        sys_mac[2] = 0x0a;
        sys_mac[3] = 0x00;
        sys_mac[4] = 0x00;
        sys_mac[5] = 0x00;
    }
    else
    {
        sys_mac[0] &= 0xfe;
        sys_mac[5] = 0x0;
    }
    sal_close(fd);

    return 0;
}

int32
lcm_mgt_get_sysmac(void* pv_arg)
{
    lcapi_lcm_get_sysmac_t *msg;
    glb_card_t* p_card = NULL;
    uint8 buf[GLB_ETH_ADDR_LEN + 1];

    GLB_PTR_VALID_CHECK(pv_arg, LCM_E_INVALID_PTR);

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm send init SDK ack : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

#ifndef _GLB_UML_SYSTEM_
#ifndef _GGEMU_ // TODO: Modified by xgu for gg emulation board, gg emulation also use random mac. 2014-5-28
    sal_memcpy(buf, p_card->sysmac, GLB_ETH_ADDR_LEN); 
#else
    lcm_mgt_get_virt_sysmac(buf);
#endif    
#endif

    msg = (lcapi_lcm_get_sysmac_t *)pv_arg;
    memcpy(msg->sysmac, buf, GLB_ETH_ADDR_LEN);
    return LCM_E_SUCCESS;
}

int32
lcm_mgt_get_sup_slot()
{
#ifdef HAVE_STACK
    return lcm_stack_self_slotid();
#else
    return 0;
#endif
}

int
lcm_mgt_ioctl (int request, caddr_t buffer)
{
  int sock;
  int ret = 0;
  int err = 0;

  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (1);
    }

  ret = ioctl (sock, request, buffer);
  if (ret < 0)
    {
      err = errno;
    }
  close (sock);
  
  if (ret < 0) 
    {
      errno = err;
      return ret;
    }
  return 0;
}

int
lcm_mgt_get_if_hwaddr (char *ifname, unsigned char *hwaddr,
               int *hwaddr_len)
{
  int ret;
  struct ifreq ifreq;
  int i;

  strncpy (ifreq.ifr_name, ifname, IFNAMSIZ);
  GLB_UNAME_TO_KNAME(ifreq.ifr_name);
  ifreq.ifr_addr.sa_family = AF_INET;

  /* Fetch Hardware address if available. */
  ret = lcm_mgt_ioctl (SIOCGIFHWADDR, (caddr_t) &ifreq);
  if (ret < 0)
    *hwaddr_len = 0;
  else
    {
      memcpy (hwaddr, ifreq.ifr_hwaddr.sa_data, 6);

      for (i = 0; i < 6; i++)
	if (hwaddr[i] != 0)
	  break;

      if (i == 6)
	*hwaddr_len = 0;
      else
	*hwaddr_len = 6;
    }
  return 0;
}

/* this is for stack system */
int32
lcm_mgt_reload(void* pv_arg)
{
    /* added by tongzb for bug 40719 0x07 means HALFAIL reboot, 2016-09-12 */
#ifdef HAVE_DEBIAN
    system("/switch/usr/sbin/reboot_record 0x07 1>/dev/null 2>&1 && sync");
#else
    system("/usr/sbin/reboot_record 0x07 1>/dev/null 2>&1 && sync");
#endif
    return lcm_reload();
}

/* Added by liuht for bug 27788, 2014-03-24 */
void
lcm_mgt_get_current_bootromver(char *bootrom_ver)
{
    char ary_tmp[256];
    char *chr_p = NULL;
    FILE * fd_proc;
    char cur_tmp[256] = {'\0'};
    
    fd_proc = sal_fopen("/proc/cmdline", "r");
    if(!fd_proc)
    {
        sal_strncpy(bootrom_ver, "UNKNOWN", MAX_BOOTROM_VER_LEN);
        return ;
    }
    sal_fgets(ary_tmp, 256, fd_proc);

    chr_p = sal_strstr(ary_tmp, "U-Boot ");

    if(chr_p != NULL)
    {
        chr_p += sal_strlen("U-Boot ");
        sal_strncpy(cur_tmp, chr_p, 256);
        chr_p = cur_tmp;
        if((chr_p = strchr(chr_p, ' ')) != NULL)
            *chr_p = '\0';

        sal_strncpy(bootrom_ver, cur_tmp, MAX_BOOTROM_VER_LEN);
    }
    else
    {
        sal_strncpy(bootrom_ver, "UNKNOWN", MAX_BOOTROM_VER_LEN);
    }

    sal_fclose(fd_proc);
}

/* Modified by liuht for bug 26911, 2014-03-27 */
int32
lcm_mgt_set_led_flag_to_kernal(int* alarm)
{
#ifndef _GLB_UML_SYSTEM_
#ifdef _CTC_LOONGSON_
    int ret = 0;
    int32 ctc_sys_led_fd = get_ctc_sys_led_fd();
	
    LCM_LOG_DEBUG(LCM_LCMGT, "Lcm managment set sys led flag to kernal.");
	
    if(-1 != ctc_sys_led_fd)
    {
        ret = ioctl(ctc_sys_led_fd, CTC_LED_RDWR, alarm);
	if(ret)
	{
	    LCM_LOG_ERR("Lcm set sys led flag to kernel fail.");
	}
    }
#endif    
#endif
    return LCM_E_SUCCESS;
}

/* timer_id:
0 - for all timer
1 - for port timer
2 - for fiber timer
3 - for env timer
*/
int32
lcm_set_timers_mask(uint8 timer_id,uint8 enable)
{
    if(timer_id == 0)
    {
        g_timers_mask = enable?0xff:0x00;
    }
    else
    {
        if(enable)
            GLB_SET_BIT(g_timers_mask,timer_id);
        else
            GLB_CLEAR_BIT(g_timers_mask,timer_id);
    }

    return 0;
}

int32
lcm_get_timers_mask(uint8 timer_id,uint8 *status)
{
    *status = g_timers_mask;
    return 0;
}

int32
lcm_check_timers_mask(uint8 type)
{
    return (g_timers_mask&(1<<type));
}

int32
lcm_mgt_set_stm_mode(uint32 mode)
{
    int32 ret = LCM_E_SUCCESS;
#ifdef _GLB_UML_SYSTEM_
#define STM_LINE_LEN_MAX                128
    char file_name[STM_LINE_LEN_MAX];
    char description[2*STM_LINE_LEN_MAX];
    char profile_mode[STM_LINE_LEN_MAX];
    FILE* fp = NULL;
    uint32 stm_mode = 0;

    stm_mode = mode;
    /*get file path*/
#ifndef CNOS_OS
    sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH_UML, GLB_STM_PROFILE_MODE);
#else
    sal_snprintf(file_name, sizeof(file_name), "%s%s", GLB_STM_PROFILE_PATH, GLB_STM_PROFILE_MODE);
#endif
    /*OPEN FILE*/
    fp = sal_fopen(file_name, "w+");
    if((NULL == fp))
    {
        return LCM_E_FILE_OPEN;
    }

    /*write file information*/
    sal_snprintf(description, sizeof(description), "%s\r\n%s\r\n",
              "#this file is only used for storing tcam allocation profile mode.",
              "#0=default, 1=access, 2=ipv4 route, 3=vlan, 4=l2vpn 5=ipv6.\r\n");
    sal_snprintf(profile_mode, sizeof(profile_mode), "[PROFILE_MODE] = %d",
            stm_mode);

    sal_fputs(description, fp);
    sal_fputs(profile_mode, fp);

    /*close file*/
    sal_fclose(fp);
    fp = NULL;
#else
    uint32 stm_mode = 0;
    glb_card_t* p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv stm set profile mode : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_TCAM_PROFILE]);
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR("Lcm attach get stm profile mode : sysenv alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }

        stm_mode = mode;
        ret = sysenv_set_tcam_profile(stm_mode, p_sysenv_info);
        if (ret < 0)
        {
            LCM_LOG_ERR("Lcm attach get stm profile mode : failed to set stm mode.");
            return ret;
        }
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_TCAM_PROFILE]);
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR("Lcm attach get stm profile mode : eeprom alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }

        stm_mode = mode;
        ret = eeprom_set_tcam_profile(stm_mode, p_eeprom_info);
        if (ret < 0)
        {
            LCM_LOG_ERR("Lcm attach get stm profile mode : failed to set stm mode.");
            return ret;
        }
    }
#endif

    return ret;
}

int32
lcm_mgt_set_tmpr_threshold(int32 low, int32 high, int32 crit)
{
    int32 ret = LCM_E_SUCCESS;

#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    uint32 sensor_num = 0;
    uint32 i;
    LCM_LOG_DEBUG(LCM_LCMGT, "rx set sensor temperature threshold message.");

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv set sensor temperature threshold: card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    sensor_num = p_card->sensor_num;
    p_card->tmpr_cfg.low = low;
    p_card->tmpr_cfg.high = high;
    p_card->tmpr_cfg.crit = crit;

    /*modified by liuht for bug 24525 to support LM75,2013-09-18*/
    if (GLB_SERIES_E580 == p_card->board_type.series)
    {
        for (i = 0; i < sensor_num; i++)
        {
            ret = sensor_dev_set_temp(i, THSYT_TEMP, 2);
            ret |= sensor_dev_set_temp(i, LOW_TEMP, p_card->tmpr_cfg.low);
            ret |= sensor_dev_set_temp(i, HIGH_TEMP, p_card->tmpr_cfg.high);
            ret |= sensor_dev_set_temp(i, CRIT_TEMP, p_card->tmpr_cfg.crit);
        }
    }
    else if (GLB_SERIES_E350 == p_card->board_type.series)
    {
        if((GLB_BOARD_E350_48T4X2Q== p_card->board_type.type)
            ||(GLB_BOARD_E350_8T4S12XG == p_card->board_type.type)
            ||(GLB_BOARD_E350_8TS12X == p_card->board_type.type)
            ||(GLB_BOARD_E350_65DV1 == p_card->board_type.type)
            ||(GLB_BOARD_E350_24X == p_card->board_type.type))
        {
    	    for (i = 0; i < sensor_num; i++)
    	    {
    	        ret = sensor_dev_set_temp(i, THSYT_TEMP, 2);
    	        ret |= sensor_dev_set_temp(i, LOW_TEMP, p_card->tmpr_cfg.low);
    	        ret |= sensor_dev_set_temp(i, HIGH_TEMP, p_card->tmpr_cfg.high);
    	        ret |= sensor_dev_set_temp(i, CRIT_TEMP, p_card->tmpr_cfg.crit);
    	    }
        }
        /* Modified by liuht for bug27056, 2014-01-24 */
        /* To distinguish phicomm board and centec board for 8T12X */
        else if((GLB_BOARD_E350_48T4XG == p_card->board_type.type)
               ||(GLB_BOARD_E350_8T12XG == p_card->board_type.type)
               ||(GLB_BOARD_E350_PHICOMM_8T12XG == p_card->board_type.type)
               ||(GLB_BOARD_E350_MT_8T12XG == p_card->board_type.type)
               ||(GLB_BOARD_E350_MTRJ_8T12XG == p_card->board_type.type)
               ||(GLB_BOARD_E350_24T4XG == p_card->board_type.type)
               ||(GLB_BOARD_V350_48P4X == p_card->board_type.type)
               ||(GLB_BOARD_V350_24P4X == p_card->board_type.type)
               ||(GLB_BOARD_E350_HF_12T4S8X == p_card->board_type.type))
        {
    	    for (i = 0; i < sensor_num; i++)
    	    {
    	        ret = sensor_dev_set_temp(i, CRIT_TEMP, p_card->tmpr_cfg.crit);
    	        /* Added by liuht for bug25658, 2014-02-12 */
    	        ret |= sensor_dev_set_temp(i, THSYT_TEMP, p_card->tmpr_cfg.crit - 2);
    	    }
        }
    }
    else
    {
        for (i = 0; i < sensor_num; i++)
        {
            ret = sensor_dev_set_temp(i, THSYT_TEMP, 2);
            ret |= sensor_dev_set_temp(i, LOW_TEMP, p_card->tmpr_cfg.low);
            ret |= sensor_dev_set_temp(i, HIGH_TEMP, p_card->tmpr_cfg.high);
            ret |= sensor_dev_set_temp(i, CRIT_TEMP, p_card->tmpr_cfg.crit);
        }
    }
#endif
    return ret;
}

#ifdef HAVE_SMARTCFG /* added by liuyang 2011-10-27 */
int32
lcm_mgt_smartcfg_set(unsigned char mode)
{
    int32 ret =LCM_E_SUCCESS;

#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    unsigned char flags;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        return LCM_E_INVALID_PTR;
    }
    flags = mode;

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        sysenv_para_t para;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_SMARTCFG]);
        if(NULL == p_sysenv_info)
        {
            return LCM_E_INVALID_PTR;
        }
        para.len = 1;
        para.offset = p_sysenv_info->base_addr;
        para.p_val = &flags;
        if (sysenv_write (p_sysenv_info->sysenv_idx, &para) < 0)
        {
            ret = RESULT_ERROR;
        }
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        eeprom_para_t para;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_SMARTCFG]);
        if(NULL == p_eeprom_info)
        {
            return LCM_E_INVALID_PTR;
        }
        para.len = 1;
        para.offset = p_eeprom_info->base_addr;
        para.p_val = &flags;
        if (eeprom_write (p_eeprom_info->eeprom_idx, &para) < 0)
        {
            ret = RESULT_ERROR;
        }
    }
#endif
    return ret;
}
#endif /* !HAVE_SMARTCFG */


int32
lcm_mgt_oem_info_get(char* buf)
{
    int32 ret =LCM_E_SUCCESS;

#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv get oem info : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_OEM_INFO]);
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR( "Lcm recv get oem info : sysenv alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret += sysenv_get_oem_info((uint8*)buf, p_sysenv_info);
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_OEM_INFO]);
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR( "Lcm recv get oem info : eeprom alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret += eeprom_get_oem_info((uint8*)buf, p_eeprom_info);
    }
#endif
    return ret;
}


int32
lcm_mgt_oem_info_set(uint8* buf)
{
    int32 ret =LCM_E_SUCCESS;

#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv get oem info : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_OEM_INFO]);
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR( "Lcm recv set oem info : sysenv alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = sysenv_set_oem_info(buf, p_sysenv_info);
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_OEM_INFO]);
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR( "Lcm recv set oem info : eeprom alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = eeprom_set_oem_info(buf, p_eeprom_info);
    }
#endif
    return ret;
}


int32
lcm_mgt_set_port_info(char* info)
{
    int32 ret =0;
    char buffer[MAX_PORT_INFO_LEN+1];

    sal_memset(buffer, 0, MAX_PORT_INFO_LEN);
    sal_memcpy(buffer, info, MAX_PORT_INFO_LEN);

#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv set port info : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_PORT_INFO]);
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR("Lcm recv set port info : sysenv alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = sysenv_set_port_info((uint8*)buffer, p_sysenv_info);
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_PORT_INFO]);
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR("Lcm recv set port info : eeprom alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = eeprom_set_port_info((uint8*)buffer, p_eeprom_info);
    }
#else
    FILE *port_flash_fd = NULL;

    port_flash_fd = sal_fopen(GLB_UML_PORT_INFO_FILE, "w");
    if(!port_flash_fd)
    {
        LCM_LOG_ERR("Open the interface info file for write failed!");
        return -1;
    }
    sal_fputs(buffer, port_flash_fd);
    sal_fclose(port_flash_fd);
#endif
    return ret;
}


int32
lcm_mgt_led_set(int32 led, int32 status)
{
#ifndef _GLB_UML_SYSTEM_
    int32 ret;

    /* remove log for bug 46731 by tongzb 2018-03-29 */
    //LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive set led message.");

    ret = led_api_op(led, status);
    if(ret < 0)
    {
        LCM_LOG_ERR( "Lcm set led fail.");
        return LCM_E_HW_LED_GEN;
    }
#endif
    return LCM_E_SUCCESS;
}


/* Added by liuht for bug 27036, 2014-03-20 */
int32
lcm_mgt_reset_reboot_info(void)
{
#ifndef _GLB_UML_SYSTEM_
    FILE *fp = NULL;
    glb_card_t *p_card = NULL;
    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv reset reboot info: card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    if(p_card->support_reboot_info)
    {
        p_card->reboot_times = 0;
        if (p_card->reboot_info)
        {
            LCM_FREE(CTCLIB_MEM_LCM_MODULE, p_card->reboot_info);
            p_card->reboot_info = NULL;
        }
        fp = sal_fopen(REBOOT_INFO_SAVE_PATH, "w");
        if(NULL == fp)
        {
            return LCM_E_FILE_OPEN;
        }
        sal_fclose(fp);
    }
#endif 
    return LCM_E_SUCCESS;
}

#ifndef _GLB_UML_SYSTEM_
static struct timespec g_last_update_bootrom_time = { .tv_sec = 0 };
#endif

int32  
lcm_mgt_update_bootrom(char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret;
    char dst_file[M_FULLPATH_MAX_LEN+1];
    FILE *fp;
    struct timespec cur_time;
    uint8 bootrom_type;
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv chsmUpdatebootrom : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
#define MAX_BOOTROM_UPDATE_INTERVAL 10

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Updatebootrom message.");

    sal_strncpy(dst_file, (char *)path, M_FULLPATH_MAX_LEN);

    sal_getuptime(&cur_time);
    if((0 == g_last_update_bootrom_time.tv_sec)
        || ((cur_time.tv_sec - g_last_update_bootrom_time.tv_sec) > MAX_BOOTROM_UPDATE_INTERVAL))
    {
        bootrom_type = p_card->bootrom_type;
        ret = update_bootrom(dst_file, bootrom_type);
        g_last_update_bootrom_time.tv_sec = cur_time.tv_sec;
        if(ret < 0)
        {
            LCM_LOG_ERR( "Update bootrom fail.");
            return LCM_E_HW_UPDATEBTROM_GEN;
        }
    }
    else
    {
        LCM_LOG_ERR( "Please don't update bootrom frequently, try again after 30 seconds.");
        return LCM_E_HW_UPDATEBTROM_GEN;
    }

    /* Create bootrom update done flag file */
    if ((fp = sal_fopen(GLB_UPDATE_BOOTROM_DONE, "w+")) != NULL)
    {
        sal_fclose(fp);
    }
#endif
    return LCM_E_SUCCESS;
}


int32
lcm_mgt_update_epld(char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret;
    char dst_file[M_FULLPATH_MAX_LEN+1];
    FILE *fp;
    struct stat stat_buf;
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv chsmConfigbootcmd : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Update EPLD message.");
    sal_strncpy(dst_file, (char *)path, M_FULLPATH_MAX_LEN);

    /* Must disable all irqs */
    if(ctc_disable_all_irqs() < 0)
    {
        LCM_LOG_ERR( "Disable all irqs fail.");
        return LCM_E_HW_UPDATEEPLD_GEN;
    }

    /* Create epld update flag */
    if ((fp = sal_fopen(GLB_UPDATE_EPLD_FLAG, "w+")) != NULL)
    {
        sal_fclose(fp);
    }

    ret = epld_update(dst_file, p_card->epld_type);
    if(ret < 0)
    {
        LCM_LOG_ERR( "Update epld fail.");
        return LCM_E_HW_UPDATEEPLD_GEN;
    }

    if(sal_stat(GLB_UPDATE_EPLD_DONE, &stat_buf))
    {
        LCM_LOG_ERR( "Update epld fail.");
        return LCM_E_HW_UPDATEEPLD_GEN;
    }

    if (sal_unlink(GLB_UPDATE_EPLD_FLAG))
    {
        LCM_LOG_ERR( "Remove update EPLD flag fail.");
        return LCM_E_HW_UPDATEEPLD_GEN;
    }
#endif
    return LCM_E_SUCCESS;
}


int32
lcm_mgt_update_phy_firmware(int32 phy_port_idx, char* path)
{
#ifndef _GLB_UML_SYSTEM_
    int ret;
    char dst_file[M_FULLPATH_MAX_LEN+1];
    uint32 port_id;
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR( "Lcm recv chsmUpdatePhyFirmware : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    LCM_LOG_DEBUG(LCM_CHSMTLK, "Receive Update phy firmware message.");

    port_id = phy_port_idx;
    sal_strncpy(dst_file, (char *)path, M_FULLPATH_MAX_LEN);

    if(!(sal_strncmp(dst_file, "check_version", 13)))
    {
        ret = phy_check_firmware_version(port_id, dst_file);
    }
    else
    {
        ret = phy_load_firmware(port_id, dst_file);
    }

    if(ret == RESULT_OK)
    {
        /* Create bootrom update done flag file */
        FILE *fp;
        if ((fp = sal_fopen(GLB_LOAD_PHY_FIRMWARE_DONE, "w+")) != NULL)
        {
            sal_fclose(fp);
        }
    }
    else
    {
        /* Create bootrom update fail flag file */
        FILE *fp;
        if ((fp = sal_fopen(GLB_LOAD_PHY_FIRMWARE_FAIL, "w+")) != NULL)
        {
            sal_fclose(fp);
        }
    }
    return LCM_E_SUCCESS;

#endif
    return LCM_E_SUCCESS;
}


int32
lcm_mgt_set_configbootcmd_exec(char* cmd)
{
#ifndef _GLB_UML_SYSTEM_
    int ret;
    glb_card_t *p_card = NULL;
    char bootcmd[M_FULLPATH_MAX_LEN+1]; /*max len is 18+128+10+8+15+15=194 byte.*/

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv chsmConfigbootcmd : card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    sal_strncpy(bootcmd, cmd, M_FULLPATH_MAX_LEN);
#if ((defined _CTC_ARM_LS1023A_) || (defined _CTC_ARM_CTC5236_)) && (defined CNOS_OS)
    ret = system(bootcmd);
#else
    ret = ctclib_reconstruct_system_cmd_chld_clone(ctclib_reconstruct_system_cmd_exec_str, (void*)bootcmd);
#endif

    if(ret < 0)
    {
        //LCM_LOG_USER(E_ERROR,LCM_3_CONFIG_BOOTCMD_FAIL);
        LCM_LOG_ERR( "Config bootcmd fail.");
        return LCM_E_HW_UPDATEBTROM_GEN;
    }
#endif
    return LCM_E_SUCCESS;
}


int32
lcm_mgt_set_configbootcmd_normal(char* cmd)
{
#ifndef _GLB_UML_SYSTEM_
    int ret;
    glb_card_t *p_card = NULL;
    char bootcmd[M_FULLPATH_MAX_LEN+1]; /*max len is 18+128+10+8+15+15=194 byte.*/

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv chsmConfigbootcmd : card is NULL.");
        return LCM_E_INVALID_PTR;
    }
    sal_strncpy(bootcmd, cmd, M_FULLPATH_MAX_LEN);
    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_BOOTCMD]);
        if(NULL == p_sysenv_info)
        {
            LCM_LOG_ERR("Lcm recv chsmConfigbootcmd : sysenv alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = sysenv_set_bootcmd(p_sysenv_info, bootcmd);
        if(ret < 0)
        {
            //LCM_LOG_USER(E_ERROR,LCM_3_CONFIG_BOOTCMD_FAIL);
            LCM_LOG_ERR("config bootcmd fail.");
            return LCM_E_HW_UPDATEBTROM_GEN;
        }
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_BOOTCMD]);
        if(NULL == p_eeprom_info)
        {
            LCM_LOG_ERR("Lcm recv chsmConfigbootcmd : eeprom alloc is NULL.");
            return LCM_E_INVALID_PTR;
        }
        ret = eeprom_set_bootcmd(p_eeprom_info, bootcmd);
        if(ret < 0)
        {
            //LCM_LOG_USER(E_ERROR,LCM_3_CONFIG_BOOTCMD_FAIL);
            LCM_LOG_ERR("config bootcmd fail.");
            return LCM_E_HW_UPDATEBTROM_GEN;
        }
    }
#endif
    return LCM_E_SUCCESS;
}



int32
lcm_mgt_mgmt_cfg_set(uint8 *buf)
{
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        return LCM_E_INVALID_PTR;
    }

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_MGMT_CFG]);
        if(NULL == p_sysenv_info)
        {
            return LCM_E_INVALID_PTR;
        }
        //ret = sysenv_set_mgmtcfg_info(buf, p_sysenv_info);///TODO:sysenv has no reg-define for mgmt now, so disable it
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_MGMT_CFG]);
        if(NULL == p_eeprom_info)
        {
            return LCM_E_INVALID_PTR;
        }
        //ret = eeprom_set_mgmtcfg_info(buf, p_eeprom_info);///TODO:eeprom has no reg-define for mgmt now, so disable it
    }
#endif
    return ret;
}


int32
lcm_mgt_config_fan_speed_rate(uint16 bottomspeed,uint16 lowspeed,uint16 highspeed,uint16 fullspeed)
{
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_
    glb_card_t *p_card = NULL;
    uint32 fan_module_num = 0;
    uint32 i;
    uint16 cfg_speed[FAN_SPEED_LEVEL_MAX];

    LCM_LOG_DEBUG(LCM_CHSMTLK, "config fan speed");

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        LCM_LOG_ERR("Lcm recv set fan speed: card is NULL.");
        return LCM_E_INVALID_PTR;
    }

    fan_module_num = p_card->fan_module_num;

    cfg_speed[FAN_SPEED_BOTTOM] = bottomspeed;
    cfg_speed[FAN_SPEED_LOW] = lowspeed;
    cfg_speed[FAN_SPEED_HIGH] = highspeed;
    cfg_speed[FAN_SPEED_FULL] = fullspeed;

    for (i = 0; i < fan_module_num; i++)
    {
        ret += fan_user_config_speed_rate(i, cfg_speed, FAN_SPEED_LEVEL_MAX);
    }
#endif
    return ret;
}

int32 
lcm_mgt_set_swver(uint8* buf)
{
    int32 ret =LCM_E_SUCCESS;
    
#ifndef _GLB_UML_SYSTEM_
    int sw_ver_len = 0;
    uint8 sw_ver[MAX_EEPROM_SWVER_LEN+1];
    sal_memset(sw_ver, 0, MAX_EEPROM_SWVER_LEN+1);
    sw_ver_len = sal_strlen((char *)buf);
    sal_memcpy(sw_ver, buf, sw_ver_len);
    glb_card_t *p_card = NULL;

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {
        return LCM_E_INVALID_PTR;
    }

    if (p_card->use_sysenv_api)
    {
        sysenv_info_t *p_sysenv_info = NULL;
        p_sysenv_info = &(p_card->p_sysenv_alloc[SYSENV_SW_VER]);
        if(NULL == p_sysenv_info)
        {
            return LCM_E_INVALID_PTR;
        }
        ret = sysenv_set_sw_ver(p_sysenv_info, sw_ver, sw_ver_len);
        if (ret < 0)
        {
            LCM_LOG_ERR("Set software version fail.");
            return LCM_E_HW_EEPROM_GEN;
        }
    }
    else
    {
        eeprom_info_t *p_eeprom_info = NULL;
        p_eeprom_info = &(p_card->p_eeprom_alloc[EEPROM_SW_VER]);
        if(NULL == p_eeprom_info)
        {
            return LCM_E_INVALID_PTR;
        }
        ret = eeprom_set_sw_ver(p_eeprom_info, sw_ver, sw_ver_len);
        if (ret < 0)
        {
            LCM_LOG_ERR("Set software version fail.");
            return LCM_E_HW_EEPROM_GEN;
        }
    }
#endif       
    return ret; 
}

void lcm_mgt_get_current_serialno(char *serial_no)
{
    FILE *fp;
    char buf[256];

    fp = sal_fopen(GLB_BOARD_INFO_FILE, "r");
    if(NULL == fp)
    {
        return ;
    }
    while(sal_fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];
        if(!sal_strncmp(buf, GLB_SERIES_NUM, 8))
        {
            sal_sscanf(buf, "%s %s", tmp, serial_no);
        }
    }
    sal_fclose(fp);
}


int32 
lcm_mgt_config_sys_poe_enable(uint8 enable)
{
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_    
    glb_card_t *p_card = NULL;
    glb_port_t *p_port = NULL;
    glb_poe_port_info_t *poe_port = NULL;
    uint8 i = 0,j = 0;
    
    LCM_LOG_DEBUG(LCM_LCMGT, "config poe sys enable");

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {    
        LCM_LOG_ERR("Lcm recv set poe enable : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    

    for(i=0;i<p_card->poe_module_num;i++)
    {
        p_card->poe_sys_info[i].sys_cfg.enable = enable;
        ///TODO:here need consider asynchronous access under poe driver
        epld_item_write(0, EPLD_POE_DISABLE_PORT, enable);
        if(enable == 0)
            poe_set_glb_enable(i,enable);/* disable directly */
        else
        {
            for (j = 0; j < p_card->port_num; j++)
            {
                p_port = p_card->pp_port[j];
                poe_port = &p_port->poe_info;
                if(p_port->poe_support&&poe_port->pse_id == i)
                {
                    if(p_port->poe_info.channel_cfg.enable || p_port->poe_info.channel_cfg.force_power)
                        poe_set_enable(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,1);
                    if(p_port->poe_info.channel_cfg.force_power)
                        poe_set_force_power(p_port->poe_info.pse_id,p_port->poe_info.pse_channel_id,1);
                }
            }
        }
    }
#endif
    return ret;

}

int32 
lcm_mgt_config_sys_poe_power_max(uint32 power_max)
{
    int32 ret =0;
#ifndef _GLB_UML_SYSTEM_    
    glb_card_t *p_card = NULL;
    uint8 i = 0;
    uint32 power_val = 0;
    LCM_LOG_DEBUG(LCM_LCMGT, "config poe sys enable");

    p_card = lcm_mgt_get_card();
    if (!p_card)
    {    
        LCM_LOG_ERR("Lcm recv set poe powermax : card is NULL.");
        return LCM_E_INVALID_PTR;
    }    

    for(i=0;i<p_card->poe_module_num;i++)
    {
        power_val = power_max==0?DEFAULT_POE_POWER_MAX:power_max;
        p_card->poe_sys_info[i].sys_cfg.power_limit = power_val;
        poe_set_glb_power(i,power_val);
    }

#endif
    return ret;
}

