#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <saicopp.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_copp.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_scheduler.h>
#include <ctc_sai_acl.h>

#ifdef GREATBELT
#include <ctc_sai_copp_gb.h>
#endif
#ifdef GOLDENGATE
#include <ctc_sai_copp_gg.h>
#endif
#ifdef USW
#include <ctc_sai_copp_d2.h>
#endif

ctc_sai_copp_info_t g_sai_copp_master;

extern int32 ftm_info_profile_type_gb;
extern int32 ftm_info_profile_type_d2;
extern int32 ftm_info_profile_type_tm;
extern int32 hybrid_profile_type;


uint32
ctc_sai_copp_get_fwd_port()
{
    return g_sai_copp_master.ctc_copp_port_info[SAI_COPP_FWD_PORT_ID].port_id;
}

uint32
ctc_sai_copp_get_reason_port()
{
    return g_sai_copp_master.ctc_copp_port_info[SAI_COPP_REASON_PORT_ID].port_id;
}

/*Note, return acl oids for this reason, user must alloc oid[4]*/
int32 
ctc_sai_copp_get_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid)
{
    if (reason_id >= CTC_SAI_COPP_EXT_MAX)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_oid[0] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0];
    acl_oid[1] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1];
    acl_oid[2] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[2];
    acl_oid[3] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[3];
    acl_oid[4] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[4];
    acl_oid[5] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[5];
    acl_oid[6] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[6];
    acl_oid[7] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[7];

    return SAI_STATUS_SUCCESS;
}

int32 
ctc_sai_copp_get_ssh_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid)
{
    if (reason_id >= CTC_SAI_COPP_EXT_MAX)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_oid[0] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[0];
    acl_oid[1] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[1];
    acl_oid[2] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[2];
    acl_oid[3] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[3];
    acl_oid[4] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[4];
    acl_oid[5] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[5];
    acl_oid[6] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[6];
    acl_oid[7] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_ssh_oid[7];

    return SAI_STATUS_SUCCESS;
}

int32 
ctc_sai_copp_get_telnet_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid)
{
    if (reason_id >= CTC_SAI_COPP_EXT_MAX)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_oid[0] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[0];
    acl_oid[1] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[1];
    acl_oid[2] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[2];
    acl_oid[3] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[3];
    acl_oid[4] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[4];
    acl_oid[5] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[5];
    acl_oid[6] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[6];
    acl_oid[7] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_tln_oid[7];

    return SAI_STATUS_SUCCESS;
}

int32 
ctc_sai_copp_get_mlag_acl_oid_from_reason(uint32 reason_id, sai_object_id_t* acl_oid)
{
    if (reason_id >= CTC_SAI_COPP_EXT_MAX)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    acl_oid[0] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[0];
    acl_oid[1] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[1];
    acl_oid[2] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[2];
    acl_oid[3] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[3];
    acl_oid[4] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[4];
    acl_oid[5] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[5];
    acl_oid[6] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[6];
    acl_oid[7] = g_sai_copp_master.ctc_reason[reason_id].acl_entry_mlag_oid[7];

    return SAI_STATUS_SUCCESS;
}

int32 
ctc_sai_copp_get_counter_oid_from_reason(uint32 reason_id, sai_object_id_t* counter_oid)
{
    if (reason_id >= CTC_SAI_COPP_EXT_MAX)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    *counter_oid  = g_sai_copp_master.ctc_reason[reason_id].counter_oid;
    return SAI_STATUS_SUCCESS;
}

uint32
ctc_sai_acl_get_iloop_port_from_copp_reason_group(uint32 copp_group_id)
{
    switch(copp_group_id)
    {
    case SAI_ACL_COPP_REASON_GROUP:
        return SAI_COPP_REASON_PORT_ID;
        
    case SAI_ACL_COPP_FWD2CPU_GROUP:
        return SAI_COPP_FWD_PORT_ID;
        
    case SAI_ACL_COPP_OPENFLOW_TO_CONTROLLER_GROUP:
        return SAI_COPP_HYBIRD_PORT_ID;
        
    case SAI_ACL_COPP_TTL_EXPIRE_GROUP:
        return SAI_COPP_TTL_EXPIRE_PORT_ID;
        
    case SAI_ACL_COPP_L2PRO_PROTOCOL_MAC_GROUP:
        return SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID;
        
    case SAI_ACL_COPP_L2PRO_GROUP_MAC_GROUP:
        return SAI_COPP_L2PRO_GROUP_MAC_PORT_ID;
#ifndef GREATBELT
    case SAI_ACL_COPP_IGMP_GROUP:
        return SAI_COPP_IGMP_PORT_ID;   
#endif
    case SAI_ACL_COPP_MTU_CHECK_FAIL_GROUP:
        return SAI_COPP_MTU_CHECK_FAIL_PORT_ID;
        
    default:
        return SAI_COPP_PORT_ID_MAX;
    }
}

