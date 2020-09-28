#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_policer.h>
#include <ctc_sai_debug.h>

#define CTC_SAI_POLICER_APPLY_DEFAULT 0xffffffff

#ifdef GREATBELT
#define CTC_SAI_POLICER_MAX_RATE_KBPS 10000000
#define CTC_SAI_POLICER_MAX_RATE_PPS 140000000
#else
#define CTC_SAI_POLICER_MAX_RATE_KBPS 100000000
#define CTC_SAI_POLICER_MAX_RATE_PPS 140000000
#endif
static ctc_sai_policer_info_t  g_sai_policer_info;
static sai_uint32_t ctc_sai_policer_is_in[4096];

/* used by openflow */
void ctc_sai_policer_alloc_policer_id(uint32* p_policer_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctclib_opb_alloc_offset(&p_sai_glb->qos_policer_opb, p_policer_id);
}

void ctc_sai_policer_release_policer_id(uint32 policer_id)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctclib_opb_free_offset(&p_sai_glb->qos_policer_opb, policer_id);
}

ctc_sai_policer_port_index_t*
ctc_sai_policer_port_index_get_by_gport(uint32 gport)
{
    ctc_sai_policer_port_index_t policer_port_index;

    sal_memset(&policer_port_index, 0, sizeof(policer_port_index));
    policer_port_index.gport = gport;
    return ctc_hash_lookup(g_sai_policer_info.ctc_sai_policer_port_index_hash, &policer_port_index);
}
ctc_sai_policer_port_index_t*
ctc_sai_policer_port_index_add_hash(uint32 gport, uint32 policer_index)
{
    ctc_sai_policer_port_index_t* p_policer_port_index = NULL;

    p_policer_port_index = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_policer_port_index_t));
    if (NULL == p_policer_port_index)
    {
        return NULL;
    }
    p_policer_port_index->gport = gport;
    p_policer_port_index->ctc_sai_policer_port_index = policer_index;
    return ctc_hash_insert(g_sai_policer_info.ctc_sai_policer_port_index_hash, p_policer_port_index);
}

ctc_sai_egress_policer_port_index_t*
ctc_sai_egress_policer_port_index_get_by_gport(uint32 gport)
{
    ctc_sai_egress_policer_port_index_t policer_egress_port_index;

    sal_memset(&policer_egress_port_index, 0, sizeof(policer_egress_port_index));
    policer_egress_port_index.gport = gport;
    return ctc_hash_lookup(g_sai_policer_info.ctc_sai_egress_policer_port_index_hash, &policer_egress_port_index);
}
ctc_sai_egress_policer_port_index_t*
ctc_sai_egress_policer_port_index_add_hash(uint32 gport, uint32 policer_index)
{
    ctc_sai_egress_policer_port_index_t* p_egress_policer_port_index = NULL;

    p_egress_policer_port_index = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_egress_policer_port_index_t));
    if (NULL == p_egress_policer_port_index)
    {
        return NULL;
    }
    p_egress_policer_port_index->gport = gport;
    p_egress_policer_port_index->ctc_sai_egress_policer_port_index = policer_index;
    return ctc_hash_insert(g_sai_policer_info.ctc_sai_egress_policer_port_index_hash, p_egress_policer_port_index);
}

uint64
ctc_sai_policer_get_pir_by_policer_id(uint32 policer_id)
{
    ctc_sai_policer_t     *ppolicer = NULL;
    sai_attribute_t   *attr_value = NULL;
    
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    attr_value = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_PIR - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        return attr_value->value.u64;
    }
    
    return SAI_STATUS_SUCCESS;
}

uint64
ctc_sai_policer_get_cir_by_policer_id(uint32 policer_id)
{
    ctc_sai_policer_t     *ppolicer = NULL;
    sai_attribute_t   *attr_value = NULL;
    
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    attr_value = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_CIR - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        return attr_value->value.u64;
    }
    
    return SAI_STATUS_SUCCESS;
}

