/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-08-07
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_aclqos_if.h"
#include "gen/tbl_sflow_global_define.h"
#include "gen/tbl_sflow_global.h"
#include "gen/tbl_sflow_counter_port_define.h"
#include "gen/tbl_sflow_counter_port.h"
#include "gen/tbl_port_stats_rate_define.h"
#include "gen/tbl_port_stats_rate.h"
#include "gen/tbl_aclqos_if_stats_define.h"
#include "gen/tbl_aclqos_if_stats.h"
#include "gen/tbl_fea_global_define.h"
#include "gen/tbl_fea_global.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
ctc_task_t *g_p_stats_scan_time = NULL; 


/******************************************************************************
* Defines and Macros
******************************************************************************/

int32
hsrv_port_stats_check_bad_crc_changes(uint32 ifindex, tbl_port_stats_t* p_db_port_stats, tbl_port_stats_t* p_new_port_stats)
{
    char ifname_ext[IFNAME_SIZE];
    char ifname[IFNAME_SIZE];
    uint64 new_crc = 0;
    uint64 total_crc = 0;
    
    if (p_db_port_stats->bad_crc == p_new_port_stats->bad_crc)
    {
        return HSRV_E_NONE;
    }

    /* check new < current */
    if (p_new_port_stats->bad_crc < p_db_port_stats->bad_crc)
    {
        return HSRV_E_NONE;
    }
    new_crc = p_new_port_stats->bad_crc - p_db_port_stats->bad_crc;
    total_crc = p_new_port_stats->bad_crc;
    tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2FULL(ifname, ifname_ext);

    logid_sys(LOG_INTERFACE_4_CRCPKT, ifname_ext, new_crc, total_crc);
    
    return HSRV_E_NONE;
}

