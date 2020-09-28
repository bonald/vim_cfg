
#ifndef __TBL_MACFILTER_H__
#define __TBL_MACFILTER_H__

int32
tbl_macfilter_add_macfilter_sync(tbl_macfilter_t *p_macflt, uint32 sync);

int32
tbl_macfilter_del_macfilter_sync(tbl_macfilter_key_t *p_macflt_key, uint32 sync);

int32
tbl_macfilter_set_macfilter_field_sync(tbl_macfilter_t *p_macflt, tbl_macfilter_field_id_t field_id, uint32 sync);

int32
tbl_macfilter_add_macfilter(tbl_macfilter_t *p_macflt);

int32
tbl_macfilter_del_macfilter(tbl_macfilter_key_t *p_macflt_key);

int32
tbl_macfilter_set_macfilter_field(tbl_macfilter_t *p_macflt, tbl_macfilter_field_id_t field_id);

tbl_macfilter_t*
tbl_macfilter_get_macfilter(tbl_macfilter_key_t *p_macflt_key);

char*
tbl_macfilter_key_val2str(tbl_macfilter_t *p_macflt, char *str, uint32 str_len);

int32
tbl_macfilter_key_str2val(char *str, tbl_macfilter_t *p_macflt);

int32
tbl_macfilter_dump_one(tbl_macfilter_t *p_macflt, tbl_iter_args_t *pargs);

char*
tbl_macfilter_key_name_dump(tbl_macfilter_t *p_macflt, char* str);

char*
tbl_macfilter_key_value_dump(tbl_macfilter_t *p_macflt, char* str);

char*
tbl_macfilter_field_name_dump(tbl_macfilter_t *p_macflt, int32 field_id, char* str);

char*
tbl_macfilter_field_value_dump(tbl_macfilter_t *p_macflt, int32 field_id, char* str);

char**
tbl_macfilter_table_dump(tbl_macfilter_t *p_macflt, char** str);

int32
tbl_macfilter_table_parser(char** array, char* key_value, tbl_macfilter_t *p_macflt);

int32
tbl_macfilter_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_macfilter_master_t*
tbl_macfilter_get_master();

tbl_macfilter_master_t*
tbl_macfilter_init_macfilter();

#endif /* !__TBL_MACFILTER_H__ */

