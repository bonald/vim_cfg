
#ifndef __TBL_SFLOW_COUNTER_PORT_H__
#define __TBL_SFLOW_COUNTER_PORT_H__

int32
tbl_sflow_counter_port_add_sflow_counter_port_sync(tbl_sflow_counter_port_t *p_sflow_counter_port, uint32 sync);

int32
tbl_sflow_counter_port_del_sflow_counter_port_sync(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key, uint32 sync);

int32
tbl_sflow_counter_port_set_sflow_counter_port_field_sync(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_sflow_counter_port_field_id_t field_id, uint32 sync);

int32
tbl_sflow_counter_port_add_sflow_counter_port(tbl_sflow_counter_port_t *p_sflow_counter_port);

int32
tbl_sflow_counter_port_del_sflow_counter_port(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key);

int32
tbl_sflow_counter_port_set_sflow_counter_port_field(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_sflow_counter_port_field_id_t field_id);

tbl_sflow_counter_port_t*
tbl_sflow_counter_port_get_sflow_counter_port(tbl_sflow_counter_port_key_t *p_sflow_counter_port_key);

char*
tbl_sflow_counter_port_key_val2str(tbl_sflow_counter_port_t *p_sflow_counter_port, char *str, uint32 str_len);

int32
tbl_sflow_counter_port_key_str2val(char *str, tbl_sflow_counter_port_t *p_sflow_counter_port);

int32
tbl_sflow_counter_port_dump_one(tbl_sflow_counter_port_t *p_sflow_counter_port, tbl_iter_args_t *pargs);

char*
tbl_sflow_counter_port_key_name_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char* str);

char*
tbl_sflow_counter_port_key_value_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char* str);

char*
tbl_sflow_counter_port_field_name_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, int32 field_id, char* str);

char*
tbl_sflow_counter_port_field_value_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, int32 field_id, char* str);

char**
tbl_sflow_counter_port_table_dump(tbl_sflow_counter_port_t *p_sflow_counter_port, char** str);

int32
tbl_sflow_counter_port_table_parser(char** array, char* key_value, tbl_sflow_counter_port_t *p_sflow_counter_port);

int32
tbl_sflow_counter_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_sflow_counter_port_master_t*
tbl_sflow_counter_port_get_master();

tbl_sflow_counter_port_master_t*
tbl_sflow_counter_port_init_sflow_counter_port();

#endif /* !__TBL_SFLOW_COUNTER_PORT_H__ */

