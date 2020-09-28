
#ifndef __TBL_OPM_DEBUG_H__
#define __TBL_OPM_DEBUG_H__

int32
tbl_opm_debug_set_opm_debug_field_sync(tbl_opm_debug_t *p_opmdbg, tbl_opm_debug_field_id_t field_id, uint32 sync);

int32
tbl_opm_debug_set_opm_debug_field(tbl_opm_debug_t *p_opmdbg, tbl_opm_debug_field_id_t field_id);

tbl_opm_debug_t*
tbl_opm_debug_get_opm_debug();

char*
tbl_opm_debug_key_val2str(tbl_opm_debug_t *p_opmdbg, char *str, uint32 str_len);

int32
tbl_opm_debug_key_str2val(char *str, tbl_opm_debug_t *p_opmdbg);

int32
tbl_opm_debug_dump_one(tbl_opm_debug_t *p_opmdbg, tbl_iter_args_t *pargs);

char*
tbl_opm_debug_key_name_dump(tbl_opm_debug_t *p_opmdbg, char* str);

char*
tbl_opm_debug_key_value_dump(tbl_opm_debug_t *p_opmdbg, char* str);

char*
tbl_opm_debug_field_name_dump(tbl_opm_debug_t *p_opmdbg, int32 field_id, char* str);

char*
tbl_opm_debug_field_value_dump(tbl_opm_debug_t *p_opmdbg, int32 field_id, char* str);

char**
tbl_opm_debug_table_dump(tbl_opm_debug_t *p_opmdbg, char** str);

int32
tbl_opm_debug_table_parser(char** array, char* key_value, tbl_opm_debug_t *p_opmdbg);

int32
tbl_opm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_opm_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_opm_debug_t*
tbl_opm_debug_init_opm_debug();

#endif /* !__TBL_OPM_DEBUG_H__ */