uint32
ctc_sai_policer_get_meter_type_by_policer_id(uint32 policer_id)
{
    ctc_sai_policer_t     *ppolicer = NULL;
    sai_attribute_t   *attr_value = NULL;
    
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    attr_value = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_METER_TYPE - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        return attr_value->value.s32;
    }
    
    return SAI_STATUS_SUCCESS;
}
uint32
ctc_sai_policer_get_mode_by_policer_id(uint32 policer_id)
{
    ctc_sai_policer_t     *ppolicer = NULL;
    sai_attribute_t   *attr_value = NULL;
    
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    attr_value = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_MODE - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        return attr_value->value.s32;
    }
    
    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_policer_get_port_policer_id(sai_object_id_t port_id, sai_attribute_t *attr)
{
    uint32_t ctc_port_id;
    ctc_sai_policer_port_index_t* p_policer_index;
    ctc_port_id = CTC_SAI_OBJECT_INDEX_GET(port_id);

    p_policer_index = ctc_sai_policer_port_index_get_by_gport(ctc_port_id);
    if(!p_policer_index)
    {
        return SAI_STATUS_FAILURE;
    }
    attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_policer_index->ctc_sai_policer_port_index);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_policer_get_port_egress_policer_id(sai_object_id_t port_id, sai_attribute_t *attr)
{
    uint32_t ctc_port_id;
    ctc_sai_egress_policer_port_index_t* p_egress_policer_index;
    ctc_port_id = CTC_SAI_OBJECT_INDEX_GET(port_id);

    p_egress_policer_index = ctc_sai_egress_policer_port_index_get_by_gport(ctc_port_id);
    if(!p_egress_policer_index)
    {
        return SAI_STATUS_FAILURE;
    }
    attr->value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_PORT, p_egress_policer_index->ctc_sai_egress_policer_port_index);

    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_policer_map_db_attribute(const sai_attribute_t *attr, ctc_sai_policer_t     *ppolicer)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t   *attr_value = NULL;
    sai_attribute_t *attr_list_db = NULL;
    attr_value = ctc_vector_get(ppolicer->policer_vector, attr->id - SAI_POLICER_ATTR_METER_TYPE);
    if(NULL == attr_value)
    {
        attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
        sal_memcpy(attr_list_db, attr, sizeof(sai_attribute_t));

        if(FALSE == ctc_vector_add(ppolicer->policer_vector, attr->id - SAI_POLICER_ATTR_METER_TYPE, attr_list_db))
        {
            ret = SAI_STATUS_FAILURE;
            return ret;
        }
    }
    else
    {
        sal_memcpy(attr_value, attr, sizeof(sai_attribute_t));
    }

    return ret;
}

sai_status_t
ctc_sai_policer_db_map_policer(ctc_sai_policer_t* psai_policer, ctc_qos_policer_t* pctc_policer)
{
    sai_attribute_t   *attr_value = NULL;
#ifdef USW
    uint32_t drop_set = 0;
#endif
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_METER_TYPE - SAI_POLICER_ATTR_METER_TYPE);
    if (attr_value)
    {
    #ifdef USW
        if(SAI_METER_TYPE_PACKETS == attr_value->value.s32)
        {
            pctc_policer->pps_en = 1;
        }
    #else
        if(SAI_METER_TYPE_PACKETS == attr_value->value.s32)
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
    #endif
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_MODE - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if(SAI_POLICER_MODE_Sr_TCM == attr_value->value.s32)
        {
            #if defined(GOLDENGATE) || defined(GREATBELT)
            pctc_policer->policer.is_srtcm = 1;    
            #endif
            pctc_policer->policer.policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
        }
        else if(SAI_POLICER_MODE_Tr_TCM == attr_value->value.s32)
        {
            #if defined(GOLDENGATE) || defined(GREATBELT)
            pctc_policer->policer.is_srtcm = 0;
            #endif
            pctc_policer->policer.policer_mode = CTC_QOS_POLICER_MODE_RFC4115;
        }
        else if(SAI_POLICER_MODE_STORM_CONTROL == attr_value->value.s32)
        {
        }
        else
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }       
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_COLOR_SOURCE - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if(SAI_POLICER_COLOR_SOURCE_BLIND == attr_value->value.s32)
        {
            pctc_policer->policer.is_color_aware = 0;
        }
        else
        {
            pctc_policer->policer.is_color_aware = 1;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_CBS - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
    #ifdef USW
        if (pctc_policer->pps_en)
        {
            pctc_policer->policer.cbs = attr_value->value.u64;
        }
        else
        {
            pctc_policer->policer.cbs = attr_value->value.u64 * 8/1000;
        }
    #else
        pctc_policer->policer.cbs = attr_value->value.u64 * 8/1000;
    #endif
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_CIR - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
     #ifdef USW
        if (pctc_policer->pps_en)
        {
            pctc_policer->policer.cir = attr_value->value.u64;
            if (pctc_policer->policer.cir > 140000000)
            {
                pctc_policer->policer.cir = 140000000;
            }
        }
        else
        {
            pctc_policer->policer.cir = attr_value->value.u64 * 8/1000;
            /* if cir > 100G, pps mode change to bps mode */
            if (pctc_policer->policer.cir > CTC_SAI_POLICER_MAX_RATE_KBPS)
            {
                pctc_policer->policer.cir = CTC_SAI_POLICER_MAX_RATE_KBPS;
            }
        }
    #else
        pctc_policer->policer.cir = attr_value->value.u64 * 8/1000;
    #endif
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_PBS - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
    #ifdef USW
        if (pctc_policer->pps_en)
        {
            pctc_policer->policer.pbs = attr_value->value.u64;
        }
        else
        {
            pctc_policer->policer.pbs = attr_value->value.u64 * 8/1000;
        }
    #else
        /*PBS(RFC2698,)or EBS (RFC2697, RFC4115,BWP)(unit :kb)*/
        pctc_policer->policer.pbs = attr_value->value.u64 * 8/1000;
    #endif
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_PIR - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
    #ifdef USW
        if (pctc_policer->pps_en)
        {
            pctc_policer->policer.pir = attr_value->value.u64;
            if (pctc_policer->policer.pir > CTC_SAI_POLICER_MAX_RATE_PPS)
            {
                pctc_policer->policer.pir = CTC_SAI_POLICER_MAX_RATE_PPS;
            }
        }
        else
        {
            pctc_policer->policer.pir = attr_value->value.u64 * 8/1000;
            /* if pir > 100G, pps mode change to bps mode */
            if (pctc_policer->policer.pir > CTC_SAI_POLICER_MAX_RATE_KBPS)
            {
                pctc_policer->policer.pir = CTC_SAI_POLICER_MAX_RATE_KBPS;
            }
        }
    #else
        /*PIR(RFC2698) or EIR(RFC4115,BWP) (unit :kbps)*/
        pctc_policer->policer.pir = attr_value->value.u64 * 8/1000;
    #endif
    }
