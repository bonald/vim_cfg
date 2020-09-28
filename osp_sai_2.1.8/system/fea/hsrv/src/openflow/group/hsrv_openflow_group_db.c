#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_nexthop.h"
#include "hsrv_openflow_group_priv.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"

/*************************
 * Global and Declaration
 *************************/
hsrv_openflow_group_master_t *g_pst_openflow_group_master = NULL;


#define _____GROUPDB_HASH_APIS_____
/*************************************************************************************************
 * Name         : _hsrv_openflow_groupdb_group_make
 * Purpose      : Fea layer groupdb hash make
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : uint32
**************************************************************************************************/
static inline uint32
_hsrv_openflow_groupdb_group_make(hsrv_openflow_group_t* p_group)
{
    return p_group->group.group_id;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_groupdb_group_compare
 * Purpose      : Fea layer groupdb hash compare
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
static inline bool
_hsrv_openflow_groupdb_group_compare(hsrv_openflow_group_t* pva, hsrv_openflow_group_t* pvb)
{
    if (pva->group.group_id != pvb->group.group_id)
    {
        return FALSE;
    }

    return TRUE;
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_groupdb_loop_watch_port
 * Purpose      : Fea layer groupdb loop group watch port
 * Input        : phb: ctclib_hash_backet_t*, user_data: void*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void 
_hsrv_openflow_groupdb_loop_watch_port(ctclib_hash_backet_t* phb, void *user_data)
{
    uint32 bucket_idx;
    uint32 n_buckets;
    glb_openflow_bucket_t* bucket = NULL;
    hsrv_openflow_group_nexthop_info_t* p_nh_info = NULL;
    /* modified by liwh for bug 54363, 2019-11-13 */
    uint32 max_bucket = 0;
    /* liwh end */

    hsrv_openflow_group_t* p_group = (hsrv_openflow_group_t*)phb->data;
    hsrv_openflow_group_watch_port_lookup_t* p_data = user_data;

    tbl_openflow_t *p_openflow = tbl_openflow_get_openflow();

    if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
    {
        if (!p_data->link_up)
        {
            if (p_group->group.choose_bucket_idx != UINT32_MAX)
            {
                bucket = &p_group->group.bucket_list[p_group->group.choose_bucket_idx];
                if (p_data->ifindex == bucket->watch_port)
                {
                    p_data->groups[p_data->group_count] = p_group->group.group_id;
                    p_data->group_count ++;
                }
            }
        }
        else
        {
            /* if ffgroup_switchback_en or no choosed bucket, do switchback */
            if (p_openflow->ffgroup_switchback_en || p_group->group.choose_bucket_idx == UINT32_MAX)
            {
                /* modified by liwh for bug 54363, 2019-11-13 
                   if no interface is up, max_bucket is all; if there is choosed bucket, only should choose former bucket */
                if (p_group->group.n_buckets < p_group->group.choose_bucket_idx)
                {
                    max_bucket  = p_group->group.n_buckets;
                }
                else
                {
                    max_bucket  = p_group->group.choose_bucket_idx;
                }
                /* liwh end */
                     
                /* loop to choose_bucket_idx is enough */
                /* modified by liwh for bug 54363, 2019-11-13 */
                //for (bucket_idx = 0; bucket_idx < p_group->group.choose_bucket_idx; bucket_idx ++)
                for (bucket_idx = 0; bucket_idx < max_bucket; bucket_idx ++)
                /* liwh end */
                {
                    bucket = &p_group->group.bucket_list[bucket_idx];
                    if (p_data->ifindex == bucket->watch_port)
                    {
                        p_data->groups[p_data->group_count] = p_group->group.group_id;
                        p_data->group_count ++;
                        /* Added by weizj for bug 37070*/
                        break;
                    }
                }
            }
        }
    }
    /* Added by weizj for bug 36895 */
    else if (GLB_OF_GROUP_TYPE_SELECT == p_group->group.group_type)
    {
        p_nh_info = &p_group->u.dft.nh_info;
        n_buckets = p_group->group.n_buckets;
        for (bucket_idx = 0; bucket_idx < n_buckets; bucket_idx ++)
        {
            if (0 == p_group->group.bucket_list[bucket_idx].n_actions)
            {
                continue;
            }

            if (p_data->ifindex == p_nh_info->buckets[bucket_idx].member_nh[0].ifindex)
            {
                p_data->groups[p_data->group_count] = p_group->group.group_id;
                p_data->group_count ++;
                /* Added by weizj for bug 37070*/
                break;
            }
        }  
    }
}

/*************************************************************************************************
 * Name         : _hsrv_openflow_groupdb_loop_update_ffgroup
 * Purpose      : Fea layer groupdb loop group watch port
 * Input        : phb: ctclib_hash_backet_t*, user_data: void*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
static void 
_hsrv_openflow_groupdb_loop_update_ffgroup(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_group_t* p_group = (hsrv_openflow_group_t*)phb->data;

    if (GLB_OF_GROUP_TYPE_FF == p_group->group.group_type)
    {
        _hsrv_openflow_group_update_ff_group_liveness(p_group);
    }
}


#define _____GROUPDB_APIS_____
/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_add_group
 * Purpose      : Fea layer groupdb add group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_add_group(hsrv_openflow_group_t* p_group)
{
    ctclib_hash_get(HSRV_OPENFLOW_GROUP_HASH, p_group, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_del_group
 * Purpose      : Fea layer groupdb delete group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_del_group(hsrv_openflow_group_t* p_group)
{
    ctclib_hash_release(HSRV_OPENFLOW_GROUP_HASH, p_group);
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_lookup_group
 * Purpose      : Fea layer groupdb lookup group
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_lookup_group(uint32 group_id, hsrv_openflow_group_t** pp_group)
{
    hsrv_openflow_group_t group_lkp;

    sal_memset(&group_lkp, 0, sizeof(hsrv_openflow_group_t));
    group_lkp.group.group_id = group_id;
 
    *pp_group = (hsrv_openflow_group_t*)ctclib_hash_lookup(HSRV_OPENFLOW_GROUP_HASH, &group_lkp);
    
    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_loop_watch_port
 * Purpose      : Fea layer groupdb loop group watch port
 * Input        : p_data: hsrv_openflow_group_watch_port_lookup_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_loop_watch_port(hsrv_openflow_group_watch_port_lookup_t* p_data)
{ 
    ctclib_hash_iterate(HSRV_OPENFLOW_GROUP_HASH, _hsrv_openflow_groupdb_loop_watch_port, p_data);

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_loop_update_ffgroup
 * Purpose      : Fea layer groupdb loop group update ffgroup bucket
 * Input        : N/A
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_loop_update_ffgroup()
{
    void *p_data = NULL;
    ctclib_hash_iterate(HSRV_OPENFLOW_GROUP_HASH, _hsrv_openflow_groupdb_loop_update_ffgroup, p_data);

    return HSRV_E_NONE;
}


/* Added by weizj for bug 35135 */
/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_set_drop_ingress
 * Purpose      : Fea layer groupdb set drop ingress
 * Input        : is_drop: uint8
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_set_drop_ingress(uint8 is_drop)
{
    HSRV_OPENFLOW_GROUP_DROP_INGRESS = is_drop;

    return HSRV_E_NONE;
}

/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_get_drop_ingress
 * Purpose      : Fea layer groupdb get drop ingress
 * Input        : N/A
 * Output       : N/A 
 * Return       : bool
**************************************************************************************************/
bool
hsrv_openflow_groupdb_get_drop_ingress(void)
{
    if (HSRV_OPENFLOW_GROUP_DROP_INGRESS)
    {
        return TRUE;
    }

    return FALSE;
}
/* End by weizj for bug 35135 */

#define _____GROUPDB_INIT_____
/*************************************************************************************************
 * Name         : hsrv_openflow_groupdb_start
 * Purpose      : Fea layer groupdb start
 * Input        : p_group: hsrv_openflow_group_t*
 * Output       : N/A 
 * Return       : HSRV_E_XXX
**************************************************************************************************/
int32
hsrv_openflow_groupdb_start(void)
{
    if (NULL == g_pst_openflow_group_master)
    {
        g_pst_openflow_group_master = HSRV_OPENFLOW_GROUP_CALLOC(sizeof(hsrv_openflow_group_master_t));
        HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_group_master);

        HSRV_OPENFLOW_GROUP_HASH = ctclib_hash_create(_hsrv_openflow_groupdb_group_make,
                                                      _hsrv_openflow_groupdb_group_compare);
        HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_GROUP_HASH);

        /* Added by weizj for bug 35135 */
        HSRV_OPENFLOW_GROUP_DROP_INGRESS = 0;
    }
    
    return HSRV_E_NONE;
}

