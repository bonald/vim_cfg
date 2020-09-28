
#ifndef __TBL_L2_PROTOCOL_H__
#define __TBL_L2_PROTOCOL_H__

int32
tbl_l2_protocol_add_l2_protocol_sync(tbl_l2_protocol_t *p_l2_protocol, uint32 sync);

int32
tbl_l2_protocol_del_l2_protocol_sync(tbl_l2_protocol_key_t *p_l2_protocol_key, uint32 sync);

int32
tbl_l2_protocol_set_l2_protocol_field_sync(tbl_l2_protocol_t *p_l2_protocol, tbl_l2_protocol_field_id_t field_id, uint32 sync);

int32
tbl_l2_protocol_add_l2_protocol(tbl_l2_protocol_t *p_l2_protocol);

int32
tbl_l2_protocol_del_l2_protocol(tbl_l2_protocol_key_t *p_l2_protocol_key);

int32
tbl_l2_protocol_set_l2_protocol_field(tbl_l2_protocol_t *p_l2_protocol, tbl_l2_protocol_field_id_t field_id);

tbl_l2_protocol_t*
tbl_l2_protocol_get_l2_protocol(tbl_l2_protocol_key_t *p_l2_protocol_key);

char*
tbl_l2_protocol_key_val2str(tbl_l2_protocol_t *p_l2_protocol, char *str, uint32 str_len);

int32
tbl_l2_protocol_key_str2val(char *str, tbl_l2_protocol_t *p_l2_protocol);

int32
tbl_l2_protocol_dump_one(tbl_l2_protocol_t *p_l2_protocol, tbl_iter_args_t *pargs);

char*
tbl_l2_protocol_key_name_dump(tbl_l2_protocol_t *p_l2_protocol, char* str);

char*
tbl_l2_protocol_key_value_dump(tbl_l2_protocol_t *p_l2_protocol, char* str);

char*
tbl_l2_protocol_field_name_dump(tbl_l2_protocol_t *p_l2_protocol, int32 field_id, char* str);

char*
tbl_l2_protocol_field_value_dump(tbl_l2_protocol_t *p_l2_protocol, int32 field_id, char* str);

char**
tbl_l2_protocol_table_dump(tbl_l2_protocol_t *p_l2_protocol, char** str);

int32
tbl_l2_protocol_table_parser(char** array, char* key_value, tbl_l2_protocol_t *p_l2_protocol);

int32
tbl_l2_protocol_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_l2_protocol_master_t*
tbl_l2_protocol_get_master();

tbl_l2_protocol_master_t*
tbl_l2_protocol_init_l2_protocol();

#endif /* !__TBL_L2_PROTOCOL_H__ */

