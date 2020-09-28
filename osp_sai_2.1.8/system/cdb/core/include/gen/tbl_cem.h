
#ifndef __TBL_CEM_H__
#define __TBL_CEM_H__

int32
tbl_cem_add_cem_sync(tbl_cem_t *p_cem, uint32 sync);

int32
tbl_cem_del_cem_sync(tbl_cem_key_t *p_cem_key, uint32 sync);

int32
tbl_cem_set_cem_field_sync(tbl_cem_t *p_cem, tbl_cem_field_id_t field_id, uint32 sync);

int32
tbl_cem_add_cem(tbl_cem_t *p_cem);

int32
tbl_cem_del_cem(tbl_cem_key_t *p_cem_key);

int32
tbl_cem_set_cem_field(tbl_cem_t *p_cem, tbl_cem_field_id_t field_id);

tbl_cem_t*
tbl_cem_get_cem(tbl_cem_key_t *p_cem_key);

char*
tbl_cem_key_val2str(tbl_cem_t *p_cem, char *str, uint32 str_len);

int32
tbl_cem_key_str2val(char *str, tbl_cem_t *p_cem);

int32
tbl_cem_dump_one(tbl_cem_t *p_cem, tbl_iter_args_t *pargs);

char*
tbl_cem_key_name_dump(tbl_cem_t *p_cem, char* str);

char*
tbl_cem_key_value_dump(tbl_cem_t *p_cem, char* str);

char*
tbl_cem_field_name_dump(tbl_cem_t *p_cem, int32 field_id, char* str);

char*
tbl_cem_field_value_dump(tbl_cem_t *p_cem, int32 field_id, char* str);

char**
tbl_cem_table_dump(tbl_cem_t *p_cem, char** str);

int32
tbl_cem_table_parser(char** array, char* key_value, tbl_cem_t *p_cem);

int32
tbl_cem_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cem_master_t*
tbl_cem_get_master();

tbl_cem_master_t*
tbl_cem_init_cem();

#endif /* !__TBL_CEM_H__ */

