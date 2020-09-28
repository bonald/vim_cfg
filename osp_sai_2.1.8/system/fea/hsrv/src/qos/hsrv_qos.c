#include "hsrv_inc.h"
#include "hsrv_qos.h"

/*CTC_QOS_COLOR_NONE is no refer in sai, use 3 instand. no one will use this color*/
uint32_t sys_to_sai_color_map[4] = {3, SAI_PACKET_COLOR_RED, SAI_PACKET_COLOR_YELLOW, SAI_PACKET_COLOR_GREEN};

extern int32
_fea_acl_action_update_by_policer_profile(sai_attribute_t *pst_attr, char* policer_profile_name);

extern int32
hsrv_aclqos_if_init_policer_stats(char* ifname, bool is_input);

int32
hsrv_qos_policer_action_map_sai_type(sai_attribute_t *attr, uint32 *attr_count, tbl_qos_policer_action_profile_t *p_db_action_profile)
{
    HSRV_PTR_CHECK(p_db_action_profile);
    /* green packet action */
    attr[*attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
    if (p_db_action_profile->confirm_action.drop_packet)
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    }
    (*attr_count) ++;
    
    attr[*attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_TC;
    if (p_db_action_profile->confirm_action.tc_valid)
    {
        attr[*attr_count].value.u8 = p_db_action_profile->confirm_action.tc;
    }
    else
    {
        attr[*attr_count].value.u8 = 0xFF;
    }
    (*attr_count) ++;

    /* yellow packet action */
    attr[*attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
    if (p_db_action_profile->exceed_action.drop_packet)
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    }
    (*attr_count) ++;

    attr[*attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_TC;
    if (p_db_action_profile->exceed_action.tc_valid)
    {
        attr[*attr_count].value.u8 = p_db_action_profile->exceed_action.tc;
    }
    else
    {
        attr[*attr_count].value.u8 = 0xFF;
    }
    (*attr_count) ++;

    /* red packet action */
    attr[*attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
    if (p_db_action_profile->violate_action.drop_packet)
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
    }
    else
    {
        attr[*attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
    }
    (*attr_count) ++;

    attr[*attr_count].id = SAI_POLICER_ATTR_RED_PACKET_TC;
    if (p_db_action_profile->violate_action.tc_valid)
    {
        attr[*attr_count].value.u8 = p_db_action_profile->violate_action.tc;
    }
    else
    {
        attr[*attr_count].value.u8 = 0xFF;
    }
    (*attr_count) ++;
    
    return 0;
}

int32
hsrv_qos_domain_creat_domain(uint8 domain_id)
{
    sai_qos_map_api_t*  qos_map_api = NULL;
    sai_object_id_t qos_map_id;
    sai_attribute_t attr[2];
    uint32_t count;
    sai_qos_map_t qos_map_list[64];
    tbl_fea_qos_domain_t fea_qos_domain;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    sal_memset(attr, 0x0, (sizeof(sai_attribute_t))*2);
    sal_memset(&fea_qos_domain, 0x0, sizeof(tbl_fea_qos_domain_t));
    fea_qos_domain.key.id = domain_id;

    /*creat dot1p to tc and color table*/
    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_COS_NUM; count ++)
    {
        qos_map_list[count].key.dot1p = count;
        qos_map_list[count].value.tc = count;
        qos_map_list[count].value.color = SAI_PACKET_COLOR_GREEN;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.dot1p_to_tc_color_map_id = qos_map_id;

    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_EXP_TO_TC_AND_COLOR;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_EXP_NUM; count ++)
    {
        qos_map_list[count].key.exp= count;
        qos_map_list[count].value.tc = count;
        qos_map_list[count].value.color = SAI_PACKET_COLOR_GREEN;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.exp_to_tc_color_map_id = qos_map_id;

    /*creat dscp to tc and color table*/
    sal_memset(attr, 0x0, (sizeof(sai_attribute_t))*2);
    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_DSCP_NUM; count ++)
    {
        qos_map_list[count].key.dscp = count;
        qos_map_list[count].value.tc = count/8;
        qos_map_list[count].value.color = SAI_PACKET_COLOR_GREEN;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.dscp_to_tc_color_map_id = qos_map_id;

    /*creat tc and color to dot1p table*/
    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_TC_NUM; count ++)
    {
        qos_map_list[count*3].key.tc = count;
        qos_map_list[count*3].key.color = SAI_PACKET_COLOR_GREEN;
        qos_map_list[count*3].value.dot1p = count;
        qos_map_list[count*3 + 1].key.tc = count;
        qos_map_list[count*3 + 1].key.color = SAI_PACKET_COLOR_YELLOW;
        qos_map_list[count*3 + 1].value.dot1p = count;
        qos_map_list[count*3 + 2].key.tc = count;
        qos_map_list[count*3 + 2].key.color = SAI_PACKET_COLOR_RED;
        qos_map_list[count*3 + 2].value.dot1p = count;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count * 3;  
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.tc_and_color_to_dot1p_map_id = qos_map_id;

    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_TC_AND_COLOR_TO_EXP;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_TC_NUM; count ++)
    {
        qos_map_list[count*3].key.tc = count;
        qos_map_list[count*3].key.color = SAI_PACKET_COLOR_GREEN;
        qos_map_list[count*3].value.exp = count;
        qos_map_list[count*3 + 1].key.tc = count;
        qos_map_list[count*3 + 1].key.color = SAI_PACKET_COLOR_YELLOW;
        qos_map_list[count*3 + 1].value.exp = count;
        qos_map_list[count*3 + 2].key.tc = count;
        qos_map_list[count*3 + 2].key.color = SAI_PACKET_COLOR_RED;
        qos_map_list[count*3 + 2].value.exp = count;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count * 3;  
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.tc_and_color_to_exp_map_id = qos_map_id;

    /*creat tc and color to dscp table*/
    attr[0].id = SAI_QOS_MAP_ATTR_TYPE;
    attr[0].value.s32 = SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP;

    attr[1].id = SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST;
    for(count = 0; count < GLB_QOS_TC_NUM; count ++)
    {
        qos_map_list[count*3].key.tc = count;
        qos_map_list[count*3].key.color = SAI_PACKET_COLOR_GREEN;
        qos_map_list[count*3].value.dscp = count *8;
        qos_map_list[count*3 + 1].key.tc = count;
        qos_map_list[count*3 + 1].key.color = SAI_PACKET_COLOR_YELLOW;
        qos_map_list[count*3 + 1].value.dscp = count *8;
        qos_map_list[count*3 + 2].key.tc = count;
        qos_map_list[count*3 + 2].key.color = SAI_PACKET_COLOR_RED;
        qos_map_list[count*3 + 2].value.dscp = count *8;
    }
    attr[1].value.qosmap.list = qos_map_list;
    attr[1].value.qosmap.count = count * 3;  
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->create_qos_map(&qos_map_id, 2, attr));
    fea_qos_domain.tc_and_color_to_dscp_map_id = qos_map_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_domain_add_fea_qos_domain(&fea_qos_domain));
    return HSRV_E_NONE;
}

int32
hsrv_qos_domain_delet_domain(uint8 domain_id)
{
    sai_status_t       ret = SAI_STATUS_SUCCESS;
    sai_qos_map_api_t*  qos_map_api;
    tbl_fea_qos_domain_t* p_fea_qos_domain = NULL;
    tbl_fea_qos_domain_key_t fea_qos_domain_key;

    sal_memset(&fea_qos_domain_key,0x0, sizeof(tbl_fea_qos_domain_key_t));
    fea_qos_domain_key.id = domain_id;
    p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
    if(NULL == p_fea_qos_domain)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->dot1p_to_tc_color_map_id));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->dscp_to_tc_color_map_id));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->tc_and_color_to_dot1p_map_id));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->tc_and_color_to_dscp_map_id));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->exp_to_tc_color_map_id));
    HSRV_IF_ERROR_RETURN(qos_map_api->remove_qos_map(p_fea_qos_domain->tc_and_color_to_exp_map_id));
    
    return ret;
}

