/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_mlag.h"
#include "hsrv_ptp.h"
#include "hsrv_wb.h"
#include "ctc_sai_port.h"
#include "../../../../sai/src//hostif/ctc_sai_hostif.h"
#include "gen/tbl_chip_define.h"
#include "gen/tbl_chip.h"
#include "gen/tbl_bhm_global_define.h"
#include "gen/tbl_bhm_global.h"
#include "gen/tbl_route_global_define.h"
#include "gen/tbl_route_global.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "ctc_register.h"
#include "ctc_api.h"

/******************************************************************************
* Defines and Macros
******************************************************************************/
typedef struct
{
    sal_time_t ts;
    char ifname[IFNAME_SIZE];
} blink_down_info_t;

/******************************************************************************
* Global and Declaration
******************************************************************************/
extern uint32 g_ip_redirects_global_en;
extern sal_mutex_t *g_hsrv_brgif_mutex;
ctclib_slist_t *g_blink_down_list = NULL;
extern int32
hsrv_tap_init_nat();

extern int32
hsrv_openflow_groupdb_loop_update_ffgroup();

/***************************************************************************************************
 * Name         : hsrv_switch_get_router_id 
 * Purpose      : Get router id which is from SAI         
 * Input        : N/A
 * Output       : routerid, from SAI                
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_switch_get_router_id(sai_object_id_t* routerid)
{
    tbl_fea_global_t* p_fea_global = NULL;

    p_fea_global = tbl_fea_global_get_fea_global();
    if(NULL == p_fea_global)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_PTR_CHECK(routerid);
    *routerid = p_fea_global->router_id;
    
    return HSRV_E_NONE;
}

int32
hsrv_switch_init_done_handle()
{
    hsrv_mlag_init_done_cb();

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_sys_global_sync 
 * Purpose      : load sys global table cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/

int32
hsrv_dhcprelay_set_excp_type(uint32 ifindex, int32 enable)
{
    uint16      vid = 0;
    uint32      flags = 0;
    sai_attribute_t  attr[1];
    tbl_interface_t  *p_interface = NULL;
    sai_port_api_t   *port_api = NULL;
    /* modified by liwh for bug 51199, 2019-03-05 */
    tbl_vlan_key_t   vlan_key;
    tbl_vlan_t          *p_vlan = NULL;
    /* liwh end */

    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    p_interface = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_interface)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /*2. get interface type*/
    if (GLB_IF_TYPE_VLAN_IF == p_interface->hw_type)
    {
        if (enable)
        {
            flags = HSRV_VLAN_FLAG_DHCP_RELAY_EN;
        }
        vid = tbl_interface_get_vid_by_ifindex(p_interface->ifindex);

        /* modified by liwh for bug 51199, 2019-03-05 
             if dhcp snooping is set, dhcp packet should be sent to CPU */
        if (!enable)
        {
            vlan_key.vid = vid;
            p_vlan = tbl_vlan_get_vlan(&vlan_key);
            if (p_vlan && p_vlan->dhcsnooping)
            {
                flags = HSRV_VLAN_FLAG_DHCP_SNOOPING_EN;
            }
        }
        /* liwh end */
        
        HSRV_IF_ERROR_RETURN(hsrv_vlan_set_dhcp_excp_type(vid, flags));
        
        return HSRV_E_NONE;
    }

    /*3. for routed port and routed agg port*/
    
    attr[0].id = SAI_PORT_ATTR_DHCPRELAY_EXCEPTION_TYPE;
    attr[0].value.s32= enable;

    if (GLB_IF_TYPE_LINKAGG_IF == p_interface->hw_type)
    {
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, attr));
    }
    else if (GLB_IF_TYPE_PORT_IF == p_interface->hw_type)
    {
        HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, attr));
    }

    return HSRV_E_NONE;
}

