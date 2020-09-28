#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_fdb.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_copp.h>
#include <ctc_sai_debug.h>

#undef  __MODULE__
#define __MODULE__ SAI_API_FDB

extern ctc_sai_copp_info_t g_sai_copp_master;

typedef struct ctc_sai_fdb_lookup_result_s
{
    sai_fdb_entry_t             *key;
    ctc_sai_fdb_entry_t         *pfdb_entry;
    uint32_t                    result_valid;
}ctc_sai_fdb_lookup_result_t;

typedef struct ctc_sai_flush_type_s
{
    bool                port_enable;
    bool                vlan_enable;
    sai_object_id_t     port;
    sai_vlan_id_t       vlan_id;
}ctc_sai_flush_type_t;

typedef sai_status_t (*ctc_sai_fdb_entry_traverse_cb)(ctc_sai_fdb_entry_t *,void *);

static ctc_sai_check_u32_range_t g_range_0_1 =
{
    .min = SAI_FDB_ENTRY_DYNAMIC,
    .max = SAI_FDB_ENTRY_MACFILTER,
};

static ctc_sai_check_object_type_range_t g_range_objtype_port_lag =
{
    .min = SAI_OBJECT_TYPE_PORT,
    .max = SAI_OBJECT_TYPE_LAG,
};

static ctc_sai_attr_entry_info_t g_sai_attr_entries[] = {
    {
        .id     = SAI_FDB_ENTRY_ATTR_TYPE,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_range_0_1,
        }
    },
    {
        .id     = SAI_FDB_ENTRY_ATTR_PORT_ID,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_object_type_range_fn,
            .func_parameter = &g_range_objtype_port_lag,
        }
    },
    {
        .id     = SAI_FDB_ENTRY_ATTR_PACKET_ACTION,
        .type   = SAI_ATTR_FALG_CREATE | SAI_ATTR_FALG_WRITE | SAI_ATTR_FALG_READ,
        .check_fn = {
            .func_fn = ctc_sai_check_i32_range_fn,
            .func_parameter = &g_packet_action_range,
        }
    },
    {
        .id     = SAI_FDB_ENTRY_ATTR_META_DATA,
        .type   = SAI_ATTR_FLAG_UNKONW,
    },
    {
        .id     = SAI_ATTR_ID_END,
        .type   = 0,
    },
};

static int32
_ctc_sai_fdb_event_notify(sai_fdb_event_notification_data_t *p_event, uint32_t count)
{
    ctc_sai_sai_sys_info_t *p_sys_info = ctc_sai_get_sys_info();
    p_sys_info->sai_switch_notification_table.on_fdb_event(count, p_event);
    return SAI_STATUS_SUCCESS;
    
}

static int32
_ctc_sai_fdb_learning_event_cb(uint8 gchip, void* p_data)
{
    sai_fdb_event_notification_data_t event_array[CTC_LEARNING_CACHE_MAX_INDEX];
    sai_attribute_t attr_array[CTC_LEARNING_CACHE_MAX_INDEX];
    ctc_learning_cache_t* p_cache = (ctc_learning_cache_t*)p_data;
    uint32_t i = 0;
    uint32_t tid = 0;
    uint32_t gport = 0;

    for (i = 0; i < p_cache->entry_num; i++)
    {
        /* pizzabox */
        event_array[i].event_type               = SAI_FDB_EVENT_LEARNED;
        event_array[i].fdb_entry.vlan_id        = p_cache->learning_entry[i].fid;
        event_array[i].fdb_entry.mac_address[0] = (p_cache->learning_entry[i].mac_sa_32to47 & 0xFF00) >> 8;
        event_array[i].fdb_entry.mac_address[1] = (p_cache->learning_entry[i].mac_sa_32to47 & 0xFF);
        event_array[i].fdb_entry.mac_address[2] = (p_cache->learning_entry[i].mac_sa_0to31 & 0xFF000000) >> 24;
        event_array[i].fdb_entry.mac_address[3] = (p_cache->learning_entry[i].mac_sa_0to31 & 0xFF0000) >> 16;
        event_array[i].fdb_entry.mac_address[4] = (p_cache->learning_entry[i].mac_sa_0to31 & 0xFF00) >> 8;
        event_array[i].fdb_entry.mac_address[5] = (p_cache->learning_entry[i].mac_sa_0to31 & 0xFF);

        gport = p_cache->learning_entry[i].global_src_port;
        event_array[i].attr_count = 1;
        event_array[i].attr = &attr_array[i];
        event_array[i].attr->id = SAI_FDB_ENTRY_ATTR_PORT_ID;
        if (CTC_IS_LINKAGG_PORT(gport))
        {    
             tid = CTC_GPORT_LINKAGG_ID(gport);
             event_array[i].attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
        }
        else
        {
             event_array[i].attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, gport);
        }
    }

    _ctc_sai_fdb_event_notify(event_array, i);

    return SAI_STATUS_SUCCESS;
}

