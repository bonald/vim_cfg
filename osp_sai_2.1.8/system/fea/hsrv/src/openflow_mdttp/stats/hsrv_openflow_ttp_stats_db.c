#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_stats_db.h"

hsrv_openflow_ttp_stats_master_t    *g_pst_openflow_ttp_stats_master = NULL;

static inline uint32
_hsrv_openflow_statsdb_stats_make(hsrv_openflow_ttp_stats_info_t* p_stats)
{
    return p_stats->stats_id;
}

static inline bool
_hsrv_openflow_statsdb_stats_compare(hsrv_openflow_ttp_stats_info_t* pva, hsrv_openflow_ttp_stats_info_t* pvb)
{
    if (pva->stats_id != pvb->stats_id)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_statsdb_add_stats(hsrv_openflow_ttp_stats_info_t* p_stats)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_STATS_HASH, p_stats, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_statsdb_del_stats(uint32 stats_id)
{
    hsrv_openflow_ttp_stats_info_t* p_stats_lkp = NULL;

    hsrv_openflow_statsdb_lookup_stats(stats_id, &p_stats_lkp);
    if (p_stats_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_STATS_HASH, p_stats_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_statsdb_lookup_stats(uint32 stats_id, hsrv_openflow_ttp_stats_info_t** pp_stats)
{
    hsrv_openflow_ttp_stats_info_t stats_lkp;

    sal_memset(&stats_lkp, 0, sizeof(hsrv_openflow_ttp_stats_info_t));
    stats_lkp.stats_id = stats_id;
 
    *pp_stats = (hsrv_openflow_ttp_stats_info_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_STATS_HASH, &stats_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_statsdb_alloc_stats_id(uint32 *stats_id)
{

    uint32 opf_stats_id = 0;

    HSRV_OPENFLOW_PTR_CHECK(stats_id);

    opf_stats_id = ctclib_opf_alloc_offset(g_pst_openflow_ttp_stats_master->pst_stats_id, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == opf_stats_id)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *stats_id = opf_stats_id;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_statsdb_release_stats_id(uint32 stats_id)
{
    ctclib_opf_free_offset(g_pst_openflow_ttp_stats_master->pst_stats_id, 0, 1, stats_id);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_statsdb_start(void)
{
    if (NULL == g_pst_openflow_ttp_stats_master)
    {
        g_pst_openflow_ttp_stats_master = HSRV_OPENFLOW_STATS_MALLOC(sizeof(hsrv_openflow_ttp_stats_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_stats_master);

        HSRV_OPENFLOW_TTP_STATS_HASH = ctclib_hash_create(_hsrv_openflow_statsdb_stats_make,
                                                      _hsrv_openflow_statsdb_stats_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_STATS_HASH);

        g_pst_openflow_ttp_stats_master->pst_stats_id = ctclib_opf_init(1);
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_stats_master->pst_stats_id);
        ctclib_opf_init_offset(g_pst_openflow_ttp_stats_master->pst_stats_id, 0, HSRV_OPF_STATS_ID_START, HSRV_OPF_STATS_ID_NUM);
    }


    
    return HSRV_E_NONE;
}

