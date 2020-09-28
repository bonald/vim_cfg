#include <sal.h>
#include "ctc_api.h"
#include "glb_macro.h"
#include "ctclib_hash.h"
#include "ctclib_slist.h"
#include "ctclib_list.h"
#include "sdb_defines.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <gen/stbl_qos_mpls_domain_define.h>
#include <gen/stbl_qos_mpls_domain.h>
#include <gen/stbl_qos_mpls_flow_define.h>
#include <gen/stbl_qos_mpls_flow.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_d2_qosmaps.h>
#include <ctc_sai_debug.h>

ctc_sai_qos_map_info_t  g_sai_qos_map_info;
extern ctc_qos_queue_drop_t    wred_default;

static ctc_sai_qos_domain_id_t ctc_sai_qos_map_domain[CTC_SAI_QOS_DOMAIN_NUMBER];
static uint8_t ctc_sai_set_ctc_color[3] = {CTC_QOS_COLOR_GREEN, CTC_QOS_COLOR_YELLOW, CTC_QOS_COLOR_RED};

void
ctc_sai_qos_map_switch_set_default_map_init()
{
    uint32_t index = 0;
    uint32_t color_index = 0;
    for(index = 0; index < 8; index ++)
    {
        g_sai_qos_map_info.qos_domain_in_default.dot1p_to_tc_color[index].tc = index;
        g_sai_qos_map_info.qos_domain_in_default.dot1p_to_tc_color[index].color = SAI_PACKET_COLOR_GREEN;
    }
    for(index = 0; index < 8; index ++)
    {
        g_sai_qos_map_info.qos_domain_in_default.exp_to_tc_color[index].tc = index;
        g_sai_qos_map_info.qos_domain_in_default.exp_to_tc_color[index].color = SAI_PACKET_COLOR_GREEN;
    }
    for(index = 0; index < 64; index ++)
    {
        g_sai_qos_map_info.qos_domain_in_default.dscp_to_tc_color[index].tc =  index/8;
        g_sai_qos_map_info.qos_domain_in_default.dscp_to_tc_color[index].color = SAI_PACKET_COLOR_GREEN;
    }
    for(index = 0; index < 8; index ++)
    {
        for(color_index = 0; color_index < 3; color_index ++)
        {
            g_sai_qos_map_info.qos_domain_out_default.tc_and_color_to_dot1p[index][color_index] = index;
            g_sai_qos_map_info.qos_domain_out_default.tc_and_color_to_exp[index][color_index] = index;  
            g_sai_qos_map_info.qos_domain_out_default.tc_and_color_to_dscp[index][color_index] = index * 8;
        }
    }
}

