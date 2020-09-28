
#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_fdb.h"

int32
ctc_ttp_drv_fdb_add_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_l2_addr_t   l2_addr;
    hsrv_bridge_flow_entry_t *p_entry = &p_flow->flow.flow_data.bridge_flow_entry;
    hsrv_openflow_ttp_group_t *p_group = NULL;
    uint32 nh_id = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    l2_addr.fid = p_entry->match_criteria.fid;
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    sal_memcpy(l2_addr.mac, p_entry->match_criteria.dst_mac, sizeof(mac_addr_t));
    p_group = hsrv_openflow_group_get_group_info(p_entry->group_id);
    if (!p_group)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    nh_id = p_group->nh_id;
    return ctc_l2_add_fdb_with_nexthop(&l2_addr, nh_id);
}

int32
ctc_ttp_drv_fdb_remove_entry(hsrv_openflow_ttp_flow_t *p_flow)
{
    ctc_l2_addr_t   l2_addr;
    hsrv_bridge_flow_entry_t *p_entry = &p_flow->flow.flow_data.bridge_flow_entry;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    l2_addr.fid = p_entry->match_criteria.fid;
    l2_addr.flag = CTC_L2_FLAG_IS_STATIC;
    sal_memcpy(l2_addr.mac, p_entry->match_criteria.dst_mac, sizeof(mac_addr_t));

    return ctc_l2_remove_fdb(&l2_addr);
}

int32
ctc_ttp_drv_fdb_add_default_entry(uint16 fid)
{
    ctc_l2dflt_addr_t   l2dflt;

    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    
    l2dflt.fid = fid;
    l2dflt.l2mc_grp_id = g_pst_ttp_bridge_flow_table_miss_group_id;
    l2dflt.share_grp_en = 1;
    
    return ctc_l2_add_default_entry(&l2dflt);
}

int32
ctc_ttp_drv_fdb_del_default_entry(uint16 fid)
{
    ctc_l2dflt_addr_t   l2dflt;

    sal_memset(&l2dflt, 0, sizeof(l2dflt));
    l2dflt.fid = fid;
    
    return ctc_l2_remove_default_entry(&l2dflt);
}


