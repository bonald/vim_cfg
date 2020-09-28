#ifndef __HAL_API_AGG_H__
#define __HAL_API_AGG_H__

int32
fei_api_agg_add_aggregator(fei_lacp_add_aggregator_t *pst_req, uint32 *p_kernel_ifindex);

int32
fei_api_agg_del_aggregator(fei_lacp_del_aggregator_t *pst_req);

int32
fei_api_agg_add_member_port(fei_lacp_util_struct_t *pst_req);

int32
fei_api_agg_del_member_port(fei_lacp_util_struct_t *pst_req);

int32
fei_api_agg_sync_up_agg_prop(fei_lacp_util_struct_t *pst_req);

int32
fei_api_agg_clear_agg_prop(fei_lacp_util_struct_t *pst_req);

int32
fei_api_agg_attach_mux_to_aggregator(fei_lacp_attach_mux_to_aggregator_t *pst_req);

int32
fei_api_agg_detach_mux_from_aggregator(fei_lacp_attach_mux_to_aggregator_t *pst_req);

int32
fei_api_agg_set_load_balance(fei_lacp_load_balance_t *pst_req);

int32
fei_api_agg_set_load_balance_mode(fei_lacp_load_balance_mode_t *pst_req);

#endif /* !__HAL_API_AGG_H__ */

