/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
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
#include "hsrv_brgif.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "glb_tempfile_define.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "hsrv_mlag.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"
#include "gen/tbl_brg_global_define.h"
#include "gen/tbl_brg_global.h"
#include "genlog.h"
#include "lib_syslimit.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
hsrv_fdb_t g_hsrv_fdb;
hsrv_fdb_loop_t g_hsrv_fdb_loop;
extern sal_mutex_t *g_hsrv_brgif_mutex;

ctc_task_t *g_p_fdb_loop_timer = NULL;

extern int32
hsrv_fdb_set_learning_en(uint32 enable);
/******************************************************************************
* Defines and Macros
******************************************************************************/

#define __HSRV_FDB_SAI_FUNC__

extern int32 sys_goldengate_l2_delete_hw_by_mac_fid(uint8 lchip, mac_addr_t mac, uint16 fid, uint8 mac_limit_en);

/***************************************************************************************************
 * Name         : hsrv_fdb_sai_add_fdb
 * Purpose      : creat static fdb entry to hardware
 * Input        : vid, VLAN ID; mac MAC Address; portid, port ID;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_sai_add_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t* p_if) 
{
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    sai_attribute_t attr[] = {
        [0] = {
           .id = SAI_FDB_ENTRY_ATTR_TYPE,
           .value.s32 = SAI_FDB_ENTRY_STATIC,
        },
        [1] = {
           .id = SAI_FDB_ENTRY_ATTR_PORT_ID,
        },
        [2] = {
           .id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION,
           .value.s32 = SAI_PACKET_ACTION_FORWARD,
        },
    };

    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        attr[0].value.s32 = SAI_FDB_ENTRY_STATIC_SECURITY;
    }
    else if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        attr[0].value.s32 = SAI_FDB_ENTRY_STATIC;
    }
    else
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC_WO_AGING;
        }
        else if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC_SECURITY;
        }
        else
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC;
        }
    }
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    
    attr[1].value.oid = p_if->portid;
    sal_memcpy(l2_addr.mac_address, p_fdb->key.mac, sizeof(mac_addr_t));
    l2_addr.vlan_id = p_fdb->key.fid;
    rc = fdb_api->create_fdb_entry(&l2_addr, sizeof(attr)/sizeof(attr[0]), attr);
    if (rc < 0)
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
        {
            p_fdb->fea_fail = TRUE;
        }
        else
        {
            /* Added by kcao 2017-08-23 for bug 45038, dynamic FDB hash conflict, should not add to CDB */
            return rc;
        }
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_fdb_sai_add_fdb_with_nexthop
 * Purpose      : add  fdb entry to hardware with nexthop
 * Input        : vid, VLAN ID; mac MAC Address; nexthop_id;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_sai_add_fdb_with_nexthop(tbl_fea_fdb_t *p_fdb, uint32_t nh_id, sai_object_id_t port_id) 
{
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    
    sal_memcpy(l2_addr.mac_address, p_fdb->key.mac, sizeof(mac_addr_t));
    l2_addr.vlan_id = p_fdb->key.fid;
    rc = fdb_api->create_fdb_entry_with_nexthop(&l2_addr, nh_id, port_id);
    if (rc < 0)
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
        {
            p_fdb->fea_fail = TRUE;
        }
        else
        {
            /* Added by kcao 2017-08-23 for bug 45038, dynamic FDB hash conflict, should not add to CDB */
            return rc;
        }
    }

    return HSRV_E_NONE;
}
/***************************************************************************************************
 * Name         : hsrv_fdb_sai_del_fdb
 * Purpose      : remove fdb entry to hardware
 * Input        : vid, VLAN ID; mac MAC Address;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_fdb_sai_del_fdb(tbl_fea_fdb_t *p_db_fdb)
{
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));

    if (p_db_fdb->fea_fail)
    {
        return HSRV_E_NONE;
    }

    sal_memcpy(l2_addr.mac_address, p_db_fdb->key.mac, sizeof(mac_addr_t));
    l2_addr.vlan_id = p_db_fdb->key.fid;
    HSRV_IF_ERROR_RETURN(fdb_api->remove_fdb_entry(&l2_addr));

    return HSRV_E_NONE;
}

int32
hsrv_fdb_sai_update_fdb_by_flush(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t* p_if) 
{
    sai_status_t rc = 0;
    sai_fdb_api_t*  fdb_api = NULL;
    sai_fdb_entry_t l2_addr;
    sai_attribute_t attr[] = {
        [0] = {
           .id = SAI_FDB_ENTRY_ATTR_TYPE,
           .value.s32 = SAI_FDB_ENTRY_STATIC,
        },
        [1] = {
           .id = SAI_FDB_ENTRY_ATTR_PORT_ID,
        },
        [2] = {
           .id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION,
           .value.s32 = SAI_PACKET_ACTION_FORWARD,
        },
    };

    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        attr[0].value.s32 = SAI_FDB_ENTRY_STATIC;
    }
    else
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC_WO_AGING;
        }
        else if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC_SECURITY;
        }
        else
        {
            attr[0].value.s32 = SAI_FDB_ENTRY_DYNAMIC;
        }
    }
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_FDB,(void**)&fdb_api));
    
    attr[1].value.oid = p_if->portid;
    sal_memcpy(l2_addr.mac_address, p_fdb->key.mac, sizeof(mac_addr_t));
    l2_addr.vlan_id = p_fdb->key.fid;
    rc = fdb_api->update_fdb_entry(&l2_addr, sizeof(attr)/sizeof(attr[0]), attr);
    if (rc < 0)
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
        {
            p_fdb->fea_fail = TRUE;
        }
    }

    return HSRV_E_NONE;
}


#define __HSRV_FDB_CONVERT_FORMAT__

static int32
_hsrv_fdb_convert_sai_to_fea(sai_fdb_event_notification_data_t *p_sai, tbl_fea_fdb_t* p_fdb, uint32 is_ageing)
{
    tbl_fea_port_if_key_t key;
    tbl_fea_port_if_t *p_db_portif = NULL;
    
    sal_memset(p_fdb, 0, sizeof(tbl_fea_fdb_t));
    p_fdb->key.fid = p_sai->fdb_entry.vlan_id;
    sal_memcpy(p_fdb->key.mac, p_sai->fdb_entry.mac_address, MAC_ADDR_LEN);

    if (!is_ageing)
    {
        if (SAI_FDB_ENTRY_ATTR_PORT_ID != p_sai->attr[0].id)
        {
            return HSRV_E_INVALID_PARAM;
        }

        /* get portid */
        p_fdb->portid = p_sai->attr[0].value.oid;

        /* get ifindex */    
        p_fdb->ifindex = GLB_INVALID_IFINDEX;
        key.portid = p_fdb->portid;
        p_db_portif = tbl_fea_port_if_get_fea_port_if(&key);
        if (NULL == p_db_portif)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        p_fdb->ifindex = p_db_portif->ifindex;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_convert_pm_to_fea(tbl_fdb_t *p_pm, tbl_fea_fdb_t* p_fdb, uint32 is_add)
{
    tbl_fea_brg_if_t *p_db_if = NULL;

    sal_memset(p_fdb, 0, sizeof(tbl_fea_fdb_t));
    p_fdb->key.fid = p_pm->key.fid;
    sal_memcpy(p_fdb->key.mac, p_pm->key.mac, MAC_ADDR_LEN);
    p_fdb->flags = p_pm->flags;

    /* delete need not ifindex */
    if (is_add)
    {
        p_fdb->ifindex = p_pm->ifindex;
        p_db_if = hsrv_brgif_get(p_fdb->ifindex);
        HSRV_PTR_CHECK(p_db_if);
        p_fdb->portid = p_db_if->portid;    /* both port and agg */
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_convert_mlag_to_fea(glb_mlag_fdb_entry_t *p_mlag, tbl_fea_fdb_t* p_fdb)
{
    tbl_mlag_port_key_t key;
    tbl_mlag_port_t *p_db_mlag = NULL;
    tbl_fea_brg_if_t *p_db_if = NULL;

    key.id = p_mlag->mlag_id;
    p_db_mlag = tbl_mlag_port_get_mlag_port(&key);
    if (NULL == p_db_mlag)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    sal_memset(p_fdb, 0, sizeof(tbl_fea_fdb_t));
    p_fdb->key.fid = p_mlag->fid;
    sal_memcpy(p_fdb->key.mac, p_mlag->mac, MAC_ADDR_LEN);
    p_fdb->flags = p_mlag->flag;
    
    p_fdb->ifindex = p_db_mlag->ifindex;
    p_db_if = hsrv_brgif_get(p_fdb->ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_INVALID_PTR;
    }
    p_fdb->portid = p_db_if->portid;    /* both port and agg */

    return HSRV_E_NONE;
}
#define __HSRV_FDB_INNER_API__


/* send FDB loop errdisable message to PM */
static int
hsrv_fdb_set_err_disable(uint32 ifindex)
{
    tbl_interface_t *p_db_if = NULL;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_INTERFACE_NOT_EXIT;
    }
    HSRV2SWITCH_MSG_SEND_NOTIFY(SWITCH_FEI_FDB_LOOP_SYNC, p_db_if, sizeof(tbl_interface_t));
 
    return HSRV_E_NONE;
}

/* add FDB loop cache */
static inline int32
hsrv_fdb_add_fdb_loop_cache(uint32 ifindex)
{
    if (0 == g_hsrv_fdb_loop.cache.count)
    {
        g_hsrv_fdb_loop.cache.index = 0;
    }
    g_hsrv_fdb_loop.cache.port_array[g_hsrv_fdb_loop.cache.index] = ifindex;
    g_hsrv_fdb_loop.cache.index++;
    g_hsrv_fdb_loop.cache.count++;

    if(g_hsrv_fdb_loop.cache.index >= HSRV_FDB_LOOP_CACHE_SIZE)
    {
        g_hsrv_fdb_loop.cache.index = 0;
    }
    
    return HSRV_E_NONE;
}

/* check whether has repeating FDB update on this port */
static inline int32
hsrv_fdb_check_repeating_fdb_update(uint32 ifindex)
{
    uint32 start = 0;
    uint32 count = 0;
    uint32 index = 0;
    uint32 i     = 0;
    uint32 incache_count = 0;
    
    if (0 == g_hsrv_fdb_loop.cache.count)
    {
        return FALSE;
    }
    if (g_hsrv_fdb_loop.cache.count >= HSRV_FDB_LOOP_CACHE_SIZE)
    {
        start = g_hsrv_fdb_loop.cache.index;
        count = HSRV_FDB_LOOP_CACHE_SIZE;
    }
    else 
    {
        start = 0;
        count = g_hsrv_fdb_loop.cache.index;
    }

    for(i = 0, index = start; i < count; i++, index++)
    {
        if (index >= HSRV_FDB_LOOP_CACHE_SIZE)
        {
            index -= HSRV_FDB_LOOP_CACHE_SIZE;
        }

        if (g_hsrv_fdb_loop.cache.port_array[index] == ifindex)
        {
            incache_count++;
        }
    }   
    if (incache_count > g_hsrv_fdb_loop.cache_err_threshold)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* clear FDB loop cache */
static inline int32
hsrv_fdb_clear_fdb_loop_cache()
{
    sal_memset(&g_hsrv_fdb_loop.cache, 0, sizeof(hsrv_fdb_loop_cache_t));
    return HSRV_E_NONE;
}

static inline int32
hsrv_fdb_update_fdb_loop_check(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_t *p_db_fdb)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_brg_global_t *p_brg_global = NULL;
    tbl_interface_t  * p_db_if = NULL;
    p_brg_global = tbl_brg_global_get_brg_global();

    int32 ret = HSRV_E_NONE;
    uint32 errdis_en = FALSE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char mac_addr_str[32];

    /* fdb loop is not applicable for vsi */
    if (p_fdb->key.fid >= GLB_MAX_VLAN_ID)
    {
        return HSRV_E_NONE;
    }

    /* MLAG failover switch not consume token */
    if (p_db_fdb->ifindex == p_db_mlag->peer_link_ifindex
     || p_fdb->ifindex == p_db_mlag->peer_link_ifindex)
    {
        return HSRV_E_NONE;
    }

    errdis_en = p_brg_global->fdb_loop_errdis_en;
    hsrv_fdb_add_fdb_loop_cache(p_fdb->ifindex);
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_fdb->ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_INTERFACE_NOT_EXIT;
    }
    
    if (p_brg_global->fdb_loop_curr_size > 0)
    {
        p_brg_global->fdb_loop_curr_size--;
    }
    else
    {
        if (hsrv_fdb_check_repeating_fdb_update(p_fdb->ifindex))
        {
            p_brg_global->fdb_loop_disc_cnt++;
            if (errdis_en)
            {
                hsrv_fdb_set_err_disable(p_fdb->ifindex);
            }
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            sal_snprintf(mac_addr_str, 32, "%02X:%02X:%02X:%02X:%02X:%02X", p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5]);
            logid_sys(LOG_FDB_4_FDBLOOP, ifname_ext, mac_addr_str, p_fdb->key.fid, errdis_en ? "errdisable":"warning");
            ret = HSRV_E_NO_MEMORY;

            /* reset bucket and cache to init state to avoid continuous block */
            p_brg_global->fdb_loop_curr_size = p_brg_global->fdb_loop_max_size;
            hsrv_fdb_clear_fdb_loop_cache();
        }
    }
    return ret;
}