#ifdef USW
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_GREEN_PACKET_ACTION - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        drop_set = 1;
        if (SAI_PACKET_ACTION_DROP == attr_value->value.s32)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN;
        }
        else if (SAI_PACKET_ACTION_FORWARD == attr_value->value.s32)
        {
            
        }
        else
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_YELLOW_PACKET_ACTION - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        drop_set = 1;
        if (SAI_PACKET_ACTION_DROP == attr_value->value.s32)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW;
        }
        else if (SAI_PACKET_ACTION_FORWARD == attr_value->value.s32)
        {
            
        }
        else
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_RED_PACKET_ACTION - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        drop_set = 1;
        if (SAI_PACKET_ACTION_DROP == attr_value->value.s32)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_DROP_RED;
        }
        else if (SAI_PACKET_ACTION_FORWARD == attr_value->value.s32)
        {
            
        }
        else
        {
            return SAI_STATUS_NOT_SUPPORTED;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_GREEN_PACKET_TC - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if (0xFF != attr_value->value.u8)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_GREEN_VALID;
            pctc_policer->action.prio_green = attr_value->value.u8*2;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_YELLOW_PACKET_TC - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if (0xFF != attr_value->value.u8)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_YELLOW_VALID;
            pctc_policer->action.prio_yellow = attr_value->value.u8*2;
        }
    }
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_RED_PACKET_TC - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if (0xFF != attr_value->value.u8)
        {
            pctc_policer->action.flag |= CTC_QOS_POLICER_ACTION_FLAG_PRIORITY_RED_VALID;
            pctc_policer->action.prio_red = attr_value->value.u8*2;
        }
    }
#endif
    attr_value = ctc_vector_get(psai_policer->policer_vector, SAI_POLICER_ATTR_ENABLE_COUNTER_LIST - SAI_POLICER_ATTR_METER_TYPE);
    if(attr_value)
    {
        if(attr_value->value.s32)
        {
            pctc_policer->policer.stats_en = 1;
        }
        else
        {
            pctc_policer->policer.stats_en = 0;
        }
    }
    /*set the default value for ctc policer*/
#ifdef DUET2
    /* duet2 action drop > drop_color */
    if (drop_set)
    {
        if (CTC_FLAG_ISSET(pctc_policer->action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_GREEN) || 
            CTC_FLAG_ISSET(pctc_policer->action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_YELLOW) ||
            CTC_FLAG_ISSET(pctc_policer->action.flag, CTC_QOS_POLICER_ACTION_FLAG_DROP_RED))
        {
            
        }
        else
        {
            /* all color packet transmit */
            pctc_policer->policer.drop_color = CTC_QOS_COLOR_NONE;
        }
    }
    else
    {
        /* default drop color is red */
        pctc_policer->policer.drop_color = CTC_QOS_COLOR_RED;
    }
#else
    pctc_policer->policer.drop_color = CTC_QOS_COLOR_RED;
#endif
    pctc_policer->policer.use_l3_length = 0;

    return SAI_STATUS_SUCCESS;
    
}

static uint32_t
_policer_port_index_hash_make(
    _In_  void* data)
{
    ctc_sai_policer_port_index_t* p_policer_port_index = (ctc_sai_policer_port_index_t*)data;

    return ctc_hash_caculate(sizeof(uint32), &p_policer_port_index->gport);
}

