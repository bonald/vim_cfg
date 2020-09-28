
#ifndef __TBL_EGRESS_MAPPING_ENTRY_H__
#define __TBL_EGRESS_MAPPING_ENTRY_H__

int32
tbl_egress_mapping_entry_add_egress_mapping_entry_sync(tbl_egress_mapping_entry_t *p_egress_mapping_entry, uint32 sync);

int32
tbl_egress_mapping_entry_del_egress_mapping_entry_sync(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key, uint32 sync);

int32
tbl_egress_mapping_entry_set_egress_mapping_entry_field_sync(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_egress_mapping_entry_field_id_t field_id, uint32 sync);

int32
tbl_egress_mapping_entry_add_egress_mapping_entry(tbl_egress_mapping_entry_t *p_egress_mapping_entry);

int32
tbl_egress_mapping_entry_del_egress_mapping_entry(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key);

int32
tbl_egress_mapping_entry_set_egress_mapping_entry_field(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_egress_mapping_entry_field_id_t field_id);

tbl_egress_mapping_entry_t*
tbl_egress_mapping_entry_get_egress_mapping_entry(tbl_egress_mapping_entry_key_t *p_egress_mapping_entry_key);

char*
tbl_egress_mapping_entry_key_val2str(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char *str, uint32 str_len);

int32
tbl_egress_mapping_entry_key_str2val(char *str, tbl_egress_mapping_entry_t *p_egress_mapping_entry);

int32
tbl_egress_mapping_entry_dump_one(tbl_egress_mapping_entry_t *p_egress_mapping_entry, tbl_iter_args_t *pargs);

char*
tbl_egress_mapping_entry_key_name_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char* str);

char*
tbl_egress_mapping_entry_key_value_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char* str);

char*
tbl_egress_mapping_entry_field_name_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, int32 field_id, char* str);

char*
tbl_egress_mapping_entry_field_value_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, int32 field_id, char* str);

char**
tbl_egress_mapping_entry_table_dump(tbl_egress_mapping_entry_t *p_egress_mapping_entry, char** str);

int32
tbl_egress_mapping_entry_table_parser(char** array, char* key_value, tbl_egress_mapping_entry_t *p_egress_mapping_entry);

int32
tbl_egress_mapping_entry_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_egress_mapping_entry_master_t*
tbl_egress_mapping_entry_get_master();

tbl_egress_mapping_entry_master_t*
tbl_egress_mapping_entry_init_egress_mapping_entry();

#endif /* !__TBL_EGRESS_MAPPING_ENTRY_H__ */

