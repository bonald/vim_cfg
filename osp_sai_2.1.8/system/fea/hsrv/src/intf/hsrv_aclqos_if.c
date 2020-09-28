#include "hsrv_inc.h"
#include "hsrv_qos.h"


int32
hsrv_aclqos_if_in_policer_creat_policer_profile(tbl_qos_policer_profile_t * p_qos_policer_profile, tbl_aclqos_if_t *p_aclqos_if, uint64* policer_oid)
{
    sai_object_id_t policer_id;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[16] = {{0}};
    uint32_t attr_count = 0;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    tbl_fea_port_policer_apply_t fea_port_policer_apply;
    tbl_qos_policer_action_profile_t *p_db_action_profile = NULL;

    sal_memset(&fea_port_policer_apply, 0x0, sizeof(tbl_fea_port_policer_apply_t));
    sal_memset(attr, 0x0, sizeof(attr));

    attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
    }
    else
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
    }
    attr_count ++;

    if(GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Sr_TCM;
        attr_count ++;
    }
    else if(GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Tr_TCM;
        attr_count ++;
    }
    else
    {
        return -1;
    }
    
    if(p_qos_policer_profile->color_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;
        attr_count ++;
    }
    else
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
        attr_count ++;
    }


    attr[attr_count].id = SAI_POLICER_ATTR_CIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_CBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->cbs;
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->ebs_or_pbs;
    attr_count ++;

    if(p_qos_policer_profile->flags & GLB_POLICER_PROFILE_FLAGS_STATS)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
        attr[attr_count].value.s32 = 1;
        attr_count ++;
    }

    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_qos_policer_profile->action_profile_name);
    if (p_db_action_profile)
    {
        hsrv_qos_policer_action_map_sai_type(attr, &attr_count, p_db_action_profile);
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_id,attr_count,attr));

    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
    if(NULL == p_fea_port_policer_apply)
    {
        sal_memcpy(fea_port_policer_apply.key.name, p_aclqos_if->key.name, IFNAME_SIZE);
        HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_add_fea_port_policer_apply(&fea_port_policer_apply));
        p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
    }
    p_fea_port_policer_apply->in_policer_id = policer_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_set_fea_port_policer_apply_field(p_fea_port_policer_apply, TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID));
    *policer_oid = policer_id;

    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_out_policer_creat_policer_profile(tbl_qos_policer_profile_t * p_qos_policer_profile, tbl_aclqos_if_t *p_aclqos_if, uint64* policer_oid)
{
    sai_object_id_t policer_id;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[16] = {{0}};
    uint32_t attr_count = 0;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    tbl_fea_port_policer_apply_t fea_port_policer_apply;
    tbl_qos_policer_action_profile_t *p_db_action_profile = NULL;

    sal_memset(&fea_port_policer_apply, 0x0, sizeof(tbl_fea_port_policer_apply_t));
    sal_memset(attr, 0x0, sizeof(attr));

    attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
    }
    else
    {
        attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
    }
    attr_count ++;

    if(GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Sr_TCM;
        attr_count ++;
    }
    else if(GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile->algorithm_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_MODE;
        attr[attr_count].value.s32 = SAI_POLICER_MODE_Tr_TCM;
        attr_count ++;
    }
    else
    {
        return -1;
    }
    
    if(p_qos_policer_profile->color_mode)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;
        attr_count ++;
    }
    else
    {
        attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
        attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
        attr_count ++;
    }


    attr[attr_count].id = SAI_POLICER_ATTR_CIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->cir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_CBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->cbs;
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PIR;
    if (p_qos_policer_profile->is_pps)
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
    }
    else
    {
        attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64* 1000/8;
    }
    attr_count ++;

    attr[attr_count].id = SAI_POLICER_ATTR_PBS;
    attr[attr_count].value.u64 = p_qos_policer_profile->ebs_or_pbs;
    attr_count ++;

    if(p_qos_policer_profile->flags & GLB_POLICER_PROFILE_FLAGS_STATS)
    {
        attr[attr_count].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
        attr[attr_count].value.s32 = 1;
        attr_count ++;
    }

    p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_qos_policer_profile->action_profile_name);
    if (p_db_action_profile)
    {
        hsrv_qos_policer_action_map_sai_type(attr, &attr_count, p_db_action_profile);
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    HSRV_IF_ERROR_RETURN(policer_api->create_policer(&policer_id,attr_count,attr));

    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
    if(NULL == p_fea_port_policer_apply)
    {
        sal_memcpy(fea_port_policer_apply.key.name, p_aclqos_if->key.name, IFNAME_SIZE);
        HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_add_fea_port_policer_apply(&fea_port_policer_apply));
        p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
    }
    p_fea_port_policer_apply->out_policer_id = policer_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_set_fea_port_policer_apply_field(p_fea_port_policer_apply, TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID));
    *policer_oid = policer_id;

    return HSRV_E_NONE;
}

