/****************************************************************************
* $Id$
*  Centec Interface private information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-07-19
* Reason        : First Create.
****************************************************************************/

#ifndef __HAGT_IF_PRIVATE_H__
#define __HAGT_IF_PRIVATE_H__

#include "glb_macro.h"
#include "ctclib_vector.h"
#include "glb_if_define.h"
#include "glb_stats_define.h"
/* modify for efm , 2011-03-17 */
#define  HAGT_IF_STATS_INTERVAL 1000 /* 1s for efm */
#define  HAGT_IF_STATS_COUNT    3    /* 3s for mac stats*/
/* modified by cuixl for bug 17874, 2012-02-21 */
#define  HAGT_IF_STATS_POLLING_INTVAL   1    /* 1s for mac stats*/
#define  HAGT_IF_STATS_STORE_CNT   6    /* 6 for mac stats*/

/*******************************************************************
*structures
********************************************************************/
/*ASIC filed info*/
struct hagt_if_tbl_filed_info_s
{
    ctc_vlantag_ctl_t tag_ctrl;
    ctc_dot1q_type_t  dot1q_type;
    bool              transmit_en;
    bool              receive_en;
    bool              keep_vlan_tag;
    bool              ingress_vlan_filter;
    bool              egress_vlan_filter;
    bool              src_discard;
};
typedef struct hagt_if_tbl_filed_info_s hagt_if_tbl_filed_info_t;
/*port struct*/
struct hagt_if_port_info_s 
{
    uint16 gportid; /*only for dump database*/
    uint16 trunkid; /* agg port which this port attatched to */
    /*Added by xgu for hardware learning, dump hw fdb database need ifname, 2013-9-16*/
    uint16 slotno;
    uint16 portno;
    uint16 subportno;
    hagt_if_flag_t flag; /*hagt_if_portagg_flag_t*/
    hagt_if_tbl_filed_info_t status;
    bool status_configued;/*If this interface have been configured; Added by xgu, 2013-1-8*/
    uint16 notify_intval[GLB_IF_STATS_REQ_MAX]; /* modified by cuixl for bug 17874, 2012-02-20 , for notify intval for RMON, EFM, SFLOW*/
    uint16 index;
    u_int64_t octets_rcv[HAGT_IF_STATS_STORE_CNT];
    u_int64_t pkts_rcv[HAGT_IF_STATS_STORE_CNT];
    u_int64_t octets_send[HAGT_IF_STATS_STORE_CNT];
    u_int64_t pkts_send[HAGT_IF_STATS_STORE_CNT];
    sal_time_t start_time;  /* port stats start store time */
    sal_time_t store_time[HAGT_IF_STATS_STORE_CNT];  /* port stats last store time */
};
typedef struct hagt_if_port_info_s hagt_if_port_info_t;

/*port stats info*/
/* Modifed by weizj for bug 36460 */
struct hagt_if_port_stats_info_s
{
    glb_if_stats_t* p_port_stats[CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP];/*CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP*/
    ctc_stats_mac_rec_t* p_mac_rx_stats[CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP];/*CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP*/
    ctc_stats_mac_snd_t* p_mac_tx_stats[CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP];/*CTC_MAX_LOCAL_CHIP_NUM * GLB_PORT_NUM_PER_CHIP*/
};
typedef struct hagt_if_port_stats_info_s hagt_if_port_stats_info_t;

/*hagt if master*/
struct hagt_if_master_s 
{
    ctclib_vector_t* pst_port_info[CTC_MAX_LOCAL_CHIP_NUM]; /*GLB_PORT_NUM_PER_CHIP, hagt_if_port_info_t*/   
    ctclib_vector_t* pst_agg_port_info; /*trunkid, hagt_if_port_info_t*/   
    hagt_if_port_stats_info_t port_stats_info;
    hagt_if_port_stats_info_t agg_port_stats_info;
    sal_mutex_t *p_port_stats_mutex;
    sal_task_t*  p_poll_port_stats_thread;  /* modified by cuixl for bug 17874, 2012-02-20 */
    uint32 p_stats_poll_cnt; /* read mac stats count */
};
typedef struct hagt_if_master_s hagt_if_master_t;

#ifdef _GLB_ENABLE_DBGSHOW_
struct hagt_if_dump_db_s
{
    int detaied;
};
typedef struct hagt_if_dump_db_s hagt_if_dump_db_t;
#endif /*_GLB_ENABLE_DBGSHOW_*/

/*******************************************************************
*debug
********************************************************************/
#define HAGT_IF_LOG_EMERG(fmt, args...)   HAGT_LOG_EMERG(fmt, ##args)
#define HAGT_IF_LOG_ALERT(fmt, args...)   HAGT_LOG_ALERT(fmt, ##args)
#define HAGT_IF_LOG_CRIT(fmt, args...)    HAGT_LOG_CRIT(fmt, ##args)
#define HAGT_IF_LOG_ERR(fmt, args...)     HAGT_LOG_ERR(fmt, ##args)
#define HAGT_IF_LOG_WARN(fmt, args...)    HAGT_LOG_WARN(fmt, ##args)
#define HAGT_IF_LOG_NOTICE(fmt, args...)  HAGT_LOG_NOTICE(fmt, ##args)
#define HAGT_IF_LOG_INFO(fmt, args...)    HAGT_LOG_INFO(fmt, ##args)

#define HAGT_IF_PTR_VALID_CHECK(ptr) \
do { \
    if (NULL == ptr) \
    {\
        HAGT_LOG_ERR("Invaid pointer, %s:%d", __FUNCTION__, __LINE__);\
        return(HAGT_E_INVALID_PTR); \
    }\
}while(0)
/*******************************************************************
*functions
********************************************************************/
/******************************************************************************
 * Name         : hagt_ifdb_init
 * Purpose      : init interface database in hagt
 * Input        : N/A                
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_init (void);
/******************************************************************************
 * Name         : hagt_l3if_register_hagt_message_callback
 * Purpose      : register layer3 interface message callback function
 * Input        : N/A                
 * Output       : N/A
 * Return       : N/A
 * Note         : N/A
******************************************************************************/
extern int32
hagt_l3if_register_hagt_message_callback (void);
/******************************************************************************
 * Name         : hagt_ifdb_create_interface
 * Purpose      : create port to database
 * Input        : gportid: global port, must be in the local linecard and must not be aggregated port               
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_create_interface (uint16 gportid, uint16 slotno, uint16 portno, uint16 subportno);
/******************************************************************************
 * Name         : hagt_ifdb_destroy_interface
 * Purpose      : destory port from database
 * Input        : gportid: global port, must be in the local linecard and must not be aggregated port              
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : N/A
******************************************************************************/
extern int32
hagt_ifdb_destroy_interface (uint16 gportid);

/******************************************************************************
 * Name         : hagt_ifdb_change_port_mode
 * Purpose      : change port to routed or switch, should change port flag
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port               
                route_en:  1=enalbe 0=disable
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : this function only used for interface module
******************************************************************************/
extern int32
hagt_ifdb_change_port_mode (uint16 gportid, bool route_en);
/*******************************************************************
*variables
********************************************************************/
extern hagt_if_master_t* g_pst_hagt_if_master;
extern int32 hagt_ifdb_is_stacking_port (uint16 gportid);
#endif /*__HAGT_IF_PRIVATE_H__*/

