
#ifndef __TBL_CFM_GLOBAL_H__
#define __TBL_CFM_GLOBAL_H__

int32
tbl_cfm_global_set_cfm_global_field_sync(tbl_cfm_global_t *p_cfm_glb, tbl_cfm_global_field_id_t field_id, uint32 sync);

int32
tbl_cfm_global_set_cfm_global_field(tbl_cfm_global_t *p_cfm_glb, tbl_cfm_global_field_id_t field_id);

tbl_cfm_global_t*
tbl_cfm_global_get_cfm_global();

char*
tbl_cfm_global_key_val2str(tbl_cfm_global_t *p_cfm_glb, char *str, uint32 str_len);

int32
tbl_cfm_global_key_str2val(char *str, tbl_cfm_global_t *p_cfm_glb);

int32
tbl_cfm_global_dump_one(tbl_cfm_global_t *p_cfm_glb, tbl_iter_args_t *pargs);

char*
tbl_cfm_global_key_name_dump(tbl_cfm_global_t *p_cfm_glb, char* str);

char*
tbl_cfm_global_key_value_dump(tbl_cfm_global_t *p_cfm_glb, char* str);

char*
tbl_cfm_global_field_name_dump(tbl_cfm_global_t *p_cfm_glb, int32 field_id, char* str);

char*
tbl_cfm_global_field_value_dump(tbl_cfm_global_t *p_cfm_glb, int32 field_id, char* str);

char**
tbl_cfm_global_table_dump(tbl_cfm_global_t *p_cfm_glb, char** str);

int32
tbl_cfm_global_table_parser(char** array, char* key_value, tbl_cfm_global_t *p_cfm_glb);

int32
tbl_cfm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_cfm_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_global_t*
tbl_cfm_global_init_cfm_global();

#endif /* !__TBL_CFM_GLOBAL_H__ */

