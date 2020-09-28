
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctclib_hash.h"
#include "hagt_inc.h"

#include "hagt.h"
#include "hagt_openflow.h"
#include "hagt_openflow_stats_db.h"

/****************************************************************************************
 *
 * Globals
 *
 ****************************************************************************************/

/****************************************************************************************
 *
 * Function
 *
 ****************************************************************************************/
static inline uint32
_adpt_openflow_stats_info_hash_make(void* p_info_void)
{
    hagt_openflow_stats_info_t* p_info = NULL;

    p_info = p_info_void;
    return (uint32) p_info->stats.resource_id;
}

static inline bool
_adpt_openflow_stats_info_hash_compare(void* p_info_in_bucket_void, void* p_info_to_lkp_void)
{
    hagt_openflow_stats_info_t* p_info_in_bucket = NULL;
    hagt_openflow_stats_info_t* p_info_to_lkp = NULL;

    p_info_in_bucket = p_info_in_bucket_void;
    p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->stats.resource_id != p_info_to_lkp->stats.resource_id)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_adpt_openflow_statsid_hash_make(void* p_info_void)
{
    hagt_openflow_statsid_t* p_info = NULL;

    p_info = p_info_void;
    return (uint32) p_info->stats_id;
}

static inline bool
_adpt_openflow_statsid_hash_compare(void* p_info_in_bucket_void, void* p_info_to_lkp_void)
{
    hagt_openflow_statsid_t* p_info_in_bucket = NULL;
    hagt_openflow_statsid_t* p_info_to_lkp = NULL;

    p_info_in_bucket = p_info_in_bucket_void;
    p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->stats_id != p_info_to_lkp->stats_id)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_adpt_openflow_policerid_hash_make(void* p_info_void)
{
    hagt_openflow_policerid_t* p_info = NULL;

    p_info = p_info_void;
    return (uint32) p_info->meter_id;
}

static inline bool
_adpt_openflow_policerid_hash_compare(void* p_info_in_bucket_void, void* p_info_to_lkp_void)
{
    hagt_openflow_policerid_t* p_info_in_bucket = NULL;
    hagt_openflow_policerid_t* p_info_to_lkp = NULL;

    p_info_in_bucket = p_info_in_bucket_void;
    p_info_to_lkp = p_info_to_lkp_void;

    if (p_info_in_bucket->meter_id != p_info_to_lkp->meter_id)
    {
        return FALSE;
    }

    return TRUE;
}

static inline hagt_openflow_stats_info_t*
_hagt_openflow_statsdb_calloc_stats_info (void)
{
    return (hagt_openflow_stats_info_t*)HAGT_OPENFLOW_CALLOC( sizeof(hagt_openflow_stats_info_t));
}

static inline int32
_hagt_openflow_statsdb_free_stats_info (hagt_openflow_stats_info_t* pst_stats_info)
{
    HAGT_OPENFLOW_PTR_CHECK(pst_stats_info);

    HAGT_OPENFLOW_FREE(pst_stats_info);

    return HAGT_E_NONE;
}

