#include "hsrv_inc.h"
#include "hsrv_ptp.h"

#include "gen/tbl_ptp_global_define.h"
#include "gen/tbl_ptp_global.h"
#include "gen/tbl_ptp_port_define.h"
#include "gen/tbl_ptp_port.h"
#include "gen/tbl_fea_ptp_global_define.h"
#include "gen/tbl_fea_ptp_global.h"
#include "gen/tbl_fea_ptp_ts_define.h"
#include "gen/tbl_fea_ptp_ts.h"

//#define PTP_NO_USE_TS

#define HSRV_PTP_LOG_EMERG(fmt, args...)   HSRV_LOG_EMERG("PTP "fmt, ##args)
#define HSRV_PTP_LOG_ALERT(fmt, args...)   HSRV_LOG_ALERT("PTP "fmt, ##args)
#define HSRV_PTP_LOG_CRIT(fmt, args...)    HSRV_LOG_CRIT("PTP "fmt, ##args)
#define HSRV_PTP_LOG_ERR(fmt, args...)     HSRV_LOG_ERR("PTP "fmt, ##args)
#define HSRV_PTP_LOG_WARN(fmt, args...)    HSRV_LOG_WARN("PTP "fmt, ##args)
#define HSRV_PTP_LOG_NOTICE(fmt, args...)  HSRV_LOG_NOTICE("PTP "fmt, ##args)
#define HSRV_PTP_LOG_INFO(fmt, args...)    HSRV_LOG_INFO("PTP "fmt, ##args)

#define HSRV_PTP_DEBUG_FUNC() \
do \
{ \
    HSRV_LOG_DEBUG(ptp, PTP_CFG, "Enter into %s.", __FUNCTION__); \
} \
while (0)

sal_mutex_t *g_hsrv_ptp_mutex = NULL;

void
_hsrv_ptp_ts_destroy_db()
{
    tbl_fea_ptp_ts_master_t * p_fea_ptp_ts_master = NULL;
    tbl_fea_ptp_ts_t *p_fea_ptp_ts = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;

    p_fea_ptp_ts_master = tbl_fea_ptp_ts_get_master();
    if (NULL == p_fea_ptp_ts_master)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp ts master does not exist");
        return;
    }
    
    CTCLIB_SLIST_LOOP_DEL(p_fea_ptp_ts_master->fea_ptp_ts_list, p_fea_ptp_ts, listnode, next)
    {
        tbl_fea_ptp_ts_del_fea_ptp_ts_sync(&(p_fea_ptp_ts->key), FALSE);
    }
    
    return;
}

void
_hsrv_ptp_reset_db()
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    
    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return ;
    }

    p_fea_ptp_global->ready = 0;
    p_fea_ptp_global->blocked = 0;
    p_fea_ptp_global->type = 0;
    p_fea_ptp_global->tx_ifindex = 0;
    p_fea_ptp_global->tx_seq_id = 0;
    p_fea_ptp_global->intr_count = 0;
    
    _hsrv_ptp_ts_destroy_db();

    return;
}

ctc_task_t *
hsrv_ptp_util_add_timer_ms(void (*func) (void *), void *arg, int32 ms)
{
    return ctc_task_add_timer_msec(TASK_PRI_NORMAL, func, arg, ms);
}

void
hsrv_ptp_util_cancel_timer(ctc_task_t **ppt)
{
    if (ppt && *ppt)
    {
        ctc_task_delete_timer(*ppt);
        *ppt = NULL;
    }

    return;
}

int32
hsrv_ptp_sync_systime()
{
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;

    HSRV_PTP_DEBUG_FUNC();
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    attr.id = SAI_PTP_ATTR_TIMESTAMP;
    attr_count++;
    
    HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, &attr));

    return HSRV_E_NONE;
}

