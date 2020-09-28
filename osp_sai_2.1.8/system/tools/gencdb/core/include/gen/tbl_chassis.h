
#ifndef __TBL_CHASSIS_H__
#define __TBL_CHASSIS_H__

int32
tbl_chassis_set_chassis_field_sync(tbl_chassis_t *p_chassis, tbl_chassis_field_id_t field_id, uint32 sync);

int32
tbl_chassis_set_chassis_field(tbl_chassis_t *p_chassis, tbl_chassis_field_id_t field_id);

tbl_chassis_t*
tbl_chassis_get_chassis();

char*
tbl_chassis_key_val2str(tbl_chassis_t *p_chassis, char *str, uint32 str_len);

int32
tbl_chassis_key_str2val(char *str, tbl_chassis_t *p_chassis);

int32
tbl_chassis_dump_one(tbl_chassis_t *p_chassis, tbl_iter_args_t *pargs);

char*
tbl_chassis_key_name_dump(tbl_chassis_t *p_chassis, char* str);

char*
tbl_chassis_key_value_dump(tbl_chassis_t *p_chassis, char* str);

char*
tbl_chassis_field_name_dump(tbl_chassis_t *p_chassis, int32 field_id, char* str);

char*
tbl_chassis_field_value_dump(tbl_chassis_t *p_chassis, int32 field_id, char* str);

char**
tbl_chassis_table_dump(tbl_chassis_t *p_chassis, char** str);

int32
tbl_chassis_table_parser(char** array, char* key_value, tbl_chassis_t *p_chassis);

int32
tbl_chassis_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_chassis_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_chassis_t*
tbl_chassis_init_chassis();

#endif /* !__TBL_CHASSIS_H__ */