static int32
_ctc_sai_fdb_aging_event_convert_and_notify(ctc_l2_addr_t *p_addr, uint32_t count)
{
    sai_fdb_event_notification_data_t event_array[CTC_LEARNING_CACHE_MAX_INDEX];
//    sai_attribute_t attr_array[CTC_LEARNING_CACHE_MAX_INDEX];
//    uint32_t tid = 0;
    uint32_t i = 0;
    
    for (i = 0; i < count; i++)
    {
        /* pizzabox */
        event_array[i].event_type               = SAI_FDB_EVENT_AGED;
        event_array[i].fdb_entry.vlan_id        = p_addr[i].fid;
        sal_memcpy(event_array[i].fdb_entry.mac_address, p_addr[i].mac, 6);

#if 0
        event_array[i].attr_count = 1;
        event_array[i].attr = &attr_array[i];
        event_array[i].attr->id = SAI_FDB_ENTRY_ATTR_PORT_ID;
        if (CTC_IS_LINKAGG_PORT(p_addr[i].gport))
        {    
             tid = CTC_GPORT_LINKAGG_ID(p_addr[i].gport);
             event_array[i].attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_LAG, tid);
        }
        else
        {
             event_array[i].attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_addr[i].gport);
        }
#endif
    }

    _ctc_sai_fdb_event_notify(event_array, count);

    return SAI_STATUS_SUCCESS;
}

static int32
_ctc_sai_fdb_aging_event_cb(uint8 gchip, void* p_data)
{
    ctc_l2_addr_t ctc_l2_addr_array[CTC_LEARNING_CACHE_MAX_INDEX];
    ctc_aging_fifo_info_t* p_fifo = (ctc_aging_fifo_info_t*)p_data;
    uint32_t i = 0;
    uint32_t entry_num = 0;
#ifdef GREATBELT
    uint32_t entry_index = 0;
    int32_t rc = SAI_STATUS_SUCCESS;
#endif

#ifdef GREATBELT
    for (i = 0; i < p_fifo->fifo_idx_num; i++)
    {
        entry_index = p_fifo->aging_index_array[i];
        rc = ctc_l2_get_fdb_by_index(entry_index, &ctc_l2_addr_array[entry_num]);
        if (CTC_E_ENTRY_NOT_EXIST == rc)
        {
            /*it might be occured when ifdown before isr exec*/
            continue;
        }

        entry_num++;
        if (entry_num >= CTC_LEARNING_CACHE_MAX_INDEX)
        {
            _ctc_sai_fdb_aging_event_convert_and_notify(ctc_l2_addr_array, entry_num);
            entry_num = 0;
        }
    }

    if (entry_num)
    {
        _ctc_sai_fdb_aging_event_convert_and_notify(ctc_l2_addr_array, entry_num);
    }
#else
    for (i = 0; i < p_fifo->fifo_idx_num; i++)
    {
        
        sal_memcpy(ctc_l2_addr_array[entry_num].mac, p_fifo->aging_entry[i].mac, sizeof(mac_addr_t));
        ctc_l2_addr_array[entry_num].fid = p_fifo->aging_entry[i].fid;

        entry_num++;
        if (entry_num >= CTC_LEARNING_CACHE_MAX_INDEX)
        {
            _ctc_sai_fdb_aging_event_convert_and_notify(ctc_l2_addr_array, entry_num);
            entry_num = 0;
        }
    }

    if (entry_num)
    {
        _ctc_sai_fdb_aging_event_convert_and_notify(ctc_l2_addr_array, entry_num);
    }
#endif

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_FDB_SAI_INNER_API_FUNC
sai_status_t
ctc_fdb_info_init()
{
    /* future_realize : interrupt api problem */
    ctc_interrupt_register_event_cb(CTC_EVENT_L2_SW_LEARNING, _ctc_sai_fdb_learning_event_cb);
    ctc_interrupt_register_event_cb(CTC_EVENT_L2_SW_AGING, _ctc_sai_fdb_aging_event_cb);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_fdb_fdb_entry_traverse_by_type(
    _In_  const ctc_sai_flush_type_t    *pflush_type,
    _In_  ctc_sai_fdb_entry_traverse_cb fdb_cb,
    _In_  void                          *pdata)
{
#define FDB_QUERY_COUNT     10
    sai_status_t            ret     = SAI_STATUS_SUCCESS;
    ctc_l2_fdb_query_rst_t  query_rst;
    ctc_l2_fdb_query_t      Query;
    uint32_t                index   = 0;
    ctc_sai_fdb_entry_t     fdb_entry;
    ctc_l2_addr_t           addr_buf[FDB_QUERY_COUNT];
    
    CTC_SAI_PTR_VALID_CHECK(pflush_type);

    sal_memset(&query_rst,  0, sizeof(ctc_l2_fdb_query_rst_t));
    sal_memset(&Query,      0, sizeof(ctc_l2_fdb_query_t));
    sal_memset(&fdb_entry,  0, sizeof(ctc_sai_fdb_entry_t));


    if(pflush_type->port_enable && pflush_type->vlan_enable)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
        Query.query_flag = CTC_L2_FDB_ENTRY_ALL;
        Query.gport      = CTC_SAI_OBJECT_INDEX_GET(pflush_type->port);
        Query.fid        = pflush_type->vlan_id;
    }
    else if(pflush_type->port_enable)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
        Query.query_flag = CTC_L2_FDB_ENTRY_ALL;
        Query.gport      = CTC_SAI_OBJECT_INDEX_GET(pflush_type->port);
    }
    else if(pflush_type->vlan_enable)
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_BY_VID;
        Query.query_flag = CTC_L2_FDB_ENTRY_ALL;
        Query.fid        = pflush_type->vlan_id;
    }else
    {
        Query.query_type = CTC_L2_FDB_ENTRY_OP_ALL;
        Query.query_flag = CTC_L2_FDB_ENTRY_ALL;
    }

    query_rst.buffer_len = sizeof(ctc_l2_addr_t) * 10;
    query_rst.buffer = addr_buf;

    if (NULL == query_rst.buffer)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    sal_memset(query_rst.buffer, 0, query_rst.buffer_len);

    do
    {
        query_rst.start_index = query_rst.next_query_index;

        if (ctc_l2_get_fdb_entry(&Query, &query_rst) < 0)
        {
            query_rst.buffer = NULL;
            return SAI_STATUS_FAILURE;
        }

        for (index = 0; index < Query.count; index++)
        {
            sal_memcpy(&fdb_entry.fdb_key.mac_address,
                query_rst.buffer[index].mac,
                sizeof(sai_mac_t));

            fdb_entry.fdb_key.vlan_id = query_rst.buffer[index].fid;

            if(query_rst.buffer[index].flag & CTC_L2_FLAG_IS_STATIC)
            {
                fdb_entry.type = SAI_FDB_ENTRY_STATIC;
            }

            if(query_rst.buffer[index].flag & CTC_L2_FLAG_DISCARD)
            {
                fdb_entry.action = SAI_PACKET_ACTION_DROP;
            }
            else if((CTC_L2_FLAG_DISCARD | CTC_L2_FLAG_COPY_TO_CPU)
                    == (query_rst.buffer[index].flag
                        & (CTC_L2_FLAG_DISCARD | CTC_L2_FLAG_COPY_TO_CPU)))
            {
                fdb_entry.action = SAI_PACKET_ACTION_TRAP;
            }
            else if(query_rst.buffer[index].flag & CTC_L2_FLAG_COPY_TO_CPU)
            {
                fdb_entry.action = SAI_PACKET_ACTION_LOG;
            }
            else
            {
                fdb_entry.action = SAI_PACKET_ACTION_FORWARD;
            }

            ret = fdb_cb(&fdb_entry,pdata);
            if(SAI_STATUS_SUCCESS != ret)
            {
                query_rst.buffer = NULL;
                return ret;
            }
        }
    }while (query_rst.is_end == 0);

    query_rst.buffer = NULL;
    return SAI_STATUS_SUCCESS;
}