sai_status_t 
ctc_sai_qos_map_get_domain_id(uint32_t qos_map_id, uint32_t *domain_id)
{
    uint32_t i = 0;

    if (SAI_NULL_OBJECT_ID == qos_map_id)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (i = 0; i < CTC_SAI_QOS_DOMAIN_NUMBER; i++)
    {
        if (ctc_sai_qos_map_domain[i].domain_instance.dot1p_to_tc_color_map_id == qos_map_id ||
            ctc_sai_qos_map_domain[i].domain_instance.dscp_to_tc_color_map_id == qos_map_id ||
            ctc_sai_qos_map_domain[i].domain_instance.exp_to_tc_color_map_id == qos_map_id || 
            ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_dot1p_map_id == qos_map_id ||
            ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_dscp_map_id == qos_map_id ||
            ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_exp_map_id == qos_map_id )
        {
            *domain_id = i;
            return SAI_STATUS_SUCCESS;
        }
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t
ctc_sai_qos_set_domain_qos_map_id(uint32_t qos_map_type, uint32_t qos_map_id, uint32_t domain_id)
{

    switch(qos_map_type)
    {
    case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
        ctc_sai_qos_map_domain[domain_id].domain_instance.dot1p_to_tc_color_map_id = qos_map_id;
        break;
    case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
        ctc_sai_qos_map_domain[domain_id].domain_instance.dscp_to_tc_color_map_id = qos_map_id;
        break;
    case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
        ctc_sai_qos_map_domain[domain_id].domain_instance.exp_to_tc_color_map_id = qos_map_id;
        break;
    case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
        ctc_sai_qos_map_domain[domain_id].domain_instance.tc_and_color_to_dot1p_map_id = qos_map_id;
        break;
    case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
        ctc_sai_qos_map_domain[domain_id].domain_instance.tc_and_color_to_dscp_map_id = qos_map_id;
        break;
    case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
        ctc_sai_qos_map_domain[domain_id].domain_instance.tc_and_color_to_exp_map_id = qos_map_id;
        break;
    }
    
    return SAI_STATUS_SUCCESS;
}

uint32_t 
ctc_sai_qos_alloc_map_domain_id(uint32_t qos_map_type)
{
    uint32_t i = 0;
    for (i = 0; i < CTC_SAI_QOS_DOMAIN_NUMBER; i++)
    {
        switch(qos_map_type)
        {
        case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
            if (!ctc_sai_qos_map_domain[i].domain_instance.dot1p_to_tc_color_map_id)
            {
                return i;
            }
            break;
        case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
            if (!ctc_sai_qos_map_domain[i].domain_instance.dscp_to_tc_color_map_id)
            {
                return i;
            }
            break;
        case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
            if (!ctc_sai_qos_map_domain[i].domain_instance.exp_to_tc_color_map_id)
            {
                return i;
            }
            break;
        case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
            if (!ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_dot1p_map_id)
            {
                return i;
            }
            break;
        case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
            if (!ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_dscp_map_id)
            {
                return i;
            }
            break;
        case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
            if (!ctc_sai_qos_map_domain[i].domain_instance.tc_and_color_to_exp_map_id)
            {
                return i;
            }
            break;
        }
    }
    
    return 0xFFFFFFFF;
}

sai_status_t
ctc_sai_qos_map_db_init()
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();

    p_sai_glb->qos_map_max_count = 1000;
    g_sai_qos_map_info.pvector = ctc_vector_init(10,100);

    if(NULL == g_sai_qos_map_info.pvector)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    if (0 != ctclib_opb_create(&p_sai_glb->qos_map_opb, 1, p_sai_glb->qos_map_max_count, "qos_map_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }

    ctc_sai_qos_map_switch_set_default_map_init();
    sal_memset(ctc_sai_qos_map_domain, 0x0, (sizeof(ctc_sai_qos_domain_id_t))*8);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_qos_map_db_alloc_qos_map(ctc_sai_qos_map_t** ppqos_map)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    ctc_sai_qos_map_t *pqos_map = NULL;
    uint32_t            index     = 0;

    if(g_sai_qos_map_info.pvector->used_cnt >= p_sai_glb->qos_map_max_count)
    {
        return SAI_STATUS_TABLE_FULL;
    }

    pqos_map = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_qos_map_t));

    if(pqos_map)
    {
        sal_memset(pqos_map,0,sizeof(ctc_sai_qos_map_t));
        ctclib_opb_alloc_offset(&p_sai_glb->qos_map_opb, &index);
        pqos_map->qos_map_id= index;
    }

    *ppqos_map = pqos_map;

    return SAI_STATUS_SUCCESS;
}

void
ctc_sai_qos_map_db_release_qos_map(ctc_sai_qos_map_t *pqos_map)
{
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_uint32_t index = 0;
    sai_uint32_t color_index = 0;
    sai_qos_map_params_t   *vaule_list_db = NULL;
    if(NULL == pqos_map)
    {
        return ;
    }

    ctclib_opb_free_offset(&p_sai_glb->qos_map_opb, pqos_map->qos_map_id);

    switch(pqos_map->qos_map_type)
    {
        /*use key-->tc and color*/
        case SAI_QOS_MAP_DOT1P_TO_TC:
        case SAI_QOS_MAP_DOT1P_TO_COLOR:
        case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
            for(index = 0; index < 8; index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
                if (vaule_list_db)
                {
                    XFREE(MEM_FEA_TEMP_TODO, vaule_list_db);
                    vaule_list_db = NULL;
                    ctc_vector_del(pqos_map->qos_map_vector, index);
                }
            }
            break;
        case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
            for(index = 0; index < 8; index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
                if (vaule_list_db)
                {
                    XFREE(MEM_FEA_TEMP_TODO, vaule_list_db);
                    vaule_list_db = NULL;
                    ctc_vector_del(pqos_map->qos_map_vector, index);
                }
            }
            break;
        case SAI_QOS_MAP_DSCP_TO_TC:
        case SAI_QOS_MAP_DSCP_TO_COLOR:
        case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
            for(index = 0; index < 64; index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
                if (vaule_list_db)
                {
                    XFREE(MEM_FEA_TEMP_TODO, vaule_list_db);
                    vaule_list_db = NULL;
                    ctc_vector_del(pqos_map->qos_map_vector, index);
                }
            }
            break;
        case SAI_QOS_MAP_TC_TO_QUEUE:
        case SAI_QOS_MAP_TC_TO_DSCP:
        case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
        case SAI_QOS_MAP_TC_TO_DOT1P:
        case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
        case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
            for(index = 0; index < 64; index ++)
            {
                for(color_index = 0; color_index < 3; color_index ++)
                {
                    vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (index << 2) + color_index);
                    if (vaule_list_db)
                    {
                        XFREE(MEM_FEA_TEMP_TODO, vaule_list_db);
                        vaule_list_db = NULL;
                        ctc_vector_del(pqos_map->qos_map_vector, (index << 2) + color_index);
                    }
                }
             }
        default :
            break;
    }

    ctc_vector_release(pqos_map->qos_map_vector);
    XFREE(MEM_FEA_TEMP_TODO, pqos_map);
}

