
#ifndef __TBL_VERSION_H__
#define __TBL_VERSION_H__

int32
tbl_version_set_version_field_sync(tbl_version_t *p_ver, tbl_version_field_id_t field_id, uint32 sync);

int32
tbl_version_set_version_field(tbl_version_t *p_ver, tbl_version_field_id_t field_id);

tbl_version_t*
tbl_version_get_version();

char*
tbl_version_key_val2str(tbl_version_t *p_ver, char *str, uint32 str_len);

int32
tbl_version_key_str2val(char *str, tbl_version_t *p_ver);

int32
tbl_version_dump_one(tbl_version_t *p_ver, tbl_iter_args_t *pargs);

char*
tbl_version_key_name_dump(tbl_version_t *p_ver, char* str);

char*
tbl_version_key_value_dump(tbl_version_t *p_ver, char* str);

char*
tbl_version_field_name_dump(tbl_version_t *p_ver, int32 field_id, char* str);

char*
tbl_version_field_value_dump(tbl_version_t *p_ver, int32 field_id, char* str);

char**
tbl_version_table_dump(tbl_version_t *p_ver, char** str);

int32
tbl_version_table_parser(char** array, char* key_value, tbl_version_t *p_ver);

int32
tbl_version_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_version_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_version_t*
tbl_version_init_version();

#endif /* !__TBL_VERSION_H__ */