#ifndef ptp_global
static int32
_hsrv_ptp_set_enable(uint32 enable)
{
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;
    tbl_fea_ptp_global_t *g_fea_ptp_master = NULL;
    
    g_fea_ptp_master = tbl_fea_ptp_global_get_fea_ptp_global();
    if (g_fea_ptp_master == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return PM_E_FAIL;
    }

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    g_fea_ptp_master->global_enable = enable;
    
    attr.id = SAI_PTP_ATTR_DEVICE_TYPE;
    if (enable)
    {
        //attr.value.u32 = SAI_PTP_DEVICE_E2E_TC; /* default type */
        attr.value.u32 = SAI_PTP_DEVICE_OC; 
    }
    else
    {
        attr.value.u32 = SAI_PTP_DEVICE_NONE;
    }
    attr_count++;

    HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, &attr));

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_set_adjust_offset(int64 offset)
{
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;
    int32 ret = 0;

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    attr.id = SAI_PTP_ATTR_ADJUST_OFFSET;
    attr.value.s64 = offset;
    attr_count++;

    HSRV_PTP_CFG_DEBUG("Set adjust_offset: %"PRId64".", offset);
    /*for bug 46524*/
    //HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, &attr));
    ret = ptp_api->set_ptp_attribute(attr_count, &attr);
    if (ret != HSRV_E_NONE)
    {
        HSRV_PTP_CFG_DEBUG("Set adjust_offset: %"PRId64", failed %d.", offset, ret);
        return ret;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_set_hsrv_device_type(uint8 dev_type)
{
    tbl_ptp_global_t *g_p_master = NULL;
    tbl_fea_ptp_global_t *g_fea_ptp_master = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    g_p_master = tbl_ptp_global_get_ptp_global();
    if (g_p_master == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Ptp global does not exist");
        return PM_E_FAIL;
    }

    g_fea_ptp_master = tbl_fea_ptp_global_get_fea_ptp_global();
    if (g_fea_ptp_master == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return PM_E_FAIL;
    }
    
    attr.id = SAI_PTP_ATTR_DEVICE_TYPE;
    if(!g_fea_ptp_master->global_enable)
    {
        attr.value.u32 = CTC_PTP_DEVICE_NONE;
        attr.value.u32 = SAI_PTP_DEVICE_NONE;
    }
    else
    {
        if(PTP_DEVICE_OC == dev_type)
        {
            attr.value.u32 = CTC_PTP_DEVICE_OC;
            attr.value.u32 = SAI_PTP_DEVICE_OC;
        }
        else if(PTP_DEVICE_BC == dev_type)
        {
            attr.value.u32 = CTC_PTP_DEVICE_BC;
            attr.value.u32 = SAI_PTP_DEVICE_BC;
        }
        else if(PTP_DEVICE_E2ETC == dev_type)
        {
            if(g_p_master->default_ds.two_step_flag == TRUE)
            {
                attr.value.u32 = CTC_PTP_DEVICE_BC;
                attr.value.u32 = SAI_PTP_DEVICE_BC;
            }
            else
            {
                attr.value.u32 = CTC_PTP_DEVICE_E2E_TC;
                attr.value.u32 = SAI_PTP_DEVICE_E2E_TC;
            }
        }
        else if(PTP_DEVICE_P2PTC == dev_type)
        {
            if(g_p_master->default_ds.two_step_flag == TRUE)
            {
                attr.value.u32 = CTC_PTP_DEVICE_BC;
                attr.value.u32 = SAI_PTP_DEVICE_BC;
            }
            else
            {
                attr.value.u32 = CTC_PTP_DEVICE_P2P_TC;
                attr.value.u32 = SAI_PTP_DEVICE_P2P_TC;
            }
        }
        else
        {
            HSRV_PTP_CFG_DEBUG("Device type %"PRIu64" is invalid", dev_type);
            return PM_E_FAIL;
        }
    }
    attr_count++;

    HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, &attr));

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_set_update_system_time(int16 utc_offset)
{
    tbl_fea_ptp_global_t * p_fea_ptp_global = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;
    uint32 rc = 0;
    int32_t  list_value[2]; 

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (p_fea_ptp_global == NULL)
    {
        HSRV_PTP_CFG_DEBUG("tbl_fea_ptp_global is NULL");
        return PM_E_FAIL;
    }
    
    attr.id = SAI_PTP_ATTR_HARDWARE_SECOND;
    attr.value.s32list.list = list_value;
    attr_count++;

    HSRV_IF_ERROR_RETURN(ptp_api->get_ptp_attribute(attr_count, &attr));

    HSRV_PTP_CFG_DEBUG("Current TAI time %u:%u", attr.value.s32list.list[0], attr.value.s32list.list[1]);

    struct sal_timeval tv;
    struct sal_timezone tz;
    
    sal_memset(&tz, 0x00, sizeof(tz));
    sal_memset(&tv, 0x00, sizeof(tv));

    rc = gettimeofday(&tv, &tz); 
    
    tv.tv_sec = attr.value.s32list.list[0] - utc_offset;
    tv.tv_usec = attr.value.s32list.list[1]/ 1000;
    rc = sal_settimeofday(&tv, &tz);
    if (rc)
    {
        HSRV_PTP_LOG_ERR("Update local system time failed %d", rc);
        return PM_E_FAIL;
    }

    /*for bug 48659 by chenc*/
    char strTime[CMD_BUF_256];
    sal_time_t date;

    sal_memset(strTime, 0x00, sizeof(strTime));
    date = sal_time(NULL);
    sal_time_strf(strTime, CMD_BUF_256, "%T %Z %a %b %d %Y", sal_localtime(&date));
    
    logid_sys(LOG_SYSTEM_4_TIMEUPDATE_BY_PTP, strTime);

    /*fix bug 18633 by zhangjx 20120713 */
     tbl_fea_ptp_global_set_fea_ptp_global_field(p_fea_ptp_global, TBL_FEA_PTP_GLOBAL_FLD_ACL_UPDATE_TIME_RANGE);
     
    return HSRV_E_NONE;
}


int32
hsrv_ptp_global_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ptp_global_t *p_global = NULL;
    int32 ret = 0;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_global = (tbl_ptp_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_PTP_GLOBAL_FLD_GLOBAL_ENABLE:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_set_enable(p_global->global_enable));
            break;
        case TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_OFFSET:
            /*for bug 46524*/
            //HSRV_IF_ERROR_RETURN(_hsrv_ptp_set_adjust_offset(p_global->hsrv_adjust_offset));
            ret = _hsrv_ptp_set_adjust_offset(p_global->hsrv_adjust_offset);
            if (ret != HSRV_E_NONE)
            {
                HSRV_PTP_CFG_DEBUG("HSRV ptp set adjust_offset: %"PRId64", failed %d.", p_global->hsrv_adjust_offset, ret);
                return ret;
            }
            break;
        case TBL_PTP_GLOBAL_FLD_HSRV_ADJUST_FFO:
            //HSRV_IF_ERROR_RETURN(_hsrv_ptp_set_adjust_ffo(p_global->hsrv_adjust_ffo));
            break;
        case TBL_PTP_GLOBAL_FLD_HSRV_DEVICE_TYPE:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_set_hsrv_device_type(p_global->hsrv_device_type));
            break;
        case TBL_PTP_GLOBAL_FLD_HSRV_UPDATE_SYSTEM_TIME:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_set_update_system_time(p_global->hsrv_update_system_time));
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}
#endif

