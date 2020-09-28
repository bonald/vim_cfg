
#ifndef __TBL_DOT1X_RADIUS_H__
#define __TBL_DOT1X_RADIUS_H__

int32
tbl_dot1x_radius_add_dot1x_radius_sync(tbl_dot1x_radius_t *p_dot1x_radius, uint32 sync);

int32
tbl_dot1x_radius_del_dot1x_radius_sync(tbl_dot1x_radius_key_t *p_dot1x_radius_key, uint32 sync);

int32
tbl_dot1x_radius_set_dot1x_radius_field_sync(tbl_dot1x_radius_t *p_dot1x_radius, tbl_dot1x_radius_field_id_t field_id, uint32 sync);

int32
tbl_dot1x_radius_add_dot1x_radius(tbl_dot1x_radius_t *p_dot1x_radius);

int32
tbl_dot1x_radius_del_dot1x_radius(tbl_dot1x_radius_key_t *p_dot1x_radius_key);

int32
tbl_dot1x_radius_set_dot1x_radius_field(tbl_dot1x_radius_t *p_dot1x_radius, tbl_dot1x_radius_field_id_t field_id);

tbl_dot1x_radius_t*
tbl_dot1x_radius_get_dot1x_radius(tbl_dot1x_radius_key_t *p_dot1x_radius_key);

char*
tbl_dot1x_radius_key_val2str(tbl_dot1x_radius_t *p_dot1x_radius, char *str, uint32 str_len);

int32
tbl_dot1x_radius_key_str2val(char *str, tbl_dot1x_radius_t *p_dot1x_radius);

int32
tbl_dot1x_radius_dump_one(tbl_dot1x_radius_t *p_dot1x_radius, tbl_iter_args_t *pargs);

char*
tbl_dot1x_radius_key_name_dump(tbl_dot1x_radius_t *p_dot1x_radius, char* str);

char*
tbl_dot1x_radius_key_value_dump(tbl_dot1x_radius_t *p_dot1x_radius, char* str);

char*
tbl_dot1x_radius_field_name_dump(tbl_dot1x_radius_t *p_dot1x_radius, int32 field_id, char* str);

char*
tbl_dot1x_radius_field_value_dump(tbl_dot1x_radius_t *p_dot1x_radius, int32 field_id, char* str);

char**
tbl_dot1x_radius_table_dump(tbl_dot1x_radius_t *p_dot1x_radius, char** str);

int32
tbl_dot1x_radius_table_parser(char** array, char* key_value, tbl_dot1x_radius_t *p_dot1x_radius);

int32
tbl_dot1x_radius_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dot1x_radius_master_t*
tbl_dot1x_radius_get_master();

tbl_dot1x_radius_master_t*
tbl_dot1x_radius_init_dot1x_radius();

#endif /* !__TBL_DOT1X_RADIUS_H__ */