int32 hsrv_aclqos_if_delete_in_policer_field(tbl_aclqos_if_t *p_aclqos_if)
{
    sai_attribute_t     attr;
    tbl_interface_t* p_interface = NULL;
    sai_port_api_t*     port_api;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    sai_policer_api_t*  policer_api;

    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if->key.name);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    
    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
    if(NULL == p_fea_port_policer_apply)
    {
        return HSRV_E_NONE;
    }
    if(0 == p_fea_port_policer_apply->in_policer_id)
    {
         return HSRV_E_NONE;
    }

    attr.id = SAI_PORT_ATTR_POLICER_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    /* added by hansf for bug 51208, remove policer from port should clear stats first */
    HSRV_IF_ERROR_RETURN(policer_api->clear_policer_statistics(p_fea_port_policer_apply->in_policer_id));
    
    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, &attr));
    
    HSRV_IF_ERROR_RETURN(policer_api->remove_policer(p_fea_port_policer_apply->in_policer_id));
    
    p_fea_port_policer_apply->in_policer_id = 0;


    HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_set_fea_port_policer_apply_field(p_fea_port_policer_apply, TBL_FEA_PORT_POLICER_APPLY_FLD_IN_POLICER_ID));

    return HSRV_E_NONE;
}

int32 hsrv_aclqos_if_delete_out_policer_field(tbl_aclqos_if_t *p_aclqos_if)
{
    sai_attribute_t     attr;
    tbl_interface_t* p_interface = NULL;
    sai_port_api_t*     port_api;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    sai_policer_api_t*  policer_api;

    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if->key.name);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    
    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);

    if(NULL == p_fea_port_policer_apply)
    {
        return HSRV_E_NONE;
    }
    if(0 == p_fea_port_policer_apply->out_policer_id)
    {
         return HSRV_E_NONE;
    }
    attr.id = SAI_PORT_ATTR_EGRESS_POLICER_ID;
    attr.value.oid = SAI_NULL_OBJECT_ID;

    /* added by hansf for bug 51208, remove policer from port should clear stats first */
    HSRV_IF_ERROR_RETURN(policer_api->clear_policer_statistics(p_fea_port_policer_apply->out_policer_id));

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, &attr));

    HSRV_IF_ERROR_RETURN(policer_api->remove_policer(p_fea_port_policer_apply->out_policer_id));
    p_fea_port_policer_apply->out_policer_id = 0;

    HSRV_IF_ERROR_RETURN(tbl_fea_port_policer_apply_set_fea_port_policer_apply_field(p_fea_port_policer_apply, TBL_FEA_PORT_POLICER_APPLY_FLD_OUT_POLICER_ID));

    return HSRV_E_NONE;
}

int32 hsrv_aclqos_if_set_in_policer_field(tbl_aclqos_if_t *p_aclqos_if)
{
    uint64 policer_oid = 0;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    sai_attribute_t     attr;
    tbl_interface_t* p_interface = NULL;
    sai_port_api_t*     port_api;

    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if->key.name);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
    HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_in_policer_creat_policer_profile(p_qos_policer_profile, p_aclqos_if, &policer_oid));

    attr.id = SAI_PORT_ATTR_POLICER_ID;
    attr.value.oid = policer_oid;

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, &attr));

    return HSRV_E_NONE;
}


int32 hsrv_aclqos_if_set_out_policer_field(tbl_aclqos_if_t *p_aclqos_if)
{
    uint64 policer_oid = 0;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    sai_attribute_t     attr;
    tbl_interface_t* p_interface = NULL;
    sai_port_api_t*     port_api;

    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if->key.name);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    
    p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
    HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_out_policer_creat_policer_profile(p_qos_policer_profile, p_aclqos_if, &policer_oid));

    attr.id = SAI_PORT_ATTR_EGRESS_POLICER_ID;
    attr.value.oid = policer_oid;

    HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(p_interface->portid, &attr));

    return HSRV_E_NONE;
}
int32
hsrv_acl_qos_if_get_port_policer_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats, bool is_ingress)
{
    sai_status_t       ret = SAI_STATUS_SUCCESS;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    sai_policer_api_t*  policer_api;
    sai_policer_stat_counter_t counter_ids[8];
    uint32_t number_of_counters = 0;;
    uint64_t counters[8];
    sai_object_id_t policer_id;

    sal_memset(&counters, 0x0,(sizeof(uint64_t))*8);
    sal_memset(&counter_ids, 0x0,(sizeof(sai_policer_stat_counter_t))*8);

    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
    if(is_ingress)
    {
        policer_id = p_fea_port_policer_apply->in_policer_id;
    }
    else
    {
         policer_id = p_fea_port_policer_apply->out_policer_id;
    }

    counter_ids[number_of_counters] = SAI_POLICER_STAT_PACKETS;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_ATTR_BYTES;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_PACKETS;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_GREEN_BYTES;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_PACKETS;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_YELLOW_BYTES;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_PACKETS;
    number_of_counters ++;
    counter_ids[number_of_counters] = SAI_POLICER_STAT_RED_BYTES;
    number_of_counters ++;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));

    HSRV_IF_ERROR_RETURN(policer_api->get_policer_statistics(policer_id, counter_ids, number_of_counters, counters));

    if(is_ingress)
    {
        p_aclqos_if_stats->green_packet = counters[2];
        p_aclqos_if_stats->green_byte = counters[3];

        p_aclqos_if_stats->yellow_packet = counters[4];
        p_aclqos_if_stats->yellow_byte = counters[5];

        p_aclqos_if_stats->red_packet = counters[6];
        p_aclqos_if_stats->red_byte = counters[7];

        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE));
        GLB_UNSET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER);
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS));
    }
    else
    {
        p_aclqos_if_stats->green_packet_out = counters[2];
        p_aclqos_if_stats->green_byte_out = counters[3];

        p_aclqos_if_stats->yellow_packet_out = counters[4];
        p_aclqos_if_stats->yellow_byte_out = counters[5];

        p_aclqos_if_stats->red_packet_out = counters[6];
        p_aclqos_if_stats->red_byte_out = counters[7];

        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT));
        GLB_UNSET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER);
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS));
    }

    return ret;
}