sai_status_t ctc_sai_qos_map_set_to_ctc_qos_map(ctc_sai_qos_map_t *pqos_map, uint32_t domain_id_new)
{
    uint32 index;
    uint32 color_index;
    sai_qos_map_params_t *vaule_list_db = NULL;
    ctc_qos_domain_map_t domain_map;

    if(NULL == pqos_map)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    
    if(SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR == pqos_map->qos_map_type)
    {
        for(index = 0; index < 8; index ++)
        {
            vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
            sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
            domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR;
            domain_map.domain_id = domain_id_new;

            domain_map.priority = (vaule_list_db->tc) *2;
            
            domain_map.color = ctc_sai_set_ctc_color[vaule_list_db->color];
            domain_map.hdr_pri.dot1p.cos = index;
            ctc_qos_set_domain_map(&domain_map);
            domain_map.hdr_pri.dot1p.dei = 1;
            /*added by wangl for bug 35404 */
            domain_map.priority = domain_map.priority + 1;
            ctc_qos_set_domain_map(&domain_map);
        }
    }

    if(SAI_QOS_MAP_EXP_TO_TC_AND_COLOR == pqos_map->qos_map_type)
    {
        for(index = 0; index < 8; index ++)
        {
            vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
            sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
            domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR;
            domain_map.domain_id = domain_id_new;

            domain_map.priority = (vaule_list_db->tc) *2;

            domain_map.color = ctc_sai_set_ctc_color[vaule_list_db->color];
            domain_map.hdr_pri.exp = index;
            ctc_qos_set_domain_map(&domain_map);
        }
    }

    if(SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR == pqos_map->qos_map_type)
    {
        for(index = 0; index < 64; index ++)
        {
            vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, index);
            sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
            domain_map.type = CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR;
            domain_map.domain_id = domain_id_new;

            domain_map.priority = (vaule_list_db->tc) *2;

            domain_map.color = ctc_sai_set_ctc_color[vaule_list_db->color];
            domain_map.hdr_pri.tos.dscp = index;
            ctc_qos_set_domain_map(&domain_map);
        }
    }

    if(SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P == pqos_map->qos_map_type)
    {
        for(index = 0; index < 8; index ++)
        {
            for(color_index = 0; color_index < 3; color_index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (index<<2) + color_index);
                sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
                domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS;
                domain_map.domain_id = domain_id_new;

                domain_map.priority = index * 2;

                domain_map.color = ctc_sai_set_ctc_color[color_index];
                domain_map.hdr_pri.dot1p.cos = vaule_list_db->dot1p;
                ctc_qos_set_domain_map(&domain_map);
                domain_map.hdr_pri.dot1p.dei = 1;
                domain_map.priority = domain_map.priority + 1;
                ctc_qos_set_domain_map(&domain_map);
            }
        }
    }

    if(SAI_QOS_MAP_TC_AND_COLOR_TO_EXP == pqos_map->qos_map_type)
    {
        for(index = 0; index < 8; index ++)
        {
            for(color_index = 0; color_index < 3; color_index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (index<<2) + color_index);
                sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
                domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP;
                domain_map.domain_id = domain_id_new;

                domain_map.priority = index * 2;

                domain_map.color = ctc_sai_set_ctc_color[color_index];
                domain_map.hdr_pri.exp = vaule_list_db->exp;
                ctc_qos_set_domain_map(&domain_map);
                /*added by hansf for bug45802*/
                domain_map.priority = domain_map.priority + 1;
                ctc_qos_set_domain_map(&domain_map);
            }
        }
    }


    if(SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP == pqos_map->qos_map_type)
    {
        for(index = 0; index < 8; index ++)
        {
            for(color_index = 0; color_index < 3; color_index ++)
            {
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (index<<2) + color_index);
                sal_memset(&domain_map, 0x0, sizeof(ctc_qos_domain_map_t));
                domain_map.type = CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP;
                domain_map.domain_id = domain_id_new;

                domain_map.priority = index * 2;

                domain_map.color = ctc_sai_set_ctc_color[color_index];
                domain_map.hdr_pri.tos.dscp = vaule_list_db->dscp;
                ctc_qos_set_domain_map(&domain_map);
                /*added by hansf for bug45802*/
                domain_map.priority = domain_map.priority + 1;
                ctc_qos_set_domain_map(&domain_map);
            }
        }
    }


    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_qos_map_port_set_ctc_qos_map(sai_object_id_t port_id,
                                const sai_attribute_t *attr)
{
    uint32_t ctc_port = 0;
    uint32_t qos_map_id = 0;
    uint32_t domain_id = 0;
    ctc_port = CTC_SAI_OBJECT_INDEX_GET(port_id);
    qos_map_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);

    if (ctc_sai_qos_map_get_domain_id(qos_map_id, &domain_id))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP == attr->id)
    {
        ctc_port_set_direction_property(ctc_port, CTC_PORT_DIR_PROP_QOS_COS_DOMAIN, CTC_INGRESS, domain_id);
    }
    else if (SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP == attr->id)
    {
        ctc_port_set_direction_property(ctc_port, CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN, CTC_INGRESS, domain_id);
    }
    else if (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP == attr->id)
    {
        ctc_port_set_direction_property(ctc_port, CTC_PORT_DIR_PROP_QOS_COS_DOMAIN, CTC_EGRESS, domain_id);
    }
    else if (SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP == attr->id)
    {
        ctc_port_set_direction_property(ctc_port, CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN, CTC_EGRESS, domain_id);
    }

     return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_qos_map_l3if_set_ctc_qos_map(uint32_t  l3if_id,
                                const sai_attribute_t *attr)
{
    uint32_t qos_map_id = 0;
    uint32_t domain_id = 0;
    qos_map_id = CTC_SAI_OBJECT_INDEX_GET(attr->value.oid);

    if (ctc_sai_qos_map_get_domain_id(qos_map_id, &domain_id))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (SAI_ROUTER_INTERFACE_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP == attr->id)
    {
        ctc_l3if_set_property((uint16_t)l3if_id, CTC_L3IF_PROP_IGS_QOS_DSCP_DOMAIN, domain_id);
    }
    else if (SAI_ROUTER_INTERFACE_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP == attr->id)
    {
        ctc_l3if_set_property((uint16_t)l3if_id, CTC_L3IF_PROP_EGS_QOS_DSCP_DOMAIN, domain_id);
    }
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_qos_map_port_set_default_map_init(sai_object_id_t port_id)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t ctc_sai_qos_map_set_switch_qos_map(_In_ const sai_attribute_t* attr)
{
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_QOS_MAP_API_FUNC
sai_status_t sai_create_qos_map(
    _Out_ sai_object_id_t* qos_map_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_qos_map_t     *pqos_map = NULL;
    sai_qos_map_params_t *vaule_list_db = NULL;
    sai_uint32_t index = 0;
    uint32_t domain_id = 0;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(qos_map_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    if(2 != attr_count)
    {
        return SAI_STATUS_FAILURE;
    }

    domain_id= ctc_sai_qos_alloc_map_domain_id( attr_list[0].value.s32);
    if (0xFFFFFFFF == domain_id)
    {
        //domian_id already run out of
        return SAI_STATUS_TABLE_FULL;
    }
    
    ret = ctc_sai_qos_map_db_alloc_qos_map(&pqos_map);
    if(NULL == pqos_map)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /*if the attr changed, this init block_size should be changed too*/
    pqos_map->qos_map_vector = ctc_vector_init(1,64);
    if(NULL == pqos_map->qos_map_vector)
    {
        ret = SAI_STATUS_NO_MEMORY;
        return ret;
    }
    pqos_map->qos_map_type = attr_list[0].value.s32; /*sai_qos_map_type_t*/
 
    for (index = 0; index < attr_list[1].value.qosmap.count; index++)
    {
        vaule_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_qos_map_params_t));
        sal_memcpy(vaule_list_db, &attr_list[1].value.qosmap.list[index].value, sizeof(sai_qos_map_params_t));
        switch(attr_list[0].value.s32)
        {
            /*use key-->tc and color*/
            case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
                if(FALSE == ctc_vector_add(pqos_map->qos_map_vector, attr_list[1].value.qosmap.list[index].key.dot1p, vaule_list_db))
                {
                    ret = SAI_STATUS_FAILURE;
                    return ret;
                } 
                break;
            case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
                if(FALSE == ctc_vector_add(pqos_map->qos_map_vector, attr_list[1].value.qosmap.list[index].key.exp, vaule_list_db))
                {
                    ret = SAI_STATUS_FAILURE;
                    return ret;
                } 
                break;
            case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
                if(FALSE == ctc_vector_add(pqos_map->qos_map_vector, attr_list[1].value.qosmap.list[index].key.dscp, vaule_list_db))
                {
                    ret = SAI_STATUS_FAILURE;
                    return ret;
                } 
                break;
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
                if(FALSE == ctc_vector_add(pqos_map->qos_map_vector, ((attr_list[1].value.qosmap.list[index].key.tc) << 2) + attr_list[1].value.qosmap.list[index].key.color, vaule_list_db))
                {
                    ret = SAI_STATUS_FAILURE;
                    return ret;
                } 
                break;
            default :
                break;
        }
     }
    /* pass at least one field */
    if (0 == pqos_map->qos_map_vector->used_cnt)
    {
        ret = SAI_STATUS_INVALID_PARAMETER;
        return ret;
    }

    if(FALSE == ctc_vector_add(g_sai_qos_map_info.pvector,
                CTC_SAI_OBJECT_INDEX_GET(pqos_map->qos_map_id),
                pqos_map))
    {
        ret = SAI_STATUS_FAILURE;
        return ret;
    }

    if (ctc_sai_qos_map_set_to_ctc_qos_map(pqos_map, domain_id))
    {
        ret = SAI_STATUS_FAILURE;
        return ret;
    }
    ctc_sai_qos_set_domain_qos_map_id(pqos_map->qos_map_type, pqos_map->qos_map_id, domain_id);

    *qos_map_id = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QOS_MAPS,pqos_map->qos_map_id);

    return ret;
}

