
#ifndef __TBL_CFM_LMEP_H__
#define __TBL_CFM_LMEP_H__

int32
tbl_cfm_lmep_add_cfm_lmep_sync(tbl_cfm_lmep_t *p_lmep, uint32 sync);

int32
tbl_cfm_lmep_del_cfm_lmep_sync(tbl_cfm_lmep_key_t *p_lmep_key, uint32 sync);

int32
tbl_cfm_lmep_set_cfm_lmep_field_sync(tbl_cfm_lmep_t *p_lmep, tbl_cfm_lmep_field_id_t field_id, uint32 sync);

int32
tbl_cfm_lmep_add_cfm_lmep(tbl_cfm_lmep_t *p_lmep);

int32
tbl_cfm_lmep_del_cfm_lmep(tbl_cfm_lmep_key_t *p_lmep_key);

int32
tbl_cfm_lmep_set_cfm_lmep_field(tbl_cfm_lmep_t *p_lmep, tbl_cfm_lmep_field_id_t field_id);

tbl_cfm_lmep_t*
tbl_cfm_lmep_get_cfm_lmep(tbl_cfm_lmep_key_t *p_lmep_key);

char*
tbl_cfm_lmep_key_val2str(tbl_cfm_lmep_t *p_lmep, char *str, uint32 str_len);

int32
tbl_cfm_lmep_key_str2val(char *str, tbl_cfm_lmep_t *p_lmep);

int32
tbl_cfm_lmep_dump_one(tbl_cfm_lmep_t *p_lmep, tbl_iter_args_t *pargs);

char*
tbl_cfm_lmep_key_name_dump(tbl_cfm_lmep_t *p_lmep, char* str);

char*
tbl_cfm_lmep_key_value_dump(tbl_cfm_lmep_t *p_lmep, char* str);

char*
tbl_cfm_lmep_field_name_dump(tbl_cfm_lmep_t *p_lmep, int32 field_id, char* str);

char*
tbl_cfm_lmep_field_value_dump(tbl_cfm_lmep_t *p_lmep, int32 field_id, char* str);

char**
tbl_cfm_lmep_table_dump(tbl_cfm_lmep_t *p_lmep, char** str);

int32
tbl_cfm_lmep_table_parser(char** array, char* key_value, tbl_cfm_lmep_t *p_lmep);

int32
tbl_cfm_lmep_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_lmep_master_t*
tbl_cfm_lmep_get_master();

tbl_cfm_lmep_master_t*
tbl_cfm_lmep_init_cfm_lmep();

#endif /* !__TBL_CFM_LMEP_H__ */

