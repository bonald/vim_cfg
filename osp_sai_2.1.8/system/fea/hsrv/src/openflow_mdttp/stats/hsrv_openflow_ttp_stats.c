
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_stats.h"
#include "hsrv_openflow_ttp_stats_db.h"

extern hsrv_openflow_ttp_stats_master_t    *g_pst_openflow_ttp_stats_master;

struct hsrv_openflow_stats_debug_data_s
{
    FILE* fp;
    int32 i;
};
typedef struct hsrv_openflow_stats_debug_data_s hsrv_openflow_stats_debug_data_t;

#define ____STATS_APIs____

static int32
hsrv_openflow_map_ctc_stats_type(glb_stats_type_t resource_type, uint32 group_or_table_id, bool is_tunnel_group, hsrv_openflow_ttp_stats_type_t *stats_type)
{
    switch(resource_type)
    {
    case GLB_STATS_TYPE_OPENFLOW_FLOW:
        if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_PORT == group_or_table_id)
        {
            //only ingress port is tunnel port support stats
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL;
        }
        if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_VLAN == group_or_table_id)
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_SCL;
        }
        else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_TERMINATION_MAC == group_or_table_id)
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_SCL;
        }
        else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_UNICAST_ROUTING == group_or_table_id)
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST;  /* table miss */
        }
        else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_BRIDGING == group_or_table_id)
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST; /* table miss */
        }
        else if (GLB_OPENFLOW_TTP_FLOW_TABLE_ID_INGRESS_ACL == group_or_table_id)
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_ACL;
        }
        break;
    case GLB_STATS_TYPE_OPENFLOW_GROUP:
        if (GLB_OPENFLOW_TTP_GROUP_TYPE_L2_INTERFACE == GLB_OPENFLOW_TTP_GROUP_TYPE(group_or_table_id))
        {
            /* is ip tunnel nexthop */
            if (is_tunnel_group)
            {
                *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL;
            }
            else
            {
                *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_NEXTHOP;
            }
        }
        else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L2_FLOOD == GLB_OPENFLOW_TTP_GROUP_TYPE(group_or_table_id))
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_MCAST;
        }
        else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L3_ECMP == GLB_OPENFLOW_TTP_GROUP_TYPE(group_or_table_id))
        {
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_ECMP;
        }
        else if (GLB_OPENFLOW_TTP_GROUP_TYPE_L3_UNICAST == GLB_OPENFLOW_TTP_GROUP_TYPE(group_or_table_id))
        {
            //ip tunnel nexthop support stats, no check here, get stats_supported from group
            *stats_type = HSRV_OPENFLOW_TTP_STATS_TYPE_TUNNEL;
        }
        break;
     default:
        return HSRV_E_FAIL;
        break;
    }

    return HSRV_E_NONE;
}


int32 
hsrv_openflow_add_stats(glb_stats_type_t resource_type, uint32 resource_id, uint32 group_or_table_id, bool is_tunnel_group, uint32 *stats_id)
{
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;
    hsrv_openflow_ttp_stats_type_t stats_type;
    uint32 opf_stats_id = 0;
    uint32 ctc_stats_id = 0;
    uint32 ret = HSRV_E_NONE;

    p_stats = HSRV_OPENFLOW_STATS_MALLOC(sizeof(hsrv_openflow_ttp_stats_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_stats);
    sal_memset(p_stats, 0, sizeof(hsrv_openflow_ttp_stats_info_t));

    p_stats->type = resource_type;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_alloc_stats_id(&opf_stats_id));
    p_stats->stats_id = opf_stats_id;
    *stats_id = opf_stats_id;
    switch(resource_type)
    {
    case GLB_STATS_TYPE_OPENFLOW_FLOW:
    case GLB_STATS_TYPE_OPENFLOW_GROUP:
        p_stats->stats_rslt.openflow_stats.resource_id = resource_id;
        ret = hsrv_openflow_map_ctc_stats_type(resource_type, group_or_table_id, is_tunnel_group, &stats_type);
        if (ret)
        {
            goto err;
        }
        ret = ctc_ttp_drv_stats_create(stats_type, &ctc_stats_id);
        if (ret)
        {
            goto err;
        }
        p_stats->ctc_stats_id = ctc_stats_id;
        break;
    case GLB_STATS_TYPE_OPENFLOW_METER:
        p_stats->stats_rslt.policer.resource_id = resource_id;
        break;
    default:
        return HSRV_E_FAIL;
        break;
    }
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_add_stats(p_stats));
    return ret;

    err:
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_release_stats_id(opf_stats_id));  
    return ret;
}