int32
hsrv_acl_qos_if_clear_port_policer_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats, bool is_ingress)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_policer_api_t*  policer_api;
    sai_object_id_t policer_id;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;

    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);

    if(is_ingress)
    {
        policer_id = p_fea_port_policer_apply->in_policer_id;
    }
    else
    {
         policer_id = p_fea_port_policer_apply->out_policer_id;
    }


    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
    HSRV_IF_ERROR_RETURN(policer_api->clear_policer_statistics(policer_id));

    return ret;
}

int32
hsrv_qos_get_queue_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats)
{
    sai_status_t       ret = SAI_STATUS_SUCCESS;
    sai_object_id_t queue_oid;
    uint32_t ctc_queue_id = 0;
    sai_queue_api_t*  queue_api;
    sai_queue_stat_counter_t counter_ids[4];
    uint32_t number_of_counters = 0;;
    uint64_t counters[4];
    tbl_interface_t* p_interface = NULL;
    uint32 queue_index = 0;
    uint32 port_id = 0;
    uint64 queue_transimt_byte[GLB_QOS_MAX_PORT_QUEUE_NUM] = {0};
    uint64 queue_transimt_pkt[GLB_QOS_MAX_PORT_QUEUE_NUM] = {0};
    uint64 queue_drop_byte[GLB_QOS_MAX_PORT_QUEUE_NUM] = {0};
    uint64 queue_drop_pkt[GLB_QOS_MAX_PORT_QUEUE_NUM] = {0};

    sal_memset(queue_transimt_byte,0, sizeof(queue_transimt_byte));
    sal_memset(queue_transimt_pkt,0, sizeof(queue_transimt_pkt));
    sal_memset(queue_drop_byte, 0, sizeof(queue_drop_byte));
    sal_memset(queue_drop_pkt, 0, sizeof(queue_drop_pkt));


    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QUEUE,(void**)&queue_api));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if_stats->key.name);
    port_id = CTC_SAI_OBJECT_INDEX_GET(p_interface->portid);
    for(queue_index = 0; queue_index < GLB_QOS_MAX_PORT_QUEUE_NUM; queue_index++)
    {
        number_of_counters = 0;
        counter_ids[number_of_counters] = SAI_QUEUE_STAT_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_QUEUE_STAT_BYTES;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_QUEUE_STAT_DROPPED_PACKETS;
        number_of_counters ++;
        counter_ids[number_of_counters] = SAI_QUEUE_STAT_DROPPED_BYTES;
        number_of_counters ++;

        ctc_queue_id = (port_id << 16) + queue_index;
        queue_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, ctc_queue_id);
        HSRV_IF_ERROR_RETURN(queue_api->get_queue_stats(queue_oid, counter_ids, number_of_counters, counters));
        
        /* mod by hansf for opimize queue stats message sync, only queue stats change sync cdb */
        queue_transimt_byte[queue_index] = counters[1] - counters[3];
        queue_transimt_pkt[queue_index] = counters[0] - counters[2];
        queue_drop_byte[queue_index] =  counters[3];
        queue_drop_pkt[queue_index] = counters[2];
    }
    
    if (sal_memcmp(p_aclqos_if_stats->queue_transimt_byte, queue_transimt_byte, sizeof(queue_transimt_byte)))
    {
        sal_memcpy(p_aclqos_if_stats->queue_transimt_byte, queue_transimt_byte, sizeof(queue_transimt_byte));
        sal_memcpy(p_aclqos_if_stats->queue_transimt_pkt, queue_transimt_pkt, sizeof(queue_transimt_pkt));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE));
    }
    if (sal_memcmp(p_aclqos_if_stats->queue_drop_byte, queue_drop_byte, sizeof(queue_drop_byte)))
    {
        sal_memcpy(p_aclqos_if_stats->queue_drop_byte, queue_drop_byte, sizeof(queue_drop_byte));
        sal_memcpy(p_aclqos_if_stats->queue_drop_pkt, queue_drop_pkt, sizeof(queue_drop_pkt));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT));
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE));
    }

    if (GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE))
    {
        GLB_UNSET_FLAG(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE);
        HSRV_IF_ERROR_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS));
    }

    return ret;
}

/**
 * clear qos queue statistics
 * @param queue_id:
 *     queue id, range is [-1, 7], -1 means clear all statistics
 */
int32
hsrv_qos_clear_queue_stats(tbl_aclqos_if_stats_t *p_aclqos_if_stats, int32 queue_id)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_queue_api_t*  queue_api;
    sai_object_id_t queue_oid;
    tbl_interface_t* p_interface = NULL;
    uint32 port_id = 0;
    uint32 queue_index = 0;
    uint32_t ctc_queue_id = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QUEUE,(void**)&queue_api));
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if_stats->key.name);
    port_id = CTC_SAI_OBJECT_INDEX_GET(p_interface->portid);

    if(queue_id < 0)
    {
        for(queue_index = 0; queue_index < GLB_QOS_MAX_PORT_QUEUE_NUM; queue_index++)
        {
            ctc_queue_id = (port_id << 16) + queue_index;
            queue_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, ctc_queue_id);
            /* clear */
            HSRV_IF_ERROR_RETURN(queue_api->clear_queue_stats(queue_oid));
        }
    }
    else
    {
        ctc_queue_id = (port_id << 16) + queue_id;
        queue_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, ctc_queue_id);
        /* clear */
        HSRV_IF_ERROR_RETURN(queue_api->clear_queue_stats(queue_oid));
    }

    return ret;
}