static int32
hsrv_dhcrelay_service_intf_onoff(tbl_route_if_t *p_intf, tbl_iter_args_t *pargs)
{
    int32 onoff = *((int32 *)pargs);

    if (NULL == p_intf) 
    {
        return HSRV_E_FAIL;
    }

/* modified by liwh for bug 51199, 2019-03-05 
     all dhcp client/relay/snooping can set dhcp exception to CPU, should count dhcp_pdu_enabled number */
#if 0
#if 0
    if (p_intf->dhcp_pdu_enabled != onoff && p_sys_global 
            && p_sys_global->dhcp_service_enable && p_sys_global->dhcp_relay_enable) 
#else
    if (p_intf->dhcp_pdu_enabled != onoff) 
#endif
    {
        hsrv_dhcprelay_set_excp_type(p_intf->ifindex, onoff);
        p_intf->dhcp_pdu_enabled = onoff;
        HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_intf, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
    }
#endif

    /* modified by liwh for bug 51199, 2019-03-05 
         all dhcp client/relay/snooping can set dhcp exception to CPU, should count dhcp_pdu_enabled number */
    if (onoff)
    {
        /* dhcp pdu is set to cpu already */
        if (p_intf->dhcp_pdu_enabled)
        {
            p_intf->dhcp_pdu_enabled++;
        }
        /* dhcp pdu should be set to cpu first time */
        else
        {
            hsrv_dhcprelay_set_excp_type(p_intf->ifindex, onoff);
            p_intf->dhcp_pdu_enabled = 1;    
        }

        HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_intf, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
    }
    else
    {
        if (p_intf->dhcp_pdu_enabled)
        {
            p_intf->dhcp_pdu_enabled--;

            if (0 == p_intf->dhcp_pdu_enabled)
            {
                hsrv_dhcprelay_set_excp_type(p_intf->ifindex, onoff);
            }

            HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_intf, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
        }
    }
    /* liwh end */

    return HSRV_E_NONE;
}

int32
hsrv_dhcp_relay_service_onoff(int onoff)
{
    tbl_route_if_iterate((TBL_ITER_CB_FUNC)hsrv_dhcrelay_service_intf_onoff, (tbl_iter_args_t *)(&onoff));
    
    return HSRV_E_NONE;
}

/* modified by liwh for bug 47781, 2018-09-17 */
int32
hsrv_dhcp_relay_update_service_when_add_delete_rtif(tbl_route_if_t *p_router_if, bool add)
{
    tbl_sys_global_t *p_sys_global = tbl_sys_global_get_sys_global();

    if (add)
    {
        if (p_sys_global->dhcp_service_enable && p_sys_global->dhcp_relay_enable) 
        {
            hsrv_dhcprelay_set_excp_type(p_router_if->ifindex, TRUE);
            p_router_if->dhcp_pdu_enabled = TRUE;
            HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_router_if, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
        }
    }
    else
    {
         if (p_router_if->dhcp_pdu_enabled)
        {
            hsrv_dhcprelay_set_excp_type(p_router_if->ifindex, FALSE);
            p_router_if->dhcp_pdu_enabled = FALSE;
            HSRV_IF_ERROR_RETURN(tbl_route_if_set_route_if_field(p_router_if, TBL_ROUTE_IF_FLD_DHCP_PDU_ENABLED));
        }
    }

    return HSRV_E_NONE;
}
/* liwh end */

#define MAX_BLINK_DOWN_COUNT    1024