/*Add ACE entry for ARP packet */
/*Key is Ether type = 0x0806*/
int32
ctc_sai_copp_add_acl_entry_for_reason_arp(sai_object_id_t nh_oid,  
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 ether_type_mask = 0x0;
    uint16 ether_type = 0x0806;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;


    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;


    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for OSPF packet */
/*Key is IPv4 or IPv6, layer3 protocol = 89*/
int32
ctc_sai_copp_add_acl_entry_for_reason_ospf(sai_object_id_t nh_oid, 
                            sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint8 l3_protocol_mask = 0x0;
    uint8 l3_protocol = 89;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = l3_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = l3_protocol_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for PIM packet */
/*Key is IPv4 or IPv6, layer3 protocol = 103*/
int32
ctc_sai_copp_add_acl_entry_for_reason_pim(sai_object_id_t nh_oid,
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint8 l3_protocol_mask = 0x0;
    uint8 l3_protocol = 103;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = l3_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = l3_protocol_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for VRRP packet */
/*Key is IPv4 or IPv6, layer3 protocol = 112*/
int32
ctc_sai_copp_add_acl_entry_for_reason_vrrp(sai_object_id_t nh_oid, 
                    sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint8 l3_protocol_mask = 0x0;
    uint8 l3_protocol = 112;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = l3_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = l3_protocol_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for BGP packet */
/*Key is TCP , DestPort = 179*/
int32
ctc_sai_copp_add_acl_entry_for_reason_bgp(sai_object_id_t nh_oid, 
                        sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 dest_port = 179;
    uint8 ip_protocol = 6;
    uint8 ip_protocol_mask = 0x0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dest_port;
    attr_list[attr_count].value.aclfield.mask.u16 = dest_port;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;
    
    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;
    
    return SAI_STATUS_SUCCESS;
}


/*Add ACE entry for DHCP packet */
/*Key is IPv4 UDP l4DestPort = 67 or 68, IPv6 UDP l4DestPort = 546 or 547*/
int32
ctc_sai_copp_add_acl_entry_for_reason_dhcp(sai_object_id_t nh_oid,
                         sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 dest_port = 67;
    uint16 dest_port1 = 68;
    uint8 ip_protocol = 17;
    uint8 ip_protocol_mask = 0x0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dest_port;
    attr_list[attr_count].value.aclfield.mask.u16 = dest_port;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dest_port1;
    attr_list[attr_count].value.aclfield.mask.u16 = dest_port1;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];
    *(acl_entry_oid+1) = acl_entry_id[1];

    #if defined GREATBELT || defined (DUET2) || defined (TSINGMA)
    if ((ftm_info_profile_type_gb == CTC_FTM_PROFILE_5) || (ftm_info_profile_type_d2 == GLB_STM_MODE_IPUCV6) || (ftm_info_profile_type_tm == GLB_STM_MODE_IPUCV6))
    {
        uint16 dest_port_v6 = 546;
        uint16 dest_port_v6_1 = 547;
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
        attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = dest_port_v6;
        attr_list[attr_count].value.aclfield.mask.u16 = dest_port_v6;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[2], attr_count, attr_list));

        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
        attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = dest_port_v6_1;
        attr_list[attr_count].value.aclfield.mask.u16 = dest_port_v6_1;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[3], attr_count, attr_list));
        *(acl_entry_oid+2) = acl_entry_id[2];
        *(acl_entry_oid+3) = acl_entry_id[3];
    }
    #endif
    
    *counter_oid = acl_counter_id;
    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for IGMP packet , ip protocol = 2*/
/*Key is ip da为D类：1110+28位任意，即224.0.0.0-239.255.255.255作为目的地址的ip报文*/
/*
SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY
SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE
SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT
SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT
SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT
*/
/*Noet, this function return 4 acl_entry_oid, user need alloc acl_entry_oid[4]*/
int32
ctc_sai_copp_add_acl_entry_for_reason_igmp(sai_object_id_t nh_oid,
                         sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint8 ip_protocol = 2;
    uint8 ip_protocol_mask = 0x0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[2] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
   #ifndef GREATBELT
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_IGMP_PORT_ID);
   #else
   attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
   #endif
   
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}


/*Add ACE entry for BPDU packet */
/*Key is Macda = 01:80:c2:00:00:00*/
/*Key is MacdaMask = ff:ff:ff:ff:ff:c0*/
int32
ctc_sai_copp_add_acl_entry_for_reason_bpdu(sai_object_id_t nh_oid,
                        sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    mac_addr_t dmac_mask = {0x0, 0x0, 0x0, 0x0, 0x0, 0x3F};
    mac_addr_t dmac = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;


    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask, sizeof(mac_addr_t));
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}


/* refer to D.3 IPv4 multicast addresses */
/*"224.0.1.129"*/
#define GLB_PTP_IPV4_ADDRESS_PRIMARY        0xE0000181
/*"224.0.0.107"*/
#define GLB_PTP_IPV4_ADDRESS_PDELAY         0xE000006B

#define GLB_PTP_IP_MAC_ADDRESS_PRIMARY      {0x01, 0x00, 0x5e, 0x00, 0x01, 0x81}
#define GLB_PTP_IP_MAC_ADDRESS_PDELAY       {0x01, 0x00, 0x5e, 0x00, 0x00, 0x6b}

/*Add ACE entry for ptp packet */

/*l3*/
/*Key is Ether type = 0x0800*/
/*Key is IPv4 UDP l4DestPort = 319 or 320*/
/*Key is ip da为GLB_PTP_IPV4_ADDRESS_PRIMARY 224.0.1.129 0xE0000181, mask 0xffffffff*/
/*Key is ip da为GLB_PTP_IPV4_ADDRESS_PDELAY 224.0.0.107 0xE000006B, mask 0xffffffff*/

/*l2*/
/*Key is Macda = GLB_PTP_MAC_ADDRESS_DEFAULT {0x01, 0x1B, 0x19, 0x00, 0x00, 0x00}*/
/*Key is MacdaMask = GLB_PTP_MAC_ADDRESS_MASK {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}*/
/*Key is Ether type = GLB_PTP_1588_ETYPE 0x88F7 0xffff*/

/*Key is Macda = GLB_PTP_MAC_ADDRESS_PDELAY {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E}*/
/*Key is MacdaMask = GLB_PTP_MAC_ADDRESS_MASK {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}*/
/*Key is Ether type = GLB_PTP_1588_ETYPE 0x88F7 0xffff*/

int32
ctc_sai_copp_add_acl_entry_for_reason_ptp(sai_object_id_t nh_oid,
                        sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;

    /*l2*/
    uint16 ether_type_mask = 0x0;
    uint16 ether_type = 0x88F7;
    mac_addr_t dmac_mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    mac_addr_t dmac_default = {0x01, 0x1B, 0x19, 0x00, 0x00, 0x00};
    mac_addr_t dmac_pdelay = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};

    /*l3*/
    uint16 dest_port_event_msg = 319;
    uint16 dest_port_general_msg = 320;
    uint8 ip_protocol = IPPROTO_UDP;
    uint8 ip_protocol_mask = 0x0;
    
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    /*l2 not pdelay*/
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac_default, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask, sizeof(mac_addr_t));
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));


    /*l2 pdelay*/
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac_pdelay, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask, sizeof(mac_addr_t));
    attr_count++;

    /* create counter id */
    //ounter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    //counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    //acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));


    /*l3 event msg*/
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dest_port_event_msg;
    attr_list[attr_count].value.aclfield.mask.u16 = dest_port_event_msg;
    attr_count++;

    /* create counter id */
    //counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    //counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    //acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[2], attr_count, attr_list));


    /*l3 general msg*/
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dest_port_general_msg;
    attr_list[attr_count].value.aclfield.mask.u16 = dest_port_general_msg;
    attr_count++;

    /* create counter id */
    //counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    //counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    //acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[3], attr_count, attr_list));

    
    *counter_oid = acl_counter_id;
    *acl_entry_oid = acl_entry_id[0];
    *(acl_entry_oid+1) = acl_entry_id[1];
    *(acl_entry_oid+2) = acl_entry_id[2];
    *(acl_entry_oid+3) = acl_entry_id[3];

    return SAI_STATUS_SUCCESS;
}

