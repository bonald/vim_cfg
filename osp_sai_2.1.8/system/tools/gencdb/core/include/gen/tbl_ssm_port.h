
#ifndef __TBL_SSM_PORT_H__
#define __TBL_SSM_PORT_H__

int32
tbl_ssm_port_add_ssm_port_sync(tbl_ssm_port_t *p_ssm_port, uint32 sync);

int32
tbl_ssm_port_del_ssm_port_sync(tbl_ssm_port_key_t *p_ssm_port_key, uint32 sync);

int32
tbl_ssm_port_set_ssm_port_field_sync(tbl_ssm_port_t *p_ssm_port, tbl_ssm_port_field_id_t field_id, uint32 sync);

int32
tbl_ssm_port_add_ssm_port(tbl_ssm_port_t *p_ssm_port);

int32
tbl_ssm_port_del_ssm_port(tbl_ssm_port_key_t *p_ssm_port_key);

int32
tbl_ssm_port_set_ssm_port_field(tbl_ssm_port_t *p_ssm_port, tbl_ssm_port_field_id_t field_id);

tbl_ssm_port_t*
tbl_ssm_port_get_ssm_port(tbl_ssm_port_key_t *p_ssm_port_key);

char*
tbl_ssm_port_key_val2str(tbl_ssm_port_t *p_ssm_port, char *str, uint32 str_len);

int32
tbl_ssm_port_key_str2val(char *str, tbl_ssm_port_t *p_ssm_port);

int32
tbl_ssm_port_dump_one(tbl_ssm_port_t *p_ssm_port, tbl_iter_args_t *pargs);

char*
tbl_ssm_port_key_name_dump(tbl_ssm_port_t *p_ssm_port, char* str);

char*
tbl_ssm_port_key_value_dump(tbl_ssm_port_t *p_ssm_port, char* str);

char*
tbl_ssm_port_field_name_dump(tbl_ssm_port_t *p_ssm_port, int32 field_id, char* str);

char*
tbl_ssm_port_field_value_dump(tbl_ssm_port_t *p_ssm_port, int32 field_id, char* str);

char**
tbl_ssm_port_table_dump(tbl_ssm_port_t *p_ssm_port, char** str);

int32
tbl_ssm_port_table_parser(char** array, char* key_value, tbl_ssm_port_t *p_ssm_port);

int32
tbl_ssm_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ssm_port_master_t*
tbl_ssm_port_get_master();

tbl_ssm_port_master_t*
tbl_ssm_port_init_ssm_port();

#endif /* !__TBL_SSM_PORT_H__ */