#ifdef GOLDENGATE
enum drv_fib_acc_mac_flag_e
{
    DRV_FIB_ACC_PORT_MAC_LIMIT_EN   =  (0x1),
    DRV_FIB_ACC_VLAN_MAC_LIMIT_EN   =  (0x1 << 1),
    DRV_FIB_ACC_DYNAMIC_MAC_LIMIT_EN=  (0x1 << 2),
    DRV_FIB_ACC_PROFILE_MAC_LIMIT_EN=  (0x1 << 3),
    DRV_FIB_ACC_CPU_KNOWN_ORG_PORT  =  (0x1 << 4),
    DRV_FIB_ACC_STATIC_COUNT_EN     =  (0x1 << 5),
    DRV_FIB_ACC_MAC_FLAG_NUM
};
typedef enum drv_fib_acc_mac_flag_e drv_fib_acc_mac_flag_t;
#endif
extern int32 sys_usw_l2_delete_hw_by_mac_fid(uint8 lchip, mac_addr_t mac, uint16 fid, uint8 mac_limit_en, uint8 is_logic, uint8 is_static);

static inline int32
_hsrv_fdb_del_pending_fdb(tbl_fea_fdb_t *p_fdb)
{
    int32 ret = HSRV_E_NONE;
    mac_addr_t mac; 
    uint16 fid = 0;
#ifdef GOLDENGATE
    uint8 mac_limit_en = 0; 
#endif 
  
    fid = p_fdb->key.fid;
    sal_memcpy(mac, p_fdb->key.mac, sizeof(mac_addr_t));
/* modified by yejl for distinguish gg and d2, 2017-08-25 */
#ifdef GOLDENGATE
    mac_limit_en = DRV_FIB_ACC_PROFILE_MAC_LIMIT_EN | DRV_FIB_ACC_VLAN_MAC_LIMIT_EN | DRV_FIB_ACC_DYNAMIC_MAC_LIMIT_EN;
    mac_limit_en |= DRV_FIB_ACC_PORT_MAC_LIMIT_EN;
    ret = sys_goldengate_l2_delete_hw_by_mac_fid(0, mac, fid, mac_limit_en);
#elif defined USW
    //ret = hsrv_fdb_flush_fdb_pending(fid, mac);
    ret = sys_usw_l2_delete_hw_by_mac_fid(0, mac, fid, TRUE, FALSE, FALSE);/*modified by yejl for bug 52275, 2019-10-12*/
#else
    /* GB has not pending entry, need no action */
    (void)fid;
#endif 
    
    return ret;
}

static int32
hsrv_fdb_check_stp_disable_learn(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    uint32 instance_index = 0;

    /* Modified by kcao for bug 42300 2016-12-27 */
    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        return HSRV_E_NONE;
    }
    
    vlan_key.vid = p_fdb->key.fid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return HSRV_E_INVALID_PTR;
    }
    inst_key.instance = p_db_vlan->instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return HSRV_E_INVALID_PTR;
    }
    instance_index = CTC_SAI_OBJECT_INDEX_GET(p_db_inst->instance_oid);
    if (instance_index >= GLB_STP_INSTANCE_MAX)
    {
        return HSRV_E_INVALID_PARAM;
    }
    if (p_db_if->disable_learn[instance_index])
    {
        return HSRV_E_FAIL;
    }
    return HSRV_E_NONE;
}

static int32
hsrv_fdb_check_mirror_dest_port(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;

    /* Modified by kcao for bug 42300 2016-12-27 */
    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        return HSRV_E_NONE;
    }
    
    vlan_key.vid = p_fdb->key.fid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return HSRV_E_INVALID_PTR;
    }

    if (p_db_if->is_mirror_dest)
    {
        return HSRV_E_FAIL;
    }
    return HSRV_E_NONE;
}

static int32
hsrv_fdb_check_g8032_control_vlan(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    
    vlan_key.vid = p_fdb->key.fid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return HSRV_E_INVALID_PTR;
    }

    if (p_db_vlan->g8032_ring_id)
    {
        return HSRV_E_FAIL;
    }
    return HSRV_E_NONE;
}


