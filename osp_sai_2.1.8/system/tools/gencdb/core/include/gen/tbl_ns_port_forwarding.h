
#ifndef __TBL_NS_PORT_FORWARDING_H__
#define __TBL_NS_PORT_FORWARDING_H__

int32
tbl_ns_port_forwarding_add_ns_port_forwarding_sync(tbl_ns_port_forwarding_t *p_ns_port_forwarding, uint32 sync);

int32
tbl_ns_port_forwarding_del_ns_port_forwarding_sync(tbl_ns_port_forwarding_key_t *p_ns_port_forwarding_key, uint32 sync);

int32
tbl_ns_port_forwarding_set_ns_port_forwarding_field_sync(tbl_ns_port_forwarding_t *p_ns_port_forwarding, tbl_ns_port_forwarding_field_id_t field_id, uint32 sync);

int32
tbl_ns_port_forwarding_add_ns_port_forwarding(tbl_ns_port_forwarding_t *p_ns_port_forwarding);

int32
tbl_ns_port_forwarding_del_ns_port_forwarding(tbl_ns_port_forwarding_key_t *p_ns_port_forwarding_key);

int32
tbl_ns_port_forwarding_set_ns_port_forwarding_field(tbl_ns_port_forwarding_t *p_ns_port_forwarding, tbl_ns_port_forwarding_field_id_t field_id);

tbl_ns_port_forwarding_t*
tbl_ns_port_forwarding_get_ns_port_forwarding(tbl_ns_port_forwarding_key_t *p_ns_port_forwarding_key);

char*
tbl_ns_port_forwarding_key_val2str(tbl_ns_port_forwarding_t *p_ns_port_forwarding, char *str, uint32 str_len);

int32
tbl_ns_port_forwarding_key_str2val(char *str, tbl_ns_port_forwarding_t *p_ns_port_forwarding);

int32
tbl_ns_port_forwarding_dump_one(tbl_ns_port_forwarding_t *p_ns_port_forwarding, tbl_iter_args_t *pargs);

char*
tbl_ns_port_forwarding_key_name_dump(tbl_ns_port_forwarding_t *p_ns_port_forwarding, char* str);

char*
tbl_ns_port_forwarding_key_value_dump(tbl_ns_port_forwarding_t *p_ns_port_forwarding, char* str);

char*
tbl_ns_port_forwarding_field_name_dump(tbl_ns_port_forwarding_t *p_ns_port_forwarding, int32 field_id, char* str);

char*
tbl_ns_port_forwarding_field_value_dump(tbl_ns_port_forwarding_t *p_ns_port_forwarding, int32 field_id, char* str);

char**
tbl_ns_port_forwarding_table_dump(tbl_ns_port_forwarding_t *p_ns_port_forwarding, char** str);

int32
tbl_ns_port_forwarding_table_parser(char** array, char* key_value, tbl_ns_port_forwarding_t *p_ns_port_forwarding);

int32
tbl_ns_port_forwarding_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ns_port_forwarding_master_t*
tbl_ns_port_forwarding_get_master();

tbl_ns_port_forwarding_master_t*
tbl_ns_port_forwarding_init_ns_port_forwarding();

#endif /* !__TBL_NS_PORT_FORWARDING_H__ */