int32
hsrv_openflow_del_stats(uint32 stats_id)
{
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    hsrv_openflow_statsdb_lookup_stats(stats_id, &p_stats);
    if (!p_stats)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    HSRV_IF_ERROR_RETURN(ctc_ttp_drv_stats_destory(p_stats->ctc_stats_id));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_release_stats_id(stats_id));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_del_stats(stats_id));
    
    HSRV_OPENFLOW_STATS_FREE(p_stats);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_clear_stats(uint32 stats_id)
{
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    hsrv_openflow_statsdb_lookup_stats(stats_id, &p_stats);
    if (!p_stats)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (GLB_STATS_TYPE_OPENFLOW_FLOW == p_stats->type || GLB_STATS_TYPE_OPENFLOW_GROUP == p_stats->type)
    {
        p_stats->stats_rslt.openflow_stats.packet_count = 0;
        p_stats->stats_rslt.openflow_stats.byte_count = 0;
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_stats_clear(p_stats->ctc_stats_id));
    }
    else if(GLB_STATS_TYPE_OPENFLOW_METER == p_stats->type)
    {
        p_stats->stats_rslt.policer.confirm_stats.packet_count = 0;
        p_stats->stats_rslt.policer.confirm_stats.byte_count = 0;
        
        p_stats->stats_rslt.policer.exceed_stats.packet_count = 0;
        p_stats->stats_rslt.policer.exceed_stats.byte_count = 0;

        p_stats->stats_rslt.policer.violate_stats.packet_count = 0;
        p_stats->stats_rslt.policer.violate_stats.byte_count = 0;
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_policer_clear_stats(p_stats->stats_rslt.policer.resource_id));
    }

    return HSRV_E_NONE;
}


hsrv_openflow_ttp_stats_info_t*
hsrv_openflow_get_stats_info(uint32 stats_id)
{
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;

    hsrv_openflow_statsdb_lookup_stats(stats_id, &p_stats);
    return  p_stats;
}

static void
_hsrv_stats_response(hsrv_openflow_ttp_stats_info_t* p_stats)
{
    if (GLB_STATS_TYPE_OPENFLOW_FLOW == p_stats->type)
    {
        HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_FLOW_STATS, &p_stats->stats_rslt, GLB_STATS_RSLT_LEN(p_stats->type));
    }
    else if (GLB_STATS_TYPE_OPENFLOW_METER == p_stats->type)
    {
        HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_METER_STATS, &p_stats->stats_rslt, GLB_STATS_RSLT_LEN(p_stats->type));
    }
    else if (GLB_STATS_TYPE_OPENFLOW_GROUP == p_stats->type)
    {
        HSRV2OPENFLOW_MSG_SEND_NOTIFY(OPENFLOW_HAL_MSG_TYPE_GROUP_STATS, &p_stats->stats_rslt, GLB_STATS_RSLT_LEN(p_stats->type));
    }
}


