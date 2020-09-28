
#ifndef __TBL_ERRDISABLE_FLAP_H__
#define __TBL_ERRDISABLE_FLAP_H__

int32
tbl_errdisable_flap_add_errdisable_flap_sync(tbl_errdisable_flap_t *p_errdisable_flap, uint32 sync);

int32
tbl_errdisable_flap_del_errdisable_flap_sync(tbl_errdisable_flap_key_t *p_errdisable_flap_key, uint32 sync);

int32
tbl_errdisable_flap_set_errdisable_flap_field_sync(tbl_errdisable_flap_t *p_errdisable_flap, tbl_errdisable_flap_field_id_t field_id, uint32 sync);

int32
tbl_errdisable_flap_add_errdisable_flap(tbl_errdisable_flap_t *p_errdisable_flap);

int32
tbl_errdisable_flap_del_errdisable_flap(tbl_errdisable_flap_key_t *p_errdisable_flap_key);

int32
tbl_errdisable_flap_set_errdisable_flap_field(tbl_errdisable_flap_t *p_errdisable_flap, tbl_errdisable_flap_field_id_t field_id);

tbl_errdisable_flap_t*
tbl_errdisable_flap_get_errdisable_flap(tbl_errdisable_flap_key_t *p_errdisable_flap_key);

char*
tbl_errdisable_flap_key_val2str(tbl_errdisable_flap_t *p_errdisable_flap, char *str, uint32 str_len);

int32
tbl_errdisable_flap_key_str2val(char *str, tbl_errdisable_flap_t *p_errdisable_flap);

int32
tbl_errdisable_flap_dump_one(tbl_errdisable_flap_t *p_errdisable_flap, tbl_iter_args_t *pargs);

char*
tbl_errdisable_flap_key_name_dump(tbl_errdisable_flap_t *p_errdisable_flap, char* str);

char*
tbl_errdisable_flap_key_value_dump(tbl_errdisable_flap_t *p_errdisable_flap, char* str);

char*
tbl_errdisable_flap_field_name_dump(tbl_errdisable_flap_t *p_errdisable_flap, int32 field_id, char* str);

char*
tbl_errdisable_flap_field_value_dump(tbl_errdisable_flap_t *p_errdisable_flap, int32 field_id, char* str);

char**
tbl_errdisable_flap_table_dump(tbl_errdisable_flap_t *p_errdisable_flap, char** str);

int32
tbl_errdisable_flap_table_parser(char** array, char* key_value, tbl_errdisable_flap_t *p_errdisable_flap);

int32
tbl_errdisable_flap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_errdisable_flap_master_t*
tbl_errdisable_flap_get_master();

tbl_errdisable_flap_master_t*
tbl_errdisable_flap_init_errdisable_flap();

#endif /* !__TBL_ERRDISABLE_FLAP_H__ */

