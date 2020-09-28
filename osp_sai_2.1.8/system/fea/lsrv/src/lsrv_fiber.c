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


/***************************************************************************************************
 * Name         : lsrv_fiber_evnent_cb
 * Purpose      : creat or delete fiber from lai notification event    
 * Input        : event, add or delete; fiber, fiber information from lai hardware
 * Output       : N/A                  
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
void
lsrv_fiber_event_cb(lai_fiber_event_t  event,lai_fiber_info_t* p_fiber)
{
    tbl_fiber_t fiber;
    tbl_fiber_t* p_db_fiber = NULL;
    tbl_interface_t *p_db_if = NULL;
    char buff[LAI_CONFIG_INFO_STR_MAX] = {0};
    char str[LAI_CONFIG_INFO_STR_MAX] = {0};
    char ifname[GLB_PHY_PORT_FULL_NAME_SZ];

    LSRV_FIBER_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_fiber)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_fiber_event_cb, FIBER is null!\n");
        return ;
    }
    sal_memset(&fiber, 0, sizeof(fiber));

    if(event == LAI_FIBER_EVENT_ADD)
    {
        LSRV_FIBER_DEBUG("FIBER insert in, slot %d, id %d\n", p_fiber->slot_id, (int32)p_fiber->oid);
        fiber.key.port_id = p_fiber->oid;
        p_db_fiber = tbl_fiber_get_fiber(&fiber.key);
        if(p_db_fiber)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_fiber_evnent_cb, FIBER add but fiber is already exist!\n");
            return;
        }
        buff[0] = CMD_ZERO_CHAR;
        str[0] = CMD_ZERO_CHAR;
        sal_strcat(buff, "fiber");
        sal_sprintf(str, "%d", fiber.key.port_id);
        sal_strcat(buff, str);
       // sal_strcpy(fiber.name, buff);
        fiber.slot = p_fiber->slot_id;

        sal_strncpy(fiber.fiber_type_name, p_fiber->fiber_type_name, LAI_NAME_INFO_STR_MAX);
        sal_memcpy(fiber.name, p_fiber->name, FIBER_VENDOR_NAME_LEN);
        /* modified by kcao for bug 39428, OUI not used */        
        sal_memset(fiber.oui, 0, sizeof(fiber.oui));
        sal_memcpy(fiber.pn, p_fiber->pn, FIBER_VENDOR_PN_LEN);
        sal_memcpy(fiber.rev, p_fiber->rev, FIBER_VENDOR_REV_LEN);
        sal_memcpy(fiber.sn, p_fiber->sn, FIBER_VENDOR_SN_LEN);
        sal_strncpy((char*)fiber.compliance_code, (char*)p_fiber->compliance_code, LAI_NAME_INFO_STR_MAX);

        sal_memcpy(fiber.tmpr, p_fiber->tmpr, sizeof(p_fiber->tmpr));
        sal_memcpy(fiber.voltage, p_fiber->voltage, sizeof(p_fiber->voltage));
        sal_memcpy(fiber.bias, p_fiber->bias, sizeof(p_fiber->bias));
        sal_memcpy(fiber.tx_pwr, p_fiber->tx_pwr, sizeof(p_fiber->tx_pwr));
        sal_memcpy(fiber.rx_pwr, p_fiber->rx_pwr, sizeof(p_fiber->rx_pwr));
        sal_memcpy(fiber.tmpr2, p_fiber->tmpr2, sizeof(p_fiber->tmpr2));
        sal_memcpy(fiber.voltage2, p_fiber->voltage2, sizeof(p_fiber->voltage2));
        sal_memcpy(fiber.bias2, p_fiber->bias2, sizeof(p_fiber->bias2));
        sal_memcpy(fiber.tx_pwr2, p_fiber->tx_pwr2, sizeof(p_fiber->tx_pwr2));
        sal_memcpy(fiber.rx_pwr2, p_fiber->rx_pwr2, sizeof(p_fiber->rx_pwr2));
        sal_memcpy(fiber.warn_flag, p_fiber->warn_flag, sizeof(p_fiber->warn_flag));

        fiber.update_cnt = p_fiber->update_cnt;
        fiber.channel_num = p_fiber->channel_num;
        fiber.channel_idx = p_fiber->channel_idx;
        fiber.is_detail = p_fiber->is_detail;
        fiber.fiber_type = p_fiber->fiber_type;
        fiber.fiber_flag = p_fiber->fiber_flag;
        fiber.is_copper = p_fiber->is_copper;
        fiber.speed = p_fiber->speed;
        fiber.ddm_support = p_fiber->ddm_support;
        fiber.externally_calibrated = p_fiber->externally_calibrated;
        fiber.channel_num = p_fiber->channel_num;

        fiber.wavelength[0] =  p_fiber->wavelength[0];
        fiber.wavelength[1] =  p_fiber->wavelength[1];
        fiber.length[0] =  p_fiber->length[0];
        fiber.length[1] =  p_fiber->length[1];
        fiber.length[2] =  p_fiber->length[2];
        fiber.length[3] =  p_fiber->length[3];
        fiber.length[4] =  p_fiber->length[4];

        sal_memset(ifname, 0, sizeof(ifname));
        if_build_ethname_by_slot_port(ifname, p_fiber->slot_id, p_fiber->pannel_port_id, p_fiber->pannel_sub_port_id);
        sal_strcpy(fiber.interface_name, ifname);
        
        tbl_fiber_add_fiber(&fiber);

        /* Fix bug 38464, update phy_type when fiber inserted */
        p_db_if = tbl_interface_get_interface_by_name(fiber.interface_name);
        if (p_db_if)
        {
            p_db_if->phy_type = fiber.fiber_type;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_TYPE);
        }
    }
    else
    {
        fiber.key.port_id = p_fiber->oid;
        p_db_fiber = tbl_fiber_get_fiber(&fiber.key);
        if (NULL == p_db_fiber)
        {
            return;
        }

        LSRV_FIBER_DEBUG("FIBER plugged out, slot %d, id %d\n", p_fiber->slot_id,(int32)p_fiber->oid);
        /* Fix bug 38464, update phy_type when fiber removed */
        p_db_if = tbl_interface_get_interface_by_name(p_db_fiber->interface_name);
        if (p_db_if)
        {
            p_db_if->phy_type = GLB_PHY_TYPE_UNKNOWN;
            tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_PHY_TYPE);
        }
        tbl_fiber_del_fiber(&fiber.key);
    }
    

    return;
}