int32
hsrv_aclqos_if_update_l3if_qos_attr(tbl_rif_t *rif)
{
    tbl_aclqos_if_t *p_aclqos_if = NULL;
    tbl_aclqos_if_key_t key;
    sai_attribute_t     attr;
    sai_router_interface_api_t* rif_api = NULL;
    tbl_fea_qos_domain_key_t fea_qos_domain_key;
    tbl_fea_qos_domain_t* p_fea_qos_domain = NULL;

    sal_memset(&key, 0, sizeof(key));
    sal_strcpy(key.name, rif->key.name);

    p_aclqos_if = tbl_aclqos_if_get_aclqos_if(&key);
    if (!p_aclqos_if)
    {
        return HSRV_E_NONE;
    }
    fea_qos_domain_key.id = p_aclqos_if->dscp_domain;
    p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));
    
    attr.id = SAI_ROUTER_INTERFACE_ATTR_TRUST_DSCP;
    if (GLB_QOS_TRUST_DSCP == p_aclqos_if->trust)
    {
        attr.value.booldata = TRUE;
    }
    else
    {
        attr.value.booldata = FALSE;
    }
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(rif->rif_id, &attr));

    attr.id = SAI_ROUTER_INTERFACE_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;
    attr.value.oid = p_fea_qos_domain->dscp_to_tc_color_map_id;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(rif->rif_id, &attr));

    attr.id = SAI_ROUTER_INTERFACE_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
    attr.value.oid = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
    HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(rif->rif_id, &attr));    
    
    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_set_field(tbl_aclqos_if_t *p_aclqos_if, uint32 field_id)
{
    uint32 queue_index = 0;
    tbl_interface_t* p_interface = NULL;
    tbl_qos_drop_profile_t* p_qos_drop_profile = NULL;
    tbl_qos_queue_shape_profile_t* p_qos_queue_shape_profile = NULL;
    tbl_qos_port_shape_profile_t* p_qos_port_shape_profile = NULL;
    tbl_qos_domain_t* p_qos_domain = NULL;
    tbl_fea_qos_drop_profile_key_t fea_qos_drop_profile_key;
    tbl_fea_qos_queue_shape_profile_key_t fea_qos_queue_shape_profile_key;
    tbl_fea_qos_port_shape_profile_key_t fea_qos_port_shape_profile_key;
    tbl_fea_qos_domain_key_t fea_qos_domain_key;
    tbl_fea_qos_drop_profile_t* p_fea_qos_drop_profile = NULL;
    tbl_fea_qos_queue_shape_profile_t* p_fea_qos_queue_shape_profile = NULL;
    tbl_fea_qos_port_shape_profile_t* p_fea_qos_port_shape_profile = NULL;
    tbl_fea_qos_domain_t* p_fea_qos_domain = NULL;
    tbl_qos_queue_smart_buffer_t *p_db_queue_smart_buffer = NULL;
    tbl_qos_queue_smart_buffer_key_t queue_smart_buffer_key;
    sai_attribute_t     attr;
    sai_object_id_t     queue_oid = 0;
    sai_object_id_t     port_oid = 0;
    sai_queue_api_t*     queue_api;
    sai_port_api_t*     port_api;
    uint32 port_id = 0;
    uint32 sai_queue_id = 0;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    sai_router_interface_api_t* rif_api = NULL;
    tbl_rif_t       *p_rif  = NULL;
    tbl_rif_key_t   rif_key;
    
    sal_memset(&fea_qos_drop_profile_key,0x0, sizeof(tbl_fea_qos_drop_profile_key_t));
    sal_memset(&fea_qos_queue_shape_profile_key,0x0, sizeof(tbl_fea_qos_queue_shape_profile_key_t));
    sal_memset(&fea_qos_domain_key,0x0, sizeof(tbl_fea_qos_domain_key_t));
    sal_memset(&attr,0x0, sizeof(sai_attribute_t));
    sal_memset(&rif_key, 0, sizeof(tbl_rif_key_t));
    sal_strcpy(rif_key.name, p_aclqos_if->key.name);
    p_rif = tbl_rif_get_rif(&rif_key);
    p_interface = tbl_interface_get_interface_by_name(p_aclqos_if->key.name);
    port_id = (uint32)CTC_SAI_OBJECT_INDEX_GET(p_interface->portid);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QUEUE,(void**)&queue_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_PORT,(void**)&port_api));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_ROUTER_INTERFACE,(void**)&rif_api));

    if (GLB_IF_TYPE_VLAN_IF == p_aclqos_if->port_type)
    {
        //aclqos_vlan_if just care trust and dscp domain
        if (TBL_ACLQOS_IF_FLD_TRUST != field_id && TBL_ACLQOS_IF_FLD_DSCP_DOMAIN != field_id)
        {
            return HSRV_E_NONE;
        }
    }
    
    switch(field_id)
    {
        case TBL_ACLQOS_IF_FLD_QUEUE_DROP_PROFILE:
            for(queue_index = 0; queue_index < GLB_QOS_PORT_QUEUE_NUM; queue_index++)
            {
                sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
                queue_smart_buffer_key.queue_id = queue_index;
                p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
                if (p_db_queue_smart_buffer)
                {
                    if (p_db_queue_smart_buffer->smart_buffer_enable)
                    {
                        continue;
                    }
                }
                if(p_aclqos_if->queue_drop_profile[queue_index])
                {
                    p_qos_drop_profile = tbl_qos_drop_profile_get_profile_by_name(p_aclqos_if->queue_drop_profile[queue_index]);
                    if(p_qos_drop_profile)
                    {
                        fea_qos_drop_profile_key.id = p_qos_drop_profile->key.id;
                        p_fea_qos_drop_profile = tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(&fea_qos_drop_profile_key);
                        /*if exist old wred id, delete it first*/
                        attr.id = SAI_QUEUE_ATTR_WRED_PROFILE_ID;
                        attr.value.oid = SAI_NULL_OBJECT_ID;
                        sai_queue_id = (port_id << 16) + queue_index;
                        queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
                        queue_api->set_queue_attribute(queue_oid,&attr);
                        /*add new wred id*/
                        attr.id = SAI_QUEUE_ATTR_WRED_PROFILE_ID;
                        attr.value.oid = p_fea_qos_drop_profile->sai_drop_id;
                        sai_queue_id = (port_id << 16) + queue_index;
                        queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
                        HSRV_IF_ERROR_RETURN(queue_api->set_queue_attribute(queue_oid,&attr));
                    }
                }
            }
            break;
        case TBL_ACLQOS_IF_FLD_QUEUE_DROP_MODE:
            for(queue_index = 0; queue_index < GLB_QOS_PORT_QUEUE_NUM; queue_index++)
            {
#if 1           
                sal_memset(&queue_smart_buffer_key, 0, sizeof(queue_smart_buffer_key));
                queue_smart_buffer_key.queue_id = queue_index;
                p_db_queue_smart_buffer = tbl_qos_queue_smart_buffer_get_qos_queue_smart_buffer(&queue_smart_buffer_key);
                if (p_db_queue_smart_buffer)
                {
                    if (p_db_queue_smart_buffer->smart_buffer_enable)
                    {
                        continue;
                    }
                }    
                attr.id = SAI_QUEUE_ATTR_DROP_TYPE;
                if(GLB_QOS_QUEUE_DROP_MODE_WTD == p_aclqos_if->queue_drop_mode[queue_index])
                    attr.value.s32 = SAI_DROP_TYPE_TAIL; /*0 for WTD*/
                else
                    attr.value.s32 = SAI_DROP_TYPE_WRED;/*1 for WRED*/
                sai_queue_id = (port_id << 16) + queue_index;
                queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
                HSRV_IF_ERROR_RETURN(queue_api->set_queue_attribute(queue_oid,&attr));
#endif
            }
            break;
        case TBL_ACLQOS_IF_FLD_QUEUE_SHAPE_PROFILE:
            for(queue_index = 0; queue_index < GLB_QOS_PORT_QUEUE_NUM; queue_index++)
            {
                if(p_aclqos_if->queue_shape_profile[queue_index])
                {
                    p_qos_queue_shape_profile = tbl_qos_queue_shape_profile_get_profile_by_name(p_aclqos_if->queue_shape_profile[queue_index]);
                    if(p_qos_queue_shape_profile)
                    {
                        fea_qos_queue_shape_profile_key.id = p_qos_queue_shape_profile->key.id;
                        p_fea_qos_queue_shape_profile = tbl_fea_qos_queue_shape_profile_get_fea_qos_queue_shape_profile(&fea_qos_queue_shape_profile_key);
                        /*if exist old scheduler id, delete it first*/
                        attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
                        attr.value.oid = SAI_NULL_OBJECT_ID;
                        sai_queue_id = (port_id << 16) + queue_index;
                        queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
                        queue_api->set_queue_attribute(queue_oid,&attr);
                        /*add new scheduler id*/
                        attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
                        attr.value.oid = p_fea_qos_queue_shape_profile->sai_scheduler_id;
                        sai_queue_id = (port_id << 16) + queue_index;
                        queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
                        HSRV_IF_ERROR_RETURN(queue_api->set_queue_attribute(queue_oid,&attr));
                    }
                }
            }
            break;
        case TBL_ACLQOS_IF_FLD_PORT_SHAPE_PROFILE:
            p_qos_port_shape_profile = tbl_qos_port_shape_profile_get_profile_by_name(p_aclqos_if->port_shape_profile);
            fea_qos_port_shape_profile_key.id = p_qos_port_shape_profile->key.id;
            p_fea_qos_port_shape_profile = tbl_fea_qos_port_shape_profile_get_fea_qos_port_shape_profile(&fea_qos_port_shape_profile_key);
            port_oid = p_interface->portid;
            /*if exist old scheduler id, delete it first*/
            attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
            attr.value.oid = SAI_NULL_OBJECT_ID;
            port_api->set_port_attribute(port_oid,&attr);

            /*add new scheduler id*/
            attr.id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID;
            attr.value.oid = p_fea_qos_port_shape_profile->sai_scheduler_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
        case TBL_ACLQOS_IF_FLD_PORT_TC:
            attr.id = SAI_PORT_ATTR_DEFAULT_COS;   
            attr.value.u32 = p_aclqos_if->port_tc;
            port_oid = p_interface->portid;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
        case TBL_ACLQOS_IF_FLD_TRUST:
            if (GLB_IF_TYPE_VLAN_IF != p_aclqos_if->port_type)
            {
                attr.id = SAI_PORT_ATTR_QOS_TRUST;
                if(GLB_QOS_TRUST_PORT == p_aclqos_if->trust)
                {
                    attr.value.u32 = SAI_PORT_QOS_TRUST_PORT;
                }
                else if(GLB_QOS_TRUST_DSCP == p_aclqos_if->trust)
                {
                    attr.value.u32 = SAI_PORT_QOS_TRUST_DSCP;
                }
                else //default trust cos
                {
                    attr.value.u32 = SAI_PORT_QOS_TRUST_COS;
                }
                port_oid = p_interface->portid;
                HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            }
            
            if (p_rif && GLB_IF_MODE_L3 == p_interface->mode)
            {
                //l3_if exist, set l3_if trust dscp
                attr.id = SAI_ROUTER_INTERFACE_ATTR_TRUST_DSCP;
                if (GLB_QOS_TRUST_DSCP == p_aclqos_if->trust)
                {
                    attr.value.booldata = TRUE;
                }
                else
                {
                    attr.value.booldata = FALSE;
                }
                HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));
            }
            
            break;
        case TBL_ACLQOS_IF_FLD_REPLACE_DSCP:
            attr.id = SAI_PORT_ATTR_UPDATE_DSCP;
            attr.value.booldata = p_aclqos_if->replace_dscp;
            port_oid = p_interface->portid;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
        case TBL_ACLQOS_IF_FLD_REPLACE_COS:
            attr.id = SAI_PORT_ATTR_UPDATE_COS;
            attr.value.booldata = p_aclqos_if->replace_cos;
            port_oid = p_interface->portid;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