/*added by yejl for bug 51016, 2019-03-20*/
#ifndef OFPRODUCT
/*Add ACE entry for EAPOL packet */
/*Key is Ether type = 0x888E*/
int32
ctc_sai_copp_add_acl_entry_for_reason_eapol(sai_object_id_t nh_oid,
                       sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 ether_type_mask = 0x0;
    uint16 ether_type = 0x888E;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;


    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;
    
    return SAI_STATUS_SUCCESS;
}
#endif

/*Add ACE entry for LLDP packet */
/*Key is Ether type = 0x88CC*/
int32
ctc_sai_copp_add_acl_entry_for_reason_lldp(sai_object_id_t nh_oid,
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 ether_type_mask = 0x0;
    uint16 ether_type = 0x88CC;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;


    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;
    
    return SAI_STATUS_SUCCESS;
}

/*Add ACE entry for ERPS packet */
/*
mac, 0x01, 0x80, 0x63, 0x07, 0x00, 0x00
mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
mac, 0x00, 0x01, 0x7A, 0x4F, 0x48, 0x26
mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
h3c:
mac, 0x00, 0x0f, 0xe2, 0x07, 0x82, 0x00
mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
mac, 0x01, 0x80, 0x63, 0x07, 0x00, 0x00
mac_mask, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
*/
/*Noet, this function return 4 acl_entry_oid, user need alloc acl_entry_oid[4]*/
int32
ctc_sai_copp_add_acl_entry_for_reason_erps(sai_object_id_t nh_oid,
                         sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    mac_addr_t dmac_mask1 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    mac_addr_t dmac1 = {0x01, 0x80, 0x63, 0x07, 0x00, 0x00};

    mac_addr_t dmac_mask2 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    mac_addr_t dmac2 = {0x00, 0x01, 0x7A, 0x4F, 0x48, 0x26};

    mac_addr_t dmac_mask3 = {0x0, 0x0, 0x0, 0x0, 0x0, 0xFF};
    mac_addr_t dmac3 = {0x00, 0x0F,0xE2, 0x07, 0x82, 0x00};

    mac_addr_t dmac_mask4 = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    mac_addr_t dmac4 = {0x01, 0x80,0x63, 0x07, 0x00, 0x00};

    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac1, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask1, sizeof(mac_addr_t));
    attr_count++;
    
    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac2, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask2, sizeof(mac_addr_t));
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac3, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask3, sizeof(mac_addr_t));
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[2], attr_count, attr_list));

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    sal_memcpy(attr_list[attr_count].value.aclfield.data.mac, &dmac4, sizeof(mac_addr_t));
    sal_memcpy(attr_list[attr_count].value.aclfield.mask.mac, &dmac_mask4, sizeof(mac_addr_t));
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[3], attr_count, attr_list));
    
    *counter_oid = acl_counter_id;
    *acl_entry_oid = acl_entry_id[0];
    *(acl_entry_oid+1) = acl_entry_id[1];
    *(acl_entry_oid+2) = acl_entry_id[2];
    *(acl_entry_oid+3) = acl_entry_id[3];
    
    return SAI_STATUS_SUCCESS;
}


/*Add ACE entry for SFLOW packet */
/*Key is Ether type = 0x8809*/
int32
ctc_sai_copp_add_acl_entry_for_reason_slow_protocol(sai_object_id_t nh_oid,
                                     sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint16 ether_type_mask = 0x0;
    uint16 ether_type = 0x8809;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;


    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = ether_type;
    attr_list[attr_count].value.aclfield.mask.u16 = ether_type_mask;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid;
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;
    
    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_fwd_to_cpu(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));

    *acl_entry_oid = acl_entry_id[0];

    #if defined GREATBELT || defined (DUET2) || defined (TSINGMA)
    if ((ftm_info_profile_type_gb == CTC_FTM_PROFILE_5) || (ftm_info_profile_type_d2 == GLB_STM_MODE_IPUCV6) || (ftm_info_profile_type_tm == GLB_STM_MODE_IPUCV6))
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_count = 0;
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
        *(acl_entry_oid+1) = acl_entry_id[1];
    }
    #endif
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