#ifndef ptp_port
static int32
_hsrv_ptp_port_set_enable(uint32 ifindex, uint32 enable)
{
    tbl_interface_t *p_db_if = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr[2];
    uint32          attr_count = 0;
    uint32          attr_count_tmp = 0;
    uint16 gport;
    tbl_interface_t *p_db_mem_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_object_id_t port_oid;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (p_db_if == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Interface of index %d does not exist", ifindex);
        /*fix bug 46616 by chenc*/
        return HSRV_E_NONE;
    }

    sal_memset(attr, 0, sizeof(sai_attribute_t) * 2);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (NULL == p_db_if->lag)
        {
            HSRV_PTP_CFG_DEBUG("Interface %s's member port does not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return HSRV_E_NONE;
        }

        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode)
        {
            port_oid = p_db_mem_if->portid;

            attr_count_tmp = attr_count;
            
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_db_mem_if->ifindex, &gport));
            attr[attr_count_tmp].id = SAI_PTP_ATTR_GPORT;
            attr[attr_count_tmp].value.u16 = gport;
            attr_count_tmp++;

            attr[attr_count_tmp].id = SAI_PTP_ATTR_PORT_ENABLE;
            attr[attr_count_tmp].value.u32 = enable;
            attr_count_tmp++;

            HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
            
            HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count_tmp, attr));
        }
    }
    else if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        port_oid = p_db_if->portid;
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        attr[attr_count].id = SAI_PTP_ATTR_GPORT;
        attr[attr_count].value.u16 = gport;
        attr_count++;

        attr[attr_count].id = SAI_PTP_ATTR_PORT_ENABLE;
        attr[attr_count].value.u32 = enable;
        attr_count++;

        HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
        
        HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, attr));
    }
    else
    {
        HSRV_PTP_CFG_DEBUG("Interface %s hw type %u error", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->hw_type);
        HSRV_IF_ERROR_RETURN(HSRV_E_FAIL);
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_port_set_path_delay(uint32 ifindex, int64 path_delay)
{
    tbl_interface_t *p_db_if = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr[2];
    uint32          attr_count = 0;
    uint32          attr_count_tmp = 0;
    uint16 gport;
    tbl_interface_t *p_db_mem_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_object_id_t port_oid;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (p_db_if == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Interface of index %d does not exist", ifindex);
        /*fix bug 46616 by chenc*/
        return HSRV_E_NONE;
    }

    sal_memset(attr, 0, sizeof(sai_attribute_t) * 2);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (NULL == p_db_if->lag)
        {
            HSRV_PTP_CFG_DEBUG("Interface %s's member port does not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return HSRV_E_NONE;
        }

        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode)
        {
            port_oid = p_db_mem_if->portid;

            attr_count_tmp = attr_count;
            
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_db_mem_if->ifindex, &gport));
            attr[attr_count_tmp].id = SAI_PTP_ATTR_GPORT;
            attr[attr_count_tmp].value.u16 = gport;
            attr_count_tmp++;

            attr[attr_count_tmp].id = SAI_PTP_ATTR_PORT_PATH_DELAY;
            attr[attr_count_tmp].value.s64 = path_delay;
            attr_count_tmp++;

            HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
            
            HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count_tmp, attr));
        }
    }
    else if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        port_oid = p_db_if->portid;
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        attr[attr_count].id = SAI_PTP_ATTR_GPORT;
        attr[attr_count].value.u16 = gport;
        attr_count++;

        attr[attr_count].id = SAI_PTP_ATTR_PORT_PATH_DELAY;
        attr[attr_count].value.s64 = path_delay;
        attr_count++;

        HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
        
        HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, attr));
    }
    else
    {
        HSRV_PTP_CFG_DEBUG("Interface %s hw type %u error", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->hw_type);
        HSRV_IF_ERROR_RETURN(HSRV_E_FAIL);
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_port_set_asymmetry_correction(uint32 ifindex, int64 asymmetry_correction)
{
    tbl_interface_t *p_db_if = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr[2];
    uint32          attr_count = 0;
    uint32          attr_count_tmp = 0;
    uint16 gport;
    tbl_interface_t *p_db_mem_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_object_id_t port_oid;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (p_db_if == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Interface of index %d does not exist", ifindex);
        /*fix bug 46616 by chenc*/
        return HSRV_E_NONE;
    }

    sal_memset(attr, 0, sizeof(sai_attribute_t) * 2);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (NULL == p_db_if->lag)
        {
            HSRV_PTP_CFG_DEBUG("Interface %s's member port does not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return HSRV_E_NONE;
        }

        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode)
        {
            port_oid = p_db_mem_if->portid;

            attr_count_tmp = attr_count;
            
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_db_mem_if->ifindex, &gport));
            attr[attr_count_tmp].id = SAI_PTP_ATTR_GPORT;
            attr[attr_count_tmp].value.u16 = gport;
            attr_count_tmp++;

            attr[attr_count_tmp].id = SAI_PTP_ATTR_PORT_ASYMMETRY_CORRECTION;
            attr[attr_count_tmp].value.s64 = asymmetry_correction;
            attr_count_tmp++;

            HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
            
            HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count_tmp, attr));
        }
    }
    else if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        port_oid = p_db_if->portid;
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        attr[attr_count].id = SAI_PTP_ATTR_GPORT;
        attr[attr_count].value.u16 = gport;
        attr_count++;

        attr[attr_count].id = SAI_PTP_ATTR_PORT_ASYMMETRY_CORRECTION;
        attr[attr_count].value.s64 = asymmetry_correction;
        attr_count++;

        HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
        
        HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, attr));
    }
    else
    {
        HSRV_PTP_CFG_DEBUG("Interface %s hw type %u error", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->hw_type);
        HSRV_IF_ERROR_RETURN(HSRV_E_FAIL);
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_port_set_xgress_latency(uint32 ifindex, int32 gress_latency, uint32 is_ingress)
{
    tbl_interface_t *p_db_if = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr[2];
    uint32          attr_count = 0;
    uint32          attr_count_tmp = 0;
    uint16 gport;
    tbl_interface_t *p_db_mem_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    sai_object_id_t port_oid;
    char ifname_ext[IFNAME_EXT_SIZE] = {0};

    HSRV_PTP_CFG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (p_db_if == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Interface of index %d does not exist", ifindex);
        /*fix bug 46616 by chenc*/
        return HSRV_E_NONE;
    }

    sal_memset(attr, 0, sizeof(sai_attribute_t) * 2);

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (NULL == p_db_if->lag)
        {
            HSRV_PTP_CFG_DEBUG("Interface %s's member port does not exist", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return HSRV_E_NONE;
        }

        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode)
        {
            port_oid = p_db_mem_if->portid;

            attr_count_tmp = attr_count;
            
            HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_db_mem_if->ifindex, &gport));
            attr[attr_count_tmp].id = SAI_PTP_ATTR_GPORT;
            attr[attr_count_tmp].value.u16 = gport;
            attr_count_tmp++;

            if (is_ingress == TRUE)
            {
                attr[attr_count_tmp].id = SAI_PTP_ATTR_PORT_INGRESS_LATENCY_CORRECTION;
            }
            else if (is_ingress == FALSE)
            {
                attr[attr_count_tmp].id = SAI_PTP_ATTR_PORT_EGRESS_LATENCY_CORRECTION;
            }
            else
            {
                HSRV_PTP_CFG_DEBUG("Flag is_ingress %d is invalid", is_ingress);
                return PM_E_FAIL;
            }
            
            attr[attr_count_tmp].value.s32 = gress_latency;
            attr_count_tmp++;

            HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
            
            HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count_tmp, attr));
        }
    }
    else if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
    {
        port_oid = p_db_if->portid;
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(ifindex, &gport));
        attr[attr_count].id = SAI_PTP_ATTR_GPORT;
        attr[attr_count].value.u16 = gport;
        attr_count++;

        if (is_ingress == TRUE)
        {
            attr[attr_count].id = SAI_PTP_ATTR_PORT_INGRESS_LATENCY_CORRECTION;
        }
        else if (is_ingress == FALSE)
        {
            attr[attr_count].id = SAI_PTP_ATTR_PORT_EGRESS_LATENCY_CORRECTION;
        }
        else
        {
            HSRV_PTP_CFG_DEBUG("Flag is_ingress %d is invalid", is_ingress);
            return PM_E_FAIL;
        }
        
        attr[attr_count].value.s32 = gress_latency;
        attr_count++;

        HSRV_PTP_CFG_DEBUG("Interface %s port oid %"PRIu64", gport %hu", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), port_oid, gport);
        
        HSRV_IF_ERROR_RETURN(ptp_api->set_ptp_attribute(attr_count, attr));
    }
    else
    {
        HSRV_PTP_CFG_DEBUG("Interface %s hw type %u error", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext), p_db_if->hw_type);
        HSRV_IF_ERROR_RETURN(HSRV_E_FAIL);
    }

    return HSRV_E_NONE;
}