sai_status_t sai_remove_qos_map (
    _In_  sai_object_id_t   qos_map_id)
{
    sai_status_t                ret = SAI_STATUS_SUCCESS;
    ctc_sai_qos_map_t     *pqos_map = NULL;
    uint32_t  ctc_qos_map_id = 0;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:qos_map_id 0x%llx", qos_map_id);

    ctc_qos_map_id = CTC_SAI_OBJECT_INDEX_GET(qos_map_id);
    pqos_map = ctc_vector_get(g_sai_qos_map_info.pvector, ctc_qos_map_id);
    if(NULL == pqos_map)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    ctc_vector_del(g_sai_qos_map_info.pvector, ctc_qos_map_id);

    ctc_sai_qos_map_db_release_qos_map(pqos_map);

    return ret;
}

 sai_status_t sai_set_qos_map_attribute(
    _In_ sai_object_id_t  qos_map_id,
    _In_ const sai_attribute_t *attr
    )
{
    uint32_t  ctc_qos_map_id = 0;
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_sai_qos_map_t     *pqos_map = NULL;
    uint32 domain_id = 0;
    uint32_t index = 0;
    sai_qos_map_params_t *vaule_list_db = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:qos_map_id 0x%llx", qos_map_id);
    
    ctc_qos_map_id = CTC_SAI_OBJECT_INDEX_GET(qos_map_id);

    pqos_map = ctc_vector_get(g_sai_qos_map_info.pvector, ctc_qos_map_id);
    if(NULL == pqos_map)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    if(pqos_map->qos_map_type != attr->id)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    for (index = 0; index < attr->value.qosmap.count; index++)
    {
        switch(pqos_map->qos_map_type )
        {
            /*use key-->tc and color*/
            case SAI_QOS_MAP_DOT1P_TO_TC:
            case SAI_QOS_MAP_DOT1P_TO_COLOR:
            case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.dot1p);
                if(vaule_list_db)
                {
                    sal_memcpy(vaule_list_db, &attr->value.qosmap.list[index].value, sizeof(sai_qos_map_params_t));
                }
                else
                {
                    vaule_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_qos_map_params_t));
                    ctc_vector_add(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.dot1p, vaule_list_db);
                }
                break;
            case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.exp);
                if(vaule_list_db)
                {
                    sal_memcpy(vaule_list_db, &attr->value.qosmap.list[index].value, sizeof(sai_qos_map_params_t));
                }
                else
                {
                    vaule_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_qos_map_params_t));
                    ctc_vector_add(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.exp, vaule_list_db);
                }
                break;
            case SAI_QOS_MAP_DSCP_TO_TC:
            case SAI_QOS_MAP_DSCP_TO_COLOR:
            case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.dscp);
                if(vaule_list_db)
                {
                    sal_memcpy(vaule_list_db, &attr->value.qosmap.list[index].value, sizeof(sai_qos_map_params_t));
                }
                else
                {
                    vaule_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_qos_map_params_t));
                    ctc_vector_add(pqos_map->qos_map_vector, attr->value.qosmap.list[index].key.dscp, vaule_list_db);
                }
                break;
            case SAI_QOS_MAP_TC_TO_QUEUE:
            case SAI_QOS_MAP_TC_TO_DSCP:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
            case SAI_QOS_MAP_TC_TO_DOT1P:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (attr->value.qosmap.list[index].key.tc << 2) + attr->value.qosmap.list[index].key.color);
                if(vaule_list_db)
                {
                    sal_memcpy(vaule_list_db, &attr->value.qosmap.list[index].value, sizeof(sai_qos_map_params_t));
                }
                else
                {
                    vaule_list_db = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(sai_qos_map_params_t));
                    ctc_vector_add(pqos_map->qos_map_vector, (attr->value.qosmap.list[index].key.tc << 2) + attr->value.qosmap.list[index].key.color, vaule_list_db);
                }
                break;
            default :
                break;
        }
     }

    ret = ctc_sai_qos_map_get_domain_id(pqos_map->qos_map_id, &domain_id);
    if (ret)
    {
        return  ret;
    }
    
    ret = ctc_sai_qos_map_set_to_ctc_qos_map(pqos_map, domain_id);

    return ret;
}