/*default ssh tcp dstport */
/*flex entry protocol ,dst_port */
int32
ctc_sai_copp_add_flex_entry_for_fwd_to_cpu(sai_object_id_t nh_oid,  uint8 ip_protocol,
                      uint16 dst_port, sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;  /*high priority than default fwd entry*/
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = 0x0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dst_port;
    attr_list[attr_count].value.aclfield.mask.u16 = dst_port;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));
    

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];
    
    #if defined GREATBELT || defined (DUET2) || defined (TSINGMA)
    if ((ftm_info_profile_type_gb == CTC_FTM_PROFILE_5) || (ftm_info_profile_type_d2 == GLB_STM_MODE_IPUCV6) || (ftm_info_profile_type_tm == GLB_STM_MODE_IPUCV6))
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 1;  /*high priority than default fwd entry*/
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
        attr_list[attr_count].value.aclfield.mask.u8 = 0x0;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.u16 = dst_port;
        attr_list[attr_count].value.aclfield.mask.u16 = dst_port;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
        *(acl_entry_oid+1) = acl_entry_id[1];
    }
    #endif
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_flex_entry_for_fwd_to_cpu_mlag(sai_object_id_t nh_oid,  uint8 ip_protocol,
                      uint16 dst_port, sai_object_id_t* acl_entry_oid, sai_object_id_t* acl_entry1_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;
    uint32 l4port_attr_count = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;  /*high priority than default fwd entry*/
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = 0x0;
    attr_count++;

    l4port_attr_count = attr_count;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = dst_port;
    attr_list[attr_count].value.aclfield.mask.u16 = dst_port;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));
    

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;

    attr_list[l4port_attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry1_oid = acl_entry_id;
    
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_tln_fwd_to_cpu(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;
    uint8 ip_protocol = 6;
    uint8 ip_protocol_mask = 0x0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;  /*high priority than default fwd entry*/
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_FWD_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = ip_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = ip_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u16 = 23;
    attr_list[attr_count].value.aclfield.mask.u16 = 23;
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_ttl_ospf_to_cpu(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t in_counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_entry_id = 0;
    uint8 l3_protocol_mask = 0x0;
    uint8 l3_protocol = 89;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_TTL_EXPIRE_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = l3_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = l3_protocol_mask;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = in_counter_oid;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;

    return SAI_STATUS_SUCCESS;
}


int32
ctc_sai_copp_add_acl_entry_for_ttl_expired_to_cpu(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
     sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_TTL_EXPIRE_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];

    /*added by yejl for 49478, 2018-10-31*/
    #ifdef GREATBELT 
    if (ftm_info_profile_type_gb == CTC_FTM_PROFILE_5)
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_count = 0;
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_TTL_EXPIRE_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
        *(acl_entry_oid+1) = acl_entry_id[1];
    }
    #endif
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_mtu_check_fail_to_cpu(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
     sai_object_id_t acl_entry_id[4] = {0};

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_MTU_CHECK_FAIL_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];

    #if defined GREATBELT || defined (DUET2) || defined (TSINGMA)
    if ((ftm_info_profile_type_gb == CTC_FTM_PROFILE_5) || (ftm_info_profile_type_d2 == GLB_STM_MODE_IPUCV6) || (ftm_info_profile_type_tm == GLB_STM_MODE_IPUCV6))
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_count = 0;
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_MTU_CHECK_FAIL_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
        *(acl_entry_oid+1) = acl_entry_id[1];
    }
    #endif
    *counter_oid = acl_counter_id;


    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_fwd_to_controller(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id[4] = {0};


    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_HYBIRD_PORT_ID);
    attr_count++;

    /* create counter id */
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*global GLB_IGS_ACL_TBLID defined*/
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[0], attr_count, attr_list));
    *acl_entry_oid = acl_entry_id[0];
    *counter_oid = acl_counter_id;

    /* added by hansf for bug 51019 */
    #ifdef GREATBELT 
    if (hybrid_profile_type == GLB_STM_MODE_OF_IPV6)
    {
        sal_memset(attr_list, 0, sizeof(attr_list));
        attr_count = 0;
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
        attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; /*COPP_IGS_ACL_TBLID*/;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
        attr_list[attr_count].value.u32 = 0;
        attr_count++;
        
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_HYBIRD_PORT_ID);
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
        attr_list[attr_count].value.aclfield.enable = TRUE;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
        attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
        attr_list[attr_count].value.aclaction.enable = TRUE;
        attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
        attr_count++;

        CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id[1], attr_count, attr_list));
        *(acl_entry_oid+1) = acl_entry_id[1];
    }
    #endif

    return SAI_STATUS_SUCCESS;
}


int32
ctc_sai_copp_add_acl_entry_for_reason_l2pro_protocol_mac(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //COPP_IGS_ACL_TBLID
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID);
    attr_count++;

    // create counter id 
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //global GLB_IGS_ACL_TBLID defined
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_add_acl_entry_for_reason_l2pro_group_mac(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //COPP_IGS_ACL_TBLID
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_L2PRO_GROUP_MAC_PORT_ID);
    attr_count++;

    // create counter id 
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //global GLB_IGS_ACL_TBLID defined
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}
#if 0
int32
ctc_sai_copp_add_acl_entry_for_reason_cpu_to_mirrror(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //COPP_IGS_ACL_TBLID
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_CPU_MIRROR_PORT_ID);
    attr_count++;

    // create counter id 
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //global GLB_IGS_ACL_TBLID defined
    acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}
#endif

#if defined GREATBELT || defined (DUET2) ||defined (TSINGMA)
int32
ctc_sai_copp_add_acl_entry_for_reason_icmpv6(sai_object_id_t nh_oid,   
                      sai_object_id_t* acl_entry_oid, sai_object_id_t* counter_oid)
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[36];
    uint32 attr_count = 0;
    uint8 l4_protocol_mask = 0xFF;
    uint8 l4_protocol = 58;
    sai_object_id_t acl_counter_id = 0;
    sai_attribute_t counter_attr;
    sai_object_id_t acl_entry_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //COPP_IGS_ACL_TBLID
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 0;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_COPP,SAI_COPP_REASON_PORT_ID);
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IPV6_ACL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.aclfield.enable = TRUE;
    attr_list[attr_count].value.aclfield.data.u8 = l4_protocol;
    attr_list[attr_count].value.aclfield.mask.u8 = l4_protocol_mask;
    attr_count++;

    // create counter id 
    counter_attr.id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    counter_attr.value.oid = g_sai_copp_master.ctc_copp_igs_tbl_oid; //global GLB_IGS_ACL_TBLID defined
    CTC_ERROR_RETURN(acl_api->create_acl_counter(&acl_counter_id, 1, &counter_attr));

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_COUNTER;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = acl_counter_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT;
    attr_list[attr_count].value.aclaction.enable = TRUE;
    attr_list[attr_count].value.aclaction.parameter.oid = nh_oid;
    attr_count++;

    CTC_ERROR_RETURN(acl_api->create_acl_entry(&acl_entry_id, attr_count, attr_list));
    *acl_entry_oid = acl_entry_id;
    *counter_oid = acl_counter_id;

    return SAI_STATUS_SUCCESS;
}

//TCP 61000
    
#endif

uint32
ctc_sai_copp_get_nhid_for_controller(void)
{
    return g_sai_copp_master.ctc_copp_port_info[SAI_COPP_HYBIRD_PORT_ID].nexthop_id;
}

