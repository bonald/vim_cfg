
#ifndef __TBL_RMON_EVENT_H__
#define __TBL_RMON_EVENT_H__

int32
tbl_rmon_event_add_rmon_event_sync(tbl_rmon_event_t *p_rmon_event, uint32 sync);

int32
tbl_rmon_event_del_rmon_event_sync(tbl_rmon_event_key_t *p_rmon_event_key, uint32 sync);

int32
tbl_rmon_event_set_rmon_event_field_sync(tbl_rmon_event_t *p_rmon_event, tbl_rmon_event_field_id_t field_id, uint32 sync);

int32
tbl_rmon_event_add_rmon_event(tbl_rmon_event_t *p_rmon_event);

int32
tbl_rmon_event_del_rmon_event(tbl_rmon_event_key_t *p_rmon_event_key);

int32
tbl_rmon_event_set_rmon_event_field(tbl_rmon_event_t *p_rmon_event, tbl_rmon_event_field_id_t field_id);

tbl_rmon_event_t*
tbl_rmon_event_get_rmon_event(tbl_rmon_event_key_t *p_rmon_event_key);

char*
tbl_rmon_event_key_val2str(tbl_rmon_event_t *p_rmon_event, char *str, uint32 str_len);

int32
tbl_rmon_event_key_str2val(char *str, tbl_rmon_event_t *p_rmon_event);

int32
tbl_rmon_event_dump_one(tbl_rmon_event_t *p_rmon_event, tbl_iter_args_t *pargs);

char*
tbl_rmon_event_key_name_dump(tbl_rmon_event_t *p_rmon_event, char* str);

char*
tbl_rmon_event_key_value_dump(tbl_rmon_event_t *p_rmon_event, char* str);

char*
tbl_rmon_event_field_name_dump(tbl_rmon_event_t *p_rmon_event, int32 field_id, char* str);

char*
tbl_rmon_event_field_value_dump(tbl_rmon_event_t *p_rmon_event, int32 field_id, char* str);

char**
tbl_rmon_event_table_dump(tbl_rmon_event_t *p_rmon_event, char** str);

int32
tbl_rmon_event_table_parser(char** array, char* key_value, tbl_rmon_event_t *p_rmon_event);

int32
tbl_rmon_event_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rmon_event_master_t*
tbl_rmon_event_get_master();

tbl_rmon_event_master_t*
tbl_rmon_event_init_rmon_event();

#endif /* !__TBL_RMON_EVENT_H__ */