sai_status_t sai_get_qos_map_attribute(
     _In_ sai_object_id_t   qos_map_id ,
     _In_ uint32_t attr_count,
     _Inout_ sai_attribute_t *attr_list
    )
{
    uint32_t  ctc_qos_map_id = 0;
    ctc_sai_qos_map_t     *pqos_map = NULL;
    uint32_t index = 0;
    sai_qos_map_params_t *vaule_list_db = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("in:qos_map_id 0x%llx, attr_count %u", qos_map_id, attr_count);

    ctc_qos_map_id = CTC_SAI_OBJECT_INDEX_GET(qos_map_id);

    pqos_map = ctc_vector_get(g_sai_qos_map_info.pvector, ctc_qos_map_id);
    if(NULL == pqos_map)
    {
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    if(pqos_map->qos_map_type != attr_list->id)
    {
         return SAI_STATUS_INVALID_PARAMETER;
    }
    for (index = 0; index < attr_list->value.qosmap.count; index++)
    {
        switch(pqos_map->qos_map_type )
        {
            /*use key-->tc and color*/
            case SAI_QOS_MAP_DOT1P_TO_TC:
            case SAI_QOS_MAP_DOT1P_TO_COLOR:
            case SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr_list->value.qosmap.list[index].key.dot1p);
                if(NULL == vaule_list_db)
                {
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
                attr_list->value.qosmap.list[index].value.tc = vaule_list_db->tc;
                attr_list->value.qosmap.list[index].value.color = vaule_list_db->color;
                break;
            case SAI_QOS_MAP_EXP_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr_list->value.qosmap.list[index].key.exp);
                if(NULL == vaule_list_db)
                {
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
                attr_list->value.qosmap.list[index].value.tc = vaule_list_db->tc;
                attr_list->value.qosmap.list[index].value.color = vaule_list_db->color;
                break;
            case SAI_QOS_MAP_DSCP_TO_TC:
            case SAI_QOS_MAP_DSCP_TO_COLOR:
            case SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, attr_list->value.qosmap.list[index].key.dscp);
                if(NULL == vaule_list_db)
                {
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
                attr_list->value.qosmap.list[index].value.tc = vaule_list_db->tc;
                attr_list->value.qosmap.list[index].value.color = vaule_list_db->color;
                break;
            case SAI_QOS_MAP_TC_TO_QUEUE:
            case SAI_QOS_MAP_TC_TO_DSCP:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP:
            case SAI_QOS_MAP_TC_TO_DOT1P:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P:
            case SAI_QOS_MAP_TC_AND_COLOR_TO_EXP:
                vaule_list_db = ctc_vector_get(pqos_map->qos_map_vector, (attr_list->value.qosmap.list[index].key.tc << 2) + attr_list->value.qosmap.list[index].key.color);
                if(NULL == vaule_list_db)
                {
                    return SAI_STATUS_INVALID_OBJECT_ID;
                }
                attr_list->value.qosmap.list[index].value.dot1p= vaule_list_db->dot1p;
                attr_list->value.qosmap.list[index].value.dscp = vaule_list_db->dscp;
                attr_list->value.qosmap.list[index].value.exp= vaule_list_db->exp;
                break;
            default :
                break;
        }
     }

    return SAI_STATUS_SUCCESS;
}

/*added by yejl for bug 51333, 2019-06-20*/
/*added by yejl for bug 52805, 2019-07-25*/
sai_status_t
ctc_sai_set_mpls_label_to_exp_domain(sai_uint32_t mpls_label, uint32 exp_domain)
{
    int32_t  ret = 0;
    stbl_qos_mpls_domain_t * p_db_mpls_domain = NULL;
    stbl_qos_mpls_domain_t mpls_domain;
    stbl_qos_mpls_flow_t * p_db_mpls_flow = NULL;
    stbl_qos_mpls_flow_key_t mpls_flow_key;
    
    ctc_mpls_property_t mpls_pro;
    ctc_mpls_ilm_qos_map_t qos_map;

    sal_memset(&mpls_flow_key, 0, sizeof(mpls_flow_key));
    sal_memset(&mpls_domain, 0, sizeof(mpls_domain));

    /*start get or add or delete qos mpls domain table*/    
    if (exp_domain)
    {
        mpls_domain.key.mpls_label = mpls_label;
        p_db_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&(mpls_domain.key));
        if (NULL == p_db_mpls_domain)
        {
            stbl_qos_mpls_domain_add_qos_mpls_domain(&mpls_domain);
            p_db_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&(mpls_domain.key));
            if (NULL == p_db_mpls_domain)
            {
                return SAI_STATUS_ITEM_NOT_FOUND;
            }
        }
        p_db_mpls_domain->exp_domain = exp_domain;
        stbl_qos_mpls_domain_set_qos_mpls_domain_field(p_db_mpls_domain, STBL_QOS_MPLS_DOMAIN_FLD_EXP_DOMAIN);
    }
    else
    {
        mpls_domain.key.mpls_label = mpls_label;
        p_db_mpls_domain = stbl_qos_mpls_domain_get_qos_mpls_domain(&(mpls_domain.key));
        if (p_db_mpls_domain)
        {
            stbl_qos_mpls_domain_del_qos_mpls_domain(&(mpls_domain.key));
        }
    }
    /*end*/

    /*start get the qos mpls flow table, if the mpls_flow_en is true, call sdk api*/
    mpls_flow_key.mpls_label = mpls_label;
    p_db_mpls_flow = stbl_qos_mpls_flow_get_qos_mpls_flow(&mpls_flow_key);
    if (p_db_mpls_flow && p_db_mpls_flow->mpls_flow_en)
    {
        sal_memset(&mpls_pro, 0x0, sizeof(mpls_pro));
        sal_memset(&qos_map, 0x0, sizeof(qos_map));
        qos_map.mode = CTC_MPLS_ILM_QOS_MAP_ELSP;
        qos_map.exp_domain = exp_domain;
        mpls_pro.space_id = 0;
        mpls_pro.label = mpls_label;
        mpls_pro.property_type = CTC_MPLS_ILM_QOS_MAP;
        mpls_pro.value = (void *)(&qos_map);
        ret = ctc_mpls_set_ilm_property(&mpls_pro);
    }
    /*end*/
    return ctc_sai_get_error_from_sdk_error(ret);
}
/*ended by yejl*/