int32
hsrv_carrier_blink_down_process(char *ifname)
{
    tbl_interface_key_t key;
    tbl_interface_t *p_db_if = NULL;
    struct timeval tv;
    blink_down_info_t *p_info = NULL;
    blink_down_info_t *p_info_last = NULL;

    sal_memset(&key, 0, sizeof(key));
    sal_strncpy(key.name, ifname, IFNAME_SIZE);
    p_db_if = tbl_interface_get_interface(&key);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    /* Added by kcao for bug 50708, if not set down hold time, does not record blink down */
    if (0 == p_db_if->carrier_down_holdtime_interval)
    {
        return HSRV_E_NONE;
    }
        
    p_info = XCALLOC(MEM_FEA_TEMP, sizeof(blink_down_info_t));
    if (p_info)
    {
        sal_gettimeofday(&tv);
        p_info->ts = tv.tv_sec;
        sal_strncpy(p_info->ifname, ifname, IFNAME_SIZE);
        sal_mutex_lock(g_hsrv_brgif_mutex);
        ctclib_slistnode_insert_sort(g_blink_down_list, p_info);
        if (g_blink_down_list->count > MAX_BLINK_DOWN_COUNT)
        {
            p_info_last = GETDATA(CTCLIB_SLISTHEAD(g_blink_down_list));
            if (p_info_last)
            {
                ctclib_slistnode_delete(g_blink_down_list, p_info_last);
                XFREE(MEM_FEA_TEMP, p_info_last);
            }
        }
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        log_sys(M_MOD_INTF, E_ERROR, "Interface %s detect link blink down", ifname);
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_carrier_blink_down_clear(cfg_cmd_para_t *para)
{
    blink_down_info_t *p_info = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    CTCLIB_SLIST_LOOP_DEL(g_blink_down_list, p_info, listnode, next)
    {
        ctclib_slistnode_delete(g_blink_down_list, p_info);
        XFREE(MEM_FEA_TEMP, p_info);
    }
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

static int32
_hsrv_carrier_blink_down_show(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    blink_down_info_t *p_info = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 index = 0;
    struct tm *tmptm = NULL;
    char strtime[CMD_BUF_64];

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "%-6s %-18s %-25s\n", "Index", "Interface", "Time");
    sal_fprintf(fp, "------+------------------+-------------------------\n");
    sal_mutex_lock(g_hsrv_brgif_mutex);
    CTCLIB_SLIST_LOOP_REV(g_blink_down_list, p_info, listnode)
    {
        index++;
        tmptm = sal_localtime(&p_info->ts);
        sal_time_strf(strtime, sizeof(strtime), "%F %T", tmptm);
        sal_fprintf(fp, "%-6u %-18s %-25s\n", index, p_info->ifname, strtime);
    }
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    sal_fclose(fp);

    return HSRV_E_NONE;
}

int32
hsrv_sys_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_sys_global_t* p_sys_global = NULL;
    sai_switch_api_t*  switch_api;
    sai_attribute_t attr[1];
    uint16_t dst_port = 0;
    /* modified by liwh for bug 51199, 2019-03-05 */
    int32       old_dhcp_relay = FALSE;
    int32       new_dhcp_relay = FALSE;
    /* liwh end */

    tbl_sys_global_t *p_db_sys_global = NULL;
    p_db_sys_global = tbl_sys_global_get_sys_global();
    if (NULL == p_db_sys_global)
    {
        return HSRV_E_FAIL;
    }
        
    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    //CTCLIB_PRINT_CON("hsrv_switch_sync, OPER = %d, field = %d", oper, field_id);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_sys_global = (tbl_sys_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_SYS_GLOBAL_FLD_INIT_DONE:
            hsrv_switch_init_done_handle();
#ifdef TAPPRODUCT  
            //hsrv_tap_init_nat();
#endif
            break;

        case TBL_SYS_GLOBAL_FLD_ROUTE_MAC:
            attr[0].id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
            sal_memcpy(attr[0].value.mac, p_sys_global->route_mac,sizeof(mac_addr_t));
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;

        case TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE:
            attr[0].id = SAI_SWITCH_ATTR_IPG_STORM_CONTROL;
            attr[0].value.booldata = p_sys_global->ipg_storm_control_enable;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;

        case TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE:
            /*1.  dhcp relay */
            /* modified by liwh for bug 50994, 2019-02-13 
                 both dhcp relay or dhcp snooping shouldn't discard dhcp in fea */
            /* if (p_sys_global->dhcp_relay_enable != p_db_sys_global->dhcp_relay_enable)
            {
                ctc_sai_hostif_db_dhcp_sys_syn(p_sys_global->dhcp_relay_enable);
            }*/

            /* modified by liwh for bug 51199, 2019-03-05 */
            old_dhcp_relay = p_db_sys_global->dhcp_service_enable && p_db_sys_global->dhcp_relay_enable;
            new_dhcp_relay = p_sys_global->dhcp_service_enable && p_sys_global->dhcp_relay_enable;

            if (old_dhcp_relay != new_dhcp_relay)
            {
                hsrv_dhcp_relay_service_onoff(new_dhcp_relay);
            }
            /* liwh end */

            /*2.  dhcp snooping */
            /* modified by liwh for bug 50994, 2019-02-13 
                 both dhcp relay or dhcp snooping shouldn't discard dhcp in fea */
            /* if (p_sys_global->dhcp_snooping_enable != p_db_sys_global->dhcp_snooping_enable)
            {
                ctc_sai_hostif_db_dhcp_sys_syn(p_sys_global->dhcp_snooping_enable);
            }*/
            break;

        case TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE:
            /* modified by liwh for bug 51199, 2019-03-05 */
            old_dhcp_relay = p_db_sys_global->dhcp_service_enable && p_db_sys_global->dhcp_relay_enable;
            new_dhcp_relay = p_sys_global->dhcp_service_enable && p_sys_global->dhcp_relay_enable;

            if (old_dhcp_relay != new_dhcp_relay)
            {
                hsrv_dhcp_relay_service_onoff(new_dhcp_relay);
            }
            /* liwh end */

            /* modified by liwh for bug 50994, 2019-02-13 
                 both dhcp relay or dhcp snooping shouldn't discard dhcp in fea */
            /* if (p_sys_global->dhcp_relay_enable != p_db_sys_global->dhcp_relay_enable)
            {
                ctc_sai_hostif_db_dhcp_sys_syn(p_sys_global->dhcp_relay_enable);
            } */
            break;

        case TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE:
            /* modified by liwh for bug 50994, 2019-02-13 
                 both dhcp relay or dhcp snooping shouldn't discard dhcp in fea */
            /* if (p_sys_global->dhcp_snooping_enable != p_db_sys_global->dhcp_snooping_enable)
            {
                ctc_sai_hostif_db_dhcp_sys_syn(p_sys_global->dhcp_snooping_enable);
            }*/ 
            break;

        case TBL_SYS_GLOBAL_FLD_PTF_PORT:
            break;
        case TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH:
            attr[0].id = SAI_SWITCH_ATTR_CUSTOM_TRUNCTION_LENGTH;
            attr[0].value.s32 = p_sys_global->trunction_length; 
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;

        case TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA:
            attr[0].id = SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_DMAC;
            sal_memcpy(attr[0].value.mac, p_sys_global->tap_ts_macda,sizeof(sai_mac_t)); 
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
            
        case TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA:
            attr[0].id = SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_SMAC;
            sal_memcpy(attr[0].value.mac, p_sys_global->tap_ts_macsa,sizeof(sai_mac_t)); 
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
            
        case TBL_SYS_GLOBAL_FLD_ETHER_TYPE:
            attr[0].id = SAI_SWITCH_ATTR_CUSTOM_TIMESTAMP_ETHER_TYPE;
            attr[0].value.u16 = p_sys_global->ether_type;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;

        /*added by yejl to discard the ospf packet when system memory is not enough, bug43529, 2017-03-31*/
        case TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE:
            attr[0].id = SAI_SWITCH_ATTR_MEMORY_CHECK_STATE;
            attr[0].value.u32 = p_sys_global->system_memchk_state;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;

        case TBL_SYS_GLOBAL_FLD_STARTUP_DONE:
#ifndef USW
            HSRV_IF_ERROR_RETURN(ctc_interrupt_register_event_cb(CTC_EVENT_PTP_TX_TS_CAPUTRE, _hsrv_ptp_tx_ts_isr));
#endif
            break;

        case TBL_SYS_GLOBAL_FLD_TELNET_PORT:
            dst_port = p_sys_global->telnet_port;
            hsrv_cpu_traffic_update_telnet_entry(dst_port);
            break;

        case TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX:
            hsrv_port_stats_get_all_stats(p_sys_global->sflow_counter_ifindex);
            break;

        default:
            break;
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

#define SAI_DEBUG_FLAG_FUNC         0x01
#define SAI_DEBUG_FLAG_PARAM        0x02
#define SAI_DEBUG_FLAG_PACKET       0x03
extern uint32 g_sai_debug_flag;
extern uint32 g_ctc_debug_flag;

int32
hsrv_chip_set_chip_type()
{
    tbl_chip_t *p_db_chip = tbl_chip_get_chip();

#ifdef GOLDENGATE
    p_db_chip->type = GLB_CHIP_GOLDENGATE;
#elif GREATBELT
    p_db_chip->type = GLB_CHIP_GREATEBELT;
#elif DUET2
    p_db_chip->type = GLB_CHIP_DUET2;
#elif TSINGMA
    p_db_chip->type = GLB_CHIP_TSINGMA;
#endif

    tbl_chip_set_chip_field(p_db_chip, TBL_CHIP_FLD_TYPE);

    return HSRV_E_NONE;
}

int32
hsrv_switch_cmd_process_blink_down(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    {
    case CDB_OPER_SET:
        _hsrv_carrier_blink_down_clear(para);
        break;

    case CDB_OPER_GET:
        _hsrv_carrier_blink_down_show(para);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

extern int32
hagt_acl_iotime_clear();

extern int32
hagt_acl_iotime_show(FILE *fp);

static int32
_hsrv_acl_iotime_show(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    hagt_acl_iotime_show(fp);
    sal_fclose(fp);

    return HSRV_E_NONE;
}

int32
hsrv_switch_cmd_process_acl_iotime(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    switch (para->oper)
    {
    case CDB_OPER_SET:
        hagt_acl_iotime_clear();
        break;

    case CDB_OPER_GET:
        _hsrv_acl_iotime_show(para);
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}

int32
hsrv_switch_cmd_process_ptf_sai(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    uint32 value = 0;
    int32 rc = HSRV_E_NONE;
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 2, 3, para->p_rs);
        if (0 == sal_strcmp("ptf", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[1], para->p_rs);
        }
        else if (0 == sal_strcmp("sai", argv[0]))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 3, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[2], para->p_rs);
            if (0 == sal_strcmp("function", argv[1]))
            {
                if (value)
                {
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_FUNC);
                }
                else
                {
                    GLB_UNSET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_FUNC);
                }
            }
            else if (0 == sal_strcmp("parameter", argv[1]))
            {
                if (value)
                {
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PARAM);
                }
                else
                {
                    GLB_UNSET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PARAM);
                }
            }
            else if (0 == sal_strcmp("sdk-function", argv[1]))
            {
                if (value)
                {
                    g_ctc_debug_flag = TRUE;
                }
                else
                {
                    g_ctc_debug_flag = FALSE;
                }
            }
            else if (0 == sal_strcmp("packet", argv[1]))
            {
                if (value)
                {
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PACKET);
                }
                else
                {
                    GLB_UNSET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PACKET);
                }
            }
            else if (0 == sal_strcmp("all", argv[1]))
            {
                if (value)
                {
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_FUNC);
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PARAM);
                    GLB_SET_FLAG(g_sai_debug_flag, SAI_DEBUG_FLAG_PACKET);
                    g_ctc_debug_flag = TRUE;
                }
                else
                {
                    g_sai_debug_flag = 0;
                    g_ctc_debug_flag = FALSE;
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid parameter %s", argv[1]);
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter");    
        }
        break;
        
    default:
        CFG_INVALID_PARAM_RET("Invalid action");  
        break;
    }

    return PM_E_NONE;
}