int
ctc_sai_is_iloop_port(int port_id)
{
    int i = 0;

    for (i = 0; i < SAI_COPP_PORT_ID_MAX; i++)
    {
        if (g_sai_copp_master.ctc_copp_port_info[i].port_id)
        {
            if (g_sai_copp_master.ctc_copp_port_info[i].port_id == port_id)
            {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

int
_ctc_sai_copp_set_exception_nexthop(uint16 cpu_reason, uint16 port_type)
{
    ctc_misc_nh_param_t nh_param;
    sai_uint32_t nexthopid = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(que_cfg));
    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    que_cfg.value.reason_dest.cpu_reason = cpu_reason;
    que_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_NHID;
    que_cfg.value.reason_dest.nhid = g_sai_copp_master.ctc_copp_port_info[port_type].nexthop_id;
    ret += ctc_qos_set_queue(&que_cfg);
    
    CTC_ERROR_RETURN(ctc_sai_nexthop_alloc_offset(&nexthopid));
    sal_memset(&que_cfg, 0, sizeof(que_cfg));
    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    que_cfg.value.reason_dest.cpu_reason = (cpu_reason+CTC_PKT_CPU_REASON_CUSTOM_BASE*2);
    que_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    ret += ctc_qos_set_queue(&que_cfg);
    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.type = CTC_MISC_NH_TYPE_TO_CPU;
    nh_param.misc_param.cpu_reason.cpu_reason_id = (cpu_reason+CTC_PKT_CPU_REASON_CUSTOM_BASE*2);
    ret += ctc_nh_add_misc(nexthopid, &nh_param);
    g_sai_copp_master.ctc_reason[cpu_reason].nexthop_oid = nexthopid; 

    return ret; 

}

int
_ctc_sai_copp_set_controller_nexthop(uint16 cpu_reason, uint16 port_type)
{
    ctc_misc_nh_param_t nh_param;
    sai_uint32_t nexthopid = 0;
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_qos_queue_cfg_t que_cfg;

    sal_memset(&que_cfg, 0, sizeof(que_cfg));
    que_cfg.type = CTC_QOS_QUEUE_CFG_QUEUE_REASON_DEST;
    que_cfg.value.reason_dest.cpu_reason = (cpu_reason+CTC_PKT_CPU_REASON_CUSTOM_BASE*2);
    que_cfg.value.reason_dest.dest_type = CTC_PKT_CPU_REASON_TO_LOCAL_CPU;
    ret += ctc_qos_set_queue(&que_cfg);

    CTC_ERROR_RETURN(ctc_sai_nexthop_alloc_offset(&nexthopid));
    sal_memset(&nh_param, 0, sizeof(nh_param));
    nh_param.type = CTC_MISC_NH_TYPE_TO_CPU;
    nh_param.misc_param.cpu_reason.cpu_reason_id = (cpu_reason+CTC_PKT_CPU_REASON_CUSTOM_BASE*2);
    ret += ctc_nh_add_misc(nexthopid, &nh_param);
    g_sai_copp_master.ctc_reason[cpu_reason].nexthop_oid = nexthopid; 

    return ret; 

}



int
ctc_sai_copp_exception_nexthop_init()
{
    sai_status_t ret = SAI_STATUS_SUCCESS;

    /*LLDP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_LLDP, 
                                                SAI_COPP_REASON_PORT_ID);

    /*BPDU Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_BPDU, 
                                                SAI_COPP_REASON_PORT_ID);

    /*SLOW PROTOCOL Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_SLOW_PROTO, 
                                                SAI_COPP_REASON_PORT_ID);
#ifndef OFPRODUCT
    /*EAPOL Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_EAPOL, 
                                                SAI_COPP_REASON_PORT_ID);
#endif

    /*ERPS Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_ERPS,
                                                SAI_COPP_REASON_PORT_ID);

    /*OSPF Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF,
                                                SAI_COPP_REASON_PORT_ID);

    /*BGP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_BGP,
                                                SAI_COPP_REASON_PORT_ID);
    
    /*VRRP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_VRRP,
                                                SAI_COPP_REASON_PORT_ID);

    /*IGMP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
#ifndef GREATBELT
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_IGMP_SNOOPING,
                                                SAI_COPP_IGMP_PORT_ID);
#else
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_IGMP_SNOOPING,
                                                SAI_COPP_REASON_PORT_ID);
#endif
    
    /*ARP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
#ifdef GREATBELT
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP_V2,
                                                SAI_COPP_REASON_PORT_ID);
#else
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP,
                                                SAI_COPP_REASON_PORT_ID);
#endif
    
    /*DHCP Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
#ifdef GREATBELT
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP_V2,
                                                SAI_COPP_REASON_PORT_ID);
#else
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP,
                                                SAI_COPP_REASON_PORT_ID);
#endif

    /*for ptp add by chenc*/
    /*ptp Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_PTP,
                                                SAI_COPP_REASON_PORT_ID);

    /*PIM Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_PIM,
                                                SAI_COPP_REASON_PORT_ID);
    
    /*fwd-to-cpu Reason, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_COPY_CPU,SAI_COPP_FWD_PORT_ID);

    /*to-controller Reason, alloc nexthop for this reason and mapping the reason to CPU*/
    /*CTC_PKT_CPU_REASON_CUSTOM_BASE + GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/
    ret +=_ctc_sai_copp_set_controller_nexthop(CTC_PKT_CPU_REASON_CUSTOM_BASE + 1,SAI_COPP_HYBIRD_PORT_ID);

    /*ttl-expired Reason, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL,SAI_COPP_TTL_EXPIRE_PORT_ID);

    /*mtu check fail Reason, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_MTU_FAIL,SAI_COPP_MTU_CHECK_FAIL_PORT_ID);

    //l2 protocol protocol mac Reason exception, alloc nexthop for this reason and mapping the reason to CPU
    ret +=_ctc_sai_copp_set_controller_nexthop(CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC,SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID);

    //l2 protocol group mac Reason exception, alloc nexthop for this reason and mapping the reason to CPU
    ret +=_ctc_sai_copp_set_controller_nexthop(CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC,SAI_COPP_L2PRO_GROUP_MAC_PORT_ID);
    //cpu mirror Reason exception, alloc nexthop for this reason and mapping the reason to CPU
    //ret +=_ctc_sai_copp_set_exception_nexthop(CTC_SAI_COPP_EXT_CPU_MIRROR,SAI_COPP_CPU_MIRROR_PORT_ID);

#if defined GREATBELT || defined (DUET2) || defined (TSINGMA) 
    /*ICMPv6 Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ICMPV6,  
                                              SAI_COPP_REASON_PORT_ID);
    /*ICMPv6 Reason exception, alloc nexthop for this reason and mapping the reason to CPU*/
    ret +=_ctc_sai_copp_set_exception_nexthop(CTC_PKT_CPU_REASON_LINK_ID_FAIL,  
                                              SAI_COPP_REASON_PORT_ID);
#endif
    
    return ret;
}



sai_status_t 
ctc_sai_copp_set_total_rate(const sai_attribute_t *attr)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ctc_qos_shape_t qos_shape;
    
    sal_memset(&qos_shape, 0x0, sizeof(qos_shape));
    qos_shape.type = CTC_QOS_SHAPE_PORT;
    qos_shape.shape.port_shape.enable = TRUE;
    qos_shape.shape.port_shape.gport = CTC_LPORT_CPU;
    qos_shape.shape.port_shape.pir = attr->value.u64;
    qos_shape.shape.port_shape.pbs = attr->value.u64*1000*2;
    ret = ctc_qos_set_shape(&qos_shape);
  
    return ret;
}



#define ________SAI_COPP_QUEUE_API_FUNC

sai_status_t sai_set_copp_queue_attribute_debug_param(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
    )
{
    CTC_SAI_DEBUG("in:queue_id 0x%llx", queue_id);
    switch(attr->id)
    {
    case SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
        CTC_SAI_DEBUG("in:SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID %u", attr->value.u32);
        //copp 8 queue
        break;
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_set_copp_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_DEBUG_FUNC();
    sai_set_copp_queue_attribute_debug_param(queue_id, attr);

    CTC_SAI_PTR_VALID_CHECK(attr);

    switch(attr->id)
    {
        case SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
            ret = ctc_sai_scheduler_copp_set_scheduler(queue_id, attr);
            break;
        case SAI_COPP_QUEUE_ATTR_TOTAL_RATE:
            ret = ctc_sai_copp_set_total_rate(attr);
            break;
        default :
            break;
    }

    return ret;
}


sai_status_t sai_get_copp_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t        attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t* attr;
    int i;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_PTR_VALID_CHECK(attr_list);
    for (i=0; i<attr_count; i++)
    {
        attr = attr_list + i;
        switch(attr->id)
        {
            case SAI_COPP_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                ret = ctc_sai_scheduler_copp_get_scheduler(queue_id, attr);
                break; 

            case SAI_COPP_QUEUE_ATTR_TOTAL_RATE:
                break;

            default :
                break;
        }
    }

    return ret;
}

stbl_copp_entry_t *
ctc_sai_copp_db_get_entry_exist(stbl_copp_entry_t *sai_copp_entry)
{
    stbl_copp_entry_master_t *p_copp_entry_master = stbl_copp_entry_get_master();
    stbl_copp_entry_t *p_db_entry = NULL;
    uint32 entry_id = 0;

    if (NULL == p_copp_entry_master)
    {
        return NULL;
    }

    for (entry_id = 0; entry_id < GLB_COPP_ENTRY_MAX; entry_id++)
    {
        p_db_entry = p_copp_entry_master->array[entry_id];
        if (NULL == p_db_entry)
        {
            continue;
        }
        if (p_db_entry->dst_port == sai_copp_entry->dst_port &&
            p_db_entry->type == sai_copp_entry->type &&
            p_db_entry->ip_type == sai_copp_entry->ip_type &&
            p_db_entry->protocol_num == sai_copp_entry->protocol_num)
        {
            return p_db_entry;
        }
    }

    return NULL;
}

stbl_copp_entry_t *
ctc_sai_copp_db_get_entry_by_entry_oid(sai_object_id_t sai_copp_entry_oid)
{
    stbl_copp_entry_master_t *p_copp_entry_master = stbl_copp_entry_get_master();
    stbl_copp_entry_t *p_db_entry = NULL;
    uint32 entry_id = 0;

    if (NULL == p_copp_entry_master)
    {
        return NULL;
    }

    for (entry_id = 0; entry_id < GLB_COPP_ENTRY_MAX; entry_id++)
    {
        p_db_entry = p_copp_entry_master->array[entry_id];
        if (NULL == p_db_entry)
        {
            continue;
        }
        if (p_db_entry->entry_oid == sai_copp_entry_oid)
        {
            return p_db_entry;
        }
    }

    return NULL;
}

/*added by yejl for bug 49756, 2018-12-12*/
sai_status_t
ctc_sai_copp_del_acl_counter(sai_object_id_t acl_counter_oid)
{
    int32 sdk_ret = 0;
    
    CTC_SAI_DEBUG_FUNC();
    sdk_ret = ctc_stats_destroy_statsid(CTC_SAI_OBJECT_INDEX_GET(acl_counter_oid));
    return sdk_ret;
}

/*
* Routine Description:
*   Create an ACL entry
*
* Arguments:
*   [out] copp_entry_id - the copp entry id
*   [in] attr_count - number of attributes
*   [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_create_copp_entry(
    _Out_ sai_object_id_t* copp_entry_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_object_id_t nh_oid;
    sai_object_id_t acl_entry_oids[8] = {0};
    sai_object_id_t acl_entry_oid = 0;
    sai_object_id_t acl_entry1_oid = 0;
    sai_object_id_t counter_oid = 0;
    uint32_t reason_id;
    uint32_t index = 0;
    stbl_copp_entry_t sai_entry;
    stbl_copp_entry_t *p_sdb_entry = NULL;
    sai_attribute_t const *pattr_entry = NULL;
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();

    CTC_SAI_PTR_VALID_CHECK(copp_entry_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    CTC_SAI_DEBUG_FUNC();

    sal_memset(&sai_entry, 0, sizeof(sai_entry));

    /* 1. get the type: ssh or telnet or mlag*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_COPP_ENTRY_ATTR_FIELD_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.type = pattr_entry->value.u8;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    /* 2. get the the ip protocol*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_COPP_ENTRY_ATTR_FIELD_IP_PROTOCOL, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.protocol_num = pattr_entry->value.u8;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    /* 3. get the the l4 dest port*/
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_COPP_ENTRY_ATTR_FIELD_L4_DST_PORT, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sai_entry.dst_port = pattr_entry->value.u16;
    }
    else
    {
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    /* 4. get the the stbl_copp_entry*/
    p_sdb_entry = ctc_sai_copp_db_get_entry_exist(&sai_entry);
    if (p_sdb_entry)
    {
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    /* 5. creat ssh, telnet, mlag copp acl entry*/
    if (CTC_SAI_MLAG == sai_entry.type)
    {
        reason_id = CTC_PKT_CPU_REASON_L3_COPY_CPU;
        nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
        ret = ctc_sai_copp_add_flex_entry_for_fwd_to_cpu_mlag(nh_oid, sai_entry.protocol_num, sai_entry.dst_port,
                                                         &acl_entry_oid, &acl_entry1_oid, &counter_oid);
        if (ret)
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }
        g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_mlag_oid[0] = acl_entry_oid;
        g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_mlag_oid[1] = acl_entry1_oid;
        g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].counter_mlag_oid = counter_oid;

        ret = ctclib_opb_alloc_offset(&p_sai_glb->copp_entry_opb, &index);
        if (ret)
        {
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
        sai_entry.key.copp_entry_id = index;
        sai_entry.counter_oid = counter_oid;
        sai_entry.entry_oid = acl_entry_oid;
        sai_entry.nh_oid = nh_oid;
        stbl_copp_entry_add_copp_entry(&sai_entry);

        ret = ctclib_opb_alloc_offset(&p_sai_glb->copp_entry_opb, &index);
        if (ret)
        {
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
        sai_entry.key.copp_entry_id = index;
        sai_entry.counter_oid = counter_oid;
        sai_entry.entry_oid = acl_entry1_oid;
        sai_entry.nh_oid = nh_oid;
        stbl_copp_entry_add_copp_entry(&sai_entry);

        /* only return 0 */
        *copp_entry_id = acl_entry_oid;
        *(copp_entry_id+1)= acl_entry1_oid;
    }
    else 
    {
        reason_id = CTC_PKT_CPU_REASON_L3_COPY_CPU;
        nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
        ret = ctc_sai_copp_add_flex_entry_for_fwd_to_cpu(nh_oid, sai_entry.protocol_num, sai_entry.dst_port, &acl_entry_oids[0], &counter_oid);
        if (ret)
        {
            return SAI_STATUS_INVALID_PARAMETER;
        }
        if (CTC_SAI_SSH == sai_entry.type)
        {   /*modified by yejl for bug 49756, 2018-12-12*/
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_ssh_oid[0] = acl_entry_oids[0];
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_ssh_oid[1] = acl_entry_oids[1];
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].counter_ssh_oid = counter_oid;
        }
        else if (CTC_SAI_TELNET == sai_entry.type)
        {
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_tln_oid[0] = acl_entry_oids[0];
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_tln_oid[1] = acl_entry_oids[1];
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].counter_tln_oid = counter_oid;
        }

        ret = ctclib_opb_alloc_offset(&p_sai_glb->copp_entry_opb, &index);
        if (ret)
        {
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
        sai_entry.key.copp_entry_id = index;
        sai_entry.counter_oid = counter_oid;
        sai_entry.entry_oid = acl_entry_oids[0];
        sai_entry.nh_oid = nh_oid;
        stbl_copp_entry_add_copp_entry(&sai_entry);
        *copp_entry_id = acl_entry_oids[0];
        
        if (acl_entry_oids[1] != 0)/*modified by yejl for bug 49756, 2018-12-12*/
        {
            ret = ctclib_opb_alloc_offset(&p_sai_glb->copp_entry_opb, &index);
            if (ret)
            {
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }
            sai_entry.key.copp_entry_id = index;
            sai_entry.counter_oid = counter_oid;
            sai_entry.entry_oid = acl_entry_oids[1];
            sai_entry.nh_oid = nh_oid;
            stbl_copp_entry_add_copp_entry(&sai_entry);
            *(copp_entry_id+1) = acl_entry_oids[1];
        }
    }
    
    return ret;
}


/*
* Routine Description:
*   Delete an copp entry
*
* Arguments:
 *  [in] copp_entry_id - the copp entry id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_delete_copp_entry(_In_ sai_object_id_t copp_entry_id)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    sai_acl_api_t *acl_api = NULL;
    stbl_copp_entry_t *p_sdb_entry = NULL;
    uint8_t i = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);
    
    p_sdb_entry = ctc_sai_copp_db_get_entry_by_entry_oid(copp_entry_id);
    if (NULL == p_sdb_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    for (i = 0; i < 8; i++)
    {
        if (copp_entry_id == g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_ssh_oid[i])
        {
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_ssh_oid[i] = 0;
        }
        if (copp_entry_id == g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_tln_oid[i])
        {
            g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_COPY_CPU].acl_entry_tln_oid[i] = 0;
        }
    }

    ctc_sai_copp_del_acl_counter(p_sdb_entry->counter_oid);/*no need to return the error*/
    CTC_ERROR_RETURN(acl_api->delete_acl_entry(copp_entry_id));
    stbl_copp_entry_del_copp_entry(&p_sdb_entry->key);
    return ret;
}

