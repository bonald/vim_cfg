
#ifndef __TBL_IPROUTE_NODE_H__
#define __TBL_IPROUTE_NODE_H__

int32
tbl_iproute_node_add_iproute_node_sync(tbl_iproute_node_t *p_rn, uint32 sync);

int32
tbl_iproute_node_del_iproute_node_sync(tbl_iproute_node_key_t *p_rn_key, uint32 sync);

int32
tbl_iproute_node_set_iproute_node_field_sync(tbl_iproute_node_t *p_rn, tbl_iproute_node_field_id_t field_id, uint32 sync);

int32
tbl_iproute_node_add_iproute_node(tbl_iproute_node_t *p_rn);

int32
tbl_iproute_node_del_iproute_node(tbl_iproute_node_key_t *p_rn_key);

int32
tbl_iproute_node_set_iproute_node_field(tbl_iproute_node_t *p_rn, tbl_iproute_node_field_id_t field_id);

tbl_iproute_node_t*
tbl_iproute_node_get_iproute_node(tbl_iproute_node_key_t *p_rn_key);

char*
tbl_iproute_node_key_val2str(tbl_iproute_node_t *p_rn, char *str, uint32 str_len);

int32
tbl_iproute_node_key_str2val(char *str, tbl_iproute_node_t *p_rn);

int32
tbl_iproute_node_dump_one(tbl_iproute_node_t *p_rn, tbl_iter_args_t *pargs);

char*
tbl_iproute_node_key_name_dump(tbl_iproute_node_t *p_rn, char* str);

char*
tbl_iproute_node_key_value_dump(tbl_iproute_node_t *p_rn, char* str);

char*
tbl_iproute_node_field_name_dump(tbl_iproute_node_t *p_rn, int32 field_id, char* str);

char*
tbl_iproute_node_field_value_dump(tbl_iproute_node_t *p_rn, int32 field_id, char* str);

char**
tbl_iproute_node_table_dump(tbl_iproute_node_t *p_rn, char** str);

int32
tbl_iproute_node_table_parser(char** array, char* key_value, tbl_iproute_node_t *p_rn);

int32
tbl_iproute_node_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_iproute_node_master_t*
tbl_iproute_node_get_master();

tbl_iproute_node_master_t*
tbl_iproute_node_init_iproute_node();

#endif /* !__TBL_IPROUTE_NODE_H__ */