#ifdef USW
        case TBL_ACLQOS_IF_FLD_COS_DOMAIN:
            p_qos_domain = tbl_qos_domain_get_qos_domain_by_id(p_aclqos_if->cos_domain);
            fea_qos_domain_key.id = p_qos_domain->key.id;
            p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
            port_oid = p_interface->portid;
            attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;
            attr.value.oid = p_fea_qos_domain->dot1p_to_tc_color_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

            attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;
            attr.value.oid = p_fea_qos_domain->tc_and_color_to_dot1p_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
        case TBL_ACLQOS_IF_FLD_DSCP_DOMAIN:
            p_qos_domain = tbl_qos_domain_get_qos_domain_by_id(p_aclqos_if->dscp_domain);
            fea_qos_domain_key.id = p_qos_domain->key.id;
            p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
            if (GLB_IF_TYPE_VLAN_IF != p_aclqos_if->port_type)
            {
                port_oid = p_interface->portid;
                attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;
                attr.value.oid = p_fea_qos_domain->dscp_to_tc_color_map_id;
                HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

                attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
                attr.value.oid = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
                HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr)); 
            }

            if (p_rif && GLB_IF_MODE_L3 == p_interface->mode)
            {
                //l3_if exist, set l3_if dscp domain
                attr.id = SAI_ROUTER_INTERFACE_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;
                attr.value.oid = p_fea_qos_domain->dscp_to_tc_color_map_id;
                HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));

                attr.id = SAI_ROUTER_INTERFACE_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
                attr.value.oid = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
                HSRV_IF_ERROR_RETURN(rif_api->set_router_interface_attribute(p_rif->rif_id, &attr));
            } 
            break;