static bool
_policer_port_index_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_policer_port_index_t* p_policer_port_index_key = data;
    ctc_sai_policer_port_index_t* p_policer_port_index_key1 = data1;

    if ((p_policer_port_index_key->gport) == (p_policer_port_index_key1->gport))
    {
        return TRUE;
    }

    return FALSE;
}

static uint32_t
_egress_policer_port_index_hash_make(
    _In_  void* data)
{
    ctc_sai_egress_policer_port_index_t* p_egress_policer_port_index = (ctc_sai_egress_policer_port_index_t*)data;

    return ctc_hash_caculate(sizeof(uint32), &p_egress_policer_port_index->gport);
}

static bool
_egress_policer_port_index_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_egress_policer_port_index_t* p_egress_policer_port_index_key = data;
    ctc_sai_egress_policer_port_index_t* p_egress_policer_port_index_key1 = data1;

    if ((p_egress_policer_port_index_key->gport) == (p_egress_policer_port_index_key1->gport))
    {
        return TRUE;
    }

    return FALSE;
}

sai_status_t
ctc_sai_policer_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    p_sai_glb->qos_policer_max_count = 65535;
    g_sai_policer_info.pvector = ctc_vector_init(300,300);
    if(NULL == g_sai_policer_info.pvector)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    if (0 != ctclib_opb_create(&p_sai_glb->qos_policer_opb, 1, p_sai_glb->qos_policer_max_count, "policer_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    g_sai_policer_info.counter_max = 2048; /*need check the number*/
    g_sai_policer_info.pcounter_vector = ctc_vector_init(32,64);

    if(NULL == g_sai_policer_info.pcounter_vector)
    {
        return SAI_STATUS_NO_MEMORY;
    }
    sal_memset(ctc_sai_policer_is_in, 0x0, sizeof(ctc_sai_policer_is_in));

    g_sai_policer_info.ctc_sai_policer_port_index_hash = ctc_hash_create(64, 32, _policer_port_index_hash_make, _policer_port_index_hash_cmp);
    g_sai_policer_info.ctc_sai_egress_policer_port_index_hash = ctc_hash_create(64, 32, _egress_policer_port_index_hash_make, _egress_policer_port_index_hash_cmp);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_policer_db_alloc_policer(ctc_sai_policer_t** pppolicer)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_policer_t *ppolicer = NULL;
    uint32_t            index     = 0;

    if(g_sai_policer_info.pvector->used_cnt >= p_sai_glb->qos_policer_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }

    ppolicer = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_policer_t));

    if(ppolicer)
    {
        sal_memset(ppolicer,0,sizeof(ctc_sai_policer_t));
        ctclib_opb_alloc_offset(&p_sai_glb->qos_policer_opb, &index);
        ppolicer->policer_id = index;
        ppolicer->used_entry_id = CTC_SAI_POLICER_APPLY_DEFAULT;
        ppolicer->used_port_id = CTC_SAI_POLICER_APPLY_DEFAULT;
    }

    *pppolicer = ppolicer;

    return SAI_STATUS_SUCCESS;
}

