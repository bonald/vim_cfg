
#ifndef __TBL_SENSOR_H__
#define __TBL_SENSOR_H__

int32
tbl_sensor_add_sensor_sync(tbl_sensor_t *p_sensor, uint32 sync);

int32
tbl_sensor_del_sensor_sync(tbl_sensor_key_t *p_sensor_key, uint32 sync);

int32
tbl_sensor_set_sensor_field_sync(tbl_sensor_t *p_sensor, tbl_sensor_field_id_t field_id, uint32 sync);

int32
tbl_sensor_add_sensor(tbl_sensor_t *p_sensor);

int32
tbl_sensor_del_sensor(tbl_sensor_key_t *p_sensor_key);

int32
tbl_sensor_set_sensor_field(tbl_sensor_t *p_sensor, tbl_sensor_field_id_t field_id);

tbl_sensor_t*
tbl_sensor_get_sensor(tbl_sensor_key_t *p_sensor_key);

char*
tbl_sensor_key_val2str(tbl_sensor_t *p_sensor, char *str, uint32 str_len);

int32
tbl_sensor_key_str2val(char *str, tbl_sensor_t *p_sensor);

int32
tbl_sensor_dump_one(tbl_sensor_t *p_sensor, tbl_iter_args_t *pargs);

char*
tbl_sensor_key_name_dump(tbl_sensor_t *p_sensor, char* str);

char*
tbl_sensor_key_value_dump(tbl_sensor_t *p_sensor, char* str);

char*
tbl_sensor_field_name_dump(tbl_sensor_t *p_sensor, int32 field_id, char* str);

char*
tbl_sensor_field_value_dump(tbl_sensor_t *p_sensor, int32 field_id, char* str);

char**
tbl_sensor_table_dump(tbl_sensor_t *p_sensor, char** str);

int32
tbl_sensor_table_parser(char** array, char* key_value, tbl_sensor_t *p_sensor);

int32
tbl_sensor_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_sensor_master_t*
tbl_sensor_get_master();

tbl_sensor_master_t*
tbl_sensor_init_sensor();

#endif /* !__TBL_SENSOR_H__ */

