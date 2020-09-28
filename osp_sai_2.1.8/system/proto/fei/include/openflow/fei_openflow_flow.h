#ifndef __FEI_OPENFLOW_FLOW_H__
#define __FEI_OPENFLOW_FLOW_H__

int32
fei_flow_add_flow(glb_openflow_flow_t* flow);

int32
fei_flow_mod_flow(glb_openflow_flow_t* p_flow);

int32
fei_flow_del_flow(uint32 flow_id);

int32
fei_flow_reset_flow(void);

int32
fei_flow_clear_stats(uint32 flow_id);

/* Added by weizj for tunnel bug 33724 */
/* Modifed by weizj for bug 35135 */
int32
fei_flow_set_drop_ingress(uint8 is_group, uint8 is_drop);

int32
fei_openflow_update_all_action_list(uint32 ifindex, bool enable);

/* Added by weizj for udf */
int32
fei_flow_set_udf_enable(uint8 enable);

int32
fei_flow_set_e2e_enable(uint8 enable);

int32
fei_flow_set_udf_parser(glb_openflow_udf_parser_t *p_info);
/* End by weizj for udf */

int32
fei_flow_add_tpoam_flow(glb_openflow_tpoam_t* p_flow);

#endif /* !__FEI_OPENFLOW_FLOW_H__ */