static bool
_hagt_openflow_fetch_stats(glb_stats_type_t resource_type, uint32 hsrv_stats_id)
{
    uint32 policer_id = 0;
    hsrv_openflow_ttp_meter_info_t *p_meter = NULL;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;
    hsrv_openflow_ttp_stats_t openflow_stats_info;
    hsrv_openflow_ttp_meter_stats_t meter_stats_info;

    sal_memset(&openflow_stats_info, 0, sizeof(hsrv_openflow_ttp_stats_t));
    sal_memset(&meter_stats_info, 0, sizeof(hsrv_openflow_ttp_meter_stats_t));
    hsrv_openflow_statsdb_lookup_stats(hsrv_stats_id, &p_stats);
    if (!p_stats)
    {
        return FALSE;
    }

    /* for meter, resource_id is meter_id, get policer id from hsrv_openflow_ttp_meter_info_t */
    if (resource_type == GLB_STATS_TYPE_OPENFLOW_METER)
    {
        p_meter = hsrv_openflow_get_meter_info(p_stats->stats_rslt.policer.resource_id);
        if (!p_meter)
        {
            return FALSE;
        }
        policer_id = p_meter->policer_id;
    }
    
    switch (resource_type)
    {
    case GLB_STATS_TYPE_OPENFLOW_FLOW:
    case GLB_STATS_TYPE_OPENFLOW_GROUP:
        if (ctc_ttp_drv_stats_get(&openflow_stats_info, p_stats->ctc_stats_id))
        {
            return FALSE;
        }
        if (openflow_stats_info.packet_count == p_stats->stats_rslt.openflow_stats.packet_count)
        {
            return FALSE;
        }
        p_stats->stats_rslt.openflow_stats.packet_count = openflow_stats_info.packet_count;
        p_stats->stats_rslt.openflow_stats.byte_count = openflow_stats_info.byte_count;
        break;
    case GLB_STATS_TYPE_OPENFLOW_METER:
        if (ctc_ttp_drv_policer_get_stats(policer_id, &meter_stats_info))
        {
            return FALSE;
        }
        if (meter_stats_info.confirm_stats.packet_count == p_stats->stats_rslt.policer.confirm_stats.packet_count && 
            meter_stats_info.exceed_stats.packet_count == p_stats->stats_rslt.policer.exceed_stats.packet_count &&
            meter_stats_info.violate_stats.packet_count == p_stats->stats_rslt.policer.violate_stats.packet_count)
        {
            return FALSE;
        }
        p_stats->stats_rslt.policer.confirm_stats.packet_count = meter_stats_info.confirm_stats.packet_count;
        p_stats->stats_rslt.policer.confirm_stats.byte_count = meter_stats_info.confirm_stats.byte_count;
        
        p_stats->stats_rslt.policer.exceed_stats.packet_count = meter_stats_info.exceed_stats.packet_count;
        p_stats->stats_rslt.policer.exceed_stats.byte_count = meter_stats_info.exceed_stats.byte_count;
        
        p_stats->stats_rslt.policer.violate_stats.packet_count = meter_stats_info.violate_stats.packet_count;
        p_stats->stats_rslt.policer.violate_stats.byte_count = meter_stats_info.violate_stats.byte_count;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}


static void
_hsrv_openflow_flow_stats_traverse_item(ctclib_hash_backet_t* phb, void *user_data)
{
    bool need_notify = TRUE;
    uint32 stats_id = 0;
    hsrv_openflow_ttp_stats_info_t* p_stats = NULL;
    
    p_stats = (hsrv_openflow_ttp_stats_info_t*)phb->data;
    stats_id = p_stats->stats_id;
    
    need_notify =  _hagt_openflow_fetch_stats(p_stats->type, stats_id);
    if (need_notify)
    {
        _hsrv_stats_response(p_stats);
    }
}


static void
_hsrv_openflow_stats_poll_handler(void* user_param)
{

    if (g_pst_openflow_ttp_stats_master->p_stats_hash->size)
    {
        ctclib_hash_iterate (g_pst_openflow_ttp_stats_master->p_stats_hash, _hsrv_openflow_flow_stats_traverse_item, NULL);
    }
    
    ctc_task_add_timer(TASK_PRI_LOW, _hsrv_openflow_stats_poll_handler, NULL, HSRV_OPENFLOW_TTP_STATS_POLLING_INTERVAL);
}


int32
hsrv_openflow_stats_init(void)
{
    if (sal_mutex_create(&g_pst_openflow_ttp_stats_master->p_stats_mutex))
    {
        goto err_handle;
    }
    
    ctc_task_add_timer(TASK_PRI_LOW, _hsrv_openflow_stats_poll_handler, NULL, HSRV_OPENFLOW_TTP_STATS_POLLING_INTERVAL);


err_handle:
    if (g_pst_openflow_ttp_stats_master->p_stats_mutex)
    {
        sal_mutex_destroy(g_pst_openflow_ttp_stats_master->p_stats_mutex);
        g_pst_openflow_ttp_stats_master->p_stats_mutex = NULL;
    }

    return HSRV_E_NONE;
}

#define ____STATS_DEBUG____

static void 
hsrv_openflow_ttp_show_stats_brief__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_ttp_stats_info_t *p_stats = (hsrv_openflow_ttp_stats_info_t*)phb->data;
    hsrv_openflow_stats_debug_data_t* p_data = user_data;
   
    sal_fprintf(p_data->fp, "%6u", p_data->i);
    sal_fprintf(p_data->fp, "%15s", STATS_TYPE_TO_STR(p_stats->type));
    sal_fprintf(p_data->fp, "%10u", p_stats->stats_id);
    if (GLB_STATS_TYPE_OPENFLOW_METER == p_stats->type)
    {
        sal_fprintf(p_data->fp, "%10u\n", p_stats->stats_rslt.policer.resource_id);
    }
    else
    {
        sal_fprintf(p_data->fp, "%10u\n", p_stats->ctc_stats_id);
    }
    
    p_data->i ++;
} 