int32
hsrv_qos_domain_set_field(tbl_qos_domain_t *p_qos_domain, uint32 field_id)
{
    sai_qos_map_api_t*  qos_map_api = NULL;
    sai_object_id_t qos_map_id;
    sai_attribute_t attr;
    sai_qos_map_t qos_map;
    uint32_t index = 0;
    uint8_t color_index = 0;
    tbl_fea_qos_domain_key_t fea_qos_domain_key;
    tbl_fea_qos_domain_t* p_fea_qos_domain = NULL;

    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    sal_memset(&qos_map, 0x0, sizeof(sai_qos_map_t));
    sal_memset(&fea_qos_domain_key, 0x0, sizeof(tbl_fea_qos_domain_key_t));

    fea_qos_domain_key.id = p_qos_domain->key.id;
    p_fea_qos_domain = tbl_fea_qos_domain_get_fea_qos_domain(&fea_qos_domain_key);
    /* modified by liul for gb osp, 2016-05-20 */
    if (NULL == p_fea_qos_domain)
    {
        return HSRV_E_NONE;
    }
    switch(field_id)
    {
        case TBL_QOS_DOMAIN_FLD_COS2COLOR:
        case TBL_QOS_DOMAIN_FLD_COS2TC:
            for(index = 0; index < GLB_QOS_COS_NUM; index ++ )
            {
                attr.id = SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR;
                qos_map.key.dot1p = index;
                qos_map.value.tc = p_qos_domain->cos2tc[index];
                qos_map.value.color = sys_to_sai_color_map[p_qos_domain->cos2color[index]];
                attr.value.qosmap.count = 1;
                attr.value.qosmap.list = &qos_map;
                qos_map_id = p_fea_qos_domain->dot1p_to_tc_color_map_id;
                HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
            }
            break;
        case TBL_QOS_DOMAIN_FLD_EXP2COLOR:
        case TBL_QOS_DOMAIN_FLD_EXP2TC:
            for(index = 0; index < GLB_QOS_EXP_NUM; index ++ )
            {
                attr.id = SAI_QOS_MAP_EXP_TO_TC_AND_COLOR;
                qos_map.key.exp = index;
                qos_map.value.tc = p_qos_domain->exp2tc[index];
                qos_map.value.color = sys_to_sai_color_map[p_qos_domain->exp2color[index]];
                attr.value.qosmap.count = 1;
                attr.value.qosmap.list = &qos_map;
                qos_map_id = p_fea_qos_domain->exp_to_tc_color_map_id;
                HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
            }
            break;
        case TBL_QOS_DOMAIN_FLD_DSCP2COLOR:
        case TBL_QOS_DOMAIN_FLD_DSCP2TC:
            for(index = 0; index < GLB_QOS_DSCP_NUM; index ++ )
            {
                attr.id = SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR;
                qos_map.key.dscp = index;
                qos_map.value.tc = p_qos_domain->dscp2tc[index];
                qos_map.value.color = sys_to_sai_color_map[p_qos_domain->dscp2color[index]];
                attr.value.qosmap.count = 1;
                attr.value.qosmap.list = &qos_map;
                qos_map_id = p_fea_qos_domain->dscp_to_tc_color_map_id;
                HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
            }
            break;
        case TBL_QOS_DOMAIN_FLD_TC_COLOR2COS:
            for(index = 0; index < GLB_QOS_TC_NUM; index ++ )
            {
                for(color_index = 1; color_index < GLB_QOS_COLOR_NUM; color_index ++)
                {
                    attr.id = SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P;
                    qos_map.key.tc = index;
                    qos_map.key.color = sys_to_sai_color_map[color_index];
                    qos_map.value.dot1p = p_qos_domain->tc_color2cos[(index <<2) + color_index];
                    attr.value.qosmap.count = 1;
                    attr.value.qosmap.list = &qos_map;
                    qos_map_id = p_fea_qos_domain->tc_and_color_to_dot1p_map_id;
                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                    HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
                }
            }
            break;
        case TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP:
            for(index = 0; index < GLB_QOS_TC_NUM; index ++ )
            {
                for(color_index = 1; color_index < GLB_QOS_COLOR_NUM; color_index ++)
                {
                    attr.id = SAI_QOS_MAP_TC_AND_COLOR_TO_EXP;
                    qos_map.key.tc = index;
                    qos_map.key.color = sys_to_sai_color_map[color_index];
                    qos_map.value.exp = p_qos_domain->tc_color2exp[(index <<2) + color_index];
                    attr.value.qosmap.count = 1;
                    attr.value.qosmap.list = &qos_map;
                    qos_map_id = p_fea_qos_domain->tc_and_color_to_exp_map_id;
                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                    HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
                }
            }
            break;
        case TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP:
            for(index = 0; index < GLB_QOS_TC_NUM; index ++ )
            {
                for(color_index = 0; color_index < GLB_QOS_COLOR_NUM; color_index ++)
                {
                    attr.id = SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP;
                    qos_map.key.tc = index;
                    qos_map.key.color = sys_to_sai_color_map[color_index];
                    qos_map.value.dscp= p_qos_domain->tc_color2dscp[(index <<2) + color_index];
                    attr.value.qosmap.count = 1;
                    attr.value.qosmap.list = &qos_map;
                    qos_map_id = p_fea_qos_domain->tc_and_color_to_dscp_map_id;
                    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
                    HSRV_IF_ERROR_RETURN(qos_map_api->set_qos_map_attribute(qos_map_id, &attr));
                }
            }
            break;
         default:
            break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_policer_set_policer_action_profile(tbl_qos_policer_action_profile_t *p_qos_action_profile, uint32 field_id)
{
    sai_object_id_t policer_oid;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[2] = {{0}};
    uint32 attr_count = 0;
    uint32 index = 0;
    uint32 id = 0;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_qos_policer_profile_t *p_db_qos_policer_profile = NULL;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;

    sal_memset(attr, 0x0, sizeof(attr));
    switch(field_id)
    {
        case TBL_QOS_POLICER_ACTION_PROFILE_FLD_CONFIRM_ACTION:
            attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
            if (p_qos_action_profile->confirm_action.drop_packet)
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
            }
            else
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
            }
            attr_count++;

            attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_TC;
            if (p_qos_action_profile->confirm_action.tc_valid)
            {
                attr[attr_count].value.u8 = p_qos_action_profile->confirm_action.tc;
            }
            else
            {
                attr[attr_count].value.u8 = 0xFF;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_ACTION_PROFILE_FLD_EXCEED_ACTION:
            attr[attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
            if (p_qos_action_profile->exceed_action.drop_packet)
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
            }
            else
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
            }
            attr_count++;

            attr[attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_TC;
            if (p_qos_action_profile->exceed_action.tc_valid)
            {
                attr[attr_count].value.u8 = p_qos_action_profile->exceed_action.tc;
            }
            else
            {
                attr[attr_count].value.u8 = 0xFF;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_ACTION_PROFILE_FLD_VIOLATE_ACTION:
            attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
            if (p_qos_action_profile->violate_action.drop_packet)
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
            }
            else
            {
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
            }
            attr_count++;

            attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_TC;
            if (p_qos_action_profile->violate_action.tc_valid)
            {
                attr[attr_count].value.u8 = p_qos_action_profile->violate_action.tc;
            }
            else
            {
                attr[attr_count].value.u8 = 0xFF;
            }
            attr_count++;
            break;
        default:
            return HSRV_E_NONE;
    }

    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();
    tbl_qos_policer_profile_master_t *p_policer_master = tbl_qos_policer_profile_get_master();

    for (id = 0; id < GLB_QOS_POLICER_PROFILE_NUM; id++)
    {
        p_db_qos_policer_profile = p_policer_master->qos_policer_profile_array[id];
        if (NULL == p_db_qos_policer_profile)
        {
            continue;
        }
        if (0 != sal_strcmp(p_db_qos_policer_profile->action_profile_name, p_qos_action_profile->name))
        {
            continue;
        }
        CTCLIB_SLIST_LOOP(p_master->if_list, p_aclqos_if, listnode)
        {
            if((!sal_strcmp(p_aclqos_if->input_policer, p_db_qos_policer_profile->name)) || (!sal_strcmp(p_aclqos_if->output_policer, p_db_qos_policer_profile->name)))
            {
                 p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
                 if(p_fea_port_policer_apply->in_policer_id)
                 {
                     policer_oid = p_fea_port_policer_apply->in_policer_id;
                     HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
                     for (index = 0; index < attr_count; index ++)
                     {
                        HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(policer_oid, &attr[index]));
                     }
                 }
                 if(p_fea_port_policer_apply->out_policer_id)
                 {
                     policer_oid = p_fea_port_policer_apply->out_policer_id;
                     HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
                     for (index = 0; index < attr_count; index ++)
                     {
                        HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(policer_oid, &attr[index]));
                     }
                 }
            }
        }
        
        for (index = 0; index < attr_count; index ++)
        {
            HSRV_IF_ERROR_RETURN(_fea_acl_action_update_by_policer_profile(&attr[index], p_db_qos_policer_profile->name));
        }
        
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_policer_set_policer_profile(tbl_qos_policer_profile_t *p_qos_policer_profile, uint32 field_id)
{
    sai_object_id_t policer_oid;
    sai_policer_api_t*  policer_api;
    sai_attribute_t attr[8] = {{0}};
    uint32 attr_count = 0;
    uint32 index = 0;
    tbl_aclqos_if_t* p_aclqos_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_fea_port_policer_apply_t* p_fea_port_policer_apply = NULL;
    tbl_qos_policer_action_profile_t *p_db_action_profile = NULL;
    uint32 init_stats = FALSE;

    sal_memset(attr, 0x0, sizeof(attr));
    switch(field_id)
    {
        case TBL_QOS_POLICER_PROFILE_FLD_ALGORITHM_MODE:
            if(GLB_QOS_POLICER_MODE_RFC2697 == p_qos_policer_profile->algorithm_mode)
            {
                attr[attr_count].id = SAI_POLICER_ATTR_MODE;
                attr[attr_count].value.s32 = SAI_POLICER_MODE_Sr_TCM;
            }
            else if(GLB_QOS_POLICER_MODE_RFC4115 == p_qos_policer_profile->algorithm_mode)
            {
                attr[attr_count].id = SAI_POLICER_ATTR_MODE;
                attr[attr_count].value.s32 = SAI_POLICER_MODE_Tr_TCM;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_IS_PPS:
            if (p_qos_policer_profile->is_pps)
            {
                attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
                attr[attr_count].value.s32 = SAI_METER_TYPE_PACKETS;
            }
            else
            {
                attr[attr_count].id = SAI_POLICER_ATTR_METER_TYPE;
                attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_COLOR_MODE:
            if(p_qos_policer_profile->color_mode)
            {
                attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
                attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_AWARE;
            }
            else
            {
                attr[attr_count].id = SAI_POLICER_ATTR_COLOR_SOURCE;
                attr[attr_count].value.s32 = SAI_POLICER_COLOR_SOURCE_BLIND;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_CIR:
            attr[attr_count].id = SAI_POLICER_ATTR_CIR;
            if (p_qos_policer_profile->is_pps)
            {
                attr[attr_count].value.u64 = p_qos_policer_profile->cir;
            }
            else
            {
                attr[attr_count].value.u64 = p_qos_policer_profile->cir;
                attr[attr_count].value.u64 = attr[attr_count].value.u64 *1000/8;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_CBS:
            attr[attr_count].id = SAI_POLICER_ATTR_CBS;
            attr[attr_count].value.u64 = p_qos_policer_profile->cbs;
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_EIR_OR_PIR:
            attr[attr_count].id = SAI_POLICER_ATTR_PIR;
            if (p_qos_policer_profile->is_pps)
            {
                attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
            }
            else
            {
                attr[attr_count].value.u64 = p_qos_policer_profile->eir_or_pir;
                attr[attr_count].value.u64 = attr[attr_count].value.u64  *1000/8;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_EBS_OR_PBS:
            attr[attr_count].id = SAI_POLICER_ATTR_PBS;
            attr[attr_count].value.u64 = p_qos_policer_profile->ebs_or_pbs;
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_FLAGS_STATS:
            attr[attr_count].id = SAI_POLICER_ATTR_ENABLE_COUNTER_LIST;
            attr[attr_count].value.s32 = 1;
            if(!GLB_FLAG_ISSET(p_qos_policer_profile->flags, GLB_POLICER_PROFILE_FLAGS_STATS))
            {
                attr[attr_count].value.s32 = 0;
            }
            else
            {
                init_stats = TRUE;
            }
            attr_count++;
            break;
        case TBL_QOS_POLICER_PROFILE_FLD_ACTION_PROFILE_NAME:
            p_db_action_profile = tbl_qos_policer_action_profile_get_profile_by_name(p_qos_policer_profile->action_profile_name);
            if (p_db_action_profile)
            {
                hsrv_qos_policer_action_map_sai_type(attr, &attr_count, p_db_action_profile);
            }
            else
            {   
                /* set defalut color action */
                attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_ACTION;
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
                attr_count++;

                attr[attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_ACTION;
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_FORWARD;
                attr_count++;

                attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_ACTION;
                attr[attr_count].value.s32 = SAI_PACKET_ACTION_DROP;
                attr_count++;

                attr[attr_count].id = SAI_POLICER_ATTR_GREEN_PACKET_TC;
                attr[attr_count].value.u8 = 0xFF;
                attr_count++;

                attr[attr_count].id = SAI_POLICER_ATTR_YELLOW_PACKET_TC;
                attr[attr_count].value.u8 = 0xFF;
                attr_count++;

                attr[attr_count].id = SAI_POLICER_ATTR_RED_PACKET_TC;
                attr[attr_count].value.u8 = 0xFF;
                attr_count++;
            }
            break;
        default:
            return HSRV_E_NONE;
    }

    tbl_aclqos_if_master_t *p_master = tbl_aclqos_if_get_master();

    CTCLIB_SLIST_LOOP(p_master->if_list, p_aclqos_if, listnode)
    {
        if((!sal_strcmp(p_aclqos_if->input_policer, p_qos_policer_profile->name)) || (!sal_strcmp(p_aclqos_if->output_policer, p_qos_policer_profile->name)))
        {
             p_fea_port_policer_apply = tbl_fea_port_policer_apply_get_fea_port_policer_apply_by_name(p_aclqos_if->key.name);
             if(p_fea_port_policer_apply->in_policer_id && 0== sal_strcmp(p_aclqos_if->input_policer, p_qos_policer_profile->name))
             {
                 policer_oid = p_fea_port_policer_apply->in_policer_id;
                 HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
                 for (index = 0; index < attr_count; index ++)
                 {
                    HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(policer_oid, &attr[index]));
                 }
                 if(init_stats)
                 {
                     HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_init_policer_stats(p_aclqos_if->key.name, TRUE));
                 }
             }
             if(p_fea_port_policer_apply->out_policer_id && 0 == sal_strcmp(p_aclqos_if->output_policer, p_qos_policer_profile->name))
             {
                 policer_oid = p_fea_port_policer_apply->out_policer_id;
                 HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_POLICER,(void**)&policer_api));
                 for (index = 0; index < attr_count; index ++)
                 {
                    HSRV_IF_ERROR_RETURN(policer_api->set_policer_attribute(policer_oid, &attr[index]));
                 }
                  if(init_stats)
                 {
                     HSRV_IF_ERROR_RETURN(hsrv_aclqos_if_init_policer_stats(p_aclqos_if->key.name, FALSE));
                 }
             }
        }
    }
    for (index = 0; index < attr_count; index ++)
    {
        HSRV_IF_ERROR_RETURN(_fea_acl_action_update_by_policer_profile(&attr[index], p_qos_policer_profile->name));
    }
    return HSRV_E_NONE;
}

int32
hsrv_qos_drop_creat_drop_profile(tbl_qos_drop_profile_t * p_qos_drop_profile)
{
    sai_object_id_t wred_id;
    sai_wred_api_t*  wred_api;
    sai_attribute_t attr[15];
    uint32_t attr_count = 0;
    tbl_fea_qos_drop_profile_t fea_qos_drop_profile;

    sal_memset(attr, 0x0, (sizeof(sai_attribute_t))*15);
    if(GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_GREEN_MAX_THRD))
    {
        attr[attr_count].id = SAI_WRED_ATTR_GREEN_ENABLE;
        attr[attr_count].value.booldata = TRUE;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_GREEN_MAX_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->green_max_threashold;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_GREEN_MIN_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->green_min_threashold;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_GREEN_DROP_PROBABILITY;
        attr[attr_count].value.s32 = p_qos_drop_profile->green_probability;
        attr_count ++;
    }

    if(GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_YELLOW_MAX_THRD))
    {
        attr[attr_count].id = SAI_WRED_ATTR_YELLOW_ENABLE;
        attr[attr_count].value.booldata = TRUE;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->yellow_max_threashold;
        attr_count ++;
        
        attr[attr_count].id = SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->yellow_min_threashold;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY;
        attr[attr_count].value.s32 = p_qos_drop_profile->yellow_probability;
        attr_count ++;
    }

    if(GLB_FLAG_ISSET(p_qos_drop_profile->flags, GLB_DROP_PROFILE_FLAGS_RED_MAX_THRD))
    {
        attr[attr_count].id = SAI_WRED_ATTR_RED_ENABLE;
        attr[attr_count].value.booldata = TRUE;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_RED_MAX_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->red_max_threashold;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_RED_MIN_THRESHOLD;
        attr[attr_count].value.s32 = p_qos_drop_profile->red_min_threashold;
        attr_count ++;

        attr[attr_count].id = SAI_WRED_ATTR_RED_DROP_PROBABILITY;
        attr[attr_count].value.s32 = p_qos_drop_profile->red_probability;
        attr_count ++;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_WRED,(void**)&wred_api));
    
    HSRV_IF_ERROR_RETURN(wred_api->create_wred_profile(&wred_id,attr_count,attr));

    fea_qos_drop_profile.key.id = p_qos_drop_profile->key.id;
    fea_qos_drop_profile.sai_drop_id = wred_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_drop_profile_add_fea_qos_drop_profile(&fea_qos_drop_profile));

    return HSRV_E_NONE;
}

