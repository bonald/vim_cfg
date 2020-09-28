
#ifndef __TBL_DOT1X_PORT_H__
#define __TBL_DOT1X_PORT_H__

int32
tbl_dot1x_port_add_dot1x_port_sync(tbl_dot1x_port_t *p_dot1x_port, uint32 sync);

int32
tbl_dot1x_port_del_dot1x_port_sync(tbl_dot1x_port_key_t *p_dot1x_port_key, uint32 sync);

int32
tbl_dot1x_port_set_dot1x_port_field_sync(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_field_id_t field_id, uint32 sync);

int32
tbl_dot1x_port_add_dot1x_port(tbl_dot1x_port_t *p_dot1x_port);

int32
tbl_dot1x_port_del_dot1x_port(tbl_dot1x_port_key_t *p_dot1x_port_key);

int32
tbl_dot1x_port_set_dot1x_port_field(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_field_id_t field_id);

tbl_dot1x_port_t*
tbl_dot1x_port_get_dot1x_port(tbl_dot1x_port_key_t *p_dot1x_port_key);

char*
tbl_dot1x_port_key_val2str(tbl_dot1x_port_t *p_dot1x_port, char *str, uint32 str_len);

int32
tbl_dot1x_port_key_str2val(char *str, tbl_dot1x_port_t *p_dot1x_port);

int32
tbl_dot1x_port_dump_one(tbl_dot1x_port_t *p_dot1x_port, tbl_iter_args_t *pargs);

char*
tbl_dot1x_port_key_name_dump(tbl_dot1x_port_t *p_dot1x_port, char* str);

char*
tbl_dot1x_port_key_value_dump(tbl_dot1x_port_t *p_dot1x_port, char* str);

char*
tbl_dot1x_port_field_name_dump(tbl_dot1x_port_t *p_dot1x_port, int32 field_id, char* str);

char*
tbl_dot1x_port_field_value_dump(tbl_dot1x_port_t *p_dot1x_port, int32 field_id, char* str);

char**
tbl_dot1x_port_table_dump(tbl_dot1x_port_t *p_dot1x_port, char** str);

int32
tbl_dot1x_port_table_parser(char** array, char* key_value, tbl_dot1x_port_t *p_dot1x_port);

int32
tbl_dot1x_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dot1x_port_master_t*
tbl_dot1x_port_get_master();

tbl_dot1x_port_master_t*
tbl_dot1x_port_init_dot1x_port();

#endif /* !__TBL_DOT1X_PORT_H__ */

