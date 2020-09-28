/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-12-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "lsrv_inc.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/
#define LSRV_CARD_ROUTINE_TIME_INTERVAL   1

static ctc_task_t *g_lsrv_card_routine_timer = NULL;

extern uint32 is_openflow_ipv6_profile;

extern void
lcm_mgt_get_combo_info(FILE* fp);

int32
_lsrv_card_change_end_year(char *output, char *input, uint32 len)
{
#if 0
    char *p1 = NULL;
    char *p2 = NULL;
    
    sal_memset(output, 0, len);
    
    p1 = input;
    p2 = sal_strstr(input, "-20");  /* 2004-20XX */
    if (p2)
    {
        p2[0] = CMD_ZERO_CHAR;
        p2++;
        while(sal_isdigit(*p2))
        {
            p2++;
        }

        sal_strcat(output, p1);
        sal_strcat(output, "-""2019");//OEM_CENTEC_YEAR
        sal_strcat(output, p2);
    }
    else
    {
        sal_strncpy(output, input, len);
    }
#endif
    return 0;
}

void
lsrv_card_update_cst_info(lai_card_info_t* p_card)
{
    tbl_version_t version;
    sal_memset(&version, 0, sizeof(version));
    
    if (p_card->is_cst)
    {
        sal_strncpy(version.cst_company, p_card->cst_company, SYS_INFO_SIZE);
        sal_strncpy(version.cst_hw_type, p_card->cst_hw_type, SYS_INFO_SIZE);
        sal_strncpy(version.cst_hw_info, p_card->cst_hw_info, SYS_INFO_SIZE);
        sal_strncpy(version.cst_hw_ver,  p_card->cst_hw_ver, SYS_INFO_SIZE);
        sal_strncpy(version.cst_sw_ver,  p_card->cst_sw_ver, SYS_INFO_SIZE);
        version.is_cst = p_card->is_cst;
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_CST_COMPANY);
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_CST_HW_TYPE);
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_CST_HW_INFO);
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_CST_HW_VER);
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_CST_SW_VER);
        tbl_version_set_version_field(&version, TBL_VERSION_FLD_IS_CST);
        /* delay for to ovs process at first */
        sal_task_sleep(10);
    }
    return;
}
/***************************************************************************************************
 * Name         : lsrv_card_event_cb
 * Purpose      : creat or delete card from lai notification event    
 * Input        : event, add or delete; card, card information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_card_event_cb(lai_card_event_t  event,lai_card_info_t* p_card)
{
    tbl_card_t card;
    tbl_card_t* p_db_card = NULL;
    tbl_chassis_t* p_db_chassis = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};
    int32 i;
    lai_card_api_t* card_api = NULL;
    lai_reboot_info_t reboot_info;
    tbl_reboot_info_t db_reboot_info;

    sal_memset(&reboot_info, 0, sizeof(reboot_info));
    
    LSRV_CARD_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_card)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_card_event_cb, CARD is null!\n");
    }
    sal_memset(&card, 0, sizeof(card));

    if(event == LAI_CARD_EVENT_ADD)
    {
        LSRV_CARD_DEBUG("CARD insert in, slot %d, id %d\n", p_card->slot, (int32)p_card->oid);
        card.key.id = p_card->oid;
        p_db_card = tbl_card_get_card(&card.key);
        if(p_db_card)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_card_event_cb, CARD add but card is already exist!\n");
            return;
        }

        /*update chassis information here cause pizzabox, if it is chassis, the chassis info should be update
           from local control-card*/

        p_db_chassis = tbl_chassis_get_chassis();
        if(NULL == p_db_chassis)
        {
            return;
        }
        p_db_chassis->slot_num += 1;
        sal_memcpy(p_db_chassis->sys_mac, p_card->mac, sizeof(p_db_chassis->sys_mac));
        tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_SYS_MAC);
        tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_SLOT_NUM);

        /* added by hansf for save profile type from lai card */
        p_db_chassis->cur_stm_mode = p_card->profile_type;

        /*modified by guoxd for bug 54208, 2019-10-31*/
        #ifndef OFPRODUCT
            if (GLB_STM_MODE_OF_IPV6 == p_card->profile_type)
            {
                p_db_chassis->cur_stm_mode = GLB_STM_MODE_DEFAULT;
                is_openflow_ipv6_profile = 0;
             }
        #endif
        
    #ifdef OFPRODUCT
        #if 1 //defined(GOLDENGATE) || defined(GREATBELT)
            if (GLB_STM_MODE_OF_IPV6 == p_card->profile_type)
            {
                p_db_chassis->cur_stm_mode = GLB_STM_MODE_OF_IPV6;
                is_openflow_ipv6_profile = 1;
            }
            else
            {
                p_db_chassis->cur_stm_mode = GLB_STM_MODE_DEFAULT;
            }
        #else
            p_db_chassis->cur_stm_mode = GLB_STM_MODE_DEFAULT;
        #endif
    #endif
        tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_CUR_STM_MODE);
        /* end hansf */

        if(GLB_FLAG_ISSET(p_card->caps, LAI_CARD_CAPS_SUPPORT_SAVE_REBOOT_INFO))
        {
            p_db_chassis->support_reboot_info = 1;
        }
        tbl_chassis_set_chassis_field(p_db_chassis, TBL_CHASSIS_FLD_SUPPORT_REBOOT_INFO);
        
        lai_api_query(LAI_API_CARD,(void**)&card_api);
        card_api->get_reboot_info(0, &reboot_info);

        for(i=0; i<reboot_info.reboot_times; i++)
        {
            sal_memset(&db_reboot_info, 0, sizeof(db_reboot_info));
            db_reboot_info.key.index = (p_card->slot << 16) + i;
            sal_memcpy(db_reboot_info.reboot_type, reboot_info.reboot_type[i], sizeof(db_reboot_info.reboot_type));
            sal_memcpy(db_reboot_info.reboot_time, reboot_info.reboot_time[i], sizeof(db_reboot_info.reboot_time));
            tbl_reboot_info_add_reboot_info(&db_reboot_info);
        }
        
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "card");
        sal_sprintf(str, "%d", card.key.id);
        sal_strcat(buff, str);
        //sal_strcpy(card.name, buff);
        card.slot = p_card->slot;

        sal_strncpy(card.hardware_name, p_card->hardware_name, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.enterprise_oid, p_card->snmp_enterprise_oid, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.product_name, p_card->product_name, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.package_name, p_card->package_name, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.company_name, p_card->vendor, LAI_NAME_INFO_STR_MAX);
        _lsrv_card_change_end_year(card.company_name, p_card->vendor, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.serial_no, p_card->serial_number, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.bootrom_ver, p_card->bootrom_version, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.epld_ver, p_card->epld_version, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.hardware_type, p_card->hardware_name, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.hardware_ver, p_card->hardware_version, LAI_NAME_INFO_STR_MAX);
        sal_strncpy(card.bootcmd, p_card->bootcmd, LAI_NAME_INFO_STR_MAX);

        card.epld_status = p_card->epld_status;
        card.flash_size = p_card->flash_size;
        card.dram_size = p_card->sdram_size;
        card.port_num = p_card->port_num;
        card.tmpr_low = p_card->tmpr_low;
        card.tmpr_high = p_card->tmpr_high;
        card.tmpr_critical = p_card->tmpr_crit;
        card.platform_type = p_card->platform_type;
        card.board_type = p_card->board_type;
        card.board_series = p_card->board_series;

        if(GLB_FLAG_ISSET(p_card->status, LAI_CARD_STATUS_READY))
        {
            card.status = 1;
        }

        /* should before add card for version in ovs */
        lsrv_card_update_cst_info(p_card);
        tbl_card_add_card(&card);
    }
    else
    {
        LSRV_CARD_DEBUG("CARD removed, slot %d,id = %d\n", p_card->slot,  (int32)p_card->oid);
        card.key.id = p_card->oid;
        tbl_card_del_card(&card.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_card_update_card
 * Purpose      : update card info in cdb
 * Input        : p_db_card, card info in db, p_lai_card, real card info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_update_card(tbl_card_t * p_db_card, lai_card_info_t* p_lai_card)
{
    int32 rc = 0;

    LSRV_CARD_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_card || NULL == p_lai_card)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_card_update_card, CARD is null!\n");
    }
    if(p_lai_card->oid != p_db_card->key.id)
    {
        return PM_E_NONE;
    }
    LSRV_CARD_DEBUG("CARD update, slot %d\n", (int32)p_lai_card->oid);

    if((GLB_FLAG_ISSET(p_lai_card->status, LAI_CARD_STATUS_READY))&&
        (p_db_card->status == 1))\
    {
        LSRV_CARD_DEBUG("CARD update, already ready\n");
        return PM_E_NONE;
    }

    if((!GLB_FLAG_ISSET(p_lai_card->status, LAI_CARD_STATUS_READY))&&
        (p_db_card->status == 0))\
    {
        LSRV_CARD_DEBUG("CARD update, hw not ready\n");
        return PM_E_NONE;
    }

    if(GLB_FLAG_ISSET(p_lai_card->status, LAI_CARD_STATUS_READY))
    {
        p_db_card->status = 1;
    }
    else
    {
        p_db_card->status = 0;
    }
    LSRV_CARD_DEBUG("CARD update, update card status to %d\n", p_db_card->status);

    PM_E_RETURN(tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_STATUS));
    PM_E_RETURN(tbl_card_set_card_field(p_db_card, TBL_CARD_FLD_EPLD_STATUS));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_card_set_iter_status
 * Purpose      : set card info from lai layer to db
 * Input        : p_db_card, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_get_iter_status(tbl_card_t* p_db_card, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_card_api_t*  card_api = NULL;
    lai_card_info_t lai_card;
    lai_object_id_t oid;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_card)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
    oid = p_db_card->key.id;
    

    PM_E_RETURN(card_api->get_card_info(oid, &lai_card));
    PM_E_RETURN(lsrv_card_update_card(p_db_card, &lai_card));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_card_routine_handle
 * Purpose      : poll all card status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_routine_handle()
{
    tbl_card_iterate((TBL_ITER_CB_FUNC) lsrv_card_get_iter_status, NULL);

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : lsrv_card_get_slot_no
 * Purpose      : get slot id of local card
 * Input        : void
 * Output       : slotid                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_get_local_slot_no()
{
    lai_card_api_t*  card_api = NULL;
    lai_card_info_t card;
    int32 rc = 0;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    sal_memset(&card, 0, sizeof(card));
    
    PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));

    PM_E_RETURN(card_api->get_card_info(0, &card));
    
    return card.slot;
}

