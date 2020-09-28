
#ifndef __TBL_MLAG_PORT_H__
#define __TBL_MLAG_PORT_H__

int32
tbl_mlag_port_add_mlag_port_sync(tbl_mlag_port_t *p_port, uint32 sync);

int32
tbl_mlag_port_del_mlag_port_sync(tbl_mlag_port_key_t *p_port_key, uint32 sync);

int32
tbl_mlag_port_set_mlag_port_field_sync(tbl_mlag_port_t *p_port, tbl_mlag_port_field_id_t field_id, uint32 sync);

int32
tbl_mlag_port_add_mlag_port(tbl_mlag_port_t *p_port);

int32
tbl_mlag_port_del_mlag_port(tbl_mlag_port_key_t *p_port_key);

int32
tbl_mlag_port_set_mlag_port_field(tbl_mlag_port_t *p_port, tbl_mlag_port_field_id_t field_id);

tbl_mlag_port_t*
tbl_mlag_port_get_mlag_port(tbl_mlag_port_key_t *p_port_key);

char*
tbl_mlag_port_key_val2str(tbl_mlag_port_t *p_port, char *str, uint32 str_len);

int32
tbl_mlag_port_key_str2val(char *str, tbl_mlag_port_t *p_port);

int32
tbl_mlag_port_dump_one(tbl_mlag_port_t *p_port, tbl_iter_args_t *pargs);

char*
tbl_mlag_port_key_name_dump(tbl_mlag_port_t *p_port, char* str);

char*
tbl_mlag_port_key_value_dump(tbl_mlag_port_t *p_port, char* str);

char*
tbl_mlag_port_field_name_dump(tbl_mlag_port_t *p_port, int32 field_id, char* str);

char*
tbl_mlag_port_field_value_dump(tbl_mlag_port_t *p_port, int32 field_id, char* str);

char**
tbl_mlag_port_table_dump(tbl_mlag_port_t *p_port, char** str);

int32
tbl_mlag_port_table_parser(char** array, char* key_value, tbl_mlag_port_t *p_port);

int32
tbl_mlag_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mlag_port_master_t*
tbl_mlag_port_get_master();

tbl_mlag_port_master_t*
tbl_mlag_port_init_mlag_port();

#endif /* !__TBL_MLAG_PORT_H__ */