int32
hsrv_qos_drop_delet_drop_profile(uint32 key_id)
{
    sai_object_id_t wred_oid;
    sai_wred_api_t*  wred_api;
    tbl_fea_qos_drop_profile_key_t fea_qos_drop_profile_key;
    tbl_fea_qos_drop_profile_t* p_fea_qos_drop_profile = NULL;

    fea_qos_drop_profile_key.id = key_id;
    p_fea_qos_drop_profile = tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(&fea_qos_drop_profile_key);
    wred_oid = p_fea_qos_drop_profile->sai_drop_id;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_WRED,(void**)&wred_api));

    HSRV_IF_ERROR_RETURN(wred_api->remove_wred_profile(wred_oid));

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_drop_profile_del_fea_qos_drop_profile(&fea_qos_drop_profile_key));

    return HSRV_E_NONE;
}

int32
hsrv_qos_drop_set_drop_profile(tbl_qos_drop_profile_t * p_qos_drop_profile, uint32 field_id)
{
    sai_object_id_t wred_oid;
    sai_wred_api_t*  wred_api;
    sai_attribute_t attr;
    tbl_fea_qos_drop_profile_key_t fea_qos_drop_profile_key;
    tbl_fea_qos_drop_profile_t* p_fea_qos_drop_profile = NULL;

    fea_qos_drop_profile_key.id = p_qos_drop_profile->key.id;
    p_fea_qos_drop_profile = tbl_fea_qos_drop_profile_get_fea_qos_drop_profile(&fea_qos_drop_profile_key);
    wred_oid = p_fea_qos_drop_profile->sai_drop_id;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_WRED,(void**)&wred_api));
    switch(field_id)
    {
        case TBL_QOS_DROP_PROFILE_FLD_GREEN_MAX_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_GREEN_MIN_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_GREEN_PROBABILITY:
            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_GREEN_ENABLE;
            attr.value.booldata = TRUE;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_GREEN_MAX_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->green_max_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_GREEN_MIN_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->green_min_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_GREEN_DROP_PROBABILITY;
            attr.value.s32 = p_qos_drop_profile->green_probability;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));
            break;
        case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MAX_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_YELLOW_MIN_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_YELLOW_PROBABILITY:
            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_YELLOW_ENABLE;
            attr.value.booldata = TRUE;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_YELLOW_MAX_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->yellow_max_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_YELLOW_MIN_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->yellow_min_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_YELLOW_DROP_PROBABILITY;
            attr.value.s32 = p_qos_drop_profile->yellow_probability;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));
            break;
        case TBL_QOS_DROP_PROFILE_FLD_RED_MAX_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_RED_MIN_THREASHOLD:
        case TBL_QOS_DROP_PROFILE_FLD_RED_PROBABILITY:
            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_RED_ENABLE;
            attr.value.booldata = TRUE;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_RED_MAX_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->red_max_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_RED_MIN_THRESHOLD;
            attr.value.s32 = p_qos_drop_profile->red_min_threashold;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));

            sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
            attr.id = SAI_WRED_ATTR_RED_DROP_PROBABILITY;
            attr.value.s32 = p_qos_drop_profile->red_probability;
            HSRV_IF_ERROR_RETURN(wred_api->set_wred_attribute(wred_oid,&attr));
            break;
        default:
            break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_scheduler_creat_scheduler_profile(tbl_qos_queue_shape_profile_t* p_qos_scheduler_profile)
{
    sai_object_id_t scheduler_id;
    sai_scheduler_api_t*  scheduler_api;
    sai_attribute_t attr[15];
    uint32_t attr_count = 0;
    tbl_fea_qos_queue_shape_profile_t fea_qos_scheduler_profile;

    sal_memset(attr, 0x0, (sizeof(sai_attribute_t))*15);
    if(GLB_FLAG_ISSET(p_qos_scheduler_profile->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_MODE))
    {
        attr[attr_count].id = SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM;
        if(SCHEDULER_MODE_SP == p_qos_scheduler_profile->mode)
        {
            attr[attr_count].value.s32 = SAI_SCHEDULING_STRICT;
        }
        else
        {
            attr[attr_count].value.s32 = SAI_SCHEDULING_DWRR;
        }
        attr_count ++;
    }
    if(GLB_FLAG_ISSET(p_qos_scheduler_profile->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_WEIGHT))
    {
        attr[attr_count].id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
        attr[attr_count].value.s32 = p_qos_scheduler_profile->queue_weight;
        attr_count ++;
    }

    attr[attr_count].id = SAI_SCHEDULER_ATTR_SHAPER_TYPE;
    attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
    attr_count ++;

    if(GLB_FLAG_ISSET(p_qos_scheduler_profile->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_CIR))
    {
        attr[attr_count].id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
        attr[attr_count].value.u64 = p_qos_scheduler_profile->cir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64 *1000/8;
        attr_count ++;
    }

    if(GLB_FLAG_ISSET(p_qos_scheduler_profile->flags, GLB_QUEUE_SHAPE_PROFILE_FLAGS_PIR))
    {
        attr[attr_count].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
        attr[attr_count].value.u64 = p_qos_scheduler_profile->pir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64 *1000/8;
        attr_count ++;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));

    HSRV_IF_ERROR_RETURN(scheduler_api->create_scheduler_profile(&scheduler_id,attr_count,attr));

    fea_qos_scheduler_profile.key.id = p_qos_scheduler_profile->key.id;
    fea_qos_scheduler_profile.sai_scheduler_id= scheduler_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_queue_shape_profile_add_fea_qos_queue_shape_profile(&fea_qos_scheduler_profile));

    return HSRV_E_NONE;
}

