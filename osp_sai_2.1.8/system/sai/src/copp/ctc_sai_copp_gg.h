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

extern ctc_sai_copp_info_t g_sai_copp_master;

#ifdef GOLDENGATE
extern int32
sys_goldengate_internal_port_set_routed_port(uint8 lchip, uint16 gport, uint32 is_routed);

extern int32
sys_goldengate_add_port_to_channel(uint8 lchip, uint16 lport, uint8 channel);

extern int32
sys_goldengate_packet_speical_set_cpu_port0(uint8 lchip, uint16 cpu_port);

extern int32
sys_goldengate_packet_speical_set_cpu_port1(uint8 lchip, uint16 cpu_port);

extern int32
sys_goldengate_packet_speical_set_cpu_port2(uint8 lchip, uint16 cpu_port);

extern int32
sys_goldengate_packet_speical_set_cpu_port3(uint8 lchip, uint16 cpu_port);
#endif

int32
ctc_sai_copp_internal_port_init()
{
    ctc_internal_port_assign_para_t alloc_iloop_port;
    ctc_loopback_nexthop_param_t alloc_iloop_nh;
    sai_uint32_t nexthopid = 0;
    uint16 inter_port = 0;
    uint32 index = 0;

    for (index = SAI_COPP_REASON_PORT_ID; index < SAI_COPP_PORT_ID_MAX; index++)
    {
        /*get the iloop port*/
        sal_memset(&alloc_iloop_port, 0, sizeof(alloc_iloop_port));
        alloc_iloop_port.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
        CTC_ERROR_RETURN(ctc_alloc_internal_port(&alloc_iloop_port));
        inter_port = alloc_iloop_port.inter_port;

        /*set the iloop port field*/
        CTC_ERROR_RETURN(ctc_port_set_property(inter_port, CTC_PORT_PROP_ROUTE_EN, FALSE));
        CTC_ERROR_RETURN(ctc_port_set_bridge_en(inter_port, FALSE));
        CTC_ERROR_RETURN(ctc_port_set_port_en(inter_port, TRUE));
        CTC_ERROR_RETURN(ctc_port_set_default_vlan(inter_port, 0));
        CTC_ERROR_RETURN(ctc_port_set_learning_en(inter_port, FALSE));
        
        sys_goldengate_internal_port_set_routed_port(0, inter_port, 1);

        /*alloc nexthopid and add iloop port with the nexthopid*/
        sal_memset(&alloc_iloop_nh, 0, sizeof(alloc_iloop_nh));
        CTC_ERROR_RETURN(ctc_sai_nexthop_alloc_offset(&nexthopid));
        /*add by yangl for cpu mirror 2019-05-20*/
        if (index == SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID)
        {
            alloc_iloop_nh.lpbk_lport = inter_port | 0x8000;
            sys_goldengate_packet_speical_set_cpu_port0(0, inter_port);
            sys_goldengate_add_port_to_channel(0, inter_port, 0x35);
        }
        else if (index == SAI_COPP_L2PRO_GROUP_MAC_PORT_ID)
        {
            alloc_iloop_nh.lpbk_lport = inter_port | 0x8000;
            sys_goldengate_packet_speical_set_cpu_port1(0, inter_port);
            sys_goldengate_add_port_to_channel(0, inter_port, 0x35);
        }
        else if (index == SAI_COPP_IGMP_PORT_ID )
        {
            alloc_iloop_nh.lpbk_lport = inter_port | 0x8000;
            sys_goldengate_packet_speical_set_cpu_port2(0, inter_port);
            sys_goldengate_add_port_to_channel(0, inter_port, 0x35);
        }
         else if (index == SAI_COPP_TTL_EXPIRE_PORT_ID)
        {
            alloc_iloop_nh.lpbk_lport = inter_port | 0x8000;
            sys_goldengate_packet_speical_set_cpu_port3(0, inter_port);
            sys_goldengate_add_port_to_channel(0, inter_port, 0x35);
        }
        else
        {

            alloc_iloop_nh.lpbk_lport = inter_port;
            
        }
        /*yangl end*/
        CTC_ERROR_RETURN(ctc_nh_add_iloop(nexthopid, &alloc_iloop_nh));
        
        g_sai_copp_master.ctc_copp_port_info[index].nexthop_id = nexthopid;
        g_sai_copp_master.ctc_copp_port_info[index].port_id = inter_port;

        /*set the iloop port property and direction property*/    
        ctc_port_set_direction_property(inter_port, CTC_PORT_DIR_PROP_ACL_EN, CTC_INGRESS, 0x5);/*tcam0 & tcam2*/

        ctc_port_set_direction_property(inter_port, CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID, CTC_INGRESS, index);
        ctc_port_set_direction_property(inter_port, CTC_PORT_DIR_PROP_ACL_USE_CLASSID, CTC_INGRESS, 0);
        ctc_port_set_property(inter_port, CTC_PORT_PROP_L3PDU_ARP_ACTION, CTC_PORT_ARP_ACTION_TYPE_FW);
        ctc_port_set_property(inter_port, CTC_PORT_PROP_L3PDU_DHCP_ACTION, CTC_PORT_DHCP_ACTION_TYPE_FW);

        /*modified by chenc for bug 51436, 2019-04-08*/
        if (index == SAI_COPP_MTU_CHECK_FAIL_PORT_ID)
        {
            ctc_port_set_property(inter_port, CTC_PORT_PROP_RAW_PKT_TYPE, 1);
        }
    }
    
    return SAI_STATUS_SUCCESS;
}