int32
_hsrv_route_global_sync_after_set_ipv6_enable(tbl_route_global_t* p_rt_glb)
{
    tbl_fea_global_t* p_fea_global          = NULL;
    sai_switch_api_t*  switch_api             = NULL;
    
    sai_attribute_t             attr[1];

    /*1. debug out infor*/
    HSRV_PTR_CHECK(p_rt_glb);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));

    p_fea_global = tbl_fea_global_get_fea_global();
    HSRV_PTR_CHECK(p_fea_global);

    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_IPV6_ENABLE;
    attr[0].value.u32 = p_rt_glb->ipv6_enable;
    
    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));

    return PM_E_NONE;
}

int32
_hsrv_route_global_sync_after_set_ip_redirects_global_en(tbl_route_global_t* p_rt_glb)
{
    /*1. debug out infor*/
    HSRV_PTR_CHECK(p_rt_glb);
    
    g_ip_redirects_global_en = p_rt_glb->ip_redirects_global_en;

    return PM_E_NONE;
}

int32
hsrv_route_global_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
      void *p_tbl, void *p_ds)
{
    tbl_route_global_t* p_rt_glb = NULL;

    HSRV_IPUC_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
    p_rt_glb = (tbl_route_global_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_ROUTE_GLOBAL_FLD_IPV6_ENABLE:
            HSRV_IF_ERROR_RETURN(_hsrv_route_global_sync_after_set_ipv6_enable(p_rt_glb));
            break;

        case TBL_ROUTE_GLOBAL_FLD_IP_REDIRECTS_GLOBAL_EN:
            HSRV_IF_ERROR_RETURN(_hsrv_route_global_sync_after_set_ip_redirects_global_en(p_rt_glb));
            break;
            
        default:
            return PM_E_INVALID_PARAM;
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_switch_init 
 * Purpose      : adaptor switch init      
 * Input        : N/A
 * Output       : N/A             
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_switch_init()
{
    sai_switch_api_t*  switch_api;
    sai_virtual_router_api_t*  router_api;
    sai_attribute_t attr[3];
    sai_object_id_t routerid = 55;
    tbl_fea_global_t* p_fea_global = NULL;  
#ifndef GREATBELT
    uint32 max_frame_index = 0;
    int32 rc = 0;
#else
    ctc_pdu_l3pdu_key_t l3pdu_key;
    ctc_pdu_global_l3pdu_action_t ctc_l3pdu_action;
#endif
    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);

    hsrv_chip_set_chip_type();

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VIRTUAL_ROUTER,(void**)&router_api));
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
    attr[0].value.booldata = TRUE;
    attr[1].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
    attr[1].value.booldata = FALSE; 
    attr[2].id = SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_ACTION;
    attr[2].value.s32 = SAI_PACKET_ACTION_TRAP;     
    HSRV_IF_ERROR_RETURN(router_api->create_virtual_router(&routerid, 3, attr));

    p_fea_global = tbl_fea_global_get_fea_global();
    if(NULL == p_fea_global)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "%s, get fea global fail", __FUNCTION__);
    }
    else
    {
        p_fea_global->router_id = routerid;
        HSRV_IF_ERROR_RETURN(tbl_fea_global_set_fea_global_field(p_fea_global, TBL_FEA_GLOBAL_FLD_ROUTER_ID));
    }