/***************************************************************************************************
 * Name         : lsrv_card_routine_timer_handle
 * Purpose      : polling card, port, fiber, card, psu, sensor status, and update db 
 * Input        : void
 * Output       :                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
static void
lsrv_card_routine_timer_handle(void *p_data)
{ 
    static uint32 count = 0;

    g_lsrv_card_routine_timer = ctc_task_add_timer(TASK_PRI_NORMAL, lsrv_card_routine_timer_handle, 
        NULL, LSRV_CARD_ROUTINE_TIME_INTERVAL);

    if(count % 3 == 0)
    {
       lsrv_fan_routine_handle();   
       lsrv_sensor_routine_handle(); 
       lsrv_poe_routine_handle();
    }
    if(count % 4 == 0)
    {
       lsrv_psu_routine_handle();
       lsrv_fiber_routine_handle();
       
    }
    if(count % 5 == 0)
    {
       lsrv_vcm_routine_handle();
    }
    /* only update card information once  */
    /* modified by liuht for bug 40668, 2016-09-09 */
#if 0    
    if(count == 0)
    {
        
       lsrv_card_routine_handle();
       //lsrv_port_routine_handle(); 
    }
#endif
    count++;

    return;
}

/***************************************************************************************************
 * Name         : lsrv_card_set_sensor_tmpr_threshold_one
 * Purpose      : set all sensor temperature threshold in db
 * Input        : low, high, critical, board temperature threshold
 * Output       :                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_set_sensor_tmpr_threshold_one(tbl_sensor_t *p_sensor, tbl_iter_args_t *pargs)
{
    int32 low; 
    int32 high; 
    int32 critical;
    
    low = *(int32*)pargs->argv[0];
    high = *(int32*)pargs->argv[1];
    critical = *(int32*)pargs->argv[2];

    if (p_sensor->is_chip)
    {
        return PM_E_NONE;
    }
    
    p_sensor->tmpr_low = low;
    p_sensor->tmpr_high = high;
    p_sensor->tmpr_critical = critical;
    tbl_sensor_set_sensor_field(p_sensor, TBL_SENSOR_FLD_TMPR_LOW);
    tbl_sensor_set_sensor_field(p_sensor, TBL_SENSOR_FLD_TMPR_HIGH);
    tbl_sensor_set_sensor_field(p_sensor, TBL_SENSOR_FLD_TMPR_CRITICAL);
    
    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_card_set_sensor_tmpr_threshold
 * Purpose      : set all sensor temperature threshold in db
 * Input        : low, high, critical, board temperature threshold
 * Output       :                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_set_sensor_tmpr_threshold(int32 low, int32 high, int32 critical)
{
    tbl_iter_args_t args;
        
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &low;
    args.argv[1] = &high;
    args.argv[2] = &critical;
 
    tbl_sensor_iterate((TBL_ITER_CB_FUNC)lsrv_card_set_sensor_tmpr_threshold_one, &args);

    return PM_E_NONE;
}

int32
lsrv_card_reboot()
{
    lai_object_id_t oid;
    lai_card_api_t*  card_api = NULL;
    tbl_card_t *p_card = tbl_card_get_default_card();
    int32 rc = 0;

    PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
    oid = p_card->slot;
    PM_E_RETURN(card_api->reboot(oid, NULL));

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : lsrv_card_sync
 * Purpose      : load card cdb to lai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_card_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_card_t *p_card = NULL;

    int32 rc = 0;
    lai_card_api_t*  card_api = NULL;
    lai_object_id_t oid;
    
    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_card = (tbl_card_t*)p_tbl;
        oid = p_card->slot;
        switch (field_id)
        {
            case TBL_CARD_FLD_TMPR_LOW:
            case TBL_CARD_FLD_TMPR_HIGH:
            case TBL_CARD_FLD_TMPR_CRITICAL:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_tmpr_info(oid, p_card->tmpr_low, p_card->tmpr_high, p_card->tmpr_critical));
                PM_E_RETURN(lsrv_card_set_sensor_tmpr_threshold(p_card->tmpr_low, p_card->tmpr_high, p_card->tmpr_critical));
                break;

            case TBL_CARD_FLD_BOOTCMD:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_bootcmd(oid, p_card->bootcmd));
                break;

            case TBL_CARD_FLD_UPDATE_EPLD_NAME:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->update_epld(oid, p_card->update_epld_name));
                break;

            case TBL_CARD_FLD_UPDATE_BOOTROM_NAME:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->update_bootrom(oid, p_card->update_bootrom_name));
                break;

            case TBL_CARD_FLD_REBOOT:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->reboot(oid, NULL));
                break;

            case TBL_CARD_FLD_UPDATE_PHY_FIRMWARE_NAME:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->update_phy_firmware(oid, p_card->update_phy_firmware_port_idx, p_card->update_phy_firmware_name));
                break;

            case TBL_CARD_FLD_FAN_SPEED_BOTTOM:
            case TBL_CARD_FLD_FAN_SPEED_LOW:
            case TBL_CARD_FLD_FAN_SPEED_HIGH:
            case TBL_CARD_FLD_FAN_SPEED_FULL:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_fan_speed(oid, p_card->fan_speed_bottom, p_card->fan_speed_low, p_card->fan_speed_high, p_card->fan_speed_full));
                break;
            case TBL_CARD_FLD_GLB_POE_ENABLE:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_poe_enable(oid, p_card->glb_poe_enable));
                break;
            case TBL_CARD_FLD_GLB_POE_POWER_MAX:
                PM_E_RETURN(lai_api_query(LAI_API_CARD,(void**)&card_api));
                PM_E_RETURN(card_api->set_poe_power_max(oid, p_card->glb_poe_power_max));
                break;

            default:
                break;
        }
        break;

    default:
        break;
    }

    return PM_E_NONE;
}

int32
lsrv_process_update_oem_info_cmd(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    lai_card_api_t*  card_api = NULL;
    lai_oem_info_t oem_info;
    lai_object_id_t oid;
    int32 rc = PM_E_NONE;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    if (NULL == p_db_card)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&oem_info, 0, sizeof(oem_info));
    
    sal_strncpy(oem_info.package_name, argv[0], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.product_name, argv[1], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.hardware_type, argv[2], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.snmp_enterprise_oid, argv[3], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.chassis_type, argv[4], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.four_sfp_name, argv[5], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.two_sfpp_name, argv[6], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.four_sfpp_name, argv[7], LAI_NAME_INFO_STR_MAX);
    sal_strncpy(oem_info.company_name, argv[8], LAI_NAME_INFO_STR_MAX);
    PM_E_RETURN(lai_api_query(LAI_API_CARD, (void**)&card_api));
    oid = p_db_card->key.id;
    
    PM_E_RETURN(card_api->set_oem_info(oid, &oem_info));

    return rc;
}

int32
lsrv_process_show_combo_cmd(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf (fp, "Current Combo Status: ");
    lcm_mgt_get_combo_info(fp);

    sal_fclose(fp);

    return 0;
}

int32
lsrv_process_update_sw_ver_cmd(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    lai_card_api_t*  card_api = NULL;
    lai_sw_version_info_t sw_ver_info;
    lai_object_id_t oid;
    int32 rc = PM_E_NONE;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);
    if (NULL == p_db_card)
    {
        return PM_E_INVALID_PARAM;
    }
    sal_memset(&sw_ver_info, 0, sizeof(sw_ver_info));

    //no set sw-ver 
    if(argc == 0)
    {    
        PM_E_RETURN(lai_api_query(LAI_API_CARD, (void**)&card_api));
        oid = p_db_card->key.id;
        PM_E_RETURN(card_api->set_sw_version_info(oid, &sw_ver_info));
    }
    else
    {
        sal_strncpy(sw_ver_info.sw_version, argv[0], sal_strlen(argv[0]));
        PM_E_RETURN(lai_api_query(LAI_API_CARD, (void**)&card_api));
        oid = p_db_card->key.id;
        PM_E_RETURN(card_api->set_sw_version_info(oid, &sw_ver_info));
    }
    return rc;
}

/***************************************************************************************************
 * Name         : lsrv_card_init
 * Purpose      : init lsrv card task
 * Input        : void
 * Output       :                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_card_init(void* data)
{
    lai_card_notification_t noti_cb = {0};
    int32 rc = 0;

    noti_cb.on_card_event = lsrv_card_event_cb;
    noti_cb.on_fiber_event = lsrv_fiber_event_cb;
    noti_cb.on_port_event = lsrv_port_event_cb;
    noti_cb.on_fan_event = lsrv_fan_event_cb;
    noti_cb.on_psu_event = lsrv_psu_event_cb;
    noti_cb.on_thermal_event = lsrv_sensor_event_cb;
    noti_cb.on_synce_dpll_state_event = lsrv_synce_dpll_state_event_cb;
    noti_cb.on_vcm_event = lsrv_vcm_event_cb;
    noti_cb.on_poe_event = lsrv_poe_event_cb;
    lai_api_initialize(0, &noti_cb);

    lai_card_api_t*  card_api = NULL;

    LSRV_CARD_DEBUG("Enter into %s.", __FUNCTION__);

    lai_api_query(LAI_API_CARD,(void**)&card_api);
    if(card_api)
    {
        rc = card_api->init();
        if(rc)
        {
            log_sys(M_MOD_INTERNAL, E_ALERT, "lsrv_card_init FAIL !!!");
        }
    }
    
    
    cdb_pm_subscribe_tbl(TBL_CARD, lsrv_card_sync,NULL);
    cdb_pm_subscribe_tbl(TBL_CHASSIS, lsrv_chassis_sync,NULL);
    cdb_pm_subscribe_tbl(TBL_IFNAME_INFO, NULL,lsrv_ifname_info_sync);

    lsrv_card_routine_handle();

    cdb_register_cfg_act_cb(ACT_UPDATE_OEM_INFO, lsrv_process_update_oem_info_cmd);
    cdb_register_cfg_act_cb(ACT_SHOW_COMBO, lsrv_process_show_combo_cmd);

    cdb_register_cfg_act_cb(ACT_UPDATE_SW_VERSION, lsrv_process_update_sw_ver_cmd);
    g_lsrv_card_routine_timer = ctc_task_add_timer(TASK_PRI_NORMAL, lsrv_card_routine_timer_handle, 
        NULL, 0);

    return;
    
}