int32
hsrv_port_stats_clear_stats(uint32 ifindex)
{
    tbl_interface_t* p_if = NULL;
    tbl_interface_t *p_if_mem = NULL; 
    tbl_interface_t *p_db_if = NULL;
    sai_port_api_t*  port_api = NULL;
    ds_lag_t*        p_lag = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_object_id_t     port_oid = 0;
    tbl_port_stats_t port_stats;
    tbl_port_stats_t* p_port_stats;
    tbl_port_stats_t* p_db_port_stats;
    uint32 ts;

    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);

    p_if = tbl_interface_get_interface_by_ifindex(ifindex);
    HSRV_PTR_CHECK(p_if);
    
    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF && p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&port_stats, 0x0, sizeof(port_stats));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    ts = (uint32)ctc_time_boottime_sec(NULL);
    
    if (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF && p_if->lag)
    {
        p_lag = p_if->lag;

        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_db_if = tbl_interface_get_interface(&p_if_mem->key);
            if (!p_db_if)
            {
                continue;
            }
            hsrv_interface_ifindex_to_objectid(p_db_if->ifindex, &port_oid);
            HSRV_IF_ERROR_RETURN(port_api->clear_port_all_stats(port_oid));
            p_port_stats = &port_stats;
            p_port_stats->key.ifindex = p_db_if->ifindex;
            p_port_stats->port_id = p_db_if->portid;

            p_db_port_stats = tbl_port_stats_get_port_stats(&p_port_stats->key);
            if(NULL == p_db_port_stats)
            {
                continue;
            }
            p_port_stats->last_clear_ts = ts;
            HSRV_IF_ERROR_RETURN(tbl_port_stats_set_port_stats_field(p_port_stats, TBL_PORT_STATS_FLD_MAX));
        }
        
        hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
        HSRV_IF_ERROR_RETURN(port_api->clear_port_all_stats(port_oid));
    }

    if (p_if->hw_type == GLB_IF_TYPE_PORT_IF)
    {
        hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);
        HSRV_IF_ERROR_RETURN(port_api->clear_port_all_stats(port_oid));
    }

    p_port_stats = &port_stats;
    p_port_stats->key.ifindex = ifindex;
    p_port_stats->port_id = p_if->portid;

    p_port_stats->last_clear_ts = ts;
    p_db_port_stats = tbl_port_stats_get_port_stats(&p_port_stats->key);
    if(NULL == p_db_port_stats)
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    HSRV_IF_ERROR_RETURN(tbl_port_stats_set_port_stats_field(p_port_stats, TBL_PORT_STATS_FLD_MAX));

    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_port_stats_get_all_stats
 * Purpose      : get ifindex all port stats           
 * Input        : ifindex, p_port_stats
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_stats_get_all_stats(uint32 ifindex)
{
    tbl_interface_t* p_if = NULL;
    tbl_interface_t *p_if_mem = NULL; 
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_port_api_t*  port_api = NULL;
    sai_object_id_t     port_oid = 0;
    sai_port_stat_counter_t counter_port_stats[SAI_PORT_STAT_IF_OUT_PAUSE_PKTS+1];
    uint64_t port_stats_rlt[SAI_PORT_STAT_IF_OUT_PAUSE_PKTS+1];
    tbl_port_stats_t port_stats;
    tbl_port_stats_t* p_port_stats;
    tbl_port_stats_t* p_db_port_stats;
    int i;
    ds_lag_t *p_lag = NULL;

    p_if = tbl_interface_get_interface_by_ifindex(ifindex);

    if (NULL == p_if)
    {
        return HSRV_E_NONE;
    }
    if (p_if->hw_type != GLB_IF_TYPE_PORT_IF && p_if->hw_type != GLB_IF_TYPE_LINKAGG_IF)
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&port_stats, 0x0, sizeof(port_stats));
    sal_memset(&port_stats_rlt, 0x0, sizeof(port_stats_rlt));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));

    for (i = 0; i <= SAI_PORT_STAT_IF_OUT_PAUSE_PKTS; i++)
    {
        counter_port_stats[i] = i;
    }

    if (p_if->hw_type == GLB_IF_TYPE_LINKAGG_IF)
    {
        if (NULL == p_if->lag)
        {
            return HSRV_E_NONE; 
        }
        p_lag = p_if->lag;
        CTCLIB_SLIST_LOOP(p_lag->member_ports.obj_list, p_if_mem, listnode)
        {
            p_db_if = tbl_interface_get_interface(&p_if_mem->key);
            if (!p_db_if)
            {
                continue;
            }
            hsrv_interface_ifindex_to_objectid(p_db_if->ifindex, &port_oid);
            /* 0 only update, not return value */
            HSRV_IF_ERROR_RETURN(port_api->get_port_stats(port_oid, counter_port_stats, 
                                0, port_stats_rlt));
        }

        /* get LAG's stats */
        hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);

        HSRV_IF_ERROR_RETURN(port_api->get_port_stats(port_oid, counter_port_stats, 
                                    SAI_PORT_STAT_IF_OUT_PAUSE_PKTS+1, port_stats_rlt));
        p_port_stats = &port_stats;
        p_port_stats->key.ifindex = ifindex;
        p_port_stats->port_id = p_if->portid;
        p_port_stats->octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_OCTETS];
        p_port_stats->pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_PKTS];
        p_port_stats->uc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_UCAST_PKTS];
        p_port_stats->brdc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BROADCAST_PKTS];
        p_port_stats->mc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_MULTICAST_PKTS];
        p_port_stats->oam_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_OAM_PKTS];
        p_port_stats->undersize_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS];
        p_port_stats->oversize_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS];
        p_port_stats->jabber_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_JABBERS];
        p_port_stats->mac_rcv_error = port_stats_rlt[SAI_PORT_STAT_IF_IN_RUNTS_PKTS];
        p_port_stats->bad_crc = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS];
        p_port_stats->frame_error = port_stats_rlt[SAI_PORT_STAT_IF_IN_FRAME_PKTS];
        p_port_stats->drop_events = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS];
        p_port_stats->pause_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_PAUSE_PKTS];
        p_port_stats->octets_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_OCTETS];
        p_port_stats->pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_PKTS];
        p_port_stats->uc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_UCAST_PKTS];
        p_port_stats->brdc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS];
        p_port_stats->mc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS];
        p_port_stats->oam_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_OAM_PKTS];
        p_port_stats->underruns = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS];
        p_port_stats->mac_transmit_err = port_stats_rlt[SAI_PORT_STAT_IF_OUT_ERRORS];
        p_port_stats->pause_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_PAUSE_PKTS];
        p_port_stats->fcs_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS];
        p_port_stats->fcs_octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS];
        p_port_stats->fcs_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_ERRORS];
        p_port_stats->fcs_octets_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS];
        p_port_stats->fragments_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_COLLISIONS];
        p_port_stats->bad_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BAD_PKTS];
        p_port_stats->bad_octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BAD_OCTETS];
        p_port_stats->pkts_64_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS];
        p_port_stats->pkts_65_to_127_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS];
        p_port_stats->pkts_128_to_255_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS];
        p_port_stats->pkts_256_to_511_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS];
        p_port_stats->pkts_512_to_1023_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS];
        p_port_stats->pkts_1024_to_1518_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS];
    }
    else
    {
        hsrv_interface_ifindex_to_objectid(ifindex, &port_oid);

        HSRV_IF_ERROR_RETURN(port_api->get_port_stats(port_oid, counter_port_stats, 
                                    SAI_PORT_STAT_IF_OUT_PAUSE_PKTS+1, port_stats_rlt));
        p_port_stats = &port_stats;
        p_port_stats->key.ifindex = ifindex;
        p_port_stats->port_id = p_if->portid;
        p_port_stats->octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_OCTETS];
        p_port_stats->pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_PKTS];
        p_port_stats->uc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_UCAST_PKTS];
        p_port_stats->brdc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BROADCAST_PKTS];
        p_port_stats->mc_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_MULTICAST_PKTS];
        p_port_stats->oam_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_OAM_PKTS];
        p_port_stats->undersize_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS];
        p_port_stats->oversize_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS];
        p_port_stats->jabber_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_JABBERS];
        p_port_stats->mac_rcv_error = port_stats_rlt[SAI_PORT_STAT_IF_IN_RUNTS_PKTS];
        p_port_stats->bad_crc = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS];
        p_port_stats->frame_error = port_stats_rlt[SAI_PORT_STAT_IF_IN_FRAME_PKTS];
        p_port_stats->drop_events = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS];
        p_port_stats->pause_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_PAUSE_PKTS];
        p_port_stats->octets_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_OCTETS];
        p_port_stats->pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_PKTS];
        p_port_stats->uc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_UCAST_PKTS];
        p_port_stats->brdc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS];
        p_port_stats->mc_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS];
        p_port_stats->oam_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_OAM_PKTS];
        p_port_stats->underruns = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS];
        p_port_stats->mac_transmit_err = port_stats_rlt[SAI_PORT_STAT_IF_OUT_ERRORS];
        p_port_stats->pause_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_PAUSE_PKTS];
        p_port_stats->fcs_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_PKTS];
        p_port_stats->fcs_octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_CRC_ERR_OCTETS];
        p_port_stats->fcs_pkts_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_ERRORS];
        p_port_stats->fcs_octets_send = port_stats_rlt[SAI_PORT_STAT_IF_OUT_FCS_ERR_OCTETS];
        p_port_stats->fragments_pkts = port_stats_rlt[SAI_PORT_STAT_ETHER_STATS_COLLISIONS];
        p_port_stats->bad_pkts_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BAD_PKTS];
        p_port_stats->bad_octets_rcv = port_stats_rlt[SAI_PORT_STAT_IF_IN_BAD_OCTETS];
        p_port_stats->pkts_64_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS];
        p_port_stats->pkts_65_to_127_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS];
        p_port_stats->pkts_128_to_255_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS];
        p_port_stats->pkts_256_to_511_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS];
        p_port_stats->pkts_512_to_1023_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS];
        p_port_stats->pkts_1024_to_1518_rcv = port_stats_rlt[SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS];
    }
    
    p_db_port_stats = tbl_port_stats_get_port_stats(&p_port_stats->key);
    if(NULL == p_db_port_stats)
    {
        return HSRV_E_INVALID_PARAM;
    }

    p_port_stats->last_clear_ts = p_db_port_stats->last_clear_ts;
    hsrv_port_stats_check_bad_crc_changes(ifindex, p_db_port_stats, p_port_stats);
    HSRV_IF_ERROR_RETURN(tbl_port_stats_set_port_stats_field(p_port_stats, TBL_PORT_STATS_FLD_MAX));
    return HSRV_E_NONE;
}