int32
ctc_sai_copp_tbl_init()
{
    sai_acl_api_t *acl_api = NULL;
    sai_attribute_t attr_list[12];
    sai_object_id_t acl_table_id = 0;

    CTC_SAI_DEBUG_FUNC();
    sai_api_query(SAI_API_ACL,(void**)&acl_api);

    /* ingress copp ACL table created */
    sal_memset(attr_list, 0, sizeof(attr_list));
    attr_list[0].id = SAI_ACL_TABLE_ATTR_STAGE;
    attr_list[0].value.s32 = SAI_ACL_STAGE_INGRESS;

    attr_list[1].id = SAI_ACL_TABLE_ATTR_PRIORITY;
    attr_list[1].value.u32 = SAI_ACL_COPP_GROUP_PRIORITY_GG;

    attr_list[2].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_list[2].value.booldata = TRUE;

    attr_list[3].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_list[3].value.booldata = TRUE;

    attr_list[4].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_list[4].value.booldata = TRUE;

    attr_list[5].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_list[5].value.booldata = TRUE;

    attr_list[6].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
    attr_list[6].value.booldata = TRUE;

    attr_list[7].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[7].value.booldata = TRUE;

    attr_list[8].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    attr_list[8].value.booldata = TRUE;

    attr_list[9].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    attr_list[9].value.booldata = TRUE;

    attr_list[10].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr_list[10].value.booldata = TRUE;

    attr_list[11].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_list[11].value.booldata = TRUE;
    CTC_ERROR_RETURN(acl_api->create_acl_table(&acl_table_id, 12, attr_list));
    g_sai_copp_master.ctc_copp_igs_tbl_oid = acl_table_id;

    return SAI_STATUS_SUCCESS;

}