void
ctc_sai_policer_db_release_policer(ctc_sai_policer_t *ppolicer)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_uint32_t index = 0;
    sai_attribute_t   *attr_value = NULL;
    if(NULL == ppolicer)
    {
        return;
    }
    ctclib_opb_free_offset(&p_sai_glb->qos_policer_opb, ppolicer->policer_id);
    for (index = SAI_POLICER_ATTR_METER_TYPE; index <= SAI_POLICER_ATTR_ENABLE_COUNTER_LIST; index++)
    {
        attr_value = ctc_vector_get(ppolicer->policer_vector, index - SAI_POLICER_ATTR_METER_TYPE);
        if (attr_value)
        {
            XFREE(MEM_FEA_TEMP_TODO, attr_value);
            attr_value = NULL;
            ctc_vector_del(ppolicer->policer_vector, index - SAI_POLICER_ATTR_METER_TYPE);
        }
    }
    
    ctc_vector_release(ppolicer->policer_vector);
    XFREE(MEM_FEA_TEMP_TODO, ppolicer);
}
sai_status_t ctc_sai_policer_port_set_policer(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  policer_id = 0;
    ctc_sai_policer_t     *ppolicer = NULL;
    ctc_qos_policer_t  ctc_policer;
    ctc_sai_policer_port_index_t* p_policer_index = NULL;

    policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    sal_memset(&ctc_policer, 0x0, sizeof(ctc_qos_policer_t));

    if(SAI_NULL_OBJECT_ID != policer_id)
    {
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if((CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id) || (CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id))
        {
            return SAI_STATUS_OBJECT_IN_USE;
        }
        p_policer_index = ctc_sai_policer_port_index_get_by_gport((CTC_SAI_OBJECT_INDEX_GET(port_id)));
        if(!p_policer_index)
        {
            p_policer_index = ctc_sai_policer_port_index_add_hash((CTC_SAI_OBJECT_INDEX_GET(port_id)), 0);
            p_policer_index->ctc_sai_policer_port_index = 0;
        }
        if(p_policer_index->ctc_sai_policer_port_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        ret = ctc_sai_policer_db_map_policer(ppolicer, &ctc_policer);
        if(SAI_STATUS_SUCCESS != ret)
        {
            return ret;
        }
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.enable = 1;
        ctc_policer.id.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;

        ppolicer->used_port_id = CTC_SAI_OBJECT_INDEX_GET(port_id);
        p_policer_index->ctc_sai_policer_port_index = ppolicer->policer_id;
        ctc_sai_policer_is_in[policer_id] = TRUE;
    }
    else
    {
        p_policer_index = ctc_sai_policer_port_index_get_by_gport((CTC_SAI_OBJECT_INDEX_GET(port_id)));
        if(!p_policer_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        policer_id = p_policer_index->ctc_sai_policer_port_index;
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if(ppolicer->used_port_id != CTC_SAI_OBJECT_INDEX_GET(port_id))
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.enable = 0;
        ctc_policer.id.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;

        ppolicer->used_port_id = CTC_SAI_POLICER_APPLY_DEFAULT;
        p_policer_index->ctc_sai_policer_port_index = 0;
        ctc_sai_policer_is_in[policer_id] = FALSE;
    }

     ret = ctc_qos_set_policer(&ctc_policer);
 
     return ret;
}

sai_status_t ctc_sai_policer_port_set_egress_policer(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    uint32_t  policer_id = 0;
    ctc_sai_policer_t     *ppolicer = NULL;
    ctc_qos_policer_t  ctc_policer;
    ctc_sai_egress_policer_port_index_t* p_egress_policer_index = NULL;

    policer_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);
    sal_memset(&ctc_policer, 0x0, sizeof(ctc_qos_policer_t));

    if(SAI_NULL_OBJECT_ID != policer_id)
    {
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if((CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id) || (CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id))
        {
            return SAI_STATUS_OBJECT_IN_USE;
        }
        p_egress_policer_index = ctc_sai_egress_policer_port_index_get_by_gport((CTC_SAI_OBJECT_INDEX_GET(port_id)));
        if(!p_egress_policer_index)
        {
            p_egress_policer_index = ctc_sai_egress_policer_port_index_add_hash((CTC_SAI_OBJECT_INDEX_GET(port_id)), 0);
            p_egress_policer_index->ctc_sai_egress_policer_port_index = 0;
        }
        if(p_egress_policer_index->ctc_sai_egress_policer_port_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        ret = ctc_sai_policer_db_map_policer(ppolicer, &ctc_policer);
        if(SAI_STATUS_SUCCESS != ret)
        {
            return ret;
        }
        ctc_policer.dir = CTC_EGRESS;
        ctc_policer.enable = 1;
        ctc_policer.id.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;

        ppolicer->used_port_id = CTC_SAI_OBJECT_INDEX_GET(port_id);
        p_egress_policer_index->ctc_sai_egress_policer_port_index = ppolicer->policer_id;
    }
    else
    {
        p_egress_policer_index = ctc_sai_egress_policer_port_index_get_by_gport((CTC_SAI_OBJECT_INDEX_GET(port_id)));
        if(!p_egress_policer_index)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        policer_id = p_egress_policer_index->ctc_sai_egress_policer_port_index;
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if(ppolicer->used_port_id != CTC_SAI_OBJECT_INDEX_GET(port_id))
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        
        ctc_policer.dir = CTC_EGRESS;
        ctc_policer.enable = 0;
        ctc_policer.id.gport = CTC_SAI_OBJECT_INDEX_GET(port_id);
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;

        ppolicer->used_port_id = CTC_SAI_POLICER_APPLY_DEFAULT;
        p_egress_policer_index->ctc_sai_egress_policer_port_index = 0;
    }

     ret = ctc_qos_set_policer(&ctc_policer);
 
     return ret;
}

sai_status_t ctc_sai_policer_acl_set_policer(uint32_t policer_id, uint32_t entry_id, uint32_t enable)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_policer_t     *ppolicer = NULL;
    ctc_qos_policer_t  ctc_policer;

    sal_memset(&ctc_policer, 0x0, sizeof(ctc_qos_policer_t));

    if(enable)
    {
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if((CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id) || (CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id))
        {
            return SAI_STATUS_OBJECT_IN_USE;
        }

        ret = ctc_sai_policer_db_map_policer(ppolicer, &ctc_policer);
        if(SAI_STATUS_SUCCESS != ret)
        {
            return ret;
        }
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.enable = 1;
        ctc_policer.id.policer_id = policer_id;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;

        ppolicer->used_entry_id = entry_id;
    }
    else
    {
        ppolicer = ctc_vector_get(g_sai_policer_info.pvector, policer_id);
        if(NULL == ppolicer)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        if(ppolicer->used_entry_id != entry_id)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.enable = 0;
        ctc_policer.id.policer_id = policer_id;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;

        ppolicer->used_entry_id = CTC_SAI_POLICER_APPLY_DEFAULT;
    }

     ret = ctc_qos_set_policer(&ctc_policer);
 
     return ret;
}

#define ________SAI_POLICER_DEBUG_FUNC
sai_status_t 
sai_get_policer_statistics_debug_param(
    _In_ sai_object_id_t policer_id,
    _In_ const sai_policer_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    )
{
    uint32_t         attr_idx    = 0;
    CTC_SAI_DEBUG("in:policer_id 0x%llx number_of_counters %u", policer_id, number_of_counters);

    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
            case SAI_POLICER_STAT_PACKETS:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_ATTR_BYTES:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_ATTR_BYTES %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_GREEN_PACKETS:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_GREEN_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_GREEN_BYTES:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_GREEN_BYTES %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_YELLOW_PACKETS:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_YELLOW_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_YELLOW_BYTES:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_YELLOW_BYTES %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_RED_PACKETS:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_RED_PACKETS %"PRIu64, counters[attr_idx]);
                break;
            case SAI_POLICER_STAT_RED_BYTES:
                CTC_SAI_DEBUG("out:SAI_POLICER_STAT_RED_BYTES %"PRIu64, counters[attr_idx]);
                break;
            default:
                break;
        }
    }
    return SAI_STATUS_SUCCESS;
}
#define ________SAI_POLICER_API_FUNC