#ifdef TAPPRODUCT
    /* TAP not return when copp init error for tap v350 flow6 profile */
    hsrv_packet_event_init();
#else
    HSRV_IF_ERROR_RETURN(hsrv_packet_event_init());
#endif
    HSRV_IF_ERROR_RETURN(hsrv_acl_init());
    /*8. init default max frame size*/
#ifndef GREATBELT
    for(max_frame_index = CTC_FRAME_SIZE_0; max_frame_index < CTC_FRAME_SIZE_MAX; max_frame_index++)
    {
        if(CTC_FRAME_SIZE_0 == max_frame_index)
        {
            //HSRV_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_0));
            rc = ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_0);
            if (rc != CTC_E_NONE)
            {
                HSRV_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_GG1_MAX_FRAME_SIZE_0));
            }           
        }
        else
        {
            //HSRV_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_1));
            rc = ctc_set_max_frame_size(max_frame_index, GLB_DFT_MAX_FRAME_SIZE_1);
            if (rc != CTC_E_NONE)
            {
                HSRV_IF_ERROR_RETURN(ctc_set_max_frame_size(max_frame_index, GLB_DFT_GG1_MAX_FRAME_SIZE_1));
            } 
        }
    }
#else
    /*currently use sai function to init max frame_size*/
#endif

    ctc_sai_port_set_all_default();