int32
hsrv_ptp_port_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ptp_port_t *p_ptp_port = NULL;

    switch (oper)
    {
    case CDB_OPER_SET:
        p_ptp_port = (tbl_ptp_port_t*)p_tbl;
        switch (field_id)
        {
        case TBL_PTP_PORT_FLD_INGRESS_LATENCY:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_port_set_xgress_latency(p_ptp_port->ifindex, p_ptp_port->ingress_latency, TRUE));
            break;
        case TBL_PTP_PORT_FLD_EGRESS_LATENCY:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_port_set_xgress_latency(p_ptp_port->ifindex, p_ptp_port->egress_latency, FALSE));
            break;
        case TBL_PTP_PORT_FLD_ASYMMETRY_CORRECTION:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_port_set_asymmetry_correction(p_ptp_port->ifindex, p_ptp_port->asymmetry_correction.v));
            break;
        case TBL_PTP_PORT_FLD_HSRV_PATH_DELAY:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_port_set_path_delay(p_ptp_port->ifindex, p_ptp_port->hsrv_path_delay));
            break;
        case TBL_PTP_PORT_FLD_ENABLE:
            HSRV_IF_ERROR_RETURN(_hsrv_ptp_port_set_enable(p_ptp_port->ifindex, p_ptp_port->enable));
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}
#endif

