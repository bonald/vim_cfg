
/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "saihash.h"


#define HSRV_INVALID_HASHID     -1

uint32 g_hsrv_hashid_array[2] = {HSRV_INVALID_HASHID, HSRV_INVALID_HASHID};
/***************************************************************************************************
 * Name         : hsrv_hash_creat
 * Purpose      : creat hash-id         
 * Input        : N/A
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_hash_creat()
{
    sai_hash_api_t*   hash_api = NULL;
    sai_u32_list_t    hash_list;
    
    sai_object_id_t   hash_oid= 0; 
    sai_attribute_t   attr[1];
    uint32            hash_fld[4]; 

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HASH, (void**)&hash_api));

    hash_fld[0] = SAI_NATIVE_HASH_FIELD_SRC_IP;
    hash_fld[1] = SAI_NATIVE_HASH_FIELD_DST_IP;
    hash_fld[2] = SAI_NATIVE_HASH_FIELD_SRC_MAC;
    hash_fld[3] = SAI_NATIVE_HASH_FIELD_DST_MAC;
    hash_list.list = hash_fld;
    hash_list.count = 4;
    
    attr[0].id = SAI_HASH_NATIVE_FIELD_LIST;
    attr[0].value.u32list.list = hash_list.list;
    attr[0].value.u32list.count = hash_list.count;
  
    HSRV_IF_ERROR_RETURN(hash_api->create_hash(&hash_oid, 1, attr));

    g_hsrv_hashid_array[0] = CTC_SAI_OBJECT_INDEX_GET(hash_oid);
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_hash_set_attribute
 * Purpose      : set hash attribute     
 * Input        : p_lag_global
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A

***************************************************************************************************/
int32
hsrv_hash_set_attribute( tbl_lag_global_t* p_lag_global )
{
    sai_hash_api_t*   hash_api = NULL;
    sai_object_id_t   hash_oid =0;
    sai_attribute_t   attr[1];
    int               i = 0;
    sai_u32_list_t    hash_list;
    uint32            fld_list[SAI_NATIVE_HASH_FIELD_MAX];
    if (NULL == p_lag_global)
    {
        return HSRV_E_INVALID_PTR;
    }
    hash_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_HASH, g_hsrv_hashid_array[0]);
    
    attr[0].id = SAI_HASH_NATIVE_FIELD_LIST;
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_SRC_MAC))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_SRC_MAC;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_DST_MAC))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_DST_MAC;
    } 
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_SRC_IP))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_SRC_IP;
    }                
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_DST_IP))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_DST_IP;
    }                   
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_IP_PROTOCOL))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_IP_PROTOCOL;
    }                
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_SRC_PORT))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_L4_SRC_PORT;
    } 
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_DST_PORT))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_L4_DST_PORT;
    } 
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_VXLAN_VNI))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_VXLAN_VNI;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_MAC))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_DST_MAC;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_MAC))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_IP))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_SRC_IP;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_IP))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_DST_IP;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_IP_PROTOCOL))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_SRC_PORT_L4))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_INNER_DST_PORT_L4))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_GRE_KEY))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_GRE_KEY;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_VNI))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_NVGRE_VNI;
    }
    if (GLB_FLAG_ISSET(p_lag_global->load_balance, GLB_LAG_LOAD_BALANCE_NVGRE_FLOW_ID))
    {
        fld_list[i++] = SAI_NATIVE_HASH_FIELD_NVGRE_FLOWID;
    }
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_HASH, (void**)&hash_api));
    hash_list.list = fld_list;
    hash_list.count = i;
    
    attr[0].value.u32list.count = hash_list.count;
    attr[0].value.u32list.list = hash_list.list;
    HSRV_IF_ERROR_RETURN(hash_api->set_hash_attribute(hash_oid, attr));
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_lag_global_sync 
 * Purpose      : load lag cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_lag_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_lag_global_t* p_lag_global = NULL;
    sai_switch_api_t*  switch_api = NULL;
    sai_attribute_t    attr[1];
       
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_lag_global = (tbl_lag_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_LAG_GLOBAL_FLD_HASH_ARITHMETIC:
             attr[0].id = SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM;
             if (GLB_LAG_HASH_ALGO_XOR == p_lag_global->hash_arithmetic)
             {
                attr[0].value.s32 = SAI_HASH_ALGORITHM_XOR;
             }
             else if (GLB_LAG_HASH_ALGO_CRC == p_lag_global->hash_arithmetic)
             {
                attr[0].value.s32 = SAI_HASH_ALGORITHM_CRC;
             }
             HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
             HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
             break;

        case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE_INNER_FIELD_EN:
             attr[0].id = SAI_SWITCH_ATTR_CUSTOM_LAG_HASH_INNER_FIELD_EN;
             attr[0].value.u32 = p_lag_global->load_balance_inner_field_en;
             HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
             HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
             break;
            
        case TBL_LAG_GLOBAL_FLD_LOAD_BALANCE:
            if (g_hsrv_hashid_array[0] == HSRV_INVALID_HASHID)
            {
                hsrv_hash_creat();
                hsrv_hash_set_attribute(p_lag_global);
            }
            else
            {
                hsrv_hash_set_attribute(p_lag_global);
            }
            
            attr[0].id = SAI_SWITCH_ATTR_LAG_HASH;
            attr[0].value.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_HASH, g_hsrv_hashid_array[0]);
            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
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


/***************************************************************************************************
 * Name         : hsrv_ecmp_global_sync 
 * Purpose      : load ecmp cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_ecmp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_ecmp_global_t* p_ecmp_global = NULL;
    sai_switch_api_t*  switch_api = NULL;
    sai_attribute_t    attr[1];
       
    HSRV_AGG_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        p_ecmp_global = (tbl_ecmp_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_ECMP_GLOBAL_FLD_HASH_ALGORITHM:
             attr[0].id = SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM;
             if (GLB_ECMP_HASH_ALGO_XOR == p_ecmp_global->hash_algorithm)
             {
                attr[0].value.s32 = SAI_HASH_ALGORITHM_XOR;
             }
             else if (GLB_ECMP_HASH_ALGO_CRC == p_ecmp_global->hash_algorithm)
             {
                attr[0].value.s32 = SAI_HASH_ALGORITHM_CRC;
             }
             HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
             HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
             break;

        case TBL_ECMP_GLOBAL_FLD_DLB_MODE_TYPE:
             attr[0].id = SAI_SWITCH_ATTR_CUSTOM_ECMP_LOAD_BALANCE_MODE;
             attr[0].value.u32 = p_ecmp_global->dlb_mode_type;
             HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
             HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
             break;

        case TBL_ECMP_GLOBAL_FLD_HASH_KEY:
            attr[0].id = SAI_SWITCH_ATTR_ECMP_HASH;
            attr[0].value.u32 = 0;

            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_MACSA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_SRC_MAC);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_MACDA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_DST_MAC);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_ETHERTYPE))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_ETHERTYPE);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_IPSA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_SRC_IP);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_IPDA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_DST_IP);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_IPPRO))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_IP_PROTOCOL);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_SRC_PORT))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_L4_SRC_PORT);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_DST_PORT))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_L4_DST_PORT);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_MACSA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_MACDA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_DST_MAC);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_IPSA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_SRC_IP);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_IPDA))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_DST_IP);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_IP_PROTOCOL))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_SRC_PORT))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT);
            }
            if (GLB_FLAG_ISSET(p_ecmp_global->hash_key, GLB_ECMP_HASH_KEY_INNER_DST_PORT))
            {
                CTC_BIT_SET(attr[0].value.u32, SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT);
            }

            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH, (void**)&switch_api));
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
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


