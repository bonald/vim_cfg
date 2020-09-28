
#ifndef __TBL_SSM_DEBUG_H__
#define __TBL_SSM_DEBUG_H__

int32
tbl_ssm_debug_set_ssm_debug_field_sync(tbl_ssm_debug_t *p_ssmdbg, tbl_ssm_debug_field_id_t field_id, uint32 sync);

int32
tbl_ssm_debug_set_ssm_debug_field(tbl_ssm_debug_t *p_ssmdbg, tbl_ssm_debug_field_id_t field_id);

tbl_ssm_debug_t*
tbl_ssm_debug_get_ssm_debug();

char*
tbl_ssm_debug_key_val2str(tbl_ssm_debug_t *p_ssmdbg, char *str, uint32 str_len);

int32
tbl_ssm_debug_key_str2val(char *str, tbl_ssm_debug_t *p_ssmdbg);

int32
tbl_ssm_debug_dump_one(tbl_ssm_debug_t *p_ssmdbg, tbl_iter_args_t *pargs);

char*
tbl_ssm_debug_key_name_dump(tbl_ssm_debug_t *p_ssmdbg, char* str);

char*
tbl_ssm_debug_key_value_dump(tbl_ssm_debug_t *p_ssmdbg, char* str);

char*
tbl_ssm_debug_field_name_dump(tbl_ssm_debug_t *p_ssmdbg, int32 field_id, char* str);

char*
tbl_ssm_debug_field_value_dump(tbl_ssm_debug_t *p_ssmdbg, int32 field_id, char* str);

char**
tbl_ssm_debug_table_dump(tbl_ssm_debug_t *p_ssmdbg, char** str);

int32
tbl_ssm_debug_table_parser(char** array, char* key_value, tbl_ssm_debug_t *p_ssmdbg);

int32
tbl_ssm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ssm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ssm_debug_t*
tbl_ssm_debug_init_ssm_debug();

#endif /* !__TBL_SSM_DEBUG_H__ */

