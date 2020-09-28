
#ifndef __TBL_TAP_GROUP_INGRESS_FLOW_H__
#define __TBL_TAP_GROUP_INGRESS_FLOW_H__

int32
tbl_tap_group_ingress_flow_add_tap_group_ingress_flow_sync(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, uint32 sync);

int32
tbl_tap_group_ingress_flow_del_tap_group_ingress_flow_sync(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key, uint32 sync);

int32
tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field_sync(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_tap_group_ingress_flow_field_id_t field_id, uint32 sync);

int32
tbl_tap_group_ingress_flow_add_tap_group_ingress_flow(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow);

int32
tbl_tap_group_ingress_flow_del_tap_group_ingress_flow(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key);

int32
tbl_tap_group_ingress_flow_set_tap_group_ingress_flow_field(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_tap_group_ingress_flow_field_id_t field_id);

tbl_tap_group_ingress_flow_t*
tbl_tap_group_ingress_flow_get_tap_group_ingress_flow(tbl_tap_group_ingress_flow_key_t *p_tap_group_ingress_flow_key);

char*
tbl_tap_group_ingress_flow_key_val2str(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char *str, uint32 str_len);

int32
tbl_tap_group_ingress_flow_key_str2val(char *str, tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow);

int32
tbl_tap_group_ingress_flow_dump_one(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, tbl_iter_args_t *pargs);

char*
tbl_tap_group_ingress_flow_key_name_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char* str);

char*
tbl_tap_group_ingress_flow_key_value_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char* str);

char*
tbl_tap_group_ingress_flow_field_name_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, int32 field_id, char* str);

char*
tbl_tap_group_ingress_flow_field_value_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, int32 field_id, char* str);

char**
tbl_tap_group_ingress_flow_table_dump(tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow, char** str);

int32
tbl_tap_group_ingress_flow_table_parser(char** array, char* key_value, tbl_tap_group_ingress_flow_t *p_tap_group_ingress_flow);

int32
tbl_tap_group_ingress_flow_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_group_ingress_flow_master_t*
tbl_tap_group_ingress_flow_get_master();

tbl_tap_group_ingress_flow_master_t*
tbl_tap_group_ingress_flow_init_tap_group_ingress_flow();

int32
tbl_tap_group_ingress_flow_get_if_exist(const char* tap_grp, const char* if_name);

int32
tbl_tap_group_ingress_flow_get_group_if_exist(const char* tap_grp, const char* if_name);

int32
tbl_tap_group_ingress_flow_get_if_flow_exist(const char* if_name, const char* flow_name);


#endif /* !__TBL_TAP_GROUP_INGRESS_FLOW_H__ */

