
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_flow_db.h"

hsrv_openflow_ttp_flow_master_t *g_pst_openflow_ttp_flow_master;

/*
struct hsrv_openflow_flow_iterate_data_s
{
    int32 (*fn)(hsrv_openflow_ttp_flow_t* p_flow);
};
typedef struct hsrv_openflow_flow_iterate_data_s hsrv_openflow_flow_iterate_data_t;
*/

#define _____FLOW_HASH_____

static inline uint32
hsrv_openflow_flowdb_flow_make(hsrv_openflow_ttp_flow_t* p_flow)
{
    return p_flow->flow.flow_id;
}

static bool
hsrv_openflow_flowdb_flow_compare(hsrv_openflow_ttp_flow_t* pva, hsrv_openflow_ttp_flow_t* pvb)
{
    if (pva->flow.flow_id != pvb->flow.flow_id)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_flowdb_add_flow(hsrv_openflow_ttp_flow_t* p_flow)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_FLOW_HASH, p_flow, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_lookup_flow(uint32 flow_id, hsrv_openflow_ttp_flow_t** pp_flow)
{
    hsrv_openflow_ttp_flow_t lookup;

    lookup.flow.flow_id = flow_id;

    *pp_flow = (hsrv_openflow_ttp_flow_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_FLOW_HASH, &lookup);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_del_flow(uint32 flow_id)
{
    hsrv_openflow_ttp_flow_t* p_flow;
    
    hsrv_openflow_flowdb_lookup_flow(flow_id, &p_flow);
    if (p_flow)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_FLOW_HASH, p_flow);
    }
    
    return HSRV_E_NONE;
}


#define ____METADATA_HASH____

int32
hsrv_openflow_flowdb_alloc_fid(uint16 *fid)
{
    uint32 opf_fid = 0;

    HSRV_OPENFLOW_PTR_CHECK(fid);

    opf_fid = ctclib_opf_alloc_offset(g_pst_openflow_ttp_flow_master->pst_metadata_fid_opf, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == opf_fid)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *fid = opf_fid;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_release_fid(uint16 fid)
{
    ctclib_opf_free_offset(g_pst_openflow_ttp_flow_master->pst_metadata_fid_opf, 0, 1, fid);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_alloc_vrf_id(uint16 *vrf_id)
{
    uint32 opf_vrf = 0;

    HSRV_OPENFLOW_PTR_CHECK(vrf_id);

    opf_vrf = ctclib_opf_alloc_offset(g_pst_openflow_ttp_flow_master->pst_metadata_vrf_opf, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == opf_vrf)
    {
        return HSRV_E_RESOURCE_FULL;
    }
    *vrf_id = opf_vrf;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_release_vrf_id(uint16 vrf_id)
{
    ctclib_opf_free_offset(g_pst_openflow_ttp_flow_master->pst_metadata_vrf_opf, 0, 1, vrf_id);
    
    return HSRV_E_NONE;
}


static inline uint32
hsrv_openflow_flowdb_metadata_make(metadata_mapping_t* p_metadata)
{
    return p_metadata->metadata;
}

static bool
hsrv_openflow_flowdb_metadata_compare(metadata_mapping_t* pva, metadata_mapping_t* pvb)
{
    if (pva->metadata != pvb->metadata)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_flowdb_add_metadata_mapping(metadata_mapping_t* p_metadata)
{
    ctclib_hash_get(P_METADATA_HASH, p_metadata, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_lookup_metadata_mapping(uint64 metadata, metadata_mapping_t** pp_metadata)
{
    metadata_mapping_t lookup;

    lookup.metadata= metadata;

    *pp_metadata = (metadata_mapping_t*)ctclib_hash_lookup(P_METADATA_HASH, &lookup);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_del_metadata_mapping(uint64 metadata)
{
    metadata_mapping_t* p_metadata;
    
    hsrv_openflow_flowdb_lookup_metadata_mapping(metadata, &p_metadata);
    if (p_metadata)
    {
        ctclib_hash_release(P_METADATA_HASH, p_metadata);
    }
    
    return HSRV_E_NONE;
}





/*
static void 
hsrv_openflow_flowdb_loop_flow__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_ttp_flow_t* p_flow = (hsrv_openflow_ttp_flow_t*)phb->data;
    hsrv_openflow_flow_iterate_data_t* p_data = user_data; 

    if (p_data->fn)
    {
        p_data->fn(p_flow);
    }
}

int32
hsrv_openflow_flowdb_loop_flow(int32 (*fn)())
{
    hsrv_openflow_flow_iterate_data_t data;

    data.fn = fn;
    
    ctclib_hash_iterate(HSRV_OPENFLOW_TTP_FLOW_HASH, hsrv_openflow_flowdb_loop_flow__, &data);

    return HSRV_E_NONE;
}
*/


#define _____INIT_____

int32
hsrv_openflow_flowdb_start(void)
{
    g_pst_openflow_ttp_flow_master = HSRV_OPENFLOW_FLOW_MALLOC(sizeof(hsrv_openflow_ttp_flow_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_flow_master);
    
    HSRV_OPENFLOW_TTP_FLOW_HASH = ctclib_hash_create(
        hsrv_openflow_flowdb_flow_make,
        hsrv_openflow_flowdb_flow_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_FLOW_HASH);

    P_METADATA_HASH = ctclib_hash_create(
        hsrv_openflow_flowdb_metadata_make,
        hsrv_openflow_flowdb_metadata_compare);
    HSRV_OPENFLOW_MEM_CHECK(P_METADATA_HASH);

    g_pst_openflow_ttp_flow_master->pst_metadata_fid_opf = ctclib_opf_init(1);
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_flow_master->pst_metadata_fid_opf);
    ctclib_opf_init_offset(g_pst_openflow_ttp_flow_master->pst_metadata_fid_opf, 0, HSRV_OPF_FID_MIN, HSRV_OPF_FID_MAX_NUM);

    g_pst_openflow_ttp_flow_master->pst_metadata_vrf_opf = ctclib_opf_init(1);
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_flow_master->pst_metadata_vrf_opf);
    ctclib_opf_init_offset(g_pst_openflow_ttp_flow_master->pst_metadata_vrf_opf, 0, HSRV_OPF_VRF_MIN, HSRV_OPF_VRF_MAX_NUM);

    return HSRV_E_NONE;
}

#define _____END_LINE_____