#else
        case TBL_ACLQOS_IF_FLD_DOMAIN:
            p_qos_domain = tbl_qos_domain_get_qos_domain_by_id(p_aclqos_if->domain);
            fea_qos_domain_key.id = p_qos_domain->key.id;
            p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
            port_oid = p_interface->portid;

            attr.id = SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP;
            attr.value.oid = p_fea_qos_domain->dot1p_to_tc_color_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

            attr.id = SAI_PORT_ATTR_QOS_EXP_TO_TC_AND_COLOR_MAP;
            attr.value.oid = p_fea_qos_domain->exp_to_tc_color_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

            attr.id = SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP;
            attr.value.oid = p_fea_qos_domain->dscp_to_tc_color_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

            attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP;
            attr.value.oid = p_fea_qos_domain->tc_and_color_to_dot1p_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            
            attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP;
            attr.value.oid = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));

            attr.id = SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_EXP_MAP;
            attr.value.oid = p_fea_qos_domain->tc_and_color_to_exp_map_id;
            HSRV_IF_ERROR_RETURN(port_api->set_port_attribute(port_oid,&attr));
            break;
#endif
        case TBL_ACLQOS_IF_FLD_INPUT_POLICER:
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
            if(p_qos_policer_profile)
                HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_set_in_policer_field(p_aclqos_if));
            else
                HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_delete_in_policer_field(p_aclqos_if));
            break;
        case TBL_ACLQOS_IF_FLD_OUTPUT_POLICER:
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
            if(p_qos_policer_profile)
                HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_set_out_policer_field(p_aclqos_if));
            else
                HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_delete_out_policer_field(p_aclqos_if));
            break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_stats_set_field(tbl_aclqos_if_stats_t *p_aclqos_if_stats, uint32 field_id)
{
    switch(field_id)
    {
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS:
            if(GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_INPUT_POLICER))
            {
                HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, TRUE));
            }
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS:
            if(GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_OUTPUT_POLICER))
            {
                HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, FALSE));
            }
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS:
            if(GLB_FLAG_ISSET(p_aclqos_if_stats->flags, GLB_ACLQOS_IF_STATS_FLAGS_PORT_GET_QUEUE))
            {
                hsrv_qos_get_queue_stats(p_aclqos_if_stats);
            }
            break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_init_policer_stats(char* ifname, bool is_input)
{
    tbl_aclqos_if_stats_t* p_aclqos_if_stats = NULL;
    int32 rc = PM_E_NONE;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    tbl_aclqos_if_stats_t aclqos_if_stats;

    sal_memset(&aclqos_if_stats, 0, sizeof(tbl_aclqos_if_stats_t));    
    p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(ifname);
    if(!p_aclqos_if_stats)
    {
        sal_strcpy(aclqos_if_stats.key.name, ifname);
        tbl_aclqos_if_stats_add_aclqos_if_stats(&aclqos_if_stats);
        p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(aclqos_if_stats.key.name);

    }
    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(p_aclqos_if_stats->key.name);
    if(!p_aclqos_if)
    {
        return HSRV_E_NONE;
    }
    p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
    if(!p_fea_port_policer_apply)
    {
        return HSRV_E_NONE;
    }

    if(is_input)
    {
        p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
        if(!p_qos_policer_profile)
        {
            return HSRV_E_NONE;
        }
        HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, TRUE));
        p_aclqos_if_stats->green_byte = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE));
        p_aclqos_if_stats->green_packet = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET));
        p_aclqos_if_stats->yellow_byte = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE));
        p_aclqos_if_stats->yellow_packet = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET));
        p_aclqos_if_stats->red_byte = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE));
        p_aclqos_if_stats->red_packet = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET));
    }
    else
    {
        p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
        if(!p_qos_policer_profile)
        {
            return HSRV_E_NONE;
        }
        HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, FALSE));
        
        p_aclqos_if_stats->green_byte_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT));
        p_aclqos_if_stats->green_packet_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT));
        p_aclqos_if_stats->yellow_byte_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT));
        p_aclqos_if_stats->yellow_packet_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT));
        p_aclqos_if_stats->red_byte_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT));
        p_aclqos_if_stats->red_packet_out = 0;
        PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT));
    }

    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_set_aclqos_if_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aclqos_if_stats_t aclqos_if_stats;
    tbl_aclqos_if_stats_t* p_aclqos_if_stats = NULL;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;
    int32 queue_id = 0;
    uint32 queue_index = 0;

    p_aclqos_if = tbl_aclqos_if_get_aclqos_if_by_name(argv[1]);
    p_db_if = tbl_interface_get_interface_by_name(argv[1]);
    if(!p_aclqos_if || !p_db_if)
    {
        CFG_INVALID_PARAM_RET("Interface %s doesn't exist.", argv[1]);
    }
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_INVALID_PARAM_RET("Interface %s isn't a physical port.", argv[1]);
    }
    
    sal_memset(&aclqos_if_stats, 0, sizeof(tbl_aclqos_if_stats_t));
    p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(argv[1]);
    if(!p_aclqos_if_stats)
    {
        sal_strcpy(aclqos_if_stats.key.name, argv[1]);
        tbl_aclqos_if_stats_add_aclqos_if_stats(&aclqos_if_stats);
        p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(aclqos_if_stats.key.name);
    }
    p_node = cdb_get_tbl(TBL_ACLQOS_IF_STATS);
    field_id = cdb_get_field_id(argv[0], p_node, &p_field);   
    switch(field_id)
    {
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_INPUT_POLICER_STATS:
            p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
            if(!p_fea_port_policer_apply)
            {
                CFG_INVALID_PARAM_RET("Policer is not applied on the interface!");
            }
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
            if(!p_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Policer profile is not exist!");
            }
            if(!GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                CFG_INVALID_PARAM_RET("The stats of policer profile is disable!");
            }
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, TRUE));
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_GET_OUTPUT_POLICER_STATS:
            p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
            if(!p_fea_port_policer_apply)
            {
                CFG_INVALID_PARAM_RET("Policer is not applied on the interface!");
            }
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
            if(!p_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Policer profile is not exist!");
            }
            if(!GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                CFG_INVALID_PARAM_RET("The stats of policer profile is disable!");
            }
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, FALSE));
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_INPUT_POLICER_STATS:
            p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
            if(!p_fea_port_policer_apply)
            {
                CFG_INVALID_PARAM_RET("Policer is not applied on the interface!");
            }
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->input_policer);
            if(!p_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Policer profile is not exist!");
            }
            if(!GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                CFG_INVALID_PARAM_RET("The stats of policer profile is disable!");
            }
            /*
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, TRUE));
            */
            /* BEGIN: clear cdb */
            p_aclqos_if_stats->green_byte = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE));
            p_aclqos_if_stats->green_packet = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET));
            p_aclqos_if_stats->yellow_byte = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE));
            p_aclqos_if_stats->yellow_packet = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET));
            p_aclqos_if_stats->red_byte = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE));
            p_aclqos_if_stats->red_packet = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET));
            /* END: clear cdb */

            /* BEGIN: clear sdk db */
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_clear_port_policer_stats(p_aclqos_if_stats, TRUE));
            /* END: clear sdk db */
            return rc;
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_PORT_CLEAR_OUTPUT_POLICER_STATS:
            p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if_stats->key.name);
            if(!p_fea_port_policer_apply)
            {
                CFG_INVALID_PARAM_RET("Policer is not applied on the interface!");
            }
            p_qos_policer_profile = tbl_qos_policer_profile_get_profile_by_name(p_aclqos_if->output_policer);
            if(!p_qos_policer_profile)
            {
                CFG_INVALID_PARAM_RET("Policer profile is not exist!");
            }
            if(!GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                CFG_INVALID_PARAM_RET("The stats of policer profile is disable!");
            }
            /*
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_get_port_policer_stats(p_aclqos_if_stats, FALSE));
            */
            
            /* BEGIN: clear cdb */
            p_aclqos_if_stats->green_byte_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_BYTE_OUT));
            p_aclqos_if_stats->green_packet_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_GREEN_PACKET_OUT));
            p_aclqos_if_stats->yellow_byte_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_BYTE_OUT));
            p_aclqos_if_stats->yellow_packet_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_YELLOW_PACKET_OUT));
            p_aclqos_if_stats->red_byte_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_BYTE_OUT));
            p_aclqos_if_stats->red_packet_out = 0;
            PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_RED_PACKET_OUT));
            /* END: clear cdb */

            /* BEGIN: clear sdk db */
            HSRV_IF_ERROR_RETURN(hsrv_acl_qos_if_clear_port_policer_stats(p_aclqos_if_stats, FALSE));
            /* END: clear sdk db */
            return rc;
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_GET_QUEUE_STATS:
            PM_E_RETURN(hsrv_qos_get_queue_stats(p_aclqos_if_stats));
            break;
        case TBL_ACLQOS_IF_STATS_FLD_FLAGS_CLEAR_QUEUE_STATS:
            queue_id = sal_atoi(argv[3]);
            /*
            PM_E_RETURN(hsrv_qos_get_queue_stats(p_aclqos_if_stats));
            */
            /* BEGIN: clear cdb */
            if(queue_id < 0)
            {
                for(queue_index = 0; queue_index < GLB_QOS_MAX_PORT_QUEUE_NUM; queue_index ++)
                {
                    p_aclqos_if_stats->queue_transimt_pkt[queue_index] = 0;
                    PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT));
                    p_aclqos_if_stats->queue_transimt_byte[queue_index]= 0;
                    PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE));
                    p_aclqos_if_stats->queue_drop_pkt[queue_index]= 0;
                    PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT));
                    p_aclqos_if_stats->queue_drop_byte[queue_index] = 0;
                    PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE));
                }
            }
            else
            {
                p_aclqos_if_stats->queue_transimt_pkt[queue_id] = 0;
                PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_PKT));
                p_aclqos_if_stats->queue_transimt_byte[queue_id]= 0;
                PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_TRANSIMT_BYTE));
                p_aclqos_if_stats->queue_drop_pkt[queue_id]= 0;
                PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_PKT));
                p_aclqos_if_stats->queue_drop_byte[queue_id] = 0;
                PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, TBL_ACLQOS_IF_STATS_FLD_QUEUE_DROP_BYTE));
            }
            /* END: clear cdb */

            /* BEGIN: clear sdk db */
            PM_E_RETURN(hsrv_qos_clear_queue_stats(p_aclqos_if_stats, queue_id));
            /* END: clear sdk db */
            return rc;
            break;
    }
    
    PM_E_RETURN(tbl_aclqos_if_stats_set_aclqos_if_stats_field(p_aclqos_if_stats, field_id));

    return rc;
}

