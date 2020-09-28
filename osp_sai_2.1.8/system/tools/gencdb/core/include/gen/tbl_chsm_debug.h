
#ifndef __TBL_CHSM_DEBUG_H__
#define __TBL_CHSM_DEBUG_H__

int32
tbl_chsm_debug_set_chsm_debug_field_sync(tbl_chsm_debug_t *p_chsmdbg, tbl_chsm_debug_field_id_t field_id, uint32 sync);

int32
tbl_chsm_debug_set_chsm_debug_field(tbl_chsm_debug_t *p_chsmdbg, tbl_chsm_debug_field_id_t field_id);

tbl_chsm_debug_t*
tbl_chsm_debug_get_chsm_debug();

char*
tbl_chsm_debug_key_val2str(tbl_chsm_debug_t *p_chsmdbg, char *str, uint32 str_len);

int32
tbl_chsm_debug_key_str2val(char *str, tbl_chsm_debug_t *p_chsmdbg);

int32
tbl_chsm_debug_dump_one(tbl_chsm_debug_t *p_chsmdbg, tbl_iter_args_t *pargs);

char*
tbl_chsm_debug_key_name_dump(tbl_chsm_debug_t *p_chsmdbg, char* str);

char*
tbl_chsm_debug_key_value_dump(tbl_chsm_debug_t *p_chsmdbg, char* str);

char*
tbl_chsm_debug_field_name_dump(tbl_chsm_debug_t *p_chsmdbg, int32 field_id, char* str);

char*
tbl_chsm_debug_field_value_dump(tbl_chsm_debug_t *p_chsmdbg, int32 field_id, char* str);

char**
tbl_chsm_debug_table_dump(tbl_chsm_debug_t *p_chsmdbg, char** str);

int32
tbl_chsm_debug_table_parser(char** array, char* key_value, tbl_chsm_debug_t *p_chsmdbg);

int32
tbl_chsm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_chsm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_chsm_debug_t*
tbl_chsm_debug_init_chsm_debug();

#endif /* !__TBL_CHSM_DEBUG_H__ */

