
#ifndef __TBL_FAN_H__
#define __TBL_FAN_H__

int32
tbl_fan_add_fan_sync(tbl_fan_t *p_fan, uint32 sync);

int32
tbl_fan_del_fan_sync(tbl_fan_key_t *p_fan_key, uint32 sync);

int32
tbl_fan_set_fan_field_sync(tbl_fan_t *p_fan, tbl_fan_field_id_t field_id, uint32 sync);

int32
tbl_fan_add_fan(tbl_fan_t *p_fan);

int32
tbl_fan_del_fan(tbl_fan_key_t *p_fan_key);

int32
tbl_fan_set_fan_field(tbl_fan_t *p_fan, tbl_fan_field_id_t field_id);

tbl_fan_t*
tbl_fan_get_fan(tbl_fan_key_t *p_fan_key);

char*
tbl_fan_key_val2str(tbl_fan_t *p_fan, char *str, uint32 str_len);

int32
tbl_fan_key_str2val(char *str, tbl_fan_t *p_fan);

int32
tbl_fan_dump_one(tbl_fan_t *p_fan, tbl_iter_args_t *pargs);

char*
tbl_fan_key_name_dump(tbl_fan_t *p_fan, char* str);

char*
tbl_fan_key_value_dump(tbl_fan_t *p_fan, char* str);

char*
tbl_fan_field_name_dump(tbl_fan_t *p_fan, int32 field_id, char* str);

char*
tbl_fan_field_value_dump(tbl_fan_t *p_fan, int32 field_id, char* str);

char**
tbl_fan_table_dump(tbl_fan_t *p_fan, char** str);

int32
tbl_fan_table_parser(char** array, char* key_value, tbl_fan_t *p_fan);

int32
tbl_fan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fan_master_t*
tbl_fan_get_master();

tbl_fan_master_t*
tbl_fan_init_fan();

#endif /* !__TBL_FAN_H__ */