int32
hsrv_aclqos_if_get_aclqos_if_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_aclqos_if_stats_t* p_aclqos_if_stats = NULL;
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;
    ctclib_slistnode_t *listnode = NULL;

    tbl_aclqos_if_stats_master_t *p_master = tbl_aclqos_if_stats_get_master();

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    p_node = cdb_get_tbl(TBL_ACLQOS_IF);
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[1] = fp;
    /*dump all aclqos_if*/
    if(argc == 0)
    {
        rc = cfg_cmd_parse_field(argv, 0, p_node, &field, FALSE);
        if (rc < 0)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        args.argv[0] = &field;
        CTCLIB_SLIST_LOOP(p_master->if_list, p_aclqos_if_stats, listnode)
        {
            tbl_aclqos_if_stats_dump_one(p_aclqos_if_stats, &args);
        }
        sal_fclose(fp);
        return PM_E_NONE;
    }
    p_aclqos_if_stats = tbl_aclqos_if_stats_get_aclqos_if_stats_by_name(argv[1]);
    if(!p_aclqos_if_stats)
    {
        sal_fclose(fp);
        return PM_E_NOT_EXIST;
    }

    if((1 == argc) || (2 == argc))
    {
        rc = cfg_cmd_parse_field(argv, 0, p_node, &field, FALSE);
        if (rc < 0)
        {
            sal_fclose(fp);
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        args.argv[0] = &field;
        tbl_aclqos_if_stats_dump_one(p_aclqos_if_stats, &args);
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_aclqos_if_sync
 * Purpose      : load aclqos_if cdb to sai          
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
hsrv_aclqos_if_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_aclqos_if_t *p_aclqos_if = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_aclqos_if = (tbl_aclqos_if_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_set_field(p_aclqos_if, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_aclqos_if_process_aclqos_if_stats(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    switch (para->oper)
    {

        case CDB_OPER_ADD:
      #if 0
        case CDB_OPER_DEL:
            break;
     #endif
        case CDB_OPER_SET:
            PM_E_RETURN(hsrv_aclqos_if_set_aclqos_if_stats(argv, argc, para));
            break;
        case CDB_OPER_GET:
            PM_E_RETURN(hsrv_aclqos_if_get_aclqos_if_stats(argv, argc, para));
            break;
        default:
            
            break;
    }
    return PM_E_NONE;
}