sai_status_t sai_create_policer(
    _Out_ sai_object_id_t *policer_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_policer_t     *ppolicer = NULL;
    sai_attribute_t *attr_list_db = NULL;
    ctc_qos_policer_t  ctc_policer;
    sai_uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(policer_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&ctc_policer, 0x0, sizeof(ctc_qos_policer_t));

    ret = ctc_sai_policer_db_alloc_policer(&ppolicer);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /*if the attr changed, this init block_size should be changed too*/
    ppolicer->policer_vector = ctc_vector_init(1,SAI_POLICER_ATTR_ENABLE_COUNTER_LIST - SAI_POLICER_ATTR_METER_TYPE + 1);
    if(NULL == ppolicer->policer_vector)
    {
        ret = SAI_STATUS_NO_MEMORY;
        return ret;
    }

    for (index = 0; index < attr_count; index++)
    {
        if ((SAI_POLICER_ATTR_METER_TYPE <= attr_list[index].id) && (SAI_POLICER_ATTR_ENABLE_COUNTER_LIST >= attr_list[index].id))
        {
            /* save entry info in db */
            attr_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_attribute_t));
            sal_memcpy(attr_list_db, &attr_list[index], sizeof(sai_attribute_t));

            if(FALSE == ctc_vector_add(ppolicer->policer_vector, attr_list[index].id - SAI_POLICER_ATTR_METER_TYPE, attr_list_db))
            {
                ret = SAI_STATUS_FAILURE;
                return ret;
            }
         }     
     }
    /* pass at least one field */
    if (0 == ppolicer->policer_vector->used_cnt)
    {
        ret = SAI_STATUS_INVALID_PARAMETER;
        return ret;
    }

    if(FALSE == ctc_vector_add(g_sai_policer_info.pvector,
                CTC_SAI_OBJECT_INDEX_GET(ppolicer->policer_id),
                ppolicer))
    {
        ret = SAI_STATUS_FAILURE;
        return ret;
    }

    *policer_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_POLICER, ppolicer->policer_id);

    return ret;
}

sai_status_t sai_remove_policer(
    _In_ sai_object_id_t policer_id
    )
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_policer_t     *ppolicer = NULL;
    uint32_t  ctc_policer_id = 0;

    CTC_SAI_DEBUG_FUNC();

    ctc_policer_id = CTC_SAI_OBJECT_INDEX_GET(policer_id);
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, ctc_policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if((CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id) || (CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id))
    {
        return SAI_STATUS_OBJECT_IN_USE;
    }

    ctc_vector_del(g_sai_policer_info.pvector, ctc_policer_id);

    ctc_sai_policer_db_release_policer(ppolicer);

    return ret;
}