/***************************************************************************************************
 * Name         : lsrv_fiber_update_fiber
 * Purpose      : update fiber info in cdb
 * Input        : p_db_fiber, fiber info in db, p_lai_fiber, real fiber info from hardware
 * Output       :                 
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fiber_update_fiber(tbl_fiber_t * p_db_fiber, lai_fiber_info_t* p_lai_fiber)
{
    int32 rc = 0;

    LSRV_FIBER_DEBUG("Enter into %s.\n", __FUNCTION__);
    if(NULL == p_db_fiber || NULL == p_lai_fiber)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "lsrv_fiber_update_fiber, FIBER is null!\n");
        return PM_E_NONE;
    }
    if(p_lai_fiber->slot_id != p_db_fiber->slot)
    {
        return PM_E_NONE;
    }
    LSRV_FIBER_DEBUG("FIBER update, slot %d, id %d\n", p_lai_fiber->slot_id, (int32)p_lai_fiber->oid);

    /* modified by kcao for bug 39428, OUI not used */
    sal_memset(p_db_fiber->oui, 0, sizeof(p_db_fiber->oui));
    sal_memcpy(p_db_fiber->pn, p_lai_fiber->pn, FIBER_VENDOR_PN_LEN);
    sal_memcpy(p_db_fiber->rev, p_lai_fiber->rev, FIBER_VENDOR_REV_LEN);
    sal_memcpy(p_db_fiber->sn, p_lai_fiber->sn, FIBER_VENDOR_SN_LEN);
    sal_memcpy(p_db_fiber->tmpr, p_lai_fiber->tmpr, sizeof(p_lai_fiber->tmpr));
    sal_memcpy(p_db_fiber->voltage, p_lai_fiber->voltage, sizeof(p_lai_fiber->voltage));
    sal_memcpy(p_db_fiber->bias, p_lai_fiber->bias, sizeof(p_lai_fiber->bias));
    sal_memcpy(p_db_fiber->tx_pwr, p_lai_fiber->tx_pwr, sizeof(p_lai_fiber->tx_pwr));
    sal_memcpy(p_db_fiber->rx_pwr, p_lai_fiber->rx_pwr, sizeof(p_lai_fiber->rx_pwr));
    sal_memcpy(p_db_fiber->tmpr2, p_lai_fiber->tmpr2, sizeof(p_lai_fiber->tmpr2));
    sal_memcpy(p_db_fiber->voltage2, p_lai_fiber->voltage2, sizeof(p_lai_fiber->voltage2));
    sal_memcpy(p_db_fiber->bias2, p_lai_fiber->bias2, sizeof(p_lai_fiber->bias2));
    sal_memcpy(p_db_fiber->tx_pwr2, p_lai_fiber->tx_pwr2, sizeof(p_lai_fiber->tx_pwr2));
    sal_memcpy(p_db_fiber->rx_pwr2, p_lai_fiber->rx_pwr2, sizeof(p_lai_fiber->rx_pwr2));
    //sal_memcpy(p_db_fiber->warn_flag, p_lai_fiber->warn_flag, sizeof(p_lai_fiber->warn_flag));

    p_db_fiber->update_cnt = p_lai_fiber->update_cnt;
    p_db_fiber->channel_num = p_lai_fiber->channel_num;
    p_db_fiber->channel_idx = p_lai_fiber->channel_idx;
    p_db_fiber->is_detail = p_lai_fiber->is_detail;
    p_db_fiber->fiber_type = p_lai_fiber->fiber_type;
    p_db_fiber->speed = p_lai_fiber->speed;
    p_db_fiber->ddm_support = p_lai_fiber->ddm_support;
    p_db_fiber->externally_calibrated = p_lai_fiber->externally_calibrated;
    p_db_fiber->channel_num = p_lai_fiber->channel_num;

    p_db_fiber->wavelength[0] =  p_lai_fiber->wavelength[0];
    p_db_fiber->wavelength[1] =  p_lai_fiber->wavelength[1];
    p_db_fiber->length[0] =  p_lai_fiber->length[0];
    p_db_fiber->length[1] =  p_lai_fiber->length[1];
    p_db_fiber->length[2] =  p_lai_fiber->length[2];
    p_db_fiber->length[3] =  p_lai_fiber->length[3];
    p_db_fiber->length[4] =  p_lai_fiber->length[4]; 