static inline char*
_hsrv_ptp_desc_bool(bool v)
{
    return (v) ? "true" : "false";
}

int32
hsrv_ptp_tx_discard_cb(void)
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    
    HSRV_PTP_DEBUG_FUNC();

    sal_mutex_lock(g_hsrv_ptp_mutex);
    
    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return HSRV_E_FAIL;
    }
#if 0
    HSRV_PTP_CFG_DEBUG(DEBUG_PTP_TX_TS,
            "  g_p_hsrv_ptp->info.{ready=%s, tx_ifindex=%u, intr_ifindex=%u, ts=%hu%u:%u}",
                _hsrv_ptp_desc_bool(g_p_hsrv_ptp->info.ready),
                g_p_hsrv_ptp->info.tx_ifindex,
                g_p_hsrv_ptp->info.intr_ifindex,
                g_p_hsrv_ptp->info.ts.sec.msb,
                g_p_hsrv_ptp->info.ts.sec.lsb,
                g_p_hsrv_ptp->info.ts.ns);
#endif
    _hsrv_ptp_reset_db();
    p_fea_ptp_global->blocked = TRUE;
    sal_mutex_unlock(g_hsrv_ptp_mutex);

    return HSRV_E_NONE;
}

int32
_hsrv_ptp_tx_ts_isr(uint8 gchip, void* p_data)
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    tbl_fea_ptp_ts_t *p_fea_ptp_ts = NULL;
    tbl_fea_ptp_ts_t fea_ptp_ts;
    uint16  seq_id;
    long	 gport;
    uint32 ifindex;
    int count;
    
    HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR gchip=%u,", gchip);

    sal_mutex_lock(g_hsrv_ptp_mutex);
    
    ctc_ptp_ts_cache_t* p_timestamp = (ctc_ptp_ts_cache_t*)p_data;

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR fea ptp global does not exist");
        return HSRV_E_FAIL;
    }

    p_fea_ptp_global->intr_count = p_timestamp->entry_count;
    for (count = 0; count < p_fea_ptp_global->intr_count; count++)
    {  
        sal_memset(&fea_ptp_ts, 0x00, sizeof(fea_ptp_ts));
        
        seq_id = p_timestamp->entry[count].seq_id;
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, p_timestamp->entry[count].u.lport);
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_ifindex_by_gportid(gport, &ifindex));

        HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR gport=%ld, seqid=%ld, lport=%d, ifindex=%d\n", gport, seq_id, p_timestamp->entry[count].u.lport, ifindex);
        HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR before g_p_hsrv_ptp->info.{ready=%s, tx_ifindex=%u, intr_ifindex=%u, ts=%hu%u:%u}",
                    _hsrv_ptp_desc_bool(p_fea_ptp_global->ready), p_fea_ptp_global->tx_ifindex, fea_ptp_ts.intr_ifindex, 
                    fea_ptp_ts.s_msb, fea_ptp_ts.s_lsb, fea_ptp_ts.ns);
    
        if (p_fea_ptp_global->ready)
        {
            HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR overlay timestamp %hu%u:%u of interface %u",
                    fea_ptp_ts.s_msb, fea_ptp_ts.s_lsb, fea_ptp_ts.ns, fea_ptp_ts.intr_ifindex);
        }

        /*fix bug 46447, agg not work ptp, by chenc start*/
        tbl_interface_t* p_db_if = NULL;
        p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
        if (NULL == p_db_if)
        {
            /*error*/
            return PM_E_FAIL;
        }
        
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            uint8 lag_id = 0;
            char ifname[IFNAME_SIZE] = {0};
            tbl_interface_t* p_db_if_agg = NULL;
            
            lag_id = p_db_if->lag->lag_id;
            sal_sprintf(ifname, "agg%u", lag_id);
            p_db_if_agg = tbl_interface_get_interface_by_name(ifname);
            if (NULL != p_db_if_agg)
            {
                ifindex = p_db_if_agg->ifindex;
            }
            else
            {
                /*error*/
                return PM_E_FAIL;
            }
        }
        /*fix bug 46447, agg not work ptp, by chenc end*/
        
        fea_ptp_ts.intr_ifindex = ifindex;
        //fea_ptp_ts.s_msb = 0;
        //fea_ptp_ts.s_lsb = (int32) p_timestamp->entry[count].ts.seconds;
        //fea_ptp_ts.ns = (int32) p_timestamp->entry[count].ts.nanoseconds;
        fea_ptp_ts.s_msb = 0;
        fea_ptp_ts.s_lsb = p_timestamp->entry[count].ts.seconds;
        fea_ptp_ts.ns = p_timestamp->entry[count].ts.nanoseconds;
        
        fea_ptp_ts.key.intr_seq_id = seq_id;

        HSRV_PTP_INTERRUPT_DEBUG("PTP TX TS ISR after g_p_hsrv_ptp->info.{ready=%s, tx_ifindex=%u, intr_ifindex=%u, ts=%hu%u:%u}",
                    _hsrv_ptp_desc_bool(p_fea_ptp_global->ready), p_fea_ptp_global->tx_ifindex, fea_ptp_ts.intr_ifindex,
                    fea_ptp_ts.s_msb, fea_ptp_ts.s_lsb, fea_ptp_ts.ns);

        p_fea_ptp_ts = tbl_fea_ptp_ts_get_fea_ptp_ts(&(fea_ptp_ts.key));
        if (NULL == p_fea_ptp_ts)
        {
            tbl_fea_ptp_ts_add_fea_ptp_ts_sync(&fea_ptp_ts, FALSE);
        }
        else
        {
            return PM_E_FAIL;
        }
        
        //HSRV_PTP_CFG_DEBUG("DEBUG_PTP_TX_TS lport=%d, gport=%ld, seqid=%ld, second=%ld, nanosecond=%ld, entrycout=%d",
        //    p_timestamp->entry[count].u.lport, time_stamp[count].gport, time_stamp[count].seqId, time_stamp[count].msb, time_stamp[count].lsb, p_timestamp->entry_count);
        //ASN_SET_ADD(&(ts_msg.opHagt2Srv.hagt2SrvMsgPTPGetTxTs.timeStampList.list),
        //                &time_stamp[count]);
        
    }
    p_fea_ptp_global->ready = TRUE;
    sal_mutex_unlock(g_hsrv_ptp_mutex);
    
    return PM_E_NONE;
}

