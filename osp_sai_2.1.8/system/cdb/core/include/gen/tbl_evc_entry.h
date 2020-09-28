
#ifndef __TBL_EVC_ENTRY_H__
#define __TBL_EVC_ENTRY_H__

int32
tbl_evc_entry_add_evc_entry_sync(tbl_evc_entry_t *p_evc_entry, uint32 sync);

int32
tbl_evc_entry_del_evc_entry_sync(tbl_evc_entry_key_t *p_evc_entry_key, uint32 sync);

int32
tbl_evc_entry_set_evc_entry_field_sync(tbl_evc_entry_t *p_evc_entry, tbl_evc_entry_field_id_t field_id, uint32 sync);

int32
tbl_evc_entry_add_evc_entry(tbl_evc_entry_t *p_evc_entry);

int32
tbl_evc_entry_del_evc_entry(tbl_evc_entry_key_t *p_evc_entry_key);

int32
tbl_evc_entry_set_evc_entry_field(tbl_evc_entry_t *p_evc_entry, tbl_evc_entry_field_id_t field_id);

tbl_evc_entry_t*
tbl_evc_entry_get_evc_entry(tbl_evc_entry_key_t *p_evc_entry_key);

char*
tbl_evc_entry_key_val2str(tbl_evc_entry_t *p_evc_entry, char *str, uint32 str_len);

int32
tbl_evc_entry_key_str2val(char *str, tbl_evc_entry_t *p_evc_entry);

int32
tbl_evc_entry_dump_one(tbl_evc_entry_t *p_evc_entry, tbl_iter_args_t *pargs);

char*
tbl_evc_entry_key_name_dump(tbl_evc_entry_t *p_evc_entry, char* str);

char*
tbl_evc_entry_key_value_dump(tbl_evc_entry_t *p_evc_entry, char* str);

char*
tbl_evc_entry_field_name_dump(tbl_evc_entry_t *p_evc_entry, int32 field_id, char* str);

char*
tbl_evc_entry_field_value_dump(tbl_evc_entry_t *p_evc_entry, int32 field_id, char* str);

char**
tbl_evc_entry_table_dump(tbl_evc_entry_t *p_evc_entry, char** str);

int32
tbl_evc_entry_table_parser(char** array, char* key_value, tbl_evc_entry_t *p_evc_entry);

int32
tbl_evc_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_evc_entry_master_t*
tbl_evc_entry_get_master();

tbl_evc_entry_master_t*
tbl_evc_entry_init_evc_entry();

#endif /* !__TBL_EVC_ENTRY_H__ */