int32
hsrv_openflow_ttp_show_stats_brief(FILE *fp)
{
    hsrv_openflow_stats_debug_data_t data;
    
    sal_fprintf(fp, "--------------HSRV OPENFLOW STATS-----------------------\n");
    sal_fprintf(fp, "Stats hash count    : %d\n", g_pst_openflow_ttp_stats_master->p_stats_hash->count);
    sal_fprintf(fp, "-------------------------------------------------------\n");

    sal_fprintf(fp, "%6s %15s %10s %10s\n", "index", "stats-type", "hsrv-stats-id", "sdk-stats-id");
    sal_fprintf(fp, "-------------------------------------------------------\n");

    data.fp = fp;
    data.i  = 0;
    
    ctclib_hash_iterate(HSRV_OPENFLOW_TTP_STATS_HASH, hsrv_openflow_ttp_show_stats_brief__, &data);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_ttp_show_stats(FILE *fp, uint32 stats_id)
{
    hsrv_openflow_ttp_stats_info_t *p_stats = NULL;
    
    hsrv_openflow_statsdb_lookup_stats(stats_id, &p_stats);
    if (NULL == p_stats)
    {
        sal_fprintf(fp, "stats_id %u not exist in db.\n", stats_id);
        return HSRV_E_NONE;
    }
    sal_fprintf(fp, "stats type : %s\n", STATS_TYPE_TO_STR(p_stats->type));
    sal_fprintf(fp, "hsrv stats id : %u\n", stats_id);
    if (GLB_STATS_TYPE_OPENFLOW_METER == p_stats->type)
    {
        sal_fprintf(fp, "flow policer id : %10u\n", p_stats->stats_rslt.policer.resource_id);
    }
    else
    {
        sal_fprintf(fp, "sdk stats id : %10u\n", p_stats->ctc_stats_id);
    }
    
    sal_fprintf(fp, "\n");
    return HSRV_E_NONE;
}

#define ____STATS_INIT____


int32
hsrv_openflow_stats_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_statsdb_start());
    HSRV_IF_ERROR_RETURN(hsrv_openflow_stats_init());

    return HSRV_E_NONE;
}