static int32
_hsrv_fdb_check_for_add_new_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if, uint32 is_peer_link, uint32 *p_allow_add)
{
    uint32 exist = FALSE;
    uint32 ifindex = 0;
    int32 rc = HSRV_E_NONE;

    *p_allow_add = FALSE;
    
    /* 2. check LAG member port */
    if (FEA_BRGIF_IS_LAG_MEMBER(p_db_if))
    {
        return GLB_E_AGG_MEMBER;
    }

    /* check dynamic fdb and interface is running */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (!(p_db_if->flags & GLB_IFF_RUNNING))
        {
            return GLB_E_NOT_LEARNING_ENABLE;
        }
    }
    
    /* 3. check whether port is security */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (p_db_if->port_security_en)
        {
            GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
        }
    }
    /* 4. check whether no mac learning is enabled for dynamic fdb */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (!p_db_if->mac_learning_en)
        {
            if (!is_peer_link)
            {
                HSRV_FDB_DEBUG("No mac learning is enabled for ifindex %u", ifindex);
                return GLB_E_NOT_LEARNING_ENABLE;
            }
        }
    }

    /* 5. for dynamic fdb, check whether port belongs to fid */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
    {
        /* check port belongs to the vlan */
        rc = hsrv_vlan_check_port_in_enable_vlan(p_fdb->key.fid, p_fdb->ifindex, &exist);
        if (!exist || HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Interface ifindex: %u does not belong to the fid: %u",
                p_fdb->ifindex, p_fdb->key.fid);
            return GLB_E_PORT_NOT_BELONG_TO_VID;
        }
    }

    /* 6. check security port maximum */
    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN) && p_fdb->key.fid < GLB_MAX_VLAN_ID
        && (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)))
    {
        rc = hsrv_security_check_security_mac_num(p_db_if, TRUE,
            (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)), p_allow_add);
        if (!*p_allow_add || HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Can't add new fdb because port %d is port security and fdb number equals maximum.",
                             p_fdb->ifindex);
            return GLB_E_PORT_SECURITY_RESOURCE_FULL;
        }
    }

    /* 7. check vlan security maximum, added by yanxa, for vlan-security, 2011-01-26*/
    if (p_fdb->key.fid < GLB_MAX_VLAN_ID)
    {
        rc = hsrv_vlan_security_check_security_mac_num(p_fdb->key.fid, TRUE, p_allow_add);
        if (!*p_allow_add || HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Can't add new fdb because fdb number equals maximum in vlan %u.",
                             p_fdb->key.fid);

            return GLB_E_VLAN_SECURITY_RESOURCE_FULL;
        }
    }

    /* 8. check stp state */
    /* Added by kcao for bug 48616 2018-08-22, if FDB is sync from MO, not check stp state */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MLAG))
    {
        rc = hsrv_fdb_check_stp_disable_learn(p_fdb, p_db_if);
        if (rc < 0)
        {
            return GLB_E_STP_DISABLE_LEARN;
        }
    }
    
    /* 9. check is mirror dest port or not */
    rc = hsrv_fdb_check_mirror_dest_port(p_fdb, p_db_if);
    if (rc < 0)
    {
        return GLB_E_IS_MIRROR_DEST_PORT;
    }
    
    /* 9. check is g8032 ring control vlan or not */
    rc = hsrv_fdb_check_g8032_control_vlan(p_fdb, p_db_if);
     if (rc < 0)
    {
        return GLB_E_IS_G8032_CONTROL_VLAN;
    }   
    /* after all check, add new fdb is allowed */
    *p_allow_add = TRUE;

    return rc;
}

static inline int32
_hsrv_brg_check_for_update_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_t *p_db_fdb, 
    tbl_fea_brg_if_t *p_db_if, tbl_fea_brg_if_t *p_old_db_if, uint32 *p_allow_add)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    uint32 peer_link_ifindex = 0;
    uint32 exist = FALSE;
    uint32 ifindex =0;
    int32 rc = HSRV_E_NONE;
    
    /* default is not allowed to update old fdb */
    *p_allow_add = FALSE;

    /* Deleted by kcao 2017-01-18 for bug 40978, MLAG failover not switch FDB when FDB is full */
#if 0
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        /*1 check learning enable */
        if (FALSE == g_hsrv_fdb.hsrv_learn_enable)
        {
            return GLB_E_RESOURCE_FULL;
        }
    }
#endif

    /* 2. check LAG member port */
    if (FEA_BRGIF_IS_LAG_MEMBER(p_db_if))
    {
        return GLB_E_AGG_MEMBER;
    }

    /* 3. check whether port is security */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (p_db_if->port_security_en)
        {
            GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);
        }
    }
    
    /* 4. same entry, should not update, but more reasons will cause vpls mac update  */
    if ((p_fdb->flags == p_db_fdb->flags) && (p_fdb->ifindex == p_db_fdb->ifindex)
        && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
    {
        HSRV_FDB_DEBUG("Same fdb entry flag %x port %d\n", p_db_fdb->flags, p_db_fdb->ifindex);
        *p_allow_add = TRUE;
        return HSRV_E_NONE;
    }

    if (mlag)
    {
        peer_link_ifindex = mlag->peer_link_ifindex;
    }

    /* 5. check whether no mac learning is enabled for dynamic fdb */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (!p_db_if->mac_learning_en && (p_db_if->key.ifindex != peer_link_ifindex))
        {
            HSRV_FDB_DEBUG("No mac learning is enabled for ifindex %u", ifindex);
            return GLB_E_NOT_LEARNING_ENABLE;
        }
    }

    /* 6. for dynamic fdb, check whether port belongs to fid */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
    {
        /* check port belongs to the vlan */
        rc = hsrv_vlan_check_port_in_enable_vlan(p_fdb->key.fid, p_fdb->ifindex, &exist);
        if (!exist || HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Interface ifindex: %u does not belong to the fid: %u",
                p_fdb->ifindex, p_fdb->key.fid);
            return GLB_E_PORT_NOT_BELONG_TO_VID;
        }
    }

    /* 8. check whether fdb should be updated.
        first check old flag, then check new flag;
        there are total 16 compoundings for old fdb and new fdb*/

    /* old entry is static and security */
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC) &&
        GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        /* fdb should not be updated. */
        HSRV_FDB_DEBUG("Static and security fdb entry exist\n");
        return GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY;
    }
    /* old entry is static but not security */
    else if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC)&&
             !GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        /* static fdb should not be updated by dynamic fdb */
        if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
        {
            HSRV_FDB_DEBUG("Static fdb entry exist\n");
            return HSRV_E_NONE;
        }
    }
    /* old entry is dynamic and security */
    else if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC)&&
            GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        /* only security and static fdb can update old fdb */
        /* fdb sync message optimization modified by liwh for bug 18503, 2012-03-29 
          old dynamic fdb should be updated by static fdb */
        if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
        /*if(!GLB_FLAG_ISSET(pst_entry->flag, GLB_FDB_FLAG_STATIC)||
            !GLB_FLAG_ISSET(pst_entry->flag, GLB_FDB_FLAG_MAC_SECURITY_EN)) */
        {
            HSRV_FDB_DEBUG("Dynamic and security fdb entry exist\n");
            return GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY;
        }
    }

    /* 9. delete security mac number for old fdb entry */   
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN) && !GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))   
    {
        hsrv_security_check_security_mac_num(p_old_db_if, FALSE,
            (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC)), p_allow_add);

#ifdef GREATBELT
        hsrv_security_op_security(p_db_fdb->ifindex, NULL);
#endif
    }

    /*9. check security port maximum */   
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN) && !GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        rc = hsrv_security_check_security_mac_num(p_db_if, TRUE, 
            (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)),p_allow_add);
        if (!*p_allow_add || HSRV_E_NONE != rc)
        {
            return GLB_E_PORT_SECURITY_RESOURCE_FULL;
        }
    }

    /*10. check vlan security maximum */
    if (p_fdb->key.fid < GLB_MAX_VLAN_ID)
    {
        rc = hsrv_vlan_security_check_security_mac_num(p_fdb->key.fid, TRUE, p_allow_add);
        if (!*p_allow_add || HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Can't add new fdb because fdb number equals maximum in vlan %u.",
                             p_fdb->key.fid);
            return GLB_E_VLAN_SECURITY_RESOURCE_FULL;
        }
    }

    /* 11. check stp state */
    /* Added by kcao for bug 48616 2018-08-22, if FDB is sync from MO, not check stp state */
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MLAG))
    {
        rc = hsrv_fdb_check_stp_disable_learn(p_fdb, p_db_if);
        if (rc < 0)
        {
            return GLB_E_STP_DISABLE_LEARN;
        }    
    }    

    /* 12. check is mirror dest port or not */
    rc = hsrv_fdb_check_mirror_dest_port(p_fdb, p_db_if);
    if (rc < 0)
    {
        return GLB_E_IS_MIRROR_DEST_PORT;
    }
    
    /* after all check, update old fdb is allowed. */
    *p_allow_add = TRUE;

    return rc;
}
    