#ifdef GREATBELT
    l3pdu_key.l3hdr_proto = GLB_PDU_L3HDR_PROTO_ICMPv6;
    HSRV_IF_ERROR_RETURN(ctc_l3pdu_classify_l3pdu(
        CTC_PDU_L3PDU_TYPE_L3HDR_PROTO,
        0,
        &l3pdu_key));

    ctc_l3pdu_action.entry_valid = TRUE;
    ctc_l3pdu_action.action_index = 6;
     HSRV_IF_ERROR_RETURN(ctc_l3pdu_set_global_action(
            CTC_PDU_L3PDU_TYPE_L3HDR_PROTO,
            0,
            &ctc_l3pdu_action));
#endif

/*
    attr[0].id = SAI_SWITCH_ATTR_DEFAULT_PORT_VLAN_ID;
    attr[0].value.u16 = 1;
    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));


    attr[0].id = SAI_SWITCH_ATTR_SWITCHING_MODE;
    attr[0].value.u32 = SAI_SWITCHING_MODE_STORE_AND_FORWARD;
    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
*/
    cdb_register_cfg_act_cb(ACT_PTF_SAI, hsrv_switch_cmd_process_ptf_sai);
    cdb_register_cfg_act_cb(ACT_CARRIER_BLINK_DOWN, hsrv_switch_cmd_process_blink_down);
    cdb_register_cfg_act_cb(ACT_ACL_IOTIME, hsrv_switch_cmd_process_acl_iotime);

    cdb_pm_subscribe_tbl(TBL_ROUTE_GLOBAL, NULL, hsrv_route_global_sync_after);
    
#ifdef TAPPRODUCT
    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_DISCARD_SAME_MACDASA_PKT;
    attr[0].value.booldata = TRUE;
    switch_api->set_switch_attribute(&attr[0]);

    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_DISCARD_SAME_IPDASA_PKT;
    attr[0].value.booldata = TRUE;
    switch_api->set_switch_attribute(&attr[0]);

    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_DISCARD_MCAST_SA_PKT;
    attr[0].value.booldata = TRUE;
    switch_api->set_switch_attribute(&attr[0]);
#else
    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_DISCARD_SAME_MACDASA_PKT;
    attr[0].value.booldata = TRUE;
    switch_api->set_switch_attribute(&attr[0]);