void
hsrv_port_stats_scan_timeout(void *parg)
{
    tbl_fea_global_t *p_fea_glb = tbl_fea_global_get_fea_global();
    static uint32 count = 0;
    uint32 i = 0;
    ctclib_slistnode_t *listnode = NULL;  
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    tbl_port_stats_t *p_db_port_stats = NULL;
    
    CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_db_port_stats, listnode)
    {
        if (i>=count)
        {
            hsrv_port_stats_get_all_stats(p_db_port_stats->key.ifindex);
            count++;
            g_p_stats_scan_time = ctc_task_add_timer_msec(TASK_PRI_NORMAL,
                    hsrv_port_stats_scan_timeout, NULL, p_fea_glb->port_stats_ms);
            return;
        }
        i++;
    }
    count = 0;
    g_p_stats_scan_time = ctc_task_add_timer_msec(TASK_PRI_NORMAL,
            hsrv_port_stats_scan_timeout, NULL, p_fea_glb->port_stats_ms);

    return;
}

void
hsrv_port_queue_stats_scan_timeout(void *parg)
{
    tbl_aclqos_if_stats_t *p_aclqos_if_stats = NULL;
    tbl_aclqos_if_stats_t  aclqos_if_stats;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_interface = NULL;
    tbl_openflow_interface_t *p_openflow_interface = NULL;
    tbl_openflow_interface_key_t p_if_key;
    ctclib_slistnode_t              *listnode = NULL;  
    static uint32 count = 0;
    uint32 i = 0;
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_interface, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF == p_interface->hw_type)
        {
            if (i>=count)
            {
                p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(p_interface->key.name);
                if(!p_aclqos_if_stats)
                {
                    sal_memset(&aclqos_if_stats, 0, sizeof(tbl_aclqos_if_stats_t));
                    sal_strcpy(aclqos_if_stats.key.name, p_interface->key.name);
                    tbl_aclqos_if_stats_add_aclqos_if_stats(&aclqos_if_stats);
                    p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(aclqos_if_stats.key.name);
                }
                
                sal_memset(&p_if_key, 0x0, sizeof(tbl_openflow_interface_key_t));
                sal_strcpy(p_if_key.name, p_aclqos_if_stats->key.name);
                p_openflow_interface = tbl_openflow_interface_get_openflow_interface(&p_if_key);
                if (p_openflow_interface && p_openflow_interface->openflow_enable)
                {
                    hsrv_qos_get_queue_stats(p_aclqos_if_stats);
                }
                count++;
                ctc_task_add_timer_msec(TASK_PRI_NORMAL,
                        hsrv_port_queue_stats_scan_timeout, NULL, GLB_PORT_POOL_QUEUE_STATS_INTERVAL);
                return;
            }
            i++;            
        }
    }

    count = 0;
    ctc_task_add_timer_msec(TASK_PRI_NORMAL,
            hsrv_port_queue_stats_scan_timeout, NULL, GLB_PORT_POOL_QUEUE_STATS_INTERVAL);

    return;
}


