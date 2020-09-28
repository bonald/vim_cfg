
#ifndef __TBL_OPM_GLOBAL_H__
#define __TBL_OPM_GLOBAL_H__

int32
tbl_opm_global_set_opm_global_field_sync(tbl_opm_global_t *p_opmglb, tbl_opm_global_field_id_t field_id, uint32 sync);

int32
tbl_opm_global_set_opm_global_field(tbl_opm_global_t *p_opmglb, tbl_opm_global_field_id_t field_id);

tbl_opm_global_t*
tbl_opm_global_get_opm_global();

char*
tbl_opm_global_key_val2str(tbl_opm_global_t *p_opmglb, char *str, uint32 str_len);

int32
tbl_opm_global_key_str2val(char *str, tbl_opm_global_t *p_opmglb);

int32
tbl_opm_global_dump_one(tbl_opm_global_t *p_opmglb, tbl_iter_args_t *pargs);

char*
tbl_opm_global_key_name_dump(tbl_opm_global_t *p_opmglb, char* str);

char*
tbl_opm_global_key_value_dump(tbl_opm_global_t *p_opmglb, char* str);

char*
tbl_opm_global_field_name_dump(tbl_opm_global_t *p_opmglb, int32 field_id, char* str);

char*
tbl_opm_global_field_value_dump(tbl_opm_global_t *p_opmglb, int32 field_id, char* str);

char**
tbl_opm_global_table_dump(tbl_opm_global_t *p_opmglb, char** str);

int32
tbl_opm_global_table_parser(char** array, char* key_value, tbl_opm_global_t *p_opmglb);

int32
tbl_opm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_opm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_opm_global_t*
tbl_opm_global_init_opm_global();

#endif /* !__TBL_OPM_GLOBAL_H__ */