static int32
_hsrv_fdb_add_fdb(tbl_fea_fdb_t *p_fdb, uint32 is_peer_link)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    uint32 allow_add = FALSE;
    int32 rc = HSRV_E_NONE;
    /* modified by yangl for g8032, 2019-06-05 */
    tbl_fea_g8032_ring_t  fea_g8032_ring;
    tbl_fea_g8032_ring_t  *p_fea_g8032_ring = NULL;
    tbl_vlan_t *p_db_vlan = NULL;
    sai_object_id_t port_id = 0;
    
     /* yangl end */  
     
    /* modified by liwh for bug 37200, 2016-08-24 */
    tbl_brg_global_t  *p_brg_glb = NULL;
    /* liwh end */
    sal_memset(&fea_g8032_ring, 0, sizeof(fea_g8032_ring));
    /*  if system memory is not enough, close learning enable, prevent too many interrupts to hsrv*/
    /*added by yejl to fix bug43233, 2017-03-14*/
    if ((!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)))
    {
        if (syslimit_memory_threshold_check(SYSLIMIT_MEMCHECK_STATE_WARNING) != 0)
        {
            hsrv_fdb_set_learning_en(FALSE);
            _hsrv_fdb_del_pending_fdb(p_fdb);
            g_hsrv_fdb.hsrv_memchk_learn_disable = TRUE;
            return rc;
        }
        else 
        {
            if (g_hsrv_fdb.hsrv_memchk_learn_disable)
            {
                hsrv_fdb_set_learning_en(TRUE);
                _hsrv_fdb_del_pending_fdb(p_fdb);
                g_hsrv_fdb.hsrv_memchk_learn_disable = FALSE;
            }
        }
    }

    p_db_if = hsrv_brgif_get(p_fdb->ifindex);
    if (NULL == p_db_if)
    {
        _hsrv_fdb_del_pending_fdb(p_fdb);/*added by yejl for bug39502*/
        return HSRV_E_INVALID_PTR;
    }

    /* is_peer_link means mlag_port is down, and synced to MLAG peer-link */
    if ((p_db_if->mlag_id && p_db_if->mlag_is_group) || is_peer_link)
    {
        GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MLAG);
    }

    /* modified by liwh for bug 37200, 2016-08-24 */
    p_brg_glb = tbl_brg_global_get_brg_global();
    if (p_brg_glb && p_brg_glb->fdb_all_flushing)
    {
        _hsrv_fdb_del_pending_fdb(p_fdb);
        return GLB_E_NOT_LEARNING_ENABLE;    
    }
    /* liwh end */

    /* 1. check whether can add fdb */
    rc = _hsrv_fdb_check_for_add_new_fdb(p_fdb, p_db_if, is_peer_link, &allow_add);
    if (HSRV_E_NONE != rc || !allow_add)
    {
        HSRV_FDB_DEBUG("New fdb is not allowd to add fail, mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u\n",
                  p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2],
                  p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5], p_fdb->key.fid);
        if (GLB_E_AGG_MEMBER == rc || GLB_E_PORT_SECURITY_RESOURCE_FULL == rc ||
            GLB_E_PORT_NOT_BELONG_TO_VID == rc || GLB_E_NOT_LEARNING_ENABLE == rc || GLB_E_STP_DISABLE_LEARN == rc ||
            GLB_E_IS_MIRROR_DEST_PORT == rc || GLB_E_IS_G8032_CONTROL_VLAN == rc)
        {
            _hsrv_fdb_del_pending_fdb(p_fdb);
        }
        
        return rc;
    }

    /* Added by kcao for bug 41562, need to remove dynamic entry before add to SAI */
    /* Added by yejl for bug 42689, 42945, delete pending entry when the resource full*/
    rc = hsrv_fdb_add_check_full(p_fdb);
    if (rc == GLB_E_RESOURCE_FULL)
    {
        _hsrv_fdb_del_pending_fdb(p_fdb);
        return rc ;
    }
    /* modified by yangl for g8032, 2019-06-05 */
   p_db_vlan = tbl_vlan_get_vlan((tbl_vlan_key_t * )(&(p_fdb->key.fid)));

   if (p_db_vlan)
   {
       fea_g8032_ring.key.instance_id= p_db_vlan->instance;
       fea_g8032_ring.key.ifindex= p_fdb->ifindex;
       p_fea_g8032_ring = tbl_fea_g8032_ring_get_fea_g8032_ring(&fea_g8032_ring.key);
        /* 2. add FDB to SAI */
        if(!p_fea_g8032_ring)
        {
            rc = hsrv_fdb_sai_add_fdb(p_fdb, p_db_if);
            if (rc < 0)
            {
                return rc;
            }
        }
        else
        {
            hsrv_interface_ifindex_to_objectid(p_fdb->ifindex, &port_id);
            hsrv_fdb_sai_add_fdb_with_nexthop(p_fdb, p_fea_g8032_ring->nh_id, port_id);
        }
   }

    /* 3. add FDB to DB */
    rc = hsrv_fdb_db_add_fdb(p_fdb, p_db_if);
    if (rc < 0)
    {
        return rc;
    }

    /* 4. for MLAG */
    p_db_fdb = tbl_fea_fdb_get_fea_fdb(&p_fdb->key);
    if (p_db_fdb)
    {
        hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_ADD);
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_del_fdb_one_by_flush(tbl_fea_fdb_t *p_fdb)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    int32 rc = HSRV_E_NONE;

    p_db_if = hsrv_brgif_get(p_fdb->ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_INVALID_PTR;
    }
    
    rc = hsrv_fdb_sai_update_fdb_by_flush(p_fdb, p_db_if);
    if (rc < 0)
    {
        return rc;
    }

    return HSRV_E_NONE;
}


/* There function should not lock/unlock g_hsrv_fdb.fdb_mutex */
static int32
_hsrv_fdb_update_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_t *p_db_fdb)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_brg_if_t *p_old_db_if = NULL;
    tbl_fea_brg_if_t *p_db_if = NULL;
    uint32 allow_add = FALSE;
    int32 rc = HSRV_E_NONE;
    /* modified by liwh for bug 37200, 2016-08-24 */
    tbl_brg_global_t  *p_brg_glb = NULL;
    /* liwh end */

    /*  if system memory is not enough, close learning enable, prevent too many interrupts to hsrv*/
    if ((!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)))
    {
        if (syslimit_memory_threshold_check(SYSLIMIT_MEMCHECK_STATE_WARNING) != 0)
        {
            hsrv_fdb_set_learning_en(FALSE);
            _hsrv_fdb_del_pending_fdb(p_fdb);
            g_hsrv_fdb.hsrv_memchk_learn_disable = TRUE;
            return rc;
        }
        else 
        {
            if (g_hsrv_fdb.hsrv_memchk_learn_disable)
            {
                hsrv_fdb_set_learning_en(TRUE);
                _hsrv_fdb_del_pending_fdb(p_fdb);
                g_hsrv_fdb.hsrv_memchk_learn_disable = FALSE;
            }
        }
    }
    
    p_old_db_if = hsrv_brgif_get(p_db_fdb->ifindex);
    if (NULL == p_old_db_if)
    {
        _hsrv_fdb_del_pending_fdb(p_db_fdb);/*added by yejl for bug39502*/
        return HSRV_E_INVALID_PTR;
    }
    
    p_db_if = hsrv_brgif_get(p_fdb->ifindex);
    if (NULL == p_db_if)
    {
        _hsrv_fdb_del_pending_fdb(p_fdb);/*added by yejl for bug39502*/
        return HSRV_E_INVALID_PTR;
    }

    /* modified by liwh for bug 37200, 2016-08-24 */
    p_brg_glb = tbl_brg_global_get_brg_global();
    if (p_brg_glb && p_brg_glb->fdb_all_flushing)
    {
        _hsrv_fdb_del_pending_fdb(p_fdb);
        return GLB_E_NOT_LEARNING_ENABLE;    
    }
    /* liwh end */

    if (p_db_if->mlag_id && p_db_if->mlag_is_group)
    {
        GLB_SET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MLAG);
    }
    /*check whether fdb_loop errdisable*/
    hsrv_fdb_update_fdb_loop_check(p_fdb,p_db_fdb);
    
    /* 1. check whether can update fdb */
    rc = _hsrv_brg_check_for_update_fdb(p_fdb, p_db_fdb, p_db_if, p_old_db_if, &allow_add);
    if (HSRV_E_NONE != rc || !allow_add)
    {
        HSRV_FDB_DEBUG("New fdb is not allowd to update fail, mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u\n",
              p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2],
              p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5], p_fdb->key.fid);

        if (GLB_E_RESOURCE_FULL == rc || GLB_E_AGG_MEMBER == rc || GLB_E_PORT_SECURITY_RESOURCE_FULL == rc || 
            GLB_E_PORT_NOT_BELONG_TO_VID == rc || GLB_E_NOT_LEARNING_ENABLE == rc || GLB_E_STP_DISABLE_LEARN == rc ||
            GLB_E_IS_MIRROR_DEST_PORT == rc)
        {
            _hsrv_fdb_del_pending_fdb(p_fdb);
        }
        return rc;
    }

    /* 2. add FDB to SAI */
    rc = hsrv_fdb_sai_add_fdb(p_fdb, p_db_if);
    if (rc < 0)
    {
        return rc;
    }

    /* 3. for MLAG, remove old synced FDB */
    /* Modified by kcao for bug 41167, static FDB should sync del to MLAG peer */
    if ((p_fdb->ifindex != p_db_mlag->peer_link_ifindex) || (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->ifindex == p_db_mlag->peer_link_ifindex)))
    {
        if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT) || GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
        {
            if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT))
            {
                hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_DEL);
            }
            /* for case mlag_func_26 */
            if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
            {
                if (p_db_fdb->mlag_source_ifindex == p_fdb->ifindex)
                {
                    /* Added by kcao for bug 41435 2016-10-31, resume MI from peer-interface, not send to peer */
                }
                else
                {
                    GLB_UNSET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN);
                    GLB_SET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
                    hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_DEL);
                }
            }
            /* Added by kcao for bug 41198 2016-10-24, for TX del to peer, should remove SYNC_OUT flag of p_fdb */
            if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT))
            {
                GLB_UNSET_FLAG(p_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
            }
        }
    }
    
    /* 4. add FDB to DB */
    rc = hsrv_fdb_db_update_fdb(p_fdb, p_db_fdb);
    if (HSRV_E_NONE != rc)
    {
        return rc;
    }

    /* 5. for MLAG, add new sync FDB */
    if ((p_fdb->ifindex != p_db_mlag->peer_link_ifindex)
     && p_db_if->mlag_id
     && !GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT)
     && !GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
    {
        hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_ADD);
    }

    return HSRV_E_NONE;
}