sai_status_t
ctc_sai_copp_db_init()
{
    sai_object_id_t nh_oid = 0;
    sai_object_id_t acl_entry_oids[8]= {0};
    sai_object_id_t acl_ttl_ospf_entry_oid = 0;
    sai_object_id_t counter_oid = 0;
    uint32 reason_id = 0;
    stbl_sai_glb_t *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    sal_memset(&g_sai_copp_master, 0, sizeof(g_sai_copp_master));

    /*inisiall port*/
    CTC_ERROR_RETURN(ctc_sai_copp_internal_port_init());

    CTC_ERROR_RETURN(ctc_sai_copp_tbl_init());

    /*inisiall port acl group*/
    CTC_ERROR_RETURN(ctc_sai_acl_copp_group_init());

    /*creat nexthop for each exception*/
    CTC_ERROR_RETURN(ctc_sai_copp_exception_nexthop_init());

    /*for ptp add by chenc*/
    /*creat acl entry for ptp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_PTP;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_ptp(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[2] = acl_entry_oids[2];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[3] = acl_entry_oids[3];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for lldp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_LLDP;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_lldp(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for slow protocol exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_SLOW_PROTO;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_slow_protocol(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for eapol exception for Godengate iloop solution*/
/*added by yejl for bug 51016, 2019-03-20*/
#ifndef OFPRODUCT
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_EAPOL;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_eapol(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;
#endif

    /*creat acl entry for arp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
#ifdef GREATBELT
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP_V2;
#else
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ARP;
#endif
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_arp(nh_oid, &acl_entry_oids[0], &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid; 

    /*creat acl entry for ospf exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_ospf(nh_oid, &acl_entry_oids[0], &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for pim exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_PIM;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_pim(nh_oid, &acl_entry_oids[0], &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for vrrp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_VRRP;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_vrrp(nh_oid, &acl_entry_oids[0], &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;
    
    /*creat acl entry for bgp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_BGP;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_bgp(nh_oid, &acl_entry_oids[0], &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for dhcp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
#ifdef GREATBELT
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP_V2;
#else
    reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_DHCP;
#endif
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_dhcp(nh_oid, acl_entry_oids, &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[2] = acl_entry_oids[2];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[3] = acl_entry_oids[3];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for igmp exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_IGMP_SNOOPING;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_igmp(nh_oid, acl_entry_oids, &counter_oid); 
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for erps exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_ERPS;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_erps(nh_oid, acl_entry_oids, &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[2] = acl_entry_oids[2];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[3] = acl_entry_oids[3];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for bpdu exception for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L2_PDU+CTC_L2PDU_ACTION_INDEX_BPDU;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_bpdu(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for fwd2cpu for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_COPY_CPU;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_fwd_to_cpu(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*to-controller Reason, alloc nexthop for this reason and mapping the reason to CPU*/
    /*CTC_PKT_CPU_REASON_CUSTOM_BASE + GLB_PKT_CUSTOM_TOCPU_OPENFLOW_MIN*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_CUSTOM_BASE + 1;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_fwd_to_controller(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    /*creat acl entry for ttl-fail-check for Godengate iloop solution*/
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_ttl_ospf_to_cpu(g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF].nexthop_oid,
                               &acl_ttl_ospf_entry_oid, g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF].counter_oid);
    g_sai_copp_master.ctc_reason[CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_OSPF].acl_entry_oid[1] = acl_ttl_ospf_entry_oid;
    ctc_sai_copp_add_acl_entry_for_ttl_expired_to_cpu(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;   

    //creat acl entry for L3 MTU check fail for iloop solution
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_PKT_CPU_REASON_L3_MTU_FAIL;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_mtu_check_fail_to_cpu(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[1] = acl_entry_oids[1];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    //creat acl entry for l2protcol protocol-mac for Godengate iloop solution
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_SAI_COPP_EXT_L2PROTO_PROTOCOLMAC;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_l2pro_protocol_mac(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    //creat acl entry for l2protocol group-mac for Godengate iloop solution
    sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
    reason_id = CTC_SAI_COPP_EXT_L2PROTO_GROUPMAC;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_l2pro_group_mac(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

    //creat acl entry for cpu mirror for Godengate iloop solution
    #if 0
    reason_id = CTC_SAI_COPP_EXT_CPU_MIRROR;
    nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
    ctc_sai_copp_add_acl_entry_for_reason_cpu_to_mirrror(nh_oid, &acl_entry_oids[0], &counter_oid);
    g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
    g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
    g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;
    #endif
    #if defined GREATBELT || defined (DUET2) || defined (TSINGMA)/*modified by yejl for bug 49942,2019-03-12*/
    if ((ftm_info_profile_type_gb == CTC_FTM_PROFILE_5) || (ftm_info_profile_type_d2 == GLB_STM_MODE_IPUCV6) || (ftm_info_profile_type_tm == GLB_STM_MODE_IPUCV6))
    {
        /*creat acl entry for icmpv6 iloop solution*/
        sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
        reason_id = CTC_PKT_CPU_REASON_L3_PDU+CTC_L3PDU_ACTION_INDEX_ICMPV6;
        nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
        ctc_sai_copp_add_acl_entry_for_reason_icmpv6(nh_oid, &acl_entry_oids[0], &counter_oid); 
        g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
        g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
        g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;

        /*creat acl entry for icmpv6 iloop solution in GB*/
        sal_memset(&acl_entry_oids, 0, sizeof(acl_entry_oids));
        reason_id = CTC_PKT_CPU_REASON_LINK_ID_FAIL;
        nh_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_NEXT_HOP, g_sai_copp_master.ctc_reason[reason_id].nexthop_oid);
        ctc_sai_copp_add_acl_entry_for_reason_icmpv6(nh_oid, &acl_entry_oids[0], &counter_oid); 
        g_sai_copp_master.ctc_reason[reason_id].acl_entry_oid[0] = acl_entry_oids[0];
        g_sai_copp_master.ctc_reason[reason_id].nexthop_oid = nh_oid;
        g_sai_copp_master.ctc_reason[reason_id].counter_oid = counter_oid;
    }
    #endif

    if (0 != ctclib_opb_create(&p_sai_glb->copp_entry_opb, 0, GLB_COPP_ENTRY_MAX, "copp_entry_id"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    return SAI_STATUS_SUCCESS;
}


#define ________SAI_COPP_INNER_FUNC

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = ctc_sai_copp_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }
    
    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_copp_api_t g_copp_sai_api_func = {
        .set_copp_queue_attribute               = sai_set_copp_queue_attribute,
        .get_copp_queue_attribute               = sai_get_copp_queue_attribute,
        .get_copp_queue_stats                   = sai_get_copp_queue_statistics,
        .clear_copp_queue_stats                 = sai_clear_copp_queue_statistics,
        .create_copp_entry                      = sai_create_copp_entry,
        .delete_copp_entry                      = sai_delete_copp_entry,
};

static ctc_sai_api_reg_info_t g_copp_api_reg_info = {
        .id         = SAI_API_COPP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_copp_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_COPP_OUTER_FUNC



sai_status_t
ctc_sai_copp_init()
{
    api_reg_register_fn(&g_copp_api_reg_info);
    
    return SAI_STATUS_SUCCESS;
}


