#ifndef _HSRV_OPENFLOW_MPLS_PRIV_H_
#define _HSRV_OPENFLOW_MPLS_PRIV_H_

typedef  int32_t(*HSRV_OPENFLOW_MPLS_OUTPUT_ACTION_SEQ_ALLOC_RES_FUNC)(glb_openflow_flow_action_t **cur_flow_action,
                                                                            hsrv_of_nh_info_t* p_member_nh_array,
                                                                            uint16_t mcast_member_index);
#endif /* _HSRV_OPENFLOW_MPLS_PRIV_H_ */
