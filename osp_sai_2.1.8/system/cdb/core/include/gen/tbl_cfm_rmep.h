
#ifndef __TBL_CFM_RMEP_H__
#define __TBL_CFM_RMEP_H__

int32
tbl_cfm_rmep_add_cfm_rmep_sync(tbl_cfm_rmep_t *p_rmep, uint32 sync);

int32
tbl_cfm_rmep_del_cfm_rmep_sync(tbl_cfm_rmep_key_t *p_rmep_key, uint32 sync);

int32
tbl_cfm_rmep_set_cfm_rmep_field_sync(tbl_cfm_rmep_t *p_rmep, tbl_cfm_rmep_field_id_t field_id, uint32 sync);

int32
tbl_cfm_rmep_add_cfm_rmep(tbl_cfm_rmep_t *p_rmep);

int32
tbl_cfm_rmep_del_cfm_rmep(tbl_cfm_rmep_key_t *p_rmep_key);

int32
tbl_cfm_rmep_set_cfm_rmep_field(tbl_cfm_rmep_t *p_rmep, tbl_cfm_rmep_field_id_t field_id);

tbl_cfm_rmep_t*
tbl_cfm_rmep_get_cfm_rmep(tbl_cfm_rmep_key_t *p_rmep_key);

char*
tbl_cfm_rmep_key_val2str(tbl_cfm_rmep_t *p_rmep, char *str, uint32 str_len);

int32
tbl_cfm_rmep_key_str2val(char *str, tbl_cfm_rmep_t *p_rmep);

int32
tbl_cfm_rmep_dump_one(tbl_cfm_rmep_t *p_rmep, tbl_iter_args_t *pargs);

char*
tbl_cfm_rmep_key_name_dump(tbl_cfm_rmep_t *p_rmep, char* str);

char*
tbl_cfm_rmep_key_value_dump(tbl_cfm_rmep_t *p_rmep, char* str);

char*
tbl_cfm_rmep_field_name_dump(tbl_cfm_rmep_t *p_rmep, int32 field_id, char* str);

char*
tbl_cfm_rmep_field_value_dump(tbl_cfm_rmep_t *p_rmep, int32 field_id, char* str);

char**
tbl_cfm_rmep_table_dump(tbl_cfm_rmep_t *p_rmep, char** str);

int32
tbl_cfm_rmep_table_parser(char** array, char* key_value, tbl_cfm_rmep_t *p_rmep);

int32
tbl_cfm_rmep_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_rmep_master_t*
tbl_cfm_rmep_get_master();

tbl_cfm_rmep_master_t*
tbl_cfm_rmep_init_cfm_rmep();

#endif /* !__TBL_CFM_RMEP_H__ */

