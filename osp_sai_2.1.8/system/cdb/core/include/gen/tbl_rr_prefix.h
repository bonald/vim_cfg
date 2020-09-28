
#ifndef __TBL_RR_PREFIX_H__
#define __TBL_RR_PREFIX_H__

int32
tbl_rr_prefix_add_rr_prefix_sync(tbl_rr_prefix_t *p_rr_prefix, uint32 sync);

int32
tbl_rr_prefix_del_rr_prefix_sync(tbl_rr_prefix_key_t *p_rr_prefix_key, uint32 sync);

int32
tbl_rr_prefix_set_rr_prefix_field_sync(tbl_rr_prefix_t *p_rr_prefix, tbl_rr_prefix_field_id_t field_id, uint32 sync);

int32
tbl_rr_prefix_add_rr_prefix(tbl_rr_prefix_t *p_rr_prefix);

int32
tbl_rr_prefix_del_rr_prefix(tbl_rr_prefix_key_t *p_rr_prefix_key);

int32
tbl_rr_prefix_set_rr_prefix_field(tbl_rr_prefix_t *p_rr_prefix, tbl_rr_prefix_field_id_t field_id);

tbl_rr_prefix_t*
tbl_rr_prefix_get_rr_prefix(tbl_rr_prefix_key_t *p_rr_prefix_key);

char*
tbl_rr_prefix_key_val2str(tbl_rr_prefix_t *p_rr_prefix, char *str, uint32 str_len);

int32
tbl_rr_prefix_key_str2val(char *str, tbl_rr_prefix_t *p_rr_prefix);

int32
tbl_rr_prefix_dump_one(tbl_rr_prefix_t *p_rr_prefix, tbl_iter_args_t *pargs);

char*
tbl_rr_prefix_key_name_dump(tbl_rr_prefix_t *p_rr_prefix, char* str);

char*
tbl_rr_prefix_key_value_dump(tbl_rr_prefix_t *p_rr_prefix, char* str);

char*
tbl_rr_prefix_field_name_dump(tbl_rr_prefix_t *p_rr_prefix, int32 field_id, char* str);

char*
tbl_rr_prefix_field_value_dump(tbl_rr_prefix_t *p_rr_prefix, int32 field_id, char* str);

char**
tbl_rr_prefix_table_dump(tbl_rr_prefix_t *p_rr_prefix, char** str);

int32
tbl_rr_prefix_table_parser(char** array, char* key_value, tbl_rr_prefix_t *p_rr_prefix);

int32
tbl_rr_prefix_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_rr_prefix_master_t*
tbl_rr_prefix_get_master();

tbl_rr_prefix_master_t*
tbl_rr_prefix_init_rr_prefix();

#endif /* !__TBL_RR_PREFIX_H__ */

