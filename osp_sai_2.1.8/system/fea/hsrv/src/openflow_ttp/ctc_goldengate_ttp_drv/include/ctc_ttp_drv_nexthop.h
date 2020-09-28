
#ifndef __CTC_TTP_DRV_NEXTHOP_H__
#define __CTC_TTP_DRV_NEXTHOP_H__

int32
ctc_ttp_drv_nexthop_alloc_nhid(uint32 *nhid);

int32
ctc_ttp_drv_nexthop_release_nhid(uint32 nhid);

int32
ctc_ttp_drv_nexthop_alloc_mc_groupid(uint32 *mc_group_id);

int32
ctc_ttp_drv_nexthop_release_mc_groupid(uint32 mc_group_id);

int32
ctc_ttp_drv_nexthop_add_mcast_nh(uint32 mc_group_id, uint32 mc_nhid, uint32 stats_id);

int32
ctc_ttp_drv_nexthop_del_mcast_nh(uint32 mc_nhid);

int32
ctc_ttp_drv_nexthop_add_mcast_member(uint32 mc_nhid, uint32 member_nhid);

int32
ctc_ttp_drv_nexthop_del_mcast_member(uint32 mc_nhid, uint32 member_nhid);

int32
ctc_ttp_drv_nexthop_add_ecmp_nh(uint32 ecmp_nhid, uint32 stats_id);

int32
ctc_ttp_drv_nexthop_del_ecmp_nh(uint32 ecmp_nhid);

int32
ctc_ttp_drv_nexthop_add_ecmp_member(uint32 ecmp_nhid, uint32 *member_nhid, uint32 member_count);

int32
ctc_ttp_drv_nexthop_del_ecmp_member(uint32 ecmp_nhid, uint32 *member_nhid, uint32 member_count);

int32
ctc_ttp_drv_nexthop_add_ipuc_nh(hsrv_openflow_ttp_group_bucket_entry_t *l3_bucket_entry, 
                                                hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, 
                                                uint32 nhid);

int32
ctc_ttp_drv_nexthop_del_ipuc_nh(uint32 nhid);

int32
ctc_ttp_drv_nexthop_add_vlan_edit_nh(hsrv_openflow_ttp_group_bucket_entry_t *l2_bucket_entry, uint32 nhid, uint32 stats_id);

int32
ctc_ttp_drv_nexthop_del_vlan_edit_nh(uint32 nhid);

uint32
ctc_ttp_drv_nexthop_get_controller_nh(void);

int32
ctc_ttp_drv_nexthop_init(void);

#endif