int32
hsrv_qos_scheduler_delet_scheduler_profile(uint32 key_id)
{
    sai_object_id_t scheduler_oid;
    sai_scheduler_api_t*  scheduler_api;

    tbl_fea_qos_queue_shape_profile_key_t fea_qos_scheduler_profile_key;
    tbl_fea_qos_queue_shape_profile_t* p_fea_qos_scheduler_profile = NULL;

    fea_qos_scheduler_profile_key.id = key_id;
    p_fea_qos_scheduler_profile = tbl_fea_qos_queue_shape_profile_get_fea_qos_queue_shape_profile(&fea_qos_scheduler_profile_key);
    scheduler_oid = p_fea_qos_scheduler_profile->sai_scheduler_id;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));

    HSRV_IF_ERROR_RETURN(scheduler_api->remove_scheduler_profile(scheduler_oid));

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_queue_shape_profile_del_fea_qos_queue_shape_profile(&fea_qos_scheduler_profile_key));

    return HSRV_E_NONE;
}

int32
hsrv_qos_scheduler_set_scheduler_profile(tbl_qos_queue_shape_profile_t * p_qos_scheduler_profile, uint32 field_id)
{
    sai_object_id_t scheduler_oid;
    sai_scheduler_api_t*  scheduler_api;
    sai_attribute_t attr;
    tbl_fea_qos_queue_shape_profile_key_t fea_qos_scheduler_profile_key;
    tbl_fea_qos_queue_shape_profile_t* p_fea_qos_scheduler_profile = NULL;

    fea_qos_scheduler_profile_key.id = p_qos_scheduler_profile->key.id;
    p_fea_qos_scheduler_profile = tbl_fea_qos_queue_shape_profile_get_fea_qos_queue_shape_profile(&fea_qos_scheduler_profile_key);
    scheduler_oid = p_fea_qos_scheduler_profile->sai_scheduler_id;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));
    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    switch(field_id)
    {
        case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_MODE:
            attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_ALGORITHM;
            if(SCHEDULER_MODE_SP == p_qos_scheduler_profile->mode)
            {
                attr.value.s32 = SAI_SCHEDULING_STRICT;
            }
            else
            {
                attr.value.s32 = SAI_SCHEDULING_DWRR;
            }
            break;
        case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_QUEUE_WEIGHT:
            attr.id = SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT;
            attr.value.s32 = p_qos_scheduler_profile->queue_weight;
            break;
        case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_CIR:
            attr.id = SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE;
            attr.value.u64 = p_qos_scheduler_profile->cir;
            attr.value.u64 = attr.value.u64 *1000/8;
            break;
        case TBL_QOS_QUEUE_SHAPE_PROFILE_FLD_PIR:
            attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
            attr.value.u64 = p_qos_scheduler_profile->pir;
            attr.value.u64 = attr.value.u64 *1000/8;
            break;
        default:
            return HSRV_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(scheduler_api->set_scheduler_attribute(scheduler_oid,&attr));

    return HSRV_E_NONE;
}

