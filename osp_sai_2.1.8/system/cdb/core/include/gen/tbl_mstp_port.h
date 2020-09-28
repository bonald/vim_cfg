
#ifndef __TBL_MSTP_PORT_H__
#define __TBL_MSTP_PORT_H__

int32
tbl_mstp_port_add_mstp_port_sync(tbl_mstp_port_t *p_mstp_port, uint32 sync);

int32
tbl_mstp_port_del_mstp_port_sync(tbl_mstp_port_key_t *p_mstp_port_key, uint32 sync);

int32
tbl_mstp_port_set_mstp_port_field_sync(tbl_mstp_port_t *p_mstp_port, tbl_mstp_port_field_id_t field_id, uint32 sync);

int32
tbl_mstp_port_add_mstp_port(tbl_mstp_port_t *p_mstp_port);

int32
tbl_mstp_port_del_mstp_port(tbl_mstp_port_key_t *p_mstp_port_key);

int32
tbl_mstp_port_set_mstp_port_field(tbl_mstp_port_t *p_mstp_port, tbl_mstp_port_field_id_t field_id);

tbl_mstp_port_t*
tbl_mstp_port_get_mstp_port(tbl_mstp_port_key_t *p_mstp_port_key);

char*
tbl_mstp_port_key_val2str(tbl_mstp_port_t *p_mstp_port, char *str, uint32 str_len);

int32
tbl_mstp_port_key_str2val(char *str, tbl_mstp_port_t *p_mstp_port);

int32
tbl_mstp_port_dump_one(tbl_mstp_port_t *p_mstp_port, tbl_iter_args_t *pargs);

char*
tbl_mstp_port_key_name_dump(tbl_mstp_port_t *p_mstp_port, char* str);

char*
tbl_mstp_port_key_value_dump(tbl_mstp_port_t *p_mstp_port, char* str);

char*
tbl_mstp_port_field_name_dump(tbl_mstp_port_t *p_mstp_port, int32 field_id, char* str);

char*
tbl_mstp_port_field_value_dump(tbl_mstp_port_t *p_mstp_port, int32 field_id, char* str);

char**
tbl_mstp_port_table_dump(tbl_mstp_port_t *p_mstp_port, char** str);

int32
tbl_mstp_port_table_parser(char** array, char* key_value, tbl_mstp_port_t *p_mstp_port);

int32
tbl_mstp_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mstp_port_master_t*
tbl_mstp_port_get_master();

tbl_mstp_port_master_t*
tbl_mstp_port_init_mstp_port();

int32
tbl_mstp_port_set_msti_list_sync(tbl_mstp_port_t *p_mstp_port, uint32 add, void *p_object, uint32 sync);

int32
tbl_mstp_port_set_msti_list(tbl_mstp_port_t *p_mstp_port, uint32 add, void *p_object);

#endif /* !__TBL_MSTP_PORT_H__ */

