
#ifndef __TBL_PORT_H__
#define __TBL_PORT_H__

int32
tbl_port_add_port_sync(tbl_port_t *p_port, uint32 sync);

int32
tbl_port_del_port_sync(tbl_port_key_t *p_port_key, uint32 sync);

int32
tbl_port_set_port_field_sync(tbl_port_t *p_port, tbl_port_field_id_t field_id, uint32 sync);

int32
tbl_port_add_port(tbl_port_t *p_port);

int32
tbl_port_del_port(tbl_port_key_t *p_port_key);

int32
tbl_port_set_port_field(tbl_port_t *p_port, tbl_port_field_id_t field_id);

tbl_port_t*
tbl_port_get_port(tbl_port_key_t *p_port_key);

char*
tbl_port_key_val2str(tbl_port_t *p_port, char *str, uint32 str_len);

int32
tbl_port_key_str2val(char *str, tbl_port_t *p_port);

int32
tbl_port_dump_one(tbl_port_t *p_port, tbl_iter_args_t *pargs);

char*
tbl_port_key_name_dump(tbl_port_t *p_port, char* str);

char*
tbl_port_key_value_dump(tbl_port_t *p_port, char* str);

char*
tbl_port_field_name_dump(tbl_port_t *p_port, int32 field_id, char* str);

char*
tbl_port_field_value_dump(tbl_port_t *p_port, int32 field_id, char* str);

char**
tbl_port_table_dump(tbl_port_t *p_port, char** str);

int32
tbl_port_table_parser(char** array, char* key_value, tbl_port_t *p_port);

int32
tbl_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_port_master_t*
tbl_port_get_master();

tbl_port_master_t*
tbl_port_init_port();

tbl_port_t*
tbl_port_get_port_by_slot_port(uint8 slot_no, uint8 port_no, uint8 sub_port_no);

#endif /* !__TBL_PORT_H__ */

