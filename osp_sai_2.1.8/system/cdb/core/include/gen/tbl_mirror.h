
#ifndef __TBL_MIRROR_H__
#define __TBL_MIRROR_H__

int32
tbl_mirror_add_mirror_sync(tbl_mirror_t *p_mirror, uint32 sync);

int32
tbl_mirror_del_mirror_sync(tbl_mirror_key_t *p_mirror_key, uint32 sync);

int32
tbl_mirror_set_mirror_field_sync(tbl_mirror_t *p_mirror, tbl_mirror_field_id_t field_id, uint32 sync);

int32
tbl_mirror_add_mirror(tbl_mirror_t *p_mirror);

int32
tbl_mirror_del_mirror(tbl_mirror_key_t *p_mirror_key);

int32
tbl_mirror_set_mirror_field(tbl_mirror_t *p_mirror, tbl_mirror_field_id_t field_id);

tbl_mirror_t*
tbl_mirror_get_mirror(tbl_mirror_key_t *p_mirror_key);

char*
tbl_mirror_key_val2str(tbl_mirror_t *p_mirror, char *str, uint32 str_len);

int32
tbl_mirror_key_str2val(char *str, tbl_mirror_t *p_mirror);

int32
tbl_mirror_dump_one(tbl_mirror_t *p_mirror, tbl_iter_args_t *pargs);

char*
tbl_mirror_key_name_dump(tbl_mirror_t *p_mirror, char* str);

char*
tbl_mirror_key_value_dump(tbl_mirror_t *p_mirror, char* str);

char*
tbl_mirror_field_name_dump(tbl_mirror_t *p_mirror, int32 field_id, char* str);

char*
tbl_mirror_field_value_dump(tbl_mirror_t *p_mirror, int32 field_id, char* str);

char**
tbl_mirror_table_dump(tbl_mirror_t *p_mirror, char** str);

int32
tbl_mirror_table_parser(char** array, char* key_value, tbl_mirror_t *p_mirror);

int32
tbl_mirror_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mirror_master_t*
tbl_mirror_get_master();

tbl_mirror_master_t*
tbl_mirror_init_mirror();

extern tbl_mirror_t*
mirror_get_session(uint32 sessionid);

#endif /* !__TBL_MIRROR_H__ */

