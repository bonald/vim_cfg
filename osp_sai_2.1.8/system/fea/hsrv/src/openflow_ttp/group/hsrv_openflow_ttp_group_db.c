#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_ttp_group.h"
#include "hsrv_openflow_ttp_group_db.h"


/*************************
 * Global and Declaration
 *************************/
hsrv_openflow_ttp_group_master_t *g_pst_openflow_ttp_group_master = NULL;


#define _____GROUPDB_HASH_APIS_____

static inline uint32
_hsrv_openflow_groupdb_group_make(hsrv_openflow_ttp_group_t* p_group)
{
    return p_group->group_id;
}

static inline bool
_hsrv_openflow_groupdb_group_compare(hsrv_openflow_ttp_group_t* pva, hsrv_openflow_ttp_group_t* pvb)
{
    if (pva->group_id != pvb->group_id)
    {
        return FALSE;
    }

    return TRUE;
}

static inline uint32
_hsrv_openflow_groupdb_group_bucket_make(hsrv_openflow_ttp_group_bucket_entry_t* p_group_bucket)
{
    return p_group_bucket->bucket_index;
}

static inline bool
_hsrv_openflow_groupdb_group_bucket_compare(hsrv_openflow_ttp_group_bucket_entry_t* pva, 
                                                                                                hsrv_openflow_ttp_group_bucket_entry_t *pvb)
{
    if (pva->bucket_index != pvb->bucket_index)
    {
        return FALSE;
    }

    return TRUE;
}

#define _____GROUPDB_APIS_____

int32
hsrv_openflow_groupdb_add_group(hsrv_openflow_ttp_group_t* p_group)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_GROUP_HASH, p_group, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_del_group(uint32 group_id)
{
    hsrv_openflow_ttp_group_t* p_group_lkp = NULL;

    hsrv_openflow_groupdb_lookup_group(group_id, &p_group_lkp);
    if (p_group_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_GROUP_HASH, p_group_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_lookup_group(uint32 group_id, hsrv_openflow_ttp_group_t** pp_group)
{
    hsrv_openflow_ttp_group_t group_lkp;

    sal_memset(&group_lkp, 0, sizeof(hsrv_openflow_ttp_group_t));
    group_lkp.group_id = group_id;
 
    *pp_group = (hsrv_openflow_ttp_group_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_GROUP_HASH, &group_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_add_group_bucket(hsrv_openflow_ttp_group_bucket_entry_t* p_group_bucket)
{
    ctclib_hash_get(HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH, p_group_bucket, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_del_group_bucket(uint32 bucket_index)
{
    hsrv_openflow_ttp_group_bucket_entry_t* p_group_bucket_lkp = NULL;

    hsrv_openflow_groupdb_lookup_group_bucket(bucket_index, &p_group_bucket_lkp);
    if (p_group_bucket_lkp)
    {
        ctclib_hash_release(HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH, p_group_bucket_lkp);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_lookup_group_bucket(uint32 bucket_index, hsrv_openflow_ttp_group_bucket_entry_t** pp_group_bucket)
{
    hsrv_openflow_ttp_group_bucket_entry_t group_bucket_lkp;

    sal_memset(&group_bucket_lkp, 0, sizeof(hsrv_openflow_ttp_group_bucket_entry_t));
    group_bucket_lkp.bucket_index = bucket_index;
 
    *pp_group_bucket = (hsrv_openflow_ttp_group_bucket_entry_t*)ctclib_hash_lookup(HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH, &group_bucket_lkp);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_alloc_bucket_index(uint32 *bucket_index)
{
    uint32 bucket_id = 0;

    HSRV_OPENFLOW_PTR_CHECK(bucket_index);

    bucket_id = ctclib_opf_alloc_offset(g_pst_openflow_ttp_group_master->pst_group_bucket_index_opf, 0, 1);
    if (CTCLIB_OPF_INVALID_OFFSET == bucket_id)
    {
        return HSRV_E_RESOURCE_FULL;
    }

    *bucket_index = bucket_id;

    return HSRV_E_NONE;
}

int32
hsrv_openflow_groupdb_release_bucket_index(uint32 bucket_index)
{
    ctclib_opf_free_offset(g_pst_openflow_ttp_group_master->pst_group_bucket_index_opf, 0, 1, bucket_index);
    
    return HSRV_E_NONE;
}


#define _____GROUPDB_INIT_____

int32
hsrv_openflow_groupdb_start(void)
{
    if (NULL == g_pst_openflow_ttp_group_master)
    {
        g_pst_openflow_ttp_group_master = HSRV_OPENFLOW_GROUP_MALLOC(sizeof(hsrv_openflow_ttp_group_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_group_master);

        HSRV_OPENFLOW_TTP_GROUP_HASH = ctclib_hash_create(_hsrv_openflow_groupdb_group_make,
                                                      _hsrv_openflow_groupdb_group_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_GROUP_HASH);

        HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH = ctclib_hash_create(_hsrv_openflow_groupdb_group_bucket_make,
                                                      _hsrv_openflow_groupdb_group_bucket_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_TTP_GROUP_BUCKET_HASH);

        g_pst_openflow_ttp_group_master->pst_group_bucket_index_opf = ctclib_opf_init(1);
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_ttp_group_master->pst_group_bucket_index_opf);
        ctclib_opf_init_offset(g_pst_openflow_ttp_group_master->pst_group_bucket_index_opf, 0, HSRV_MIN_BUCKET_INDEX, HSRV_MAX_BCUKET_INDEX);
    }
    
    return HSRV_E_NONE;
}

