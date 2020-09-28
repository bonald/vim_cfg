#ifndef __HAGT_OPENFLOW_NEXTHOP_H__
#define __HAGT_OPENFLOW_NEXTHOP_H__

#define HAGT_OFNH_GROUP_MCAST_MEM_HASH        g_pst_hagt_openflow_master->pst_mcast_mem_hash

struct hagt_openflow_mcast_mem_e
{
    /* param */
    ctc_mcast_nh_param_group_t nh_param;
    
    /* Private data */
    uint32 nhid;                /**< Mcast group nhid */
    uint32 ref_cnt;             /**< Reference count */
    uint8 is_source_check_dis;
};
typedef struct hagt_openflow_mcast_mem_e hagt_openflow_mcast_mem_t;

int32
hagt_openflow_nexthop_init(void);

int32
hagt_openflow_nexthop_register_callback(void);



#endif /* !__HAGT_OPENFLOW_NEXTHOP_H__ */