int32
hagt_openflow_statsdb_add_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id,
        hagt_openflow_stats_info_t *p_stats_info)
{
    hagt_openflow_stats_info_t *p_new_stats_info = NULL;
    p_new_stats_info = _hagt_openflow_statsdb_calloc_stats_info();
    p_new_stats_info->stats_id = p_stats_info->stats_id;
    p_new_stats_info->stats.resource_id = p_stats_info->stats.resource_id;
    
    switch (resource_type)
    {
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            p_stats_info = ctclib_hash_get(g_pst_hagt_openflow_master->pst_flow_stats_info_hash, p_new_stats_info,
                    ctclib_hash_alloc_intern);
            if (!p_stats_info)
            {
                _hagt_openflow_statsdb_free_stats_info(p_new_stats_info);
                return HAGT_E_RESOURCE_FULL;
            }
            break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            p_stats_info = ctclib_hash_get(g_pst_hagt_openflow_master->pst_meter_stats_info_hash, p_new_stats_info,
                    ctclib_hash_alloc_intern);
            if (!p_stats_info)
            {
                _hagt_openflow_statsdb_free_stats_info(p_new_stats_info);
                return HAGT_E_RESOURCE_FULL;
            }
            break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            p_stats_info = ctclib_hash_get(g_pst_hagt_openflow_master->pst_group_stats_info_hash, p_new_stats_info,
                    ctclib_hash_alloc_intern);
            if (!p_stats_info)
            {
                _hagt_openflow_statsdb_free_stats_info(p_new_stats_info);
                return HAGT_E_RESOURCE_FULL;
            }
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}

int32
hagt_openflow_statsdb_del_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id)
{
    hagt_openflow_stats_info_t stats_info;
    hagt_openflow_stats_info_t *p_stats_info;

    memset(&stats_info, 0, sizeof(hagt_openflow_stats_info_t));
    stats_info.stats.resource_id = resource_id;

    switch (resource_type)
    {
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            p_stats_info = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_flow_stats_info_hash, &stats_info);
            if (p_stats_info)
            {
                ctclib_hash_release(g_pst_hagt_openflow_master->pst_flow_stats_info_hash, p_stats_info);
                _hagt_openflow_statsdb_free_stats_info(p_stats_info);
            }
            break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            p_stats_info = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_meter_stats_info_hash, &stats_info);
            if (p_stats_info)
            {
                ctclib_hash_release(g_pst_hagt_openflow_master->pst_meter_stats_info_hash, p_stats_info);
                _hagt_openflow_statsdb_free_stats_info(p_stats_info);
            }
            break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            p_stats_info = ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_group_stats_info_hash, &stats_info);
            if (p_stats_info)
            {
                ctclib_hash_release(g_pst_hagt_openflow_master->pst_group_stats_info_hash, p_stats_info);
                _hagt_openflow_statsdb_free_stats_info(p_stats_info);
            }
            break;
        default:
            return HAGT_E_INVALID_PARAM;
    }
            
    return HAGT_E_NONE;
}

void*
hagt_openflow_statsdb_get_stats(hagt_openflow_stats_resource_type_t resource_type, uint32 resource_id)
{
    hagt_openflow_stats_info_t info_to_lkp;

    info_to_lkp.stats.resource_id = resource_id;
    switch (resource_type)
    {
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_FLOW:
            return ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_flow_stats_info_hash, &info_to_lkp);
        break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_METER:
            return ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_meter_stats_info_hash, &info_to_lkp);
        break;
        case HAGT_OPENFLOW_STATS_RESOURCE_TYPE_GROUP:
            return ctclib_hash_lookup(g_pst_hagt_openflow_master->pst_group_stats_info_hash, &info_to_lkp);
        break;
        default:
            return NULL;
        break;
    }
    return NULL;
}

int32
hagt_openflow_statsdb_init(void)
{
    g_pst_hagt_openflow_master->pst_flow_stats_info_hash = ctclib_hash_create(
            _adpt_openflow_stats_info_hash_make,
            _adpt_openflow_stats_info_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_flow_stats_info_hash);
    g_pst_hagt_openflow_master->pst_meter_stats_info_hash = ctclib_hash_create(
            _adpt_openflow_stats_info_hash_make,
            _adpt_openflow_stats_info_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_meter_stats_info_hash);
    g_pst_hagt_openflow_master->pst_group_stats_info_hash = ctclib_hash_create(
            _adpt_openflow_stats_info_hash_make,
            _adpt_openflow_stats_info_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_group_stats_info_hash);

    g_pst_hagt_openflow_master->pst_openflow_statsid_hash = ctclib_hash_create(
            _adpt_openflow_statsid_hash_make,
            _adpt_openflow_statsid_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_openflow_statsid_hash);

    g_pst_hagt_openflow_master->pst_openflow_policerid_hash = ctclib_hash_create(
            _adpt_openflow_policerid_hash_make,
            _adpt_openflow_policerid_hash_compare);
    HAGT_OPENFLOW_MEM_CHECK(g_pst_hagt_openflow_master->pst_openflow_policerid_hash);
    
    return HAGT_E_NONE;
}