int32
_hsrv_fdb_del_fdb(tbl_fea_fdb_t *p_fdb, uint32 is_static, uint32 need_sync)
{
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;

    /* 1. lookup DB */
    p_db_fdb = hsrv_fdb_lookup_fdb(p_fdb);
    if (NULL == p_db_fdb)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        if (!is_static)
        {
            /* only is_static can remove static FDB entry */
            return HSRV_E_NONE;
        }
    }

    /* 2. delete fdb from sdk */
    rc = hsrv_fdb_sai_del_fdb(p_db_fdb);
    if (rc < 0)
    {
        /* deleting process will continue */
        log_sys(M_MOD_INTERNAL, E_ERROR, "Delete fdb from SAI failure, ret is %d, mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u",
                      rc, p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2],
                      p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5], p_fdb->key.fid);
    }

    /* 3. sync to peer, when synced from MLAG peer, should set need_sync = FALSE */
    if (need_sync 
     && (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG) && GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT)))
    {
        hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_DEL);
    }
    
    /* 4. delete from DB */
    rc = hsrv_fdb_db_del_fdb(p_db_fdb);
    if (rc < 0)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Delete fdb from HSRV failure, ret is %d, mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u\n",
                      rc, p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2],
                      p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5], p_fdb->key.fid);
    }

    return HSRV_E_NONE;
}

#define __HSRV_FDB_WITH_MUTEX_LOCK_UNLOCK__

