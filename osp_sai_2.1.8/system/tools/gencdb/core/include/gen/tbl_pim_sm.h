
#ifndef __TBL_PIM_SM_H__
#define __TBL_PIM_SM_H__

int32
tbl_pim_sm_add_pim_sm_sync(tbl_pim_sm_t *p_pim_sm, uint32 sync);

int32
tbl_pim_sm_del_pim_sm_sync(tbl_pim_sm_key_t *p_pim_sm_key, uint32 sync);

int32
tbl_pim_sm_set_pim_sm_field_sync(tbl_pim_sm_t *p_pim_sm, tbl_pim_sm_field_id_t field_id, uint32 sync);

int32
tbl_pim_sm_add_pim_sm(tbl_pim_sm_t *p_pim_sm);

int32
tbl_pim_sm_del_pim_sm(tbl_pim_sm_key_t *p_pim_sm_key);

int32
tbl_pim_sm_set_pim_sm_field(tbl_pim_sm_t *p_pim_sm, tbl_pim_sm_field_id_t field_id);

tbl_pim_sm_t*
tbl_pim_sm_get_pim_sm(tbl_pim_sm_key_t *p_pim_sm_key);

char*
tbl_pim_sm_key_val2str(tbl_pim_sm_t *p_pim_sm, char *str, uint32 str_len);

int32
tbl_pim_sm_key_str2val(char *str, tbl_pim_sm_t *p_pim_sm);

int32
tbl_pim_sm_dump_one(tbl_pim_sm_t *p_pim_sm, tbl_iter_args_t *pargs);

char*
tbl_pim_sm_key_name_dump(tbl_pim_sm_t *p_pim_sm, char* str);

char*
tbl_pim_sm_key_value_dump(tbl_pim_sm_t *p_pim_sm, char* str);

char*
tbl_pim_sm_field_name_dump(tbl_pim_sm_t *p_pim_sm, int32 field_id, char* str);

char*
tbl_pim_sm_field_value_dump(tbl_pim_sm_t *p_pim_sm, int32 field_id, char* str);

char**
tbl_pim_sm_table_dump(tbl_pim_sm_t *p_pim_sm, char** str);

int32
tbl_pim_sm_table_parser(char** array, char* key_value, tbl_pim_sm_t *p_pim_sm);

int32
tbl_pim_sm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pim_sm_master_t*
tbl_pim_sm_get_master();

tbl_pim_sm_master_t*
tbl_pim_sm_init_pim_sm();

#endif /* !__TBL_PIM_SM_H__ */

