
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_ipuc.h"


int32
ctc_ttp_drv_ipuc_add_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_ipuc_param_t    ipuc_info;
    hsrv_uincast_routing_flow_entry_t *p_entry = &p_flow->flow.flow_data.unicast_routing_flow_entry;
    hsrv_openflow_ttp_group_t *p_group = NULL;
    uint8 mask_len = 0;
    
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    
    ipuc_info.ip_ver = CTC_IP_VER_4;
    sal_memcpy(&ipuc_info.ip.ipv4, &p_entry->match_criteria.dst_ip, sizeof(ip_addr_t));
    IPV4_MASK_TO_LEN(p_entry->match_criteria.dst_ip_mask, mask_len);
    ipuc_info.masklen = mask_len;
    
    if (p_entry->send_to_controller)
    {
        ipuc_info.nh_id = ctc_ttp_drv_nexthop_get_controller_nh();
    }
    else
    {
        p_group = hsrv_openflow_group_get_group_info(p_entry->group_id);
        if (!p_group)
        {
            return HSRV_E_ENTRY_NOT_EXIST;
        }
        ipuc_info.nh_id = p_group->nh_id;
    }
    ipuc_info.vrf_id = p_entry->match_criteria.vrf;

    return ctc_ipuc_add(&ipuc_info);
}

int32
ctc_ttp_drv_ipuc_remove_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_ipuc_param_t    ipuc_info;
    hsrv_uincast_routing_flow_entry_t *p_entry = &p_flow->flow.flow_data.unicast_routing_flow_entry;
    uint8 mask_len = 0;
    
    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));
    
    ipuc_info.ip_ver = CTC_IP_VER_4;
    sal_memcpy(&ipuc_info.ip.ipv4, &p_entry->match_criteria.dst_ip, sizeof(ip_addr_t));
    IPV4_MASK_TO_LEN(p_entry->match_criteria.dst_ip_mask, mask_len);
    ipuc_info.masklen = mask_len;
    ipuc_info.vrf_id = p_entry->match_criteria.vrf;

    return ctc_ipuc_remove(&ipuc_info);
}

int32
ctc_ttp_drv_ipuc_add_default_entry(uint16 vrf_id)
{
    ctc_ipuc_param_t    ipuc_info;

    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));

    ipuc_info.vrf_id = vrf_id;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.nh_id = g_pst_ttp_unicast_flow_table_miss_nhid;
    ipuc_info.ip.ipv4 = 0;
    return ctc_ipuc_add(&ipuc_info);
}

int32
ctc_ttp_drv_ipuc_del_default_entry(uint16 vrf_id)
{
    ctc_ipuc_param_t    ipuc_info;

    sal_memset(&ipuc_info, 0, sizeof(ctc_ipuc_param_t));

    ipuc_info.vrf_id = vrf_id;
    ipuc_info.ip_ver = CTC_IP_VER_4;
    ipuc_info.ip.ipv4 = 0;
    ipuc_info.nh_id = g_pst_ttp_unicast_flow_table_miss_nhid;
    return ctc_ipuc_remove(&ipuc_info);
}