sai_status_t sai_set_policer_attribute(
    _In_ sai_object_id_t policer_id,
    _In_ const sai_attribute_t *attr
    )
{
    uint32_t  ctc_policer_id = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_qos_policer_t  ctc_policer;
    ctc_sai_policer_t     *ppolicer = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:policer_id 0x%llx", policer_id);
    
    ctc_policer_id = CTC_SAI_OBJECT_INDEX_GET(policer_id);
    sal_memset(&ctc_policer, 0x0, sizeof(ctc_qos_policer_t));

    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, ctc_policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    ctc_sai_policer_map_db_attribute(attr, ppolicer);
    ctc_sai_policer_db_map_policer(ppolicer, &ctc_policer);

    if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id)
    {
        if(ctc_sai_policer_is_in[ctc_policer_id])
        {
            ctc_policer.dir = CTC_INGRESS;
        }
        else
        {
            ctc_policer.dir = CTC_EGRESS;
        }
        ctc_policer.enable = 1;
        ctc_policer.id.gport = ppolicer->used_port_id;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_PORT;

        ret = ctc_qos_set_policer(&ctc_policer);
    }
    else if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id)
    {
        ctc_policer.dir = CTC_INGRESS;
        ctc_policer.enable = 1;
        ctc_policer.id.policer_id = ctc_policer_id;
        ctc_policer.type = CTC_QOS_POLICER_TYPE_FLOW;

        ret = ctc_qos_set_policer(&ctc_policer);
    }

    return ret;
}

sai_status_t sai_get_policer_attribute(
    _In_ sai_object_id_t policer_id,
    _In_ uint32_t attr_count,
    _Out_ sai_attribute_t *attr_list
    )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    sai_attribute_t *attr        = NULL;
    uint32_t         attr_idx    = 0;
    uint32_t        ctc_policer_id = 0;
    ctc_sai_policer_t         *ppolicer = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:policer_id 0x%llx attr_count %u", policer_id, attr_count);

    CTC_SAI_PTR_VALID_CHECK(attr_list);

    ctc_policer_id = CTC_SAI_OBJECT_INDEX_GET(policer_id);
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, ctc_policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for(attr_idx = 0; attr_idx < attr_count; attr_idx++)
    {
        attr = attr_list + attr_idx;
        if ((SAI_POLICER_ATTR_METER_TYPE <= attr->id) && (SAI_POLICER_ATTR_ENABLE_COUNTER_LIST >= attr->id))
        {
            attr = ctc_vector_get(ppolicer->policer_vector, attr->id - SAI_POLICER_ATTR_METER_TYPE);
            if(NULL == attr)
            {
                continue;
            }
            sal_memcpy(attr_list + attr_idx,attr,sizeof(sai_attribute_t));
        }
    }

    return ret;
}

sai_status_t sai_get_policer_statistics(
    _In_ sai_object_id_t policer_id,
    _In_ const sai_policer_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    uint32_t         attr_idx    = 0;
    sai_attribute_t *attr        = NULL;
    ctc_qos_policer_stats_t policer_stats;
    uint32_t        ctc_policer_id = 0;
    ctc_sai_policer_t         *ppolicer = NULL;
    
    CTC_SAI_DEBUG_FUNC();

    sal_memset(&policer_stats, 0x0, sizeof(ctc_qos_policer_stats_t));
    ctc_policer_id = CTC_SAI_OBJECT_INDEX_GET(policer_id);
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, ctc_policer_id);
    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    if((CTC_SAI_POLICER_APPLY_DEFAULT == ppolicer->used_entry_id) && (CTC_SAI_POLICER_APPLY_DEFAULT == ppolicer->used_port_id))
    {
        return SAI_STATUS_OBJECT_IN_USE;
    }
    attr = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_ENABLE_COUNTER_LIST);
    if(NULL == attr)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    else
    {
        if(0 == attr->value.s32)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }
    if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id)
    {
        policer_stats.type = CTC_QOS_POLICER_TYPE_PORT;
        if(ctc_sai_policer_is_in[ctc_policer_id])
            policer_stats.dir = CTC_INGRESS;
        else
            policer_stats.dir = CTC_EGRESS;
        policer_stats.id.gport = ppolicer->used_port_id;
    }
    else if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id)
    {
        policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
        policer_stats.dir = CTC_INGRESS;
        policer_stats.id.policer_id = ppolicer->policer_id;
    }
    ret = ctc_qos_query_policer_stats(&policer_stats);
    if(ret)
    {
        return ret;
    }
    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
            case SAI_POLICER_STAT_PACKETS:
                counters[attr_idx] = policer_stats.stats.confirm_pkts + policer_stats.stats.exceed_pkts + policer_stats.stats.violate_pkts;
                break;
            case SAI_POLICER_STAT_ATTR_BYTES:
                counters[attr_idx] = policer_stats.stats.confirm_bytes + policer_stats.stats.exceed_bytes + policer_stats.stats.violate_bytes;
                break;
            case SAI_POLICER_STAT_GREEN_PACKETS:
                counters[attr_idx] = policer_stats.stats.confirm_pkts;
                //counters[attr_idx] = 1;
                break;
            case SAI_POLICER_STAT_GREEN_BYTES:
                counters[attr_idx] = policer_stats.stats.confirm_bytes;
                //counters[attr_idx] = 2;
                break;
            case SAI_POLICER_STAT_YELLOW_PACKETS:
                counters[attr_idx] = policer_stats.stats.exceed_pkts;
               // counters[attr_idx] = 3;
                break;
            case SAI_POLICER_STAT_YELLOW_BYTES:
                counters[attr_idx] = policer_stats.stats.exceed_bytes;
                //counters[attr_idx] = 4;
                break;
            case SAI_POLICER_STAT_RED_PACKETS:
                counters[attr_idx] = policer_stats.stats.violate_pkts;
                //counters[attr_idx] = 5;
                break;
            case SAI_POLICER_STAT_RED_BYTES:
                counters[attr_idx] = policer_stats.stats.violate_bytes;
                //counters[attr_idx] = 6;
                break;
            default:
                break;
        }
    }

    sai_get_policer_statistics_debug_param(policer_id, counter_ids, number_of_counters, counters);
    return ret;
}

