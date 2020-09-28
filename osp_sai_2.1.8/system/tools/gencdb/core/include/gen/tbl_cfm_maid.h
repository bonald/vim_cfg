
#ifndef __TBL_CFM_MAID_H__
#define __TBL_CFM_MAID_H__

int32
tbl_cfm_maid_add_cfm_maid_sync(tbl_cfm_maid_t *p_maid, uint32 sync);

int32
tbl_cfm_maid_del_cfm_maid_sync(tbl_cfm_maid_key_t *p_maid_key, uint32 sync);

int32
tbl_cfm_maid_set_cfm_maid_field_sync(tbl_cfm_maid_t *p_maid, tbl_cfm_maid_field_id_t field_id, uint32 sync);

int32
tbl_cfm_maid_add_cfm_maid(tbl_cfm_maid_t *p_maid);

int32
tbl_cfm_maid_del_cfm_maid(tbl_cfm_maid_key_t *p_maid_key);

int32
tbl_cfm_maid_set_cfm_maid_field(tbl_cfm_maid_t *p_maid, tbl_cfm_maid_field_id_t field_id);

tbl_cfm_maid_t*
tbl_cfm_maid_get_cfm_maid(tbl_cfm_maid_key_t *p_maid_key);

char*
tbl_cfm_maid_key_val2str(tbl_cfm_maid_t *p_maid, char *str, uint32 str_len);

int32
tbl_cfm_maid_key_str2val(char *str, tbl_cfm_maid_t *p_maid);

int32
tbl_cfm_maid_dump_one(tbl_cfm_maid_t *p_maid, tbl_iter_args_t *pargs);

char*
tbl_cfm_maid_key_name_dump(tbl_cfm_maid_t *p_maid, char* str);

char*
tbl_cfm_maid_key_value_dump(tbl_cfm_maid_t *p_maid, char* str);

char*
tbl_cfm_maid_field_name_dump(tbl_cfm_maid_t *p_maid, int32 field_id, char* str);

char*
tbl_cfm_maid_field_value_dump(tbl_cfm_maid_t *p_maid, int32 field_id, char* str);

char**
tbl_cfm_maid_table_dump(tbl_cfm_maid_t *p_maid, char** str);

int32
tbl_cfm_maid_table_parser(char** array, char* key_value, tbl_cfm_maid_t *p_maid);

int32
tbl_cfm_maid_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_maid_master_t*
tbl_cfm_maid_get_master();

tbl_cfm_maid_master_t*
tbl_cfm_maid_init_cfm_maid();

#endif /* !__TBL_CFM_MAID_H__ */