static void
_hsrv_ptp_trigger_tx_ts(void* p_data)
{
    int32 ret = HSRV_E_NONE;
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;
    tbl_fea_ptp_ts_master_t * p_fea_ptp_ts_master = NULL;
    hal_ptp_get_tx_ts_resp_t resp;

    sal_memset(&resp, 0x00, sizeof(hal_ptp_get_tx_ts_resp_t));

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return ;
    }

    p_fea_ptp_global->hsrv_ptp_timer = NULL;

    p_fea_ptp_ts_master = tbl_fea_ptp_ts_get_master();
    if (NULL == p_fea_ptp_ts_master)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp ts master does not exist");
        return ;
    }
    
    HSRV_PTP_DEBUG_FUNC();

    sal_mutex_lock(g_hsrv_ptp_mutex);

    HSRV_PTP_CFG_DEBUG("Fea ptp global parameters {ready=%s, tx_ifindex=%u, tx_seq_id=%hu}",
        _hsrv_ptp_desc_bool(p_fea_ptp_global->ready), p_fea_ptp_global->tx_ifindex, p_fea_ptp_global->tx_seq_id);
    
    if (p_fea_ptp_global->blocked)
    {
        resp.blocked = TRUE;

        /*DEBUG_PTP_TX_TS*/
        HSRV_PTP_CFG_DEBUG("Send resp: cannot get TS for blocked\n");
        ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
        _hsrv_ptp_reset_db();
        sal_mutex_unlock(g_hsrv_ptp_mutex);
        HSRV_IF_ERROR_RETURN_NULL(ret);
        return ;
    }

#ifdef _GLB_UML_SYSTEM_
    ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
    if (0 != ret)
    {
        HSRV_PTP_CFG_DEBUG("Send resp failed");
    }
    _hsrv_ptp_reset_db();
    sal_mutex_unlock(g_hsrv_ptp_mutex);
    HSRV_IF_ERROR_RETURN_NULL(ret);
    return ;