sai_status_t sai_clear_policer_statistics(
    _In_ sai_object_id_t policer_id
    )
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32_t        ctc_policer_id = 0;
    ctc_sai_policer_t         *ppolicer = NULL;
    ctc_qos_policer_stats_t policer_stats;
    sai_attribute_t *attr        = NULL;

    sal_memset(&policer_stats, 0x0, sizeof(ctc_qos_policer_stats_t));

    ctc_policer_id = CTC_SAI_OBJECT_INDEX_GET(policer_id);
    ppolicer = ctc_vector_get(g_sai_policer_info.pvector, ctc_policer_id);

    if(NULL == ppolicer)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    if((CTC_SAI_POLICER_APPLY_DEFAULT == ppolicer->used_entry_id) && (CTC_SAI_POLICER_APPLY_DEFAULT == ppolicer->used_port_id))
    {
        return SAI_STATUS_OBJECT_IN_USE;
    }
    attr = ctc_vector_get(ppolicer->policer_vector, SAI_POLICER_ATTR_ENABLE_COUNTER_LIST);
    if(NULL == attr)
    {
        /* modified by hansf 2019/03/26 for bug51427, if policer stats disable, just return ok, do nothing */
        //return SAI_STATUS_INVALID_OBJECT_ID;
        return SAI_STATUS_SUCCESS;
    }
    else
    {
        if(0 == attr->value.s32)
        {
            /* modified by hansf 2019/03/26 for bug51427, if policer stats disable, just return ok, do nothing */
            //return SAI_STATUS_INVALID_OBJECT_ID;
            return SAI_STATUS_SUCCESS;
        }
    }
    if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_port_id)
    {
        policer_stats.type = CTC_QOS_POLICER_TYPE_PORT;
        if(ctc_sai_policer_is_in[ctc_policer_id])
            policer_stats.dir = CTC_INGRESS;
        else
            policer_stats.dir = CTC_EGRESS;
        policer_stats.id.gport = ppolicer->used_port_id;
    }
    else if(CTC_SAI_POLICER_APPLY_DEFAULT != ppolicer->used_entry_id)
    {
        policer_stats.type = CTC_QOS_POLICER_TYPE_FLOW;
        policer_stats.dir = CTC_INGRESS;
        policer_stats.id.policer_id = ppolicer->policer_id;
    }
    ret = ctc_qos_clear_policer_stats(&policer_stats);

    return ret;
}


#define ________SAI_POLICER_INNER_FUNC
sai_status_t __policer_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_policer_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __policer_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_policer_api_t      g_sai_policer_api_func = {
    .create_policer           = sai_create_policer,
    .remove_policer           = sai_remove_policer,
    .set_policer_attribute    = sai_set_policer_attribute,
    .get_policer_attribute    = sai_get_policer_attribute,
    .get_policer_statistics   = sai_get_policer_statistics,
/* SYSTEM MODIFIED BEGIN: add clear qos stats interface by wangqj at 2016/8/8 */
    .clear_policer_statistics = sai_clear_policer_statistics,
/* END: add clear qos stats interface by wangqj at 2016/8/8 */
};


static ctc_sai_api_reg_info_t g_policer_api_reg_info = {
        .id  = SAI_API_POLICER,
        .init_func = __policer_init_mode_fn,
        .exit_func = __policer_exit_mode_fn,
        .api_method_table = &g_sai_policer_api_func,
        .private_data     = NULL,
};

#define ________SAI_POLICER_OUTER_FUNC
sai_status_t ctc_sai_policer_init()
{
    api_reg_register_fn(&g_policer_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

