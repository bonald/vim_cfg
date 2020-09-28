
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_meter.h"
#include "hsrv_openflow_ttp_meter_db.h"

#define ____METER_EXTERNAL_APIs____

hsrv_openflow_ttp_meter_info_t*
hsrv_openflow_get_meter_info(uint32 meter_id)
{
    hsrv_openflow_ttp_meter_info_t* p_meter = NULL;

    hsrv_openflow_meterdb_lookup_meter(meter_id, &p_meter);
    return  p_meter;
}

#define ____METER_INTERNAL_APIs____
static  int32
_hsrv_openflow_alloc_meter_entry(glb_openflow_meter_t *p_glb_meter, hsrv_openflow_ttp_meter_info_t **pp_meter)
{
    hsrv_openflow_ttp_meter_info_t  *p_meter = NULL;
    
    p_meter = HSRV_OPENFLOW_METER_MALLOC(sizeof(hsrv_openflow_ttp_meter_info_t));
    HSRV_OPENFLOW_MEM_CHECK(p_meter);
    HSRV_OPENFLOW_MEM_CHECK(p_glb_meter);

    p_meter->meter_id = p_glb_meter->meter_id;
    p_meter->policer_id = 0;
    p_meter->stats_en = p_glb_meter->stats_enable;

    *pp_meter = p_meter;
    
    return HSRV_E_NONE;
}

static  int32
_hsrv_openflow_alloc_policer_id(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    uint32 policer_id = 0;
    ctc_ttp_drv_policer_id_alloc(&policer_id);
    p_meter->policer_id = policer_id;
    
    return HSRV_E_NONE;
}

static  int32
_hsrv_openflow_release_policer_id(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    ctc_ttp_drv_policer_id_free(p_meter->policer_id);
    return HSRV_E_NONE;
}

static  int32
_hsrv_openflow_add_meter_entry(glb_openflow_meter_t *p_glb_meter, hsrv_openflow_ttp_meter_info_t *p_meter)
{
    uint32 policer_id = p_meter->policer_id;

    if (policer_id)
    {
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_policer_set(p_glb_meter, policer_id));
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

static  int32
_hsrv_openflow_del_meter_entry(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    uint32 policer_id = p_meter->policer_id;

    if (policer_id)
    {
        HSRV_IF_ERROR_RETURN(ctc_ttp_drv_policer_remove(policer_id));
    }
    else
    {
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

static  int32
_hsrv_openflow_meter_create_stats(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    uint32 stats_id = 0;

    HSRV_OPENFLOW_MEM_CHECK(p_meter);
    //stats_supported inherit from ovs
    //default all policer support stats
    if (p_meter->stats_en)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_add_stats(GLB_STATS_TYPE_OPENFLOW_METER, p_meter->meter_id, 0, FALSE, &stats_id));
        p_meter->stats_id = stats_id;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_meter_delete_stats(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    HSRV_OPENFLOW_MEM_CHECK(p_meter);
    if (p_meter->stats_en)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_del_stats(p_meter->stats_id));
        p_meter->stats_id = 0;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_meter_clear_stats(hsrv_openflow_ttp_meter_info_t *p_meter)
{
    HSRV_OPENFLOW_MEM_CHECK(p_meter);
    if (p_meter->stats_en)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_clear_stats(p_meter->stats_id));
    }

    return HSRV_E_NONE;
}

#define ____METER_HAL_APIs____

int32 
fei_api_openflow_add_meter(fei_openflow_add_meter_req_t* pst_req)
{
    glb_openflow_meter_t* p_meter_info = NULL;
    hsrv_openflow_ttp_meter_info_t* p_meter = NULL;

    p_meter_info = &pst_req->meter;
    hsrv_openflow_meterdb_lookup_meter(p_meter_info->meter_id, &p_meter);
    if (p_meter)
    {
        return HSRV_E_ENTRY_EXIST;
    }

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_alloc_meter_entry(p_meter_info, &p_meter));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_alloc_policer_id(p_meter));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_meter_create_stats(p_meter));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_meter_entry(p_meter_info, p_meter));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_meterdb_add_meter(p_meter));

    return HSRV_E_NONE;
}

int32
fei_api_openflow_mod_meter(fei_openflow_mod_meter_req_t* pst_req)
{
    glb_openflow_meter_t* p_meter_info = NULL;
    hsrv_openflow_ttp_meter_info_t* p_meter = NULL;

    p_meter_info = &pst_req->meter;
    hsrv_openflow_meterdb_lookup_meter(p_meter_info->meter_id, &p_meter);
    if (!p_meter)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_add_meter_entry(p_meter_info, p_meter));
    p_meter->stats_en = p_meter_info->stats_enable;
    //TODO policer stats clear?
    
    return HSRV_E_NONE;
}


int32
fei_api_openflow_del_meter(fei_openflow_del_meter_req_t* pst_req)
{
    uint32 meter_id;
    hsrv_openflow_ttp_meter_info_t* p_meter = NULL;

    meter_id = pst_req->meter_id;
    hsrv_openflow_meterdb_lookup_meter(meter_id, &p_meter);
    if (!p_meter)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_del_meter_entry(p_meter));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_release_policer_id(p_meter));
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_meter_delete_stats(p_meter));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_meterdb_del_meter(p_meter->meter_id));
    
    HSRV_OPENFLOW_METER_FREE(p_meter);
    
    return HSRV_E_NONE;
}

int32
fei_api_openflow_clear_meter_stats(fei_openflow_clear_stats_req_t *pst_req)
{
    hsrv_openflow_ttp_meter_info_t* p_meter = NULL;

    hsrv_openflow_meterdb_lookup_meter(pst_req->id, &p_meter);
    if (NULL == p_meter)
    {
        return HSRV_E_NONE;
    }
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_meter_clear_stats(p_meter));

    return HSRV_E_NONE;
}

#define ____METER_INIT____

int32
hsrv_openflow_meter_start(void)
{
    HSRV_IF_ERROR_RETURN(hsrv_openflow_meterdb_start());

    return HSRV_E_NONE;
}