static int32
_hsrv_port_stats_dump_all(tbl_port_stats_t *p_port_stats,  cfg_cmd_para_t *para)
{
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    tbl_iter_args_t args;
    FILE *fp = NULL;
    field_parse_t field;
    ctclib_slistnode_t *listnode = NULL;
    tbl_port_stats_t *p_db_port_stats = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    if (NULL == p_port_stats)
    {
        CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_db_port_stats, listnode)
        {
            tbl_port_stats_dump_one(p_db_port_stats, &args);
        }
    }
    else
    {
        tbl_port_stats_dump_one(p_port_stats, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_port_stats_cmd_process 
 * Purpose      : process port_stats command for FEA         
 * Input        : argv: CLI parameter; argc: parameter counter; para: related parameter;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_port_stats_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_port_stats_t port_stats;
    tbl_port_stats_t *p_port_stats = NULL;
    int32 rc = HSRV_E_NONE;
    uint32 ifindex = 0;
    ctclib_slistnode_t *listnode = NULL;  
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    tbl_port_stats_t *p_db_port_stats = NULL;
    tbl_interface_t intf;
    tbl_interface_t *p_db_if = NULL;

    CFG_DBG_FUNC();

    sal_memset(&port_stats, 0, sizeof(port_stats));
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (!argc)
        {
            CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_db_port_stats, listnode)
            {
                hsrv_port_stats_clear_stats(p_db_port_stats->key.ifindex);
            }
        }
        else
        {
            sal_memset(&intf, 0, sizeof(intf));
            if (sal_isdigit(argv[0][0]))
            {
                PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
                rc = hsrv_port_stats_clear_stats(ifindex);
            }
            else
            {
                sal_strncpy(intf.key.name, argv[0], IFNAME_SIZE);
                p_db_if = tbl_interface_get_interface(&intf.key);
                if (p_db_if)
                {
                    rc = hsrv_port_stats_clear_stats(p_db_if->ifindex);
                }
                else
                {
                    CFG_FUNC_ERR_RET("Can't find interface name %s\n", argv[0]);
                }
            }
            if (HSRV_E_NONE != rc)
            {
                 CFG_FUNC_ERR_RET("Clear Error\n");
            }
        }
        
        break;

    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            if (sal_isdigit(argv[0][0]))
            {
                PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
            }
            else
            {
                ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
            }
            port_stats.key.ifindex = ifindex;
            p_port_stats = tbl_port_stats_get_port_stats(&port_stats.key);
            if (NULL == p_port_stats)
            {
                CFG_FUNC_ERR_RET("Port stats ifindex %d not exist\n", ifindex);
            }            
        }
        _hsrv_port_stats_dump_all(p_port_stats, para);
        break;

    case CDB_OPER_SHOW:
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
hsrv_port_stats_cmd_stats_update(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    ctclib_slistnode_t *listnode = NULL;  
    tbl_port_stats_master_t *p_master = tbl_port_stats_get_master();
    tbl_port_stats_t *p_db_port_stats = NULL;
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (argc)
        {
            ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
            if (0 == sal_memcmp(argv[0], GLB_IFNAME_ETH_PREFIX, GLB_IFNAME_ETH_PREFIX_LEN)
                ||0 == sal_memcmp(argv[0], GLB_IFNAME_AGG_PREFIX, GLB_IFNAME_AGG_PREFIX_LEN))
            {
                hsrv_port_stats_get_all_stats(ifindex);
            }
        }
        else
        {
            CTCLIB_SLIST_LOOP(p_master->port_stats_list, p_db_port_stats, listnode)
            {
                hsrv_port_stats_get_all_stats(p_db_port_stats->key.ifindex);
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static uint64
hsrv_port_get_output_discard(tbl_port_stats_rate_t *p_db_stats_rate)
{
    uint64  packets_discard = 0;
    uint32  queue_index = 0;
    char    ifname[IFNAME_SIZE];
    tbl_aclqos_if_stats_t *p_aclqos_if_stats = NULL;
    tbl_aclqos_if_stats_t aclqos_if_stats;

    tbl_interface_get_name_by_ifindex(p_db_stats_rate->key.ifindex, ifname, IFNAME_SIZE);
    
    sal_memset(&aclqos_if_stats, 0, sizeof(tbl_aclqos_if_stats_t));
    p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(ifname);
    if (!p_aclqos_if_stats)
    {
        sal_strcpy(aclqos_if_stats.key.name, ifname);
        tbl_aclqos_if_stats_add_aclqos_if_stats(&aclqos_if_stats);
        p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(aclqos_if_stats.key.name);
    }

    hsrv_qos_get_queue_stats(p_aclqos_if_stats);

    for (queue_index = 0; queue_index < GLB_QOS_MAX_PORT_QUEUE_NUM; queue_index++)
    {
        packets_discard += p_aclqos_if_stats->queue_drop_pkt[queue_index];
    }
    
    return packets_discard;
}

void
hsrv_port_output_discard_timer(void *p_data)
{
    uint64  packets_discard = 0;
    char    ifname[IFNAME_SIZE];
    char    ifname_ext[IFNAME_EXT_SIZE];

    tbl_port_stats_rate_t *p_db_stats_rate = (tbl_port_stats_rate_t *)p_data;
    if (NULL == p_db_stats_rate)
    {
        return;
    }
    p_db_stats_rate->t_output_discard_timer = NULL;
    p_db_stats_rate->output_discard_running_interval++;

    tbl_interface_get_name_by_ifindex(p_db_stats_rate->key.ifindex, ifname, IFNAME_SIZE);

    packets_discard = hsrv_port_get_output_discard(p_db_stats_rate);

    if (packets_discard > p_db_stats_rate->output_discard_total_packets)
    {
        p_db_stats_rate->output_discard_packets += (packets_discard - p_db_stats_rate->output_discard_total_packets);
    }
    p_db_stats_rate->output_discard_total_packets = packets_discard;

    if (p_db_stats_rate->output_discard_running_interval >= p_db_stats_rate->output_discard_interval)
    {
        if (p_db_stats_rate->output_discard_packets >= p_db_stats_rate->output_discard_threshold)
        {
            if (p_db_stats_rate->output_discard_state == LOG_THRESHOLD_DISCARD_RESUME)
            {
                logid_sys(LOG_INTERFACE_4_DROP_OVERFLOW, IFNAME_ETH2FULL(ifname, ifname_ext), "output",
                  p_db_stats_rate->output_discard_packets, p_db_stats_rate->output_discard_interval);
                p_db_stats_rate->output_discard_state = LOG_THRESHOLD_DISCARD_OVER;
            }
        }
        else
        {
            if (p_db_stats_rate->output_discard_state == LOG_THRESHOLD_DISCARD_OVER)
            {
                logid_sys(LOG_INTERFACE_4_DROP_RESUME, IFNAME_ETH2FULL(ifname, ifname_ext), "output",
                  p_db_stats_rate->output_discard_threshold, p_db_stats_rate->output_discard_interval);
                p_db_stats_rate->output_discard_state = LOG_THRESHOLD_DISCARD_RESUME;
            }
        }
        p_db_stats_rate->output_discard_running_interval = 0;
        p_db_stats_rate->output_discard_packets = 0;
    }
    else
    {
        if (p_db_stats_rate->output_discard_packets >= p_db_stats_rate->output_discard_threshold)
        {
            if (p_db_stats_rate->output_discard_state == LOG_THRESHOLD_DISCARD_RESUME)
            {
                logid_sys(LOG_INTERFACE_4_DROP_OVERFLOW, IFNAME_ETH2FULL(ifname, ifname_ext), "output",
                  p_db_stats_rate->output_discard_packets, p_db_stats_rate->output_discard_running_interval);
                p_db_stats_rate->output_discard_state = LOG_THRESHOLD_DISCARD_OVER;
                p_db_stats_rate->output_discard_running_interval = 0;
                p_db_stats_rate->output_discard_packets = 0;
            }
        }
    }

    p_db_stats_rate->t_output_discard_timer = CTC_TASK_ADD_TIME(hsrv_port_output_discard_timer, 
                     p_db_stats_rate, GLB_PORT_DISCARD_STATS_INTERVAL);
    return;
}

int32
hsrv_port_stats_rate_set_discard_threshold(tbl_port_stats_rate_key_t *p_port_stats_rate_key)
{
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;
    p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(p_port_stats_rate_key);

    if (NULL == p_db_stats_rate)
    {
        return HSRV_E_NONE;
    }

    p_db_stats_rate->output_discard_packets = 0;
    p_db_stats_rate->output_discard_total_packets = 0;
    p_db_stats_rate->output_discard_state = LOG_THRESHOLD_DISCARD_RESUME;
    p_db_stats_rate->output_discard_running_interval = 0;

    CTC_TASK_STOP_TIMER(p_db_stats_rate->t_output_discard_timer);

    if (0 != p_db_stats_rate->output_discard_threshold)
    {
        p_db_stats_rate->output_discard_total_packets = hsrv_port_get_output_discard(p_db_stats_rate);
        p_db_stats_rate->t_output_discard_timer = CTC_TASK_ADD_TIME(hsrv_port_output_discard_timer,
                         p_db_stats_rate, GLB_PORT_DISCARD_STATS_INTERVAL);
    }

    return HSRV_E_NONE;
}

int32
hsrv_port_stats_rate_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id,
        void *p_tbl, void *p_ds)
{
    tbl_port_stats_rate_t *p_db_stats_rate = NULL;
    tbl_port_stats_rate_key_t port_stats_rate_key;

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        p_db_stats_rate = (tbl_port_stats_rate_t *)p_tbl;
        HSRV_PTR_CHECK(p_db_stats_rate);

        sal_memset(&port_stats_rate_key, 0, sizeof(port_stats_rate_key));
        port_stats_rate_key.ifindex = p_db_stats_rate->key.ifindex;
        p_db_stats_rate = tbl_port_stats_rate_get_port_stats_rate(&port_stats_rate_key);
        HSRV_PTR_CHECK(p_db_stats_rate);

        CTC_TASK_STOP_TIMER(p_db_stats_rate->t_output_discard_timer);
        break;

    case CDB_OPER_SET:
        p_db_stats_rate = (tbl_port_stats_rate_t *)p_tbl;
        HSRV_PTR_CHECK(p_db_stats_rate);

        sal_memset(&port_stats_rate_key, 0, sizeof(port_stats_rate_key));
        port_stats_rate_key.ifindex = p_db_stats_rate->key.ifindex;
        if (field_id == TBL_PORT_STATS_RATE_FLD_OUTPUT_DISCARD_INTERVAL)
        {
            HSRV_IF_ERROR_RETURN(hsrv_port_stats_rate_set_discard_threshold(&port_stats_rate_key));
        }

        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}


int32 
hsrv_port_stats_init()
{
    tbl_fea_global_t *p_fea_glb = tbl_fea_global_get_fea_global();

    p_fea_glb->port_stats_ms = GLB_PORT_POOL_STATS_INTERVAL;
    CTC_TASK_STOP_TIMER(g_p_stats_scan_time);
    g_p_stats_scan_time = ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_port_stats_scan_timeout, NULL, GLB_PORT_STATS_INTERVAL);

    /* added by hansf for ovs dump-queue-stats */
    ctc_task_add_timer(TASK_PRI_NORMAL, hsrv_port_queue_stats_scan_timeout, NULL, GLB_PORT_STATS_INTERVAL);
   
    cdb_pm_subscribe_tbl(TBL_PORT_STATS_RATE, hsrv_port_stats_rate_sync, NULL);

    cdb_register_cfg_tbl_cb(TBL_PORT_STATS, hsrv_port_stats_cmd_process);
    
    cdb_register_cfg_act_cb(ACT_UPDATE_IF_STATS, hsrv_port_stats_cmd_stats_update);

    return HSRV_E_NONE;
    
}