int32
hsrv_qos_port_shape_creat_port_shape_profile(tbl_qos_port_shape_profile_t* p_qos_port_shape_profile)
{
    sai_object_id_t scheduler_id;
    sai_scheduler_api_t*  scheduler_api;
    sai_attribute_t attr[15];
    uint32_t attr_count = 0;
    tbl_fea_qos_port_shape_profile_t fea_qos_port_shape_profile;

    sal_memset(attr, 0x0, (sizeof(sai_attribute_t))*15);

    attr[attr_count].id = SAI_SCHEDULER_ATTR_SHAPER_TYPE;
    attr[attr_count].value.s32 = SAI_METER_TYPE_BYTES;
    attr_count ++;

    if(GLB_FLAG_ISSET(p_qos_port_shape_profile->flags, GLB_PORT_SHAPE_PROFILE_FLAGS_PIR))
    {
        attr[attr_count].id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
        attr[attr_count].value.u64 = p_qos_port_shape_profile->pir;
        attr[attr_count].value.u64 = attr[attr_count].value.u64 *1000/8;
        attr_count ++;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));

    HSRV_IF_ERROR_RETURN(scheduler_api->create_scheduler_profile(&scheduler_id,attr_count,attr));

    fea_qos_port_shape_profile.key.id = p_qos_port_shape_profile->key.id;
    fea_qos_port_shape_profile.sai_scheduler_id= scheduler_id;

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_port_shape_profile_add_fea_qos_port_shape_profile(&fea_qos_port_shape_profile));

    return HSRV_E_NONE;
}