sai_status_t 
sai_set_mpls_label_to_exp_domain_attribute(
    _In_ sai_uint32_t  mpls_label,
    _In_ const sai_attribute_t *attr)
{
    int32_t  ret = 0;
    uint32 exp_domain = 0;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);
    exp_domain = attr->value.s32;

    switch (attr->id)
    {
    case SAI_QOS_MAP_ATTR_MPLS_LABEL_TO_EXP_DOMAIN: 
        ret = ctc_sai_set_mpls_label_to_exp_domain(mpls_label, exp_domain);
        break;
    default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t 
ctc_sai_set_queue_smart_buffer(sai_object_id_t queue_oid, uint32 smart_buffer_en)
{
    int32_t  ret = 0;
    uint32_t  sai_queue_id = 0;
    uint32_t  ctc_queue_id = 0;
    uint32_t  ctc_port = 0;
    ctc_qos_drop_t  ctc_drop;
    
    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_oid);
    ctc_port = CTC_SAI_GET_PORT_BY_QUEUE_ID(sai_queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);
    
    sal_memset(&ctc_drop, 0x0, sizeof(ctc_qos_drop_t));
    ctc_drop.queue.queue_type = CTC_QUEUE_TYPE_NETWORK_EGRESS;
    ctc_drop.queue.gport = ctc_port;
    ctc_drop.queue.queue_id = ctc_queue_id;
    ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
    if (smart_buffer_en)
    {
        ctc_drop.drop.is_dynamic = 1;
        ctc_drop.drop.drop_factor[0] = 10;
        ctc_drop.drop.drop_factor[1] = 10;
        ctc_drop.drop.drop_factor[2] = 10;
        ctc_drop.drop.drop_factor[3] = 10;
    }
    else
    {
        sal_memcpy(&ctc_drop.drop, &wred_default, sizeof(ctc_qos_queue_drop_t));
        ctc_drop.drop.mode = CTC_QUEUE_DROP_WTD;
    }
    
    ret = ctc_qos_set_drop_scheme(&ctc_drop);
    return ctc_sai_get_error_from_sdk_error(ret);
}