uint32
hsrv_mlag_is_established()
{
    tbl_mlag_peer_t *p_mlag_peer = tbl_mlag_peer_get_mlag_peer();

    if (p_mlag_peer)
    {
        if (5 == p_mlag_peer->mpf_state)    /* MPF_STATE_ESTABLISHED */
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
hsrv_fdb_mlag_tx_fdb_cache_send()
{
    glb_mlag_fdb_sync_t *sync = &g_hsrv_fdb.mlag_sync_cache;
    uint32 msg_len = 0;
    uint32 is_established = FALSE;
    
    is_established = hsrv_mlag_is_established();
    if (!is_established)
    {
        sync->count = 0;
        return HSRV_E_NONE;
    }

    sal_mutex_lock(g_hsrv_fdb.mlag_sync_mutex);
    if (sync->count)
    {
        HSRV_FDB_DEBUG("MLAG sync %u FDB entry to peer", sync->count);
        msg_len = FEI_MLAG_FDB_SYNC_LEN(sync->count);
        HSRV2SWITCH_MSG_SEND_NOTIFY(SWITCH_FEI_MLAG_FDB_SYNC, sync, msg_len);
        sync->count = 0;
    }
    sal_mutex_unlock(g_hsrv_fdb.mlag_sync_mutex);

    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_tx_fdb_cache_add_entry(glb_mlag_fdb_entry_t *p_entry)
{
    glb_mlag_fdb_sync_t *sync = &g_hsrv_fdb.mlag_sync_cache;
    uint32 is_established = FALSE;
    
    is_established = hsrv_mlag_is_established();
    if (!is_established)
    {
        return HSRV_E_NONE;
    }
    
    sal_mutex_lock(g_hsrv_fdb.mlag_sync_mutex);

    /* Modified by kcao for bug 41171 2016-10-18 */
    if (GLB_MLAG_SYNC_COUNT_MAX <= sync->count)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "MLAG FDB cache is overflow");
    }
    sal_memcpy(&sync->array[sync->count], p_entry, sizeof(glb_mlag_fdb_entry_t));
    sync->count++;

    sal_mutex_unlock(g_hsrv_fdb.mlag_sync_mutex);

    if (GLB_MLAG_SYNC_COUNT_MAX <= sync->count)
    {
        hsrv_fdb_mlag_tx_fdb_cache_send();
    }

    return HSRV_E_NONE;
}

/* There function should lock/unlock g_hsrv_fdb.fdb_mutex */
int32
_hsrv_fdb_learning_event_cb(sai_fdb_event_notification_data_t *data, uint32 count)
{
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 i = 0;
    int32 rc = HSRV_E_NONE;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    for (i = 0; i < count; i++)
    {
        rc = _hsrv_fdb_convert_sai_to_fea(&data[i], p_fdb, FALSE);
        if (rc < 0)
        {
            continue;
        }
        
        p_db_fdb = hsrv_fdb_lookup_fdb(p_fdb);
        if (NULL == p_db_fdb)
        {
            rc = _hsrv_fdb_add_fdb(p_fdb, FALSE);
            if (HSRV_E_NONE != rc)
            {
                HSRV_FDB_DEBUG("Add dynamic new fdb failure, ret is %d, "
                                  " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                                  rc, 
                                  p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                                  p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                                  p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
                continue;
            }
        }
        /* update old fdb */
        else
        {
            rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
            if (HSRV_E_NONE != rc)
            {
                HSRV_FDB_DEBUG("Update dynamic fdb failure, ret is %d, "
                                  " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                                  rc, 
                                  p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                                  p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                                  p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
                continue;
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();

    return HSRV_E_NONE;
}

int32
_hsrv_fdb_aging_event_cb(sai_fdb_event_notification_data_t *data, uint32 count)
{
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    uint32 i = 0;
    int32 rc = HSRV_E_NONE;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    for (i = 0; i < count; i++)
    {
        rc = _hsrv_fdb_convert_sai_to_fea(&data[i], p_fdb, TRUE);
        if (rc < 0)
        {
            continue;
        }
        _hsrv_fdb_del_fdb(p_fdb, FALSE, TRUE);
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();

    return HSRV_E_NONE;
}

int32
hsrv_fdb_mac_filter_check(uint8 *mac)
{
    tbl_macfilter_key_t key;
    tbl_macfilter_t *p_db_macflt = NULL;

    sal_memcpy(key.mac, mac, sizeof(key.mac));
    p_db_macflt = tbl_macfilter_get_macfilter(&key);
    if (p_db_macflt)
    {
        return HSRV_E_ENTRY_EXIST;
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_learn_arp_fdb(uint16 vid, uint32 ifindex, uint8 *mac)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;

    if (p_db_mlag->peer_link_ifindex == ifindex)
    {
        return HSRV_E_NONE;
    }

    sal_memset(&fdb, 0, sizeof(fdb));
    sal_memcpy(p_fdb->key.mac, mac, MAC_ADDR_LEN);
    p_fdb->key.fid = vid;
    p_fdb->ifindex = ifindex;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    rc = hsrv_fdb_mac_filter_check(p_fdb->key.mac);
    if (rc < 0)
    {
        sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
        sal_mutex_unlock(g_hsrv_brgif_mutex);
        return HSRV_E_NONE;
    }
    
    p_db_fdb = hsrv_fdb_lookup_fdb(p_fdb);
    if (NULL == p_db_fdb)
    {
        rc = _hsrv_fdb_add_fdb(p_fdb, FALSE);
        if (HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Add dynamic new fdb failure, ret is %d, "
                              " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                              rc, 
                              p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                              p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                              p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
        }
    }
    /* update old fdb */
    else
    {
        if (p_fdb->ifindex != p_db_fdb->ifindex)
        {
            rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
            if (HSRV_E_NONE != rc)
            {
                HSRV_FDB_DEBUG("Update dynamic fdb failure, ret is %d, "
                                  " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                                  rc, 
                                  p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                                  p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                                  p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();
    
    return HSRV_E_NONE;
}

int32
hsrv_fdb_sync_add_fdb(tbl_fdb_t *p_pm_fdb)
{
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;
    
    HSRV_IF_ERROR_RETURN(_hsrv_fdb_convert_pm_to_fea(p_pm_fdb, p_fdb, TRUE));
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    p_db_fdb = hsrv_fdb_lookup_fdb(p_fdb);
    if (NULL == p_db_fdb)
    {
        rc = _hsrv_fdb_add_fdb(p_fdb, FALSE);
        if (HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Add static new fdb failure, ret is %d, "
                              " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                              rc, 
                              p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                              p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                              p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
            goto EXIT;
        }
    }
    else
    {
        rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
        if (HSRV_E_NONE != rc)
        {
            if (GLB_E_RESOURCE_FULL == rc)
            {
                HSRV_FDB_DEBUG("Update static FDB, mac number exceed maximum");
                rc = GLB_E_RESOURCE_FULL;
            }
            else if (GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY == rc)
            {
                HSRV_FDB_DEBUG("Update static FDB, cannot overwrite security entry");
                rc = GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY;
            }
            goto EXIT;
        }
    }

EXIT:
    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();

    return rc;
}

int32
hsrv_fdb_sync_del_fdb(tbl_fdb_t *p_pm_fdb)
{
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    int32 rc = HSRV_E_NONE;
    
    rc = _hsrv_fdb_convert_pm_to_fea(p_pm_fdb, p_fdb, FALSE);
    if (rc < 0)
    {
        return rc;
    }
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    rc = _hsrv_fdb_del_fdb(p_fdb, TRUE, TRUE);
    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();

    return rc;
}

#define __HSRV_FDB_MLAG__

static int32
_hsrv_fdb_mlag_rx_add_fdb(glb_mlag_fdb_entry_t *entry)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_interface_t *p_db_if = NULL;
    tbl_fea_brg_if_t *p_db_brgif_peer_link = NULL;
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    tbl_fea_fdb_t *p_db_fdb = NULL;
    uint32 is_running = FALSE;
    uint32 is_peer_link = FALSE;
    int32 rc = HSRV_E_NONE;
    
    rc = _hsrv_fdb_convert_mlag_to_fea(entry, p_fdb);
    if (rc < 0)
    {
        return HSRV_E_NONE;
    }
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_fdb->ifindex);
    if (p_db_if)
    {
        is_running = tbl_interface_is_running(p_db_if);
    }

    if (!is_running)
    {
        if (0 == p_db_mlag->peer_link_ifindex)
        {
            return HSRV_E_NONE;
        }
        p_db_brgif_peer_link = hsrv_brgif_get(p_db_mlag->peer_link_ifindex);
        if (NULL == p_db_brgif_peer_link)
        {
            return HSRV_E_NONE;
        }
        p_fdb->mlag_source_ifindex = p_fdb->ifindex;
        p_fdb->ifindex = p_db_mlag->peer_link_ifindex;
        p_fdb->portid = p_db_brgif_peer_link->portid;    /* both port and agg */
        is_peer_link = TRUE;
    }

    p_db_fdb = hsrv_fdb_lookup_fdb(p_fdb);
    if (NULL == p_db_fdb)
    {
        rc = _hsrv_fdb_add_fdb(p_fdb, is_peer_link);
        if (HSRV_E_NONE != rc)
        {
            HSRV_FDB_DEBUG("Add MLAG new fdb failure, ret is %d, "
                              " mac:%02X:%02X:%02X:%02X:%02X:%02X vid: %u gport: %u\n",
                              rc, 
                              p_fdb->key.mac[0], p_fdb->key.mac[1], p_fdb->key.mac[2], 
                              p_fdb->key.mac[3], p_fdb->key.mac[4], p_fdb->key.mac[5],
                              p_fdb->key.fid, CTC_SAI_OBJECT_INDEX_GET(p_fdb->portid));
            goto EXIT;
        }
    }
    else
    {
        /* Added by kcao for bug 48616 2018-08-22, check update */
        if (p_fdb->ifindex != p_db_fdb->ifindex)
        {
            rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
            if (HSRV_E_NONE != rc)
            {
                if (GLB_E_RESOURCE_FULL == rc)
                {
                    HSRV_FDB_DEBUG("Update MLAG FDB, mac number exceed maximum");
                    rc = GLB_E_RESOURCE_FULL;
                }
                else if (GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY == rc)
                {
                    HSRV_FDB_DEBUG("Update MLAG FDB, cannot overwrite security entry");
                    rc = GLB_ERR_FDB_CANT_OW_SECURITY_ENTRY;
                }
                goto EXIT;
            }
        }
    }

EXIT:
    return rc;
}

static int32
_hsrv_fdb_mlag_rx_del_fdb(glb_mlag_fdb_entry_t *entry)
{
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    int32 rc = HSRV_E_NONE;
    
    rc = _hsrv_fdb_convert_mlag_to_fea(entry, p_fdb);
    if (rc < 0)
    {
        return rc;
    }
    rc = _hsrv_fdb_del_fdb(p_fdb, FALSE, FALSE);
    return rc;
}

static int32
_hsrv_fdb_mlag_rx_flush_fdb(glb_mlag_fdb_entry_t *req)
{
    tbl_fea_fdb_master_t *p_master = tbl_fea_fdb_get_master();
    tbl_mlag_port_key_t key;
    tbl_mlag_port_t  *p_db_mlag = NULL;
    tbl_fea_brg_if_t *p_db_if = NULL;
    tbl_fea_fdb_t    *p_db_fdb = NULL;
    ctclib_hash_backet_t *hb_next = NULL;
    ctclib_hash_backet_t *pst_hb = NULL;
    int32 index = 0;

    if (GLB_MLAG_FDB_SYNC_FLUSH_PORT == req->act || GLB_MLAG_FDB_SYNC_FLUSH_PORT_VLAN == req->act)
    {
        key.id = req->mlag_id;
        p_db_mlag = tbl_mlag_port_get_mlag_port(&key);
        if (NULL == p_db_mlag)
        {
            return HSRV_E_NONE;
        }

        p_db_if = hsrv_brgif_get(p_db_mlag->ifindex);
        if (NULL == p_db_if)
        {
            return HSRV_E_NONE;
        }
    }
    
    /* flush fdb from hsrv db */
    
    for (index = 0; index < p_master->fdb_hash->size; index++)
    {
        for (pst_hb = p_master->fdb_hash->index[index]; pst_hb; pst_hb = hb_next)
        {
            hb_next = pst_hb->next;
            if (!pst_hb->data)
            {
                continue;
            }
            p_db_fdb = (tbl_fea_fdb_t*)pst_hb->data;

            /*modified by dingyi for mlag with overlay, 2015-04-09*/
            /*flush sync in/out entries*/
            if (!(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
                && !(GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT)))
            {
                continue;
            }
            
            if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
            {
                continue;
            }
            
            if ((GLB_MLAG_FDB_SYNC_FLUSH_PORT == req->act)
                && (p_db_if->key.ifindex != p_db_fdb->ifindex)
                && (p_db_if->key.ifindex != p_db_fdb->mlag_source_ifindex))
            {
                continue;
            }
            else if ((GLB_MLAG_FDB_SYNC_FLUSH_VLAN == req->act)
                && (req->fid != p_db_fdb->key.fid))
            {
                continue;
            }
            else if ((GLB_MLAG_FDB_SYNC_FLUSH_PORT_VLAN == req->act)
                && (p_db_if->key.ifindex != p_db_fdb->ifindex)
                && (p_db_if->key.ifindex != p_db_fdb->mlag_source_ifindex)
                && (req->fid != p_db_fdb->key.fid))
            {
                continue;
            }
            else if ((GLB_MLAG_FDB_SYNC_FLUSH_MAC == req->act)
                && (0 != sal_memcmp(req->mac, p_db_fdb->key.mac, MAC_ADDR_LEN)))
            {
                continue;
            }
            
            _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_rx_fdb(void *pv_msg_data)
{
    glb_mlag_fdb_sync_t *sync = (glb_mlag_fdb_sync_t*)(pv_msg_data);
    glb_mlag_fdb_entry_t *entry = NULL;
    uint32 i = 0;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    
    for (i = 0; i < sync->count; i++)
    {
        entry = &sync->array[i];
        switch (entry->act)
        {
        case GLB_MLAG_FDB_SYNC_ADD:
            _hsrv_fdb_mlag_rx_add_fdb(entry);
            break;

        case GLB_MLAG_FDB_SYNC_DEL:
            _hsrv_fdb_mlag_rx_del_fdb(entry);
            break;

        case GLB_MLAG_FDB_SYNC_FLUSH_ALL:
        case GLB_MLAG_FDB_SYNC_FLUSH_PORT:
        case GLB_MLAG_FDB_SYNC_FLUSH_VLAN:
        case GLB_MLAG_FDB_SYNC_FLUSH_PORT_VLAN:
        case GLB_MLAG_FDB_SYNC_FLUSH_MAC:
            _hsrv_fdb_mlag_rx_flush_fdb(entry);
            break;

        default:
            break;
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_fdb_cache_send();
    
    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_tx_fdb(tbl_fea_fdb_t *p_db_fdb, glb_mlag_fdb_sync_act_t act)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_brg_if_t *p_db_if = NULL;
    glb_mlag_fdb_entry_t entry;
    uint32 if_type = 0;
    uint32 working_ifindex = 0;

    if (GLB_MLAG_FDB_SYNC_FLUSH_ALL == act
     || GLB_MLAG_FDB_SYNC_FLUSH_VLAN == act
     || GLB_MLAG_FDB_SYNC_FLUSH_MAC == act)
    {

    }
    else
    {
        /* 1. check sync condiction */
        if (p_db_fdb->ifindex == p_db_mlag->peer_link_ifindex)
        {
            working_ifindex = p_db_fdb->mlag_source_ifindex;
        }
        else
        {
            working_ifindex = p_db_fdb->ifindex;
        }

        p_db_if = hsrv_brgif_get(working_ifindex);
        if (NULL == p_db_if)
        {
            /* not sync non-brgif */
            return HSRV_E_NONE;
        }

        if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
        {
            /* not sync MLAG sync-in FDB */
            return HSRV_E_NONE;
        }
        
        if (!p_db_if->mlag_id)
        {
            /* not sync MLAG not configured */
            return HSRV_E_NONE;
        }
        
        if (!p_db_if->mlag_peer_conf)
        {
            /* not sync MLAG peer not configured */
            return HSRV_E_NONE;
        }
    }

    if (GLB_MLAG_FDB_SYNC_ADD == act)
    {
        /* Modified by kcao for bug41199 2016-10-24, should uses working_ifindex for peer_link */
        if_type = tbl_interface_get_type_by_ifindex(working_ifindex);
        if (GLB_IF_TYPE_LINKAGG_IF != if_type)
        {
            /* not sync non-LAG interface */
            return HSRV_E_NONE;
        }
        GLB_SET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
    }
    else if(GLB_MLAG_FDB_SYNC_DEL == act)
    {
        /* not sync MLAG sync-out FDB delete */
        if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT))
        {
            return HSRV_E_NONE;
        }
        GLB_UNSET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
    }

    sal_memcpy(entry.mac, p_db_fdb->key.mac, MAC_ADDR_LEN);
    entry.fid = p_db_fdb->key.fid;
    if (GLB_MLAG_FDB_SYNC_FLUSH_ALL == act
     || GLB_MLAG_FDB_SYNC_FLUSH_VLAN == act
     || GLB_MLAG_FDB_SYNC_FLUSH_MAC == act)
    {
        entry.mlag_id = 0;
    }
    else
    {
        entry.mlag_id = p_db_if->mlag_id;
    }
    entry.flag = p_db_fdb->flags;
    entry.act = act;
    
    hsrv_fdb_mlag_tx_fdb_cache_add_entry(&entry);

    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_tx_flush_all()
{
    tbl_fea_fdb_t fdb;
    int32 rc = HSRV_E_NONE;

    sal_memset(&fdb, 0, sizeof(fdb));
    rc = hsrv_fdb_mlag_tx_fdb(&fdb, GLB_MLAG_FDB_SYNC_FLUSH_ALL);
    rc |= hsrv_fdb_mlag_tx_fdb_cache_send();
    return rc;
}

int32
hsrv_fdb_mlag_tx_flush_vlan(vid_t vid)
{
    tbl_fea_fdb_t fdb;
    int32 rc = HSRV_E_NONE;

    sal_memset(&fdb, 0, sizeof(fdb));
    fdb.key.fid = vid;
    rc = hsrv_fdb_mlag_tx_fdb(&fdb, GLB_MLAG_FDB_SYNC_FLUSH_VLAN);
    rc |= hsrv_fdb_mlag_tx_fdb_cache_send();
    return rc;
}

int32
hsrv_fdb_mlag_tx_flush_port(uint32 ifindex)
{
    tbl_fea_fdb_t fdb;
    int32 rc = HSRV_E_NONE;

    sal_memset(&fdb, 0, sizeof(fdb));
    fdb.ifindex = ifindex;
    rc = hsrv_fdb_mlag_tx_fdb(&fdb, GLB_MLAG_FDB_SYNC_FLUSH_PORT);
    rc |= hsrv_fdb_mlag_tx_fdb_cache_send();
    return rc;
}

int32
hsrv_fdb_mlag_tx_flush_port_vlan(uint32 ifindex, vid_t vid)
{
    tbl_fea_fdb_t fdb;
    int32 rc = HSRV_E_NONE;

    sal_memset(&fdb, 0, sizeof(fdb));
    fdb.ifindex = ifindex;
    rc = hsrv_fdb_mlag_tx_fdb(&fdb, GLB_MLAG_FDB_SYNC_FLUSH_PORT_VLAN);
    rc |= hsrv_fdb_mlag_tx_fdb_cache_send();
    return rc;
}
    
int32
hsrv_fdb_mlag_tx_flush_mac(uint8 *mac)
{
    tbl_fea_fdb_t fdb;
    int32 rc = HSRV_E_NONE;

    sal_memset(&fdb, 0, sizeof(fdb));
    sal_memcpy(fdb.key.mac, mac, MAC_ADDR_LEN);
    rc = hsrv_fdb_mlag_tx_fdb(&fdb, GLB_MLAG_FDB_SYNC_FLUSH_MAC);
    rc |= hsrv_fdb_mlag_tx_fdb_cache_send();
    return rc;
}

int32
hsrv_fdb_mlag_peer_conf_cb(uint32 ifindex, uint32 enable)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_brg_if_t *p_db_if = NULL;  
    tbl_fea_brg_if_t *p_db_brgif_peer = NULL;
    uint32 peer_ifindex = 0;
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (NULL == p_db_if)
    {
        goto EXIT;
    }

    /* 1. sync interface */
#ifdef FDB_SLIST
    CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
    CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
    {
        if (enable)
        {
        	if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG))
            {
            	hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_ADD);
        	}
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
            {
                _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
            }
            else
            {
                GLB_UNSET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
            }
        }
    }

    /* Added by kcao for bug41199 2016-10-24, should sync peer-interface's protected FDB */
    peer_ifindex = p_db_mlag->peer_link_ifindex;
    p_db_brgif_peer = hsrv_brgif_get(peer_ifindex);
    if (NULL == p_db_brgif_peer)
    {
        goto EXIT;
    }

    /* 2. sync peer-interface */
#ifdef FDB_SLIST
    CTCLIB_SLIST_LOOP_DEL(p_db_brgif_peer->fdb_list, p_db_fdb, listnode, next)
#else
    CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_brgif_peer->fdb_queue, port_entry)
#endif
    {
        if (ifindex != p_db_fdb->mlag_source_ifindex)
        {
            continue;
        }
        
        if (enable)
        {
            hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_ADD);
        }
        else
        {
            if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
            {
                _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
            }
            else
            {
                GLB_UNSET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
            }
        }
    }

    /* Added by kcao for bug 41123 2016-10-15, need to send cache to peer */
    hsrv_fdb_mlag_tx_fdb_cache_send();

EXIT:
    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_set_protect_en(uint32 mlag_id, uint32 protect_en)
{
    tbl_mlag_port_key_t key;
    tbl_mlag_port_t *p_db_port = NULL;
    tbl_fea_brg_if_t *p_db_brgif = NULL;
    tbl_fea_brg_if_t *p_db_brgif_peer = NULL;
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    uint32 working_ifindex = 0;
    uint32 peer_ifindex = 0;
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    tbl_fea_fdb_t fdb;
    tbl_fea_fdb_t *p_fdb = &fdb;
    int32 rc = HSRV_E_NONE;

    key.id = mlag_id;
    p_db_port = tbl_mlag_port_get_mlag_port(&key);
    if (NULL == p_db_port)
    {
        return HSRV_E_ENTRY_NOT_EXIST;        
    }
    working_ifindex = p_db_port->ifindex;
    peer_ifindex = p_db_mlag->peer_link_ifindex;
    
    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    p_db_brgif = hsrv_brgif_get(working_ifindex);
    if (NULL == p_db_brgif)
    {
        goto EXIT;
    }

    p_db_brgif_peer = hsrv_brgif_get(peer_ifindex);
    if (NULL == p_db_brgif_peer)
    {
        goto EXIT;
    }

    if (protect_en)
    {
        /*flush fdb instead of update when remote is also down*/
        if (!hsrv_mlag_is_peer_up(p_db_port))
        {
            sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
            sal_mutex_unlock(g_hsrv_brgif_mutex);
            //hsrv_fdb_flush_fdb_port(working_ifindex);
            return HSRV_E_NONE;
        }

#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_db_brgif->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_brgif->fdb_queue, port_entry)
#endif
        {
            sal_memcpy(p_fdb, p_db_fdb, sizeof(tbl_fea_fdb_t));
            p_fdb->mlag_source_ifindex = p_fdb->ifindex;
            p_fdb->ifindex = peer_ifindex;
            p_fdb->portid = p_db_brgif_peer->portid;
            rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
            if (rc < 0)
            {
                /* Added by kcao for bug 41509, need check update fail case */
                _hsrv_fdb_del_fdb(p_db_fdb, FALSE, TRUE);
            }
        }
    }
    else
    {
#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_db_brgif_peer->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_brgif_peer->fdb_queue, port_entry)
#endif
        {
            if (p_db_fdb->mlag_source_ifindex != working_ifindex)
            {
                continue;
            }
            sal_memcpy(p_fdb, p_db_fdb, sizeof(tbl_fea_fdb_t));
            p_fdb->ifindex = p_fdb->mlag_source_ifindex;
            p_fdb->portid = p_db_brgif->portid;
            p_fdb->mlag_source_ifindex = 0;
            rc = _hsrv_fdb_update_fdb(p_fdb, p_db_fdb);
            if (rc < 0)
            {
                /* Added by kcao for bug 41509, need check update fail case */
                _hsrv_fdb_del_fdb(p_db_fdb, FALSE, TRUE);
            }
        }
    }

EXIT:

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    /* Added by kcao for bug 41198 2016-10-24, fix miss send cache problem */
    hsrv_fdb_mlag_tx_fdb_cache_send();
    
    return HSRV_E_NONE;
}

/* Added by kcao for bug 41121, flush protected FDB at peer-link port when MLAG is removed */
int32
hsrv_fdb_mlag_flush_peer_link_fdb(uint32 ifindex)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_brg_if_t *p_db_if = NULL;
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;

    /* get port fdb queue */
    p_db_if = hsrv_brgif_get(p_db_mlag->peer_link_ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }   

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    
#ifdef FDB_SLIST
    CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
    CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
    {
        if (p_db_fdb->mlag_source_ifindex == ifindex)
        {
            rc = _hsrv_fdb_del_fdb(p_db_fdb, TRUE, TRUE);
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    (void)rc;
    return HSRV_E_NONE;
}

/* Added by kcao for bug 41102, flush protected FDB at peer-link port when MLAG is removed */
int32
hsrv_fdb_mlag_sync_flush_peer_link(uint32 ifindex)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_fea_brg_if_t *p_db_if = NULL;
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;

    /* get port fdb queue */
    p_db_if = hsrv_brgif_get(p_db_mlag->peer_link_ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }   

    /* not peer link, return */
    if (p_db_if->key.ifindex != ifindex)
    {
        return HSRV_E_NONE;
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    
#ifdef FDB_SLIST
    CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
    CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
    {
        if (p_db_fdb->mlag_source_ifindex)
        {
            rc = hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_DEL);
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex);
    /* liwh end */

    (void)rc;
    return HSRV_E_NONE;
}

int32
hsrv_fdb_mlag_peer_link_destory_cb(uint32 ifindex)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    p_db_if = hsrv_brgif_get(ifindex);
    if (NULL == p_db_if)
    {
        goto EXIT;
    }

    /* 1. sync interface */
#ifdef FDB_SLIST
    CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
    CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
    if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        /* only flush protected MLAG FDB */
        if (p_db_fdb->mlag_source_ifindex)
        {
            _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
        }
    }

EXIT:
    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return HSRV_E_NONE;
}

#define __HSRV_FDB_APIS__

void 
hsrv_fdb_event_notification_cb(
    _In_ uint32_t count,
    _In_ sai_fdb_event_notification_data_t *data)
{
    if (SAI_FDB_EVENT_LEARNED == data[0].event_type)
    {
        _hsrv_fdb_learning_event_cb(data, count);
    }
    else if (SAI_FDB_EVENT_AGED == data[0].event_type)
    {
        _hsrv_fdb_aging_event_cb(data, count);
    }

    return;
}

int32
hsrv_fdb_get_count_number(uint32 *static_count, uint32 *static_security_count, uint32 *dynamic_count, uint32 *dynamic_security_count, uint32 *total_count)
{
    tbl_brg_global_t *p_db_glb = tbl_brg_global_get_brg_global();
    
    if (p_db_glb->hw_learning_enable)
    {
        return hsrv_fdb_hw_get_count_number(static_count, dynamic_count, total_count);
    }
    else
    {
        *static_count = g_hsrv_fdb.static_fdb_cnt;
        *static_security_count = g_hsrv_fdb.static_security_entry_cnt;
        *dynamic_count = g_hsrv_fdb.dynamic_fdb_cnt;
        *dynamic_security_count = g_hsrv_fdb.dynamic_security_entry_cnt;
        *total_count = g_hsrv_fdb.static_fdb_cnt + g_hsrv_fdb.dynamic_fdb_cnt 
                        + g_hsrv_fdb.dynamic_security_entry_cnt + g_hsrv_fdb.static_security_entry_cnt;
    }

    return HSRV_E_NONE;
}

static void
hsrv_fdb_loop_timer_handler()
{
    uint32 curr_bucket = 0;
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();
    
    g_p_fdb_loop_timer = CTC_TASK_ADD_TIME(hsrv_fdb_loop_timer_handler, NULL, HSRV_FDB_LOOP_BUCKET_TIME);

    curr_bucket = p_brg_global->fdb_loop_curr_size + p_brg_global->fdb_loop_add_rate;
    p_brg_global->fdb_loop_curr_size = (curr_bucket < p_brg_global->fdb_loop_max_size) ?
                                       curr_bucket : p_brg_global->fdb_loop_max_size;
    return;
}

static int32
hsrv_fdb_errdis_fdb_loop_init()
{
    tbl_brg_global_t *p_brg_global = tbl_brg_global_get_brg_global();

    g_hsrv_fdb_loop.cache_err_threshold = 4;
    p_brg_global->fdb_loop_curr_size = 0;
    p_brg_global->fdb_loop_disc_cnt = 0;

    g_p_fdb_loop_timer = CTC_TASK_ADD_TIME(hsrv_fdb_loop_timer_handler, NULL, HSRV_FDB_LOOP_BUCKET_TIME);
    
    return HSRV_E_NONE;
}

int32
hsrv_fdb_set_fdb_loop_detect_en(tbl_brg_global_t* p_brg_global)
{
    tbl_brg_global_t *p_db_brg_global = tbl_brg_global_get_brg_global();
    p_db_brg_global->fdb_loop_errdis_en = p_brg_global->fdb_loop_errdis_en;
    
    return HSRV_E_NONE; 
}

int32
hsrv_fdb_set_fdb_loop(tbl_brg_global_t* p_brg_global)
{
    tbl_brg_global_t *p_db_brg_global = tbl_brg_global_get_brg_global();
    p_db_brg_global->fdb_loop_curr_size = p_brg_global->fdb_loop_max_size;
    
    hsrv_fdb_clear_fdb_loop_cache();
    
    return HSRV_E_NONE; 
}
/***************************************************************************************************
 * Name         : hsrv_fdb_sync 
 * Purpose      : load fdb cdb to sai          
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
hsrv_fdb_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_fdb_key_t*   p_fdb_key = NULL;
    tbl_fdb_t *      p_fdb = NULL;
    int32 rc = HSRV_E_NONE;
    
    HSRV_FDB_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);
        
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_fdb = (tbl_fdb_t*)p_tbl;
        HSRV_FDB_DEBUG("add fdb mac = %x%x%x%x%x%x, vlan = %d, ifindex=%d", p_fdb->key.mac[0],p_fdb->key.mac[1],
        p_fdb->key.mac[2],p_fdb->key.mac[3],p_fdb->key.mac[4],p_fdb->key.mac[5], p_fdb->key.fid,p_fdb->ifindex);

        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) || GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            HSRV_IF_ERROR_RETURN(hsrv_fdb_sync_add_fdb(p_fdb));  
        }
        break;

    case CDB_OPER_DEL:
        p_fdb_key = (tbl_fdb_key_t*)p_tbl;
        HSRV_FDB_DEBUG("del fdb mac = %x%x%x%x%x%x, vlan = %d", p_fdb_key->mac[0],p_fdb_key->mac[1],
        p_fdb_key->mac[2],p_fdb_key->mac[3],p_fdb_key->mac[4],p_fdb_key->mac[5], p_fdb_key->fid);
        p_fdb = tbl_fdb_get_fdb(p_fdb_key);
        HSRV_PTR_CHECK(p_fdb);
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) || GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            /* Added by kcao for bug 41092 2016-10-13, check mlag peer FDB exist */
            rc = hsrv_is_mlag_peer_fdb(p_fdb);
            if (HSRV_E_NONE == rc)
            {
                HSRV_IF_ERROR_RETURN(hsrv_fdb_sync_del_fdb(p_fdb));
            }
        }
        break;

    case CDB_OPER_SET:
        p_fdb = (tbl_fdb_t*)p_tbl;
        HSRV_FDB_DEBUG("update fdb mac = %x%x%x%x%x%x, vlan = %d, ifindex=%d", p_fdb->key.mac[0],p_fdb->key.mac[1],
        p_fdb->key.mac[2],p_fdb->key.mac[3],p_fdb->key.mac[4],p_fdb->key.mac[5], p_fdb->key.fid,p_fdb->ifindex);

        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) || GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            HSRV_IF_ERROR_RETURN(hsrv_fdb_sync_add_fdb(p_fdb));  
        }
        break;

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_start()
{
    sal_memset(&g_hsrv_fdb, 0, sizeof(g_hsrv_fdb));
    sal_memset(&g_hsrv_fdb_loop, 0, sizeof(g_hsrv_fdb_loop));

    hsrv_fdb_db_init();
    hsrv_fdb_hw_init();
    hsrv_fdb_errdis_fdb_loop_init();

    hsrv_message_set_cb_from_fei(FEI_MLAG_FDB_SYNC, hsrv_fdb_mlag_rx_fdb);

    return HSRV_E_NONE;
}