#endif

    g_blink_down_list = ctclib_slist_create(NULL, NULL);

/* modified by liwh for bug 52440, 2019-06-18 */
//#ifdef  GOLDENGATE
#if defined(GOLDENGATE) || defined(DUET2) || defined(TSINGMA)
/* liwh end */
    uint32 randomLogShitf = 0; 
    ctc_global_ctl_set(CTC_GLOBAL_IGS_RANDOM_LOG_SHIFT, &randomLogShitf);/*modify by yangl for bug 45988*/
    ctc_global_ctl_set(CTC_GLOBAL_EGS_RANDOM_LOG_SHIFT, &randomLogShitf);
#endif
    return HSRV_E_NONE;
}

int32
hsrv_brg_set_vlan_tpid(tbl_brg_global_t* p_brg_global, uint32 tpid_type)
{
    sai_switch_api_t*  switch_api;
    sai_attribute_t attr[1];

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    
    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_SVLAN_TPID;

    /* added by hansf for bug 48781, hybrid set global tpid use fun hsrv_openflow_set_stpid/hsrv_openflow_set_ctpid */
#ifdef OFPRODUCT
    return HSRV_E_NONE;
#endif
    
    if (GLB_TPID_TYPE_1 == tpid_type)
    {
        attr[0].value.s32 = p_brg_global->vlan_tpid_type1;
    }
    else
    {
        attr[0].value.s32 = p_brg_global->vlan_tpid_type2 + (1 << 16);
    }
    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_set_stpid(uint32 svlan_tpid)
{
    sai_switch_api_t*  switch_api;
    sai_attribute_t attr[1];

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    
    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_SVLAN_TPID;
    
    attr[0].value.s32 = svlan_tpid;

    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));

    return HSRV_E_NONE;
}

int32
hsrv_openflow_set_ctpid(uint32 cvlan_tpid)
{
    sai_switch_api_t*  switch_api;
    sai_attribute_t attr[1];

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    
    attr[0].id = SAI_SWITCH_ATTR_CUSTOM_CVLAN_TPID;
    
    attr[0].value.s32 = cvlan_tpid;

    HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));

    return HSRV_E_NONE;
}

