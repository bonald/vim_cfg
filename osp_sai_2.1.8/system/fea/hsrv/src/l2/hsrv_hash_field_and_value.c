#include "hsrv_inc.h"
#include "saihash.h"
#include "glb_hash_field_value_define.h"
#include "hsrv_hash_field_and_value.h"
#include "gen/tbl_hash_field_profile_define.h"
#include "gen/tbl_hash_field_profile.h"
#include "gen/tbl_hash_value_profile_define.h"
#include "gen/tbl_hash_value_profile.h"
#include "gen/tbl_hash_value_global_define.h"
#include "gen/tbl_hash_value_global.h"
#include <linux/ip.h>
#include <sys/statfs.h> 
#include "sal_common.h"

int32
hsrv_hash_field_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_hash_field_profile_t* p_hash_fld_pro = NULL;
    tbl_hash_field_profile_t* p_db_hash_fld_pro = NULL;
    sai_hash_api_t*   hash_api = NULL;
    sai_attribute_t   attr;
    sai_attribute_t   hash_attr[15];
    uint32            count = 0;
    uint32            index = 0;
    
    sal_memset(&attr, 0, sizeof(attr));
    sal_memset(&hash_attr, 0, sizeof(hash_attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HASH, (void**)&hash_api));
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_hash_fld_pro = (tbl_hash_field_profile_t*)p_tbl;
        HSRV_PTR_CHECK(p_hash_fld_pro);
        index = p_hash_fld_pro->index;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_L2_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->l2_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_IP_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->ip_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_IPV6_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->ipv6_set;
        count++;
     
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_GRE_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->gre_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_VXLAN_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->vxlan_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_NVGRE_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->nvgre_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_MPLS_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->mpls_set;
        count++;
        /*added by yejl for bug 54080, 2019-10-24*/
        /* 
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_VPWS_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->vpws_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_VPLS_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->vpls_set;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_L3VPN_SET;
        hash_attr[count].value.s32 = p_hash_fld_pro->l3vpn_set;
        count++;
        */
        /*end*/
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_MODE_SYMMETRY;
        hash_attr[count].value.s32 = p_hash_fld_pro->mode_symmetry;
        count++;
        hash_attr[count].id = SAI_NATIVE_HASH_FIELD_PROFILE_HASH_ARITHMETIC;
        hash_attr[count].value.s32 = p_hash_fld_pro->hash_arithmetic;
        count++;
        HSRV_IF_ERROR_RETURN(hash_api->create_hash_profile(index, count, hash_attr));
        break;
    case CDB_OPER_DEL:
        break;
    case CDB_OPER_SET:
        p_hash_fld_pro = (tbl_hash_field_profile_t*)p_tbl;
        p_db_hash_fld_pro = tbl_hash_field_profile_get_hash_field_profile(&p_hash_fld_pro->key);
        HSRV_PTR_CHECK(p_db_hash_fld_pro);
        index = p_db_hash_fld_pro->index;
        switch (field_id)
        {
        case TBL_HASH_FIELD_PROFILE_FLD_L2_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_L2_SET;
            attr.value.s32 = p_hash_fld_pro->l2_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_IP_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_IP_SET;
            attr.value.s32 = p_hash_fld_pro->ip_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_IPV6_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_IPV6_SET;
            attr.value.s32 = p_hash_fld_pro->ipv6_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
           
        case TBL_HASH_FIELD_PROFILE_FLD_GRE_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_GRE_SET;
            attr.value.s32 = p_hash_fld_pro->gre_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_VXLAN_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_VXLAN_SET;
            attr.value.s32 = p_hash_fld_pro->vxlan_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_NVGRE_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_NVGRE_SET;
            attr.value.s32 = p_hash_fld_pro->nvgre_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_MPLS_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_MPLS_SET;
            attr.value.s32 = p_hash_fld_pro->mpls_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_VPLS_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_VPLS_SET;
            attr.value.s32 = p_hash_fld_pro->vpls_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_VPWS_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_VPWS_SET;
            attr.value.s32 = p_hash_fld_pro->vpws_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_L3VPN_SET:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_L3VPN_SET;
            attr.value.s32 = p_hash_fld_pro->l3vpn_set;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
            
        case TBL_HASH_FIELD_PROFILE_FLD_HASH_ARITHMETIC:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_HASH_ARITHMETIC;
            attr.value.s32 = p_hash_fld_pro->hash_arithmetic;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_MODE_SYMMETRY:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_MODE_SYMMETRY;
            attr.value.s32 = p_hash_fld_pro->mode_symmetry;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        case TBL_HASH_FIELD_PROFILE_FLD_BITMAP_DISABLE:
            attr.id = SAI_NATIVE_HASH_FIELD_PROFILE_BITMAP_DISABLE;
            attr.value.s32 = p_hash_fld_pro->bitmap_disable;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            break;
        default:
            break;
        }
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_hash_value_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_hash_value_profile_t* p_hash_value_pro = NULL;
    tbl_hash_value_profile_t* p_db_hash_value_pro = NULL;
    sai_hash_api_t*   hash_api = NULL;
    sai_attribute_t   attr;
    uint32            index = 0;
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HASH, (void**)&hash_api));

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        p_hash_value_pro = (tbl_hash_value_profile_t*)p_tbl;
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&p_hash_value_pro->key);
        HSRV_PTR_CHECK(p_db_hash_value_pro);
        index = p_db_hash_value_pro->index;

        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_AGG_STATIC)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_AGG_STATIC;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_AGG_DYNAMIC;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_L3;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
        if (p_db_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID)
        {
            attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID;
            attr.value.s32 = GLB_HASH_VALUE_PROFILE_INVALID_OFFSET;
            HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
        }
                
        break;
    case CDB_OPER_SET:
        p_hash_value_pro = (tbl_hash_value_profile_t*)p_tbl;
        p_db_hash_value_pro = tbl_hash_value_profile_get_hash_value_profile(&p_hash_value_pro->key);
        HSRV_PTR_CHECK(p_db_hash_value_pro);
        index = p_db_hash_value_pro->index;
        switch (field_id)
        {
        case TBL_HASH_VALUE_PROFILE_FLD_BITMAP_DATA:
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_AGG_STATIC)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_AGG_STATIC;
                attr.value.s32 = p_hash_value_pro->agg_static;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_AGG_DYNAMIC)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_AGG_DYNAMIC;
                attr.value.s32 = p_hash_value_pro->agg_dynamic;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_L3)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_L3;
                attr.value.s32 = p_hash_value_pro->ecmp_static_l3;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_NVGRE;
                attr.value.s32 = p_hash_value_pro->ecmp_static_nvgre;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_STATIC_VXLAN;
                attr.value.s32 = p_hash_value_pro->ecmp_static_vxlan;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC;
                attr.value.s32 = p_hash_value_pro->ecmp_dynamic;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_pro->bitmap_data & GLB_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_PROFILE_ECMP_DYNAMIC_FLOW_ID;
                attr.value.s32 = p_hash_value_pro->ecmp_dynamic_flow_id;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            break;
        default:
            break;
        }
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_hash_value_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_hash_value_global_t* p_hash_value_glb = NULL;
    sai_hash_api_t*   hash_api = NULL;
    sai_attribute_t   attr;
    uint32            index = 0;

    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HASH, (void**)&hash_api));

    switch (oper)
    {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        break;
    case CDB_OPER_SET:
        p_hash_value_glb = (tbl_hash_value_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_HASH_VALUE_GLOBAL_FLD_BITMAP_DATA:
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_AGG)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_AGG;
                attr.value.s32 = p_hash_value_glb->agg_hash;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_L3)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_L3;
                attr.value.s32 = p_hash_value_glb->ecmp_static_l3;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_NVGRE;
                attr.value.s32 = p_hash_value_glb->ecmp_static_nvgre;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_STATIC_VXLAN;
                attr.value.s32 = p_hash_value_glb->ecmp_static_vxlan;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_DYNAMIC;
                attr.value.s32 = p_hash_value_glb->ecmp_dynamic;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_ECMP_DYNAMIC_FLOW_ID;
                attr.value.s32 = p_hash_value_glb->ecmp_dynamic_flow_id;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            if (p_hash_value_glb->bitmap_data & GLB_HASH_VALUE_GLOBAL_LBID)
            {
                attr.id = SAI_NATIVE_HASH_VALUE_GLOBAL_LBID;
                attr.value.s32 = p_hash_value_glb->lbid;
                HSRV_IF_ERROR_RETURN(hash_api->set_hash_profile_attribute(index, &attr));
            }
            break;
        default:
            break;
        }
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_hash_field_and_value_start()
{
    cdb_pm_subscribe_tbl(TBL_HASH_FIELD_PROFILE, hsrv_hash_field_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_HASH_VALUE_PROFILE, hsrv_hash_value_profile_sync, NULL);
    cdb_pm_subscribe_tbl(TBL_HASH_VALUE_GLOBAL, hsrv_hash_value_global_sync, NULL);
    return HSRV_E_NONE;
}

