#ifndef _HSRV_OPENFLOW_MPLS_H_
#define _HSRV_OPENFLOW_MPLS_H_

int32
hsrv_openflow_mpls_map_group_output_action(glb_openflow_bucket_t* p_bucket,
          hsrv_of_nh_info_t* p_member_nh_array,
          uint32* p_member_cnt);

int32
hsrv_openflow_mpls_map_flow_output_action(hsrv_openflow_flow_t* p_flow);

int32
hsrv_openflow_mpls_init(hsrv_system_spec_info_t *p_system_info);

#endif /* _HSRV_OPENFLOW_MPLS_H_ */