sai_status_t
__ctc_fdb_create_entry(
    _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    int32_t             sdk_ret     = 0;
    uint32              pgport = 0;

    sal_memset(&l2_addr,0,sizeof(l2_addr));

    if (pfdb_entry->type == SAI_FDB_ENTRY_STATIC_SECURITY)
    {
        l2_addr.flag = CTC_L2_FLAG_IS_STATIC | CTC_L2_FLAG_BIND_PORT;
    }
    else if (pfdb_entry->type == SAI_FDB_ENTRY_STATIC)
    {
        l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    }
    else if (pfdb_entry->type == SAI_FDB_ENTRY_DYNAMIC_WO_AGING)
    {
        l2_addr.flag = CTC_L2_FLAG_AGING_DISABLE;
    }
    else if (pfdb_entry->type == SAI_FDB_ENTRY_DYNAMIC_SECURITY)
    {
        l2_addr.flag = CTC_L2_FLAG_BIND_PORT;
    }

    switch(pfdb_entry->action)
    {
    case SAI_PACKET_ACTION_DROP:
        l2_addr.flag |= CTC_L2_FLAG_DISCARD;
        break;
    case SAI_PACKET_ACTION_FORWARD:

        break;
    case SAI_PACKET_ACTION_TRAP:
        l2_addr.flag |= CTC_L2_FLAG_DISCARD;
        l2_addr.flag |= CTC_L2_FLAG_COPY_TO_CPU;
        break;
    case SAI_PACKET_ACTION_LOG:
        l2_addr.flag |= CTC_L2_FLAG_COPY_TO_CPU;
        break;
    default:
        break;
    }

    sal_memcpy(l2_addr.mac, pfdb_entry->fdb_key.mac_address, sizeof(pfdb_entry->fdb_key.mac_address));

    l2_addr.fid  =  pfdb_entry->fdb_key.vlan_id; 
    ctc_sai_port_objectid_to_gport(pfdb_entry->port, &pgport);
    l2_addr.gport = pgport;
    sdk_ret = ctc_l2_add_fdb(&l2_addr);
    if (sdk_ret <0)
    {
        goto error;
    }

error:
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_creat_macfilter_fdb(
   _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    int32_t             sdk_ret     = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    l2_addr.flag = CTC_L2_FLAG_IS_STATIC | CTC_L2_FLAG_DISCARD | CTC_L2_FLAG_SRC_DISCARD;
    l2_addr.fid  = 0xFFFF; 
    sal_memcpy(l2_addr.mac, pfdb_entry->fdb_key.mac_address, sizeof(pfdb_entry->fdb_key.mac_address));

    sdk_ret = ctc_l2_add_fdb(&l2_addr);    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_creat_macfilter_fdb_with_nexthop(
    _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    uint32              nexthopid = 0;
    int32_t             sdk_ret = 0;
    
    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    if (SAI_FDB_L2PROTOCOL_FLAG_GROUP_MAC == pfdb_entry->flag)
    {
        nexthopid = g_sai_copp_master.ctc_copp_port_info[SAI_COPP_L2PRO_GROUP_MAC_PORT_ID].nexthop_id;
    }
    else if (SAI_FDB_L2PROTOCOL_FLAG_PROTOCOL_MAC == pfdb_entry->flag)
    {
        nexthopid = g_sai_copp_master.ctc_copp_port_info[SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID].nexthop_id;
    }
    else
    {
        return ctc_sai_get_error_from_sdk_error(sdk_ret);    
    }

    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.fid  = 0xFFFF; 
    sal_memcpy(l2_addr.mac, pfdb_entry->fdb_key.mac_address, sizeof(pfdb_entry->fdb_key.mac_address));
    sdk_ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, nexthopid);    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_fdb_create_entry(
    _In_  ctc_sai_fdb_entry_t* pfdb_entry)
{
    sai_status_t ret    = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(__ctc_fdb_create_entry(pfdb_entry),ret,out);

    /* static update neighbor, dynamic uses ARP packet FDB learning */
    if (SAI_FDB_ENTRY_STATIC == pfdb_entry->type 
        || SAI_FDB_ENTRY_DYNAMIC == pfdb_entry->type
        || SAI_FDB_ENTRY_DYNAMIC_WO_AGING == pfdb_entry->type
        || SAI_FDB_ENTRY_DYNAMIC_SECURITY == pfdb_entry->type)
    {
        ctc_sai_fdbevnet_notifier_call_notifiers(SAI_FDB_EVENT_LEARNED, pfdb_entry);
    }

out:
    return ret;
}

sai_status_t
__ctc_fdb_remove_entry(
    _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    int32_t             sdk_ret     = 0;

    sal_memset(&l2_addr,0,sizeof(l2_addr));

    if (pfdb_entry->type == SAI_FDB_ENTRY_STATIC)
    {
        l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    }
    sal_memcpy(l2_addr.mac,pfdb_entry->fdb_key.mac_address,sizeof(pfdb_entry->fdb_key.mac_address));

    l2_addr.fid = pfdb_entry->fdb_key.vlan_id;
    sdk_ret = ctc_l2_remove_fdb(&l2_addr);
    if (sdk_ret<0)
    {
        goto error;
    }
    
error:
    /* when MLAG loop, maybe remove non-existed FDB entry, not print error log */
    if (CTC_E_NOT_EXIST == sdk_ret)
    {
        sdk_ret = CTC_E_NONE;
    }
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_remove_macfilter_fdb(
    _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    int32_t             sdk_ret = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    l2_addr.flag = CTC_L2_FLAG_IS_STATIC | CTC_L2_FLAG_DISCARD | CTC_L2_FLAG_SRC_DISCARD;
    l2_addr.fid = 0xFFFF;
    sal_memcpy(l2_addr.mac, pfdb_entry->fdb_key.mac_address, sizeof(pfdb_entry->fdb_key.mac_address));
    
    sdk_ret = ctc_l2_remove_fdb(&l2_addr);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_remove_l2protocol_fdb(
    _In_  const ctc_sai_fdb_entry_t* pfdb_entry)
{
    ctc_l2_addr_t       l2_addr;
    int32_t             sdk_ret = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    l2_addr.fid = 0xFFFF;
    sal_memcpy(l2_addr.mac, pfdb_entry->fdb_key.mac_address, sizeof(pfdb_entry->fdb_key.mac_address));
    
    sdk_ret = ctc_l2_remove_fdb(&l2_addr);
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}

sai_status_t
ctc_fdb_flush_entry(
    _In_  ctc_sai_fdb_entry_t* pfdb_entry)
{
    sai_status_t ret    = SAI_STATUS_SUCCESS;

    if (SAI_FDB_ENTRY_DYNAMIC == pfdb_entry->type
        || SAI_FDB_ENTRY_DYNAMIC_SECURITY == pfdb_entry->type)
    {
        ctc_sai_fdbevnet_notifier_call_notifiers(SAI_FDB_EVENT_FLUSHED, pfdb_entry);
    }

    return ret;
}

sai_status_t
ctc_fdb_remove_fdb_entry(
    _In_  ctc_sai_fdb_entry_t* pfdb_entry)
{
    sai_status_t ret    = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(__ctc_fdb_remove_entry(pfdb_entry),ret,out);

    /* static update neighbor when remove fdb entry */
    if (SAI_FDB_ENTRY_STATIC == pfdb_entry->type 
        || SAI_FDB_ENTRY_DYNAMIC == pfdb_entry->type
        || SAI_FDB_ENTRY_DYNAMIC_WO_AGING == pfdb_entry->type
        || SAI_FDB_ENTRY_DYNAMIC_SECURITY == pfdb_entry->type)
    {
        ctc_sai_fdbevnet_notifier_call_notifiers(SAI_FDB_EVENT_AGED, pfdb_entry);
    }

out:
    return ret;
}

sai_status_t
__ctc_fdb_lookup(
    _In_        ctc_sai_fdb_entry_t *pfdb_entry,
    _Inout_     void                *pdata)
{
    ctc_sai_fdb_lookup_result_t     *presult = NULL;

    presult = pdata;

    if(!sal_memcmp(presult->key,&pfdb_entry->fdb_key,sizeof(sai_fdb_entry_t)))
    {
        sal_memcpy(presult->pfdb_entry,pfdb_entry,sizeof(ctc_sai_fdb_entry_t));
        presult->result_valid = 1;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_fdb_lookup_entry(
    _In_  const sai_fdb_entry_t *key,
    _Out_  ctc_sai_fdb_entry_t  *pfdb_entry)
{
    ctc_sai_flush_type_t        flush_type;
    ctc_sai_fdb_lookup_result_t fdb_result;
    sai_fdb_entry_t             key_tmp;

    sal_memset(&flush_type,0,sizeof(flush_type));
    sal_memset(&fdb_result,0,sizeof(ctc_sai_fdb_lookup_result_t));

    sal_memcpy(&key_tmp,key,sizeof(sai_fdb_entry_t));

    flush_type.port_enable  = false;
    flush_type.vlan_enable  = false;
    fdb_result.key          = &key_tmp;
    fdb_result.pfdb_entry   = pfdb_entry;

    ctc_fdb_fdb_entry_traverse_by_type(
        &flush_type,
        __ctc_fdb_lookup,
        &fdb_result);

    if(fdb_result.result_valid)
    {
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t
    _In_  ctc_fdb_set_entry_type(
    _In_  ctc_sai_fdb_entry_t   *psai_fdb_info,
    _In_  const sai_attribute_t *pfdb_entry)
{
    psai_fdb_info->type = pfdb_entry->value.s32;

    return __ctc_fdb_create_entry(psai_fdb_info);
}

sai_status_t
ctc_fdb_set_entry_port_id(
    _In_  ctc_sai_fdb_entry_t   *psai_fdb_info,
    _In_  const sai_attribute_t *pfdb_entry)
{
    psai_fdb_info->port = pfdb_entry->value.oid;

    return __ctc_fdb_create_entry(psai_fdb_info);
}

sai_status_t
ctc_fdb_set_entry_packet_action(
    _In_  ctc_sai_fdb_entry_t   *psai_fdb_info,
    _In_  const sai_attribute_t *pfdb_entry)
{
    psai_fdb_info->action= pfdb_entry->value.s32;

    return __ctc_fdb_create_entry(psai_fdb_info);
}

sai_status_t
ctc_fdb_get_entry_type(
    _In_  const ctc_sai_fdb_entry_t *psai_fdb_info,
    _Inout_ sai_attribute_t         *pfdb_entry)
{
    pfdb_entry->value.s32 = psai_fdb_info->type;

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_fdb_get_entry_port_id(
    _In_  const ctc_sai_fdb_entry_t *psai_fdb_info,
    _Inout_ sai_attribute_t         *pfdb_entry)
{
    pfdb_entry->value.oid = psai_fdb_info->port;

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_fdb_get_entry_packet_action(
    _In_  const ctc_sai_fdb_entry_t *psai_fdb_info,
    _Inout_ sai_attribute_t         *pfdb_entry)
{
    pfdb_entry->value.s32 = psai_fdb_info->action;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
__ctc_fdb_entry_event_with_type(
    _In_ ctc_sai_fdb_entry_t    *pfdb_entry,
    _In_ sai_fdb_event_t           event_type)
{
    sai_fdb_event_notification_data_t data;
#if 0
    sai_attribute_t attr[3] = {};

    attr[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr[0].value.s32 = pfdb_entry->type;
    attr[1].id = SAI_FDB_ENTRY_ATTR_PORT_ID;
    attr[1].value.oid = pfdb_entry->port;
    attr[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr[2].value.s32 = pfdb_entry->action;
#endif
    if(ctc_sai_get_sys_info()->sai_switch_notification_table.on_fdb_event)
    {
        ctc_sai_get_sys_info()->sai_switch_notification_table.on_fdb_event(
            1, &data);
    }

    return SAI_STATUS_SUCCESS;
}

int32
ctc_fdb_get_by_port_and_vlanid(uint32 vlanid, uint32 gport)
{
    ctc_l2_addr_t           query_buffer[10];
    ctc_l2_fdb_query_t      fdb_query;
    ctc_l2_fdb_query_rst_t  query_rst;
    int                     ret = 0;

    sal_memset(&fdb_query, 0, sizeof(fdb_query));
    fdb_query.gport = gport;
    fdb_query.query_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
    //fdb_query.query_flag = CTC_L2_FDB_ENTRY_STATIC;
    fdb_query.query_flag = CTC_L2_FDB_ENTRY_ALL;
    fdb_query.fid        = vlanid;
    fdb_query.query_hw   = FALSE;

    query_rst.buffer_len = sizeof(ctc_l2_addr_t) * 10;
    query_rst.buffer = query_buffer;
    sal_memset(query_rst.buffer, 0, query_rst.buffer_len);

    ret = ctc_l2_get_fdb_entry(&fdb_query, &query_rst);
    if ((0 == ret) && (fdb_query.count > 0))
    {
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_FAILURE;
}

#define ________SAI_FDB_SAI_API_DEBUG_FUNC
sai_status_t
ctc_sai_set_fdb_entry_attribute_debug_param(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ const sai_attribute_t *attr)
{

    switch(attr->id)
    {
    case SAI_FDB_ENTRY_ATTR_TYPE:
        CTC_SAI_DEBUG("in:SAI_FDB_ENTRY_ATTR_TYPE %d", attr->value.s32);
        break;
    case SAI_FDB_ENTRY_ATTR_PORT_ID:
        CTC_SAI_DEBUG("in:SAI_FDB_ENTRY_ATTR_PORT_ID 0x%llx", attr->value.oid);
        break;
    case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
        CTC_SAI_DEBUG("in:SAI_FDB_ENTRY_ATTR_PACKET_ACTION %d", attr->value.s32);
        break;
    default:
        break;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_get_fdb_entry_attribute_debug_param(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_attribute_t *pattr_entry = NULL;
    uint32_t                    attr_idx = 0;

    CTC_SAI_DEBUG("in:vlan_id = %u, mac_address = %x:%x:%x:%x:%x:%x", 
        fdb_entry->vlan_id, 
        fdb_entry->mac_address[5], fdb_entry->mac_address[4], fdb_entry->mac_address[3],
        fdb_entry->mac_address[2], fdb_entry->mac_address[1], fdb_entry->mac_address[0]);

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = &attr_list[attr_idx];
        switch (pattr_entry->id)
        {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            CTC_SAI_DEBUG("out:SAI_FDB_ENTRY_ATTR_TYPE %d", pattr_entry->value.s32);
            break;
        case SAI_FDB_ENTRY_ATTR_PORT_ID:
            CTC_SAI_DEBUG("out:SAI_FDB_ENTRY_ATTR_PORT_ID 0x%llx", pattr_entry->value.oid);
            break;
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            CTC_SAI_DEBUG("out:SAI_FDB_ENTRY_ATTR_PACKET_ACTION %d", pattr_entry->value.s32);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_flush_fdb_entries_debug_param(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    const sai_attribute_t *pattr_entry = NULL;
    uint32_t                    attr_idx = 0;

    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        pattr_entry = &attr_list[attr_idx];
        switch (pattr_entry->id)
        {
        case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
            switch (pattr_entry->value.s32)
            {
                case SAI_FDB_FLUSH_ENTRY_DYNAMIC:
                    CTC_SAI_DEBUG("in:SAI_FDB_FLUSH_ENTRY_DYNAMIC");
                    break;
                case SAI_FDB_FLUSH_ENTRY_STATIC:
                    CTC_SAI_DEBUG("in:SAI_FDB_FLUSH_ENTRY_STATIC");
                    break;
                 default:
                    CTC_SAI_DEBUG("in:NOT_SUPPORTED");
                    break;
            }
            break;
        case SAI_FDB_FLUSH_ATTR_PORT_ID:
            CTC_SAI_DEBUG("in:SAI_FDB_FLUSH_ATTR_PORT_ID 0x%llx", pattr_entry->value.oid);
            break;
        case SAI_FDB_FLUSH_ATTR_VLAN_ID:
            CTC_SAI_DEBUG("in:SAI_FDB_FLUSH_ATTR_VLAN_ID %u", pattr_entry->value.u16);
            break;
        case SAI_FDB_FLUSH_ATTR_MAC:
            CTC_SAI_DEBUG("in:SAI_FDB_FLUSH_ATTR_MAC %02X:%02X:%02X:%02X:%02X:%02X", 
                pattr_entry->value.mac[0], pattr_entry->value.mac[1], pattr_entry->value.mac[2],
                pattr_entry->value.mac[3], pattr_entry->value.mac[4], pattr_entry->value.mac[5]);
            break;
         default:
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

#define ________SAI_FDB_SAI_API_FUNC
/*
* Routine Description:
*    Create FDB entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_fdb_entry(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   *pattr_entry_list = NULL;
    ctc_sai_fdb_entry_t         new_fdb_entry;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&new_fdb_entry,0,sizeof(new_fdb_entry));

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);
#if 0
    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);
#endif
    new_fdb_entry.type =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_TYPE].value.s32;
    new_fdb_entry.action  =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_PACKET_ACTION].value.s32;
    new_fdb_entry.port  =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_PORT_ID].value.oid;

    sal_memcpy(&new_fdb_entry.fdb_key, fdb_entry, sizeof(sai_fdb_entry_t));

    CTC_ERROR_GOTO(ctc_fdb_create_entry(&new_fdb_entry),ret,out);

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}


sai_status_t ctc_sai_create_fdb_entry_with_nexthop(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ const uint32_t nh_id,
    _In_ sai_object_id_t port_id
    )
{
    ctc_l2_addr_t       l2_addr;
    uint32                  pgport = 0;
    int32_t             sdk_ret     = 0;
    sal_memset(&l2_addr, 0, sizeof(l2_addr));
    l2_addr.fid  = fdb_entry->vlan_id; 
    
    sal_memcpy(l2_addr.mac, fdb_entry->mac_address, sizeof(fdb_entry->mac_address));
    ctc_sai_port_objectid_to_gport(port_id, &pgport);
    l2_addr.gport = pgport;
    sdk_ret = ctc_l2_add_fdb_with_nexthop(&l2_addr, nh_id);    
    return ctc_sai_get_error_from_sdk_error(sdk_ret);
}
/*
* Routine Description:
*    Create macfilter fdb 
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_macfilter_fdb(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   *pattr_entry_list = NULL;
    ctc_sai_fdb_entry_t         new_fdb_entry;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&new_fdb_entry,0,sizeof(new_fdb_entry));

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);
    
    new_fdb_entry.type = pattr_entry_list[SAI_FDB_ENTRY_ATTR_TYPE].value.s32;

    sal_memcpy(&new_fdb_entry.fdb_key, fdb_entry, sizeof(sai_fdb_entry_t));

    CTC_ERROR_GOTO(ctc_creat_macfilter_fdb(&new_fdb_entry),ret,out);

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

/*
* Routine Description:
*    Create l2protocol fdb 
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_create_l2protocol_fdb(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t*  attr = NULL;
    ctc_sai_fdb_entry_t new_fdb_entry;
    uint32_t attr_idx = 0; 

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&new_fdb_entry,0,sizeof(new_fdb_entry));
    
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch (attr->id)
        {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            new_fdb_entry.type = attr->value.s32;
            break;
        case SAI_FDB_ENTRY_L2PROTOCOL_FLAG:
            new_fdb_entry.flag = attr->value.s32;
            break;
        }
    }
    sal_memcpy(&new_fdb_entry.fdb_key, fdb_entry, sizeof(sai_fdb_entry_t));

    ret = ctc_creat_macfilter_fdb_with_nexthop(&new_fdb_entry);
    return ret;
}


/*
* Routine Description:
*    Remove FDB entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_fdb_entry(
    _In_ const sai_fdb_entry_t* fdb_entry)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_fdb_entry_t         sai_fdb_entry;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:vlan_id = %u, mac_address = %x:%x:%x:%x:%x:%x", 
        fdb_entry->vlan_id, 
        fdb_entry->mac_address[5], fdb_entry->mac_address[4], fdb_entry->mac_address[3],
        fdb_entry->mac_address[2], fdb_entry->mac_address[1], fdb_entry->mac_address[0]);

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    sal_memset(&sai_fdb_entry,0,sizeof(ctc_sai_fdb_entry_t));
    sal_memcpy(sai_fdb_entry.fdb_key.mac_address, fdb_entry->mac_address, sizeof(fdb_entry->mac_address));
    sai_fdb_entry.fdb_key.vlan_id = fdb_entry->vlan_id;

    ret = ctc_fdb_remove_fdb_entry(&sai_fdb_entry);
    return ret;
}

/*
* Routine Description:
*    Remove macfilter FDB
*
* Arguments:
*    [in] fdb_entry - fdb entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_macfilter_fdb(
    _In_ const sai_fdb_entry_t* fdb_entry)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_fdb_entry_t         sai_fdb_entry;

    sal_memset(&sai_fdb_entry, 0, sizeof(ctc_sai_fdb_entry_t));
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    
    sal_memcpy(sai_fdb_entry.fdb_key.mac_address, fdb_entry->mac_address, sizeof(fdb_entry->mac_address));

    ret = ctc_remove_macfilter_fdb(&sai_fdb_entry);
    return ret;
}


/*
* Routine Description:
*    Remove l2protocol entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_remove_l2protocol_fdb(
    _In_ const sai_fdb_entry_t* fdb_entry)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_fdb_entry_t         sai_fdb_entry;

    sal_memset(&sai_fdb_entry, 0, sizeof(ctc_sai_fdb_entry_t));
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    
    sal_memcpy(sai_fdb_entry.fdb_key.mac_address, fdb_entry->mac_address, sizeof(fdb_entry->mac_address));

    ret = ctc_remove_l2protocol_fdb(&sai_fdb_entry);
    return ret;
}


/*
* Routine Description:
*    Create FDB entry
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_update_fdb_entry(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_attr_entry_list_t   *pattr_entry_list = NULL;
    ctc_sai_fdb_entry_t         new_fdb_entry;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&new_fdb_entry,0,sizeof(new_fdb_entry));

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_alloc_attr_entry_list(g_sai_attr_entries,
                            attr_list,
                            attr_count,
                            &pattr_entry_list),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_attr_entry_list(g_sai_attr_entries,
                            pattr_entry_list),ret,out);

    new_fdb_entry.type =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_TYPE].value.s32;
    new_fdb_entry.action  =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_PACKET_ACTION].value.s32;
    new_fdb_entry.port  =
            pattr_entry_list[SAI_FDB_ENTRY_ATTR_PORT_ID].value.oid;

    sal_memcpy(&new_fdb_entry.fdb_key, fdb_entry, sizeof(sai_fdb_entry_t));

    CTC_ERROR_GOTO(ctc_fdb_flush_entry(&new_fdb_entry),ret,out);

out:
    if(pattr_entry_list)
    {
        ctc_sai_attr_free_attr_entry_list(pattr_entry_list);
    }
    return ret;
}

/*
* Routine Description:
*    Set fdb entry attribute value
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_set_fdb_entry_attribute(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_fdb_entry_t pfdb_entry;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_set_fdb_entry_attribute_debug_param(fdb_entry, attr);

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr);

    sal_memset(&pfdb_entry,0,sizeof(ctc_sai_fdb_entry_t));

    CTC_SAI_ERROR_GOTO(ctc_sai_attr_check_write_attr(g_sai_attr_entries,attr),ret,out);

    CTC_SAI_ERROR_GOTO(ctc_fdb_lookup_entry(fdb_entry,&pfdb_entry),ret,out)

    switch(attr->id)
    {
    case SAI_FDB_ENTRY_ATTR_TYPE:
        ret = ctc_fdb_set_entry_type(&pfdb_entry,attr);
        break;
    case SAI_FDB_ENTRY_ATTR_PORT_ID:
        ret = ctc_fdb_set_entry_port_id(&pfdb_entry,attr);
        break;
    case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
        ret = ctc_fdb_set_entry_packet_action(&pfdb_entry,attr);
        break;
    default:
        break;
    }

out:
    return ret;
}

/*
* Routine Description:
*    Get fdb entry attribute value
*
* Arguments:
*    [in] fdb_entry - fdb entry
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_get_fdb_entry_attribute(
    _In_ const sai_fdb_entry_t* fdb_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t* attr        = NULL;
    uint32_t         attr_idx    = 0;
    ctc_sai_fdb_entry_t pfdb_entry;

    CTC_SAI_DEBUG_FUNC();
    ctc_sai_get_fdb_entry_attribute_debug_param(fdb_entry, attr_count, attr_list);

    CTC_SAI_PTR_VALID_CHECK(fdb_entry);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&pfdb_entry,0,sizeof(ctc_sai_fdb_entry_t));

    CTC_SAI_ERROR_GOTO(ctc_fdb_lookup_entry(fdb_entry,&pfdb_entry),ret,out);

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        ret = ctc_sai_attr_check_read_attr(g_sai_attr_entries,attr);

        if(ret != SAI_STATUS_SUCCESS)
        {
            return ret + attr_idx;
        }

        switch(attr->id)
        {
        case SAI_FDB_ENTRY_ATTR_TYPE:
            ret = ctc_fdb_get_entry_type(&pfdb_entry,attr);
            break;
        case SAI_FDB_ENTRY_ATTR_PORT_ID:
            ret = ctc_fdb_get_entry_port_id(&pfdb_entry,attr);
            break;
        case SAI_FDB_ENTRY_ATTR_PACKET_ACTION:
            ret = ctc_fdb_get_entry_packet_action(&pfdb_entry,attr);
            break;
        default:
            break;
        }

        if(ret != SAI_STATUS_SUCCESS)
        {
            break;
        }
    }

out:
    return ret;
}


/*
* Routine Description:
*    Remove  FDB entries 
*
* Arguments:
*    [in] attr_list - array of attributes
     [in] attr_count - number of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
ctc_sai_flush_fdb_entries(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    uint32         attr_idx = 0;  
    uint32         gport = 0;
    bool           has_port = false;
    bool           has_vlan = false;
    bool           has_mac = false;
    #if defined(USW)
    bool           has_pending = false;
    #endif
    
    /* SYSTEM MODIFIED by liwh for bug 37200, 2016-08-25 */
    bool           has_hardware = false;
    /* liwh end */
    
    sai_attribute_t*  attr = NULL;
    ctc_l2_flush_fdb_t Flush;
    sal_memset(&Flush,0,sizeof(Flush));
    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;
    
    CTC_SAI_DEBUG_FUNC();
    ctc_sai_flush_fdb_entries_debug_param(attr_count, attr_list);

    CTC_SAI_PTR_VALID_CHECK(attr_list);
    for (attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = (sai_attribute_t*)attr_list + attr_idx;
        switch(attr->id)
        {
        case SAI_FDB_FLUSH_ATTR_ENTRY_TYPE:
            if (SAI_FDB_FLUSH_ENTRY_DYNAMIC == attr->value.s32)
            {
                 Flush.flush_flag = CTC_L2_FDB_ENTRY_DYNAMIC;
            } 
            else if (SAI_FDB_FLUSH_ENTRY_STATIC == attr->value.s32)
            {
                 Flush.flush_flag = CTC_L2_FDB_ENTRY_STATIC;
            }
            else 
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }
            break;
        #if defined(USW)
        case SAI_FDB_FLUSH_ATTR_PENDING:
            has_pending = TRUE;
            Flush.flush_flag = CTC_L2_FDB_ENTRY_PENDING;
            break;
        #endif
            
        case SAI_FDB_FLUSH_ATTR_PORT_ID:
            has_port = TRUE;
            ctc_sai_port_objectid_to_gport(attr->value.oid, &gport);
            Flush.gport = gport;
            break;

        case SAI_FDB_FLUSH_ATTR_VLAN_ID:
            has_vlan = TRUE;
            Flush.fid = attr->value.u16;
            break;
            
        case SAI_FDB_FLUSH_ATTR_MAC:
            has_mac = TRUE;
            sal_memcpy(Flush.mac, attr->value.mac, sizeof(mac_addr_t));
            break;

        /* SYSTEM MODIFIED by liwh for bug 37200, 2016-08-25 */
        case SAI_FDB_FLUSH_ATTR_HARDWARE:
            has_hardware = TRUE;
            break;
        /* liwh end */
            
        default:
            break;
        }
    }

    #if defined(USW)
    if (has_pending)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    }
    else if (has_port && has_vlan)
    #else
    if (has_port && has_vlan)
    #endif
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT_VLAN;
    }
    else if (has_port)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_PORT;
    }
    else if (has_vlan)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_VID; 
    }
    else if (has_mac)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_BY_MAC;  
    }
    /* SYSTEM MODIFIED by liwh for bug 37200, 2016-08-25 */
    else if (has_hardware)
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_HARDWARE;    
    }
    /* liwh end */
    else
    {
        Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    }

    if(CTC_E_NONE != ctc_l2_flush_fdb(&Flush))
    {
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;   
}

#define ________SAI_FDB_INNER_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_fdb_info_init(),ret,out);

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_fdb_api_t      g_sai_fdb_api_func = {
    .create_fdb_entry           = ctc_sai_create_fdb_entry,
    .remove_fdb_entry           = ctc_sai_remove_fdb_entry,
    .create_macfilter_fdb       = ctc_sai_create_macfilter_fdb,/*added by yejl to add the function of blackhole mac, bug37480*/
    .remove_macfilter_fdb       = ctc_sai_remove_macfilter_fdb,/*added by yejl to add the function of blackhole mac, bug37480*/
    .create_l2protocol_fdb      = ctc_sai_create_l2protocol_fdb,/*added by yejl for l2protocol*/
    .remove_l2protocol_fdb      = ctc_sai_remove_l2protocol_fdb,/*added by yejl for l2protocol*/
    .set_fdb_entry_attribute    = ctc_sai_set_fdb_entry_attribute,
    .get_fdb_entry_attribute    = ctc_sai_get_fdb_entry_attribute,
    .flush_fdb_entries          = ctc_sai_flush_fdb_entries,
    .update_fdb_entry           = ctc_sai_update_fdb_entry,    
    .create_fdb_entry_with_nexthop  = ctc_sai_create_fdb_entry_with_nexthop,
};

static ctc_sai_api_reg_info_t g_fdb_api_reg_info = {
        .id         = SAI_API_FDB,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_fdb_api_func,
        .private_data     = NULL,
};

#define ________SAI_FDB_OUTER_FUNC
sai_status_t
ctc_sai_fdb_init()
{
    api_reg_register_fn(&g_fdb_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