int32
hsrv_qos_port_shape_delet_port_shape_profile(uint32 key_id)
{
    sai_object_id_t scheduler_oid;
    sai_scheduler_api_t*  scheduler_api;

    tbl_fea_qos_port_shape_profile_key_t fea_qos_port_shape_profile_key;
    tbl_fea_qos_port_shape_profile_t* p_fea_qos_port_shape_profile = NULL;

    fea_qos_port_shape_profile_key.id = key_id;
    p_fea_qos_port_shape_profile = tbl_fea_qos_port_shape_profile_get_fea_qos_port_shape_profile(&fea_qos_port_shape_profile_key);
    scheduler_oid = p_fea_qos_port_shape_profile->sai_scheduler_id;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));

    HSRV_IF_ERROR_RETURN(scheduler_api->remove_scheduler_profile(scheduler_oid));

    HSRV_IF_ERROR_RETURN(tbl_fea_qos_port_shape_profile_del_fea_qos_port_shape_profile(&fea_qos_port_shape_profile_key));

    return HSRV_E_NONE;
}

int32
hsrv_qos_port_shape_set_port_shape_profile(tbl_qos_port_shape_profile_t * p_qos_port_shape_profile, uint32 field_id)
{
    sai_object_id_t scheduler_oid;
    sai_scheduler_api_t*  scheduler_api;
    sai_attribute_t attr;
    tbl_fea_qos_port_shape_profile_key_t fea_qos_port_shape_profile_key;
    tbl_fea_qos_port_shape_profile_t* p_fea_qos_port_shape_profile = NULL;

    fea_qos_port_shape_profile_key.id = p_qos_port_shape_profile->key.id;
    p_fea_qos_port_shape_profile = tbl_fea_qos_port_shape_profile_get_fea_qos_port_shape_profile(&fea_qos_port_shape_profile_key);
    scheduler_oid = p_fea_qos_port_shape_profile->sai_scheduler_id;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SCHEDULER,(void**)&scheduler_api));
    sal_memset(&attr, 0x0, sizeof(sai_attribute_t));
    switch(field_id)
    {
        case TBL_QOS_PORT_SHAPE_PROFILE_FLD_PIR:
            attr.id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE;
            attr.value.u64 = p_qos_port_shape_profile->pir;
            attr.value.u64 = attr.value.u64 *1000/8;
            break;
        default:
            return HSRV_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(scheduler_api->set_scheduler_attribute(scheduler_oid,&attr));

    return HSRV_E_NONE;
}