/* system can set fdb flow valid only after reboot, pre-create all vlan */
int32
hsrv_openflow_set_fdb_flow_en()
{
    sai_vlan_api_t*  vlan_api = NULL;
    uint16 vid = 1;
    sai_attribute_t attr[1];
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    
    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    attr[0].id = SAI_VLAN_ATTR_LEARN_DISABLE;
    attr[0].value.booldata = TRUE;

    for (vid = 1; vid <= GLB_VLAN_MAX; ++vid)
    {
        vlan_api->create_vlan(vid);

        key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (NULL != p_db_vlan)
        {
            continue;
        }
        vlan_api->set_vlan_attribute(vid, attr);
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_set_ffgroup_switchback_en(uint8 enable)
{
    /* update all ffgroup choosed bucket */
    if (enable)
    {
        hsrv_openflow_groupdb_loop_update_ffgroup();
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_switch_sync 
 * Purpose      : load switch global table cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_switch_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_brg_global_t* p_brg_global = NULL;
    sai_switch_api_t*  switch_api;
    sai_attribute_t attr[1];
    

    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    //CTCLIB_PRINT_CON("hsrv_switch_sync, OPER = %d, field = %d", oper, field_id);
        
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_brg_global = (tbl_brg_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_BRG_GLOBAL_FLD_OPER_FDB_AGING_TIME:
            attr[0].id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
            attr[0].value.u32 = p_brg_global->oper_fdb_aging_time;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
        case TBL_BRG_GLOBAL_FLD_PORT_ISOLATE_MODE:
            attr[0].id = SAI_SWITCH_ATTR_ISOLATION_MODE;
            if (GLB_PORT_ISOLATE_L2 == p_brg_global->port_isolate_mode)
            {
                attr[0].value.s32 = SAI_ISOLATION_MODE_L2;
            }
            else
            {
                attr[0].value.s32 = SAI_ISOLATION_MODE_ALL;
            }
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
        case TBL_BRG_GLOBAL_FLD_FDB_LOOP_ERRDIS_EN:
            HSRV_IF_ERROR_RETURN(hsrv_fdb_set_fdb_loop_detect_en(p_brg_global));
            break;
        case TBL_BRG_GLOBAL_FLD_FDB_LOOP_MAX_SIZE:
            HSRV_IF_ERROR_RETURN(hsrv_fdb_set_fdb_loop(p_brg_global));
            break;
        case TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE1:
            HSRV_IF_ERROR_RETURN(hsrv_brg_set_vlan_tpid(p_brg_global, GLB_TPID_TYPE_1));
            break;
        case TBL_BRG_GLOBAL_FLD_VLAN_TPID_TYPE2:
            HSRV_IF_ERROR_RETURN(hsrv_brg_set_vlan_tpid(p_brg_global, GLB_TPID_TYPE_2));
            break;

        default:
            break;
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

extern int32
ctc_sdk_deinit(void);
extern int32
lsrv_card_reboot();

int32
hsrv_switch_bhm_reboot_normal() 
{
    lsrv_card_reboot();
    
    return HSRV_E_NONE;
}

int32
hsrv_switch_bhm_reboot_fast() 
{
    lsrv_card_reboot();
    ctc_sdk_deinit();
    
    return HSRV_E_NONE;
}

int32
hsrv_switch_bhm_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_bhm_global_t* p_bhm = NULL;
    
    HSRV_SWITCH_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_SET:
        p_bhm = (tbl_bhm_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_BHM_GLOBAL_FLD_REBOOT:
            if (GLB_WARMBOOT_NORMAL == p_bhm->reboot)
            {
                hsrv_switch_bhm_reboot_normal();
            }
            else if (GLB_WARMBOOT_FAST == p_bhm->reboot)
            {
                hsrv_switch_bhm_reboot_fast();
            }
            else if (GLB_WARMBOOT_HITLESS == p_bhm->reboot)
            {
                hsrv_wb_do_reboot();
            }
            break;
        default:
            break;
        }

        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

 /**
 * Routine Description:
 *   @brief Monitor event notification callback
 *
 * Arguments:
 *    @param[in] port_oid - OID of the port
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *                     attr_list[0] - u32 (sai_monitor_type_t)
 *                     attr_list[1] - u32 (sai_monitor_event_type_t)
 *                     attr_list[2] - u32 (buffer count, valid for SAI_MONITOR_TYPE_BUFFER) or u64 (latency, valid for SAI_MONITOR_TYPE_LATENCY)
 *
 * Return Values:
 *		None
 */
void hsrv_switch_monitor_event_notification_cb(
    _In_ sai_object_id_t  port_oid,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list  
    )
{
    tbl_interface_t* p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};
    
    hsrv_interface_get_if_by_oid(port_oid, &p_db_if);
    if (NULL == p_db_if)
    {
        return;
    }
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);      
    
    if (SAI_MONITOR_TYPE_BUFFER == attr_list[0].value.u32)
    {
        if (SAI_MONITOR_EVENT_START == attr_list[1].value.u32)
        {
             logid_sys(LOG_MONITOR_3_BUFFER_START, ifname_ext, attr_list[2].value.u32);
        }
        else if (SAI_MONITOR_EVENT_END == attr_list[1].value.u32)
        {
            logid_sys(LOG_MONITOR_4_BUFFER_END, ifname_ext, attr_list[2].value.u32); 
        }
    }
    else if (SAI_MONITOR_TYPE_LATENCY == attr_list[0].value.u32)
    {
        if (SAI_MONITOR_EVENT_START == attr_list[1].value.u32)
        {
            logid_sys(LOG_MONITOR_3_LATENCY_START, ifname_ext, attr_list[2].value.u64);
        }
        else if (SAI_MONITOR_EVENT_END == attr_list[1].value.u32)
        {
            logid_sys(LOG_MONITOR_4_LATENCY_END, ifname_ext, attr_list[2].value.u64);
        }
    }

    return;
}

int32
test_get_switch()
{
    sai_switch_api_t*  switch_api = NULL;
    uint32_t sai_attr_count = 2;
    sai_object_id_t portid_array[64];
    sai_attribute_t sai_attr_list[2];
    sai_status_t rc = SAI_STATUS_SUCCESS;

    /* get from SAI */
    sai_attr_list[0].id = SAI_SWITCH_ATTR_PORT_NUMBER;
    sai_attr_list[1].id = SAI_SWITCH_ATTR_PORT_LIST;
    sai_attr_list[1].value.objlist.count = 64;
    sai_attr_list[1].value.objlist.list = portid_array;
    rc = sai_api_query(SAI_API_SWITCH, (void**)&switch_api);
    rc = switch_api->get_switch_attribute(sai_attr_count, sai_attr_list);

    return rc;
}

