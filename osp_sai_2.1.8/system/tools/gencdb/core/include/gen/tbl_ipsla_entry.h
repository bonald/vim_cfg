
#ifndef __TBL_IPSLA_ENTRY_H__
#define __TBL_IPSLA_ENTRY_H__

int32
tbl_ipsla_entry_add_ipsla_entry_sync(tbl_ipsla_entry_t *p_ipsla_entry, uint32 sync);

int32
tbl_ipsla_entry_del_ipsla_entry_sync(tbl_ipsla_entry_key_t *p_ipsla_entry_key, uint32 sync);

int32
tbl_ipsla_entry_set_ipsla_entry_field_sync(tbl_ipsla_entry_t *p_ipsla_entry, tbl_ipsla_entry_field_id_t field_id, uint32 sync);

int32
tbl_ipsla_entry_add_ipsla_entry(tbl_ipsla_entry_t *p_ipsla_entry);

int32
tbl_ipsla_entry_del_ipsla_entry(tbl_ipsla_entry_key_t *p_ipsla_entry_key);

int32
tbl_ipsla_entry_set_ipsla_entry_field(tbl_ipsla_entry_t *p_ipsla_entry, tbl_ipsla_entry_field_id_t field_id);

tbl_ipsla_entry_t*
tbl_ipsla_entry_get_ipsla_entry(tbl_ipsla_entry_key_t *p_ipsla_entry_key);

char*
tbl_ipsla_entry_key_val2str(tbl_ipsla_entry_t *p_ipsla_entry, char *str, uint32 str_len);

int32
tbl_ipsla_entry_key_str2val(char *str, tbl_ipsla_entry_t *p_ipsla_entry);

int32
tbl_ipsla_entry_dump_one(tbl_ipsla_entry_t *p_ipsla_entry, tbl_iter_args_t *pargs);

char*
tbl_ipsla_entry_key_name_dump(tbl_ipsla_entry_t *p_ipsla_entry, char* str);

char*
tbl_ipsla_entry_key_value_dump(tbl_ipsla_entry_t *p_ipsla_entry, char* str);

char*
tbl_ipsla_entry_field_name_dump(tbl_ipsla_entry_t *p_ipsla_entry, int32 field_id, char* str);

char*
tbl_ipsla_entry_field_value_dump(tbl_ipsla_entry_t *p_ipsla_entry, int32 field_id, char* str);

char**
tbl_ipsla_entry_table_dump(tbl_ipsla_entry_t *p_ipsla_entry, char** str);

int32
tbl_ipsla_entry_table_parser(char** array, char* key_value, tbl_ipsla_entry_t *p_ipsla_entry);

int32
tbl_ipsla_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipsla_entry_master_t*
tbl_ipsla_entry_get_master();

tbl_ipsla_entry_master_t*
tbl_ipsla_entry_init_ipsla_entry();

#endif /* !__TBL_IPSLA_ENTRY_H__ */