#else

    #ifdef PTP_NO_USE_TS
        ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
        if (0 != ret)
        {
            HSRV_PTP_CFG_DEBUG("Send resp failed");
        }
        _hsrv_ptp_reset_db();
        sal_mutex_unlock(g_hsrv_ptp_mutex);
        HSRV_IF_ERROR_RETURN_NULL(ret);
        return ;
    #else
    
    tbl_fea_ptp_ts_t *p_fea_ptp_ts = NULL;
    ctclib_slistnode_t  *listnode = NULL;
    ctclib_slistnode_t  *next = NULL;
        
    if (p_fea_ptp_global->ready)
    {
        CTCLIB_SLIST_LOOP_DEL(p_fea_ptp_ts_master->fea_ptp_ts_list, p_fea_ptp_ts, listnode, next)
        {
            /*DEBUG_PTP_TX_TS*/
            HSRV_PTP_CFG_DEBUG("Need get TS for ifindex=%ld, seqid=%ld, list ifindex=%ld, seqid=%ld\n", 
                p_fea_ptp_global->tx_ifindex,p_fea_ptp_global->tx_seq_id, p_fea_ptp_ts->intr_ifindex,p_fea_ptp_ts->key.intr_seq_id);

            if (p_fea_ptp_global->tx_ifindex == p_fea_ptp_ts->intr_ifindex &&
                p_fea_ptp_global->tx_seq_id == p_fea_ptp_ts->key.intr_seq_id)
            {
                resp.ts.sec.msb = p_fea_ptp_ts->s_msb;
                resp.ts.sec.lsb = p_fea_ptp_ts->s_lsb;
                resp.ts.ns = p_fea_ptp_ts->ns;

                /*DEBUG_PTP_TX_TS*/
                HSRV_PTP_CFG_DEBUG("Send resp: TS sec.lsb = %d, ns = %d  \n", resp.ts.sec.lsb, resp.ts.ns); 

                ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
                _hsrv_ptp_reset_db();
                sal_mutex_unlock(g_hsrv_ptp_mutex);
                HSRV_IF_ERROR_RETURN_NULL(ret);
                return ;
            }
        }
         HSRV_PTP_LOG_WARN("Timestamp of interface %u is out of date.",
                p_fea_ptp_global->tx_ifindex);
              p_fea_ptp_global->ready = FALSE;
    }

    p_fea_ptp_global->retry_cnt++;
    /*DEBUG_PTP_TX_TS*/
    HSRV_PTP_CFG_DEBUG("Retry_cnt %hhu\n", p_fea_ptp_global->retry_cnt);
    
    if (p_fea_ptp_global->retry_cnt > HSRV_PTP_RETRY_TIMES)
    {
        resp.invalid = TRUE;

        /*DEBUG_PTP_TX_TS*/
        HSRV_PTP_CFG_DEBUG("Send resp: cannot get TS for timeout getting timestamp of interface %u.\n", p_fea_ptp_global->tx_ifindex);
        ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
        _hsrv_ptp_reset_db();
        sal_mutex_unlock(g_hsrv_ptp_mutex);
        HSRV_IF_ERROR_RETURN_NULL(ret);
        return ;
    }

    p_fea_ptp_global->hsrv_ptp_timer = hsrv_ptp_util_add_timer_ms(_hsrv_ptp_trigger_tx_ts, NULL, HSRV_PTP_RETRY_INTERVAL);
    if (NULL == p_fea_ptp_global->hsrv_ptp_timer)
    {
        HSRV_PTP_CFG_DEBUG("Start timer for hsrv ptp failed");
    }
    sal_mutex_unlock(g_hsrv_ptp_mutex);

    return ;
    #endif
#endif
}


static int32 
_hsrv_ptp_get_tx_ts(void* p_msg_data)
{
    tbl_fea_ptp_global_t *p_fea_ptp_global = NULL;

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        return PM_E_FAIL;
    }
    
    HSRV_PTP_DEBUG_FUNC();

    hal_ptp_get_tx_ts_req_t* p_req = p_msg_data;
    HSRV_PTP_PTR_CHECK(p_req);

    sal_mutex_lock(g_hsrv_ptp_mutex);

    p_fea_ptp_global->tx_ifindex = p_req->ifindex;
    p_fea_ptp_global->type = p_req->type;
    p_fea_ptp_global->tx_seq_id = p_req->seq_id;
    p_fea_ptp_global->retry_cnt = 0;

    HSRV_PTP_CFG_DEBUG("Fea ptp global parameters {ready=%s, tx_ifindex=%u, tx_seq_id=%hu}",
            _hsrv_ptp_desc_bool(p_fea_ptp_global->ready), p_fea_ptp_global->tx_ifindex, p_fea_ptp_global->tx_seq_id);

    if(1)
    {
        hsrv_ptp_util_cancel_timer(&p_fea_ptp_global->hsrv_ptp_timer);
        p_fea_ptp_global->hsrv_ptp_timer = hsrv_ptp_util_add_timer_ms(_hsrv_ptp_trigger_tx_ts, NULL, HSRV_PTP_RETRY_INTERVAL);
        if (NULL == p_fea_ptp_global->hsrv_ptp_timer)
        {
            HSRV_PTP_CFG_DEBUG("Start timer for hsrv ptp failed");
        }
    }
    else
    {
        int ret = 0;
        hal_ptp_get_tx_ts_resp_t resp;
        sal_memset(&resp, 0x00, sizeof(hal_ptp_get_tx_ts_resp_t));

        ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_TX_TS, &resp, sizeof(resp));
        if (0 != ret)
        {
            HSRV_PTP_CFG_DEBUG("Send resp: get TX TS failed");
        }
        _hsrv_ptp_reset_db();
        sal_mutex_unlock(g_hsrv_ptp_mutex);
        HSRV_IF_ERROR_RETURN(ret);
        return PM_E_NONE;
    }

    sal_mutex_unlock(g_hsrv_ptp_mutex);

    return PM_E_NONE;
}


static int32
_hsrv_ptp_reset_tx_ts()
{
    int32 ret = HSRV_E_NONE;
    int32 resp = 0;
    tbl_fea_ptp_global_t * p_fea_ptp_global = NULL;
    
    HSRV_PTP_DEBUG_FUNC();

    p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();
    if (NULL == p_fea_ptp_global)
    {
        HSRV_PTP_CFG_DEBUG("Fea ptp global does not exist");
        resp = FALSE;
        ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_RESET_TX_TS, &resp, sizeof(resp));
        HSRV_IF_ERROR_RETURN(ret);
        
        return HSRV_E_NONE;
    }

    sal_mutex_lock(g_hsrv_ptp_mutex);

    hsrv_ptp_util_cancel_timer(&p_fea_ptp_global->hsrv_ptp_timer);
    _hsrv_ptp_reset_db();
    p_fea_ptp_global->retry_cnt = 0;

    resp = TRUE;
    ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_RESET_TX_TS, &resp, sizeof(resp));
    HSRV_IF_ERROR_RETURN(ret);

    sal_mutex_unlock(g_hsrv_ptp_mutex);
    
    return HSRV_E_NONE;
}