sai_status_t 
sai_set_queue_smart_buffer_attribute(
    _In_ sai_object_id_t  queue_oid,
    _In_ const sai_attribute_t *attr)
{
    int32_t  ret = 0;
    uint32 smart_buffer_en = 0;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);
    smart_buffer_en = attr->value.s32;

    switch (attr->id)
    {
    case SAI_QOS_MAP_ATTR_QUEUE_SMART_BUFFER: 
        ret = ctc_sai_set_queue_smart_buffer(queue_oid, smart_buffer_en);
        break;
    default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
    return ctc_sai_get_error_from_sdk_error(ret);
}

#define ________SAI_QOS_MAP_INNER_FUNC
sai_status_t __qos_map_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_qos_map_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __qos_map_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_qos_map_api_t      g_sai_qos_map_api_func = {
    .create_qos_map           = sai_create_qos_map,
    .remove_qos_map           = sai_remove_qos_map,
    .set_qos_map_attribute    = sai_set_qos_map_attribute,
    .get_qos_map_attribute    = sai_get_qos_map_attribute,
    .set_mpls_label_to_exp_domain    = sai_set_mpls_label_to_exp_domain_attribute,
    .set_queue_smart_buffer    = sai_set_queue_smart_buffer_attribute,
};

static ctc_sai_api_reg_info_t g_qosmaps_api_reg_info = {
        .id  = SAI_API_QOS_MAPS,
        .init_func = __qos_map_init_mode_fn,
        .exit_func = __qos_map_exit_mode_fn,
        .api_method_table = &g_sai_qos_map_api_func,
        .private_data     = NULL,
};

#define ________SAI_QOSMAPS_OUTER_FUNC
sai_status_t ctc_sai_qosmaps_init()
{    
    api_reg_register_fn(&g_qosmaps_api_reg_info);
    return SAI_STATUS_SUCCESS;
}

