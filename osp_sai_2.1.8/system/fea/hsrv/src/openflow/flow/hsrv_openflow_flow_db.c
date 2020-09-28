
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_msg_openflow.h"
#include "hsrv_openflow_nexthop.h"
#include "hsrv_openflow_flow.h"
#include "hsrv_openflow_flow_priv.h"

extern hsrv_openflow_flow_master_t *g_pst_openflow_flow_master;

struct hsrv_openflow_flow_iterate_data_s
{
    int32 (*fn)(hsrv_openflow_flow_t* p_flow);
};
typedef struct hsrv_openflow_flow_iterate_data_s hsrv_openflow_flow_iterate_data_t;

#define _____FLOW_HASH_____

static inline uint32
hsrv_openflow_flowdb_flow_make(hsrv_openflow_flow_t* p_flow)
{
    return p_flow->flow.flow_id;
}

static bool
hsrv_openflow_flowdb_flow_compare(hsrv_openflow_flow_t* pva, hsrv_openflow_flow_t* pvb)
{
    if (pva->flow.flow_id != pvb->flow.flow_id)
    {
        return FALSE;
    }

    return TRUE;
}

int32
hsrv_openflow_flowdb_add_flow(hsrv_openflow_flow_t* p_flow)
{
    ctclib_hash_get(HSRV_OPENFLOW_FLOW_HASH, p_flow, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_lookup_flow(uint32 flow_id, hsrv_openflow_flow_t** pp_flow)
{
    hsrv_openflow_flow_t lookup;

    lookup.flow.flow_id = flow_id;

    *pp_flow = (hsrv_openflow_flow_t*)ctclib_hash_lookup(HSRV_OPENFLOW_FLOW_HASH, &lookup);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_del_flow(hsrv_openflow_flow_t* p_flow)
{
    ctclib_hash_release(HSRV_OPENFLOW_FLOW_HASH, p_flow);
    
    return HSRV_E_NONE;
}

static void 
hsrv_openflow_flowdb_loop_flow__(ctclib_hash_backet_t* phb, void *user_data)
{
    hsrv_openflow_flow_t* p_flow = (hsrv_openflow_flow_t*)phb->data;
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
    
    ctclib_hash_iterate(HSRV_OPENFLOW_FLOW_HASH, hsrv_openflow_flowdb_loop_flow__, &data);

    return HSRV_E_NONE;
}

#define _____FLOW_ACTION_LIST_HASH_____

static inline uint32
hsrv_openflow_flowdb_actlist_make(hsrv_actlist_info_t* p_flow)
{
    uint32 hash = 0;
    int32 count = 0;
    
    ofp_hash_key_init(&hash);
    
    for (count = 0; count < p_flow->actlist.action_count; count ++)
    {
        glb_openflow_action_hash(&hash, p_flow->actlist.actlist + count);
    }
    return hash;
}

static bool
hsrv_openflow_flowdb_actlist_compare(hsrv_actlist_info_t* pva, hsrv_actlist_info_t* pvb)
{
    glb_openflow_flow_action_t *p_action1;
    glb_openflow_flow_action_t *p_action2;
    bool has_skip = FALSE;
    int32 count1 = 0;
    int32 count2 = 0;

    if (count1 == pva->actlist.action_count ||
        count2 == pvb->actlist.action_count)
    {
        return FALSE;
    }

    while (count1 != pva->actlist.action_count &&
           count2 != pvb->actlist.action_count) 
    {
        p_action1 = pva->actlist.actlist + count1;
        p_action2 = pvb->actlist.actlist + count2;

        has_skip = FALSE;
        /* skip group, meter, queue */
        if (p_action1->type == GLB_OPENFLOW_ACTION_TYPE_GROUP ||
            p_action1->type == GLB_OPENFLOW_ACTION_TYPE_INST_METER ||
            p_action1->type == GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE)
        {
            count1 ++;
            has_skip = TRUE;
        }

        if (p_action2->type == GLB_OPENFLOW_ACTION_TYPE_GROUP ||
            p_action2->type == GLB_OPENFLOW_ACTION_TYPE_INST_METER ||
            p_action2->type == GLB_OPENFLOW_ACTION_TYPE_SET_QUEUE)
        {
            count2 ++;
            has_skip = TRUE;
        }
        if (has_skip)
        {
            continue;
        }
        if (!glb_openflow_action_is_same(p_action1, p_action2))
        {
            return FALSE;
        }

        if (GLB_OPENFLOW_ACTION_TYPE_OUTPUT == p_action1->type)
        {
            if (p_action1->value.output.ofport == GLB_OFPP_CONTROLLER &&
                (pva->mdata.table_id               != pvb->mdata.table_id ||
                 pva->mdata.is_table_mis           != pvb->mdata.is_table_mis ||
                 p_action1->value.output.max_len != p_action2->value.output.max_len))
            {
                return FALSE;
            }
        }
        /* Both ipv4 and ipv6 have these actions, so we must make sure
         * the ethe-type of match materials is same */
        if (GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_TCP_SRC == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_TCP_DST == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_UDP_SRC == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_UDP_DST == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_DEC_IP_TTL == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_IP_ECN == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_IP_DSCP == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_DEC_TTL == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_IP_TTL == p_action1->type)
        {
            if (pva->mdata.eth_type != pvb->mdata.eth_type)
            {
                return FALSE;
            }
        }
        /* Both TCP and UDP have these actions, so we must make sure
         * the ip-proto of match materials is same */
        if (GLB_OPENFLOW_ACTION_TYPE_SET_TP_SRC == p_action1->type ||
            GLB_OPENFLOW_ACTION_TYPE_SET_TP_DST == p_action1->type)
        {
            if (pva->mdata.ip_proto != pvb->mdata.ip_proto)
            {
                return FALSE;
            }
        }

        count1 ++;
        count2 ++;
    }

    /* both lists must reach end */
    if (count1 != pva->actlist.action_count ||
        count2 != pvb->actlist.action_count)
    {
        return FALSE;
    }

    return TRUE;    
}

int32
hsrv_openflow_flowdb_lookup_flow_actlist(hsrv_openflow_flow_t *p_flow, hsrv_actlist_info_t** pp_actions_info)
{
    hsrv_actlist_info_t lookup;

    sal_memset(&lookup, 0, sizeof(hsrv_actlist_info_t));
    sal_memcpy(&lookup.actlist, &p_flow->flow.actlist, sizeof(glb_openflow_flow_action_list_t));
    sal_memcpy(&lookup.mdata.in_port, &p_flow->flow.key.in_port, sizeof(glb_openflow_inport_t));

    lookup.mdata.table_id = p_flow->flow.table;
    if (p_flow->flow.key.mac.flags.ipv4_packet)
    {
        lookup.mdata.eth_type = ETH_TYPE_IP;
        lookup.mdata.ip_proto = p_flow->flow.key.rule.ipv4_rule.l4_protocol;
    }
    else if (p_flow->flow.key.mac.flags.arp_packet)
    {
        lookup.mdata.eth_type = ETH_TYPE_ARP;
    }
    else
    {
        lookup.mdata.eth_type = p_flow->flow.key.mac.eth_type;
    }

    *pp_actions_info = ctclib_hash_lookup(HSRV_OPENFLOW_ACTLIST_HASH, &lookup);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_add_flow_actlist(hsrv_actlist_info_t* p_actions_info)
{
    ctclib_hash_get(HSRV_OPENFLOW_ACTLIST_HASH, p_actions_info, ctclib_hash_alloc_intern);
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_flowdb_del_flow_actlist(hsrv_actlist_info_t* p_actions_info)
{
    ctclib_hash_release(HSRV_OPENFLOW_ACTLIST_HASH, p_actions_info);
    
    return HSRV_E_NONE;
}

/* Added by weizj for tunnel bug 33724 */
int32
hsrv_openflow_flowdb_set_drop_ingress(uint8 is_drop)
{
    HSRV_OPENFLOW_FLOW_DROP_INGRESS = is_drop;

    return HSRV_E_NONE;
}

bool
hsrv_openflow_flowdb_get_drop_ingress(void)
{
    if (HSRV_OPENFLOW_FLOW_DROP_INGRESS)
    {
        return TRUE;
    }

    return FALSE;
}
/* End by weizj for tunnel */

/* Added by weizj for udf */
int32
hsrv_openflow_flowdb_udf_enable(uint8 enable)
{
    HSRV_OPENFLOW_FLOW_UDF_ENABLE = enable;

    return HSRV_E_NONE;
}

#define _____INIT_____

int32
hsrv_openflow_flowdb_start(void)
{
    HSRV_OPENFLOW_FLOW_HASH = ctclib_hash_create(
        hsrv_openflow_flowdb_flow_make,
        hsrv_openflow_flowdb_flow_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_FLOW_HASH);

    HSRV_OPENFLOW_ACTLIST_HASH = ctclib_hash_create(
        hsrv_openflow_flowdb_actlist_make,
        hsrv_openflow_flowdb_actlist_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_ACTLIST_HASH);

    /* Added by weizj for tunnel bug 33724 */
    /* Modifed by weizj for bug 35135 */
    HSRV_OPENFLOW_FLOW_DROP_INGRESS = 0;
    /* Added by weizj for udf */
    HSRV_OPENFLOW_FLOW_UDF_ENABLE = 0;

    return HSRV_E_NONE;
}

#define _____END_LINE_____
