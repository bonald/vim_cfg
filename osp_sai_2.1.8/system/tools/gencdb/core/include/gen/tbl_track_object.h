
#ifndef __TBL_TRACK_OBJECT_H__
#define __TBL_TRACK_OBJECT_H__

int32
tbl_track_object_add_track_object_sync(tbl_track_object_t *p_track_object, uint32 sync);

int32
tbl_track_object_del_track_object_sync(tbl_track_object_key_t *p_track_object_key, uint32 sync);

int32
tbl_track_object_set_track_object_field_sync(tbl_track_object_t *p_track_object, tbl_track_object_field_id_t field_id, uint32 sync);

int32
tbl_track_object_add_track_object(tbl_track_object_t *p_track_object);

int32
tbl_track_object_del_track_object(tbl_track_object_key_t *p_track_object_key);

int32
tbl_track_object_set_track_object_field(tbl_track_object_t *p_track_object, tbl_track_object_field_id_t field_id);

tbl_track_object_t*
tbl_track_object_get_track_object(tbl_track_object_key_t *p_track_object_key);

char*
tbl_track_object_key_val2str(tbl_track_object_t *p_track_object, char *str, uint32 str_len);

int32
tbl_track_object_key_str2val(char *str, tbl_track_object_t *p_track_object);

int32
tbl_track_object_dump_one(tbl_track_object_t *p_track_object, tbl_iter_args_t *pargs);

char*
tbl_track_object_key_name_dump(tbl_track_object_t *p_track_object, char* str);

char*
tbl_track_object_key_value_dump(tbl_track_object_t *p_track_object, char* str);

char*
tbl_track_object_field_name_dump(tbl_track_object_t *p_track_object, int32 field_id, char* str);

char*
tbl_track_object_field_value_dump(tbl_track_object_t *p_track_object, int32 field_id, char* str);

char**
tbl_track_object_table_dump(tbl_track_object_t *p_track_object, char** str);

int32
tbl_track_object_table_parser(char** array, char* key_value, tbl_track_object_t *p_track_object);

int32
tbl_track_object_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_track_object_master_t*
tbl_track_object_get_master();

tbl_track_object_master_t*
tbl_track_object_init_track_object();

#endif /* !__TBL_TRACK_OBJECT_H__ */

