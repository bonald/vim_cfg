int32
tbl_route_node_iterate_vrf(uint8 family, vrf_id_t vrf_id, TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_route_node_iterate_v4(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_route_node_iterate_v6(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
rt_route_delete_node(tbl_route_node_t *node);

tbl_route_node_t*
rt_route_next(tbl_route_node_t *node);

tbl_route_node_t*
rt_route_table_top(tbl_route_table_t *table);

int32
rt_route_table_add_table(uint8 family, vrf_id_t vrf_id);

int32
rt_route_table_del_table(uint8 family, vrf_id_t vrf_id);

tbl_route_table_t*
rt_route_table_get_table(uint8 family, vrf_id_t vrf_id);