static int32
_hsrv_ptp_get_hardware_second()
{
    tbl_ptp_global_t *g_p_master = NULL;
    sai_ptp_api_t       *ptp_api = NULL;
    sai_attribute_t     attr = {0};
    uint32          attr_count = 0;
    int32 ret = HSRV_E_NONE;

    uint32_t  list_value[2]; 

    HSRV_PTP_DEBUG_FUNC();
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PTP,(void**)&ptp_api));

    g_p_master = tbl_ptp_global_get_ptp_global();
    if (g_p_master == NULL)
    {
        HSRV_PTP_CFG_DEBUG("Ptp global does not exist");
        return PM_E_FAIL;
    }
    
    attr.id = SAI_PTP_ATTR_HARDWARE_SECOND;
    attr.value.u32list.list = list_value;
            
    attr_count++;

    HSRV_IF_ERROR_RETURN(ptp_api->get_ptp_attribute(attr_count, &attr));
    HSRV_PTP_CFG_DEBUG("Current TAI time %u:%u", attr.value.s32list.list[0], attr.value.s32list.list[1]);
    hal_ptp_get_hardware_second_resp_t resp;
    sal_memset(&resp, 0x00, sizeof(hal_ptp_get_hardware_second_resp_t));
    resp.second = (uint64)attr.value.s32list.list[0];
    resp.nanoseconds = (uint64)attr.value.s32list.list[1];
    ret = HSRV2PTP_MSG_SEND_RESP(FEI_PTP_GET_HW_SECOND, &resp, sizeof(resp));
    HSRV_IF_ERROR_RETURN(ret);
    
    return HSRV_E_NONE;
}

static int32
_hsrv_ptp_global_cmd_get_fea_ptp_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_fea_ptp_global_t* p_fea_ptp_glb = NULL;
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_fea_ptp_glb = tbl_fea_ptp_global_get_fea_ptp_global();
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_fea_ptp_global_dump_one(p_fea_ptp_glb, &args);
    sal_fclose(fp);

    return HSRV_E_NONE;
}

int32
hsrv_ptp_cmd_process_fea_ptp_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_GLOBAL);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        rc = _hsrv_ptp_global_cmd_get_fea_ptp_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return HSRV_E_NONE;
}


static int32
_hsrv_ptp_ts_cmd_get_fea_ptp_ts(tbl_fea_ptp_ts_t* p_db_fea_ptp_ts, cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_fea_ptp_ts_master_t *p_master = tbl_fea_ptp_ts_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    
    if (NULL == p_db_fea_ptp_ts)
    {
        CTCLIB_SLIST_LOOP(p_master->fea_ptp_ts_list, p_db_fea_ptp_ts, listnode)
        {
            tbl_fea_ptp_ts_dump_one(p_db_fea_ptp_ts, &args);
        }
    }
    else
    {
        tbl_fea_ptp_ts_dump_one(p_db_fea_ptp_ts, &args);
    }
    
    sal_fclose(fp);
    
    return HSRV_E_NONE;
}


int32
hsrv_ptp_ts_cmd_process_fea_ptp_ts(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_fea_ptp_ts_t* p_db_fea_ptp_ts = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = HSRV_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_PTP_PORT);

    switch (para->oper)
    {
    case CDB_OPER_SET:
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        
        rc = _hsrv_ptp_ts_cmd_get_fea_ptp_ts(p_db_fea_ptp_ts, para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
fea_ptp_global_init()
{   
    tbl_fea_ptp_global_t *p_fea_ptp_global = tbl_fea_ptp_global_get_fea_ptp_global();

    p_fea_ptp_global->hsrv_ptp_timer = NULL;

    return PM_E_NONE;
}

int32
hsrv_ptp_register_cb(void)
{
    hsrv_message_set_cb_from_fei(FEI_PTP_GET_TX_TS, _hsrv_ptp_get_tx_ts);
    hsrv_message_set_cb_from_fei(FEI_PTP_RESET_TX_TS, _hsrv_ptp_reset_tx_ts);
    hsrv_message_set_cb_from_fei(FEI_PTP_GET_HW_SECOND, _hsrv_ptp_get_hardware_second);

    return HSRV_E_NONE;
}

int32
hsrv_ptp_start()
{
    sal_mutex_create(&g_hsrv_ptp_mutex);

    cdb_register_cfg_tbl_cb(TBL_FEA_PTP_GLOBAL, hsrv_ptp_cmd_process_fea_ptp_global);
    cdb_register_cfg_tbl_cb(TBL_FEA_PTP_TS, hsrv_ptp_ts_cmd_process_fea_ptp_ts);
    
    cdb_pm_subscribe_tbl(TBL_PTP_GLOBAL, NULL, hsrv_ptp_global_sync_after);
    cdb_pm_subscribe_tbl(TBL_PTP_PORT, NULL, hsrv_ptp_port_sync_after);
    
    HSRV_IF_ERROR_RETURN(hsrv_ptp_register_cb());

    fea_ptp_global_init();
    
    return HSRV_E_NONE;
}

