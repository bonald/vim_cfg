
#ifndef __STBL_QOS_MPLS_FLOW_H__
#define __STBL_QOS_MPLS_FLOW_H__

int32
stbl_qos_mpls_flow_add_qos_mpls_flow_sync(stbl_qos_mpls_flow_t *p_qos_mpls_flow, uint32 sync);

int32
stbl_qos_mpls_flow_del_qos_mpls_flow_sync(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key, uint32 sync);

int32
stbl_qos_mpls_flow_set_qos_mpls_flow_field_sync(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_qos_mpls_flow_field_id_t field_id, uint32 sync);

int32
stbl_qos_mpls_flow_add_qos_mpls_flow(stbl_qos_mpls_flow_t *p_qos_mpls_flow);

int32
stbl_qos_mpls_flow_del_qos_mpls_flow(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key);

int32
stbl_qos_mpls_flow_set_qos_mpls_flow_field(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_qos_mpls_flow_field_id_t field_id);

stbl_qos_mpls_flow_t*
stbl_qos_mpls_flow_get_qos_mpls_flow(stbl_qos_mpls_flow_key_t *p_qos_mpls_flow_key);

char*
stbl_qos_mpls_flow_key_val2str(stbl_qos_mpls_flow_t *p_qos_mpls_flow, char *str, uint32 str_len);

int32
stbl_qos_mpls_flow_key_str2val(char *str, stbl_qos_mpls_flow_t *p_qos_mpls_flow);

int32
stbl_qos_mpls_flow_dump_one(stbl_qos_mpls_flow_t *p_qos_mpls_flow, stbl_iter_args_t *pargs);

int32
stbl_qos_mpls_flow_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_qos_mpls_flow_master_t*
stbl_qos_mpls_flow_get_master();

stbl_qos_mpls_flow_master_t*
stbl_qos_mpls_flow_init_qos_mpls_flow();

#endif /* !__STBL_QOS_MPLS_FLOW_H__ */