#if 0
    LSRV_PRINT_CON("lsrv_fiber_update_fiber port %d %s", p_db_fiber->key.port_id, p_db_fiber->name);
    LSRV_PRINT_CON("bias = %f %f %f %f, bias2 = %f %f %f %f", 
        p_db_fiber->bias[0],p_db_fiber->bias[1], p_db_fiber->bias[2],p_db_fiber->bias[3],
        p_db_fiber->bias2[0], p_db_fiber->bias2[1], p_db_fiber->bias2[2], p_db_fiber->bias2[3]);
    LSRV_PRINT_CON("txpwr = %f %f %f %f, txpwr2 = %f %f %f %f \n",
        p_db_fiber->tx_pwr[0],p_db_fiber->tx_pwr[3],p_db_fiber->tx_pwr[2],p_db_fiber->tx_pwr[3],
        p_db_fiber->tx_pwr2[0],p_db_fiber->tx_pwr2[1],p_db_fiber->tx_pwr2[2],p_db_fiber->tx_pwr2[3])
#endif
    PM_E_RETURN(tbl_fiber_set_fiber_field(p_db_fiber, TBL_FIBER_FLD_MAX));

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_fiber_set_iter_status
 * Purpose      : set fiber info from lai layer to db
 * Input        : p_db_fiber, cdb data, parags, iter args.
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fiber_get_iter_status(tbl_fiber_t* p_db_fiber, tbl_iter_args_t* pargs)
{
    int32 rc = 0;
    lai_fiber_api_t*  fiber_api = NULL;
    lai_fiber_info_t lai_fiber;
    lai_object_id_t oid;

    LSRV_FIBER_DEBUG("Enter into %s.", __FUNCTION__);
    if(NULL == p_db_fiber)
    {
        return PM_E_INVALID_PARAM;
    }
    PM_E_RETURN(lai_api_query(LAI_API_FIBER,(void**)&fiber_api));
    oid = p_db_fiber->key.port_id;

    sal_memset(&lai_fiber, 0, sizeof(lai_fiber));
    PM_E_RETURN(fiber_api->get_fiber_info(oid, &lai_fiber));

    if (lai_fiber.updated)
    {
        PM_E_RETURN(lsrv_fiber_update_fiber(p_db_fiber, &lai_fiber));
    }

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : lsrv_fiber_routine_handle
 * Purpose      : poll all fiber status and update to db
 * Input        : 
 * Output       :               
 * Return       : PM_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
lsrv_fiber_routine_handle()
{
    tbl_fiber_iterate((TBL_ITER_CB_FUNC) lsrv_fiber_get_iter_status, NULL);

    return PM_E_NONE;
}