int32
ctc_sai_acl_copp_group_init()
{    
    ctc_acl_group_info_t acl_group;
    sai_uint32_t group_id = 0;
    sai_uint32_t port_bitmap = 0; 
    uint32 index = 0;
    uint32 iloop_port = 0;

    for (index = SAI_ACL_COPP_REASON_GROUP; index < SAI_ACL_COPP_REASON_GROUP_MAX; index++)
    {
        if (index == SAI_ACL_COPP_ACL_GROUP)
        {
            continue;
        }
        sal_memset(&acl_group, 0, sizeof(acl_group));
        group_id = index;
        acl_group.type = CTC_ACL_GROUP_TYPE_PORT_BITMAP;
        acl_group.dir  = CTC_INGRESS;
        acl_group.priority = SAI_ACL_COPP_GROUP_PRIORITY_GG;   /* tcam2, low priority than tcam0 */

        iloop_port = ctc_sai_acl_get_iloop_port_from_copp_reason_group(index);
        port_bitmap = (1 << iloop_port);
        acl_group.un.port_bitmap[0] = port_bitmap;
        
        ctc_acl_create_group(group_id, &acl_group);
        ctc_acl_install_group(group_id, &acl_group);
    }

    /*normal acl for copp*/
    sal_memset(&acl_group, 0, sizeof(acl_group));
    group_id = SAI_ACL_COPP_ACL_GROUP;
    acl_group.type     = CTC_ACL_GROUP_TYPE_PORT_BITMAP;
    acl_group.dir      = CTC_INGRESS;
    acl_group.priority = 0; 
    port_bitmap = (1 << SAI_COPP_REASON_PORT_ID) | (1 << SAI_COPP_FWD_PORT_ID) |
                  (1 << SAI_COPP_HYBIRD_PORT_ID) | (1 << SAI_COPP_TTL_EXPIRE_PORT_ID)|
                  (1 << SAI_COPP_L2PRO_PROTOCOL_MAC_PORT_ID) | (1 << SAI_COPP_L2PRO_GROUP_MAC_PORT_ID)|
                  (1<<SAI_COPP_IGMP_PORT_ID)|(1 << SAI_COPP_MTU_CHECK_FAIL_PORT_ID);
    acl_group.un.port_bitmap[0] = port_bitmap;
    ctc_acl_create_group(group_id, &acl_group);
    ctc_acl_install_group(group_id, &acl_group);

    /* add by hansf for gg copp */
    ctc_acl_entry_t  acl_entry;  /*gg default entry used to sperated with normal ace*/
    sal_memset(&acl_entry, 0x0, sizeof(acl_entry));
    acl_entry.key.type = CTC_ACL_KEY_IPV4;
    acl_entry.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
    acl_entry.priority_valid = 1;
    acl_entry.priority = SAI_ACL_COPP_DEFAULT_ACL_PRIORITY;
    acl_entry.entry_id = SAI_ACL_COPP_DEFAULT_ACL_ENTRY_ID;
    ctc_acl_add_entry(group_id, &acl_entry);
    ctc_acl_install_entry(SAI_ACL_COPP_DEFAULT_ACL_ENTRY_ID);
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_get_copp_queue_statistics(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_copp_queue_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
    )
{
    sai_status_t     ret         = SAI_STATUS_SUCCESS;
    uint32_t         attr_idx    = 0;
    ctc_qos_queue_stats_t queue_stats;
    uint32_t  sai_queue_id =0;
    uint32_t  ctc_queue_id = 0;
    uint16_t  ctc_reason_id = 0;

    uint32 reason_id[8] = 
       {CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,0,0,
       };
    
    CTC_SAI_DEBUG_FUNC();

    sal_memset(&queue_stats, 0x0, sizeof(queue_stats));
    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);
    if (ctc_queue_id >= 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    ctc_reason_id = reason_id[ctc_queue_id];
    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    queue_stats.queue.cpu_reason = ctc_reason_id;
    ret = ctc_qos_query_queue_stats(&queue_stats);
    if(ret)
    {
        return ret;
    }
    
    for(attr_idx = 0; attr_idx < number_of_counters; attr_idx++)
    {
        switch(counter_ids[attr_idx])
        {
            case SAI_COPP_QUEUE_STAT_PACKETS:
                counters[attr_idx] = queue_stats.stats.deq_packets;
                break;
            case SAI_COPP_QUEUE_STAT_BYTES:
                counters[attr_idx] = queue_stats.stats.deq_bytes;
                break;
            case SAI_COPP_QUEUE_STAT_DROPPED_PACKETS:
                counters[attr_idx] = queue_stats.stats.drop_packets;
                break;
            case SAI_COPP_QUEUE_STAT_DROPPED_BYTES:
                counters[attr_idx] = queue_stats.stats.drop_bytes;
                break;
            default:
                ret = SAI_STATUS_NOT_SUPPORTED;
                break;
        }
    }
    
    return ret;
}

sai_status_t 
sai_clear_copp_queue_statistics(_In_ sai_object_id_t queue_id)
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    uint32_t  sai_queue_id =0;
    uint32_t  ctc_queue_id = 0;
    ctc_qos_queue_stats_t queue_stats;
    uint16_t  ctc_reason_id = 0;

    uint32 reason_id[8] = 
       {CTC_PKT_CPU_REASON_IP_TTL_CHECK_FAIL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_PDU + CTC_L3PDU_ACTION_INDEX_ARP+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_CUSTOM_BASE + 1 + CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_EAPOL+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L2_PDU + CTC_L2PDU_ACTION_INDEX_BPDU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,
        CTC_PKT_CPU_REASON_L3_COPY_CPU+CTC_PKT_CPU_REASON_CUSTOM_BASE*2,0,0,
       };

    CTC_SAI_DEBUG_FUNC();

    sai_queue_id = CTC_SAI_OBJECT_INDEX_GET(queue_id);
    ctc_queue_id = CTC_SAI_GET_QUEUE_OFFSET_BY_QUEUE_ID(sai_queue_id);
    if (ctc_queue_id >= 8)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    /*added by yejl for bug48956, 2018-10-12*/
    sal_memset(&queue_stats, 0, sizeof(queue_stats));
    ctc_reason_id = reason_id[ctc_queue_id];
    queue_stats.queue.queue_type = CTC_QUEUE_TYPE_EXCP_CPU;
    queue_stats.queue.cpu_reason = ctc_reason_id;
    ret = ctc_qos_clear_queue_stats(&queue_stats);
    return ret;
}
 
