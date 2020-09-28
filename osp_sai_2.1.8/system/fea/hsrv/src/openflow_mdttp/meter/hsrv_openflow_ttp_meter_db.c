#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_meter_db.h"

hsrv_openflow_ttp_meter_master_t    *g_pst_openflow_ttp_meter_master = NULL;

static inline uint32
_hsrv_openflow_meterdb_meter_make(hsrv_openflow_ttp_meter_info_t* p_meter)
{
    return p_meter->meter_id;
}

static inline bool
_hsrv_openflow_meterdb_meter_compare(hsrv_openflow_ttp_meter_info_t* pva, hsrv_openflow_ttp_meter_info_t* pvb)
{
    if (pva->meter_id != pvb->meter_id)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_meterdb_add_meter(hsrv_openflow_ttp_meter_info_t* p_meter)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_METER_HASH, p_meter, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_meterdb_del_meter(uint32 meter_id)
{
    hsrv_openflow_ttp_meter_info_t* p_meter_lkp = NULL;

    hsrv_openflow_meterdb_lookup_meter(meter_id, &p_meter_lkp);
    if (p_meter_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_METER_HASH, p_meter_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_meterdb_lookup_meter(uint32 meter_id, hsrv_openflow_ttp_meter_info_t** pp_meter)
{
    hsrv_openflow_ttp_meter_info_t meter_lkp;

    sal_memset(&meter_lkp, 0, sizeof(hsrv_openflow_ttp_meter_info_t));
    meter_lkp.meter_id = meter_id;
 
    *pp_meter = (hsrv_openflow_ttp_meter_info_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_METER_HASH, &meter_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_meterdb_start(void)
{
    if (NULL == g_pst_openflow_ttp_meter_master)
    {
        g_pst_openflow_ttp_meter_master = HSRV_OPENFLOW_METER_MALLOC(sizeof(hsrv_openflow_ttp_meter_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_meter_master);

        HSRV_OPENFLOW_TTP_METER_HASH = ctclib_hash_create(_hsrv_openflow_meterdb_meter_make,
                                                      _hsrv_openflow_meterdb_meter_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_METER_HASH);
    }
    
    return HSRV_E_NONE;
}