int32
hsrv_qos_global(tbl_qos_global_t* p_glb, uint32 field_id)
{
    ctc_qos_glb_cfg_t glb_cfg;
    sai_switch_api_t * switch_api = NULL;
    sai_attribute_t attr[1];
    uint32_t  threshold[2]; 
    uint32_t range[8];
    
    sal_memset(&glb_cfg, 0x0, sizeof(ctc_qos_glb_cfg_t));
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_glb);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_SWITCH,(void**)&switch_api));
    
    switch(field_id)
    {
  #ifndef USW
        case TBL_QOS_GLOBAL_FLD_PORT_POLICER_FIRST_ENABLE:
            glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_FLOW_FIRST_EN;
            /* high 16bit is dir, low 16bit is enable value*/
            glb_cfg.u.value = CTC_BOTH_DIRECTION << 16 | (!p_glb->port_policer_first_enable);
            HSRV_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));

            /* added by hansf for bug 51135 */
            glb_cfg.cfg_type = CTC_QOS_GLB_CFG_POLICER_SEQENCE_EN;
            glb_cfg.u.value = 1;
            HSRV_IF_ERROR_RETURN(ctc_qos_set_global_config(&glb_cfg));
            break;
  #endif
        case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_ENABLE:
            attr[0].id = SAI_SWITCH_ATTR_MONITOR_BUFFER_EN;
            attr[0].value.u32 = p_glb->buffer_event_enable;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
        case TBL_QOS_GLOBAL_FLD_BUFFER_EVENT_HIGH:
            attr[0].id = SAI_SWITCH_ATTR_MONITOR_BUFFER_THRESHOLD;
            attr[0].value.u32list.list = threshold;
            attr[0].value.u32list.list[0] = p_glb->buffer_event_low;
            attr[0].value.u32list.list[1] = p_glb->buffer_event_high;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;     
        case TBL_QOS_GLOBAL_FLD_LATENCY_RANGE:
            attr[0].id = SAI_SWITCH_ATTR_MONITOR_LATENCY_RANGE;
            attr[0].value.u32list.list = range;
            attr[0].value.u32list.list[0] = p_glb->latency_range[0];
            attr[0].value.u32list.list[1] = p_glb->latency_range[1];
            attr[0].value.u32list.list[2] = p_glb->latency_range[2];
            attr[0].value.u32list.list[3] = p_glb->latency_range[3];
            attr[0].value.u32list.list[4] = p_glb->latency_range[4];
            attr[0].value.u32list.list[5] = p_glb->latency_range[5];
            attr[0].value.u32list.list[6] = p_glb->latency_range[6];
            attr[0].value.u32list.list[7] = p_glb->latency_range[7];
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
        case TBL_QOS_GLOBAL_FLD_QOS_IPG_ENABLE:
            attr[0].id = SAI_SWITCH_ATTR_QOS_IPG_ENABLE;
            attr[0].value.u8 = p_glb->qos_ipg_enable;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
        /*added by yejl for gg1.0 queue stats, bug 54212, 2019-10-31*/  
        case TBL_QOS_GLOBAL_FLD_QUEUE_SHAPE_ENABLE:
            attr[0].id = SAI_SWITCH_ATTR_QOS_QUEUE_SHAPE_ENABLE;
            attr[0].value.u8 = p_glb->queue_shape_enable;
            HSRV_IF_ERROR_RETURN(switch_api->set_switch_attribute(attr));
            break;
    }

    return HSRV_E_NONE;
}
/***************************************************************************************************
 * Name         : hsrv_qos_domain_sync
 * Purpose      : load qos domain cdb to sai          
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
hsrv_qos_domain_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_domain_t *p_qos_domain = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_domain = (tbl_qos_domain_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_qos_domain_creat_domain(p_qos_domain->key.id));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_qos_domain_delet_domain(p_qos_domain->key.id));
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_domain_set_field(p_qos_domain, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_qos_policer_sync
 * Purpose      : load qos policer cdb to sai          
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
hsrv_qos_policer_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_policer_profile_t* p_qos_policer_profile = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_policer_profile = (tbl_qos_policer_profile_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_policer_set_policer_profile(p_qos_policer_profile, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_policer_action_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_policer_action_profile_t* p_qos_action_profile = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_action_profile = (tbl_qos_policer_action_profile_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_policer_set_policer_action_profile(p_qos_action_profile, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_drop_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_drop_profile_t* p_qos_drop_profile = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_drop_profile = (tbl_qos_drop_profile_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_qos_drop_creat_drop_profile(p_qos_drop_profile));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_qos_drop_delet_drop_profile(p_qos_drop_profile->key.id));
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_drop_set_drop_profile(p_qos_drop_profile, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_scheduler_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_queue_shape_profile_t* p_qos_scheduler_profile = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_scheduler_profile = (tbl_qos_queue_shape_profile_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_qos_scheduler_creat_scheduler_profile(p_qos_scheduler_profile));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_qos_scheduler_delet_scheduler_profile(p_qos_scheduler_profile->key.id));
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_scheduler_set_scheduler_profile(p_qos_scheduler_profile, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_port_shape_profile_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_port_shape_profile_t* p_qos_port_shape_profile = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_port_shape_profile = (tbl_qos_port_shape_profile_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        HSRV_IF_ERROR_RETURN(hsrv_qos_port_shape_creat_port_shape_profile(p_qos_port_shape_profile));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_qos_port_shape_delet_port_shape_profile(p_qos_port_shape_profile->key.id));
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_port_shape_set_port_shape_profile(p_qos_port_shape_profile, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_global_t* p_glb = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_glb = (tbl_qos_global_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_DEL:
        break;

    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_global(p_glb, field_id));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_qos_mpls_domain_set_mpls_domain(tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    sai_qos_map_api_t*  qos_map_api = NULL;
    sai_attribute_t attr;
    uint32 mpls_label = 0;

    if (NULL == p_qos_mpls_domain)
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));

    mpls_label = p_qos_mpls_domain->key.mpls_label;
    attr.id = SAI_QOS_MAP_ATTR_MPLS_LABEL_TO_EXP_DOMAIN;
    attr.value.s32 = p_qos_mpls_domain->exp_domain;
    HSRV_IF_ERROR_RETURN(qos_map_api->set_mpls_label_to_exp_domain(mpls_label, &attr));
    return HSRV_E_NONE;
}

int32
hsrv_qos_mpls_domain_delete_mpls_domain(tbl_qos_mpls_domain_t *p_qos_mpls_domain)
{
    sai_qos_map_api_t*  qos_map_api = NULL;
    sai_attribute_t attr;
    uint32 mpls_label = 0;

    if (NULL == p_qos_mpls_domain)
    {
        return HSRV_E_NONE;
    }
    
    sal_memset(&attr, 0, sizeof(attr));
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));

    mpls_label = p_qos_mpls_domain->key.mpls_label;
    attr.id = SAI_QOS_MAP_ATTR_MPLS_LABEL_TO_EXP_DOMAIN;
    attr.value.s32 = 0;
    HSRV_IF_ERROR_RETURN(qos_map_api->set_mpls_label_to_exp_domain(mpls_label, &attr));
    return HSRV_E_NONE;
}

int32
hsrv_qos_mpls_domain_synce(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_mpls_domain_t *p_qos_mpls_domain = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_mpls_domain = (tbl_qos_mpls_domain_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_mpls_domain_set_mpls_domain(p_qos_mpls_domain));
        break;

    case CDB_OPER_DEL:
        HSRV_IF_ERROR_RETURN(hsrv_qos_mpls_domain_delete_mpls_domain(p_qos_mpls_domain));
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

/*added by yejl for bug 54031, 2019-11-14*/
int32
hsrv_qos_queue_smart_buffer_set_smart_buffer(tbl_qos_queue_smart_buffer_t *p_qos_sm_buffer)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t* p_db_if = NULL;
    sai_qos_map_api_t*  qos_map_api = NULL;
    sai_attribute_t attr;
    
    uint32 queue_id = 0;
    uint32 port_id = 0;
    uint32 sai_queue_id = 0;
    sai_object_id_t queue_oid = 0;

    if (NULL == p_qos_sm_buffer)
    {
        return HSRV_E_NONE;
    }
    queue_id = p_qos_sm_buffer->key.queue_id;
    
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF == p_db_if->hw_type)
        {   
            sal_memset(&attr, 0, sizeof(attr));
            HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_QOS_MAPS,(void**)&qos_map_api));
            port_id = (uint32)CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
            sai_queue_id = (port_id << 16) + queue_id;
            queue_oid  = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_QUEUE, sai_queue_id);
            attr.id = SAI_QOS_MAP_ATTR_QUEUE_SMART_BUFFER;
            attr.value.s32 = p_qos_sm_buffer->smart_buffer_enable;
            HSRV_IF_ERROR_RETURN(qos_map_api->set_queue_smart_buffer(queue_oid, &attr));
        }
    }
    return HSRV_E_NONE;
}

int32
hsrv_qos_queue_smart_buffer(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_qos_queue_smart_buffer_t *p_qos_sm_buffer = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    p_qos_sm_buffer = (tbl_qos_queue_smart_buffer_t*)p_tbl;
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_SET:
        HSRV_IF_ERROR_RETURN(hsrv_qos_queue_smart_buffer_set_smart_buffer(p_qos_sm_buffer));
        break;
    default:
        break;
    }

    return HSRV_E_NONE;
}
/*ended by yejl*/

