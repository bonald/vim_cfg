
#ifndef __TBL_PIM_DM_H__
#define __TBL_PIM_DM_H__

int32
tbl_pim_dm_add_pim_dm_sync(tbl_pim_dm_t *p_pim_dm, uint32 sync);

int32
tbl_pim_dm_del_pim_dm_sync(tbl_pim_dm_key_t *p_pim_dm_key, uint32 sync);

int32
tbl_pim_dm_set_pim_dm_field_sync(tbl_pim_dm_t *p_pim_dm, tbl_pim_dm_field_id_t field_id, uint32 sync);

int32
tbl_pim_dm_add_pim_dm(tbl_pim_dm_t *p_pim_dm);

int32
tbl_pim_dm_del_pim_dm(tbl_pim_dm_key_t *p_pim_dm_key);

int32
tbl_pim_dm_set_pim_dm_field(tbl_pim_dm_t *p_pim_dm, tbl_pim_dm_field_id_t field_id);

tbl_pim_dm_t*
tbl_pim_dm_get_pim_dm(tbl_pim_dm_key_t *p_pim_dm_key);

char*
tbl_pim_dm_key_val2str(tbl_pim_dm_t *p_pim_dm, char *str, uint32 str_len);

int32
tbl_pim_dm_key_str2val(char *str, tbl_pim_dm_t *p_pim_dm);

int32
tbl_pim_dm_dump_one(tbl_pim_dm_t *p_pim_dm, tbl_iter_args_t *pargs);

char*
tbl_pim_dm_key_name_dump(tbl_pim_dm_t *p_pim_dm, char* str);

char*
tbl_pim_dm_key_value_dump(tbl_pim_dm_t *p_pim_dm, char* str);

char*
tbl_pim_dm_field_name_dump(tbl_pim_dm_t *p_pim_dm, int32 field_id, char* str);

char*
tbl_pim_dm_field_value_dump(tbl_pim_dm_t *p_pim_dm, int32 field_id, char* str);

char**
tbl_pim_dm_table_dump(tbl_pim_dm_t *p_pim_dm, char** str);

int32
tbl_pim_dm_table_parser(char** array, char* key_value, tbl_pim_dm_t *p_pim_dm);

int32
tbl_pim_dm_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pim_dm_master_t*
tbl_pim_dm_get_master();

tbl_pim_dm_master_t*
tbl_pim_dm_init_pim_dm();

#endif /* !__TBL_PIM_DM_H__ */

