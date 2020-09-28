
#ifndef __TBL_RMON_ALARM_H__
#define __TBL_RMON_ALARM_H__

int32
tbl_rmon_alarm_add_rmon_alarm_sync(tbl_rmon_alarm_t *p_rmon_alarm, uint32 sync);

int32
tbl_rmon_alarm_del_rmon_alarm_sync(tbl_rmon_alarm_key_t *p_rmon_alarm_key, uint32 sync);

int32
tbl_rmon_alarm_set_rmon_alarm_field_sync(tbl_rmon_alarm_t *p_rmon_alarm, tbl_rmon_alarm_field_id_t field_id, uint32 sync);

int32
tbl_rmon_alarm_add_rmon_alarm(tbl_rmon_alarm_t *p_rmon_alarm);

int32
tbl_rmon_alarm_del_rmon_alarm(tbl_rmon_alarm_key_t *p_rmon_alarm_key);

int32
tbl_rmon_alarm_set_rmon_alarm_field(tbl_rmon_alarm_t *p_rmon_alarm, tbl_rmon_alarm_field_id_t field_id);

tbl_rmon_alarm_t*
tbl_rmon_alarm_get_rmon_alarm(tbl_rmon_alarm_key_t *p_rmon_alarm_key);

char*
tbl_rmon_alarm_key_val2str(tbl_rmon_alarm_t *p_rmon_alarm, char *str, uint32 str_len);

int32
tbl_rmon_alarm_key_str2val(char *str, tbl_rmon_alarm_t *p_rmon_alarm);

int32
tbl_rmon_alarm_dump_one(tbl_rmon_alarm_t *p_rmon_alarm, tbl_iter_args_t *pargs);

char*
tbl_rmon_alarm_key_name_dump(tbl_rmon_alarm_t *p_rmon_alarm, char* str);

char*
tbl_rmon_alarm_key_value_dump(tbl_rmon_alarm_t *p_rmon_alarm, char* str);

char*
tbl_rmon_alarm_field_name_dump(tbl_rmon_alarm_t *p_rmon_alarm, int32 field_id, char* str);

char*
tbl_rmon_alarm_field_value_dump(tbl_rmon_alarm_t *p_rmon_alarm, int32 field_id, char* str);

char**
tbl_rmon_alarm_table_dump(tbl_rmon_alarm_t *p_rmon_alarm, char** str);

int32
tbl_rmon_alarm_table_parser(char** array, char* key_value, tbl_rmon_alarm_t *p_rmon_alarm);

int32
tbl_rmon_alarm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rmon_alarm_master_t*
tbl_rmon_alarm_get_master();

tbl_rmon_alarm_master_t*
tbl_rmon_alarm_init_rmon_alarm();

#endif /* !__TBL_RMON_ALARM_H__ */

